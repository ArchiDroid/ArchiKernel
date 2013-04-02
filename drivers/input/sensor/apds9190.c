/*
 *  apds9190.c - Linux kernel modules for proximity sensor
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include CONFIG_LGE_BOARD_HEADER_FILE

#include <mach/gpio.h>

#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <mach/vreg.h>
#include <linux/wakelock.h>
#include <mach/msm_i2ckbd.h>
#include <linux/spinlock.h>

#include <linux/string.h>

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-27 Sanghun,Lee(eee3114.@lge.com) apds9190 proximity sensor calibration
define APDS9190_PROXIMITY_CAL
*/
#define APDS9190_PROXIMITY_CAL
#endif

#if defined(APDS9190_PROXIMITY_CAL)
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define PS_DEFAULT_CROSS_TALK 100
#endif


#ifdef CONFIG_LGE_PCB_VERSION
extern void lge_set_hw_version_string(char *pcb_version);
#endif

#define APDS9190_DRV_NAME	"proximity_apds9190"
#define DRIVER_VERSION		"1.0.8"
/* Change History 
 *
 * 1.0.1	Functions apds9190_show_rev(), apds9190_show_id() and apds9190_show_status()
 *			have missing CMD_BYTE in the i2c_smbus_read_byte_data(). APDS-9190 needs
 *			CMD_BYTE for i2c write/read byte transaction.
 *
 *
 * 1.0.2	Include PS switching threshold level when interrupt occurred
 *
 *
 * 1.0.3	Implemented ISR and delay_work, correct PS threshold storing
 *
 * 1.0.4	Added Input Report Event
 */

/*
 * Defines
 */

#define APDS9190_ENABLE_REG		0x00
#define APDS9190_ATIME_REG		0x01 //ALS Non Use, Reserved
#define APDS9190_PTIME_REG		0x02
#define APDS9190_WTIME_REG		0x03
#define APDS9190_AILTL_REG		0x04 //ALS Non Use, Reserved
#define APDS9190_AILTH_REG		0x05 //ALS Non Use, Reserved
#define APDS9190_AIHTL_REG		0x06 //ALS Non Use, Reserved
#define APDS9190_AIHTH_REG		0x07 //ALS Non Use, Reserved
#define APDS9190_PILTL_REG		0x08
#define APDS9190_PILTH_REG		0x09
#define APDS9190_PIHTL_REG		0x0A
#define APDS9190_PIHTH_REG		0x0B
#define APDS9190_PERS_REG		0x0C
#define APDS9190_CONFIG_REG		0x0D
#define APDS9190_PPCOUNT_REG	0x0E
#define APDS9190_CONTROL_REG	0x0F
#define APDS9190_REV_REG		0x11
#define APDS9190_ID_REG			0x12
#define APDS9190_STATUS_REG		0x13
#define APDS9190_CDATAL_REG		0x14 //ALS Non Use, Reserved
#define APDS9190_CDATAH_REG		0x15 //ALS Non Use, Reserved
#define APDS9190_IRDATAL_REG	0x16 //ALS Non Use, Reserved
#define APDS9190_IRDATAH_REG	0x17 //ALS Non Use, Reserved
#define APDS9190_PDATAL_REG		0x18
#define APDS9190_PDATAH_REG		0x19

#define CMD_BYTE				0x80
#define CMD_WORD				0xA0
#define CMD_SPECIAL				0xE0

#define CMD_CLR_PS_INT			0xE5
#define CMD_CLR_ALS_INT			0xE6
#define CMD_CLR_PS_ALS_INT		0xE7

#define APDS9190_ENABLE_PIEN 	0x20 
#define APDS9190_ENABLE_AIEN 	0x10 
#define APDS9190_ENABLE_WEN 	0x08 
#define APDS9190_ENABLE_PEN 	0x04 
//Proximity false Interrupt by strong ambient light
#define APDS9190_ENABLE_AEN 	0x02 
#define APDS9190_ENABLE_PON 	0x01

//Proximity false Interrupt by strong ambient light
#define ATIME 	 				0xDB 	// 100.64ms . minimum ALS integration time //ALS Non Use
#define WTIME 				 	0xFF 	// 2.72 ms . 
#define PTIME 	 				0xFF 	

#define PDIODE 				 	0x20 	// IR Diode
#define PGAIN 				 	0x00 	//1x Prox gain
#define AGAIN 					0x00 	//1x ALS gain, Reserved ALS Non Use

#ifdef CONFIG_LGE_PCB_VERSION
#define REV_B_APDS_PROXIMITY_HIGH_THRESHHOLD		600
#define REV_B_APDS_PROXIMITY_LOW_THRESHHOLD			550

#define REV_C_APDS_PROXIMITY_HIGH_THRESHHOLD		400
#define REV_C_APDS_PROXIMITY_LOW_THRESHHOLD			250

#define REV_D_APDS_PROXIMITY_HIGH_THRESHHOLD		400
#define REV_D_APDS_PROXIMITY_LOW_THRESHHOLD			200

#define REV_B_PPCOUNT 			10 		//prox pulse count
#define REV_B_PDRIVE 	 		0 		//100mA of LED Power

#define REV_C_PPCOUNT 			13 		//prox pulse count
#define REV_C_PDRIVE 	 		0 		//100mA of LED Power

#define REV_D_PPCOUNT 			15 		//prox pulse count
#define REV_D_PDRIVE 		 	0 		//50mA of LED Power
#endif

#define DEFAULT_APDS_PROXIMITY_HIGH_THRESHHOLD				600
#define DEFAULT_APDS_PROXIMITY_LOW_THRESHHOLD				550

#define DEFAULT_PPCOUNT 		18 		//prox pulse count
#define DEFAULT_PDRIVE 	 		0 		//100mA of LED Power

#define APDS9190_STATUS_PINT_AINT	0x30
#define APDS9190_STATUS_PINT	0x20
#define APDS9190_STATUS_AINT	0x10 // ALS Interrupt STATUS ALS Non Use

#define APDS900_SENSOR_DEBUG 0
#if APDS900_SENSOR_DEBUG
#define DEBUG_MSG(args...)  printk(args)
#else
#define DEBUG_MSG(args...)
#endif

/*
 * Structs
 */

struct apds9190_data {
	struct i2c_client *client;
	struct mutex update_lock;
	struct input_dev *input_dev_ps;
	struct work_struct dwork;

	unsigned int enable;
	unsigned int atime;
	unsigned int ptime;
	unsigned int wtime;
	unsigned int ailt;
	unsigned int aiht;
	unsigned int pilt;
	unsigned int piht;
	unsigned int pers;
	unsigned int config;
	unsigned int ppcount;
	unsigned int control;

/* control flag from HAL */
//	unsigned int enable_ps_sensor;

	unsigned int GA;
	unsigned int DF;
	unsigned int LPC;
#if defined(APDS9190_PROXIMITY_CAL)
	/* PS parameters */
	unsigned int ps_threshold;
	unsigned int ps_hysteresis_threshold; /* always lower than ps_threshold */
	unsigned int ps_detection;		/* 0 = near-to-far; 1 = far-to-near */
#endif
	int irq;

	unsigned int sw_mode;
	spinlock_t lock;

#if defined(APDS9190_PROXIMITY_CAL)
	int cross_talk;
	bool read_ps_cal_data;
	bool ps_cal_result;
	struct wake_lock wakelock;	
#endif

};

#if defined(APDS9190_PROXIMITY_CAL)
static int apds9190_read_crosstalk_data_fs(void);
static void apds9190_Set_PS_Threshold_Adding_Cross_talk(struct i2c_client *client, int cal_data);
#endif
/*
 * Global data
 */
enum apds9190_dev_status {
		PROX_STAT_SHUTDOWN = 0,
		PROX_STAT_OPERATING,
};

enum apds9190_input_event {
		PROX_INPUT_NEAR = 0,
		PROX_INPUT_FAR,
};
static struct i2c_client *apds_9190_i2c_client = NULL;
static struct workqueue_struct *proximity_wq = NULL;

/*
 * Management functions
 */
static int apds9190_init_client(struct i2c_client *client);
static int apds9190_suspend(struct i2c_client *i2c_dev, pm_message_t state);
static int apds9190_resume(struct i2c_client *i2c_dev);

static int apds9190_set_command(struct i2c_client *client, int command)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret, i;
	int clearInt;

	if (command == 0)
	{
		clearInt = CMD_CLR_PS_INT;
#if defined (APDS9190_PROXIMITY_DEBUG)		
		printk(KERN_INFO "%s, clear [CMD_CLR_PS_INT]\n",__func__);
#endif
	}
	else if (command == 1)
	{
		clearInt = CMD_CLR_ALS_INT;
	}
	else
	{
		clearInt = CMD_CLR_PS_ALS_INT;
#if defined (APDS9190_PROXIMITY_DEBUG)				
		printk(KERN_INFO "%s, clear [CMD_CLR_PS_ALS_INT]\n",__func__);			
#endif
	}

	mutex_lock(&data->update_lock);
	for(i = 0;i<10;i++) {
		ret = i2c_smbus_write_byte(client, clearInt);
		if(ret < 0) {
			printk("%s, I2C Fail\n",__func__);
			continue;
		}
		else
			break;
			
	}
	mutex_unlock(&data->update_lock);

	return ret;
}

static int apds9190_set_enable(struct i2c_client *client, int enable)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_ENABLE_REG, enable);
	mutex_unlock(&data->update_lock);
	DEBUG_MSG("apds9190_set_enable = [%x] \n",enable);
#if defined (APDS9190_PROXIMITY_DEBUG)					
	printk("%s apds9190_set_enable = [%x] \n", __FUNCTION__, enable);
#endif
	data->enable = enable;

	return ret;
}

static int apds9190_set_atime(struct i2c_client *client, int atime)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_ATIME_REG, atime);
	mutex_unlock(&data->update_lock);

	data->atime = atime;

	return ret;
}

static int apds9190_set_ptime(struct i2c_client *client, int ptime)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_PTIME_REG, ptime);
	mutex_unlock(&data->update_lock);

	data->ptime = ptime;

	return ret;
}

static int apds9190_set_wtime(struct i2c_client *client, int wtime)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_WTIME_REG, wtime);
	mutex_unlock(&data->update_lock);

	data->wtime = wtime;

	return ret;
}
static int apds9190_set_ailt(struct i2c_client* client, int threshold)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_AILTL_REG, threshold);
	mutex_unlock(&data->update_lock);	
	data->ailt = threshold;

		return ret;
}
static int apds9190_set_aiht(struct i2c_client* client, int threshold)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_AIHTL_REG, threshold);
	mutex_unlock(&data->update_lock);
	data->aiht = threshold;

	return ret;
}

#if defined(APDS9190_PROXIMITY_CAL)
static int apds9190_set_pilt(struct i2c_client *client, int threshold)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_PILTL_REG, threshold);
	mutex_unlock(&data->update_lock);

	data->pilt = threshold;

	return ret;
}

static int apds9190_set_piht(struct i2c_client *client, int threshold)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&data->update_lock);
		ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_PIHTL_REG, threshold);
	mutex_unlock(&data->update_lock);
	
	data->piht = threshold;

	return ret;
}

#endif

static int apds9190_set_pers(struct i2c_client *client, int pers)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_PERS_REG, pers);
	mutex_unlock(&data->update_lock);

	if(ret < 0) {
		printk(KERN_INFO "%s, pers Register Write Fail\n",__func__);
		return -1;
	}

	data->pers = pers;

	return ret;
}

static int apds9190_set_config(struct i2c_client *client, int config)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_CONFIG_REG, config);
	mutex_unlock(&data->update_lock);

	data->config = config;

	return ret;
}

static int apds9190_set_ppcount(struct i2c_client *client, int ppcount)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_PPCOUNT_REG, ppcount);
	mutex_unlock(&data->update_lock);

	data->ppcount = ppcount;

	return ret;
}

static int apds9190_set_control(struct i2c_client *client, int control)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_CONTROL_REG, control);
	mutex_unlock(&data->update_lock);

	data->control = control;
	return ret;
}

#if defined(APDS9190_PROXIMITY_CAL)
void apds9190_swap(int *x, int *y)
{
     int temp = *x;
     *x = *y;
     *y = temp;
}

 static int apds9190_backup_crosstalk_data_fs(unsigned int val)
{
	int fd;
	int ret = 0;
	char buf[50];
	mm_segment_t old_fs = get_fs();

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", val);
#if defined (APDS9190_PROXIMITY_DEBUG)						

	printk("%s Enter\n", __FUNCTION__ );
	printk("%s\n", buf);
#endif
	set_fs(KERNEL_DS);
	fd = sys_open("/mpt/prox_calibration.dat",O_WRONLY|O_CREAT, 0664);

	if(fd >=0)
	{
		sys_write(fd, buf, sizeof(buf));
		sys_close(fd);
		set_fs(old_fs);
	}
	else
	{
		ret++;
		sys_close(fd);
		set_fs(old_fs);
		return ret	;
	}

	return ret;
}
static int apds9190_read_crosstalk_data_fs(void)
{
	int fd;
	int ret = 0;
	char read_buf[50];
	mm_segment_t old_fs = get_fs();

#if defined (APDS9190_PROXIMITY_DEBUG)						  
	printk("%s Enter\n", __FUNCTION__);
#endif
	memset(read_buf, 0, sizeof(read_buf));
	set_fs(KERNEL_DS);

	fd = sys_open("/mpt/prox_calibration.dat",O_RDONLY, 0);
	if(fd >=0)
	{
#if defined (APDS9190_PROXIMITY_DEBUG)					
		printk("%s Success read Prox Cross-talk from FS\n", __FUNCTION__);
#endif
		sys_read(fd, read_buf, sizeof(read_buf));
		sys_close(fd);
		set_fs(old_fs);
	}
	else
	{
#if defined (APDS9190_PROXIMITY_DEBUG)						
		printk("%s Prox Cross-talk get default value FS\n", __FUNCTION__);
		printk("%s Return default value : %d\n", __FUNCTION__, fd);
#endif		
		ret = -1;
		sys_close(fd);
		set_fs(old_fs);
		return ret;	  
	} 	  

	return (simple_strtol(read_buf, NULL, 10));

}

static int apds9190_Run_Cross_talk_Calibration(struct i2c_client *client)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	unsigned int sum_of_pdata = 0,temp_pdata[20];
	unsigned int ret=0,i=0,j=0,ArySize = 20,cal_check_flag = 0;
	unsigned int old_enable = 0;
	
#if defined (APDS9190_PROXIMITY_DEBUG)						
	printk("%s Enter \n", __FUNCTION__);
#endif

#if defined(APDS9190_PROXIMITY_CAL) //2012-12-05 Sanghun,Lee(eee3114.@lge.com) apds9190 proximity sensor calibration bug calibration after set enable
	old_enable = data->enable;
#endif

RE_CALIBRATION:

	sum_of_pdata = 0;
	apds9190_set_enable(client, 0x0D);
	
	msleep(50);

	for(i =0;i<20;i++)	{
		mutex_lock(&data->update_lock);	
		temp_pdata[i] = i2c_smbus_read_word_data(client, CMD_WORD|APDS9190_PDATAL_REG);
		mutex_unlock(&data->update_lock); 

		mdelay(6);
	}

	for(i=0; i<ArySize-1; i++)
		for(j=i+1; j<ArySize; j++)
			if(temp_pdata[i] > temp_pdata[j])
				apds9190_swap(temp_pdata+i, temp_pdata+j);

	for (i = 5;i<15;i++)
		sum_of_pdata = sum_of_pdata + temp_pdata[i];
	
	data->cross_talk = sum_of_pdata/10;
	if (data->cross_talk>770)
	{
		if (cal_check_flag == 0)
		{
			cal_check_flag = 1;
			goto RE_CALIBRATION;
		}
		else
		{
#if defined(APDS9190_PROXIMITY_CAL) //2012-12-05 Sanghun,Lee(eee3114.@lge.com) apds9190 proximity sensor calibration bug calibration after set enable
			apds9190_set_enable(client,0x00);
#endif			
			apds9190_set_enable(client,old_enable); 
			return -1;
		}
	}

	data->ps_threshold = 250 + data->cross_talk;
	data->ps_hysteresis_threshold = data->ps_threshold - 80;

	ret = apds9190_backup_crosstalk_data_fs(data->cross_talk);
#if defined (APDS9190_PROXIMITY_DEBUG)						
	printk("%s threshold : %d\n", __FUNCTION__, data->ps_threshold);
	printk("%s Hysteresis_threshold : %d\n",__FUNCTION__, data->ps_hysteresis_threshold);
#endif

#if defined(APDS9190_PROXIMITY_CAL) //2012-12-05 Sanghun,Lee(eee3114.@lge.com) apds9190 proximity sensor calibration bug calibration after set enable
	apds9190_set_enable(client,0x00);
#endif
	apds9190_set_enable(client,old_enable); 
#if defined (APDS9190_PROXIMITY_DEBUG)						
	printk("%s Leave\n", __FUNCTION__);
#endif
	return data->cross_talk;   
}

static void apds9190_Set_PS_Threshold_Adding_Cross_talk(struct i2c_client *client, int cal_data)
{
	struct apds9190_data *data = i2c_get_clientdata(client);

	if (cal_data>770)
		cal_data = 770;
	if (cal_data<0)
		cal_data = 0;
	
	data->cross_talk = cal_data; 
	data->ps_threshold = 250 + cal_data;
	data->ps_hysteresis_threshold = data->ps_threshold - 80;  

}
 static ssize_t apds9190_show_run_calibration(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%x\n", data->ps_cal_result);
}

static ssize_t apds9190_store_run_calibration(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);

	int ret;

	ret = apds9190_Run_Cross_talk_Calibration(client);
	if(ret < 0)
	{
#if defined (APDS9190_PROXIMITY_DEBUG)							
		printk("%s get default value :  %d\n", __FUNCTION__, ret);
#endif
		data->ps_cal_result = 0;
	}
	else
	{
#if defined (APDS9190_PROXIMITY_DEBUG)							
		printk("%s Succes cross-talk :  %d\n", __FUNCTION__, ret);
#endif
		data->ps_cal_result = 1;
	}

	return count;
}
static DEVICE_ATTR(run_calibration,  S_IWUSR | S_IRUGO|S_IWGRP |S_IRGRP |S_IROTH/*|S_IWOTH*/,
		   apds9190_show_run_calibration, apds9190_store_run_calibration);

 static ssize_t apds9190_show_crosstalk_data(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;
	
	ret = apds9190_read_crosstalk_data_fs();
	if(ret<0)
		return sprintf(buf, "Read fail\n");
	
	return sprintf(buf, "%d\n", ret);
}

static ssize_t apds9190_store_crosstalk_data(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret = 0;
	unsigned long val = simple_strtoul(buf, NULL, 10);

#if defined (APDS9190_PROXIMITY_DEBUG)						
	printk("%s Enter\n", __FUNCTION__ );
#endif
	ret = apds9190_backup_crosstalk_data_fs(val);
	if(ret != 0)
		return printk("File open get default value %d\n", ret);	

	data->cross_talk = val;
#if defined (APDS9190_PROXIMITY_DEBUG)						
	printk("Saved cross_talk val : %d\n", (int)val);
#endif
	
	return count;
}

static DEVICE_ATTR(prox_cal_data,  S_IWUSR | S_IRUGO|S_IWGRP |S_IRGRP |S_IROTH/*|S_IWOTH*/,
		   apds9190_show_crosstalk_data, apds9190_store_crosstalk_data);
#endif

void apds9190_change_ps_threshold(struct i2c_client *client) 	
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int irdata=0, ps_data = 0;

	apds9190_set_pers(apds_9190_i2c_client, 0x33);	// 29-Feb-2012 KK
													// repeat this because of the force first interrupt
	ps_data = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_PDATAL_REG);		
	irdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_IRDATAL_REG);
#if defined (APDS9190_PROXIMITY_DEBUG)						
	printk(">>>>>>>>>> apds9190_change_ps_threshold ps_data  = %d \n",ps_data);
	printk(">>>>>>>>>> apds9190_change_ps_threshold data->piht = %d \n",data->piht);
	printk(">>>>>>>>>> apds9190_change_ps_threshold data->pilt = %d \n",data->pilt);
#endif		
	if((ps_data > data->pilt) && (ps_data >= data->piht) && (irdata != (100*(1024*(256-data->atime)))/100))
	{
		/* far-to-NEAR */
		data->ps_detection = PROX_INPUT_NEAR;
#if defined (APDS9190_PROXIMITY_DEBUG)								
		printk("\n ===> far-to-Near\n");
#endif		
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, PROX_INPUT_NEAR);/* FAR-to-NEAR detection */	
		input_sync(data->input_dev_ps);
#if defined (APDS9190_PROXIMITY_DEBUG)								
		printk(">>>>>>>>>> apds9190_change_ps_threshold 1 \n");
		printk(">>>>>>>>>> apds9190_change_ps_threshold 1 ps_data= %d\n",ps_data);
		printk(">>>>>>>>>> apds9190_change_ps_threshold 1 ps_threshold= %d\n",data->ps_threshold);				
#endif		
		apds9190_set_pilt(apds_9190_i2c_client, data->ps_hysteresis_threshold);
		apds9190_set_piht(apds_9190_i2c_client, 1023);

	}
	else if((ps_data < data->piht) && (ps_data <= data->pilt)) 
	{
		/* near-to-FAR */
		data->ps_detection = PROX_INPUT_FAR;

		input_report_abs(data->input_dev_ps, ABS_DISTANCE, PROX_INPUT_FAR);/* NEAR-to-FAR detection */	
		input_sync(data->input_dev_ps);

		apds9190_set_pilt(client, 0);
		apds9190_set_piht(client, data->ps_threshold);
#if defined (APDS9190_PROXIMITY_DEBUG)								
		printk("\n ===> near-to-FAR\n");
#endif
	}
	else if ( (irdata == (100*(1024*(256-data->atime)))/100) && (data->ps_detection == PROX_INPUT_NEAR) )
	{
		/* under strong ambient light condition*/
		/* near-to-FAR */
		data->ps_detection = PROX_INPUT_FAR;
#if defined (APDS9190_PROXIMITY_DEBUG)										
		printk(">>>>>>>>>> ps_threshold 3 \n");
#endif
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, PROX_INPUT_FAR);/* NEAR-to-FAR detection */	
		input_sync(data->input_dev_ps);

		/*Keep the threshold NEAR condition to prevent the frequent Interrup under strong ambient light*/

		apds9190_set_pilt(client, data->ps_hysteresis_threshold);
		apds9190_set_piht(client, 1023);
#if defined (APDS9190_PROXIMITY_DEBUG)								
		printk("* Set PS Threshold NEAR condition to prevent the frequent Interrupt under strong ambient light\n");		
		printk("\n ===> near-to-FAR\n\n");
#endif		
	}
	else if ( (data->pilt == 1023) && (data->piht == 0) )
	{
		/* this is the first near-to-far forced interrupt */
		data->ps_detection = PROX_INPUT_FAR;
#if defined (APDS9190_PROXIMITY_DEBUG)										
		printk(">>>>>>>>>> ps_threshold 4 \n");
#endif
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, PROX_INPUT_FAR);/* NEAR-to-FAR detection */	
		input_sync(data->input_dev_ps);

		apds9190_set_pilt(client, 0);
		apds9190_set_piht(client, data->ps_threshold);
#if defined (APDS9190_PROXIMITY_DEBUG)								
		printk("\n near-to-FAR detected\n\n");
#endif
	}
}


/*
 * SysFS support
 */

static ssize_t
apds9190_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%d\n", data->sw_mode);
}

static ssize_t
apds9190_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *pdev = i2c_get_clientdata(client);
	pm_message_t dummy_state;
	int mode;

	dummy_state.event = 0;

	sscanf(buf, "%d", &mode);

	if ((mode != PROX_STAT_SHUTDOWN) && (mode != PROX_STAT_OPERATING)) {
#if defined (APDS9190_PROXIMITY_DEBUG)										
		printk(KERN_INFO "Usage: echo [0 | 1] > enable");
		printk(KERN_INFO " 0: disable\n");
		printk(KERN_INFO " 1: enable\n");
#endif		
		return count;
	}

#if defined(APDS9190_PROXIMITY_CAL)
	if (mode)
	{
		pdev->cross_talk = apds9190_read_crosstalk_data_fs();
						
		if(pdev->cross_talk <= 0 || pdev->cross_talk>770)
		pdev->cross_talk = PS_DEFAULT_CROSS_TALK;
#if defined (APDS9190_PROXIMITY_DEBUG)												
				printk("%s Cross_talk : %d\n", __FUNCTION__, pdev->cross_talk);
#endif		
		apds9190_Set_PS_Threshold_Adding_Cross_talk(client, pdev->cross_talk);	
#if defined (APDS9190_PROXIMITY_DEBUG)										

		printk("%s apds9190_Set_PS_Threshold_Adding_Cross_talk\n", __FUNCTION__);	
		printk("%s apds9190_Set_PS_Threshold_Adding_Cross_talk = %d\n", __FUNCTION__,pdev->cross_talk);
#endif		
	}
#endif


	if (mode == pdev->sw_mode)
	{
#if defined (APDS9190_PROXIMITY_DEBUG)												
		printk(KERN_INFO "mode is already %d\n", pdev->sw_mode);
#endif
		return count;
	}
	else 
	{
		if (mode)
		{
			apds9190_resume(client);
#if defined (APDS9190_PROXIMITY_DEBUG)													
			printk(KERN_INFO "Power On Enable\n");
#endif
		}
		else 
		{
			apds9190_suspend(client, dummy_state);
#if defined (APDS9190_PROXIMITY_DEBUG)													
			printk(KERN_INFO "Power Off Disable\n");
#endif
		}
	}

	return count;
}
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, apds9190_enable_show, apds9190_enable_store);	


static ssize_t apds9190_show_ptime(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);
	if(client != NULL)
		return sprintf(buf, "%d\n",data->ptime);
	else
		return -1;
}

static ssize_t apds9190_store_ptime(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long rdata = simple_strtoul(buf, NULL, 10);	

	if(client != NULL)
		apds9190_set_ptime(client,rdata);
	else
		return -1;

	return count;
}

static DEVICE_ATTR(ptime, S_IRUGO | S_IWUSR, apds9190_show_ptime, apds9190_store_ptime);	

static ssize_t apds9190_show_wtime(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);

	if(client != NULL)
		return sprintf(buf, "%d\n",data->wtime);
	else
		return -1;
}

static ssize_t apds9190_store_wtime(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long rdata = simple_strtoul(buf, NULL, 10);	

	if(client != NULL)
		apds9190_set_wtime(client,rdata);
	else
		return -1;

	return count;
}

static DEVICE_ATTR(wtime, S_IRUGO | S_IWUSR, apds9190_show_wtime, apds9190_store_wtime);	

static ssize_t apds9190_show_ppcount(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);

	if(client != NULL)
		return sprintf(buf, "%d\n",data->ppcount);
	else
		return -1;
}

static ssize_t apds9190_store_ppcount(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	int rdata;
	sscanf(buf, "%d", &rdata);

	if(client != NULL)
		apds9190_set_ppcount(client,rdata);
	else
		return -1;

	return count;
}

static DEVICE_ATTR(ppcount, S_IRUGO | S_IWUSR, apds9190_show_ppcount, apds9190_store_ppcount);	

static ssize_t apds9190_show_pers(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);

	if(client != NULL)
		return sprintf(buf, "%d\n",data->pers);
	else
		return -1;
}

static ssize_t apds9190_store_pers(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long rdata = simple_strtoul(buf, NULL, 10);	

	if(client != NULL)
		apds9190_set_pers(client,rdata);
	else
		return -1;

	return count;
}

static DEVICE_ATTR(pers, S_IRUGO | S_IWUSR, apds9190_show_pers, apds9190_store_pers);

#if defined(APDS9190_PROXIMITY_CAL)
static ssize_t apds9190_show_control(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct apds9190_data *data = i2c_get_clientdata(client);
    int control = 0;

    control = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS9190_CONTROL_REG);
	if (control < 0) 
	{
		dev_err(&client->dev, "%s: i2c error %d in reading reg 0x%x\n",  __func__, control, CMD_BYTE|APDS9190_CONTROL_REG);
		return control;
	} 

    return sprintf(buf, "reg(%x),buf(%x)\n", control,data->control);
}


static ssize_t apds9190_store_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    unsigned long val = simple_strtoul(buf, NULL, 10);
    int ret = 0;

    ret = apds9190_set_control(client, val);

    if (ret < 0)
        return ret;

    return count;
}
static DEVICE_ATTR(control,  S_IWUSR | S_IRUGO | S_IRGRP | S_IROTH , apds9190_show_control, apds9190_store_control);

static ssize_t apds9190_show_ps_threshold(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%x\n", data->ps_threshold);
}

static ssize_t apds9190_store_ps_threshold(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t ps_th)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);

	data->ps_threshold = val;

	return ps_th;
}

static DEVICE_ATTR(ps_threshold,  S_IWUSR | S_IRUGO|S_IWGRP |S_IRGRP |S_IROTH/*|S_IWOTH*/,
		   apds9190_show_ps_threshold, apds9190_store_ps_threshold);

static ssize_t apds9190_show_ps_hysteresis_threshold(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%x\n", data->ps_hysteresis_threshold);
}

static ssize_t apds9190_store_ps_hysteresis_threshold(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t ps_hy_th)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);

	data->ps_hysteresis_threshold = val;

	return ps_hy_th;
}

static DEVICE_ATTR(ps_hysteresis,  S_IWUSR | S_IRUGO|S_IWGRP |S_IRGRP |S_IROTH/*|S_IWOTH*/,
		   apds9190_show_ps_hysteresis_threshold, apds9190_store_ps_hysteresis_threshold);

#endif

static ssize_t apds9190_show_pdata(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);

	if(client != NULL){
			int pdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS9190_PDATAL_REG);

			return sprintf(buf, "%d\n",pdata);
	}
	else{
			return -1;
	}
}

static DEVICE_ATTR(pdata, S_IRUGO,  apds9190_show_pdata, NULL);


static ssize_t apds9190_status_show(struct device *dev, 
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);

	if(client != NULL)
		return sprintf(buf, "%d\n",data->ps_detection);
	else 
		return -1;
}
static DEVICE_ATTR(show, S_IRUGO | S_IWUSR, apds9190_status_show, NULL);	

static struct attribute *apds9190_attributes[] = {
	&dev_attr_show.attr,
	&dev_attr_enable.attr,
	&dev_attr_ptime.attr,
	&dev_attr_wtime.attr,
	&dev_attr_ppcount.attr,
	&dev_attr_pers.attr,
	&dev_attr_pdata.attr,
#if defined(APDS9190_PROXIMITY_CAL)
	&dev_attr_control.attr,
	&dev_attr_ps_threshold.attr,
	&dev_attr_ps_hysteresis.attr,
	&dev_attr_run_calibration.attr,
	&dev_attr_prox_cal_data.attr,
#endif
	NULL
};

static const struct attribute_group apds9190_attr_group = {
		.attrs = apds9190_attributes,
};

/*
 * Initialization function
 */
static int apds9190_init_client(struct i2c_client *client)
{
	struct apds9190_data *data = i2c_get_clientdata(apds_9190_i2c_client);
	int err = 0;
	int id;

	err = apds9190_set_enable(apds_9190_i2c_client, 0);

	if (err < 0)
		return err;

	id = i2c_smbus_read_byte_data(apds_9190_i2c_client, CMD_BYTE|APDS9190_ID_REG);
	if (id == 0x29) {
		printk("APDS-9190\n");
	}
	else {
		printk("NOT APDS-9190\n");
		return -EIO;
	}
	
	err =  apds9190_set_atime(apds_9190_i2c_client, ATIME);
	if(err < 0){
		printk(KERN_INFO "%s, atime set Fail\n",__func__);
		goto EXIT;
	}	

	err = apds9190_set_ptime(apds_9190_i2c_client, PTIME);
	if(err < 0){
		printk(KERN_INFO "%s, ptime set Fail\n",__func__);
		goto EXIT;
	}
	err = apds9190_set_wtime(apds_9190_i2c_client, WTIME);
	if(err < 0){
		printk(KERN_INFO "%s, wtime set Faile\n",__func__);
		goto EXIT;
	}

	err = apds9190_set_ppcount(apds_9190_i2c_client, DEFAULT_PPCOUNT); // Pulse count for proximity
	if(err < 0){
		printk(KERN_INFO "%s, ppcount set Fail\n",__func__);
		goto EXIT;
	}
	err = apds9190_set_config(apds_9190_i2c_client, 0x00); // Wait long timer <- no needs so set 0
	if(err < 0){
		printk(KERN_INFO "%s, config set Fail\n",__func__);
		goto EXIT;
	}
	err = apds9190_set_control(apds_9190_i2c_client, DEFAULT_PDRIVE| PDIODE | PGAIN | AGAIN);
	if(err < 0){
		printk(KERN_INFO "%s, control set Fail\n",__func__);
		goto EXIT;
	}

	err = apds9190_set_pilt(apds_9190_i2c_client, 1023); // init threshold for proximity
	if(err < 0){
		printk(KERN_INFO "%s, pilt set Fail\n",__func__);
		goto EXIT;
	}
	
	err = apds9190_set_piht(apds_9190_i2c_client, 0);
	if(err < 0){
		printk(KERN_INFO "%s, piht set Fail\n",__func__);
		goto EXIT;
	}

	data->ps_detection = PROX_INPUT_FAR;			// we are forcing Near-to-Far interrupt, so this is defaulted to 1
	err = apds9190_set_ailt(apds_9190_i2c_client, 0);
	if(err < 0){
		printk(KERN_INFO "%s, ailt set Fail\n",__func__);
		goto EXIT;
	}	
	err = apds9190_set_aiht(apds_9190_i2c_client, (99*(1024*(256-data->atime)))/100);
	if(err < 0){
		printk(KERN_INFO "%s, aiht set Fail\n",__func__);
		goto EXIT;
	}	
	apds9190_set_pers(client, 0x03);	// 3 consecutive Interrupt persistence
								// 29-Feb-2012 KK, 30-Aug-2012 SH
								// Force PS interrupt every PS conversion cycle
								// instead of comparing threshold value
	if(err < 0){
		printk(KERN_INFO "%s, pers set Fail\n",__func__);
		goto EXIT;
	}
	return 0;
EXIT:
	return err;
}

/* PS interrupt routine */
static void apds9190_work_handler(struct work_struct *work)
{
	struct apds9190_data *data = 
			container_of(work, struct apds9190_data, dwork);
	struct proximity_platform_data		*pdev = NULL;	
	int	status=0,cdata=0;
	int irdata=0;
#if	APDS900_SENSOR_DEBUG 
	int pdata=0;
	int ailt=0,aiht=0;
	int pilt=0,piht=0;
#endif
	int org_enable = data->enable;
	
	pdev = data->client->dev.platform_data;


	if(wake_lock_active(&data->wakelock))
		wake_unlock(&data->wakelock);
	wake_lock_timeout(&data->wakelock, 2 * HZ);


	status = i2c_smbus_read_byte_data(apds_9190_i2c_client, CMD_BYTE|APDS9190_STATUS_REG);

	i2c_smbus_write_byte_data(apds_9190_i2c_client, CMD_BYTE|APDS9190_ENABLE_REG, 1);	/* disable 9190's ADC first */
#if defined (APDS9190_PROXIMITY_DEBUG)										
	printk("status = %x\n", status);
#endif
	if((status & APDS9190_STATUS_PINT_AINT) == 0x30) 
	{
		/* both ALS and PS are interrupted */
		disable_irq(data->irq);

		/* Change ALS threshold under the strong ambient light */
		cdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_CDATAL_REG);
	
//			printk(KERN_INFO "%s, [APDS9190_STATUS_PINT_AINT1] status : %d,   cdata : %d, isNear : %d\n",__func__, status, cdata, data->isNear);		

		if (cdata == 100*(1024*(256-data->atime))/100)
		{
//				pdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_PDATAL_REG);													
//				printk(KERN_INFO "%s, [APDS9190_STATUS_PINT_AINT] cdata : %d, pdata : %d\n", __func__, cdata, pdata);

//				printk(KERN_INFO "%s, [APDS9190_STATUS_PINT_AINT] status change Near to Far while Near status but couldn't recognize Far\n", __func__);
//				printk(KERN_INFO "%s, [APDS9190_STATUS_PINT_AINT] Force status to change Far\n",__func__);

			apds9190_set_ailt(apds_9190_i2c_client, (99*(1024*(256-data->atime)))/100);		
			apds9190_set_aiht(apds_9190_i2c_client, (100*(1024*(256-data->atime)))/100);
#if defined (APDS9190_PROXIMITY_DEBUG)													
			printk("* Set ALS Theshold under the strong sunlight\n");
#endif
		}
		else
		{
			apds9190_set_ailt(apds_9190_i2c_client, 0);		
			apds9190_set_aiht(apds_9190_i2c_client, (99*(1024*(256-data->atime)))/100);
#if defined (APDS9190_PROXIMITY_DEBUG)													
			printk("* Set ALS Theshold for normal mode\n");
#endif
		}

		/* check if this is triggered by strong ambient light  */		
		irdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_IRDATAL_REG);
		if (irdata != (100*(1024*(256-data->atime)))/100)
			apds9190_change_ps_threshold(apds_9190_i2c_client);
		else 
		{
			if (data->ps_detection == PROX_INPUT_NEAR) 
			{
				apds9190_change_ps_threshold(apds_9190_i2c_client);			
#if defined (APDS9190_PROXIMITY_DEBUG)											
				printk("* Triggered by background ambient noise\n");	
				printk("\n ==> near-to-FAR\n");
#endif				
			}
			else 
			{
				/*Keep the threshold NEAR condition to prevent the frequent Interrupt under strong ambient light*/
				apds9190_set_pilt(apds_9190_i2c_client, data->ps_hysteresis_threshold);
				apds9190_set_piht(apds_9190_i2c_client, 1023);
#if defined (APDS9190_PROXIMITY_DEBUG)														
				printk("* Set PS Threshold NEAR condition to prevent the frequent Interrupt under strong ambient light\n");
				printk("* Triggered by background ambient noise\n");
				printk("\n ==> maintain FAR \n");
#endif				
			}
		}
		apds9190_set_command(apds_9190_i2c_client, 2);	/* 2 = CMD_CLR_PS_ALS_INT */
		enable_irq(data->irq);
	}
	else if((status & APDS9190_STATUS_PINT)  == 0x20) 
	{
		/* only PS is interrupted */	
		disable_irq(data->irq);

		/* Change ALS threshold under the strong ambient light */
		cdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_CDATAL_REG);
//			printk(KERN_INFO "%s, [APDS9190_STATUS_PINT2] status : %d,   cdata : %d, isNear : %d\n",__func__, status, cdata, data->isNear);		
		if (cdata == 100*(1024*(256-data->atime))/100)
		{
			apds9190_set_ailt(apds_9190_i2c_client, (99*(1024*(256-data->atime)))/100);		
			apds9190_set_aiht(apds_9190_i2c_client, (100*(1024*(256-data->atime)))/100);
#if defined (APDS9190_PROXIMITY_DEBUG)													
			printk("* Set ALS Theshold under the strong sunlight\n");
#endif
		}
		else
		{
			apds9190_set_ailt(apds_9190_i2c_client, 0);		
			apds9190_set_aiht(apds_9190_i2c_client, (99*(1024*(256-data->atime)))/100);
#if defined (APDS9190_PROXIMITY_DEBUG)													
			printk("* Set ALS Theshold for normal mode\n");
#endif
		}

		
		/* check if this is triggered by strong ambient light */
		irdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_IRDATAL_REG);
		if (irdata != (100*(1024*(256-data->atime)))/100)
		apds9190_change_ps_threshold(apds_9190_i2c_client);
		else 
		{
			if (data->ps_detection == PROX_INPUT_NEAR) 
			{
				apds9190_change_ps_threshold(apds_9190_i2c_client);			
#if defined (APDS9190_PROXIMITY_DEBUG)														
		       	printk("* Triggered by background ambient noise\n");	
				printk("\n ==> near-to-FAR\n");
#endif				
			}
			else 
			{
				/*Keep the threshold NEAR condition to prevent the frequent Interrupt under strong ambient light*/
				apds9190_set_pilt(apds_9190_i2c_client, data->ps_hysteresis_threshold);
				apds9190_set_piht(apds_9190_i2c_client, 1023);
#if defined (APDS9190_PROXIMITY_DEBUG)														
				printk("* Set PS Threshold NEAR condition to prevent the frequent Interrupt under strong ambient light\n");
				printk("* Triggered by background ambient noise\n");
				printk("\n ==> maintain FAR \n");
#endif				
			}
		}
		apds9190_set_command(apds_9190_i2c_client, 0);	/* 0 = CMD_CLR_PS_INT */
		enable_irq(data->irq);
	}
	else if((status & APDS9190_STATUS_AINT) == 0x10) 
	{
		/* only ALS is interrupted */

		/* Change ALS Threshold under the strong ambient light */
		cdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_CDATAL_REG);		
		if (cdata == 100*(1024*(256-data->atime))/100)
		{
			apds9190_set_ailt(apds_9190_i2c_client, (99*(1024*(256-data->atime)))/100);		
			apds9190_set_aiht(apds_9190_i2c_client, (100*(1024*(256-data->atime)))/100);
#if defined (APDS9190_PROXIMITY_DEBUG)																	
			printk("* Set ALS Theshold under the strong sunlight\n");
#endif
		}
		else
		{
			apds9190_set_ailt(apds_9190_i2c_client, 0);		
			apds9190_set_aiht(apds_9190_i2c_client, (99*(1024*(256-data->atime)))/100);
			printk("* Set ALS Theshold for normal mode\n");
		}
	
		/* check if this is triggered by the strong ambient light */
		irdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_IRDATAL_REG);
		if (irdata != (100*(1024*(256-data->atime)))/100)
			apds9190_change_ps_threshold(apds_9190_i2c_client);
		else {
			 
			if (data->ps_detection == PROX_INPUT_NEAR) {
				apds9190_change_ps_threshold(apds_9190_i2c_client);			
#if defined (APDS9190_PROXIMITY_DEBUG)																					
				printk("* Triggered by background ambient noise\n");
				printk("\n ==> near-to-FAR\n");
#endif				
			}
			else {

				/*Keep the threshold NEAR condition to prevent the frequent Interrupt under strong ambient light*/
				apds9190_set_pilt(apds_9190_i2c_client, data->ps_hysteresis_threshold);
				apds9190_set_piht(apds_9190_i2c_client, 1023);
#if defined (APDS9190_PROXIMITY_DEBUG)														
				printk("* Set PS Threshold NEAR condition to prevent the frequent Interrupt under strong ambient light\n");
				printk("* Triggered by background ambient noise\n");
				printk("\n ==> maintain FAR \n");
#endif				
			}
		}
		apds9190_set_command(apds_9190_i2c_client, 1);	/* 1 = CMD_CLR_ALS_INT */
	}	

#if APDS900_SENSOR_DEBUG
	pdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_PDATAL_REG);
	cdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_CDATAL_REG);
	irdata = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_IRDATAL_REG);
	ailt = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_AILTL_REG);
	aiht = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_AIHTL_REG);
	pilt = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_PILTL_REG);
	piht = i2c_smbus_read_word_data(apds_9190_i2c_client, CMD_WORD|APDS9190_PIHTL_REG);

	printk("cdata = %d, irdata = %d, pdata = %d\n", cdata,irdata,pdata);
	printk("ailt = %d, aiht = %d\n", ailt,aiht);
	printk("pilt = %d, piht = %d\n", pilt,piht);

	printk("--------------------\n");
#endif

	i2c_smbus_write_byte_data(apds_9190_i2c_client, CMD_BYTE|APDS9190_ENABLE_REG, org_enable);	
}

static irqreturn_t apds_9190_irq_handler(int irq, void *dev_id)						   
{
	struct apds9190_data *data = dev_id;
	struct proximity_platform_data		*pdata;	
	unsigned long delay;
	int ret;

	pdata = data->client->dev.platform_data;
	spin_lock(&data->lock);

	delay = msecs_to_jiffies(0);
	ret = queue_work(proximity_wq, &data->dwork);
	if(ret < 0){
#if defined (APDS9190_PROXIMITY_DEBUG)												
		printk(KERN_ERR "%s, queue_work Erro\n",__func__);
#endif
	}
	spin_unlock(&data->lock);
	return IRQ_HANDLED;
}	

/*
 * I2C init/probing/exit functions
 */

static struct i2c_driver apds9190_driver;
static int __devinit apds9190_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct apds9190_data *data;
	struct proximity_platform_data		*pdata;
	pm_message_t dummy_state;
	int err = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		err = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct apds9190_data), GFP_KERNEL);
	apds_9190_i2c_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}

	memset(data, 0x00, sizeof(struct apds9190_data));

#if defined(APDS9190_PROXIMITY_CAL)
	data->cross_talk=PS_DEFAULT_CROSS_TALK;
#endif	

	wake_lock_init(&data->wakelock, WAKE_LOCK_SUSPEND, "apds9190");

	INIT_WORK(&data->dwork, apds9190_work_handler);		

	data->client = client;
	apds_9190_i2c_client = client;
	i2c_set_clientdata(data->client, data);

	data->ps_detection = PROX_INPUT_FAR;
	data->input_dev_ps = input_allocate_device();		

#if defined(APDS9190_PROXIMITY_CAL) // 2012-12-20Sanghun,Lee(eee3114.@lge.com) apds9190 probe wbt 313217 null check
		if (!data->input_dev_ps) {		
		printk(KERN_ERR "%s: not enough memory for input device\n", __func__);
		goto exit_kfree;
	}
#endif

	data->input_dev_ps->name = "proximity";
	data->input_dev_ps->phys = "proximity/input2";	
	set_bit(EV_SYN, data->input_dev_ps->evbit);
	set_bit(EV_ABS, data->input_dev_ps->evbit);
	input_set_abs_params(data->input_dev_ps, ABS_DISTANCE, 0, 1, 0, 0);

	err = input_register_device(data->input_dev_ps);

	if (err) {
		DEBUG_MSG("Unable to register input device: %s\n",
						data->input_dev_ps->name);
		goto exit_input_register_device_failed;
	}

	pdata = data->client->dev.platform_data;
	if(NULL == pdata){
		printk(KERN_INFO "platform data is NULL");
		return -1;
	}

	data->irq = gpio_to_irq(pdata->irq_num);

	spin_lock_init(&data->lock);
	mutex_init(&data->update_lock);

	data->enable = 0;	/* default mode is standard */
	dev_info(&client->dev, "enable = %s\n", data->enable ? "1" : "0");

	err = pdata->power(1);
	if(err < 0) {
		printk(KERN_INFO "%s,Proximity Power On Fail in Probe\n",__func__);
		goto exit_kfree;
	}
	
	mdelay(50);

	/* Initialize the APDS9190 chip */
	err = apds9190_init_client(apds_9190_i2c_client);
	if(err < 0) {
		printk(KERN_INFO "%s,Proximity apds9190_init_client Fail in Probe\n",__func__);
		goto exit_kfree;
	}
	
	if(request_irq(data->irq,apds_9190_irq_handler,IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,"proximity_irq", data) < 0){
		err = -EIO;
		goto exit_request_irq_failed;
	}
	err = irq_set_irq_wake(data->irq, 1);
	if (err)
		irq_set_irq_wake(data->irq, 0);

		
	data->sw_mode = PROX_STAT_OPERATING;

	/* Register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &apds9190_attr_group);
	if (err)
		goto exit_kfree;

	dummy_state.event = 0;
	apds9190_suspend(data->client, dummy_state);


	return 0;
exit_input_register_device_failed:	
exit_request_irq_failed:
exit_kfree:
 	/* LGE_CHANGE - bohyun.jung@lge.com [2013.02.08]
	 * destory wakelock before kfree data, otherwise corrupted wakelock remains in inactive_locks. (kernel panic) */
	wake_lock_destroy(&data->wakelock);
	dev_info(&client->dev, "probe error\n");
	kfree(data);
exit:
	return err;
}

static int __devexit apds9190_remove(struct i2c_client *client)
{
	struct apds9190_data *data = i2c_get_clientdata(client);

	DEBUG_MSG("apds9190_remove\n");

	apds9190_set_enable(client, 0);

	irq_set_irq_wake(data->irq, 0);
	free_irq(data->irq, NULL);
	input_unregister_device(data->input_dev_ps);
	input_free_device(data->input_dev_ps);

 	/* LGE_CHANGE - bohyun.jung@lge.com [2013.02.08]
	 * destory wakelock before kfree data, otherwise corrupted wakelock remains in inactive_locks. (kernel panic) */
	wake_lock_destroy(&data->wakelock);
	kfree(data);		
	/* Power down the device */

	sysfs_remove_group(&client->dev.kobj, &apds9190_attr_group);

	return 0;
}

static int apds9190_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct apds9190_data *data = i2c_get_clientdata(apds_9190_i2c_client);	
	struct proximity_platform_data* pdata = NULL;
	int err;

	if(!data->sw_mode)
			return 0;

	pdata = data->client->dev.platform_data;

#if defined (APDS9190_PROXIMITY_DEBUG)																					
	printk(KERN_INFO "%s, apds9190_suspend\n",__func__);
#endif
	if(NULL == pdata){
			printk(KERN_INFO "Platform data is NULL\n");
			return -1;
	}

	apds9190_set_enable(client, 0);
	apds9190_set_command(apds_9190_i2c_client, 2);
	
	cancel_work_sync(&data->dwork);
	flush_work(&data->dwork);
	flush_workqueue(proximity_wq);

	data->sw_mode = PROX_STAT_SHUTDOWN;
    	disable_irq(data->irq);
	err = pdata->power(0);
	if(err < 0) {
#if defined (APDS9190_PROXIMITY_DEBUG)												
		printk(KERN_INFO "%s, Proximity Power Off Fail in susped\n",__func__);
#endif
		return err;
	}
		
	irq_set_irq_wake(data->irq, 0);

	
	return 0;
}

static int apds9190_setting(void)
{

#ifdef CONFIG_LGE_PCB_VERSION
	if (!strncmp(hw_pcb_version, "Unknown", 7)) 
	{
		printk(KERN_INFO "%s, Get PCB Version using rpc\n", __func__);
		lge_set_hw_version_string((char *)hw_pcb_version);
		printk(KERN_INFO "%s, PCB Ver. : %s\n",__func__, hw_pcb_version);
	}

	if(!strncmp(hw_pcb_version, "B", 1)) 
	{
		apds_proxi_high_threshold = REV_B_APDS_PROXIMITY_HIGH_THRESHHOLD;
		apds_proxi_low_threshold = REV_B_APDS_PROXIMITY_LOW_THRESHHOLD;			

		apds_ppcount = REV_B_PPCOUNT;
		apds_pdrive = REV_B_PDRIVE;
	}
	else if(!strncmp(hw_pcb_version, "C", 1))
	{
		apds_proxi_high_threshold = REV_C_APDS_PROXIMITY_HIGH_THRESHHOLD;
		apds_proxi_low_threshold = REV_C_APDS_PROXIMITY_LOW_THRESHHOLD;			

		apds_ppcount = REV_C_PPCOUNT;
		apds_pdrive = REV_C_PDRIVE;
	}
	else if(!strncmp(hw_pcb_version, "D", 1) || !strncmp(hw_pcb_version, "E", 1)|| (!strncmp(hw_pcb_version, "1.0", 3)))
	{
		apds_proxi_high_threshold = REV_D_APDS_PROXIMITY_HIGH_THRESHHOLD;
		apds_proxi_low_threshold = REV_D_APDS_PROXIMITY_LOW_THRESHHOLD;						

		apds_ppcount = REV_D_PPCOUNT;
		apds_pdrive = REV_D_PDRIVE;
	}
	else
	{
		printk(KERN_INFO "%s, Init Threshold, Pulse Count, LED Power Fail\n",__func__);
		apds_proxi_high_threshold = DEFAULT_APDS_PROXIMITY_HIGH_THRESHHOLD;
		apds_proxi_low_threshold = DEFAULT_APDS_PROXIMITY_LOW_THRESHHOLD;

		apds_ppcount = DEFAULT_PPCOUNT;
		apds_pdrive = DEFAULT_PDRIVE;
		return 0;
	}
	return 1;
#else	
#if defined (APDS9190_PROXIMITY_DEBUG)										

	printk(KERN_INFO "%s, Non PCB Rev Read\n",__func__);
#endif
	return -1;

#endif
}
static int apds9190_resume(struct i2c_client *client)
{
	struct apds9190_data *data = i2c_get_clientdata(apds_9190_i2c_client);	
	struct proximity_platform_data* pdata = NULL;
	int ret;

	pdata = data->client->dev.platform_data;
#if defined (APDS9190_PROXIMITY_DEBUG)												
	printk(KERN_INFO "%s, apds9190_resume\n",__func__);
#endif
	if(NULL == pdata){
#if defined (APDS9190_PROXIMITY_DEBUG)												
			printk(KERN_INFO "Platform data is NULL");
#endif
			return -1;
	}

	enable_irq(data->irq);
	ret = pdata->power(1);
	if(ret < 0) {
#if defined (APDS9190_PROXIMITY_DEBUG)												
		printk(KERN_INFO "%s,Proximity Power On Fail in Resume\n",__func__);
#endif
		return ret;
	}
		
	mdelay(50);

	ret = apds9190_setting();

	ret = apds9190_init_client(apds_9190_i2c_client);
	
	ret = apds9190_set_enable(apds_9190_i2c_client,0x3F);

	if(ret < 0) {
#if defined (APDS9190_PROXIMITY_DEBUG)												
		printk(KERN_INFO "%s,Proximity apds_9190_initialize Fail in Resume\n",__func__);
#endif
		return ret;
	}

	data->sw_mode = PROX_STAT_OPERATING;

	ret = irq_set_irq_wake(data->irq, 1);
	if(ret)
		irq_set_irq_wake(data->irq, 0);

	apds9190_set_command(apds_9190_i2c_client, 2);

	return 0;
}


static const struct i2c_device_id apds9190_id[] = {
		{ "proximity_apds9190", 0 },
		{ }
};
MODULE_DEVICE_TABLE(i2c, apds9190_id);

static struct i2c_driver apds9190_driver = {
	.driver = {
		.name	= APDS9190_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.probe	= apds9190_probe,
	.remove	= __devexit_p(apds9190_remove),
	.id_table = apds9190_id,
};

static int __init apds9190_init(void)
{
	int err;
	if(proximity_wq == NULL) {
		proximity_wq = create_singlethread_workqueue("proximity_wq");
			if(NULL == proximity_wq)
				return -ENOMEM;
		err = i2c_add_driver(&apds9190_driver);
		if(err < 0){
			printk(KERN_INFO "Failed to i2c_add_driver \n");
			return err;
		}
	}
	return 0;
}

static void __exit apds9190_exit(void)
{
	i2c_del_driver(&apds9190_driver);
	if(proximity_wq != NULL) {
		if(proximity_wq)
			destroy_workqueue(proximity_wq);		
	}
}

MODULE_AUTHOR("Lee Kai Koon <kai-koon.lee@avagotech.com>");
MODULE_DESCRIPTION("APDS9190 proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(apds9190_init);
module_exit(apds9190_exit);


