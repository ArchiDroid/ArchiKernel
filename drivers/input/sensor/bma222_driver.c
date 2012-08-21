/*
 * Date: 2010/08/26 11:40:00
 * Revision: 1.1
 *
 *
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html
 * (C) Copyright 2010 Bosch Sensortec GmbH
 * All Rights Reserved
 *
 * BMA222_driver.c
 * This file contains all function implementations for the BMA222 in linux
 * Details.
 *
 * Modified by LG Electronics.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#ifdef BMA222_HAS_EARLYSUSPEND
#include<linux/earlysuspend.h>
#endif

#include <linux/poll.h>
#include "bma222.h"
#include "bma222_driver.h"

#include <mach/board_lge.h> /* platform data */
#include <linux/akm8975.h> /* akm daemon ioctl set define */
#define BMA222_BLOCK_IOCTL_CHECK 1

/* #define BMA222_DEBUG */
/* #define LGE_DEBUG */

#ifdef BMA222_MODULES
#include "bma222.c"
#endif

/* LGE_CHANGE_S [adwardk.kim@lge.com] 2011-03-25 */
#define BMA222_BUFSIZE 256
static atomic_t bma222_report_enabled = ATOMIC_INIT(0);
/* LGE_CHANGE_E [adwardk.kim@lge.com] 2011-03-25 */


#ifdef BMA222_HAS_EARLYSUSPEND
static void bma222_early_suspend(struct early_suspend *h);
static void bma222_late_resume(struct early_suspend *h);
#endif

/* LGE_CHANGE_S */
struct acceleration_platform_data *accel_pdata;
/*  bandwidth Possible Values :
 *               unsigned char BW
 *
 *               bw       Selected Bandwidth[Hz]       Tupdate [?s]
 *              =================================================
 *               0            7.81                       64000
 *               1            15.63                      32000
 *               2            31.25                      16000
 *               3            62.50                      8000
 *               4            125                        4000
 *               5            250                        2000
 *               6            500                        1000
 *               7            1000                       500
 *
 */
static u8 bandwidth = 2; /* 31.25 Hz */
/* LGE_CHANGE_E */

/* LGE_CHANGE,
 * offer acceleration access functions to share with ecompass sensor
 * 2011-07-29, jihyun.seong@lge.com
 */
extern int accsns_get_acceleration_data(int *xyz);
extern void accsns_activate(int flgatm, int flg);
/* LGE_CHANGE end */
/* LGE_CHANGE,
 * enable state check and run suspend/resume.
 * 2011-07-29, jihyun.seong@lge.com
 */
static void run_suspend_resume(int mode);
/* LGE_CHANGE end */
/* i2c operation for bma222 API */
static char bma222_i2c_write(unsigned char reg_addr, \
unsigned char *data, unsigned char len);
static char bma222_i2c_read(unsigned char reg_addr, \
unsigned char *data, unsigned char len);
static void bma222_i2c_delay(unsigned int msec);

/* globe variant */
static struct i2c_client *bma222_client = NULL;
struct bma222_data {
	bma222_t			bma222;
	int IRQ;
	struct fasync_struct *async_queue;
#ifdef BMA222_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

#ifdef BMA222_ENABLE_IRQ
static int bma222_interrupt_config(void);

static int bma222_interrupt_config()
{
#ifdef BMA222_DEBUG
	printk(KERN_INFO "%s\n", __func__);
#endif


	return 0;
}

static irqreturn_t bma222_irq_handler(int irq, void *_id)
{
	struct bma222_data *data;
    unsigned long flags;
	if (((bma222_t *)_id)->chip_id != 0x03) {
#ifdef BMA222_DEBUG
		printk(KERN_INFO "%s error\n", __func__);
#endif
		return IRQ_HANDLED;
	}
	if (bma222_client == NULL)
		return IRQ_HANDLED;
    printk(KERN_INFO "bma222 irq handler\n");
    data = i2c_get_clientdata(bma222_client);
    if (data == NULL)
		return IRQ_HANDLED;
	local_irq_save(flags);
	if (data->async_queue)
		kill_fasync(&data->async_queue, SIGIO, POLL_IN);
	local_irq_restore(flags);
	return IRQ_HANDLED;
}
#endif

/*	i2c delay routine for eeprom	*/
static inline void bma222_i2c_delay(unsigned int msec)
{
	mdelay(msec);
}

/*	i2c write routine for bma222	*/
static inline char bma222_i2c_write(unsigned char reg_addr, \
unsigned char *data, unsigned char len)
{
	s32 dummy;
#ifndef BMA222_SMBUS
	unsigned char buffer[2];
#endif
	if (bma222_client == NULL)	/* No global client pointer? */
		return -ENODEV;

	while (len--) {
#ifdef BMA222_SMBUS
		dummy = i2c_smbus_write_byte_data(bma222_client, \
										  reg_addr, *data);
#else
		buffer[0] = reg_addr;
		buffer[1] = *data;
		dummy = i2c_master_send(bma222_client, (char *)buffer, 2);
#endif
		reg_addr++;
		data++;
		if (dummy < 0)
			return -EIO;
	}
	return 0;
}

/*	i2c read routine for bma222	*/
static inline char bma222_i2c_read(unsigned char reg_addr, \
unsigned char *data, unsigned char len)
{
	s32 dummy;
	if (bma222_client == NULL)	/* No global client pointer? */
		return -ENODEV;

	while (len--) {
#ifdef BMA222_SMBUS
		dummy = i2c_smbus_read_byte_data(bma222_client, reg_addr);
		if (dummy < 0)
			return -EIO;
		*data = dummy & 0x000000ff;
#else
		dummy = i2c_master_send(bma222_client, (char *)&reg_addr, 1);
		if (dummy < 0)
			return -EIO;
		dummy = i2c_master_recv(bma222_client, (char *)data, 1);
		if (dummy < 0)
			return -EIO;
#endif
		reg_addr++;
		data++;
	}
	return 0;
}

/*	read command for BMA222 device file	*/
static ssize_t bma222_read(struct file *file, \
char __user *buf, size_t count, loff_t *offset)
{
	bma222acc_t acc;
	int ret;
	if (bma222_client == NULL) {
#ifdef BMA222_DEBUG
		printk(KERN_INFO "I2C driver not install\n");
#endif
		return -ENODEV;
	}

	bma222_read_accel_xyz(&acc);
#ifdef BMA222_DEBUG
	printk(KERN_INFO "BMA222: X/Y/Z axis: %-8d %-8d %-8d\n" ,
		(int)acc.x, (int)acc.y, (int)acc.z);
#endif

	if (count != sizeof(acc)) {
		return -EINVAL;
	}
	ret = copy_to_user(buf, &acc, sizeof(acc));
	if (ret != 0) {
#ifdef BMA222_DEBUG
	printk(KERN_INFO "BMA222: copy_to_user result: %d\n", ret);
#endif
	}
	return sizeof(acc);
}

/*	write command for BMA222 device file	*/
static ssize_t bma222_write(struct file *file, \
const char __user *buf, size_t count, loff_t *offset)
{
	if (bma222_client == NULL)
		return -ENODEV;
#ifdef BMA222_DEBUG
	printk(KERN_INFO \
"BMA222 should be accessed with ioctl command\n");
#endif
	return 0;
}

static unsigned int bma222_poll(struct file *file, poll_table *wait)
{
    unsigned int mask = 0;
    if (bma222_client == NULL) {
#ifdef BMA222_DEBUG
		printk(KERN_INFO "I2C driver not install\n");
#endif
		return -ENODEV;
	}
    mask |= POLLIN|POLLRDNORM|POLLOUT|POLLWRNORM;

#ifdef BMA222_DEBUG
	printk(KERN_INFO "%s\n", __func__);
#endif
    return mask;
}
extern int accel_power(unsigned char onoff);

/* LGE_CHANGE,
 * enable state check and run suspend/resume.
 *
 * 2011-07-29, jihyun.seong@lge.com
 */
static void run_suspend_resume(int mode)
{
	if (mode) {
		 /* if already mode normal, pass this routine.*/
		if (atomic_read(&bma222_report_enabled) == 0) {
			/* for debugging */
			printk(KERN_INFO"%s: accel_pdata %p\n", __func__, accel_pdata);
			printk(KERN_INFO"%s: accel_pdata->power %p\n", __func__, accel_pdata->power);
				
			/* turn on vreg power */
#if 0
			accel_pdata->power(1);
#else
			accel_power(1);
#endif
			mdelay(2);
			bma222_set_mode(bma222_MODE_NORMAL);
			bma222_set_bandwidth(bandwidth);/* bandwidth set */
			atomic_set(&bma222_report_enabled, 1);
#ifdef LGE_DEBUG
			printk(KERN_INFO "ACCEL_Power On\n");
#endif
		} else { /* already power on state */
			bma222_set_bandwidth(bandwidth);/* bandwidth set */
		}
	} else {
		bma222_set_mode(bma222_MODE_SUSPEND);
		atomic_set(&bma222_report_enabled, 0);
#ifdef LGE_DEBUG
		printk(KERN_INFO "ACCEL_Power Off\n");
#endif
		/* for debugging */
		printk(KERN_INFO"%s: accel_pdata %p\n", __func__, accel_pdata);
		printk(KERN_INFO"%s: accel_pdata->power %p\n", __func__, accel_pdata);

		/* turn off vreg power */
#if 0
		accel_pdata->power(0);
#else
		accel_power(0);
#endif
    }
	return;
}
/* LGE_CHANGE end */

/* LGE_CHANGE_S [adwardk.kim@lge.com] 2011-03-25 */
static ssize_t show_bma222_enable(struct device *dev, \
struct device_attribute *attr, char *buf)
{
    char strbuf[BMA222_BUFSIZE];
    snprintf(strbuf, PAGE_SIZE, "%d", atomic_read(&bma222_report_enabled));
    return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);
}

static ssize_t store_bma222_enable(struct device *dev,\
struct device_attribute *attr, const char *buf, size_t count)
{
    int mode = 0;

    sscanf(buf, "%d", &mode);
	/* actual routine */
	run_suspend_resume(mode);

    return 0;
}

static ssize_t show_bma222_sensordata(struct device *dev, \
struct device_attribute *attr, char *buf)
{
    char strbuf[BMA222_BUFSIZE];
    int x = 0, y = 0, z = 0;

    bma222acc_t acc;

    bma222_read_accel_xyz(&acc);

    memset(strbuf, 0x00, BMA222_BUFSIZE);

    /* snprintf(strbuf, PAGE_SIZE, "%d %d %d", (int)acc.x, (int)acc.y, (int)acc.z); */
    x = ((int)acc.x);
    y = ((int)acc.y);
    z = ((int)acc.z);
    snprintf(strbuf, PAGE_SIZE, "%-8d %-8d %-8d", x, y, z);
    return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);
}

/* LGE_CHANGE,
 * add bandwidth node.
 * 2011-07-19, jihyun.seong@lge.com
 */
static ssize_t show_bma222_bandwidth(struct device *dev, \
struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", bandwidth);
}

static ssize_t store_bma222_bandwidth(struct device *dev, \
struct device_attribute *attr, const char *buf, size_t count)
{
    int value = 0;

    sscanf(buf, "%d", &value);
	bandwidth = (u8)value;
	bma222_set_bandwidth(bandwidth);

    return 0;
}

static ssize_t show_bma222_sensordataX(struct device *dev, struct device_attribute *attr, char *buf)
{
    char strbuf[BMA222_BUFSIZE];
    int x = 0, y = 0, z = 0;

    bma222acc_t acc;

    bma222_read_accel_xyz(&acc);

    memset(strbuf, 0x00, BMA222_BUFSIZE);

    /* snprintf(strbuf, PAGE_SIZE, "%d %d %d", (int)acc.x, (int)acc.y, (int)acc.z); */
    x = ((int)acc.x);
    y = ((int)acc.y);
    z = ((int)acc.z);
    snprintf(strbuf, PAGE_SIZE, "%-8d %-8d %-8d", x, y, z);
    return snprintf(buf, PAGE_SIZE, "%d\n", x);
}

static ssize_t show_bma222_sensordataY(struct device *dev, \
struct device_attribute *attr, char *buf)
{
    char strbuf[BMA222_BUFSIZE];
    int x = 0, y = 0, z = 0;

    bma222acc_t acc;

    bma222_read_accel_xyz(&acc);

    memset(strbuf, 0x00, BMA222_BUFSIZE);

    /* snprintf(strbuf, PAGE_SIZE, "%d %d %d", (int)acc.x, (int)acc.y, (int)acc.z); */
    x = ((int)acc.x);
    y = ((int)acc.y);
    z = ((int)acc.z);
    snprintf(strbuf, PAGE_SIZE, "%-8d %-8d %-8d", x, y, z);
    return snprintf(buf, PAGE_SIZE, "%d\n", y);
}

static ssize_t show_bma222_sensordataZ(struct device *dev, \
struct device_attribute *attr, char *buf)
{
    char strbuf[BMA222_BUFSIZE];
    int x = 0, y = 0, z = 0;

    bma222acc_t acc;

    bma222_read_accel_xyz(&acc);

    memset(strbuf, 0x00, BMA222_BUFSIZE);

    /* snprintf(strbuf, PAGE_SIZE, "%d %d %d", (int)acc.x, (int)acc.y, (int)acc.z); */
    x = ((int)acc.x);
    y = ((int)acc.y);
    z = ((int)acc.z);
    snprintf(strbuf, PAGE_SIZE, "%-8d %-8d %-8d", x, y, z);
    return snprintf(buf, PAGE_SIZE, "%d\n", z);
}


static DEVICE_ATTR(bma222_enable, S_IRUGO | S_IWUSR | S_IWGRP, \
show_bma222_enable, store_bma222_enable);
static DEVICE_ATTR(bma222_sensordata, S_IRUGO, \
show_bma222_sensordata, NULL);
/* add bandwidth node */
static DEVICE_ATTR(bma222_bandwidth, S_IRUGO | S_IWUSR | S_IWGRP, \
show_bma222_bandwidth, store_bma222_bandwidth);

static DEVICE_ATTR(bma222_x, S_IRUGO, show_bma222_sensordataX, NULL);

static DEVICE_ATTR(bma222_y, S_IRUGO, show_bma222_sensordataY, NULL);

static DEVICE_ATTR(bma222_z, S_IRUGO, show_bma222_sensordataZ, NULL);

static struct attribute *bma222_attributes[] = {
    &dev_attr_bma222_enable.attr,
    &dev_attr_bma222_sensordata.attr,
/* add bandwidth node */
	&dev_attr_bma222_bandwidth.attr,
	&dev_attr_bma222_x.attr,
	&dev_attr_bma222_y.attr,
	&dev_attr_bma222_z.attr,
    NULL,
};

static struct attribute_group bma222_attribute_group = {
    .attrs = bma222_attributes
};
/* LGE_CHANGE_E [adwardk.kim@lge.com] 2011-03-25 */

/* LGE_CHANGE,
 * offer acceleration access functions to share with ecompass sensor
 * 2011-07-29, jihyun.seong@lge.com
 */
int accsns_get_acceleration_data(int *xyz)
{
	bma222acc_t acc;
	int err;

    err = bma222_read_accel_xyz(&acc);

	/* raw * 4 */
    xyz[0] = ((int)acc.x) * 4;
    xyz[1] = ((int)acc.y) * 4;
    xyz[2] = ((int)acc.z) * 4;

#ifdef LGE_DEBUG
	/*** DEBUG OUTPUT - REMOVE ***/
	printk("Acc_I2C, x:%d, y:%d, z:%d\n", xyz[0], xyz[1], xyz[2]);
	/*** <end> DEBUG OUTPUT - REMOVE ***/
#endif

	return err;
}

void accsns_activate(int flgatm, int flg)
{
	if (flg != 0)
		flg = 1;

	/* if (flg == 1) then sensor activate */
	run_suspend_resume(flg);
}

EXPORT_SYMBOL(accsns_get_acceleration_data);
EXPORT_SYMBOL(accsns_activate);
/* LGE_CHANGE end */
/*	open command for BMA222 device file	*/
static int bma222_open(struct inode *inode, struct file *file)
{
#ifdef BMA222_DEBUG
		printk(KERN_INFO "%s\n", __func__);
#endif

	if (bma222_client == NULL) {
#ifdef BMA222_DEBUG
		printk(KERN_INFO "I2C driver not install\n");
#endif
		return -ENODEV;
	}

#ifdef BMA222_DEBUG
	printk(KERN_INFO "BMA222 has been opened\n");
#endif
	return 0;
}

/*	release command for BMA222 device file	*/
static int bma222_close(struct inode *inode, struct file *file)
{
#ifdef BMA222_DEBUG
	printk(KERN_INFO "%s\n", __func__);
#endif
	return 0;
}


/*	ioctl command for BMA222 device file	*/
/* LGE_CHANGE_S [jihyun.seong@lge.com] 2011-05-24,
   replace unlocked ioctl - from kernel 2.6.36.x */
static long bma222_ioctl(struct file *file, \
unsigned int cmd, unsigned long arg)
{
	/* don't need to use inode */
	/* struct inode *inode = file->f_path.dentry->d_inode; */

	int err = 0;
	unsigned char data[6];
	struct bma222_data *pdata;
	pdata = i2c_get_clientdata(bma222_client);


#ifdef BMA222_DEBUG
	printk(KERN_INFO "%s\n", __func__);
#endif

#ifndef BMA222_BLOCK_IOCTL_CHECK
	/* check cmd */
	if (_IOC_TYPE(cmd) != BMA222_IOC_MAGIC) {
#ifdef BMA222_DEBUG
		printk(KERN_INFO "cmd magic type error\n");
#endif
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > BMA222_IOC_MAXNR) {
#ifdef BMA222_DEBUG
		printk(KERN_INFO "cmd number error\n");
#endif
		return -ENOTTY;
	}

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) {
#ifdef BMA222_DEBUG
		printk(KERN_INFO "cmd access_ok error\n");
#endif
		return -EFAULT;
	}
	/* check bam150_client */
	if (bma222_client == NULL) {
#ifdef BMA222_DEBUG
		printk(KERN_INFO "I2C driver not install\n");
#endif
		return -EFAULT;
	}
#endif

	/* cmd mapping */
	switch (cmd) {

	case BMA222_SOFT_RESET:
		err = bma222_soft_reset();
		return err;

	case BMA222_SET_RANGE:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_range(*data);
		return err;

	case BMA222_GET_RANGE:
		err = bma222_get_range(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_MODE:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_mode(*data);
		return err;

	case BMA222_GET_MODE:
		err = bma222_get_mode(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_BANDWIDTH:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_bandwidth(*data);
		return err;

	case BMA222_GET_BANDWIDTH:
		err = bma222_get_bandwidth(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_READ_REG:
		if (copy_from_user(data, (unsigned char *)arg, 3) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_read_reg(data[0], data+1, data[2]);
		if (copy_to_user((unsigned char *)arg, data, 3) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_WRITE_REG:
		if (copy_from_user(data, (unsigned char *)arg, 3) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_write_reg(data[0], data+1, data[2]);
		return err;

	case BMA222_RESET_INTERRUPT:
		err = bma222_reset_interrupt();
		return err;

	case BMA222_READ_ACCEL_X:
		err = bma222_read_accel_x((short *)data);
		if (copy_to_user((short *)arg, (short *)data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_READ_ACCEL_Y:
		err = bma222_read_accel_y((short *)data);
		if (copy_to_user((short *)arg, (short *)data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_READ_ACCEL_Z:
		err = bma222_read_accel_z((short *)data);
		if (copy_to_user((short *)arg, (short*)data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_INTERRUPTSTATUS1:
		err = bma222_get_interruptstatus1(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_INTERRUPTSTATUS2:
		err = bma222_get_interruptstatus2(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_READ_ACCEL_XYZ:
		err = bma222_read_accel_xyz((bma222acc_t *)data);
		if (copy_to_user((bma222acc_t *)arg, (bma222acc_t *)data, 6) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to error\n");
#endif
			return -EFAULT;
		}
		return err;

/* LGE_CHANGE,
 * add read ioctl for akmd2 daemon,
 * based on [hyesung.shin@lge.com] for <Sensor driver structure>
 *
 * 2011-06-21
*/
	case AKMD2_TO_ACCEL_IOCTL_READ_XYZ:
		err = bma222_read_accel_xyz((bma222acc_t *)data);

#if 0 /* origin */
		if (copy_to_user((bma222acc_t *)arg, (bma222acc_t *)data, 6) != 0) {
			printk(KERN_INFO "copy_to error\n");
			return -EFAULT;
		}
		return err;
#endif
		/* LGE_CHANGE,
		 * change bma222 readxyz data structure, when e-compass daemon access here.
		 * based on [adwardk.kim@lge.com]
		 *
		 * 2011-06-26
		 */
		if (copy_to_user((bma222acc_t *)arg, (bma222acc_t *)data, sizeof(int) * 3) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_LOW_G_INTERRUPT:
		err = bma222_get_Low_G_interrupt(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_HIGH_G_INTERRUPT:
		err = bma222_get_High_G_Interrupt(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_SLOPE_INTERRUPT:
		err = bma222_get_slope_interrupt(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_DOUBLE_TAP_INTERRUPT:
		err = bma222_get_double_tap_interrupt(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_SINGLE_TAP_INTERRUPT:
		err = bma222_get_single_tap_interrupt(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_ORIENT_INTERRUPT:
		err = bma222_get_orient_interrupt(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_FLAT_INTERRUPT:
		err = bma222_get_flat_interrupt(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_DATA_INTERRUPT:
		err = bma222_get_data_interrupt(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_SLOPE_FIRST:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_slope_first(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_SLOPE_SIGN:
		err = bma222_get_slope_sign(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_TAP_FIRST:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_tap_first(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_TAP_SIGN:
		err = bma222_get_tap_sign(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_HIGH_FIRST:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_HIGH_first(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_HIGH_SIGN:
		err = bma222_get_HIGH_sign(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_ORIENT_STATUS:
		err = bma222_get_orient_status(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_ORIENT_FLAT_STATUS:
		err = bma222_get_orient_flat_status(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_SLEEP_DURATION:
		err = bma222_get_sleep_duration(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_SLEEP_DURATION:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_sleep_duration(*data);
		return err;

	case BMA222_SET_SUSPEND:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_suspend(*data);
		return err;

	case BMA222_GET_SUSPEND:
		err = bma222_get_suspend(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_LOWPOWER:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_lowpower(*data);
		return err;

	case BMA222_GET_LOWPOWER_EN:
		err = bma222_get_lowpower_en(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_LOW_NOISE_CTRL:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_low_noise_ctrl(*data);
		return err;

	case BMA222_GET_LOW_NOISE_CTRL:
		err = bma222_get_low_noise_ctrl(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_SHADOW_DISABLE:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_shadow_disable(*data);
		return err;

	case BMA222_GET_SHADOW_DISABLE:
		err = bma222_get_shadow_disable(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_UNFILT_ACC:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_unfilt_acc(*data);
		return err;

	case BMA222_GET_UNFILT_ACC:
		err = bma222_get_unfilt_acc(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_ENABLE_TAP_INTERRUPT:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_enable_tap_interrupt(*data);
		return err;

	case BMA222_GET_ENABLE_TAP_INTERRUPT:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_enable_tap_interrupt(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_ENABLE_HIGH_G_INTERRUPT:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_enable_high_g_interrupt(*data);
		return err;

	case BMA222_GET_ENABLE_HIGH_G_INTERRUPT:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_enable_high_g_interrupt(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_ENABLE_SLOPE_INTERRUPT:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_enable_slope_interrupt(*data);
		return err;

	case BMA222_GET_ENABLE_SLOPE_INTERRUPT:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_enable_slope_interrupt(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_ENABLE_LOW_G_INTERRUPT:
		err = bma222_set_enable_low_g_interrupt();
		return err;

	case BMA222_GET_ENABLE_LOW_G_INTERRUPT:
		err = bma222_get_enable_low_g_interrupt(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_ENABLE_DATA_INTERRUPT:
		err = bma222_set_enable_data_interrupt();
		return err;

	case BMA222_GET_ENABLE_DATA_INTERRUPT:
		err = bma222_get_enable_data_interrupt(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_INT1_PAD_SEL:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_int1_pad_sel(*data);
		return err;

	case BMA222_GET_INT1_PAD_SEL:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_int1_pad_sel(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_INT_DATA_SEL:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_int_data_sel(*data);
		return err;

	case BMA222_GET_INT_DATA_SEL:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_int_data_sel(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_INT2_PAD_SEL:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_int2_pad_sel(*data);
		return err;

	case BMA222_GET_INT2_PAD_SEL:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_int2_pad_sel(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_INT_SRC:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_int_src(*data);
		return err;

	case BMA222_GET_INT_SRC:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_int_src(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_INT_SET:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_int_set(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_INT_SET:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_int_set(data[0], data[1]);
		return err;

	case BMA222_GET_MODE_CTRL:
		err = bma222_get_mode_ctrl(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_LOW_DURATION:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_low_g_duration(*data);
		return err;

	case BMA222_GET_LOW_DURATION:
		err = bma222_get_low_g_duration(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_LOW_G_THRESHOLD:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_low_g_threshold(*data);
		return err;

	case BMA222_GET_LOW_G_THRESHOLD:
		err = bma222_get_low_g_threshold(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_HIGH_G_DURATION:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_high_g_duration(*data);
		return err;

	case BMA222_GET_HIGH_G_DURATION:
		err = bma222_get_high_g_duration(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_HIGH_G_THRESHOLD:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_high_g_threshold(*data);
		return err;

	case BMA222_GET_HIGH_G_THRESHOLD:
		err = bma222_get_high_g_threshold(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_SLOPE_DURATION:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_slope_duration(*data);
		return err;

	case BMA222_GET_SLOPE_DURATION:
		err = bma222_get_slope_duration(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_SLOPE_THRESHOLD:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_slope_threshold(*data);
		return err;

	case BMA222_GET_SLOPE_THRESHOLD:
		err = bma222_get_slope_threshold(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_TAP_DURATION:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_tap_duration(*data);
		return err;

	case BMA222_GET_TAP_DURATION:
		err = bma222_get_tap_duration(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_TAP_SHOCK:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_tap_shock(*data);
		return err;

	case BMA222_GET_TAP_SHOCK:
		err = bma222_get_tap_shock(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_TAP_QUIET_DURATION:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_tap_quiet_duration(*data);
		return err;

	case BMA222_GET_TAP_QUIET:
		err = bma222_get_tap_quiet(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_TAP_THRESHOLD:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_tap_threshold(*data);
		return err;

	case BMA222_GET_TAP_THRESHOLD:
		err = bma222_get_tap_threshold(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_TAP_SAMP:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_tap_samp(*data);
		return err;

	case BMA222_GET_TAP_SAMP:
		err = bma222_get_tap_samp(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_ORIENT_MODE:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_orient_mode(*data);
		return err;

	case BMA222_GET_ORIENT_MODE:
		err = bma222_get_orient_mode(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_ORIENT_BLOCKING:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_orient_blocking(*data);
		return err;

	case BMA222_GET_ORIENT_BLOCKING:
		err = bma222_get_orient_blocking(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;
	case BMA222_SET_ORIENT_HYST:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_orient_hyst(*data);
		return err;

	case BMA222_GET_ORIENT_HYST:
		err = bma222_get_orient_hyst(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;
	case BMA222_SET_THETA_BLOCKING:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_theta_blocking(*data);
		return err;

	case BMA222_GET_THETA_BLOCKING:
		err = bma222_get_theta_blocking(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_ORIENT_EX:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_orient_ex(*data);
		return err;

	case BMA222_GET_ORIENT_EX:
		err = bma222_get_orient_ex(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_THETA_FLAT:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_theta_flat(*data);
		return err;

	case BMA222_GET_THETA_FLAT:
		err = bma222_get_theta_flat(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;
	case BMA222_SET_FLAT_HOLD_TIME:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_flat_hold_time(*data);
		return err;

	case BMA222_GET_FLAT_HOLD_TIME:
		err = bma222_get_flat_hold_time(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_GET_LOW_POWER_STATE:
		err = bma222_get_low_power_state(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_SELFTEST_ST:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_selftest_st(*data);
		return err;

	case BMA222_GET_SELFTEST_ST:
		err = bma222_get_selftest_st(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_SELFTEST_STN:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_selftest_stn(*data);
		return err;

	case BMA222_GET_SELFTEST_STN:
		err = bma222_get_selftest_stn(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_SELFTEST_ST_AMP:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_selftest_st_amp(*data);
		return err;

	case BMA222_GET_SELFTEST_ST_AMP:
		err = bma222_get_selftest_st_amp(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_EE_W:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_ee_w(*data);
		return err;

	case BMA222_GET_EE_W:
		err = bma222_get_ee_w(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_EE_PROG_TRIG:
		err = bma222_set_ee_prog_trig();
		return err;

	case BMA222_GET_EEPROM_WRITING_STATUS:
		err = bma222_get_eeprom_writing_status(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_UPDATE_IMAGE:
		err = bma222_set_update_image();
		return err;

	case BMA222_SET_I2C_WDT_TIMER:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_i2c_wdt_timer(*data);
		return err;

	case BMA222_GET_I2C_WDT_TIMER:
		err = bma222_get_i2c_wdt_timer(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

  /*  case BMA222_SET_UNLOCK_TRIMMING_PART:
		err = bma222_set_unlock_trimming_part();
		return err;
*/
	case BMA222_GET_HP_EN:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_get_hp_en(data[0], data+1);
		if (copy_to_user((unsigned char *)arg, data, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_HP_EN:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_hp_en(data[0], data[1]);
		return err;

	case BMA222_SET_CAL_TRIGGER:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_cal_trigger(*data);
		return err;

	case BMA222_GET_CAL_READY:
		err = bma222_get_cal_ready(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_OFFSET_RESET:
		err = bma222_set_offset_reset();
		return err;

	case BMA222_SET_OFFSET_CUTOFF:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_offset_cutoff(*data);
		return err;

	case BMA222_GET_OFFSET_CUTOFF:
		err = bma222_get_offset_cutoff(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_OFFSET_TARGET_X:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_offset_target_x(*data);
		return err;

	case BMA222_GET_OFFSET_TARGET_X:
		err = bma222_get_offset_target_x(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_OFFSET_TARGET_Y:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_offset_target_y(*data);
		return err;

	case BMA222_GET_OFFSET_TARGET_Y:
		err = bma222_get_offset_target_y(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_OFFSET_TARGET_Z:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_offset_target_z(*data);
		return err;

	case BMA222_GET_OFFSET_TARGET_Z:
		err = bma222_get_offset_target_z(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_OFFSET_FILT_X:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_offset_filt_x(*data);
		return err;

	case BMA222_GET_OFFSET_FILT_X:
		err = bma222_get_offset_filt_x(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_OFFSET_FILT_Y:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_offset_filt_y(*data);
		return err;

	case BMA222_GET_OFFSET_FILT_Y:
		err = bma222_get_offset_filt_y(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_OFFSET_FILT_Z:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_offset_filt_z(*data);
		return err;

	case BMA222_GET_OFFSET_FILT_Z:
		err = bma222_get_offset_filt_z(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_OFFSET_UNFILT_X:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_offset_unfilt_x(*data);
		return err;

	case BMA222_GET_OFFSET_UNFILT_X:
		err = bma222_get_offset_unfilt_x(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_OFFSET_UNFILT_Y:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_offset_unfilt_y(*data);
		return err;

	case BMA222_GET_OFFSET_UNFILT_Y:
		err = bma222_get_offset_unfilt_y(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_OFFSET_UNFILT_Z:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_offset_unfilt_z(*data);
		return err;

	case BMA222_GET_OFFSET_UNFILT_Z:
		err = bma222_get_offset_unfilt_z(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_INT_MODE:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_Int_Mode(*data);
		return err;

	case BMA222_GET_INT_MODE:
		err = bma222_get_Int_Mode(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case BMA222_SET_INT_ENABLE:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_Int_Enable(data[0], data[1]);
		return err;

	case BMA222_WRITE_EE:
		if (copy_from_user(data, (unsigned char *)arg, 2) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_write_ee(data[0], data[1]);
		return err;

	case BMA222_SET_LOW_HY:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_low_hy(*data);
		return err;

	case BMA222_SET_HIGH_HY:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_high_hy(*data);
		return err;

	case BMA222_SET_LOW_MODE:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = bma222_set_low_mode(*data);
		return err;

	case BMA222_GET_UPDATE_IMAGE_STATUS:
		err = bma222_get_update_image_status(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
#ifdef BMA222_DEBUG
			printk(KERN_INFO "copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	default:
		return 0;
	}
}
/* LGE_CHANGE_E */

static int bma222_fasync(int fd, struct file *file, int mode)
{
    struct bma222_data *data;
#ifdef BMA222_DEBUG
	printk(KERN_INFO "%s\n", __func__);
#endif
	data = i2c_get_clientdata(bma222_client);
	return fasync_helper(fd, file, mode, &data->async_queue);
	return 0;
}

static const struct file_operations bma222_fops = {
	.owner = THIS_MODULE,
	.read = bma222_read,
	.write = bma222_write,
    .poll = bma222_poll,
	.open = bma222_open,
	.release = bma222_close,
/* LGE_CHANGE_S [jihyun.seong@lge.com] 2011-05-24,
   replace unlocked ioctl - from kernel 2.6.36.x */
	.unlocked_ioctl = bma222_ioctl,
/* LGE_CHANGE_E */
	.fasync = bma222_fasync,
};

static struct miscdevice bma_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bma222",
	.fops = &bma222_fops,
};

/* LGE_CHANGE [jihyun.seong@lge.com] 2011-05-25,
   remove "int kind" argument */
static int bma222_detect(struct i2c_client *client,
			  struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
#ifdef BMA222_DEBUG
	printk(KERN_INFO "%s\n", __func__);
#endif
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;

	strlcpy(info->type, "bma222", I2C_NAME_SIZE);

	return 0;
}

static int bma222_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int err = 0;
	int tempvalue;
	struct bma222_data *data;

#ifdef BMA222_DEBUG
	printk(KERN_INFO "%s\n", __func__);
#endif

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_INFO "i2c_check_functionality error\n");
		goto exit;
	}
	data = kzalloc(sizeof(struct bma222_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
/* LGE_CHANGE_S */
	accel_pdata = client->dev.platform_data;

    accel_pdata->power(1);

	mdelay(2);

	atomic_set(&bma222_report_enabled, 1);
/* LGE_CHANGE_E */

	/* read chip id */
	tempvalue = 0;
#ifdef BMA222_SMBUS
	tempvalue = i2c_smbus_read_word_data(client, 0x00);
#else
	i2c_master_send(client, (char *)&tempvalue, 1);
	i2c_master_recv(client, (char *)&tempvalue, 1);
#endif
	if ((tempvalue&0x00FF) == 0x0003) {
		printk(KERN_INFO \
 "Bosch Sensortec Device detected!\n BMA222 registered I2C driver!\n");
		bma222_client = client;
	} else {
		printk(KERN_INFO \
"Bosch Sensortec Device not found, i2c error %d \n", tempvalue);
		bma222_client = NULL;
		err = -1;
		goto kfree_exit;
	}
	i2c_set_clientdata(bma222_client, data);

	err = misc_register(&bma_device);
	if (err) {
		printk(KERN_ERR "bma222 device register failed\n");
		goto kfree_exit;
	}

/* LGE_CHANGE_S [adwardk.kim@lge.com] 2011-03-25 */
    /* Register sysfs hooks */
    err = sysfs_create_group(&client->dev.kobj, &bma222_attribute_group);
    if (err) {
		printk(KERN_ERR "bma222 sysfs register failed\n");
		goto exit_sysfs_create_group_failed;
    }
/* LGE_CHANGE_E [adwardk.kim@lge.com] 2011-03-25 */

	printk(KERN_INFO "bma222 device create ok\n");

	/* bma222 sensor initial */
	data->bma222.bus_write = bma222_i2c_write;
	data->bma222.bus_read = bma222_i2c_read;
	data->bma222.delay_msec = bma222_i2c_delay;
	bma222_init(&data->bma222);

	/* LGE_CHANGE,
	 * To reduce shaking output data
	 * 2011-07-19, jihyun.seong@lge.com
	 */
	bma222_set_bandwidth(bandwidth);

#if 0 /* driver original */
	bma222_set_bandwidth(5);/* bandwidth 250Hz */
#endif

	bma222_set_range(0);/* range +/-2G */

	/* register interrupt */
#ifdef BMA222_ENABLE_IRQ

	err = bma222_interrupt_config();
	if (err < 0)
		goto exit_dereg;
    data->IRQ = client->irq;
/*	err = request_irq(data->IRQ, bma222_irq_handler, IRQF_TRIGGER_RISING, "bma222", &data->bma222);
	if (err) {
		printk(KERN_ERR "could not request irq\n");
		goto exit_dereg;
	}
*/ /* not support interrupt in this version */
#endif

#ifdef BMA222_HAS_EARLYSUSPEND
    data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
    data->early_suspend.suspend = bma222_early_suspend;
    data->early_suspend.resume = bma222_late_resume;
    register_early_suspend(&data->early_suspend);
#endif

	return 0;

#ifdef BMA222_ENABLE_IRQ
exit_dereg:
    misc_deregister(&bma_device);
#endif

/* LGE_CHANGE_S [adwardk.kim@lge.com] 2011-03-25 */
exit_sysfs_create_group_failed:
    sysfs_remove_group(&client->dev.kobj, &bma222_attribute_group);
/* LGE_CHANGE_E [adwardk.kim@lge.com] 2011-03-25 */

kfree_exit:
	kfree(data);
exit:
	return err;
}

#ifdef BMA222_HAS_EARLYSUSPEND
static void bma222_early_suspend(struct early_suspend *h)
{
#ifdef BMA222_DEBUG
    printk(KERN_INFO "%s\n", __func__);
#endif
    bma222_set_mode(bma222_MODE_SUSPEND);
}

static void bma222_late_resume(struct early_suspend *h)
{
#ifdef BMA222_DEBUG
    printk(KERN_INFO "%s\n", __func__);
#endif
    bma222_set_mode(bma222_MODE_NORMAL);
}
#endif
static int bma222_suspend(struct i2c_client *client, pm_message_t mesg)
{
#ifdef BMA222_DEBUG
    printk(KERN_INFO "%s\n", __func__);
#endif
    bma222_set_mode(bma222_MODE_SUSPEND);
    return 0;
}
static int bma222_resume(struct i2c_client *client)
{
#ifdef BMA222_DEBUG
    printk(KERN_INFO "%s\n", __func__);
#endif
    bma222_set_mode(bma222_MODE_NORMAL);
    return 0;
}

static int bma222_remove(struct i2c_client *client)
{
	struct bma222_data *data = i2c_get_clientdata(client);
/* LGE_CHANGE_S [adwardk.kim@lge.com] 2011-03-25 */
    sysfs_remove_group(&client->dev.kobj, &bma222_attribute_group);
/* LGE_CHANGE_E [adwardk.kim@lge.com] 2011-03-25 */

#ifdef BMA222_DEBUG
	printk(KERN_INFO "%s\n", __func__);
#endif
#ifdef BMA222_HAS_EARLYSUSPEND
    unregister_early_suspend(&data->early_suspend);
#endif
	misc_deregister(&bma_device);
#ifdef BMA222_ENABLE_IRQ
	free_irq(data->IRQ, &data->bma222);
#endif
	kfree(data);
	bma222_client = NULL;
	return 0;
}

static unsigned short normal_i2c[] = { I2C_CLIENT_END };

/* LGE_CHANGE [jihyun.seong@lge.com] 2011-05-25,
   not used */
#if 0
I2C_CLIENT_INSMOD_1(bma222);
#endif

static const struct i2c_device_id bma222_id[] = {
	{ "bma222", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, bma222_id);

static struct i2c_driver bma222_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "bma222",
	},
	.class		= I2C_CLASS_HWMON,
	.id_table	= bma222_id,
#if 0 /* FIXME: */
	.address_data	= &addr_data,
#endif
	.address_list = normal_i2c,
	.probe		= bma222_probe,
	.remove		= bma222_remove,
	.detect		= bma222_detect,
    .suspend    = bma222_suspend,
    .resume     = bma222_resume,
};

static int __init BMA222_init(void)
{
#ifdef BMA222_DEBUG
	printk(KERN_INFO "%s\n", __func__);
#endif
	return i2c_add_driver(&bma222_driver);
}

static void __exit BMA222_exit(void)
{
	i2c_del_driver(&bma222_driver);
	printk(KERN_ERR "BMA222 exit\n");
}

MODULE_DESCRIPTION("BMA222 driver");

module_init(BMA222_init);
module_exit(BMA222_exit);
