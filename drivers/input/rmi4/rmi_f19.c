/*
 * Copyright (c) 2011 Synaptics Incorporated
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <linux/kernel.h>
#include <linux/input/rmi.h>
#include <linux/input/rmi_driver.h>
#include <linux/input.h>
#include <linux/slab.h>

#define QUERY_BASE_INDEX 1
#define MAX_LEN 256
#define MAX_BUFFER_LEN 80

#define SENSOR_MAP_MIN			0
#define SENSOR_MAP_MAX			127
#define SENSITIVITY_ADJ_MIN		0
#define SENSITIVITY_ADJ_MAX		31
#define HYSTERESIS_THRESHOLD_MIN	0
#define HYSTERESIS_THRESHOLD_MAX	15

union f19_0d_query {
	struct {
		u8 configurable:1;
		u8 has_sensitivity_adjust:1;
		u8 has_hysteresis_threshold:1;
		u8 reserved_1:5;

		u8 button_count:5;
		u8 reserved_2:3;
	};
	u8 regs[2];
};

union f19_0d_control_0 {
	struct {
		u8 button_usage:2;
		u8 filter_mode:2;
	};
	u8 regs[1];
};

struct f19_0d_control_3 {
	u8 sensor_map_button:7;
	/*u8 sensitivity_button;*/
};

struct f19_0d_control_5 {
	u8 sensitivity_adj:5;
};

struct f19_0d_control_6 {
	u8 hysteresis_threshold:4;
};

struct f19_0d_control {
	union f19_0d_control_0 general_control;
	u8 *int_enabled_button;
	u8 *single_button;
	struct f19_0d_control_3 *sensor_map;
	struct f19_0d_control_5 all_button_sensitivity_adj;
	struct f19_0d_control_6 all_button_hysteresis_threshold;
};
/* data specific to fn $19 that needs to be kept around */
struct f19_data {
	struct f19_0d_control button_control;
	union f19_0d_query button_query;
	u8 button_rezero;
	unsigned char button_count;
	unsigned char button_bitmask_size;
	unsigned char *button_data_buffer;
	unsigned char *button_map;
	char input_name[MAX_LEN];
	char input_phys[MAX_LEN];
	struct input_dev *input;
	int general_control_address;
	int int_enable_button_address;
	int single_button_address;
	int sensor_map_address;
	int all_button_sensitivity_address;
	int all_button_threshold_address;
};

static ssize_t rmi_f19_button_count_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t rmi_f19_button_map_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t rmi_f19_button_map_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t rmi_f19_rezero_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t rmi_f19_rezero_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t rmi_f19_has_hysteresis_threshold_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t rmi_f19_has_sensitivity_adjust_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t rmi_f19_configurable_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t rmi_f19_filter_mode_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t rmi_f19_filter_mode_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t rmi_f19_button_usage_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t rmi_f19_button_usage_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t rmi_f19_interrupt_enable_button_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t rmi_f19_interrupt_enable_button_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t rmi_f19_single_button_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t rmi_f19_single_button_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t rmi_f19_sensor_map_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t rmi_f19_sensor_map_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t rmi_f19_sensitivity_adjust_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t rmi_f19_sensitivity_adjust_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t rmi_f19_hysteresis_threshold_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t rmi_f19_hysteresis_threshold_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);



static int rmi_f19_alloc_memory(struct rmi_function_container *fc);

static void rmi_f19_free_memory(struct rmi_function_container *fc);

static int rmi_f19_initialize(struct rmi_function_container *fc);

static int rmi_f19_register_device(struct rmi_function_container *fc);

static int rmi_f19_create_sysfs(struct rmi_function_container *fc);

static int rmi_f19_config(struct rmi_function_container *fc);

static int rmi_f19_reset(struct rmi_function_container *fc);

static struct device_attribute attrs[] = {
	__ATTR(button_count, RMI_RO_ATTR,
		rmi_f19_button_count_show, rmi_store_error),
	__ATTR(button_map, RMI_RW_ATTR,
		rmi_f19_button_map_show, rmi_f19_button_map_store),
	__ATTR(rezero, RMI_RW_ATTR,
		rmi_f19_rezero_show, rmi_f19_rezero_store),
	__ATTR(has_hysteresis_threshold, RMI_RO_ATTR,
		rmi_f19_has_hysteresis_threshold_show, rmi_store_error),
	__ATTR(has_sensitivity_adjust, RMI_RO_ATTR,
		rmi_f19_has_sensitivity_adjust_show, rmi_store_error),
	__ATTR(configurable, RMI_RO_ATTR,
		rmi_f19_configurable_show, rmi_store_error),
	__ATTR(filter_mode, RMI_RW_ATTR,
		rmi_f19_filter_mode_show, rmi_f19_filter_mode_store),
	__ATTR(button_usage, RMI_RW_ATTR,
		rmi_f19_button_usage_show, rmi_f19_button_usage_store),
	__ATTR(interrupt_enable_button, RMI_RW_ATTR,
		rmi_f19_interrupt_enable_button_show,
		rmi_f19_interrupt_enable_button_store),
	__ATTR(single_button, RMI_RW_ATTR,
		rmi_f19_single_button_show, rmi_f19_single_button_store),
	__ATTR(sensor_map, RMI_RW_ATTR,
		rmi_f19_sensor_map_show, rmi_f19_sensor_map_store),
	__ATTR(sensitivity_adjust, RMI_RW_ATTR,
		rmi_f19_sensitivity_adjust_show,
		rmi_f19_sensitivity_adjust_store),
	__ATTR(hysteresis_threshold, RMI_RW_ATTR,
		rmi_f19_hysteresis_threshold_show,
		rmi_f19_hysteresis_threshold_store)
};


int rmi_f19_read_control_parameters(struct rmi_device *rmi_dev,
	struct f19_data *f19)
{
	int error = 0;
	struct f19_0d_control *button_control = &f19->button_control;
	union f19_0d_query *button_query = &f19->button_query;

	int ctrl_base_addr = f19->general_control_address;

	error = rmi_read_block(rmi_dev, ctrl_base_addr,
			(u8 *)&button_control->general_control,
			sizeof(union f19_0d_control_0));
	if (error < 0) {
		printk( "FTUCH : F19 Device(%s) failed to read f19_0d_control_0(%d)\n", dev_name( &rmi_dev->dev ), error );
		return error;
	}
	ctrl_base_addr = ctrl_base_addr +
			sizeof(union f19_0d_control_0);

	f19->int_enable_button_address = ctrl_base_addr;
	if (button_control->int_enabled_button) {
		error = rmi_read_block(rmi_dev, ctrl_base_addr,
			button_control->int_enabled_button,
			f19->button_bitmask_size);
		if (error < 0) {
			printk( "FTUCH : F19 Device(%s) failed to read f19_0d_control_2(%d)\n", dev_name( &rmi_dev->dev ), error );
			return error;
		}
		ctrl_base_addr = ctrl_base_addr + f19->button_bitmask_size;
	}

	if (button_control->single_button) {
		f19->single_button_address = ctrl_base_addr;
		error = rmi_read_block(rmi_dev, ctrl_base_addr,
				button_control->single_button,
				f19->button_bitmask_size);
		if (error < 0) {
			printk( "FTUCH : F19 Device(%s) failed to read f19_0d_control_2(%d)\n", dev_name( &rmi_dev->dev ), error );
			return error;
		}
		ctrl_base_addr = ctrl_base_addr + f19->button_bitmask_size;
	}

	if (button_control->sensor_map) {
		f19->sensor_map_address = ctrl_base_addr;
		error = rmi_read_block(rmi_dev, ctrl_base_addr,
			(u8 *)(button_control->sensor_map),
			sizeof(struct f19_0d_control_3)*f19->button_count);
		if (error < 0) {
			printk( "FTUCH : F19 Device(%s) failed to read f19_0d_control_3(%d)\n", dev_name( &rmi_dev->dev ), error );
			return error;
		}
		ctrl_base_addr = ctrl_base_addr +
			(sizeof(struct f19_0d_control_3)*f19->button_count);
		ctrl_base_addr += (sizeof(u8)*f19->button_count);
	}

	if (button_query->has_sensitivity_adjust) {
		f19->all_button_sensitivity_address = ctrl_base_addr;
		error = rmi_read_block(rmi_dev, ctrl_base_addr,
				(u8 *)&button_control->
					all_button_sensitivity_adj,
				sizeof(struct f19_0d_control_5));
		if (error < 0) {
			printk( "FTUCH : F19 Device(%s) failed to read f19_0d_control_5(%d)\n", dev_name( &rmi_dev->dev ), error );
			return error;
		}
		ctrl_base_addr = ctrl_base_addr +
			sizeof(struct f19_0d_control_5);
	}
	if (button_query->has_hysteresis_threshold) {
		f19->all_button_threshold_address = ctrl_base_addr;
		error = rmi_read_block(rmi_dev, ctrl_base_addr,
				(u8 *)&button_control->
					all_button_hysteresis_threshold,
				sizeof(struct f19_0d_control_6));
		if (error < 0) {
			printk( "FTUCH : F19 Device(%s) failed to read f19_0d_control_6(%d)\n", dev_name( &rmi_dev->dev ), error );
			return error;
		}
		ctrl_base_addr = ctrl_base_addr +
			sizeof(struct f19_0d_control_6);
	}
	return 0;
}


static int rmi_f19_init(struct rmi_function_container *fc)
{
	int rc;

	rc = rmi_f19_alloc_memory(fc);
	if (rc < 0)
		goto err_free_data;

	rc = rmi_f19_initialize(fc);
	if (rc < 0)
		goto err_free_data;

	rc = rmi_f19_register_device(fc);
	if (rc < 0)
		goto err_free_data;

	rc = rmi_f19_create_sysfs(fc);
	if (rc < 0)
		goto err_free_data;

	return 0;

err_free_data:
	rmi_f19_free_memory(fc);

	return rc;
}


static int rmi_f19_alloc_memory(struct rmi_function_container *fc)
{
	struct f19_data *f19;
	int rc;

	f19 = kzalloc(sizeof(struct f19_data), GFP_KERNEL);
	if (!f19) {
		printk( "FTUCH : F19 Device(%s) failed to allocate function data\n", dev_name( &fc->dev ) );
		return -ENOMEM;
	}
	fc->data = f19;

	rc = rmi_read_block(fc->rmi_dev,
						fc->fd.query_base_addr,
						(u8 *)&f19->button_query,
						sizeof(union f19_0d_query));
	if (rc < 0) {
		printk( "FTUCH : F19 Device(%s) failed to read query register\n", dev_name( &fc->dev ) );
		return rc;
	}

	f19->button_count = f19->button_query.button_count;
	f19->button_bitmask_size = sizeof(u8)*(f19->button_count + 7) / 8;
	f19->button_data_buffer =
	    kcalloc(f19->button_bitmask_size,
		    sizeof(unsigned char), GFP_KERNEL);
	if (!f19->button_data_buffer) {
		printk( "FTUCH : F19 Device(%s) failed to allocate button data buffer\n", dev_name( &fc->dev ) );
		return -ENOMEM;
	}

	f19->button_map = kcalloc(f19->button_count,
				sizeof(unsigned char), GFP_KERNEL);
	if (!f19->button_map) {
		printk( "FTUCH : F19 Device(%s) failed to allocate button map\n", dev_name( &fc->dev ) );
		return -ENOMEM;
	}

	f19->button_control.int_enabled_button =
		kzalloc(f19->button_bitmask_size, GFP_KERNEL);
	if (!f19->button_control.int_enabled_button) {
		printk( "FTUCH : F19 Device(%s) failed to allocate interrupt button\n", dev_name( &fc->dev ) );
		return -ENOMEM;
	}

	f19->button_control.single_button =
		kzalloc(f19->button_bitmask_size, GFP_KERNEL);
	if (!f19->button_control.single_button) {
		printk( "FTUCH : F19 Device(%s) failed to allocate single button\n", dev_name( &fc->dev ) );
		return -ENOMEM;
	}

	f19->button_control.sensor_map = kzalloc(f19->button_count *
				sizeof(struct f19_0d_control_3), GFP_KERNEL);
	if (!f19->button_control.sensor_map) {
		printk( "FTUCH : F19 Device(%s) failed to allocate f19_0d_control_3\n", dev_name( &fc->dev ) );
		return -ENOMEM;
	}

	return 0;
}



static void rmi_f19_free_memory(struct rmi_function_container *fc)
{
	struct f19_data *f19 = fc->data;

	if (f19) {
		kfree(f19->button_data_buffer);
		kfree(f19->button_map);
		kfree(f19->button_control.int_enabled_button);
		kfree(f19->button_control.single_button);
		kfree(f19->button_control.sensor_map);
		kfree(f19);
		fc->data = NULL;
	}
}


static int rmi_f19_initialize(struct rmi_function_container *fc)
{
	struct rmi_device *rmi_dev = fc->rmi_dev;
	struct rmi_device_platform_data *pdata;
	struct f19_data *f19 = fc->data;
	int i;
	int rc;

	printk( "FTUCH : F19 Device(%s) intializing values\n", dev_name( &fc->dev ) );

	/* initial all default values for f19 data here */
	rc = rmi_read(rmi_dev, fc->fd.command_base_addr,
		(u8 *)&f19->button_rezero);
	if (rc < 0) {
		printk( "FTUCH : F19 Device(%s) failed to read command register\n", dev_name( &fc->dev ) );
		return rc;
	}
	f19->button_rezero = f19->button_rezero & 1;

	pdata = to_rmi_platform_data(rmi_dev);
	if (pdata) {
		if (!pdata->button_map) {
			printk( "FTUCH : F19 Device(%s) button_map is NULL\n", dev_name( &fc->dev ) );
		} else if (pdata->button_map->nbuttons != f19->button_count) {
			printk( "FTUCH : F19 Device(%s) platformdata button map size (%d) != number of buttons on device (%d) - ignored\n", dev_name( &fc->dev ), pdata->button_map->nbuttons, f19->button_count );
		} else if (!pdata->button_map->map) {
			printk( "FTUCH : F19 Device(%s) platformdata button map is missing\n", dev_name( &fc->dev ) );
		} else {
			for (i = 0; i < pdata->button_map->nbuttons; i++)
				f19->button_map[i] = pdata->button_map->map[i];
		}
	}

	f19->general_control_address = fc->fd.control_base_addr;
	rc = rmi_f19_read_control_parameters(rmi_dev, f19);
	if (rc < 0) {
		printk( "FTUCH : F19 Device(%s) failed to initialize control params\n", dev_name( &fc->dev ) );
		return rc;
	}

	return 0;
}



static int rmi_f19_register_device(struct rmi_function_container *fc)
{
	struct rmi_device *rmi_dev = fc->rmi_dev;
	struct input_dev *input_dev;
	struct f19_data *f19 = fc->data;
	int i;
	int rc;

	input_dev = input_allocate_device();
	if (!input_dev) {
		printk( "FTUCH : F19 Device(%s) failed to allocate input device\n", dev_name( &fc->dev ) );
		return -ENOMEM;
	}

	f19->input = input_dev;
	snprintf(f19->input_name, MAX_LEN, "%sfn%02x", dev_name(&rmi_dev->dev),
		fc->fd.function_number);
	input_dev->name = f19->input_name;
	snprintf(f19->input_phys, MAX_LEN, "%s/input0", input_dev->name);
	input_dev->phys = f19->input_phys;
	input_dev->dev.parent = &rmi_dev->dev;
	input_set_drvdata(input_dev, f19);

	/* Set up any input events. */
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);
	/* set bits for each button... */
	for (i = 0; i < f19->button_count; i++)
		set_bit(f19->button_map[i], input_dev->keybit);
	rc = input_register_device(input_dev);
	if (rc < 0) {
		printk( "FTUCH : F19 Device(%s) failed to register input device\n", dev_name( &fc->dev ) );
		goto error_free_device;
	}

	return 0;

error_free_device:
	input_free_device(input_dev);

	return rc;
}


static int rmi_f19_create_sysfs(struct rmi_function_container *fc)
{
	int attr_count = 0;
	int rc;
	char *name;
	struct f19_data *data;
	union f19_0d_query *button_query;

	data = fc->data;
	button_query = &data->button_query;

	dev_dbg(&fc->dev, "Creating sysfs files.\n");
	/* Set up sysfs device attributes. */
	for (attr_count = 0; attr_count < ARRAY_SIZE(attrs); attr_count++) {
		name = (char *) attrs[attr_count].attr.name;
		if (!button_query->has_sensitivity_adjust &&
			!strcmp(name, "sensitivity_adjust"))
			continue;
		if (!button_query->has_hysteresis_threshold &&
			!strcmp(name, "hysteresis_threshold"))
			continue;
		if (sysfs_create_file
		    (&fc->dev.kobj, &attrs[attr_count].attr) < 0) {
			printk( "FTUCH : F19 Device(%s) failed to create sysfs file for %s\n", dev_name( &fc->dev ), attrs[attr_count].attr.name );
			rc = -ENODEV;
			goto err_remove_sysfs;
		}
	}

	return 0;

err_remove_sysfs:
	for (attr_count--; attr_count >= 0; attr_count--)
		sysfs_remove_file(&fc->dev.kobj,
						  &attrs[attr_count].attr);
	return rc;

}



static int rmi_f19_config(struct rmi_function_container *fc)
{
	struct f19_data *data;
	int retval;
	union f19_0d_query *button_query;

	data = fc->data;
	button_query = &data->button_query;
	retval = rmi_write_block(fc->rmi_dev, data->general_control_address,
		(u8 *)&data->button_control.general_control,
			sizeof(union f19_0d_control_0));
	if (retval < 0) {
		printk( "FTUCH : F19 Device(%s) could not write general_control to 0x%x\n", dev_name( &fc->dev ), fc->fd.control_base_addr );
		return retval;
	}

	retval = rmi_write_block(fc->rmi_dev, data->int_enable_button_address,
		data->button_control.int_enabled_button,
		data->button_bitmask_size);
	if (retval < 0) {
		printk( "FTUCH : F19 Device(%s) could not write interrupt_enable_store to 0x%x\n", dev_name( &fc->dev ), data->int_enable_button_address );
		return retval;
	}

	retval = rmi_write_block(fc->rmi_dev, data->single_button_address,
		data->button_control.single_button,
		data->button_bitmask_size);
	if (retval < 0) {
		printk( "FTUCH : F19 Device(%s) could not write interrupt_enable_store to 0x%x\n", dev_name( &fc->dev ), data->single_button_address );
		return -EINVAL;
	}

	retval = rmi_write_block(fc->rmi_dev,  data->sensor_map_address,
		(u8 *)data->button_control.sensor_map,
			sizeof(struct f19_0d_control_3)*data->button_count);
	if (retval < 0) {
		printk( "FTUCH : F19 Device(%s) could not sensor_map_store to 0x%x\n", dev_name( &fc->dev ), data->sensor_map_address );
		return -EINVAL;
	}
	if (button_query->has_sensitivity_adjust) {
		retval = rmi_write_block(fc->rmi_dev,
			data->all_button_sensitivity_address,
			(u8 *)&data->button_control.all_button_sensitivity_adj,
			sizeof(struct f19_0d_control_5));
		if (retval < 0) {
			printk( "FTUCH : F19 Device(%s) could not sensitivity_adjust_store to 0x%x\n", dev_name( &fc->dev ), data->all_button_sensitivity_address );
			return retval;
		}
	}
	if (!button_query->has_hysteresis_threshold) {
		retval = rmi_write_block(fc->rmi_dev,
			data->all_button_threshold_address,
			(u8 *)&data->button_control.
				all_button_hysteresis_threshold,
			sizeof(struct f19_0d_control_6));
		if (retval < 0) {
			printk( "FTUCH : F19 Device(%s) could not write all_button hysteresis threshold to 0x%x\n", dev_name( &fc->dev ), data->all_button_threshold_address );
			return -EINVAL;
		}
	}
	return 0;
}


static int rmi_f19_reset(struct rmi_function_container *fc)
{
	/* we do nnothing here */
	return 0;
}


static void rmi_f19_remove(struct rmi_function_container *fc)
{
	struct f19_data *f19 = fc->data;
	int attr_count = 0;

	for (attr_count = 0; attr_count < ARRAY_SIZE(attrs); attr_count++)
		sysfs_remove_file(&fc->dev.kobj,
				  &attrs[attr_count].attr);

	input_unregister_device(f19->input);

	rmi_f19_free_memory(fc);
}

int rmi_f19_attention(struct rmi_function_container *fc, u8 *irq_bits)
{
	struct rmi_device *rmi_dev = fc->rmi_dev;
	struct f19_data *f19 = fc->data;
	int data_base_addr = fc->fd.data_base_addr;
	int error;
	int button;

	/* Read the button data. */

	error = rmi_read_block(rmi_dev, data_base_addr, f19->button_data_buffer,
			f19->button_bitmask_size);
	if (error < 0) {
		printk( "FTUCH : F19 Device(%s) failed to read button data registers\n", dev_name( &fc->dev ) );
		return error;
	}

	/* Generate events for buttons that change state. */
	for (button = 0; button < f19->button_count;
	     button++) {
		int button_reg;
		int button_shift;
		bool button_status;

		/* determine which data byte the button status is in */
		button_reg = button / 8;
		/* bit shift to get button's status */
		button_shift = button % 8;
		button_status =
		    ((f19->button_data_buffer[button_reg] >> button_shift)
			& 0x01) != 0;

		/* if the button state changed from the last time report it
		 * and store the new state */
			/* Generate an event here. */
		input_report_key(f19->input, f19->button_map[button],
				 button_status);
	}

	input_sync(f19->input); /* sync after groups of events */
	return 0;
}

static struct rmi_function_handler function_handler = {
	.func = 0x19,
	.init = rmi_f19_init,
	.config = rmi_f19_config,
	.reset = rmi_f19_reset,
	.attention = rmi_f19_attention,
	.remove = rmi_f19_remove
};

static int __init rmi_f19_module_init(void)
{
	int error;

	error = rmi_register_function_driver(&function_handler);
	if (error < 0) {
		printk( "FTUCH : F19 register failed!!!\n" );
		return error;
	}

	return 0;
}

static void rmi_f19_module_exit(void)
{
	rmi_unregister_function_driver(&function_handler);
}

static ssize_t rmi_f19_filter_mode_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;

	fc = to_rmi_function_container(dev);
	data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n",
			data->button_control.general_control.filter_mode);

}

static ssize_t rmi_f19_filter_mode_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	unsigned int new_value;
	unsigned int old_value;
	int result;

	fc = to_rmi_function_container(dev);
	data = fc->data;
	if (sscanf(buf, "%u", &new_value) < 1) {
		printk( "FTUCH : F19 Device(%s) ERROR - filter_mode_store has an invalid len\n", dev_name( dev ) );
		return -EINVAL;
	}

	if (new_value < 0 || new_value > 4) {
		printk( "FTUCH : F19 Device(%s) ERROR - filter_mode_store has an invalid value(%d)\n", dev_name( dev ), new_value );
		return -EINVAL;
	}
	old_value = data->button_control.general_control.filter_mode;
	data->button_control.general_control.filter_mode = new_value;
	result = rmi_write_block(fc->rmi_dev, data->general_control_address,
		(u8 *)&(data->button_control.general_control),
			sizeof(union f19_0d_control_0));
	if (result < 0) {
		data->button_control.general_control.filter_mode = old_value;
		printk( "FTUCH : F19 Device(%s) could not write filter_mode_store to 0x%x\n", dev_name( dev ), fc->fd.control_base_addr );
		return result;
	}

	return count;
}

static ssize_t rmi_f19_button_usage_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;

	fc = to_rmi_function_container(dev);
	data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n",
			data->button_control.general_control.button_usage);

}

static ssize_t rmi_f19_button_usage_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	unsigned int new_value;
	unsigned int old_value;
	int result;

	fc = to_rmi_function_container(dev);
	data = fc->data;
	if (sscanf(buf, "%u", &new_value) < 1) {
		printk( "FTUCH : F19 Device(%s) ERROR - button_usage_store has an invalid len\n", dev_name( dev ) );
		return -EINVAL;
	}

	if (new_value < 0 || new_value > 4) {
		printk( "FTUCH : F19 Device(%s) ERROR - button_usage_store has an invalid value(%d)\n", dev_name( dev ), new_value );
		return -EINVAL;
	}
	old_value = data->button_control.general_control.button_usage;
	data->button_control.general_control.button_usage = new_value;
	result = rmi_write_block(fc->rmi_dev, data->general_control_address,
		(u8 *)&(data->button_control.general_control),
			sizeof(union f19_0d_control_0));
	if (result < 0) {
		data->button_control.general_control.button_usage = old_value;
		printk( "FTUCH : F19 Device(%s) could not write button_usage_store to 0x%x\n", dev_name( dev ), fc->fd.control_base_addr );
		return result;
	}

	return count;

}

static ssize_t rmi_f19_interrupt_enable_button_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	int i, len, total_len = 0;
	char *current_buf = buf;
	fc = to_rmi_function_container(dev);
	data = fc->data;

	/* loop through each button map value and copy its
	 * string representation into buf */
	for (i = 0; i < data->button_count; i++) {
		int button_reg;
		int button_shift;
		int interrupt_button;

		button_reg = i / 8;
		button_shift = i % 8;
		interrupt_button =
			((data->button_control.int_enabled_button[button_reg]
				>> button_shift) & 0x01);

		/* get next button mapping value and write it to buf */
		len = snprintf(current_buf, PAGE_SIZE - total_len,
			"%u ", interrupt_button);
		/* bump up ptr to next location in buf if the
		 * snprintf was valid.  Otherwise issue an error
		 * and return. */
		if (len > 0) {
			current_buf += len;
			total_len += len;
		} else {
			printk( "FTUCH : F19 Device(%s) failed to build interrupt button buffer(%d)\n", dev_name( dev ), len );
			return snprintf(buf, PAGE_SIZE, "unknown\n");
		}
	}
	len = snprintf(current_buf, PAGE_SIZE - total_len, "\n");
	if (len > 0)
		total_len += len;
	else
		printk( "FTUCH : F19 Device(%s) failed to append carriage return\n", dev_name( dev ) );
	return total_len;

}

static ssize_t rmi_f19_interrupt_enable_button_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	int i;
	int button_count = 0;
	int retval = count;
	int button_reg = 0;
	u8 *new_value;

	fc = to_rmi_function_container(dev);
	data = fc->data;
	new_value = kzalloc(data->button_bitmask_size, GFP_KERNEL);
	if (!new_value) {
		printk( "FTUCH : F19 Device(%s) ERROR - failed to allocate button interrupt enable\n", dev_name( dev ) );
		return -ENOMEM;
	}
	for (i = 0; i < data->button_count && *buf != 0;
	     i++, buf += 2) {
		int button_shift;
		int button;
		int result;

		button_reg = i / 8;
		button_shift = i % 8;
		/* get next button mapping value and store and bump up to
		 * point to next item in buf */
		result = sscanf(buf, "%u", &button);

		if ((result != 1) || (button != 0 && button != 1)) {
			printk( "FTUCH : F19 Device(%s) ERROR - interrupt enable button for button %d is not a valid value 0x%x\n", dev_name( dev ), i, button );
			return -EINVAL;
		}

		if (button)
			new_value[button_reg] |= (1 << button_shift);

		button_count++;
	}

	/* Make sure the button count matches */
	if (button_count != data->button_count) {
		printk( "FTUCH : F19 Device(%s) ERROR - interrupt enable button count of %d doesn't match device button count of %d\n", dev_name( dev ), button_count, data->button_count );
		kfree(new_value);
		return -EINVAL;
	}

	/* write back to the control register */
	retval = rmi_write_block(fc->rmi_dev, data->int_enable_button_address,
			new_value,
			data->button_bitmask_size);
	if (retval < 0) {
		printk( "FTUCH : F19 Device(%s) could not write interrupt_enable_store to 0x%x\n", dev_name( dev ), data->int_enable_button_address );
		kfree(new_value);
		return retval;
	}
	memcpy(data->button_control.int_enabled_button,
		new_value, data->button_bitmask_size);

	kfree(new_value);

	return count;
}

static ssize_t rmi_f19_single_button_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	int i, len, total_len = 0;
	char *current_buf = buf;

	fc = to_rmi_function_container(dev);
	data = fc->data;
	/* loop through each button map value and copy its
	 * string representation into buf */
	for (i = 0; i < data->button_count; i++) {
		int button_reg;
		int button_shift;
		int single_button;

		button_reg = i / 8;
		button_shift = i % 8;
		single_button =
			((data->button_control.single_button[button_reg]
				>> button_shift) & 0x01);

		/* get next button mapping value and write it to buf */
		len = snprintf(current_buf, PAGE_SIZE - total_len,
			"%u ", single_button);
		/* bump up ptr to next location in buf if the
		 * snprintf was valid.  Otherwise issue an error
		 * and return. */
		if (len > 0) {
			current_buf += len;
			total_len += len;
		} else {
			printk( "FTUCH : F19 Device(%s) failed to build signle button buffer(%d)\n", dev_name( dev ), len );
			return snprintf(buf, PAGE_SIZE, "unknown\n");
		}
	}
	len = snprintf(current_buf, PAGE_SIZE - total_len, "\n");
	if (len > 0)
		total_len += len;
	else
		printk( "FTUCH : F19 Device(%s) failed to append carriage return\n", dev_name( dev ) );
	return total_len;

}

static ssize_t rmi_f19_single_button_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	int i;
	int button_count = 0;
	int retval = count;
	int button_reg = 0;
	u8 *new_value;

	fc = to_rmi_function_container(dev);
	data = fc->data;
	new_value = kzalloc(data->button_bitmask_size, GFP_KERNEL);
	if (!new_value) {
		printk( "FTUCH : F19 Device(%s) ERROR - failed to allocate button interrupt enable\n", dev_name( dev ) );
		return -ENOMEM;
	}
	for (i = 0; i < data->button_count && *buf != 0;
	     i++, buf += 2) {
		int button_shift;
		int button;
		int result;

		button_reg = i / 8;
		button_shift = i % 8;
		/* get next button mapping value and store and bump up to
		 * point to next item in buf */
		result = sscanf(buf, "%u", &button);

		if ((result != 1) || (button != 0 && button != 1)) {
			printk( "FTUCH : F19 Device(%s) ERROR - single button for button %d is not a valid value 0x%x\n", dev_name( dev ), i, button );
			kfree(new_value);
			return -EINVAL;
		}

		if (button) {
			new_value[button_reg] |=
				(1 << button_shift);
		}
		button_count++;
	}

	/* Make sure the button count matches */
	if (button_count != data->button_count) {
		printk( "FTUCH : F19 Device(%s) ERROR - single button count of %d doesn't match device button count of %d\n", dev_name( dev ), button_count, data->button_count );
		kfree(new_value);
		return -EINVAL;
	}
	/* write back to the control register */
	retval = rmi_write_block(fc->rmi_dev, data->single_button_address,
			new_value,
			data->button_bitmask_size);
	if (retval < 0) {
		printk( "FTUCH : F19 Device(%s) could not write interrupt_enable_store to 0x%x\n", dev_name( dev ), data->single_button_address );
		kfree(new_value);
		return -EINVAL;
	}
	memcpy(data->button_control.single_button,
		new_value, data->button_bitmask_size);

	kfree(new_value);
	return count;
}

static ssize_t rmi_f19_sensor_map_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	int i, len, total_len = 0;
	char *current_buf = buf;

	fc = to_rmi_function_container(dev);
	data = fc->data;

	for (i = 0; i < data->button_count; i++) {
		len = snprintf(current_buf, PAGE_SIZE - total_len,
			"%u ", data->button_control.sensor_map[i].
			sensor_map_button);
		/* bump up ptr to next location in buf if the
		 * snprintf was valid.  Otherwise issue an error
		 * and return. */
		if (len > 0) {
			current_buf += len;
			total_len += len;
		} else {
			printk( "FTUCH : F19 Device(%s) failed to build sensor map buffer(%d)\n", dev_name( dev ), len );
			return snprintf(buf, PAGE_SIZE, "unknown\n");
		}
	}
	len = snprintf(current_buf, PAGE_SIZE - total_len, "\n");
	if (len > 0)
		total_len += len;
	else
		printk( "FTUCH : F19 Device(%s) failed to append carriage return\n", dev_name( dev ) );
	return total_len;


}

static ssize_t rmi_f19_sensor_map_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	int i;
	int retval = count;
	int result;
	int button_count = 0;
	char sensor_map_name[MAX_BUFFER_LEN];
	u8 *new_value;
	fc = to_rmi_function_container(dev);
	data = fc->data;
	new_value = kzalloc(data->button_count, GFP_KERNEL);
	if (!new_value) {
		printk( "FTUCH : F19 Device(%s) ERROR - failed to allocate sensor map\n", dev_name( dev ) );
		return -ENOMEM;
	}

	if (data->button_query.configurable == 0) {
		printk( "FTUCH : F19 Device(%s) ERROR - sensor map is not configuralbe at run-time\n", dev_name( dev ) );
		return -EINVAL;
	}

	for (i = 0; i < data->button_count && *buf != 0; i++) {
		result = sscanf(buf, "%s ", sensor_map_name);
		if (result < 1) {
			printk( "FTUCH : F19 Device(%s) ERROR - scanf threshold button\n", dev_name( dev ) );
			kfree(new_value);
			return -EINVAL;
		}
		result = sscanf(sensor_map_name, "%u",
			(unsigned int *)&new_value[i]);
		buf += strlen(sensor_map_name)+1;

		/* Make sure the key is a valid key */
		if ((result < 1) || (new_value[i] < SENSOR_MAP_MIN ||
			new_value[i] > SENSOR_MAP_MAX)) {
			printk( "FTUCH : F19 Device(%s) ERROR - sensor map for button %d is not a valid value 0x%x\n", dev_name( dev ), i, new_value[i] );
			return -EINVAL;
		}
		button_count++;
	}

	if (button_count != data->button_count) {
		printk( "FTUCH : F19 Device(%s) ERROR - button map count of %d doesn't match device button count of %d\n", dev_name( dev ), button_count, data->button_count );
		return -EINVAL;
	}

	/* write back to the control register */
	retval = rmi_write_block(fc->rmi_dev, data->sensor_map_address,
			new_value,
			sizeof(struct f19_0d_control_3)*button_count);
	if (retval < 0) {
		printk( "FTUCH : F19 Device(%s) could not sensor_map_store to 0x%x\n", dev_name( dev ), data->sensor_map_address );
		return -EINVAL;
	}
	memcpy(data->button_control.sensor_map, new_value, data->button_count);
	kfree(new_value);
	return count;
}

static ssize_t rmi_f19_sensitivity_adjust_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;

	fc = to_rmi_function_container(dev);
	data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n", data->button_control.
		all_button_sensitivity_adj.sensitivity_adj);

}

static ssize_t rmi_f19_sensitivity_adjust_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	unsigned int old_value;
	unsigned int new_value;
	int len;
	struct f19_0d_control_5 *all_button_sensitivity_adj;
	fc = to_rmi_function_container(dev);
	data = fc->data;
	all_button_sensitivity_adj =
			 &data->button_control.all_button_sensitivity_adj;

	if (data->button_query.configurable == 0) {
		printk( "FTUCH : F19 Device(%s) ERROR - sensitivity_adjust is not configuralbe at run-time\n", dev_name( dev ) );
		return -EINVAL;
	}

	len = sscanf(buf, "%u", &new_value);
	if (len < 1 || new_value < SENSITIVITY_ADJ_MIN ||
		new_value > SENSITIVITY_ADJ_MAX)
		return -EINVAL;
	old_value = all_button_sensitivity_adj->sensitivity_adj;
	all_button_sensitivity_adj->sensitivity_adj = new_value;
	/* write back to the control register */
	len = rmi_write_block(fc->rmi_dev, data->all_button_sensitivity_address,
		(u8 *)all_button_sensitivity_adj,
			sizeof(struct f19_0d_control_5));
	if (len < 0) {
		all_button_sensitivity_adj->sensitivity_adj = old_value;
		printk( "FTUCH : F19 Device(%s) could not sensitivity_adjust_store to 0x%x\n", dev_name( dev ), data->all_button_sensitivity_address );
		return len;
	}

	return len;
}

static ssize_t rmi_f19_hysteresis_threshold_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;

	fc = to_rmi_function_container(dev);
	data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n", data->button_control.
		all_button_hysteresis_threshold.hysteresis_threshold);

}
static ssize_t rmi_f19_hysteresis_threshold_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	unsigned int new_value;
	unsigned int old_value;
	int len;
	struct f19_0d_control_6 *all_button_hysteresis_threshold;

	fc = to_rmi_function_container(dev);
	data = fc->data;
	all_button_hysteresis_threshold =
		&data->button_control.all_button_hysteresis_threshold;
	len = sscanf(buf, "%u", &new_value);
	if (new_value < HYSTERESIS_THRESHOLD_MIN ||
		new_value > HYSTERESIS_THRESHOLD_MAX) {
		printk( "FTUCH : F19 Device(%s) ERROR - hysteresis_threshold_store has an invalid value(%d)\n", dev_name( dev ), new_value );
		return -EINVAL;
	}
	old_value = all_button_hysteresis_threshold->hysteresis_threshold;
	all_button_hysteresis_threshold->hysteresis_threshold = new_value;
	/* write back to the control register */
	len = rmi_write_block(fc->rmi_dev, data->all_button_threshold_address,
		(u8 *)all_button_hysteresis_threshold,
			sizeof(struct f19_0d_control_6));
	if (len < 0) {
		all_button_hysteresis_threshold->hysteresis_threshold = old_value;
		printk( "FTUCH : F19 Device(%s) could not write all_button hysteresis threshold to 0x%x\n", dev_name( dev ), data->all_button_threshold_address );
		return -EINVAL;
	}
	return count;
}

static ssize_t rmi_f19_has_hysteresis_threshold_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;

	fc = to_rmi_function_container(dev);
	data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n",
			data->button_query.has_hysteresis_threshold);
}

static ssize_t rmi_f19_has_sensitivity_adjust_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;

	fc = to_rmi_function_container(dev);
	data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n",
			data->button_query.has_sensitivity_adjust);
}

static ssize_t rmi_f19_configurable_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;

	fc = to_rmi_function_container(dev);
	data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n",
			data->button_query.configurable);
}

static ssize_t rmi_f19_rezero_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;

	fc = to_rmi_function_container(dev);
	data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n",
			data->button_rezero);

}

static ssize_t rmi_f19_rezero_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf,
					 size_t count)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	unsigned int new_value;
	int len;

	fc = to_rmi_function_container(dev);
	data = fc->data;
	len = sscanf(buf, "%u", &new_value);
	if (new_value != 0 && new_value != 1) {
		printk( "FTUCH : F19 Device(%s) ERROR - rezero is not a valid value 0x%x\n", dev_name( dev ), new_value );
		return -EINVAL;
	}
	data->button_rezero = new_value & 1;
	len = rmi_write(fc->rmi_dev, fc->fd.command_base_addr,
		data->button_rezero);

	if (len < 0) {
		printk( "FTUCH : F19 Device(%s) could not write rezero to 0x%x\n", dev_name( dev ), fc->fd.command_base_addr );
		return -EINVAL;
	}
	return count;
}

static ssize_t rmi_f19_button_count_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct rmi_function_container *fc;
	struct f19_data *data;

	fc = to_rmi_function_container(dev);
	data = fc->data;
	return snprintf(buf, PAGE_SIZE, "%u\n",
			data->button_count);
}

static ssize_t rmi_f19_button_map_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{

	struct rmi_function_container *fc;
	struct f19_data *data;
	int i, len, total_len = 0;
	char *current_buf = buf;

	fc = to_rmi_function_container(dev);
	data = fc->data;
	/* loop through each button map value and copy its
	 * string representation into buf */
	for (i = 0; i < data->button_count; i++) {
		/* get next button mapping value and write it to buf */
		len = snprintf(current_buf, PAGE_SIZE - total_len,
			"%u ", data->button_map[i]);
		/* bump up ptr to next location in buf if the
		 * snprintf was valid.  Otherwise issue an error
		 * and return. */
		if (len > 0) {
			current_buf += len;
			total_len += len;
		} else {
			printk( "FTUCH : F19 Device(%s) failed to build button map buffer(%d)\n", dev_name( dev ), len );
			return snprintf(buf, PAGE_SIZE, "unknown\n");
		}
	}
	len = snprintf(current_buf, PAGE_SIZE - total_len, "\n");
	if (len > 0)
		total_len += len;
	else
		printk( "FTUCH : F19 Device(%s) failed to append carriage return\n", dev_name( dev ) );
	return total_len;
}

static ssize_t rmi_f19_button_map_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	struct rmi_function_container *fc;
	struct f19_data *data;
	unsigned int button;
	int i;
	int retval = count;
	int button_count = 0;
	unsigned char temp_button_map[KEY_MAX];

	fc = to_rmi_function_container(dev);
	data = fc->data;

	/* Do validation on the button map data passed in.  Store button
	 * mappings into a temp buffer and then verify button count and
	 * data prior to clearing out old button mappings and storing the
	 * new ones. */
	for (i = 0; i < data->button_count && *buf != 0;
	     i++) {
		/* get next button mapping value and store and bump up to
		 * point to next item in buf */
		sscanf(buf, "%u", &button);

		/* Make sure the key is a valid key */
		if (button > KEY_MAX) {
			printk( "FTUCH : F19 Device(%s) ERROR - button map for button %d is not a valid value 0x%x\n", dev_name( dev ), i, button );
			retval = -EINVAL;
			goto err_ret;
		}

		temp_button_map[i] = button;
		button_count++;

		/* bump up buf to point to next item to read */
		while (*buf != 0) {
			buf++;
			if (*(buf - 1) == ' ')
				break;
		}
	}

	/* Make sure the button count matches */
	if (button_count != data->button_count) {
		printk( "FTUCH : F19 Device(%s) ERROR - button map count of %d doesn't match device button count of %d\n", dev_name( dev ), button_count, data->button_count );
		retval = -EINVAL;
		goto err_ret;
	}

	/* Clear the key bits for the old button map. */
	for (i = 0; i < button_count; i++)
		clear_bit(data->button_map[i], data->input->keybit);

	/* Switch to the new map. */
	memcpy(data->button_map, temp_button_map,
	       data->button_count);

	/* Loop through the key map and set the key bit for the new mapping. */
	for (i = 0; i < button_count; i++)
		set_bit(data->button_map[i], data->input->keybit);

err_ret:
	return retval;
}

module_init(rmi_f19_module_init);
module_exit(rmi_f19_module_exit);

MODULE_AUTHOR("Vivian Ly <vly@synaptics.com>");
MODULE_DESCRIPTION("RMI F19 module");
MODULE_LICENSE("GPL");
MODULE_VERSION(RMI_DRIVER_VERSION);
