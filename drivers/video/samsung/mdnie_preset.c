/*
 * Author: andip71, 20.12.2013
 * 
 * Version: V2
 *
 * credits: Hardcore for the mdnie settings
 *          Gokhanmoral for some implementation ideas
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

#include <linux/miscdevice.h>
#include <linux/kallsyms.h>

#include "s3cfb.h"
#include "s3cfb_mdnie.h"

#include "mdnie.h"

void mdnie_update(struct mdnie_info *mdnie);


/*****************************************/
// Static variables
/*****************************************/

extern int mdnie_preset;


/*****************************************/
// sysfs interface functions
/*****************************************/

static ssize_t mdnie_preset_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	// return value of current preset
	return sprintf(buf, "MDNIE preset: %d", mdnie_preset);
}


static ssize_t mdnie_preset_store(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned int ret = -EINVAL;
	unsigned int val = 0;
	struct mdnie_info *m;

	// get pointer to global mdnie control structure
	m = *((void **)kallsyms_lookup_name("g_mdnie"));

	// read value from input buffer
	ret = sscanf(buf, "%d", &val);

	// check received preset value for validity
	if ((val >= 0) && (val <= 1))
	{
		// store preset in global variable
		mdnie_preset = val;

		// update mdnie settings
		mdnie_update(m);

		// write debug info
		if (mdnie_preset == 0)
			printk("Boeffla-kernel: mdnie preset set to original\n");
		else
			printk("Boeffla-kernel: mdnie preset set to Hardcore speedmod\n");
	}

	return count;
}



/*****************************************/
// Initialize mdnie preset sysfs folder
/*****************************************/

// define objects
static DEVICE_ATTR(mdnie_preset, S_IRUGO | S_IWUGO, mdnie_preset_show, mdnie_preset_store);

// define attributes
static struct attribute *mdnie_preset_attributes[] = {
	&dev_attr_mdnie_preset.attr,
	NULL
};

// define attribute group
static struct attribute_group mdnie_preset_control_group = {
	.attrs = mdnie_preset_attributes,
};

// define control device
static struct miscdevice mdnie_preset_control_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mdnie_preset",
};


/*****************************************/
// Driver init and exit functions
/*****************************************/

static int mdnie_preset_init(void)
{
	// register mdnie preset control device
	misc_register(&mdnie_preset_control_device);
	if (sysfs_create_group(&mdnie_preset_control_device.this_device->kobj,
				&mdnie_preset_control_group) < 0) {
		printk("Boeffla-kernel: failed to create sys fs object.\n");
		return 0;
	}

	// Print debug info
	printk("Boeffla-kernel: mdnie control device started\n");

	return 0;
}


static void mdnie_preset_exit(void)
{
	// remove mdnie preset control device
	sysfs_remove_group(&mdnie_preset_control_device.this_device->kobj,
                           &mdnie_preset_control_group);

	// Print debug info
	printk("Boeffla-kernel: mdnie control device stopped\n");
}


/* define driver entry points */

module_init(mdnie_preset_init);
module_exit(mdnie_preset_exit);
