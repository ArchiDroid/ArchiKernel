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
#define SENSOR_NAME "hi707"
#define PLATFORM_DRIVER_NAME "msm_camera_hi707"
#define hi707_obj hi707_##obj

#define SENSOR_REG_PAGE_ADDR 0x03
#define SENSOR_REG_PAGE_0 0x00
#define SENSOR_REG_PAGE_20 0x20

#define SENSOR_PREVIEW_WIDTH 640
#define SENSOR_PREVIEW_HEIGHT 480

//#define LGE_CAMERA_ANTIBAND_50HZ // @kernel/drivers/media/video/msm/sensors/Makefile

DEFINE_MUTEX(hi707_mut);
static struct msm_sensor_ctrl_t hi707_s_ctrl;

static int PREV_EFFECT = -1;
static int PREV_EXPOSURE = -1;
static int PREV_WB = -1;
static int PREV_FPS = -1;
static int PREV_NIGHT_MODE = -1;

typedef enum {
  HI707_60HZ,
  HI707_50HZ,
  HI707_HZ_MAX_NUM,
} HI707AntibandingType;

static int hi707_antibanding = HI707_50HZ;

static struct msm_camera_i2c_reg_conf hi707_start_settings[] = {
// not need
};

static struct msm_camera_i2c_reg_conf hi707_stop_settings[] = {
// not need
};

static struct msm_camera_i2c_reg_conf hi707_prev_settings[] = {
};

static struct msm_camera_i2c_reg_conf hi707_recommend_settings[HI707_HZ_MAX_NUM][436] = {
	////////////////////////////////////////////
	// 
	// Hi-707 
	//
	// 704 base + 706 pulse timing + mipi_async_fifo + EXPFIX
	// Frame rate: 7.5~30fps
	// Flicker : Auto 60Hz
	
	//QVGA	: DISP_W/H, preview on, mipidiv 1/2, winrow, MIPI(LPWC,CLK_POST_TIME)
	//QQVGA : DISP_W/H, preview2 on, mipidiv 1/2, winr/c, MIPI(HS_TRAIL_TIME,LPWC,CLK_POST_TIME)
	//BAYER : DISP_FORMAT, STMIPID02(0x17~18), mipidiv1/4, HBLANK, VSYNC,
	//2012.09.25
	//LGE SKT VT 
	//[SENSOR_INITIALIZATION]
	//DISP_DATE = "2012-09-22"
	//DISP_NAME = "STEVE_init"
	//DISP_WIDTH = 640
	//DISP_HEIGHT = 480
	//DISP_FORMAT = YUV422_MIPI
	//DISP_DATAORDER = YUYV
	//MCLK = 24.00
	//PLL = 1.00

	//BEGIN

	////////////////////////////////////////////
	// Hi-707 Setting
	////////////////////////////////////////////

	//I2C_ID = 0x60
	//I2C_BYTE  = 0x11
	// 60Hz
	{
		{0x01, 0x71},  // reset op.
		{0x01, 0x73},
		{0x01, 0x71},
		
		{0x03, 0x22},	//AWB off
		{0x10, 0x7b},
		{0x03, 0x20},	//AE off
		{0x10, 0x0c}, 
		
		///////////////////////////// Page 0	-  Image Size & Sync.
		{0x03, 0x00}, 
		{0x08, 0x0f}, //Parallel NO Output_PAD Out									   
		{0x10, 0x00},	//VDOCTL1 [5:4]subsample:1,1/2,1/4, [0]preview_en
	/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
		{0x11, 0x93},	// xy flip //VDOCTL2 , 90 : FFR off, 94 : FFR on
#else
		{0x11, 0x90},	// no flip //VDOCTL2 , 90 : FFR off, 94 : FFR on
#endif
	/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
		{0x12, 0x00},	//CLK_CTL
		{0x14, 0x88},	//[7]fix_frm_spd:prevent hounting, [3]fix_frm_opt:inc. exp.time
		
		{0x0b, 0xaa}, 
		{0x0c, 0xaa}, 
		{0x0d, 0xaa}, 

		{0xc0, 0x95}, 
		{0xc1, 0x18}, 
		{0xc2, 0x91}, 
		{0xc3, 0x00}, 
		{0xc4, 0x01}, 

		{0x03, 0x20},  //page 20
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
		{0x10, 0x1c},  //ae off
#else
		{0x10, 0x0c},  //ae off
#endif
		{0x03, 0x22},  //page 22
		{0x10, 0x7b},  //awb off

		{0x03, 0x00}, 
		{0x12, 0x00}, 
		{0x20, 0x00},	//WINROW
		{0x21, 0x04}, //06},	// - VGA:04, QVGA,QQVGA:02
		{0x22, 0x00},	//WINCOL
		{0x23, 0x04}, //06},	// - VGA,QVGA:04, QQVGA:02
		{0x40, 0x00},	//HBLANK 
		{0x41, 0x90},	// - YUV422:0090, BAYER:0158
		{0x42, 0x00},	//VSYNCH
		{0x43, 0x02},	// - YUV422:0002, BAYER:0014
		
		{0x80, 0x2e},  //don't touch
		{0x81, 0x7e},  //don't touch
		{0x82, 0x90},  //don't touch
		{0x83, 0x30},  //don't touch
		{0x84, 0x2c},  //don't touch
		{0x85, 0x4b},  //don't touch
		{0x86, 0x01},  //don't touch
		{0x88, 0x47},  //don't touch
		{0x89, 0x48}, //BLC hold
		{0x90, 0x0b}, //TIME_IN  12/100  _100318
		{0x91, 0x0b}, //TIME_OUT 12/100  _100318
		{0x92, 0x48}, //AG_IN
		{0x93, 0x48}, //AG_OUT
		{0x98, 0x38},
		{0x99, 0x00}, //41}, //43 40}, //Out BLC LHC
		{0xa0, 0x02}, //00}, //Dark BLC
		{0xa8, 0x42}, //40}, //Normal BLC LHC
		
		{0xc0, 0x95},	//PLL Mode									  
		{0xc1, 0x18},									  
		{0xc2, 0x91},	//[4]plldiv_en, [3:2]mipi4xclkdiv:bypass,1/2,1/4,1/8, [0]ispclkdiv:1/2,1/4
		{0xc3, 0x00},									  
		{0xc4, 0x01},	
		
		///////////////////////////// Page 2	-  Analog Circuit Control
		{0x03, 0x02},
		{0x10, 0x00},	//MODE_TEST
		{0x11, 0x00},	//MODE_DEAD_TEST
		{0x13, 0x40},	//MODE_ANA_TEST
		{0x14, 0x04},	//MODE_MEMORY
		
		{0x18, 0x1c},	//Analog mode
		{0x19, 0x00},	//[0]pmos_off
		{0x1a, 0x00}, 
		{0x1b, 0x08},
		
		{0x1c, 0x9c},	//DC-DC
		{0x1d, 0x03},
		
		{0x20, 0x33},	//PX bias
		{0x21, 0x77},	//ADC/ASP bias
		{0x22, 0xa7},	//Main bias
		{0x23, 0x32},	//Clamp 
		
		{0x24, 0x33},
		{0x2b, 0x40},	//Fixed frame counter end
		{0x2d, 0x32},	//Fixed frame counter start


		{0x31, 0x99},	//shared control
		{0x32, 0x00},
		{0x33, 0x00},
		{0x34, 0x3c},
		{0x35, 0x0d},
		{0x3b, 0x80},  //SF 60
		              //timing control 1
		{0x50, 0x21},
		{0x51, 0x1C},
		{0x52, 0xaa},
		{0x53, 0x5a},
		{0x54, 0x30},
		{0x55, 0x10},
		{0x56, 0x0c},
		{0x58, 0x00}, 
		{0x59, 0x0F},
						  //Row Timing Control
		{0x60, 0x34},	//addr_en - Exp.
		{0x61, 0x3a},
		{0x62, 0x34},	//rx1
		{0x63, 0x39},
		{0x64, 0x34},	//rx2
		{0x65, 0x39},
		{0x72, 0x35},	//tx1
		{0x73, 0x38},
		{0x74, 0x35},	//tx2
		{0x75, 0x38},
		{0x80, 0x02},	//addr_en - Read.
		{0x81, 0x2e},
		{0x82, 0x0d},	//rx1
		{0x83, 0x10},
		{0x84, 0x0d},	//rx2
		{0x85, 0x10},
		{0x92, 0x1d},	//tx1
		{0x93, 0x20},
		{0x94, 0x1d},	//tx2
		{0x95, 0x20},
		{0xa0, 0x03},	//sx
		{0xa1, 0x2d},
		{0xa4, 0x2d},	//sxb
		{0xa5, 0x03},
		{0xa8, 0x12},	//wrst
		{0xa9, 0x1b},
		{0xaa, 0x22},	//wsig
		{0xab, 0x2b},
		{0xac, 0x10},	//rx_off_rst
		{0xad, 0x0e},	//tx_off_rst
		{0xb8, 0x33},	//rx pwr - exp.
		{0xb9, 0x35},
		{0xbc, 0x0c},	//rx pwr - read
		{0xbd, 0x0e},
		{0xc0, 0x3a},	//addr_en1 - Fixed Exp.
		{0xc1, 0x3f},
		{0xc2, 0x3a},	//addr_en2
		{0xc3, 0x3f},
		{0xc4, 0x3a},	//sx1
		{0xc5, 0x3e},
		{0xc6, 0x3a},	//sx2
		{0xc7, 0x3e},
		{0xc8, 0x3a},	//rx1
		{0xc9, 0x3e},
		{0xca, 0x3a},	//rx2
		{0xcb, 0x3e},
		{0xcc, 0x3b},	//tx1
		{0xcd, 0x3d},
		{0xce, 0x3b},	//tx2
		{0xcf, 0x3d},
		
		{0xd0, 0x33},	//Exposure domain valid
		{0xd1, 0x3f},


		{0x03, 0x10}, //Page 10 - Format, Image Effect
		{0x10, 0x03}, //ISPCTL - [7:4]0:YUV322, 6:BAYER, [1:0]VYUY, UYVY, YVYU, YUYV
		{0x11, 0x43}, // - [0x1010:1011]YUV422:0343, BAYER:6000
		{0x12, 0x30}, //Y offet, dy offseet enable          
		{0x40, 0x00}, //
		{0x41, 0x00}, //DYOFS  00->10-> 00  STEVE_130110(black scene face saturation)
		{0x48, 0x80}, //Contrast  88->84  _100318
		{0x50, 0xe0}, //AGBRT
		{0x60, 0x0b}, //         
		{0x61, 0x00}, //default
		{0x62, 0x78}, //SATB  (1.4x)
		{0x63, 0x78}, //SATR  (1.2x)
		{0x64, 0xa0}, //AGSAT
		{0x66, 0x90}, //wht_th2
		{0x67, 0x36}, //wht_gain  Dark (0.4x), Normal (0.75x)

		///////////////////////////// Page 11	- D,Z,W-LPF
		{0x03, 0x11},
		{0x10, 0x25}, //LPF_CTL1 //0x01
		{0x11, 0x07}, //1f},	//Test Setting
		{0x20, 0x00}, //LPF_AUTO_CTL
		{0x21, 0x60}, //38},	//LPF_PGA_TH
		{0x23, 0x0a}, //LPF_TIME_TH
		{0x60, 0x13}, //ZARA_SIGMA_TH //40->10
		{0x61, 0x85},
		{0x62, 0x00},	//ZARA_HLVL_CTL
		{0x63, 0x00}, //83},//ZARA_LLVL_CTL
		{0x64, 0x00}, //83},//ZARA_DY_CTL
		{0x67, 0x70}, //60},//70}, //F0},	//Dark
		{0x68, 0x24}, //24},//30},	//Middle
		{0x69, 0x04}, //10},//High
		
		{0x03, 0x12}, //Page 12 - 2D : YC1D,YC2D,DPC,Demosaic																				  
		{0x40, 0xd3}, //d6}, //d7},//YC2D_LPF_CTL1 //bc 															
		{0x41, 0x09},	//YC2D_LPF_CTL2 																 
		{0x50, 0x18}, //10}, //18}, //Test Setting																	 
		{0x51, 0x24},	//Test Setting																	 
		{0x70, 0x1f},	//GBGR_CTL1 //0x1f																 
		{0x71, 0x00},	//Test Setting																	 
		{0x72, 0x00},	//Test Setting																	 
		{0x73, 0x00},	//Test Setting																	 
		{0x74, 0x12},	//GBGR_G_UNIT_TH//12															 
		{0x75, 0x12},	//GBGR_RB_UNIT_TH//12															 
		{0x76, 0x20},	//GBGR_EDGE_TH																	 
		{0x77, 0x80},	//GBGR_HLVL_TH																	 
		{0x78, 0x88},	//GBGR_HLVL_COMP																 
		{0x79, 0x18},	//Test Setting 
		
		{0x90, 0x3d},																				   
		{0x91, 0x34},																				   
		{0x99, 0x28},																				   
		{0x9c, 0x05}, //14 For defect																   
		{0x9d, 0x08}, //15 For defect																   
		{0x9e, 0x28},																				   
		{0x9f, 0x28},																				   
																									   
		{0xb0, 0x7d}, //75 White Defect 															   
		{0xb5, 0x44},																				   
		{0xb6, 0x82},																				   
		{0xb7, 0x52},																				   
		{0xb8, 0x44},																				   
		{0xb9, 0x15},
				
		{0x03, 0x13}, //Page 13 - Sharpness
		{0x10, 0x01},	
		{0x11, 0x89}, //89},	
		{0x12, 0x14},	
		{0x13, 0x19},	
		{0x14, 0x08},	//Test Setting
		{0x20, 0x03},	//SHARP_Negative
		{0x21, 0x04}, //03},	//SHARP_Positive
		{0x23, 0x25},	//SHARP_DY_CTL
		{0x24, 0x21},	//40->33
		{0x25, 0x08},	//SHARP_PGA_TH
		{0x26, 0x40},	//Test Setting
		{0x27, 0x00},	//Test Setting
		{0x28, 0x08},	//Test Setting
		{0x29, 0x50},	//AG_TH
		{0x2a, 0xe0},	//region ratio
		{0x2b, 0x10},	//Test Setting
		{0x2c, 0x28},	//Test Setting
		{0x2d, 0x40},	//Test Setting
		{0x2e, 0x00},	//Test Setting
		{0x2f, 0x00},	//Test Setting
		{0x30, 0x11},	//Test Setting
		{0x80, 0x05},	//SHARP2D_CTL
		{0x81, 0x07},	//Test Setting
		{0x90, 0x04},	//SHARP2D_SLOPE
		{0x91, 0x05},	//SHARP2D_DIFF_CTL
		{0x92, 0x00},	//SHARP2D_HI_CLIP
		{0x93, 0x30},	//SHARP2D_DY_CTL
		{0x94, 0x30},	//Test Setting
		{0x95, 0x10},	//Test Setting 
		
		{0x03, 0x14}, //Page 14 - Lens Shading Correction
		{0x10, 0x01},
		{0x20, 0x80}, //60},   //XCEN LHC
		{0x21, 0x80}, //YCEN
		{0x22, 0x88}, //7b}, //6a}, //50},
		{0x23, 0x5c}, //50}, //44}, //40},
		{0x24, 0x49}, //44}, //32}, //3d},
		
		//15page//////////////////////////
		{0x03, 0x15}, 
		{0x10, 0x03},
		{0x14, 0x52},	//CMCOFSGM 
		{0x16, 0x3a},	//CMCOFSGL
		{0x17, 0x2f},	//CMC SIGN

		//CMC
		{0x30, 0xf1},
		{0x31, 0x71},
		{0x32, 0x00},
		{0x33, 0x1f},
		{0x34, 0xe1},
		{0x35, 0x42},
		{0x36, 0x01},
		{0x37, 0x31},
		{0x38, 0x72},
		//CMC OFS
		{0x40, 0x90},
		{0x41, 0x82},
		{0x42, 0x12},
		{0x43, 0x86},
		{0x44, 0x92},
		{0x45, 0x18},
		{0x46, 0x84},
		{0x47, 0x02},
		{0x48, 0x02},
		
		
		{0x03,0x16}, //gamma   0.44 //S_32																
		{0x30,0x00},																				   
		{0x31,0x08}, //0x0d //0e																		
		{0x32,0x1c}, //(0216_kjh_0x1b-->1c) 0x18 //1a													
		{0x33,0x2f}, //0x2c //2f																		
		{0x34,0x53},																				   
		{0x35,0x76},																				   
		{0x36,0x93},																				   
		{0x37,0xac},																				   
		{0x38,0xc0},																				   
		{0x39,0xd0},																				   
		{0x3a,0xdc},																				   
		{0x3b,0xed},																				   
		{0x3c,0xf4}, //f7																			   
		{0x3d,0xf6}, //fc																			   
		{0x3e,0xfa}, //ff	
			   
		{0x03, 0x17},
		{0xc0, 0x01},
		
		///////////////////////////// Page 20	- Auto Exposure 
		{0x03, 0x20},
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
		{0x10, 0x1c},	//AECTL
#else
		{0x10, 0x0c},	//AECTL
#endif
		{0x11, 0x04},
		{0x18, 0x30},
		{0x20, 0x01},	//FrameCTL
		{0x28, 0x27},	//FineCTL
		{0x29, 0xa1},
		{0x2a, 0xf0},
		{0x2b, 0x34},
			   
		{0x30, 0xf8},	//AE_Escape_CTL
		{0x39, 0x22},
		{0x3a, 0xde},
		{0x3b, 0x23},
		{0x3c, 0xde},
		
		{0x60, 0x70}, //0x71}, //51}, //70 11																			 
		{0x61, 0x00}, //0x11}, //12}, //12 11																							   
		{0x62, 0x70}, //0x71}, //51}, //70 31																				 
		{0x63, 0x00}, //0x11}, //12}, //20}, //20 11  

		{0x68, 0x41}, //AE_CEN
		{0x69, 0x81},
		{0x6A, 0x38},
		{0x6B, 0xb8},

		{0x70, 0x34},//Y Targe 32
		{0x76, 0x88}, //22}, // Unlock bnd1
		{0x77, 0xfe}, //02}, // Unlock bnd2
		{0x78, 0x22}, //12}, // Yth 1
		{0x79, 0x26}, //Yth 2
		{0x7a, 0x23}, //Yth 3
		{0x7c, 0x1c}, //Yth 2
		{0x7d, 0x22}, //Yth 4
		
		{0x98, 0x8c}, //BRIGHT_MEASURE_TH
		{0x99, 0x23},
		
		{0xa0, 0x02}, //EXP_MAX100
		{0xa1, 0x49},
		{0xa2, 0xf0},
		
		{0xb0, 0x1d},
		{0xb1, 0x14}, //14
		{0xb2, 0xa0}, //80
		{0xb3, 0x17}, //AGLVL //17
		{0xb4, 0x17},
		{0xb5, 0x3e},
		{0xb6, 0x2b},
		{0xb7, 0x24},
		{0xb8, 0x21},
		{0xb9, 0x1f},
		{0xba, 0x1e},
		{0xbb, 0x1d},
		{0xbc, 0x1c},
		{0xbd, 0x1b},
		
		{0xc0, 0x1a}, //PGA_sky
		{0xc3, 0x48}, //PGA_dark_on
		{0xc4, 0x48}, //PGA_dark_off
		
		{0x03, 0x22}, //Page 22 AWB
		{0x10, 0xe2},
		{0x11, 0x2E}, //26},
		{0x20, 0x41}, //01 //69 
		{0x21, 0x40},
		{0x30, 0x80},
		{0x31, 0x80},
		{0x38, 0x12},
		{0x39, 0x33},
		{0x40, 0x93}, //f0},
		//{0x03, 0x22},

		// STEVE Yellowish	
		{0x41, 0x54},//0x44}, //0x54},
		{0x42, 0x33},//0x22}, //0x33},
		{0x43, 0xf3},//0x93}, //0xf3},
		{0x44, 0x55},//0x55}, //0x55},
		{0x45, 0x44},//0x44}, //0x44},
		{0x46, 0x02},//0x08}, //0x02},

		{0x80, 0x3d}, // R
		{0x81, 0x20}, // G
		{0x82, 0x40}, // B

		{0x83, 0x5a}, //52}, //RMAX
		{0x84, 0x20}, //1d}, //RMIN
		{0x85, 0x53}, //BMAX 5a
		{0x86, 0x24}, //BMIN 

		// STEVE Yellowish		
		{0x87, 0x4a}, //42
		{0x88, 0x3c},
		{0x89, 0x3e},
		{0x8a, 0x34},

		{0x8b, 0x02}, //0x08}, //02}, //OUT TH
		{0x8d, 0x3a}, //0x11}, //22},
		{0x8e, 0xb3}, //0x11}, //71}, 

		{0x8f, 0x65}, //0x63},
		{0x90, 0x61}, //0x60},
		{0x91, 0x5C}, //0x5c},
		{0x92, 0x56}, //0x56},
		{0x93, 0x4E}, //0x52},
		{0x94, 0x43}, //0x4c},
		{0x95, 0x3A}, //0x3e}, //36},
		{0x96, 0x32}, //0x2f}, //31},
		{0x97, 0x2A}, //0x28}, //2e},
		{0x98, 0x24}, //0x23}, //2a},
		{0x99, 0x20}, //0x21}, //29},
		{0x9a, 0x1C}, //0x20}, //26},
		{0x9b, 0x0b}, //0x07}, //09},
		
		////////////////////////////clock calculator//////////////////////////////
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
		{0x03, 0x00}, //Page 0
		{0x40, 0x00}, //Hblank 144
		{0x41, 0x90}, 
		{0x42, 0x00}, //Vblank 4
		{0x43, 0x04}, 

		{0x03, 0x20}, //Page 20
		{0x83, 0x00}, //EXP Normal 33.33 fps 
		{0x84, 0xaf}, 
		{0x85, 0xc8}, 

#else
		{0x03, 0x00}, //Page 0
		{0x40, 0x00}, //Hblank 144
		{0x41, 0x90}, 
		{0x42, 0x00}, //Vblank 4
		{0x43, 0x04}, 

		{0x03, 0x20}, //Page 20
		{0x83, 0x00}, //EXP Normal 30.00 fps 
		{0x84, 0xc3}, 
		{0x85, 0x50}, 

#endif
		{0x86, 0x00}, //EXPMin 7500.00 fps
		{0x87, 0xc8}, 
		{0x88, 0x02}, //EXP Max(120Hz) 10.00 fps 
		{0x89, 0x49}, 
		{0x8a, 0xf0}, 
		{0xa0, 0x02}, //EXP Max(100Hz) 10.00 fps 
		{0xa1, 0x49}, 
		{0xa2, 0xf0}, 
		{0x8B, 0x3a}, //EXP100 
		{0x8C, 0x98}, 
		{0x8D, 0x30}, //EXP120 
		{0x8E, 0xd4}, 
		{0x9c, 0x04}, //EXP Limit 1250.00 fps 
		{0x9d, 0xb0}, 
		{0x9e, 0x00}, //EXP Unit 
		{0x9f, 0xc8}, 

	//AntiBand Unlock
		{0x03, 0x20}, //Page 20 
		{0x2b, 0x34}, 
		{0x30, 0x78}, 

		
		//BLC 
		{0x03, 0x00}, //PAGE 0
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
		{0x90, 0x0a}, //BLC_TIME_TH_ON
		{0x91, 0x0a}, //BLC_TIME_TH_OFF 

#else
		{0x90, 0x0c}, //BLC_TIME_TH_ON
		{0x91, 0x0c}, //BLC_TIME_TH_OFF 

#endif
		{0x92, 0xe8}, //BLC_AG_TH_ON
		{0x93, 0xe0}, //BLC_AG_TH_OFF
		
		/////////////////////////////
		// Page 48 MIPI /////////////
		/////////////////////////////													  
															 
		{0x03, 0x48},																						 
		{0x10, 0x05},									  
		{0x11, 0x00}, //88},																						 
		{0x41, 0x20},									  
		{0x42, 0x00},									  
		{0x3c, 0x00},									  
		{0x3d, 0xfa},									  
															 
		{0x16, 0x88}, //0xc4},									  
		{0x35, 0x02}, //0x03},									  
		{0x1f, 0x04}, //0x06},									  
		{0x1a, 0x00},									  
		{0x34, 0x01}, //0x02},									  
		{0x1c, 0x02},									  
		{0x1d, 0x04},									  
		{0x36, 0x01},									  
		{0x37, 0x03}, 
											
		{0x38, 0x00}, 
		{0x39, 0x4a}, 
															 
		{0x3f, 0x10},									  
		{0x40, 0x00},									  
																												 
		{0x1e, 0x07},									  
		{0x32, 0x06},									  
		{0x20, 0x00},									  
															 
		{0x31, 0x00},									  
		{0x30, 0x05},									  
																								   
		{0x03, 0x22},
		{0x10, 0xfb},
		{0x03, 0x20},
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
		{0x10, 0x9c}, // 100hz first
#else
		{0x10, 0x8c}, // 120hz first
#endif
		{0x03, 0x00},
		{0x01, 0x70},
		
	},
	// 50Hz
	{
		{0x01, 0x71},  // reset op.
		{0x01, 0x73},
		{0x01, 0x71},
		
		{0x03, 0x22},	//AWB off
		{0x10, 0x7b},
		{0x03, 0x20},	//AE off
		{0x10, 0x0c}, 
		
		///////////////////////////// Page 0	-  Image Size & Sync.
		{0x03, 0x00}, 
		{0x08, 0x0f}, //Parallel NO Output_PAD Out									   
		{0x10, 0x00},	//VDOCTL1 [5:4]subsample:1,1/2,1/4, [0]preview_en
	/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
		{0x11, 0x93},	// xy flip //VDOCTL2 , 90 : FFR off, 94 : FFR on
#else
		{0x11, 0x90},	// no flip //VDOCTL2 , 90 : FFR off, 94 : FFR on
#endif
	/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
		{0x12, 0x00},	//CLK_CTL
		{0x14, 0x88},	//[7]fix_frm_spd:prevent hounting, [3]fix_frm_opt:inc. exp.time
		
		{0x0b, 0xaa}, 
		{0x0c, 0xaa}, 
		{0x0d, 0xaa}, 

		{0xc0, 0x95}, 
		{0xc1, 0x18}, 
		{0xc2, 0x91}, 
		{0xc3, 0x00}, 
		{0xc4, 0x01}, 

		{0x03, 0x20},  //page 20
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
		{0x10, 0x1c},  //ae off
#else
		{0x10, 0x0c},  //ae off
#endif
		{0x03, 0x22},  //page 22
		{0x10, 0x7b},  //awb off

		{0x03, 0x00}, 
		{0x12, 0x00}, 
		{0x20, 0x00},	//WINROW
		{0x21, 0x04}, //06},	// - VGA:04, QVGA,QQVGA:02
		{0x22, 0x00},	//WINCOL
		{0x23, 0x04}, //06},	// - VGA,QVGA:04, QQVGA:02
		{0x40, 0x00},	//HBLANK 
		{0x41, 0x90},	// - YUV422:0090, BAYER:0158
		{0x42, 0x00},	//VSYNCH
		{0x43, 0x02},	// - YUV422:0002, BAYER:0014
		
		{0x80, 0x2e},  //don't touch
		{0x81, 0x7e},  //don't touch
		{0x82, 0x90},  //don't touch
		{0x83, 0x30},  //don't touch
		{0x84, 0x2c},  //don't touch
		{0x85, 0x4b},  //don't touch
		{0x86, 0x01},  //don't touch
		{0x88, 0x47},  //don't touch
		{0x89, 0x48}, //BLC hold
		{0x90, 0x0b}, //TIME_IN  12/100  _100318
		{0x91, 0x0b}, //TIME_OUT 12/100  _100318
		{0x92, 0x48}, //AG_IN
		{0x93, 0x48}, //AG_OUT
		{0x98, 0x38},
		{0x99, 0x00}, //41}, //43 40}, //Out BLC LHC
		{0xa0, 0x02}, //00}, //Dark BLC
		{0xa8, 0x42}, //40}, //Normal BLC LHC
		
		{0xc0, 0x95},	//PLL Mode									  
		{0xc1, 0x18},									  
		{0xc2, 0x91},	//[4]plldiv_en, [3:2]mipi4xclkdiv:bypass,1/2,1/4,1/8, [0]ispclkdiv:1/2,1/4
		{0xc3, 0x00},									  
		{0xc4, 0x01},	
		
		///////////////////////////// Page 2	-  Analog Circuit Control
		{0x03, 0x02},
		{0x10, 0x00},	//MODE_TEST
		{0x11, 0x00},	//MODE_DEAD_TEST
		{0x13, 0x40},	//MODE_ANA_TEST
		{0x14, 0x04},	//MODE_MEMORY
		
		{0x18, 0x1c},	//Analog mode
		{0x19, 0x00},	//[0]pmos_off
		{0x1a, 0x00}, 
		{0x1b, 0x08},
		
		{0x1c, 0x9c},	//DC-DC
		{0x1d, 0x03},
		
		{0x20, 0x33},	//PX bias
		{0x21, 0x77},	//ADC/ASP bias
		{0x22, 0xa7},	//Main bias
		{0x23, 0x32},	//Clamp 
		
		{0x24, 0x33},
		{0x2b, 0x40},	//Fixed frame counter end
		{0x2d, 0x32},	//Fixed frame counter start


		{0x31, 0x99},	//shared control
		{0x32, 0x00},
		{0x33, 0x00},
		{0x34, 0x3c},
		{0x35, 0x0d},
		{0x3b, 0x80},  //SF 60
		              //timing control 1
		{0x50, 0x21},
		{0x51, 0x1C},
		{0x52, 0xaa},
		{0x53, 0x5a},
		{0x54, 0x30},
		{0x55, 0x10},
		{0x56, 0x0c},
		{0x58, 0x00}, 
		{0x59, 0x0F},
						  //Row Timing Control
		{0x60, 0x34},	//addr_en - Exp.
		{0x61, 0x3a},
		{0x62, 0x34},	//rx1
		{0x63, 0x39},
		{0x64, 0x34},	//rx2
		{0x65, 0x39},
		{0x72, 0x35},	//tx1
		{0x73, 0x38},
		{0x74, 0x35},	//tx2
		{0x75, 0x38},
		{0x80, 0x02},	//addr_en - Read.
		{0x81, 0x2e},
		{0x82, 0x0d},	//rx1
		{0x83, 0x10},
		{0x84, 0x0d},	//rx2
		{0x85, 0x10},
		{0x92, 0x1d},	//tx1
		{0x93, 0x20},
		{0x94, 0x1d},	//tx2
		{0x95, 0x20},
		{0xa0, 0x03},	//sx
		{0xa1, 0x2d},
		{0xa4, 0x2d},	//sxb
		{0xa5, 0x03},
		{0xa8, 0x12},	//wrst
		{0xa9, 0x1b},
		{0xaa, 0x22},	//wsig
		{0xab, 0x2b},
		{0xac, 0x10},	//rx_off_rst
		{0xad, 0x0e},	//tx_off_rst
		{0xb8, 0x33},	//rx pwr - exp.
		{0xb9, 0x35},
		{0xbc, 0x0c},	//rx pwr - read
		{0xbd, 0x0e},
		{0xc0, 0x3a},	//addr_en1 - Fixed Exp.
		{0xc1, 0x3f},
		{0xc2, 0x3a},	//addr_en2
		{0xc3, 0x3f},
		{0xc4, 0x3a},	//sx1
		{0xc5, 0x3e},
		{0xc6, 0x3a},	//sx2
		{0xc7, 0x3e},
		{0xc8, 0x3a},	//rx1
		{0xc9, 0x3e},
		{0xca, 0x3a},	//rx2
		{0xcb, 0x3e},
		{0xcc, 0x3b},	//tx1
		{0xcd, 0x3d},
		{0xce, 0x3b},	//tx2
		{0xcf, 0x3d},
		
		{0xd0, 0x33},	//Exposure domain valid
		{0xd1, 0x3f},


		{0x03, 0x10}, //Page 10 - Format, Image Effect
		{0x10, 0x03}, //ISPCTL - [7:4]0:YUV322, 6:BAYER, [1:0]VYUY, UYVY, YVYU, YUYV
		{0x11, 0x43}, // - [0x1010:1011]YUV422:0343, BAYER:6000
		{0x12, 0x30}, //Y offet, dy offseet enable          
		{0x40, 0x00}, //
		{0x41, 0x00}, //DYOFS  00->10-> 00  STEVE_130110(black scene face saturation)
		{0x48, 0x80}, //Contrast  88->84  _100318
		{0x50, 0xe0}, //AGBRT
		{0x60, 0x0b}, //         
		{0x61, 0x00}, //default
		{0x62, 0x78}, //SATB  (1.4x)
		{0x63, 0x78}, //SATR  (1.2x)
		{0x64, 0xa0}, //AGSAT
		{0x66, 0x90}, //wht_th2
		{0x67, 0x36}, //wht_gain  Dark (0.4x), Normal (0.75x)

		///////////////////////////// Page 11	- D,Z,W-LPF
		{0x03, 0x11},
		{0x10, 0x25}, //LPF_CTL1 //0x01
		{0x11, 0x07}, //1f},	//Test Setting
		{0x20, 0x00}, //LPF_AUTO_CTL
		{0x21, 0x60}, //38},	//LPF_PGA_TH
		{0x23, 0x0a}, //LPF_TIME_TH
		{0x60, 0x13}, //ZARA_SIGMA_TH //40->10
		{0x61, 0x85},
		{0x62, 0x00},	//ZARA_HLVL_CTL
		{0x63, 0x00}, //83},//ZARA_LLVL_CTL
		{0x64, 0x00}, //83},//ZARA_DY_CTL
		{0x67, 0x70}, //60},//70}, //F0},	//Dark
		{0x68, 0x24}, //24},//30},	//Middle
		{0x69, 0x04}, //10},//High
		
		{0x03, 0x12}, //Page 12 - 2D : YC1D,YC2D,DPC,Demosaic																				  
		{0x40, 0xd3}, //d6}, //d7},//YC2D_LPF_CTL1 //bc 															
		{0x41, 0x09},	//YC2D_LPF_CTL2 																 
		{0x50, 0x18}, //10}, //18}, //Test Setting																	 
		{0x51, 0x24},	//Test Setting																	 
		{0x70, 0x1f},	//GBGR_CTL1 //0x1f																 
		{0x71, 0x00},	//Test Setting																	 
		{0x72, 0x00},	//Test Setting																	 
		{0x73, 0x00},	//Test Setting																	 
		{0x74, 0x12},	//GBGR_G_UNIT_TH//12															 
		{0x75, 0x12},	//GBGR_RB_UNIT_TH//12															 
		{0x76, 0x20},	//GBGR_EDGE_TH																	 
		{0x77, 0x80},	//GBGR_HLVL_TH																	 
		{0x78, 0x88},	//GBGR_HLVL_COMP																 
		{0x79, 0x18},	//Test Setting 
		
		{0x90, 0x3d},																				   
		{0x91, 0x34},																				   
		{0x99, 0x28},																				   
		{0x9c, 0x05}, //14 For defect																   
		{0x9d, 0x08}, //15 For defect																   
		{0x9e, 0x28},																				   
		{0x9f, 0x28},																				   
																									   
		{0xb0, 0x7d}, //75 White Defect 															   
		{0xb5, 0x44},																				   
		{0xb6, 0x82},																				   
		{0xb7, 0x52},																				   
		{0xb8, 0x44},																				   
		{0xb9, 0x15},
				
		{0x03, 0x13}, //Page 13 - Sharpness
		{0x10, 0x01},	
		{0x11, 0x89}, //89},	
		{0x12, 0x14},	
		{0x13, 0x19},	
		{0x14, 0x08},	//Test Setting
		{0x20, 0x03},	//SHARP_Negative
		{0x21, 0x04}, //03},	//SHARP_Positive
		{0x23, 0x25},	//SHARP_DY_CTL
		{0x24, 0x21},	//40->33
		{0x25, 0x08},	//SHARP_PGA_TH
		{0x26, 0x40},	//Test Setting
		{0x27, 0x00},	//Test Setting
		{0x28, 0x08},	//Test Setting
		{0x29, 0x50},	//AG_TH
		{0x2a, 0xe0},	//region ratio
		{0x2b, 0x10},	//Test Setting
		{0x2c, 0x28},	//Test Setting
		{0x2d, 0x40},	//Test Setting
		{0x2e, 0x00},	//Test Setting
		{0x2f, 0x00},	//Test Setting
		{0x30, 0x11},	//Test Setting
		{0x80, 0x05},	//SHARP2D_CTL
		{0x81, 0x07},	//Test Setting
		{0x90, 0x04},	//SHARP2D_SLOPE
		{0x91, 0x05},	//SHARP2D_DIFF_CTL
		{0x92, 0x00},	//SHARP2D_HI_CLIP
		{0x93, 0x30},	//SHARP2D_DY_CTL
		{0x94, 0x30},	//Test Setting
		{0x95, 0x10},	//Test Setting 
		
		{0x03, 0x14}, //Page 14 - Lens Shading Correction
		{0x10, 0x01},
		{0x20, 0x80}, //60},   //XCEN LHC
		{0x21, 0x80}, //YCEN
		{0x22, 0x88}, //7b}, //6a}, //50},
		{0x23, 0x5c}, //50}, //44}, //40},
		{0x24, 0x49}, //44}, //32}, //3d},
		
		//15page//////////////////////////
		{0x03, 0x15}, 
		{0x10, 0x03},
		{0x14, 0x52},	//CMCOFSGM 
		{0x16, 0x3a},	//CMCOFSGL
		{0x17, 0x2f},	//CMC SIGN

		//CMC
		{0x30, 0xf1},
		{0x31, 0x71},
		{0x32, 0x00},
		{0x33, 0x1f},
		{0x34, 0xe1},
		{0x35, 0x42},
		{0x36, 0x01},
		{0x37, 0x31},
		{0x38, 0x72},
		//CMC OFS
		{0x40, 0x90},
		{0x41, 0x82},
		{0x42, 0x12},
		{0x43, 0x86},
		{0x44, 0x92},
		{0x45, 0x18},
		{0x46, 0x84},
		{0x47, 0x02},
		{0x48, 0x02},
		
		
		{0x03,0x16}, //gamma   0.44 //S_32																
		{0x30,0x00},																				   
		{0x31,0x08}, //0x0d //0e																		
		{0x32,0x1c}, //(0216_kjh_0x1b-->1c) 0x18 //1a													
		{0x33,0x2f}, //0x2c //2f																		
		{0x34,0x53},																				   
		{0x35,0x76},																				   
		{0x36,0x93},																				   
		{0x37,0xac},																				   
		{0x38,0xc0},																				   
		{0x39,0xd0},																				   
		{0x3a,0xdc},																				   
		{0x3b,0xed},																				   
		{0x3c,0xf4}, //f7																			   
		{0x3d,0xf6}, //fc																			   
		{0x3e,0xfa}, //ff	
			   
		{0x03, 0x17},
		{0xc0, 0x01},
		
		///////////////////////////// Page 20	- Auto Exposure 
		{0x03, 0x20},
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
		{0x10, 0x1c},	//AECTL
#else
		{0x10, 0x0c},	//AECTL
#endif
		{0x11, 0x04},
		{0x18, 0x30},
		{0x20, 0x01},	//FrameCTL
		{0x28, 0x27},	//FineCTL
		{0x29, 0xa1},
		{0x2a, 0xf0},
		{0x2b, 0x34},
			   
		{0x30, 0xf8},	//AE_Escape_CTL
		{0x39, 0x22},
		{0x3a, 0xde},
		{0x3b, 0x23},
		{0x3c, 0xde},
		
		{0x60, 0x70}, //0x71}, //51}, //70 11																			 
		{0x61, 0x00}, //0x11}, //12}, //12 11																							   
		{0x62, 0x70}, //0x71}, //51}, //70 31																				 
		{0x63, 0x00}, //0x11}, //12}, //20}, //20 11  

		{0x68, 0x41}, //AE_CEN
		{0x69, 0x81},
		{0x6A, 0x38},
		{0x6B, 0xb8},

		{0x70, 0x34},//Y Targe 32
		{0x76, 0x88}, //22}, // Unlock bnd1
		{0x77, 0xfe}, //02}, // Unlock bnd2
		{0x78, 0x22}, //12}, // Yth 1
		{0x79, 0x26}, //Yth 2
		{0x7a, 0x23}, //Yth 3
		{0x7c, 0x1c}, //Yth 2
		{0x7d, 0x22}, //Yth 4
		
		{0x98, 0x8c}, //BRIGHT_MEASURE_TH
		{0x99, 0x23},
		
		{0xa0, 0x02}, //EXP_MAX100
		{0xa1, 0x49},
		{0xa2, 0xf0},
		
		{0xb0, 0x1d},
		{0xb1, 0x14}, //14
		{0xb2, 0xa0}, //80
		{0xb3, 0x17}, //AGLVL //17
		{0xb4, 0x17},
		{0xb5, 0x3e},
		{0xb6, 0x2b},
		{0xb7, 0x24},
		{0xb8, 0x21},
		{0xb9, 0x1f},
		{0xba, 0x1e},
		{0xbb, 0x1d},
		{0xbc, 0x1c},
		{0xbd, 0x1b},
		
		{0xc0, 0x1a}, //PGA_sky
		{0xc3, 0x48}, //PGA_dark_on
		{0xc4, 0x48}, //PGA_dark_off
		
		{0x03, 0x22}, //Page 22 AWB
		{0x10, 0xe2},
		{0x11, 0x2E}, //26},
		{0x20, 0x41}, //01 //69 
		{0x21, 0x40},
		{0x30, 0x80},
		{0x31, 0x80},
		{0x38, 0x12},
		{0x39, 0x33},
		{0x40, 0x93}, //f0},
		//{0x03, 0x22},

		// STEVE Yellowish	
		{0x41, 0x54},//0x44}, //0x54},
		{0x42, 0x33},//0x22}, //0x33},
		{0x43, 0xf3},//0x93}, //0xf3},
		{0x44, 0x55},//0x55}, //0x55},
		{0x45, 0x44},//0x44}, //0x44},
		{0x46, 0x02},//0x08}, //0x02},

		{0x80, 0x3d}, // R
		{0x81, 0x20}, // G
		{0x82, 0x40}, // B

		{0x83, 0x5a}, //52}, //RMAX
		{0x84, 0x20}, //1d}, //RMIN
		{0x85, 0x53}, //BMAX 5a
		{0x86, 0x24}, //BMIN 

		// STEVE Yellowish		
		{0x87, 0x4a}, //42
		{0x88, 0x3c},
		{0x89, 0x3e},
		{0x8a, 0x34},

		{0x8b, 0x02}, //0x08}, //02}, //OUT TH
		{0x8d, 0x3a}, //0x11}, //22},
		{0x8e, 0xb3}, //0x11}, //71}, 

		{0x8f, 0x65}, //0x63},
		{0x90, 0x61}, //0x60},
		{0x91, 0x5C}, //0x5c},
		{0x92, 0x56}, //0x56},
		{0x93, 0x4E}, //0x52},
		{0x94, 0x43}, //0x4c},
		{0x95, 0x3A}, //0x3e}, //36},
		{0x96, 0x32}, //0x2f}, //31},
		{0x97, 0x2A}, //0x28}, //2e},
		{0x98, 0x24}, //0x23}, //2a},
		{0x99, 0x20}, //0x21}, //29},
		{0x9a, 0x1C}, //0x20}, //26},
		{0x9b, 0x0b}, //0x07}, //09},
		
		////////////////////////////clock calculator//////////////////////////////
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
		{0x03, 0x00}, //Page 0
		{0x40, 0x00}, //Hblank 144
		{0x41, 0x90}, 
		{0x42, 0x00}, //Vblank 4
		{0x43, 0x04}, 

		{0x03, 0x20}, //Page 20
		{0x83, 0x00}, //EXP Normal 33.33 fps 
		{0x84, 0xaf}, 
		{0x85, 0xc8}, 

#else
		{0x03, 0x00}, //Page 0
		{0x40, 0x00}, //Hblank 144
		{0x41, 0x90}, 
		{0x42, 0x00}, //Vblank 4
		{0x43, 0x04}, 

		{0x03, 0x20}, //Page 20
		{0x83, 0x00}, //EXP Normal 30.00 fps 
		{0x84, 0xc3}, 
		{0x85, 0x50}, 

#endif
		{0x86, 0x00}, //EXPMin 7500.00 fps
		{0x87, 0xc8}, 
		{0x88, 0x02}, //EXP Max(120Hz) 10.00 fps 
		{0x89, 0x49}, 
		{0x8a, 0xf0}, 
		{0xa0, 0x02}, //EXP Max(100Hz) 10.00 fps 
		{0xa1, 0x49}, 
		{0xa2, 0xf0}, 
		{0x8B, 0x3a}, //EXP100 
		{0x8C, 0x98}, 
		{0x8D, 0x30}, //EXP120 
		{0x8E, 0xd4}, 
		{0x9c, 0x04}, //EXP Limit 1250.00 fps 
		{0x9d, 0xb0}, 
		{0x9e, 0x00}, //EXP Unit 
		{0x9f, 0xc8}, 

	//AntiBand Unlock
		{0x03, 0x20}, //Page 20 
		{0x2b, 0x34}, 
		{0x30, 0x78}, 

		
		//BLC 
		{0x03, 0x00}, //PAGE 0
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
		{0x90, 0x0a}, //BLC_TIME_TH_ON
		{0x91, 0x0a}, //BLC_TIME_TH_OFF 

#else
		{0x90, 0x0c}, //BLC_TIME_TH_ON
		{0x91, 0x0c}, //BLC_TIME_TH_OFF 

#endif
		{0x92, 0xe8}, //BLC_AG_TH_ON
		{0x93, 0xe0}, //BLC_AG_TH_OFF
		
		/////////////////////////////
		// Page 48 MIPI /////////////
		/////////////////////////////													  
															 
		{0x03, 0x48},																						 
		{0x10, 0x05},									  
		{0x11, 0x00}, //88},																						 
		{0x41, 0x20},									  
		{0x42, 0x00},									  
		{0x3c, 0x00},									  
		{0x3d, 0xfa},									  
															 
		{0x16, 0x88}, //0xc4},									  
		{0x35, 0x02}, //0x03},									  
		{0x1f, 0x04}, //0x06},									  
		{0x1a, 0x00},									  
		{0x34, 0x01}, //0x02},									  
		{0x1c, 0x02},									  
		{0x1d, 0x04},									  
		{0x36, 0x01},									  
		{0x37, 0x03}, 
											
		{0x38, 0x00}, 
		{0x39, 0x4a}, 
															 
		{0x3f, 0x10},									  
		{0x40, 0x00},									  
																												 
		{0x1e, 0x07},									  
		{0x32, 0x06},									  
		{0x20, 0x00},									  
															 
		{0x31, 0x00},									  
		{0x30, 0x05},									  
																								   
		{0x03, 0x22},
		{0x10, 0xfb},
		{0x03, 0x20},
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
		{0x10, 0x9c}, // 100hz first
#else
		{0x10, 0x8c}, // 120hz first
#endif
		{0x03, 0x00},
		{0x01, 0x70},
		
	}
	
};


static struct v4l2_subdev_info hi707_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8, /* For YUV type sensor (YUV422) */
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order  = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array hi707_init_conf[] = {
	{&hi707_recommend_settings[HI707_60HZ][0],
	ARRAY_SIZE(hi707_recommend_settings[HI707_60HZ]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&hi707_recommend_settings[HI707_50HZ][0],
	ARRAY_SIZE(hi707_recommend_settings[HI707_50HZ]), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array hi707_confs[] = {
	{&hi707_prev_settings[0],
	ARRAY_SIZE(hi707_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t hi707_dimensions[] = {
	{
		.x_output = 0x280, // 640
		.y_output = 0x1E0, // 480
		.line_length_pclk = 0xD06,
		.frame_length_lines = 0x04ED,
		.vt_pixel_clk = 45600000,
		.op_pixel_clk = 45600000,
		.binning_factor = 1,
	},
};
static struct msm_camera_i2c_reg_conf hi707_exposure[][2] = {
	{{0x03, 0x10}, {0x40, 0xBC}}, /*EXPOSURECOMPENSATIONN6*/
	{{0x03, 0x10}, {0x40, 0xB2}}, /*EXPOSURECOMPENSATIONN5*/
	{{0x03, 0x10}, {0x40, 0xA8}}, /*EXPOSURECOMPENSATIONN4*/
	{{0x03, 0x10}, {0x40, 0x9E}}, /*EXPOSURECOMPENSATIONN3*/
	{{0x03, 0x10}, {0x40, 0x94}}, /*EXPOSURECOMPENSATIONN2*/
	{{0x03, 0x10}, {0x40, 0x8A}}, /*EXPOSURECOMPENSATIONN1*/
	{{0x03, 0x10}, {0x40, 0x80}}, /*EXPOSURECOMPENSATIOND*/
	{{0x03, 0x10}, {0x40, 0x0A}}, /*EXPOSURECOMPENSATIONp1*/
	{{0x03, 0x10}, {0x40, 0x14}}, /*EXPOSURECOMPENSATIONp2*/
	{{0x03, 0x10}, {0x40, 0x1E}}, /*EXPOSURECOMPENSATIONp3*/
	{{0x03, 0x10}, {0x40, 0x28}}, /*EXPOSURECOMPENSATIONp4*/
	{{0x03, 0x10}, {0x40, 0x32}}, /*EXPOSURECOMPENSATIONp5*/
	{{0x03, 0x10}, {0x40, 0x3C}}, /*EXPOSURECOMPENSATIONp6*/
};

static struct msm_camera_i2c_conf_array hi707_exposure_confs[][1] = {
	{{hi707_exposure[0], ARRAY_SIZE(hi707_exposure[0]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[1], ARRAY_SIZE(hi707_exposure[1]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[2], ARRAY_SIZE(hi707_exposure[2]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[3], ARRAY_SIZE(hi707_exposure[3]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[4], ARRAY_SIZE(hi707_exposure[4]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[5], ARRAY_SIZE(hi707_exposure[5]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[6], ARRAY_SIZE(hi707_exposure[6]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[7], ARRAY_SIZE(hi707_exposure[7]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[8], ARRAY_SIZE(hi707_exposure[8]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[9], ARRAY_SIZE(hi707_exposure[9]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[10], ARRAY_SIZE(hi707_exposure[10]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[11], ARRAY_SIZE(hi707_exposure[11]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[12], ARRAY_SIZE(hi707_exposure[12]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_exposure[13], ARRAY_SIZE(hi707_exposure[13]), 0,
		MSM_CAMERA_I2C_BYTE_DATA},},
};

static int hi707_exposure_enum_map[] = {
	MSM_V4L2_EXPOSURE_N6,
	MSM_V4L2_EXPOSURE_N5,
	MSM_V4L2_EXPOSURE_N4,
	MSM_V4L2_EXPOSURE_N3,
	MSM_V4L2_EXPOSURE_N2,
	MSM_V4L2_EXPOSURE_N1,
	MSM_V4L2_EXPOSURE_D,
	MSM_V4L2_EXPOSURE_P1,
	MSM_V4L2_EXPOSURE_P2,
	MSM_V4L2_EXPOSURE_P3,
	MSM_V4L2_EXPOSURE_P4,
	MSM_V4L2_EXPOSURE_P5,
	MSM_V4L2_EXPOSURE_P6,
};

static struct msm_camera_i2c_enum_conf_array hi707_exposure_enum_confs = {
	.conf = &hi707_exposure_confs[0][0],
	.conf_enum = hi707_exposure_enum_map,
	.num_enum = ARRAY_SIZE(hi707_exposure_enum_map),
	.num_index = ARRAY_SIZE(hi707_exposure_confs),
	.num_conf = ARRAY_SIZE(hi707_exposure_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

static struct msm_camera_i2c_reg_conf hi707_no_effect[] = {
};

static struct msm_camera_i2c_conf_array hi707_no_effect_confs[] = {
	{&hi707_no_effect[0],
	ARRAY_SIZE(hi707_no_effect), 0,
	MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_reg_conf hi707_special_effect[][5] = {
	{{0x03, 0x10}, {0x11, 0x43}, {0x12, 0x30}, {0x44, 0x80}, {0x45, 0x80}},	/*for special effect OFF*/
	{{0x03, 0x10}, {0x11, 0x03}, {0x12, 0x33}, {0x44, 0x80}, {0x45, 0x80}},	/*for special effect MONO*/
	{{0x03, 0x10}, {0x11, 0x03}, {0x12, 0x38}, {0x44, 0x80}, {0x45, 0x80}},	/*for special efefct Negative*/
	{{0x03, 0x10}, {0x11, 0x03}, {0x12, 0x33}, {0x44, 0x70}, {0x45, 0x98}},	/*for sepia*/
};

static struct msm_camera_i2c_conf_array hi707_special_effect_confs[][1] = {
	{{hi707_special_effect[0],  ARRAY_SIZE(hi707_special_effect[0]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_special_effect[1],  ARRAY_SIZE(hi707_special_effect[1]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_special_effect[2],  ARRAY_SIZE(hi707_special_effect[2]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_special_effect[3],  ARRAY_SIZE(hi707_special_effect[3]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
};

static int hi707_special_effect_enum_map[] = {
	MSM_V4L2_EFFECT_OFF,
	MSM_V4L2_EFFECT_MONO,
	MSM_V4L2_EFFECT_NEGATIVE,
	MSM_V4L2_EFFECT_SEPIA,
};

static struct msm_camera_i2c_enum_conf_array
		 hi707_special_effect_enum_confs = {
	.conf = &hi707_special_effect_confs[0][0],
	.conf_enum = hi707_special_effect_enum_map,
	.num_enum = ARRAY_SIZE(hi707_special_effect_enum_map),
	.num_index = ARRAY_SIZE(hi707_special_effect_confs),
	.num_conf = ARRAY_SIZE(hi707_special_effect_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

static struct msm_camera_i2c_reg_conf hi707_wb_oem[][17] = {
	{
		/*WHITEBALNACE AUTO*/
		{0x03, 0x22},
		{0x10, 0x7b},
		{0x11, 0x2e},
		
		{0x80, 0x3d},
		{0x81, 0x20}, 
		{0x82, 0x40},
		
		{0x83, 0x5a}, //RMAX
		{0x84, 0x20}, //RMIN
		{0x85, 0x53}, //BMAX
		{0x86, 0x24}, //BMIN
		
	// STEVE Yellowish		
		{0x87, 0x50}, //0x50}, //4d}, //RMAXB 
		{0x88, 0x38}, //0x3d}, //38}, //RMINB 
		{0x89, 0x40}, //0x38}, //3e}, //BMAXB
		{0x8a, 0x28}, //0x28}, //29}, //BMINB
		
		{0x8d, 0x3a}, //IN/OUT slop R
		{0x8e, 0xb3}, //IN/OUT slop B
		
		{0x10, 0xfb},
	}, 
	/*INCANDISCENT*/
	{
		{0x03, 0x22},
		{0x10, 0x7b},
		{0x11, 0x26},
		//INCA
		{0x80, 0x20},
		{0x81, 0x20},
		{0x82, 0x60},
		
		{0x83, 0x27}, //0x2F},
		{0x84, 0x20}, //0x11},
		{0x85, 0x6f}, //0x67},
		{0x86, 0x60}, //0x58},
		
		{0x87, 0x27}, //0x60}, //RMAX
		{0x88, 0x20}, //0x20}, //RMIN
		{0x89, 0x6f}, //0x60}, //BMAX
		{0x8a, 0x60}, //0x20}, //BMIN
		
		{0x8d, 0x00}, //IN/OUT slop R
		{0x8e, 0x00}, //IN/OUT slop B
		
		{0x10, 0xfb},
	},	
	/*FLUORESCENT*/
	{
		{0x03, 0x22},
		{0x10, 0x7b},
		{0x11, 0x26},
		//TL84
		{0x80, 0x28},
		{0x81, 0x20},
		{0x82, 0x50},
		
		{0x83, 0x2d}, //0x37},
		{0x84, 0x23}, //0x23},
		{0x85, 0x55}, //0x55},
		{0x86, 0x4b}, //0x4B},
		
		{0x87, 0x2d}, //0x60}, //RMAX
		{0x88, 0x23}, //0x20}, //RMIN
		{0x89, 0x55}, //0x60}, //BMAX
		{0x8a, 0x4b}, //0x20}, //BMIN
		
		{0x8d, 0x00}, //IN/OUT slop R
		{0x8e, 0x00}, //IN/OUT slop B
		
		{0x10, 0xfb},
	},	
	/*DAYLIGHT*/
	{
		{0x03, 0x22},
		{0x10, 0x7b},
		{0x11, 0x26},
		
		//D50
		{0x80, 0x42},
		{0x81, 0x20},
		{0x82, 0x3d},
		{0x83, 0x49},
		{0x84, 0x3A},
		{0x85, 0x47},
		{0x86, 0x33},		
		
		{0x87, 0x60}, //RMAX
		{0x88, 0x20}, //RMIN
		{0x89, 0x60}, //BMAX
		{0x8a, 0x20}, //BMIN
		
		{0x8d, 0x00}, //IN/OUT slop R
		{0x8e, 0x00}, //IN/OUT slop B
		
		{0x10, 0xfb},
	},	
	/*CLOUDY*/
	{
		{0x03, 0x22},
		{0x10, 0x7b},
		{0x11, 0x26},

		// Cloudy
		{0x80, 0x60},
		{0x81, 0x20},
		{0x82, 0x20},
		{0x83, 0x70},
		{0x84, 0x51},
		{0x85, 0x2A},
		{0x86, 0x20},		
		
		{0x87, 0x60}, //RMAX
		{0x88, 0x20}, //RMIN
		{0x89, 0x60}, //BMAX
		{0x8a, 0x20}, //BMIN
		
		{0x8d, 0x00}, //IN/OUT slop R
		{0x8e, 0x00}, //IN/OUT slop B
		
		{0x10, 0xfb},
	},	
};

static struct msm_camera_i2c_conf_array hi707_wb_oem_confs[][1] = {
	{{hi707_wb_oem[0], ARRAY_SIZE(hi707_wb_oem[0]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_wb_oem[1], ARRAY_SIZE(hi707_wb_oem[1]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_wb_oem[2], ARRAY_SIZE(hi707_wb_oem[2]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_wb_oem[3], ARRAY_SIZE(hi707_wb_oem[3]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{hi707_wb_oem[4], ARRAY_SIZE(hi707_wb_oem[4]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
};

static int hi707_wb_oem_enum_map[] = {
	MSM_V4L2_WB_AUTO ,
	MSM_V4L2_WB_INCANDESCENT,
	MSM_V4L2_WB_FLUORESCENT,
	MSM_V4L2_WB_DAYLIGHT,
	MSM_V4L2_WB_CLOUDY_DAYLIGHT,
};

static struct msm_camera_i2c_enum_conf_array hi707_wb_oem_enum_confs = {
	.conf = &hi707_wb_oem_confs[0][0],
	.conf_enum = hi707_wb_oem_enum_map,
	.num_enum = ARRAY_SIZE(hi707_wb_oem_enum_map),
	.num_index = ARRAY_SIZE(hi707_wb_oem_confs),
	.num_conf = ARRAY_SIZE(hi707_wb_oem_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

static struct msm_camera_i2c_reg_conf hi707_night_mode[HI707_HZ_MAX_NUM][2][40] = {
	{ // 60Hz
		/*NIGHT MODE OFF*/
		{
			//		{0x03, 0x00},                            
			//		{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
			                                 
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 

			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x0a}, //BLC_TIME_TH_ON           
			{0x91, 0x0a}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 
			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x0c}, //BLC_TIME_TH_ON           
			{0x91, 0x0c}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      
#endif    

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

			{0x86, 0x00}, //EXPMin 7500.00 fps       
			{0x87, 0xc8},  
			          
			{0x88, 0x02}, //EXP Max(120Hz) 10.00 fps 
			{0x89, 0x49},                            
			{0x8a, 0xf0},  
			          
			{0xa0, 0x02}, //EXP Max(100Hz) 10.00 fps 
			{0xa1, 0x49},                            
			{0xa2, 0xf0},   
			         
			{0x8B, 0x3a}, //EXP100                   
			{0x8C, 0x98},    

			{0x8D, 0x30}, //EXP120                   
			{0x8E, 0xd4},    
			        
			{0x9c, 0x04}, //EXP Limit 1250.00 fps    
			{0x9d, 0xb0},    
			        
			{0x9e, 0x00}, //EXP Unit                 
			{0x9f, 0xc8},                            
			                          
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x9c}, // AE ON  
#else
			{0x10, 0x8c}, // AE ON  
#endif
			                         
			{0x03, 0x00},                            
			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x11, 0x90}, // Fixed OFF   
#endif
			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
			//		{0x01, 0x70}, // sleep OFF

			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
		}, 

		/*NIGHT MODE ON*/
		{
			//		{0x03, 0x00},                            
			//		{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 

			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x14}, //BLC_TIME_TH_ON           
			{0x91, 0x14}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 
			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x18}, //BLC_TIME_TH_ON           
			{0x91, 0x18}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      
#endif    

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

			{0x86, 0x00}, //EXPMin 7500.00 fps       
			{0x87, 0xc8},  

			{0x88, 0x04}, //EXP Max(120Hz) 5.00 fps 
			{0x89, 0x93}, 
			{0x8a, 0xe0}, 
			{0xa0, 0x04}, //EXP Max(100Hz) 5.00 fps 
			{0xa1, 0x93}, 
			{0xa2, 0xe0}, 

			{0x8B, 0x3a}, //EXP100                   
			{0x8C, 0x98},    

			{0x8D, 0x30}, //EXP120                   
			{0x8E, 0xd4},    

			{0x9c, 0x04}, //EXP Limit 1250.00 fps    
			{0x9d, 0xb0},    

			{0x9e, 0x00}, //EXP Unit                 
			{0x9f, 0xc8},                            

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x9c}, // AE ON  
#else
			{0x10, 0x8c}, // AE ON  
#endif

			{0x03, 0x00},                            
			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x11, 0x90}, // Fixed OFF   
#endif
			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
			//		{0x01, 0x70}, // sleep OFF

			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF

		},	
	}, 
	{ // 50Hz
		/*NIGHT MODE OFF*/
		{
			//		{0x03, 0x00},                            
			//		{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
			                                 
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 

			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x0a}, //BLC_TIME_TH_ON           
			{0x91, 0x0a}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 
			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x0c}, //BLC_TIME_TH_ON           
			{0x91, 0x0c}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      
#endif    

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

			{0x86, 0x00}, //EXPMin 7500.00 fps       
			{0x87, 0xc8},  
			          
			{0x88, 0x02}, //EXP Max(120Hz) 10.00 fps 
			{0x89, 0x49},                            
			{0x8a, 0xf0},  
			          
			{0xa0, 0x02}, //EXP Max(100Hz) 10.00 fps 
			{0xa1, 0x49},                            
			{0xa2, 0xf0},   
			         
			{0x8B, 0x3a}, //EXP100                   
			{0x8C, 0x98},    

			{0x8D, 0x30}, //EXP120                   
			{0x8E, 0xd4},    
			        
			{0x9c, 0x04}, //EXP Limit 1250.00 fps    
			{0x9d, 0xb0},    
			        
			{0x9e, 0x00}, //EXP Unit                 
			{0x9f, 0xc8},                            
			                          
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x9c}, // AE ON  
#else
			{0x10, 0x8c}, // AE ON  
#endif
			                         
			{0x03, 0x00},                            
			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x11, 0x90}, // Fixed OFF   
#endif
			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
			//		{0x01, 0x70}, // sleep OFF

			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
		}, 

		/*NIGHT MODE ON*/
		{
			//		{0x03, 0x00},                            
			//		{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 

			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x14}, //BLC_TIME_TH_ON           
			{0x91, 0x14}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 
			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x18}, //BLC_TIME_TH_ON           
			{0x91, 0x18}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      
#endif    

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

			{0x86, 0x00}, //EXPMin 7500.00 fps       
			{0x87, 0xc8},  

			{0x88, 0x04}, //EXP Max(120Hz) 5.00 fps 
			{0x89, 0x93}, 
			{0x8a, 0xe0}, 
			{0xa0, 0x04}, //EXP Max(100Hz) 5.00 fps 
			{0xa1, 0x93}, 
			{0xa2, 0xe0}, 

			{0x8B, 0x3a}, //EXP100                   
			{0x8C, 0x98},    

			{0x8D, 0x30}, //EXP120                   
			{0x8E, 0xd4},    

			{0x9c, 0x04}, //EXP Limit 1250.00 fps    
			{0x9d, 0xb0},    

			{0x9e, 0x00}, //EXP Unit                 
			{0x9f, 0xc8},                            

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x9c}, // AE ON  
#else
			{0x10, 0x8c}, // AE ON  
#endif

			{0x03, 0x00},                            
			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x11, 0x90}, // Fixed OFF   
#endif
			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
			//		{0x01, 0x70}, // sleep OFF

			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF

		},	
	}
};

static struct msm_camera_i2c_conf_array hi707_night_mode_confs[HI707_HZ_MAX_NUM][2][1] = {
	{ // 60Hz
		{{hi707_night_mode[HI707_60HZ][0], ARRAY_SIZE(hi707_night_mode[HI707_60HZ][0]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
		{{hi707_night_mode[HI707_60HZ][1], ARRAY_SIZE(hi707_night_mode[HI707_60HZ][1]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
	},
	{ // 50Hz
		{{hi707_night_mode[HI707_50HZ][0], ARRAY_SIZE(hi707_night_mode[HI707_50HZ][0]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
		{{hi707_night_mode[HI707_50HZ][1], ARRAY_SIZE(hi707_night_mode[HI707_50HZ][1]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
	},
	
};

static int hi707_night_mode_enum_map[] = {
	MSM_V4L2_NIGHT_MODE_OFF ,
	MSM_V4L2_NIGHT_MODE_ON,
};

static struct msm_camera_i2c_enum_conf_array hi707_night_mode_enum_confs[HI707_HZ_MAX_NUM] = {
	{ // 60Hz
		.conf = &hi707_night_mode_confs[HI707_60HZ][0][0],
		.conf_enum = hi707_night_mode_enum_map,
		.num_enum = ARRAY_SIZE(hi707_night_mode_enum_map),
		.num_index = ARRAY_SIZE(hi707_night_mode_confs[HI707_60HZ]),
		.num_conf = ARRAY_SIZE(hi707_night_mode_confs[HI707_60HZ][0]),
		.data_type = MSM_CAMERA_I2C_BYTE_DATA,
	},
	{ // 50Hz
		.conf = &hi707_night_mode_confs[HI707_50HZ][0][0],
		.conf_enum = hi707_night_mode_enum_map,
		.num_enum = ARRAY_SIZE(hi707_night_mode_enum_map),
		.num_index = ARRAY_SIZE(hi707_night_mode_confs[HI707_50HZ]),
		.num_conf = ARRAY_SIZE(hi707_night_mode_confs[HI707_50HZ][0]),
		.data_type = MSM_CAMERA_I2C_BYTE_DATA,
	}
	
};

static struct msm_camera_i2c_reg_conf hi707_fps_range[HI707_HZ_MAX_NUM][5][49] = {
	{ // 60Hz
		/*HI707_FPS_15000_15000*/
		{		
			{0x03, 0x00},
			{0x01, 0x71}, // SLEEP ON

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x06}, //BLC_TIME_TH_ON
			{0x91, 0x06}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF
#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x07}, //BLC_TIME_TH_ON
			{0x91, 0x07}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF
#endif		
			//60hz_15Fixed
			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x83, 0x00}, //EXP Normal 33.33 fps 
			{0x84, 0xaf}, 
			{0x85, 0xc8}, 
#else
			{0x83, 0x00}, //EXP Normal 30.00 fps 
			{0x84, 0xc3}, 
			{0x85, 0x50}, 
#endif 

			{0x86, 0x00}, //EXPMin 7500.00 fps
			{0x87, 0xc8}, 

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x88, 0x01}, //EXP Max(120Hz) 17.14 fps 
			{0x89, 0x5f}, 
			{0x8a, 0x90}, 
#else
			{0x88, 0x01}, //EXP Max(120Hz) 17.14 fps 
			{0x89, 0x55}, 
			{0x8a, 0xcc}, 
#endif
			{0xa0, 0x01}, //EXP Max(100Hz) 16.67 fps 
			{0xa1, 0x5f}, 
			{0xa2, 0x90}, 

			{0x8B, 0x3a}, //EXP100 
			{0x8C, 0x98}, 

			{0x8D, 0x30}, //EXP120 
			{0x8E, 0xd4}, 

			{0x91, 0x01}, //EXP Fix 15.00 fps
			{0x92, 0x86}, 
			{0x93, 0xa0}, 

			{0x9c, 0x04}, //EXP Limit 1250.00 fps 
			{0x9d, 0xb0}, 

			{0x9e, 0x00}, //EXP Unit 
			{0x9f, 0xc8}, 

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x9c}, // AE ON
#else
			{0x10, 0x8c}, // AE ON
#endif

			{0x03, 0x00},
			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x97}, // xy flip Fixed ON
#else
			{0x11, 0x94}, // Fixed ON
#endif
			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
			{0x01, 0x70}, // sleep OFF

			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
		}, // 49EA Fixed frame
		
		/*HI707_FPS_10000_30000*/
		{
			{0x03, 0x00},                            
			{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
			                                     
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 

			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x0a}, //BLC_TIME_TH_ON           
			{0x91, 0x0a}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 
			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x0c}, //BLC_TIME_TH_ON           
			{0x91, 0x0c}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      
#endif    

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x83, 0x00}, //EXP Normal 33.33 fps 
			{0x84, 0xaf}, 
			{0x85, 0xc8}, 
#else
			{0x83, 0x00}, //EXP Normal 30.00 fps 
			{0x84, 0xc3}, 
			{0x85, 0x50}, 
#endif 		

			{0x86, 0x00}, //EXPMin 7500.00 fps       
			{0x87, 0xc8},  
			              
			{0x88, 0x02}, //EXP Max(120Hz) 10.00 fps 
			{0x89, 0x49},                            
			{0x8a, 0xf0},  
			              
			{0xa0, 0x02}, //EXP Max(100Hz) 10.00 fps 
			{0xa1, 0x49},                            
			{0xa2, 0xf0},   
			             
			{0x8B, 0x3a}, //EXP100                   
			{0x8C, 0x98},    
			    
			{0x8D, 0x30}, //EXP120                   
			{0x8E, 0xd4},    
			            
			{0x9c, 0x04}, //EXP Limit 1250.00 fps    
			{0x9d, 0xb0},    
			            
			{0x9e, 0x00}, //EXP Unit                 
			{0x9f, 0xc8},                            
			                              
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x9c}, // AE ON  
#else
			{0x10, 0x8c}, // AE ON  
#endif
			                             
			{0x03, 0x00},                            
			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x11, 0x90}, // Fixed OFF   
#endif
			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
			{0x01, 0x70}, // sleep OFF

			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
			{0x03, 0x00}, // dummy1
			{0x03, 0x00}, // dummy2
			{0x03, 0x00}, // dummy3

		}, //46+3EA Variable frame
		
		/*HI707_FPS_20000_30000*/
		{
			{0x03, 0x00},                            
			{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
			                             
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04},  
			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x05}, //BLC_TIME_TH_ON
			{0x91, 0x05}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 
			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x06}, //BLC_TIME_TH_ON
			{0x91, 0x06}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF
#endif                          

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x83, 0x00}, //EXP Normal 33.33 fps 
			{0x84, 0xaf}, 
			{0x85, 0xc8}, 
#else
			{0x83, 0x00}, //EXP Normal 30.00 fps 
			{0x84, 0xc3}, 
			{0x85, 0x50}, 
#endif 	

			{0x86, 0x00}, //EXPMin 7500.00 fps
			{0x87, 0xc8}, 

			{0x88, 0x01}, //EXP Max(120Hz) 20.00 fps 
			{0x89, 0x24}, 
			{0x8a, 0xf8}, 

			{0xa0, 0x01}, //EXP Max(100Hz) 20.00 fps 
			{0xa1, 0x24}, 
			{0xa2, 0xf8}, 

			{0x8B, 0x3a}, //EXP100 
			{0x8C, 0x98}, 

			{0x8D, 0x30}, //EXP120 
			{0x8E, 0xd4}, 

			{0x9c, 0x04}, //EXP Limit 1250.00 fps 
			{0x9d, 0xb0}, 

			{0x9e, 0x00}, //EXP Unit 
			{0x9f, 0xc8}, 

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x9c}, // AE ON	
#else
			{0x10, 0x8c}, // AE ON  
#endif
			                             
			{0x03, 0x00},                            
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x11, 0x90}, // Fixed OFF   
#endif
			{0x01, 0x70}, // sleep OFF    
			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
			{0x03, 0x00}, // dummy1
			{0x03, 0x00}, // dummy2
			{0x03, 0x00}, // dummy3

		}, //46+3 EA Variable frame
		
		/*HI707_FPS_24000_30000*/
		{
			{0x03, 0x00},                            
			{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
			                                         
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank 60hz
			{0x43, 0x04}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x04}, //BLC_TIME_TH_ON
			{0x91, 0x04}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank 60hz
			{0x43, 0x04}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x05}, //BLC_TIME_TH_ON
			{0x91, 0x05}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF
#endif						   

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x83, 0x00}, //EXP Normal 33.33 fps 
			{0x84, 0xaf}, 
			{0x85, 0xc8}, 
#else
			{0x83, 0x00}, //EXP Normal 30.00 fps 
			{0x84, 0xc3}, 
			{0x85, 0x50}, 
#endif 	

			{0x86, 0x00}, //EXPMin 7500.00 fps
			{0x87, 0xc8}, 

			{0x88, 0x00}, //EXP Max(120Hz) 24.00 fps 
			{0x89, 0xf4}, 
			{0x8a, 0x24}, 

			{0xa0, 0x00}, //EXP Max(100Hz) 25.00 fps 
			{0xa1, 0xea}, 
			{0xa2, 0x60}, 

			{0x8B, 0x3a}, //EXP100 
			{0x8C, 0x98}, 

			{0x8D, 0x30}, //EXP120 
			{0x8E, 0xd4}, 

			{0x9c, 0x04}, //EXP Limit 1250.00 fps 
			{0x9d, 0xb0}, 

			{0x9e, 0x00}, //EXP Unit 
			{0x9f, 0xc8}, 

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
			{0x10, 0x9c}, // AE ON				  
#else
			{0x10, 0x8c}, // AE ON
#endif
							   
			{0x03, 0x00}, 						   
#ifdef CONFIG_HI707_ROT_180 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x11, 0x90}, // Fixed OFF   
#endif
			{0x01, 0x70}, // sleep OFF	

			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
			{0x03, 0x00}, // dummy1
			{0x03, 0x00}, // dummy2
			{0x03, 0x00}, // dummy3		
		}, //46+3EA Variable frame
		
		/*HI707_FPS_30000_30000*/
		{		
			{0x03, 0x00},                            
			{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x02}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x03}, //BLC_TIME_TH_ON
			{0x91, 0x03}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x02}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x03}, //BLC_TIME_TH_ON
			{0x91, 0x03}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF
#endif						   

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x83, 0x00}, //EXP Normal 33.33 fps 
			{0x84, 0xaf}, 
			{0x85, 0xc8}, 
#else
			{0x83, 0x00}, //EXP Normal 30.00 fps 
			{0x84, 0xc3}, 
			{0x85, 0x50}, 
#endif 

			{0x86, 0x00}, //EXPMin 7500.00 fps
			{0x87, 0xc8}, 

			{0x88, 0x00}, //EXP Max(120Hz) 40.00 fps 
#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
			{0x89, 0xaf}, 
			{0x8a, 0xc8}, 
#else
			{0x89, 0x92}, 
			{0x8a, 0x7c}, 
#endif

			{0xa0, 0x00}, //EXP Max(100Hz) 33.33 fps 
			{0xa1, 0xaf}, 
			{0xa2, 0xc8}, 

			{0x8B, 0x3a}, //EXP100 
			{0x8C, 0x98}, 

			{0x8D, 0x30}, //EXP120 
			{0x8E, 0xd4}, 

			{0x91, 0x00}, //EXP Fix 30.00 fps
			{0x92, 0xc3}, 
			{0x93, 0x50}, 

			{0x9c, 0x04}, //EXP Limit 1250.00 fps 
			{0x9d, 0xb0}, 

			{0x9e, 0x00}, //EXP Unit 
			{0x9f, 0xc8}, 


#if 0 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
			{0x10, 0x9c}, // AE ON
#else
			{0x10, 0x8c}, // AE ON
#endif

			{0x03, 0x00},
			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x97}, // xy flip Fixed ON
#else
			{0x11, 0x94}, // Fixed ON
#endif
			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
			{0x01, 0x70}, // sleep OFF
			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
		}, //49 EA Fixed frame
	},
	{ // 50Hz
		/*HI707_FPS_15000_15000*/
		{		
			{0x03, 0x00},
			{0x01, 0x71}, // SLEEP ON

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x06}, //BLC_TIME_TH_ON
			{0x91, 0x06}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF
#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x07}, //BLC_TIME_TH_ON
			{0x91, 0x07}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF
#endif		
			//60hz_15Fixed
			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x83, 0x00}, //EXP Normal 33.33 fps 
			{0x84, 0xaf}, 
			{0x85, 0xc8}, 
#else
			{0x83, 0x00}, //EXP Normal 30.00 fps 
			{0x84, 0xc3}, 
			{0x85, 0x50}, 
#endif 

			{0x86, 0x00}, //EXPMin 7500.00 fps
			{0x87, 0xc8}, 

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x88, 0x01}, //EXP Max(120Hz) 17.14 fps 
			{0x89, 0x5f}, 
			{0x8a, 0x90}, 
#else
			{0x88, 0x01}, //EXP Max(120Hz) 17.14 fps 
			{0x89, 0x55}, 
			{0x8a, 0xcc}, 
#endif
			{0xa0, 0x01}, //EXP Max(100Hz) 16.67 fps 
			{0xa1, 0x5f}, 
			{0xa2, 0x90}, 

			{0x8B, 0x3a}, //EXP100 
			{0x8C, 0x98}, 

			{0x8D, 0x30}, //EXP120 
			{0x8E, 0xd4}, 

			{0x91, 0x01}, //EXP Fix 15.00 fps
			{0x92, 0x86}, 
			{0x93, 0xa0}, 

			{0x9c, 0x04}, //EXP Limit 1250.00 fps 
			{0x9d, 0xb0}, 

			{0x9e, 0x00}, //EXP Unit 
			{0x9f, 0xc8}, 

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x9c}, // AE ON
#else
			{0x10, 0x8c}, // AE ON
#endif

			{0x03, 0x00},
			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x97}, // xy flip Fixed ON
#else
			{0x11, 0x94}, // Fixed ON
#endif
			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
			{0x01, 0x70}, // sleep OFF

			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
		}, // 49EA Fixed frame
		
		/*HI707_FPS_10000_30000*/
		{
			{0x03, 0x00},                            
			{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
			                                     
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 

			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x0a}, //BLC_TIME_TH_ON           
			{0x91, 0x0a}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 
			//BLC                                    
			{0x03, 0x00}, //PAGE 0                   
			{0x90, 0x0c}, //BLC_TIME_TH_ON           
			{0x91, 0x0c}, //BLC_TIME_TH_OFF          
			{0x92, 0xe8}, //BLC_AG_TH_ON             
			{0x93, 0xe0}, //BLC_AG_TH_OFF      
#endif    

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x83, 0x00}, //EXP Normal 33.33 fps 
			{0x84, 0xaf}, 
			{0x85, 0xc8}, 
#else
			{0x83, 0x00}, //EXP Normal 30.00 fps 
			{0x84, 0xc3}, 
			{0x85, 0x50}, 
#endif 		

			{0x86, 0x00}, //EXPMin 7500.00 fps       
			{0x87, 0xc8},  
			              
			{0x88, 0x02}, //EXP Max(120Hz) 10.00 fps 
			{0x89, 0x49},                            
			{0x8a, 0xf0},  
			              
			{0xa0, 0x02}, //EXP Max(100Hz) 10.00 fps 
			{0xa1, 0x49},                            
			{0xa2, 0xf0},   
			             
			{0x8B, 0x3a}, //EXP100                   
			{0x8C, 0x98},    
			    
			{0x8D, 0x30}, //EXP120                   
			{0x8E, 0xd4},    
			            
			{0x9c, 0x04}, //EXP Limit 1250.00 fps    
			{0x9d, 0xb0},    
			            
			{0x9e, 0x00}, //EXP Unit                 
			{0x9f, 0xc8},                            
			                              
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x9c}, // AE ON  
#else
			{0x10, 0x8c}, // AE ON  
#endif
			                             
			{0x03, 0x00},                            
			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x11, 0x90}, // Fixed OFF   
#endif
			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
			{0x01, 0x70}, // sleep OFF

			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
			{0x03, 0x00}, // dummy1
			{0x03, 0x00}, // dummy2
			{0x03, 0x00}, // dummy3

		}, //46+3EA Variable frame
		
		/*HI707_FPS_20000_30000*/
		{
			{0x03, 0x00},                            
			{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
			                             
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04},  
			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x05}, //BLC_TIME_TH_ON
			{0x91, 0x05}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x04}, 
			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x06}, //BLC_TIME_TH_ON
			{0x91, 0x06}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF
#endif                          

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x83, 0x00}, //EXP Normal 33.33 fps 
			{0x84, 0xaf}, 
			{0x85, 0xc8}, 
#else
			{0x83, 0x00}, //EXP Normal 30.00 fps 
			{0x84, 0xc3}, 
			{0x85, 0x50}, 
#endif 	

			{0x86, 0x00}, //EXPMin 7500.00 fps
			{0x87, 0xc8}, 

			{0x88, 0x01}, //EXP Max(120Hz) 20.00 fps 
			{0x89, 0x24}, 
			{0x8a, 0xf8}, 

			{0xa0, 0x01}, //EXP Max(100Hz) 20.00 fps 
			{0xa1, 0x24}, 
			{0xa2, 0xf8}, 

			{0x8B, 0x3a}, //EXP100 
			{0x8C, 0x98}, 

			{0x8D, 0x30}, //EXP120 
			{0x8E, 0xd4}, 

			{0x9c, 0x04}, //EXP Limit 1250.00 fps 
			{0x9d, 0xb0}, 

			{0x9e, 0x00}, //EXP Unit 
			{0x9f, 0xc8}, 

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x9c}, // AE ON	
#else
			{0x10, 0x8c}, // AE ON  
#endif
			                             
			{0x03, 0x00},                            
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x11, 0x90}, // Fixed OFF   
#endif
			{0x01, 0x70}, // sleep OFF    
			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
			{0x03, 0x00}, // dummy1
			{0x03, 0x00}, // dummy2
			{0x03, 0x00}, // dummy3

		}, //46+3 EA Variable frame
		
		/*HI707_FPS_24000_30000*/
		{
			{0x03, 0x00},                            
			{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
			                                         
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank 60hz
			{0x43, 0x04}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x04}, //BLC_TIME_TH_ON
			{0x91, 0x04}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank 60hz
			{0x43, 0x04}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x05}, //BLC_TIME_TH_ON
			{0x91, 0x05}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF
#endif						   

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x83, 0x00}, //EXP Normal 33.33 fps 
			{0x84, 0xaf}, 
			{0x85, 0xc8}, 
#else
			{0x83, 0x00}, //EXP Normal 30.00 fps 
			{0x84, 0xc3}, 
			{0x85, 0x50}, 
#endif 	

			{0x86, 0x00}, //EXPMin 7500.00 fps
			{0x87, 0xc8}, 

			{0x88, 0x00}, //EXP Max(120Hz) 24.00 fps 
			{0x89, 0xf4}, 
			{0x8a, 0x24}, 

			{0xa0, 0x00}, //EXP Max(100Hz) 25.00 fps 
			{0xa1, 0xea}, 
			{0xa2, 0x60}, 

			{0x8B, 0x3a}, //EXP100 
			{0x8C, 0x98}, 

			{0x8D, 0x30}, //EXP120 
			{0x8E, 0xd4}, 

			{0x9c, 0x04}, //EXP Limit 1250.00 fps 
			{0x9d, 0xb0}, 

			{0x9e, 0x00}, //EXP Unit 
			{0x9f, 0xc8}, 

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
			{0x10, 0x9c}, // AE ON				  
#else
			{0x10, 0x8c}, // AE ON
#endif
							   
			{0x03, 0x00}, 						   
#ifdef CONFIG_HI707_ROT_180 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x11, 0x90}, // Fixed OFF   
#endif
			{0x01, 0x70}, // sleep OFF	

			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
			{0x03, 0x00}, // dummy1
			{0x03, 0x00}, // dummy2
			{0x03, 0x00}, // dummy3		
		}, //46+3EA Variable frame
		
		/*HI707_FPS_30000_30000*/
		{		
			{0x03, 0x00},                            
			{0x01, 0x71}, // SLEEP ON                

			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x03, 0x00}, 
			{0x11, 0x93}, // xy flip Fixed OFF
#else
			{0x03, 0x00}, 
			{0x11, 0x90}, // Fixed OFF     
#endif

			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */       
			{0x03, 0x20},		
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x10, 0x1c}, // AE OFF      
#else
			{0x10, 0x0c}, // AE OFF         
#endif
			{0x18, 0x38}, // AE reset ON 
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ			
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x02}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x03}, //BLC_TIME_TH_ON
			{0x91, 0x03}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF

#else
			{0x03, 0x00}, //Page 0
			{0x40, 0x00}, //Hblank 144
			{0x41, 0x90}, 
			{0x42, 0x00}, //Vblank
			{0x43, 0x02}, 

			//BLC 
			{0x03, 0x00}, //PAGE 0
			{0x90, 0x03}, //BLC_TIME_TH_ON
			{0x91, 0x03}, //BLC_TIME_TH_OFF 
			{0x92, 0xe8}, //BLC_AG_TH_ON
			{0x93, 0xe0}, //BLC_AG_TH_OFF
#endif						   

			{0x03, 0x20}, //Page 20  
			{0x2a, 0xf0}, //0xf0 : speed up, 0x90: slow(Fixed)
			{0x2b, 0x34}, //0x34 : 1/100, 0xf5 : 2/100	   
			{0x30, 0xf8},	//AE_Escape_CTL 0x78 1/100sec gain release at unlock0

#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ	
			{0x83, 0x00}, //EXP Normal 33.33 fps 
			{0x84, 0xaf}, 
			{0x85, 0xc8}, 
#else
			{0x83, 0x00}, //EXP Normal 30.00 fps 
			{0x84, 0xc3}, 
			{0x85, 0x50}, 
#endif 

			{0x86, 0x00}, //EXPMin 7500.00 fps
			{0x87, 0xc8}, 

			{0x88, 0x00}, //EXP Max(120Hz) 40.00 fps 
#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
			{0x89, 0xaf}, 
			{0x8a, 0xc8}, 
#else
			{0x89, 0x92}, 
			{0x8a, 0x7c}, 
#endif

			{0xa0, 0x00}, //EXP Max(100Hz) 33.33 fps 
			{0xa1, 0xaf}, 
			{0xa2, 0xc8}, 

			{0x8B, 0x3a}, //EXP100 
			{0x8C, 0x98}, 

			{0x8D, 0x30}, //EXP120 
			{0x8E, 0xd4}, 

			{0x91, 0x00}, //EXP Fix 30.00 fps
			{0x92, 0xc3}, 
			{0x93, 0x50}, 

			{0x9c, 0x04}, //EXP Limit 1250.00 fps 
			{0x9d, 0xb0}, 

			{0x9e, 0x00}, //EXP Unit 
			{0x9f, 0xc8}, 


#if 1 // #ifdef LGE_CAMERA_ANTIBAND_50HZ
			{0x10, 0x9c}, // AE ON
#else
			{0x10, 0x8c}, // AE ON
#endif

			{0x03, 0x00},
			/* LGE_CHANGE_S : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
#ifdef CONFIG_HI707_ROT_180	
			{0x11, 0x97}, // xy flip Fixed ON
#else
			{0x11, 0x94}, // Fixed ON
#endif
			/* LGE_CHANGE_E : 2012-10-13 sungmin.cho@lge.com hi707 is ratated 180 degree. EVB2 no flip, others xy flip */
			{0x01, 0x70}, // sleep OFF
			{0x03, 0x20},                
			{0x18, 0x30}, // AE reset OFF
		}, //49 EA Fixed frame
	}
	
};

static struct msm_camera_i2c_conf_array hi707_fps_range_confs[HI707_HZ_MAX_NUM][5][1] = {
	{ // 60Hz
		{{hi707_fps_range[HI707_60HZ][0], ARRAY_SIZE(hi707_fps_range[HI707_60HZ][0]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
		{{hi707_fps_range[HI707_60HZ][1], ARRAY_SIZE(hi707_fps_range[HI707_60HZ][1]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
		{{hi707_fps_range[HI707_60HZ][2], ARRAY_SIZE(hi707_fps_range[HI707_60HZ][2]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
		{{hi707_fps_range[HI707_60HZ][3], ARRAY_SIZE(hi707_fps_range[HI707_60HZ][3]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
		{{hi707_fps_range[HI707_60HZ][4], ARRAY_SIZE(hi707_fps_range[HI707_60HZ][4]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
	}, 
	{ // 50Hz
		{{hi707_fps_range[HI707_50HZ][0], ARRAY_SIZE(hi707_fps_range[HI707_50HZ][0]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
		{{hi707_fps_range[HI707_50HZ][1], ARRAY_SIZE(hi707_fps_range[HI707_50HZ][1]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
		{{hi707_fps_range[HI707_50HZ][2], ARRAY_SIZE(hi707_fps_range[HI707_50HZ][2]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
		{{hi707_fps_range[HI707_50HZ][3], ARRAY_SIZE(hi707_fps_range[HI707_50HZ][3]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
		{{hi707_fps_range[HI707_50HZ][4], ARRAY_SIZE(hi707_fps_range[HI707_50HZ][4]),  0,
			MSM_CAMERA_I2C_BYTE_DATA},},
	}
};

static int hi707_fps_range_enum_map[] = {
	MSM_V4L2_FPS_15_15, 
	MSM_V4L2_FPS_10_30,
	MSM_V4L2_FPS_20_30,
	MSM_V4L2_FPS_24_30,
	MSM_V4L2_FPS_30_30,
};

static struct msm_camera_i2c_enum_conf_array hi707_fps_range_enum_confs[HI707_HZ_MAX_NUM] = {
	{ // 60Hz
		.conf = &hi707_fps_range_confs[HI707_60HZ][0][0],
		.conf_enum = hi707_fps_range_enum_map,
		.num_enum = ARRAY_SIZE(hi707_fps_range_enum_map),
		.num_index = ARRAY_SIZE(hi707_fps_range_confs[HI707_60HZ]),
		.num_conf = ARRAY_SIZE(hi707_fps_range_confs[HI707_60HZ][0]),
		.data_type = MSM_CAMERA_I2C_BYTE_DATA,
	},
	{ // 50Hz
		.conf = &hi707_fps_range_confs[HI707_50HZ][0][0],
		.conf_enum = hi707_fps_range_enum_map,
		.num_enum = ARRAY_SIZE(hi707_fps_range_enum_map),
		.num_index = ARRAY_SIZE(hi707_fps_range_confs[HI707_50HZ]),
		.num_conf = ARRAY_SIZE(hi707_fps_range_confs[HI707_50HZ][0]),
		.data_type = MSM_CAMERA_I2C_BYTE_DATA,
	}
};

#if 0
int hi707_effect_msm_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;

	effect_value = value;
	if (effect_value == CAMERA_EFFECT_OFF) {
		rc = msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			s_ctrl->msm_sensor_reg->no_effect_settings, 0);
		if (rc < 0) {
			CDBG("write faield\n");
			return rc;
		}
	} else {
		rc = msm_sensor_write_enum_conf_array(
			s_ctrl->sensor_i2c_client,
			ctrl_info->enum_cfg_settings, value);
	}
	return rc;
}
#endif

int hi707_msm_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;

	//pr_err("%s is called enum num: %d , value = %d\n", __func__, ctrl_info->ctrl_id, value);
	switch(ctrl_info->ctrl_id) {
		case V4L2_CID_EXPOSURE:
			printk("%s : PREV_EXPOSURE = %d, value = %d\n", __func__, PREV_EXPOSURE, value);
			if(PREV_EXPOSURE == value) {
				return rc;
			}else {
				PREV_EXPOSURE = value;
				rc = msm_sensor_write_enum_conf_array(
					s_ctrl->sensor_i2c_client,
					ctrl_info->enum_cfg_settings, value);
			}
			break;
			
		case V4L2_CID_SPECIAL_EFFECT:
			printk("%s : PREV_EFFECT = %d, value = %d\n", __func__, PREV_EFFECT, value);
			if(PREV_EFFECT == value) {
				return rc;
			}else {
				PREV_EFFECT = value;
				rc = msm_sensor_write_enum_conf_array(
					s_ctrl->sensor_i2c_client,
					ctrl_info->enum_cfg_settings, value);
			}
			break;
			
		case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
			printk("%s : PREV_WB = %d, value = %d\n", __func__, PREV_WB, value);
			if(PREV_WB == value) {
				return rc;
			}else {
				PREV_WB = value;
				rc = msm_sensor_write_enum_conf_array(
					s_ctrl->sensor_i2c_client,
					ctrl_info->enum_cfg_settings, value);
			}
			break;
			
		case V4L2_CID_NIGHT_MODE:
			printk("%s : PREV_NIGHT_MODE = %d, value = %d\n", __func__, PREV_NIGHT_MODE, value);
			if(PREV_NIGHT_MODE == value) {
				return rc;
			}else {
				PREV_NIGHT_MODE = value;
				rc = msm_sensor_write_enum_conf_array(
					s_ctrl->sensor_i2c_client,
					&hi707_night_mode_enum_confs[hi707_antibanding], value);
			}
			break;
			
		case V4L2_CID_FPS_RANGE:
			printk("%s : PREV_FPS = %d, value = %d\n", __func__, PREV_FPS, value);
			if(PREV_FPS == value){
				return rc;
			}else {
				PREV_FPS = value;
				rc = msm_sensor_write_enum_conf_array(
					s_ctrl->sensor_i2c_client,
					&hi707_fps_range_enum_confs[hi707_antibanding], value);
			}
			break;
		
		default:
			break;
	}

	if (rc < 0) {
		CDBG("write faield\n");
		return rc;
	}
	return rc;
}

struct msm_sensor_v4l2_ctrl_info_t hi707_v4l2_ctrl_info[] = {
	{
		.ctrl_id = V4L2_CID_EXPOSURE,
		.min = MSM_V4L2_EXPOSURE_N6,
		.max = MSM_V4L2_EXPOSURE_P6,
		.step = 1,
		.enum_cfg_settings = &hi707_exposure_enum_confs,
		.s_v4l2_ctrl = hi707_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_SPECIAL_EFFECT,
		.min = MSM_V4L2_EFFECT_OFF,
		.max = MSM_V4L2_EFFECT_SEPIA,
		.step = 1,
		.enum_cfg_settings = &hi707_special_effect_enum_confs,
		.s_v4l2_ctrl = hi707_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_WHITE_BALANCE_TEMPERATURE,
		.min = MSM_V4L2_WB_AUTO,
		.max = MSM_V4L2_WB_CLOUDY_DAYLIGHT,
		.step = 1,
		.enum_cfg_settings = &hi707_wb_oem_enum_confs,
		.s_v4l2_ctrl = hi707_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_NIGHT_MODE,
		.min = MSM_V4L2_NIGHT_MODE_OFF,
		.max = MSM_V4L2_NIGHT_MODE_ON,
		.step = 1,
		.enum_cfg_settings = &hi707_night_mode_enum_confs[0], // not used
		.s_v4l2_ctrl = hi707_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_FPS_RANGE,
		.min = MSM_V4L2_FPS_15_15,
		.max = MSM_V4L2_FPS_30_30,
		.step = 1,
		.enum_cfg_settings = &hi707_fps_range_enum_confs[0], // not used
		.s_v4l2_ctrl = hi707_msm_sensor_s_ctrl_by_enum,
	},
};

static struct msm_camera_csi_params hi707_csic_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x14,
};

static struct msm_camera_csi_params *hi707_csic_params_array[] = {
	&hi707_csic_params,
};

// not used
static struct msm_sensor_output_reg_addr_t hi707_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t hi707_id_info = {
	.sensor_id_reg_addr = 0x4,
	.sensor_id = 0xB8,
};

//static struct sensor_calib_data hi707_calib_data;
static const struct i2c_device_id hi707_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&hi707_s_ctrl},
	{ }
};

static ssize_t hi707_antibanding_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	sscanf(buf,"%d",&val);
	printk("hi707: antibanding type [0x%x] \n",val);

	/* 1 : Antibanding 60Hz
	 * 2 : Antibanding 50Hz */
	switch(val)
	{
		case 1:
			hi707_antibanding = HI707_60HZ;
			break;
		case 2:
			hi707_antibanding = HI707_50HZ;
			break;
		default:
			pr_err("hi707: invalid antibanding type[%d] \n",val);
			hi707_antibanding = HI707_50HZ;			
			break;	
	}
	return n;	
}

static DEVICE_ATTR(antibanding, /*S_IRUGO|S_IWUGO*/ 0664, NULL, hi707_antibanding_store);

static struct attribute* hi707_sysfs_attrs[] = {
	&dev_attr_antibanding.attr,
};

static int hi707_sysfs_add(struct kobject* kobj)
{
	int i, n, ret;
	
	n = ARRAY_SIZE(hi707_sysfs_attrs);
	for(i = 0; i < n; i++){
		if(hi707_sysfs_attrs[i]){
			ret = sysfs_create_file(kobj, hi707_sysfs_attrs[i]);
			if(ret < 0){
				pr_err("hi707 sysfs is not created\n");
			}
		}
	}
	return 0;
}

int32_t hi707_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	struct msm_sensor_ctrl_t *s_ctrl;
	CDBG("%s %s_i2c_probe called\n", __func__, client->name);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s %s i2c_check_functionality failed\n",
			__func__, client->name);
		rc = -EFAULT;
		return rc;
	}

	s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);
	if (s_ctrl->sensor_i2c_client != NULL) {
		s_ctrl->sensor_i2c_client->client = client;
		if (s_ctrl->sensor_i2c_addr != 0)
			s_ctrl->sensor_i2c_client->client->addr =
				s_ctrl->sensor_i2c_addr;
	} else {
		pr_err("%s %s sensor_i2c_client NULL\n",
			__func__, client->name);
		rc = -EFAULT;
		return rc;
	}

	s_ctrl->sensordata = client->dev.platform_data;
	if (s_ctrl->sensordata == NULL) {
		pr_err("%s %s NULL sensor data\n", __func__, client->name);
		return -EFAULT;
	}

	rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
	if (rc < 0) {
		pr_err("%s %s power up failed\n", __func__, client->name);
		return rc;
	}

	if (s_ctrl->func_tbl->sensor_match_id)
		rc = s_ctrl->func_tbl->sensor_match_id(s_ctrl);
	else
		rc = msm_sensor_match_id(s_ctrl);
	if (rc < 0)
		goto probe_fail;

	snprintf(s_ctrl->sensor_v4l2_subdev.name,
		sizeof(s_ctrl->sensor_v4l2_subdev.name), "%s", id->name);
	v4l2_i2c_subdev_init(&s_ctrl->sensor_v4l2_subdev, client,
		s_ctrl->sensor_v4l2_subdev_ops);

	msm_sensor_register(&s_ctrl->sensor_v4l2_subdev);
	goto power_down;
probe_fail:
	pr_err("%s %s_i2c_probe failed\n", __func__, client->name);
power_down:
	if (rc > 0)
		rc = 0;
	s_ctrl->func_tbl->sensor_power_down(s_ctrl);
/* LGE_CHANGE_S : 2013-02-20 sungmin.cho@lge.com for setting antibanding in runtime */
	rc = hi707_sysfs_add(&client->dev.kobj);
	if (rc < 0){
		pr_err("hi707: failed hi707_sysfs_add\n");
	}
/* LGE_CHANGE_E : 2013-02-20 sungmin.cho@lge.com for setting antibanding in runtime */
	return rc;
}


static struct i2c_driver hi707_i2c_driver = {
	.id_table = hi707_i2c_id,
	.probe  = hi707_i2c_probe, //msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client hi707_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};


static int __init msm_sensor_init_module(void)
{
	int rc = 0;
	CDBG("hi707\n");

	rc = i2c_add_driver(&hi707_i2c_driver);

	return rc;
}

static struct v4l2_subdev_core_ops hi707_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

int hi707_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	printk("%s: E %d\n", __func__, __LINE__);
	
	PREV_EFFECT = -1;
	PREV_EXPOSURE = -1;
	PREV_WB = -1;
	PREV_FPS = -1;
	PREV_NIGHT_MODE = -1;
	
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

	// 1. RESET LOW
	if (data->sensor_reset_enable) {
		rc = gpio_request(data->sensor_reset, "VT_RESET");
		if (rc < 0)
			pr_err("%s: gpio_request:VT_RESET %d failed\n", __func__, data->sensor_reset);
		rc = gpio_direction_output(data->sensor_reset, 0);
		if (rc < 0)
			pr_err("%s: gpio:VT_RESET %d direction can't be set\n", __func__, data->sensor_reset);
	}

	// 2. PWDN LOW
	rc = gpio_request(data->sensor_pwd, "VT_PWDN");
	if (rc < 0)
		pr_err("%s: gpio_request:VT_PWDN %d failed\n", __func__, data->sensor_pwd);
	rc = gpio_direction_output(data->sensor_pwd, 0);
	if (rc < 0)
		pr_err("%s: gpio:VT_PWDN %d direction can't be set\n", __func__, data->sensor_pwd);
	msleep(1);

	// 3. VT CAM PWR ON
	rc = msm_camera_config_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
		goto config_gpio_failed;
	}
	msleep(1);

	// 4. MCLK Enable
	if (s_ctrl->clk_rate != 0)
		cam_clk_info->clk_rate = s_ctrl->clk_rate;

	rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n", __func__);
		goto enable_clk_failed;
	}
	msleep(1);
	
	// 5. PWDN HIGH
	rc = gpio_direction_output(data->sensor_pwd, 1);
	if (rc < 0)
		pr_err("%s: gpio:VT_PWDN %d direction can't be set\n", __func__, data->sensor_pwd);
	msleep(30);

	// 6. RESET HIGH
	rc = gpio_direction_output(data->sensor_reset, 1);
	if (rc < 0)
		pr_err("%s: gpio:VT_RESET %d direction can't be set\n", __func__, data->sensor_reset);

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(1);
	
	printk("%s: X %d\n", __func__, __LINE__);

	return rc;
enable_clk_failed:
	msm_camera_request_gpio_table(data, 0);
request_gpio_failed:
	msm_camera_config_gpio_table(data, 0);
config_gpio_failed:
	kfree(s_ctrl->reg_ptr);
	return rc;
}

int hi707_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	int32_t rc = 0;
	printk("%s: E %d\n", __func__, __LINE__);
	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(0);

	// 1. RESET LOW
	if (data->sensor_reset_enable) {
		rc = gpio_direction_output(data->sensor_reset, 0);
		if (rc < 0)
			pr_err("%s: gpio:VT_RESET %d direction can't be set\n", __func__, data->sensor_reset);
		gpio_free(data->sensor_reset);
	}
	msleep(30);

	// 2. PWDN LOW
	rc = gpio_direction_output(data->sensor_pwd, 0);
	if (rc < 0)
		pr_err("%s: gpio:VT_PWDN %d direction can't be set\n", __func__, data->sensor_pwd);
	gpio_free(data->sensor_pwd);
	msleep(2);
	
	// 3. MCLK disable
	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);
	msleep(2);

	// 4. VT CAM PWR OFF
	msm_camera_config_gpio_table(data, 0);
	msm_camera_request_gpio_table(data, 0);
	kfree(s_ctrl->reg_ptr);
	printk("%s: X %d\n", __func__, __LINE__);
	return 0;
}

int32_t hi707_camera_i2c_write_tbl(struct msm_camera_i2c_client *client,
                                     struct msm_camera_i2c_reg_conf *reg_conf_tbl, uint16_t size,
                                     enum msm_camera_i2c_data_type data_type)                                    
{
	int i;
	int32_t rc = -EFAULT;
	for (i = 0; i < size; i++) {
		enum msm_camera_i2c_data_type dt;
		if (reg_conf_tbl->cmd_type == MSM_CAMERA_I2C_CMD_POLL) {
			rc = msm_camera_i2c_poll(client, reg_conf_tbl->reg_addr,
				reg_conf_tbl->reg_data, reg_conf_tbl->dt);
		} else {
			if (reg_conf_tbl->dt == 0)
				dt = data_type;
			else
				dt = reg_conf_tbl->dt;

			switch (dt) {
			case MSM_CAMERA_I2C_BYTE_DATA:
			case MSM_CAMERA_I2C_WORD_DATA:
				rc = msm_camera_i2c_write(
					client,
					reg_conf_tbl->reg_addr,
					reg_conf_tbl->reg_data, dt);
				break;
			
			default:
				pr_err("%s: Unsupport data type: %d\n",
					__func__, dt);
				break;
			}
		}
		if (rc < 0)
			break;
		reg_conf_tbl++;
	}
	return rc;
}

void hi707_sensor_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	hi707_camera_i2c_write_tbl(
	        s_ctrl->sensor_i2c_client,
	        s_ctrl->msm_sensor_reg->start_stream_conf,
	        s_ctrl->msm_sensor_reg->start_stream_conf_size,
	        s_ctrl->msm_sensor_reg->default_data_type);
}

void hi707_sensor_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	hi707_camera_i2c_write_tbl(
	        s_ctrl->sensor_i2c_client,
	        s_ctrl->msm_sensor_reg->stop_stream_conf,
	        s_ctrl->msm_sensor_reg->stop_stream_conf_size,
	        s_ctrl->msm_sensor_reg->default_data_type);
}

int32_t hi707_sensor_write_init_settings(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc;
	printk("%s : E\n", __func__); 
	/*
	rc = msm_sensor_write_all_conf_array(
		s_ctrl->sensor_i2c_client,
		&(s_ctrl->msm_sensor_reg->init_settings[hi707_antibanding]),
		s_ctrl->msm_sensor_reg->init_size);
	*/
	rc = msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			s_ctrl->msm_sensor_reg->init_settings, hi707_antibanding);
	
	printk("%s : X\n", __func__); 
	return rc;
}

int32_t hi707_sensor_setting1(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;

	printk("%s : E\n", __func__); 
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		hi707_sensor_write_init_settings(s_ctrl); // msm_sensor_write_init_settings(s_ctrl);
		csi_config = 0;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
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
		}
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE,
			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		msleep(50);
	}
	printk("%s : X\n", __func__); 
	return rc;
}

int32_t hi707_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	uint16_t chipid = 0;

	rc = msm_camera_i2c_write(
			s_ctrl->sensor_i2c_client,
			SENSOR_REG_PAGE_ADDR,
			SENSOR_REG_PAGE_0,
			MSM_CAMERA_I2C_BYTE_DATA);
	
	rc = msm_camera_i2c_read(
			s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_id_info->sensor_id_reg_addr, &chipid,
			MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
		return rc;
	}

	CDBG("hi707_sensor id: %d\n", chipid);
	if (chipid != s_ctrl->sensor_id_info->sensor_id) {
		pr_err("hi707_match chip id doesnot match\n");
		return -ENODEV;
	}
	return rc;
}

/* LGE_CHANGE_S : 2012-10-09 sungmin.cho@lge.com vt camera touch aec */
#define AEC_ROI_DX (192) // (128)
#define AEC_ROI_DY (192) // (128) // (96)
/*
// default 256 X 256
{0x68, 0x41}, //AE_CEN
{0x69, 0x81},
{0x6A, 0x38},
{0x6B, 0xb8},

{0x03, 0x20}, // register page 20
{0x68, (Xstartpoint/4)},
{0x69, (Xendpoint/4)},
{0x6a, (Ystartpoint/2)},
{0x6b, (Yendpoint/2)},
*/
static int8_t hi707_set_aec_roi(struct msm_sensor_ctrl_t *s_ctrl, int32_t aec_roi)
{
	int16_t coordinate_x, coordinate_y;
	int16_t x_start, x_end, y_start, y_end;
	int32_t rc = 0;

	coordinate_x = ((aec_roi >> 16)&0xFFFF);
	coordinate_y = aec_roi&0xFFFF;

	printk("%s : coordinate (%d, %d), front camera mirroring\n", __func__, coordinate_x, coordinate_y);

	if(coordinate_x == -1 && coordinate_y == -1) {
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
						SENSOR_REG_PAGE_ADDR,
						SENSOR_REG_PAGE_20,
						MSM_CAMERA_I2C_BYTE_DATA);
		/* AE weight */
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x60, 0x70, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x61, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x62, 0x70, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x63, 0x00, MSM_CAMERA_I2C_BYTE_DATA);

		/* AE window */
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x68, 0x41, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x69, 0x81, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x6a, 0x38, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x6b, 0xb8, MSM_CAMERA_I2C_BYTE_DATA);	

		if (rc < 0) {
				pr_err("%s: %s: failed\n", __func__,
					s_ctrl->sensordata->sensor_name);
				return rc;
		}
	}
	else {			
#ifdef CONFIG_HI707_ROT_180
		coordinate_x = SENSOR_PREVIEW_WIDTH - coordinate_x; 
		coordinate_y = SENSOR_PREVIEW_HEIGHT -coordinate_y;
#endif
		x_start = ((coordinate_x - (AEC_ROI_DX/2) > 0)? coordinate_x - (AEC_ROI_DX/2) : 0)/4;
		x_end = ((coordinate_x + (AEC_ROI_DX/2) < SENSOR_PREVIEW_WIDTH)? coordinate_x + (AEC_ROI_DX/2) : SENSOR_PREVIEW_WIDTH)/4;

		y_start = ((coordinate_y - (AEC_ROI_DY/2) > 0)? coordinate_y - (AEC_ROI_DY/2) : 0)/2;
		y_end = ((coordinate_y + (AEC_ROI_DY/2) < SENSOR_PREVIEW_HEIGHT)? coordinate_y + (AEC_ROI_DY/2) : SENSOR_PREVIEW_HEIGHT)/2;
		
		printk("%s : (%d, %d), (%d, %d)\n", __func__, x_start, y_start, x_end, y_end);
		
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, SENSOR_REG_PAGE_ADDR, SENSOR_REG_PAGE_20, MSM_CAMERA_I2C_BYTE_DATA);

		/* AE weight */
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x60, 0x70, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x61, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x62, 0x70, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x63, 0x00, MSM_CAMERA_I2C_BYTE_DATA);

		/* AE window */
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x68, x_start, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x69, x_end, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x6a, y_start, MSM_CAMERA_I2C_BYTE_DATA);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x6b, y_end, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) {
				pr_err("%s: %s: failed\n", __func__,
					s_ctrl->sensordata->sensor_name);
				return rc;
		}
	}
	return rc;
}
/* LGE_CHANGE_E : 2012-10-09 sungmin.cho@lge.com vt camera touch aec */

/* LGE_CHANGE_S : 2013-03-04 hyungtae.lee@lge.com Modified EXIF data for V7 */
int8_t hi707_get_snapshot_data(struct msm_sensor_ctrl_t *s_ctrl, struct snapshot_soc_data_cfg *snapshot_data) {
	int rc = 0;
	u16 analogGain = 0;
	u32 exposureTime = 0;
	u32 isoSpeed = 0;
	u16 Exposure1 = 0;
	u16 Exposure2 = 0;
	u16 Exposure3 = 0;
	int ExposureTotal = 0;
	
	//ISO Speed
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x03, 0x20, MSM_CAMERA_I2C_BYTE_DATA);	
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0xb0, &analogGain, MSM_CAMERA_I2C_BYTE_DATA);	

	if (rc < 0) {
		pr_err("%s: error to get analog & digital gain \n", __func__);
		return rc;
	}
	//Exposure Time
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x03, 0x20, MSM_CAMERA_I2C_BYTE_DATA);
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x80, &Exposure1, MSM_CAMERA_I2C_BYTE_DATA);
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x81, &Exposure2, MSM_CAMERA_I2C_BYTE_DATA);
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x82, &Exposure3, MSM_CAMERA_I2C_BYTE_DATA);

	
	if (rc < 0) {
		pr_err("%s: error to get exposure time \n", __func__);
		return rc;
	}

	if( analogGain <= 0x28 ){
		//printk("[CHECK]%s : iso speed - analogGain = 0x%x/n",  __func__, analogGain);
		analogGain = 0x28;  		//analogGain cannot move down than 0x28
	}
	//ISO speed
	isoSpeed = ((analogGain / 32) * 100);
	
	//Exposure Time
	ExposureTotal = ((Exposure1<<16) * 524288)|((Exposure2<<8) * 2048)|((Exposure3<<2) * 8);

	if (ExposureTotal <= 0) {
		exposureTime = 600000;
	}else {
	exposureTime = ExposureTotal;
	}

	snapshot_data->iso_speed = isoSpeed;
	snapshot_data->exposure_time = exposureTime;
	//printk("[CHECK]Camera Snapshot Data iso_speed = %d, exposure_time = %d \n", snapshot_data->iso_speed, snapshot_data->exposure_time); 

	return 0;
	
}
/* LGE_CHANGE_E : 2013-03-04 hyungtae.lee@lge.com Modified EXIF data for V7 */

static struct v4l2_subdev_video_ops hi707_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops hi707_subdev_ops = {
	.core = &hi707_subdev_core_ops,
	.video  = &hi707_subdev_video_ops,
};

static struct msm_sensor_fn_t hi707_func_tbl = {
	.sensor_start_stream = hi707_sensor_start_stream,
	.sensor_stop_stream = hi707_sensor_stop_stream,
	.sensor_csi_setting = hi707_sensor_setting1, // msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = hi707_sensor_power_up,
	.sensor_power_down = hi707_sensor_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
#ifdef CONFIG_MACH_LGE
//	.sensor_set_wb = hi707_set_wb,
//	.sensor_set_effect = hi707_set_effect,
//	.sensor_set_brightness = hi707_set_brightness,
//	.sensor_set_soc_minmax_fps = hi707_set_fps,
	.sensor_match_id = hi707_match_id,
	.sensor_set_aec_roi = hi707_set_aec_roi,
	.sensor_get_soc_snapshotdata = hi707_get_snapshot_data,	/* LGE_CHANGE: 2013-03-04 hyungtae.lee@lge.com Modified EXIF data for V7 */
#endif
};

static struct msm_sensor_reg_t hi707_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = hi707_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(hi707_start_settings),
	.stop_stream_conf = hi707_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(hi707_stop_settings),
	.init_settings = &hi707_init_conf[0],
	.init_size = ARRAY_SIZE(hi707_init_conf),
	.mode_settings = &hi707_confs[0],
	.no_effect_settings = &hi707_no_effect_confs[0],
	.output_settings = &hi707_dimensions[0],
	.num_conf = ARRAY_SIZE(hi707_confs),
};

static struct msm_sensor_ctrl_t hi707_s_ctrl = {
	.msm_sensor_reg = &hi707_regs,
	.msm_sensor_v4l2_ctrl_info = hi707_v4l2_ctrl_info,
	.num_v4l2_ctrl = ARRAY_SIZE(hi707_v4l2_ctrl_info),
	.sensor_i2c_client = &hi707_sensor_i2c_client,
	.sensor_i2c_addr = 0x60,
	.sensor_output_reg_addr = &hi707_reg_addr,
	.sensor_id_info = &hi707_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &hi707_csic_params_array[0],
	.msm_sensor_mutex = &hi707_mut,
	.sensor_i2c_driver = &hi707_i2c_driver,
	.sensor_v4l2_subdev_info = hi707_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(hi707_subdev_info),
	.sensor_v4l2_subdev_ops = &hi707_subdev_ops,
	.func_tbl = &hi707_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Hynix VGA sensor driver");
MODULE_LICENSE("GPL v2");

