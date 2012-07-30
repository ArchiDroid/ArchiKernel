/*
 * PMIC Vibrator device 
 *
 */
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <../../../../drivers/staging/android/timed_output.h>
#include <linux/sched.h>

#include <mach/msm_rpcrouter.h>
#include <mach/pmic.h>

#define PMIC_VIBRATOR_LEVEL	3000

static struct work_struct work_vibrator_on;
static struct work_struct work_vibrator_off;
static struct hrtimer vibe_timer;

static int is_vib_congiged = 0;

static int config_pmic_vibrator(void)
{
	/* Disable the vibrator motor. */
	pmic_vib_mot_set_volt(0);

#if 1 /* Configures vibrator motor to manual mode control */
	if (pmic_vib_mot_set_mode(PM_VIB_MOT_MODE__MANUAL) < 0) {
		printk(KERN_ERR "Failed to set Vibrator manual mode\n");
		return -EFAULT;
	}
#else /* configures vibrator motor to automatic and use DBUS[1 or 2 or 3] as the control line */
	if (pmic_vib_mot_set_mode(PM_VIB_MOT_MODE__DBUS2) < 0) {
		printk(KERN_ERR "Failed to set Vibrator mode\n");
		return -EFAULT;
	}

	/* configures DBUS polarity */
	if (pmic_vib_mot_set_polarity(PM_VIB_MOT_POL__ACTIVE_HIGH) < 0) {
		printk(KERN_ERR "Failed to set Vibrator polarity \n");
		return -EFAULT;
	}
#endif
	return 0;
}

static int set_pmic_vibrator(int on)
{
	if (on) {
		if (!is_vib_congiged) {
			config_pmic_vibrator();
			is_vib_congiged = 1;
		}
		if (pmic_vib_mot_set_volt(PMIC_VIBRATOR_LEVEL) < 0 ) {
			printk(KERN_ERR "Failed to set Vibrator Motor Voltage level\n");
			return -EFAULT;
		}
	}
	else {
		pmic_vib_mot_set_volt(0);
	}

	return 0;
}

static void pmic_vibrator_on(struct work_struct *work)
{
	set_pmic_vibrator(1);
}

static void pmic_vibrator_off(struct work_struct *work)
{
	set_pmic_vibrator(0);
}

static void timed_vibrator_on(struct timed_output_dev *sdev)
{
	schedule_work(&work_vibrator_on);
}

static void timed_vibrator_off(struct timed_output_dev *sdev)
{
	schedule_work(&work_vibrator_off);
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	hrtimer_cancel(&vibe_timer);

	if (value == 0)
		timed_vibrator_off(dev);
	else {
		value = (value > 15000 ? 15000 : value);

		timed_vibrator_on(dev);

		hrtimer_start(&vibe_timer,
			      ktime_set(value / 1000, (value % 1000) * 1000000),
			      HRTIMER_MODE_REL);
	}
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	if (hrtimer_active(&vibe_timer)) {
		ktime_t r = hrtimer_get_remaining(&vibe_timer);
		return r.tv.sec * 1000 + r.tv.nsec / 1000000;
	} else
		return 0;
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	timed_vibrator_off(NULL);
	return HRTIMER_NORESTART;
}

static struct timed_output_dev pmic_vibrator = {
	.name = "vibrator",
	.get_time = vibrator_get_time,
	.enable = vibrator_enable,
};

static int __init lge_add_pmic_vibrator(void)
{
	INIT_WORK(&work_vibrator_on, pmic_vibrator_on);
	INIT_WORK(&work_vibrator_off, pmic_vibrator_off);

	hrtimer_init(&vibe_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vibe_timer.function = vibrator_timer_func;

	timed_output_dev_register(&pmic_vibrator);
	return 0;
}

module_init(lge_add_pmic_vibrator);

MODULE_DESCRIPTION("timed output pmic vibrator device");
MODULE_LICENSE("GPL");

