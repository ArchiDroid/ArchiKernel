#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/regulator/rt8053.h>
#include <linux/regulator/consumer.h>

#include <mach/board_lge.h>

/* Sub-PMIC */
static struct gpio_i2c_pin subpm_i2c_pin = {
	.sda_pin = 13,
	.scl_pin = 14,
};

static struct i2c_gpio_platform_data subpm_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device subpm_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &subpm_i2c_pdata,
};

static struct regulator_consumer_supply rt8053_vreg_supply[] = {
	REGULATOR_SUPPLY("RT8053_LDO1", NULL),
	REGULATOR_SUPPLY("RT8053_LDO2", NULL),
	REGULATOR_SUPPLY("RT8053_LDO3", NULL),
	REGULATOR_SUPPLY("RT8053_LDO4", NULL),
	REGULATOR_SUPPLY("RT8053_LDO5", NULL),
	REGULATOR_SUPPLY("RT8053_DCDC1", NULL),
};

#define RT8053_VREG_INIT(_id, _min_uV, _max_uV) \
	{ \
		.id = RT8053_##_id, \
		.initdata = { \
			.constraints = { \
				.valid_modes_mask = REGULATOR_MODE_NORMAL, \
				.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | \
					REGULATOR_CHANGE_STATUS, \
				.min_uV = _min_uV, \
				.max_uV = _max_uV, \
			}, \
			.num_consumer_supplies = 1, \
			.consumer_supplies = \
				&rt8053_vreg_supply[RT8053_##_id], \
		}, \
	}

static struct rt8053_regulator_subdev rt8053_regulators[] = {
	RT8053_VREG_INIT(LDO1, 1200000, 3300000),
	RT8053_VREG_INIT(LDO2, 1200000, 3300000),
	RT8053_VREG_INIT(LDO3, 1200000, 3300000),
	RT8053_VREG_INIT(LDO4,  800000, 2850000),
	RT8053_VREG_INIT(LDO5, 1200000, 3300000),
	RT8053_VREG_INIT(DCDC1, 800000, 2300000),
};

static struct rt8053_platform_data rt8053_data = {
	.num_regulators = 6,
	.enable_gpio = 23,
	.regulators = rt8053_regulators,
};

static struct i2c_board_info subpm_i2c_bdinfo[] = {
	{
		I2C_BOARD_INFO("rt8053", 0x7D),
		.platform_data = &rt8053_data,
	},
};

void __init msm7x27a_m4_init_i2c_subpm(int bus_num)
{
	subpm_i2c_device.id = bus_num;

// START: youngbae.choi@lge.com 2011-09-23
#if defined(CONFIG_MACH_MSM7X25A_M4EU_EVB)
	if (lge_bd_rev == EVB) {
		struct rt8053_platform_data *pdata =
			subpm_i2c_bdinfo[0].platform_data;
		if (pdata)
			pdata->enable_gpio = 58;
	}
#endif
// END: youngbae.choi@lge.com 2011-09-23 

	lge_init_gpio_i2c_pin(&subpm_i2c_pdata, subpm_i2c_pin,
		&subpm_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &subpm_i2c_bdinfo[0], 1);
	platform_device_register(&subpm_i2c_device);
}

/* common interface */
void __init lge_add_pm_devices(void)
{
#if defined(CONFIG_MACH_MSM7X25A_M4EU_EVB)
	lge_add_gpio_i2c_device(msm7x27a_m4_init_i2c_subpm);
#elif (defined(CONFIG_MACH_MSM7X25A_M4EU_REV_A) || defined(CONFIG_MACH_MSM7X25A_M4EU_REV_B) || defined(CONFIG_MACH_MSM7X25A_M4BR_REV_B)	\
		|| defined(CONFIG_MACH_MSM7X25A_M4CA_BELL_REV_B) || defined(CONFIG_MACH_MSM7X25A_M4CA_TLS_REV_B))
	/* REV A : No use SUBPMIC */
#endif
}
