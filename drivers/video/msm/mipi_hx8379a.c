/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_hx8379a.h"
#include <linux/gpio.h>

#include <mach/vreg.h>
//#include <mach/board_lge.h>
#include CONFIG_LGE_BOARD_HEADER_FILE
//LGE_S, sohyun.name@lge.com porting tovis panel, 12-12-26
#include <linux/module.h> // export_symbol
//LGE_E, sohyun.name@lge.com porting tovis panel, 12-12-26

#define HX8479A_CMD_DELAY  0

#ifndef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
//static boolean lglogo_firstboot = TRUE;
#endif

/*LGE_CHANGE_S: Kiran.kanneganti@lge.com 05-03-2012*/
/*LCD Reset After data pulled Down*/
void mipi_ldp_lcd_hx8379a_panel_poweroff(void);
/*LGE_CHANGE_E LCD Reset After Data Pulled Down*/

static struct msm_panel_common_pdata *mipi_hx8379a_pdata;

static struct dsi_buf hx8379a_tx_buf;
static struct dsi_buf hx8379a_rx_buf;

static char config_set_extc[] = {
	0xB9, 0xFF, 0x83, 0x79
};
static char config_set_mipi[] = {
	0xBA, 0x51
};
static char config_set_power[] = {
	0xB1, 0x00, 0x50, 0x24, 0xEA, 0x90, 0x08, 0x11,
	0x11, 0x71, 0x2C, 0x34, 0x9F, 0x1F, 0x42, 0x0B,
	0x6E, 0xF1, 0x00, 0xE6
};
static char config_set_display[] = {
	0xB2, 0x00, 0x00, 0x3C, 0x08, 0x0C, 0x19, 0x22,
	0x00, 0xFF, 0x08, 0x0C, 0x19, 0x20
};
static char config_set_cyc[] = {
	0xB4, 0x80, 0x08, 0x00, 0x32, 0x10, 0x05, 0x32,
	0x13, 0x31, 0x32, 0x10, 0x08, 0x37, 0x01, 0x20,
	0x07, 0x37, 0x08, 0x28, 0x08, 0x30, 0x30, 0x04,
	0x00, 0x40, 0x08, 0x28, 0x08, 0x30, 0x30, 0x04
};
static char config_set_gip[] = {
	0xD5, 0x00, 0x00, 0x0A, 0x00, 0x01, 0x05, 0x00,
	0x0A, 0x00, 0x88, 0x88, 0x89, 0x88, 0x23, 0x01,
	0x67, 0x45, 0x88, 0x01, 0x88, 0x45, 0x88, 0x88,
	0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x54, 0x76,
	0x10, 0x32, 0x88, 0x54, 0x88, 0x10, 0x88, 0x88,
	0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static char config_set_gamma[] = {
	0xE0, 0x79, 0x00, 0x06, 0x09, 0x3A, 0x3A, 0x3E,
	0x15, 0x45, 0x04, 0x0A, 0x0D, 0x11, 0x14, 0x13,
	0x15, 0x15, 0x1A, 0x00, 0x06, 0x09, 0x3A, 0x3A,
	0x3E, 0x15, 0x45, 0x04, 0x0A, 0x0D, 0x12, 0x14,
	0x13, 0x14, 0x15, 0x1A
};
static char config_set_panel_gip_forward_scan[] = {
	0xCC, 0x02
};
static char config_set_vcom[] = {
	0xB6, 0x00, 0xA0, 0x00, 0xA0
};

/*---------------------- display_on ----------------------------*/
static char disp_sleep_out[] = {0x11,0x00};
static char disp_display_on[] = {0x29,0x00};

/*---------------------- sleep_mode_on ----------------------------*/
static char sleep_display_off[1] = {0x28};
static char sleep_mode_on[1] = {0x10};

/*---------------------- dtype ----------------------------*/
#define DTYPE_DCS_WRITE		0x05	/* short write, 0 parameter */
#define DTYPE_DCS_WRITE1	0x15	/* short write, 1 parameter */
#define DTYPE_DCS_READ		0x06	/* read */
#define DTYPE_DCS_LWRITE	0x39	/* long write */

static struct dsi_cmd_desc hx8379a_init_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, HX8479A_CMD_DELAY,
		sizeof(config_set_extc), config_set_extc}, 
	{DTYPE_DCS_LWRITE, 1, 0, 0, HX8479A_CMD_DELAY,
		sizeof(config_set_mipi), config_set_mipi},
	{DTYPE_DCS_LWRITE, 1, 0, 0, HX8479A_CMD_DELAY,
		sizeof(config_set_power), config_set_power},
	{DTYPE_DCS_LWRITE, 1, 0, 0, HX8479A_CMD_DELAY,
		sizeof(config_set_display), config_set_display},
	{DTYPE_DCS_LWRITE, 1, 0, 0, HX8479A_CMD_DELAY,
		sizeof(config_set_cyc), config_set_cyc},
	{DTYPE_DCS_LWRITE, 1, 0, 0, HX8479A_CMD_DELAY,
		sizeof(config_set_gip), config_set_gip},
	{DTYPE_DCS_LWRITE, 1, 0, 0, HX8479A_CMD_DELAY,
		sizeof(config_set_gamma), config_set_gamma},
	{DTYPE_DCS_LWRITE, 1, 0, 0, HX8479A_CMD_DELAY,
		sizeof(config_set_panel_gip_forward_scan), config_set_panel_gip_forward_scan},
	{DTYPE_DCS_LWRITE, 1, 0, 0, HX8479A_CMD_DELAY,
		sizeof(config_set_vcom), config_set_vcom},
};

static struct dsi_cmd_desc hx8379a_sleep_out_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 150,
		sizeof(disp_sleep_out), disp_sleep_out},
};
//wait 80ms 
static struct dsi_cmd_desc hx8379a_disp_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 10,
		sizeof(disp_display_on), disp_display_on}
};

static struct dsi_cmd_desc hx8379a_disp_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 50,
		sizeof(sleep_display_off), sleep_display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, /*HX8479A_CMD_DELAY*/10,
		sizeof(sleep_mode_on), sleep_mode_on},
};

#if defined(CONFIG_MACH_MSM8X25_V7)
extern int lcd_on_completed;
#endif
static int mipi_hx8379a_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
	int result=0;

	mfd = platform_get_drvdata(pdev);
	mipi  = &mfd->panel_info.mipi;

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
#if 0
#ifndef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
	if(!lglogo_firstboot)
#endif
	{
#endif
		printk( "mipi_hx8379a_lcd_on START\n");

		mipi_set_tx_power_mode(1);

		result=mipi_dsi_cmds_tx(&hx8379a_tx_buf, hx8379a_init_on_cmds,
				ARRAY_SIZE(hx8379a_init_on_cmds));

		msleep(80);
		
		result=mipi_dsi_cmds_tx(&hx8379a_tx_buf, hx8379a_sleep_out_cmds,
				ARRAY_SIZE(hx8379a_sleep_out_cmds));
#if 0
		/*[LGSI_SP4_BSP_BEGIN] [kiran.jainapure@lge.com]: Sometimes display is blank or distorted during bootlogo*/
		if(lglogo_firstboot){
			mdelay(120);
		}
		/*[LGSI_SP4_BSP_END] [kiran.jainapure@lge.com]*/
#endif	
		result=mipi_dsi_cmds_tx(&hx8379a_tx_buf, hx8379a_disp_on_cmds,
				ARRAY_SIZE(hx8379a_disp_on_cmds));

		mipi_set_tx_power_mode(0);

#if defined(CONFIG_MACH_MSM8X25_V7)
		lcd_on_completed = 1;
#endif

		printk( "mipi_hx8379a_lcd_on FINISH\n");
//	}
	return 0;
}

static int mipi_hx8379a_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	printk(KERN_INFO "mipi_hx8379a_chip_lcd_off START\n");
		
	mipi_dsi_cmds_tx(&hx8379a_tx_buf, hx8379a_disp_off_cmds,
		ARRAY_SIZE(hx8379a_disp_off_cmds));

	msleep(50);
	
	
	return 0;
}

ssize_t mipi_hx8379a_lcd_show_onoff(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	printk( "%s : start\n", __func__);
	return 0;
}

ssize_t mipi_hx8379a_lcd_store_onoff(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	/*struct platform_device dummy_pdev;*/
	int onoff;

	sscanf(buf, "%d", &onoff);
	printk( "%s: onoff : %d\n", __func__, onoff);
	if (onoff)
		mipi_hx8379a_lcd_on(NULL);
	else
		mipi_hx8379a_lcd_off(NULL);

	return count;
}
DEVICE_ATTR(lcd_hx8379a_onoff, 0664, mipi_hx8379a_lcd_show_onoff,
	mipi_hx8379a_lcd_store_onoff);
	

static int __devinit mipi_hx8379a_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;

	if (pdev->id == 0) {
		mipi_hx8379a_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);
	/*this for AT Command*/
	rc = device_create_file(&pdev->dev, &dev_attr_lcd_hx8379a_onoff);
	
	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_hx8379a_lcd_probe,
	.driver = {
		.name   = "mipi_hx8379a",
	},
};

static struct msm_fb_panel_data hx8379a_panel_data = {
	.on		= mipi_hx8379a_lcd_on,
	.off	= mipi_hx8379a_lcd_off,
};

static int ch_used[3];

int mipi_hx8379a_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;
	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_hx8379a", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	hx8379a_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &hx8379a_panel_data,
		sizeof(hx8379a_panel_data));
	if (ret) {
		pr_err("%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		pr_err("%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

extern unsigned int maker_id;
static int __init mipi_hx8379a_lcd_init(void)
{
	int bufsize=0;
	
	bufsize=mipi_dsi_buf_alloc(&hx8379a_tx_buf, DSI_BUF_SIZE);
	
	bufsize=mipi_dsi_buf_alloc(&hx8379a_rx_buf, DSI_BUF_SIZE);

	if( maker_id == 0)
		return platform_driver_register(&this_driver);
	else
		return 0;
}
/*LGE_CHANGE_S: Kiran.kanneganti@lge.com 05-03-2012*/
/*LCD Reset After data pulled Down*/
void mipi_ldp_lcd_hx8379a_panel_poweroff(void)
{
	printk("%s start \n", __func__);
// LGE_S, bohyun.jung@lge.com, 12-11-28 without this U0 JB does not go 1.8mA
#if 1
	gpio_set_value(GPIO_U0_LCD_RESET, 0);  /* LGE_CHANGE  [yoonsoo.kim@lge.com] 20110906: LCD Pinname */
	printk("%s LCD_RESET low.. \n", __func__);
	msleep(20);
#endif
// LGE_E, bohyun.jung@lge.com, 12-11-28 without this U0 JB does not go 1.8mA
	printk("%s end \n", __func__);
}
EXPORT_SYMBOL(mipi_ldp_lcd_hx8379a_panel_poweroff);
/*LGE_CHANGE_E LCD Reset After Data Pulled Down*/
module_init(mipi_hx8379a_lcd_init);
