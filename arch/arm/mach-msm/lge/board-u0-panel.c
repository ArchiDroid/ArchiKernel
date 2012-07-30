#include <linux/err.h>
#include <linux/gpio.h>

#include <asm/mach-types.h>

#include <mach/msm_rpcrouter.h>
#include <mach/rpc_pmapp.h>
#include <mach/board.h>

#include "devices.h"
#include "board-u0.h"
#include <mach/board_lge.h>
/* LGE_CHANGE_S : lcd regulator patch
 * 2011-12-21, sinjo.mattappallil@lge.com,
 * vreg is converted to regulator framework.
 */
//#include <mach/vreg.h>
#include <linux/regulator/consumer.h>
/* LGE_CHANGE_E : lcd regulator patch */
#ifdef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
#include "../clock-pcom.h"
#include "../proc_comm.h"
#endif
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

/* LGE_CHANGE_S [wonjung.yun@lge.com] 20111110 : Edit Device mipi_lg4572b to mipi_lg4573b*/
static struct platform_device mipi_dsi_lg4573b_panel_device = {
	.name = "mipi_lg4573b",
	.id = 0,
};
/* LGE_CHANGE_E [wonjung.yun@lge.com] 20111110 */

/* input platform device */
/* LGE_CHANGE_S [wonjung.yun@lge.com] 20111110 : Edit Device mipi_lg4572b to mipi_lg4573b */
static struct platform_device *u0_panel_devices[] __initdata = {
	&mipi_dsi_lg4573b_panel_device,
};
/* LGE_CHANGE_E [wonjung.yun@lge.com] 20111110 */

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = 97,						/*LCD_VSYNC_P*/
	.mdp_rev = MDP_REV_303,
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
static int dsi_gpio_initialized;
static int Isfirstbootend;

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
/* LGE_CHANGE_S : lcd regulator patch
 * 2011-12-21, sinjo.mattappallil@lge.com,
 * vreg is converted to regulator framework.
 */
static struct regulator *regulator_mipi_dsi[2];

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

        rc = regulator_set_voltage(regulator_mipi_dsi[1], 2800000,2800000);
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
	printk("mipi_dsi_panel_power : %d \n", on);

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
/* LGE_CHANGE_E : lcd regulator patch */

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

static void __init msm_fb_add_devices(void)
{
	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("lcdc", 0);
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
}

void __init msm7x27a_u0_init_i2c_backlight(int bus_num)
{
	bl_i2c_device.id = bus_num;
	bl_i2c_bdinfo[0].platform_data = &lm3530bl_data;

	#if 1
	bl_i2c_pin.scl_pin = 111;
	bl_i2c_pin.sda_pin = 112;
	#else
	if (lge_bd_rev == LGE_REV_A) {
		bl_i2c_pin.scl_pin = 122;
		bl_i2c_pin.sda_pin = 123;
	} else if (lge_bd_rev >= LGE_REV_B) {
		bl_i2c_pin.scl_pin = 111;
		bl_i2c_pin.sda_pin = 112;
	}
	#endif

	/* workaround for HDK rev_a no pullup */
	lge_init_gpio_i2c_pin_pullup(&bl_i2c_pdata, bl_i2c_pin, &bl_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &bl_i2c_bdinfo[0], 1);
	platform_device_register(&bl_i2c_device);
}

void __init lge_add_lcd_devices(void)
{
	platform_add_devices(u0_panel_devices, ARRAY_SIZE(u0_panel_devices));
	msm_fb_add_devices();
	lge_add_gpio_i2c_device(msm7x27a_u0_init_i2c_backlight);
        mipi_dsi_regulator_init();
}

