/* drivers/i2c/chips/AMI306.c - AMI306 compass driver
 *
 * Copyright (C) 2009 AMIT Technology Inc.
 * Author: Kyle Chen <sw-support@amit-inc.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include "ami306.h"

#include <mach/board_lge.h>

#define AMI306_DRV_NAME		"ami306"
#define DRIVER_VERSION		"1.0.11.19"

#define AMI306_DEBUG_PRINT	1
#define AMI306_ERROR_PRINT	1

//#define AMI306_TEST		1

/* AMI306 Debug mask value
 * usage: echo [mask_value] > /sys/module/AMI306/parameters/debug_mask
 * All		: 127
 * No msg	: 0
 * default	: 2
 */
enum {
	AMI306_DEBUG_ERR_CHECK		= 1U << 0,
	AMI306_DEBUG_USER_ERROR		= 1U << 1,
	AMI306_DEBUG_FUNC_TRACE		= 1U << 2,
	AMI306_DEBUG_DEV_STATUS		= 1U << 3,
	AMI306_DEBUG_DEV_DEBOUNCE	= 1U << 4,
	AMI306_DEBUG_GEN_INFO		= 1U << 5,
	AMI306_DEBUG_INTR_INFO		= 1U << 6,
};

static unsigned int ami306_debug_mask = AMI306_DEBUG_USER_ERROR;

module_param_named(debug_mask, ami306_debug_mask, int,
		S_IRUGO | S_IWUSR | S_IWGRP);

#if defined(AMI306_DEBUG_PRINT)
#define AMID(fmt, args...)  printk(KERN_ERR "AMI306-DBG[%-18s:%5d]" fmt, __FUNCTION__, __LINE__, ## args)
#else
#define AMID(fmt, args...)
#endif


#if defined(AMI306_ERROR_PRINT)
#define AMIE(fmt, args...)  printk(KERN_ERR "AMI306-ERR[%-18s:%5d]" fmt, __FUNCTION__, __LINE__, ## args)
#else
#define AMIE(fmt, args...)
#endif

static struct i2c_client *ami306_i2c_client = NULL;

#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>

struct early_suspend ami306_sensor_early_suspend;

static void ami306_early_suspend(struct early_suspend *h);
static void ami306_late_resume(struct early_suspend *h);
static atomic_t ami306_report_enabled = ATOMIC_INIT(0);
#endif

#if defined(CONFIG_PM)
static int ami306_suspend(struct device *device);
static int ami306_resume(struct device *device);
#endif

/* Addresses to scan */
// static unsigned short normal_i2c[] = { AMI306_I2C_ADDRESS, I2C_CLIENT_END };

/* Insmod parameters */
// I2C_CLIENT_INSMOD;

struct _ami306_data {
	rwlock_t lock;
	int chipset;
	int mode;
	int rate;
	volatile int updated;
} ami306_data;

typedef struct {
	int x;
	int y;
	int z;
}ami306_vec_t;

typedef struct {
	unsigned long pedo_step;
	unsigned long pedo_time;
	int pedo_stat;
}ami306_pedo_t;

struct _ami306mid_data {
	rwlock_t datalock;
	rwlock_t ctrllock;
	int controldata[AMI306_CB_LENGTH];	
	int pedometerparam[AMI306_PD_LENGTH];
	int yaw;
	int roll;
	int pitch;
	ami306_vec_t nm;
	ami306_vec_t na;
	ami306_vec_t gyro;
	ami306_pedo_t pedo;	
	int status;
} ami306mid_data;

struct ami306_i2c_data {
	struct input_dev *input_dev;
	struct i2c_client *client;
};

static atomic_t dev_open_count;
static atomic_t hal_open_count;
static atomic_t daemon_open_count;

static u8 i2c_read_addr, i2c_read_len;

static int AMI306_I2c_Read(u8 regaddr, u8 *buf, u8 buf_len)
{
	int res = 0;

	res = i2c_master_send(ami306_i2c_client, &regaddr, 1);
	if (res <= 0) {
		printk(KERN_ERR "%s AMI306_I2c_Read error res = %d\n", __func__, res);
		return res;
	}
	res = i2c_master_recv(ami306_i2c_client, buf, buf_len);
	if (res <= 0) {
		printk(KERN_ERR "%s AMI306_I2c_Read error res = %d\n", __func__, res);
		return res;
	}
	
	return res;
}

static int AMI306_I2c_Write(u8 reg_adr, u8 *buf, u8 buf_len)
{
	int res = 0;
	u8 databuf[64];
	
	// LGE_CHANGE [dojip.kim@lge.com] 2010-10-05, check the buf_len
	if ( (buf_len+2) > 64)
		return -EINVAL;

	databuf[0] = reg_adr;
	memcpy(&databuf[1], buf, buf_len);
	databuf[buf_len+1] = 0x00;
	res = i2c_master_send(ami306_i2c_client, databuf, buf_len+1);	
	if (res <= 0)
		printk(KERN_ERR "%s AMI306_I2c_Write error res = %d\n", __func__, res);

	return res;
}

static int AMI306_Chipset_Init(int mode, int chipset)
{
	u8 databuf[10];
	u8 regaddr;
	u8 ctrl1, ctrl2, ctrl3;
	unsigned char ctrl4[2];
	
	regaddr = AMI306_REG_CTRL1;
	i2c_master_send(ami306_i2c_client, &regaddr, 1);
	i2c_master_recv(ami306_i2c_client, &ctrl1, 1);

	regaddr = AMI306_REG_CTRL2;
	i2c_master_send(ami306_i2c_client, &regaddr, 1);
	i2c_master_recv(ami306_i2c_client, &ctrl2, 1);
	
	regaddr = AMI306_REG_CTRL3;
	i2c_master_send(ami306_i2c_client, &regaddr, 1);
	i2c_master_recv(ami306_i2c_client, &ctrl3, 1);		

//	regaddr = AMI306_REG_CTRL4; //2 bytes
//	i2c_master_send(ami306_i2c_client, &regaddr, 1);
//	i2c_master_recv(ami306_i2c_client, &(ctrl4[0]), 2);
	
	databuf[0] = AMI306_REG_CTRL1;
	if( mode == AMI306_FORCE_MODE ) {
		databuf[1] = ctrl1 | AMI306_CTRL1_PC1 | AMI306_CTRL1_FS1_FORCE;
		write_lock(&ami306_data.lock);
		ami306_data.mode = AMI306_FORCE_MODE;
		write_unlock(&ami306_data.lock);
	}
	else {
		databuf[1] = ctrl1 | AMI306_CTRL1_PC1 | AMI306_CTRL1_FS1_NORMAL | AMI306_CTRL1_ODR1;
		write_lock(&ami306_data.lock);
		ami306_data.mode = AMI306_NORMAL_MODE;
		write_unlock(&ami306_data.lock);
	}
	i2c_master_send(ami306_i2c_client, databuf, 2);		
	
	databuf[0] = AMI306_REG_CTRL2;
	databuf[1] = ctrl2 | AMI306_CTRL2_DREN | AMI306_CTRL2_DRP;
	i2c_master_send(ami306_i2c_client, databuf, 2);		
	
	databuf[0] = AMI306_REG_CTRL3;
	databuf[1] = ctrl3 | AMI306_CTRL3_B0_LO_CLR;
	i2c_master_send(ami306_i2c_client, databuf, 2);
	
	databuf[0] = AMI306_REG_CTRL4;	
	if( chipset == AMI304_CHIPSET ) //AMI306
	{
//		ctrl4[1]   = ctrl4[1] & AMI304_CTRL4_COMPASS_MODE; 	 //0x5D
		ctrl4[0] = 0x00;
		ctrl4[1] = 0x00;
	}
	else	//AMI306	//AMI306_CHIPSET
	{
//		ctrl4[1]   = ctrl4[1] | AMI306_CTRL4_HIGHSPEED_MODE; //0x5D
		ctrl4[0] = 0x7e;
		ctrl4[1] = 0xa0;
	}	
	databuf[1] = ctrl4[0];
	databuf[2] = ctrl4[1];
	i2c_master_send(ami306_i2c_client, databuf, 3);				
	
	return 0;
}

static int AMI306_SetMode(int newmode)
{
	int mode = 0;
	int chipset = 0;

	read_lock(&ami306_data.lock);
	mode = ami306_data.mode;
	chipset = ami306_data.chipset;
	read_unlock(&ami306_data.lock);

	if (mode == newmode)
		return 0;

	return AMI306_Chipset_Init(newmode, chipset);
}

static int AMI306_ReadChipInfo(char *buf, int bufsize)
{
	if ((!buf)||(bufsize<=30))
		return -1;
		
	if (!ami306_i2c_client) {
		*buf = 0;
		return -2;
	}

	if (ami306_data.chipset == AMI306_CHIPSET)	{
		sprintf(buf, "AMI306 Chip");
	}
	else {
		sprintf(buf, "AMI304 Chip");
	}

	return 0;
}

static int AMI306_WIA(char *wia, int bufsize)
{
	char cmd;
	unsigned char databuf[10];

	if ((!wia)||(bufsize<=30))
		return -1;	
		
	if (!ami306_i2c_client) {
		*wia = 0;
		return -2;
	}

	cmd = AMI306_REG_WIA;
	i2c_master_send(ami306_i2c_client, &cmd, 1);	
	udelay(20);
	i2c_master_recv(ami306_i2c_client, &(databuf[0]), 1);	
	
	sprintf(wia, "%02x", databuf[0]);
	
	return 0;
}

static int Identify_AMI_Chipset(void)
{
	char strbuf[AMI306_BUFSIZE];
	int WIARet = 0;
	int ret;
	
	if( (ret=AMI306_WIA(strbuf, AMI306_BUFSIZE))!=0 )
		return ret;
		
	sscanf(strbuf, "%x", &WIARet);	
	
	if (WIARet == AMI306_WIA_VALUE)	{
		ami306_data.chipset = AMI306_CHIPSET;
	}
	else {
		ami306_data.chipset = AMI304_CHIPSET;
	}
	
	return 0;
}

static int AMI306_ReadSensorData(char *buf, int bufsize)
{
	char cmd;
	int mode = 0;
	unsigned char databuf[10] = {0,};
	int res = 0;
	int mx, my, mz;
	mx = my = mz = 0;

	if ((!buf)||(bufsize<=80))
		return -1;

	if (!ami306_i2c_client) {
		*buf = 0;
		return -2;
	}

	read_lock(&ami306_data.lock);
	mode = ami306_data.mode;
	read_unlock(&ami306_data.lock);

	databuf[0] = AMI306_REG_CTRL3;
	databuf[1] = AMI306_CTRL3_FORCE_BIT;
	res = i2c_master_send(ami306_i2c_client, databuf, 2);	
	if (res <= 0) 
		goto exit_AMI306_ReadSensorData;
	//udelay(700);
	msleep(1);
	// We can read all measured data in once
	cmd = AMI306_REG_DATAXH;
	res = i2c_master_send(ami306_i2c_client, &cmd, 1);	
	if (res <= 0) 
		goto exit_AMI306_ReadSensorData;
//	udelay(20);
	res = i2c_master_recv(ami306_i2c_client, &(databuf[0]), 6);
	if (res <= 0) 
		goto exit_AMI306_ReadSensorData;

	sprintf(buf, "%02x %02x %02x %02x %02x %02x", 
			databuf[0], databuf[1], databuf[2], 
			databuf[3], databuf[4], databuf[5]);

	//printk(KERN_INFO "[jaekyung83..lee]databuf : %s", buf);
	

	mx = (int)(databuf[0] | (databuf[1] << 8));
	my = (int)(databuf[2] | (databuf[3] << 8));
	mz = (int)(databuf[4] | (databuf[5] << 8));

	if (mx>32768)  mx = mx-65536;
	if (my>32768)  my = my-65536;
	if (mz>32768)  mz = mz-65536;

	//printk(KERN_INFO "[jaekyung83.lee]Magnetic Raw Data: X=%d, Y=%d, Z=%d\n", mx, my, mz);
	
	if (AMI306_DEBUG_DEV_STATUS & ami306_debug_mask) {
		int mx, my, mz;
		mx = my = mz = 0;

		mx = (int)(databuf[0] | (databuf[1] << 8));
		my = (int)(databuf[2] | (databuf[3] << 8));
		mz = (int)(databuf[4] | (databuf[5] << 8));

		if (mx>32768)  mx = mx-65536;
		if (my>32768)  my = my-65536;
		if (mz>32768)  mz = mz-65536;

		AMID("Magnetic Raw Data: X=%d, Y=%d, Z=%d\n", mx, my, mz);
	}

exit_AMI306_ReadSensorData:
	if (res <= 0) {
		if (printk_ratelimit()) {
			AMIE("I2C error: ret value=%d\n", res);
		}
		return -3;
	}
	return 0;
}

static int AMI306_ReadPostureData(char *buf, int bufsize)
{
	if ((!buf)||(bufsize<=80))
		return -1;

	read_lock(&ami306mid_data.datalock);
	sprintf(buf, "%d %d %d %d", 
			ami306mid_data.yaw, 
			ami306mid_data.pitch, 
			ami306mid_data.roll, 
			ami306mid_data.status);
	read_unlock(&ami306mid_data.datalock);
	return 0;
}

static int AMI306_ReadCaliData(char *buf, int bufsize)
{
	if ((!buf)||(bufsize<=80))
		return -1;

	read_lock(&ami306mid_data.datalock);
	sprintf(buf, "%d %d %d %d %d %d %d", 
			ami306mid_data.nm.x, 
			ami306mid_data.nm.y, 
			ami306mid_data.nm.z,
			ami306mid_data.na.x,
			ami306mid_data.na.y,
			ami306mid_data.na.z,
			ami306mid_data.status);
	read_unlock(&ami306mid_data.datalock);
	return 0;
}

static int AMI306_ReadGyroData(char *buf, int bufsize)
{
	if ((!buf)||(bufsize<=80))
		return -1;

	read_lock(&ami306mid_data.datalock);
	sprintf(buf, "%d %d %d", 
			ami306mid_data.gyro.x, 
			ami306mid_data.gyro.y,
			ami306mid_data.gyro.z);
	read_unlock(&ami306mid_data.datalock);
	return 0;
}

static int AMI306_ReadPedoData(char *buf, int bufsize)
{
	if ((!buf)||(bufsize<=80))
		return -1;

	read_lock(&ami306mid_data.datalock);
	sprintf(buf, "%ld %ld %d",
			ami306mid_data.pedo.pedo_step,
			ami306mid_data.pedo.pedo_time,
			ami306mid_data.pedo.pedo_stat);
	read_unlock(&ami306mid_data.datalock);
	return 0;		
}

static int AMI306_ReadMiddleControl(char *buf, int bufsize)
{
	if ((!buf)||(bufsize<=80))
		return -1;

	read_lock(&ami306mid_data.ctrllock);
	sprintf(buf, "%d %d %d %d %d %d %d %d %d %d",
		ami306mid_data.controldata[AMI306_CB_LOOPDELAY], ami306mid_data.controldata[AMI306_CB_RUN], ami306mid_data.controldata[AMI306_CB_ACCCALI], ami306mid_data.controldata[AMI306_CB_MAGCALI],
		ami306mid_data.controldata[AMI306_CB_ACTIVESENSORS], ami306mid_data.controldata[AMI306_CB_PD_RESET], ami306mid_data.controldata[AMI306_CB_PD_EN_PARAM], ami306mid_data.controldata[AMI306_CB_QWERTY],
		ami306mid_data.controldata[AMI306_CB_CHANGE_WINDOW], ami306mid_data.controldata[AMI306_CB_UNDEFINE_2] );
	read_unlock(&ami306mid_data.ctrllock);
	return 0;
}

static int AMI306_Report_Value(int iEnable)
{
	int controlbuf[AMI306_CB_LENGTH];
	struct ami306_i2c_data *data = i2c_get_clientdata(ami306_i2c_client);
	int report_enable = 0;

	if( !iEnable )
		return -1;

	read_lock(&ami306mid_data.ctrllock);
	memcpy(controlbuf, &ami306mid_data.controldata[0], sizeof(controlbuf));
	read_unlock(&ami306mid_data.ctrllock);			

	if(controlbuf[AMI306_CB_ACTIVESENSORS] & AMIT_BIT_ACCELEROMETER) {
		input_report_abs(data->input_dev, ABS_X, ami306mid_data.na.x);/* x-axis raw acceleration */
		input_report_abs(data->input_dev, ABS_Y, ami306mid_data.na.y);/* y-axis raw acceleration */
		input_report_abs(data->input_dev, ABS_Z, ami306mid_data.na.z);/* z-axis raw acceleration */
		report_enable = 1;
	}

	if(controlbuf[AMI306_CB_ACTIVESENSORS] & AMIT_BIT_MAGNETIC_FIELD) {
		input_report_abs(data->input_dev, ABS_HAT0X, ami306mid_data.nm.x); /* x-axis of raw magnetic vector */
		input_report_abs(data->input_dev, ABS_HAT0Y, ami306mid_data.nm.y); /* y-axis of raw magnetic vector */
		input_report_abs(data->input_dev, ABS_BRAKE, ami306mid_data.nm.z); /* z-axis of raw magnetic vector */
		input_report_abs(data->input_dev, ABS_WHEEL, ami306mid_data.status);/* status of magnetic sensor */
		report_enable = 1;
	}

	if(controlbuf[AMI306_CB_ACTIVESENSORS] & AMIT_BIT_ORIENTATION) {
		input_report_abs(data->input_dev, ABS_RX, ami306mid_data.yaw);	/* yaw */
		input_report_abs(data->input_dev, ABS_RY, ami306mid_data.pitch);/* pitch */
		input_report_abs(data->input_dev, ABS_RZ, ami306mid_data.roll);/* roll */
		input_report_abs(data->input_dev, ABS_RUDDER, ami306mid_data.status);/* status of orientation sensor */
		report_enable = 1;
	}

	// LGE_CHANGE [dojip.kim@lge.com] 2010-10-28, not supported
#if 0
	if(controlbuf[AMI306_CB_ACTIVESENSORS] & AMIT_BIT_GYROSCOPE) {
		input_report_abs(data->input_dev, ABS_HAT1X, ami306mid_data.gyro.x);/* x-axis of gyro sensor */
		input_report_abs(data->input_dev, ABS_HAT1Y, ami306mid_data.gyro.y);/* y-axis of gyro sensor */
		input_report_abs(data->input_dev, ABS_THROTTLE, ami306mid_data.gyro.z);/* z-axis of gyro sensor */
		report_enable = 1;
	}
#endif
		

	if (AMI306_DEBUG_DEV_DEBOUNCE & ami306_debug_mask) {
		AMID("yaw: %d, pitch: %d, roll: %d\n", ami306mid_data.yaw, ami306mid_data.pitch, ami306mid_data.roll);
		AMID("nax: %d, nay: %d, naz: %d\n", ami306mid_data.na.x, ami306mid_data.na.y, ami306mid_data.na.z);
		AMID("nmx: %d, nmy: %d, nmz: %d\n", ami306mid_data.nm.x, ami306mid_data.nm.y, ami306mid_data.nm.z);
		AMID("mag_status: %d\n", ami306mid_data.status);

	}

	if (report_enable)
		input_sync(data->input_dev);

	return 0;
}

static ssize_t show_chipinfo_value(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	char strbuf[AMI306_BUFSIZE];
	AMI306_ReadChipInfo(strbuf, AMI306_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);
}

static ssize_t show_sensordata_value(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	char strbuf[AMI306_BUFSIZE];
	AMI306_ReadSensorData(strbuf, AMI306_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);
}

static ssize_t show_posturedata_value(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	char strbuf[AMI306_BUFSIZE];
	AMI306_ReadPostureData(strbuf, AMI306_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);
}

static ssize_t show_calidata_value(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	char strbuf[AMI306_BUFSIZE];
	AMI306_ReadCaliData(strbuf, AMI306_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);
}

static ssize_t show_gyrodata_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	char strbuf[AMI306_BUFSIZE];
	AMI306_ReadGyroData(strbuf, AMI306_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);			
}

static ssize_t show_midcontrol_value(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	char strbuf[AMI306_BUFSIZE];
	AMI306_ReadMiddleControl(strbuf, AMI306_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);
}

static ssize_t store_midcontrol_value(struct device *dev, 
		struct device_attribute *attr, const char *buf, size_t count)
{
	write_lock(&ami306mid_data.ctrllock);
	memcpy(&ami306mid_data.controldata[0], buf, sizeof(int)*AMI306_CB_LENGTH);	
 	write_unlock(&ami306mid_data.ctrllock);
	return count;
}

static ssize_t show_mode_value(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	int mode=0;
	read_lock(&ami306_data.lock);
	mode = ami306_data.mode;
	read_unlock(&ami306_data.lock);
	return sprintf(buf, "%d\n", mode);
}

static ssize_t store_mode_value(struct device *dev, 
		struct device_attribute *attr, const char *buf, size_t count)
{
	int mode = 0;
	sscanf(buf, "%d", &mode);
 	AMI306_SetMode(mode);
	return count;
}

static ssize_t show_wia_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	char strbuf[AMI306_BUFSIZE];
	AMI306_WIA(strbuf, AMI306_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);			
}

/* Test mode attribute */
static ssize_t show_pitch_value(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", ami306mid_data.pitch);
}

static ssize_t show_roll_value(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", ami306mid_data.roll);
}

static DEVICE_ATTR(chipinfo, S_IRUGO, show_chipinfo_value, NULL);
static DEVICE_ATTR(sensordata, S_IRUGO, show_sensordata_value, NULL);
static DEVICE_ATTR(posturedata, S_IRUGO, show_posturedata_value, NULL);
static DEVICE_ATTR(calidata, S_IRUGO, show_calidata_value, NULL);
static DEVICE_ATTR(gyrodata, S_IRUGO, show_gyrodata_value, NULL);
static DEVICE_ATTR(midcontrol, S_IRUGO | S_IWUSR, show_midcontrol_value, store_midcontrol_value );
static DEVICE_ATTR(mode, S_IRUGO | S_IWUSR, show_mode_value, store_mode_value );
static DEVICE_ATTR(wia, S_IRUGO, show_wia_value, NULL);
static DEVICE_ATTR(pitch, S_IRUGO | S_IWUSR, show_pitch_value, NULL);
static DEVICE_ATTR(roll, S_IRUGO | S_IWUSR, show_roll_value, NULL);

static struct attribute *ami306_attributes[] = {
	&dev_attr_chipinfo.attr,
	&dev_attr_sensordata.attr,
	&dev_attr_posturedata.attr,
	&dev_attr_calidata.attr,
	&dev_attr_gyrodata.attr,
	&dev_attr_midcontrol.attr,
	&dev_attr_mode.attr,
	&dev_attr_wia.attr,
	/* Test mode attribute */
	&dev_attr_pitch.attr,
	&dev_attr_roll.attr,
	NULL,
};

static struct attribute_group ami306_attribute_group = {
	.attrs = ami306_attributes
};

static int ami306_open(struct inode *inode, struct file *file)
{
	int res = -1;
	if (atomic_cmpxchg(&dev_open_count, 0, 1) == 0) {
		if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
			AMID("Open device node:ami306\n");
		res = nonseekable_open(inode, file);
	}
	return res;
}

static int ami306_release(struct inode *inode, struct file *file)
{
	atomic_set(&dev_open_count, 0);
	if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
		AMID("Release device node:ami306\n");
	return 0;
}

static long ami306_ioctl(struct file *file, unsigned int cmd,unsigned long arg)
{
	char strbuf[AMI306_BUFSIZE];
	int controlbuf[AMI306_CB_LENGTH];
	int valuebuf[4];
	int calidata[7];
	int gyrodata[3];
	long pedodata[3];	
	int pedoparam[AMI306_PD_LENGTH];
	void __user *data;
	int retval=0;
	int mode=0,chipset=0;
	int iEnReport;

	switch (cmd) {
		case AMI306_IOCTL_INIT:
			read_lock(&ami306_data.lock);
			mode = ami306_data.mode;
			chipset = ami306_data.chipset;
			read_unlock(&ami306_data.lock);
			AMI306_Chipset_Init(mode, chipset);			
			break;

		case AMI306_IOCTL_READ_CHIPINFO:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			AMI306_ReadChipInfo(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		case AMI306_IOCTL_READ_SENSORDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			AMI306_ReadSensorData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		case AMI306_IOCTL_READ_POSTUREDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			AMI306_ReadPostureData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}
			break;
	 
	 	case AMI306_IOCTL_WRITE_POSTUREDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(&valuebuf, data, sizeof(valuebuf))) {
				retval = -EFAULT;
				goto err_out;
			}				
			write_lock(&ami306mid_data.datalock);
			ami306mid_data.yaw   = valuebuf[0];
			ami306mid_data.pitch = valuebuf[1];
			ami306mid_data.roll  = valuebuf[2];
			ami306mid_data.status = valuebuf[3];
			write_unlock(&ami306mid_data.datalock);		 	
	 		break;
	 	 
	        case AMI306_IOCTL_READ_CALIDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			AMI306_ReadCaliData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}
	        	break;
	        
		case AMI306_IOCTL_WRITE_CALIDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(&calidata, data, sizeof(calidata))) {
				retval = -EFAULT;
				goto err_out;
			}	
			write_lock(&ami306mid_data.datalock);			
			ami306mid_data.nm.x = calidata[0];
			ami306mid_data.nm.y = calidata[1];
			ami306mid_data.nm.z = calidata[2];
			ami306mid_data.na.x = calidata[3];
			ami306mid_data.na.y = calidata[4];
			ami306mid_data.na.z = calidata[5];
			ami306mid_data.status = calidata[6];
			write_unlock(&ami306mid_data.datalock);
			break;    

		case AMI306_IOCTL_READ_GYRODATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			AMI306_ReadGyroData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}				
			break;
			
		case AMI306_IOCTL_WRITE_GYRODATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(&gyrodata, data, sizeof(gyrodata))) {
				retval = -EFAULT;
				goto err_out;
			}	
			write_lock(&ami306mid_data.datalock);			
			ami306mid_data.gyro.x = gyrodata[0];
			ami306mid_data.gyro.y = gyrodata[1];
			ami306mid_data.gyro.z = gyrodata[2];
			write_unlock(&ami306mid_data.datalock);		
			break;
			
		case AMI306_IOCTL_READ_PEDODATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			AMI306_ReadPedoData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}				
			break;

		case AMI306_IOCTL_WRITE_PEDODATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(&pedodata, data, sizeof(pedodata))) {
				retval = -EFAULT;
				goto err_out;
			}	
			write_lock(&ami306mid_data.datalock);			
			ami306mid_data.pedo.pedo_step = pedodata[0];
			ami306mid_data.pedo.pedo_time = pedodata[1];
			ami306mid_data.pedo.pedo_stat = (int)pedodata[2];
			write_unlock(&ami306mid_data.datalock);  		
			break;

		case AMI306_IOCTL_READ_PEDOPARAM:
			read_lock(&ami306mid_data.ctrllock);
			memcpy(pedoparam, &ami306mid_data.pedometerparam[0], sizeof(pedoparam));
			read_unlock(&ami306mid_data.ctrllock);			
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_to_user(data, pedoparam, sizeof(pedoparam))) {
				retval = -EFAULT;
				goto err_out;
			}			
			break;
			
		case AMI306_IOCTL_WRITE_PEDOPARAM:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(pedoparam, data, sizeof(pedoparam))) {
				retval = -EFAULT;
				goto err_out;
			}	
			write_lock(&ami306mid_data.ctrllock);
			memcpy(&ami306mid_data.pedometerparam[0], pedoparam, sizeof(pedoparam));
			write_unlock(&ami306mid_data.ctrllock);
			break;	
	        
	        case AMI306_IOCTL_READ_CONTROL:
			read_lock(&ami306mid_data.ctrllock);
			memcpy(controlbuf, &ami306mid_data.controldata[0], sizeof(controlbuf));
			read_unlock(&ami306mid_data.ctrllock);
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if (copy_to_user(data, controlbuf, sizeof(controlbuf))) {
				retval = -EFAULT;
				goto err_out;
			}
	        	break;

		case AMI306_IOCTL_WRITE_CONTROL:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if (copy_from_user(controlbuf, data, sizeof(controlbuf))) {
				retval = -EFAULT;
				goto err_out;
			}
			write_lock(&ami306mid_data.ctrllock);
			memcpy(&ami306mid_data.controldata[0], controlbuf, sizeof(controlbuf));
			write_unlock(&ami306mid_data.ctrllock);
			break;

		case AMI306_IOCTL_WRITE_MODE:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if (copy_from_user(&mode, data, sizeof(mode))) {
				retval = -EFAULT;
				goto err_out;
			}
			AMI306_SetMode(mode);
			break;
					        				
		case AMI306_IOCTL_WRITE_REPORT:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(&iEnReport, data, sizeof(iEnReport))) {
				retval = -EFAULT;
				goto err_out;
			}				
			AMI306_Report_Value(iEnReport);		
			break;
		
		case AMI306_IOCTL_READ_WIA:
			data = (void __user *) arg;
			if (data == NULL)
				break;		
			AMI306_WIA(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}								
			break;
					        				
		default:
			if (AMI306_DEBUG_USER_ERROR & ami306_debug_mask)
				AMIE("not supported command= 0x%04x\n", cmd);
			retval = -ENOIOCTLCMD;
			break;
	}

err_out:
	return retval;
}

static int ami306daemon_open(struct inode *inode, struct file *file)
{
	int res = -1;

	if (atomic_cmpxchg(&daemon_open_count, 0, 1) == 0) {
		if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
			AMID("Open device node:ami306daemon\n");
		res = 0;
	}
	return res;
}

static int ami306daemon_release(struct inode *inode, struct file *file)
{
	atomic_set(&daemon_open_count, 0);
	if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
		AMID("Release device node:ami306daemon\n");
	return 0;
}

static long ami306daemon_ioctl(struct file *file, unsigned int cmd,
	   unsigned long arg)
{
	int valuebuf[4];
	int calidata[7];
	int gyrodata[3];
	long pedodata[3];
	int controlbuf[AMI306_CB_LENGTH];
	char strbuf[AMI306_BUFSIZE];
	int pedoparam[AMI306_PD_LENGTH];	
	char i2creaddata[3];
	void __user *data;
	int retval=0;
	int mode;
	int k = 0;
#if !defined(CONFIG_HAS_EARLYSUSPEND)
	int iEnReport;
#endif

	
	switch (cmd) {
		case AMI306DAE_IOCTL_GET_SENSORDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			AMI306_ReadSensorData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		case AMI306DAE_IOCTL_SET_POSTURE:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if (copy_from_user(&valuebuf, data, sizeof(valuebuf))) {
				retval = -EFAULT;
				goto err_out;
			}
			write_lock(&ami306mid_data.datalock);
			ami306mid_data.yaw   = valuebuf[0];
			ami306mid_data.pitch = valuebuf[1];
			ami306mid_data.roll  = valuebuf[2];
			ami306mid_data.status = valuebuf[3];
			write_unlock(&ami306mid_data.datalock);
			break;

		case AMI306DAE_IOCTL_SET_CALIDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if (copy_from_user(&calidata, data, sizeof(calidata))) {
				retval = -EFAULT;
				goto err_out;
			}
			write_lock(&ami306mid_data.datalock);
			ami306mid_data.nm.x = calidata[0];
			ami306mid_data.nm.y = calidata[1];
			ami306mid_data.nm.z = calidata[2];
			ami306mid_data.na.x = calidata[3];
			ami306mid_data.na.y = calidata[4];
			ami306mid_data.na.z = calidata[5];
			ami306mid_data.status = calidata[6];
			write_unlock(&ami306mid_data.datalock);
#if defined(CONFIG_HAS_EARLYSUSPEND)
			/*
			 * Disable input report at early suspend state
			 * On-Demand Governor set max cpu frequency when input event is appeared
			 */
			AMI306_Report_Value(atomic_read(&ami306_report_enabled));
#endif
			break;


		case AMI306DAE_IOCTL_SET_GYRODATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(&gyrodata, data, sizeof(gyrodata))) {
				retval = -EFAULT;
				goto err_out;
			}	
			write_lock(&ami306mid_data.datalock);			
			ami306mid_data.gyro.x = gyrodata[0];
			ami306mid_data.gyro.y = gyrodata[1];
			ami306mid_data.gyro.z = gyrodata[2];
			write_unlock(&ami306mid_data.datalock);
			break;
        
		case AMI306DAE_IOCTL_SET_PEDODATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(&pedodata, data, sizeof(pedodata))) {
				retval = -EFAULT;
				goto err_out;
			}	
			write_lock(&ami306mid_data.datalock);			
			ami306mid_data.pedo.pedo_step = pedodata[0];
			ami306mid_data.pedo.pedo_time = pedodata[1];
			ami306mid_data.pedo.pedo_stat = (int)pedodata[2];
			write_unlock(&ami306mid_data.datalock);				
			break;								

		case AMI306DAE_IOCTL_GET_PEDOPARAM:
			read_lock(&ami306mid_data.ctrllock);
			memcpy(pedoparam, &ami306mid_data.pedometerparam[0],
					sizeof(pedoparam));
			read_unlock(&ami306mid_data.ctrllock);			
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_to_user(data, pedoparam, sizeof(pedoparam))) {
				retval = -EFAULT;
				goto err_out;
			}					
			break;

		case AMI306DAE_IOCTL_SET_PEDOPARAM:
			data = (void __user *) arg;			
			if (data == NULL)
				break;	
			if (copy_from_user(pedoparam, data, sizeof(pedoparam))) {
				retval = -EFAULT;
				goto err_out;
			}	
			write_lock(&ami306mid_data.ctrllock);
			memcpy(&ami306mid_data.pedometerparam[0], pedoparam, sizeof(pedoparam));
			write_unlock(&ami306mid_data.ctrllock);					
			break;	

		case AMI306DAE_IOCTL_GET_CONTROL:
			read_lock(&ami306mid_data.ctrllock);
			memcpy(controlbuf, &ami306mid_data.controldata[0], sizeof(controlbuf));
			read_unlock(&ami306mid_data.ctrllock);
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if (copy_to_user(data, controlbuf, sizeof(controlbuf))) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		case AMI306DAE_IOCTL_SET_CONTROL:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if (copy_from_user(controlbuf, data, sizeof(controlbuf))) {
				retval = -EFAULT;
				goto err_out;
			}
			write_lock(&ami306mid_data.ctrllock);
			memcpy(&ami306mid_data.controldata[0], controlbuf, sizeof(controlbuf));
			write_unlock(&ami306mid_data.ctrllock);
			break;

		case AMI306DAE_IOCTL_SET_MODE:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if (copy_from_user(&mode, data, sizeof(mode))) {
				retval = -EFAULT;
				goto err_out;
			}
			AMI306_SetMode(mode);
			break;
								
		//Add for input_device sync			
		case AMI306DAE_IOCTL_SET_REPORT:
#if defined(CONFIG_HAS_EARLYSUSPEND)
			break;
#else
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(&iEnReport, data, sizeof(iEnReport))) {
				retval = -EFAULT;
				goto err_out;
			}				
			AMI306_Report_Value(iEnReport);
#endif
			break;
		
		case AMI306DAE_IOCTL_GET_WIA:
			data = (void __user *) arg;
			if (data == NULL)
				break;		
			AMI306_WIA(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		case AMI306DAE_IOCTL_SET_I2CDATA:
			data = (void __user *)arg;
			if (data == NULL)
				break;
			if (copy_from_user(strbuf, data, sizeof(strbuf))) {
				retval = -EFAULT;
				goto err_out;
			}
			AMI306_I2c_Write(strbuf[0], &strbuf[2], strbuf[1]);
			break;

		case AMI306DAE_IOCTL_SET_I2CADDR:
			data = (void __user *)arg;
			if (data == NULL){
						break;
			}
						
   			if ((k=copy_from_user(i2creaddata, data, 2))) {
				retval = -EFAULT;
				goto err_out;
			}
			i2c_read_addr = i2creaddata[0];
			i2c_read_len = i2creaddata[1];
			break;

		case AMI306DAE_IOCTL_GET_I2CDATA:
			AMI306_I2c_Read(i2c_read_addr, &strbuf[0], i2c_read_len);
			data = (void __user *)arg;
			if (data == NULL)
				break;
			if (copy_to_user(data, strbuf, i2c_read_len)) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		default:
			if (AMI306_DEBUG_USER_ERROR & ami306_debug_mask)
				AMIE("not supported command= 0x%04x\n", cmd);
			retval = -ENOIOCTLCMD;
			break;
	}

err_out:
	return retval;
}

static int ami306hal_open(struct inode *inode, struct file *file)
{
	atomic_inc(&hal_open_count);
	if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
		AMID("Open device node:ami306hal %d times.\n", atomic_read(&hal_open_count));
	return 0;
}

static int ami306hal_release(struct inode *inode, struct file *file)
{
	atomic_dec(&hal_open_count);
	if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
		AMID("Release ami306hal, remainder is %d times.\n", atomic_read(&hal_open_count));
	return 0;
}

static long ami306hal_ioctl(struct file *file, unsigned int cmd,unsigned long arg)
{
	int controlbuf[AMI306_CB_LENGTH];
	char strbuf[AMI306_BUFSIZE];
	int pedoparam[AMI306_PD_LENGTH];		
	void __user *data;
	int retval=0;
	switch (cmd) {
		case AMI306HAL_IOCTL_GET_SENSORDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			AMI306_ReadSensorData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		case AMI306HAL_IOCTL_GET_POSTURE:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			AMI306_ReadPostureData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		case AMI306HAL_IOCTL_GET_CALIDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			AMI306_ReadCaliData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}
	        	break;

		case AMI306HAL_IOCTL_GET_GYRODATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			AMI306_ReadGyroData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}				
			break;
			
		case AMI306HAL_IOCTL_GET_PEDODATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			AMI306_ReadPedoData(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}						
			break;

		case AMI306HAL_IOCTL_GET_PEDOPARAM:
			read_lock(&ami306mid_data.ctrllock);
			memcpy(pedoparam, &ami306mid_data.pedometerparam[0],
					sizeof(pedoparam));
			read_unlock(&ami306mid_data.ctrllock);			
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_to_user(data, pedoparam, sizeof(pedoparam))) {
				retval = -EFAULT;
				goto err_out;
			}			
			break;
			
		case AMI306HAL_IOCTL_SET_PEDOPARAM:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(pedoparam, data, sizeof(pedoparam))) {
				retval = -EFAULT;
				goto err_out;
			}	
			write_lock(&ami306mid_data.ctrllock);
			memcpy(&ami306mid_data.pedometerparam[0], pedoparam, sizeof(pedoparam));
			write_unlock(&ami306mid_data.ctrllock);
			break;	

		case AMI306HAL_IOCTL_GET_CONTROL:
			read_lock(&ami306mid_data.ctrllock);
			memcpy(controlbuf, &ami306mid_data.controldata[0], sizeof(controlbuf));
			read_unlock(&ami306mid_data.ctrllock);
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if (copy_to_user(data, controlbuf, sizeof(controlbuf))) {
				retval = -EFAULT;
				goto err_out;
			}
			break;


		case AMI306HAL_IOCTL_SET_CONTROL:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if (copy_from_user(controlbuf, data, sizeof(controlbuf))) {
				retval = -EFAULT;
				goto err_out;
			}
			write_lock(&ami306mid_data.ctrllock);
			memcpy(&ami306mid_data.controldata[0], controlbuf, sizeof(controlbuf));
			write_unlock(&ami306mid_data.ctrllock);
			break;	

		case AMI306HAL_IOCTL_GET_WIA:
			data = (void __user *) arg;
			if (data == NULL)
				break;		
			AMI306_WIA(strbuf, AMI306_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		default:
			if (AMI306_DEBUG_USER_ERROR & ami306_debug_mask)
				AMIE("not supported command= 0x%04x\n", cmd);
			retval = -ENOIOCTLCMD;
			break;
	}

err_out:
	return retval;
}

static struct file_operations ami306_fops = {
	.owner = THIS_MODULE,
	.open = ami306_open,
	.release = ami306_release,
	.unlocked_ioctl = ami306_ioctl,
};

static struct miscdevice ami306_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ami306",
	.fops = &ami306_fops,
};


static struct file_operations ami306daemon_fops = {
	.owner = THIS_MODULE,
	.open = ami306daemon_open,
	.release = ami306daemon_release,
	.unlocked_ioctl = ami306daemon_ioctl,
};

static struct miscdevice ami306daemon_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ami306daemon",
	.fops = &ami306daemon_fops,
};

static struct file_operations ami306hal_fops = {
	.owner = THIS_MODULE,
	.open = ami306hal_open,
	.release = ami306hal_release,
	.unlocked_ioctl = ami306hal_ioctl,
};

static struct miscdevice ami306hal_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ami306hal",
	.fops = &ami306hal_fops,
};

static int ami306_input_init(struct ami306_i2c_data *data)
{
	int err=0;
	data->input_dev = input_allocate_device();
	if (!data->input_dev) {
		err = -ENOMEM;
		AMIE("ami306_i2c_detect: Failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	set_bit(EV_ABS, data->input_dev->evbit);

	/* yaw */
	input_set_abs_params(data->input_dev, ABS_RX, 0, (360*10), 0, 0);
	/* pitch */
	input_set_abs_params(data->input_dev, ABS_RY, -(180*10), (180*10), 0, 0);
	/* roll */
	input_set_abs_params(data->input_dev, ABS_RZ, -(90*10), (90*10), 0, 0);
	/* status of orientation sensor */	
	input_set_abs_params(data->input_dev, ABS_RUDDER, 0, 5, 0, 0);
	
	/* x-axis of raw acceleration and the range is -2g to +2g */
	input_set_abs_params(data->input_dev, ABS_X, -(1000*2), (1000*2), 0, 0);
	/* y-axis of raw acceleration and the range is -2g to +2g */
	input_set_abs_params(data->input_dev, ABS_Y, -(1000*2), (1000*2), 0, 0);
	/* z-axis of raw acceleration and the range is -2g to +2g */
	input_set_abs_params(data->input_dev, ABS_Z, -(1000*2), (1000*2), 0, 0);
	
	/* x-axis of raw magnetic vector and the range is -3g to +3g */
	input_set_abs_params(data->input_dev, ABS_HAT0X, -(4000*3), (4000*3), 0, 0);
	/* y-axis of raw magnetic vector and the range is -3g to +3g */
	input_set_abs_params(data->input_dev, ABS_HAT0Y, -(4000*3), (4000*3), 0, 0);
	/* z-axis of raw magnetic vector and the range is -3g to +3g */
	input_set_abs_params(data->input_dev, ABS_BRAKE, -(4000*3), (4000*3), 0, 0);
	/* status of magnetic sensor */
	input_set_abs_params(data->input_dev, ABS_WHEEL, 0, 5, 0, 0);	

	/* x-axis of gyro sensor */
	input_set_abs_params(data->input_dev, ABS_HAT1X, -10000, 10000, 0, 0);
	/* y-axis of gyro sensor */
	input_set_abs_params(data->input_dev, ABS_HAT1Y, -10000, 10000, 0, 0);
	/* z-axis of gyro sensor */
	input_set_abs_params(data->input_dev, ABS_THROTTLE, -10000, 10000, 0, 0);

	data->input_dev->name = "Acompass";

	err = input_register_device(data->input_dev);
	if (err) {
		AMIE("ami306_i2c_detect: Unable to register input device: %s\n",
		       data->input_dev->name);
		goto exit_input_register_device_failed;
	}
	if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
	        AMID("register input device successfully!!!\n");
	return 0;

exit_input_register_device_failed:
	input_free_device(data->input_dev);	
exit_input_dev_alloc_failed:
	return err;	
}

static int __devinit ami306_probe(struct i2c_client *client, 
		const struct i2c_device_id * devid)
{
	int err = 0;
	struct ami306_i2c_data *data;
	struct ecom_platform_data* ecom_pdata;

	if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
		AMID("motion start....!\n");

	if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		AMIE("adapter can NOT support I2C_FUNC_I2C.\n");
		return -ENODEV;
	}

	if (!(data = kmalloc(sizeof(struct ami306_i2c_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}
	memset(data, 0, sizeof(struct ami306_i2c_data));

	data->client = client;
	i2c_set_clientdata(client, data);
	ami306_i2c_client = client;

	ecom_pdata = ami306_i2c_client->dev.platform_data;
	ecom_pdata->power(1);

#if defined(CONFIG_HAS_EARLYSUSPEND)
	ami306_sensor_early_suspend.suspend = ami306_early_suspend;
	ami306_sensor_early_suspend.resume = ami306_late_resume;
	register_early_suspend(&ami306_sensor_early_suspend);

	atomic_set(&ami306_report_enabled, 1);
#endif
	err=Identify_AMI_Chipset();
	if (err != 0) {  //get ami306_data.chipset
		printk(KERN_INFO "Failed to identify AMI_Chipset!\n");	
		goto exit_kfree;
	}

	AMI306_Chipset_Init(AMI306_FORCE_MODE, ami306_data.chipset); // default is Force State	
	dev_info(&client->dev, "%s operating mode\n", ami306_data.mode? "force" : "normal");

	printk(KERN_INFO "Register input device!\n");	
	err = ami306_input_init(data);
	if(err)
		goto exit_kfree;

	//register misc device:ami306	       
	err = misc_register(&ami306_device);
	if (err) {
		AMIE("ami306_device register failed\n");
		goto exit_misc_ami306_device_register_failed;
	}
	//register misc device:ami306daemon	
	err = misc_register(&ami306daemon_device);
	if (err) {
		AMIE("ami306daemon_device register failed\n");
		goto exit_misc_ami306daemon_device_register_failed;
	}
	//register misc device:ami306hal
	err = misc_register(&ami306hal_device);
	if (err) {
		AMIE("ami306hal_device register failed\n");
		goto exit_misc_ami306hal_device_register_failed;
	}

	/* Register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &ami306_attribute_group);
	if (err) {
		AMIE("ami306 sysfs register failed\n");
		goto exit_sysfs_create_group_failed;
	}
	printk(KERN_INFO "[jaekyung83.lee] ami306 probe");
	return 0;

exit_sysfs_create_group_failed:	
	sysfs_remove_group(&client->dev.kobj, &ami306_attribute_group);
exit_misc_ami306hal_device_register_failed:
	misc_deregister(&ami306daemon_device);
exit_misc_ami306daemon_device_register_failed:
	misc_deregister(&ami306_device);
exit_misc_ami306_device_register_failed:
	input_unregister_device(data->input_dev);
	input_free_device(data->input_dev);
exit_kfree:
	kfree(data);
exit:
	return err;
}

static int __devexit ami306_remove(struct i2c_client *client)
{
	struct ami306_i2c_data *data = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &ami306_attribute_group);

	misc_deregister(&ami306_device);
	misc_deregister(&ami306daemon_device);
	misc_deregister(&ami306hal_device);

	input_unregister_device(data->input_dev);
	input_free_device(data->input_dev);

	ami306_i2c_client = NULL;
	kfree(data);

#if defined(CONFIG_HAS_EARLYSUSPEND)
	unregister_early_suspend(&ami306_sensor_early_suspend);
#endif

	return 0;
}

#if defined(CONFIG_HAS_EARLYSUSPEND)
static void ami306_early_suspend(struct early_suspend *h)
{
	atomic_set(&ami306_report_enabled, 0);
}

static void ami306_late_resume(struct early_suspend *h)
{
	atomic_set(&ami306_report_enabled, 1);
}
#endif

#if defined(CONFIG_PM)
static int ami306_suspend(struct device *device)
{
	struct ecom_platform_data* ecom_pdata;

	if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
		AMID("AMI306 suspend....!\n");

	ecom_pdata = ami306_i2c_client->dev.platform_data;
	ecom_pdata->power(0);

	return 0;
}

static int ami306_resume(struct device *device)
{
	struct ecom_platform_data* ecom_pdata;
	ecom_pdata = ami306_i2c_client->dev.platform_data;

	if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
		AMID("AMI306 resume....!\n");

	ecom_pdata->power(1);
	AMI306_Chipset_Init(ami306_data.mode, ami306_data.chipset);

	return 0;
}
#endif

static const struct i2c_device_id ami306_ids[] = {
	{ "ami306_sensor", 0 },
	{ },
};

#if defined(CONFIG_PM)
static struct dev_pm_ops ami306_pm_ops = {
	.suspend = ami306_suspend,
	.resume = ami306_resume,
};
#endif

static struct i2c_driver ami306_i2c_driver = {
	.probe		= ami306_probe,
	.remove		= __devexit_p(ami306_remove),
	.id_table	= ami306_ids,
	.driver = {
		.owner = THIS_MODULE,
		.name	= "ami306_sensor",
#if defined(CONFIG_PM)
		.pm	= &ami306_pm_ops,
#endif
	},
};

static int __init ami306_init(void)
{
	int res;

	if (AMI306_DEBUG_FUNC_TRACE & ami306_debug_mask)
		AMID("AMI306 MI sensor driver: init\n");
	rwlock_init(&ami306mid_data.ctrllock);
	rwlock_init(&ami306mid_data.datalock);
	rwlock_init(&ami306_data.lock);
	memset(&ami306mid_data.controldata[0], 0, sizeof(int)*10);
	/* LGE_CHANGE [dojip.kim@lge.com] 2010-05-27, [LS670]
	 * 200ms is too slow to calibrate, so set 100ms
	 */
	/* LGE_CHANGE [dojip.kim@lge.com] 2010-08-11, [LS670]
	 * 20 ms by sprint request
	 */
	ami306mid_data.controldata[AMI306_CB_LOOPDELAY] = 20;  // Loop Delay
	ami306mid_data.controldata[AMI306_CB_RUN] = 1;         // Run	
	ami306mid_data.controldata[AMI306_CB_ACCCALI] = 0;     // Start-AccCali
	ami306mid_data.controldata[AMI306_CB_MAGCALI] = 1;     // Start-MagCali
	ami306mid_data.controldata[AMI306_CB_ACTIVESENSORS] = 0;   // Active Sensors
	ami306mid_data.controldata[AMI306_CB_PD_RESET] = 0;    // Pedometer not reset    
	ami306mid_data.controldata[AMI306_CB_PD_EN_PARAM] = 0; // Disable parameters of Pedometer
	ami306mid_data.controldata[AMI306_CB_QWERTY] =   0;   // Qwerty Keyboard : close -> 0, open -> 1.
	ami306mid_data.controldata[AMI306_CB_CHANGE_WINDOW] =   0;   //ADC_WINDOW_CONTROL: ADC_WINDOW_NORMAL->0 ADC_WINDOW_CHANGED->1 ADC_WINDOW_EXCEEDED->2
	memset(&ami306mid_data.pedometerparam[0], 0, sizeof(int)*AMI306_PD_LENGTH);	
	atomic_set(&dev_open_count, 0);
	atomic_set(&hal_open_count, 0);
	atomic_set(&daemon_open_count, 0);

	res = i2c_add_driver(&ami306_i2c_driver);

	printk(KERN_INFO "[jaekyung83.lee]ami init\n");
	if (res) {
		AMIE("failed to probe i2c \n");
		i2c_del_driver(&ami306_i2c_driver);
	}

	return res;
}

static void __exit ami306_exit(void)
{
	atomic_set(&dev_open_count, 0);
	atomic_set(&hal_open_count, 0);
	atomic_set(&daemon_open_count, 0);
	i2c_del_driver(&ami306_i2c_driver);
}

module_init(ami306_init);
module_exit(ami306_exit);

MODULE_AUTHOR("Kyle K.Y. Chen");
MODULE_DESCRIPTION("AMI306 MI-Sensor driver without DRDY");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
