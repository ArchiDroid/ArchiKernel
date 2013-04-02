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
#include <linux/i2c.h>
#include <linux/input/rmi_platformdata.h>
#include <linux/input/rmi_i2c.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <mach/rpc_server_handset.h>
#include <mach/pmic.h>
#include <linux/ktime.h>
#include <mach/vreg.h>
/*LGE_CHANGE_S : seven.kim@lge.com kernel3.4 for v3/v5*/
#if defined (CONFIG_MACH_LGE)
#include "../../devices.h"
#include "../../board-msm7627a.h"
#include "../../devices-msm7x2xa.h"
#include CONFIG_LGE_BOARD_HEADER_FILE
#else /*qct original*/
#include "devices.h"
#include "board-msm7627a.h"
#include "devices-msm7x2xa.h"
#endif /*CONFIG_MACH_LGE*/
/*LGE_CHANGE_E : seven.kim@lge.com kernel3.4 for v3/v5*/

#ifdef CONFIG_LGE_NFC
#include <linux/nfc/pn544_lge.h>
#endif

#define HARD_HOME_KEY 172

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
 
static unsigned int keypad_row_gpios[] = {36, 37,38};
static unsigned int keypad_col_gpios[] = {32,33};

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(keypad_row_gpios) + (row))

static const unsigned short keypad_keymap_m4[] = {
	[KEYMAP_INDEX(0, 0)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(0, 1)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(1, 2)] = HARD_HOME_KEY,
};

int m4_matrix_info_wrapper(struct gpio_event_input_devs *input_dev,
			   struct gpio_event_info *info, void **data, int func)
{
	int ret;

	if(func == GPIO_EVENT_FUNC_INIT){
		gpio_tlmm_config(
			GPIO_CFG(keypad_col_gpios[0], 0,
					 GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_col_gpios[1], 0,
					 GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[0], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[1], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[2], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	}
	if (func == GPIO_EVENT_FUNC_RESUME) {
		gpio_tlmm_config(
			GPIO_CFG(keypad_col_gpios[0], 0,
					 GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_col_gpios[1], 0,
					 GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[0], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[1], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[2], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	}
	
	ret = gpio_event_matrix_func(input_dev, info, data, func);
	return ret ;
}

static int m4_gpio_matrix_power(const struct gpio_event_platform_data *pdata, bool on)
{
	/* this is dummy function
	 * to make gpio_event driver register suspend function
	 * 2010-01-29, cleaneye.kim@lge.com
	 * copy from ALOHA code
	 * 2010-04-22 younchan.kim@lge.com
	 */
	return 0;
}

static struct gpio_event_matrix_info m4_keypad_matrix_info = {
	.info.func	= m4_matrix_info_wrapper,
	.keymap		= keypad_keymap_m4,
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

static struct gpio_event_info *m4_keypad_info[] = {
	&m4_keypad_matrix_info.info
};

static struct gpio_event_platform_data m4_keypad_data = {
	.name		= "7x27a_kp",
	.info		= m4_keypad_info,
	.info_count	= ARRAY_SIZE(m4_keypad_info),
	.power          = m4_gpio_matrix_power,
};

struct platform_device keypad_device_m4 = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &m4_keypad_data,
	},
};

/* LED platform data */
static struct platform_device msm_device_pmic_leds = {
	.name = "pmic-leds",
	.id = -1,
};

/* input platform device */
static struct platform_device *m4_input_devices[] __initdata = {
	&hs_pdev,
	&keypad_device_m4,
	&msm_device_pmic_leds,
};

#ifdef CONFIG_LGE_DIAGTEST
static struct platform_device lg_diag_input_device = {
	.name = "ats_input",
	.id = -1,
	.dev = { .platform_data = 0, },
};

static struct platform_device *m4_ats_input_devices[] __initdata = {
       &lg_diag_input_device,
};
#endif

/* LGE_CHANGE_S [seven.kim@lge.com] 20110922 New Bosch compass+accel Sensor Porting*/ 
static struct gpio_i2c_pin accel_i2c_pin[] = {
	[0] = {
		.sda_pin	= SENSOR_GPIO_I2C_SDA,
		.scl_pin	= SENSOR_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ACCEL_GPIO_INT,
	},
};

static struct gpio_i2c_pin ecom_i2c_pin[] = {
	[0] = {
		.sda_pin	= SENSOR_GPIO_I2C_SDA,
		.scl_pin	= SENSOR_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ECOM_GPIO_INT,
	},
};

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
		I2C_BOARD_INFO("bma250", ACCEL_I2C_ADDRESS),
		.type = "bma250",
	},
	[1] = {
		I2C_BOARD_INFO("bmm050", ECOM_I2C_ADDRESS),
		.type = "bmm050",
	},
};

static void __init m4_init_i2c_sensor(int bus_num)
{
	sensor_i2c_device.id = bus_num;

	lge_init_gpio_i2c_pin(&sensor_i2c_pdata, accel_i2c_pin[0], &sensor_i2c_bdinfo[0]);
	lge_init_gpio_i2c_pin(&sensor_i2c_pdata, ecom_i2c_pin[0], &sensor_i2c_bdinfo[1]);

	i2c_register_board_info(bus_num, sensor_i2c_bdinfo, ARRAY_SIZE(sensor_i2c_bdinfo));

	platform_device_register(&sensor_i2c_device);
}

/* proximity */
static int prox_power_set(unsigned char onoff)
{

	return 0;
}

static struct proximity_platform_data proxi_pdata = {
	.irq_num	= PROXI_GPIO_DOUT,
	.power		= prox_power_set,
	.methods		= 1,
		.operation_mode		= 1,
		.debounce	 = 0,
		.cycle = 0,
};

static struct i2c_board_info prox_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("proximity_gp2ap", PROXI_I2C_ADDRESS),
		.type = "proximity_gp2ap",
		.platform_data = &proxi_pdata,
	},
};
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


static void __init m4_init_i2c_prox(int bus_num)
{
	proxi_i2c_device.id = bus_num;

	lge_init_gpio_i2c_pin(&proxi_i2c_pdata, proxi_i2c_pin[0], &prox_i2c_bdinfo[0]);

	i2c_register_board_info(bus_num, &prox_i2c_bdinfo[0], 1);
	platform_device_register(&proxi_i2c_device);
}


#if defined(CONFIG_TOUCHSCREEN_MELFAS_TS)
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


//static struct regulator *regulator_ts;
//static char is_touch_Initialized = 0;

int ts_set_vreg(unsigned char onoff)
{
		int rc = 0;	

		struct vreg *vreg_touch;
		
		printk(KERN_INFO "[Touch] %s() onoff:%d\n",
			   __func__, onoff);
	
		vreg_touch = vreg_get(0, "usim2");
	
		if (IS_ERR(vreg_touch)) {
			printk(KERN_INFO "[Touch] vreg_get fail : touch\n");
			return -EBUSY;
		}
	
		if (onoff) {
			rc = vreg_set_level(vreg_touch, 2850);
			if (rc != 0) {
				printk(KERN_INFO "[Touch] vreg_set_level failed\n");
				return -EBUSY;
			}
			vreg_enable(vreg_touch);
		} else {
			vreg_disable(vreg_touch);
		}

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

/* LGE_CHANGE_S: seven.kim@lge.com [V3 Touch Porting] : Fix Touch GPIO Warning Message*/
	gpio_request(TS_GPIO_I2C_SDA, "Melfas_I2C_SDA");
	gpio_request(TS_GPIO_I2C_SCL, "Melfas_I2C_SCL");
	gpio_request(TS_GPIO_IRQ, "Melfas_I2C_INT");
/* LGE_CHANGE_E: seven.kim@lge.com [V3 Touch Porting] : Fix Touch GPIO Warning Message*/

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

#if 0	/*LGE_CHANGE_S : seven.kim@lge.com for V3 Touch Porting*/
#if (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4BR_REV_B))
	gpio_tlmm_config(
		GPIO_CFG(TS_GPIO_POWER, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif
#endif /*LGE_CHANGE_E : seven.kim@lge.com for V3 Touch Porting*/

	if (gpio_i2c_pin.irq_pin) {
		gpio_tlmm_config(
			GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		i2c_board_info_data->irq =
			MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
	}

/* LGE_CHANGE_S : seven.kim@lge.com [V3 Touch Porting] : Fix Touch GPIO Warning Message*/
	gpio_free(TS_GPIO_I2C_SDA);
	gpio_free(TS_GPIO_I2C_SCL);
	gpio_free(TS_GPIO_IRQ);
/* LGE_CHANGE_E: seven.kim@lge.com [V3 Touch Porting] : Fix Touch GPIO Warning Message*/

	return 0;
}

static void __init m4_init_i2c_touch(int bus_num)
{
	ts_i2c_device.id = bus_num;

	init_gpio_i2c_pin_touch(
		&ts_i2c_pdata, ts_i2c_pin[0], &ts_i2c_bdinfo[0]);
	i2c_register_board_info(
		bus_num, &ts_i2c_bdinfo[0], 1);
	platform_device_register(&ts_i2c_device);
}
/*LGE_CHANGE_E : seven.kim@lge.com for v3 melfas mms128s touch*/
#endif 

#ifdef CONFIG_LGE_NFC
// 2012.09.26 garam.kim@lge.com NFC registration
static struct gpio_i2c_pin nfc_i2c_pin[] = {
	[0] = {
		.sda_pin	= NFC_GPIO_I2C_SDA,
		.scl_pin	= NFC_GPIO_I2C_SCL,
		.reset_pin	= NFC_GPIO_VEN,
		.irq_pin	= NFC_GPIO_IRQ,
	},
};

static struct i2c_gpio_platform_data nfc_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device nfc_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &nfc_i2c_pdata,
};

static struct pn544_i2c_platform_data nfc_pdata = {
	.ven_gpio 		= NFC_GPIO_VEN,
	.irq_gpio 	 	= NFC_GPIO_IRQ,
	.scl_gpio		= NFC_GPIO_I2C_SCL,
	.sda_gpio		= NFC_GPIO_I2C_SDA,
	.firm_gpio		= NFC_GPIO_FIRM,
};

static struct i2c_board_info nfc_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("pn544", NFC_I2C_SLAVE_ADDR),
		.platform_data = &nfc_pdata,
		.irq = MSM_GPIO_TO_INT(NFC_GPIO_IRQ),
	},
};

static void __init m4_init_i2c_nfc(int bus_num)
{
	int ret;
/*
 	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_FIRM, 0, GPIO_CFG_OUTPUT,
 				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_VEN, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_IRQ, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE); 
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_I2C_SDA, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE); 
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_I2C_SCL, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE); 
*/

	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_FIRM, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_VEN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(NFC_GPIO_VEN, 1);
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_IRQ, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
    nfc_i2c_bdinfo->irq = MSM_GPIO_TO_INT(NFC_GPIO_IRQ);
	nfc_i2c_device.id = bus_num;

	ret = lge_init_gpio_i2c_pin(&nfc_i2c_pdata, nfc_i2c_pin[0],	&nfc_i2c_bdinfo[0]);
	
	ret = i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID, &nfc_i2c_bdinfo[0], 1);
	
	platform_device_register(&nfc_i2c_device);	
}
#endif

void __init msm7627a_add_io_devices(void)
{
	/*LGE_CHANGE_S : seven.kim@lge.com JB 2035.2B Migration*/
	/* ignore end key as this target doesn't need it */
	hs_platform_data.ignore_end_key = true;
	/*LGE_CHANGE_E : seven.kim@lge.com JB 2035.2B Migration*/
	
	platform_add_devices(m4_input_devices, ARRAY_SIZE(m4_input_devices));

	
/*LGE_CHANGE_S : seven.kim@lge.com for v3 mms128 touch*/
#if defined(CONFIG_TOUCHSCREEN_MELFAS_TS)
	lge_add_gpio_i2c_device(m4_init_i2c_touch);
#endif

#ifdef CONFIG_LGE_DIAGTEST
	platform_add_devices(m4_ats_input_devices, ARRAY_SIZE(m4_ats_input_devices));
#endif

/*LGE_CHANGE_E : seven.kim@lge.com for v3 mms128 touch*/

#if defined (CONFIG_SENSORS_BMM050) ||defined(CONFIG_SENSORS_BMA250)
	lge_add_gpio_i2c_device(m4_init_i2c_sensor);
#endif	

	//eee3114.lee@lge.com sensor
	lge_add_gpio_i2c_device(m4_init_i2c_prox);

#ifdef CONFIG_LGE_NFC
    lge_add_gpio_i2c_device(m4_init_i2c_nfc);
#endif

#if 0
	/* keypad */
	platform_device_register(&kp_pdev);

	/* headset */
	platform_device_register(&hs_pdev);

	/* LED: configure it as a pdm function */
	if (gpio_tlmm_config(GPIO_CFG(LED_GPIO_PDM, 3,
				GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE))
		pr_err("%s: gpio_tlmm_config for %d failed\n",
			__func__, LED_GPIO_PDM);
	else
		platform_device_register(&led_pdev);
#endif
	/* Vibrator */
	//if (machine_is_msm7x27a_ffa() || machine_is_msm7625a_ffa()
	//				|| machine_is_msm8625_ffa())
		msm_init_pmic_vibrator();
}


