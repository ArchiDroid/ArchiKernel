/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 * Copyright(C) 2013 Foxconn International Holdings, Ltd. All rights reserved.
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

#include "msm_sensor.h"
#define SENSOR_NAME "hm03d5"
#define PLATFORM_DRIVER_NAME "msm_camera_hm03d5"
#define hm03d5_obj hm03d5_##obj

DEFINE_MUTEX(hm03d5_mut);
static struct msm_sensor_ctrl_t hm03d5_s_ctrl;

static struct msm_camera_i2c_reg_conf hm03d5_start_settings[] = {
	{0x0005, 0x01,},//Turn on rolling shutter
};

static struct msm_camera_i2c_reg_conf hm03d5_stop_settings[] = {
	{0x0005, 0x00,},//Turn off rolling shutter
};


static struct msm_camera_i2c_reg_conf hm03d5_recommend_settings[] = {
/*************************************************************************
     Sensor: HM03D5
     I2C ID: 30
     Resolution: 640x480(VGA)
     Lens: 
     Flicker: 
     PCLK: 24M
     MIPI: 96M
     Description: 
       1. Clock lane continuous clock
       2. Update analog gain setting - by gene
       3. For WLM
     Note:
     
     IQ Rev.: 
     Release: 
     -----------
     $Revision: 179 $
     $Date:: 2012-10-05 15:11:00 +0800#$
*************************************************************************/
//---------------------------------------------
// Reset
//---------------------------------------------
    {0x0022, 0x00,},//Reset

    {0x0015, 0x03,},/* HarrisWeng20130514_AE_Fast_15fps */
    {0x0016, 0xF8,},/* HarrisWeng20130514_AE_Fast_15fps */
//---------------------------------------------
// Analog
//---------------------------------------------
    {0x0025, 0x80,},//use mclk
    {0x0024, 0x40,},//mipi enable
    
    {0x0027, 0x10,},//CbYCrY
    {0x0040, 0x0F,},//BLC_phase1 target
    {0x0053, 0x0F,},//BLC_phase2 target
    {0x0044, 0x06,},//enable BLC_phase2
    {0x0046, 0xD8,},//enable BLC_phase1, enable BLC_phase2_dithering
    {0x0055, 0x41,},//extend 2 black rows
    {0x004A, 0x04,},//BLC_phase1 0~3th black rows, BLC_phase2 4~9th black rows

    {0x0026, 0x07,},
    {0x002a, 0x1f,},
    {0x008e, 0x80,},
    {0x0028, 0x00,},

    {0x0088, 0xCD,},//lower PVDD level & increase vcmi level 20120703
    {0x0090, 0x00,},//update analog gain table               20120706
    {0x0091, 0x10,},//update analgo gain table               20120706
    {0x0092, 0x11,},//update analog gain table               20120706
    {0x0093, 0x12,},//update analgo gain table               20120706
    {0x0094, 0x13,},//update analog gain table               20120706
    {0x0095, 0x17,},//update analgo gain table               20120706
    {0x0085, 0x50,},//ADC speed               20131211 /*MM-UW-fix bad front camera preview-00+{*/
    {0x0086, 0x82,},//for dark sun issue                       20120711
    {0x0080, 0x80,},//SF current turn on only CDS          20120711
    {0x00A0, 0xA1,},//turn on thick oxide support           20120820

//---------------------------------------------
//digital
//---------------------------------------------
	{0x011F, 0x54,},//
	{0x0121, 0x80,},//
	{0x0122, 0x6B,},//SIM_BPC,LSC_ON,A6_ON
	{0x0123, 0xAC,},//[0]ISP image boundary process control. [3]enhance cluster hot pixel correction.
	{0x0124, 0xD2,},//
	{0x0125, 0xDF,},//
	{0x0126, 0x71,},//71=EV_SEL 3x5,61=EV_SEL 3x3

	{0x0140, 0x14,},//BPC
	{0x0141, 0x0A,},
	{0x0142, 0x14,},
	{0x0143, 0x0A,},
	{0x0144, 0x08,},
	{0x0145, 0x00,},
	{0x0146, 0xF3,},//New BPC
	{0x0148, 0x50,},
	{0x0149, 0x0C,},

	{0x014A, 0xA0,},// 0x60 /* MM-UW-fix vga color noise-00+{*/
	{0x014B, 0x28,},
	{0x014C, 0x20,},
	{0x014D, 0x2E,},
	{0x014E, 0x05,},
	{0x014F, 0x0A,},//cluster hot pixel correction strength low light control for BR channel. 
	{0x0150, 0x0D,},
	{0x0155, 0x00,},
	{0x0156, 0x0A,},
	{0x0157, 0x0A,},
	{0x0158, 0x0A,},
	{0x0159, 0x0A,},
	{0x015A, 0x0C,},
	{0x015B, 0x0C,},

	{0x0160, 0x14,},//SIM_BPC_th
	{0x0161, 0x14,},//SIM_BPC_th Alpha
	{0x0162, 0x28,},//SIM_BPC_Directional
	{0x0163, 0x28,},//SIM_BPC_Directional Alpha

	{0x0164, 0x0A,},
	{0x0165, 0x0C,},
	{0x0166, 0x0A,},
	{0x0167, 0x0C,},
	{0x0168, 0x04,},
	{0x0169, 0x08,},
	{0x016A, 0x04,},
	{0x016B, 0x08,},

    {0x01B0, 0x33,},//G1G2 Balance
    {0x01B1, 0x10,},
    {0x01B2, 0x10,},
    {0x01B3, 0x0C,},
    {0x01B4, 0x0A,},

    {0x01D8, 0x0F,},//Bayer Denoise
    {0x01DE, 0x0F,},
    {0x01E4, 0x0B,},
    {0x01E5, 0x10,},

    {0x0220, 0x10,},//LSC
    {0x0221, 0x76,},//0xA0,0x88
    {0x0222, 0xFF,},
    {0x0223, 0x7D,}, //0x80
    {0x0224, 0x58,}, //0x70
    {0x0225, 0x00,},
    {0x0226, 0x8C,},//IsaacLiu20130114 change from 0x8C to 0x8A
    {0x022A, 0x48,}, // 0x70
    {0x022B, 0x00,},
    {0x022C, 0x93,},//IsaacLiu20130114 change from 0x8C to 0x93
    {0x022D, 0x10,},
    {0x022E, 0x0B,},
    {0x022F, 0x10,},
    {0x0230, 0x10,},
    {0x0233, 0x10,},
    {0x0234, 0x10,},
    {0x0235, 0x44,},
    {0x0236, 0x01,},
    {0x0237, 0x44,},
    {0x0238, 0x01,},
    {0x023B, 0x44,},
    {0x023C, 0x01,},
    {0x023D, 0xF2,},
    {0x023E, 0x00,},
    {0x023F, 0xF2,},
    {0x0240, 0x00,},
    {0x0243, 0xF2,},
    {0x0244, 0x00,},
    {0x0251, 0x0B,}, // 0x0C
    {0x0252, 0x08,},

    {0x0280, 0x0A,},//Gamma
    {0x0282, 0x14,},
    {0x0284, 0x26,},
    {0x0286, 0x4A,},
    {0x0288, 0x5A,},
    {0x028A, 0x67,},
    {0x028C, 0x73,},
    {0x028E, 0x7D,},
    {0x0290, 0x86,},
    {0x0292, 0x8E,},
    {0x0294, 0x9A,},
    {0x0296, 0xA6,},
    {0x0298, 0xBB,},
    {0x029A, 0xCF,},
    {0x029C, 0xE2,},
    {0x029E, 0x26,},

    {0x02A0, 0x04,},

    {0x02C0, 0xB1,},//CCM
    {0x02C1, 0x01,},
    {0x02C2, 0x7D,},
    {0x02C3, 0x07,},
    {0x02C4, 0xD2,},
    {0x02C5, 0x07,},
    {0x02C6, 0xC4,},
    {0x02C7, 0x07,},
    {0x02C8, 0x79,},
    {0x02C9, 0x01,},
    {0x02CA, 0xC4,},
    {0x02CB, 0x07,},
    {0x02CC, 0xF7,},
    {0x02CD, 0x07,},
    {0x02CE, 0x3B,},
    {0x02CF, 0x07,},
    {0x02D0, 0xCF,},
    {0x02D1, 0x01,},

    {0x02E0, 0x04,},
    
    {0x02F0, 0x5E,},//A CCM
    {0x02F1, 0x07,},
    {0x02F2, 0xA0,},
    {0x02F3, 0x00,},
    {0x02F4, 0x02,},
    {0x02F5, 0x00,},
    {0x02F6, 0xC4,},
    {0x02F7, 0x07,},
    {0x02F8, 0x11,},
    {0x02F9, 0x00,},
    {0x02FA, 0x2A,},
    {0x02FB, 0x00,},
    {0x02FC, 0xA1,},
    {0x02FD, 0x07,},
    {0x02FE, 0xB8,},
    {0x02FF, 0x07,},
    {0x0300, 0xA7,},
    {0x0301, 0x00,},

    {0x0328, 0x00,},//Win Pick P Min
    {0x0329, 0x04,},

    {0x032D, 0x66,},//Initial Channel Gain
    {0x032E, 0x01,},
    {0x032F, 0x00,},
    {0x0330, 0x01,},
    {0x0331, 0x66,},
    {0x0332, 0x01,},

    {0x0333, 0x00,},//AWB channel offset
    {0x0334, 0x00,},
    {0x0335, 0x85,},//IsaacLiu20130114 change from 0x84 to 0x85

    {0x0340, 0x38,},//AWB
    {0x0341, 0x88,},
    {0x0342, 0x4A,},
    {0x0343, 0x40,},
    {0x0344, 0x6E,},
    {0x0345, 0x50,},
    {0x0346, 0x64,},
    {0x0347, 0x58,},
    {0x0348, 0x5A,},
    {0x0349, 0x63,},
    {0x034A, 0x54,},
    {0x034B, 0x6D,},
    {0x034C, 0x4C,},
    {0x0350, 0x68,},
    {0x0352, 0x08,},

    {0x0354, 0x73,},//R Max
    {0x0355, 0x45,},//G Max
    {0x0356, 0x80,},//B Max
    {0x0357, 0xD0,},//R+B Max
    {0x0358, 0x05,},//R comp Max
    {0x035A, 0x05,},//
    {0x035B, 0xA0,},//R+B Min

    {0x0381, 0x47,},/* HarrisWeng20130514_AE_Fast_15fps */
    {0x0382, 0x33,},/* HarrisWeng20130514_AE_Fast_15fps */
    {0x0383, 0x20,},
    {0x038A, 0x80,},/* HarrisWeng20130514_AE_Fast_15fps */
    {0x038B, 0x10,},/* HarrisWeng20130514_AE_Fast_15fps */
    {0x038C, 0xC1,},
    {0x038E, 0x3D,},/* HarrisWeng20130514_AE_Fast_15fps */
    {0x038F, 0x03,},/* HarrisWeng20130514_AE_Fast_15fps */
    {0x0390, 0xF8,},/* HarrisWeng20130514_AE_Fast_15fps */
    {0x0391, 0x10,},//AE Min exposure
    {0x0393, 0x80,},
    {0x0395, 0x11,},/* HarrisWeng20130514_AE_Fast_15fps */

    {0x0398, 0x02,},//Frame rate control
    {0x0399, 0x00,},
    {0x039A, 0x03,},
    {0x039B, 0x00,},
    {0x039C, 0x04,},
    {0x039D, 0x00,},
    {0x039E, 0x05,},
    {0x039F, 0x00,},
    {0x03A0, 0x06,},
    {0x03A1, 0x00,},

    {0x03A6, 0x10,},
    {0x03A7, 0x10,},
    {0x03A8, 0x36,},
    {0x03A9, 0x40,},
    {0x03AE, 0x35,},
    {0x03AF, 0x2B,},
    {0x03B0, 0x0C,},
    {0x03B1, 0x0A,},
    {0x03B3, 0x00,},
    {0x03B5, 0x08,},
    {0x03B7, 0xA0,},
    {0x03B9, 0xD0,},

    {0x03BB, 0xFF,},//AE Winding
    {0x03BC, 0xFF,},
    {0x03BE, 0x04,},
    {0x03BF, 0x1D,},
    {0x03C0, 0x2E,},

    {0x03C3, 0x0F,},

    {0x03D0, 0xE0,},

    {0x0420, 0x87,},//BLC Offset //IsaacLiu20130114 change from 0x86 to 0x85 //0x85
    {0x0421, 0x00,},
    {0x0422, 0x00,},
    {0x0423, 0x00,},//0x84

    {0x0430, 0x10,},//ABLC
    {0x0431, 0x68,},//Max
    {0x0432, 0x38,},//ABLC
    {0x0433, 0x30,},
    {0x0434, 0x00,},
    {0x0435, 0x40,},
    {0x0436, 0x00,},

    {0x0450, 0xFF,},
    {0x0451, 0xFF,},
    {0x0452, 0xC0,},
    {0x0453, 0x70,},
    {0x0454, 0x00,},

    {0x045A, 0x00,},
    {0x045B, 0x10,},
    {0x045C, 0x00,},
    {0x045D, 0xA0,},

    {0x0465, 0x02,},/* HarrisWeng20130514_AE_Fast_15fps */
    {0x0466, 0x14,},/* HarrisWeng20130514_AE_Fast_15fps */

    {0x047A, 0x00,},//ODEL Rayer denoise
    {0x047B, 0x00,},//ODEL Y denoise

    {0x0480, 0x50,},//Sat //BLC Offset //IsaacLiu20130114 change from 0x56 to 0x50
    {0x0481, 0x06,},//Sat by Alpha

    {0x04B0, 0x4C,},//Contrast
    {0x04B6, 0x38,},
    {0x04B9, 0x0C,},
    {0x04B3, 0x08,},
    {0x04B1, 0x84,},
    {0x04B4, 0x00,},

    {0x0540, 0x00,},//60Hz Flicker step
    {0x0541, 0x7F,},//60Hz Flicker step
    {0x0542, 0x00,},//50Hz Flicker step
    {0x0543, 0x98,},//50Hz Flicker step

    {0x0580, 0x04,},//Y Blurring
    {0x0581, 0x08,},//Y Blurring Alpha

    {0x0590, 0x20,},//UV denoise
    {0x0591, 0x30,},//UV denoise Alpha

    {0x0594, 0x02,},//UV Gray TH
    {0x0595, 0x08,},//UV Gray TH Alpha 

    {0x05A0, 0x07,},//Sharpness Curve Edge THL
    {0x05A1, 0x14,},//Edge THL Alpha
    {0x05A2, 0x30,},//Edge THH Dark
    {0x05A3, 0x30,},//Edge THH Dark Alpha
    {0x05A5, 0x20,},//Edge THH Bright
    {0x05A6, 0x20,},//Edge THH Bright Alpha
    {0x05B0, 0x30,},//Edge strength
    {0x05B1, 0x04,},
    {0x05B2, 0x28,},//Edge strength
    {0x05B3, 0x04,},

    {0x05D0, 0x10,},//F2B Start Mean
    {0x05D1, 0x06,},//F2B

    {0x05E4, 0x04,},//Windowing for 640x480
    {0x05E5, 0x00,},
    {0x05E6, 0x83,},
    {0x05E7, 0x02,},
    {0x05E8, 0x07,},/* IsaacLiu20130521_Line_Buffer_Delay */
    {0x05E9, 0x00,},/* IsaacLiu20130521_Line_Buffer_Delay */
    {0x05EA, 0xE6,},/* IsaacLiu20130521_Line_Buffer_Delay */
    {0x05EB, 0x01,},/* IsaacLiu20130521_Line_Buffer_Delay */

	// MIPI Continuous Clock, with LS/LE
	//{0x0b20, 0x9E,},//FIH-SW-MM-MC-BringUpCameraYUVSensorForHM03D5-00*
	{0x0b20, 0xBE,},//FIH-SW-MM-MC-BringUpCameraYUVSensorForHM03D5-00*
	{0x007C, 0x07,},//pre-hsync //FIH-SW-MM-MC-BringUpCameraYUVSensorForHM03D5-00*
	{0x007D, 0x3C,},//pre-vsync 

//---------------------------------------------
// CMU and rolling shutter
//---------------------------------------------
	{0x0000, 0x01,},
	{0x0100, 0x01,},
	{0x0101, 0x01,},
};

static struct msm_camera_i2c_reg_conf hm03d5_full_settings[] = {
    //{0x000F, 0x18,},//Fix FPS
    {0x05E4, 0x04,},//Windowing for 640x480
    {0x05E5, 0x00,},
    {0x05E6, 0x83,},
    {0x05E7, 0x02,},
    {0x05E8, 0x04,},
    {0x05E9, 0x00,},
    {0x05EA, 0xE3,},
    {0x05EB, 0x01,},
    //Enable for show color pattern
    //{0x0028, 0x80,},//Test Img: Black and White Checkerboard
    //{0x0028, 0x81,},//Test Img: Gray Ramp (Black to White, from left to right)
    //{0x0028, 0x82,},//Test Img: Gray Step Ramp (Black to White, from left to right)
    //{0x0028, 0x83,},//Test Img: Color Bar Ramp (Black to Color from top to bottom)
    //{0x0028, 0x84,},//Test Img: Color Bar
};

/* MM-UW-Fix MMS can't record 23s-00+{ */
static struct msm_camera_i2c_reg_conf hm03d5_fps_15_30[] = {
    {0x0010, 0x00,},
    {0x0011, 0x02,},
    {0x038F, 0x03,},
    {0x0390, 0xF8,},
    
    {0x0000, 0x01,},
    {0x0100, 0x01,},
    {0x0101, 0x01,},
};

static struct msm_camera_i2c_reg_conf hm03d5_fps_10_14[] = {
    {0x0010, 0x02,},
    {0x0011, 0x53,},
    {0x038F, 0x05,},
    {0x0390, 0xF4,},
    
    {0x0000, 0x01,},
    {0x0100, 0x01,},
    {0x0101, 0x01,},
};
/* MM-UW-Fix MMS can't record 23s-00+} */

static struct v4l2_subdev_info hm03d5_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array hm03d5_init_conf[] = {
	{hm03d5_recommend_settings,
	ARRAY_SIZE(hm03d5_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array hm03d5_confs[] = {
	{&hm03d5_full_settings[0],
	ARRAY_SIZE(hm03d5_full_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

/* VFE maximum clock is 266 MHz.
    For sensor bringup you can keep .op_pixel_clk = maximum VFE clock.
    and .vt_pixel_clk = line_length_pclk * frame_length_lines * FPS.         */
static struct msm_sensor_output_info_t hm03d5_dimensions[] = {
	{
		.x_output = 0x280,//640
		.y_output = 0x1E0,//480
		.line_length_pclk = 0x290,//656
		.frame_length_lines = 0x1F2,//498
        .vt_pixel_clk = 9860000,//9800640
        .op_pixel_clk = 19720000,
		.binning_factor = 0,
	},
};

static struct msm_cam_clk_info cam_8960_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

static struct msm_cam_clk_info cam_8974_clk_info[] = {
	{"cam_src_clk", 19200000},
	{"cam_clk", -1},
};

int32_t hm03d5_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	struct device *dev = NULL;
	if (s_ctrl->sensor_device_type == MSM_SENSOR_PLATFORM_DEVICE)
		dev = &s_ctrl->pdev->dev;
	else
		dev = &s_ctrl->sensor_i2c_client->client->dev;

	rc = msm_camera_request_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		goto request_gpio_failed;
	}

	rc = msm_camera_config_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
		goto config_gpio_failed;
	}

	if (s_ctrl->sensor_device_type == MSM_SENSOR_I2C_DEVICE) {
		if (s_ctrl->clk_rate != 0)
			cam_8960_clk_info->clk_rate = s_ctrl->clk_rate;

		rc = msm_cam_clk_enable(dev, cam_8960_clk_info,
			s_ctrl->cam_clk, ARRAY_SIZE(cam_8960_clk_info), 1);
		if (rc < 0) {
			pr_err("%s: clk enable failed\n", __func__);
			goto enable_clk_failed;
		}
	} else {
		rc = msm_cam_clk_enable(dev, cam_8974_clk_info,
			s_ctrl->cam_clk, ARRAY_SIZE(cam_8974_clk_info), 1);
		if (rc < 0) {
			pr_err("%s: clk enable failed\n", __func__);
			goto enable_clk_failed;
		}
	}

	if (!s_ctrl->power_seq_delay)
		usleep_range(1000, 2000);
	else if (s_ctrl->power_seq_delay < 20)
		usleep_range((s_ctrl->power_seq_delay * 1000),
			((s_ctrl->power_seq_delay * 1000) + 1000));
	else
		msleep(s_ctrl->power_seq_delay);

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(1);

	if (data->sensor_platform_info->i2c_conf &&
		data->sensor_platform_info->i2c_conf->use_i2c_mux)
		msm_sensor_enable_i2c_mux(data->sensor_platform_info->i2c_conf);

	if (s_ctrl->sensor_device_type == MSM_SENSOR_PLATFORM_DEVICE) {
		rc = msm_sensor_cci_util(s_ctrl->sensor_i2c_client,
			MSM_CCI_INIT);
		if (rc < 0) {
			pr_err("%s cci_init failed\n", __func__);
			goto cci_init_failed;
		}
	}
	s_ctrl->curr_res = MSM_SENSOR_INVALID_RES;
	return rc;

cci_init_failed:
	if (data->sensor_platform_info->i2c_conf &&
		data->sensor_platform_info->i2c_conf->use_i2c_mux)
		msm_sensor_disable_i2c_mux(
			data->sensor_platform_info->i2c_conf);
enable_clk_failed:
		msm_camera_config_gpio_table(data, 0);
config_gpio_failed:
	msm_camera_request_gpio_table(data, 0);
request_gpio_failed:

	return rc;
}

int32_t hm03d5_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	struct device *dev = NULL;
	if (s_ctrl->sensor_device_type == MSM_SENSOR_PLATFORM_DEVICE)
		dev = &s_ctrl->pdev->dev;
	else
		dev = &s_ctrl->sensor_i2c_client->client->dev;
	if (s_ctrl->sensor_device_type == MSM_SENSOR_PLATFORM_DEVICE) {
		msm_sensor_cci_util(s_ctrl->sensor_i2c_client,
			MSM_CCI_RELEASE);
	}

	if (data->sensor_platform_info->i2c_conf &&
		data->sensor_platform_info->i2c_conf->use_i2c_mux)
		msm_sensor_disable_i2c_mux(
			data->sensor_platform_info->i2c_conf);

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(0);
	if (s_ctrl->sensor_device_type == MSM_SENSOR_I2C_DEVICE)
		msm_cam_clk_enable(dev, cam_8960_clk_info, s_ctrl->cam_clk,
			ARRAY_SIZE(cam_8960_clk_info), 0);
	else
		msm_cam_clk_enable(dev, cam_8974_clk_info, s_ctrl->cam_clk,
			ARRAY_SIZE(cam_8974_clk_info), 0);
	msm_camera_config_gpio_table(data, 0);

	msm_camera_request_gpio_table(data, 0);

	return 0;
}

/* MM-UW-Fix MMS can't record 23s-00+{ */
int32_t hm03d5_set_fps(struct msm_sensor_ctrl_t *s_ctrl,
						struct fps_cfg *fps)
{
	int32_t rc = 0;
    
       if(fps->fps_div > 2000)
      {
        	rc = msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, hm03d5_fps_10_14,
		ARRAY_SIZE(hm03d5_fps_10_14), MSM_CAMERA_I2C_BYTE_DATA);
      }else
      {
              rc = msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, hm03d5_fps_15_30,
		ARRAY_SIZE(hm03d5_fps_15_30), MSM_CAMERA_I2C_BYTE_DATA);
      }

       pr_err("%s: fps_divider =%d\n", __func__, s_ctrl->fps_divider); 

	return rc;
}
/* MM-UW-Fix MMS can't record 23s-00+} */

int32_t hm03d5_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	uint16_t chipid = 0;
    uint16_t id_low  = 0;
    uint16_t id_high = 0;

	rc = msm_camera_i2c_read(
			s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_id_info->sensor_id_reg_addr, &id_high,
			MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id_high failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
		return rc;
	}

	rc = msm_camera_i2c_read(
			s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_id_info->sensor_id_reg_addr2, &id_low,
			MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id_low failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
		return rc;
	}

    chipid = (id_high << 8 | id_low); 

	printk("%s: id_high = 0x%x,  id_low = 0x%x:\n", __func__, id_high, id_low);
	printk("%s: read id: 0x%x expected id 0x%x:\n", __func__, chipid, s_ctrl->sensor_id_info->sensor_id);

	if (chipid != s_ctrl->sensor_id_info->sensor_id) {
		pr_err("hm03d5_match_id chip id doesnot match\n");
		return -ENODEV;
	}
	return rc;
}

static struct msm_sensor_output_reg_addr_t hm03d5_reg_addr = {
	.x_output = 0xFFFF,//SOC sensor not support
	.y_output = 0xFFFF,//SOC sensor not support
	.line_length_pclk = 0xFFFF,//SOC sensor not support
	.frame_length_lines = 0xFFFF,//SOC sensor not support
};

static enum msm_camera_vreg_name_t hm03d5_veg_seq[] = {
	CAM_VIO,
	CAM_VDIG,
	CAM_VANA,
};

static struct msm_sensor_id_info_t hm03d5_id_info = {
	.sensor_id_reg_addr = 0x0001,//Chip ID High Byte
	.sensor_id = 0x03d5,
    .sensor_id_reg_addr2 = 0x0002,//Chip ID Low Byte
};

static const struct i2c_device_id hm03d5_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&hm03d5_s_ctrl},
	{ }
};

static struct i2c_driver hm03d5_i2c_driver = {
	.id_table = hm03d5_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client hm03d5_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&hm03d5_i2c_driver);
}

static struct v4l2_subdev_core_ops hm03d5_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops hm03d5_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops hm03d5_subdev_ops = {
	.core = &hm03d5_subdev_core_ops,
	.video  = &hm03d5_subdev_video_ops,
};

static struct msm_sensor_fn_t hm03d5_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_setting = msm_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = hm03d5_power_up,
	.sensor_power_down = hm03d5_power_down,
	.sensor_match_id = hm03d5_match_id,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
	.sensor_set_fps = hm03d5_set_fps,/* MM-UW-Fix MMS can't record 23s-00+{ */
};

static struct msm_sensor_reg_t hm03d5_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = hm03d5_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(hm03d5_start_settings),
	.stop_stream_conf = hm03d5_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(hm03d5_stop_settings),
	.init_settings = &hm03d5_init_conf[0],
	.init_size = ARRAY_SIZE(hm03d5_init_conf),
	.mode_settings = &hm03d5_confs[0],
	.output_settings = &hm03d5_dimensions[0],
	.num_conf = ARRAY_SIZE(hm03d5_confs),
};

static struct msm_sensor_ctrl_t hm03d5_s_ctrl = {
	.msm_sensor_reg = &hm03d5_regs,
	.sensor_i2c_client = &hm03d5_sensor_i2c_client,
	.sensor_i2c_addr = 0x60,
	.vreg_seq = hm03d5_veg_seq,
	.num_vreg_seq = ARRAY_SIZE(hm03d5_veg_seq),
	.sensor_output_reg_addr = &hm03d5_reg_addr,
	.sensor_id_info = &hm03d5_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.min_delay = 30,
	.power_seq_delay = 60,
	.msm_sensor_mutex = &hm03d5_mut,
	.sensor_i2c_driver = &hm03d5_i2c_driver,
	.sensor_v4l2_subdev_info = hm03d5_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(hm03d5_subdev_info),
	.sensor_v4l2_subdev_ops = &hm03d5_subdev_ops,
	.func_tbl = &hm03d5_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Himax VGA YUV sensor driver");
MODULE_LICENSE("GPL v2");
