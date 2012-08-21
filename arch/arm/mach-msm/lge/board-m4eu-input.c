#include <linux/init.h>
#include <linux/platform_device.h>
#include <mach/rpc_server_handset.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/delay.h>
#include <linux/gpio_event.h>
#ifdef CONFIG_PN544_NFC_XXX	//wongab.jeon@lge.com
#include <linux/nfc/pn544.h> // 2011.06.24 kiwon.jeon@lge.com NFC
#endif
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/pmic.h>
#include <mach/board_lge.h>
#include <linux/regulator/consumer.h>

#include "devices-msm7x2xa.h"
#include "board-m4eu.h"

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
#if defined(CONFIG_MACH_MSM7X25A_M4EU_EVB)
static unsigned int keypad_row_gpios[] = {
	36, 37
};

static unsigned int keypad_col_gpios[] = {33};

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(keypad_row_gpios) + (row))

static const unsigned short keypad_keymap_m4eu[] = {
	[KEYMAP_INDEX(0, 0)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(0, 1)] = KEY_VOLUMEDOWN,
};
#elif (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4EU_REV_B) \
		|| defined(CONFIG_MACH_MSM7X25A_M4CA_BELL_REV_B) || defined(CONFIG_MACH_MSM7X25A_M4CA_TLS_REV_B))
static unsigned int keypad_row_gpios[] = {
	36, 37,38
};

static unsigned int keypad_col_gpios[] = {32,33};

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(keypad_row_gpios) + (row))

static const unsigned short keypad_keymap_m4eu[] = {
	[KEYMAP_INDEX(0, 0)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(0, 1)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(1, 2)] = HARD_HOME_KEY,
};
#elif (defined(CONFIG_MACH_MSM7X25A_M4BR_REV_B))
static unsigned int keypad_row_gpios[] = {
	36, 37
};

static unsigned int keypad_col_gpios[] = {32};

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(keypad_row_gpios) + (row))

static const unsigned short keypad_keymap_m4eu[] = {
	[KEYMAP_INDEX(0, 0)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(0, 1)] = KEY_VOLUMEDOWN,
};
#endif

int m4eu_matrix_info_wrapper(struct gpio_event_input_devs *input_dev,
							 struct gpio_event_info *info, void **data, int func)
{
	int ret;

	if(func == GPIO_EVENT_FUNC_INIT){
		gpio_tlmm_config(
			GPIO_CFG(keypad_col_gpios[0], 0,
					 GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#if (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4EU_REV_B) 	\
	|| defined(CONFIG_MACH_MSM7X25A_M4CA_BELL_REV_B) || defined(CONFIG_MACH_MSM7X25A_M4CA_TLS_REV_B))		
		gpio_tlmm_config(
			GPIO_CFG(keypad_col_gpios[1], 0,
					 GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[0], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[1], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#if (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4EU_REV_B) 	\
	|| defined(CONFIG_MACH_MSM7X25A_M4CA_BELL_REV_B) || defined(CONFIG_MACH_MSM7X25A_M4CA_TLS_REV_B))
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[2], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif
	}
	if (func == GPIO_EVENT_FUNC_RESUME) {
		gpio_tlmm_config(
			GPIO_CFG(keypad_col_gpios[0], 0,
					 GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#if (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4EU_REV_B) 	\
	|| defined(CONFIG_MACH_MSM7X25A_M4CA_BELL_REV_B) || defined(CONFIG_MACH_MSM7X25A_M4CA_TLS_REV_B))		
		gpio_tlmm_config(
			GPIO_CFG(keypad_col_gpios[1], 0,
					 GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[0], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[1], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#if (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4EU_REV_B) 	\
	|| defined(CONFIG_MACH_MSM7X25A_M4CA_BELL_REV_B) || defined(CONFIG_MACH_MSM7X25A_M4CA_TLS_REV_B))
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[2], 0,
					 GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif
	}
	ret = gpio_event_matrix_func(input_dev, info, data, func);
	return ret ;
}

static int m4eu_gpio_matrix_power(const struct gpio_event_platform_data *pdata, bool on)
{
	/* this is dummy function
	 * to make gpio_event driver register suspend function
	 * 2010-01-29, cleaneye.kim@lge.com
	 * copy from ALOHA code
	 * 2010-04-22 younchan.kim@lge.com
	 */

	return 0;
}

static struct gpio_event_matrix_info m4eu_keypad_matrix_info = {
	.info.func	= m4eu_matrix_info_wrapper,
	.keymap		= keypad_keymap_m4eu,
	.output_gpios	= keypad_col_gpios,
	.input_gpios	= keypad_row_gpios,
	.noutputs	= ARRAY_SIZE(keypad_col_gpios),
	.ninputs	= ARRAY_SIZE(keypad_row_gpios),
//LGE_CHANGE_S, [youngbae.choi@lge.com] , 2011-12-08
 /* gpio_event_matrix_info structure member was changed, ktime_t -> struct timespec */	
	.settle_time.tv_nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv_nsec = 20 * NSEC_PER_MSEC,
//LGE_CHANGE_E, [youngbae.choi@lge.com] , 2011-12-08
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_DRIVE_INACTIVE
};

static struct gpio_event_info *m4eu_keypad_info[] = {
	&m4eu_keypad_matrix_info.info
};

static struct gpio_event_platform_data m4eu_keypad_data = {
	.name		= "m4_keypad",
	.info		= m4eu_keypad_info,
	.info_count	= ARRAY_SIZE(m4eu_keypad_info),
	.power          = m4eu_gpio_matrix_power,
};

struct platform_device keypad_device_m4eu = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &m4eu_keypad_data,
	},
};


/* input platform device */
static struct platform_device *m4eu_input_devices[] __initdata = {
	&hs_pdev,
};

static struct platform_device *m4eu_gpio_input_devices[] __initdata = {
	&keypad_device_m4eu,
};

/* Melfas MCS8000 Touch (mms-128)*/
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

int ts_set_vreg(unsigned char onoff)
{
	int rc;
	
#if defined(CONFIG_MACH_MSM7X25A_M4EU_EVB)
	static struct regulator *ldo1 = NULL;
	static int init = 0;
	ldo1 = regulator_get(NULL, "RT8053_LDO1");
	if (ldo1 == NULL)
		pr_err(
			"%s: regulator_get(ldo1) failed\n",
			__func__);

	if (onoff) {
		rc = regulator_set_voltage(ldo1, 3000000, 3000000);
		if (rc < 0)
			pr_err(
				"%s: regulator_set_voltage(ldo1) failed\n",
				__func__);

		rc = regulator_enable(ldo1);
		if (rc < 0)
			pr_err(
				"%s: regulator_enable(ldo1) failed\n",
				__func__);

		init = 1;
	} else {
		if (init > 0) {
			rc = regulator_disable(ldo1);
			if (rc < 0)
				pr_err(
					"%s: regulator_disble(ldo1) failed\n",
					__func__);

			regulator_put(ldo1);
		}
	}
#elif (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4BR_REV_B))
	
	if (onoff){
		printk("[Melfas]touch power on\n");
		rc = gpio_direction_output(TS_GPIO_POWER, 1);
	}
	else{
		printk("[Melfas]touch power off\n");
		rc = gpio_direction_output(TS_GPIO_POWER, 0);
	}	
	
#else // (CONFIG_MACH_MSM7X25A_M4EU_REV_B) 
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
#endif	
	msleep(20);
	return 0;
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
/* LGE_CHANGE_S : RECENT_APPS_KEY (Bell Operator in Canada) */ 
#if defined(CONFIG_MACH_MSM7X25A_M4CA_BELL_REV_B)
		I2C_BOARD_INFO("touch_mcs8000_bell", TS_I2C_SLAVE_ADDR),
		.type = "touch_mcs8000_bell",
#else
		I2C_BOARD_INFO("touch_mcs8000", TS_I2C_SLAVE_ADDR),
		.type = "touch_mcs8000",
#endif
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
#if (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4BR_REV_B))
	gpio_tlmm_config(
		GPIO_CFG(TS_GPIO_POWER, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif

	if (gpio_i2c_pin.irq_pin) {
		gpio_tlmm_config(
			GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		i2c_board_info_data->irq =
			MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
	}

	return 0;
}

static void __init m4eu_init_i2c_touch(int bus_num)
{
	ts_i2c_device.id = bus_num;

	init_gpio_i2c_pin_touch(
		&ts_i2c_pdata, ts_i2c_pin[0], &ts_i2c_bdinfo[0]);
	i2c_register_board_info(
		bus_num, &ts_i2c_bdinfo[0], 1);
	platform_device_register(&ts_i2c_device);
}
#endif /* CONFIG_TOUCH_MCS8000 */

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

/* Atmel Touch for M4EU EVB */
#if defined(CONFIG_TOUCHSCREEN_MXT140)

static struct gpio_i2c_pin ts_i2c_pin = {
	.sda_pin = TS_GPIO_I2C_SDA,
	.scl_pin = TS_GPIO_I2C_SCL,
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

static int ts_set_vreg(unsigned char onoff)
{
	struct vreg *vreg_touch;
	int rc;

	printk(KERN_INFO "[Touch] %s() onoff:%d\n",
		   __func__, onoff);

	vreg_touch = vreg_get(0, "bt");

	if (IS_ERR(vreg_touch)) {
		printk(KERN_INFO "[Touch] vreg_get fail : touch\n");
		return -EBUSY;
	}

	if (onoff) {
		rc = vreg_set_level(vreg_touch, 3000);
		if (rc != 0) {
			printk(KERN_INFO "[Touch] vreg_set_level failed\n");
			return -EBUSY;
		}
		vreg_enable(vreg_touch);
	} else {
		vreg_disable(vreg_touch);
	}

	return 0;
}

static struct touch_platform_data ts_pdata = {
	.ts_x_min   = TS_X_MIN,
	.ts_x_max   = TS_X_MAX,
	.ts_y_min   = TS_Y_MIN,
	.ts_y_max   = TS_Y_MAX,
	.ts_y_start = 0,
	.ts_y_scrn_max = 480,
	.power      = ts_set_vreg,
	.gpio_int   = TS_GPIO_IRQ,
	.irq 	  = MSM_GPIO_TO_INT(TS_GPIO_IRQ),
	.scl      = TS_GPIO_I2C_SCL,
	.sda      = TS_GPIO_I2C_SDA,
	.hw_i2c     = 0,
};

static struct i2c_board_info ts_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("qt602240_ts", 0x4A),
		.type = "qt602240_ts",
		.platform_data = &ts_pdata,
	},
};

static void __init m4eu_init_i2c_touch(int bus_num)
{
	ts_i2c_device.id = bus_num;
	/* workaround for HDK rev_a no pullup */
	lge_init_gpio_i2c_pin(
		&ts_i2c_pdata, ts_i2c_pin, &ts_i2c_bdinfo[0]);
	i2c_register_board_info(
		bus_num, &ts_i2c_bdinfo[0], 1);
	platform_device_register(&ts_i2c_device);
}
#endif /* CONFIG_TOUCH_mxt_140 */

/** accelerometer **/
#if defined (CONFIG_SENSORS_BMM050) ||defined(CONFIG_SENSORS_BMA250)
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

static void __init m4eu_init_i2c_sensor(int bus_num)
{
	sensor_i2c_device.id = bus_num;

	lge_init_gpio_i2c_pin(&sensor_i2c_pdata, accel_i2c_pin[0], &sensor_i2c_bdinfo[0]);
	lge_init_gpio_i2c_pin(&sensor_i2c_pdata, ecom_i2c_pin[0], &sensor_i2c_bdinfo[1]);

	i2c_register_board_info(bus_num, sensor_i2c_bdinfo, ARRAY_SIZE(sensor_i2c_bdinfo));

	platform_device_register(&sensor_i2c_device);
}
#else
static struct gpio_i2c_pin accel_i2c_pin[] = {
	[0] = {
		.sda_pin	= ACCEL_GPIO_I2C_SDA,
		.scl_pin	= ACCEL_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ACCEL_GPIO_INT,
	},
};

static struct i2c_gpio_platform_data accel_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device accel_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &accel_i2c_pdata,
};

static struct i2c_board_info accel_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("bma222", ACCEL_I2C_ADDRESS),
		.type = "bma222",
	},
};

static void __init m4eu_init_i2c_acceleration(int bus_num)
{
	accel_i2c_device.id = bus_num;
	lge_init_gpio_i2c_pin(
		&accel_i2c_pdata, accel_i2c_pin[0], &accel_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &accel_i2c_bdinfo[0], 1);
	platform_device_register(&accel_i2c_device);
}
#endif
/* proximity */
static int prox_power_set(unsigned char onoff)
{	
/* need to be fixed  - for vreg using SUB PMIC */
	int ret = 0;
#if defined(CONFIG_MACH_MSM7X25A_M4EU_EVB)
	static bool init_done =0;
	struct regulator *ldo5 = NULL;

	ldo5 = regulator_get(NULL, "RT8053_LDO5");
	if (ldo5 == NULL)
		pr_err(
			"%s: regulator_get(ldo5) failed\n",
			__func__);

	printk(KERN_INFO "[Proximity] %s() : Power %s\n",
		   __func__, onoff ? "On" : "Off");

	if (init_done == 0 && onoff) {
		if (onoff) {
			printk(KERN_INFO "LDO5 vreg set.\n");
			ret = regulator_set_voltage(ldo5, 2800000, 2800000);
			if (ret < 0)
				pr_err(
					"%s: regulator_set_voltage(ldo5) failed\n",
					__func__);

			ret = regulator_enable(ldo5);
			if (ret < 0)
				pr_err(
					"%s: regulator_enable(ldo5) failed\n",
					__func__);

			init_done = 1;
		} else {
			ret = regulator_disable(ldo5);
			if (ret < 0)
				pr_err(
					"%s: regulator_disable(ldo5) failed\n",
					__func__);

		}
	}
#endif
	return ret;
}

static struct proximity_platform_data proxi_pdata[] = {
	[0]={
		.irq_num	= PROXI_GPIO_DOUT,
		.power		= prox_power_set,
		.methods		= 1,
		.operation_mode		= 2,
		.debounce	 = 0,
		.cycle = 0,
	},
	[1]={
		.irq_num	= PROXI_GPIO_DOUT,
		.power		= prox_power_set,
		.methods		= 1,
		.operation_mode		= 1,
		.debounce	 = 0,
		.cycle = 0,
	}
};

static struct i2c_board_info prox_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("proximity_gp2ap", PROXI_I2C_ADDRESS),
		.type = "proximity_gp2ap",
		.platform_data = &proxi_pdata[0],
	},
	[1] = {
		I2C_BOARD_INFO("proximity_gp2ap", PROXI_I2C_ADDRESS),
		.type = "proximity_gp2ap",
		.platform_data = &proxi_pdata[1],
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

static void __init m4eu_init_i2c_prox(int bus_num)
{
	proxi_i2c_device.id = bus_num;
	if(lge_bd_rev==LGE_REV_11){
		lge_init_gpio_i2c_pin(&proxi_i2c_pdata, proxi_i2c_pin[0], &prox_i2c_bdinfo[1]);
		i2c_register_board_info(bus_num, &prox_i2c_bdinfo[1], 1);
	}else{
		lge_init_gpio_i2c_pin(&proxi_i2c_pdata, proxi_i2c_pin[0], &prox_i2c_bdinfo[0]);
		i2c_register_board_info(bus_num, &prox_i2c_bdinfo[0], 1);
	}

	platform_device_register(&proxi_i2c_device);
}

#ifdef CONFIG_PN544_NFC_XXX	//wongab.jeon@lge.com
// [START] 2011.06.24 kiwon.jeon@lge.com NFC
static struct gpio_i2c_pin nfc_i2c_pin[] = 
{
	[0] = 
	{
		.sda_pin		= NFC_GPIO_I2C_SDA,
		.scl_pin		= NFC_GPIO_I2C_SCL,
		.reset_pin	= NFC_GPIO_VEN,		
		.irq_pin		= NFC_GPIO_IRQ,
	},
};

static struct i2c_gpio_platform_data nfc_i2c_pdata = 
{
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device nfc_i2c_device = 
{
	.name = "i2c-gpio",
	.dev.platform_data = &nfc_i2c_pdata,
};

static struct pn544_i2c_platform_data nfc_pdata = 
{
	.ven_gpio 	= NFC_GPIO_VEN,
	.irq_gpio 	 	= NFC_GPIO_IRQ,
	.scl_gpio		= NFC_GPIO_I2C_SCL,
	.sda_gpio		= NFC_GPIO_I2C_SDA,
	.firm_gpio	= NFC_GPIO_FIRM,
};

static struct i2c_board_info nfc_i2c_bdinfo[] = 
{
	[0] = {
		I2C_BOARD_INFO("pn544", NFC_I2C_SLAVE_ADDR),
		.type = "pn544",
		.platform_data = &nfc_pdata,
	},
};

static void __init m4eu_init_i2c_nfc(int bus_num)
{
	int ret;

	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_FIRM, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	nfc_i2c_device.id = bus_num;

	ret = lge_init_gpio_i2c_pin(&nfc_i2c_pdata, nfc_i2c_pin[0],	&nfc_i2c_bdinfo[0]);

	printk(KERN_INFO "[NFC] lge_init_gpio_i2c_pin[%d]\n", ret);
  
	ret = i2c_register_board_info(bus_num, &nfc_i2c_bdinfo[0], 1);

	printk(KERN_INFO "[NFC] i2c_register_board_info[%d]\n", ret);	

	platform_device_register(&nfc_i2c_device);
}

//#else	//wongab.jeon@lge.com
static void m4eu_nfc_gpio_sleep_set(void) 
{
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_IRQ, 0, GPIO_CFG_INPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_VEN, 0, GPIO_CFG_INPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_FIRM, 0, GPIO_CFG_INPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_I2C_SDA, 0, GPIO_CFG_INPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(NFC_GPIO_I2C_SCL, 0, GPIO_CFG_INPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
}
#endif
// [END] 2011.06.24 kiwon.jeon@lge.com NFC

#if (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4BR_REV_B))
void touch_init(void)
{
	int rc;
	
	/* gpio init */
	rc = gpio_request(TS_GPIO_POWER, "TOUCH_PANEL_PWR");
	
}
#endif
void __init lge_add_input_devices(void)
{
	platform_add_devices(
		m4eu_input_devices, ARRAY_SIZE(m4eu_input_devices));
	platform_add_devices(
		m4eu_gpio_input_devices, ARRAY_SIZE(m4eu_gpio_input_devices));	
	lge_add_gpio_i2c_device(m4eu_init_i2c_touch);

#ifdef CONFIG_LGE_DIAGTEST
	platform_add_devices(m4_ats_input_devices, ARRAY_SIZE(m4_ats_input_devices));
#endif

#if (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4BR_REV_B))	
	touch_init();
#endif	
#if defined (CONFIG_SENSORS_BMM050) ||defined(CONFIG_SENSORS_BMA250)
	lge_add_gpio_i2c_device(m4eu_init_i2c_sensor);
#else
	lge_add_gpio_i2c_device(m4eu_init_i2c_acceleration);
#endif
	lge_add_gpio_i2c_device(m4eu_init_i2c_prox);
	
#ifdef CONFIG_PN544_NFC_XXX	//wongab.jeon@lge.com
	lge_add_gpio_i2c_device(m4eu_init_i2c_nfc); // 2011.06.24 kiwon.jeon@lge.com NFC
//#else	//wongab.jeon@lge.com
	m4eu_nfc_gpio_sleep_set();
#endif	
}
