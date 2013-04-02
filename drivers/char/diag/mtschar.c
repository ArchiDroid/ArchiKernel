/***************************************************************
 * DESCRIPTION
 *
 * MTS char driver for LG MTS (Manufacturing Traceability System)
 * Copyright (c) 2012 by LGE Incorporated.  All Rights Reserved.
 *
 ***************************************************************
 * EDIT HISTORY
 *
 * when         who             what, where, why
 * --------     ------          ---------------------------
 * 12/17/12     SW PARK         Created.
 *
 ***************************************************************
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include "diagchar.h"
#include "mtschar.h"

struct mtschar_dev *mtschar;

static int mtschar_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int mtschar_close(struct inode *inode, struct file *filp)
{
	return 0;
}

static int mtschar_read(struct file *filp, char __user *buf, size_t count,
		 loff_t *f_pos)
{
	char usb_state[2];
	int ret = -1;

	memset(usb_state, '\0', sizeof(usb_state));
	switch (driver->usb_connected) {
	case 0:
		usb_state[0] = '0';
		break;
	case 1:
		usb_state[0] = '1';
		break;
	default:
		goto fail;
	}
	ret = copy_to_user(buf, usb_state, 1);
	if (ret != 0)
		printk(KERN_DEBUG "[%s] fail to copy_to_user\n", __func__);
	return 0;
fail:
	printk(KERN_DEBUG "[%s] unknown value: usb connection\n", __func__);
	return -1;
}

static int mtschar_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *f_pos)
{
	return 0;
}

static long mtschar_ioctl(struct file *filp, unsigned int cmd,
			  unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
	case MTS_IOCTL_CHCEK_USB_CONNECTION:
		if (driver->usb_connected != 1) {
			wait_event_interruptible(mtschar->waitq,
						 driver->usb_connected == 1);
		} else {
			wait_event_interruptible(mtschar->waitq,
						 driver->usb_connected == 0);
		}
		ret = copy_to_user((void *)arg,
				   (const void *)&driver->usb_connected,
				   sizeof(int));
		if (ret != 0) {
			printk(KERN_DEBUG "fail to copy_to_user: %s\n",
			       __func__);
		}
		break;
	default:
		break;
	}
	return 0;
}

static const struct file_operations mtschar_fops =
{
	.owner = THIS_MODULE,
	.read = mtschar_read,
	.write = mtschar_write,
	.unlocked_ioctl	= mtschar_ioctl,
	.open = mtschar_open,
	.release = mtschar_close,
};

static int mtschar_setup_cdev(dev_t devno)
{
	int err;

	cdev_init(mtschar->cdev, &mtschar_fops);

	mtschar->cdev->owner = THIS_MODULE;
	mtschar->cdev->ops = &mtschar_fops;

	err = cdev_add(mtschar->cdev, devno, 1);

	if (err) {
		printk(KERN_INFO "mts char dev registration failed\n");
		return -1;
	}

	mtschar->mtschar_class = class_create(THIS_MODULE, MTSCHAR);

	if (IS_ERR(mtschar->mtschar_class)) {
		printk(KERN_ERR "Error creating mts char dev class.\n");
		return -1;
	}

	device_create(mtschar->mtschar_class, NULL, devno,
		      (void *)mtschar, MTSCHAR);
	return 0;
}

static int mtschar_cleanup(void)
{
	if (mtschar) {
		if (mtschar->cdev) {
			device_destroy(mtschar->mtschar_class,
				       MKDEV(mtschar->major, mtschar->minor));
			cdev_del(mtschar->cdev);
		}
		if (!IS_ERR(mtschar->mtschar_class))
			class_destroy(mtschar->mtschar_class);
		kfree(mtschar);
	}
	return 0;
}

static int __init init_mtschar(void)
{
	dev_t dev;
	int error;

	mtschar = kzalloc(sizeof(struct mtschar_dev) + 8, GFP_KERNEL);

	if (mtschar) {
		mtschar->num = 1;
		mtschar->name = ((void *)mtschar) + sizeof(struct mtschar_dev);
		strlcpy(mtschar->name, MTSCHAR, 7);
		error = alloc_chrdev_region(&dev, mtschar->minor,
					    mtschar->num, mtschar->name);
		if (!error) {
			mtschar->major = MAJOR(dev);
			mtschar->minor = MINOR(dev);
		} else {
			printk(KERN_INFO "[%s] Major number not allocated\n",
			       __func__);
		}
		mtschar->cdev = cdev_alloc();
		error = mtschar_setup_cdev(dev);
		if (error)
			goto fail;
	} else {
		printk(KERN_INFO "[%s] kzalloc failed\n", __func__);
		goto fail;
	}
	init_waitqueue_head(&mtschar->waitq);
	return 0;
fail:
	mtschar_cleanup();
	return -1;
}

static void __exit exit_mtschar(void)
{
	mtschar_cleanup();
	printk(KERN_INFO "done mtschar exit\n");
}

module_init(init_mtschar);
module_exit(exit_mtschar);

MODULE_DESCRIPTION("LGE MTS CHAR DEVICE");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Se-Woong Park (lg-msp@lge.com)");

