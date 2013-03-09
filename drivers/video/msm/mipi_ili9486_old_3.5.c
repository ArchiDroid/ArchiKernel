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
#include "mipi_ili9486.h"
#include <asm/gpio.h>
#include <mach/vreg.h>
#include <mach/board_lge.h>

#define ILI9486_CMD_DELAY 0
#define ILI9486_DEBUG 0

static void mipi_ldp_lcd_panel_poweroff(void);

static struct msm_panel_common_pdata *mipi_ili9486_pdata;

static struct dsi_buf ili9486_tx_buf;
static struct dsi_buf ili9486_rx_buf;

#define LOG_MSG 1

//#if dfined(LOG_MSG)
static struct dsi_buf *ili9486_tx_buf_msg;
static char *ili9486_tmp;
//#endif



/* --------------------start configuration---------------------*/

static char config_mem_acc[2] = {0x36, 0x08}; //DCS
static char config_pixel_format[2]={0x3A,0x66}; //DCS
static char config_read_dispaly_MADCTL[3]={0x0B,0x00,0x00}; //Gen

static char config_positive_gamma_cont[16]={0xE0,0x00,0x16,0x1E,0x06,0x14,0x08,0x42,0x78,0x50,0x09,0x0F,0x0C,0x1B,0x1F,0x0F};  //Gen
static char config_negativ_gamma_correc[16]={0xE1,0x00,0x20,0x23,0x04,0x10,0x06,0x39,0x45,0x49,0x06,0x0F,0x0B,0x28,0x2B,0x0F}; //Gen

static char config_F7[6]={0xF7,0xA9,0x91,0x2D,0x8A,0x4F}; //Gen
static char config_F8[3]={0xF8,0x21,0x06}; //repair control //Gen

static char config_vcom[4]={0xC5,0x00,0x4E,0x80}; //Gen
static char config_power_ctrl_1[3]={0xC0,0x12,0x12}; //Gen
static char config_power_ctrl_2[2]={0xC1,0x42}; //Gen

static char config_display_func[4]={0xB6,0x02,0x22,0x3B};  //Gen
static char config_display_inversion_cont[2]={0xB4,0x02}; //Gen
static char config_frame_rate[3]={0xB1,0xB0,0x11}; //Gen
static char config_sleep_out[1]={0x11}; //DCS

static char config_display_on[1]={0x29}; //DCS

static char column_address_set[5]={0x2A, 0x00, 0x00, 0x01, 0x3F}; //DCS
static char page_address_set[5]={0x2B, 0x00, 0x00, 0x01, 0xDF}; //DCS

static char config_memory_write[1]={0x2C}; //DCS

static char config_display_off[1]={0x28}; //DCS
static char config_sleep_in[1]={0x10}; //DCS
/* --------------------end configuration-----------------------*/



/*----------------------start setting cmd----------------------*/

static struct dsi_cmd_desc ili9486_init_on_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_mem_acc), config_mem_acc},
	{DTYPE_DCS_WRITE1, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_pixel_format), config_pixel_format},
	{DTYPE_GEN_WRITE2, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_read_dispaly_MADCTL),config_read_dispaly_MADCTL },
	{DTYPE_GEN_LWRITE, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_positive_gamma_cont),config_positive_gamma_cont },
	{DTYPE_GEN_LWRITE, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_negativ_gamma_correc),config_negativ_gamma_correc },
	{DTYPE_GEN_LWRITE, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_F7), config_F7},
	{DTYPE_GEN_WRITE2, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_F8), config_F8},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_vcom), config_vcom},
	{DTYPE_GEN_WRITE2, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_power_ctrl_1),config_power_ctrl_1},
	{DTYPE_GEN_WRITE1, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_power_ctrl_2), config_power_ctrl_2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_display_func),config_display_func},
	{DTYPE_GEN_WRITE1, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_display_inversion_cont),config_display_inversion_cont},
	{DTYPE_GEN_WRITE2, 1, 0, 0, ILI9486_CMD_DELAY,
		sizeof(config_frame_rate),config_frame_rate}
};

static struct dsi_cmd_desc ili9486_sleep_out_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, 
		sizeof(config_sleep_out), config_sleep_out}
};

static struct dsi_cmd_desc ili9486_disp_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 10,
		sizeof(config_display_on), config_display_on}
};


static struct dsi_cmd_desc ili9486_column_add_set[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, ILI9486_CMD_DELAY,
			sizeof(column_address_set), column_address_set},
};

static struct dsi_cmd_desc ili9486_page_add_set[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, ILI9486_CMD_DELAY,
			sizeof(page_address_set),page_address_set },
};

static struct dsi_cmd_desc ili9486_memory_write_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, ILI9486_CMD_DELAY,
			sizeof(config_memory_write), config_memory_write},
};

static struct dsi_cmd_desc ili9486_disp_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 10,
		sizeof(config_display_off),config_display_off },
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(config_sleep_in), config_sleep_in}	
};

/*----------------------end setting cmd----------------------*/


static int mipi_ili9486_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;

	mfd = platform_get_drvdata(pdev);
	mipi  = &mfd->panel_info.mipi;

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	printk("mipi_ili9486_lcd_on START\n");
	mdelay(10);
	mipi_set_tx_power_mode(1);
	mipi_dsi_cmds_tx(mfd, &ili9486_tx_buf, ili9486_init_on_cmds,
			ARRAY_SIZE(ili9486_init_on_cmds));

#if ILI9486_DEBUG
	printk("mipi_ili9486_init_on_cmd ..\n");
#endif
	ili9486_tx_buf_msg=&ili9486_tx_buf;
        ili9486_tmp=ili9486_tx_buf_msg->data;

#if ILI9486_DEBUG
	printk("mipi_ili9486_init_on_cmd %s\n", ili9486_tmp);
	printk("ili9486_init_on_cmds: %0x3d\n", ili9486_tx_buf);
#endif	

	mipi_dsi_cmds_tx(mfd, &ili9486_tx_buf, ili9486_sleep_out_cmds,
			ARRAY_SIZE(ili9486_sleep_out_cmds));

#if ILI9486_DEBUG
	printk("mipi_ili9486_sleep_out_cmds..\n");
	printk("mipi_ili9486_init_on_cmd %s\n", ili9486_tmp);
#endif
	
        mipi_dsi_cmds_tx(mfd, &ili9486_tx_buf, ili9486_disp_on_cmds,
                        ARRAY_SIZE(ili9486_disp_on_cmds));
                        
#if ILI9486_DEBUG
	printk("mipi_ili9486_disp_on_cmd..\n");
	printk("mipi_ili9486_init_on_cmd %s\n", ili9486_tmp);
#endif

//start add cmds, ili9486_column_add_set, ili9486_column_add_set
	mipi_dsi_cmds_tx(mfd, &ili9486_tx_buf, ili9486_column_add_set, ARRAY_SIZE(ili9486_column_add_set));
	mipi_dsi_cmds_tx(mfd, &ili9486_tx_buf, ili9486_column_add_set, ARRAY_SIZE(ili9486_page_add_set));
//end add cmds

	mipi_dsi_cmds_tx(mfd, &ili9486_tx_buf, ili9486_memory_write_cmds,
			ARRAY_SIZE(ili9486_memory_write_cmds));
			
#if ILI9486_DEBUG
	printk("mipi_ili9486_memory_write_cmds..\n");
        printk("mipi_ili9486_init_on_cmd %s\n", ili9486_tmp);
#endif

	mipi_set_tx_power_mode(0);

	
	printk("mipi_ili9486_lcd_on FINISH\n");
	return 0;
}

static int mipi_ili9486_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	printk("mipi_ili9486_lcd_off START\n");

	mipi_dsi_cmds_tx(mfd, &ili9486_tx_buf, ili9486_disp_off_cmds,
			ARRAY_SIZE(ili9486_disp_off_cmds));

	mipi_ldp_lcd_panel_poweroff();

	return 0;
}

ssize_t mipi_ili9486_lcd_show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("%s : start\n", __func__);
	return 0;
}

ssize_t mipi_ili9486_lcd_store_onoff(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/*struct platform_device dummy_pdev;*/
	int onoff;
	struct platform_device *pdev = to_platform_device(dev);

	sscanf(buf, "%d", &onoff);
	printk("%s: onoff : %d\n", __func__, onoff);
	if (onoff) {
		mipi_ili9486_lcd_on(pdev);
	} else {
		mipi_ili9486_lcd_off(pdev);
	}
	return count;
}

DEVICE_ATTR(lcd_onoff, 0664, mipi_ili9486_lcd_show_onoff, mipi_ili9486_lcd_store_onoff);


static int __devinit mipi_ili9486_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;

	if (pdev->id == 0) {
		mipi_ili9486_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);
	/*this for AT Command*/
	rc = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_ili9486_lcd_probe,
	.driver = {
		.name   = "mipi_ili9486",
	},
};

static struct msm_fb_panel_data ili9486_panel_data = {
	.on		= mipi_ili9486_lcd_on,
	.off	= mipi_ili9486_lcd_off,
};

static int ch_used[3];

int mipi_ili9486_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;
	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_ili9486", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	ili9486_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &ili9486_panel_data,
		sizeof(ili9486_panel_data));
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

static int __init mipi_ili9486_lcd_init(void)
{
	mipi_dsi_buf_alloc(&ili9486_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&ili9486_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

static void mipi_ldp_lcd_panel_poweroff(void)
{
	gpio_set_value(GPIO_LCD_RESET, 0);
	mdelay(10);
}
module_init(mipi_ili9486_lcd_init);
