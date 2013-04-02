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
 *
 */

#include "msm_sensor.h"
#include "msm.h"
#include "msm_ispif.h"
#define SENSOR_NAME "imx111"
#define PLATFORM_DRIVER_NAME "msm_camera_imx111"
#define imx111_obj imx111_##obj

/* LGE_CHANGE_S, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */
#define CAPTURE_10FPS_444Mhz
/* LGE_CHANGE_E, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */

DEFINE_MUTEX(imx111_mut);
static struct msm_sensor_ctrl_t imx111_s_ctrl;

static struct msm_camera_i2c_reg_conf imx111_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf imx111_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf imx111_groupon_settings[] = {
	{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf imx111_groupoff_settings[] = {
	{0x104, 0x00},
};

static struct msm_camera_i2c_reg_conf imx111_prev_settings[] = {
	{0x30B1, 0x03}, // recovery clamp setting
	{0x0101, 0x03}, // XY flip
	{0x0305, 0x02},
	{0x0307, 0x38},
	{0x30A4, 0x02},
	{0x303C, 0x4B},
	{0x0340, 0x04},
	{0x0341, 0xF2},
	{0x0342, 0x0D},
	{0x0343, 0xD0},
	{0x0344, 0x00},
	{0x0345, 0x08},
	{0x0346, 0x00},
	{0x0347, 0x30},
	{0x0348, 0x0C},
	{0x0349, 0xD7},
	{0x034A, 0x09},
	{0x034B, 0xCF},
	{0x034C, 0x06},
	{0x034D, 0x68},
	{0x034E, 0x04},
	{0x034F, 0xD0},
	{0x0381, 0x01},
	{0x0383, 0x03},
	{0x0385, 0x01},
	{0x0387, 0x03},
	{0x3033, 0x00},
	{0x303D, 0x10},
	{0x303E, 0x40},
	{0x3040, 0x08},
	{0x3041, 0x97},
	{0x3048, 0x01},
	{0x304C, 0x6F},
	{0x304D, 0x03},
	{0x3064, 0x12},
	{0x3073, 0x00},
	{0x3074, 0x11},
	{0x3075, 0x11},
	{0x3076, 0x11},
	{0x3077, 0x11},
	{0x3079, 0x00},
	{0x307A, 0x00},
	{0x309B, 0x28},
	{0x309C, 0x13},
	{0x309E, 0x00},
	{0x30A0, 0x14},
	{0x30A1, 0x09},
	{0x30AA, 0x02},
	{0x30B2, 0x05},
	{0x30D5, 0x09},
	{0x30D6, 0x01},
	{0x30D7, 0x01},
	{0x30D8, 0x64},
	{0x30D9, 0x89},
	{0x30DA, 0x00},
	{0x30DB, 0x00},
	{0x30DC, 0x00},
	{0x30DD, 0x00},
	{0x30DE, 0x02},
	{0x30DF, 0x20},
	{0x3102, 0x08},
	{0x3103, 0x22},
	{0x3104, 0x20},
	{0x3105, 0x00},
	{0x3106, 0x87},
	{0x3107, 0x00},
	{0x3108, 0x03},
	{0x3109, 0x02},
	{0x310A, 0x03},
	{0x315C, 0x9C},
	{0x315D, 0x9B},
	{0x316E, 0x9D},
	{0x316F, 0x9C},
	{0x3301, 0x00},
	{0x3304, 0x05},
	{0x3305, 0x05},
	{0x3306, 0x15},
	{0x3307, 0x02},
	{0x3308, 0x0D},
	{0x3309, 0x07},
	{0x330A, 0x09},
	{0x330B, 0x05},
	{0x330C, 0x08},
	{0x330D, 0x06},
	{0x330E, 0x03},
	{0x3318, 0x72},
	{0x3322, 0x03},
	{0x3342, 0x00},
	{0x3348, 0xE0},
};

static struct msm_camera_i2c_reg_conf imx111_video_settings[] = {
};

static struct msm_camera_i2c_reg_conf imx111_snap_settings[] = {
	{0x0101, 0x03}, // XY flip
	{0x0305, 0x02},
	{0x0307, 0x38},
	{0x30A4, 0x02},
	{0x303C, 0x4B},
	{0x0340, 0x09},
	{0x0341, 0xE2},
	{0x0342, 0x0D},
	{0x0343, 0xD0},
	{0x0344, 0x00},
	{0x0345, 0x08},
	{0x0346, 0x00},
	{0x0347, 0x30},
	{0x0348, 0x0C},
	{0x0349, 0xD7},
	{0x034A, 0x09},
	{0x034B, 0xCF},
	{0x034C, 0x0C},
	{0x034D, 0xD0},
	{0x034E, 0x09},
	{0x034F, 0xA0},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x3033, 0x00},
	{0x303D, 0x10},
	{0x303E, 0x40},
	{0x3040, 0x08},
	{0x3041, 0x97},
	{0x3048, 0x00},
	{0x304C, 0x6F},
	{0x304D, 0x03},
	{0x3064, 0x12},
	{0x3073, 0x00},
	{0x3074, 0x11},
	{0x3075, 0x11},
	{0x3076, 0x11},
	{0x3077, 0x11},
	{0x3079, 0x00},
	{0x307A, 0x00},
	{0x309B, 0x20},
	{0x309C, 0x13},
	{0x309E, 0x00},
	{0x30A0, 0x14},
	{0x30A1, 0x08},
	{0x30AA, 0x02},
	{0x30B2, 0x07},
	{0x30D5, 0x00},
	{0x30D6, 0x85},
	{0x30D7, 0x2A},
	{0x30D8, 0x64},
	{0x30D9, 0x89},
	{0x30DA, 0x00},
	{0x30DB, 0x00},
	{0x30DC, 0x00},
	{0x30DD, 0x00},
	{0x30DE, 0x00},
	{0x30DF, 0x20},
	{0x3102, 0x08},
	{0x3103, 0x22},
	{0x3104, 0x20},
	{0x3105, 0x00},
	{0x3106, 0x87},
	{0x3107, 0x00},
	{0x3108, 0x03},
	{0x3109, 0x02},
	{0x310A, 0x03},
	{0x315C, 0x9C},
	{0x315D, 0x9B},
	{0x316E, 0x9D},
	{0x316F, 0x9C},
	{0x3301, 0x00},
	{0x3304, 0x05},
	{0x3305, 0x05},
	{0x3306, 0x15},
	{0x3307, 0x02},
	{0x3308, 0x0D},
	{0x3309, 0x07},
	{0x330A, 0x09},
	{0x330B, 0x05},
	{0x330C, 0x08},
	{0x330D, 0x06},
	{0x330E, 0x03},
	{0x3318, 0x62},
	{0x3322, 0x03},
	{0x3342, 0x00},
	{0x3348, 0xE0},
};

static struct msm_camera_i2c_reg_conf imx111_recommend_settings[] = {
	{0x3080, 0x50},
	{0x3087, 0x53},
	{0x309D, 0x94},
	{0x30B1, 0x03},
	{0x30C6, 0x00},
	{0x30C7, 0x00},
	{0x3115, 0x0B},
	{0x3118, 0x30},
	{0x311D, 0x25},
	{0x3121, 0x0A},
	{0x3212, 0xF2},
	{0x3213, 0x0F},
	{0x3215, 0x0F},
	{0x3217, 0x0B},
	{0x3219, 0x0B},
	{0x321B, 0x0D},
	{0x321D, 0x0D},
	{0x32AA, 0x11},
	{0x3032, 0x40},
};

static struct msm_camera_i2c_reg_conf imx111_comm1_settings[] = {
	{0x3035, 0x10},
	{0x303B, 0x14},
	{0x3312, 0x45},
	{0x3313, 0xC0},
	{0x3310, 0x20},
	{0x3310, 0x00},
	{0x303B, 0x04},
	{0x303D, 0x00},
	{0x0100, 0x10},
	{0x3035, 0x00},
};

static struct msm_camera_i2c_reg_conf imx111_comm2_part1_settings[] = {
/* LGE_CHANGE_S, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */
#if defined(CAPTURE_10FPS_444Mhz)
	{0x0307, 0x25},
	{0x0340, 0x09},
	{0x0341, 0xD0},
#else //15 fps 672HMz
	{0x0340, 0x09},
	{0x0341, 0xE2},
#endif	
	{0x034C, 0x0C},
	{0x034D, 0xD0},
	{0x034E, 0x09},
	{0x034F, 0xA0},
	{0x0383, 0x01},
	{0x0387, 0x01},
	{0x3048, 0x00},
	{0x309B, 0x20},
	{0x30A1, 0x08},
	{0x30B2, 0x07},	
	{0x30D5, 0x00},
	{0x30D6, 0x85},
	{0x30D7, 0x2A},
	{0x30DE, 0x00},
#if defined(CAPTURE_10FPS_444Mhz)
	{0x3318, 0x66},
#else //15 fps 672HMz
	{0x3318, 0x62},
#endif
/* LGE_CHANGE_E, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */
};

static struct msm_camera_i2c_reg_conf imx111_comm2_part2_settings[] = {
	{0x30B1, 0x43},
	{0x3311, 0x80},
	{0x3311, 0x00},
};

static struct msm_camera_i2c_conf_array imx111_comm_confs[] = {
	{&imx111_comm1_settings[0],
	ARRAY_SIZE(imx111_comm1_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_comm2_part1_settings[0],
	ARRAY_SIZE(imx111_comm2_part1_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_comm2_part2_settings[0],
	ARRAY_SIZE(imx111_comm2_part2_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct v4l2_subdev_info imx111_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array imx111_init_conf[] = {
	{&imx111_recommend_settings[0],
	ARRAY_SIZE(imx111_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array imx111_confs[] = {
	{&imx111_snap_settings[0],
	ARRAY_SIZE(imx111_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_prev_settings[0],
	ARRAY_SIZE(imx111_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_video_settings[0],
	ARRAY_SIZE(imx111_video_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
/* LGE_CHANGE_S, Camera Zero shutter lag patch, 2012.01.12 jungryoul.choi@lge.com */
	{&imx111_snap_settings[0],
	ARRAY_SIZE(imx111_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
/* LGE_CHANGE_E, Camera Zero shutter lag patch, 2012.01.12 jungryoul.choi@lge.com */
};

static struct msm_sensor_output_info_t imx111_dimensions[] = {
/* LGE_CHANGE_S, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */
#if defined(CAPTURE_10FPS_444Mhz)
	{
		/* 10 fps */
		.x_output = 0x0CD0, /* 3280 */
		.y_output = 0x9A0, /* 2464 */
		.line_length_pclk = 0xDD0, /* 3536 */
		.frame_length_lines = 0x9D0, /* 2512 */  
		.vt_pixel_clk = 88800000,
		.op_pixel_clk = 88800000,
		.binning_factor = 1,

	},
#else
	{
		/* 15 fps */
		.x_output = 0x0CD0, /* 3280 */
		.y_output = 0x9A0, /* 2464 */
		.line_length_pclk = 0xDD0, /* 3536 */
		.frame_length_lines = 0x9E2, /* 2530 */  
		.vt_pixel_clk = 134400000,
		.op_pixel_clk = 134400000,
		.binning_factor = 1,

	},
#endif 
/* LGE_CHANGE_E, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */
	{
		/* 30 fps preview */
		.x_output = 0x668, /* 1640 */
		.y_output = 0x4D0, /* 1232 */
		.line_length_pclk = 0xDD0, /* 3536 */
		.frame_length_lines = 0x4F2, /* 1266 */
		.vt_pixel_clk = 134400000,
		.op_pixel_clk = 134400000,
		.binning_factor = 1,
	},
	{
		/* not used */
		/* 30 fps video */
		.x_output = 0x7A0,
		.y_output = 0x44A,
		.line_length_pclk = 0xDAC,
		.frame_length_lines = 0x75C,
		.vt_pixel_clk = 200000000,
		.op_pixel_clk = 200000000,
		.binning_factor = 1,
	},
/* LGE_CHANGE_S, Camera Zero shutter lag patch, 2012.01.12 jungryoul.choi@lge.com */
	{
		/* not used */
		/* 22.5 fps */
		.x_output = 0x0CD0, /* 3280 */
		.y_output = 0x9A0, /* 2464 */
		.line_length_pclk = 0xDD0, /* 3536 */
		.frame_length_lines = 0x9D4, /* 2516 */
		.vt_pixel_clk = 199200000,
		.op_pixel_clk = 199200000,
		.binning_factor = 1,
	},
/* LGE_CHANGE_E, Camera Zero shutter lag patch, 2012.01.12 jungryoul.choi@lge.com */
};

static struct msm_camera_csi_params imx111_csic_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x12, // 0x14,
};

static struct msm_camera_csi_params *imx111_csic_params_array[] = {
	&imx111_csic_params,
	&imx111_csic_params,
	&imx111_csic_params,
	&imx111_csic_params,
};

static struct msm_camera_csid_vc_cfg imx111_cid_cfg[] = {
	{0, CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
	{2, CSI_RESERVED_DATA_0, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params imx111_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = ARRAY_SIZE(imx111_cid_cfg),
			.vc_cfg = imx111_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		//.settle_cnt = 0x14,
		.settle_cnt = 0x12,
	},
};

static struct msm_camera_csi2_params *imx111_csi_params_array[] = {
	&imx111_csi_params,
	&imx111_csi_params,
	&imx111_csi_params,
/* LGE_CHANGE_S, Camera Zero shutter lag patch, 2012.01.12 jungryoul.choi@lge.com */
	&imx111_csi_params,
/* LGE_CHANGE_E, Camera Zero shutter lag patch, 2012.01.12 jungryoul.choi@lge.com */
};

static struct msm_sensor_output_reg_addr_t imx111_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t imx111_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x0111,
};

static struct msm_sensor_exp_gain_info_t imx111_exp_gain_info = {
	.coarse_int_time_addr = 0x202,
	.global_gain_addr = 0x204,
	.vert_offset = 5,
};


static const struct i2c_device_id imx111_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&imx111_s_ctrl},
	{ }
};

static struct i2c_driver imx111_i2c_driver = {
	.id_table = imx111_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client imx111_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	pr_err("__jrchoi: %s: E\n", __func__);
	return i2c_add_driver(&imx111_i2c_driver);
}

static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

static struct v4l2_subdev_core_ops imx111_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

int32_t imx111_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;

/* LGE_CHANGE_S, migration for LA1730, 2012.08.03, donghyun.kwon@lge.com */
#if 0  
	v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
		NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
		PIX_0, ISPIF_OFF_IMMEDIATELY));
#endif
/* LGE_CHANGE_E, migration for LA1730, 2012.08.03, donghyun.kwon@lge.com */
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		msm_sensor_write_init_settings(s_ctrl);
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		if (res == 0) {
			msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
				(struct msm_camera_i2c_reg_conf *)
				imx111_comm_confs[0].conf,
				imx111_comm_confs[0].size,
				imx111_comm_confs[0].data_type);
         printk("[tykim] %s: csi_lane_assign not setting\n", __func__); /* LGE_CHANGE, Camera Setting for ES2, 2012.04.03, soojung.lim@lge.com */
		} else {
			msm_sensor_write_res_settings(s_ctrl, res);
			if (s_ctrl->curr_csi_params != s_ctrl->csi_params[res]) {
				s_ctrl->curr_csi_params = s_ctrl->csi_params[res];
				s_ctrl->curr_csi_params->csid_params.lane_assign =
					s_ctrl->sensordata->sensor_platform_info->
					csi_lane_params->csi_lane_assign;
				s_ctrl->curr_csi_params->csiphy_params.lane_mask =
					s_ctrl->sensordata->sensor_platform_info->
					csi_lane_params->csi_lane_mask;
				v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
						NOTIFY_CSID_CFG,
						&s_ctrl->curr_csi_params->csid_params);
/* LGE_CHANGE_S, migration for LA1730, 2012.08.03, donghyun.kwon@lge.com */
#if 0  
				v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
							NOTIFY_CID_CHANGE, NULL);
#endif
/* LGE_CHANGE_E, migration for LA1730, 2012.08.03, donghyun.kwon@lge.com */
				mb();
				v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
						NOTIFY_CSIPHY_CFG,
						&s_ctrl->curr_csi_params->csiphy_params);
				mb();
				msleep(20);
			}

			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
				output_settings[res].op_pixel_clk);
/* LGE_CHANGE_S, migration for LA1730, 2012.08.03, donghyun.kwon@lge.com */
#if 0  
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
				PIX_0, ISPIF_ON_FRAME_BOUNDARY));
#endif
/* LGE_CHANGE_E, migration for LA1730, 2012.08.03, donghyun.kwon@lge.com */
			s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
			msleep(30);
		}
	}
	printk("%s: X", __func__);
	return rc;
}

/* LGE_CHANGE_S, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */
void imx111_calculate_integration_time(struct msm_sensor_ctrl_t *s_ctrl, int res, int prev_res)
{
	
	int32_t rc = 0;
	uint16_t int_time = 0;
	uint32_t curr_clk = s_ctrl->msm_sensor_reg->output_settings[res].vt_pixel_clk/100000;
	uint32_t prev_clk = s_ctrl->msm_sensor_reg->output_settings[prev_res].vt_pixel_clk/100000;
	
	CDBG("%s : E\n", __func__);
	if(res != prev_res && prev_res != -1){
		rc = msm_camera_i2c_read(
				s_ctrl->sensor_i2c_client,
				s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, &int_time,
				MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0) {
			pr_err("%s: %s: read int time failed\n", __func__,
				s_ctrl->sensordata->sensor_name);
		}
		CDBG("%s : read int time = 0x%x\n", __func__, int_time);
	
		int_time = (uint32_t)((curr_clk*1000/prev_clk) * int_time)/1000;	
		CDBG("%s : write int time = 0x%x\n", __func__, int_time);
		rc = msm_camera_i2c_write(
				s_ctrl->sensor_i2c_client,
				s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, int_time,
				MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0) {
			pr_err("%s: %s: write int time failed\n", __func__,
				s_ctrl->sensordata->sensor_name);
		}
	}
	CDBG("%s : X\n", __func__);
}
/* LGE_CHANGE_E, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */

int32_t imx111_sensor_setting1(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;
	static int prev_res = -1;   /* LGE_CHANGE, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */

	printk("%s : E\n", __func__);
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		msm_sensor_write_init_settings(s_ctrl);
		csi_config = 0;
		prev_res = -1;	/* LGE_CHANGE, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */
 	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
 		CDBG("PERIODIC : %d\n", res);
 		if (res == 0) { // need to check
			msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
				(struct msm_camera_i2c_reg_conf *)
				imx111_comm_confs[0].conf,
				imx111_comm_confs[0].size,
				imx111_comm_confs[0].data_type);
			printk("[tykim] %s: csi_lane_assign not setting\n", __func__); /* LGE_CHANGE, Camera Setting for ES2, 2012.04.03, soojung.lim@lge.com */
		} else {
			msm_sensor_write_conf_array(
				s_ctrl->sensor_i2c_client,
				s_ctrl->msm_sensor_reg->mode_settings, res);
			msleep(30);
			if (!csi_config) {
				s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
				CDBG("CSI config in progress\n");
				v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
					NOTIFY_CSIC_CFG,
					s_ctrl->curr_csic_params);
				CDBG("CSI config is done\n");
				mb();
				msleep(30);
				csi_config = 1;
			}/* LGE_CHANGE_S, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */
			else {				
				imx111_calculate_integration_time(s_ctrl, res, prev_res);
				msleep(30);
			}
			/* LGE_CHANGE_E, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */
			
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_PCLK_CHANGE,
				&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

			s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
			msleep(50);
		}
 	}
	prev_res = res;	/* LGE_CHANGE, Fix line noise with 10fps and 444MHz, 2013-02-15 hyungtae.lee@lge.com */
	printk("%s : X\n", __func__);
	return rc;
}

/* LGE_CHANGE_S, Use qcomm fps code, 2012-06-22 jeongda.lee@lge.com */
#if 0
int32_t imx111_sensor_set_fps(struct msm_sensor_ctrl_t *s_ctrl,
						struct fps_cfg *fps)
{
	uint16_t total_lines_per_frame;
	int32_t rc = 0;
	s_ctrl->fps_divider = fps->fps_div;

	if (s_ctrl->curr_res != MSM_SENSOR_INVALID_RES) {
		uint16_t fl_read = 0;
		total_lines_per_frame = (uint16_t)
			((s_ctrl->curr_frame_length_lines) *
			s_ctrl->fps_divider/Q10);

		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			&fl_read, MSM_CAMERA_I2C_WORD_DATA);

		CDBG("%s: before_fl = %d, new_fl = %d", __func__, fl_read, total_lines_per_frame);

		if(fl_read < total_lines_per_frame) {
			pr_err("%s: Write new_fl (before_fl = %d, new_fl = %d)", __func__, fl_read, total_lines_per_frame);
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
				s_ctrl->sensor_output_reg_addr->frame_length_lines,
				total_lines_per_frame, MSM_CAMERA_I2C_WORD_DATA);
		}
	}
	return rc;
}
#endif
/* LGE_CHANGE_E, Use qcomm fps code, 2012-06-22 jeongda.lee@lge.com */

int32_t imx111_sensor_write_exp_gain1(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint32_t fl_lines;
	uint8_t offset;
	fl_lines = s_ctrl->curr_frame_length_lines;
	fl_lines = (fl_lines * s_ctrl->fps_divider) / Q10;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (line > (fl_lines - offset))
		fl_lines = line + offset;

	CDBG("\n%s:Gain:%d, Linecount:%d\n", __func__, gain, line);
	if (s_ctrl->curr_res == 0) { 
		msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
			(struct msm_camera_i2c_reg_conf *)
			imx111_comm_confs[1].conf,
			imx111_comm_confs[1].size,
			imx111_comm_confs[1].data_type);

		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			fl_lines, MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			line, MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
			MSM_CAMERA_I2C_WORD_DATA);

		msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
			(struct msm_camera_i2c_reg_conf *)
			imx111_comm_confs[2].conf,
			imx111_comm_confs[2].size,
			imx111_comm_confs[2].data_type);

		if (s_ctrl->curr_csic_params != s_ctrl->csic_params[s_ctrl->curr_res]) {
			s_ctrl->curr_csic_params = s_ctrl->csic_params[s_ctrl->curr_res];
			CDBG("CSI config in progress\n");
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
				s_ctrl->curr_csic_params);
			CDBG("CSI config is done\n");
			mb();
			msleep(30);
		}
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_PCLK_CHANGE,
				&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
	} else {
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
			MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
			MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
			MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	}
	return 0;
}

//Start :randy@qualcomm.com for calibration 2012.03.25
#define IMX111_EEPROM_SADDR 	0x50
#define IMX111_EEPROM_PAGE_SIZE	0x100
#define RED_START 	0x0A
#define GR_START 	0xE7
#define GB_START 	0xC4
#define BLUE_START 	0xA1
#define CRC_ADDR	0x7E
#define R_REF_ADDR  0x80
int32_t imx_i2c_read_eeprom_burst(unsigned char saddr, 
		unsigned char *rxdata, int length)
{
	int32_t rc = 0;
	unsigned char tmp_buf = 0;

	struct i2c_msg msgs[] = {
		{
			.addr  = saddr,
			.flags = 0,
			.len   = 1,
			.buf   = &tmp_buf,
		},
		{
			.addr  = saddr,
			.flags = I2C_M_RD,
			.len   = length,
			.buf   = rxdata,
		},
	};
	rc = i2c_transfer(imx111_s_ctrl.sensor_i2c_client->client->adapter, msgs, 2);
	if (rc < 0)
		pr_err("imx111_i2c_rxdata failed 0x%x\n", saddr);
	return rc;
}

static int imx111_read_eeprom_data(struct msm_sensor_ctrl_t *s_ctrl, struct sensor_cfg_data *cfg)
{
	int32_t rc = 0;
	uint8_t eepromdata[IMX111_EEPROM_PAGE_SIZE];
	uint32_t crc_5100= 0 /*, crc_2850 = 0*/;
	int i;

	printk("%s: E\n", __func__);

	memset(eepromdata, 0, sizeof(eepromdata));
	// for LSC data
	if(imx_i2c_read_eeprom_burst(IMX111_EEPROM_SADDR | 0x0 /* page_no:0 */, 
		eepromdata, IMX111_EEPROM_PAGE_SIZE) < 0) {
		pr_err("%s: Error Reading EEPROM : page_no:0 \n", __func__);
		return rc;
	}
	// for AWB data
	cfg->cfg.calib_info.r_over_g = (eepromdata[1]<<8) |eepromdata[0];
	printk("[QCTK_EEPROM] r_over_g = 0x%4x\n", cfg->cfg.calib_info.r_over_g);
	cfg->cfg.calib_info.b_over_g = (eepromdata[3]<<8) |eepromdata[2];
	printk("[QCTK_EEPROM] b_over_g = 0x%4x\n", cfg->cfg.calib_info.b_over_g);
	cfg->cfg.calib_info.gr_over_gb = (eepromdata[5]<<8) |eepromdata[4];
	printk("[QCTK_EEPROM] gr_over_gb = 0x%4x\n", cfg->cfg.calib_info.gr_over_gb);

	for (i = 0; i < ROLLOFF_CALDATA_SIZE; i++) {
		cfg->cfg.calib_info.rolloff.r_gain[i] = eepromdata[RED_START + i];
		crc_5100 += eepromdata[RED_START + i];
		//printk("[QCTK_EEPROM] R (0x%x, %d)\n", RED_START + i, eepromdata[RED_START + i]);
	}

	for (i = 0; i < IMX111_EEPROM_PAGE_SIZE - GR_START; i++) {
		cfg->cfg.calib_info.rolloff.gr_gain[i] = eepromdata[GR_START + i];
		crc_5100 += eepromdata[GR_START + i];
		//printk("[QCTK_EEPROM] GR (0x%x, %d)\n", GR_START + i, eepromdata[GR_START + i]);
	}

 	memset(eepromdata, 0, sizeof(eepromdata));
	if(imx_i2c_read_eeprom_burst(IMX111_EEPROM_SADDR | 0x1 /* page_no:1 */, 
		eepromdata, IMX111_EEPROM_PAGE_SIZE) < 0) {
		pr_err("%s: Error Reading EEPROM : page_no:1 \n", __func__);
		return rc;
	}

	// rolloff_size : 221, Gr_start: 231, eep_page_size: 256
	// i < 221 +231 - 256 (= 196) ==> in 2nd page of eeprom
	for (i = 0; i < ROLLOFF_CALDATA_SIZE + GR_START - IMX111_EEPROM_PAGE_SIZE; i++) {
		cfg->cfg.calib_info.rolloff.gr_gain[IMX111_EEPROM_PAGE_SIZE - GR_START + i] = eepromdata[i];
		crc_5100 += eepromdata[i];
		//printk("[QCTK_EEPROM] GR (0x%x, %d)\n", i, eepromdata[i]);
	}

	for (i = 0; i < IMX111_EEPROM_PAGE_SIZE - GB_START; i++) {
		cfg->cfg.calib_info.rolloff.gb_gain[i] = eepromdata[GB_START + i];
		crc_5100 += eepromdata[GB_START + i];
		//printk("[QCTK_EEPROM] GB (0x%x, %d)\n", GB_START + i, eepromdata[GB_START + i]);
	}

 	memset(eepromdata, 0, sizeof(eepromdata));
	if(imx_i2c_read_eeprom_burst(IMX111_EEPROM_SADDR | 0x2 /* page_no:2 */, 
		eepromdata, IMX111_EEPROM_PAGE_SIZE) < 0) {
		pr_err("%s: Error Reading EEPROM : page_no:2 \n", __func__);
		return rc;
	}
	for (i = 0; i < ROLLOFF_CALDATA_SIZE + GB_START - IMX111_EEPROM_PAGE_SIZE; i++) {
		cfg->cfg.calib_info.rolloff.gb_gain[IMX111_EEPROM_PAGE_SIZE - GB_START + i] = eepromdata[i];
		crc_5100 += eepromdata[i];
		//printk("[QCTK_EEPROM] GB (0x%x, %d)\n", i, eepromdata[i]);
	}

	for (i = 0; i < IMX111_EEPROM_PAGE_SIZE - BLUE_START; i++) {
		cfg->cfg.calib_info.rolloff.b_gain[i] = eepromdata[BLUE_START + i];
		crc_5100 += eepromdata[BLUE_START + i];
		//printk("[QCTK_EEPROM] B (0x%x, %d)\n", BLUE_START + i, eepromdata[BLUE_START + i]);
	}

 	memset(eepromdata, 0, sizeof(eepromdata));
	if(imx_i2c_read_eeprom_burst(IMX111_EEPROM_SADDR | 0x3 /* page_no:3 */, 
		eepromdata, ROLLOFF_CALDATA_SIZE + BLUE_START - IMX111_EEPROM_PAGE_SIZE + 4 /*checksum*/ + 17 /*red_ref*/) < 0) {
    	pr_err("%s: Error Reading EEPROM : page_no:3 \n", __func__);
    	return rc;
	}
	for (i = 0; i < ROLLOFF_CALDATA_SIZE + BLUE_START - IMX111_EEPROM_PAGE_SIZE; i++) {
		cfg->cfg.calib_info.rolloff.b_gain[IMX111_EEPROM_PAGE_SIZE - BLUE_START + i] = eepromdata[i];
		crc_5100 += eepromdata[i];
		//printk("[QCTK_EEPROM] B(0x%x, %d)\n", i, eepromdata[i]);
	}

	for (i = 0; i < 17; i++) {
		cfg->cfg.calib_info.rolloff.red_ref[i] = eepromdata[R_REF_ADDR + i];
		//printk("[QCTK_EEPROM] R_ref(0x%x)\n", cfg->cfg.calib_info.rolloff.red_ref[i]);
		}

	printk("%s: crc_from_eeprom(0x%x), cal_crc(0x%x)\n", __func__,
		(eepromdata[CRC_ADDR] << 8) | eepromdata[CRC_ADDR+1], crc_5100& 0xFFFF);
	#if 0	
	// CRC check
	if (((eepromdata[CRC_ADDR]<<8)+eepromdata[CRC_ADDR+1]) != crc_5100)
		{
			pr_err("%s: CRC error R(read crc:0x%x, cal crc:0x%x)\n", __func__, 
			(eepromdata[CRC_ADDR]<<8)+eepromdata[CRC_ADDR+1], crc_red);
			// return -EFAULT;
		}
	#endif

	printk("%s: X\n", __func__);
	return 0;
}
//End :randy@qualcomm.com for calibration 2012.03.25
////////////////////////////////////////////////


static struct v4l2_subdev_video_ops imx111_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops imx111_subdev_ops = {
	.core = &imx111_subdev_core_ops,
	.video  = &imx111_subdev_video_ops,
};

int32_t imx111_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	printk("%s: E %d\n", __func__, __LINE__);
	s_ctrl->reg_ptr = kzalloc(sizeof(struct regulator *)
			* data->sensor_platform_info->num_vreg, GFP_KERNEL);
	if (!s_ctrl->reg_ptr) {
		pr_err("%s: could not allocate mem for regulators\n", __func__);
		return -ENOMEM;
	}

	rc = msm_camera_request_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		goto request_gpio_failed;
	}

	/* RESET LOW*/
	if (data->sensor_reset_enable) {
		rc = gpio_request(data->sensor_reset, "CAM_RESET");
		if (rc < 0)
			pr_err("%s: gpio_request:CAM_RESET %d failed\n", __func__, data->sensor_reset);
		rc = gpio_direction_output(data->sensor_reset, 0);
		if (rc < 0)
			pr_err("%s: gpio:CAM_RESET %d direction can't be set\n", __func__, data->sensor_reset);
	}

	rc = msm_camera_config_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
		goto config_gpio_failed;
	}	
	usleep_range(1000, 2000);

	/* RESET HIGH */
	if (data->sensor_reset_enable) {
		rc = gpio_direction_output(data->sensor_reset, 1);
		if (rc < 0)
			pr_err("%s: gpio:CAM_RESET %d direction can't be set\n", __func__, data->sensor_reset);
	}
	usleep_range(1000, 2000);

	if (s_ctrl->clk_rate != 0)
		cam_clk_info->clk_rate = s_ctrl->clk_rate;

	rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n", __func__);
		goto enable_clk_failed;
	}	
	usleep_range(1000, 2000);	

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(1);

	printk("%s: X %d\n", __func__, __LINE__);
	return rc;

enable_clk_failed:
	msm_camera_config_gpio_table(data, 0);

config_gpio_failed:
	if (data->sensor_reset_enable) {
		gpio_direction_output(data->sensor_reset, 0);
		usleep_range(1000, 2000);
	}

	msm_camera_request_gpio_table(data, 0);
request_gpio_failed:
	kfree(s_ctrl->reg_ptr);
	printk("%s: X %d failed\n", __func__, __LINE__);
	return rc;
}

int32_t imx111_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	int32_t rc = 0;

	printk("%s : E\n", __func__);

	/* RESET LOW*/
	if (data->sensor_reset_enable) {
		rc = gpio_direction_output(data->sensor_reset, 0);
		if (rc < 0)
			pr_err("%s: gpio:CAM_RESET %d direction can't be set\n", __func__, data->sensor_reset);
		gpio_free(data->sensor_reset);
	}

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(0);
	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);
	msm_camera_config_gpio_table(data, 0);
	msm_camera_request_gpio_table(data, 0);
	kfree(s_ctrl->reg_ptr);
	printk("%s : X\n", __func__);
	return 0;
}

static struct msm_sensor_fn_t imx111_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
#if 0 /* removed at M8960AAAAANLYA1022 */
	.sensor_get_prev_lines_pf = msm_sensor_get_prev_lines_pf,
	.sensor_get_prev_pixels_pl = msm_sensor_get_prev_pixels_pl,
	.sensor_get_pict_lines_pf = msm_sensor_get_pict_lines_pf,
	.sensor_get_pict_pixels_pl = msm_sensor_get_pict_pixels_pl,
	.sensor_get_pict_max_exp_lc = msm_sensor_get_pict_max_exp_lc,
	.sensor_get_pict_fps = msm_sensor_get_pict_fps,
#endif
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = imx111_sensor_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = imx111_sensor_write_exp_gain1,
	.sensor_setting = msm_sensor_setting,
	.sensor_csi_setting = imx111_sensor_setting1, // V7
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_get_eeprom_data = imx111_read_eeprom_data,	//randy@qualcomm.com for calibration 2012.03.25
	.sensor_power_up = imx111_sensor_power_up, // msm_sensor_power_up,
	.sensor_power_down = imx111_sensor_power_down, // msm_sensor_power_down,
	.sensor_adjust_frame_lines = msm_sensor_adjust_frame_lines,
	.sensor_get_csi_params = msm_sensor_get_csi_params,  /* LGE_CHANGE, migration for LA1730, 2012.08.03, donghyun.kwon@lge.com */

};

static struct msm_sensor_reg_t imx111_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = imx111_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(imx111_start_settings),
	.stop_stream_conf = imx111_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(imx111_stop_settings),
	.group_hold_on_conf = imx111_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(imx111_groupon_settings),
	.group_hold_off_conf = imx111_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(imx111_groupoff_settings),
	.init_settings = &imx111_init_conf[0],
	.init_size = ARRAY_SIZE(imx111_init_conf),
	.mode_settings = &imx111_confs[0],
	.output_settings = &imx111_dimensions[0],
	.num_conf = ARRAY_SIZE(imx111_confs),
};

static struct msm_sensor_ctrl_t imx111_s_ctrl = {
	.msm_sensor_reg = &imx111_regs,
	.sensor_i2c_client = &imx111_sensor_i2c_client,
	.sensor_i2c_addr = 0x34,
	.sensor_output_reg_addr = &imx111_reg_addr,
	.sensor_id_info = &imx111_id_info,
	.sensor_exp_gain_info = &imx111_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &imx111_csic_params_array[0], // V7
	.csi_params = &imx111_csi_params_array[0],
	.msm_sensor_mutex = &imx111_mut,
	.sensor_i2c_driver = &imx111_i2c_driver,
	.sensor_v4l2_subdev_info = imx111_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(imx111_subdev_info),
	.sensor_v4l2_subdev_ops = &imx111_subdev_ops,
	.func_tbl = &imx111_func_tbl,
.clk_rate = MSM_SENSOR_MCLK_24HZ, // ADD
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Sony 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
