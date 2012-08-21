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
#include "mipi_r61529.h"
#include <asm/gpio.h>
#include <mach/vreg.h>
#include <mach/board_lge.h>

#define R61529_CMD_DELAY 0 /* 50 */

static void mipi_ldp_lcd_panel_poweroff(void);

static struct msm_panel_common_pdata *mipi_r61529_pdata;

static struct dsi_buf r61529_tx_buf;
static struct dsi_buf r61529_rx_buf;

static char config_mcap[2] = {0xb0, 0x04};
static char config_set_tear_on[2] = {0x35, 0x00};
static char config_set_tear_scanline[3] = {0x44, 0x00, 0x00};
static char config_set_addr[2] = {0x36, 0x08};
static char config_pixel_format[2] = {0x3a, 0x77};
static char config_column_addr[5] = {0x2a, 0x00, 0x00, 0x01, 0x3f};
static char config_page_addr[5] = {0x2b, 0x00, 0x00, 0x01, 0xdf};
static char config_frame_mem_if[5] = {0xb3, 0x02, 0x00, 0x00, 0x00};
static char config_panel_drv[9] = {0xc0, 0x01, 0xdf, 0x40, 0x10, 0x00, 0x01, 0x00, 0x33};
static char config_display_timing[6] = {0xc1, 0x07, 0x27, 0x08, 0x08, 0x10};
static char config_src_gate_timing[5] = {0xc4, 0x77, 0x00, 0x03, 0x01};
static char config_dpi_polarity[2] = {0xc6, 0x00};
static char config_gamma_a[25] = {0xc8, 0x00, 0x10, 0x18, 0x24, 0x2f, 0x48, 0x38, 0x24,
								  0x18, 0x0e, 0x06, 0x00, 0x00, 0x10, 0x18, 0x24,
								  0x2f, 0x48, 0x38, 0x24, 0x18, 0x0e, 0x06, 0x00};
static char config_gamma_b[25] = {0xc9, 0x00, 0x10, 0x18, 0x24, 0x2f, 0x48, 0x38, 0x24,
								  0x18, 0x0e, 0x06, 0x00, 0x00, 0x10, 0x18, 0x24,
								  0x2f, 0x48, 0x38, 0x24, 0x18, 0x0e, 0x06, 0x00};
static char config_gamma_c[25] = {0xca, 0x00, 0x10, 0x18, 0x24, 0x2f, 0x48, 0x38, 0x24,
								  0x18, 0x0e, 0x06, 0x00, 0x00, 0x10, 0x18, 0x24,
								  0x2f, 0x48, 0x38, 0x24, 0x18, 0x0e, 0x06, 0x00};
static char config_power_chg_pump[17] = {0xd0, 0xA9, 0x06, 0x08, 0x20, 0x31, 0x04, 0x01,
								  0x00, 0x08, 0x01, 0x00, 0x06, 0x01, 0x00, 0x00,
								  0x20};
static char config_vcom[5] = {0xd1, 0x02, 0x22, 0x22, 0x33};
static char config_backlight_ctrl1[21] = {0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										  0x00, 0x00, 0x00, 0x00, 0x00};
static char config_backlight_ctrl2[5] = {0xb9, 0x00, 0x00, 0x00, 0x00};
/*static char config_backlight_ctrl3[3] = {0xba, 0x00, 0x00};*/

static char config_nvm_access[5] = {0xe0, 0x00, 0x00, 0x00, 0x00};
static char config_ddb_write[7] = {0xe1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static char config_nvm_load[2] = {0xe2, 0x80};
static char config_write_memory[2] = {0x2c, 0x00};

/*---------------------- display_on ----------------------------*/
static char disp_sleep_out[1] = {0x11};
static char disp_display_on[1] = {0x29};

/*---------------------- sleep_mode_on ----------------------------*/
static char sleep_display_off[1] = {0x28};
static char sleep_mode_on[1] = {0x10};
static char sleep_mcap[2] = {0xb0, 0x00};
static char sleep_low_power_mode[2] = {0xb1, 0x01};

static struct dsi_cmd_desc r61529_init_on_cmds[] = {
	{DTYPE_GEN_WRITE2, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_mcap), config_mcap},
	{DTYPE_DCS_WRITE1, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_set_tear_on), config_set_tear_on},
	{DTYPE_DCS_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_set_tear_scanline), config_set_tear_scanline},
	{DTYPE_DCS_WRITE1, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_set_addr), config_set_addr},
	{DTYPE_DCS_WRITE1, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_pixel_format), config_pixel_format},
	{DTYPE_DCS_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_column_addr), config_column_addr},
	{DTYPE_DCS_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_page_addr), config_page_addr},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_frame_mem_if), config_frame_mem_if},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_panel_drv), config_panel_drv},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_display_timing), config_display_timing},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_src_gate_timing), config_src_gate_timing},
	{DTYPE_GEN_WRITE2, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_dpi_polarity), config_dpi_polarity},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_gamma_a), config_gamma_a},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_gamma_b), config_gamma_b},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_gamma_c), config_gamma_c},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_power_chg_pump), config_power_chg_pump},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_vcom), config_vcom},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_backlight_ctrl1), config_backlight_ctrl1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_backlight_ctrl2), config_backlight_ctrl2},
/*	{DTYPE_GEN_READ1, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_backlight_ctrl3), config_backlight_ctrl3},*/
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_nvm_access), config_nvm_access},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_ddb_write), config_ddb_write},
	{DTYPE_GEN_WRITE2, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_nvm_load), config_nvm_load},
	{DTYPE_DCS_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(config_write_memory), config_write_memory}
};

static struct dsi_cmd_desc r61529_disp_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 40,
		sizeof(disp_display_on), disp_display_on}
};

static struct dsi_cmd_desc r61529_disp_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 40,
		sizeof(sleep_display_off), sleep_display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 100,
		sizeof(sleep_mode_on), sleep_mode_on},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(sleep_mcap), sleep_mcap},
	{DTYPE_GEN_LWRITE, 1, 0, 0, R61529_CMD_DELAY,
		sizeof(sleep_low_power_mode), sleep_low_power_mode}
};

static struct dsi_cmd_desc r61529_sleep_out_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(disp_sleep_out), disp_sleep_out},
};

static int mipi_r61529_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;

	mfd = platform_get_drvdata(pdev);
	mipi  = &mfd->panel_info.mipi;

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	printk("mipi_r61529_lcd_on START\n");

	mipi_dsi_cmds_tx(mfd, &r61529_tx_buf, r61529_sleep_out_cmds,
			ARRAY_SIZE(r61529_sleep_out_cmds));
	mipi_set_tx_power_mode(1);
	mipi_dsi_cmds_tx(mfd, &r61529_tx_buf, r61529_init_on_cmds,
			ARRAY_SIZE(r61529_init_on_cmds));

	mipi_dsi_cmds_tx(mfd, &r61529_tx_buf, r61529_disp_on_cmds,
			ARRAY_SIZE(r61529_disp_on_cmds));

	mipi_set_tx_power_mode(0);

	printk("mipi_r61529_lcd_on FINISH\n");
	return 0;
}

static int mipi_r61529_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	printk("mipi_r61529_lcd_off START\n");

	mipi_dsi_cmds_tx(mfd, &r61529_tx_buf, r61529_disp_off_cmds,
			ARRAY_SIZE(r61529_disp_off_cmds));

	mipi_ldp_lcd_panel_poweroff();

	return 0;
}

ssize_t mipi_r61529_lcd_show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("%s : start\n", __func__);
	return 0;
}

ssize_t mipi_r61529_lcd_store_onoff(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/*struct platform_device dummy_pdev;*/
	int onoff;
	struct platform_device *pdev = to_platform_device(dev);

	sscanf(buf, "%d", &onoff);
	printk("%s: onoff : %d\n", __func__, onoff);
	
	if (onoff) {
		mipi_r61529_lcd_on(pdev);
	} else {
		mipi_r61529_lcd_off(pdev);
	}
	return count;
}

DEVICE_ATTR(lcd_onoff, 0664, mipi_r61529_lcd_show_onoff, mipi_r61529_lcd_store_onoff);


static int __devinit mipi_r61529_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;

	if (pdev->id == 0) {
		mipi_r61529_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);
	/*this for AT Command*/
	rc = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_r61529_lcd_probe,
	.driver = {
		.name   = "mipi_r61529",
	},
};

static struct msm_fb_panel_data r61529_panel_data = {
	.on		= mipi_r61529_lcd_on,
	.off	= mipi_r61529_lcd_off,
};

static int ch_used[3];

int mipi_r61529_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;
	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_r61529", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	r61529_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &r61529_panel_data,
		sizeof(r61529_panel_data));
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

static int __init mipi_r61529_lcd_init(void)
{
	mipi_dsi_buf_alloc(&r61529_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&r61529_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

static void mipi_ldp_lcd_panel_poweroff(void)
{
        /* stay GPIO_LCD_RESET high in deep sleep*/
	/*gpio_set_value(GPIO_LCD_RESET, 0);*/
	msleep(10);
}


module_init(mipi_r61529_lcd_init);
