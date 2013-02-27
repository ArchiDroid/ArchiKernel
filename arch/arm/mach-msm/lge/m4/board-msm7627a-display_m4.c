#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/bootmem.h>

#include <asm/mach-types.h>

#include <mach/msm_rpcrouter.h>
#include <mach/rpc_pmapp.h>
#include <mach/board.h>

#include "devices.h"
#include "../../devices.h"
#include "../../board-msm7627a.h"
#include CONFIG_LGE_BOARD_HEADER_FILE

/* LGE_CHANGE_S : lcd regulator patch
 * 2011-12-21, sinjo.mattappallil@lge.com,
 * vreg is converted to regulator framework.
 */
//#include <mach/vreg.h>
#include <linux/regulator/consumer.h>
/* LGE_CHANGE_E : lcd regulator patch */


#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
#define MSM_FB_SIZE		0x1C2000
#define MSM7x25A_MSM_FB_SIZE    0x1C2000
#define MSM8x25_MSM_FB_SIZE	0x5FA000
#else
#define MSM_FB_SIZE		0x32A000
#define MSM7x25A_MSM_FB_SIZE	0x12C000
#define MSM8x25_MSM_FB_SIZE	0x3FC000
#endif

static unsigned fb_size = MSM_FB_SIZE;
static int __init fb_size_setup(char *p)
{
	fb_size = memparse(p, NULL);
	return 0;
}
early_param("fb_size", fb_size_setup);

static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

static int msm_fb_detect_panel(const char *name)
{
	int ret = -ENODEV;

	if (!strncmp(name, "lcdc_toshiba_fwvga_pt", 21))
		ret = 0;

	return ret;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_fb_resources),
	.resource       = msm_fb_resources,
	.dev    = {
		.platform_data = &msm_fb_pdata,
	}
};

static struct platform_device *msm_fb_devices[] __initdata = {
	&msm_fb_device,
};


/* LGE_CHANGE_S, backlight device, sohyun.nam@lge.com, 2012-10-25 */
static struct gpio_i2c_pin bl_i2c_pin = {
	.sda_pin = 112,
	.scl_pin = 111,
	.reset_pin = 124,
};

static struct i2c_gpio_platform_data bl_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device bl_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &bl_i2c_pdata,
};


#if defined (CONFIG_BACKLIGHT_LM3530)
static struct lge_backlight_platform_data lm3530bl_data = {
	.gpio = 124,
	.version = 3530,
};

static struct i2c_board_info bl_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("lm3530bl", 0x38),
		.type = "lm3530bl",
	},
};
#endif
/* LGE_CHANGE_E, backlight device, sohyun.nam@lge.com, 2012-10-25 */

/* LGE_CHANGE_S, lcd device , sohyun.nam@lge.com, 2012-10-25*/
#if defined (CONFIG_FB_MSM_MIPI_DSI_ILI9486)
static struct platform_device mipi_dsi_ili9486_panel_device = {
	.name = "mipi_ili9486",
	.id = 0,
};

/* input platform device */
static struct platform_device *m4_panel_devices[] __initdata = {
	&mipi_dsi_ili9486_panel_device,
};
#endif
/* LGE_CHANGE_E, lcd device, sohyun.nam@lge.com, 2012-10-25 */

void __init msm_msm7627a_allocate_memory_regions(void)
{
	void *addr;
	unsigned long fb_size;

	if (machine_is_msm7625a_surf() || machine_is_msm7625a_ffa())
		fb_size = MSM7x25A_MSM_FB_SIZE;
	else if (machine_is_msm7627a_evb() || machine_is_msm8625_evb()
						|| machine_is_msm8625_evt())
		fb_size = MSM8x25_MSM_FB_SIZE;
	else
		fb_size = MSM_FB_SIZE;

	addr = alloc_bootmem_align(fb_size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + fb_size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n", fb_size,
						addr, __pa(addr));

#ifdef CONFIG_MSM_V4L2_VIDEO_OVERLAY_DEVICE
	fb_size = MSM_V4L2_VIDEO_OVERLAY_BUF_SIZE;
	addr = alloc_bootmem_align(fb_size, 0x1000);
	msm_v4l2_video_overlay_resources[0].start = __pa(addr);
	msm_v4l2_video_overlay_resources[0].end =
		msm_v4l2_video_overlay_resources[0].start + fb_size - 1;
	pr_debug("allocating %lu bytes at %p (%lx physical) for v4l2\n",
		fb_size, addr, __pa(addr));
#endif

}


static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = 97,						/*LCD_VSYNC_P*/
	.mdp_rev = MDP_REV_303,
/* [LGSI_SP4_BSP_BEGIN] [kiran.jainapure@lge.com] - QCT supported continuation of display logo image from LK*/	
	.cont_splash_enabled = 0x1,
/* [LGSI_SP4_BSP_END] [kiran.jainapure@lge.com] */
        .splash_screen_addr = 0x00,
        .splash_screen_size = 0x00,	
};

enum {
	DSI_SINGLE_LANE = 1,
	DSI_TWO_LANES,
};

static int msm_fb_get_lane_config(void)
{
	int rc = DSI_TWO_LANES;
#if 0
	if (cpu_is_msm7x25a() || cpu_is_msm7x25aa()) {
		rc = DSI_SINGLE_LANE;
		pr_info("DSI Single Lane\n");
	} else {
		pr_info("DSI Two Lanes\n");
	}
#endif
	return rc;
}

#define GPIO_LCD_RESET 125
static int dsi_gpio_initialized = 0;
static int Isfirstbootend = 0;
/* LGE_CHANGE_S : lcd regulator patch
 * 2011-12-21, sinjo.mattappallil@lge.com,
 * vreg is converted to regulator framework.
 */
static struct regulator *vreg_mipi_dsi_v28;

static int __init mipi_dsi_regulator_init(void)
{
	int rc = 0;

        vreg_mipi_dsi_v28 = regulator_get(0, "emmc");
        if (IS_ERR(vreg_mipi_dsi_v28)) {
                pr_err("%s: vreg_get for emmc failed\n", __func__);
                return PTR_ERR(vreg_mipi_dsi_v28);
        }

        rc = regulator_set_voltage(vreg_mipi_dsi_v28, 2800000,2800000);
        if (rc) {
                pr_err("%s: vreg_set_level failed for mipi_dsi_v28\n", __func__);
                goto vreg_put_dsi_v28;
        }

	return 0;
vreg_put_dsi_v28:
        regulator_put(vreg_mipi_dsi_v28);

        return rc;

}

int mipi_dsi_panel_power(int on)
{
	int rc = 0;

	printk("mipi_dsi_panel_power : %d \n", on);

	if (!dsi_gpio_initialized) {

		/* Resetting LCD Panel*/
		rc = gpio_request(GPIO_LCD_RESET, "lcd_reset");
		if (rc) {
			pr_err("%s: gpio_request GPIO_LCD_RESET failed\n", __func__);
		}
		rc = gpio_tlmm_config(GPIO_CFG(GPIO_LCD_RESET, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		if (rc) {
			printk(KERN_ERR "%s: Failed to configure GPIO %d\n",
					__func__, rc);
		}

		dsi_gpio_initialized = 1;
	}

	if (on) {
		
		rc = regulator_enable(vreg_mipi_dsi_v28);
		if (rc) {
			pr_err("%s: vreg_enable failed for mipi_dsi_v28\n", __func__);
			goto vreg_put_dsi_v28;
		}

		rc = gpio_direction_output(GPIO_LCD_RESET, 1);
		if (rc) {
			pr_err("%s: gpio_direction_output failed for lcd_reset\n", __func__);
			goto vreg_put_dsi_v28;
		}
		if (Isfirstbootend) 
		{
			printk("gpio lcd reset on...\n");
			msleep(10);
			gpio_set_value(GPIO_LCD_RESET, 0);
			msleep(10);
			gpio_set_value(GPIO_LCD_RESET, 1);
			//LGE_CHANGE_S : youngbae.choi@lge.com [2012-04-28]		
			//Although Recommand is 120ms, 60ms is no problem when wake up. (120ms --> 60ms)
			msleep(120);
			//LGE_CHANGE_E : youngbae.choi@lge.com [2012-04-28]

			
		} else{
			Isfirstbootend = 1;
		}		
		
	} else {
		rc = regulator_disable(vreg_mipi_dsi_v28);
		if (rc) {
			pr_err("%s: vreg_disable failed for mipi_dsi_v28\n", __func__);
			goto vreg_put_dsi_v28;
		}
	}

	return 0;
	
vreg_put_dsi_v28:
	regulator_put(vreg_mipi_dsi_v28);
	
	return rc;
}
/* LGE_CHANGE_E : lcd regulator patch */

/* LGE_CHANGE_S, MDP setting, sohyun.nam@lge.com, 2012-10-25*/
#define MDP_303_VSYNC_GPIO 97

#ifdef CONFIG_FB_MSM_MDP303
static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.vsync_gpio = MDP_303_VSYNC_GPIO,
	.dsi_power_save   = mipi_dsi_panel_power,
#ifndef CONFIG_MACH_LGE
	.dsi_client_reset = msm_fb_dsi_client_reset,
#endif
	.get_lane_config = msm_fb_get_lane_config,
};
#endif
/* LGE_CHANGE_E, MDP setting, sohyun.nam@lge.com, 2012-10-25 */

void __init msm7x27a_m4_init_i2c_backlight(int bus_num)
{
	bl_i2c_device.id = bus_num;
	bl_i2c_bdinfo[0].platform_data = &lm3530bl_data;
	bl_i2c_pin.scl_pin = 111;
	bl_i2c_pin.sda_pin = 112;

	/* workaround for HDK rev_a no pullup */
	lge_init_gpio_i2c_pin_pullup(&bl_i2c_pdata, bl_i2c_pin, &bl_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &bl_i2c_bdinfo[0], 1);
	platform_device_register(&bl_i2c_device);
}

void __init msm_fb_add_devices(void)
{
	platform_add_devices(msm_fb_devices, ARRAY_SIZE(msm_fb_devices));
	platform_add_devices(m4_panel_devices, ARRAY_SIZE(m4_panel_devices));
	msm_fb_register_device("mdp", &mdp_pdata);
	//msm_fb_register_device("lcdc", 0);
#ifdef CONFIG_FB_MSM_MIPI_DSI
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
#endif
	lge_add_gpio_i2c_device(msm7x27a_m4_init_i2c_backlight);
	mipi_dsi_regulator_init();
}
