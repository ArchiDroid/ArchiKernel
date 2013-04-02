#ifndef _MTSCHAR_H_
#define _MTSCHAR_H_

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

/* mts char device name */
#define MTSCHAR	"mtschar"

/* mts char ioctl values */
#define MTS_IOCTL_CHCEK_USB_CONNECTION	0

struct mtschar_dev {
	wait_queue_head_t waitq;
	struct file_operations *fops;
	struct cdev *cdev;
	struct class *mtschar_class;
	char *name;
	int major;
	int minor;
	int num;
};

extern struct mtschar_dev *mtschar;
#endif /* _MTSCHAR_H_ */

