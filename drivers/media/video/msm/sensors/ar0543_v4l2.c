/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
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
#include "fih_read_otp.h"//MM-MC-ImplementReadOtpDataFeature-00+

#define SENSOR_NAME "ar0543"
#define PLATFORM_DRIVER_NAME "msm_camera_ar0543"
#define ar0543_obj ar0543_##obj

DEFINE_MUTEX(ar0543_mut);
static struct msm_sensor_ctrl_t ar0543_s_ctrl;
static int32_t v_fps = 1024; /* MM-UW-Fix MMS can't record 23s-00+} */

static struct msm_camera_i2c_reg_conf ar0543_start_settings[] = {
	{0x0100, 0x01, 1},
};

static struct msm_camera_i2c_reg_conf ar0543_stop_settings[] = {
	{0x0100, 0x00, 1},
};

static struct msm_camera_i2c_reg_conf ar0543_groupon_settings[] = {
	{0x104, 0x01, 1},
};

static struct msm_camera_i2c_reg_conf ar0543_groupoff_settings[] = {
	{0x104, 0x00, 1},
};

static struct msm_camera_i2c_reg_conf ar0543_prev_settings[] = {
    //Preview 1296x972 30fps
    { 0x0104, 0x01, MSM_CAMERA_I2C_BYTE_DATA},  // GROUPED_PARAMETER_HOLD
    { 0x3004, 0x0008}, // X_ADDR_START_
    { 0x3008, 0x0A25}, // X_ADDR_END_
    { 0x3002, 0x0008}, // Y_ADDR_START_
    { 0x3006, 0x079D}, // Y_ADDR_END_
    { 0x3040, 0x04C3}, // READ_MODE
    { 0x034C, 0x0510}, // X_OUTPUT_SIZE
    { 0x034E, 0x03CC}, // Y_OUTPUT_SIZE
    
    { 0x300C, 0x0CF0}, //line_length_pck
    { 0x300A, 0x0457}, //frame_length_lines

    { 0x3012, 0x0495}, //coarse_integration_time // 33.33ms integration time         
    { 0x3014, 0x0908}, //fine_integration_time               
    { 0x3010, 0x0184}, //fine_correction             
     
    { 0x0104, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //group parameter Hold 
};

static struct msm_camera_i2c_reg_conf ar0543_video_settings[] = {
    /* Video 1288x728 30fps */
    { 0x0104, 0x01, MSM_CAMERA_I2C_BYTE_DATA},  // GROUPED_PARAMETER_HOLD
    { 0x3004, 0x0008},  // X_ADDR_START_
    { 0x3008, 0x0A15},  // X_ADDR_END_
    { 0x3002, 0x0008},  // Y_ADDR_START_
    { 0x3006, 0x05B5},  // Y_ADDR_END_
    { 0x3040, 0x04C3},  // READ_MODE
    { 0x034C, 0x0508},  // X_OUTPUT_SIZE
    { 0x034E, 0x02D8},  // Y_OUTPUT_SIZE
    { 0x300C, 0x11f2},  // LINE_LENGTH_PCK		// akeemchen modify to have 30fps exactuly
    { 0x300A, 0x0321},  // FRAME_LENGTH_LINES_
    { 0x3012, 0x0320},  // COARSE_INTEGRATION_TIME_
    { 0x3014, 0x05F8},  // FINE_INTEGRATION_TIME_
    { 0x3010, 0x0184},  // FINE_CORRECTION
    { 0x0104, 0x00, MSM_CAMERA_I2C_BYTE_DATA},  // GROUPED_PARAMETER_HOLD

};

static struct msm_camera_i2c_reg_conf ar0543_snap_settings[] = {
    //Snapshot 2592*1944 14.8fps
    { 0x0104, 0x01, MSM_CAMERA_I2C_BYTE_DATA},  // GROUPED_PARAMETER_HOLD
    { 0x3004, 0x0008},  // X_ADDR_START_
    { 0x3008, 0x0A27},  // X_ADDR_END_
    { 0x3002, 0x0008},  // Y_ADDR_START_
    { 0x3006, 0x079F},  // Y_ADDR_END_
    { 0x3040, 0x0041},  // READ_MODE             // full resolution
    { 0x034C, 0x0A20},  // X_OUTPUT_SIZE
    { 0x034E, 0x0798},  // Y_OUTPUT_SIZE
    { 0x300C, 0x0E70},  // LINE_LENGTH_PCK_
    { 0x300A, 0x07E5},  // FRAME_LENGTH_LINES_
    { 0x3012, 0x07E4},  // COARSE_INTEGRATION_TIME_
    { 0x3014, 0x0C8C},  // FINE_INTEGRATION_TIME_
    { 0x3010, 0x00A0},  // FINE_CORRECTION
    { 0x0104, 0x00, MSM_CAMERA_I2C_BYTE_DATA},  // GROUPED_PARAMETER_HOLD
};

/* MM-MC-ImplementRegSwitchMechanismForShading-01+{ */
static struct msm_camera_i2c_reg_conf ar0543_recommend_settings1[] = {
    /* Golden module */
    //[648x488 30fps Mclk=24MHz_dual_lane_MIPI_interface]
    //[1]LOAD=stop_streaming
    { 0x0100, 0x00, MSM_CAMERA_I2C_BYTE_DATA},  // MODE_SELECT
    
    //[2]LOAD=dual_lane_MIPI_interface
    { 0x301A, 0x0018},  // RESET_REGISTER
    { 0x3064, 0xB800},  // RESERVED_MFR_3064
    { 0x31AE, 0x0202},  // SERIAL_FORMAT
    
    //[3]LOAD=REV1_recommended_settings
    { 0x316A, 0x8400},  // RESERVED_MFR_316A
    { 0x316C, 0x8400},  // RESERVED_MFR_316C
    { 0x316E, 0x8400},  // RESERVED_MFR_316E
    { 0x3EFA, 0x1A1F},  // RESERVED_MFR_3EFA
    { 0x3ED2, 0xD965},  // RESERVED_MFR_3ED2
    { 0x3ED8, 0x7F1B},  // RESERVED_MFR_3ED8
    { 0x3EDA, 0x2F11},  // RESERVED_MFR_3EDA
    { 0x3EE2, 0x0060},  // RESERVED_MFR_3EE2
    { 0x3EF2, 0xD965},  // RESERVED_MFR_3EF2
    { 0x3EF8, 0x797F},  // RESERVED_MFR_3EF8
    { 0x3EFC, 0x286F},  // RESERVED_MFR_3EFC
    { 0x3EFE, 0x2C01},  // RESERVED_MFR_3EFE
    { 0x3E00, 0x042F},  // RESERVED_MFR_3E00
    { 0x3E02, 0xFFFF},  // RESERVED_MFR_3E02
    { 0x3E04, 0xFFFF},  // RESERVED_MFR_3E04
    { 0x3E06, 0xFFFF},  // RESERVED_MFR_3E06
    { 0x3E08, 0x8071},  // RESERVED_MFR_3E08
    { 0x3E0A, 0x7281},  // RESERVED_MFR_3E0A
    { 0x3E0C, 0x4011},  // RESERVED_MFR_3E0C
    { 0x3E0E, 0x8010},  // RESERVED_MFR_3E0E
    { 0x3E10, 0x60A5},  // RESERVED_MFR_3E10
    { 0x3E12, 0x4080},  // RESERVED_MFR_3E12
    { 0x3E14, 0x4180},  // RESERVED_MFR_3E14
    { 0x3E16, 0x0018},  // RESERVED_MFR_3E16
    { 0x3E18, 0x46B7},  // RESERVED_MFR_3E18
    { 0x3E1A, 0x4994},  // RESERVED_MFR_3E1A
    { 0x3E1C, 0x4997},  // RESERVED_MFR_3E1C
    { 0x3E1E, 0x4682},  // RESERVED_MFR_3E1E
    { 0x3E20, 0x0018},  // RESERVED_MFR_3E20
    { 0x3E22, 0x4241},  // RESERVED_MFR_3E22
    { 0x3E24, 0x8000},  // RESERVED_MFR_3E24
    { 0x3E26, 0x1880},  // RESERVED_MFR_3E26
    { 0x3E28, 0x4785},  // RESERVED_MFR_3E28
    { 0x3E2A, 0x4992},  // RESERVED_MFR_3E2A
    { 0x3E2C, 0x4997},  // RESERVED_MFR_3E2C
    { 0x3E2E, 0x4780},  // RESERVED_MFR_3E2E
    { 0x3E30, 0x4D80},  // RESERVED_MFR_3E30
    { 0x3E32, 0x100C},  // RESERVED_MFR_3E32
    { 0x3E34, 0x8000},  // RESERVED_MFR_3E34
    { 0x3E36, 0x184A},  // RESERVED_MFR_3E36
    { 0x3E38, 0x8042},  // RESERVED_MFR_3E38
    { 0x3E3A, 0x001A},  // RESERVED_MFR_3E3A
    { 0x3E3C, 0x9610},  // RESERVED_MFR_3E3C
    { 0x3E3E, 0x0C80},  // RESERVED_MFR_3E3E
    { 0x3E40, 0x4DC6},  // RESERVED_MFR_3E40
    { 0x3E42, 0x4A80},  // RESERVED_MFR_3E42
    { 0x3E44, 0x0018},  // RESERVED_MFR_3E44
    { 0x3E46, 0x8042},  // RESERVED_MFR_3E46
    { 0x3E48, 0x8041},  // RESERVED_MFR_3E48
    { 0x3E4A, 0x0018},  // RESERVED_MFR_3E4A
    { 0x3E4C, 0x804B},  // RESERVED_MFR_3E4C
    { 0x3E4E, 0xB74B},  // RESERVED_MFR_3E4E
    { 0x3E50, 0x8010},  // RESERVED_MFR_3E50
    { 0x3E52, 0x6056},  // RESERVED_MFR_3E52
    { 0x3E54, 0x001C},  // RESERVED_MFR_3E54
    { 0x3E56, 0x8211},  // RESERVED_MFR_3E56
    { 0x3E58, 0x8056},  // RESERVED_MFR_3E58
    { 0x3E5A, 0x827C},  // RESERVED_MFR_3E5A
    { 0x3E5C, 0x0970},  // RESERVED_MFR_3E5C
    { 0x3E5E, 0x8082},  // RESERVED_MFR_3E5E
    { 0x3E60, 0x7281},  // RESERVED_MFR_3E60
    { 0x3E62, 0x4C40},  // RESERVED_MFR_3E62
    { 0x3E64, 0x8E4D},  // RESERVED_MFR_3E64
    { 0x3E66, 0x8110},  // RESERVED_MFR_3E66
    { 0x3E68, 0x0CAF},  // RESERVED_MFR_3E68
    { 0x3E6A, 0x4D80},  // RESERVED_MFR_3E6A
    { 0x3E6C, 0x100C},  // RESERVED_MFR_3E6C
    { 0x3E6E, 0x8440},  // RESERVED_MFR_3E6E
    { 0x3E70, 0x4C81},  // RESERVED_MFR_3E70
    { 0x3E72, 0x7C5F},  // RESERVED_MFR_3E72
    { 0x3E74, 0x7000},  // RESERVED_MFR_3E74
    { 0x3E76, 0x0000},  // RESERVED_MFR_3E76
    { 0x3E78, 0x0000},  // RESERVED_MFR_3E78
    { 0x3E7A, 0x0000},  // RESERVED_MFR_3E7A
    { 0x3E7C, 0x0000},  // RESERVED_MFR_3E7C
    { 0x3E7E, 0x0000},  // RESERVED_MFR_3E7E
    { 0x3E80, 0x0000},  // RESERVED_MFR_3E80
    { 0x3E82, 0x0000},  // RESERVED_MFR_3E82
    { 0x3E84, 0x0000},  // RESERVED_MFR_3E84
    { 0x3E86, 0x0000},  // RESERVED_MFR_3E86
    { 0x3E88, 0x0000},  // RESERVED_MFR_3E88
    { 0x3E8A, 0x0000},  // RESERVED_MFR_3E8A
    { 0x3E8C, 0x0000},  // RESERVED_MFR_3E8C
    { 0x3E8E, 0x0000},  // RESERVED_MFR_3E8E
    { 0x3E90, 0x0000},  // RESERVED_MFR_3E90
    { 0x3E92, 0x0000},  // RESERVED_MFR_3E92
    { 0x3E94, 0x0000},  // RESERVED_MFR_3E94
    { 0x3E96, 0x0000},  // RESERVED_MFR_3E96
    { 0x3E98, 0x0000},  // RESERVED_MFR_3E98
    { 0x3E9A, 0x0000},  // RESERVED_MFR_3E9A
    { 0x3E9C, 0x0000},  // RESERVED_MFR_3E9C
    { 0x3E9E, 0x0000},  // RESERVED_MFR_3E9E
    { 0x3EA0, 0x0000},  // RESERVED_MFR_3EA0
    { 0x3EA2, 0x0000},  // RESERVED_MFR_3EA2
    { 0x3EA4, 0x0000},  // RESERVED_MFR_3EA4
    { 0x3EA6, 0x0000},  // RESERVED_MFR_3EA6
    { 0x3EA8, 0x0000},  // RESERVED_MFR_3EA8
    { 0x3EAA, 0x0000},  // RESERVED_MFR_3EAA
    { 0x3EAC, 0x0000},  // RESERVED_MFR_3EAC
    { 0x3EAE, 0x0000},  // RESERVED_MFR_3EAE
    { 0x3EB0, 0x0000},  // RESERVED_MFR_3EB0
    { 0x3EB2, 0x0000},  // RESERVED_MFR_3EB2
    { 0x3EB4, 0x0000},  // RESERVED_MFR_3EB4
    { 0x3EB6, 0x0000},  // RESERVED_MFR_3EB6
    { 0x3EB8, 0x0000},  // RESERVED_MFR_3EB8
    { 0x3EBA, 0x0000},  // RESERVED_MFR_3EBA
    { 0x3EBC, 0x0000},  // RESERVED_MFR_3EBC
    { 0x3EBE, 0x0000},  // RESERVED_MFR_3EBE
    { 0x3EC0, 0x0000},  // RESERVED_MFR_3EC0
    { 0x3EC2, 0x0000},  // RESERVED_MFR_3EC2
    { 0x3EC4, 0x0000},  // RESERVED_MFR_3EC4
    { 0x3EC6, 0x0000},  // RESERVED_MFR_3EC6
    { 0x3EC8, 0x0000},  // RESERVED_MFR_3EC8
    { 0x3ECA, 0x0000},  // RESERVED_MFR_3ECA
    { 0x3170, 0x2150},  // RESERVED_MFR_3170
    { 0x317A, 0x0150},  // RESERVED_MFR_317A
    { 0x3ECC, 0x2200},  // RESERVED_MFR_3ECC
    { 0x3174, 0x0000},  // RESERVED_MFR_3174
    { 0x3176, 0x0000},  // RESERVED_MFR_3176
    { 0x30BC, 0x0384},  // CALIB_GLOBAL
    { 0x30C0, 0x1220},  // CALIB_CONTROL
    { 0x30D4, 0x9200},  // RESERVED_MFR_30D4
    { 0x30B2, 0xC000},  // RESERVED_MFR_30B2
    { 0x31B0, 0x00C4},  // FRAME_PREAMBLE
    { 0x31B2, 0x0064},  // LINE_PREAMBLE
    { 0x31B4, 0x0E77},  // MIPI_TIMING_0
    { 0x31B6, 0x0D24},  // MIPI_TIMING_1
    { 0x31B8, 0x020E},  // MIPI_TIMING_2
    { 0x31BA, 0x0710},  // MIPI_TIMING_3
    { 0x31BC, 0x2A0D},  // MIPI_TIMING_4
    { 0x31BE, 0xC007},  // RESERVED_MFR_31BE

    //MM-MC-UpdateTuningForLensCorrectionIn20130408-00*{
    { 0x305E, 0x1129},  // GLOBAL_GAIN
    
    //[Lens Correction 85% 01/29/13 15:16:57]
    { 0x3780, 0x0000},  //
    { 0x3600, 0x00D0},  // P_GR_P0Q0
    { 0x3602, 0xA96B},  // P_GR_P0Q1
    { 0x3604, 0x0491},  // P_GR_P0Q2
    { 0x3606, 0x034C},  // P_GR_P0Q3
    { 0x3608, 0xFC70},  // P_GR_P0Q4
    { 0x360A, 0x0190},  // P_RD_P0Q0
    { 0x360C, 0xA26C},  // P_RD_P0Q1
    { 0x360E, 0x0311},  // P_RD_P0Q2
    { 0x3610, 0x3A2E},  // P_RD_P0Q3
    { 0x3612, 0x91D1},  // P_RD_P0Q4
    { 0x3614, 0x0190},  // P_BL_P0Q0
    { 0x3616, 0xE30A},  // P_BL_P0Q1
    { 0x3618, 0x3E70},  // P_BL_P0Q2
    { 0x361A, 0xA80D},  // P_BL_P0Q3
    { 0x361C, 0xC630},  // P_BL_P0Q4
    { 0x361E, 0x01D0},  // P_GB_P0Q0
    { 0x3620, 0xD68C},  // P_GB_P0Q1
    { 0x3622, 0x04F1},  // P_GB_P0Q2
    { 0x3624, 0x784D},  // P_GB_P0Q3
    { 0x3626, 0x8CF1},  // P_GB_P0Q4
    { 0x3640, 0x98CB},  // P_GR_P1Q0
    { 0x3642, 0xC04D},  // P_GR_P1Q1
    { 0x3644, 0xA70D},  // P_GR_P1Q2
    { 0x3646, 0x754A},  // P_GR_P1Q3
    { 0x3648, 0x1E8E},  // P_GR_P1Q4
    { 0x364A, 0xD26A},  // P_RD_P1Q0
    { 0x364C, 0x3AEE},  // P_RD_P1Q1
    { 0x364E, 0x8F8B},  // P_RD_P1Q2
    { 0x3650, 0x968F},  // P_RD_P1Q3
    { 0x3652, 0x4C8A},  // P_RD_P1Q4
    { 0x3654, 0x428A},  // P_BL_P1Q0
    { 0x3656, 0x08CE},  // P_BL_P1Q1
    { 0x3658, 0x092E},  // P_BL_P1Q2
    { 0x365A, 0xD86D},  // P_BL_P1Q3
    { 0x365C, 0xDA8E},  // P_BL_P1Q4
    { 0x365E, 0x8E2A},  // P_GB_P1Q0
    { 0x3660, 0x832E},  // P_GB_P1Q1
    { 0x3662, 0x274E},  // P_GB_P1Q2
    { 0x3664, 0x2B6E},  // P_GB_P1Q3
    { 0x3666, 0x8CCF},  // P_GB_P1Q4
    { 0x3680, 0x0591},  // P_GR_P2Q0
    { 0x3682, 0x5FEE},  // P_GR_P2Q1
    { 0x3684, 0xAC10},  // P_GR_P2Q2
    { 0x3686, 0xF74C},  // P_GR_P2Q3
    { 0x3688, 0xF2B2},  // P_GR_P2Q4
    { 0x368A, 0x2D51},  // P_RD_P2Q0
    { 0x368C, 0x950C},  // P_RD_P2Q1
    { 0x368E, 0xFF70},  // P_RD_P2Q2
    { 0x3690, 0xEDEF},  // P_RD_P2Q3
    { 0x3692, 0xEB12},  // P_RD_P2Q4
    { 0x3694, 0x7330},  // P_BL_P2Q0
    { 0x3696, 0x5D0D},  // P_BL_P2Q1
    { 0x3698, 0xC0B1},  // P_BL_P2Q2
    { 0x369A, 0x6C2F},  // P_BL_P2Q3
    { 0x369C, 0xF78F},  // P_BL_P2Q4
    { 0x369E, 0x0031},  // P_GB_P2Q0
    { 0x36A0, 0x27CD},  // P_GB_P2Q1
    { 0x36A2, 0xC510},  // P_GB_P2Q2
    { 0x36A4, 0xA92F},  // P_GB_P2Q3
    { 0x36A6, 0xD2D2},  // P_GB_P2Q4
    { 0x36C0, 0xA60D},  // P_GR_P3Q0
    { 0x36C2, 0xE9AC},  // P_GR_P3Q1
    { 0x36C4, 0xD80C},  // P_GR_P3Q2
    { 0x36C6, 0x686F},  // P_GR_P3Q3
    { 0x36C8, 0x950D},  // P_GR_P3Q4
    { 0x36CA, 0x022E},  // P_RD_P3Q0
    { 0x36CC, 0x828E},  // P_RD_P3Q1
    { 0x36CE, 0xE1CF},  // P_RD_P3Q2
    { 0x36D0, 0x2E4E},  // P_RD_P3Q3
    { 0x36D2, 0x808C},  // P_RD_P3Q4
    { 0x36D4, 0xDA2D},  // P_BL_P3Q0
    { 0x36D6, 0x8BCE},  // P_BL_P3Q1
    { 0x36D8, 0x6348},  // P_BL_P3Q2
    { 0x36DA, 0x98CD},  // P_BL_P3Q3
    { 0x36DC, 0xDE90},  // P_BL_P3Q4
    { 0x36DE, 0xEB8D},  // P_GB_P3Q0
    { 0x36E0, 0x03CE},  // P_GB_P3Q1
    { 0x36E2, 0xABEB},  // P_GB_P3Q2
    { 0x36E4, 0xEA0D},  // P_GB_P3Q3
    { 0x36E6, 0x9630},  // P_GB_P3Q4
    { 0x3700, 0xDFB0},  // P_GR_P4Q0
    { 0x3702, 0x218C},  // P_GR_P4Q1
    { 0x3704, 0x9694},  // P_GR_P4Q2
    { 0x3706, 0xAFD2},  // P_GR_P4Q3
    { 0x3708, 0x0196},  // P_GR_P4Q4
    { 0x370A, 0x9111},  // P_RD_P4Q0
    { 0x370C, 0x38AF},  // P_RD_P4Q1
    { 0x370E, 0xB674},  // P_RD_P4Q2
    { 0x3710, 0xD690},  // P_RD_P4Q3
    { 0x3712, 0x1AF6},  // P_RD_P4Q4
    { 0x3714, 0xFAF0},  // P_BL_P4Q0
    { 0x3716, 0x3F2E},  // P_BL_P4Q1
    { 0x3718, 0xA1B3},  // P_BL_P4Q2
    { 0x371A, 0xC592},  // P_BL_P4Q3
    { 0x371C, 0x32F5},  // P_BL_P4Q4
    { 0x371E, 0xBAB0},  // P_GB_P4Q0
    { 0x3720, 0x164E},  // P_GB_P4Q1
    { 0x3722, 0xA0D4},  // P_GB_P4Q2
    { 0x3724, 0x9FD1},  // P_GB_P4Q3
    { 0x3726, 0x07F6},  // P_GB_P4Q4
    { 0x3782, 0x0510},  // POLY_ORIGIN_C
    { 0x3784, 0x03CC},  // POLY_ORIGIN_R
    { 0x37C0, 0x0000},  // P_GR_Q5
    { 0x37C2, 0x0000},  // P_RD_Q5
    { 0x37C4, 0x0000},  // P_BL_Q5
    { 0x37C6, 0x0000},  // P_GB_Q5

    //STATE= Lens Correction Falloff, 85
    { 0x3780, 0x8000},  //POLY_SC_ENABLE
    //MM-MC-UpdateTuningForLensCorrectionIn20130408-00*}

    { 0x0400, 0x00, MSM_CAMERA_I2C_BYTE_DATA},   //scaling_mode
    { 0x0404, 0x10, MSM_CAMERA_I2C_BYTE_DATA},   //scaling_m
    
    //[4 LOAD=PLL_]
    { 0x0300, 0x0005},  // VT_PIX_CLK_DIV
    { 0x0302, 0x0001},  // VT_SYS_CLK_DIV
    { 0x0304, 0x0002},  // PRE_PLL_CLK_DIV
    { 0x0306, 0x002E},  // PLL_MULTIPLIER
    { 0x0308, 0x000A},  // OP_PIX_CLK_DIV
    { 0x030A, 0x0001},  // OP_SYS_CLK_DIV
    //DELAY=1 
};

static struct msm_camera_i2c_reg_conf ar0543_recommend_settings_2[] = {
    /* Corner module */
    //[648x488 30fps Mclk=24MHz_dual_lane_MIPI_interface]
    //[1]LOAD=stop_streaming
    { 0x0100, 0x00, MSM_CAMERA_I2C_BYTE_DATA},  // MODE_SELECT
    
    //[2]LOAD=dual_lane_MIPI_interface
    { 0x301A, 0x0018},  // RESET_REGISTER
    { 0x3064, 0xB800},  // RESERVED_MFR_3064
    { 0x31AE, 0x0202},  // SERIAL_FORMAT
    
    //[3]LOAD=REV1_recommended_settings
    { 0x316A, 0x8400},  // RESERVED_MFR_316A
    { 0x316C, 0x8400},  // RESERVED_MFR_316C
    { 0x316E, 0x8400},  // RESERVED_MFR_316E
    { 0x3EFA, 0x1A1F},  // RESERVED_MFR_3EFA
    { 0x3ED2, 0xD965},  // RESERVED_MFR_3ED2
    { 0x3ED8, 0x7F1B},  // RESERVED_MFR_3ED8
    { 0x3EDA, 0x2F11},  // RESERVED_MFR_3EDA
    { 0x3EE2, 0x0060},  // RESERVED_MFR_3EE2
    { 0x3EF2, 0xD965},  // RESERVED_MFR_3EF2
    { 0x3EF8, 0x797F},  // RESERVED_MFR_3EF8
    { 0x3EFC, 0x286F},  // RESERVED_MFR_3EFC
    { 0x3EFE, 0x2C01},  // RESERVED_MFR_3EFE
    { 0x3E00, 0x042F},  // RESERVED_MFR_3E00
    { 0x3E02, 0xFFFF},  // RESERVED_MFR_3E02
    { 0x3E04, 0xFFFF},  // RESERVED_MFR_3E04
    { 0x3E06, 0xFFFF},  // RESERVED_MFR_3E06
    { 0x3E08, 0x8071},  // RESERVED_MFR_3E08
    { 0x3E0A, 0x7281},  // RESERVED_MFR_3E0A
    { 0x3E0C, 0x4011},  // RESERVED_MFR_3E0C
    { 0x3E0E, 0x8010},  // RESERVED_MFR_3E0E
    { 0x3E10, 0x60A5},  // RESERVED_MFR_3E10
    { 0x3E12, 0x4080},  // RESERVED_MFR_3E12
    { 0x3E14, 0x4180},  // RESERVED_MFR_3E14
    { 0x3E16, 0x0018},  // RESERVED_MFR_3E16
    { 0x3E18, 0x46B7},  // RESERVED_MFR_3E18
    { 0x3E1A, 0x4994},  // RESERVED_MFR_3E1A
    { 0x3E1C, 0x4997},  // RESERVED_MFR_3E1C
    { 0x3E1E, 0x4682},  // RESERVED_MFR_3E1E
    { 0x3E20, 0x0018},  // RESERVED_MFR_3E20
    { 0x3E22, 0x4241},  // RESERVED_MFR_3E22
    { 0x3E24, 0x8000},  // RESERVED_MFR_3E24
    { 0x3E26, 0x1880},  // RESERVED_MFR_3E26
    { 0x3E28, 0x4785},  // RESERVED_MFR_3E28
    { 0x3E2A, 0x4992},  // RESERVED_MFR_3E2A
    { 0x3E2C, 0x4997},  // RESERVED_MFR_3E2C
    { 0x3E2E, 0x4780},  // RESERVED_MFR_3E2E
    { 0x3E30, 0x4D80},  // RESERVED_MFR_3E30
    { 0x3E32, 0x100C},  // RESERVED_MFR_3E32
    { 0x3E34, 0x8000},  // RESERVED_MFR_3E34
    { 0x3E36, 0x184A},  // RESERVED_MFR_3E36
    { 0x3E38, 0x8042},  // RESERVED_MFR_3E38
    { 0x3E3A, 0x001A},  // RESERVED_MFR_3E3A
    { 0x3E3C, 0x9610},  // RESERVED_MFR_3E3C
    { 0x3E3E, 0x0C80},  // RESERVED_MFR_3E3E
    { 0x3E40, 0x4DC6},  // RESERVED_MFR_3E40
    { 0x3E42, 0x4A80},  // RESERVED_MFR_3E42
    { 0x3E44, 0x0018},  // RESERVED_MFR_3E44
    { 0x3E46, 0x8042},  // RESERVED_MFR_3E46
    { 0x3E48, 0x8041},  // RESERVED_MFR_3E48
    { 0x3E4A, 0x0018},  // RESERVED_MFR_3E4A
    { 0x3E4C, 0x804B},  // RESERVED_MFR_3E4C
    { 0x3E4E, 0xB74B},  // RESERVED_MFR_3E4E
    { 0x3E50, 0x8010},  // RESERVED_MFR_3E50
    { 0x3E52, 0x6056},  // RESERVED_MFR_3E52
    { 0x3E54, 0x001C},  // RESERVED_MFR_3E54
    { 0x3E56, 0x8211},  // RESERVED_MFR_3E56
    { 0x3E58, 0x8056},  // RESERVED_MFR_3E58
    { 0x3E5A, 0x827C},  // RESERVED_MFR_3E5A
    { 0x3E5C, 0x0970},  // RESERVED_MFR_3E5C
    { 0x3E5E, 0x8082},  // RESERVED_MFR_3E5E
    { 0x3E60, 0x7281},  // RESERVED_MFR_3E60
    { 0x3E62, 0x4C40},  // RESERVED_MFR_3E62
    { 0x3E64, 0x8E4D},  // RESERVED_MFR_3E64
    { 0x3E66, 0x8110},  // RESERVED_MFR_3E66
    { 0x3E68, 0x0CAF},  // RESERVED_MFR_3E68
    { 0x3E6A, 0x4D80},  // RESERVED_MFR_3E6A
    { 0x3E6C, 0x100C},  // RESERVED_MFR_3E6C
    { 0x3E6E, 0x8440},  // RESERVED_MFR_3E6E
    { 0x3E70, 0x4C81},  // RESERVED_MFR_3E70
    { 0x3E72, 0x7C5F},  // RESERVED_MFR_3E72
    { 0x3E74, 0x7000},  // RESERVED_MFR_3E74
    { 0x3E76, 0x0000},  // RESERVED_MFR_3E76
    { 0x3E78, 0x0000},  // RESERVED_MFR_3E78
    { 0x3E7A, 0x0000},  // RESERVED_MFR_3E7A
    { 0x3E7C, 0x0000},  // RESERVED_MFR_3E7C
    { 0x3E7E, 0x0000},  // RESERVED_MFR_3E7E
    { 0x3E80, 0x0000},  // RESERVED_MFR_3E80
    { 0x3E82, 0x0000},  // RESERVED_MFR_3E82
    { 0x3E84, 0x0000},  // RESERVED_MFR_3E84
    { 0x3E86, 0x0000},  // RESERVED_MFR_3E86
    { 0x3E88, 0x0000},  // RESERVED_MFR_3E88
    { 0x3E8A, 0x0000},  // RESERVED_MFR_3E8A
    { 0x3E8C, 0x0000},  // RESERVED_MFR_3E8C
    { 0x3E8E, 0x0000},  // RESERVED_MFR_3E8E
    { 0x3E90, 0x0000},  // RESERVED_MFR_3E90
    { 0x3E92, 0x0000},  // RESERVED_MFR_3E92
    { 0x3E94, 0x0000},  // RESERVED_MFR_3E94
    { 0x3E96, 0x0000},  // RESERVED_MFR_3E96
    { 0x3E98, 0x0000},  // RESERVED_MFR_3E98
    { 0x3E9A, 0x0000},  // RESERVED_MFR_3E9A
    { 0x3E9C, 0x0000},  // RESERVED_MFR_3E9C
    { 0x3E9E, 0x0000},  // RESERVED_MFR_3E9E
    { 0x3EA0, 0x0000},  // RESERVED_MFR_3EA0
    { 0x3EA2, 0x0000},  // RESERVED_MFR_3EA2
    { 0x3EA4, 0x0000},  // RESERVED_MFR_3EA4
    { 0x3EA6, 0x0000},  // RESERVED_MFR_3EA6
    { 0x3EA8, 0x0000},  // RESERVED_MFR_3EA8
    { 0x3EAA, 0x0000},  // RESERVED_MFR_3EAA
    { 0x3EAC, 0x0000},  // RESERVED_MFR_3EAC
    { 0x3EAE, 0x0000},  // RESERVED_MFR_3EAE
    { 0x3EB0, 0x0000},  // RESERVED_MFR_3EB0
    { 0x3EB2, 0x0000},  // RESERVED_MFR_3EB2
    { 0x3EB4, 0x0000},  // RESERVED_MFR_3EB4
    { 0x3EB6, 0x0000},  // RESERVED_MFR_3EB6
    { 0x3EB8, 0x0000},  // RESERVED_MFR_3EB8
    { 0x3EBA, 0x0000},  // RESERVED_MFR_3EBA
    { 0x3EBC, 0x0000},  // RESERVED_MFR_3EBC
    { 0x3EBE, 0x0000},  // RESERVED_MFR_3EBE
    { 0x3EC0, 0x0000},  // RESERVED_MFR_3EC0
    { 0x3EC2, 0x0000},  // RESERVED_MFR_3EC2
    { 0x3EC4, 0x0000},  // RESERVED_MFR_3EC4
    { 0x3EC6, 0x0000},  // RESERVED_MFR_3EC6
    { 0x3EC8, 0x0000},  // RESERVED_MFR_3EC8
    { 0x3ECA, 0x0000},  // RESERVED_MFR_3ECA
    { 0x3170, 0x2150},  // RESERVED_MFR_3170
    { 0x317A, 0x0150},  // RESERVED_MFR_317A
    { 0x3ECC, 0x2200},  // RESERVED_MFR_3ECC
    { 0x3174, 0x0000},  // RESERVED_MFR_3174
    { 0x3176, 0x0000},  // RESERVED_MFR_3176
    { 0x30BC, 0x0384},  // CALIB_GLOBAL
    { 0x30C0, 0x1220},  // CALIB_CONTROL
    { 0x30D4, 0x9200},  // RESERVED_MFR_30D4
    { 0x30B2, 0xC000},  // RESERVED_MFR_30B2
    { 0x31B0, 0x00C4},  // FRAME_PREAMBLE
    { 0x31B2, 0x0064},  // LINE_PREAMBLE
    { 0x31B4, 0x0E77},  // MIPI_TIMING_0
    { 0x31B6, 0x0D24},  // MIPI_TIMING_1
    { 0x31B8, 0x020E},  // MIPI_TIMING_2
    { 0x31BA, 0x0710},  // MIPI_TIMING_3
    { 0x31BC, 0x2A0D},  // MIPI_TIMING_4
    { 0x31BE, 0xC007},  // RESERVED_MFR_31BE

    //MM-MC-UpdateTuningForLensCorrectionIn20130408-00*{
    { 0x305E, 0x1129},  // GLOBAL_GAIN
    
    //[Lens Correction 85% 01/29/13 15:16:57]
    { 0x3780, 0x0000},  //
    { 0x3600, 0x00D0},  // P_GR_P0Q0
    { 0x3602, 0xA96B},  // P_GR_P0Q1
    { 0x3604, 0x0491},  // P_GR_P0Q2
    { 0x3606, 0x034C},  // P_GR_P0Q3
    { 0x3608, 0xFC70},  // P_GR_P0Q4
    { 0x360A, 0x0190},  // P_RD_P0Q0
    { 0x360C, 0xA26C},  // P_RD_P0Q1
    { 0x360E, 0x0311},  // P_RD_P0Q2
    { 0x3610, 0x3A2E},  // P_RD_P0Q3
    { 0x3612, 0x91D1},  // P_RD_P0Q4
    { 0x3614, 0x0190},  // P_BL_P0Q0
    { 0x3616, 0xE30A},  // P_BL_P0Q1
    { 0x3618, 0x3E70},  // P_BL_P0Q2
    { 0x361A, 0xA80D},  // P_BL_P0Q3
    { 0x361C, 0xC630},  // P_BL_P0Q4
    { 0x361E, 0x01D0},  // P_GB_P0Q0
    { 0x3620, 0xD68C},  // P_GB_P0Q1
    { 0x3622, 0x04F1},  // P_GB_P0Q2
    { 0x3624, 0x784D},  // P_GB_P0Q3
    { 0x3626, 0x8CF1},  // P_GB_P0Q4
    { 0x3640, 0x98CB},  // P_GR_P1Q0
    { 0x3642, 0xC04D},  // P_GR_P1Q1
    { 0x3644, 0xA70D},  // P_GR_P1Q2
    { 0x3646, 0x754A},  // P_GR_P1Q3
    { 0x3648, 0x1E8E},  // P_GR_P1Q4
    { 0x364A, 0xD26A},  // P_RD_P1Q0
    { 0x364C, 0x3AEE},  // P_RD_P1Q1
    { 0x364E, 0x8F8B},  // P_RD_P1Q2
    { 0x3650, 0x968F},  // P_RD_P1Q3
    { 0x3652, 0x4C8A},  // P_RD_P1Q4
    { 0x3654, 0x428A},  // P_BL_P1Q0
    { 0x3656, 0x08CE},  // P_BL_P1Q1
    { 0x3658, 0x092E},  // P_BL_P1Q2
    { 0x365A, 0xD86D},  // P_BL_P1Q3
    { 0x365C, 0xDA8E},  // P_BL_P1Q4
    { 0x365E, 0x8E2A},  // P_GB_P1Q0
    { 0x3660, 0x832E},  // P_GB_P1Q1
    { 0x3662, 0x274E},  // P_GB_P1Q2
    { 0x3664, 0x2B6E},  // P_GB_P1Q3
    { 0x3666, 0x8CCF},  // P_GB_P1Q4
    { 0x3680, 0x0591},  // P_GR_P2Q0
    { 0x3682, 0x5FEE},  // P_GR_P2Q1
    { 0x3684, 0xAC10},  // P_GR_P2Q2
    { 0x3686, 0xF74C},  // P_GR_P2Q3
    { 0x3688, 0xF2B2},  // P_GR_P2Q4
    { 0x368A, 0x2D51},  // P_RD_P2Q0
    { 0x368C, 0x950C},  // P_RD_P2Q1
    { 0x368E, 0xFF70},  // P_RD_P2Q2
    { 0x3690, 0xEDEF},  // P_RD_P2Q3
    { 0x3692, 0xEB12},  // P_RD_P2Q4
    { 0x3694, 0x7330},  // P_BL_P2Q0
    { 0x3696, 0x5D0D},  // P_BL_P2Q1
    { 0x3698, 0xC0B1},  // P_BL_P2Q2
    { 0x369A, 0x6C2F},  // P_BL_P2Q3
    { 0x369C, 0xF78F},  // P_BL_P2Q4
    { 0x369E, 0x0031},  // P_GB_P2Q0
    { 0x36A0, 0x27CD},  // P_GB_P2Q1
    { 0x36A2, 0xC510},  // P_GB_P2Q2
    { 0x36A4, 0xA92F},  // P_GB_P2Q3
    { 0x36A6, 0xD2D2},  // P_GB_P2Q4
    { 0x36C0, 0xA60D},  // P_GR_P3Q0
    { 0x36C2, 0xE9AC},  // P_GR_P3Q1
    { 0x36C4, 0xD80C},  // P_GR_P3Q2
    { 0x36C6, 0x686F},  // P_GR_P3Q3
    { 0x36C8, 0x950D},  // P_GR_P3Q4
    { 0x36CA, 0x022E},  // P_RD_P3Q0
    { 0x36CC, 0x828E},  // P_RD_P3Q1
    { 0x36CE, 0xE1CF},  // P_RD_P3Q2
    { 0x36D0, 0x2E4E},  // P_RD_P3Q3
    { 0x36D2, 0x808C},  // P_RD_P3Q4
    { 0x36D4, 0xDA2D},  // P_BL_P3Q0
    { 0x36D6, 0x8BCE},  // P_BL_P3Q1
    { 0x36D8, 0x6348},  // P_BL_P3Q2
    { 0x36DA, 0x98CD},  // P_BL_P3Q3
    { 0x36DC, 0xDE90},  // P_BL_P3Q4
    { 0x36DE, 0xEB8D},  // P_GB_P3Q0
    { 0x36E0, 0x03CE},  // P_GB_P3Q1
    { 0x36E2, 0xABEB},  // P_GB_P3Q2
    { 0x36E4, 0xEA0D},  // P_GB_P3Q3
    { 0x36E6, 0x9630},  // P_GB_P3Q4
    { 0x3700, 0xDFB0},  // P_GR_P4Q0
    { 0x3702, 0x218C},  // P_GR_P4Q1
    { 0x3704, 0x9694},  // P_GR_P4Q2
    { 0x3706, 0xAFD2},  // P_GR_P4Q3
    { 0x3708, 0x0196},  // P_GR_P4Q4
    { 0x370A, 0x9111},  // P_RD_P4Q0
    { 0x370C, 0x38AF},  // P_RD_P4Q1
    { 0x370E, 0xB674},  // P_RD_P4Q2
    { 0x3710, 0xD690},  // P_RD_P4Q3
    { 0x3712, 0x1AF6},  // P_RD_P4Q4
    { 0x3714, 0xFAF0},  // P_BL_P4Q0
    { 0x3716, 0x3F2E},  // P_BL_P4Q1
    { 0x3718, 0xA1B3},  // P_BL_P4Q2
    { 0x371A, 0xC592},  // P_BL_P4Q3
    { 0x371C, 0x32F5},  // P_BL_P4Q4
    { 0x371E, 0xBAB0},  // P_GB_P4Q0
    { 0x3720, 0x164E},  // P_GB_P4Q1
    { 0x3722, 0xA0D4},  // P_GB_P4Q2
    { 0x3724, 0x9FD1},  // P_GB_P4Q3
    { 0x3726, 0x07F6},  // P_GB_P4Q4
    { 0x3782, 0x0510},  // POLY_ORIGIN_C
    { 0x3784, 0x03CC},  // POLY_ORIGIN_R
    { 0x37C0, 0x0000},  // P_GR_Q5
    { 0x37C2, 0x0000},  // P_RD_Q5
    { 0x37C4, 0x0000},  // P_BL_Q5
    { 0x37C6, 0x0000},  // P_GB_Q5

    //STATE= Lens Correction Falloff, 85
    { 0x3780, 0x8000},  //POLY_SC_ENABLE
    //MM-MC-UpdateTuningForLensCorrectionIn20130408-00*}

    { 0x0400, 0x00, MSM_CAMERA_I2C_BYTE_DATA},   //scaling_mode
    { 0x0404, 0x10, MSM_CAMERA_I2C_BYTE_DATA},   //scaling_m
    
    //[4 LOAD=PLL_]
    { 0x0300, 0x0005},  // VT_PIX_CLK_DIV
    { 0x0302, 0x0001},  // VT_SYS_CLK_DIV
    { 0x0304, 0x0002},  // PRE_PLL_CLK_DIV
    { 0x0306, 0x002E},  // PLL_MULTIPLIER
    { 0x0308, 0x000A},  // OP_PIX_CLK_DIV
    { 0x030A, 0x0001},  // OP_SYS_CLK_DIV
    //DELAY=1 
};


static struct msm_camera_i2c_reg_conf ar0543_recommend_settings_common[] = {
    //[5]LOAD=full_resolution
    { 0x0104, 0x01, MSM_CAMERA_I2C_BYTE_DATA},  // GROUPED_PARAMETER_HOLD
    { 0x3004, 0x0008},  // X_ADDR_START_
    { 0x3008, 0x0A25},  // X_ADDR_END_
    { 0x3002, 0x0008},  // Y_ADDR_START_
    { 0x3006, 0x079D},  // Y_ADDR_END_
    { 0x3040, 0x04C3},  // Y_ADDR_END_    
    { 0x034C, 0x0510},  // X_OUTPUT_SIZE   //0x0288
    { 0x034E, 0x03CC},  // Y_OUTPUT_SIZE   //0x01e8
    { 0x0342, 0x0C4C},  //line_length_pck             
    { 0x0340, 0x0496},  //frame_length_lines              
    { 0x0202, 0x0495},  //coarse_integration_time             // 33.33ms 
    { 0x3014, 0x0908},  //fine_integration_time               
    { 0x3010, 0x0184},  //fine_correction             
    { 0x0104, 0x00, MSM_CAMERA_I2C_BYTE_DATA},  //group parameter Hold

    //[6]LOAD=start_streaming
    { 0x0100, 0x01, MSM_CAMERA_I2C_BYTE_DATA},  // MODE_SELECT
};

static struct v4l2_subdev_info ar0543_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array ar0543_init_conf[] = {
	{&ar0543_recommend_settings_2[0],
	ARRAY_SIZE(ar0543_recommend_settings_2), 1, MSM_CAMERA_I2C_WORD_DATA},//Delay 1ms
       {&ar0543_recommend_settings_common[0],
	ARRAY_SIZE(ar0543_recommend_settings_common), 0, MSM_CAMERA_I2C_WORD_DATA},
};
/* MM-MC-ImplementRegSwitchMechanismForShading-01+} */

static struct msm_camera_i2c_conf_array ar0543_confs[] = {
	{&ar0543_snap_settings[0],
	ARRAY_SIZE(ar0543_snap_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&ar0543_prev_settings[0],
	ARRAY_SIZE(ar0543_prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&ar0543_video_settings[0],
	ARRAY_SIZE(ar0543_video_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

/* VFE maximum clock is 266 MHz.
    For sensor bringup you can keep .op_pixel_clk = maximum VFE clock.
    and .vt_pixel_clk = line_length_pclk * frame_length_lines * FPS.         */
static struct msm_sensor_output_info_t ar0543_dimensions[] = {
	{
        /* 15 fps */
        .x_output = 0x0A20,           /* 2592 */
        .y_output = 0x0798,           /* 1944 */
        .line_length_pclk = 0x0E70,   /* 3696 */
        .frame_length_lines = 0x07E5, /* 2021 */
        .vt_pixel_clk = 110600000,    /* =14.8*3696*2021 */
        .op_pixel_clk = 110600000,
        .binning_factor = 0,
	},
	{
        /* 30 fps preview */
        .x_output = 0x0510,           /* 1296 */
        .y_output = 0x03CC,           /* 972 */
        .line_length_pclk   = 0x0CF0, /* 3312 */
        .frame_length_lines = 0x0457, /* 1111 */
        .vt_pixel_clk = 110600000,    /* =30.003*3312*1111 */
        .op_pixel_clk = 110600000,
        .binning_factor = 0,
	},
    {
        /* 30 fps video */  
        .x_output = 0x508,            // akeemchen modified /* 1288 */
        .y_output = 0x2d8,            // akeemchen modified /* 728 */
        .line_length_pclk = 0x11f2,   // akeemchen modified /* 4594 */
        .frame_length_lines = 0x0321, // akeemchen modified /* 801 */
        .vt_pixel_clk = 110400000,    // akeemchen modified, 30.000169*4594*801 ~= 110400000
        .op_pixel_clk = 110400000,    // akeemchen modified
        .binning_factor = 0,
    },
};

static struct msm_sensor_output_reg_addr_t ar0543_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x1A16,//0x342,
	.frame_length_lines = 0x022F,//0x340,
};

static struct msm_sensor_id_info_t ar0543_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x4800,
};

static struct msm_sensor_exp_gain_info_t ar0543_exp_gain_info = {
    .coarse_int_time_addr      = 0x3012,//MM-MC-ImplementExpsureFunctionForAR0543-00*
    .global_gain_addr          = 0x305E,//MM-MC-ImplementExpsureFunctionForAR0543-00*
	.vert_offset = 0,
};

static struct msm_cam_clk_info cam_8960_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

static struct msm_cam_clk_info cam_8974_clk_info[] = {
	{"cam_src_clk", 19200000},
	{"cam_clk", -1},
};

int32_t ar0543_power_up(struct msm_sensor_ctrl_t *s_ctrl)
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

int32_t ar0543_power_down(struct msm_sensor_ctrl_t *s_ctrl)
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

int32_t ar0543_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *s_info = NULL;
    struct msm_sensor_ctrl_t *s_ctrl      = NULL;

    //MM-MC-ImplementReadOtpDataFeature-00*{
    if (g_IsFihMainCamProbe == 1)
    {
        printk("ar0543_sensor_i2c_probe: Main camera has probe success before !\n");
        return -EFAULT;
    }

    /* MM-MC-FixMainCameraI2cBusy-00-{ */
    s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);
    s_ctrl->sensor_i2c_addr = 0x6C;
	rc = msm_sensor_i2c_probe(client, id);//Probe for 0x6C
    if(rc < 0){
        pr_err("%s %s_i2c_probe failed\n", __func__, client->name);
        return rc;
    }
    /* MM-MC-FixMainCameraI2cBusy-00-} */
    g_IsFihMainCamProbe = 1;
    printk("ar0543_sensor_i2c_probe: sensor_i2c_addr(0x6e/0x6c) = 0x%x, g_IsFihMainCamProbe = %d.\n", s_ctrl->sensor_i2c_addr, g_IsFihMainCamProbe);
    //MM-MC-ImplementReadOtpDataFeature-00*}
    
	s_info = client->dev.platform_data;
	if (s_info == NULL) {
		pr_err("%s %s NULL sensor data\n", __func__, client->name);
		return -EFAULT;
	}

	return rc;
}

/* MM-SL-Improve camera open performance-00*{ */
/* MM-UW-Improve camera open performance-00+{ */
/* MM-MC-ImplementOtpReadFunctionForAR0543-00+{ */
int32_t ar0543_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc            = 0;
    int32_t productid     = 0;
    int32_t shading_index = 0;/* MM-MC-ImplementRegSwitchMechanismForShading-01+ */
    uint16_t chipid = 0;
	
	if(!g_IsOtpInitDone_ar0543){
    	printk("ar0543_match_id\n"); 
		rc = msm_camera_i2c_read(
			s_ctrl->sensor_i2c_client,
				s_ctrl->sensor_id_info->sensor_id_reg_addr, &chipid,
					MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0) {
			pr_err("%s: %s: read id failed\n", __func__,
				s_ctrl->sensordata->sensor_name);
			return rc;
		}

		CDBG("%s: read id: %x expected id %x:\n", __func__, chipid,
			s_ctrl->sensor_id_info->sensor_id);
		if (chipid != s_ctrl->sensor_id_info->sensor_id) {
			pr_err("ar0543_match_id: chip id does not match\n");
			return -ENODEV;
		}

    	if (s_ctrl->sensor_i2c_addr == 0x6C)
    	{
            rc = fih_init_otp_ar0543(s_ctrl);
            if (rc < 0)
                return rc;

            rc = fih_get_otp_data_ar0543(OTP_MI_PID , &productid);
            if (rc < 0)
                return rc;
            
            if (productid != PID_AR0543)
            {
                pr_err("ar0543_match_id: Product id does not match\n");
                return -ENODEV;
            }
    	}

        /* MM-MC-ImplementRegSwitchMechanismForShading-00+{ */
        rc = fih_get_otp_data_ar0543(OTP_SHADING_INDEX , &shading_index);
        if (rc < 0)
            return rc;
        
        if (shading_index == OTP_SHADING_1)
        {
            ar0543_init_conf[0].conf = &ar0543_recommend_settings1[0];
            ar0543_init_conf[0].size = ARRAY_SIZE(ar0543_recommend_settings1);
            ar0543_init_conf[0].delay = 1;
            ar0543_init_conf[0].data_type = MSM_CAMERA_I2C_WORD_DATA;
            printk("ar0543_match_id: Load shading setting for golden module !\n");
        }
        else
            printk("ar0543_match_id: shading_index = %d, Load shading setting for corner module !\n", shading_index);
        /* MM-MC-ImplementRegSwitchMechanismForShading-00+} */
	}//SEVEN++
    
	return rc;
}
/* MM-MC-ImplementOtpReadFunctionForAR0543-00+} */
/* MM-UW-Improve camera open performance-00+} */
/* MM-SL-Improve camera open performance-00*} */

/* MM-MC-ImplementRegSwitchMechanismForShading-00+{ */
int32_t ar0543_get_output_info(struct msm_sensor_ctrl_t *s_ctrl,
		struct sensor_output_info_t *sensor_output_info)
{
	int rc = 0;
    int32_t shading_index = 0;
    
    rc = fih_get_otp_data_ar0543(OTP_SHADING_INDEX , &shading_index);
    if (rc < 0)
        return rc;

    sensor_output_info->shading_index = shading_index;
    printk("ar0543_get_output_info: shading_index = %d !\n", shading_index);
    
	sensor_output_info->num_info = s_ctrl->msm_sensor_reg->num_conf;
	if (copy_to_user((void *)sensor_output_info->output_info,
		s_ctrl->msm_sensor_reg->output_settings,
		sizeof(struct msm_sensor_output_info_t) *
		s_ctrl->msm_sensor_reg->num_conf))
		rc = -EFAULT;

	return rc;
}
/* MM-MC-ImplementRegSwitchMechanismForShading-00+} */


/* MM-UW-Fix MMS can't record 23s-00+{ */
int32_t ar0543_set_fps(struct msm_sensor_ctrl_t *s_ctrl,
						struct fps_cfg *fps)
{
	int32_t rc = 0;
	v_fps = fps->fps_div;

	return rc;
}

/* MM-UW-fix MMS recording fail-01+{ */  
/*MM-MC-ImplementExpsureFunctionForAR0543-00+{ */
static int32_t ar0543_write_prev_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
  uint16_t gain, uint32_t line, int32_t luma_avg, uint16_t fgain)
{
  uint16_t max_legal_gain = 0xE7F; //32x
  uint16_t min_legal_gain = 0x127; //1.584375x
  int32_t rc = 0;

  pr_err("%s: line = %d, v_fps = %d\n", __func__, line, v_fps);
  //line = (line * v_fps)/ Q10; /* MM-UW-fix MMS recording fail-00+{ */  

  if (gain < min_legal_gain) {
    CDBG("gain < min_legal_gain, gain = %d\n", gain);
    gain = min_legal_gain;
	}
  if (gain > max_legal_gain) {
    CDBG("gain > max_legal_gain, gain = %d\n", gain);
    gain = max_legal_gain;
}
  CDBG("%s gain = 0x%x (%d)\n", __func__,gain,gain);
  CDBG("%s line = 0x%x (%d)\n", __func__,line,line);


  s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
  rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x305E,(gain|0x1000),
    MSM_CAMERA_I2C_WORD_DATA);
  rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3012,line,
    MSM_CAMERA_I2C_WORD_DATA);
  if(v_fps > 2000)
      rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x300A,0xA22, MSM_CAMERA_I2C_WORD_DATA); //2594
  else
      rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x300A,0x0457, MSM_CAMERA_I2C_WORD_DATA); 
  s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

  return rc;
}
/*add end*/
/* MM-UW-Fix MMS can't record 23s-00+} */
/* MM-UW-fix MMS recording fail-01+} */  

static int32_t ar0543_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line, int32_t luma_avg, uint16_t fgain)
{
  int32_t rc = 0;
  //rc = ar0543_write_prev_exp_gain(s_ctrl,gain,line);   //uriwei
  rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x301A,(0x065C|0x2),
    MSM_CAMERA_I2C_WORD_DATA);
  return rc;
}
//MM-MC-ImplementExpsureFunctionForAR0543-00+}

static const struct i2c_device_id ar0543_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ar0543_s_ctrl},
	{ }
};

static struct i2c_driver ar0543_i2c_driver = {
	.id_table = ar0543_i2c_id,
	.probe  = ar0543_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ar0543_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&ar0543_i2c_driver);
}

static struct v4l2_subdev_core_ops ar0543_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};
static struct v4l2_subdev_video_ops ar0543_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ar0543_subdev_ops = {
	.core = &ar0543_subdev_core_ops,
	.video  = &ar0543_subdev_video_ops,
};

static struct msm_sensor_fn_t ar0543_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = ar0543_set_fps,
	//MM-MC-ImplementExpsureFunctionForAR0543-00*{
	//.sensor_write_exp_gain = msm_sensor_write_exp_gain1,
	//.sensor_write_snapshot_exp_gain = msm_sensor_write_exp_gain1,
    .sensor_write_exp_gain          = ar0543_write_prev_exp_gain,
    .sensor_write_snapshot_exp_gain = ar0543_write_pict_exp_gain,
    //MM-MC-ImplementExpsureFunctionForAR0543-00*}
	.sensor_setting = msm_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = ar0543_get_output_info,/* MM-MC-ImplementRegSwitchMechanismForShading-00+ */
	.sensor_config = msm_sensor_config,
	.sensor_power_up = ar0543_power_up,
	.sensor_power_down = ar0543_power_down,
	.sensor_adjust_frame_lines = msm_sensor_adjust_frame_lines1,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
	.sensor_match_id = ar0543_match_id,/* MM-MC-ImplementOtpReadFunctionForAR0543-00+ */
};

static struct msm_sensor_reg_t ar0543_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA,
	.start_stream_conf = ar0543_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ar0543_start_settings),
	.stop_stream_conf = ar0543_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ar0543_stop_settings),
	.group_hold_on_conf = ar0543_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(ar0543_groupon_settings),
	.group_hold_off_conf = ar0543_groupoff_settings,
	.group_hold_off_conf_size = ARRAY_SIZE(ar0543_groupoff_settings),
	.init_settings = &ar0543_init_conf[0],
	.init_size = ARRAY_SIZE(ar0543_init_conf),
	.mode_settings = &ar0543_confs[0],
	.output_settings = &ar0543_dimensions[0],
	.num_conf = ARRAY_SIZE(ar0543_confs),
};

static struct msm_sensor_ctrl_t ar0543_s_ctrl = {
	.msm_sensor_reg = &ar0543_regs,
	.sensor_i2c_client = &ar0543_sensor_i2c_client,
	.sensor_i2c_addr = 0x6E,//MM-MC-ImplementSlaveAddressSwitchMechanism-00*
	.sensor_output_reg_addr = &ar0543_reg_addr,
	.sensor_id_info = &ar0543_id_info,
	.sensor_exp_gain_info = &ar0543_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.msm_sensor_mutex = &ar0543_mut,
	.sensor_i2c_driver = &ar0543_i2c_driver,
	.sensor_v4l2_subdev_info = ar0543_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ar0543_subdev_info),
	.sensor_v4l2_subdev_ops = &ar0543_subdev_ops,
	.func_tbl = &ar0543_func_tbl,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Sony 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
