/*
 * Author: andip71, 10.01.2013
 *
 * Added sysfs for touchboost frequency : Jean-Pierre Rasquin <yank555.lu@gmail.com>
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

#include <linux/init.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include <linux/cpufreq.h>
#include <mach/cpufreq.h>

#include "touchboost_switch.h"


/*****************************************/
// Global variables
/*****************************************/

int tb_switch 	  = TOUCHBOOST_ON;
int tb_freq	  = TOUCHBOOST_DEFAULT_FREQ;
int tb_freq_level = TOUCHBOOST_FREQ_UNDEFINED;


/*****************************************/
// sysfs interface functions
/*****************************************/

static ssize_t touchboost_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	// return value of current touchboost status
	return sprintf(buf, "Touchboost status: %d", tb_switch);
}


static ssize_t touchboost_switch_store(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned int ret = -EINVAL;
	unsigned int val = 0;

	// read value from input buffer
	ret = sscanf(buf, "%d", &val);

	// check if new status is valid
	if ((val == TOUCHBOOST_OFF) || (val == TOUCHBOOST_ON))
	{
		// store status in global variable
		tb_switch = val;
	}

	return count;
}

static ssize_t touchboost_freq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	// return value of current touchboost status

	return sprintf(buf, "%d - Touchboost frequency\n", tb_freq);

}


static ssize_t touchboost_freq_store(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned int ret = -EINVAL;
	unsigned int input = 0;
	int i;
	struct cpufreq_frequency_table *table;	// Yank : Use system frequency table

	// read value from input buffer
	ret = sscanf(buf, "%d", &input);

	if (ret != 1)
		return -EINVAL;

	table = cpufreq_frequency_get_table(0);	// Yank : Get system frequency table

	if (!table) {
		return -EINVAL;
	} else {
		for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++) // Yank : Allow only frequencies in the system table
			if (table[i].frequency == input) {
				tb_freq = input;
				ret = exynos_cpufreq_get_level(tb_freq, &tb_freq_level); // Yank : Get new frequency level
				if (ret < 0)
					pr_err("Touchboost switch : exynos_cpufreq_get_level error");
				return count;
			}
	}
	return -EINVAL;
}

/*****************************************/
// Initialize touchboost switch sysfs
/*****************************************/

// define objects
static DEVICE_ATTR(touchboost_switch, S_IRUGO | S_IWUGO, touchboost_switch_show, touchboost_switch_store);
static DEVICE_ATTR(touchboost_freq  , S_IRUGO | S_IWUGO, touchboost_freq_show  , touchboost_freq_store  );

// define attributes
static struct attribute *touchboost_switch_attributes[] = {
	&dev_attr_touchboost_switch.attr,
	&dev_attr_touchboost_freq.attr,
	NULL
};

// define attribute group
static struct attribute_group touchboost_switch_control_group = {
	.attrs = touchboost_switch_attributes,
};

// define control device
static struct miscdevice touchboost_switch_control_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "touchboost_switch",
};


/*****************************************/
// Driver init and exit functions
/*****************************************/

static int touchboost_switch_init(void)
{
	int ret;

	// register touchboost switch device
	misc_register(&touchboost_switch_control_device);
	if (sysfs_create_group(&touchboost_switch_control_device.this_device->kobj,
				&touchboost_switch_control_group) < 0) {
		printk("Touchboost switch : failed to create touchboost switch sys fs object.\n");
		return 0;
	}

	// Yank : Get startup frequency level if not yet defined
	if (tb_freq_level == TOUCHBOOST_FREQ_UNDEFINED) {
		ret = exynos_cpufreq_get_level(tb_freq, &tb_freq_level);
		if (ret < 0)
			pr_err("Touchboost switch : exynos_cpufreq_get_level error");
	}

	// Print debug info
	printk("Touchboost switch : device initialized\n");

	return 0;
}


static void touchboost_switch_exit(void)
{
	// remove touchboost switch device
	sysfs_remove_group(&touchboost_switch_control_device.this_device->kobj,
                           &touchboost_switch_control_group);

	// Print debug info
	printk("Touchboost switch : device stopped\n");
}


/* define driver entry points */

module_init(touchboost_switch_init);
module_exit(touchboost_switch_exit);
