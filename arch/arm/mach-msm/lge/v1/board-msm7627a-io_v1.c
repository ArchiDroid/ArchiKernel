/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio_event.h>
#include <linux/leds.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/i2c.h>
#include <linux/input/rmi_platformdata.h>
#include <linux/input/rmi_i2c.h>
#include <linux/delay.h>
#include <linux/atmel_maxtouch.h>
#include <linux/input/ft5x06_ts.h>
#include <linux/leds-msm-tricolor.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <mach/rpc_server_handset.h>
#include <mach/pmic.h>
#include <linux/ktime.h>

#ifdef CONFIG_TOUCHSCREEN_MELFAS_MMS100S
#include <linux/platform_data/mms_ts.h>
#endif

#ifdef CONFIG_LEDS_LP5521
#include <linux/leds-lp5521.h>
#endif
#if defined (CONFIG_MACH_LGE)
#include CONFIG_LGE_BOARD_HEADER_FILE
#endif
#include "devices.h"
#include "board-msm7627a.h"
#include "devices-msm7x2xa.h"

#define ATMEL_TS_I2C_NAME "maXTouch"
#define ATMEL_X_OFFSET 13
#define ATMEL_Y_OFFSET 0

#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_RMI4_I2C) || \
	defined(CONFIG_TOUCHSCREEN_SYNAPTICS_RMI4_I2C_MODULE)

#ifndef CLEARPAD3000_ATTEN_GPIO
#define CLEARPAD3000_ATTEN_GPIO (48)
#endif

#ifndef CLEARPAD3000_RESET_GPIO
#define CLEARPAD3000_RESET_GPIO (26)
#endif

#define KP_INDEX(row, col) ((row)*ARRAY_SIZE(kp_col_gpios) + (col))

static unsigned int kp_row_gpios[] = {31, 32, 33, 34, 35};
static unsigned int kp_col_gpios[] = {36, 37, 38, 39, 40};

static const unsigned short keymap[ARRAY_SIZE(kp_col_gpios) *
					  ARRAY_SIZE(kp_row_gpios)] = {
	[KP_INDEX(0, 0)] = KEY_7,
	[KP_INDEX(0, 1)] = KEY_DOWN,
	[KP_INDEX(0, 2)] = KEY_UP,
	[KP_INDEX(0, 3)] = KEY_RIGHT,
	[KP_INDEX(0, 4)] = KEY_ENTER,

	[KP_INDEX(1, 0)] = KEY_LEFT,
	[KP_INDEX(1, 1)] = KEY_SEND,
	[KP_INDEX(1, 2)] = KEY_1,
	[KP_INDEX(1, 3)] = KEY_4,
	[KP_INDEX(1, 4)] = KEY_CLEAR,

	[KP_INDEX(2, 0)] = KEY_6,
	[KP_INDEX(2, 1)] = KEY_5,
	[KP_INDEX(2, 2)] = KEY_8,
	[KP_INDEX(2, 3)] = KEY_3,
	[KP_INDEX(2, 4)] = KEY_NUMERIC_STAR,

	[KP_INDEX(3, 0)] = KEY_9,
	[KP_INDEX(3, 1)] = KEY_NUMERIC_POUND,
	[KP_INDEX(3, 2)] = KEY_0,
	[KP_INDEX(3, 3)] = KEY_2,
	[KP_INDEX(3, 4)] = KEY_SLEEP,

	[KP_INDEX(4, 0)] = KEY_BACK,
	[KP_INDEX(4, 1)] = KEY_HOME,
	[KP_INDEX(4, 2)] = KEY_MENU,
	[KP_INDEX(4, 3)] = KEY_VOLUMEUP,
	[KP_INDEX(4, 4)] = KEY_VOLUMEDOWN,
};

/* SURF keypad platform device information */
static struct gpio_event_matrix_info kp_matrix_info = {
	.info.func	= gpio_event_matrix_func,
	.keymap		= keymap,
	.output_gpios	= kp_row_gpios,
	.input_gpios	= kp_col_gpios,
	.noutputs	= ARRAY_SIZE(kp_row_gpios),
	.ninputs	= ARRAY_SIZE(kp_col_gpios),
	.settle_time.tv64 = 40 * NSEC_PER_USEC,
	.poll_time.tv64 = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_DRIVE_INACTIVE |
			  GPIOKPF_PRINT_UNMAPPED_KEYS,
};

static struct gpio_event_info *kp_info[] = {
	&kp_matrix_info.info
};

static struct gpio_event_platform_data kp_pdata = {
	.name		= "7x27a_kp",
	.info		= kp_info,
	.info_count	= ARRAY_SIZE(kp_info)
};

static struct platform_device kp_pdev = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &kp_pdata,
	},
};

/* 8625 keypad device information */
static unsigned int kp_row_gpios_8625[] = {31};
static unsigned int kp_col_gpios_8625[] = {36, 37};

static const unsigned short keymap_8625[] = {
	KEY_VOLUMEUP,
	KEY_VOLUMEDOWN,
};

static const unsigned short keymap_8625_evt[] = {
	KEY_VOLUMEDOWN,
	KEY_VOLUMEUP,
};

static struct gpio_event_matrix_info kp_matrix_info_8625 = {
	.info.func      = gpio_event_matrix_func,
	.keymap         = keymap_8625,
	.output_gpios   = kp_row_gpios_8625,
	.input_gpios    = kp_col_gpios_8625,
	.noutputs       = ARRAY_SIZE(kp_row_gpios_8625),
	.ninputs        = ARRAY_SIZE(kp_col_gpios_8625),
	.settle_time.tv64 = 40 * NSEC_PER_USEC,
	.poll_time.tv64 = 20 * NSEC_PER_MSEC,
	.flags          = GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_DRIVE_INACTIVE |
			  GPIOKPF_PRINT_UNMAPPED_KEYS,
};

static struct gpio_event_info *kp_info_8625[] = {
	&kp_matrix_info_8625.info,
};

static struct gpio_event_platform_data kp_pdata_8625 = {
	.name           = "7x27a_kp",
	.info           = kp_info_8625,
	.info_count     = ARRAY_SIZE(kp_info_8625)
};

static struct platform_device kp_pdev_8625 = {
	.name   = GPIO_EVENT_DEV_NAME,
	.id     = -1,
	.dev    = {
		.platform_data  = &kp_pdata_8625,
	},
};

#define LED_GPIO_PDM 96

#define MXT_TS_IRQ_GPIO         48
#define MXT_TS_RESET_GPIO       26
#define MAX_VKEY_LEN		100

static ssize_t mxt_virtual_keys_register(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	char *virtual_keys = __stringify(EV_KEY) ":" __stringify(KEY_MENU) \
		":60:840:120:80" ":" __stringify(EV_KEY) \
		":" __stringify(KEY_HOME)   ":180:840:120:80" \
		":" __stringify(EV_KEY) ":" \
		__stringify(KEY_BACK) ":300:840:120:80" \
		":" __stringify(EV_KEY) ":" \
		__stringify(KEY_SEARCH)   ":420:840:120:80" "\n";

	return snprintf(buf, strnlen(virtual_keys, MAX_VKEY_LEN) + 1 , "%s",
			virtual_keys);
}

static struct kobj_attribute mxt_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.atmel_mxt_ts",
		.mode = S_IRUGO,
	},
	.show = &mxt_virtual_keys_register,
};

static struct attribute *mxt_virtual_key_properties_attrs[] = {
	&mxt_virtual_keys_attr.attr,
	NULL,
};

static struct attribute_group mxt_virtual_key_properties_attr_group = {
	.attrs = mxt_virtual_key_properties_attrs,
};

struct kobject *mxt_virtual_key_properties_kobj;

static int mxt_vkey_setup(void)
{
	int retval = 0;

	mxt_virtual_key_properties_kobj =
		kobject_create_and_add("board_properties", NULL);
	if (mxt_virtual_key_properties_kobj)
		retval = sysfs_create_group(mxt_virtual_key_properties_kobj,
				&mxt_virtual_key_properties_attr_group);
	if (!mxt_virtual_key_properties_kobj || retval)
		pr_err("failed to create mxt board_properties\n");

	return retval;
}

static const u8 mxt_config_data[] = {
	/* T6 Object */
	0, 0, 0, 0, 0, 0,
	/* T38 Object */
	16, 1, 0, 0, 0, 0, 0, 0,
	/* T7 Object */
	32, 16, 50,
	/* T8 Object */
	30, 0, 20, 20, 0, 0, 20, 0, 50, 0,
	/* T9 Object */
	3, 0, 0, 18, 11, 0, 32, 75, 3, 3,
	0, 1, 1, 0, 10, 10, 10, 10, 31, 3,
	223, 1, 11, 11, 15, 15, 151, 43, 145, 80,
	100, 15, 0, 0, 0,
	/* T15 Object */
	131, 0, 11, 11, 1, 1, 0, 45, 3, 0,
	0,
	/* T18 Object */
	0, 0,
	/* T19 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	/* T23 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	/* T25 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T40 Object */
	0, 0, 0, 0, 0,
	/* T42 Object */
	0, 0, 0, 0, 0, 0, 0, 0,
	/* T46 Object */
	0, 2, 32, 48, 0, 0, 0, 0, 0,
	/* T47 Object */
	1, 20, 60, 5, 2, 50, 40, 0, 0, 40,
	/* T48 Object */
	1, 12, 80, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 6, 6, 0, 0, 100, 4, 64,
	10, 0, 20, 5, 0, 38, 0, 20, 0, 0,
	0, 0, 0, 0, 16, 65, 3, 1, 1, 0,
	10, 10, 10, 0, 0, 15, 15, 154, 58, 145,
	80, 100, 15, 3,
};

static const u8 mxt_config_data_evt[] = {
	/* T6 Object */
	0, 0, 0, 0, 0, 0,
	/* T38 Object */
	20, 1, 0, 25, 9, 12, 0, 0,
	/* T7 Object */
	24, 12, 10,
	/* T8 Object */
	30, 0, 20, 20, 0, 0, 0, 0, 10, 192,
	/* T9 Object */
	131, 0, 0, 18, 11, 0, 16, 70, 2, 1,
	0, 2, 1, 62, 10, 10, 10, 10, 107, 3,
	223, 1, 2, 2, 20, 20, 172, 40, 139, 110,
	10, 15, 0, 0, 0,
	/* T15 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
	/* T18 Object */
	0, 0,
	/* T19 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	/* T23 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	/* T25 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T40 Object */
	0, 0, 0, 0, 0,
	/* T42 Object */
	3, 20, 45, 40, 128, 0, 0, 0,
	/* T46 Object */
	0, 2, 16, 16, 0, 0, 0, 0, 0,
	/* T47 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* T48 Object */
	1, 12, 64, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 6, 6, 0, 0, 100, 4, 64,
	10, 0, 20, 5, 0, 38, 0, 20, 0, 0,
	0, 0, 0, 0, 16, 65, 3, 1, 1, 0,
	10, 10, 10, 0, 0, 15, 15, 154, 58, 145,
	80, 100, 15, 3,
};

static struct mxt_config_info mxt_config_array[] = {
	{
		.config		= mxt_config_data,
		.config_length	= ARRAY_SIZE(mxt_config_data),
		.family_id	= 0x81,
		.variant_id	= 0x01,
		.version	= 0x10,
		.build		= 0xAA,
	},
};

static int mxt_key_codes[MXT_KEYARRAY_MAX_KEYS] = {
	[0] = KEY_HOME,
	[1] = KEY_MENU,
	[9] = KEY_BACK,
	[10] = KEY_SEARCH,
};

static struct mxt_platform_data mxt_platform_data = {
	.config_array		= mxt_config_array,
	.config_array_size	= ARRAY_SIZE(mxt_config_array),
	.panel_minx		= 0,
	.panel_maxx		= 479,
	.panel_miny		= 0,
	.panel_maxy		= 799,
	.disp_minx		= 0,
	.disp_maxx		= 479,
	.disp_miny		= 0,
	.disp_maxy		= 799,
	.irqflags		= IRQF_TRIGGER_FALLING,
	.i2c_pull_up		= true,
	.reset_gpio		= MXT_TS_RESET_GPIO,
	.irq_gpio		= MXT_TS_IRQ_GPIO,
	.key_codes		= mxt_key_codes,
};

static struct i2c_board_info mxt_device_info[] __initdata = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x4a),
		.platform_data = &mxt_platform_data,
		.irq = MSM_GPIO_TO_INT(MXT_TS_IRQ_GPIO),
	},
};

static int synaptics_touchpad_setup(void);

static struct msm_gpio clearpad3000_cfg_data[] = {
	{GPIO_CFG(CLEARPAD3000_ATTEN_GPIO, 0, GPIO_CFG_INPUT,
			GPIO_CFG_NO_PULL, GPIO_CFG_6MA), "rmi4_attn"},
	{GPIO_CFG(CLEARPAD3000_RESET_GPIO, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA), "rmi4_reset"},
};

static struct rmi_XY_pair rmi_offset = {.x = 0, .y = 0};
static struct rmi_range rmi_clipx = {.min = 48, .max = 980};
static struct rmi_range rmi_clipy = {.min = 7, .max = 1647};
static struct rmi_f11_functiondata synaptics_f11_data = {
	.swap_axes = false,
	.flipX = false,
	.flipY = false,
	.offset = &rmi_offset,
	.button_height = 113,
	.clipX = &rmi_clipx,
	.clipY = &rmi_clipy,
};

#define MAX_LEN		100

static ssize_t clearpad3000_virtual_keys_register(struct kobject *kobj,
		     struct kobj_attribute *attr, char *buf)
{
	char *virtual_keys = __stringify(EV_KEY) ":" __stringify(KEY_MENU) \
			     ":60:830:120:60" ":" __stringify(EV_KEY) \
			     ":" __stringify(KEY_HOME)   ":180:830:120:60" \
				":" __stringify(EV_KEY) ":" \
				__stringify(KEY_SEARCH) ":300:830:120:60" \
				":" __stringify(EV_KEY) ":" \
			__stringify(KEY_BACK)   ":420:830:120:60" "\n";

	return snprintf(buf, strnlen(virtual_keys, MAX_LEN) + 1 , "%s",
			virtual_keys);
}

static struct kobj_attribute clearpad3000_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.sensor00fn11",
		.mode = S_IRUGO,
	},
	.show = &clearpad3000_virtual_keys_register,
};

static struct attribute *virtual_key_properties_attrs[] = {
	&clearpad3000_virtual_keys_attr.attr,
	NULL
};

static struct attribute_group virtual_key_properties_attr_group = {
	.attrs = virtual_key_properties_attrs,
};

struct kobject *virtual_key_properties_kobj;

static struct rmi_functiondata synaptics_functiondata[] = {
	{
		.function_index = RMI_F11_INDEX,
		.data = &synaptics_f11_data,
	},
};

static struct rmi_functiondata_list synaptics_perfunctiondata = {
	.count = ARRAY_SIZE(synaptics_functiondata),
	.functiondata = synaptics_functiondata,
};

static struct rmi_sensordata synaptics_sensordata = {
	.perfunctiondata = &synaptics_perfunctiondata,
	.rmi_sensor_setup	= synaptics_touchpad_setup,
};

static struct rmi_i2c_platformdata synaptics_platformdata = {
	.i2c_address = 0x2c,
	.irq_type = IORESOURCE_IRQ_LOWLEVEL,
	.sensordata = &synaptics_sensordata,
};

static struct i2c_board_info synaptic_i2c_clearpad3k[] = {
	{
	I2C_BOARD_INFO("rmi4_ts", 0x2c),
	.platform_data = &synaptics_platformdata,
	},
};

static int synaptics_touchpad_setup(void)
{
	int retval = 0;

	virtual_key_properties_kobj =
		kobject_create_and_add("board_properties", NULL);
	if (virtual_key_properties_kobj)
		retval = sysfs_create_group(virtual_key_properties_kobj,
				&virtual_key_properties_attr_group);
	if (!virtual_key_properties_kobj || retval)
		pr_err("failed to create ft5202 board_properties\n");

	retval = msm_gpios_request_enable(clearpad3000_cfg_data,
		    sizeof(clearpad3000_cfg_data)/sizeof(struct msm_gpio));
	if (retval) {
		pr_err("%s:Failed to obtain touchpad GPIO %d. Code: %d.",
				__func__, CLEARPAD3000_ATTEN_GPIO, retval);
		retval = 0; /* ignore the err */
	}
	synaptics_platformdata.irq = gpio_to_irq(CLEARPAD3000_ATTEN_GPIO);

	gpio_set_value(CLEARPAD3000_RESET_GPIO, 0);
	usleep(10000);
	gpio_set_value(CLEARPAD3000_RESET_GPIO, 1);
	usleep(50000);

	return retval;
}
#endif

/* handset device */
static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 500, /* 0 will disable end key */
};

static struct platform_device hs_pdev = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};
 
#if defined(CONFIG_MACH_MSM7X25A_V1)
static unsigned int keypad_row_gpios[] = {36, 37};
static unsigned int keypad_col_gpios[] = {33};

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(keypad_row_gpios) + (row))

static const unsigned short keypad_keymap_v1[] = {
	[KEYMAP_INDEX(0, 0)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(0, 1)] = KEY_VOLUMEDOWN,
};
#else
static unsigned int keypad_row_gpios[] = {36, 37, 38};
static unsigned int keypad_col_gpios[] = {33, 32};

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(keypad_row_gpios) + (row))

static const unsigned short keypad_keymap_v1[] = {
	[KEYMAP_INDEX(0, 0)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(0, 1)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(1, 2)] = KEY_HOMEPAGE,
};
#endif

int v1_matrix_info_wrapper(struct gpio_event_input_devs *input_dev,
			   struct gpio_event_info *info, void **data, int func)
{
	int ret;
	int i;
	if (func == GPIO_EVENT_FUNC_RESUME) {
		for(i = 0; i < ARRAY_SIZE(keypad_row_gpios); i++)  {
			gpio_tlmm_config(GPIO_CFG(keypad_row_gpios[i], 0, GPIO_CFG_INPUT,
				 GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	        }
	}
	
	ret = gpio_event_matrix_func(input_dev, info, data, func);
	return ret ;
}

static int v1_gpio_matrix_power(const struct gpio_event_platform_data *pdata, bool on)
{
	/* this is dummy function
	 * to make gpio_event driver register suspend function
	 * 2010-01-29, cleaneye.kim@lge.com
	 * copy from ALOHA code
	 * 2010-04-22 younchan.kim@lge.com
	 */
	return 0;
}

static struct gpio_event_matrix_info v1_keypad_matrix_info = {
	.info.func	= v1_matrix_info_wrapper,
	.keymap		= keypad_keymap_v1,
	.output_gpios	= keypad_col_gpios,
	.input_gpios	= keypad_row_gpios,
	.noutputs	= ARRAY_SIZE(keypad_col_gpios),
	.ninputs	= ARRAY_SIZE(keypad_row_gpios),	
/*LGE_CHANGE_S : seven.kim@lge.com kernel3.0 proting
 * gpio_event_matrix_info structure member was changed, ktime_t -> struct timespec */	
	.settle_time.tv64 = 40 * NSEC_PER_USEC,
	.poll_time.tv64 = 20 * NSEC_PER_MSEC,
/*LGE_CHANGE_E : seven.kim@lge.com kernel3.0 proting*/
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_DRIVE_INACTIVE
};

static struct gpio_event_info *v1_keypad_info[] = {
	&v1_keypad_matrix_info.info
};

static struct gpio_event_platform_data v1_keypad_data = {
	.name		= "v1_keypad",
	.info		= v1_keypad_info,
	.info_count	= ARRAY_SIZE(v1_keypad_info),
	.power          = v1_gpio_matrix_power,
};

struct platform_device keypad_device_v1 = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &v1_keypad_data,
	},
};

/* input platform device */
static struct platform_device *v1_input_devices[] __initdata = {
	&hs_pdev,
	&keypad_device_v1,
};


/* LGE_CHANGE_S [seven.kim@lge.com] 20110922 New Bosch compass+accel Sensor Porting*/ 
#if 1//defined (CONFIG_SENSORS_BMM050) || defined (CONFIG_SENSORS_BMA250) //jinseok.choi 2013-01-31 - v1 doesn't have compass
static struct gpio_i2c_pin accel_i2c_pin[] = {
	[0] = {
		.sda_pin	= SENSOR_GPIO_I2C_SDA,
		.scl_pin	= SENSOR_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ACCEL_GPIO_INT,
	},
};

#if 0 //jinseok.choi 2013-01-31 - v1 doesn't have compass
static struct gpio_i2c_pin ecom_i2c_pin[] = {
	[0] = {
		.sda_pin	= SENSOR_GPIO_I2C_SDA,
		.scl_pin	= SENSOR_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ECOM_GPIO_INT,
	},
};
#endif

static struct i2c_gpio_platform_data sensor_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device sensor_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &sensor_i2c_pdata,
};

static struct i2c_board_info sensor_i2c_bdinfo[] = {
	[0] = {
#if defined (CONFIG_SENSORS_BMA2X2)
/*#LGE_CHANGE : 2012-10-24 Sanghun,Lee(eee3114.@lge.com) sensor change from bmc150 to bmc050
*/
		I2C_BOARD_INFO("bma2x2", ACCEL_I2C_ADDRESS),
		.type = "bma2x2",
#else
		I2C_BOARD_INFO("bma250", ACCEL_I2C_ADDRESS),
		.type = "bma250",
#endif		
	},
#if 0 //jinseok.choi 2013-01-31 - v1 doesn't have compass
	[1] = {
		I2C_BOARD_INFO("bmm050", ECOM_I2C_ADDRESS),
		.type = "bmm050",
	},
#endif
};

#if defined (CONFIG_MACH_MSM7X25A_V1)
static void __init v1_init_i2c_sensor(int bus_num)
{
	sensor_i2c_device.id = bus_num;
	lge_init_gpio_i2c_pin(&sensor_i2c_pdata, accel_i2c_pin[0], &sensor_i2c_bdinfo[0]);
	//jinseok.choi 2013-01-31 - v1 doesn't have compass
	//lge_init_gpio_i2c_pin(&sensor_i2c_pdata, ecom_i2c_pin[0], &sensor_i2c_bdinfo[1]);
	i2c_register_board_info(bus_num, sensor_i2c_bdinfo, ARRAY_SIZE(sensor_i2c_bdinfo));
	platform_device_register(&sensor_i2c_device);
}
#else
static void __init v1_init_i2c_sensor(int bus_num)
{
	sensor_i2c_device.id = bus_num;

	lge_init_gpio_i2c_pin(&sensor_i2c_pdata, accel_i2c_pin[0], &sensor_i2c_bdinfo[0]);
	lge_init_gpio_i2c_pin(&sensor_i2c_pdata, ecom_i2c_pin[0], &sensor_i2c_bdinfo[1]);

	i2c_register_board_info(bus_num, sensor_i2c_bdinfo, ARRAY_SIZE(sensor_i2c_bdinfo));

	platform_device_register(&sensor_i2c_device);
}
#endif
#endif /* LGE_CHANGE_E [seven.kim@lge.com 20110922 New Bosch compass+accel Sensor Porting*/ 

#if defined (CONFIG_SENSOR_APDS9190)
extern int rt9396_ldo_enable(struct device *dev, unsigned num, unsigned enable);
#endif

/* proximity */
/* [LGE_BSP_S] jihyun2.jang@lge.com, 20130130 - RT8966A Proximity Sensor LDO control */
#ifdef CONFIG_MACH_MSM7X25A_V1
/* [LGE_BSP_S] jihyun2.jang@lge.com, 20130212 - RT8966A miniABB regulator : PROXIMITY SENSOR LDO */
#ifdef CONFIG_MINIABB_REGULATOR
static struct regulator *regulator_prox;
//static struct regulator *regulator_accl;
#else /* !CONFIG_MINIABB_REGULATOR */
extern int lge_rt8966a_ldo_control( int ldo_id, int onoff );
#endif /* CONFIG_MINIABB_REGULATOR */
/* [LGE_BSP_E] jihyun2.jang@lge.com, 20130212 - RT8966A miniABB regulator : PROXIMITY SENSOR LDO */
#endif /* CONFIG_MACH_MSM7X25A_V1 */
/* [LGE_BSP_E] jihyun2.jang@lge.com, 20130130 - RT8966A Proximity Sensor LDO control */

static int prox_power_set(struct device *dev, unsigned char onoff)
{
/* [LGE_BSP_S] jihyun2.jang@lge.com, 20130130 - RT8966A Proximity Sensor LDO control */
#ifdef CONFIG_MACH_MSM7X25A_V1
/* [LGE_BSP_S] jihyun2.jang@lge.com, 20130212 - RT8966A miniABB regulator : PROXIMITY SENSOR LDO */
#ifdef CONFIG_MINIABB_REGULATOR
	int rc = -ENODEV;

	pr_info("### %s: initialize regulator for prox. sensor ###\n", __func__);

	regulator_prox = regulator_get( dev, "prox" );
	if( regulator_prox == NULL ) {
		pr_err("### %s: could not get regulators: prox ###\n", __func__);
		goto err_prox_power_set;
	}
	else {
		rc = regulator_set_voltage( regulator_prox, 3000000, 3000000 );
		if( rc ) {
			pr_err("### %s: could not set prox voltages: %d ###\n", __func__, rc);
			goto err_prox_power_set;
		}
	}

	pr_info("### %s: onoff=%d ###\n", __func__, onoff);

	if( regulator_prox != NULL )
	{
		int is_on = regulator_is_enabled(regulator_prox);

		if( onoff == is_on )
			return 0;

		if( onoff ) {
			rc = regulator_enable( regulator_prox );
			if( rc < 0 )
			{
				pr_err("### %s: regulator_enable(regulator_prox) failed ###\n", __func__);
				goto err_prox_power_set;
			}
		}
		else {
			rc = regulator_disable( regulator_prox );
			if( rc < 0 )
			{
				pr_err("### %s: regulator_disble(regulator_prox) failed ###\n", __func__);
				goto err_prox_power_set;
			}
		}
	}
err_prox_power_set:
	if(regulator_prox)
		regulator_put(regulator_prox);
	regulator_prox = NULL;

	return rc;
#else /* !CONFIG_MINIABB_REGULATOR */
/* [LGE_BSP_S] jihyun2.jang@lge.com, 20130130 - RT8966A Proximity Sensor LDO control */
	int ret = 0;

	ret = lge_rt8966a_ldo_control( 1, onoff );
	printk( KERN_INFO "%s, *********** Proximity LDO Control onoff=%d, error=%d ***********\n", __func__, onoff, ret );
	
	return ret;
#endif /* CONFIG_MINIABB_REGULATOR */
/* [LGE_BSP_E] jihyun2.jang@lge.com, 20130212 - RT8966A miniABB regulator : PROXIMITY SENSOR LDO */
#else /* !CONFIG_MACH_MSM7X25A_V1 */
#if defined (CONFIG_SENSOR_APDS9190)
	if(onoff == 1)
	{
		rt9396_ldo_enable(NULL,1,1);
		printk(KERN_INFO "%s,***********Proximity probe enter when power on*****\n",__func__);
	} else
	{
		rt9396_ldo_enable(NULL,1,0);
		printk(KERN_INFO "%s,***********Proximity probe enter when power on*****\n",__func__);
	}

	return 0;
#endif
#endif /* CONFIG_MACH_MSM7X25A_V1 */
/* [LGE_BSP_E] jihyun2.jang@lge.com, 20130130 - RT8966A Proximity Sensor LDO control */
}

#if defined (CONFIG_SENSOR_APDS9190)

static struct proximity_platform_data proxi_pdata = {
	.irq_num	= PROXI_GPIO_DOUT,
	.power		= prox_power_set,
	.methods		= 1,
	.operation_mode		= 0,
};

static struct i2c_board_info prox_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("proximity_apds9190", PROXI_I2C_ADDRESS),
		.type = "proximity_apds9190",
		.platform_data = &proxi_pdata,
	},
};
//LGE_CHANGE : 2013-02-04 jinseok.choi@lge.com V1 uses Rohm RPR0400 for Proximity Sensor
#elif defined(CONFIG_SENSOR_RPR0400)

static struct proximity_platform_data proxi_pdata = {
	.irq_num	= PROXI_GPIO_DOUT,
	.power		= prox_power_set,
	.methods		= 1,
	.operation_mode		= 0,
};

static struct i2c_board_info prox_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("rpr400", PROXI_I2C_ADDRESS),
		.type = "rpr400",
		.platform_data = &proxi_pdata,
	},
};


#else
static struct proximity_platform_data proxi_pdata = {
	.irq_num = PROXI_GPIO_DOUT,
	.power = prox_power_set,
	.methods = 0,
	.operation_mode = 0,
	.debounce = 0,
	.cycle = 2,
};

static struct i2c_board_info prox_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("proximity_gp2ap", PROXI_I2C_ADDRESS),
		.type = "proximity_gp2ap",
		.platform_data = &proxi_pdata,
	},
};
#endif

static struct gpio_i2c_pin proxi_i2c_pin[] = {
	[0] = {
		.sda_pin	= PROXI_GPIO_I2C_SDA,
		.scl_pin	= PROXI_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= PROXI_GPIO_DOUT,
	},
};

static struct i2c_gpio_platform_data proxi_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device proxi_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &proxi_i2c_pdata,
};


#if defined (CONFIG_SENSOR_APDS9190)
static void __init v1_init_i2c_prox(int bus_num)
{
	proxi_i2c_device.id = bus_num;
	lge_init_gpio_i2c_pin(&proxi_i2c_pdata, proxi_i2c_pin[0], &prox_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &prox_i2c_bdinfo[0], 1);
	platform_device_register(&proxi_i2c_device);
}
//LGE_CHANGE : 2013-02-04 jinseok.choi@lge.com V1 uses Rohm RPR0400 for Proximity Sensor
#elif defined(CONFIG_SENSOR_RPR0400)
static void __init v1_init_i2c_prox(int bus_num)
{
	proxi_i2c_device.id = bus_num;
	lge_init_gpio_i2c_pin(&proxi_i2c_pdata, proxi_i2c_pin[0], &prox_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &prox_i2c_bdinfo[0], 1);
	platform_device_register(&proxi_i2c_device);
}


#else
static void __init v1_init_i2c_prox(int bus_num)
{
	proxi_i2c_device.id = bus_num;

	lge_init_gpio_i2c_pin(
		&proxi_i2c_pdata, proxi_i2c_pin[0], &prox_i2c_bdinfo[0]);

	i2c_register_board_info(bus_num, &prox_i2c_bdinfo[0], 1);
	platform_device_register(&proxi_i2c_device);
}
#endif
/*[LGE_BSP_S][yunmo.yang@lge.com] LP5521 RGB Driver*/
#ifdef CONFIG_LEDS_LP5521

static struct lp5521_led_config lp5521_led_config[] = {
	{
		.name = "R",
		.chan_nr	= 0,
		//.led_current	= 170,
		//.max_current	= 170,
		.led_current	= 150,
		.max_current	= 150,
	},
	{
		.name = "G",
		.chan_nr	= 1,
		//.led_current	= 200,
		//.max_current	= 200,
		.led_current	= 70,
		.max_current	= 70,
	},
	{
		.name = "B",
		.chan_nr	= 2,
		//.led_current	= 130,
		//.max_current	= 130,
		.led_current	= 40,
		.max_current	= 40,
	},
};


//[pattern_id : 1, PowerOn_Animation]
//static u8 mode1_red[] = {0x40, 0x00, 0x08, 0x7e, 0x08, 0x7f, 0x08, 0xff, 0x08, 0xfe};
//static u8 mode1_green[] = {0x40, 0x00, 0x08, 0x7e, 0x08, 0x7f, 0x08, 0xff, 0x08, 0xfe};
//static u8 mode1_blue[] = {0x40, 0x00, 0x08, 0x7e, 0x08, 0x7f, 0x08, 0xff, 0x08, 0xfe};
//[Tuning 20121119]
static u8 mode1_red[] = {0xE0, 0x0C, 0x40, 0x00, 0x0C, 0x2F, 0x06, 0x28, 0x05, 0x2D, 0x06, 0x2A, 0x06, 0x25, 0x03, 0xDC, 0x02, 0xFA, 0x48, 0x00, 0x03, 0x54, 0x44, 0x01, 0x23, 0x06, 0x31, 0x84, 0x06, 0xA8, 0x0C, 0xAF};
static u8 mode1_green[] = {0xE0, 0x80, 0x40, 0x00, 0x52, 0x00, 0x0B, 0x15, 0x05, 0x2D, 0x03, 0x48, 0x03, 0x4B, 0x09, 0x1B, 0x02, 0x63, 0x19, 0x89, 0x03, 0xCA, 0x04, 0xC1, 0x05, 0xB2, 0x06, 0xA6, 0x12, 0x8D, 0x52, 0x00};
static u8 mode1_blue[] = {0xE0, 0x80, 0x40, 0x00, 0x12, 0xFE, 0x40, 0xC0, 0x0A, 0x18, 0x06, 0xA6, 0x06, 0xAA, 0x03, 0xCF, 0x04, 0xB6, 0x52, 0x00};

//[pattern_id : 2, Not used, LCDOn]
static u8 mode2_red[]={0x40, 0xff, 0x4d, 0x00, 0x0a, 0xff, 0x0a, 0xfe, 0xc0, 0x00};
static u8 mode2_green[]={0x40, 0xff, 0x4d, 0x00, 0x0a, 0xff, 0x0a, 0xfe, 0xc0, 0x00};
static u8 mode2_blue[]={0x40, 0xff, 0x4d, 0x00, 0x0a, 0xff, 0x0a, 0xfe, 0xc0, 0x00};

//[pattern_id : 3, Charging0_99]
//static u8 mode3_red[] = {0x40, 0x19, 0x27, 0x19, 0x0c, 0x65, 0x0c, 0x65, 0x0c, 0xe5, 0x0c, 0xe5, 0x29, 0x98, 0x5a, 0x00};
//[Tuning 20121119]
static u8 mode3_red[] = {0x40, 0x0D, 0x44, 0x0C, 0x24, 0x32, 0x24, 0x32, 0x66, 0x00, 0x24, 0xB2, 0x24, 0xB2, 0x44, 0x8C};

//[pattern_id : 4, Charging100]
//static u8 mode4_green[]={0x40, 0xff};
//[Tuning 20121119]
static u8 mode4_green[]={0x40, 0x80};

//[pattern_id : 5, Not used, Charging16_99]
static u8 mode5_red[]={0x40, 0x19, 0x27, 0x19, 0xe0, 0x04, 0x0c, 0x65, 0xe0, 0x04, 0x0c, 0x65, 0xe0, 0x04, 0x0c, 0xe5, 0xe0, 0x04, 0x0c, 0xe5, 0xe0, 0x04, 0x29, 0x98, 0xe0, 0x04, 0x5a, 0x00};
static u8 mode5_green[]={0x40, 0x0c, 0x43, 0x0b, 0xe0, 0x80, 0x19, 0x30, 0xe0, 0x80, 0x19, 0x30, 0xe0, 0x80, 0x19, 0xb0, 0xe0, 0x80, 0x19, 0xb0, 0xe0, 0x80, 0x43, 0x8b, 0xe0, 0x80, 0x5a, 0x00};


//[pattern_id : 6, PowerOff]
//static u8 mode6_red[] = {0x08, 0x7e, 0x08, 0x7f, 0x10, 0xff, 0x10, 0xFe};
//static u8 mode6_green[] = {0x08, 0x7e, 0x08, 0x7f, 0x10, 0xff, 0x10, 0xFe};
//static u8 mode6_blue[] = {0x08, 0x7e, 0x08, 0x7f, 0x10, 0xff, 0x10, 0xFe};
//[Tuning 20121119]
static u8 mode6_red[] = {0xE0, 0x0C, 0x40, 0x00, 0x0C, 0x2F, 0x06, 0x28, 0x05, 0x2D, 0x06, 0x2A, 0x06, 0x25, 0x03, 0xDC, 0x02, 0xFA, 0x48, 0x00, 0x03, 0x54, 0x44, 0x01, 0x23, 0x06, 0x31, 0x84, 0x06, 0xA8, 0x0C, 0xAF};
static u8 mode6_green[] = {0xE0, 0x80, 0x40, 0x00, 0x52, 0x00, 0x0B, 0x15, 0x05, 0x2D, 0x03, 0x48, 0x03, 0x4B, 0x09, 0x1B, 0x02, 0x63, 0x19, 0x89, 0x03, 0xCA, 0x04, 0xC1, 0x05, 0xB2, 0x06, 0xA6, 0x12, 0x8D, 0x52, 0x00};
static u8 mode6_blue[] = {0xE0, 0x80, 0x40, 0x00, 0x12, 0xFE, 0x40, 0xC0, 0x0A, 0x18, 0x06, 0xA6, 0x06, 0xAA, 0x03, 0xCF, 0x04, 0xB6, 0x52, 0x00,};


//[pattern_id : 7, MissedNoti]
//static u8 mode7_green[]={0x40, 0xff, 0x02, 0xff, 0x02, 0xfe, 0x48, 0x00, 0x40, 0xff, 0x02, 0xff, 0x02, 0xfe, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x52, 0x00};
//static u8 mode7_blue[]={0x40, 0xff, 0x02, 0xff, 0x02, 0xfe, 0x48, 0x00, 0x40, 0xff, 0x02, 0xff, 0x02, 0xfe, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x52, 0x00};
//[Tuning 20121119]
static u8 mode7_red[]={0x40, 0x00, 0x10, 0xFE, 0x40, 0x5D, 0xE2, 0x00, 0x07, 0xAD, 0xE2, 0x00, 0x07, 0xAE, 0xE2, 0x00, 0x48, 0x00, 0x40, 0x5D, 0xE2, 0x00, 0x07, 0xAD, 0xE2, 0x00, 0x07, 0xAE, 0xE2, 0x00, 0x25, 0xFE,};
static u8 mode7_green[]={0x40, 0x00, 0x10, 0xFE, 0x40, 0xCD, 0xE2, 0x00, 0x03, 0xE6, 0xE2, 0x00, 0x03, 0xE5, 0xE2, 0x00, 0x48, 0x00, 0x40, 0xCD, 0xE2, 0x00, 0x03, 0xE6, 0xE2, 0x00, 0x03, 0xE5, 0xE2, 0x00, 0x25, 0xFE,};
static u8 mode7_blue[]={0x40, 0x00, 0x10, 0xFE, 0x40, 0xE6, 0xE0, 0x06, 0x03, 0xF2, 0xE0, 0x06, 0x03, 0xF2, 0xE0, 0x06, 0x48, 0x00, 0x40, 0xE6, 0xE0, 0x06, 0x03, 0xF2, 0xE0, 0x06, 0x03, 0xF2, 0xE0, 0x06, 0x25, 0xFE,};

//[pattern_id : 14, MissedNoti(favorite)]
static u8 mode8_red[]={0x40, 0x00, 0x10, 0xFE, 0x40, 0xE6, 0xE2, 0x00, 0x03, 0xF2, 0xE2, 0x00, 0x03, 0xF2, 0xE2, 0x00, 0x48, 0x00, 0x40, 0xE6, 0xE2, 0x00, 0x03, 0xF2, 0xE2, 0x00, 0x03, 0xF2, 0xE2, 0x00, 0x25, 0xFE,};
static u8 mode8_green[]={0x40, 0x00, 0x10, 0xFE, 0x40, 0x66, 0x4F, 0x00, 0x0B, 0xA8, 0xE0, 0x80, 0x0B, 0xA8, 0xE0, 0x80, 0x40, 0x66, 0x4F, 0x00, 0x09, 0xB2, 0xE0, 0x80, 0x09, 0xB2, 0xE0, 0x80, 0x1A, 0xFE,};
static u8 mode8_blue[]={0x40, 0x00, 0x10, 0xFE, 0x40, 0x73, 0x4F, 0x00, 0x08, 0xBC, 0xE0, 0x80, 0x0F, 0x9E, 0xE0, 0x80, 0x40, 0x73, 0x4F, 0x00, 0x05, 0xD5, 0xE0, 0x80, 0x10, 0x9C, 0xE0, 0x80, 0x1A, 0xFE,};

static struct lp5521_led_pattern board_led_patterns[] = {
	{
		.r = mode1_red,
		.g = mode1_green,
		.b = mode1_blue,
		.size_r = ARRAY_SIZE(mode1_red),
		.size_g = ARRAY_SIZE(mode1_green),
		.size_b = ARRAY_SIZE(mode1_blue),
	},
	{
		.r = mode2_red,
		.g = mode2_green,
		.b = mode2_blue,
		.size_r = ARRAY_SIZE(mode2_red),
		.size_g = ARRAY_SIZE(mode2_green),
		.size_b = ARRAY_SIZE(mode2_blue),
		},
	{
		.r = mode3_red,
		/*.g = mode3_green,*/
		/*.b = mode3_blue,*/
		.size_r = ARRAY_SIZE(mode3_red),
		/*.size_g = ARRAY_SIZE(mode3_green),*/
		/*.size_b = ARRAY_SIZE(mode3_blue),*/
	},
	{
//		.r = mode4_red,
		.g = mode4_green,
//		.b = mode4_blue,
//		.size_r = ARRAY_SIZE(mode4_red),
		.size_g = ARRAY_SIZE(mode4_green),
//		.size_b = ARRAY_SIZE(mode4_blue),
	},
	{
		.r = mode5_red,
		.g = mode5_green,
//		.b = mode5_blue,
		.size_r = ARRAY_SIZE(mode5_red),
		.size_g = ARRAY_SIZE(mode5_green),
//		.size_b = ARRAY_SIZE(mode5_blue),
	},
	{
		.r = mode6_red,
		.g = mode6_green,
		.b = mode6_blue,
		.size_r = ARRAY_SIZE(mode6_red),
		.size_g = ARRAY_SIZE(mode6_green),
		.size_b = ARRAY_SIZE(mode6_blue),
	},
	{
		.r = mode7_red,
		.g = mode7_green,
		.b = mode7_blue,
		.size_r = ARRAY_SIZE(mode7_red),
		.size_g = ARRAY_SIZE(mode7_green),
		.size_b = ARRAY_SIZE(mode7_blue),
	},
	{
		.r = mode8_red,
		.g = mode8_green,
		.b = mode8_blue,
		.size_r = ARRAY_SIZE(mode8_red),
		.size_g = ARRAY_SIZE(mode8_green),
		.size_b = ARRAY_SIZE(mode8_blue),
	},
};


#define LP5521_ENABLE PM8921_GPIO_PM_TO_SYS(21)

static struct gpio_i2c_pin rgb_i2c_pin[] = {
	[0] = {
		.sda_pin	= RGB_GPIO_I2C_SDA,
		.scl_pin	= RGB_GPIO_I2C_SCL,
		.reset_pin	= 0,
	},
};

static int lp5521_setup(void)       
{
       
       int rc = 0;

       printk("lp5521_enable\n\n");
       rc = gpio_request(RGB_GPIO_RGB_EN, "lp5521_led");

       if(rc){
              printk("lp5521_request failed\n");
              return rc;
       }

	rc = gpio_tlmm_config(GPIO_CFG(RGB_GPIO_RGB_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	
       if(rc){
              printk("lp5521_config failed\n");
              return rc;
       }
       return rc;
}

static void lp5521_enable(bool state)
{
       if(state){
              gpio_set_value(RGB_GPIO_RGB_EN, 1);
              printk(KERN_INFO"lp5521_enable_set\n");
       }
       else{
              gpio_set_value(RGB_GPIO_RGB_EN, 0);
              printk(KERN_INFO"lp5521_disable_set\n");
       }
       
       return;
}
      
#define LP5521_CONFIGS	(LP5521_PWM_HF | LP5521_PWRSAVE_EN | \
			LP5521_CP_MODE_AUTO | \
			LP5521_CLK_SRC_EXT)

static struct lp5521_platform_data lp5521_pdata = {
	.led_config = lp5521_led_config,
	.num_channels = ARRAY_SIZE(lp5521_led_config),
	.clock_mode = LP5521_CLOCK_EXT,
	.update_config = LP5521_CONFIGS,
	.patterns = board_led_patterns,
	.num_patterns = ARRAY_SIZE(board_led_patterns),
       .setup_resources = lp5521_setup,
       .enable = lp5521_enable
};

static struct i2c_board_info lp5521_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("lp5521", 0x32),
		.platform_data = &lp5521_pdata,
	},
};
static struct i2c_gpio_platform_data rgb_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 1,
};

static struct platform_device rgb_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &rgb_i2c_pdata,
};

static void __init lp5521_init_i2c_rgb(int bus_num)
{
	int rc=0;
	
	rgb_i2c_device.id = bus_num;

	lge_init_gpio_i2c_pin_pullup(&rgb_i2c_pdata, rgb_i2c_pin[0], &lp5521_board_info[0]);

	i2c_register_board_info(bus_num, lp5521_board_info, ARRAY_SIZE(lp5521_board_info));

	platform_device_register(&rgb_i2c_device);

	rc = gpio_tlmm_config(GPIO_CFG(RGB_GPIO_RGB_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	if (rc) {
			pr_err("[LP5521] %s: Error requesting GPIO gpio_tlmm_config, ret %d\n", __func__, rc);
		} else {
		    pr_err ("[LP5521] %s: success gpio_tlmm_config, ret %d\n", __func__, rc);
		}
}


#endif /*LP5521*/

/*[LGE_BSP_E][yunmo.yang@lge.com] LP5521 RGB Driver*/

#if defined(CONFIG_TOUCHSCREEN_MELFAS_MMS128S) || defined(CONFIG_TOUCHSCREEN_MELFAS_MMS128S_V1)
static struct gpio_i2c_pin ts_i2c_pin[] = {
	[0] = {
		.sda_pin	= TS_GPIO_I2C_SDA,
		.scl_pin	= TS_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= TS_GPIO_IRQ,
	},
};

static struct i2c_gpio_platform_data ts_i2c_pdata = {
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay			= 1,
};

static struct platform_device ts_i2c_device = {
	.name	= "i2c-gpio",
	.dev.platform_data = &ts_i2c_pdata,
};


static struct regulator *regulator_ts;
static char is_touch_Initialized = 0;
int ts_set_vreg(unsigned char onoff)
{
	int rc;
	
#if defined(CONFIG_MACH_MSM7X25A_V1) 
	//if (is_touch_Initialized == 0) {
	if (1) {
		regulator_ts = regulator_get(NULL, "rfrx1");
		if (regulator_ts == NULL)
				pr_err("%s: regulator_get(regulator_ts) failed\n",__func__);
			
		rc = regulator_set_voltage(regulator_ts, 3000000, 3000000);
		if (rc < 0)
				pr_err("%s: regulator_set_voltage(regulator_ts) failed\n", __func__);
		
		is_touch_Initialized = 1;
	}

	if (onoff) {
		rc = regulator_enable(regulator_ts);
		if (rc < 0)
			pr_err("%s: regulator_enable(regulator_ts) failed\n", __func__);

	} else {
		rc = regulator_disable(regulator_ts);
		if (rc < 0)
				pr_err("%s: regulator_disble(regulator_ts) failed\n", __func__);
	}

#endif  

	return rc;
}

static struct touch_platform_data ts_pdata = {
	.ts_x_min = TS_X_MIN,
	.ts_x_max = TS_X_MAX,
	.ts_y_min = TS_Y_MIN,
	.ts_y_max = TS_Y_MAX,
	.power 	  = ts_set_vreg,
	.irq 	  = TS_GPIO_IRQ,
	.scl      = TS_GPIO_I2C_SCL,
	.sda      = TS_GPIO_I2C_SDA,
};

static struct i2c_board_info ts_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("touch_mcs8000", TS_I2C_SLAVE_ADDR),
		.type = "touch_mcs8000",
/* LGE_CHANGE_S : RECENT_APPS_KEY (Bell Operator in Canada) */ 
		.platform_data = &ts_pdata,
	},
};

/* this routine should be checked for nessarry */
static int init_gpio_i2c_pin_touch(
	struct i2c_gpio_platform_data *i2c_adap_pdata,
	struct gpio_i2c_pin gpio_i2c_pin,
	struct i2c_board_info *i2c_board_info_data)
{
	i2c_adap_pdata->sda_pin = gpio_i2c_pin.sda_pin;
	i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;

	gpio_request(TS_GPIO_I2C_SDA, "Melfas_I2C_SDA");
	gpio_request(TS_GPIO_I2C_SCL, "Melfas_I2C_SCL");
	gpio_request(TS_GPIO_IRQ, "Melfas_I2C_INT");

	gpio_tlmm_config(
		GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(
		GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(gpio_i2c_pin.sda_pin, 1);
	gpio_set_value(gpio_i2c_pin.scl_pin, 1);

	if (gpio_i2c_pin.reset_pin) {
		gpio_tlmm_config(
			GPIO_CFG(gpio_i2c_pin.reset_pin, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.reset_pin, 1);
	}

	if (gpio_i2c_pin.irq_pin) {
		gpio_tlmm_config(
			GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		i2c_board_info_data->irq =
			MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
	}

	gpio_free(TS_GPIO_I2C_SDA);
	gpio_free(TS_GPIO_I2C_SCL);
	gpio_free(TS_GPIO_IRQ);

	return 0;
}

static void __init v1_init_i2c_touch(int bus_num)
{
	ts_i2c_device.id = bus_num;

	init_gpio_i2c_pin_touch(
		&ts_i2c_pdata, ts_i2c_pin[0], &ts_i2c_bdinfo[0]);
	i2c_register_board_info(
		bus_num, &ts_i2c_bdinfo[0], 1);
	platform_device_register(&ts_i2c_device);
}

//jinseok.choi 2013-02-14 mms100s_ts driver for V1
#elif defined(CONFIG_TOUCHSCREEN_MELFAS_MMS100S)
#if defined(CONFIG_MACH_MSM7X25A_V1)
int ts_set_power(struct device *dev, int on)
{
	static struct regulator *power = NULL;
	int rc = -ENODEV;

	if(!power) {
		power = regulator_get(dev, "rfrx1");
	}
	if(!power) {
		dev_err(dev, "regulator_get failed\n");
		goto err_ts_set_power;
	} else {
		rc = regulator_set_voltage(power, 3000000, 3000000);
		if(rc) {
			dev_err(dev, "regulator_set_voltage failed\n");
			goto err_ts_set_power;
		}
	}

	if(on) {
		rc = regulator_enable(power);
		if(rc) {
			dev_err(dev, "regulator_enable failed\n");
			goto err_ts_set_power;
		}
	} else {
		rc = regulator_disable(power);
		if(rc) {
			dev_err(dev, "regulator_enable failed\n");
			goto err_ts_set_power;
		}
	}

	return rc;

err_ts_set_power:
	if(power)
		regulator_put(power);
	power = NULL;
	return rc;
}
#endif

static struct mms_ts_platform_data ts_pdata = {
	.max_x = 240,
	.max_y = 320,
	.power = ts_set_power,
};

static struct gpio_i2c_pin ts_i2c_pin[] = {
	[0] = {
		.sda_pin	= 10,
		.scl_pin	= 9,
		.reset_pin	= 0,
		.irq_pin	= 39,
	},
};

static struct i2c_gpio_platform_data ts_i2c_pdata = {
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay			= 1,
};

static struct platform_device ts_i2c_device = {
	.name	= "i2c-gpio",
	.dev.platform_data = &ts_i2c_pdata,
};

static struct i2c_board_info ts_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("mms_ts", 0x48),
		.type = "mms_ts",
		.platform_data = &ts_pdata,
	},
};

static int init_gpio_i2c_pin_touch(
	struct i2c_gpio_platform_data *i2c_adap_pdata,
	struct gpio_i2c_pin gpio_i2c_pin,
	struct i2c_board_info *i2c_board_info_data)
{
	i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;

	if(gpio_i2c_pin.sda_pin) {
		i2c_adap_pdata->sda_pin = gpio_i2c_pin.sda_pin;
		gpio_request(gpio_i2c_pin.sda_pin, "Melfas_I2C_SDA");
		gpio_tlmm_config(
		GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.sda_pin, 1);
		gpio_free(gpio_i2c_pin.sda_pin);
	}

	if(gpio_i2c_pin.scl_pin) {
		i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;
		gpio_request(gpio_i2c_pin.scl_pin, "Melfas_I2C_SCL");
		gpio_tlmm_config(
		GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.scl_pin, 1);
		gpio_free(gpio_i2c_pin.scl_pin);
	}

	if (gpio_i2c_pin.reset_pin) {
		gpio_request(gpio_i2c_pin.reset_pin, "Melfas_I2C_Reset");
		gpio_tlmm_config(
			GPIO_CFG(gpio_i2c_pin.reset_pin, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.reset_pin, 1);
		gpio_free(gpio_i2c_pin.reset_pin);
	}

	if (gpio_i2c_pin.irq_pin) {
		i2c_board_info_data->irq = MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
		gpio_request(gpio_i2c_pin.irq_pin, "Melfas_I2C_INT");
		gpio_tlmm_config(
			GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_free(gpio_i2c_pin.irq_pin);
	}

	return 0;
}

static void __init v1_init_i2c_touch(int bus_num)
{
	ts_i2c_device.id = bus_num;

	init_gpio_i2c_pin_touch(
		&ts_i2c_pdata, ts_i2c_pin[0], &ts_i2c_bdinfo[0]);
	i2c_register_board_info(
		bus_num, &ts_i2c_bdinfo[0], 1);
	platform_device_register(&ts_i2c_device);
}
#else /*qct original touch codes*/
static struct regulator_bulk_data regs_atmel[] = {
	{ .supply = "ldo12", .min_uV = 2700000, .max_uV = 3300000 },
	{ .supply = "smps3", .min_uV = 1800000, .max_uV = 1800000 },
};

#define ATMEL_TS_GPIO_IRQ 82

static int atmel_ts_power_on(bool on)
{
	int rc = on ?
		regulator_bulk_enable(ARRAY_SIZE(regs_atmel), regs_atmel) :
		regulator_bulk_disable(ARRAY_SIZE(regs_atmel), regs_atmel);

	if (rc)
		pr_err("%s: could not %sable regulators: %d\n",
				__func__, on ? "en" : "dis", rc);
	else
		msleep(50);

	return rc;
}

static int atmel_ts_platform_init(struct i2c_client *client)
{
	int rc;
	struct device *dev = &client->dev;

	rc = regulator_bulk_get(dev, ARRAY_SIZE(regs_atmel), regs_atmel);
	if (rc) {
		dev_err(dev, "%s: could not get regulators: %d\n",
				__func__, rc);
		goto out;
	}

	rc = regulator_bulk_set_voltage(ARRAY_SIZE(regs_atmel), regs_atmel);
	if (rc) {
		dev_err(dev, "%s: could not set voltages: %d\n",
				__func__, rc);
		goto reg_free;
	}

	rc = gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE);
	if (rc) {
		dev_err(dev, "%s: gpio_tlmm_config for %d failed\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto reg_free;
	}

	/* configure touchscreen interrupt gpio */
	rc = gpio_request(ATMEL_TS_GPIO_IRQ, "atmel_maxtouch_gpio");
	if (rc) {
		dev_err(dev, "%s: unable to request gpio %d\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto ts_gpio_tlmm_unconfig;
	}

	rc = gpio_direction_input(ATMEL_TS_GPIO_IRQ);
	if (rc < 0) {
		dev_err(dev, "%s: unable to set the direction of gpio %d\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto free_ts_gpio;
	}
	return 0;

free_ts_gpio:
	gpio_free(ATMEL_TS_GPIO_IRQ);
ts_gpio_tlmm_unconfig:
	gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
reg_free:
	regulator_bulk_free(ARRAY_SIZE(regs_atmel), regs_atmel);
out:
	return rc;
}

static int atmel_ts_platform_exit(struct i2c_client *client)
{
	gpio_free(ATMEL_TS_GPIO_IRQ);
	gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	regulator_bulk_free(ARRAY_SIZE(regs_atmel), regs_atmel);
	return 0;
}

static u8 atmel_ts_read_chg(void)
{
	return gpio_get_value(ATMEL_TS_GPIO_IRQ);
}

static u8 atmel_ts_valid_interrupt(void)
{
	return !atmel_ts_read_chg();
}


static struct maxtouch_platform_data atmel_ts_pdata = {
	.numtouch = 4,
	.init_platform_hw = atmel_ts_platform_init,
	.exit_platform_hw = atmel_ts_platform_exit,
	.power_on = atmel_ts_power_on,
	.display_res_x = 480,
	.display_res_y = 864,
	.min_x = ATMEL_X_OFFSET,
	.max_x = (505 - ATMEL_X_OFFSET),
	.min_y = ATMEL_Y_OFFSET,
	.max_y = (863 - ATMEL_Y_OFFSET),
	.valid_interrupt = atmel_ts_valid_interrupt,
	.read_chg = atmel_ts_read_chg,
};

static struct i2c_board_info atmel_ts_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO(ATMEL_TS_I2C_NAME, 0x4a),
		.platform_data = &atmel_ts_pdata,
		.irq = MSM_GPIO_TO_INT(ATMEL_TS_GPIO_IRQ),
	},
};
#endif 

#ifdef CONFIG_LGE_DIAGTEST
static struct platform_device lg_diag_input_device = {
	.name = "ats_input",
	.id = -1,
	.dev = { .platform_data = 0, },
};

static struct platform_device *v1_ats_input_devices[] __initdata = {
       &lg_diag_input_device,
};
#endif

void __init msm7627a_add_io_devices(void)
{
	/*LGE_CHANGE_S : seven.kim@lge.com JB 2035.2B Migration*/
	/* ignore end key as this target doesn't need it */
	hs_platform_data.ignore_end_key = true;
	/*LGE_CHANGE_E : seven.kim@lge.com JB 2035.2B Migration*/
	platform_add_devices(v1_input_devices, ARRAY_SIZE(v1_input_devices));

#if defined(CONFIG_TOUCHSCREEN_MELFAS_MMS128S) || defined(CONFIG_TOUCHSCREEN_MELFAS_MMS128S_V1) || defined(CONFIG_TOUCHSCREEN_MELFAS_MMS100S)
	lge_add_gpio_i2c_device(v1_init_i2c_touch);
#else /*qct original*/
	/* touchscreen */
	if (machine_is_msm7625a_surf() || machine_is_msm7625a_ffa()) {
		atmel_ts_pdata.min_x = 0;
		atmel_ts_pdata.max_x = 480;
		atmel_ts_pdata.min_y = 0;
		atmel_ts_pdata.max_y = 320;
	}

	i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				atmel_ts_i2c_info,
				ARRAY_SIZE(atmel_ts_i2c_info));
#endif

#ifdef CONFIG_LGE_DIAGTEST
	platform_add_devices(v1_ats_input_devices, ARRAY_SIZE(v1_ats_input_devices));
#endif

/* [LGE_BSP_S] jihyun2.jang@lge.com, 20130212 - RT8966A miniABB regulator : PROXIMITY SENSOR LDO */
#ifdef CONFIG_MINIABB_REGULATOR
#if 0
	{
		int rc;

		pr_info("### %s: initialize regulator for prox. sensor ###\n", __func__);

		regulator_prox = regulator_get( NULL, "prox" );
		if( regulator_prox == NULL ) {
			pr_err("### %s: could not get regulators: prox ###\n", __func__);
		}
		else {
			rc = regulator_set_voltage( regulator_prox, 3000000, 3000000 );
			if( rc ) {
				pr_err("### %s: could not set prox voltages: %d ###\n", __func__, rc);
			}
		}

		pr_info("### %s: initialize regulator for accl. sensor ###\n", __func__);

		regulator_accl = regulator_get( NULL, "accl" );
		if( regulator_accl == NULL ) {
			pr_err("### %s: could not get regulators: accl ###\n", __func__);
		}
		else {
			rc = regulator_set_voltage( regulator_accl, 3000000, 3000000 );
			if( rc ) {
				pr_err("### %s: could not set accl voltages: %d ###\n", __func__, rc);
			}
		}

		if( regulator_accl != NULL )
		{
			if( !regulator_is_enabled( regulator_accl ) ) {
				rc = regulator_enable( regulator_accl );
				if( rc < 0 )
					pr_err("### %s: regulator_enable(regulator_accl) failed ###\n", __func__);
			}
		}

	}
#endif
#endif /* CONFIG_MINIABB_REGULATOR */
/* [LGE_BSP_E] jihyun2.jang@lge.com, 20130212 - RT8966A miniABB regulator : PROXIMITY SENSOR LDO */

#if 1//defined (CONFIG_SENSORS_BMM050) ||defined(CONFIG_SENSORS_BMA250)
	lge_add_gpio_i2c_device(v1_init_i2c_sensor);
#endif

	lge_add_gpio_i2c_device(v1_init_i2c_prox);
	
#ifdef CONFIG_LEDS_LP5521	
	lge_add_gpio_i2c_device(lp5521_init_i2c_rgb);
#endif	
	
	msm_init_pmic_vibrator();
}

void __init qrd7627a_add_io_devices(void)
{
}
