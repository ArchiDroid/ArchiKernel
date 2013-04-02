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
#include "mipi_lg4573b.h"


static struct msm_panel_info pinfo;

/* LGE_CHANGE_S : U0 Display Panel Framerate Control
 * 2012-01-14, yoonsoo@lge.com
 * U0 Display Panel Framerate Control 
 */
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db =
{

#if 0//defined(CONFIG_MACH_MSM8X25_V7) //QCT reccomend, youngbae.choi@lge.com, 2012-12-29
	/*63fps*/
	/* DSI Bit Clock at 335MHz, 2 lane, RGB888 */
	/* regulator */
	{0x03, 0x01, 0x01, 0x00},
	/* timing */
	{0xa8, 0x89, 0x16, 0x00, 0x90, 0x95, 0x19, 0x8b, 0x10, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xbb, 0x02, 0x06, 0x00},
	/* pll control */
	{0x00, 0x5d, 0x31, 0xD2, 0x00, 0x40, 0x37, 0x62, 0x01, 0x0F, 0x07, 
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0}, 
#else //QCT reccomend, sohyun.nam@lge.com, 2012-11-16
	/*60fps*/ 
	/* DSI Bit Clock at 335MHz, 2 lane, RGB888 */ 
	/* regulator */ 
	{0x03, 0x01, 0x01, 0x00}, 
	/* timing */ 
	{0xA6, 0x89, 0x15, 0x00, 0x90, 0x8F, 0x18, 0x8B, 0x0F, 0x03, 0x04}, 
	/* phy ctrl */ 
	{0x7f, 0x00, 0x00, 0x00}, 
	/* strength */ 
	{0xbb, 0x02, 0x06, 0x00}, 
	/* pll control */ 
	{0x00, 0x4B, 0x31, 0xD2, 0x00, 0x40, 0x37, 0x62, 0x01, 0x0F, 0x07, 
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0}, 
#endif

#if 0
	/*60fps*/
	/* DSI Bit Clock at 335MHz, 2 lane, RGB888 */
	/* regulator */
	{0x03, 0x01, 0x01, 0x00},
	/* timing */
	{0x64, 0x2A, 0x0D, 0x00, 0x35, 0x3E, 0x11, 0x2D, 0x0F, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xbb, 0x02, 0x06, 0x00},
	/* pll control */
	{0x00, 0x4A, 0x31, 0xD2, 0x00, 0x40, 0x37, 0x62, 0x01, 0x0F, 0x07, 
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0}, 
#endif

#if 0 /*50fps parameter*/
	/* DSI_BIT_CLK at 200MHz, 2 lane, RGB888 */
	/* regulator */
	{0x03, 0x01, 0x01, 0x00},
	/* timing	*/
	{0x66, 0x26, 0x15, 0x00, 0x18, 0x8D, 0x1E, 0x8B,
	0x18, 0x03, 0x04},
	{0x7f, 0x00, 0x00, 0x00},		/* phy ctrl */
	{0xee, 0x03, 0x86, 0x03},		/* strength */
	/* pll control */
	{0x41, 0x52, 0x01, 0x1A, 0x00, 0x50, 0x48, 0x63,
	0x33, 0x1f, 0x0f,
	0x05, 0x14, 0x03, 0x03, 0x03, 0x54, 0x06, 0x10, 0x04, 0x03 },
#endif /* 50fps parameter*/

#if 0 /* 30 parameter*/
	/*Height * Width * Bits per Pixel * FPS */
	/*825 * 564 *24* 30 /2 = 167.5 Mbps */
	/* DSI Bit Clock at 168MHz, 2 lane, RGB888 */ 
	/* regulator */ 
	{0x03, 0x01, 0x01, 0x00}, 
	/* timing */ 
	{0x92, 0x84, 0x09, 0x00, 0x87, 0x89, 0x0e, 0x86, 
	0x07, 0x03, 0x04}, 
	/* phy ctrl */ 
	{0x7f, 0x00, 0x00, 0x00}, 
	/* strength */ 
	{0xbb, 0x02, 0x06, 0x00}, 
	/* pll control */ 
	{0x00, 0x4b, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62, 
	0x03, 0x1f, 0x0f, 
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0}, 
#endif /* 30 fps parameter*/
/* LGE_CHANGE_E : U0 Display Panel Framerate Control */ 

};

static int __init mipi_video_lg4573b_hvga_pt_init(void)
{
	int ret;

#ifdef CONFIG_FB_MSM_MIPI_PANEL_DETECT
	if (msm_fb_detect_client("mipi_video_lg4573b_hvga"))
		return 0;
#endif

	pinfo.xres = 480;
	pinfo.yres = 800;
	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	pinfo.lcdc.h_back_porch = 72;
	pinfo.lcdc.h_front_porch = 8;
	pinfo.lcdc.h_pulse_width = 4;
	pinfo.lcdc.v_back_porch = 16;
	pinfo.lcdc.v_front_porch = 8;
	pinfo.lcdc.v_pulse_width = 1;
	pinfo.clk_rate = 335000000;/*335000000 : 60fps, 200000000 : 50fps,  167000000 : 30fps */

	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 147;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;

	pinfo.mipi.mode = DSI_VIDEO_MODE;

//LGE_CHANGE_S [Kiran] Change LCD sleep sequence
#if 0 //for LP mode from jangsu
	pinfo.mipi.pulse_mode_hsa_he = TRUE;
	pinfo.mipi.hfp_power_stop = FALSE;
	pinfo.mipi.hbp_power_stop = FALSE;
	pinfo.mipi.hsa_power_stop = FALSE;
	pinfo.mipi.eof_bllp_power_stop = TRUE;
	pinfo.mipi.bllp_power_stop = FALSE;
#else
	pinfo.mipi.pulse_mode_hsa_he = TRUE;
	pinfo.mipi.hfp_power_stop = TRUE;
	pinfo.mipi.hbp_power_stop = TRUE;
	pinfo.mipi.hsa_power_stop = TRUE;
	pinfo.mipi.eof_bllp_power_stop = TRUE;
	pinfo.mipi.bllp_power_stop = TRUE;
	pinfo.mipi.force_clk_lane_hs = TRUE;
#endif
//LGE_CHANGE_S [Kiran] Change LCD sleep sequence

	pinfo.mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
	pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.t_clk_post = 0x04;
	pinfo.mipi.t_clk_pre = 0x17;
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_NONE; /*DSI_CMD_TRIGGER_SW;*/
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
#if 0//defined(CONFIG_MACH_MSM8X25_V7) //QCT reccomend, youngbae.choi@lge.com, 2012-12-29
	pinfo.mipi.frame_rate = 63;	/* 60fps, 50fps, 30fps */
#else
	pinfo.mipi.frame_rate = 60;	/* 60fps, 50fps, 30fps */
#endif
	pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;
	pinfo.mipi.dlane_swap = 0x01;	
	pinfo.mipi.tx_eot_append = 0x01;


	ret = mipi_lg4573b_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_FWVGA_PT);
	if (ret)
		pr_err("%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_video_lg4573b_hvga_pt_init);
