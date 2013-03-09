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

#define USE_HW_VSYNC

static struct msm_panel_info pinfo;

static struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db = {
#if 0	/* DSI_BIT_CLK at 380MHz, 1 lane, RGB888 */
	/* regulator */
	{0x03, 0x01, 0x01, 0x00},
	/* timing   */
	{0xab, 0x8a, 0x17, 0x00, 0x92, 0x91, 0x1b, 0x8c, 
	0x12, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xee, 0x00, 0x86, 0x00},
	/* pll control */
	{0x40, 0x77, 0x31, 0xd2, 0x00, 0x50, 0x48, 0x63,
	/* default set to 1 lane */
	0x01, 0x0f, 0x0f,
	0x05, 0x14, 0x03, 0x00, 0x00, 0x54, 0x06, 0x10, 0x04, 0x00},
#elif 0 //before 02-23
	/* DSI_BIT_CLK at 293MHz, 1 lane, RGB888 */
	{0x03, 0x01, 0x01, 0x00},	/* regulator */
	/* timing   */
	{0x22, 0x0c, 0x07, 0x00, 0x10, 0x20, 0x10,
	0x0d, 0x08, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xee, 0x00, 0x06, 0x00},
	/* pll control */
	{0x40, 0x9b, 0xb1, 0xda, 0x00, 0x50, 0x48, 0x63,
	/* set to 1 lane */
	0x01, 0x0f, 0x0f,
	0x05, 0x14, 0x03, 0x00, 0x00, 0x54, 0x06, 0x10, 0x04, 0x00},
#elif 0 //after 02-23 tuning
	/* DSI_BIT_CLK at 293MHz, 1 lane, RGB888, Tclk_trail, Tclk_prepare+Tclk_zero */
	{0x03, 0x01, 0x01, 0x00},	/* regulator */
	/* timing	*/
	{0x47, 0x16, 0x0f, 0x00, 0x2a, 0x20, 0x16,
	0x17, 0x0e, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xee, 0x00, 0x06, 0x00},
	/* pll control */
	{0x40, 0x9b, 0xb1, 0xda, 0x00, 0x50, 0x48, 0x63,
	/* set to 1 lane */
	0x01, 0x0f, 0x0f,
	0x05, 0x14, 0x03, 0x00, 0x00, 0x54, 0x06, 0x10, 0x04, 0x00},
#elif 0  //02_27 result tuing
	/* DSI_BIT_CLK at 320MHz, 1 lane, RGB888, Tclk_trail, Tclk_prepare+Tclk_zero */
	{0x03, 0x01, 0x01, 0x00},	/* regulator */
	/* timing	*/
	{0x4e, 0x18, 0x11, 0x00, 0x2e, 0x20, 0x18,
	0x19, 0x0f, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xee, 0x00, 0x06, 0x00},
	/* pll control */
	{0x40, 0xc1, 0xb1, 0xda, 0x00, 0x50, 0x48, 0x63,
	/* set to 1 lane */
	0x01, 0x0f, 0x0f,
	0x05, 0x14, 0x03, 0x00, 0x00, 0x54, 0x06, 0x10, 0x04, 0x00},
#elif 0 //03_07 result tuing [ because of cold environment issue , rollback]
	/* DSI_BIT_CLK at 293MHz, 1 lane, RGB888, Tclk_trail, Tclk_prepare+Tclk_zero */
	{0x03, 0x01, 0x01, 0x00},	/* regulator */
	/* timing	*/
	{0x47, 0x16, 0x0f, 0x00, 0x2a, 0x20, 0x16,
	0x17, 0x0e, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xee, 0x00, 0x06, 0x00},
	/* pll control */
	{0x40, 0x9b, 0xb1, 0xda, 0x00, 0x50, 0x48, 0x63,
	/* set to 1 lane */
	0x01, 0x0f, 0x0f,
	0x05, 0x14, 0x03, 0x00, 0x00, 0x54, 0x06, 0x10, 0x04, 0x00},
#else  //05_26 result tuing [ because of T-EXIT Margin for BTA]
	/* DSI_BIT_CLK at 293MHz, 1 lane, RGB888, Tclk_trail, Tclk_prepare+Tclk_zero */
	{0x03, 0x01, 0x01, 0x00},	/* regulator */
	/* timing	*/
	{0x47, 0x16, 0x0f, 0x00, 0x46, 0x20, 0x16,
	0x17, 0x0e, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xee, 0x00, 0x06, 0x00},
	/* pll control */
	{0x40, 0x9b, 0xb1, 0xda, 0x00, 0x50, 0x48, 0x63,
	/* set to 1 lane */
	0x01, 0x0f, 0x0f,
	0x05, 0x14, 0x03, 0x00, 0x00, 0x54, 0x06, 0x10, 0x04, 0x00},
#endif
};

static int __init mipi_cmd_ili9486_hvga_pt_init(void)
{
	int ret;

#ifdef CONFIG_FB_MSM_MIPI_PANEL_DETECT
	if (msm_fb_detect_client("mipi_cmd_ili9486_hvga"))
		return 0;
#endif

	pinfo.xres = 320;
	pinfo.yres = 480;
	pinfo.type = MIPI_CMD_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	pinfo.lcdc.h_back_porch = 20;
	pinfo.lcdc.h_front_porch = 40;
	pinfo.lcdc.h_pulse_width = 4;
	pinfo.lcdc.v_back_porch = 8;
	pinfo.lcdc.v_front_porch = 8;
	pinfo.lcdc.v_pulse_width = 4;

	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 100;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;

	pinfo.clk_rate =293000000;

#ifdef USE_HW_VSYNC
	pinfo.lcd.vsync_enable = TRUE;
	pinfo.lcd.hw_vsync_mode = TRUE;
#endif
	pinfo.lcd.refx100 = 6150; /* adjust refx100 to prevent tearing */

	pinfo.mipi.mode = DSI_CMD_MODE;
	pinfo.mipi.dst_format = DSI_CMD_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = FALSE;
	pinfo.mipi.data_lane2 = FALSE;
	pinfo.mipi.data_lane3 = FALSE;
	pinfo.mipi.t_clk_post = 0x1E;
	pinfo.mipi.t_clk_pre = 0x27;
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
#ifdef USE_HW_VSYNC
	pinfo.mipi.te_sel = 1; /* TE from vsync gpio */
#else
	pinfo.mipi.te_sel = 0; /* TE from vsync gpio */
#endif
	pinfo.mipi.interleave_max = 1;
	pinfo.mipi.insert_dcs_cmd = TRUE;
	pinfo.mipi.wr_mem_continue = 0x3c;
	pinfo.mipi.wr_mem_start = 0x2c;
	pinfo.mipi.dsi_phy_db = &dsi_cmd_mode_phy_db;	
	pinfo.mipi.tx_eot_append = 0x01;
	pinfo.mipi.rx_eot_ignore = 0;
	pinfo.mipi.dlane_swap = 0x01;

	ret = mipi_ili9486_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_FWVGA_PT);
	if (ret)
		pr_err("%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_cmd_ili9486_hvga_pt_init);

