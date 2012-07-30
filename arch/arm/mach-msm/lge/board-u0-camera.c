#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#include <asm/mach-types.h>

#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/board_lge.h>

#include "devices-msm7x2xa.h"
#include "board-u0.h"
/* LGE_CHANGE_S :  U0 led flash driver
 20011-10-27, samjinjang@lge.com, 
 Porting AS3647 LED Flash Driver */
#include <linux/leds-as364x.h>		
/*  LGE_CHANGE_E :   U0 led flash driver */
#include <mach/pmic.h>


//====================================================================================
//                             MAIN CAM GPIO SETTING
//====================================================================================
#if defined (CONFIG_HI542)
#define CAM_GPIO_MCLK					(15)
	#define CAM_MAIN_I2C_SLAVE_ADDR         (0x20 >> 1)     
	#define CAM_AF_I2C_SLAVE_ADDR           (0x18 >> 2)     
	#define CAM_MAIN_GPIO_RESET_N (0)
	#define CAM_GPIO_VCM	31		/* GPIO 31 : AF_PWDN_EN */
	#define CAM_GPIO_IOVDD	128		/* GPIO 128 : CAM_IOVDD_1.8V_EN */
	#define CAM_GPIO_AVDD	48		/* GPIO 48 : CAM_AVDD_2.8V_EN */
	#define CAM_GPIO_DVDD	49		/* GPIO 49 : CAM_DVDD_1.8V_EN */
	#define CAM_GPIO_AF		23		/* GPIO 23 : CAM_AF_2.8V_EN */
#elif defined(CONFIG_MT9E013_LGIT)
	#define CAM_GPIO_MCLK (15)
	#define CAM_MAIN_I2C_SLAVE_ADDR (0x6C>>1)
	#define CAM_MAIN_GPIO_RESET_N (0)
#define CAM_GPIO_VCM	31		/* GPIO 31 : AF_PWDN_EN */
#define CAM_GPIO_IOVDD	128		/* GPIO 128 : CAM_IOVDD_1.8V_EN */
#define CAM_GPIO_AVDD	48		/* GPIO 48 : CAM_AVDD_2.8V_EN */
#define CAM_GPIO_DVDD	49		/* GPIO 49 : CAM_DVDD_1.8V_EN */
#define CAM_GPIO_AF		23		/* GPIO 23 : CAM_AF_2.8V_EN */
#endif

#ifdef CONFIG_HI542
static int power_gpio_initialzed = 0;
#endif

//====================================================================================
//                             FRONT CAM GPIO SETTING
//====================================================================================
#ifdef CONFIG_MT9V113
#define CAM_VGA_I2C_SLAVE_ADDR			(0x7A>>1)
#define CAM_VGA_GPIO_RESET_N			(7)
#define CAM_VGA_GPIO_PWDN				(8)
#else
	#define CAM_VGA_I2C_SLAVE_ADDR  (0x7A>>1)
	#define CAM_VGA_GPIO_RESET_N  (7)
	#define CAM_VGA_GPIO_PWDN (8)
#endif


//====================================================================================
//                             GPIO CONFIG SETTING
//====================================================================================
#ifdef CONFIG_MSM_CAMERA
static uint32_t camera_off_gpio_table[] = {
	GPIO_CFG(CAM_GPIO_MCLK, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
#if defined(CONFIG_MT9E013_LGIT)
	GPIO_CFG(GPIO_CAM_RESET, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
#endif
};

static uint32_t camera_on_gpio_table[] = {
	GPIO_CFG(CAM_GPIO_MCLK, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
#if defined(CONFIG_MT9E013_LGIT)
	GPIO_CFG(GPIO_CAM_RESET, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
#endif
};


//====================================================================================
//                             MAIN CAM LDO On / Off
//====================================================================================
#ifdef CONFIG_HI542
static void msm_camera_vreg_config(int vreg_en)
{
	static int gpio_initialzed = 0;

	pr_info("[CAM] %s: vreg_en[%d] power_gpio_initialzed[%d]\n", __func__, vreg_en, power_gpio_initialzed);

#if 0
	if (!power_gpio_initialzed) {
		gpio_request(GPIO_CAM_RESET, "cam_reset");
		gpio_direction_output(GPIO_CAM_RESET, 0);
		power_gpio_initialzed = 1;
	}
#endif

	if (!gpio_initialzed) {
		gpio_request(CAM_GPIO_IOVDD, "cam_iovdd_en");	/* GPIO 128 : CAM_IOVDD_1.8V_EN */
		gpio_direction_output(CAM_GPIO_IOVDD, 0);
		gpio_request(CAM_GPIO_AVDD, "cam_avdd_en");	/* GPIO 48 : CAM_AVDD_2.8V_EN */
		gpio_direction_output(CAM_GPIO_AVDD, 0);
		gpio_request(CAM_GPIO_DVDD, "cam_dvdd_en");	/* GPIO 49 : CAM_DVDD_1.8V_EN */
		gpio_direction_output(CAM_GPIO_DVDD, 0);
		gpio_request(CAM_GPIO_AF, "cam_af_en"); 	/* GPIO 23 : CAM_AF_2.8V_EN */
		gpio_direction_output(CAM_GPIO_AF, 0);
		gpio_request(GPIO_CAM_RESET, "cam_reset");
		gpio_direction_output(GPIO_CAM_RESET, 0);
		gpio_initialzed = 1;
	}

	if (vreg_en) {
		gpio_set_value(CAM_GPIO_IOVDD, 1);
		gpio_set_value(CAM_GPIO_AVDD, 1);
		gpio_set_value(CAM_GPIO_DVDD, 1);
		gpio_set_value(CAM_GPIO_AF, 1);
		mdelay(1);
                gpio_set_value(GPIO_CAM_RESET, 1);
	} else {
		gpio_set_value(GPIO_CAM_RESET, 0);
		mdelay(1);
		gpio_set_value(CAM_GPIO_AF, 0);
		gpio_set_value(CAM_GPIO_DVDD, 0);
		gpio_set_value(CAM_GPIO_AVDD, 0);
		gpio_set_value(CAM_GPIO_IOVDD, 0);

		power_gpio_initialzed = 0;
	}

	return;
}
#elif defined(CONFIG_MT9E013_LGIT)
static void msm_camera_vreg_config(int vreg_en)
{
	printk("msm_camera_vreg_config(%d)\n", vreg_en);

	if(vreg_en)
	{
		if(lge_bd_rev < LGE_REV_C)
		{
			pmic_miniabb_ldo_control(3, 1);
			pmic_miniabb_ldo_control(2, 1);
			pmic_miniabb_ldo_control(1, 1);
			pmic_miniabb_ldo_control(4, 1);
		}
		else
		{
			//DVDD 1.2V
			gpio_tlmm_config(GPIO_CFG(49, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			gpio_set_value(49, 1);
			mdelay(10);
			
			//IOVDD 1.8V
			gpio_tlmm_config(GPIO_CFG(128, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			gpio_set_value(128, 1);
			mdelay(10);
			
			//AVDD 2.8V
			gpio_tlmm_config(GPIO_CFG(48, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			gpio_set_value(48, 1);
			mdelay(10);
			
			//AF
			gpio_tlmm_config(GPIO_CFG(23, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			gpio_set_value(23, 1);
		}
	}
	else
	{
		if(lge_bd_rev < LGE_REV_C)
		{
			pmic_miniabb_ldo_control(4, 0);
			pmic_miniabb_ldo_control(1, 0);
			pmic_miniabb_ldo_control(2, 0);
			pmic_miniabb_ldo_control(3, 0);
		}
		else
		{
			//AVDD 2.8V
			gpio_tlmm_config(GPIO_CFG(48, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			gpio_set_value(48, 0);
			mdelay(10);
			
			//DVDD 1.2V
			gpio_tlmm_config(GPIO_CFG(49, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			gpio_set_value(49, 0);
			mdelay(10);
			
			//IOVDD 1.8V
			gpio_tlmm_config(GPIO_CFG(128, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			gpio_set_value(128, 0);
			mdelay(10);

			//AF
			gpio_tlmm_config(GPIO_CFG(23, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			gpio_set_value(23, 0);
		}
	}
	
	return;
}
#endif

static int config_gpio_table(uint32_t *table, int len)
{
	int rc = 0, i = 0;

	for (i = 0; i < len; i++) {
		rc = gpio_tlmm_config(table[i], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s not able to get gpio\n", __func__);
			for (i--; i >= 0; i--)
				gpio_tlmm_config(camera_off_gpio_table[i],
							GPIO_CFG_ENABLE);
			break;
		}
	}
	return rc;
}

static int config_camera_on_gpios_rear(void)
{
	int rc = 0;

#if defined(CONFIG_MT9E013_LGIT)
	printk("config_camera_on_gpios_rear\n");
#else
	msm_camera_vreg_config(1);
#endif

	rc = config_gpio_table(camera_on_gpio_table,
			ARRAY_SIZE(camera_on_gpio_table));
	if (rc < 0) {
		pr_err("%s: CAMSENSOR gpio table request"
		"failed\n", __func__);
		return rc;
	}

	return rc;
}

static void config_camera_off_gpios_rear(void)
{
	msm_camera_vreg_config(0);

	config_gpio_table(camera_off_gpio_table,
			ARRAY_SIZE(camera_off_gpio_table));
}

static int config_camera_on_gpios_front(void)
{
	int rc = 0;
	rc = config_gpio_table(camera_on_gpio_table,
			ARRAY_SIZE(camera_on_gpio_table));
	if (rc < 0) {
		pr_err("%s: CAMSENSOR gpio table request"
			"failed\n", __func__);
		return rc;
	}
	return rc;
}

static void config_camera_off_gpios_front(void)
{
	config_gpio_table(camera_off_gpio_table,
			ARRAY_SIZE(camera_off_gpio_table));
}

static int camera_power_on_rear(void)
{
#if defined(CONFIG_MT9E013_LGIT)
	printk("camera_power_on_rear\n");
	msm_camera_vreg_config(1);
#endif
	return 0;
}

static int camera_power_off_rear(void)
{
#if defined(CONFIG_MT9E013_LGIT)
	printk("camera_power_off_rear\n");
	msm_camera_vreg_config(0);
#endif
	return 0;
}

static int camera_power_on_front(void)
{
#ifdef CONFIG_MT9V113
	/* GPIO 128 : CAM_IOVDD_1.8V_EN */
	printk("### [DEBUG] FRONT LDO ON ");
	gpio_tlmm_config(GPIO_CFG(CAM_GPIO_IOVDD, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(CAM_GPIO_IOVDD, 1);
	
	/* GPIO 48 : CAM_AVDD_2.8V_EN */
	gpio_tlmm_config(GPIO_CFG(CAM_GPIO_AVDD, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(CAM_GPIO_AVDD, 1);

	/* GPIO 49 : CAM_DVDD_1.8V_EN */
	gpio_tlmm_config(GPIO_CFG(CAM_GPIO_DVDD, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(CAM_GPIO_DVDD, 1);
#elif defined(CONFIG_MT9E013_LGIT)
 	if(lge_bd_rev < LGE_REV_C)
 	{
		pmic_miniabb_ldo_control( 3, 1);
		pmic_miniabb_ldo_control( 2, 1);
		pmic_miniabb_ldo_control( 1, 1);
	}
	else
	{
		//IOVDD
		gpio_tlmm_config(GPIO_CFG(128, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(128, 1);
		//AVDD
		gpio_tlmm_config(GPIO_CFG(48, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(48, 1);
		//DVDD
		gpio_tlmm_config(GPIO_CFG(49, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(49, 1);
	}
#endif
	return 0;
}

static int camera_power_off_front(void)
{
#ifdef CONFIG_MT9V113
	printk("### [DEBUG] FRONT LDO ON ");
	/* GPIO 49 : CAM_DVDD_1.8V_EN */
	gpio_tlmm_config(GPIO_CFG(CAM_GPIO_DVDD, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(CAM_GPIO_DVDD, 0);

	/* GPIO 48 : CAM_AVDD_2.8V_EN */
	gpio_tlmm_config(GPIO_CFG(CAM_GPIO_AVDD, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(CAM_GPIO_AVDD, 0);

	/* GPIO 128 : CAM_IOVDD_1.8V_EN */
	gpio_tlmm_config(GPIO_CFG(CAM_GPIO_IOVDD, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(CAM_GPIO_IOVDD, 0);
#elif defined(CONFIG_MT9E013_LGIT)
 	if(lge_bd_rev < LGE_REV_C)
 	{
		pmic_miniabb_ldo_control( 1, 0);
		pmic_miniabb_ldo_control( 2, 0);
		pmic_miniabb_ldo_control( 3, 0);
	}
	else
	{
		//DVDD
		gpio_tlmm_config(GPIO_CFG(49, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(49, 0);
		//AVDD
		gpio_tlmm_config(GPIO_CFG(48, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(48, 0);
		//IOVDD
		gpio_tlmm_config(GPIO_CFG(128, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(128, 0);		
	}
#endif
	return 0;
}

struct msm_camera_device_platform_data msm_camera_device_data_rear = {
	.camera_gpio_on  = config_camera_on_gpios_rear,
	.camera_gpio_off = config_camera_off_gpios_rear,
	.ioext.csiphy = 0xA1000000,
	.ioext.csisz  = 0x00100000,
	.ioext.csiirq = INT_CSI_IRQ_1,
	.ioclk.mclk_clk_rate = 24000000,
	.ioclk.vfe_clk_rate  = 192000000,
	.ioext.appphy = MSM_CLK_CTL_PHYS,
	.ioext.appsz  = MSM_CLK_CTL_SIZE,
	.camera_power_on   = camera_power_on_rear,
	.camera_power_off  = camera_power_off_rear,
};

struct msm_camera_device_platform_data msm_camera_device_data_front = {
	.camera_gpio_on  = config_camera_on_gpios_front,
	.camera_gpio_off = config_camera_off_gpios_front,
	.ioext.csiphy = 0xA0F00000,
	.ioext.csisz  = 0x00100000,
	.ioext.csiirq = INT_CSI_IRQ_0,
	.ioclk.mclk_clk_rate =  28000000,	//24000000,	// agkwon_test
	.ioclk.vfe_clk_rate  = 192000000,
	.ioext.appphy = MSM_CLK_CTL_PHYS,
	.ioext.appsz  = MSM_CLK_CTL_SIZE,
	.camera_power_on   = camera_power_on_front,
	.camera_power_off  = camera_power_off_front,
};

// FLASH LIGHT --------------------------------------------------------------------
#ifdef CONFIG_LEDS_AS364X
static struct msm_camera_sensor_flash_src led_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_CURRENT_DRIVER,
};

static struct msm_camera_sensor_flash_data led_flash_data = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &led_flash_src,
};
#else
static struct msm_camera_sensor_flash_data led_flash_data = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
	.flash_src  = NULL,
};
#endif


// REAR CAMERA --------------------------------------------------------------------
#ifdef CONFIG_HI542
static struct msm_camera_sensor_platform_info hi542_sensor_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_info msm_camera_sensor_hi542_data = {
	.sensor_name    = "hi542",
	.sensor_reset_enable = 1,
	.sensor_reset   = GPIO_CAM_RESET,
	.sensor_pwd     = 0,
	.vcm_pwd        = 31,
	.vcm_enable     = 1,
	.pdata          = &msm_camera_device_data_rear,
	.flash_data     = &led_flash_data,
	.csi_if         = 1,
	.sensor_platform_info = &hi542_sensor_info,
};

static struct platform_device msm_camera_sensor_hi542 = {
        .name      = "msm_camera_hi542",
        .dev       = {
                .platform_data = &msm_camera_sensor_hi542_data,
        },
};
#elif defined(CONFIG_MT9E013_LGIT)
static struct msm_camera_sensor_platform_info mt9e013_lgit_sensor_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9e013_lgit_data = {
        .sensor_name          = "mt9e013_lgit",
        .sensor_reset         = GPIO_CAM_RESET,
        .sensor_pwd           = CAM_MAIN_GPIO_RESET_N,
        .vcm_pwd              = 1,
        .vcm_enable           = 1,
        .pdata                = &msm_camera_device_data_rear,
        .flash_data           = &led_flash_data,
		.sensor_platform_info  = &mt9e013_lgit_sensor_info,
        .csi_if               = 1
};

static struct platform_device msm_camera_sensor_mt9e013_lgit = {
        .name      = "msm_camera_mt9e013_lgit",
        .dev       = {
                .platform_data = &msm_camera_sensor_mt9e013_lgit_data,
        },
};
#endif


// FRONT CAMERA --------------------------------------------------------------------
#ifdef CONFIG_MT9V113
static struct msm_camera_sensor_platform_info mt9v113_sensor_info = {
	.mount_angle = 270   /* LGE_CHANGE [donghyun.kwon@lge.com] 20120102 : MCU_DATA_0, Flip = OFF, Mirror = ON */
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9v113_data = {
        .sensor_name          = "mt9v113",
        .sensor_reset         = CAM_VGA_GPIO_RESET_N,
        .sensor_pwd           = CAM_VGA_GPIO_PWDN,
        .vcm_pwd              = 1,
        .vcm_enable           = 0,
        .pdata                = &msm_camera_device_data_front,
        .flash_data           = &led_flash_data,
		.sensor_platform_info  = &mt9v113_sensor_info,
        .csi_if               = 1
};

static struct platform_device msm_camera_sensor_mt9v113 = {
        .name      = "msm_camera_mt9v113",
        .dev       = {
                .platform_data = &msm_camera_sensor_mt9v113_data,
        },
};
#endif



// CAMERA DEVICES  --------------------------------------------------------------------
static struct i2c_board_info i2c_camera_devices[] = {
#ifdef CONFIG_HI542
	{
		I2C_BOARD_INFO("hi542", CAM_MAIN_I2C_SLAVE_ADDR),
	},
	{
		I2C_BOARD_INFO("hi542_af", CAM_AF_I2C_SLAVE_ADDR),
	},
#elif defined(CONFIG_MT9E013_LGIT)
	{
		I2C_BOARD_INFO("mt9e013_lgit", CAM_MAIN_I2C_SLAVE_ADDR),
	},
#endif
#ifdef CONFIG_MT9V113
	{
		I2C_BOARD_INFO("mt9v113", CAM_VGA_I2C_SLAVE_ADDR),
	},
#endif
};

// FLASH LIGHT  --------------------------------------------------------------------
#ifdef CONFIG_LEDS_AS364X
static struct gpio_i2c_pin flash_i2c_pin[] = {
	{
		.sda_pin	= GPIO_FLASH_I2C_SDA,
		.scl_pin	= GPIO_FLASH_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= 0,
	},
};

static struct i2c_gpio_platform_data flash_i2c_pdata = {
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay			    = 2,
};

static struct platform_device flash_i2c_device = {
	.name	= "i2c-gpio",
	.dev.platform_data = &flash_i2c_pdata,
};

static struct as364x_platform_data as364x_pdata = {
	.use_tx_mask = 1,
	.I_limit_mA = 2500,
	.txmasked_current_mA = 200,
	.vin_low_v_run_mV = 3300,
	.vin_low_v_mV = 3000,
	.strobe_type = 0, /* 0=edge */
#ifdef CONFIG_AS3648
	.max_peak_current_mA = 1000,
	.max_peak_duration_ms = 500,/* values for LUW FQ6N @1000mA */
#endif
#ifdef CONFIG_AS3647
	.max_peak_current_mA = 1600, /* LUW FQ6N can take up to 2000 */
	.max_peak_duration_ms = 100,/* value for LUW FQ6N @1600mA */
#endif
	.max_sustained_current_mA = 200,
	.min_current_mA = 50,
	.freq_switch_on = 0,
	.led_off_when_vin_low = 1, /* if 0 txmask current is used */
};

static struct i2c_board_info i2c_camera_flash_devices[] = {
	{
		I2C_BOARD_INFO(LEDS_CAMERA_FLASH_NAME, FLASH_I2C_ADDRESS),	
		.flags = I2C_CLIENT_WAKE,
		.irq = 0,
		.platform_data = &as364x_pdata,
	},
};
#endif
#endif /* CONFIG_MSM_CAMERA */

static struct platform_device *u0_camera_devices[] __initdata = {
#ifdef CONFIG_HI542
    &msm_camera_sensor_hi542,
#elif defined(CONFIG_MT9E013_LGIT)
	&msm_camera_sensor_mt9e013_lgit,
#endif
#ifdef CONFIG_MT9V113
	&msm_camera_sensor_mt9v113,
#endif

};

#ifdef CONFIG_LEDS_AS364X
static void __init u0_init_i2c_camera(int bus_num)
{
	flash_i2c_device.id = bus_num;

	lge_init_gpio_i2c_pin(&flash_i2c_pdata, flash_i2c_pin[0],
		&i2c_camera_flash_devices[0]);
	i2c_register_board_info(bus_num, i2c_camera_flash_devices,
		ARRAY_SIZE(i2c_camera_flash_devices));
	platform_device_register(&flash_i2c_device);
}
#endif

void __init lge_add_camera_devices(void)
{
#ifdef CONFIG_MSM_CAMERA
	i2c_register_board_info(MSM_GSBI0_QUP_I2C_BUS_ID,
		i2c_camera_devices,
		ARRAY_SIZE(i2c_camera_devices));
#endif
	platform_add_devices(u0_camera_devices,
		ARRAY_SIZE(u0_camera_devices));
#ifdef CONFIG_LEDS_AS364X
	lge_add_gpio_i2c_device(u0_init_i2c_camera);
#endif
}

