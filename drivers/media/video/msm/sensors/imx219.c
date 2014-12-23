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
#define SENSOR_NAME "imx219"
#define PLATFORM_DRIVER_NAME "msm_camera_imx219"
#define imx219_obj imx219_##obj

#define IMX219_USE_FRAME_BANK
#ifdef IMX219_USE_FRAME_BANK
#define IMX219_FRAME_BANK_ENABLE_ADDR 0x0150
#endif

DEFINE_MUTEX(imx219_mut);
static struct msm_sensor_ctrl_t imx219_s_ctrl;

static struct msm_camera_i2c_reg_conf imx219_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf imx219_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf imx219_groupon_settings[] = {
	//{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf imx219_groupoff_settings[] = {
	//{0x104, 0x00},
};

static struct msm_camera_i2c_reg_conf imx219_prev_settings[] = {
#ifdef IMX219_USE_FRAME_BANK
    /*AE start point 2014 0206_soyoung.baek@lge.com*/
/*
    {0x015A, 0x04}, //coarse int time  
	{0x015B, 0xF4},
    {0x025A, 0x04},
	{0x025B, 0xF4},
    {0x0157, 0xDD}, //Again 
	{0x0257, 0xDD}, 
    {0x0158, 0x01}, //Dgain 
	{0x0258, 0x01},
    {0x0159, 0x00}, 
	{0x0259, 0x00},
*/	
	{0x0160, 0x04},
	{0x0161, 0xF2},
	{0x0162, 0x0D},
	{0x0163, 0x78},
	{0x0164, 0x00},
	{0x0165, 0x00},
	{0x0166, 0x0C},
	{0x0167, 0xCF},
	{0x0168, 0x00},
	{0x0169, 0x00},
	{0x016A, 0x09},
	{0x016B, 0x9F},
	{0x016C, 0x06},
	{0x016D, 0x68},
	{0x016E, 0x04},
	{0x016F, 0xD0},
	{0x0170, 0x01},
	{0x0171, 0x01},
	{0x0174, 0x01},
	{0x0175, 0x01},
	{0x018C, 0x0A},
	{0x018D, 0x0A},
	{0x0301, 0x05},
	{0x0303, 0x02},
	{0x0304, 0x03},
	{0x0305, 0x03},
	{0x0306, 0x00},
	{0x0307, 0x52},
	{0x0309, 0x0A},
	{0x030B, 0x01},
	{0x030C, 0x00},
	{0x030D, 0x52},
	{0x4767, 0x0F},
	{0x4750, 0x14},
	{0x47B4, 0x14},
	{0x0172, 0x03}, // XY flip

	{0x0260, 0x04},
	{0x0261, 0xF2},
	{0x0262, 0x0D},
	{0x0263, 0x78},
	{0x0264, 0x00},
	{0x0265, 0x00},
	{0x0266, 0x0C},
	{0x0267, 0xCF},
	{0x0268, 0x00},
	{0x0269, 0x00},
	{0x026A, 0x09},
	{0x026B, 0x9F},
	{0x026C, 0x06},
	{0x026D, 0x68},
	{0x026E, 0x04},
	{0x026F, 0xD0},
	{0x0270, 0x01},
	{0x0271, 0x01},
	{0x0274, 0x01},
	{0x0275, 0x01},
	{0x028C, 0x0A},
	{0x028D, 0x0A},
	{0x0272, 0x03}, // XY flip
#else
    {0x015A, 0x04}, //coarse int time  
	{0x015B, 0xF4},
    {0x025A, 0x04},
	{0x025B, 0xF4},
    {0x0157, 0xDD}, //Again 
	{0x0257, 0xDD}, 
    {0x0158, 0x01}, //Dgain 
	{0x0258, 0x01},
    {0x0159, 0x00}, 
	{0x0259, 0x00},
	
	{0x0160, 0x04}, // frame length
	{0x0161, 0xF2},
	{0x0162, 0x0D}, // line length
	{0x0163, 0x78},
	{0x0164, 0x00},
	{0x0165, 0x00},
	{0x0166, 0x0C},
	{0x0167, 0xCF},
	{0x0168, 0x00},
	{0x0169, 0x00},
	{0x016A, 0x09},
	{0x016B, 0x9F},
	{0x016C, 0x06}, // x output
	{0x016D, 0x68},		
	{0x016E, 0x04}, // y output
	{0x016F, 0xD0},			
	{0x0170, 0x01},
	{0x0171, 0x01},	
	{0x0174, 0x01},
	{0x0175, 0x01},
	{0x018C, 0x0A},
	{0x018D, 0x0A},	
	{0x0301, 0x05},
	{0x0303, 0x02},
	{0x0304, 0x03},
	{0x0305, 0x03},
	{0x0306, 0x00},
	{0x0307, 0x52},
	{0x0309, 0x0A},
	{0x030B, 0x01},
	{0x030C, 0x00},
	{0x030D, 0x52},
	{0x4767, 0x0F},
	{0x4750, 0x14},
	{0x47B4, 0x14},
	{0x0172, 0x03}, // XY flip
#endif
};

static struct msm_camera_i2c_reg_conf imx219_video_settings[] = {
};

static struct msm_camera_i2c_reg_conf imx219_snap_settings[] = {
	{0x0160, 0x09},
	{0x0161, 0xC8},
	{0x0162, 0x0D},
	{0x0163, 0x78},
	{0x0164, 0x00},
	{0x0165, 0x00},
	{0x0166, 0x0C},
	{0x0167, 0xCF},
	{0x0168, 0x00},
	{0x0169, 0x00},
	{0x016A, 0x09},
	{0x016B, 0x9F},
	{0x016C, 0x0C},
	{0x016D, 0xD0},
	{0x016E, 0x09},
	{0x016F, 0xA0},
	{0x0170, 0x01},
	{0x0171, 0x01},
	{0x0174, 0x00},
	{0x0175, 0x00},
	{0x018C, 0x0A},
	{0x018D, 0x0A},
	{0x0301, 0x05},
	{0x0303, 0x02},
	{0x0304, 0x03},
	{0x0305, 0x03},
	{0x0306, 0x00},
	{0x0307, 0x36},
	{0x0309, 0x0A},
	{0x030B, 0x01},
	{0x030C, 0x00},
	{0x030D, 0x36},
	{0x4767, 0x0F},
	{0x4750, 0x14},
	{0x47B4, 0x14},
	{0x0172, 0x03}, // XY flip
};

static struct msm_camera_i2c_reg_conf imx219_recommend_settings[] = {	
	{0x30EB, 0x05},
	{0x30EB, 0x0C},
	{0x300A, 0xFF},
	{0x300B, 0xFF},
	{0x30EB, 0x05},
	{0x30EB, 0x09}, 	
	{0x0114, 0x01}, 
	{0x0128, 0x00},
	{0x012A, 0x18},
	{0x012B, 0x00},
	{0x5041, 0x00}, // delete embedded data line 0

	 /*AE start point 2014 0206_soyoung.baek@lge.com*/
    {0x015A, 0x04}, //coarse int time  
	{0x015B, 0xF4},
    {0x025A, 0x04},
	{0x025B, 0xF4},
    {0x0157, 0xDD}, //Again 
	{0x0257, 0xDD}, 
    {0x0158, 0x01}, //Dgain 
	{0x0258, 0x01},
    {0x0159, 0x00}, 
	{0x0259, 0x00},
};

static struct v4l2_subdev_info imx219_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array imx219_init_conf[] = {
	{&imx219_recommend_settings[0],
	ARRAY_SIZE(imx219_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array imx219_confs[] = {
	{&imx219_snap_settings[0],
	ARRAY_SIZE(imx219_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx219_prev_settings[0],
	ARRAY_SIZE(imx219_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx219_video_settings[0],
	ARRAY_SIZE(imx219_video_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
/* LGE_CHANGE_S, Camera Zero shutter lag patch, 2012.01.12 jungryoul.choi@lge.com */
	{&imx219_snap_settings[0],
	ARRAY_SIZE(imx219_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
/* LGE_CHANGE_E, Camera Zero shutter lag patch, 2012.01.12 jungryoul.choi@lge.com */
};

static struct msm_sensor_output_info_t imx219_dimensions[] = {
	{
		/* 10fps 432Mbps */
		.x_output = 0x0CD0, /* 3280 */
		.y_output = 0x09A0, /* 2464 */
		.line_length_pclk = 0x0D78, /* 3448 */
		.frame_length_lines = 0x09C8, /* 2504 */  
		.vt_pixel_clk = 86400000,
		.op_pixel_clk = 86400000,
		.binning_factor = 1,
	},	
	{
		/* 30 fps preview 656Mbps */
		.x_output = 0x0668, /* 1640 */		
		.y_output = 0x04D0, /* 1232 */
		.line_length_pclk =  0x0D78, /* 3448 */
		.frame_length_lines = 0x04F2, /* 1266 */
		.vt_pixel_clk = 131200000,
		.op_pixel_clk = 131200000,
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

static struct msm_camera_csi_params imx219_csic_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x12,
};

static struct msm_camera_csi_params *imx219_csic_params_array[] = {
	&imx219_csic_params,
	&imx219_csic_params,
	&imx219_csic_params,
	&imx219_csic_params,
};

static struct msm_camera_csid_vc_cfg imx219_cid_cfg[] = {
	{0, CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
	{2, CSI_RESERVED_DATA_0, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params imx219_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = ARRAY_SIZE(imx219_cid_cfg),
			.vc_cfg = imx219_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		.settle_cnt = 0x12,		
	},
};

static struct msm_camera_csi2_params *imx219_csi_params_array[] = {
	&imx219_csi_params,
	&imx219_csi_params,
	&imx219_csi_params,
/* LGE_CHANGE_S, Camera Zero shutter lag patch, 2012.01.12 jungryoul.choi@lge.com */
	&imx219_csi_params,
/* LGE_CHANGE_E, Camera Zero shutter lag patch, 2012.01.12 jungryoul.choi@lge.com */
};

static struct msm_sensor_output_reg_addr_t imx219_reg_addr = {
	.x_output = 0x016C,
	.y_output = 0x016E,
	.line_length_pclk = 0x0162,
	.frame_length_lines = 0x0160,
/*  LGE_CHANGE_S : frame bank register 2014-01-27 sungmin.cho@lge.com */
#ifdef IMX219_USE_FRAME_BANK
	.frame_length_lines_bank = {0x0160, 0x0260},
#endif
/*  LGE_CHANGE_S : frame bank register 2014-01-27 sungmin.cho@lge.com */
};

static struct msm_sensor_id_info_t imx219_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x0219,
};

static struct msm_sensor_exp_gain_info_t imx219_exp_gain_info = {
	.coarse_int_time_addr = 0x015A,
/*	LGE_CHANGE_S : frame bank register 2014-01-27 sungmin.cho@lge.com */
#ifdef IMX219_USE_FRAME_BANK
	.coarse_int_time_addr_bank = {0x015A, 0x025A},
#endif
/*	LGE_CHANGE_S : frame bank register 2014-01-27 sungmin.cho@lge.com */
	.global_gain_addr = 0x0157,
/*	LGE_CHANGE_S : frame bank register 2014-01-27 sungmin.cho@lge.com */
#ifdef IMX219_USE_FRAME_BANK
	.global_again_addr_bank = {0x0157, 0x0257},
	.global_dgain_addr_bank = {0x0158, 0x0258},
#endif
/*	LGE_CHANGE_S : frame bank register 2014-01-27 sungmin.cho@lge.com */
	.vert_offset = 5,
};


static const struct i2c_device_id imx219_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&imx219_s_ctrl},
	{ }
};

static struct i2c_driver imx219_i2c_driver = {
	.id_table = imx219_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client imx219_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	pr_err("__jrchoi: %s: E\n", __func__);
	return i2c_add_driver(&imx219_i2c_driver);
}

static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

static struct v4l2_subdev_core_ops imx219_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

/* LGE_CHANGE_S : 2014-01-27 sungmin.cho@lge.com IMX219 bank register control  */
#ifdef IMX219_USE_FRAME_BANK
void imx219_sensor_read_frame_bank(struct msm_sensor_ctrl_t *s_ctrl, uint16_t *bank)
{
	uint16_t temp; // 0x00 : Table A, 0x01 : Table B
	
	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, IMX219_FRAME_BANK_ENABLE_ADDR, &temp, MSM_CAMERA_I2C_BYTE_DATA);
	*bank = temp>>1;		
}

void imx219_sensor_write_frame_bank(struct msm_sensor_ctrl_t *s_ctrl, uint16_t bank)
{	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, IMX219_FRAME_BANK_ENABLE_ADDR, bank, MSM_CAMERA_I2C_BYTE_DATA);		
}
#endif
/* LGE_CHANGE_E : 2014-01-27 sungmin.cho@lge.com IMX219 bank register control  */

void imx219_calculate_integration_time(struct msm_sensor_ctrl_t *s_ctrl, int res, int prev_res)
{	
	int32_t rc = 0;
	uint16_t int_time = 0;
	uint32_t curr_clk = s_ctrl->msm_sensor_reg->output_settings[res].vt_pixel_clk/100000;
	uint32_t prev_clk = s_ctrl->msm_sensor_reg->output_settings[prev_res].vt_pixel_clk/100000;
#ifdef IMX219_USE_FRAME_BANK
	uint16_t bank;
#endif
	
	CDBG("%s : E\n", __func__);
	if(res != prev_res && prev_res != -1){
#ifdef IMX219_USE_FRAME_BANK
		imx219_sensor_read_frame_bank(s_ctrl, &bank);

		rc = msm_camera_i2c_read(
				s_ctrl->sensor_i2c_client,
				s_ctrl->sensor_exp_gain_info->coarse_int_time_addr_bank[bank], &int_time,
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
				s_ctrl->sensor_exp_gain_info->coarse_int_time_addr_bank[bank], int_time,
				MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0) {
			pr_err("%s: %s: write int time failed\n", __func__,
				s_ctrl->sensordata->sensor_name);
		}
		imx219_sensor_write_frame_bank(s_ctrl, bank);
#else
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
#endif
		
		msleep(30);
	}	
	CDBG("%s : X\n", __func__);
}

int32_t imx219_sensor_setting1(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;
	static int prev_res = -1;

	printk("%s : update_type = %d, res = %dE\n", __func__, update_type, res);
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		msm_sensor_write_init_settings(s_ctrl);
		csi_config = 0;
		prev_res = -1;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
		msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			s_ctrl->msm_sensor_reg->mode_settings, res);
		if(res != 0) {// preview
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
			} else {
				imx219_calculate_integration_time(s_ctrl, res, prev_res);
			}
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_PCLK_CHANGE,
				&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

			s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
			msleep(50);
		}
	}
	prev_res = res;
	printk("%s : X\n", __func__);
	return rc;
}


int32_t imx219_sensor_write_exp_gain1(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	/*AE hunting debug_bsy_2014.01.22*/
	uint32_t fl_lines, again, dgain;
	uint8_t offset;
#ifdef IMX219_USE_FRAME_BANK
	uint16_t bank;
#endif

	fl_lines = s_ctrl->curr_frame_length_lines;
	fl_lines = (fl_lines * s_ctrl->fps_divider) / Q10;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (line > (fl_lines - offset))
		fl_lines = line + offset;

	if((gain >> 8) >0){
	  again =	0xE0;
	  dgain = gain;
	} else {
	  again = gain;
	  dgain = 0x0100;
	}

	pr_err("\n%s:Gain:%d, Linecount:%d, Again: %d, Dgain: %d\n", __func__, gain, line, again, dgain);
	
#ifdef IMX219_USE_FRAME_BANK
	if (s_ctrl->curr_res == 0) { // snapshot
		bank = 0; // A Table only
	}else {
		imx219_sensor_read_frame_bank(s_ctrl, &bank);
		bank = (bank+1)%2;
	}
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines_bank[bank],
		fl_lines, MSM_CAMERA_I2C_WORD_DATA);
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr_bank[bank],
		line, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_again_addr_bank[bank],
		again,
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_dgain_addr_bank[bank],
		dgain,
		MSM_CAMERA_I2C_WORD_DATA);
	
	imx219_sensor_write_frame_bank(s_ctrl, bank);

	if (s_ctrl->curr_res == 0) { // snapshot
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
	}
#else
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines,
		fl_lines, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		line, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, again,
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x0158, dgain,
		MSM_CAMERA_I2C_WORD_DATA);
	
	if (s_ctrl->curr_res == 0) { // snapshot
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
	}
#endif
	return 0;
}

//Start :randy@qualcomm.com for calibration 2012.03.25
#define IMX219_EEPROM_SADDR 	0x50
#define IMX219_EEPROM_PAGE_SIZE	0x100
#define RED_START 	0x0A
#define GR_START 	0xE7
#define GB_START 	0xC4
#define BLUE_START 	0xA1
#define CRC_ADDR	0x7E
#define R_REF_ADDR  0x80
int32_t imx219_i2c_read_eeprom_burst(unsigned char saddr, 
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
	rc = i2c_transfer(imx219_s_ctrl.sensor_i2c_client->client->adapter, msgs, 2);
	if (rc < 0)
		pr_err("imx219_i2c_rxdata failed 0x%x\n", saddr);
	return rc;
}

static int imx219_read_eeprom_data(struct msm_sensor_ctrl_t *s_ctrl, struct sensor_cfg_data *cfg)
{
	int32_t rc = 0;
	uint8_t eepromdata[IMX219_EEPROM_PAGE_SIZE];
	uint32_t crc_5100= 0 /*, crc_2850 = 0*/;
	int i;

	printk("%s: E\n", __func__);

	memset(eepromdata, 0, sizeof(eepromdata));
	// for LSC data
	if(imx219_i2c_read_eeprom_burst(IMX219_EEPROM_SADDR | 0x0 /* page_no:0 */, 
		eepromdata, IMX219_EEPROM_PAGE_SIZE) < 0) {
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

	for (i = 0; i < IMX219_EEPROM_PAGE_SIZE - GR_START; i++) {
		cfg->cfg.calib_info.rolloff.gr_gain[i] = eepromdata[GR_START + i];
		crc_5100 += eepromdata[GR_START + i];
		//printk("[QCTK_EEPROM] GR (0x%x, %d)\n", GR_START + i, eepromdata[GR_START + i]);
	}

 	memset(eepromdata, 0, sizeof(eepromdata));
	if(imx219_i2c_read_eeprom_burst(IMX219_EEPROM_SADDR | 0x1 /* page_no:1 */, 
		eepromdata, IMX219_EEPROM_PAGE_SIZE) < 0) {
		pr_err("%s: Error Reading EEPROM : page_no:1 \n", __func__);
		return rc;
	}

	// rolloff_size : 221, Gr_start: 231, eep_page_size: 256
	// i < 221 +231 - 256 (= 196) ==> in 2nd page of eeprom
	for (i = 0; i < ROLLOFF_CALDATA_SIZE + GR_START - IMX219_EEPROM_PAGE_SIZE; i++) {
		cfg->cfg.calib_info.rolloff.gr_gain[IMX219_EEPROM_PAGE_SIZE - GR_START + i] = eepromdata[i];
		crc_5100 += eepromdata[i];
		//printk("[QCTK_EEPROM] GR (0x%x, %d)\n", i, eepromdata[i]);
	}

	for (i = 0; i < IMX219_EEPROM_PAGE_SIZE - GB_START; i++) {
		cfg->cfg.calib_info.rolloff.gb_gain[i] = eepromdata[GB_START + i];
		crc_5100 += eepromdata[GB_START + i];
		//printk("[QCTK_EEPROM] GB (0x%x, %d)\n", GB_START + i, eepromdata[GB_START + i]);
	}

 	memset(eepromdata, 0, sizeof(eepromdata));
	if(imx219_i2c_read_eeprom_burst(IMX219_EEPROM_SADDR | 0x2 /* page_no:2 */, 
		eepromdata, IMX219_EEPROM_PAGE_SIZE) < 0) {
		pr_err("%s: Error Reading EEPROM : page_no:2 \n", __func__);
		return rc;
	}
	for (i = 0; i < ROLLOFF_CALDATA_SIZE + GB_START - IMX219_EEPROM_PAGE_SIZE; i++) {
		cfg->cfg.calib_info.rolloff.gb_gain[IMX219_EEPROM_PAGE_SIZE - GB_START + i] = eepromdata[i];
		crc_5100 += eepromdata[i];
		//printk("[QCTK_EEPROM] GB (0x%x, %d)\n", i, eepromdata[i]);
	}

	for (i = 0; i < IMX219_EEPROM_PAGE_SIZE - BLUE_START; i++) {
		cfg->cfg.calib_info.rolloff.b_gain[i] = eepromdata[BLUE_START + i];
		crc_5100 += eepromdata[BLUE_START + i];
		//printk("[QCTK_EEPROM] B (0x%x, %d)\n", BLUE_START + i, eepromdata[BLUE_START + i]);
	}

 	memset(eepromdata, 0, sizeof(eepromdata));
	if(imx219_i2c_read_eeprom_burst(IMX219_EEPROM_SADDR | 0x3 /* page_no:3 */, 
		eepromdata, ROLLOFF_CALDATA_SIZE + BLUE_START - IMX219_EEPROM_PAGE_SIZE + 4 /*checksum*/ + 17 /*red_ref*/) < 0) {
    	pr_err("%s: Error Reading EEPROM : page_no:3 \n", __func__);
    	return rc;
	}
	for (i = 0; i < ROLLOFF_CALDATA_SIZE + BLUE_START - IMX219_EEPROM_PAGE_SIZE; i++) {
		cfg->cfg.calib_info.rolloff.b_gain[IMX219_EEPROM_PAGE_SIZE - BLUE_START + i] = eepromdata[i];
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


static struct v4l2_subdev_video_ops imx219_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops imx219_subdev_ops = {
	.core = &imx219_subdev_core_ops,
	.video  = &imx219_subdev_video_ops,
};

int32_t imx219_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
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
	
	if (s_ctrl->clk_rate != 0)
			cam_clk_info->clk_rate = s_ctrl->clk_rate;
	
		rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
			cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
		if (rc < 0) {
			pr_err("%s: clk enable failed\n", __func__);
			goto enable_clk_failed;
		}	
		usleep_range(1000, 2000);	

	/* RESET HIGH */
	if (data->sensor_reset_enable) {
		rc = gpio_direction_output(data->sensor_reset, 1);
		if (rc < 0)
			pr_err("%s: gpio:CAM_RESET %d direction can't be set\n", __func__, data->sensor_reset);
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

int32_t imx219_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
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
	
	msm_camera_config_gpio_table(data, 0);
	msm_camera_request_gpio_table(data, 0);
	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);
	kfree(s_ctrl->reg_ptr);
	printk("%s : X\n", __func__);
	return 0;
}

static struct msm_sensor_fn_t imx219_func_tbl = {
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
	.sensor_write_exp_gain = imx219_sensor_write_exp_gain1, // msm_sensor_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = imx219_sensor_write_exp_gain1, // msm_sensor_write_exp_gain1,
	.sensor_setting = msm_sensor_setting,
	.sensor_csi_setting = imx219_sensor_setting1,//msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_get_eeprom_data = imx219_read_eeprom_data,	//randy@qualcomm.com for calibration 2012.03.25
	.sensor_power_up = imx219_sensor_power_up,
	.sensor_power_down = imx219_sensor_power_down,
	.sensor_adjust_frame_lines = msm_sensor_adjust_frame_lines,
	.sensor_get_csi_params = msm_sensor_get_csi_params,  /* LGE_CHANGE, migration for LA1730, 2012.08.03, donghyun.kwon@lge.com */

};

static struct msm_sensor_reg_t imx219_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = imx219_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(imx219_start_settings),
	.stop_stream_conf = imx219_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(imx219_stop_settings),
	.group_hold_on_conf = imx219_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(imx219_groupon_settings),
	.group_hold_off_conf = imx219_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(imx219_groupoff_settings),
	.init_settings = &imx219_init_conf[0],
	.init_size = ARRAY_SIZE(imx219_init_conf),
	.mode_settings = &imx219_confs[0],
	.output_settings = &imx219_dimensions[0],
	.num_conf = ARRAY_SIZE(imx219_confs),
};

static struct msm_sensor_ctrl_t imx219_s_ctrl = {
	.msm_sensor_reg = &imx219_regs,
	.sensor_i2c_client = &imx219_sensor_i2c_client,
	.sensor_i2c_addr = 0x34, // 0x20,
	.sensor_output_reg_addr = &imx219_reg_addr,
	.sensor_id_info = &imx219_id_info,
	.sensor_exp_gain_info = &imx219_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &imx219_csic_params_array[0], // V7
	.csi_params = &imx219_csi_params_array[0],
	.msm_sensor_mutex = &imx219_mut,
	.sensor_i2c_driver = &imx219_i2c_driver,
	.sensor_v4l2_subdev_info = imx219_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(imx219_subdev_info),
	.sensor_v4l2_subdev_ops = &imx219_subdev_ops,
	.func_tbl = &imx219_func_tbl,
.clk_rate = MSM_SENSOR_MCLK_24HZ, // ADD
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Sony 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
