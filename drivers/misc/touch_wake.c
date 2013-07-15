/* drivers/misc/touch_wake.c
 *
 * Copyright 2011  Ezekeel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * --------------------------------------------------------------------------------------
 *
 * Fixed issues with random misbehaving when powering off device via Powerkey
 *
 * Bumped version to 1.1
 *
 *                                         Jean-Pierre Rasquin <yank555.lu@gmail.com>
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/touch_wake.h>
#include <linux/workqueue.h>
#include <linux/earlysuspend.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/input.h>

extern void touchscreen_enable(void);
extern void touchscreen_disable(void);

static bool touchwake_enabled = false;
static bool touch_disabled = false;
static bool device_suspended = false;
static bool timed_out = true;
static bool prox_near = false;
static unsigned int touchoff_delay = 45000;

static void touchwake_touchoff(struct work_struct * touchoff_work);
static DECLARE_DELAYED_WORK(touchoff_work, touchwake_touchoff);
static void press_powerkey(struct work_struct * presspower_work);
static DECLARE_WORK(presspower_work, press_powerkey);
static DEFINE_MUTEX(lock);

static struct input_dev * powerkey_device;
static struct wake_lock touchwake_wake_lock;
static struct timeval last_powerkeypress;

#define TOUCHWAKE_VERSION "1.1"
#define TIME_LONGPRESS 500
#define POWERPRESS_DELAY 100
#define POWERPRESS_TIMEOUT 1000

// #define DEBUG_PRINT

static void touchwake_disable_touch(void)
{
	#ifdef DEBUG_PRINT
	pr_info("[TOUCHWAKE] Disable touch controls\n");
	#endif
	touchscreen_disable();
	touch_disabled = true;

	return;
}

static void touchwake_enable_touch(void)
{
	#ifdef DEBUG_PRINT
	pr_info("[TOUCHWAKE] Enable touch controls\n");
	#endif
	touchscreen_enable();
	touch_disabled = false;
	return;
}

static void touchwake_early_suspend(struct early_suspend * h)
{
	#ifdef DEBUG_PRINT
	pr_info("[TOUCHWAKE] Enter early suspend\n");
	#endif

	if (touchwake_enabled) {
		if (likely(touchoff_delay > 0))	{
			if (timed_out && !prox_near) {
				#ifdef DEBUG_PRINT
				pr_info("[TOUCHWAKE] Early suspend - enable touch delay\n");
				#endif
				wake_lock(&touchwake_wake_lock);

				schedule_delayed_work(&touchoff_work, msecs_to_jiffies(touchoff_delay));
			} else {
				#ifdef DEBUG_PRINT
				pr_info("[TOUCHWAKE] Early suspend - disable touch immediately\n");
				#endif
				touchwake_disable_touch();
			}
		} else {
			if (timed_out && !prox_near) {
				#ifdef DEBUG_PRINT
				pr_info("[TOUCHWAKE] Early suspend - keep touch enabled indefinately\n");
				#endif
				wake_lock(&touchwake_wake_lock);
			} else {
				#ifdef DEBUG_PRINT
				pr_info("[TOUCHWAKE] Early suspend - disable touch immediately (indefinate mode)\n");
				#endif
				touchwake_disable_touch();
			}
		}
	} else {
		#ifdef DEBUG_PRINT
		pr_info("[TOUCHWAKE] Early suspend - disable touch immediately (TouchWake disabled)\n");
		#endif
		touchwake_disable_touch();
	}

	device_suspended = true;

	return;
}

static void touchwake_late_resume(struct early_suspend * h)
{
	#ifdef DEBUG_PRINT
	pr_info("[TOUCHWAKE] Enter late resume\n");
	#endif

	cancel_delayed_work(&touchoff_work);
	flush_scheduled_work();

	wake_unlock(&touchwake_wake_lock);

	if (touch_disabled)
		touchwake_enable_touch();

	timed_out = true;
	device_suspended = false;

	return;
}

static struct early_suspend touchwake_suspend_data =
{
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
	.suspend = touchwake_early_suspend,
	.resume = touchwake_late_resume,
};

static void touchwake_touchoff(struct work_struct * touchoff_work)
{
	touchwake_disable_touch();
	wake_unlock(&touchwake_wake_lock);

	return;
}

static void press_powerkey(struct work_struct * presspower_work)
{
	input_event(powerkey_device, EV_KEY, KEY_POWER, 1);
	input_event(powerkey_device, EV_SYN, 0, 0);
	msleep(POWERPRESS_DELAY);

	input_event(powerkey_device, EV_KEY, KEY_POWER, 0);
	input_event(powerkey_device, EV_SYN, 0, 0);
	msleep(POWERPRESS_DELAY);

	msleep(POWERPRESS_TIMEOUT);

	mutex_unlock(&lock);

	return;
}

static ssize_t touchwake_status_read(struct device * dev, struct device_attribute * attr, char * buf)
{
	return sprintf(buf, "%u\n", (touchwake_enabled ? 1 : 0));
}

static ssize_t touchwake_status_write(struct device * dev, struct device_attribute * attr, const char * buf, size_t size)
{
	unsigned int data;

	if(sscanf(buf, "%u\n", &data) == 1) {
		pr_devel("%s: %u \n", __FUNCTION__, data);

		if (data == 1) {
			#ifdef DEBUG_PRINT
			pr_info("[TOUCHWAKE] %s: TOUCHWAKE function enabled\n", __FUNCTION__);
			#endif
			touchwake_enabled = true;
		} else if (data == 0) {
			#ifdef DEBUG_PRINT
			pr_info("[TOUCHWAKE] %s: TOUCHWAKE function disabled\n", __FUNCTION__);
			#endif
			touchwake_enabled = false;
		#ifdef DEBUG_PRINT
		} else {
			pr_info("[TOUCHWAKE] %s: invalid input range %u\n", __FUNCTION__, data);
		#endif
		}
	#ifdef DEBUG_PRINT
	} else 	{
		pr_info("[TOUCHWAKE] %s: invalid input\n", __FUNCTION__);
	#endif
	}

	return size;
}

static ssize_t touchwake_delay_read(struct device * dev, struct device_attribute * attr, char * buf)
{
	return sprintf(buf, "%u\n", touchoff_delay);
}

static ssize_t touchwake_delay_write(struct device * dev, struct device_attribute * attr, const char * buf, size_t size)
{
	unsigned int data;

	if(sscanf(buf, "%u\n", &data) == 1) {
		touchoff_delay = data;
		#ifdef DEBUG_PRINT
		pr_info("[TOUCHWAKE] Delay set to %u\n", touchoff_delay); 
		#endif
	#ifdef DEBUG_PRINT
	} else 	{
		pr_info("[TOUCHWAKE] %s: invalid input\n", __FUNCTION__);
	#endif
	}

	return size;
}

static ssize_t touchwake_version(struct device * dev, struct device_attribute * attr, char * buf)
{
	return sprintf(buf, "%s\n", TOUCHWAKE_VERSION);
}

#ifdef DEBUG_PRINT
static ssize_t touchwake_debug(struct device * dev, struct device_attribute * attr, char * buf)
{
	return sprintf(buf, "timed_out : %u\nprox_near : %u\n", (unsigned int) timed_out, (unsigned int) prox_near);
}
#endif

static DEVICE_ATTR(enabled, S_IRUGO | S_IWUGO, touchwake_status_read, touchwake_status_write);
static DEVICE_ATTR(delay, S_IRUGO | S_IWUGO, touchwake_delay_read, touchwake_delay_write);
static DEVICE_ATTR(version, S_IRUGO , touchwake_version, NULL);
#ifdef DEBUG_PRINT
static DEVICE_ATTR(debug, S_IRUGO , touchwake_debug, NULL);
#endif

static struct attribute *touchwake_notification_attributes[] =
{
	&dev_attr_enabled.attr,
	&dev_attr_delay.attr,
	&dev_attr_version.attr,
#ifdef DEBUG_PRINT
	&dev_attr_debug.attr,
#endif
	NULL
};

static struct attribute_group touchwake_notification_group =
{
	.attrs  = touchwake_notification_attributes,
};

static struct miscdevice touchwake_device =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "touchwake",
};

void proximity_detected(void)
{   
	prox_near = true;
	#ifdef DEBUG_PRINT
	pr_info("[TOUCHWAKE] Proximity enabled\n");
	#endif

	return;
}
EXPORT_SYMBOL(proximity_detected);

void proximity_off(void)
{   
	prox_near = false;
	#ifdef DEBUG_PRINT
	pr_info("[TOUCHWAKE] Proximity disabled\n");
	#endif

	return;
}
EXPORT_SYMBOL(proximity_off);

void powerkey_pressed(void)
{
	#ifdef DEBUG_PRINT
	pr_info("[TOUCHWAKE] Powerkey pressed\n");
	#endif

	do_gettimeofday(&last_powerkeypress);
	timed_out = false; // Yank555 : consider user is indeed turning off the device

	return;
}
EXPORT_SYMBOL(powerkey_pressed);

void powerkey_released(void)
{
	#ifdef DEBUG_PRINT
	pr_info("[TOUCHWAKE] Powerkey released\n");
	#endif

	struct timeval now;
	int time_pressed;

	do_gettimeofday(&now);

	time_pressed = (now.tv_sec - last_powerkeypress.tv_sec) * MSEC_PER_SEC +
	(now.tv_usec - last_powerkeypress.tv_usec) / USEC_PER_MSEC;

	if (unlikely(time_pressed > TIME_LONGPRESS || device_suspended)) {
		timed_out = true; // Yank555 : OK, user is not turning off device, but long-pressing Powerkey, or turing on device, so back to normal
		#ifdef DEBUG_PRINT
		pr_info("[TOUCHWAKE] Powerkey longpress detected released\n");
		#endif
	#ifdef DEBUG_PRINT
	} else {
		pr_info("[TOUCHWAKE] Device being turned off\n");
	#endif
	}

	return;
}
EXPORT_SYMBOL(powerkey_released);

void touch_press(void)
{   
	#ifdef DEBUG_PRINT
	pr_info("[TOUCHWAKE] Touch press detected\n");
	#endif

	if (unlikely(device_suspended && touchwake_enabled && !prox_near && mutex_trylock(&lock)))
		schedule_work(&presspower_work);

	return;
}
EXPORT_SYMBOL(touch_press);

void set_powerkeydev(struct input_dev * input_device)
{   
	#ifdef DEBUG_PRINT
	pr_info("[TOUCHWAKE] Powerkey device set to: %p\n", input_device);
	#endif

	powerkey_device = input_device;

	return;
}
EXPORT_SYMBOL(set_powerkeydev);

bool device_is_suspended(void)
{
	return device_suspended;
}
EXPORT_SYMBOL(device_is_suspended);

static int __init touchwake_control_init(void)
{
	int ret;

	pr_info("%s misc_register(%s)\n", __FUNCTION__, touchwake_device.name);
	ret = misc_register(&touchwake_device);

	if (ret) {
		pr_err("%s misc_register(%s) fail\n", __FUNCTION__, touchwake_device.name);

		return 1;
	}

	if (sysfs_create_group(&touchwake_device.this_device->kobj, &touchwake_notification_group) < 0) {
		pr_err("%s sysfs_create_group fail\n", __FUNCTION__);
		pr_err("Failed to create sysfs group for device (%s)!\n", touchwake_device.name);
	}

	register_early_suspend(&touchwake_suspend_data);

	wake_lock_init(&touchwake_wake_lock, WAKE_LOCK_SUSPEND, "touchwake_wake");

	do_gettimeofday(&last_powerkeypress);

	return 0;
}

device_initcall(touchwake_control_init);
