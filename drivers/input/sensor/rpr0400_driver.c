/******************************************************************************
 * MODULE       : rpr0400_driver.c
 * FUNCTION     : Driver source for RPR400,
 *              : Proximity Sensor(PS) and Ambient Light Sensor(ALS) IC.
 * AUTHOR       : Masafumi Seike <masafumi.seike@dsn.rohm.co.jp >
 * PROGRAMMED   : Sensor system Development Group, ROHM CO.,LTD.
 * MODIFICATION : Modified by ROHM, Mar/05/2013
 * REMARKS      :
 * COPYRIGHT    : Copyright (C) 2012 ROHM CO.,LTD.
 *              : This program is free software; you can redistribute it and/or
 *              : modify it under the terms of the GNU General Public License
 *              : as published by the Free Software Foundation; either version 2
 *              : of the License, or (at your option) any later version.
 *              :
 *              : This program is distributed in the hope that it will be useful,
 *              : but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              : MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *              : GNU General Public License for more details.
 *              :
 *              : You should have received a copy of the GNU General Public License
 *              : along with this program; if not, write to the Free Software
 *              : Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *****************************************************************************/
/*
 *  This is Linux kernel modules for ambient light + proximity sensor
 *  Revision History
 *  2012-7-19:    Ver. 1.0  New release together with a porting guide.
 *  2012-8-14:    Ver. 1.1  Added calibration and set thresholds methods. 
                            Besides, the thresholds are automatically changed if 
                            a ps int is triggered to avoid constant interrupts.
 *  2013-3-05:    Ver. 1.2  correspondence source about LGMC 
 */
#define DEBUG
#include <linux/debugfs.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/wakelock.h>

#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include CONFIG_LGE_BOARD_HEADER_FILE

 /*************** Definitions ******************/
/* GENERAL */
#define RPR400_DRV_NAME  ("rpr400")
#define DRIVER_VERSION   ("1.2")

/* RPR400 REGSTER */
#define REG_SYSTEMCONTROL         (0x40)
#define REG_MODECONTROL           (0x41)
#define REG_ALSPSCONTROL          (0x42)
#define REG_PERSISTENCE           (0x43)
#define REG_PSDATA                (0x44)
    #define REG_PSDATA_LSB            (0x44)
    #define REG_PSDATA_MBS            (0x45)
#define REG_ALSDATA0              (0x46)
    #define REG_ALSDATA0_LSB          (0x46)
    #define REG_ALSDATA0_MBS          (0x47)
#define REG_ALSDATA1              (0x48)
    #define REG_ALSDATA1_LSB          (0x48)
    #define REG_ALSDATA1_MBS          (0x49)
#define REG_INTERRUPT             (0x4A)
#define REG_PSTH                  (0x4B)
    #define REG_PSTH_LSB              (0x4B)
    #define REG_PSTH_MBS              (0x4C)
#define REG_PSTL                  (0x4D)
    #define REG_PSTL_LSB              (0x4D)
    #define REG_PSTL_MBS              (0x4E)
#define REG_ALSDATA0TH            (0x4F)
    #define REG_ALSDATA0TH_LSB       (0x4F)
    #define REG_ALSDATA0TH_MBS       (0x50)
#define REG_ALSDATA0TL            (0x51)
    #define REG_ALSDATA0TL_LSB        (0x51)
    #define REG_ALSDATA0TL_MBS        (0x52)

/* SETTINGS */
#define CUT_UNIT         (10)		//Andy 2012.6.6: it was (1000) previously. But I don't think the customer need als data with such accuracy.
#define CALC_ERROR        (0x80000000)
#define MASK_LONG         (0xFFFFFFFF)
#define MASK_CHAR         (0xFF)
#define BOTH_STANDBY	(0)
#define ALS100MS	(0x5)
#define PS100MS		(0x3)
#define BOTH100MS	(0x6)
#define LEDCURRENT_025MA    (0)
#define LEDCURRENT_050MA    (1)
#define LEDCURRENT_100MA    (2)
#define LEDCURRENT_200MA    (3)
#define ALSGAIN_X1X1        (0x0 << 2)
#define ALSGAIN_X1X2        (0x1 << 2)
#define ALSGAIN_X2X2        (0x5 << 2)
#define ALSGAIN_X64X64      (0xA << 2)
#define ALSGAIN_X128X64     (0xE << 2)
#define ALSGAIN_X128X128    (0xF << 2)
#define NORMAL_MODE         (0 << 4)
#define LOW_NOISE_MODE      (1 << 4)
#define PS_INT_MASK         (1 << 7)
#define ALS_INT_MASK        (1 << 6)
#define PS_THH_ONLY         (0 << 4)
#define PS_THH_BOTH_HYS     (1 << 4)
#define PS_THH_BOTH_OUTSIDE (2 << 4)
#define POLA_ACTIVEL        (0 << 3)
#define POLA_INACTIVEL      (1 << 3)
#define OUTPUT_ANYTIME      (0 << 2)
#define OUTPUT_LATCH        (1 << 2)
#define MODE_NONUSE         (0)
#define MODE_PROXIMITY      (1)
#define MODE_ILLUMINANCE    (2)
#define MODE_BOTH           (3)

/* RANGE */
#define REG_PSTH_MAX     (0xFFF)
#define REG_PSTL_MAX     (0xFFF)
#define PERSISTENCE_MAX     (0x0F)
#define GEN_READ_MAX 	(19)
#define REG_ALSPSCTL_MAX    (0x3F)
#define REG_INTERRUPT_MAX   (0x2F)

/* INIT PARAM */
#define PS_ALS_SET_MODE_CONTROL   (NORMAL_MODE)
#define PS_ALS_SET_ALSPS_CONTROL  (LEDCURRENT_200MA | ALSGAIN_X128X128)	//Set high gain value to acquire high accuracy
#define PS_ALS_SET_INTR_PERSIST   (1)
#define PS_ALS_SET_INTR           (PS_THH_BOTH_OUTSIDE| POLA_ACTIVEL | OUTPUT_LATCH | MODE_PROXIMITY)
#define PS_ALS_SET_PS_TH          (35)	//Customer should change the threshold value according to their mechanical design and measured data
#define PS_ALS_SET_PS_TL          (30)	//Changed from (0x000)
#define PS_ALS_SET_ALS_TH         (2000) 	//Compare with ALS_DATA0. ALS_Data equals 0.192*ALS_DATA0 roughly. Usually not used.
#define PS_ALS_SET_ALS_TL         (0x0000)	//Usually not used.
#define PS_ALS_SET_MIN_DELAY_TIME (100)	//Andy Mi: Changed from 125 to 100. I have no idea why it is 125 previously. 

/* OTHER */
#ifdef _ALS_BIG_ENDIAN_
#define CONVERT_TO_BE(value) ((((value) >> 8) & 0xFF) | (((value) << 8) & 0xFF00))
#else
#define CONVERT_TO_BE(value) (value)
#endif

/*************** Structs ******************/
struct  wake_lock ps_lock;
struct ALS_PS_DATA {
	struct i2c_client *client;
	struct mutex update_lock;
	struct delayed_work	dwork;	/* for PS interrupt */
	struct delayed_work    als_dwork; /* for ALS polling */
	struct input_dev *input_dev_als;
	struct input_dev *input_dev_ps;

    unsigned int enable;        /* used to indicate working mode */
    unsigned int als_time;      /* als measurement time */
    unsigned int ps_time;       /* ps measurement time */
    unsigned int ps_th_l;       /* ps threshold low */
    unsigned int ps_th_h;       /* ps threshold high */
    unsigned int ps_th_l_back;  /* ps threshold low backup */
    unsigned int ps_th_h_back;  /* ps threshold high backup */
    unsigned int als_th_l;      /* als threshold low, not used in the program */
    unsigned int als_th_h;      /* als threshold high, not used in the program */
    unsigned int persistence;   /* persistence */
    unsigned int control;       /* als_ps_control */

    /* register value */
    unsigned short als_data0_raw;  /* register value of data0 */
    unsigned short als_data1_raw;  /* register value of data1 */
    unsigned short ps_data_raw;    /* register value of ps */

    /* control flag from HAL */
    unsigned int enable_ps_sensor;
    unsigned int enable_als_sensor;

    /* PS parameters */

	unsigned int ps_direction;		/* 0 = near-to-far; 1 = far-to-near */
    unsigned int ps_data;        /* to store PS data */
    float ps_distance;
    unsigned int ledcurrent;     /* led current */

    /* ALS parameters */
    unsigned int als_data;       /* to store ALS data */
    unsigned int als_level;
    unsigned int gain0;          /* als data0 gain */
    unsigned int gain1;          /* als data1 gain */
    unsigned int als_poll_delay; /* the unit is ms I think. needed for als polling */

};

typedef struct {
    unsigned long long data;
    unsigned long long data0;
    unsigned long long data1;
    unsigned char      gain_data0;
    unsigned char      gain_data1;
    unsigned long      dev_unit;
    unsigned char      als_time;
    unsigned short     als_data0;
    unsigned short     als_data1;
} CALC_DATA;

typedef struct {
    unsigned long positive;
    unsigned long decimal;
} CALC_ANS;

/* structure to set initial value to sensor */
typedef struct {
    unsigned char  mode_ctl;         /* value of PS and ALS function          */
    unsigned char  psals_ctl;        /* value of PS and ALS control           */
    unsigned char  persist;          /* value of PS interrupt persistence     */
    unsigned char  intr;             /* interruption setting value            */
    unsigned int psth_upper;       /* threshold value of high level for PS  */
    unsigned int psth_low;         /* threshold value of low level for PS   */
    unsigned int alsth_upper;      /* threshold value of high level for ALS */
    unsigned int alsth_low;        /* threshold value of low level for ALS  */
} INIT_ARG;

/* calibration file path and file name */
#define CALIB_FILE    ("/mpt/prox_calibration.dat")
/* Length value for cailbration data size which is from number to char */
#define CALB_CHAR_LEN      (10)
#define CALB_TIMES         (20)
#define PS10MS             (1)
#define REG_SW_RESET       (1 << 7)
#define REG_INT_RESET      (1 << 6)
/* effect calibrate */
#define HIGH_THRESHOLD_CAL (17)
#define LOW_THRESHOLD_CAL  (10)
/* prohibit calibrate */
#define HIGH_THRESHOLD_NON_CAL (40)
#define LOW_THRESHOLD_NON_CAL  (35)

/*************** Global Data ******************/
/* parameter for als calculation */

/* mode control table */
#define MODE_CTL_FACTOR (16)
static const struct MCTL_TABLE {
    short ALS;
    short PS;
} mode_table[MODE_CTL_FACTOR] = {
    {  0,   0},   /*  0 */
    {  0,  10},   /*  1 */
    {  0,  40},   /*  2 */
    {  0, 100},   /*  3 */
    {  0, 400},   /*  4 */
    {100,   0},   /*  5 */
    {100, 100},   /*  6 */
    {100, 400},   /*  7 */
    {400,   0},   /*  8 */
    {400, 100},   /*  9 */
    {400,   0},   /* 10 */
    {400, 400},   /* 11 */
    {  0,   0},   /* 12 */
    {  0,   0},   /* 13 */
    {  0,   0},   /* 14 */
    {  0,   0}    /* 15 */
};

/* gain table */
#define GAIN_FACTOR (16)
static const struct GAIN_TABLE {
    char DATA0;
    char DATA1;
} gain_table[GAIN_FACTOR] = {
    {  1,   1},   /*  0 */
    {  2,   1},   /*  1 */
    {  0,   0},   /*  2 */
    {  0,   0},   /*  3 */
    {  0,   0},   /*  4 */
    {  2,   2},   /*  5 */
    {  0,   0},   /*  6 */
    {  0,   0},   /*  7 */
    {  0,   0},   /*  8 */
    {  0,   0},   /*  9 */
    { 64,  64},   /* 10 */
    {  0,   0},   /* 11 */
    {  0,   0},   /* 12 */
    {  0,   0},   /* 13 */
    {128,  64},   /* 14 */
    {128, 128}    /* 15 */
};

static int rpr400_init_client(struct i2c_client *client);
static int rpr400_resume(struct i2c_client *client);
static int rpr400_suspend(struct i2c_client *client, pm_message_t mesg);
/*************** Functions ******************/
/******************************************************************************
 * NAME       : rpr400_set_enable
 * FUNCTION   : set measurement time according to enable
 * REMARKS    : this function will overwrite the work mode. if it is called improperly, 
 *			   you may shutdown some part unexpectedly. please check als_ps->enable first.
 *			   I assume it is run in normal mode. If you want low noise mode, the code should be modified.
 *****************************************************************************/
static int rpr400_set_enable(struct i2c_client *client, int enable)
{
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	int ret;
	
	dev_dbg(&client->dev,  "%s, enable : %d\n", __func__, enable );
	
	if(enable > 0x0B)
	{
		dev_info(&client->dev, "%s: invalid measurement time setting.\n", __func__);
		return -EINVAL;
	}
	else
	{
		mutex_lock(&als_ps->update_lock);
		ret = i2c_smbus_write_byte_data(client, REG_MODECONTROL, enable);
		mutex_unlock(&als_ps->update_lock);

		als_ps->enable = enable;
		als_ps->als_time = mode_table[(enable & 0xF)].ALS;
		als_ps->ps_time = mode_table[(enable & 0xF)].PS;

		return ret;
	}
}

static int rpr400_set_ps_threshold_low(struct i2c_client *client, int threshold)
{
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	int ret;
	unsigned short write_data;

    /* check whether the parameter is valid */
	if(threshold > REG_PSTL_MAX)
	{
		dev_info(&client->dev,  "%s: exceed maximum possible value.\n", __func__);
		return -EINVAL;
	}
	if(threshold > als_ps->ps_th_h)
	{
		dev_info(&client->dev,  "%s: higher than threshold high.\n", __func__);
		return -EINVAL;
	}
	
    /* write register to RPR400 via i2c */
	write_data = CONVERT_TO_BE(threshold);
	mutex_lock(&als_ps->update_lock);
	ret = i2c_smbus_write_i2c_block_data(client, REG_PSTL, sizeof(write_data), (unsigned char *)&write_data);
	mutex_unlock(&als_ps->update_lock);
	if(ret < 0)
	{
		dev_info(&client->dev,  "%s: write i2c fail.\n", __func__);
		return ret;
	}
	als_ps->ps_th_l = threshold;	//Update the value after successful i2c write to avoid difference. 
		
	return 0;
}

static int rpr400_set_ps_threshold_high(struct i2c_client *client, int threshold)
{
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	int ret;
	unsigned short write_data;

    /* check whether the parameter is valid */
	if(threshold > REG_PSTH_MAX)
	{
		dev_info(&client->dev, "%s: exceed maximum possible value.\n", __func__);
		return -EINVAL;
	}
	if(threshold < als_ps->ps_th_l)
	{
		dev_info(&client->dev, "%s: lower than threshold low.\n", __func__);
		return -EINVAL;
	}
	
    /* write register to RPR400 via i2c */
	write_data = CONVERT_TO_BE(threshold);
	mutex_lock(&als_ps->update_lock);
	ret = i2c_smbus_write_i2c_block_data(client, REG_PSTH, sizeof(write_data), (unsigned char *)&write_data);
	mutex_unlock(&als_ps->update_lock);
	if(ret < 0)
	{
		dev_info(&client->dev, "%s: write i2c fail.\n", __func__);
		return ret;
	}
	als_ps->ps_th_h = threshold;	//Update the value after successful i2c write to avoid difference. 
		
	return 0;
}

/******************************************************************************
 * NAME       : calculate_ps_data
 * FUNCTION   : calculate proximity data for RPR400
 * REMARKS    : 12 bit output
 *****************************************************************************/
static int calc_rohm_ps_data(unsigned short ps_reg_data)
{
    return (ps_reg_data & 0xFFF);
}
static void rpr400_reschedule_work(struct ALS_PS_DATA *als_ps,
					  unsigned long delay)
{
	unsigned long flags;

	spin_lock_irqsave(&als_ps->update_lock.wait_lock, flags);

	/*
	 * If work is already scheduled then subsequent schedules will not
	 * change the scheduled time that's why we have to cancel it first.
	 */
	__cancel_delayed_work(&als_ps->dwork);
	schedule_delayed_work(&als_ps->dwork, delay);

	spin_unlock_irqrestore(&als_ps->update_lock.wait_lock, flags);
}
/* PS interrupt routine */
static void rpr400_ps_int_work_handler(struct work_struct *work)
{
	struct ALS_PS_DATA *als_ps = container_of((struct delayed_work *)work, struct ALS_PS_DATA, dwork);
	struct i2c_client *client=als_ps->client;
	int tmp;

	tmp =  i2c_smbus_read_byte_data(client, REG_INTERRUPT);
    if(tmp < 0) {
		dev_info(&client->dev, "%s: i2c read interrupt status fail. \n", __func__);
		return;
	}
    if ((tmp & PS_INT_MASK) && (als_ps->ps_time != 0)) {   //Interrupt is caused by PS and PS measurement time is not 0
        tmp = i2c_smbus_read_byte_data(client, REG_ALSPSCONTROL);
        if (tmp < 0) {
            dev_info(&client->dev, "%s: i2c read led current fail. \n", __func__);
            return;
        }
        als_ps->ledcurrent = tmp & 0x3;
        
        tmp = i2c_smbus_read_word_data(client, REG_PSDATA);
        if (tmp < 0) {
            dev_info(&client->dev, "%s: i2c read ps data fail. \n", __func__);
            return;
        }
        /* Over process */
        if (((als_ps->ps_th_h == REG_PSTH_MAX) && (tmp == REG_PSTH_MAX)) || \
            ((als_ps->ps_th_l == 0) && (tmp == 0))) {
                /* non process. please write process here if you want to write error process */
         } else {

            als_ps->ps_data_raw = (unsigned short)tmp;
            als_ps->ps_data = calc_rohm_ps_data(als_ps->ps_data_raw);
            if(als_ps->ps_data > als_ps->ps_th_h) {        
                als_ps->ps_direction = 0;
                rpr400_set_ps_threshold_high(client, REG_PSTH_MAX);
                rpr400_set_ps_threshold_low(client, als_ps->ps_th_l_back);
            } else if (als_ps->ps_data < als_ps->ps_th_l) {
                als_ps->ps_direction = 1;
                rpr400_set_ps_threshold_high(client, als_ps->ps_th_h_back);
                rpr400_set_ps_threshold_low(client, 0);
			if(wake_lock_active(&ps_lock))
				wake_unlock(&ps_lock);
			wake_lock_timeout(&ps_lock, 2 * HZ);
            }
            dev_dbg(&client->dev, "RPR400 ps report: raw_data = %d, data = %d, direction = %d. \n", \
                als_ps->ps_data_raw, als_ps->ps_data, als_ps->ps_direction);
        
            input_report_abs(als_ps->input_dev_ps, ABS_DISTANCE, als_ps->ps_direction); 
            input_sync(als_ps->input_dev_ps);
        }    
    } else {
        dev_info(&client->dev,  "%s: unknown interrupt source.\n", __func__);
    }
    
	enable_irq(client->irq);
}

/* assume this is ISR */
static irqreturn_t rpr400_interrupt(int vec, void *info)
{
	struct i2c_client *client=(struct i2c_client *)info;
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	disable_irq_nosync(client->irq);

	dev_dbg(&client->dev,  "%s\n", __func__);
	rpr400_reschedule_work(als_ps, 0);

	return IRQ_HANDLED;
}

/*************** SysFS Support ******************/
static int rpr400_read_crosstalk_data_fs(void)
{
	int fd;
	char read_buf[CALB_CHAR_LEN];
	int data;
	mm_segment_t old_fs;
	
	old_fs = get_fs();

	memset(read_buf, 0, sizeof(read_buf));
	set_fs(KERNEL_DS);

	fd = sys_open(CALIB_FILE,O_RDONLY, 0);
	if (fd >= 0) {
		sys_read(fd, read_buf, sizeof(read_buf));
		data = simple_strtol(read_buf, NULL, 10);
	} else {
		printk(KERN_ERR "file open error!!");
		data = -1;
	}
	sys_close(fd);
	set_fs(old_fs);

	return (data);

}
static int rpr400_backup_crosstalk_data_fs(unsigned int val)
{
	int fd;
	char buf[CALB_CHAR_LEN];
	mm_segment_t old_fs;
	int result;
	
	old_fs = get_fs();

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", val);

	set_fs(KERNEL_DS);
	fd = sys_open(CALIB_FILE,O_WRONLY|O_CREAT, 0664);

	if(fd >= 0) {
		sys_write(fd, buf, sizeof(buf));
		result = 0;
	} else {
		printk(KERN_ERR "file open error!!");
		result = 1;
	}
	sys_close(fd);
	set_fs(old_fs);

	return (result);
}

static void rpr400_make_threshold_value(struct i2c_client *client, unsigned int *high, unsigned int *low)
{
	int data;
	
	dev_dbg(&client->dev, "%s \n", __func__);
	
	data = rpr400_read_crosstalk_data_fs();

	dev_dbg(&client->dev, "data : %d \n", data);
	
	if(data < 0) {
		*high = HIGH_THRESHOLD_NON_CAL;
		*low  = LOW_THRESHOLD_NON_CAL;
	} else {
		if(data > HIGH_THRESHOLD_NON_CAL)
		{
			dev_info(&client->dev, "cross_talk value is over HIGH_THRESHOLD_NON_CAL, go as NON_CAL value\n" );
			*high = HIGH_THRESHOLD_NON_CAL;
			*low  = LOW_THRESHOLD_NON_CAL;	
		}
		else
		{
			*high = data + HIGH_THRESHOLD_CAL;
			*low  = data + LOW_THRESHOLD_CAL;
		}
	}
	
	return ;
}

static ssize_t rpr400_show_enable_ps_sensor(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	
	return sprintf(buf, "%d\n", als_ps->enable_ps_sensor);
}

static ssize_t rpr400_store_enable_ps_sensor(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	unsigned int val = simple_strtoul(buf, NULL, 10);
	pm_message_t dummy_state;

	dummy_state.event = 0;

	dev_dbg(&client->dev,  "RPR400 enable PS sensor -> %d \n", val);

	if ((val != 0) && (val != 1)) 
	{
		dev_info(&client->dev,  "%s:store unvalid value=%d\n", __func__, val);
		return count;
	}
	
	if(val == 1) 
	{
		//turn on p sensor
		//wake_lock(&ps_lock);
		if (als_ps->enable_ps_sensor == 0) 
		{
			als_ps->enable_ps_sensor = 1;
			rpr400_resume(client);
		}
		else
			dev_info(&client->dev,  "ps sensor is already enable!\n");
	} 
	else 
	{
		if(als_ps->enable_ps_sensor == 1)
		{
			als_ps->enable_ps_sensor = 0;
			rpr400_suspend(client, dummy_state);
		//	wake_unlock(&ps_lock);
		}
		else
			dev_info(&client->dev,  "ps sensor is already disable!\n");
	}
		
	return count;
}

static ssize_t rpr400_show_ps_thres_high(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	
	return sprintf(buf, "%d\n", als_ps->ps_th_h);	
}

static ssize_t rpr400_store_ps_thres_high(struct device *dev,
					struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	
	if(!(rpr400_set_ps_threshold_high(client, val)))
		als_ps->ps_th_h_back = als_ps->ps_th_h;
	
	return count;
}

static ssize_t rpr400_show_ps_thres_low(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	
	return sprintf(buf, "%d\n", als_ps->ps_th_l);	
}

static ssize_t rpr400_store_ps_thres_low(struct device *dev,
					struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	
	if(!(rpr400_set_ps_threshold_low(client, val)))
		als_ps->ps_th_l_back = als_ps->ps_th_l;
	
	return count;
}

static ssize_t rpr400_show_ps_calib(struct device *dev,
				struct device_attribute *attr, char *buf)
{
    int data;
	
    data = rpr400_read_crosstalk_data_fs();
	
    return sprintf(buf, "%d\n", data);    
}

static ssize_t rpr400_store_ps_calib(struct device *dev,
					struct device_attribute *attr, const char *buf, size_t count)
{
	struct ALS_PS_DATA *als_ps;
	struct i2c_client *client = to_i2c_client(dev);
    	unsigned int average, high, low;
 	int i, tmp;
    	unsigned int val = simple_strtoul(buf, NULL, 10);

	dev_dbg(&client->dev, "%s\n", __func__);
	
	als_ps = i2c_get_clientdata(client);
	average = 0;

	rpr400_set_enable(client, PS10MS);	//PS 10ms

	for(i = 0; i < CALB_TIMES; i ++) {
		tmp = i2c_smbus_read_word_data(client, REG_PSDATA);
		if(tmp < 0) {
			goto err_exit;
		}
		average += tmp;
	}
	average /= CALB_TIMES;

	dev_info(&client->dev, "cross-talk value : %d\n", average);

	rpr400_backup_crosstalk_data_fs(average);

    high = average + HIGH_THRESHOLD_CAL;
    low  = average + LOW_THRESHOLD_CAL;
    switch(val){
    case 0: /* wirte the crosstalk value to file. */
        break;
    case 1: /* wirte the crosstalk value to file and set both threshold value to register */
        /* set high threshold */
        rpr400_set_ps_threshold_high(client, high);
        als_ps->ps_th_h = high;
        als_ps->ps_th_h_back = als_ps->ps_th_h;
        /* set low threshold */
        rpr400_set_ps_threshold_low(client, low);
        als_ps->ps_th_l = low;
        als_ps->ps_th_l_back = als_ps->ps_th_l;
       break;
    default : /* wirte the crosstalk value to file. */
         break;
    }
    if(als_ps->enable_ps_sensor == 0) {
        rpr400_set_enable(client, 0);
    } else {
        rpr400_set_enable(client, PS100MS);
    }
	return count;

err_exit:
    rpr400_set_enable(client, 0);
    if(als_ps->enable_ps_sensor == 1) {
        als_ps->enable_ps_sensor = 0;
        disable_irq(client->irq);
    }
	return -1;
	
}

static ssize_t rpr400_show_pdata(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);

	if(client != NULL){
			int pdata = i2c_smbus_read_word_data(client, REG_PSDATA);

			return sprintf(buf, "%d\n",pdata);
	}
	else{
			return -1;
	}
}

static ssize_t rpr400_show_crosstalk_data(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;
	
	ret = rpr400_read_crosstalk_data_fs();
	if(ret<0)
		return sprintf(buf, "Read fail\n");
	
	return sprintf(buf, "%d\n", ret);
}

static ssize_t rpr400_store_crosstalk_data(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	int ret = 0;
	unsigned long val = simple_strtoul(buf, NULL, 10);

	ret = rpr400_backup_crosstalk_data_fs(val);
	if(ret != 0)
		return printk("File open get default value %d\n", ret);	

	dev_dbg(&client->dev, "saved cross_talk val : %d\n", (int)val);
	return count;
}

static ssize_t rpr400_status_show(struct device *dev, 
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);

	if(client != NULL)
		return sprintf(buf, "%d\n",als_ps->ps_direction);
	else 
		return -1;
}

static DEVICE_ATTR(enable,  S_IRUGO | S_IWUSR/*S_IWUGO*/ ,
				   rpr400_show_enable_ps_sensor, rpr400_store_enable_ps_sensor);

static DEVICE_ATTR(piht,  S_IRUGO | S_IWUSR/*S_IWUGO*/ ,
				  rpr400_show_ps_thres_high, rpr400_store_ps_thres_high);

static DEVICE_ATTR(pilt,  S_IRUGO | S_IWUSR/*S_IWUGO*/ ,
				   rpr400_show_ps_thres_low, rpr400_store_ps_thres_low);

static DEVICE_ATTR(run_calibration,  S_IWUSR | S_IRUGO|S_IWGRP |S_IRGRP |S_IROTH ,
				   rpr400_show_ps_calib, rpr400_store_ps_calib);

static DEVICE_ATTR(prox_cal_data,  S_IWUSR | S_IRUGO|S_IWGRP |S_IRGRP |S_IROTH/*|S_IWOTH*/,
		   rpr400_show_crosstalk_data, rpr400_store_crosstalk_data);

static DEVICE_ATTR(pdata, S_IRUGO,  rpr400_show_pdata, NULL);

static DEVICE_ATTR(show, S_IRUGO | S_IWUSR, rpr400_status_show, NULL);	

static struct attribute *rpr400_attributes[] = {
	&dev_attr_show.attr,
	&dev_attr_enable.attr,
	&dev_attr_piht.attr,
	&dev_attr_pilt.attr,
	&dev_attr_run_calibration.attr,
	&dev_attr_prox_cal_data.attr,
	&dev_attr_pdata.attr,
	NULL
};

static const struct attribute_group rpr400_attr_group = {
	.attrs = rpr400_attributes,
};

/*************** Initialze Functions ******************/
static int ps_als_driver_reset(struct i2c_client *client)
{
    int result;
    dev_dbg(&client->dev, "%s \n", __func__);
    /* set soft ware reset */
    result = i2c_smbus_write_byte_data(client, REG_SYSTEMCONTROL, (REG_SW_RESET | REG_INT_RESET));

    return (result);
}

static int ps_als_driver_init(INIT_ARG *data, struct i2c_client *client)
{
    struct init_func_write_data {
        unsigned char mode_ctl;
        unsigned char psals_ctl;
        unsigned char persist;
        unsigned char reserved0;
        unsigned char reserved1;
        unsigned char reserved2;
        unsigned char reserved3;
        unsigned char reserved4;
        unsigned char reserved5;
        unsigned char intr;
        unsigned char psth_hl;
        unsigned char psth_hh;
        unsigned char psth_ll;
        unsigned char psth_lh;
        unsigned char alsth_hl;
        unsigned char alsth_hh;
        unsigned char alsth_ll;
        unsigned char alsth_lh;
    } write_data;
    int result;

    dev_dbg(&client->dev, "%s \n", __func__);
	
    /* execute software reset */
    result = ps_als_driver_reset(client);
    if (result != 0) {
        return (result);
    }

    /* not check parameters are psth_upper, psth_low, alsth_upper, alsth_low */
    /* check the PS orerating mode */
    if ((NORMAL_MODE != data->mode_ctl) && (LOW_NOISE_MODE != data->mode_ctl)) {
        return (-EINVAL);
    }
    /* check the parameter of ps and als control */
    if (data->psals_ctl > REG_ALSPSCTL_MAX) {
        return (-EINVAL);
    }
    /* check the parameter of ps interrupt persistence */
    if (data->persist > PERSISTENCE_MAX) {
        return (-EINVAL);
    }
    /* check the parameter of interrupt */
    if (data->intr > REG_INTERRUPT_MAX) {
        return (-EINVAL);
    }
    /* check the parameter of proximity sensor threshold high */
    if (data->psth_upper > REG_PSTH_MAX) {
        return (-EINVAL);
    }
    /* check the parameter of proximity sensor threshold low */
    if (data->psth_low > REG_PSTL_MAX) {
        return (-EINVAL);
    }
    write_data.mode_ctl  = data->mode_ctl;
    write_data.psals_ctl = data->psals_ctl;
    write_data.persist   = data->persist;
    write_data.reserved0 = 0;
    write_data.reserved1 = 0;
    write_data.reserved2 = 0;
    write_data.reserved3 = 0;
    write_data.reserved4 = 0;
    write_data.reserved5 = 0;
    write_data.intr      = data->intr;
    write_data.psth_hl   = CONVERT_TO_BE(data->psth_upper) & MASK_CHAR;
    write_data.psth_hh   = CONVERT_TO_BE(data->psth_upper) >> 8;
    write_data.psth_ll   = CONVERT_TO_BE(data->psth_low) & MASK_CHAR;
    write_data.psth_lh   = CONVERT_TO_BE(data->psth_low) >> 8;
    write_data.alsth_hl  = CONVERT_TO_BE(data->alsth_upper) & MASK_CHAR;
    write_data.alsth_hh  = CONVERT_TO_BE(data->alsth_upper) >> 8;
    write_data.alsth_ll  = CONVERT_TO_BE(data->alsth_low) & MASK_CHAR;
    write_data.alsth_lh  = CONVERT_TO_BE(data->alsth_low) >> 8;
    result               = i2c_smbus_write_i2c_block_data(client, REG_MODECONTROL, sizeof(write_data), (unsigned char *)&write_data);

    return (result);
}

static int rpr400_init_client(struct i2c_client *client)
{
    int result;
    INIT_ARG data;
    unsigned char gain;
    struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);

	dev_dbg(&client->dev, "%s \n", __func__);
	/* make initial */
	/* set threshold */
	rpr400_make_threshold_value(client, &data.psth_upper, &data.psth_low);

	dev_dbg(&client->dev, "high thres : %d, low thres : %d \n", data.psth_upper, data.psth_low);
	
	data.mode_ctl    = PS_ALS_SET_MODE_CONTROL;
	data.psals_ctl   = PS_ALS_SET_ALSPS_CONTROL;
	data.persist     = PS_ALS_SET_INTR_PERSIST;
	data.intr        = PS_ALS_SET_INTR;
	data.alsth_upper = PS_ALS_SET_ALS_TH;
	data.alsth_low   = PS_ALS_SET_ALS_TL;
	
	result = ps_als_driver_init(&data, client);
	if(result < 0) {
		dev_info(&client->dev, "%s: i2c write fail. \n", __func__);
		return result;
	}

    gain = (data.psals_ctl & 0x3C) >> 2;    //gain setting values
    als_ps->enable       = data.mode_ctl;
    als_ps->als_time     = mode_table[(data.mode_ctl & 0xF)].ALS;
    als_ps->ps_time      = mode_table[(data.mode_ctl & 0xF)].PS;
    als_ps->persistence  = data.persist;
    als_ps->ps_th_l      = data.psth_low;
    als_ps->ps_th_h      = data.psth_upper;
    als_ps->als_th_l     = data.alsth_low;
    als_ps->als_th_h     = data.alsth_upper;
    als_ps->control      = data.psals_ctl;
    als_ps->gain0        = gain_table[gain].DATA0;
    als_ps->gain1        = gain_table[gain].DATA1;
    als_ps->ledcurrent   = data.psals_ctl & 0x03;
	als_ps->ps_th_h_back = als_ps->ps_th_h;
	als_ps->ps_th_l_back = als_ps->ps_th_l;
	
    return (result);
}

/*********** I2C init/probing/exit functions ****************/

static struct i2c_driver rpr400_driver;

static int __devinit rpr400_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
#define ROHM_PSALS_PSMAX  (4095)

	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct ALS_PS_DATA *als_ps;
	struct proximity_platform_data		*pdata; 
	pm_message_t dummy_state;
    

	int err = 0;

	dev_dbg(&client->dev, "%s started.\n",__func__);
	wake_lock_init(&ps_lock,WAKE_LOCK_SUSPEND,"rpr0400");

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		err = -EIO;
		goto exit;
	}

	als_ps = kzalloc(sizeof(struct ALS_PS_DATA), GFP_KERNEL);
	if (!als_ps) {
		err = -ENOMEM;
		goto exit;
	}
	als_ps->client = client;
	i2c_set_clientdata(client, als_ps);

	mutex_init(&als_ps->update_lock);

	INIT_DELAYED_WORK(&als_ps->dwork, rpr400_ps_int_work_handler);

	pdata = als_ps->client->dev.platform_data;
	if(NULL == pdata){
		dev_info(&client->dev, "platform data is NULL");
		return -1;
	}
	
	err = pdata->power(&client->dev, 1);
	if(err < 0) {
		dev_info(&client->dev, "%s,Proximity Power On Fail in Probe\n",__func__);
		goto exit_kfree;
	}
	
	mdelay(50);
    
	/* Initialize the RPR400 chip */
	err = rpr400_init_client(client);
	if (err)
		goto exit_kfree;


	als_ps->als_poll_delay = PS_ALS_SET_MIN_DELAY_TIME;	
	als_ps->input_dev_ps = input_allocate_device();
	if (!als_ps->input_dev_ps) {
		err = -ENOMEM;		
		dev_info(&client->dev, "%s: Failed to allocate input device ps\n", __func__);
		goto exit;
	}
	set_bit(EV_ABS, als_ps->input_dev_ps->evbit);
	input_set_abs_params(als_ps->input_dev_ps, ABS_DISTANCE, 0, ROHM_PSALS_PSMAX, 0, 0);

	als_ps->input_dev_ps->name = "proximity";

	err = input_register_device(als_ps->input_dev_ps);
	if (err < 0) {
		input_free_device(als_ps->input_dev_ps);
		dev_info(&client->dev, "%s: Unable to register input device ps: %s\n", __func__, 
		       als_ps->input_dev_ps->name);	
		goto exit_kfree;
	}	
	
	/* Register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &rpr400_attr_group);
		
	if (err)
	{
		dev_info(&client->dev, "%s sysfs_create_groupX\n", __func__);
		goto exit_unregister_dev_ps;
	}

	dev_dbg(&client->dev, "%s support ver. %s enabled\n", __func__, DRIVER_VERSION);

	if (request_irq(client->irq, rpr400_interrupt, IRQF_DISABLED|IRQ_TYPE_EDGE_FALLING,
		RPR400_DRV_NAME, (void *)client)) {
		dev_info(&client->dev, "%s Could not allocate RPR400_INT !\n", __func__);
		goto exit;
	}

	err = irq_set_irq_wake(client->irq, 1);
	if (err)
		irq_set_irq_wake(client->irq, 0);

	dummy_state.event = 0;

	rpr400_suspend(client, dummy_state);
	dev_dbg(&client->dev,  "%s: INT No. %d", __func__, client->irq);
	return 0;

exit_unregister_dev_ps:
	input_unregister_device(als_ps->input_dev_ps);	
exit_kfree:
	kfree(als_ps);
exit:
	return err;

#undef ROHM_PSALS_PSMAX
}

static int __devexit rpr400_remove(struct i2c_client *client)
{
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	
	input_unregister_device(als_ps->input_dev_ps);
	input_free_device(als_ps->input_dev_ps);

	free_irq(client->irq, client);

	sysfs_remove_group(&client->dev.kobj, &rpr400_attr_group);

	/* Power down the device */
	rpr400_set_enable(client, 0);

	kfree(als_ps);

	return 0;
}

static int rpr400_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret, err;
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	struct proximity_platform_data* pdata = NULL;

	dev_dbg(&client->dev,"%s\n", __func__);

	pdata = als_ps->client->dev.platform_data;
	if(pdata == NULL)	{
		dev_info(&client->dev, "Platform data is NULL");
		return -1;
	}

	ret = rpr400_set_enable(client, 0);
	disable_irq(client->irq);
	err = pdata->power(&client->dev, 0);
	
	irq_set_irq_wake(client->irq, 0);
	//wake_unlock(&ps_lock);
	return ret; 
}

static int rpr400_resume(struct i2c_client *client)
{
	int err,ret;
	struct ALS_PS_DATA *als_ps = i2c_get_clientdata(client);
	struct proximity_platform_data* pdata = NULL;

	dev_dbg(&client->dev,"%s\n", __func__);
	
	pdata = als_ps->client->dev.platform_data;
	if(pdata == NULL)	{
		dev_info(&client->dev, "Platform data is NULL");
		return -1;
	}
	//wake_lock(&ps_lock);
	enable_irq(client->irq);
	err = pdata->power(&client->dev, 1);
	mdelay(50);

	err = rpr400_init_client(client);
	ret = rpr400_set_enable(client, PS100MS);

	ret = irq_set_irq_wake(client->irq, 1);
	if(ret)
		irq_set_irq_wake(client->irq, 0);
	
	return 0;
}


MODULE_DEVICE_TABLE(i2c, rpr400_id);

static const struct i2c_device_id rpr400_id[] = {
	{ "rpr400", 0 },
	{ }
};
 
static struct i2c_driver rpr400_driver = {
	.driver = {
		.name	= RPR400_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.probe	= rpr400_probe,
	.remove	= __devexit_p(rpr400_remove),
	.id_table = rpr400_id,
};

static int __init rpr400_init(void)
{
	return i2c_add_driver(&rpr400_driver);
}

static void __exit rpr400_exit(void)
{
	i2c_del_driver(&rpr400_driver);
}

MODULE_DESCRIPTION("RPR400 proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(rpr400_init);
module_exit(rpr400_exit);
