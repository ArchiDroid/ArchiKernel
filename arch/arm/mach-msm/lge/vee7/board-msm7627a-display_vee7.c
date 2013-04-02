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
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/regulator/consumer.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>
#include <mach/rpc_pmapp.h>
/*LGE_CHANGE_S : seven.kim@lge.com kernel3.4 for v3/v5*/
#if defined (CONFIG_MACH_LGE)
#include "../../devices.h"
#include "../../board-msm7627a.h"
#include CONFIG_LGE_BOARD_HEADER_FILE
#else /*qct original*/
#include "devices.h"
#include "board-msm7627a.h"
#endif /*CONFIG_MACH_LGE*/
/*LGE_CHANGE_E : seven.kim@lge.com kernel3.4 for v3/v5*/
//LGE_S, sohyun.name@lge.com porting tovis panel, 12-12-26
#include <linux/module.h> // export_symbol
//LGE_E, sohyun.name@lge.com porting tovis panel, 12-12-26

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
#define MSM_FB_SIZE		0x4BF000
#define MSM7x25A_MSM_FB_SIZE    0x1C2000
#define MSM8x25_MSM_FB_SIZE	0x5FA000
#else
#define MSM_FB_SIZE		0x32A000
#define MSM7x25A_MSM_FB_SIZE	0x12C000
#define MSM8x25_MSM_FB_SIZE	0x3FC000
#endif

//LGE_S, sohyun.name@lge.com porting tovis panel, 12-12-26
#define  GPIO_LCD_MAKER_ID      126
unsigned int maker_id;
EXPORT_SYMBOL(maker_id);
//LGE_E, sohyun.name@lge.com porting tovis panel, 12-12-26

static bool firstbootend=0;

static unsigned fb_size = MSM_FB_SIZE;
static int __init fb_size_setup(char *p)
{
	fb_size = memparse(p, NULL);
	return 0;
}

early_param("fb_size", fb_size_setup);

/* backlight device */
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


void __init msm7x27a_vee7e_init_i2c_backlight(int bus_num)
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

#ifdef CONFIG_FB_MSM_MIPI_DSI
#ifdef CONFIG_FB_MSM_MIPI_DSI_LG4573B
static struct platform_device mipi_dsi_lg4573b_panel_device = {
	.name = "mipi_lg4573b",
	.id = 0,
};

static struct platform_device *vee7e_panel_devices[] __initdata = {
	&mipi_dsi_lg4573b_panel_device,
};
#endif
#ifdef CONFIG_FB_MSM_MIPI_DSI_HX8379A
static struct platform_device mipi_dsi_hx8379a_panel_device = {
	.name = "mipi_hx8379a",
	.id = 0,
};

static struct platform_device *vee7_hx8379a_panel_devices[] __initdata = {
	&mipi_dsi_hx8379a_panel_device,
};
#endif
#endif

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
		fb_size = MSM8x25_MSM_FB_SIZE;

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
	.gpio = 97,
	.mdp_rev = MDP_REV_303,
/* [LGSI_SP4_BSP_BEGIN] [kiran.jainapure@lge.com] - QCT supported continuation of display logo image from LK*/	
	.cont_splash_enabled = 0x1,
/* [LGSI_SP4_BSP_END] [kiran.jainapure@lge.com] */	
};

static char mipi_dsi_splash_is_enabled(void)
{
	return mdp_pdata.cont_splash_enabled;
}

enum {
	DSI_SINGLE_LANE = 1,
	DSI_TWO_LANES,
};

static int msm_fb_get_lane_config(void)
{
	int rc = DSI_TWO_LANES;
	return rc;
}

#define GPIO_LCD_RESET 125
static int dsi_gpio_initialized;

static struct regulator *regulator_mipi_dsi;


static int __init mipi_dsi_regulator_init(void)
{
	int rc = 0;
        regulator_mipi_dsi = regulator_get(0, "emmc");
        if (IS_ERR(regulator_mipi_dsi)) {
                pr_err("%s: vreg_get for emmc failed\n", __func__);
                return PTR_ERR(regulator_mipi_dsi);
        }

        rc = regulator_set_voltage(regulator_mipi_dsi, 2850000,2850000);
        if (rc) {
                pr_err("%s: vreg_set_level failed for mipi_dsi_v28\n", __func__);
                goto vreg_put_dsi_v28;
        }

	return 0;
vreg_put_dsi_v28:
        regulator_put(regulator_mipi_dsi);


        return rc;
}

static int mipi_dsi_panel_power(int on)
{
	int rc = 0;

/*LGE_CHANGE_S, youngbae.choi@lge.com, 12-12-28, When Mipi dsi on, regulator control prevent. */
	//if(maker_id == 1)
	{
		if(on == 0)
		{
			if(firstbootend == 0){
				firstbootend = 1;
				return 0;
			}
		}
	}
/*LGE_CHANGE_E, youngbae.choi@lge.com, 12-12-28, When Mipi dsi on, regulator control prevent. */

		if (unlikely(!dsi_gpio_initialized)) {
		
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
			if(maker_id > 0) //LG4572b
				{
			msleep(3);
			rc = regulator_enable(regulator_mipi_dsi);
			if (rc) {
				pr_err("%s: vreg_enable failed for mipi_dsi_v28\n", __func__);
				goto vreg_put_dsi_v28;
			}
			rc = gpio_direction_output(GPIO_LCD_RESET, 1);
			if (rc) {
				pr_err("%s: gpio_direction_output failed for lcd_reset\n", __func__);
				goto vreg_put_dsi_v28;
			}
			
			if (firstbootend) {

				gpio_set_value(GPIO_LCD_RESET, 0);
				msleep(8);
				
				rc = regulator_disable(regulator_mipi_dsi);//2.8v
				msleep(15);
				
				if (rc) {
					pr_err("%s: vreg_disable failed for mipi_dsi_v28\n", __func__);
					goto vreg_put_dsi_v28;
				}			
			
				rc = regulator_enable(regulator_mipi_dsi);//2.8v 
				msleep(15);

				gpio_set_value(GPIO_LCD_RESET, 1);
  				msleep(8);
			
				if (rc) {
					pr_err("%s: vreg_enable failed for mipi_dsi_v28\n", __func__);
					goto vreg_put_dsi_v28;
				}		
			} 
			else {
				firstbootend=1;
			}
		}
		else if( maker_id == 0) //tovis hx8379a
		{
			printk("%s  hx8379a start \n",__func__);

			msleep(3); //LGE_CHANGE_S [changbum.lee] 20120130 : add delay
			rc = regulator_enable(regulator_mipi_dsi);
			udelay(50);
			if (rc) {
				pr_err("%s: vreg_enable failed for mipi_dsi_v28\n", __func__);
				goto vreg_put_dsi_v28;
			}

			if (firstbootend) {
				rc = gpio_direction_output(GPIO_LCD_RESET, 1);
				if (rc) {
					pr_err("%s: gpio_direction_output failed for lcd_reset\n", __func__);
					goto vreg_put_dsi_v28;
				}
				printk("%s RESET of off start \n",__func__);

				//start hardware reset
				gpio_set_value(GPIO_LCD_RESET, 1); 
				msleep(2); 			

				gpio_set_value(GPIO_LCD_RESET, 0); 
				msleep(2);

				gpio_set_value(GPIO_LCD_RESET, 1); 
				msleep(10);

				printk("%s  hx8379a end \n",__func__);

			} 
			//else {
			//	firstbootend=1; /* [LGSI_SP4_BSP] [kiran.jainapure@lge.com] */
			//}

		}
	}
		else//off 
		{
	
			rc = regulator_disable(regulator_mipi_dsi);
			if (rc) {
				pr_err("%s: vreg_disable failed for mipi_dsi_v28\n", __func__);
				goto vreg_put_dsi_v28;
			}
		}
	
		return 0;
	
vreg_put_dsi_v28:
	regulator_put(regulator_mipi_dsi);


	return rc;
	
}

#define MDP_303_VSYNC_GPIO 97

#ifdef CONFIG_FB_MSM_MIPI_DSI
static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.vsync_gpio		= MDP_303_VSYNC_GPIO,
	.dsi_power_save		= mipi_dsi_panel_power,
#ifndef CONFIG_MACH_LGE
	.dsi_client_reset       = msm_fb_dsi_client_reset,
#endif
	.get_lane_config	= msm_fb_get_lane_config,
	
	.splash_is_enabled	= mipi_dsi_splash_is_enabled,

};
#endif

void __init msm_fb_add_devices(void)
{
		gpio_tlmm_config(GPIO_CFG(GPIO_LCD_MAKER_ID, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	maker_id = gpio_get_value(GPIO_LCD_MAKER_ID);

	printk("%s, maker_id : %d\n",__func__,maker_id);
	platform_add_devices(msm_fb_devices, ARRAY_SIZE(msm_fb_devices));

#ifdef CONFIG_FB_MSM_MIPI_DSI_LG4573B
	if( maker_id == 1 )
		platform_add_devices(vee7e_panel_devices, ARRAY_SIZE(vee7e_panel_devices));
#endif
#ifdef CONFIG_FB_MSM_MIPI_DSI_HX8379A		
	else if( maker_id == 0 )
		platform_add_devices(vee7_hx8379a_panel_devices, ARRAY_SIZE(vee7_hx8379a_panel_devices));		
#endif		
	msm_fb_register_device("mdp", &mdp_pdata);
	//msm_fb_register_device("lcdc", 0);
#ifdef CONFIG_FB_MSM_MIPI_DSI
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
#endif

	lge_add_gpio_i2c_device(msm7x27a_vee7e_init_i2c_backlight);

	mipi_dsi_regulator_init();
}
