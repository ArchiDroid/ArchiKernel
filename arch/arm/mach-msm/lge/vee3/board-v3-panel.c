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
#include <mach/msm_bus_board.h>
#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>
#include <mach/rpc_pmapp.h>
#include "devices.h"
#include "board-msm7627a.h"

#include <linux/fb.h>
#include <mach/board_lge.h>

#ifdef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
#include "../clock-pcom.h"
#include "../proc_comm.h"
#endif

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
#define MSM_FB_SIZE		0x1C3000		/* 20121025 Yoonsoo-Kim[yoonsoo.kim@lge.com] [V3] : QVGA Customization ; 1.76MB  0x4BF000->0x1C3000  */
#define MSM7x25A_MSM_FB_SIZE    0x1C2000
#define MSM8x25_MSM_FB_SIZE	0x5FA000
#else
#define MSM_FB_SIZE		0x32A000
#define MSM7x25A_MSM_FB_SIZE	0x12C000
#define MSM8x25_MSM_FB_SIZE	0x3FC000
#endif

/*LGE_CHANGE_S : seven.kim@lge.com for v3 lcd*/
#define GPIO_LCD_RESET_N 125
#define MDP_303_VSYNC_GPIO 97
/*LGE_CHANGE_E : seven.kim@lge.com for v3 lcd*/

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

#ifdef CONFIG_BACKLIGHT_LM3530
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

void __init msm7x27a_u0_init_i2c_backlight(int bus_num)
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
#endif /*CONFIG_BACKLIGHT_LM3530*/

#ifdef CONFIG_BACKLIGHT_BU61800
static struct lge_backlight_platform_data bu61800bl_data = {
	.gpio = 124,
	.version = 61800,
};

static struct i2c_board_info bl_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("bu61800bl", 0x76),
		.type = "bu61800bl",
	},
};

void __init msm7x27a_v3eu_init_i2c_backlight(int bus_num)
{
	bl_i2c_device.id = bus_num;
	bl_i2c_bdinfo[0].platform_data = &bu61800bl_data;

	/* workaround for HDK rev_a no pullup */
	lge_init_gpio_i2c_pin_pullup(&bl_i2c_pdata, bl_i2c_pin, &bl_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &bl_i2c_bdinfo[0], 1);
	platform_device_register(&bl_i2c_device);
}
#endif /*CONFIG_BACKLIGHT_BU61800*/

#ifdef CONFIG_BACKLIGHT_RT9396
static struct lge_backlight_platform_data rt9396bl_data = {
	.gpio = 124,
	.version = 9396,
};

static struct i2c_board_info bl_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("rt9396bl", 0x54),
		.type = "rt9396bl",
	},
};

void __init msm7x27a_v3eu_init_i2c_backlight(int bus_num)
{
	bl_i2c_device.id = bus_num;
	bl_i2c_bdinfo[0].platform_data = &rt9396bl_data;

	/* workaround for HDK rev_a no pullup */
	lge_init_gpio_i2c_pin_pullup(&bl_i2c_pdata, bl_i2c_pin, &bl_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &bl_i2c_bdinfo[0], 1);
	platform_device_register(&bl_i2c_device);
}
#endif /*CONFIG_BACKLIGHT_RT9396*/

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
static struct platform_device mipi_dsi_lg4573b_panel_device = {
	.name = "mipi_lg4573b",
	.id = 0,
};

static struct platform_device *u0_panel_devices[] __initdata = {
	&mipi_dsi_lg4573b_panel_device,
};
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

#ifdef CONFIG_FB_MSM_EBI2_TOVIS_QVGA
static int ebi2_tovis_power_save(int on);
static struct msm_panel_ilitek_pdata ebi2_tovis_panel_data = {
	.gpio = GPIO_LCD_RESET_N,
	.lcd_power_save = ebi2_tovis_power_save,
	.maker_id = PANEL_ID_TOVIS,
	.initialized = 1,
};

static struct platform_device ebi2_tovis_panel_device = {
	.name	= "ebi2_tovis_qvga",
	.id 	= 0,
	.dev	= {
		.platform_data = &ebi2_tovis_panel_data,
	}
};

/* input platform device */
static struct platform_device *v3eu_panel_devices[] __initdata = {
	&ebi2_tovis_panel_device,
};
#endif /*CONFIG_FB_MSM_EBI2_TOVIS_QVGA*/

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = MDP_303_VSYNC_GPIO,
	.mdp_rev = MDP_REV_303,
};


#ifdef CONFIG_FB_MSM_MIPI_DSI
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
static int Isfirstbootend;

static struct regulator *regulator_mipi_dsi[2];

#ifdef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
static void u0_panel_mipi_dsi_clk_disable(void)
{
        int id = P_DSI_CLK;
        msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, NULL);//dsi_clk

	id = P_DSI_PIXEL_CLK;
        msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, NULL);//dsi_pixel_clk
        
        id = P_DSI_ESC_CLK;
        msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, NULL);//dsi_esc_clk
        
	id = P_DSI_BYTE_CLK;
        msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, NULL);//dsi_byte_div_clk

        id = P_MDP_DSI_P_CLK;
        msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, NULL);//mdp_dsi_p_clk

        id = P_AHB_M_CLK;
        msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, NULL);//ahb_m_clk

        id = P_AHB_S_CLK;
        msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, NULL);//ahb_s_clk

        id = P_DSI_REF_CLK;
        msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, NULL);//dsi_ref_clk
}
#endif

static int __init mipi_dsi_regulator_init(void)
{
	int rc = 0;
        regulator_mipi_dsi[0] = regulator_get(0, "rfrx1");
        if (IS_ERR(regulator_mipi_dsi[0])) {
                pr_err("%s: vreg_get for rfrx1 failed\n", __func__);
                return PTR_ERR(regulator_mipi_dsi[0]);
        }

        rc = regulator_set_voltage(regulator_mipi_dsi[0], 1800000,1800000);
	
	if (rc) {
                pr_err("%s: vreg_set_level failed for mipi_dsi_v18\n", __func__);
		goto vreg_put_dsi_v18;
        }

        regulator_mipi_dsi[1] = regulator_get(0, "emmc");
        if (IS_ERR(regulator_mipi_dsi[1])) {
                pr_err("%s: vreg_get for emmc failed\n", __func__);
                return PTR_ERR(regulator_mipi_dsi[1]);
        }

        rc = regulator_set_voltage(regulator_mipi_dsi[1], 3000000,3000000);
        if (rc) {
                pr_err("%s: vreg_set_level failed for mipi_dsi_v28\n", __func__);
                goto vreg_put_dsi_v28;
        }

	return 0;
vreg_put_dsi_v28:
        regulator_put(regulator_mipi_dsi[1]);

vreg_put_dsi_v18:
        regulator_put(regulator_mipi_dsi[0]);

        return rc;
}

static int mipi_dsi_panel_power(int on)
{
	int rc = 0;

#ifdef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
	static bool lglogo_firstboot = 1;
#endif
//	return 0;
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
			rc = regulator_enable(regulator_mipi_dsi[0]);
			if (rc) {
				pr_err("%s: vreg_enable failed for mipi_dsi_v18\n", __func__);
				goto vreg_put_dsi_v18;
			}
			
			msleep(3); //LGE_CHANGE_S [changbum.lee] 20120130 : add delay
			rc = regulator_enable(regulator_mipi_dsi[1]);
			if (rc) {
				pr_err("%s: vreg_enable failed for mipi_dsi_v28\n", __func__);
				goto vreg_put_dsi_v28;
			}
	
			if (Isfirstbootend) {
	//LGE_CHANGE_S [changbum.lee] 20120130 //Toggle 2.8V
				msleep(15);
				rc = regulator_disable(regulator_mipi_dsi[1]);//2.8v
				if (rc) {
					pr_err("%s: vreg_disable failed for mipi_dsi_v28\n", __func__);
					goto vreg_put_dsi_v28;
				}			
				mdelay(15);
				rc = regulator_enable(regulator_mipi_dsi[1]);//2.8v 
				if (rc) {
					pr_err("%s: vreg_enable failed for mipi_dsi_v28\n", __func__);
					goto vreg_put_dsi_v28;
				}		
	//LGE_CHANGE_E [changbum.lee] 20120130 //Toggle 2.8V
			} else{
				Isfirstbootend = 1;
			}
			msleep(8);//5
		}
		else//off 
		{
	
#ifdef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
			if(lglogo_firstboot)
			{
				printk(KERN_INFO "[DISPLAY]::%s\n",__func__);
				lglogo_firstboot = 0;
				//u0_panel_mipi_dsi_ahb_ctrl_disable();
				u0_panel_mipi_dsi_clk_disable();
			}
#endif
			rc = regulator_disable(regulator_mipi_dsi[0]);
			if (rc) {
				pr_err("%s: vreg_disable failed for mipi_dsi_v18\n", __func__);
				goto vreg_put_dsi_v18;
			}
			
			rc = regulator_disable(regulator_mipi_dsi[1]);
			if (rc) {
				pr_err("%s: vreg_disable failed for mipi_dsi_v28\n", __func__);
				goto vreg_put_dsi_v28;
			}
		}
	
		return 0;
	
vreg_put_dsi_v28:
	regulator_put(regulator_mipi_dsi[1]);

vreg_put_dsi_v18:
	regulator_put(regulator_mipi_dsi[0]);

	return rc;
	
}
#endif /*CONFIG_FB_MSM_MIPI_DSI*/

#ifdef CONFIG_FB_MSM_EBI2
#define REGULATOR_OP(name, op, level)                       			 	\
	do {																	\
		vreg = regulator_get(0, name);										\
		regulator_set_voltage(vreg, level, level);							\
		if (regulator_##op(vreg))											\
			printk(KERN_ERR "%s: %s vreg operation failed \n",				\
				(regulator_##op == regulator_enable) ? "regulator_enable"   \
				: "regulator_disable", name);								\
	} while (0)


static char *msm_fb_vreg[] = {
	"wlan_tcx0",
	"emmc",
};

static int mddi_power_save_on;

static int ebi2_tovis_power_save(int on)
{
	struct regulator *vreg;	
	int flag_on = !!on;

	printk(KERN_INFO "%s: on=%d\n", __func__, flag_on);

	if (mddi_power_save_on == flag_on)
		return 0;

	mddi_power_save_on = flag_on;

	if (on) {
		//REGULATOR_OP(msm_fb_vreg[0], enable, 1800000);
		REGULATOR_OP(msm_fb_vreg[1], enable, 2800000);	
	} else{
		//REGULATOR_OP(msm_fb_vreg[0], disable, 0);
		REGULATOR_OP(msm_fb_vreg[1], disable, 2800000);
	}
	return 0;
}

static int v3eu_fb_event_notify(struct notifier_block *self,
	unsigned long action, void *data)
{
	struct fb_event *event = data;
	struct fb_info *info = event->info;
	struct fb_var_screeninfo *var = &info->var;
	if(action == FB_EVENT_FB_REGISTERED) {
		var->width = 43;
		var->height = 58;
	}
	return 0;
}

static struct notifier_block v3eu_fb_event_notifier = {
	.notifier_call	= v3eu_fb_event_notify,
};
#endif /*CONFIG_FB_MSM_EBI2*/

#ifdef CONFIG_FB_MSM_MIPI_DSI
static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.vsync_gpio		= MDP_303_VSYNC_GPIO,
	.dsi_power_save		= mipi_dsi_panel_power,
#ifndef CONFIG_MACH_LGE
	.dsi_client_reset       = msm_fb_dsi_client_reset,
#endif
	.get_lane_config	= msm_fb_get_lane_config,
};
#endif

void __init msm_fb_add_devices(void)
{
/*LGE_CHANGE_S : seven.kim@lge.com for v3 lcd*/
	if(ebi2_tovis_panel_data.initialized)
		ebi2_tovis_power_save(1);

	fb_register_client(&v3eu_fb_event_notifier);
/*LGE_CHANGE_E : seven.kim@lge.com for v3 lcd*/	
	
	platform_add_devices(msm_fb_devices, ARRAY_SIZE(msm_fb_devices));
	platform_add_devices(v3eu_panel_devices, ARRAY_SIZE(v3eu_panel_devices));
	
	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("lcdc", 0);
#ifdef CONFIG_FB_MSM_EBI2
    msm_fb_register_device("ebi2", 0);
#endif
#ifdef CONFIG_FB_MSM_MIPI_DSI
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
#endif

/*LGE_CHANGE_S : seven.kim@lge.com for v3 lcd*/
	lge_add_gpio_i2c_device(msm7x27a_v3eu_init_i2c_backlight);
/*LGE_CHANGE_E : seven.kim@lge.com for v3 lcd*/	

#ifdef CONFIG_FB_MSM_MIPI_DSI
	mipi_dsi_regulator_init();
#endif	
}
