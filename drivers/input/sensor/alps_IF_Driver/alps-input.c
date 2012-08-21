#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/input-polldev.h>

#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include "alps_io.h"
/* LGE_NOTICE
* The alps-input driver originally use functions which are offered
* from alps's hscd, accsns sensors driver.
* However, we don't need to use accsns's default functions.
* So we use our acceleration sensor's functions for independant driver porting.
*
* acceleration sensor driver has to offer following functions !
* => accsns_get_accelration_data(), accsns_activate()
* 2011-07-29, jihyun.seong@lge.com
*/
extern int accsns_get_acceleration_data(int *xyz);
extern int hscd_get_magnetic_field_data(int *xyz);
extern void hscd_activate(int flgatm, int flg, int dtime);
extern void accsns_activate(int flgatm, int flg);
extern int hscd_self_test_A(void);
extern int hscd_self_test_B(void);

static DEFINE_MUTEX(alps_lock);

static struct platform_device *pdev;
static struct input_polled_dev *alps_idev;

#define EVENT_TYPE_ACCEL_X          ABS_X
#define EVENT_TYPE_ACCEL_Y          ABS_Y
#define EVENT_TYPE_ACCEL_Z          ABS_Z
#define EVENT_TYPE_ACCEL_STATUS     ABS_WHEEL

#define EVENT_TYPE_YAW              ABS_RX
#define EVENT_TYPE_PITCH            ABS_RY
#define EVENT_TYPE_ROLL             ABS_RZ
#define EVENT_TYPE_ORIENT_STATUS    ABS_RUDDER

#define EVENT_TYPE_MAGV_X           ABS_HAT0X
#define EVENT_TYPE_MAGV_Y           ABS_HAT0Y
#define EVENT_TYPE_MAGV_Z           ABS_BRAKE

#define ALPS_POLL_INTERVAL		100	/* msecs */
#define ALPS_INPUT_FUZZ		0	/* input event threshold */
#define ALPS_INPUT_FLAT		0

#define POLL_STOP_TIME		400	/* (msec) */

static int flgM = 0, flgA = 0;
static int delay = 200;
static int poll_stop_cnt = 0;

static char calibration=0;
static char debug_enable=0;
/*****************************************************************************/
/* for I/O Control */

/* LGE_CHANGE_S [jihyun.seong@lge.com] 2011-05-24,
   replace unlocked ioctl - from kernel 2.6.36.x */
//static int alps_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg)
static long alps_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int ret = -1, tmpval;

	switch (cmd) {
	case ALPSIO_SET_MAGACTIVATE:
		ret = copy_from_user(&tmpval, argp, sizeof(tmpval));
		if (ret) {
			printk("error : alps_ioctl(cmd = ALPSIO_SET_MAGACTIVATE)\n");
			return -EFAULT;
		}
#ifdef ALPS_DEBUG
		printk("alps_ioctl(cmd = ALPSIO_SET_MAGACTIVATE), flgM = %d\n", tmpval);
#endif
		mutex_lock(&alps_lock);
		flgM = tmpval;
		hscd_activate(1, tmpval, delay);
		mutex_unlock(&alps_lock);
		break;

	case ALPSIO_SET_ACCACTIVATE:
		ret = copy_from_user(&tmpval, argp, sizeof(tmpval));
		if (ret) {
			printk("error : alps_ioctl(cmd = ALPSIO_SET_ACCACTIVATE)\n");
			return -EFAULT;
		}
#ifdef ALPS_DEBUG
		printk("alps_ioctl(cmd = ALPSIO_SET_ACCACTIVATE), flgA = %d\n", tmpval);
#endif
		mutex_lock(&alps_lock);
		flgA = tmpval;
		accsns_activate(1, flgA);
		mutex_unlock(&alps_lock);
		break;

	case ALPSIO_SET_DELAY:
		ret = copy_from_user(&tmpval, argp, sizeof(tmpval));
		if (ret) {
			printk("error : alps_ioctl(cmd = ALPSIO_SET_DELAY)\n");
			return -EFAULT;
		}
#ifdef ALPS_DEBUG
		printk("alps_ioctl(cmd = ALPSIO_SET_DELAY)\n");
#endif
		if (tmpval <= 10)
			tmpval = 10;

		else if (tmpval <= 20)
			tmpval = 20;

		else if (tmpval <= 60)
			tmpval = 50;

		else
			tmpval = 100;

		mutex_lock(&alps_lock);
		delay = tmpval;
		poll_stop_cnt = POLL_STOP_TIME / tmpval;
		hscd_activate(1, flgM, delay);
		mutex_unlock(&alps_lock);
#ifdef ALPS_DEBUG
		printk("     delay = %d\n", delay);
#endif
		break;

	case ALPSIO_ACT_SELF_TEST_A:
#ifdef ALPS_DEBUG
	   	printk("alps_ioctl(cmd = ALPSIO_ACT_SELF_TEST_A)\n");
#endif
        mutex_lock(&alps_lock);
        ret = hscd_self_test_A();
	    mutex_unlock(&alps_lock);
#ifdef ALPS_DEBUG
        printk("[HSCD] Self test-A result : %d\n", ret);
#endif
        if (copy_to_user(argp, &ret, sizeof(ret))) {
			printk( "error : alps_ioctl(cmd = ALPSIO_ACT_SELF_TEST_A)\n" );
			return -EFAULT;
        }
		break;

	case ALPSIO_ACT_SELF_TEST_B:
#ifdef ALPS_DEBUG
	   	printk("alps_ioctl(cmd = ALPSIO_ACT_SELF_TEST_B)\n");
#endif
        mutex_lock(&alps_lock);
        ret = hscd_self_test_B();
	    mutex_unlock(&alps_lock);
#ifdef ALPS_DEBUG
        printk("[HSCD] Self test-B result : %d\n", ret);
#endif
        if (copy_to_user(argp, &ret, sizeof(ret))) {
			printk( "error : alps_ioctl(cmd = ALPSIO_ACT_SELF_TEST_B)\n" );
			return -EFAULT;
        }
		break;

	default:
		return -ENOTTY;
	}
	return 0;
}

static int
alps_io_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int
alps_io_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations alps_fops = {
	.owner   = THIS_MODULE,
	.open    = alps_io_open,
	.release = alps_io_release,
/* LGE_CHANGE_S [jihyun.seong@lge.com] 2011-05-24,
   replace unlocked ioctl - from kernel 2.6.36.x */
	.unlocked_ioctl   = alps_ioctl,
/* LGE_CHANGE_E */
};

static struct miscdevice alps_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "alps_io",
	.fops  = &alps_fops,
};


/*****************************************************************/
/* for input device */

static ssize_t accsns_position_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	int x, y, z;
	int xyz[3];

	if (accsns_get_acceleration_data(xyz) == 0) {
		x = xyz[0];
		y = xyz[1];
		z = xyz[2];
	} else {
		x = 0;
		y = 0;
		z = 0;
	}
	return snprintf(buf, PAGE_SIZE, "accel (%d %d %d)  ", x, y, z);
}

static ssize_t hscd_position_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	int x, y, z;
	int xyz[3];

	if (hscd_get_magnetic_field_data(xyz) == 0) {
		x = xyz[0];
		y = xyz[1];
		z = xyz[2];
	} else {
		x = 0;
		y = 0;
		z = 0;
	}
	return snprintf(buf, PAGE_SIZE, "hscd (%d %d %d)\n", x, y, z);
}

static ssize_t alps_position_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	size_t cnt = 0;
	mutex_lock(&alps_lock);
	cnt += accsns_position_show(dev, attr, buf);
	cnt += hscd_position_show(dev, attr, buf);
	mutex_unlock(&alps_lock);
	return cnt;
}
/*LGE_CHANGE attribute add: x,y,z, cablibration, debug*/	
static ssize_t alps_x_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	int x, y, z;
	int xyz[3];

	if (hscd_get_magnetic_field_data(xyz) == 0) {
		x = xyz[0];
		y = xyz[1];
		z = xyz[2];
	} else {
		x = 0;
		y = 0;
		z = 0;
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", x);
}


static ssize_t alps_y_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	int x, y, z;
	int xyz[3];

	if (hscd_get_magnetic_field_data(xyz) == 0) {
		x = xyz[0];
		y = xyz[1];
		z = xyz[2];
	} else {
		x = 0;
		y = 0;
		z = 0;
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", y);
}


static ssize_t alps_z_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	int x, y, z;
	int xyz[3];

	if (hscd_get_magnetic_field_data(xyz) == 0) {
		x = xyz[0];
		y = xyz[1];
		z = xyz[2];
	} else {
		x = 0;
		y = 0;
		z = 0;
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", z);
}
static ssize_t alps_calibration_show(struct device *dev, \
struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%c\n", calibration);
}
static ssize_t alps_calibration_store(struct device *dev,\
struct device_attribute *attr, const char *buf, size_t count)
{

    sscanf(buf, "%c", &calibration);
    return 0;
}
static ssize_t alps_debug_show(struct device *dev, \
struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%c\n", debug_enable);
}
static ssize_t alps_debug_store(struct device *dev,\
struct device_attribute *attr, const char *buf, size_t count)
{

    sscanf(buf, "%c", &debug_enable);
    return 0;
}
static DEVICE_ATTR(position, 0444, alps_position_show, NULL);
static DEVICE_ATTR(x, 0444, alps_x_show, NULL);
static DEVICE_ATTR(y, 0444, alps_y_show, NULL);
static DEVICE_ATTR(z, 0444, alps_z_show, NULL);
static DEVICE_ATTR(calibration,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP, alps_calibration_show, alps_calibration_store);
static DEVICE_ATTR(debug, S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP, alps_debug_show, alps_debug_store);

static struct attribute *alps_attributes[] = {
	&dev_attr_position.attr,
	&dev_attr_x.attr,
	&dev_attr_y.attr,
	&dev_attr_z.attr,
	&dev_attr_calibration.attr,
	&dev_attr_debug.attr,
	NULL,
};

static struct attribute_group alps_attribute_group = {
	.attrs = alps_attributes,
};

static int alps_probe(struct platform_device *dev)
{
	printk(KERN_INFO "alps: alps_probe\n");
	return 0;
}

static int alps_remove(struct platform_device *dev)
{
	printk(KERN_INFO "alps: alps_remove\n");
	return 0;
}

static struct platform_driver alps_driver = {
	.driver	= {
		.name = "alps-input",
		.owner = THIS_MODULE,
	},
    .probe = alps_probe,
    .remove = alps_remove,
};

static void accsns_poll(struct input_dev *idev)
{
	int xyz[3];

	if (accsns_get_acceleration_data(xyz) == 0) {
		input_report_abs(idev, EVENT_TYPE_ACCEL_X, xyz[0]);
		input_report_abs(idev, EVENT_TYPE_ACCEL_Y, xyz[1]);
		input_report_abs(idev, EVENT_TYPE_ACCEL_Z, xyz[2]);
		idev->sync = 0;
		input_event(idev, EV_SYN, SYN_REPORT, 1);
	}
}

static void hscd_poll(struct input_dev *idev)
{
	int xyz[3];

	if (hscd_get_magnetic_field_data(xyz) == 0) {
		input_report_abs(idev, EVENT_TYPE_MAGV_X, xyz[0]);
		input_report_abs(idev, EVENT_TYPE_MAGV_Y, xyz[1]);
		input_report_abs(idev, EVENT_TYPE_MAGV_Z, xyz[2]);
		idev->sync = 0;
		input_event(idev, EV_SYN, SYN_REPORT, 2);
	}
}


static void alps_poll(struct input_polled_dev *dev)
{
	struct input_dev *idev = dev->input;

	mutex_lock(&alps_lock);
	dev->poll_interval = delay;
	if (poll_stop_cnt-- < 0) {
		poll_stop_cnt = -1;
		if (flgM)
			hscd_poll(idev);
		if (flgA)
			accsns_poll(idev);
	}
/*	else printk("pollinf stop. delay = %d, poll_stop_cnt = %d\n", delay, poll_stop_cnt); */
	mutex_unlock(&alps_lock);
}

static int __init alps_init(void)
{
	struct input_dev *idev;
	int ret;

	ret = platform_driver_register(&alps_driver);
	if (ret)
		goto out_region;
	printk(KERN_INFO "alps-init: platform_driver_register\n");

	pdev = platform_device_register_simple("alps", -1, NULL, 0);
	if (IS_ERR(pdev)) {
		ret = PTR_ERR(pdev);
		goto out_driver;
	}
	printk(KERN_INFO "alps-init: platform_device_register_simple\n");

	ret = sysfs_create_group(&pdev->dev.kobj, &alps_attribute_group);
	if (ret)
		goto out_device;
	printk(KERN_INFO "alps-init: sysfs_create_group\n");

	alps_idev = input_allocate_polled_device();
	if (!alps_idev) {
		ret = -ENOMEM;
		goto out_group;
	}
	printk(KERN_INFO "alps-init: input_allocate_polled_device\n");

	alps_idev->poll = alps_poll;
	alps_idev->poll_interval = ALPS_POLL_INTERVAL;

	/* initialize the input class */
	idev = alps_idev->input;
	idev->name = "alps";
	idev->phys = "alps/input0";
	idev->id.bustype = BUS_HOST;
	idev->dev.parent = &pdev->dev;
	idev->evbit[0] = BIT_MASK(EV_ABS);

	input_set_abs_params(idev, EVENT_TYPE_ACCEL_X,
			-4096, 4096, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
	input_set_abs_params(idev, EVENT_TYPE_ACCEL_Y,
			-4096, 4096, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
	input_set_abs_params(idev, EVENT_TYPE_ACCEL_Z,
			-4096, 4096, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);

	input_set_abs_params(idev, EVENT_TYPE_MAGV_X,
			-4096, 4096, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
	input_set_abs_params(idev, EVENT_TYPE_MAGV_Y,
			-4096, 4096, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
	input_set_abs_params(idev, EVENT_TYPE_MAGV_Z,
			-4096, 4096, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);

	ret = input_register_polled_device(alps_idev);
	if (ret)
		goto out_idev;
	printk(KERN_INFO "alps-init: input_register_polled_device\n");

	ret = misc_register(&alps_device);
	if (ret) {
		printk("alps-init: alps_io_device register failed\n");
		goto exit_misc_device_register_failed;
	}
	printk("alps-init: misc_register\n");

	return 0;

exit_misc_device_register_failed:
out_idev:
	input_free_polled_device(alps_idev);
	printk(KERN_INFO "alps-init: input_free_polled_device\n");
out_group:
	sysfs_remove_group(&pdev->dev.kobj, &alps_attribute_group);
	printk(KERN_INFO "alps-init: sysfs_remove_group\n");
out_device:
	platform_device_unregister(pdev);
	printk(KERN_INFO "alps-init: platform_device_unregister\n");
out_driver:
	platform_driver_unregister(&alps_driver);
	printk(KERN_INFO "alps-init: platform_driver_unregister\n");
out_region:
	return ret;
}

static void __exit alps_exit(void)
{
	misc_deregister(&alps_device);
	printk(KERN_INFO "alps-exit: misc_deregister\n");
	input_unregister_polled_device(alps_idev);
	printk(KERN_INFO "alps-exit: input_unregister_polled_device\n");
	input_free_polled_device(alps_idev);
	printk(KERN_INFO "alps-exit: input_free_polled_device\n");
	sysfs_remove_group(&pdev->dev.kobj, &alps_attribute_group);
	printk(KERN_INFO "alps-exit: sysfs_remove_group\n");
	platform_device_unregister(pdev);
	printk(KERN_INFO "alps-exit: platform_device_unregister\n");
	platform_driver_unregister(&alps_driver);
	printk(KERN_INFO "alps-exit: platform_driver_unregister\n");
}

module_init(alps_init);
module_exit(alps_exit);

MODULE_DESCRIPTION("Alps Input Device");
MODULE_AUTHOR("ALPS");
MODULE_LICENSE("GPL v2");
