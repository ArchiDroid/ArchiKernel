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
#include "mipi_lg4572b.h"
#include <linux/gpio.h>
#include <mach/vreg.h>
#include <mach/board_lge.h>

#define LG4572B_CMD_DELAY  0

static void mipi_ldp_lcd_panel_poweroff(void);

static struct msm_panel_common_pdata *mipi_lg4572b_pdata;

static struct dsi_buf lg4572b_tx_buf;
static struct dsi_buf lg4572b_rx_buf;

/*LGE_CHANGE_S [yoonsoo.kim@lge.com] 20110906: Justine LCD porting*/
static char config_mipi[] = {0x03,0x00};	/* MIPI DSI config */
static char config_display_inversion[] = {0x20};	/* Display Inversion */
static char config_tearing_effect[] = {0x35,0x00};	 /* tearing effect  */
static char config_panel_char_setting[] = {0xB2, 0x00, 0xC8};	 /* Panel Characteristics Setting */
static char config_panel_drv_setting[] = {0xB3, 0x00};	 /* Panel Drive Setting */
static char config_display_mode_ctl[] = {0xB4, 0x04};	/* Display Mode Control */
static char config_display_ctl_1[] = {0xB5, 0x42, 0x10, 0x10, 0x00, 0x20};	 /* Display Control 1 */
static char config_display_ctl_2[] = {0xB6, 0x0B, 0x0F, 0x3C, 0x13, 0x13, 0xE8};	 /* Display Control 2 */
static char config_display_ctl_3[] = {0xB7, 0x49, 0x06, 0x1F, 0x00, 0x00};	 /* Display Control 3 */
static char config_oscil_set[] = {0xC0, 0x01, 0x11};	/* oscilator setting */
static char config_pwr_ctl_3[] = {0xC3, 0x07, 0x03, 0x04, 0x04, 0x04};	/* Power Control 3 */
static char config_pwr_ctl_4[] = {0xC4, 0x12, 0x24, 0x18, 0x18, 0x05, 0x49};	/* Power Control 4 */
static char config_pwr_ctl_5[] = {0xC5, 0x6B};	/* Power Control 5 */
static char config_pwr_ctl_6[] = {0xC6, 0x41, 0x63};	/* Power Control 6 */

static char config_gamma_r_pos[] = {0xD0, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03}; /* Positive Gamma Curve for Red */
static char config_gamma_r_neg[] = {0xD1, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03}; /* Negative Gamma Curve for Red */
static char config_gamma_g_pos[] = {0xD2, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03}; /* Positive Gamma Curve for Green */
static char config_gamma_g_neg[] = {0xD3, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03}; /* Negative Gamma Curve for Green */
static char config_gamma_b_pos[] = {0xD4, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03}; /* Positive Gamma Curve for Blue */
static char config_gamma_b_neg[] = {0xD5, 0x00, 0x16, 0x62, 0x35, 0x02, 0x00, 0x30, 0x00, 0x03}; /* Negative Gamma Curve for Blue */	

/*---------------------- display_on ----------------------------*/
static char disp_sleep_out[1] = {0x11};
static char disp_display_on[1] = {0x29};

/*---------------------- sleep_mode_on ----------------------------*/
static char sleep_display_off[1] = {0x28};
static char sleep_mode_on[1] = {0x10};


#define DTYPE_DCS_WRITE		0x05	/* short write, 0 parameter */
#define DTYPE_DCS_WRITE1	0x15	/* short write, 1 parameter */
#define DTYPE_DCS_READ		0x06	/* read */
#define DTYPE_DCS_LWRITE	0x39	/* long write */

static struct dsi_cmd_desc lg4572b_init_on_cmds[] = {
	{DTYPE_GEN_WRITE2, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_mipi), config_mipi},
	{DTYPE_DCS_WRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_display_inversion), config_display_inversion},
	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_tearing_effect), config_tearing_effect},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_panel_char_setting), config_panel_char_setting},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_panel_drv_setting), config_panel_drv_setting},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_display_mode_ctl), config_display_mode_ctl},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_display_ctl_1), config_display_ctl_1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_display_ctl_2), config_display_ctl_2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_display_ctl_3), config_display_ctl_3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_oscil_set),config_oscil_set},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_pwr_ctl_3), config_pwr_ctl_3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_pwr_ctl_4), config_pwr_ctl_4},
	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_pwr_ctl_5), config_pwr_ctl_5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,	
		sizeof(config_pwr_ctl_6), config_pwr_ctl_6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_gamma_r_pos), config_gamma_r_pos},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_gamma_r_neg), config_gamma_r_neg},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_gamma_g_pos), config_gamma_g_pos},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_gamma_g_neg), config_gamma_g_neg},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_gamma_b_pos), config_gamma_b_pos},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_gamma_b_neg), config_gamma_b_neg},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(config_gamma_b_neg), config_gamma_b_neg}
};

static struct dsi_cmd_desc lg4572b_disp_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 40,
		sizeof(disp_display_on), disp_display_on}
};

static struct dsi_cmd_desc lg4572b_disp_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 40,
		sizeof(sleep_display_off), sleep_display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, LG4572B_CMD_DELAY,
		sizeof(sleep_mode_on), sleep_mode_on},
};

static struct dsi_cmd_desc lg4572b_sleep_out_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(disp_sleep_out), disp_sleep_out},
};

static int mipi_lg4572b_lcd_on(struct platform_device *pdev)
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
	
	printk( "mipi_lg4572b_lcd_on START\n");

	mipi_set_tx_power_mode(1);

	result=mipi_dsi_cmds_tx(mfd, &lg4572b_tx_buf, lg4572b_init_on_cmds,
			ARRAY_SIZE(lg4572b_init_on_cmds));

	mdelay(10);
	
	result=mipi_dsi_cmds_tx(mfd, &lg4572b_tx_buf, lg4572b_sleep_out_cmds,
			ARRAY_SIZE(lg4572b_sleep_out_cmds));


	mdelay(120);
	
	result=mipi_dsi_cmds_tx(mfd, &lg4572b_tx_buf, lg4572b_disp_on_cmds,
			ARRAY_SIZE(lg4572b_disp_on_cmds));

	mipi_set_tx_power_mode(0);

	printk( "mipi_lg4572b_lcd_on FINISH\n");
	return 0;
}

static int mipi_lg4572b_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	printk(KERN_INFO "mipi_lg4572b_chip_lcd_off START\n");

	//display off
	mipi_dsi_cmds_tx(mfd, &lg4572b_tx_buf, lg4572b_disp_off_cmds,
			ARRAY_SIZE(lg4572b_disp_off_cmds));

	mipi_ldp_lcd_panel_poweroff();

	return 0;
}

ssize_t mipi_lg4572b_lcd_show_onoff(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	printk( "%s : start\n", __func__);
	return 0;
}

ssize_t mipi_lg4572b_lcd_store_onoff(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	/*struct platform_device dummy_pdev;*/
	int onoff;

	sscanf(buf, "%d", &onoff);
	printk( "%s: onoff : %d\n", __func__, onoff);
	if (onoff)
		mipi_lg4572b_lcd_on(NULL);
	else
		mipi_lg4572b_lcd_off(NULL);

	return count;
}

DEVICE_ATTR(lcd_onoff, 0664, mipi_lg4572b_lcd_show_onoff,
	mipi_lg4572b_lcd_store_onoff);

static int __devinit mipi_lg4572b_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;

	if (pdev->id == 0) {
		mipi_lg4572b_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);
	/*this for AT Command*/
	rc = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);
	
	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_lg4572b_lcd_probe,
	.driver = {
		.name   = "mipi_lg4572b",
	},
};

static struct msm_fb_panel_data lg4572b_panel_data = {
	.on		= mipi_lg4572b_lcd_on,
	.off	= mipi_lg4572b_lcd_off,
};

static int ch_used[3];

int mipi_lg4572b_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;
	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_lg4572b", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	lg4572b_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &lg4572b_panel_data,
		sizeof(lg4572b_panel_data));
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


static int __init mipi_lg4572b_lcd_init(void)
{
	int bufsize=0;
	
	bufsize=mipi_dsi_buf_alloc(&lg4572b_tx_buf, DSI_BUF_SIZE);
	
	bufsize=mipi_dsi_buf_alloc(&lg4572b_rx_buf, DSI_BUF_SIZE);
	
	return platform_driver_register(&this_driver);
}

static void mipi_ldp_lcd_panel_poweroff(void)
{

	gpio_set_value(GPIO_U0_LCD_RESET, 0);  /* LGE_CHANGE  [yoonsoo.kim@lge.com] 20110906: LCD Pinname */
	msleep(20);
}

module_init(mipi_lg4572b_lcd_init);
