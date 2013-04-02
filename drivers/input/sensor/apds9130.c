/*
 *  apds9130.c - Linux kernel modules for proximity sensor
 *
 *  Copyright (C) 2012 Lee Kai Koon <kai-koon.lee@avagotech.com>
 *  Copyright (C) 2012 Avago Technologies
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
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/wakelock.h>

//apds9130 proximity sensor calibration
#define APDS9130_PROXIMITY_CAL
#if defined(APDS9130_PROXIMITY_CAL)
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define PS_DEFAULT_CROSS_TALK 200
#endif

#define APDS9130_DRV_NAME	"apds9130"
#define DRIVER_VERSION		"1.0.3"

#define APDS9130_PS_DETECTION_THRESHOLD		600
#define APDS9130_PS_HSYTERESIS_THRESHOLD	500

#ifdef CONFIG_LEDS_LP5521
#define APDS9130_PS_PULSE_NUMBER		12
#else
#define APDS9130_PS_PULSE_NUMBER		9
#endif
/* Change History
 *
 * 1.0.0	Funcamental Functions of APDS-9130
 *
 * 1.0.1	Remove ioctl interface, remain using sysfs
 * 1.0.2	Add LG cross-talk calibration process
 * 1.0.3	Remove Polling mode
 */

#define APDS9130_IOCTL_PS_ENABLE		1
#define APDS9130_IOCTL_PS_GET_ENABLE	2
#define APDS9130_IOCTL_PS_GET_PDATA		3	// pdata
#define APDS9130_IOCTL_PS_GET_PSAT		4	// ps saturation - used to detect if ps is triggered by bright light
#define APDS9130_IOCTL_PS_POLL_DELAY	5

#define APDS9130_DISABLE_PS				0
#define APDS9130_ENABLE_PS_WITH_INT		1
#define APDS9130_ENABLE_PS_NO_INT		2

#define APDS9130_PS_POLL_SLOW			0	// 1 Hz (1s)
#define APDS9130_PS_POLL_MEDIUM			1	// 10 Hz (100ms)
#define APDS9130_PS_POLL_FAST			2	// 20 Hz (50ms)

/*
 * Defines
 */

#define APDS9130_ENABLE_REG	0x00
#define APDS9130_PTIME_REG	0x02
#define APDS9130_WTIME_REG	0x03
#define APDS9130_PILTL_REG	0x08
#define APDS9130_PILTH_REG	0x09
#define APDS9130_PIHTL_REG	0x0A
#define APDS9130_PIHTH_REG	0x0B
#define APDS9130_PERS_REG	0x0C
#define APDS9130_CONFIG_REG	0x0D
#define APDS9130_PPCOUNT_REG	0x0E
#define APDS9130_CONTROL_REG	0x0F
#define APDS9130_REV_REG	0x11
#define APDS9130_ID_REG		0x12
#define APDS9130_STATUS_REG	0x13
#define APDS9130_PDATAL_REG	0x18
#define APDS9130_PDATAH_REG	0x19

#define CMD_BYTE		0x80
#define CMD_WORD		0xA0
#define CMD_SPECIAL		0xE0

#define CMD_CLR_PS_INT		0xE5
#define CMD_CLR_ALS_INT		0xE6
#define CMD_CLR_PS_ALS_INT	0xE7

/* Register Value define : PERS */
#define APDS9130_PPERS_0	0x00  /* Every proximity ADC cycle */
#define APDS9130_PPERS_1	0x10  /* 1 consecutive proximity value out of range */
#define APDS9130_PPERS_2	0x20  /* 2 consecutive proximity value out of range */
#define APDS9130_PPERS_3	0x30  /* 3 consecutive proximity value out of range */
#define APDS9130_PPERS_4	0x40  /* 4 consecutive proximity value out of range */
#define APDS9130_PPERS_5	0x50  /* 5 consecutive proximity value out of range */
#define APDS9130_PPERS_6	0x60  /* 6 consecutive proximity value out of range */
#define APDS9130_PPERS_7	0x70  /* 7 consecutive proximity value out of range */
#define APDS9130_PPERS_8	0x80  /* 8 consecutive proximity value out of range */
#define APDS9130_PPERS_9	0x90  /* 9 consecutive proximity value out of range */
#define APDS9130_PPERS_10	0xA0  /* 10 consecutive proximity value out of range */
#define APDS9130_PPERS_11	0xB0  /* 11 consecutive proximity value out of range */
#define APDS9130_PPERS_12	0xC0  /* 12 consecutive proximity value out of range */
#define APDS9130_PPERS_13	0xD0  /* 13 consecutive proximity value out of range */
#define APDS9130_PPERS_14	0xE0  /* 14 consecutive proximity value out of range */
#define APDS9130_PPERS_15	0xF0  /* 15 consecutive proximity value out of range */

#define APDS9130_PRX_IR_DIOD	0x20  /* Proximity uses CH1 diode */

#define APDS9130_PGAIN_1X	0x00  /* PS GAIN 1X */
#define APDS9130_PGAIN_2X	0x04  /* PS GAIN 2X */
#define APDS9130_PGAIN_4X	0x08  /* PS GAIN 4X */
#define APDS9130_PGAIN_8X	0x0C  /* PS GAIN 8X */

#define APDS9130_PDRVIE_100MA	0x00  /* PS 100mA LED drive */
#define APDS9130_PDRVIE_50MA	0x40  /* PS 50mA LED drive */
#define APDS9130_PDRVIE_25MA	0x80  /* PS 25mA LED drive */
#define APDS9130_PDRVIE_12_5MA	0xC0  /* PS 12.5mA LED drive */

/*
 * Structs
 */

struct apds9130_data {
	struct i2c_client *client;
	struct mutex update_lock;
	struct delayed_work	dwork;		/* for PS interrupt */
	struct input_dev *input_dev_ps;
	struct wake_lock ps_wlock;

	unsigned int enable;
	unsigned int ptime;
	unsigned int wtime;
	unsigned int pilt;
	unsigned int piht;
	unsigned int pers;
	unsigned int config;
	unsigned int ppcount;
	unsigned int control;

	unsigned int pDrive;

	/* control flag from HAL */
	unsigned int enable_ps_sensor;

	/* PS parameters */
	unsigned int ps_threshold;
	unsigned int ps_hysteresis_threshold; 	/* always lower than ps_threshold */
	unsigned int ps_detection;				/* 0 = near-to-far; 1 = far-to-near */
	unsigned int ps_data;					/* to store PS data */
	unsigned int ps_sat;					/* to store PS saturation bit */
	unsigned int ps_poll_delay;				/* needed for PS polling */

	unsigned int sw_mode;

#if defined(APDS9130_PROXIMITY_CAL)
	int cross_talk;
	bool read_ps_cal_data;
	int ps_cal_result;  //[LGSI_SP4_BSP][kirankumar.vm@lge.com] Proximity Testmode changes
#endif
};

/*
 * Global data
 */
static struct i2c_client *apds9130_i2c_client; /* global i2c_client to support ioctl */
static struct workqueue_struct *apds9130_workqueue;

enum apds9130_dev_status {
		PROX_STAT_SHUTDOWN = 0,
		PROX_STAT_OPERATING,
};

enum apds9130_input_event {
		PROX_INPUT_NEAR = 0,
		PROX_INPUT_FAR,
};

#if defined(APDS9130_PROXIMITY_CAL)
static int apds9130_read_crosstalk_data_fs(void);
static void apds9130_Set_PS_Threshold_Adding_Cross_talk(struct i2c_client *client, int cal_data);
#endif


/*
 * Management functions
 */

static int apds9130_set_pilt(struct i2c_client *client, int threshold)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS9130_PILTL_REG, threshold);
	mutex_unlock(&data->update_lock);

	data->pilt = threshold;

	return ret;
}

static int apds9130_set_piht(struct i2c_client *client, int threshold)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS9130_PIHTL_REG, threshold);
	mutex_unlock(&data->update_lock);

	data->piht = threshold;

	return ret;
}

static int apds9130_set_command(struct i2c_client *client, int command)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;
	int clearInt;

	if (command == 0)
		clearInt = CMD_CLR_PS_INT;
	else if (command == 1)
		clearInt = CMD_CLR_ALS_INT;
	else
		clearInt = CMD_CLR_PS_ALS_INT;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte(client, clearInt);
	mutex_unlock(&data->update_lock);

	return ret;
}

static int apds9130_set_enable(struct i2c_client *client, int enable)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9130_ENABLE_REG, enable);
	mutex_unlock(&data->update_lock);

	data->enable = enable;

	return ret;
}

#if defined(APDS9130_PROXIMITY_CAL)
void apds9130_swap(int *x, int *y)
{
     int temp = *x;
     *x = *y;
     *y = temp;
}

 static int apds9130_backup_crosstalk_data_fs(unsigned int val)
{
	int fd;
	int ret = 0;
	char buf[50];
	mm_segment_t old_fs = get_fs();

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", val);

	printk(KERN_INFO"%s Enter\n", __FUNCTION__ );
	printk(KERN_INFO"%s\n", buf);

	set_fs(KERNEL_DS);

/*[LGE_BSP_START][yunmo.yang@lge.com] 2012-11-08 change save path*/
#if defined(APDS9130_PROXIMITY_CAL)
    fd = sys_open("/mpt/prox_calibration.dat",O_WRONLY|O_CREAT, 0664);
#else
    fd = sys_open("/persist/prox_calibration.dat",O_WRONLY|O_CREAT, 0664);
#endif
/*[LGE_BSP_END][yunmo.yang@lge.com] 2012-11-08 change save path*/
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
static int apds9130_read_crosstalk_data_fs(void)
{
	int fd;
	int ret = 0;
	char read_buf[50];
	mm_segment_t old_fs = get_fs();

	printk(KERN_INFO"%s Enter\n", __FUNCTION__);
	memset(read_buf, 0, sizeof(read_buf));
	set_fs(KERNEL_DS);
/*[LGE_BSP_START][yunmo.yang@lge.com] 2012-11-08 change save path*/
#if defined(APDS9130_PROXIMITY_CAL) 
	fd = sys_open("/mpt/prox_calibration.dat",O_RDONLY, 0);
#else
	fd = sys_open("/persist/prox_calibration.dat",O_RDONLY, 0);
#endif
/*[LGE_BSP_END][yunmo.yang@lge.com] 2012-11-08 change save path*/
    if(fd >=0)
	{
		printk(KERN_INFO"%s Success read Prox Cross-talk from FS\n", __FUNCTION__);
		sys_read(fd, read_buf, sizeof(read_buf));
		sys_close(fd);
		set_fs(old_fs);
	}
	else
	{
		printk(KERN_INFO"%s Fail read Prox Cross-talk FS\n", __FUNCTION__);
		printk(KERN_INFO"%s Return error code : %d\n", __FUNCTION__, fd);
		ret = -1;
		sys_close(fd);
		set_fs(old_fs);
		return ret;
	}

	return (simple_strtol(read_buf, NULL, 10));

}

static void apds9130_Set_PS_Threshold_Adding_Cross_talk(struct i2c_client *client, int cal_data)
{
	struct apds9130_data *data = i2c_get_clientdata(client);

	if (cal_data>870)
		cal_data = 870;
	if (cal_data<0)
		cal_data = 0;

	data->cross_talk = cal_data;

#ifdef CONFIG_LEDS_LP5521
	data->ps_threshold = 100 + cal_data;
	data->ps_hysteresis_threshold = data->ps_threshold - 70;
#else
	data->ps_threshold = 150 + cal_data;
	data->ps_hysteresis_threshold = data->ps_threshold - 60;
#endif
}

static int apds9130_Run_Cross_talk_Calibration(struct i2c_client *client)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	unsigned int sum_of_pdata = 0,temp_pdata[20];
	unsigned int ret=0,i=0,j=0,ArySize = 20,cal_check_flag = 0;
	unsigned int old_enable = 0;

	printk(KERN_INFO"%s Enter \n", __FUNCTION__);

	old_enable = data->enable;

RE_CALIBRATION:

	sum_of_pdata = 0;
	apds9130_set_enable(client, 0x0D);

	msleep(50);

	for(i =0;i<20;i++)	{
		temp_pdata[i] = i2c_smbus_read_word_data(client, CMD_WORD|APDS9130_PDATAL_REG);
		mdelay(6);
	}

	for(i=0; i<ArySize-1; i++)
		for(j=i+1; j<ArySize; j++)
			if(temp_pdata[i] > temp_pdata[j])
				apds9130_swap(temp_pdata+i, temp_pdata+j);

	for (i = 5;i<15;i++)
		sum_of_pdata = sum_of_pdata + temp_pdata[i];

	data->cross_talk = sum_of_pdata/10;
	if (data->cross_talk>870)
	{
		if (cal_check_flag == 0)
		{
			cal_check_flag = 1;
			goto RE_CALIBRATION;
		}
		else
		{
			apds9130_set_enable(client,0x00);
			apds9130_set_enable(client,old_enable);
			return -1;
		}
	}

#ifdef CONFIG_LEDS_LP5521
	data->ps_threshold = 100 + data->cross_talk;
	data->ps_hysteresis_threshold = data->ps_threshold - 70;
#else
	data->ps_threshold = 150 + data->cross_talk;
	data->ps_hysteresis_threshold = data->ps_threshold - 60;
#endif
	ret = apds9130_backup_crosstalk_data_fs(data->cross_talk);

	printk(KERN_INFO"%s threshold : %d\n", __FUNCTION__, data->ps_threshold);
	printk(KERN_INFO"%s Hysteresis_threshold : %d\n",__FUNCTION__, data->ps_hysteresis_threshold);

	apds9130_set_enable(client,0x00);
	apds9130_set_enable(client,old_enable);
	printk(KERN_INFO"%s Leave\n", __FUNCTION__);
	return data->cross_talk;
}

 static ssize_t apds9130_show_run_calibration(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9130_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->ps_cal_result); //[LGSI_SP4_BSP][kirankumar.vm@lge.com] Proximity Testmode changes
}

static ssize_t apds9130_store_run_calibration(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9130_data *data = i2c_get_clientdata(client);

	int ret;

	ret = apds9130_Run_Cross_talk_Calibration(client);
	if(ret < 0)
	{
		printk(KERN_INFO"%s Fail error :  %d\n", __FUNCTION__, ret);
		data->ps_cal_result = 0;
	}
	else
	{
		printk(KERN_INFO"%s Succes cross-talk :  %d\n", __FUNCTION__, ret);
		data->ps_cal_result = 1;
	}

	return count;
}
static DEVICE_ATTR(run_calibration,  S_IWUSR | S_IRUGO|S_IWGRP |S_IRGRP |S_IROTH/*|S_IWOTH*/,
		   apds9130_show_run_calibration, apds9130_store_run_calibration);

 static ssize_t apds9130_show_crosstalk_data(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;

	ret = apds9130_read_crosstalk_data_fs();
	if(ret<0)
		return sprintf(buf, "Read fail\n");

	return sprintf(buf, "%d\n", ret);
}

static ssize_t apds9130_store_crosstalk_data(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret = 0;
	unsigned long val = simple_strtoul(buf, NULL, 10);


	printk(KERN_INFO"%s Enter\n", __FUNCTION__ );
	ret = apds9130_backup_crosstalk_data_fs(val);
	if(ret != 0)
		return printk(KERN_INFO"File open fail %d\n", ret);

	data->cross_talk = val;

	printk(KERN_INFO"Saved cross_talk val : %d\n", (int)val);


	return count;
}

static DEVICE_ATTR(prox_cal_data,  S_IWUSR | S_IRUGO|S_IWGRP |S_IRGRP |S_IROTH/*|S_IWOTH*/,
		   apds9130_show_crosstalk_data, apds9130_store_crosstalk_data);
#endif


static int apds9130_set_ptime(struct i2c_client *client, int ptime)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9130_PTIME_REG, ptime);
	mutex_unlock(&data->update_lock);

	data->ptime = ptime;

	return ret;
}

static int apds9130_set_wtime(struct i2c_client *client, int wtime)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9130_WTIME_REG, wtime);
	mutex_unlock(&data->update_lock);

	data->wtime = wtime;

	return ret;
}

static int apds9130_set_pers(struct i2c_client *client, int pers)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9130_PERS_REG, pers);
	mutex_unlock(&data->update_lock);

	data->pers = pers;

	return ret;
}

static int apds9130_set_config(struct i2c_client *client, int config)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9130_CONFIG_REG, config);
	mutex_unlock(&data->update_lock);

	data->config = config;

	return ret;
}

static int apds9130_set_ppcount(struct i2c_client *client, int ppcount)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9130_PPCOUNT_REG, ppcount);
	mutex_unlock(&data->update_lock);

	data->ppcount = ppcount;

	return ret;
}

static int apds9130_set_control(struct i2c_client *client, int control)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9130_CONTROL_REG, control);
	mutex_unlock(&data->update_lock);

	data->control = control;

	return ret;
}

static void apds9130_change_ps_threshold(struct i2c_client *client)
{
	struct apds9130_data *data = i2c_get_clientdata(client);

	data->ps_data =	i2c_smbus_read_word_data(client, CMD_WORD|APDS9130_PDATAL_REG);

	if ( (data->ps_data > data->pilt) && (data->ps_data >= data->piht) ) {
		/* far-to-near detected */
		data->ps_detection = 1;

		input_report_abs(data->input_dev_ps, ABS_DISTANCE, PROX_INPUT_NEAR);/* FAR-to-NEAR detection */
		input_sync(data->input_dev_ps);

		i2c_smbus_write_word_data(client, CMD_WORD|APDS9130_PILTL_REG, data->ps_hysteresis_threshold);
		i2c_smbus_write_word_data(client, CMD_WORD|APDS9130_PIHTL_REG, 1023);

		data->pilt = data->ps_hysteresis_threshold;
		data->piht = 1023;

		printk(KERN_INFO"far-to-near detected\n");
	}
	else if ( (data->ps_data <= data->pilt) && (data->ps_data < data->piht) ) {
		/* near-to-far detected */
		data->ps_detection = 0;

		input_report_abs(data->input_dev_ps, ABS_DISTANCE, PROX_INPUT_FAR);/* NEAR-to-FAR detection */
		input_sync(data->input_dev_ps);

		i2c_smbus_write_word_data(client, CMD_WORD|APDS9130_PILTL_REG, 0);
		i2c_smbus_write_word_data(client, CMD_WORD|APDS9130_PIHTL_REG, data->ps_threshold);

		data->pilt = 0;
		data->piht = data->ps_threshold;

		printk(KERN_INFO"near-to-far detected\n");
	}
}

static void apds9130_reschedule_work(struct apds9130_data *data,
					  unsigned long delay)
{
	/*
	 * If work is already scheduled then subsequent schedules will not
	 * change the scheduled time that's why we have to cancel it first.
	 */
	__cancel_delayed_work(&data->dwork);
	queue_delayed_work(apds9130_workqueue, &data->dwork, delay);

}

/* PS interrupt routine */
static void apds9130_work_handler(struct work_struct *work)
{
	struct apds9130_data *data = container_of(work, struct apds9130_data, dwork.work);
	struct i2c_client *client=data->client;
	int status;
	int enable;


	if(wake_lock_active(&data->ps_wlock))
		wake_unlock(&data->ps_wlock);
	wake_lock_timeout(&data->ps_wlock, 2 * HZ);


	status = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS9130_STATUS_REG);
	enable = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS9130_ENABLE_REG);

	i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9130_ENABLE_REG, 1);	/* disable 9130 first */

	printk(KERN_INFO"status = %x\n", status);

	if ((status & enable & 0x30) == 0x30) {
		/* both PS and ALS are interrupted - never happened*/

		if ( (status&0x40) != 0x40 ) // no PSAT bit set
			apds9130_change_ps_threshold(client);
		else {
			if (data->ps_detection == 1) {
				apds9130_change_ps_threshold(client);
			}
			else {
				printk(KERN_INFO"Triggered by background ambient noise\n");
			}
		}

		apds9130_set_command(client, 2);	/* 2 = CMD_CLR_PS_ALS_INT */
	}
	else if ((status & enable & 0x20) == 0x20) {
		/* only PS is interrupted */

		if ( (status&0x40) != 0x40 ) // no PSAT bit set
			apds9130_change_ps_threshold(client);	// far-to-near
		else {
			if (data->ps_detection == 1) {
				apds9130_change_ps_threshold(client); // near-to-far
			}
			else {
				printk(KERN_INFO"Triggered by background ambient noise\n");
			}
		}

		apds9130_set_command(client, 0);	/* 0 = CMD_CLR_PS_INT */
	}
	else if ((status & enable & 0x10) == 0x10) {
		/* only ALS is interrupted - will never happened*/

		apds9130_set_command(client, 1);	/* 1 = CMD_CLR_ALS_INT */
	}

	i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9130_ENABLE_REG, data->enable);
}

/* assume this is ISR */
static irqreturn_t apds9130_interrupt(int vec, void *info)
{
	struct i2c_client *client=(struct i2c_client *)info;
	struct apds9130_data *data = i2c_get_clientdata(client);

	printk(KERN_INFO"==> apds9130_interrupt\n");
	apds9130_reschedule_work(data, 0);

	return IRQ_HANDLED;
}

static int apds9130_enable_ps_sensor(struct i2c_client *client, int val)
{
	struct apds9130_data *data = i2c_get_clientdata(client);

	printk(KERN_INFO"enable ps sensor ( %d)\n", val);

	/* APDS9130_DISABLE_PS (0) = Disable PS */
	/* APDS9130_ENABLE_PS_WITH_INT (1) = Enable PS with interrupt enabled */

	if(val == APDS9130_ENABLE_PS_WITH_INT ) {

#if defined(APDS9130_PROXIMITY_CAL)
		data->cross_talk = apds9130_read_crosstalk_data_fs();

		if(data->cross_talk < 0 || data->cross_talk>870)
			data->cross_talk = PS_DEFAULT_CROSS_TALK;
		printk(KERN_INFO"%s Cross_talk : %d\n", __FUNCTION__, data->cross_talk);

		apds9130_Set_PS_Threshold_Adding_Cross_talk(client, data->cross_talk);

		printk(KERN_INFO"%s apds9130_Set_PS_Threshold_Adding_Cross_talk\n", __FUNCTION__);
		printk(KERN_INFO"%s apds9130_Set_PS_Threshold_Adding_Cross_talk = %d\n", __FUNCTION__,data->cross_talk);
#endif

		//turn on p sensor
		data->enable_ps_sensor = val;

		apds9130_set_enable(client,0); /* Power Off */

		apds9130_set_pilt(client, 0);		// init threshold for proximity
		apds9130_set_piht(client, data->ps_threshold); //[LGSI_SP4_BSP][kirankumar.vm@lge.com] add calibrated threshold

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com] Report the Far Detection evertytime when u enable the sensor 07-11-2012
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, PROX_INPUT_FAR);/* NEAR-to-FAR detection */
		input_sync(data->input_dev_ps);
//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com] Report the Far Detection evertytime when u enable the sensor

		apds9130_set_enable(client, 0x2D);	 /* enable PS interrupt */

	}
	else {
		apds9130_set_enable(client, 0);
		data->enable_ps_sensor = 0;
	}

	return 0;
}

/*
 * SysFS support
 */
static ssize_t apds9130_show_pdata(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9130_data *data = i2c_get_clientdata(client);
	int pdata;

	mutex_lock(&data->update_lock);
	pdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS9130_PDATAL_REG);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", pdata);
}

static DEVICE_ATTR(pdata, S_IRUGO,
		   apds9130_show_pdata, NULL);static ssize_t apds9130_show_enable(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9130_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->enable_ps_sensor);
}

static ssize_t apds9130_store_enable(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long val = simple_strtoul(buf, NULL, 10);

	printk(KERN_INFO"%s: enable ps senosr ( %ld)\n", __func__, val);

	if (val)
		apds9130_enable_ps_sensor(client, APDS9130_ENABLE_PS_WITH_INT);
	else
		apds9130_enable_ps_sensor(client, APDS9130_DISABLE_PS);

	return count;
}

static DEVICE_ATTR(enable, S_IWUGO | S_IRUGO,
		apds9130_show_enable, apds9130_store_enable);

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com] 31-10-2012 Added sys Fs entry for PPcount
static ssize_t apds9130_show_ppcount(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9130_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n",data->ppcount);
}

static ssize_t apds9130_store_ppcount(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	int err;
	unsigned int val = simple_strtoul(buf, NULL, 10);

	err = apds9130_set_ppcount(client, val);
	if(err < 0){
	printk(KERN_INFO"[APDS9130] apds9130_set_ppcount failed to set ppcount %d\n",val);
	return err;
	}
	return count;
}
static DEVICE_ATTR(ppcount, S_IWUSR | S_IRUGO, apds9130_show_ppcount, apds9130_store_ppcount);
//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com] 31-10-2012 Added sys Fs entry for PPcount
#if defined(APDS9130_PROXIMITY_CAL)
static ssize_t apds9130_show_control(struct device *dev, struct device_attribute *attr, char *buf)
{
    int control = 0;
    struct i2c_client *client = to_i2c_client(dev);

    control = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS9130_CONTROL_REG);
    if (control < 0) {
    dev_err(&client->dev, "%s: i2c error %d in reading reg 0x%x\n",  __func__, control, CMD_BYTE|APDS9130_CONTROL_REG);
    return control;
    }

    return sprintf(buf, "%d\n", control);
}

static ssize_t apds9130_store_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret = 0;
    unsigned long val;
    struct i2c_client *client = to_i2c_client(dev);

    val = simple_strtoul(buf, NULL, 10);

    ret = apds9130_set_control(client, val);

    if (ret < 0)
        return ret;

    return count;
}
static DEVICE_ATTR(control,  S_IWUSR | S_IRUGO , apds9130_show_control, apds9130_store_control);
#endif

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com] Added Sys Fs access to show proximity status for Testmode
static ssize_t apds9130_show_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int show = PROX_INPUT_FAR;
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9130_data *data = i2c_get_clientdata(client);

	if(data->enable_ps_sensor) {
		if(data->ps_detection) {
			show = PROX_INPUT_NEAR;
		}
		else {
			show = PROX_INPUT_FAR;
		}
	}
	else
		show = PROX_INPUT_FAR;  //If not enabled show it to far by default

	return sprintf(buf, "%d\n", show);
}

static DEVICE_ATTR(show, S_IWUSR | S_IRUGO , apds9130_show_show, NULL);
//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]

static ssize_t apds9130_show_pdrive(struct device *dev,
				struct device_attribute *attr, char *buf)
{
		struct i2c_client *client = to_i2c_client(dev);
		struct apds9130_data *data = i2c_get_clientdata(client);

		if(client != NULL)
				return sprintf(buf, "%d\n",data->pDrive);

		else
				return -1;
}

static ssize_t apds9130_store_pdrive(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
		struct i2c_client *client = to_i2c_client(dev);
		struct apds9130_data *data = i2c_get_clientdata(client);
		unsigned int rdata;

		sscanf(buf, "%d", &rdata);

		if(client != NULL){
				data->pDrive= rdata;
				apds9130_set_control(client,(data->pDrive |APDS9130_PRX_IR_DIOD|APDS9130_PGAIN_2X));
		}
		else
				return -1;

		return count;
}

static DEVICE_ATTR(pdrive, S_IRUGO | S_IWUSR, apds9130_show_pdrive, apds9130_store_pdrive);	

static ssize_t apds9130_show_pilt(struct device *dev,
				struct device_attribute *attr, char *buf)
{
		struct i2c_client *client = to_i2c_client(dev);
		struct apds9130_data *data = i2c_get_clientdata(client);

		if(client != NULL)
				return sprintf(buf, "%d\n",data->pilt);
		else
				return -1;
}

static ssize_t apds9130_store_pilt(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
		struct i2c_client *client = to_i2c_client(dev);
		unsigned int rdata;

		sscanf(buf, "%d", &rdata);
#if 0
#ifdef APDS9190_TUNE		
		g_pilt = rdata;
#endif
#endif
		if(client != NULL)
			apds9130_set_pilt(client, rdata);
		else
				return -1;

		return count;
}

static DEVICE_ATTR(pilt, S_IRUGO | S_IWUSR, apds9130_show_pilt, apds9130_store_pilt);	


static ssize_t apds9130_show_piht(struct device *dev,
				struct device_attribute *attr, char *buf)
{
		struct i2c_client *client = to_i2c_client(dev);
		struct apds9130_data *data = i2c_get_clientdata(client);

		if(client != NULL)
				return sprintf(buf, "%d\n",data->piht);
		else
				return -1;
}

static ssize_t apds9130_store_piht(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
		struct i2c_client *client = to_i2c_client(dev);
		unsigned int rdata;

		sscanf(buf, "%d", &rdata);
#if 0
#ifdef APDS9190_TUNE		
		g_piht = rdata;
#endif
#endif
		if(client != NULL)
			apds9130_set_piht(client, rdata);
		else
				return -1;

		return count;
}

static DEVICE_ATTR(piht, S_IRUGO | S_IWUSR, apds9130_show_piht, apds9130_store_piht);	

static struct attribute *apds9130_attributes[] = {
	&dev_attr_pdata.attr,
	&dev_attr_enable.attr,
#if defined(APDS9130_PROXIMITY_CAL)
	&dev_attr_control.attr,
	&dev_attr_run_calibration.attr,
	&dev_attr_prox_cal_data.attr,
#endif
	&dev_attr_show.attr, //[LGSI_SP4_BSP][kirankumar.vm@lge.com] Added Sys Fs access to show proximity status for Testmode
	&dev_attr_ppcount.attr, //[LGSI_SP4_BSP][kirankumar.vm@lge.com] Added ppcount sysfs Entry
	&dev_attr_pdrive.attr,/*[LGE_BSP][yunmo.yang@lge.com]add pDrive sysfs Entry*/
	&dev_attr_pilt.attr,
	&dev_attr_piht.attr,
	NULL
};

static const struct attribute_group apds9130_attr_group = {
	.attrs = apds9130_attributes,
};

/*
 * Initialization function
 */

static int apds9130_init_client(struct i2c_client *client)
{
	struct apds9130_data *data = i2c_get_clientdata(client);
	int err;
	int id;

	err = apds9130_set_enable(client, 0);
	if (err < 0)
		return err;

	data->pDrive = APDS9130_PDRVIE_100MA;

	id = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS9130_ID_REG);
	if (id == 0x39) {
		printk(KERN_INFO"APDS-9130\n");
	}
	else {
		printk(KERN_INFO"Not APDS-9130 %x\n", id);
		return -EIO;
	}

	err = apds9130_set_ptime(client, 0xFF);	// 2.72ms Prox integration time
	if (err < 0) return err;

	err = apds9130_set_wtime(client, 0xDC);	// 100ms Wait time for POLL_MEDIUM
	if (err < 0) return err;

	err = apds9130_set_ppcount(client, APDS9130_PS_PULSE_NUMBER);
	if (err < 0) return err;

	err = apds9130_set_config(client, 0);		// no long wait
	if (err < 0) return err;

	err = apds9130_set_control(client, APDS9130_PDRVIE_100MA|APDS9130_PRX_IR_DIOD|APDS9130_PGAIN_2X);	// 2012.10.10 PGAIN 4x-> 2x chkim.
	if (err < 0) return err;

	err = apds9130_set_pilt(client, 0);		// init threshold for proximity
	if (err < 0) return err;

	err = apds9130_set_piht(client, APDS9130_PS_DETECTION_THRESHOLD);
	if (err < 0) return err;

	err = apds9130_set_pers(client, APDS9130_PPERS_2);	// 2 consecutive Interrupt persistence
	if (err < 0) return err;

	// sensor is in disabled mode but all the configurations are preset
/* Temp block the below code as no need to set cross talk threshold during proximity OFF state [LGSI_SP4_BSP][kirankumar.vm@lge.com]
#if defined(APDS9130_PROXIMITY_CAL)
	err = apds9130_set_enable(client,0);
	if(err < 0){
		printk(KERN_INFO "%s, enable set Fail\n",__func__);
		return err;
	}
#endif
*/
	return 0;
}

/*
 * I2C init/probing/exit functions
 */

static struct i2c_driver apds9130_driver;
static int __devinit apds9130_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct apds9130_data *data;
	int err = 0;

	printk(KERN_INFO "%s %d\n ",__func__,__LINE__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		err = -EIO;
		printk("apds9130 probe failed due to  I2C_FUNC_SMBUS_BYTE. Ret = %d\n",err);
		goto exit;
	}

	data = kzalloc(sizeof(struct apds9130_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		printk("apds9130 probe failed due to no memory %d\n",err);
		goto exit;
	}

	data->client = client;
	apds9130_i2c_client = client;

	i2c_set_clientdata(client, data);
	wake_lock_init(&data->ps_wlock, WAKE_LOCK_SUSPEND, "proxi_wakelock");

	data->enable = 0;	/* default mode is standard */
	data->ps_threshold = APDS9130_PS_DETECTION_THRESHOLD;
	data->ps_hysteresis_threshold = APDS9130_PS_HSYTERESIS_THRESHOLD;
	data->ps_detection = 0;	/* default to no detection */
	data->enable_ps_sensor = 0;	// default to 0
#if defined(APDS9130_PROXIMITY_CAL)
	data->cross_talk=PS_DEFAULT_CROSS_TALK;
#endif

	mutex_init(&data->update_lock);

	/* Initialize the APDS-9130 chip */
	err = apds9130_init_client(client);
	if (err < 0){
		printk("apds9130 probe failed due to init client Ret = %d\n",err);
		goto exit_kfree;
	}

	INIT_DELAYED_WORK(&data->dwork, apds9130_work_handler);

	if (request_irq(client->irq, apds9130_interrupt, IRQF_DISABLED|IRQ_TYPE_EDGE_FALLING,
		APDS9130_DRV_NAME, (void *)client)) {
		printk(KERN_INFO"%s Could not allocate APDS9130_INT %d !\n", __func__,client->irq);

		goto exit_kfree;
	}

	enable_irq_wake(client->irq);


	printk(KERN_INFO"%s interrupt is hooked\n", __func__);

	data->input_dev_ps = input_allocate_device();
	if (!data->input_dev_ps) {
		err = -ENOMEM;
		printk(KERN_INFO"Failed to allocate input device ps\n");
		goto exit_free_irq;
	}

	set_bit(EV_ABS, data->input_dev_ps->evbit);

	input_set_abs_params(data->input_dev_ps, ABS_DISTANCE, 0, 1, 0, 0);

	data->input_dev_ps->name = "proximity";

	err = input_register_device(data->input_dev_ps);
	if (err) {
		err = -ENOMEM;
		printk(KERN_INFO"Unable to register input device ps: %s\n",
		       data->input_dev_ps->name);
		goto exit_free_dev_ps;
	}

	data->sw_mode = PROX_STAT_OPERATING;

	/* Register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &apds9130_attr_group);
	if (err)
		goto exit_unregister_dev_ps;

	printk(KERN_INFO"%s support ver. %s enabled\n", __func__, DRIVER_VERSION);

	return 0;


//	sysfs_remove_group(&client->dev.kobj, &apds9130_attr_group);         //Need to disable this kirankumar.vm@lge.com
exit_unregister_dev_ps:
	input_unregister_device(data->input_dev_ps);
exit_free_dev_ps:
	input_free_device(data->input_dev_ps);
exit_free_irq:
	free_irq(client->irq, client);
exit_kfree:
//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com] 30-11-2012 Destroy the mutex after its usage
	wake_lock_destroy(&data->ps_wlock);
	mutex_destroy(&data->update_lock);
//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]
	kfree(data);
exit:
	return err;
}

static int __devexit apds9130_remove(struct i2c_client *client)
{
	struct apds9130_data *data = i2c_get_clientdata(client);

	/* Power down the device */
	apds9130_set_enable(client, 0);

	sysfs_remove_group(&client->dev.kobj, &apds9130_attr_group);

	disable_irq_wake(client->irq);

	free_irq(client->irq, client);

	input_unregister_device(data->input_dev_ps);

	input_free_device(data->input_dev_ps);

	wake_lock_destroy(&data->ps_wlock);

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com] 30-11-2012 Destroy the mutex after its usage
	mutex_destroy(&data->update_lock);
//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]
	kfree(data);

	return 0;
}

#ifdef CONFIG_PM

static int apds9130_suspend(struct i2c_client *client, pm_message_t mesg)
{
#if 1
#else
	struct apds9130_data *data = i2c_get_clientdata(client);

	if(data->sw_mode == PROX_STAT_SHUTDOWN)
		return 0;

	apds9130_set_enable(client, 0);
	apds9130_set_command(client, 2);

	__cancel_delayed_work(&data->dwork);
	flush_delayed_work(&data->dwork);
	flush_workqueue(apds9130_workqueue);

	data->sw_mode = PROX_STAT_SHUTDOWN;
	disable_irq(client->irq);
/*	err = pdata->power(0);
	if(err < 0) {
		printk(KERN_INFO "%s, Proximity Power Off Fail in susped\n",__func__);
		return err;
	}
*/
	irq_set_irq_wake(client->irq, 0);
	if(NULL != apds9130_workqueue){
		destroy_workqueue(apds9130_workqueue);
		printk(KERN_INFO "%s, Destroy workqueue\n",__func__);
		apds9130_workqueue = NULL;
	}
#endif
	return 0;
}

static int apds9130_resume(struct i2c_client *client)
{
#if 1
#else
	struct apds9130_data *data = i2c_get_clientdata(client);
	int ret;
	int err = 0;

	if(apds9130_workqueue == NULL) {
		apds9130_workqueue = create_workqueue("proximity");
		if(NULL == apds9130_workqueue)
			return -ENOMEM;
	}

	printk(KERN_INFO"apds9190_resume \n");
	if(data->sw_mode == PROX_STAT_OPERATING)
		return 0;

	enable_irq(client->irq);

	mdelay(50);

	err = apds9130_set_enable(client, 0x2D);

	if(err < 0){
	printk(KERN_INFO "%s, enable set Fail\n",__func__);
	return -1;
	}

	data->sw_mode = PROX_STAT_OPERATING;

	ret = irq_set_irq_wake(client->irq, 1);
	if(ret)
		irq_set_irq_wake(client->irq, 0);

	apds9130_set_command(client, 0);
#endif
	return 0;
}

#else

#define apds9130_suspend	NULL
#define apds9130_resume		NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id apds9130_id[] = {
	{ "apds9130", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, apds9130_id);

static struct i2c_driver apds9130_driver = {
	.driver = {
		.name	= APDS9130_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.suspend = apds9130_suspend,
	.resume	= apds9130_resume,
	.probe	= apds9130_probe,
	.remove	= __devexit_p(apds9130_remove),
	.id_table = apds9130_id,
};

static int __init apds9130_init(void)
{
	apds9130_workqueue = create_workqueue("proximity");
	
	if (!apds9130_workqueue)
		return -ENOMEM;

	return i2c_add_driver(&apds9130_driver);
}

static void __exit apds9130_exit(void)
{
	if (apds9130_workqueue)
		destroy_workqueue(apds9130_workqueue);

	apds9130_workqueue = NULL;

	i2c_del_driver(&apds9130_driver);
}

MODULE_AUTHOR("Lee Kai Koon <kai-koon.lee@avagotech.com>");
MODULE_DESCRIPTION("APDS9130 proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(apds9130_init);
module_exit(apds9130_exit);

