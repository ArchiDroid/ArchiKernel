/*
 * Author: andip71, 27.01.2013
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


#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/charge_level.h>


/* sysfs interface for charge levels */

static ssize_t charge_level_ac_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	// print current value
	return sprintf(buf, "%d mA", ac_level);
}


static ssize_t charge_level_ac_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int ret = -EINVAL;
	int val;

	// read value from input buffer
	ret = sscanf(buf, "%d", &val);

	// check whether value is within the valid ranges and adjust accordingly
	if (val > AC_CHARGE_LEVEL_MAX)
		val = AC_CHARGE_LEVEL_MAX;

	if (val < AC_CHARGE_LEVEL_MIN)
		val = AC_CHARGE_LEVEL_MIN;

	// store value
	ac_level = val;

	return count;
}


static ssize_t charge_level_usb_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	// print current value
	return sprintf(buf, "%d mA", usb_level);
}


static ssize_t charge_level_usb_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int ret = -EINVAL;
	int val;

	// read value from input buffer
	ret = sscanf(buf, "%d", &val);

	// check whether value is within the valid ranges and adjust accordingly
	if (val > USB_CHARGE_LEVEL_MAX)
		val = USB_CHARGE_LEVEL_MAX;

	if (val < USB_CHARGE_LEVEL_MIN)
		val = USB_CHARGE_LEVEL_MIN;

	// store value
	usb_level = val;

	return count;
}

static ssize_t charge_level_wireless_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	// print current value
	return sprintf(buf, "%d mA", wireless_level);
}


static ssize_t charge_level_wireless_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int ret = -EINVAL;
	int val;

	// read value from input buffer
	ret = sscanf(buf, "%d", &val);

	// check whether value is within the valid ranges and adjust accordingly
	if (val > WIRELESS_CHARGE_LEVEL_MAX)
		val = WIRELESS_CHARGE_LEVEL_MAX;

	if (val < WIRELESS_CHARGE_LEVEL_MIN)
		val = WIRELESS_CHARGE_LEVEL_MIN;

	// store value
	wireless_level = val;

	return count;
}


static ssize_t ignore_unstable_power_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	// print current value
	return sprintf(buf, "%d", ignore_unstable_power);
}


static ssize_t ignore_unstable_power_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int ret = -EINVAL;
	int val;

	// read value from input buffer
	ret = sscanf(buf, "%d", &val);

	// check whether value is valid
	if ((val == 0) || (val == 1))
		ignore_unstable_power = val;

	return count;
}


static ssize_t ignore_safety_margin_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	// print current value
	return sprintf(buf, "%d", ignore_safety_margin);
}


static ssize_t ignore_safety_margin_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int ret = -EINVAL;
	int val;

	// read value from input buffer
	ret = sscanf(buf, "%d", &val);

	// check whether value is valid
	if ((val == 0) || (val == 1))
		ignore_safety_margin = val;

	return count;
}



/* Initialize charge level sysfs folder */

static struct kobj_attribute charge_level_ac_attribute =
__ATTR(charge_level_ac, 0666, charge_level_ac_show, charge_level_ac_store);

static struct kobj_attribute charge_level_usb_attribute =
__ATTR(charge_level_usb, 0666, charge_level_usb_show, charge_level_usb_store);

static struct kobj_attribute charge_level_wireless_attribute =
__ATTR(charge_level_wireless, 0666, charge_level_wireless_show, charge_level_wireless_store);

static struct kobj_attribute ignore_unstable_power_attribute =
__ATTR(ignore_unstable_power, 0666, ignore_unstable_power_show, ignore_unstable_power_store);

static struct kobj_attribute ignore_safety_margin_attribute =
__ATTR(ignore_safety_margin, 0666, ignore_safety_margin_show, ignore_safety_margin_store);

static struct attribute *charge_level_attrs[] = 
{
	&charge_level_ac_attribute.attr,
	&charge_level_usb_attribute.attr,
	&charge_level_wireless_attribute.attr,
	&ignore_unstable_power_attribute.attr,
	&ignore_safety_margin_attribute.attr,
	NULL,
};

static struct attribute_group charge_level_attr_group = 
{
	.attrs = charge_level_attrs,
};

static struct kobject *charge_level_kobj;


int charge_level_init(void)
{
	int charge_level_retval;

	charge_level_kobj = kobject_create_and_add("charge_levels", kernel_kobj);

	if (!charge_level_kobj)
	{
		printk("Boeffla-Kernel: failed to create kernel object for charge level interface.\n");
		return -ENOMEM;
	}

	charge_level_retval = sysfs_create_group(charge_level_kobj, &charge_level_attr_group);

	if (charge_level_retval)
	{
		kobject_put(charge_level_kobj);
		printk("Boeffla-Kernel: failed to create fs object for charge level interface.\n");
		return (charge_level_retval);
	}

	// print debug info
	printk("Boeffla-Kernel: charge level interface started.\n");

    return (charge_level_retval);
}


void charge_level_exit(void)
{
	kobject_put(charge_level_kobj);

	// print debug info
	printk("Boeffla-Kernel: charge level interface stopped.\n");
}


/* define driver entry points */
module_init(charge_level_init);
module_exit(charge_level_exit);
