/*
 * Author: Jean-Pierre Rasquin <yank555.lu@gmail.com>
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

/*
 * Early Suspend Delay v1.0a - SysFS interface :
 * ---------------------------------------------
 *
 * /sys/kernel/early_suspend/early_suspend_delay (rw)
 *
 *   time to pause in miliseconds (0 - 700ms)
 *
 * /sys/kernel/early_suspend/version (ro)
 *
 *   display early suspend delay version information
 *
 */

#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/fbearlysuspenddelay.h>

int early_suspend_delay;

/* sysfs interface for "early_suspend_delay" */
static ssize_t early_suspend_delay_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	switch (early_suspend_delay) {
		case EARLY_SUSPEND_DISABLED:		return sprintf(buf, "0ms - Disabled (default)\n");
		case EARLY_SUSPEND_MAX:			return sprintf(buf, "700ms - maximum allowed\n");
		default:				return sprintf(buf, "%dms\n",early_suspend_delay);
	}
}

static ssize_t early_suspend_delay_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	int new_early_suspend_delay;

	sscanf(buf, "%du", &new_early_suspend_delay);

	if (EARLY_SUSPEND_DISABLED <= new_early_suspend_delay && new_early_suspend_delay <= EARLY_SUSPEND_MAX)
		early_suspend_delay = new_early_suspend_delay;
	return count;
}

static struct kobj_attribute early_suspend_delay_attribute =
__ATTR(early_suspend_delay, 0666, early_suspend_delay_show, early_suspend_delay_store);

static struct attribute *early_suspend_delay_attrs[] = {
&early_suspend_delay_attribute.attr,
NULL,
};

static struct attribute_group early_suspend_delay_attr_group = {
.attrs = early_suspend_delay_attrs,
};

/* sysfs interface for "version" */
static ssize_t version_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "Early suspend delay v1.0a\n");
}

static ssize_t version_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
/* no user change allowed */
return count;
}

static struct kobj_attribute version_attribute =
__ATTR(version, 0444, version_show, version_store);

static struct attribute *version_attrs[] = {
&version_attribute.attr,
NULL,
};

static struct attribute_group version_attr_group = {
.attrs = version_attrs,
};

/* Initialize early suspend sysfs folder */
static struct kobject *early_suspend_delay_kobj;

int early_suspend_delay_init(void)
{
	int early_suspend_delay_retval;
	int version_retval;

	early_suspend_delay = EARLY_SUSPEND_DEFAULT; /* set default early suspend delay */

        early_suspend_delay_kobj = kobject_create_and_add("early_suspend", kernel_kobj);
        if (!early_suspend_delay_kobj) {
                return -ENOMEM;
        }
        early_suspend_delay_retval = sysfs_create_group(early_suspend_delay_kobj, &early_suspend_delay_attr_group);
        version_retval = sysfs_create_group(early_suspend_delay_kobj, &version_attr_group);
        if (early_suspend_delay_retval && version_retval)
                kobject_put(early_suspend_delay_kobj);
        return (early_suspend_delay_retval && version_retval);
}
/* end sysfs interface */

void early_suspend_delay_exit(void)
{
	kobject_put(early_suspend_delay_kobj);
}

module_init(early_suspend_delay_init);
module_exit(early_suspend_delay_exit);
