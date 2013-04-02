/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
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
#include "msm_actuator.h"
#include <linux/gpio.h>
#include <mach/gpio.h>

#define SENSOR_NAME "mt9e013_lgit"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9e013_lgit"
#define mt9e013_lgit_obj mt9e013_lgit_##obj

DEFINE_MUTEX(mt9e013_lgit_mut);
static struct msm_sensor_ctrl_t mt9e013_lgit_s_ctrl;

enum mt9e013_lgit_reg_mode {
	E013_X_ADDR_START,
	E013_X_ADDR_END,
	E013_Y_ADDR_START,
	E013_Y_ADDR_END,
	E013_X_OUTPUT_SIZE,
	E013_Y_OUTPUT_SIZE,
	E013_DATAPATH_SELECT,
	E013_READ_MODE,
	E013_ANALOG_CONTROL5,
	E013_DAC_LD_4_5,
	E013_SCALING_MODE,
	E013_SCALE_M,
	E013_LINE_LENGTH_PCK,
	E013_FRAME_LENGTH_LINES,
	E013_COARSE_INTEGRATION_TIME,
	E013_FINE_INTEGRATION_TIME,
	E013_FINE_CORRECTION
};

/* AF Total steps parameters */
#define MT9E013_LGIT_TOTAL_STEPS_NEAR_TO_FAR		32

/* PLL registers */
#define REG_FRAME_LENGTH_LINES		0x0340
#define REG_VCM_CONTROL			0x30F0
#define REG_VCM_NEW_CODE			0x30F2
#define REG_VCM_STEP_TIME			0x30F4
#define REG_LENS_SHADING	        		0x3780

#define LSC_ON						1
#define LSC_OFF 						0

#define MT9E013_LGIT_EEPROM_SLAVE_ADDR	0xA0>>1 	//EEPROM Slave Address for 5100K(Page #1)
#define LSC_FIRST_PHASE1_DATA_SIZE	(5)
#define LSC_FIRST_PHASE2_DATA_SIZE	(20)
#define LSC_SECOND_PHASE_DATA_SIZE	(2)
#define LSC_THIRD_PHASE_DATA_SIZE	(4)

#define MT9E013_LGIT_QTR_SIZE_WIDTH 			(1640)
#define MT9E013_LGIT_QTR_SIZE_HEIGHT 			(1232)
#define MT9E013_LGIT_QTR_SIZE_DUMMY_PIXELS 	(0)
#define MT9E013_LGIT_QTR_SIZE_DUMMY_LINES	(0)

#define MT9E013_LGIT_FULL_SIZE_WIDTH			(3280)
#define MT9E013_LGIT_FULL_SIZE_HEIGHT			(2464)
#define MT9E013_LGIT_FULL_SIZE_DUMMY_PIXELS 	(0)
#define MT9E013_LGIT_FULL_SIZE_DUMMY_LINES	(0)

#define MT9E013_LGIT_FHD_SIZE_WIDTH 			(2640)
#define MT9E013_LGIT_FHD_SIZE_HEIGHT 			(1486)
#define MT9E013_LGIT_FHD_SIZE_DUMMY_PIXELS 	(0)
#define MT9E013_LGIT_FHD_SIZE_DUMMY_LINES 	(0)

static struct msm_camera_i2c_reg_conf mipi_settings[] = {
	{0x3064, 0x7800},	//embedded_data_enable
	{0x31AE, 0x0202},	//2-lane MIPI SERIAL_FORMAT
//	{0x31B8, 0x0E3F},	//MIPI_timing
	/*set data to RAW10 format*/
	{0x0112, 0x0A0A},	/*CCP_DATA_FORMAT*/
	{0x30F0, 0x800D},	/*VCM CONTROL : Enable Power collapse, vcm_slew =5*/
};

/*PLL Configuration
(Ext=24MHz, vt_pix_clk=174MHz, op_pix_clk=69.6MHz)*/
static struct msm_camera_i2c_reg_conf pll_settings[] = {
	{0x0300, 0x0004},	//VT_PIX_CLK_DIV=4
	{0x0302, 0x0001},	//VT_SYS_CLK_DIV=1
	{0x0304, 0x0002},	//PRE_PLL_CLK_DIV=2 //Note: 24MHz/2=12MHz
	{0x0306, 0x0040},	//PLL_MULTIPLIER=64 //Note: Running at 768MHz
	{0x0308, 0x000A},	//OP_PIX_CLK_DIV=10
	{0x030A, 0x0001},	//OP_SYS_CLK_DIV=1
};

static struct msm_camera_i2c_reg_conf prev_settings[] = {
	/*Output Size (1640x1232)*/
// LGE_DOM_UPDATE_S sungsik.kim 2011/10/05 {
	{0x0344, 0x0000},/*X_ADDR_START*/
	{0x0348, 0x0CCF},/*X_ADDR_END*/	//3279				//jungki.kim@lge.com  2011-12-12  Remove Black Line on the Left-side		//{0x0348, 0x0CD1},/*X_ADDR_END*/
	{0x0346, 0x0000},/*Y_ADDR_START*/
	{0x034A, 0x09A1},/*Y_ADDR_END*/
	{0x034C, 0x0668},/*X_OUTPUT_SIZE*/
	{0x034E, 0x04D0},/*Y_OUTPUT_SIZE*/
// LGE_DOM_UPDATE_E sungsik.kim 2011/10/05 }

	{0x306E, 0xFCB0},/*DATAPATH_SELECT*/
	{ 0x3040, 0x04C3}, /*READ_MODE*/
	{0x3178, 0x0000},/*ANALOG_CONTROL5*/
	{0x3ED0, 0x1E24},/*DAC_LD_4_5*/
	{0x0400, 0x0002},/*SCALING_MODE*/
	{0x0404, 0x0010},/*SCALE_M*/
	/*Timing configuration*/
	{0x0342, 0x1280},/*LINE_LENGTH_PCK*/
	{0x0340, 0x0563},/*FRAME_LENGTH_LINES*/
	{0x0202, 0x055F},/*COARSE_INTEGRATION_TIME*/
	{0x3014, 0x0846},/*FINE_INTEGRATION_TIME_*/
	{0x3010, 0x0130},/*FINE_CORRECTION*/	
};

static struct msm_camera_i2c_reg_conf snap_settings[] = {
	/*Output Size (3280x2464)*/
	//[2-lane MIPI 3280x2464 14.8FPS 67.6ms RAW10 Ext=24MHz Vt_pix_clk=192MHz Op_pix_clk=76.8MHz FOV=3280x2464] 
	{0x0344, 0x0000},		//X_ADDR_START 0
	{0x0348, 0x0CCF},		//X_ADDR_END 3279
	{0x0346, 0x0000},		//Y_ADDR_START 0
	{0x034A, 0x099F},		//Y_ADDR_END 2463
	{0x034C, 0x0CD0},		//X_OUTPUT_SIZE 3280
	{0x034E, 0x09A0},		//Y_OUTPUT_SIZE 2464

	{0x306E, 0xFC80},/*DATAPATH_SELECT*/
	{0x3040, 0x0041},/*READ_MODE*/
	{0x3178, 0x0000},/*ANALOG_CONTROL5*/
	{0x3ED0, 0x1E24},/*DAC_LD_4_5*/
	{0x0400, 0x0000},/*SCALING_MODE*/
	{0x0404, 0x0010},/*SCALE_M*/

	/*Timing configuration*/
	{0x0342, 0x1370},/*LINE_LENGTH_PCK*/
	{0x0340, 0x0A2F},/*FRAME_LENGTH_LINES*/
	//{0x0202, 0x0A1F},/*COARSE_INTEGRATION_TIME*/
	{0x3014, 0x03F6},/*FINE_INTEGRATION_TIME_ */
	{0x3010, 0x0078},/*FINE_CORRECTION*/
};

static struct msm_camera_i2c_reg_conf FHD_settings[] = {
	/*Output Size (2640x1486)*/
	{0x0344, 0x0140},	//X_ADDR_START 320
	{0x0348, 0x0B8F},	//X_ADDR_END 2959
	{0x0346, 0x01EA},	//Y_ADDR_START 490
	{0x034A, 0x07B7},	//Y_ADDR_END 1975
	{0x034C, 0x0A50},	//X_OUTPUT_SIZE 2640
	{0x034E, 0x05CE},	//Y_OUTPUT_SIZE 1486

	{0x306E, 0xFC80},/*DATAPATH_SELECT*/
	{0x3040, 0x0041},/*READ_MODE*/
	{0x3178, 0x0000},/*ANALOG_CONTROL5*/
	{0x3ED0, 0x1E24},/*DAC_LD_4_5*/
	{0x0400, 0x0000},/*SCALING_MODE*/
	{0x0404, 0x0010},/*SCALE_M*/
	/*Timing configuration*/
	{0x0342, 0x0FD8},/*LINE_LENGTH_PCK*/		//4056
	{0x0340, 0x065D},/*FRAME_LENGTH_LINES*/		//1629
	{0x0202, 0x0629},/*COARSE_INTEGRATION_TIME*///1577
	{0x3014, 0x0C82},/*FINE_INTEGRATION_TIME_ *///3202
	{0x3010, 0x0078},/*FINE_CORRECTION*///120
};


static struct msm_camera_i2c_reg_conf recommend_settings[] = {
	//mipi timing setting
	{0x31B0, 0x0083},
	{0x31B2, 0x004D},
	{0x31B4, 0x0E67},
	{0x31B6, 0x0D24},
	{0x31B8, 0x020E},
	{0x31BA, 0x0710},
	{0x31BC, 0x2A0D},
	{0x31BE, 0xC007},	// 0xC007 : continuous, 0xC003 : noncontinuous

	//Recommended Settings
	{0x3044, 0x0590},
	{0x306E, 0xFC80},
	{0x30B2, 0xC000},
	{0x30D6, 0x0800},
	{0x316C, 0xB42A},
	{0x316E, 0x869C},	//{0x316E, 0x869B},	// jungki.kim@lge.com  2011-10-24  not to appear the sun to be black
	{0x3170, 0x210E},
	{0x317A, 0x010E},
	{0x31E0, 0x1FB9},
	{0x31E6, 0x07FC},
	{0x37C0, 0x0000},
	{0x37C2, 0x0000},
	{0x37C4, 0x0000},
	{0x37C6, 0x0000},
	{0x3E00, 0x0011},
	{0x3E02, 0x8801},
	{0x3E04, 0x2801},
	{0x3E06, 0x8449},
	{0x3E08, 0x6841},
	{0x3E0A, 0x400C},
	{0x3E0C, 0x1001},
	{0x3E0E, 0x2603},
	{0x3E10, 0x4B41},
	{0x3E12, 0x4B24},
	{0x3E14, 0xA3CF},
	{0x3E16, 0x8802},
	{0x3E18, 0x8401},
	{0x3E1A, 0x8601},
	{0x3E1C, 0x8401},
	{0x3E1E, 0x840A},
	{0x3E20, 0xFF00},
	{0x3E22, 0x8401},
	{0x3E24, 0x00FF},
	{0x3E26, 0x0088},
	{0x3E28, 0x2E8A},
	{0x3E30, 0x0000},
	{0x3E32, 0x00FF},
	{0x3E34, 0x4029},
	{0x3E36, 0x00FF},
	{0x3E38, 0x8469},
	{0x3E3A, 0x00FF},
	{0x3E3C, 0x2801},
	{0x3E3E, 0x3E2A},
	{0x3E40, 0x1C01},
	{0x3E42, 0xFF84},
	{0x3E44, 0x8401},
	{0x3E46, 0x0C01},
	{0x3E48, 0x8401},
	{0x3E4A, 0x00FF},
	{0x3E4C, 0x8402},
	{0x3E4E, 0x8984},
	{0x3E50, 0x6628},
	{0x3E52, 0x8340},
	{0x3E54, 0x00FF},
	{0x3E56, 0x4A42},
	{0x3E58, 0x2703},
	{0x3E5A, 0x6752},
	{0x3E5C, 0x3F2A},
	{0x3E5E, 0x846A},
	{0x3E60, 0x4C01},
	{0x3E62, 0x8401},
	{0x3E66, 0x3901},
	{0x3E90, 0x2C01},
	{0x3E98, 0x2B02},
	{0x3E92, 0x2A04},
	{0x3E94, 0x2509},
	{0x3E96, 0xF000},
	{0x3E9A, 0x2905},
	{0x3E9C, 0x00FF},
	{0x3ECC, 0x00D8},	//{0x3ECC, 0x00E4},	//{0x3ECC, 0x00EB},	// jungki.kim@lge.com  2011-12-19  not to appear the sun to be black and vertical lines
	{0x3ED0, 0x1E24},
	{0x3ED4, 0xFAA4},
	{0x3ED6, 0x909B},
	{0x3EE0, 0x2424},
	{0x3EE4, 0xC100},
	{0x3EE6, 0x0540},
	{0x3174, 0x8000},
};

// jungki.kim@lge.com  2011-10-05  Read From EEPROM (LSC data)
static struct msm_camera_i2c_reg_conf lsc_data[LSC_FIRST_PHASE1_DATA_SIZE*LSC_FIRST_PHASE2_DATA_SIZE+LSC_SECOND_PHASE_DATA_SIZE+LSC_THIRD_PHASE_DATA_SIZE] ;
// jungki.kim@lge.com  2011-10-05  Read From EEPROM (LSC data)

static struct sensor_extra_t {
	uint16_t curr_lens_pos;
	uint16_t curr_step_pos;
//	uint16_t my_reg_gain;
//	uint32_t my_reg_line_count;
//	uint16_t total_lines_per_frame;
} mt9e013_lgit_extra;
static struct sensor_extra_t * mt9e013_lgit_ctrl = &mt9e013_lgit_extra;

static uint16_t mt9e013_lgit_linear_total_step = MT9E013_LGIT_TOTAL_STEPS_NEAR_TO_FAR;
static uint16_t mt9e013_lgit_step_position_table[MT9E013_LGIT_TOTAL_STEPS_NEAR_TO_FAR+1];
// jungki.kim@lge.com  2011-11-21  Fast AF tuning {
static uint16_t af_infinity = 30;  // sungmin.woo : at least 64~64+96
static uint16_t mt9e013_lgit_nl_region_boundary1 = 0;
static uint16_t mt9e013_lgit_nl_region_code_per_step1 = 0;
static uint16_t mt9e013_lgit_l_region_code_per_step = 5;
static uint16_t mt9e013_lgit_vcm_step_time;
static uint16_t mt9e013_lgit_sw_damping_time_wait;
// jungki.kim@lge.com  2011-11-21  Fast AF tuning }

static struct msm_camera_i2c_reg_conf mt9e013_lgit_groupon_settings[] = {
	{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf mt9e013_lgit_groupoff_settings[] = {
	{0x0104, 0x00},
};

static struct v4l2_subdev_info mt9e013_lgit_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array mt9e013_lgit_init_conf[] = {
	{&mipi_settings[0],
	ARRAY_SIZE(mipi_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&pll_settings[0],
	ARRAY_SIZE(pll_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&recommend_settings[0],
	ARRAY_SIZE(recommend_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9e013_lgit_confs[] = {
	{&snap_settings[0],
	ARRAY_SIZE(snap_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&prev_settings[0],
	ARRAY_SIZE(prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&FHD_settings[0],
	ARRAY_SIZE(FHD_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};


static struct msm_sensor_output_info_t mt9e013_lgit_dimensions[] = {
	//(Ext=24MHz, vt_pix_clk=174MHz, op_pix_clk=69.6MHz)*/
	{ //MSM_SENSOR_RES_FULL
		.x_output = MT9E013_LGIT_FULL_SIZE_WIDTH,
		.y_output = MT9E013_LGIT_FULL_SIZE_HEIGHT,
		.line_length_pclk = 0x1370, //0x1370, 
		.frame_length_lines = 0x0A2F, //0x0A2F,
		.vt_pixel_clk = 192000000, // 330000000, //174000000,
		.op_pixel_clk = 76800000, // 69600000,
		.binning_factor = 1,	
	},
	{ //MSM_SENSOR_RES_QTR
		.x_output = MT9E013_LGIT_QTR_SIZE_WIDTH,
		.y_output = MT9E013_LGIT_QTR_SIZE_HEIGHT,
		.line_length_pclk = 0x1280, //0x1018,
		.frame_length_lines = 0x0563, //0x055B,
		.vt_pixel_clk = 192000000, //174000000,
		.op_pixel_clk = 76800000, // 69600000,
		.binning_factor = 1,	
	},
	{ //MSM_SENSOR_RES_2
		.x_output = MT9E013_LGIT_FHD_SIZE_WIDTH,
		.y_output = MT9E013_LGIT_FHD_SIZE_HEIGHT,
		.line_length_pclk = 0x0FD8, //0x0FD8,
		.frame_length_lines = 0x065D, //0x065D,
		.vt_pixel_clk = 192000000, //174000000,
		.op_pixel_clk = 76800000, // 69600000,
		.binning_factor = 1,	
	},
};

static struct msm_camera_csi_params mt9e013_lgit_csi_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x18,
};

static struct msm_camera_csi_params *mt9e013_lgit_csi_params_array[] = {
	&mt9e013_lgit_csi_params,
	&mt9e013_lgit_csi_params, //NULL, //when res is FULL_SIZE, Nothing 
	&mt9e013_lgit_csi_params,
};

static struct msm_sensor_output_reg_addr_t mt9e013_lgit_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t mt9e013_lgit_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x4B00,
};

static struct msm_sensor_exp_gain_info_t mt9e013_lgit_exp_gain_info = {
	.coarse_int_time_addr = 0x3012, //REG_COARSE_INTEGRATION_TIME
	.global_gain_addr = 0x305E, //REG_GLOBAL_GAIN
	.vert_offset = 0,
};

static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

// jungki.kim@lge.com  2011-10-03  Read from EEPROM
static int32_t mt9e013_lgit_i2c_read_w_eeprom(struct msm_camera_i2c_client* sensor_i2c_client, 
	uint16_t reg_addr, uint16_t *rdata)
{
	int32_t rc = 0;
	unsigned char buf;
	
	if (!rdata)
		return -EIO;

	//Read 2 bytes in sequence 
	//Big Endian address:
	buf = reg_addr;
	buf = (reg_addr & 0xFF00) >> 8;
	rc = msm_camera_i2c_rxdata_manual(sensor_i2c_client->client->adapter, MT9E013_LGIT_EEPROM_SLAVE_ADDR, &buf, 1);
	if (rc < 0) {
		printk(KERN_EMERG "[CAMERA]1 :mt9e013_lgit_i2c_read_eeprom 0x%x failed!\n", buf);
		return rc;
	}
	*rdata = buf;

	buf = (reg_addr & 0x00FF);	
	rc = msm_camera_i2c_rxdata_manual(sensor_i2c_client->client->adapter, MT9E013_LGIT_EEPROM_SLAVE_ADDR, &buf, 1);
	if (rc < 0) {
		printk(KERN_EMERG "[CAMERA]2 :mt9e013_lgit_i2c_read_eeprom 0x%x failed!\n", buf);
		return rc;
	}
	*rdata = (*rdata<<8)|buf;

	return rc;
}

static int32_t mt9e013_lgit_read_5100k_data(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	int32_t index_lsc = 0;
	int32_t i,j;
	int32_t n=0x01;	
	uint16_t reg_addr_read = 0x0001;
	uint16_t eepromdata = 0;
	uint16_t reg_addr_write;

	//First Phase
	reg_addr_write = 0x3600;
	printk(KERN_EMERG "[CAMERA][EEPROM] %s : Start : 0x%04x 0x%04x\n", __func__, reg_addr_read, reg_addr_write);
	for( j = 0 ; j < LSC_FIRST_PHASE1_DATA_SIZE ; j++) {
		for( i = 0 ; i < LSC_FIRST_PHASE2_DATA_SIZE ; i++) {
			rc = mt9e013_lgit_i2c_read_w_eeprom(s_ctrl->sensor_i2c_client, reg_addr_read, &eepromdata);
			if (rc < 0) {
				printk(KERN_EMERG "[CAMERA]%s : Fail to read!! First Phase (j=%d,i=%d) Source line number : %d\n", __func__, j, i, __LINE__);
				return rc;
			}
#ifdef EEPROM_CHECK
			printk("[CAMERA][TEST]!!!! GOGO raddr:0x%04x waddr:0x%04x eepromdata: 0x%04x\n", reg_addr_read, reg_addr_write, eepromdata);
#endif			
			lsc_data[index_lsc].reg_addr=reg_addr_write;
			lsc_data[index_lsc].reg_data=eepromdata;
			
			reg_addr_read++;
			reg_addr_write += 2;
			n += 2;
			index_lsc++;
			reg_addr_read = (reg_addr_read<<8) |n;
		}
		reg_addr_write = reg_addr_write + 0x0018;
	}

	
	// Second Phase
	reg_addr_write = 0x3782;
	for( i=0 ; i < LSC_SECOND_PHASE_DATA_SIZE ; i++) {
		rc = mt9e013_lgit_i2c_read_w_eeprom(s_ctrl->sensor_i2c_client, reg_addr_read, &eepromdata);
		if (rc < 0) {
			printk(KERN_EMERG "[CAMERA]%s : Fail to read!! Second Phase i=%d Source line number : %d\n", __func__,  i, __LINE__);
			return rc;
		}
#ifdef EEPROM_CHECK
		printk("[CAMERA][TEST]!!!! GOGO raddr:0x%04x waddr:0x%04x eepromdata: 0x%04x\n", reg_addr_read, reg_addr_write, eepromdata);
#endif
		lsc_data[index_lsc].reg_addr=reg_addr_write;
		lsc_data[index_lsc].reg_data=eepromdata;
		
		reg_addr_read++;
		reg_addr_write += 2;
		n += 2; 
		index_lsc++;
		reg_addr_read = (reg_addr_read<<8) |n; // reg_addr_write->reg_addr_read
	}

	// Third Phase
	reg_addr_write = 0x37C0;
	for( i=0 ; i < LSC_THIRD_PHASE_DATA_SIZE; i++) {
		rc = mt9e013_lgit_i2c_read_w_eeprom(s_ctrl->sensor_i2c_client, reg_addr_read, &eepromdata);
		if (rc < 0) {
			printk(KERN_EMERG "[CAMERA]%s : Fail to read!! Third Phase i=%d Source line number : %d\n", __func__, i, __LINE__);
			return rc;
		}
#ifdef EEPROM_CHECK
		printk("[CAMERA][TEST]!!!! GOGO raddr:0x%04x reg_addr:0x%04x eepromdata: 0x%04x\n", reg_addr_read, reg_addr_write, eepromdata);
#endif
		lsc_data[index_lsc].reg_addr=reg_addr_write;
		lsc_data[index_lsc].reg_data=eepromdata;

		reg_addr_read++;
		reg_addr_write += 2;
		n += 2;
		index_lsc++;
		reg_addr_read = (reg_addr_read<<8) |n;
	}	
	
#ifdef EEPROM_CHECK
	for(i = 0; i <ARRAY_SIZE(lsc_data); i++) {
		printk("[CAMERA][EEPROM_CHECK]index: %03d reg_addr:0x%04x eepromdata: 0x%04x\n", i, lsc_data[i].reg_addr, lsc_data[i].reg_data);
	}
#endif

	return rc;
}

static int32_t mt9e013_lgit_read_awb_data(struct msm_sensor_ctrl_t *s_ctrl, struct sensor_calib_data *sensor_cablib_data, bool bresult)
{
	int32_t rc = 0;

	uint16_t reg_addr=0;
	uint16_t eepromdata = 0;

	reg_addr=0xD4D5;	// R/G
	rc = mt9e013_lgit_i2c_read_w_eeprom(s_ctrl->sensor_i2c_client, reg_addr, &eepromdata);
	if (rc < 0) {
		printk(KERN_EMERG "[CAMERA]%s : Fail to read!! Source line number : %d\n", __func__, __LINE__);
		return rc;
	}
	// If there is no data in EEPROM, Apply static value.
	if(!bresult) {
		sensor_cablib_data->r_over_g = 0x0300;	//0x005E;
	} else {
		sensor_cablib_data->r_over_g = eepromdata;
	}
	printk("[CAMERA]%s : line:%d : R/G 0x%04x\n", __func__, __LINE__, sensor_cablib_data->r_over_g);

	reg_addr=0xD6D7;	// B/G
	rc = mt9e013_lgit_i2c_read_w_eeprom(s_ctrl->sensor_i2c_client, reg_addr, &eepromdata);
	if (rc < 0) {
		printk(KERN_EMERG "[CAMERA]%s : Fail to read!! Source line number : %d\n", __func__, __LINE__);
		return rc;
	}
	// If there is no data in EEPROM, Apply static value.
	if(!bresult) {
		sensor_cablib_data->b_over_g = 0x0289;	//0x0051;
	} else {
		sensor_cablib_data->b_over_g = eepromdata;
	}
	printk("[CAMERA]%s : line:%d : B/G 0x%04x\n", __func__, __LINE__, sensor_cablib_data->b_over_g);

	reg_addr=0xD4D5;	// Gr/Gb
	rc = mt9e013_lgit_i2c_read_w_eeprom(s_ctrl->sensor_i2c_client, reg_addr, &eepromdata);
	if (rc < 0) {
		printk(KERN_EMERG "[CAMERA]%s : Fail to read!! Source line number : %d\n", __func__, __LINE__);
		return rc;
	}
	// If there is no data in EEPROM, Apply static value.
	if(!bresult) {
		sensor_cablib_data->gr_over_gb = 0x0300;	//0x005E;
	} else {
		sensor_cablib_data->gr_over_gb = eepromdata;
	}
	printk(KERN_EMERG "[CAMERA]%s : line:%d : GR/GB 0x%04x\n", __func__, __LINE__, sensor_cablib_data->gr_over_gb);

	return rc;
}

static int32_t mt9e013_lgit_write_5100k_data(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc=0;

	if (s_ctrl->func_tbl->sensor_group_hold_on)
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);

	rc = msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, &lsc_data[0], ARRAY_SIZE(lsc_data), MSM_CAMERA_I2C_WORD_DATA);

	if (s_ctrl->func_tbl->sensor_group_hold_off)
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	
	return rc;
}

static int32_t mt9e013_lgit_lens_shading_enable(struct msm_sensor_ctrl_t *s_ctrl, uint8_t is_enable)
{
	int32_t rc = 0;

	printk("[CAMERA]%s: entered. enable = %d\n", __func__, is_enable);

	if (s_ctrl->func_tbl->sensor_group_hold_on)
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);

	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
			REG_LENS_SHADING, ((uint16_t) is_enable) << 15, MSM_CAMERA_I2C_WORD_DATA);	
	if (rc < 0)
		return rc;

	if (s_ctrl->func_tbl->sensor_group_hold_off)
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

	printk("[CAMERA]%s: exiting. rc = %d\n", __func__, rc);
	return rc;
}

static int mt9e013_lgit_read_eeprom_data(struct msm_sensor_ctrl_t *s_ctrl,
		struct sensor_calib_data *sensor_cablib_data)
{
	int32_t rc = 0;
	uint16_t eepromdata = 0;
	uint16_t reg_addr = 0;
	bool bresult = false;

	printk("[CAMERA][MT9E013_LGIT_EEPROM] Start reading EEPROM\n");	

	/*#1. Model ID for checking EEPROM READ*/
	reg_addr = 0xFEFF;
	rc = mt9e013_lgit_i2c_read_w_eeprom(s_ctrl->sensor_i2c_client, reg_addr, &eepromdata);
	if (rc < 0) {
		printk(KERN_EMERG "[CAMERA]%s: Error Reading EEPROM @ 0x%x\n", __func__, reg_addr);
		return rc;
	}
	if(eepromdata == 0x0AFF) 
		bresult = true;
	else 
		bresult = false;
	
	printk(KERN_EMERG "[CAMERA][QCTK_EEPROM] Product version = 0x%x bresult:%d\n", eepromdata,bresult); 

	/*#2. 5100K LSC : Read LSC table Data from EEPROM */
	rc = mt9e013_lgit_read_5100k_data(s_ctrl); // read LSC data
	if (rc < 0) {
		printk(KERN_EMERG "[CAMERA]%s:%d: Error Reading\n", __func__,__LINE__);
		return rc;
	}
	
	/*#3. 5100K AWB Data from EEPROM */ 
	mt9e013_lgit_read_awb_data(s_ctrl, sensor_cablib_data, bresult);
	printk("[CAMERA]%s:%d: AWB: r/g:0x%04x b/g:0x%04x gr/gb:0x%04x\n", __func__,__LINE__,
			sensor_cablib_data->r_over_g, sensor_cablib_data->b_over_g, sensor_cablib_data->gr_over_gb);
	
	/*#4. Write LSC data to sensor - it will be enabled in setting routine*/
	//Write LSC table to the sensor
	rc = mt9e013_lgit_write_5100k_data(s_ctrl);
	if (rc < 0) {
		printk(KERN_EMERG "[CAMERA]%s:%d: Error Writing\n", __func__,__LINE__);
		return rc;
	}
	
	/*Enable Aptina Lens shading */
	mt9e013_lgit_lens_shading_enable(s_ctrl, LSC_ON); // LSC_OFF->LSC_ON

	return rc;
}


static int32_t mt9e013_lgit_get_pict_fps(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t fps, uint16_t *pfps)
{
	/* input fps is preview fps in Q8 format */
	int32_t rc = 0;
	uint32_t divider, d1, d2;

	d1 = prev_settings[E013_FRAME_LENGTH_LINES].reg_data * 0x00000400
		/ snap_settings[E013_FRAME_LENGTH_LINES].reg_data;
	d2 = prev_settings[E013_LINE_LENGTH_PCK].reg_data * 0x00000400
		/ snap_settings[E013_LINE_LENGTH_PCK].reg_data;
	divider = d1 * d2 / 0x400;
	printk("[CAMERA]mt9e013_lgit_get_pict_fps: divider = %d, d1 = %d, d2 = %d \n", divider, d1, d2);

	/*Verify PCLK settings and frame sizes.*/
	*pfps = (uint16_t) (fps * divider / 0x400);
	/* 2 is the ratio of no.of snapshot channels
	to number of preview channels */

	printk("[CAMERA]mt9e013_lgit_get_pict_fps:fps = %d, pfps = %d\n", fps, *pfps);
	return rc;
}

static int32_t mt9e013_lgit_get_prev_lines_pf(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t *p_prevl_pf)
{
	int32_t rc = 0;

	printk("[CAMERA]mt9e013_lgit_get_prev_lines_pf\n");

	if (s_ctrl->curr_res == MSM_SENSOR_RES_QTR /* QTR_SIZE */)
		*p_prevl_pf = prev_settings[E013_FRAME_LENGTH_LINES].reg_data;
	else if (s_ctrl->curr_res == MSM_SENSOR_RES_2 /* FHD_SIZE */)
		*p_prevl_pf = FHD_settings[E013_FRAME_LENGTH_LINES].reg_data;
	else //MSM_SENSOR_RES_FULL /* FULL_SIZE */
		*p_prevl_pf = snap_settings[E013_FRAME_LENGTH_LINES].reg_data;

	return rc;
}

static int32_t mt9e013_lgit_get_prev_pixels_pl(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t *p_prevp_pl)
{
	int32_t rc = 0;

	printk("[CAMERA]mt9e013_lgit_get_prev_pixels_pl\n");

	if (s_ctrl->curr_res == MSM_SENSOR_RES_QTR /* QTR_SIZE */)
		*p_prevp_pl = prev_settings[E013_LINE_LENGTH_PCK].reg_data;
	else if (s_ctrl->curr_res == MSM_SENSOR_RES_2 /* FHD_SIZE */)
		*p_prevp_pl = FHD_settings[E013_LINE_LENGTH_PCK].reg_data;
	else //MSM_SENSOR_RES_FULL /* FULL_SIZE */
		*p_prevp_pl = snap_settings[E013_LINE_LENGTH_PCK].reg_data;

	return rc;	
}

static int32_t mt9e013_lgit_get_pict_lines_pf(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t *p_pictl_pf)
{
	int32_t rc = 0;

	printk("[CAMERA]mt9e013_lgit_get_pict_lines_pf\n");

	if (s_ctrl->curr_res == MSM_SENSOR_RES_QTR /* QTR_SIZE */)
		*p_pictl_pf = prev_settings[E013_FRAME_LENGTH_LINES].reg_data;
	else if (s_ctrl->curr_res == MSM_SENSOR_RES_2 /* FHD_SIZE */)
		*p_pictl_pf = FHD_settings[E013_FRAME_LENGTH_LINES].reg_data;
	else //MSM_SENSOR_RES_FULL /* FULL_SIZE */
		*p_pictl_pf = snap_settings[E013_FRAME_LENGTH_LINES].reg_data;

	return rc;		
}

static int32_t mt9e013_lgit_get_pict_pixels_pl(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t *p_pictp_pl)
{
	int32_t rc = 0;

	printk("[CAMERA]mt9e013_lgit_get_pict_pixels_pl\n");

	if (s_ctrl->curr_res == MSM_SENSOR_RES_QTR /* QTR_SIZE */)
		*p_pictp_pl = prev_settings[E013_LINE_LENGTH_PCK].reg_data;
	else if (s_ctrl->curr_res == MSM_SENSOR_RES_2 /* FHD_SIZE */)
		*p_pictp_pl = FHD_settings[E013_LINE_LENGTH_PCK].reg_data;
	else //MSM_SENSOR_RES_FULL /* FULL_SIZE */
		*p_pictp_pl = snap_settings[E013_LINE_LENGTH_PCK].reg_data;

	return rc;			
}

static int32_t mt9e013_lgit_get_pict_max_exp_lc(struct msm_sensor_ctrl_t *s_ctrl,
	uint32_t *p_pict_max_exp_lc)
{
	int32_t rc = 0;

	printk("[CAMERA]mt9e013_lgit_get_pict_max_exp_lc\n");

	if (s_ctrl->curr_res == MSM_SENSOR_RES_QTR /* QTR_SIZE */)
		*p_pict_max_exp_lc = prev_settings[E013_FRAME_LENGTH_LINES].reg_data * 24;
	else if (s_ctrl->curr_res == MSM_SENSOR_RES_2 /* FHD_SIZE */)
		*p_pict_max_exp_lc = FHD_settings[E013_FRAME_LENGTH_LINES].reg_data * 24;
	else //MSM_SENSOR_RES_FULL /* FULL_SIZE */
		*p_pict_max_exp_lc = snap_settings[E013_FRAME_LENGTH_LINES].reg_data * 24;

	return rc;			
}

static int32_t mt9e013_lgit_get_af_max_steps(struct msm_sensor_ctrl_t *s_ctrl,
	uint8_t *pmax_steps)
{
	int32_t rc = 0;

	*pmax_steps = mt9e013_lgit_linear_total_step;
	
	return rc;			
}

static int32_t mt9e013_lgit_set_fps(struct msm_sensor_ctrl_t *s_ctrl,
						struct fps_cfg *fps)
{
	uint16_t total_lines_per_frame;
	int32_t rc = 0;

	printk("[CAMERA]mt9e013_lgit_set_fps mode:%d\n", s_ctrl->curr_res);
	
	if (s_ctrl->curr_res == MSM_SENSOR_RES_QTR /* QTR_SIZE */)
		total_lines_per_frame =	prev_settings[E013_FRAME_LENGTH_LINES].reg_data;
	else if (s_ctrl->curr_res == MSM_SENSOR_RES_2 /* FHD_SIZE */)
		total_lines_per_frame =	FHD_settings[E013_FRAME_LENGTH_LINES].reg_data;
	else //MSM_SENSOR_RES_FULL /* FULL_SIZE */
		total_lines_per_frame =	snap_settings[E013_FRAME_LENGTH_LINES].reg_data;


	s_ctrl->fps_divider = fps->fps_div;
//No on JB	s_ctrl->pict_fps_divider = fps->pict_fps_div;
	printk("[CAMERA][1] mt9e013_lgit_set_fps : total_lines_per_frame = %d, fps->fps_div = %d, fps->pict_fps_div = %d\n", 
												total_lines_per_frame, fps->fps_div, fps->pict_fps_div);

//No on JB	if (s_ctrl->curr_res == FULL_SIZE) {
//No on JB		total_lines_per_frame = (uint16_t)
//No on JB		(total_lines_per_frame * mt9e013_lgit_ctrl->pict_fps_divider/0x400);
//No on JB	} else {
		total_lines_per_frame = (uint16_t)
		(total_lines_per_frame * s_ctrl->fps_divider/0x400);
//No on JB	}
	printk("[CAMERA][2] mt9e013_lgit_set_fps : total_lines_per_frame = %d, fps->fps_div = %d, fps->pict_fps_div = %d\n", 
												total_lines_per_frame, fps->fps_div,  fps->pict_fps_div );

	if (s_ctrl->func_tbl->sensor_group_hold_on)
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_FRAME_LENGTH_LINES, total_lines_per_frame, 
			MSM_CAMERA_I2C_WORD_DATA);

	if (s_ctrl->func_tbl->sensor_group_hold_off)
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	
	return rc;
}

static int32_t mt9e013_lgit_write_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint16_t max_legal_gain = 0xE7F;
	//uint16_t frame_length_lines;

	int32_t rc = 0;

	printk("mt9e013_lgit_write_exp_gain entering.... \n");
//No on JB	if (mt9e013_lgit_ctrl->curr_res == SENSOR_PREVIEW_MODE) {
//No on JB		mt9e013_lgit_ctrl->my_reg_gain = gain;
//No on JB		mt9e013_lgit_ctrl->my_reg_line_count = (uint16_t) line;
//No on JB	}

	if (gain > max_legal_gain) {
		printk("Max legal gain Line:%d\n", __LINE__);
		gain = max_legal_gain;
	}

//No on JB	if (mt9e013_lgit_ctrl->sensormode != SENSOR_SNAPSHOT_MODE) {
		line = (uint32_t) (line * s_ctrl->fps_divider /  0x00000400 /*Q10*/);
//No on JB	} else {
//No on JB		line = (uint32_t) (line * mt9e013_lgit_ctrl->pict_fps_divider /  0x00000400);
//No on JB	}

	printk("[CAMERA][1] mt9e013_lgit_write_exp_gain : frame_length_lines = NONE, mt9e013_lgit_ctrl->fps_divider = %d, mt9e013_lgit_ctrl->pict_fps_divider = %d\n", 
									/*frame_length_lines, */s_ctrl->fps_divider, -1 /* mt9e013_lgit_ctrl->pict_fps_divider */);	
	printk("[CAMERA][2] mt9e013_lgit_write_exp_gain : line=%d gain=%d\n",line, gain); 

	gain |= 0x1000;

	if (s_ctrl->func_tbl->sensor_group_hold_on)
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
		MSM_CAMERA_I2C_WORD_DATA);
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
		MSM_CAMERA_I2C_WORD_DATA);
	
	if (s_ctrl->func_tbl->sensor_group_hold_off)
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

	return rc;
}

static int32_t mt9e013_lgit_write_exp_snapshot_gain(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t gain, uint32_t line)
{
	int32_t rc = 0;

	printk("[CAMERA]mt9e013_lgit_set_pict_exp_gain : gain = %d,line = %d \n", gain, line);

	if (s_ctrl->func_tbl->sensor_write_exp_gain)
		rc = s_ctrl->func_tbl->sensor_write_exp_gain(s_ctrl, gain, line);
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, (0x065C|0x2), MSM_CAMERA_I2C_WORD_DATA);	
	
	return rc;
}

static int32_t mt9e013_lgit_move_focus(struct msm_sensor_ctrl_t *s_ctrl,
	int direction, int32_t num_steps)
{
	int16_t step_direction, dest_lens_position, dest_step_position;

	printk("[CAMERA]mt9e013_lgit_move_focus : direction = %d,num_steps = %d \n", direction, num_steps);

	if (direction == MOVE_NEAR)
		step_direction = 1;
	else
		step_direction = -1;

	dest_step_position = mt9e013_lgit_ctrl->curr_step_pos
						+ (step_direction * num_steps);

	if (dest_step_position < 0)
		dest_step_position = 0;
	else if (dest_step_position > mt9e013_lgit_linear_total_step)
		dest_step_position = mt9e013_lgit_linear_total_step;

	if (dest_step_position == mt9e013_lgit_ctrl->curr_step_pos)
		return 0;
	printk("[CAMERA]__debug:MoveFocus, dest_step_position:%d \n", dest_step_position);
	dest_lens_position = mt9e013_lgit_step_position_table[dest_step_position];

	if ((dest_step_position <= 4) && (step_direction == 1)) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			REG_VCM_STEP_TIME, 0x0000, MSM_CAMERA_I2C_WORD_DATA);	
		if (num_steps == 4) {
			printk("[CAMERA]__debug:MoveFocus, jumpvalue:%d \n",
				mt9e013_lgit_nl_region_boundary1 * mt9e013_lgit_nl_region_code_per_step1);
			msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
				REG_VCM_NEW_CODE, mt9e013_lgit_nl_region_boundary1 * mt9e013_lgit_nl_region_code_per_step1, MSM_CAMERA_I2C_WORD_DATA);				
		} else {
			if (dest_step_position <= mt9e013_lgit_nl_region_boundary1) {
				printk("[CAMERA]__debug:MoveFocus, fine search:%d \n",
					dest_lens_position);
				msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
					REG_VCM_NEW_CODE, dest_lens_position, MSM_CAMERA_I2C_WORD_DATA);					
				mt9e013_lgit_ctrl->curr_lens_pos = dest_lens_position;
				mt9e013_lgit_ctrl->curr_step_pos = dest_step_position;
				return 0;
			}
		}
	}

	if(step_direction < 0) {
		if(num_steps > 20) {
			/*macro to infinity*/
			mt9e013_lgit_vcm_step_time = 0x0050;
			mt9e013_lgit_sw_damping_time_wait = 5;
		} else if (num_steps <= 4) {
			/*reverse search fine step  dir - macro to infinity*/
			mt9e013_lgit_vcm_step_time = 0x0400;
			mt9e013_lgit_sw_damping_time_wait = 4;
		} else {
			/*reverse search Coarse Jump ( > 4) dir - macro to infinity*/
			mt9e013_lgit_vcm_step_time = 0x96;
			mt9e013_lgit_sw_damping_time_wait = 3;
			}
	} else {
		if(num_steps >= 4) {
			/*coarse jump  dir - infinity to macro*/
			mt9e013_lgit_vcm_step_time = 0x0200;
			mt9e013_lgit_sw_damping_time_wait = 2;
		} else {
			/*fine step  dir - infinity to macro*/
			mt9e013_lgit_vcm_step_time = 0x0400;
			mt9e013_lgit_sw_damping_time_wait = 4;
		}
	}

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		REG_VCM_STEP_TIME, mt9e013_lgit_vcm_step_time, MSM_CAMERA_I2C_WORD_DATA);	
	
	printk("[CAMERA]__debug:MoveFocus, mt9e013_lgit_vcm_step_time:%d \n", mt9e013_lgit_vcm_step_time);
	printk("[CAMERA]__debug:MoveFocus, DestLensPosition:%d \n", dest_lens_position);
	if (mt9e013_lgit_ctrl->curr_lens_pos != dest_lens_position) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			REG_VCM_NEW_CODE, dest_lens_position, MSM_CAMERA_I2C_WORD_DATA);			
		usleep(mt9e013_lgit_sw_damping_time_wait * 1000);
	}
	mt9e013_lgit_ctrl->curr_lens_pos = dest_lens_position;
	mt9e013_lgit_ctrl->curr_step_pos = dest_step_position;
	
	return 0;
}

static int32_t mt9e013_lgit_set_default_focus(struct msm_sensor_ctrl_t *s_ctrl,
	uint8_t af_step)
{
	int32_t rc = 0;

	printk("[CAMERA]mt9e013_lgit_set_default_focus : af_step = %d \n", af_step);

	if (mt9e013_lgit_ctrl->curr_step_pos != 0) {
		rc = mt9e013_lgit_move_focus(s_ctrl,MOVE_FAR, mt9e013_lgit_ctrl->curr_step_pos);
	} else {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, REG_VCM_NEW_CODE, 0x00, MSM_CAMERA_I2C_WORD_DATA);			
	}

	mt9e013_lgit_ctrl->curr_lens_pos = 0;
	mt9e013_lgit_ctrl->curr_step_pos = 0;

	return rc;
}

static int32_t mt9e013_lgit_set_effect (struct msm_sensor_ctrl_t *s_ctrl,
	int8_t effect)
{
	return mt9e013_lgit_set_default_focus(s_ctrl, (uint8_t)effect);
}


static void mt9e013_lgit_init_focus(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint8_t i;

	printk("[CAMERA]mt9e013_lgit_init_focus\n");

	mt9e013_lgit_step_position_table[0] = af_infinity;

	for (i = 1; i <= mt9e013_lgit_linear_total_step; i++) 
	{
		if (i <= mt9e013_lgit_nl_region_boundary1) 
		{
			mt9e013_lgit_step_position_table[i] = mt9e013_lgit_step_position_table[i-1]+ mt9e013_lgit_nl_region_code_per_step1;
		}
		else 
		{
			mt9e013_lgit_step_position_table[i] = mt9e013_lgit_step_position_table[i-1] + mt9e013_lgit_l_region_code_per_step;
		}
		if (mt9e013_lgit_step_position_table[i] > 255)
			mt9e013_lgit_step_position_table[i] = 255;
	}
	mt9e013_lgit_ctrl->curr_lens_pos = 0;
}

static int32_t mt9e013_lgit_af_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	printk("[CAMERA]mt9e013_lgit_af_power_down\n");

	if (mt9e013_lgit_ctrl->curr_lens_pos != 0)
	{
		mt9e013_lgit_set_default_focus(s_ctrl, 0);
		msleep(40);
	}
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		REG_VCM_CONTROL, 0x00, MSM_CAMERA_I2C_WORD_DATA);		
	return 0;
}

static int32_t mt9e013_lgit_power_up(struct msm_sensor_ctrl_t *s_ctrl) 
{
#if 1
	int32_t rc = 0;
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	printk("%s: E %d\n", __func__, __LINE__);

	memset(mt9e013_lgit_ctrl, 0x00, sizeof(*mt9e013_lgit_ctrl));
	s_ctrl->fps_divider = 1 * 0x00000400;
	s_ctrl->curr_res = MSM_SENSOR_RES_QTR /* QTR_SIZE */;
	mt9e013_lgit_init_focus(s_ctrl);

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

	//DVDD 1.2V
	gpio_tlmm_config(GPIO_CFG(49, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(49, 1);
	mdelay(10);
	
	//IOVDD 1.8V
	gpio_tlmm_config(GPIO_CFG(128, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(128, 1);
	mdelay(10);
	
	//AVDD 2.8V
	gpio_tlmm_config(GPIO_CFG(48, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(48, 1);
	mdelay(10);

	//AF
	gpio_tlmm_config(GPIO_CFG(23, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(23, 1);

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		{
		
		data->sensor_platform_info->ext_power_ctrl(1);
		}

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

#else
	printk("%s : E\n",__func__);

	memset(mt9e013_lgit_ctrl, 0x00, sizeof(*mt9e013_lgit_ctrl));
	s_ctrl->fps_divider = 1 * 0x00000400;
//No on JB	mt9e013_lgit_ctrl->pict_fps_divider = 1 * 0x00000400;
//No on JB	mt9e013_lgit_ctrl->set_test = TEST_OFF;
//	mt9e013_lgit_ctrl->prev_res = QTR_SIZE;
	s_ctrl->curr_res = MSM_SENSOR_RES_QTR /* QTR_SIZE */;
//No on JB	mt9e013_lgit_ctrl->pict_res = FULL_SIZE;	
	mt9e013_lgit_init_focus(s_ctrl);
	return msm_sensor_power_up(s_ctrl);
#endif	
}

static int32_t mt9e013_lgit_power_down(struct msm_sensor_ctrl_t * s_ctrl)
{
#if 1
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	int32_t rc = 0;

	printk("%s : E\n", __func__);
	mt9e013_lgit_af_power_down(s_ctrl);

	/* RESET LOW*/
	if (data->sensor_reset_enable) {
		rc = gpio_direction_output(data->sensor_reset, 0);
		if (rc < 0)
			pr_err("%s: gpio:CAM_RESET %d direction can't be set\n", __func__, data->sensor_reset);
		gpio_free(data->sensor_reset);
	}

	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);
	
	msm_camera_config_gpio_table(data, 0);
	msm_camera_request_gpio_table(data, 0);

	//AVDD 2.8V
	gpio_tlmm_config(GPIO_CFG(48, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(48, 0);
	mdelay(10);
	
	//DVDD 1.2V
	gpio_tlmm_config(GPIO_CFG(49, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(49, 0);
	mdelay(10);
	
	//IOVDD 1.8V
	gpio_tlmm_config(GPIO_CFG(128, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(128, 0);
	mdelay(10);

	//AF
	gpio_tlmm_config(GPIO_CFG(23, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(23, 0);

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(0);
	
	kfree(s_ctrl->reg_ptr);
	printk("%s : X\n", __func__);
	return 0;
#else
	printk("%s : E",__func__);
	mt9e013_lgit_af_power_down(s_ctrl);
	return msm_sensor_power_down(s_ctrl);
#endif	
}

static int32_t mt9e013_lgit_config(struct msm_sensor_ctrl_t *s_ctrl, void __user *argp)
{
	struct sensor_cfg_data cdata;
	long   rc = 0;
	if (copy_from_user(&cdata,
		(void *)argp,
		sizeof(struct sensor_cfg_data)))
		return -EFAULT;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	printk("%s: cfgtype = %d\n", __func__, cdata.cfgtype);
	switch (cdata.cfgtype) {
		case CFG_SET_EFFECT:
			rc = mt9e013_lgit_set_effect(s_ctrl,
					cdata.cfg.effect);
			break;

		case CFG_GET_CALIB_DATA:
			rc = mt9e013_lgit_read_eeprom_data(s_ctrl,
				&cdata.cfg.calib_info);
			
			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;					
			break;

		case CFG_MOVE_FOCUS:
			rc = mt9e013_lgit_move_focus(s_ctrl, 
				cdata.cfg.focus.dir, 
				cdata.cfg.focus.steps);			
			break;

		case CFG_SET_DEFAULT_FOCUS:		
			rc = mt9e013_lgit_set_default_focus(s_ctrl, 
				cdata.cfg.focus.steps);			
			break;	
			
		case CFG_GET_PICT_FPS:
			rc = mt9e013_lgit_get_pict_fps(s_ctrl,
				cdata.cfg.gfps.prevfps,
				&cdata.cfg.gfps.pictfps);

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;			
			break;			

		case CFG_GET_PREV_L_PF:
			rc = mt9e013_lgit_get_prev_lines_pf(s_ctrl,
				&cdata.cfg.prevl_pf);

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;				
			break;

		case CFG_GET_PREV_P_PL:
			rc = mt9e013_lgit_get_prev_pixels_pl(s_ctrl,
				&cdata.cfg.prevp_pl);

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;						
			break;

		case CFG_GET_PICT_L_PF:
			rc = mt9e013_lgit_get_pict_lines_pf(s_ctrl,
				&cdata.cfg.pictl_pf);

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;						
			break;

		case CFG_GET_PICT_P_PL:
			rc = mt9e013_lgit_get_pict_pixels_pl(s_ctrl,
				&cdata.cfg.pictp_pl);

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;				
			break;

		case CFG_GET_PICT_MAX_EXP_LC:
			rc = mt9e013_lgit_get_pict_max_exp_lc(s_ctrl,
				&cdata.cfg.pict_max_exp_lc);

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;					
			break;

		case CFG_GET_AF_MAX_STEPS:
			rc = mt9e013_lgit_get_af_max_steps(s_ctrl,
				&cdata.max_steps);

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;					
			break;

		default:
			rc = -EINVAL;
			break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	if (rc == -EINVAL) {
		rc = msm_sensor_config(s_ctrl, argp);
	}
	
	return rc;
}

static int32_t mt9e013_lgit_actuator_move_focus(
	struct msm_actuator_ctrl_t *a_ctrl,
	struct msm_actuator_move_params_t *move_params)
{
	int direction = move_params->dir;
	int32_t num_steps = move_params->num_steps;
	int16_t step_direction, dest_lens_position, dest_step_position;

	printk("[CAMERA]mt9e013_lgit_move_focus : direction = %d,num_steps = %d \n", direction, num_steps);

	if (direction == MOVE_NEAR)
		step_direction = 1;
	else
		step_direction = -1;

	dest_step_position = mt9e013_lgit_ctrl->curr_step_pos
						+ (step_direction * num_steps);

	if (dest_step_position < 0)
		dest_step_position = 0;
	else if (dest_step_position > mt9e013_lgit_linear_total_step)
		dest_step_position = mt9e013_lgit_linear_total_step;

	if (dest_step_position == mt9e013_lgit_ctrl->curr_step_pos)
		return 0;
	printk("[CAMERA]__debug:MoveFocus, dest_step_position:%d \n", dest_step_position);
	dest_lens_position = mt9e013_lgit_step_position_table[dest_step_position];

	if ((dest_step_position <= 4) && (step_direction == 1)) {
		msm_camera_i2c_write(&a_ctrl->i2c_client,
			REG_VCM_STEP_TIME, 0x0000, MSM_CAMERA_I2C_WORD_DATA);	
		if (num_steps == 4) {
			printk("[CAMERA]__debug:MoveFocus, jumpvalue:%d \n", mt9e013_lgit_nl_region_boundary1 * mt9e013_lgit_nl_region_code_per_step1);
			msm_camera_i2c_write(&a_ctrl->i2c_client,
				REG_VCM_NEW_CODE, mt9e013_lgit_nl_region_boundary1 * mt9e013_lgit_nl_region_code_per_step1, MSM_CAMERA_I2C_WORD_DATA);				
		} else {
			if (dest_step_position <= mt9e013_lgit_nl_region_boundary1) {
				printk("[CAMERA]__debug:MoveFocus, fine search:%d \n", dest_lens_position);
				msm_camera_i2c_write(&a_ctrl->i2c_client,
					REG_VCM_NEW_CODE, dest_lens_position, MSM_CAMERA_I2C_WORD_DATA);					
				mt9e013_lgit_ctrl->curr_lens_pos = dest_lens_position;
				mt9e013_lgit_ctrl->curr_step_pos = dest_step_position;
				a_ctrl->curr_step_pos = mt9e013_lgit_ctrl->curr_step_pos;
				return 0;
			}
		}
	}

	if(step_direction < 0) {
		if(num_steps > 20) {
			/*macro to infinity*/
			mt9e013_lgit_vcm_step_time = 0x0050;
			mt9e013_lgit_sw_damping_time_wait = 5;
		} else if (num_steps <= 4) {
			/*reverse search fine step	dir - macro to infinity*/
			mt9e013_lgit_vcm_step_time = 0x0400;
			mt9e013_lgit_sw_damping_time_wait = 4;
		} else {
			/*reverse search Coarse Jump ( > 4) dir - macro to infinity*/
			mt9e013_lgit_vcm_step_time = 0x96;
			mt9e013_lgit_sw_damping_time_wait = 3;
			}
	} else {
		if(num_steps >= 4) {
			/*coarse jump  dir - infinity to macro*/
			mt9e013_lgit_vcm_step_time = 0x0200;
			mt9e013_lgit_sw_damping_time_wait = 2;
		} else {
			/*fine step  dir - infinity to macro*/
			mt9e013_lgit_vcm_step_time = 0x0400;
			mt9e013_lgit_sw_damping_time_wait = 4;
		}
	}

	msm_camera_i2c_write(&a_ctrl->i2c_client,
		REG_VCM_STEP_TIME, mt9e013_lgit_vcm_step_time, MSM_CAMERA_I2C_WORD_DATA);	
	
	printk("[CAMERA]__debug:MoveFocus, mt9e013_lgit_vcm_step_time:%d \n", mt9e013_lgit_vcm_step_time);
	printk("[CAMERA]__debug:MoveFocus, DestLensPosition:%d \n", dest_lens_position);
	if (mt9e013_lgit_ctrl->curr_lens_pos != dest_lens_position) {
		msm_camera_i2c_write(&a_ctrl->i2c_client,
			REG_VCM_NEW_CODE, dest_lens_position, MSM_CAMERA_I2C_WORD_DATA);			
		usleep(mt9e013_lgit_sw_damping_time_wait * 1000);
	}
	mt9e013_lgit_ctrl->curr_lens_pos = dest_lens_position;
	mt9e013_lgit_ctrl->curr_step_pos = dest_step_position;
	a_ctrl->curr_step_pos = mt9e013_lgit_ctrl->curr_step_pos;
	
	return 0;
}


static int32_t mt9e013_lgit_actuator_set_default_focus(
	struct msm_actuator_ctrl_t *a_ctrl,
	struct msm_actuator_move_params_t *move_params)
{
	int32_t rc = 0;

	if (mt9e013_lgit_ctrl->curr_step_pos != 0) {
		move_params->dir = MOVE_FAR;
		move_params->num_steps = mt9e013_lgit_ctrl->curr_step_pos;
		rc = mt9e013_lgit_actuator_move_focus(a_ctrl,
				move_params);
	} else {
	
		msm_camera_i2c_write(&a_ctrl->i2c_client,
			REG_VCM_NEW_CODE, 0x00, MSM_CAMERA_I2C_WORD_DATA);			
	}

	mt9e013_lgit_ctrl->curr_lens_pos = 0;
	mt9e013_lgit_ctrl->curr_step_pos = 0;

	a_ctrl->curr_step_pos = mt9e013_lgit_ctrl->curr_step_pos;

	return rc;
}

#ifdef CONFIG_MT9E013_LGIT_ACT
struct msm_actuator msm_actuator_table_mt9e013_lgit = {
	.act_type = ACTUATOR_VCM,
	.func_tbl = {
		.actuator_init_step_table = NULL,
		.actuator_move_focus = mt9e013_lgit_actuator_move_focus,
		.actuator_write_focus = NULL,
		.actuator_set_default_focus = mt9e013_lgit_actuator_set_default_focus,
		.actuator_init_focus = NULL,
		.actuator_i2c_write = NULL,
	},
};
#endif

static void mt9e013_lgit_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	printk("%s : E\n",__func__);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x8250, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
		0x301A, 0x8650, MSM_CAMERA_I2C_WORD_DATA);  
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x8658, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x0104, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x065C, MSM_CAMERA_I2C_WORD_DATA);
}

static void mt9e013_lgit_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	printk("%s : E\n",__func__);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x0058, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x0050, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x0104, 0x01, MSM_CAMERA_I2C_BYTE_DATA);
}

static const struct i2c_device_id mt9e013_lgit_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9e013_lgit_s_ctrl},
	{ }
};

static struct i2c_driver mt9e013_lgit_i2c_driver = {
	.id_table = mt9e013_lgit_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9e013_lgit_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&mt9e013_lgit_i2c_driver);
}

static struct v4l2_subdev_core_ops mt9e013_lgit_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9e013_lgit_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9e013_lgit_subdev_ops = {
	.core = &mt9e013_lgit_subdev_core_ops,
	.video  = &mt9e013_lgit_subdev_video_ops,
};

static struct msm_sensor_fn_t mt9e013_lgit_func_tbl = {
	.sensor_start_stream = mt9e013_lgit_start_stream,
	.sensor_stop_stream = mt9e013_lgit_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = mt9e013_lgit_set_fps,
	.sensor_write_exp_gain = mt9e013_lgit_write_exp_gain,
	.sensor_write_snapshot_exp_gain = mt9e013_lgit_write_exp_snapshot_gain,
	.sensor_csi_setting = msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = mt9e013_lgit_config,
	.sensor_power_up = mt9e013_lgit_power_up,
	.sensor_power_down = mt9e013_lgit_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t mt9e013_lgit_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.group_hold_on_conf = mt9e013_lgit_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(mt9e013_lgit_groupon_settings),
	.group_hold_off_conf = mt9e013_lgit_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(mt9e013_lgit_groupoff_settings),
	.init_settings = &mt9e013_lgit_init_conf[0],
	.init_size = ARRAY_SIZE(mt9e013_lgit_init_conf),
	.mode_settings = &mt9e013_lgit_confs[0],
	.output_settings = &mt9e013_lgit_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9e013_lgit_confs),
};

static struct msm_sensor_ctrl_t mt9e013_lgit_s_ctrl = {
	.msm_sensor_reg = &mt9e013_lgit_regs,
	.sensor_i2c_client = &mt9e013_lgit_sensor_i2c_client,
	.sensor_i2c_addr = 0x6C,
	.sensor_output_reg_addr = &mt9e013_lgit_reg_addr,
	.sensor_id_info = &mt9e013_lgit_id_info,
	.sensor_exp_gain_info = &mt9e013_lgit_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &mt9e013_lgit_csi_params_array[0],
	.msm_sensor_mutex = &mt9e013_lgit_mut,
	.sensor_i2c_driver = &mt9e013_lgit_i2c_driver,
	.sensor_v4l2_subdev_info = mt9e013_lgit_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9e013_lgit_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9e013_lgit_subdev_ops,
	.func_tbl = &mt9e013_lgit_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");


