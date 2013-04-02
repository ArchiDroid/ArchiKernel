#ifndef HI351_REG_H
#define HI351_REG_H

#include "msm_sensor.h"


static struct msm_camera_i2c_reg_conf hi351_no_active_settings[] = {
	{-1,-1,-1,-1,-1}
};

static struct msm_camera_i2c_reg_conf hi351_start_settings[] = {
	{ 0x03, 0x00},
	{ 0x01, 0xf0},
};

static struct msm_camera_i2c_reg_conf hi351_stop_settings[] = {
	{ 0x03, 0x00},
	{ 0x01, 0xf1},
};
static struct msm_camera_i2c_reg_conf hi351_prev_settings[] = {
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x01, 0xf1, MSM_CAMERA_I2C_BYTE_DATA}, //Sleep on

	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA}, //DMA&Adaptive Off
	{0x36, 0xa3, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0xc4, MSM_CAMERA_I2C_BYTE_DATA}, //AE off
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)
#else
	{0x10, 0x60, MSM_CAMERA_I2C_BYTE_DATA},
#endif
	{0x03, 0xc5, MSM_CAMERA_I2C_BYTE_DATA}, //AWB off
	{0x10, 0x30, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x19, MSM_CAMERA_I2C_BYTE_DATA}, //Scaler Setting
	{0x10, 0x87, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x13, MSM_CAMERA_I2C_BYTE_DATA}, //Sub1/2 + Pre2
	{0x13, 0x80, MSM_CAMERA_I2C_BYTE_DATA}, //Fix AE Set Off
	{0x14, 0x70, MSM_CAMERA_I2C_BYTE_DATA}, // for Pre2mode
	{0x03, 0xFE, MSM_CAMERA_I2C_BYTE_DATA},
	{0xFE, 0x0A, MSM_CAMERA_I2C_BYTE_DATA}, //Delay STEVE DV2 MUST WAIT 10msec

	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x20, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x21, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, //preview row start set.
	{0x03, 0x15, MSM_CAMERA_I2C_BYTE_DATA},  //Shading
	{0x10, 0x81, MSM_CAMERA_I2C_BYTE_DATA},
	{0x20, 0x04, MSM_CAMERA_I2C_BYTE_DATA},  //Shading Width 1024 (pre2)
	{0x21, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x22, 0x03, MSM_CAMERA_I2C_BYTE_DATA},  //Shading Height 768
	{0x23, 0x00, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x48, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x1C, MSM_CAMERA_I2C_BYTE_DATA}, //MIPI On
	{0x16, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x30, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //640 * 2
	{0x31, 0x08, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE for 1024x768 5-)8

	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x36, 0x28, MSM_CAMERA_I2C_BYTE_DATA}, //Preview set

	{0x03, 0xFE, MSM_CAMERA_I2C_BYTE_DATA},
	{0xFE, 0x0A, MSM_CAMERA_I2C_BYTE_DATA}, //Delay 10ms

	{0x03, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0xf0, 0x0d, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE Dark mode for Sawtooth

	{0x03, 0xc4, MSM_CAMERA_I2C_BYTE_DATA}, //AE en
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#else
	{0x10, 0xe1, MSM_CAMERA_I2C_BYTE_DATA},
#endif

	{0x03, 0xFE, MSM_CAMERA_I2C_BYTE_DATA},
	{0xFE, 0x0A, MSM_CAMERA_I2C_BYTE_DATA}, //Delay 10ms

	{0x03, 0xc5, MSM_CAMERA_I2C_BYTE_DATA}, //AWB en
	{0x10, 0xb1, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x01, 0xf0, MSM_CAMERA_I2C_BYTE_DATA}, //sleep off

	{0x03, 0xcf, MSM_CAMERA_I2C_BYTE_DATA}, //Adaptive On
	{0x10, 0xaf, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0xc0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x33, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x32, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, //DMA On



};

static struct msm_camera_i2c_reg_conf hi351_snap_settings[] = {

	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x01, 0xf1, MSM_CAMERA_I2C_BYTE_DATA}, //Sleep on

	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA}, //DMA&Adaptive Off
	{0x36, 0xa3, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0xc4, MSM_CAMERA_I2C_BYTE_DATA}, //AE off
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68, MSM_CAMERA_I2C_BYTE_DATA},  // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)
#else
	{0x10, 0x60, MSM_CAMERA_I2C_BYTE_DATA},
#endif
	{0x03, 0xc5, MSM_CAMERA_I2C_BYTE_DATA}, //AWB off
	{0x10, 0x30, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x19, MSM_CAMERA_I2C_BYTE_DATA}, //Scaler Off
	{0x10, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x14, 0x20, MSM_CAMERA_I2C_BYTE_DATA}, // for Full mode
	{0x10, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //Full
	{0x03, 0xFE, MSM_CAMERA_I2C_BYTE_DATA},
	{0xFE, 0x0A, MSM_CAMERA_I2C_BYTE_DATA}, //Delay STEVE DV2 MUST WAIT 10msec - change to 8ms in code which measured actual 10 ms delay
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x20, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x21, 0x03, MSM_CAMERA_I2C_BYTE_DATA}, //preview row start set.
	{0x03, 0x15, MSM_CAMERA_I2C_BYTE_DATA}, //Shading
	{0x10, 0x83, MSM_CAMERA_I2C_BYTE_DATA}, // 00 -> 83 LSC ON
	{0x20, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x21, 0xf8, MSM_CAMERA_I2C_BYTE_DATA},
	{0x22, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x23, 0xf8, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0xf0, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE Dark mode for Sawtooth

	{0x03, 0x48, MSM_CAMERA_I2C_BYTE_DATA}, //MIPI Setting
	{0x10, 0x1C, MSM_CAMERA_I2C_BYTE_DATA},
	{0x16, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x30, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //2048 * 2
	{0x31, 0x10, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x36, 0x29, MSM_CAMERA_I2C_BYTE_DATA}, //Capture
	{0x03, 0xFE, MSM_CAMERA_I2C_BYTE_DATA},
	{0xFE, 0x14, MSM_CAMERA_I2C_BYTE_DATA}, //Delay 20ms	16 ms delay but actual delay in measurement is 20 ms

	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x01, 0xf0, MSM_CAMERA_I2C_BYTE_DATA}, //sleep off

	{0x03, 0xcf, MSM_CAMERA_I2C_BYTE_DATA}, //Adaptive On
	{0x10, 0xaf, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0xc0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x33, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x32, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, //DMA On


};


//DONE
static struct msm_camera_i2c_reg_conf hi351_exposure[][3] = {
// need to check
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0x38},}, /*EXPOSURECOMPENSATIONN6*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0x48},}, /*EXPOSURECOMPENSATIONN5*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0x58},}, /*EXPOSURECOMPENSATIONN4*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0x68},}, /*EXPOSURECOMPENSATIONN3*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0x70},}, /*EXPOSURECOMPENSATIONN2*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0x78},}, /*EXPOSURECOMPENSATIONN1*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0x80},}, /*EXPOSURECOMPENSATIOND*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0x88},}, /*EXPOSURECOMPENSATIONp1*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0x90},}, /*EXPOSURECOMPENSATIONp2*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0x98},}, /*EXPOSURECOMPENSATIONp3*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0xa8},}, /*EXPOSURECOMPENSATIONp4*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0xb8},}, /*EXPOSURECOMPENSATIONp5*/
	{{0x03, 0x10}, {0x13, 0x0a}, {0x4A, 0xc8},}, /*EXPOSURECOMPENSATIONp6*/
};
//DONE
/*
static struct msm_camera_i2c_reg_conf hi351_no_effect[] = {
	//Filter OFF
	{0x03, 0x10},
	{0x12, 0x10}, //constant OFF
	{0x44, 0x80}, //cb_constant
	{0x45, 0x80}, //cr_constant
};
*/

static struct msm_camera_i2c_reg_conf hi351_special_effect[][4] = {
// need to check
	{
		//Filter OFF
		{0x03, 0x10},
		{0x12, 0x10}, //constant OFF
		{0x44, 0x80}, //cb_constant
		{0x45, 0x80}, //cr_constant
	},	/*for special effect OFF*/
	{
		// Mono
		{0x03, 0x10},
		{0x12, 0x13}, //constant ON
		{0x44, 0x80}, //cb_constant
		{0x45, 0x80}, //cr_constant
	},	/*for special effect MONO*/
	{
		//Negative
		{0x03, 0x10},
		{0x12, 0x18}, //constant OFF
		{0x44, 0x80}, //cb_constant
		{0x45, 0x80}, //cr_constant
	},	/*for special efefct Negative*/
	{
		//Sepia
		{0x03, 0x10},
		{0x12, 0x13}, //constant ON
		{0x44, 0x60}, //cb_constant
		{0x45, 0xA3}, //cr_constant},
	},/*for sepia*/
	{
		{-1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1},
	},		/*MAX value*/
};

static struct msm_camera_i2c_reg_conf hi351_wb_oem[][54] = {
// need to check
	{
		{0x03, 0xc5},
		{0x10, 0x30}, //AWB Off

		//AWB target RG angle
		{0xaa, 0x28},//aInRgTgtOfs_a00_n00
		{0xab, 0x1e},//aInRgTgtOfs_a01_n00
		{0xac, 0x14},//aInRgTgtOfs_a02_n00
		{0xad, 0x0a},//aInRgTgtOfs_a03_n00
		{0xae, 0x00},//aInRgTgtOfs_a04_n00
		{0xaf, 0x81},//aInRgTgtOfs_a05_n00
		{0xb0, 0x84},//aInRgTgtOfs_a06_n00
		{0xb1, 0x85},//aInRgTgtOfs_a07_n00

		//AWB target BG angle
		{0xb2, 0xa8},//aInBgTgtOfs_a00_n00
		{0xb3, 0x9e},//aInBgTgtOfs_a01_n00
		{0xb4, 0x94},//aInBgTgtOfs_a02_n00
		{0xb5, 0x8a},//aInBgTgtOfs_a03_n00
		{0xb6, 0x00},//aInBgTgtOfs_a04_n00
		{0xb7, 0x01},//aInBgTgtOfs_a05_n00
		{0xb8, 0x04},//aInBgTgtOfs_a06_n00
		{0xb9, 0x05},//aInBgTgtOfs_a07_n00

		{0x03, 0xc6},
		{0x18, 0x40},//bInRgainMin_a00_n00
		{0x19, 0xf0},//bInRgainMax_a00_n00
		{0x1a, 0x40},//bInBgainMin_a00_n00
		{0x1b, 0xf0},//bInBgainMax_a00_n00

		{0xb9, 0x58},//bOutRgainMin_a00_n00
		{0xba, 0xf0},//bOutRgainMax_a00_n00
		{0xbb, 0x40},//bOutBgainMin_a00_n00
		{0xbc, 0x90},//bOutBgainMax_a00_n00

		{0x03, 0xc5},
		{0x10, 0xb1},//AWB On

		///////////////////////////////////////////
		// D1 Page Adaptive R/B saturation
		///////////////////////////////////////////
		{0x03, 0xd1},//Page d1

		//Cb
		{0x2b, 0x88}, //SATB_00 STEVE for Low Light
		{0x2c, 0x88}, //SATB_01 STEVE for Low Light
		{0x2d, 0x88}, //SATB_02 STEVE for Low Light
		{0x2e, 0x98}, //SATB_03
		{0x2f, 0x98}, //SATB_04
		{0x30, 0x98}, //SATB_05
		{0x31, 0xa0}, //SATB_06
		{0x32, 0xa0}, //SATB_07
		{0x33, 0xa0}, //SATB_08
		{0x34, 0xa8}, //SATB_09
		{0x35, 0xa8}, //SATB_10
		{0x36, 0xa8}, //SATB_11

		//Cr

		{0x37, 0x88},//SATR_00 STEVE for Low Light
		{0x38, 0x88},//SATR_01 STEVE for Low Light
		{0x39, 0x90},//SATR_02 STEVE for Low Light
		{0x3a, 0x98},//SATR_03
		{0x3b, 0x98},//SATR_04
		{0x3c, 0x98},//SATR_05
		{0x3d, 0xa0},//SATR_06
		{0x3e, 0xa0},//SATR_07
		{0x3f, 0xa0},//SATR_08
		{0x40, 0xa8},//SATR_09
		{0x41, 0xa8},//SATR_10
		{0x42, 0xa8},//SATR_11
	}, /*WHITEBALNACE AUTO*/
	{
		// case CAM_WB_INCANDESCENCE:
		{0x03, 0xc5},
		{0x10, 0x30},//AWB Off

		//AWB target RG angle
		{0xaa, 0x00},//aInRgTgtOfs_a00_n00
		{0xab, 0x00},//aInRgTgtOfs_a01_n00
		{0xac, 0x00},//aInRgTgtOfs_a02_n00
		{0xad, 0x00},//aInRgTgtOfs_a03_n00
		{0xae, 0x00},//aInRgTgtOfs_a04_n00
		{0xaf, 0x00},//aInRgTgtOfs_a05_n00
		{0xb0, 0x00},//aInRgTgtOfs_a06_n00
		{0xb1, 0x00},//aInRgTgtOfs_a07_n00

		//AWB target BG angle
		{0xb2, 0x00},//aInBgTgtOfs_a00_n00
		{0xb3, 0x00},//aInBgTgtOfs_a01_n00
		{0xb4, 0x00},//aInBgTgtOfs_a02_n00
		{0xb5, 0x00},//aInBgTgtOfs_a03_n00
		{0xb6, 0x00},//aInBgTgtOfs_a04_n00
		{0xb7, 0x00},//aInBgTgtOfs_a05_n00
		{0xb8, 0x00},//aInBgTgtOfs_a06_n00
		{0xb9, 0x00},//aInBgTgtOfs_a07_n00

		{0x03, 0xc6},
		{0x18, 0x3C},//bInRgainMin_a00_n00
		{0x19, 0x59},//bInRgainMax_a00_n00
		{0x1a, 0x88},//bInBgainMin_a00_n00
		{0x1b, 0xA0},//bInBgainMax_a00_n00

		{0xb9, 0x3C},//bOutRgainMin_a00_n00
		{0xba, 0x59},//bOutRgainMax_a00_n00
		{0xbb, 0x88},//bOutBgainMin_a00_n00
		{0xbc, 0xA0},//bOutBgainMax_a00_n00

		{0x03, 0xc5},
		{0x10, 0xb1},//AWB On

		///////////////////////////////////////////
		// D1 Page Adaptive R/B saturation
		///////////////////////////////////////////
		{0x03, 0xd1},//Page d1

		//Cb
		{0x2b, 0x88}, //SATB_00 STEVE for Low Light
		{0x2c, 0x88}, //SATB_01 STEVE for Low Light
		{0x2d, 0x88}, //SATB_02 STEVE for Low Light
		{0x2e, 0xd0}, //SATB_03
		{0x2f, 0xd0}, //SATB_04
		{0x30, 0xd0}, //SATB_05
		{0x31, 0xd0}, //SATB_06
		{0x32, 0xd0}, //SATB_07
		{0x33, 0xd0}, //SATB_08
		{0x34, 0xd0}, //SATB_09
		{0x35, 0xd0}, //SATB_10
		{0x36, 0xd0}, //SATB_11

		//Cr

		{0x37, 0x88},//SATR_00 STEVE for Low Light
		{0x38, 0x88},//SATR_01 STEVE for Low Light
		{0x39, 0x90},//SATR_02 STEVE for Low Light
		{0x3a, 0xb0},//SATR_03
		{0x3b, 0xb0},//SATR_04
		{0x3c, 0xb0},//SATR_05
		{0x3d, 0xb0},//SATR_06
		{0x3e, 0xb0},//SATR_07
		{0x3f, 0xb0},//SATR_08
		{0x40, 0xb8},//SATR_09
		{0x41, 0xb8},//SATR_10
		{0x42, 0xb8},//SATR_11

	},	/*INCANDISCENT*/
	{
		//	DISP_NAME = " Florecent(tl84)"
		// case CAM_WB_FLUORESCENT:
		{0x03, 0xc5},
		{0x10, 0x30},//AWB Off

		//AWB target RG angle
		{0xaa, 0x28},//aInRgTgtOfs_a00_n00
		{0xab, 0x1e},//aInRgTgtOfs_a01_n00
		{0xac, 0x14},//aInRgTgtOfs_a02_n00
		{0xad, 0x0a},//aInRgTgtOfs_a03_n00
		{0xae, 0x00},//aInRgTgtOfs_a04_n00
		{0xaf, 0x81},//aInRgTgtOfs_a05_n00
		{0xb0, 0x84},//aInRgTgtOfs_a06_n00
		{0xb1, 0x85},//aInRgTgtOfs_a07_n00

		//AWB target BG angle
		{0xb2, 0xa8},//aInBgTgtOfs_a00_n00
		{0xb3, 0x9e},//aInBgTgtOfs_a01_n00
		{0xb4, 0x94},//aInBgTgtOfs_a02_n00
		{0xb5, 0x8a},//aInBgTgtOfs_a03_n00
		{0xb6, 0x00},//aInBgTgtOfs_a04_n00
		{0xb7, 0x01},//aInBgTgtOfs_a05_n00
		{0xb8, 0x04},//aInBgTgtOfs_a06_n00
		{0xb9, 0x05},//aInBgTgtOfs_a07_n00

		{0x03, 0xc6},
		{0x18, 0x43},//bInRgainMin_a00_n00
		{0x19, 0x60},//bInRgainMax_a00_n00
		{0x1a, 0x69},//bInBgainMin_a00_n00
		{0x1b, 0x90},//bInBgainMax_a00_n00

		{0xb9, 0x43},//bOutRgainMin_a00_n00
		{0xba, 0x60},//bOutRgainMax_a00_n00
		{0xbb, 0x69},//bOutBgainMin_a00_n00
		{0xbc, 0x90},//bOutBgainMax_a00_n00

		{0x03, 0xc5},
		{0x10, 0xb1},//AWB On

		///////////////////////////////////////////
		// D1 Page Adaptive R/B saturation
		///////////////////////////////////////////
		{0x03, 0xd1},//Page d1

		//Cb
		{0x2b, 0x88}, //SATB_00 STEVE for Low Light
		{0x2c, 0x88}, //SATB_01 STEVE for Low Light
		{0x2d, 0x88}, //SATB_02 STEVE for Low Light
		{0x2e, 0x98}, //SATB_03
		{0x2f, 0x98}, //SATB_04
		{0x30, 0x98}, //SATB_05
		{0x31, 0xa0}, //SATB_06
		{0x32, 0xa0}, //SATB_07
		{0x33, 0xa0}, //SATB_08
		{0x34, 0xa8}, //SATB_09
		{0x35, 0xa8}, //SATB_10
		{0x36, 0xa8}, //SATB_11

		//Cr

		{0x37, 0x88},//SATR_00 STEVE for Low Light
		{0x38, 0x88},//SATR_01 STEVE for Low Light
		{0x39, 0x90},//SATR_02 STEVE for Low Light
		{0x3a, 0x98},//SATR_03
		{0x3b, 0x98},//SATR_04
		{0x3c, 0x98},//SATR_05
		{0x3d, 0xa0},//SATR_06
		{0x3e, 0xa0},//SATR_07
		{0x3f, 0xa0},//SATR_08
		{0x40, 0xa8},//SATR_09
		{0x41, 0xa8},//SATR_10
		{0x42, 0xa8},//SATR_11
	},	/*FLOURESECT */
	{
		{0x03, 0xc5},
		{0x10, 0x30},//AWB Off

		//AWB target RG angle
		{0xaa, 0x28},//aInRgTgtOfs_a00_n00
		{0xab, 0x1e},//aInRgTgtOfs_a01_n00
		{0xac, 0x14},//aInRgTgtOfs_a02_n00
		{0xad, 0x0a},//aInRgTgtOfs_a03_n00
		{0xae, 0x00},//aInRgTgtOfs_a04_n00
		{0xaf, 0x81},//aInRgTgtOfs_a05_n00
		{0xb0, 0x84},//aInRgTgtOfs_a06_n00
		{0xb1, 0x85},//aInRgTgtOfs_a07_n00

		//AWB target BG angle
		{0xb2, 0xa8},//aInBgTgtOfs_a00_n00
		{0xb3, 0x9e},//aInBgTgtOfs_a01_n00
		{0xb4, 0x94},//aInBgTgtOfs_a02_n00
		{0xb5, 0x8a},//aInBgTgtOfs_a03_n00
		{0xb6, 0x00},//aInBgTgtOfs_a04_n00
		{0xb7, 0x01},//aInBgTgtOfs_a05_n00
		{0xb8, 0x04},//aInBgTgtOfs_a06_n00
		{0xb9, 0x05},//aInBgTgtOfs_a07_n00

		{0x03, 0xc6},
		{0x18, 0x4B},//bInRgainMin_a00_n00
		{0x19, 0x6A},//bInRgainMax_a00_n00
		{0x1a, 0x5d},//bInBgainMin_a00_n00
		{0x1b, 0x78},//bInBgainMax_a00_n00

		{0xb9, 0x4B},//bOutRgainMin_a00_n00
		{0xba, 0x6A},//bOutRgainMax_a00_n00
		{0xbb, 0x5d},//bOutBgainMin_a00_n00
		{0xbc, 0x78},//bOutBgainMax_a00_n00

		{0x03, 0xc5},
		{0x10, 0xb1},//AWB On

		///////////////////////////////////////////
		// D1 Page Adaptive R/B saturation
		///////////////////////////////////////////
		{0x03, 0xd1},//Page d1

		//Cb
		{0x2b, 0x88}, //SATB_00 STEVE for Low Light
		{0x2c, 0x88}, //SATB_01 STEVE for Low Light
		{0x2d, 0x88}, //SATB_02 STEVE for Low Light
		{0x2e, 0x98}, //SATB_03
		{0x2f, 0x98}, //SATB_04
		{0x30, 0x98}, //SATB_05
		{0x31, 0xa0}, //SATB_06
		{0x32, 0xa0}, //SATB_07
		{0x33, 0xa0}, //SATB_08
		{0x34, 0xa8}, //SATB_09
		{0x35, 0xa8}, //SATB_10
		{0x36, 0xa8}, //SATB_11

		//Cr

		{0x37, 0x88},//SATR_00 STEVE for Low Light
		{0x38, 0x88},//SATR_01 STEVE for Low Light
		{0x39, 0x90},//SATR_02 STEVE for Low Light
		{0x3a, 0x98},//SATR_03
		{0x3b, 0x98},//SATR_04
		{0x3c, 0x98},//SATR_05
		{0x3d, 0xa0},//SATR_06
		{0x3e, 0xa0},//SATR_07
		{0x3f, 0xa0},//SATR_08
		{0x40, 0xa8},//SATR_09
		{0x41, 0xa8},//SATR_10
		{0x42, 0xa8},//SATR_11
	},	/*DAYLIGHT*/
	{
		{0x03, 0xc5},
		{0x10, 0x30},

		//AWB target RG angle
		{0xaa, 0x28},//aInRgTgtOfs_a00_n00
		{0xab, 0x1e},//aInRgTgtOfs_a01_n00
		{0xac, 0x14},//aInRgTgtOfs_a02_n00
		{0xad, 0x0a},//aInRgTgtOfs_a03_n00
		{0xae, 0x00},//aInRgTgtOfs_a04_n00
		{0xaf, 0x81},//aInRgTgtOfs_a05_n00
		{0xb0, 0x84},//aInRgTgtOfs_a06_n00
		{0xb1, 0x85},//aInRgTgtOfs_a07_n00

		//AWB target BG angle
		{0xb2, 0xa8},//aInBgTgtOfs_a00_n00
		{0xb3, 0x9e},//aInBgTgtOfs_a01_n00
		{0xb4, 0x94},//aInBgTgtOfs_a02_n00
		{0xb5, 0x8a},//aInBgTgtOfs_a03_n00
		{0xb6, 0x00},//aInBgTgtOfs_a04_n00
		{0xb7, 0x01},//aInBgTgtOfs_a05_n00
		{0xb8, 0x04},//aInBgTgtOfs_a06_n00
		{0xb9, 0x05},//aInBgTgtOfs_a07_n00

		{0x03, 0xc6},
		{0x18, 0x75},//bInRgainMin_a00_n00
		{0x19, 0x8F},//bInRgainMax_a00_n00
		{0x1a, 0x40},//bInBgainMin_a00_n00
		{0x1b, 0x56},//bInBgainMax_a00_n00

		{0xb9, 0x75},//bOutRgainMin_a00_n00
		{0xba, 0x8F},//bOutRgainMax_a00_n00
		{0xbb, 0x40},//bOutBgainMin_a00_n00
		{0xbc, 0x56},//bOutBgainMax_a00_n00

		{0x03, 0xc5},
		{0x10, 0xb1},

		///////////////////////////////////////////
		// D1 Page Adaptive R/B saturation
		///////////////////////////////////////////
		{0x03, 0xd1},//Page d1

		//Cb
		{0x2b, 0x88}, //SATB_00 STEVE for Low Light
		{0x2c, 0x88}, //SATB_01 STEVE for Low Light
		{0x2d, 0x88}, //SATB_02 STEVE for Low Light
		{0x2e, 0x98}, //SATB_03
		{0x2f, 0x98}, //SATB_04
		{0x30, 0x98}, //SATB_05
		{0x31, 0xa0}, //SATB_06
		{0x32, 0xa0}, //SATB_07
		{0x33, 0xa0}, //SATB_08
		{0x34, 0xa8}, //SATB_09
		{0x35, 0xa8}, //SATB_10
		{0x36, 0xa8}, //SATB_11

		//Cr

		{0x37, 0x88},//SATR_00 STEVE for Low Light
		{0x38, 0x88},//SATR_01 STEVE for Low Light
		{0x39, 0x90},//SATR_02 STEVE for Low Light
		{0x3a, 0x98},//SATR_03
		{0x3b, 0x98},//SATR_04
		{0x3c, 0x98},//SATR_05
		{0x3d, 0xa0},//SATR_06
		{0x3e, 0xa0},//SATR_07
		{0x3f, 0xa0},//SATR_08
		{0x40, 0xa8},//SATR_09
		{0x41, 0xa8},//SATR_10
		{0x42, 0xa8},//SATR_11
	},	/*CLOUDY*/
};

static struct msm_camera_i2c_reg_conf hi351_iso[][2] = {
	{
		{0x03, 0xD5},
		{0x10, 0x06}, // ISO Mode Select B[6:4] 000 Auto,001 ISO1,010 ISO2,011 ISO3,100 ISO4,101 ISO5,110 ISO6
	},   /*ISO_AUTO*/
	{
		{-1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1},
	},   /*ISO_DEBLUR*/
	{
		{0x03, 0xD5},
		{0x10, 0x16}, // ISO 1 control
	},   /*ISO_100*/
	{
		{0x03, 0xD5},
		{0x10, 0x26}, // ISO 2 control
	},   /*ISO_200*/
	{
		{0x03, 0xD5},
		{0x10, 0x36}, // ISO 3 control
	},   /*ISO_400*/
	{
		{0x03, 0xD5},
		{0x10, 0x46}, // ISO 4 control
	},   /*ISO_800*/

};


static struct msm_camera_i2c_reg_conf hi351_auto_fps_settings0[] = {

	//30~8fps
	{0x03, 0x00},
	{0x01, 0xf1}, //Sleep on

	{0x03, 0x30}, //DMA&Adaptive Off
	{0x36, 0xa3},

	{0x03, 0xc4}, //AE off
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68},  // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)
#else
	{0x10, 0x60},
#endif
	{0x03, 0xc5}, //AWB off
	{0x10, 0x30},

	{0x03, 0xe3}, // For Camera Spatial LPF design parameters
  	{0xae, 0x03}, //14 page
	{0xaf, 0x14},
	{0xb0, 0x10}, //Dark2 0x1410
	{0xb1, 0x00},
	{0xb2, 0x11}, //Dark2 0x1411
	{0xb3, 0x00},
	{0xb4, 0x12}, //Dark2 0x1412
	{0xb5, 0x40}, //Top H_Clip
	{0xb6, 0x13}, //Dark2 0x1413
	{0xb7, 0xc8},
	{0xb8, 0x14}, //Dark2 0x1414
	{0xb9, 0x50},
	{0xba, 0x15}, //Dark2 0x1415	   //sharp positive hi
	{0xbb, 0x19},
	{0xbc, 0x16}, //Dark2 0x1416	   //sharp positive mi
	{0xbd, 0x19},
	{0xbe, 0x17}, //Dark2 0x1417	   //sharp positive low
	{0xbf, 0x19},
	{0xc0, 0x18}, //Dark2 0x1418	   //sharp negative hi
	{0xc1, 0x33},
	{0xc2, 0x19}, //Dark2 0x1419	   //sharp negative mi
	{0xc3, 0x33},
	{0xc4, 0x1a}, //Dark2 0x141a	   //sharp negative low
	{0xc5, 0x33},
	{0xc6, 0x20}, //Dark2 0x1420
	{0xc7, 0x80},

	{0x03, 0x20}, //Page 20
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x24, 0x00}, //EXP Max 8.33 fps (STEVE FOR 50Hz)
	{0x25, 0x41},
	{0x26, 0xe4},
	{0x27, 0x40},
#else
	{0x24, 0x00}, //EXP Max 8.00 fps
	{0x25, 0x44},
	{0x26, 0xa3},
	{0x27, 0x18},
#endif
	{0x28, 0x00}, //EXPMin 25210.08 fps
	{0x29, 0x0b},
	{0x2a, 0x28},
	{0x30, 0x05}, //EXP100
	{0x31, 0x7d},
	{0x32, 0xb0},
	{0x33, 0x04}, //EXP120
	{0x34, 0x93},
	{0x35, 0x68},
	{0x36, 0x00}, //EXP Unit
	{0x37, 0x05},
	{0x38, 0x94},

	{0x03, 0x30},
	{0x36, 0x28}, //preview function

};

static struct msm_camera_i2c_reg_conf hi351_auto_fps_settings1[] = {

//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms

	{0x03, 0x00},
	{0x10, 0x13},  // scale
	{0x11, 0x83}, //

	{0x03, 0x18}, //Page 18
	{0xC4, 0x7e}, //FLK200
	{0xC5, 0x69}, //FLK240

	{0x03, 0x00}, //PAGE 0
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x90, 0x0c}, //BLC_TIME_TH_ON  // STEVE for 50Hz
	{0x91, 0x0c}, //BLC_TIME_TH_OFF // STEVE for 50Hz
	{0x92, 0xe8}, //BLC_AG_TH_ON    //// STEVE fixed AGC 0xD0
	{0x93, 0xe0}, //BLC_AG_TH_OFF   //// STEVE fixed AGC 0xD0

	//DCDC
	{0x03, 0x02}, //PAGE 2
	{0xd4, 0x0c}, //DCDC_TIME_TH_ON  // STEVE for 50Hz
	{0xd5, 0x0c}, //DCDC_TIME_TH_OFF // STEVE for 50Hz
#else
	{0x90, 0x0f}, //BLC_TIME_TH_ON  // STEVE fixed
	{0x91, 0x0f}, //BLC_TIME_TH_OFF // STEVE fixed
	{0x92, 0xe8}, //BLC_AG_TH_ON    //// STEVE fixed AGC 0xD0
	{0x93, 0xe0}, //BLC_AG_TH_OFF   //// STEVE fixed AGC 0xD0

	//DCDC
	{0x03, 0x02}, //PAGE 2
	{0xd4, 0x0f}, //DCDC_TIME_TH_ON
	{0xd5, 0x0f}, //DCDC_TIME_TH_OFF
#endif
	{0xd6, 0xe8}, //DCDC_AG_TH_ON
	{0xd7, 0xe0}, //DCDC_AG_TH_OFF

	{0x03, 0xcf}, // STEVE : EV max -> 94 b6 b4 [fixed frame : not enough int. time]
	{0x13, 0x02}, //Y_LUM_MAX 8fps, AG 0xF0
	{0x14, 0x60},
	{0x15, 0x00},
	{0x16, 0x00},

	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#else
	{0x10, 0xe1}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#endif

};

static struct msm_camera_i2c_reg_conf hi351_auto_fps_settings2[] = {

//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms
	{0x03, 0xc5}, //AWB en
	{0x10, 0xb1},

	{0x03, 0x00},
	{0x01, 0xf0}, //sleep off

	{0x03, 0xcf}, //Adaptive On
	{0x10, 0xaf},  // STEVE all on

	{0x03, 0xc0},
	{0x33, 0x00},
	{0x32, 0x01}, //DMA On

};

static struct msm_camera_i2c_reg_conf hi351_fixed_fps_settings0[] = {
// CAMCODER MODE : 30 ~ 22FPS
	{0x03, 0x00},
	{0x01, 0xf1}, //Sleep on

	{0x03, 0x30}, //DMA&Adaptive Off
	{0x36, 0xa3},

	{0x03, 0xc4}, //AE off
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68},  // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)
#else
	{0x10, 0x60},
#endif
	{0x03, 0xc5}, //AWB off
	{0x10, 0x30},

	{0x03, 0xe3}, //  For Camcording Spatial LPF design parameters
	{0xae, 0x03}, // dark2
	{0xaf, 0x14},
	{0xb0, 0x10},
	{0xb1, 0x27},
	{0xb2, 0x11},
	{0xb3, 0x0f},
	{0xb4, 0x12},
	{0xb5, 0x0f},
	{0xb6, 0x13},
	{0xb7, 0xa3},
	{0xb8, 0x14},
	{0xb9, 0x3c},
	{0xba, 0x15},
	{0xbb, 0x18},
	{0xbc, 0x16},
	{0xbd, 0x10},
	{0xbe, 0x17},
	{0xbf, 0x08},
	{0xc0, 0x18},
	{0xc1, 0x32},
	{0xc2, 0x19},
	{0xc3, 0x30},
	{0xc4, 0x1a},
	{0xc5, 0x30},
	{0xc6, 0x20},
	{0xc7, 0x80},

	{0x03, 0x20}, //Page 20
	{0x24, 0x00}, //EXP Max 24.00 fps
	{0x25, 0x16},
	{0x26, 0xE1},
	{0x27, 0x08},
	{0x28, 0x00}, //EXPMin 25210.08 fps
	{0x29, 0x0b},
	{0x2a, 0x28},
	{0x30, 0x05}, //EXP100
	{0x31, 0x7d},
	{0x32, 0xb0},
	{0x33, 0x04}, //EXP120
	{0x34, 0x93},
	{0x35, 0x68},
	{0x36, 0x00}, //EXP Unit
	{0x37, 0x05},
	{0x38, 0x94},

	{0x03, 0x30},
	{0x36, 0x28}, //preview function

};

static struct msm_camera_i2c_reg_conf hi351_fixed_fps_settings1[] = {

//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms

	{0x03, 0x00},
	{0x10, 0x13}, // scale
	{0x11, 0x83}, //

	{0x03, 0x18}, //Page 18
	{0xC4, 0x7e}, //FLK200
	{0xC5, 0x69}, //FLK240

	{0x03, 0x00}, //PAGE 0
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x90, 0x04}, //BLC_TIME_TH_ON  // STEVE for 50hz
	{0x91, 0x04}, //BLC_TIME_TH_OFF // STEVE for 50hz
	{0x92, 0xe8}, //BLC_AG_TH_ON  //STEVE AGC OxD0
	{0x93, 0xe0}, //BLC_AG_TH_OFF //STEVE AGC OxD0

	//DCDC
	{0x03, 0x02}, //PAGE 2
	{0xd4, 0x04}, //DCDC_TIME_TH_ON  // STEVE for 50hz
	{0xd5, 0x04}, //DCDC_TIME_TH_OFF // STEVE for 50hz
#else
	{0x90, 0x05}, //BLC_TIME_TH_ON
	{0x91, 0x05}, //BLC_TIME_TH_OFF
	{0x92, 0xe8}, //BLC_AG_TH_ON  //STEVE AGC OxD0
	{0x93, 0xe0}, //BLC_AG_TH_OFF //STEVE AGC OxD0

	//DCDC
	{0x03, 0x02}, //PAGE 2
	{0xd4, 0x05}, //DCDC_TIME_TH_ON
	{0xd5, 0x05}, //DCDC_TIME_TH_OFF
#endif
	{0xd6, 0xe8}, //DCDC_AG_TH_ON
	{0xd7, 0xe0}, //DCDC_AG_TH_OFF

	{0x03, 0xcf}, // STEVE : EV max -> 94 b6 b4 [fixed frame : not enough int. time]
	{0x13, 0x00}, //Y_LUM_MAX 10fps, AG 0xA0
	{0x14, 0xB0},
	{0x15, 0x00},
	{0x16, 0x00},

	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#else
	{0x10, 0xe1}, // STEVE AE en
#endif

};

static struct msm_camera_i2c_reg_conf hi351_fixed_fps_settings2[] = {


//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms

	{0x03, 0xc5}, //AWB en
	{0x10, 0xb1},

	{0x03, 0x00},
	{0x01, 0xf0}, //sleep off

	{0x03, 0xcf}, //Adaptive On
	{0x10, 0xaf},

	{0x03, 0xc0},
	{0x33, 0x00},
	{0x32, 0x01}, //DMA On
};

static struct msm_camera_i2c_reg_conf hi351_attached_fps_settings0[] = {

	{0x03, 0x00},
	{0x01, 0xf1}, //Sleep on

	{0x03, 0x30}, //DMA&Adaptive Off
	{0x36, 0xa3},

	{0x03, 0xc4}, //AE off
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68},  // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)
#else
	{0x10, 0x60},
#endif
	{0x03, 0xc5}, //AWB off
	{0x10, 0x30},

	{0x03, 0xe3}, //  For Camcording Spatial LPF design parameters
	{0xae, 0x03}, // dark2
	{0xaf, 0x14},
	{0xb0, 0x10},
	{0xb1, 0x27},
	{0xb2, 0x11},
	{0xb3, 0x0f},
	{0xb4, 0x12},
	{0xb5, 0x0f},
	{0xb6, 0x13},
	{0xb7, 0xa3},
	{0xb8, 0x14},
	{0xb9, 0x3c},
	{0xba, 0x15},
	{0xbb, 0x18},
	{0xbc, 0x16},
	{0xbd, 0x10},
	{0xbe, 0x17},
	{0xbf, 0x08},
	{0xc0, 0x18},
	{0xc1, 0x32},
	{0xc2, 0x19},
	{0xc3, 0x30},
	{0xc4, 0x1a},
	{0xc5, 0x30},
	{0xc6, 0x20},
	{0xc7, 0x80},

	{0x03, 0x20}, //Page 20
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x20, 0x00}, //EXP Normal 33.33 fps (STEVE for 50Hz)
	{0x21, 0x10},
	{0x22, 0x79},
	{0x23, 0x10},
	{0x24, 0x00}, //EXP Max 33.33 fps (STEVE for 50Hz)
	{0x25, 0x10},
	{0x26, 0x79},
	{0x27, 0x10},
#else
	{0x20, 0x00}, //EXP Normal 30.00 fps
	{0x21, 0x12},
	{0x22, 0x4d},
	{0x23, 0xa0},
	{0x24, 0x00}, //EXP Max 30.00 fps
	{0x25, 0x12},
	{0x26, 0x4d},
	{0x27, 0xa0},
#endif
	{0x28, 0x00}, //EXPMin 25210.08 fps
	{0x29, 0x0b},
	{0x2a, 0x28},
	{0x30, 0x05}, //EXP100
	{0x31, 0x7d},
	{0x32, 0xb0},
	{0x33, 0x04}, //EXP120
	{0x34, 0x93},
	{0x35, 0x68},
	{0x3c, 0x00}, //EXP Fix 15.01 fps
	{0x3d, 0x24},
	{0x3e, 0x9b},
	{0x3f, 0x40},
	{0x36, 0x00}, //EXP Unit
	{0x37, 0x05},
	{0x38, 0x94},

	{0x03, 0x30},
	{0x36, 0x28}, //preview function
};

static struct msm_camera_i2c_reg_conf hi351_attached_fps_settings1[] = {

//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms

	{0x03, 0x00},
	{0x10, 0x13}, // scale
	{0x11, 0x87}, //bit2 on

	{0x03, 0x18}, //Page 18
	{0xC4, 0x7e}, //FLK200
	{0xC5, 0x69}, //FLK240

	{0x03, 0x00}, //PAGE 0
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x90, 0x03}, //BLC_TIME_TH_ON  //STEVE for 50hz
	{0x91, 0x03}, //BLC_TIME_TH_OFF //STEVE for 50hz
	{0x92, 0xe8}, //BLC_AG_TH_ON    STEVE fixed AGC 0xD0
	{0x93, 0xe0}, //BLC_AG_TH_OFF   STEVE fixed AGC 0xD0

	//DCDC
	{0x03, 0x02}, //PAGE 2
	{0xd4, 0x03}, //DCDC_TIME_TH_ON  //STEVE for 50hz
	{0xd5, 0x03}, //DCDC_TIME_TH_OFF //STEVE for 50hz
#else
	{0x90, 0x04}, //BLC_TIME_TH_ON  STEVE fixed
	{0x91, 0x04}, //BLC_TIME_TH_OFF STEVE fixed
	{0x92, 0xe8}, //BLC_AG_TH_ON    STEVE fixed AGC 0xD0
	{0x93, 0xe0}, //BLC_AG_TH_OFF   STEVE fixed AGC 0xD0

	//DCDC
	{0x03, 0x02}, //PAGE 2
	{0xd4, 0x04}, //DCDC_TIME_TH_ON
	{0xd5, 0x04}, //DCDC_TIME_TH_OFF
#endif
	{0xd6, 0xe8}, //DCDC_AG_TH_ON
	{0xd7, 0xe0}, //DCDC_AG_TH_OFF

	{0x03, 0xcf}, // STEVE : EV max -> 76f890 [fixed frame : not enough int. time]
	{0x13, 0x00}, //Y_LUM_MAX 10fps, AG 0xA0
	{0x14, 0x88},
	{0x15, 0x00},
	{0x16, 0x00},

	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#else
	{0x10, 0xe1}, // STEVE AE ON
#endif

};

static struct msm_camera_i2c_reg_conf hi351_attached_fps_settings2[] = {

//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms

	{0x03, 0xc5}, //AWB en
	{0x10, 0xb1},

	{0x03, 0x00},
	{0x01, 0xf0}, //sleep off

	{0x03, 0xcf}, //Adaptive On
	{0x10, 0xaf},

	{0x03, 0xc0},
	{0x33, 0x00},
	{0x32, 0x01}, //DMA On

};


// Scene MODE

static struct msm_camera_i2c_reg_conf hi351_scene_active_settings[] = {
	//AWB On
	{0x03, 0xc5},
	{0x10, 0xb1},
};

static struct msm_camera_i2c_reg_conf hi351_scene_normal_settings[] = {


	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS Auto
	{0x03, 0x20},
	{0x24, 0x00}, //EXP Max 8.33 fps (STEVE FOR 50Hz)
	{0x25, 0x41},
	{0x26, 0xe4},
	{0x27, 0x40},
#else
	{0x10, 0x60}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS Auto
	{0x03, 0x20},
	{0x24, 0x00}, //EXP Max 8.00 fps
	{0x25, 0x44},
	{0x26, 0xa3},
	{0x27, 0x18},
#endif

	//Meteriyng - Center
	{0x03, 0xc3},

	{0x3A, 0x11},
	{0x3B, 0x11},
	{0x3C, 0x11},
	{0x3D, 0x11},
	{0x3E, 0x11},
	{0x3F, 0x11},
	{0x40, 0x11},
	{0x41, 0x11},
	{0x42, 0x11},
	{0x43, 0x11},
	{0x44, 0x11},
	{0x45, 0x11},
	{0x46, 0x11},
	{0x47, 0x11},
	{0x48, 0x11},
	{0x49, 0x11},
	{0x4A, 0x11},
	{0x4B, 0x11},
	{0x4C, 0x11},
	{0x4D, 0x21},
	{0x4E, 0x44},
	{0x4F, 0x44},
	{0x50, 0x12},
	{0x51, 0x11},
	{0x52, 0x11},
	{0x53, 0x22},
	{0x54, 0x44},
	{0x55, 0x44},
	{0x56, 0x22},
	{0x57, 0x11},
	{0x58, 0x21},
	{0x59, 0x44},
	{0x5A, 0x44},
	{0x5B, 0x44},
	{0x5C, 0x44},
	{0x5D, 0x12},
	{0x5E, 0x21},
	{0x5F, 0x44},
	{0x60, 0x44},
	{0x61, 0x44},
	{0x62, 0x44},
	{0x63, 0x12},
	{0x64, 0x21},
	{0x65, 0x44},
	{0x66, 0x44},
	{0x67, 0x44},
	{0x68, 0x44},
	{0x69, 0x12},
	{0x6A, 0x11},
	{0x6B, 0x11},
	{0x6C, 0x11},
	{0x6D, 0x11},
	{0x6E, 0x11},
	{0x6F, 0x11},

	//WB Auto
	{0x03, 0xc5}, //Page 0xc5
	{0x10, 0xb1}, //STEVE AWB on

	{0x03, 0xc6},
	{0x18, 0x40}, //bInRgainMin_a00_n00
	{0x19, 0xf0}, //bInRgainMax_a00_n00
	{0x1a, 0x40}, //bInBgainMin_a00_n00
	{0x1b, 0xf0}, //bInBgainMax_a00_n00

	{0xb9, 0x58}, //STEVE bOutRgainMin_a00_n00
	{0xba, 0xf0}, //bOutRgainMax_a00_n00
	{0xbb, 0x40}, //bOutBgainMin_a00_n00
	{0xbc, 0x90}, //STEVE bOutBgainMax_a00_n00


	//Saturation 0
	{0x03, 0xd1}, //page D1(Adaptive)
  // Cb
	{0x2b, 0x98}, //SATB_00 STEVE for Low Light
	{0x2c, 0x98}, //SATB_01 STEVE for Low Light
	{0x2d, 0x98}, //SATB_02 STEVE for Low Light
	{0x2e, 0x98}, //SATB_03
	{0x2f, 0x98}, //SATB_04
	{0x30, 0x98}, //SATB_05
	{0x31, 0xa0}, //SATB_06
	{0x32, 0xa0}, //SATB_07
	{0x33, 0xa0}, //SATB_08
	{0x34, 0xa8}, //SATB_09
	{0x35, 0xa8}, //SATB_10
	{0x36, 0xa8}, //SATB_11

	// Cr
	{0x37, 0x98}, //SATR_00 STEVE for Low Light
	{0x38, 0x98}, //SATR_01 STEVE for Low Light
	{0x39, 0xa0}, //SATR_02 STEVE for Low Light
	{0x3a, 0x98}, //SATR_03
	{0x3b, 0x98}, //SATR_04
	{0x3c, 0x98}, //SATR_05
	{0x3d, 0xa0}, //SATR_06
	{0x3e, 0xa0}, //SATR_07
	{0x3f, 0xa0}, //SATR_08
	{0x40, 0xa8}, //SATR_09
	{0x41, 0xa8}, //SATR_10
	{0x42, 0xa8}, //SATR_11
	{0x42, 0x98}, //SATR_11

	// STEVE Sharpness 0
	{0x03, 0xda},
	{0x1d, 0x20}, //outdoor

	{0x03, 0xdd},
	{0x1d, 0x20}, //indoor

	{0x03, 0xe0},
	{0x1d, 0x24}, //dark1

	{0x03, 0xe3},
	{0x1d, 0x26}, //dark2


	//AE On
	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#else
	{0x10, 0xe1},
#endif
//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms

};



static struct msm_camera_i2c_reg_conf hi351_scene_portrait_settings[] = {


	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS Auto
	{0x03, 0x20}, //Page 20
	{0x24, 0x00}, //EXP Max 8.33 fps (STEVE FOR 50Hz)
	{0x25, 0x41},
	{0x26, 0xe4},
	{0x27, 0x40},
#else
	{0x10, 0x60}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS Auto
	{0x03, 0x20},
	{0x24, 0x00}, //EXP Max 8.00 fps
	{0x25, 0x44},
	{0x26, 0xa3},
	{0x27, 0x18},
#endif

	//Metering - Center
	{0x03, 0xc3},

	{0x3A, 0x11},
	{0x3B, 0x11},
	{0x3C, 0x11},
	{0x3D, 0x11},
	{0x3E, 0x11},
	{0x3F, 0x11},
	{0x40, 0x11},
	{0x41, 0x11},
	{0x42, 0x11},
	{0x43, 0x11},
	{0x44, 0x11},
	{0x45, 0x11},
	{0x46, 0x11},
	{0x47, 0x11},
	{0x48, 0x11},
	{0x49, 0x11},
	{0x4A, 0x11},
	{0x4B, 0x11},
	{0x4C, 0x11},
	{0x4D, 0x21},
	{0x4E, 0x44},
	{0x4F, 0x44},
	{0x50, 0x12},
	{0x51, 0x11},
	{0x52, 0x11},
	{0x53, 0x22},
	{0x54, 0x44},
	{0x55, 0x44},
	{0x56, 0x22},
	{0x57, 0x11},
	{0x58, 0x21},
	{0x59, 0x44},
	{0x5A, 0x44},
	{0x5B, 0x44},
	{0x5C, 0x44},
	{0x5D, 0x12},
	{0x5E, 0x21},
	{0x5F, 0x44},
	{0x60, 0x44},
	{0x61, 0x44},
	{0x62, 0x44},
	{0x63, 0x12},
	{0x64, 0x21},
	{0x65, 0x44},
	{0x66, 0x44},
	{0x67, 0x44},
	{0x68, 0x44},
	{0x69, 0x12},
	{0x6A, 0x11},
	{0x6B, 0x11},
	{0x6C, 0x11},
	{0x6D, 0x11},
	{0x6E, 0x11},
	{0x6F, 0x11},


	//WB Auto
	{0x03, 0xc5}, //Page 0xc5
	{0x10, 0xb1}, //STEVE AWB on

	{0x03, 0xc6},
	{0x18, 0x40}, //bInRgainMin_a00_n00
	{0x19, 0xf0}, //bInRgainMax_a00_n00
	{0x1a, 0x40}, //bInBgainMin_a00_n00
	{0x1b, 0xf0}, //bInBgainMax_a00_n00

	{0xb9, 0x58}, //bOutRgainMin_a00_n00
	{0xba, 0xf0}, //bOutRgainMax_a00_n00
	{0xbb, 0x40}, //bOutBgainMin_a00_n00
	{0xbc, 0x90}, //bOutBgainMax_a00_n00


	//Saturation 0
	{0x03, 0xd1}, //page D1(Adaptive)

	// STEVE SATB
	{0x2b, 0x78},//SATB_00"
	{0x2c, 0x78},//SATB_01"
	{0x2d, 0x78},//SATB_02"
	{0x2e, 0x80},//SATB_03"
	{0x2f, 0x80},//SATB_04"
	{0x30, 0x80},//SATB_05"
	{0x31, 0x88},//SATB_06"
	{0x32, 0x88},//SATB_07"
	{0x33, 0x88},//SATB_08"
	{0x34, 0x88},//SATB_09"
	{0x35, 0x88},//SATB_10"
	{0x36, 0x88},//SATB_11"

	// STEVE SATR
	{0x37, 0x78},//SATR_00"
	{0x38, 0x78},//SATR_01"
	{0x39, 0x78},//SATR_02"
	{0x3a, 0x80},//SATR_03"
	{0x3b, 0x80},//SATR_04"
	{0x3c, 0x80},//SATR_05"
	{0x3d, 0x88},//SATR_06"
	{0x3e, 0x88},//SATR_07"
	{0x3f, 0x88},//SATR_08"
	{0x40, 0x88},//SATR_09"
	{0x41, 0x88},//SATR_10"
	{0x42, 0x88},//SATR_11"

	//Sharpness -1
	{0x03, 0xda},
	{0x1d, 0x24},//outdoor

	{0x03, 0xdd},
	{0x1d, 0x24},//indoor

	{0x03, 0xe0},
	{0x1d, 0x26},//dark1

	{0x03, 0xe3},
	{0x1d, 0x26},//dark2


	//AE On
	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#else
	{0x10, 0xe1},
#endif
//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms

};

static struct msm_camera_i2c_reg_conf hi351_scene_landscape_settings[] = {


	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS Auto
	{0x03, 0x20}, //Page 20
	{0x24, 0x00}, //EXP Max 8.33 fps (STEVE FOR 50Hz)
	{0x25, 0x41},
	{0x26, 0xe4},
	{0x27, 0x40},
#else
	{0x10, 0x60}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS Auto
	{0x03, 0x20},
	{0x24, 0x00}, //EXP Max 8.00 fps
	{0x25, 0x44},
	{0x26, 0xa3},
	{0x27, 0x18},
#endif

	//Metering - Matrix
	{0x03, 0xc3},

	{0x3A, 0x11},
	{0x3B, 0x11},
	{0x3C, 0x11},
	{0x3D, 0x11},
	{0x3E, 0x11},
	{0x3F, 0x11},
	{0x40, 0x11},
	{0x41, 0x11},
	{0x42, 0x11},
	{0x43, 0x11},
	{0x44, 0x11},
	{0x45, 0x11},
	{0x46, 0x11},
	{0x47, 0x11},
	{0x48, 0x11},
	{0x49, 0x11},
	{0x4A, 0x11},
	{0x4B, 0x11},
	{0x4C, 0x11},
	{0x4D, 0x11},
	{0x4E, 0x11},
	{0x4F, 0x11},
	{0x50, 0x11},
	{0x51, 0x11},
	{0x52, 0x11},
	{0x53, 0x11},
	{0x54, 0x11},
	{0x55, 0x11},
	{0x56, 0x11},
	{0x57, 0x11},
	{0x58, 0x11},
	{0x59, 0x11},
	{0x5A, 0x11},
	{0x5B, 0x11},
	{0x5C, 0x11},
	{0x5D, 0x11},
	{0x5E, 0x11},
	{0x5F, 0x11},
	{0x60, 0x11},
	{0x61, 0x11},
	{0x62, 0x11},
	{0x63, 0x11},
	{0x64, 0x11},
	{0x65, 0x11},
	{0x66, 0x11},
	{0x67, 0x11},
	{0x68, 0x11},
	{0x69, 0x11},
	{0x6A, 0x11},
	{0x6B, 0x11},
	{0x6C, 0x11},
	{0x6D, 0x11},
	{0x6E, 0x11},
	{0x6F, 0x11},

	//WB Auto
	{0x03, 0xc5}, //Page 0xc5
	{0x10, 0xb1}, //STEVE AWB on

	{0x03, 0xc6},
	{0x18, 0x40}, //bInRgainMin_a00_n00
	{0x19, 0xf0}, //bInRgainMax_a00_n00
	{0x1a, 0x40}, //bInBgainMin_a00_n00
	{0x1b, 0xf0}, //bInBgainMax_a00_n00

	{0xb9, 0x58}, //bOutRgainMin_a00_n00
	{0xba, 0xf0}, //bOutRgainMax_a00_n00
	{0xbb, 0x40}, //bOutBgainMin_a00_n00
	{0xbc, 0x90}, //bOutBgainMax_a00_n00


	//Saturation 1
	{0x03, 0xd1}, //page D1(Adaptive)

	//SATB
	{0x2b, 0x80},//SATB_00"
	{0x2c, 0x80},//SATB_01"
	{0x2d, 0x80},//SATB_02"
	{0x2e, 0xaa},//SATB_03"
	{0x2f, 0xa4},//SATB_04"
	{0x30, 0xa4},//SATB_05"
	{0x31, 0xa8},//SATB_06"
	{0x32, 0xa8},//SATB_07"
	{0x33, 0xa8},//SATB_08"
	{0x34, 0xa8},//SATB_09"
	{0x35, 0xa8},//SATB_10"
	{0x36, 0xa8},//SATB_11"

	//SATR
	{0x37, 0x80},//SATR_00"
	{0x38, 0x80},//SATR_01"
	{0x39, 0x80},//SATR_02"
	{0x3a, 0x9a},//SATR_03"
	{0x3b, 0x98},//SATR_04"
	{0x3c, 0x98},//SATR_05"
	{0x3d, 0x9c},//SATR_06"
	{0x3e, 0x9c},//SATR_07"
	{0x3f, 0x9c},//SATR_08"
	{0x40, 0x9c},//SATR_09"
	{0x41, 0x9c},//SATR_10"
	{0x42, 0x9c},//SATR_11"

	//Sharpness 1
	{0x03, 0xda},
	{0x1d, 0x1f},//outdoor

	{0x03, 0xdd},
	{0x1d, 0x1f},//indoor

	{0x03, 0xe0},
	{0x1d, 0x22},//dark1

	{0x03, 0xe3},
	{0x1d, 0x24},//dark2


	//AE On
	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#else
	{0x10, 0xe1},
#endif
//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms

};

static struct msm_camera_i2c_reg_conf hi351_scene_sport_settings[] = {


	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS Sports
	{0x03, 0x20},
	{0x24, 0x00}, //EXP Max 30.00 fps
	{0x25, 0x15},
	{0x26, 0xf6},
	{0x27, 0xc0},
#else
	{0x10, 0x60}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS Sports
	{0x03, 0x20},
	{0x24, 0x00}, //EXP Max 30.00 fps
	{0x25, 0x16},
	{0x26, 0xE1},
	{0x27, 0x08},
#endif

	//Metering - Center
	{0x03, 0xc3},

	{0x3A, 0x11},
	{0x3B, 0x11},
	{0x3C, 0x11},
	{0x3D, 0x11},
	{0x3E, 0x11},
	{0x3F, 0x11},
	{0x40, 0x11},
	{0x41, 0x11},
	{0x42, 0x11},
	{0x43, 0x11},
	{0x44, 0x11},
	{0x45, 0x11},
	{0x46, 0x11},
	{0x47, 0x11},
	{0x48, 0x11},
	{0x49, 0x11},
	{0x4A, 0x11},
	{0x4B, 0x11},
	{0x4C, 0x11},
	{0x4D, 0x21},
	{0x4E, 0x44},
	{0x4F, 0x44},
	{0x50, 0x12},
	{0x51, 0x11},
	{0x52, 0x11},
	{0x53, 0x22},
	{0x54, 0x44},
	{0x55, 0x44},
	{0x56, 0x22},
	{0x57, 0x11},
	{0x58, 0x21},
	{0x59, 0x44},
	{0x5A, 0x44},
	{0x5B, 0x44},
	{0x5C, 0x44},
	{0x5D, 0x12},
	{0x5E, 0x21},
	{0x5F, 0x44},
	{0x60, 0x44},
	{0x61, 0x44},
	{0x62, 0x44},
	{0x63, 0x12},
	{0x64, 0x21},
	{0x65, 0x44},
	{0x66, 0x44},
	{0x67, 0x44},
	{0x68, 0x44},
	{0x69, 0x12},
	{0x6A, 0x11},
	{0x6B, 0x11},
	{0x6C, 0x11},
	{0x6D, 0x11},
	{0x6E, 0x11},
	{0x6F, 0x11},

	//WB Auto
	{0x03, 0xc5}, //Page 0xc5
	{0x10, 0xb1}, //STEVE AWB on

	{0x03, 0xc6},
	{0x18, 0x40}, //bInRgainMin_a00_n00
	{0x19, 0xf0}, //bInRgainMax_a00_n00
	{0x1a, 0x40}, //bInBgainMin_a00_n00
	{0x1b, 0xf0}, //bInBgainMax_a00_n00

	{0xb9, 0x58}, //bOutRgainMin_a00_n00
	{0xba, 0xf0}, //bOutRgainMax_a00_n00
	{0xbb, 0x40}, //bOutBgainMin_a00_n00
	{0xbc, 0x90}, //bOutBgainMax_a00_n00


	//Saturation 0
	{0x03, 0xd1}, //page D1(Adaptive)

	// STEVE SATB
	{0x2b, 0x70},//SATB_00"
	{0x2c, 0x70},//SATB_01"
	{0x2d, 0x70},//SATB_02"
	{0x2e, 0x80},//SATB_03"
	{0x2f, 0x80},//SATB_04"
	{0x30, 0x80},//SATB_05"
	{0x31, 0x88},//SATB_06"
	{0x32, 0x88},//SATB_07"
	{0x33, 0x88},//SATB_08"
	{0x34, 0x88},//SATB_09"
	{0x35, 0x88},//SATB_10"
	{0x36, 0x88},//SATB_11"

	// STEVE SATR
	{0x37, 0x70},//SATR_00"
	{0x38, 0x70},//SATR_01"
	{0x39, 0x70},//SATR_02"
	{0x3a, 0x80},//SATR_03"
	{0x3b, 0x80},//SATR_04"
	{0x3c, 0x80},//SATR_05"
	{0x3d, 0x88},//SATR_06"
	{0x3e, 0x88},//SATR_07"
	{0x3f, 0x88},//SATR_08"
	{0x40, 0x88},//SATR_09"
	{0x41, 0x88},//SATR_10"
	{0x42, 0x88},//SATR_11"

	// STEVE Sharpness 0
	{0x03, 0xda},
	{0x1d, 0x20},//outdoor

	{0x03, 0xdd},
	{0x1d, 0x20},//indoor

	{0x03, 0xe0},
	{0x1d, 0x24},//dark1

	{0x03, 0xe3},
	{0x1d, 0x26},//dark2


	//AE On
	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#else
	{0x10, 0xe1},
#endif
//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms

};

static struct msm_camera_i2c_reg_conf hi351_scene_sunset_settings[] = {

	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS Auto
	{0x03, 0x20},
	{0x24, 0x00}, //EXP Max 8.33 fps (STEVE FOR 50Hz)
	{0x25, 0x41},
	{0x26, 0xe4},
	{0x27, 0x40},
#else
	{0x10, 0x60}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS Auto
	{0x03, 0x20},
	{0x24, 0x00}, //EXP Max 8.00 fps
	{0x25, 0x44},
	{0x26, 0xa3},
	{0x27, 0x18},
#endif

	//Metering - Center
	{0x03, 0xc3},

	{0x3A, 0x11},
	{0x3B, 0x11},
	{0x3C, 0x11},
	{0x3D, 0x11},
	{0x3E, 0x11},
	{0x3F, 0x11},
	{0x40, 0x11},
	{0x41, 0x11},
	{0x42, 0x11},
	{0x43, 0x11},
	{0x44, 0x11},
	{0x45, 0x11},
	{0x46, 0x11},
	{0x47, 0x11},
	{0x48, 0x11},
	{0x49, 0x11},
	{0x4A, 0x11},
	{0x4B, 0x11},
	{0x4C, 0x11},
	{0x4D, 0x21},
	{0x4E, 0x44},
	{0x4F, 0x44},
	{0x50, 0x12},
	{0x51, 0x11},
	{0x52, 0x11},
	{0x53, 0x22},
	{0x54, 0x44},
	{0x55, 0x44},
	{0x56, 0x22},
	{0x57, 0x11},
	{0x58, 0x21},
	{0x59, 0x44},
	{0x5A, 0x44},
	{0x5B, 0x44},
	{0x5C, 0x44},
	{0x5D, 0x12},
	{0x5E, 0x21},
	{0x5F, 0x44},
	{0x60, 0x44},
	{0x61, 0x44},
	{0x62, 0x44},
	{0x63, 0x12},
	{0x64, 0x21},
	{0x65, 0x44},
	{0x66, 0x44},
	{0x67, 0x44},
	{0x68, 0x44},
	{0x69, 0x12},
	{0x6A, 0x11},
	{0x6B, 0x11},
	{0x6C, 0x11},
	{0x6D, 0x11},
	{0x6E, 0x11},
	{0x6F, 0x11},

	//WB Daylight
	{0x03, 0xc5},
	{0x10, 0xb1}, //STEVE AWB on

	{0x03, 0xc6},
  {0x18, 0x59}, //bInRgainMin
  {0x19, 0x61}, //bInRgainMax
  {0x1a, 0x66}, //bInBgainMin
  {0x1b, 0x6e}, //bInBgainMax

  {0xb9, 0x59}, //bOutRgainMin
  {0xba, 0x61}, //bOutRgainMax
  {0xbb, 0x66}, //bOutBgainMin
  {0xbc, 0x6e}, //bOutBgainMax


	//Saturation 0
	{0x03, 0xd1}, //page D1(Adaptive)
  // Cb
	{0x2b, 0x98}, //SATB_00 STEVE for Low Light
	{0x2c, 0x98}, //SATB_01 STEVE for Low Light
	{0x2d, 0x98}, //SATB_02 STEVE for Low Light
	{0x2e, 0x98}, //SATB_03
	{0x2f, 0x98}, //SATB_04
	{0x30, 0x98}, //SATB_05
	{0x31, 0xa0}, //SATB_06
	{0x32, 0xa0}, //SATB_07
	{0x33, 0xa0}, //SATB_08
	{0x34, 0xa8}, //SATB_09
	{0x35, 0xa8}, //SATB_10
	{0x36, 0xa8}, //SATB_11

	// Cr
	{0x37, 0x98}, //SATR_00 STEVE for Low Light
	{0x38, 0x98}, //SATR_01 STEVE for Low Light
	{0x39, 0xa0}, //SATR_02 STEVE for Low Light
	{0x3a, 0x98}, //SATR_03
	{0x3b, 0x98}, //SATR_04
	{0x3c, 0x98}, //SATR_05
	{0x3d, 0xa0}, //SATR_06
	{0x3e, 0xa0}, //SATR_07
	{0x3f, 0xa0}, //SATR_08
	{0x40, 0xa8}, //SATR_09
	{0x41, 0xa8}, //SATR_10
	{0x42, 0xa8}, //SATR_11
	{0x42, 0x98}, //SATR_11

	// STEVE Sharpness 0
	{0x03, 0xda},
	{0x1d, 0x20}, //outdoor

	{0x03, 0xdd},
	{0x1d, 0x20}, //indoor

	{0x03, 0xe0},
	{0x1d, 0x24}, //dark1

	{0x03, 0xe3},
	{0x1d, 0x26}, //dark2


	//AE On
	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#else
	{0x10, 0xe1},
#endif
//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms



};

static struct msm_camera_i2c_reg_conf hi351_scene_night_settings[] = {

	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x68}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)
#else
	{0x10, 0x60}, // STEVE AE OFF   (50Hz : 0x68, 60hz : 0x60)
#endif

	//AWB OFF
	{0x03, 0xc5},
	{0x10, 0x30},

	//FPS NIGHT
	{0x03, 0x20},
	{0x24, 0x00}, //EXP Max 5.00 fps
	{0x25, 0x6d},
	{0x26, 0xd1},
	{0x27, 0xc0},

	//Metering - Center
	{0x03, 0xc3},

	{0x3A, 0x11},
	{0x3B, 0x11},
	{0x3C, 0x11},
	{0x3D, 0x11},
	{0x3E, 0x11},
	{0x3F, 0x11},
	{0x40, 0x11},
	{0x41, 0x11},
	{0x42, 0x11},
	{0x43, 0x11},
	{0x44, 0x11},
	{0x45, 0x11},
	{0x46, 0x11},
	{0x47, 0x11},
	{0x48, 0x11},
	{0x49, 0x11},
	{0x4A, 0x11},
	{0x4B, 0x11},
	{0x4C, 0x11},
	{0x4D, 0x21},
	{0x4E, 0x44},
	{0x4F, 0x44},
	{0x50, 0x12},
	{0x51, 0x11},
	{0x52, 0x11},
	{0x53, 0x22},
	{0x54, 0x44},
	{0x55, 0x44},
	{0x56, 0x22},
	{0x57, 0x11},
	{0x58, 0x21},
	{0x59, 0x44},
	{0x5A, 0x44},
	{0x5B, 0x44},
	{0x5C, 0x44},
	{0x5D, 0x12},
	{0x5E, 0x21},
	{0x5F, 0x44},
	{0x60, 0x44},
	{0x61, 0x44},
	{0x62, 0x44},
	{0x63, 0x12},
	{0x64, 0x21},
	{0x65, 0x44},
	{0x66, 0x44},
	{0x67, 0x44},
	{0x68, 0x44},
	{0x69, 0x12},
	{0x6A, 0x11},
	{0x6B, 0x11},
	{0x6C, 0x11},
	{0x6D, 0x11},
	{0x6E, 0x11},
	{0x6F, 0x11},

	//WB Auto
	{0x03, 0xc5}, //Page 0xc5
	{0x10, 0xb1}, //STEVE AWB on

	{0x03, 0xc6},
	{0x18, 0x40}, //bInRgainMin_a00_n00
	{0x19, 0xf0}, //bInRgainMax_a00_n00
	{0x1a, 0x40}, //bInBgainMin_a00_n00
	{0x1b, 0xf0}, //bInBgainMax_a00_n00

	{0xb9, 0x58}, //bOutRgainMin_a00_n00
	{0xba, 0xf0}, //bOutRgainMax_a00_n00
	{0xbb, 0x40}, //bOutBgainMin_a00_n00
	{0xbc, 0x90}, //bOutBgainMax_a00_n00


	//Saturation 0
	{0x03, 0xd1}, //page D1(Adaptive)

	// STEVE SATB
	{0x2b, 0x70},//SATB_00"
	{0x2c, 0x70},//SATB_01"
	{0x2d, 0x70},//SATB_02"
	{0x2e, 0x80},//SATB_03"
	{0x2f, 0x80},//SATB_04"
	{0x30, 0x80},//SATB_05"
	{0x31, 0x88},//SATB_06"
	{0x32, 0x88},//SATB_07"
	{0x33, 0x88},//SATB_08"
	{0x34, 0x88},//SATB_09"
	{0x35, 0x88},//SATB_10"
	{0x36, 0x88},//SATB_11"

	// STEVE SATR
	{0x37, 0x70},//SATR_00"
	{0x38, 0x70},//SATR_01"
	{0x39, 0x70},//SATR_02"
	{0x3a, 0x80},//SATR_03"
	{0x3b, 0x80},//SATR_04"
	{0x3c, 0x80},//SATR_05"
	{0x3d, 0x88},//SATR_06"
	{0x3e, 0x88},//SATR_07"
	{0x3f, 0x88},//SATR_08"
	{0x40, 0x88},//SATR_09"
	{0x41, 0x88},//SATR_10"
	{0x42, 0x88},//SATR_11"

	// STEVE Sharpness 0
	{0x03, 0xda},
	{0x1d, 0x20},//outdoor

	{0x03, 0xdd},
	{0x1d, 0x20},//indoor

	{0x03, 0xe0},
	{0x1d, 0x24},//dark1

	{0x03, 0xe3},
	{0x1d, 0x26},//dark2


	//AE On
	{0x03, 0xc4},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)
#else
	{0x10, 0xe1},
#endif
//	{0x03, 0xFE},
//	{0xFE, 0x0A}, //Delay 10ms


};



static struct msm_camera_i2c_reg_conf hi351_recommend_settings[] = {

	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x01, 0xf1, MSM_CAMERA_I2C_BYTE_DATA}, //Initial_111221_AWB(EV)_target_ColorRatio_lsc75p_AGC_D0_50_deSat_Ysat


	{0x01, 0xf3, MSM_CAMERA_I2C_BYTE_DATA},
	{0x01, 0xf1, MSM_CAMERA_I2C_BYTE_DATA},

	///////////////////////////////////////////
	// 0 Page PLL setting
	///////////////////////////////////////////
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x07, 0x25, MSM_CAMERA_I2C_BYTE_DATA}, //24/(5+1) = 4Mhz
	{0x08, 0x48, MSM_CAMERA_I2C_BYTE_DATA}, // 72Mhz
	{0x09, 0x82, MSM_CAMERA_I2C_BYTE_DATA},
	{0x07, 0xa5, MSM_CAMERA_I2C_BYTE_DATA},
	{0x07, 0xa5, MSM_CAMERA_I2C_BYTE_DATA},
	{0x09, 0xa2, MSM_CAMERA_I2C_BYTE_DATA},
//LGE_CHANGE_S MR LOW Light Tuning
	{0x0A, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, // MCU hardware reset
	{0x0A, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x0A, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x0A, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
//LGE_CHANGE_E
	///////////////////////////////////////////
	// 20 Page OTP/ROM LSC download select setting
	///////////////////////////////////////////
	{0x03, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x3a, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x3b, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x3c, 0x00, MSM_CAMERA_I2C_BYTE_DATA},



	///////////////////////////////////////////
	// 30 Page MCU reset, enable setting
	///////////////////////////////////////////
	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x30, 0x86, MSM_CAMERA_I2C_BYTE_DATA},
	{0x31, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x32, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0xe0, 0x02, MSM_CAMERA_I2C_BYTE_DATA},// CLK INVERSION
	{0x24, 0x02, MSM_CAMERA_I2C_BYTE_DATA},// PCON WRITE SET
	{0x25, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},// PCON WAKE NORMAL
	{0x10, 0x81, MSM_CAMERA_I2C_BYTE_DATA}, // mcu reset high
	{0x10, 0x89, MSM_CAMERA_I2C_BYTE_DATA}, // mcu enable high
	{0x11, 0x08, MSM_CAMERA_I2C_BYTE_DATA}, // xdata memory reset high
	{0x11, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, // xdata memory reset low

	///////////////////////////////////////////
	// 7 Page OTP/ROM color ratio download select setting
	///////////////////////////////////////////
	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x12, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x40, 0x0E, MSM_CAMERA_I2C_BYTE_DATA},
	{0x47, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x2e, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x2f, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x30, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x31, 0xD6, MSM_CAMERA_I2C_BYTE_DATA},
	{0x32, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x33, 0xFF, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x02, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA}, //delay
	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA},

	{0x2e, 0x03, MSM_CAMERA_I2C_BYTE_DATA}, // color ratio reg down
	{0x2f, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x30, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x31, 0xa6, MSM_CAMERA_I2C_BYTE_DATA},
	{0x32, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x33, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x02, MSM_CAMERA_I2C_BYTE_DATA},


	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA}, //delay
	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA},

	{0x12, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x98, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x97, 0x01, MSM_CAMERA_I2C_BYTE_DATA},

	{0x8C, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x8F, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x92, 0x4C, MSM_CAMERA_I2C_BYTE_DATA},
	{0x93, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, // Set OTP Offset
	{0x94, 0xF0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x95, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, // Full Size normal XY-flip
	///////////////////////////////////////////
	// 30 Page MCU reset, enable setting
	///////////////////////////////////////////
	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x09, MSM_CAMERA_I2C_BYTE_DATA}, // mcu reset low  = mcu start!!

	///////////////////////////////////////////
	// 0 Page
	///////////////////////////////////////////
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x0B, 0x02, MSM_CAMERA_I2C_BYTE_DATA}, //PLL lock time
	{0x10, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x13, 0x80, MSM_CAMERA_I2C_BYTE_DATA},
	{0x14, 0x70, MSM_CAMERA_I2C_BYTE_DATA},
	{0x15, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x17, 0x04, MSM_CAMERA_I2C_BYTE_DATA}, //Parallel, MIPI : 04, JPEG : 0c

	{0x20, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //Start Width
	{0x21, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x22, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //Start Height
	{0x23, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},

	{0x24, 0x06, MSM_CAMERA_I2C_BYTE_DATA}, //Widht Size
	{0x25, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x26, 0x08, MSM_CAMERA_I2C_BYTE_DATA}, //Height Size
	{0x27, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x50, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, // HBLANK 1140 + 288 = 1428
	{0x51, 0x20, MSM_CAMERA_I2C_BYTE_DATA},

	{0x52, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //VBLANK = 50
	{0x53, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	//BLC
	{0x80, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x81, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x83, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x8c, MSM_CAMERA_I2C_BURST_DATA},
	{0x85, 0x0c, MSM_CAMERA_I2C_BURST_DATA},//blc on
	{0x86, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x87, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0x89, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x8b, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x80, MSM_CAMERA_I2C_BYTE_DATA},
	{0x8f, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x90, 0x0c, MSM_CAMERA_I2C_BYTE_DATA}, //BLC_TIME_TH_ON // STEVE for 50Hz
	{0x91, 0x0c, MSM_CAMERA_I2C_BYTE_DATA}, //BLC_TIME_TH_OFF // STEVE for 50Hz
#else
	{0x90, 0x0f, MSM_CAMERA_I2C_BYTE_DATA}, //BLC_TIME_TH_ON
	{0x91, 0x0f, MSM_CAMERA_I2C_BYTE_DATA}, //BLC_TIME_TH_OFF
#endif
	{0x92, 0xe8, MSM_CAMERA_I2C_BYTE_DATA}, //BLC_AG_TH_ON  // STEVE AGC 0xd0
	{0x93, 0xe0, MSM_CAMERA_I2C_BYTE_DATA}, //BLC_AG_TH_OFF // STEVE AGC 0xd0
	{0x96, 0xfe, MSM_CAMERA_I2C_BYTE_DATA}, //BLC_OUT_TH
	{0x97, 0xfd, MSM_CAMERA_I2C_BYTE_DATA}, //BLC_OUT_TH
	{0x98, 0x20, MSM_CAMERA_I2C_BYTE_DATA},



	{0xa0, 0x85, MSM_CAMERA_I2C_BURST_DATA}, //odd_adj_normal
	{0xa1, 0x85, MSM_CAMERA_I2C_BURST_DATA}, //out r
	{0xa2, 0x85, MSM_CAMERA_I2C_BURST_DATA}, //in
	{0xa3, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //dark	MR Low Light Tuning
	{0xa4, 0x85, MSM_CAMERA_I2C_BURST_DATA}, //even_adj_normal
	{0xa5, 0x85, MSM_CAMERA_I2C_BURST_DATA}, //out b
	{0xa6, 0x85, MSM_CAMERA_I2C_BURST_DATA}, //in
	{0xa7, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //dark	 MR Low Light Tuning


	{0xbb, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	///////////////////////////////////////////
	// 2 Page
	///////////////////////////////////////////

	{0x03, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x13, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x14, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x15, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x1a, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//ncp adaptive off
	{0x1b, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x1c, 0xc0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x1d, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//MCU update bit[4]
	{0x20, 0x44, MSM_CAMERA_I2C_BYTE_DATA},
	{0x21, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x22, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x23, 0x30, MSM_CAMERA_I2C_BYTE_DATA},//clamp on 10 -30
	{0x24, 0x77, MSM_CAMERA_I2C_BYTE_DATA},
	{0x2b, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x0C, MSM_CAMERA_I2C_BURST_DATA},
	{0x2d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x2e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x2f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x30, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x31, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x33, 0x42, MSM_CAMERA_I2C_BURST_DATA}, // STEVE01 0x02 -)0x42 DV3 fix
	{0x34, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x35, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x37, 0x40, MSM_CAMERA_I2C_BURST_DATA}, // STEVE01 0x20 -) 0x40 DV3 fix
	{0x38, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x39, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x3d, 0x70, MSM_CAMERA_I2C_BYTE_DATA},
	{0x3e, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x3f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x40, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x41, 0x8a, MSM_CAMERA_I2C_BURST_DATA},
	{0x42, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x43, 0x25, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x47, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x48, 0x3C, MSM_CAMERA_I2C_BURST_DATA},
	{0x49, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x4b, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x4d, 0x70, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x4f, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x51, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x70, MSM_CAMERA_I2C_BURST_DATA},
	{0x53, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x57, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0x0E, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5b, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5d, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x61, 0xe2, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x63, 0xc8, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x67, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0x69, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x6b, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6d, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x70, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x71, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0x73, 0xa5, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x75, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x77, 0xfa, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x79, 0xb4, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x7b, 0xb8, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7d, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xa0, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0xa1, 0xEB, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xa3, 0x2D, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xa5, 0xB9, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0xa7, 0xED, MSM_CAMERA_I2C_BURST_DATA},
	{0xa8, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xa9, 0xEB, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0xab, 0xED, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xad, 0x79, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xaf, 0x2D, MSM_CAMERA_I2C_BURST_DATA},
	{0xb0, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xb1, 0x56, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0xb3, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xb5, 0x2B, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xb7, 0x2B, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xb9, 0x56, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xbb, 0xC8, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xbd, 0x2B, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0xbf, 0xAB, MSM_CAMERA_I2C_BURST_DATA},
	{0xc0, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc1, 0x54, MSM_CAMERA_I2C_BURST_DATA},
	{0xc2, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0xc3, 0x0A, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc5, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xc7, 0x2D, MSM_CAMERA_I2C_BURST_DATA},
	{0xc8, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc9, 0x54, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xcb, 0xCA, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xcd, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0xcf, 0xAD, MSM_CAMERA_I2C_BURST_DATA},
	{0xd0, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xd1, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xd2, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xd3, 0x00, MSM_CAMERA_I2C_BURST_DATA},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0xd4, 0x0c, MSM_CAMERA_I2C_BURST_DATA},//DCDC_TIME_TH_ON  // STEVE for 50hz
	{0xd5, 0x0c, MSM_CAMERA_I2C_BURST_DATA},//DCDC_TIME_TH_OFF // STEVE for 50hz
#else
	{0xd4, 0x0f, MSM_CAMERA_I2C_BURST_DATA},//DCDC_TIME_TH_ON // STEVE
	{0xd5, 0x0f, MSM_CAMERA_I2C_BURST_DATA},//DCDC_TIME_TH_OFF // STEVE
#endif
	{0xd6, 0xe8, MSM_CAMERA_I2C_BURST_DATA},//DCDC_AG_TH_ON	 // STEVE AGC 0xf0
	{0xd7, 0xe0, MSM_CAMERA_I2C_BURST_DATA},//DCDC_AG_TH_OFF  // STEVE AGC 0xf0
	{0xE0, 0xf0, MSM_CAMERA_I2C_BYTE_DATA},//ncp adaptive
	{0xE1, 0xf0, MSM_CAMERA_I2C_BURST_DATA},//ncp adaptive
	{0xE2, 0xf0, MSM_CAMERA_I2C_BURST_DATA},//ncp adaptive
	{0xE3, 0xf0, MSM_CAMERA_I2C_BURST_DATA},//ncp adaptive
	{0xE4, 0xd0, MSM_CAMERA_I2C_BURST_DATA},//ncp adaptive
	{0xE5, 0x00, MSM_CAMERA_I2C_BURST_DATA},//ncp adaptive
	{0xE6, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xE7, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xE8, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xE9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xEA, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0xEB, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0xEC, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0xED, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0xEE, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0xEF, 0x65, MSM_CAMERA_I2C_BURST_DATA},
	{0xF0, 0x0c, MSM_CAMERA_I2C_BURST_DATA},
	{0xF3, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0xF4, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xF5, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0xF6, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0xF7, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0xF8, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0xF9, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0xFA, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0xFB, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0xFC, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0xFD, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0xFE, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	///////////////////////////////////////////
	//3Page
	///////////////////////////////////////////
	{0x03, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x11, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x13, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x15, 0x51, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x17, 0x59, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x19, 0x97, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x1b, 0x7C, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x1d, 0x97, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x1f, 0x7C, MSM_CAMERA_I2C_BURST_DATA},
	{0x20, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x21, 0x97, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x23, 0xe3, MSM_CAMERA_I2C_BURST_DATA}, //cds 2 off time sunspot
	{0x24, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x25, 0x97, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x27, 0xe3, MSM_CAMERA_I2C_BURST_DATA}, //cds 2 off time  sunspot

	{0x28, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x29, 0x97, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x2b, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x2d, 0x97, MSM_CAMERA_I2C_BURST_DATA},
	{0x2e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x2f, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
	{0x30, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x31, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x33, 0x31, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x35, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BURST_DATA},
	{0x38, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x39, 0x0A, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x3b, 0xB0, MSM_CAMERA_I2C_BURST_DATA},
	{0x3c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x3d, 0x0A, MSM_CAMERA_I2C_BURST_DATA},
	{0x3e, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x3f, 0xB0, MSM_CAMERA_I2C_BURST_DATA},
	{0x40, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x41, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x42, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x43, 0x1c, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x45, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x47, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0x48, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x49, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x4b, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x4d, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x4f, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x51, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x53, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x57, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x5A, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5b, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5d, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5f, 0x1c, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x61, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x63, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x67, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x69, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x6A, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6B, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0x6C, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6D, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6E, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6F, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x70, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x71, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x73, 0x86, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x75, 0x6B, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x77, 0x93, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x79, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x7b, 0x88, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x7d, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x7f, 0x88, MSM_CAMERA_I2C_BURST_DATA},
	{0x80, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x81, 0x13, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x83, 0x3B, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x85, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x87, 0x88, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x89, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x8b, 0x88, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x8d, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x8f, 0x42, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x91, 0x68, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x93, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x95, 0x68, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x97, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x99, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x9b, 0x68, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x9d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x9f, 0x68, MSM_CAMERA_I2C_BURST_DATA},
	{0xa0, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xa1, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xa3, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xa5, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xa7, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xa8, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xa9, 0x73, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xab, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xad, 0x73, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xaf, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0xc0, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0xc1, 0x1d, MSM_CAMERA_I2C_BURST_DATA},
	{0xc2, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc3, 0x2f, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc5, 0x1d, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc7, 0x2f, MSM_CAMERA_I2C_BURST_DATA},
	{0xc8, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc9, 0x1f, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xcb, 0x2d, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xcd, 0x1f, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xcf, 0x2d, MSM_CAMERA_I2C_BURST_DATA},
	{0xd0, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xd1, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0xd2, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xd3, 0x2b, MSM_CAMERA_I2C_BURST_DATA},
	{0xd4, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xd5, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0xd6, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xd7, 0x2b, MSM_CAMERA_I2C_BURST_DATA},
	{0xd8, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xd9, 0x23, MSM_CAMERA_I2C_BURST_DATA},
	{0xdA, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xdB, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0xdC, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xdD, 0x23, MSM_CAMERA_I2C_BURST_DATA},
	{0xdE, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xdF, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0xe0, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xe1, 0x6B, MSM_CAMERA_I2C_BURST_DATA},
	{0xe2, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xe3, 0xE8, MSM_CAMERA_I2C_BURST_DATA},
	{0xe4, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xe5, 0xEB, MSM_CAMERA_I2C_BURST_DATA},
	{0xe6, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0xe7, 0x7E, MSM_CAMERA_I2C_BURST_DATA},
	{0xe8, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xe9, 0x95, MSM_CAMERA_I2C_BURST_DATA},
	{0xea, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xeb, 0xF1, MSM_CAMERA_I2C_BURST_DATA},
	{0xec, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xed, 0xdd, MSM_CAMERA_I2C_BURST_DATA},
	{0xee, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xef, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0xf0, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xf1, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0xf2, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	///////////////////////////////////////////
	// 10 Page
	///////////////////////////////////////////
	{0x03, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0xe0, 0xff, MSM_CAMERA_I2C_BYTE_DATA},
	{0xe1, 0x3f, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xe2, 0xff, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xe3, 0xff, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xe4, 0xf7, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xe5, 0x79, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xe6, 0xce, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xe7, 0x1f, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xe8, 0x5f, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xe9, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xea, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xeb, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xec, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xed, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xf0, 0x3f, MSM_CAMERA_I2C_BYTE_DATA},
	{0xf1, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update
	{0xf2, 0x40, MSM_CAMERA_I2C_BYTE_DATA}, // don't touch update

	{0x10, 0x03, MSM_CAMERA_I2C_BYTE_DATA}, //YUV422-YUYV
	{0x12, 0x10, MSM_CAMERA_I2C_BYTE_DATA}, //Y,DY offset Enb
	{0x13, 0x02, MSM_CAMERA_I2C_BYTE_DATA}, //Bright2, Contrast Enb
	{0x20, 0x80, MSM_CAMERA_I2C_BYTE_DATA},

	{0x60, 0x03, MSM_CAMERA_I2C_BYTE_DATA}, //Sat, Trans Enb
	{0x61, 0x80, MSM_CAMERA_I2C_BYTE_DATA},
	{0x62, 0x80, MSM_CAMERA_I2C_BYTE_DATA},
	//Desat - Chroma
	// STEVE for achromatic color
//LGE_CHANGE_S MR LOW Light Tuning
	{0x03, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x70, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x71, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0xbe, MSM_CAMERA_I2C_BURST_DATA},
	{0x73, 0x88, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x51, MSM_CAMERA_I2C_BURST_DATA},
	{0x75, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x23, MSM_CAMERA_I2C_BURST_DATA},
	{0x77, 0x31, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0xeb, MSM_CAMERA_I2C_BURST_DATA},
	{0x79, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x51, MSM_CAMERA_I2C_BURST_DATA},
	{0x7b, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7d, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x7f, 0x38, MSM_CAMERA_I2C_BURST_DATA},

	///////////////////////////////////////////
	// 11 page D-LPF
	///////////////////////////////////////////
	//DLPF
	{0x03, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0xf0, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xf1, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xf2, 0xe8, MSM_CAMERA_I2C_BURST_DATA}, //in/dark1 PGA
	{0xf3, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0xf4, 0xfe, MSM_CAMERA_I2C_BURST_DATA},
	{0xf5, 0xfd, MSM_CAMERA_I2C_BURST_DATA},
	{0xf6, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xf7, 0x00, MSM_CAMERA_I2C_BURST_DATA},
//LGE_CHANGE_E
	// STEVE Luminanace level setting (Add to DMA)
	{0x32, 0x8b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x33, 0x54, MSM_CAMERA_I2C_BYTE_DATA},
	{0x34, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x35, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x36, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x38, 0x17, MSM_CAMERA_I2C_BYTE_DATA},

	///////////////////////////////////////////
	// 12 page DPC / GBGR /LensDebulr
	///////////////////////////////////////////
	{0x03, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x57, MSM_CAMERA_I2C_BURST_DATA},
	{0x11, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x13, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x15, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x17, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x18, 0xc8, MSM_CAMERA_I2C_BURST_DATA},
	{0x19, 0x7d, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x1b, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x1d, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x1f, 0x28, MSM_CAMERA_I2C_BURST_DATA},

	{0x20, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x21, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x0f, MSM_CAMERA_I2C_BURST_DATA},
	{0x23, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0x25, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x27, 0x3c, MSM_CAMERA_I2C_BURST_DATA},

	{0x28, 0x78, MSM_CAMERA_I2C_BURST_DATA},
	{0x29, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0xb4, MSM_CAMERA_I2C_BURST_DATA},
	{0x2b, 0x08, MSM_CAMERA_I2C_BURST_DATA},//DPC threshold
	{0x2c, 0x08, MSM_CAMERA_I2C_BURST_DATA},//DPC threshold
	{0x2d, 0x08, MSM_CAMERA_I2C_BURST_DATA},//DPC threshold
	{0x2e, 0x06, MSM_CAMERA_I2C_BURST_DATA},//DPC threshold
	{0x2f, 0x64, MSM_CAMERA_I2C_BURST_DATA},

	{0x30, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0x31, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	//GBGR
	{0x33, 0xaa, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x96, MSM_CAMERA_I2C_BURST_DATA},
	{0x35, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x0e, MSM_CAMERA_I2C_BURST_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BURST_DATA},

	{0x38, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x39, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x3b, 0x0c, MSM_CAMERA_I2C_BURST_DATA},
	{0x3C, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x3D, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x3E, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x3F, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x40, 0x33, MSM_CAMERA_I2C_BYTE_DATA},
	{0xE0, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0xE1, 0x58, MSM_CAMERA_I2C_BYTE_DATA},
	{0xEC, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0xEE, 0x03, MSM_CAMERA_I2C_BYTE_DATA},

	///////////////////////////////////////////
	// 13 page YC2D LPF
	///////////////////////////////////////////
	{0x03, 0x13, MSM_CAMERA_I2C_BYTE_DATA},

	{0x10, 0x33, MSM_CAMERA_I2C_BYTE_DATA}, //Don't touch
	{0xa0, 0x0f, MSM_CAMERA_I2C_BYTE_DATA}, //Don't touch

	{0xe1, 0x07, MSM_CAMERA_I2C_BYTE_DATA},

	///////////////////////////////////////////
	// 14 page Sharpness
	///////////////////////////////////////////
	{0x03, 0x14, MSM_CAMERA_I2C_BYTE_DATA},

	{0x10, 0x27, MSM_CAMERA_I2C_BYTE_DATA}, //Don't touch
	{0x11, 0x02, MSM_CAMERA_I2C_BYTE_DATA}, //Don't touch
	{0x12, 0x40, MSM_CAMERA_I2C_BYTE_DATA}, //Don't touch
	{0x20, 0x82, MSM_CAMERA_I2C_BYTE_DATA}, //Don't touch
	{0x30, 0x82, MSM_CAMERA_I2C_BYTE_DATA}, //Don't touch
	{0x40, 0x84, MSM_CAMERA_I2C_BYTE_DATA}, //Don't touch
	{0x50, 0x84, MSM_CAMERA_I2C_BYTE_DATA}, //Don't touch

	///////////////////////////////////////////
	// 15 Page LSC off
	///////////////////////////////////////////
	{0x03, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x82, MSM_CAMERA_I2C_BYTE_DATA}, //lsc off

	{0x03, 0xFE, MSM_CAMERA_I2C_BYTE_DATA}, //need to merge for solving preview rainbow problem
	{0xFE, 0x0A, MSM_CAMERA_I2C_BYTE_DATA},

	///////////////////////////////////////////
	// 7 Page LSC data (STEVE 75p)
	///////////////////////////////////////////
	{0x03, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x12, 0x04, MSM_CAMERA_I2C_BYTE_DATA},//07
	{0x34, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x35, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x13, 0x85, MSM_CAMERA_I2C_BYTE_DATA},
	{0x13, 0x05, MSM_CAMERA_I2C_BYTE_DATA},

	//================ LSC set start
	//start
	{0x37, 0x39, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x34, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x40, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x46, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x48, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x4a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x33, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x39, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x43, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x43, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x16, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x16, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x33, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x40, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x40, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x33, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x34, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x16, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x16, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x38, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x42, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x44, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x38, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x34, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x34, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x39, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x43, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x48, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x4b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x4d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x39, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x42, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x46, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x46, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x42, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x43, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x39, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x33, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x39, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x16, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x34, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x43, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x44, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x34, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x41, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x47, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x48, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x41, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x34, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x39, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x39, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x16, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x41, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x33, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x33, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x38, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x43, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x47, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x4a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x38, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x34, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x41, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x42, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x16, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x16, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x13, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x33, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x0f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x29, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x12, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x35, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x26, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x21, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x37, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x42, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x42, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x33, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2b, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x27, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x25, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x22, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x23, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x24, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x2c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x36, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x3c, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x41, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x46, MSM_CAMERA_I2C_BYTE_DATA},
	{0x37, 0x46, MSM_CAMERA_I2C_BYTE_DATA},
	//END

	//================ LSC set end

	{0x12, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x13, 0x00, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x15, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x83, MSM_CAMERA_I2C_BYTE_DATA}, // LSC ON

	///////////////////////////////////////////
	// 16 Page CMC
	///////////////////////////////////////////
	{0x03, 0x16, MSM_CAMERA_I2C_BYTE_DATA},

	{0x10, 0x0f, MSM_CAMERA_I2C_BYTE_DATA}, //cmc
	{0x17, 0x2f, MSM_CAMERA_I2C_BYTE_DATA}, //CMC SIGN
	{0x60, 0xff, MSM_CAMERA_I2C_BYTE_DATA}, //mcmc steve MCMC ON 20111221 MR

	// STEVE automatic saturation according Y level
	{0x8a, 0x5c, MSM_CAMERA_I2C_BURST_DATA},
	{0x8b, 0x73, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x7b, MSM_CAMERA_I2C_BURST_DATA},
	{0x8d, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x8f, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x91, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x93, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x95, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x97, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x99, 0x7c, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0x78, MSM_CAMERA_I2C_BURST_DATA},

	//Dgain
	{0xa0, 0x81, MSM_CAMERA_I2C_BYTE_DATA}, //Manual WB gain enable
	{0xa1, 0x00, MSM_CAMERA_I2C_BYTE_DATA},

	{0xa2, 0x68, MSM_CAMERA_I2C_BYTE_DATA}, //R_dgain_byr
	{0xa3, 0x70, MSM_CAMERA_I2C_BYTE_DATA}, //B_dgain_byr

	{0xa6, 0xa0, MSM_CAMERA_I2C_BYTE_DATA}, //r max
	{0xa8, 0xa0, MSM_CAMERA_I2C_BYTE_DATA}, //b max
			// Pre WB gain setting(after AWB setting)
//LGE_CHANGE_S MR LOW Light Tuning
	{0xF0, 0x01, MSM_CAMERA_I2C_BYTE_DATA},//Pre WB gain enable Gain resolution_1x	STEVE LOW
	{0xF1, 0x40, MSM_CAMERA_I2C_BYTE_DATA},
	{0xF2, 0x40, MSM_CAMERA_I2C_BYTE_DATA},
	{0xF3, 0x40, MSM_CAMERA_I2C_BYTE_DATA},
	{0xF4, 0x40, MSM_CAMERA_I2C_BYTE_DATA},
//LGE_CHANGE_E
	///////////////////////////////////////////
	// 17 Page Gamma
	///////////////////////////////////////////
	{0x03, 0x17, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x01, MSM_CAMERA_I2C_BYTE_DATA},

	///////////////////////////////////////////
	// 18 Page Histogram
	///////////////////////////////////////////
	{0x03, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0xc0, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0xc4, 0x7e, MSM_CAMERA_I2C_BYTE_DATA},//110927
	{0xc5, 0x69, MSM_CAMERA_I2C_BYTE_DATA},

	///////////////////////////////////////////
	// 20 Page AE
	///////////////////////////////////////////
	{0x03, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0x9f, MSM_CAMERA_I2C_BYTE_DATA},// STEVE Mananual 50Hz flicker (50Hz: 9f, 60hz: 8f)
	{0x12, 0x6d, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE Dgain ON for Low Light Spec. (2d -) 6d)  MR
	{0x17, 0xa0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x1f, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x20, MSM_CAMERA_I2C_BYTE_DATA}, //Page 20
	{0x20, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //EXP Normal 30.00 fps
	{0x21, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x79, MSM_CAMERA_I2C_BURST_DATA},
	{0x23, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //EXP Max 8.00 fps
	{0x25, 0x41, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0xe4, MSM_CAMERA_I2C_BURST_DATA},
	{0x27, 0x40, MSM_CAMERA_I2C_BURST_DATA},
#else
	{0x10, 0x8f, MSM_CAMERA_I2C_BYTE_DATA},//auto flicker auto 60hz select
	{0x12, 0x6d, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE Dgain ON for Low Light Spec. (2d -) 6d)  MR
	{0x17, 0xa0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x1f, 0x1f, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x20, MSM_CAMERA_I2C_BYTE_DATA}, //Page 20
	{0x20, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //EXP Normal 30.00 fps
	{0x21, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x4d, MSM_CAMERA_I2C_BURST_DATA},
	{0x23, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //EXP Max 8.00 fps
	{0x25, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0xa3, MSM_CAMERA_I2C_BURST_DATA},
	{0x27, 0x18, MSM_CAMERA_I2C_BURST_DATA},
#endif

	{0x28, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //EXPMin 25210.08 fps
	{0x29, 0x0b, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x28, MSM_CAMERA_I2C_BURST_DATA},

	{0x30, 0x05, MSM_CAMERA_I2C_BYTE_DATA}, //EXP100
	{0x31, 0x7d, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0xb0, MSM_CAMERA_I2C_BURST_DATA},

	{0x33, 0x04, MSM_CAMERA_I2C_BURST_DATA}, //EXP120
	{0x34, 0x93, MSM_CAMERA_I2C_BURST_DATA},
	{0x35, 0x68, MSM_CAMERA_I2C_BURST_DATA},
	
	{0x36, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //EXP Unit
	{0x37, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0x38, 0x94, MSM_CAMERA_I2C_BURST_DATA},

#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x40, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //exp 12000
	{0x41, 0x05, MSM_CAMERA_I2C_BYTE_DATA},
	{0x42, 0x7d, MSM_CAMERA_I2C_BYTE_DATA},
#else
	{0x40, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //exp 12000
	{0x41, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x42, 0x93, MSM_CAMERA_I2C_BYTE_DATA},
#endif
  {0x43, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x51, 0xF0, MSM_CAMERA_I2C_BYTE_DATA}, //pga_max_total A0 -) D0 STEVE MR
	{0x52, 0x28, MSM_CAMERA_I2C_BYTE_DATA}, //pga_min_total

	{0x71, 0xD0, MSM_CAMERA_I2C_BYTE_DATA}, //DG MAX 0x80 -) 0xD0 STEVE MR
	{0x72, 0x80, MSM_CAMERA_I2C_BYTE_DATA}, //DG MIN

	{0x80, 0x36, MSM_CAMERA_I2C_BYTE_DATA}, //AE target 34 -> 36 STEVE

	///////////////////////////////////////////
	// Preview Setting
	///////////////////////////////////////////

	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x13, MSM_CAMERA_I2C_BYTE_DATA}, //Pre2

	{0x20, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x21, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, //preview row start set.

	{0x03, 0x15, MSM_CAMERA_I2C_BYTE_DATA},  //Shading
	{0x10, 0x81, MSM_CAMERA_I2C_BYTE_DATA},  //
	{0x20, 0x04, MSM_CAMERA_I2C_BYTE_DATA},  //Shading Width 2048
	{0x21, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x22, 0x03, MSM_CAMERA_I2C_BYTE_DATA},  //Shading Height 768
	{0x23, 0x00, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x19, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x87, MSM_CAMERA_I2C_BYTE_DATA},//MODE_ZOOM
	{0x11, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//MODE_ZOOM2
	{0x12, 0x06, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_CONFIG
	{0x13, 0x01, MSM_CAMERA_I2C_BYTE_DATA},//Test Setting

  //Steve for 1024x768
	{0x20, 0x04, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_DST_WIDTH_H
	{0x21, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_DST_WIDTH_L
	{0x22, 0x03, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_DST_HEIGHT_H
	{0x23, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_DST_HEIGHT_L
	{0x24, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_WIN_STX_H
	{0x25, 0x03, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_WIN_STX_L
	{0x26, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_WIN_STY_H
	{0x27, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_WIN_STY_L
	{0x28, 0x04, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_WIN_ENX_H
	{0x29, 0x03, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_WIN_ENX_L
	{0x2a, 0x03, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_WIN_ENY_H
	{0x2b, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_WIN_ENY_L
	{0x2c, 0x08, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_VER_STEP_H
	{0x2d, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_VER_STEP_L
	{0x2e, 0x08, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_HOR_STEP_H
	{0x2f, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_HOR_STEP_L
	{0x30, 0x04, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_FIFO_DELAY
	{0x30, 0x04, MSM_CAMERA_I2C_BYTE_DATA},//ZOOM_FIFO_DELAY

	///////////////////////////////////////////
	// 30 Page DMA address set
	///////////////////////////////////////////
	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA}, //DMA
	{0x7c, 0x2c, MSM_CAMERA_I2C_BYTE_DATA}, //Extra str
	{0x7d, 0xce, MSM_CAMERA_I2C_BYTE_DATA},
	{0x7e, 0x2c, MSM_CAMERA_I2C_BYTE_DATA}, //Extra end
	{0x7f, 0xd1, MSM_CAMERA_I2C_BYTE_DATA},
	{0x80, 0x24, MSM_CAMERA_I2C_BYTE_DATA}, //Outdoor str
	{0x81, 0x70, MSM_CAMERA_I2C_BYTE_DATA},
	{0x82, 0x24, MSM_CAMERA_I2C_BYTE_DATA}, //Outdoor end
	{0x83, 0x73, MSM_CAMERA_I2C_BYTE_DATA},
	{0x84, 0x21, MSM_CAMERA_I2C_BYTE_DATA}, //Indoor str
	{0x85, 0xa6, MSM_CAMERA_I2C_BYTE_DATA},
	{0x86, 0x21, MSM_CAMERA_I2C_BYTE_DATA}, //Indoor end
	{0x87, 0xa9, MSM_CAMERA_I2C_BYTE_DATA},
	{0x88, 0x27, MSM_CAMERA_I2C_BYTE_DATA}, //Dark1 str
	{0x89, 0x3a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x8a, 0x27, MSM_CAMERA_I2C_BYTE_DATA}, //Dark1 end
	{0x8b, 0x3d, MSM_CAMERA_I2C_BYTE_DATA},
	{0x8c, 0x2a, MSM_CAMERA_I2C_BYTE_DATA}, //Dark2 str
	{0x8d, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x8e, 0x2a, MSM_CAMERA_I2C_BYTE_DATA}, //Dark2 end
	{0x8f, 0x07, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0xC0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x2F, 0xf0, MSM_CAMERA_I2C_BYTE_DATA}, //DMA busy flag check
	{0x31, 0x20, MSM_CAMERA_I2C_BYTE_DATA}, //Delay before DMA write
	{0x33, 0x20, MSM_CAMERA_I2C_BYTE_DATA}, //DMA full stuck mode
	{0x32, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, //DMA on first

	{0x03, 0xC0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x2F, 0xf0, MSM_CAMERA_I2C_BYTE_DATA}, //DMA busy flag check
	{0x31, 0x20, MSM_CAMERA_I2C_BYTE_DATA}, //Delay before DMA write
	{0x33, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x32, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, //DMA on second


	{0x03, 0xC0, MSM_CAMERA_I2C_BYTE_DATA},
	{0xe1, 0x80, MSM_CAMERA_I2C_BYTE_DATA},// PCON Enable option
	{0xe1, 0x80, MSM_CAMERA_I2C_BYTE_DATA},// PCON MODE ON

	//MCU Set
	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x12, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x20, 0x08, MSM_CAMERA_I2C_BYTE_DATA},
	{0x50, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0xE0, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0xF0, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x11, 0x05, MSM_CAMERA_I2C_BYTE_DATA},// M2i Hold
	{0x03, 0xc0, MSM_CAMERA_I2C_BYTE_DATA},
	{0xe4, 0x64, MSM_CAMERA_I2C_BYTE_DATA}, //delay
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x01, 0xF0, MSM_CAMERA_I2C_BYTE_DATA}, // sleep off

	///////////////////////////////////////////
	// CD Page Adaptive Mode(Color ratio)
	///////////////////////////////////////////
//LGE_CHANGE_S MR LOW Light Tuning
	{0x03, 0xCD, MSM_CAMERA_I2C_BYTE_DATA},
	{0x47, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x12, 0x40, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE LOW
	{0x13, 0x40, MSM_CAMERA_I2C_BYTE_DATA}, //Ratio WB R gain min
	{0x14, 0x46, MSM_CAMERA_I2C_BYTE_DATA}, //Ratio WB R gain max
	{0x15, 0x40, MSM_CAMERA_I2C_BYTE_DATA}, //Ratio WB B gain min
	{0x16, 0x46, MSM_CAMERA_I2C_BYTE_DATA}, //Ratio WB B gain max
	{0x10, 0xB9, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE 38 -) b9 Enable

//LGE_CHANGE_E
	///////////////////////////////////////////
	// 1F Page SSD
	///////////////////////////////////////////
	{0x03, 0x1f, MSM_CAMERA_I2C_BYTE_DATA}, //1F page
	{0x11, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //bit[5:4]: debug mode
	{0x12, 0x60, MSM_CAMERA_I2C_BYTE_DATA},
	{0x13, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x14, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x15, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x20, 0x18, MSM_CAMERA_I2C_BYTE_DATA}, //ssd_x_start_pos
	{0x21, 0x14, MSM_CAMERA_I2C_BYTE_DATA}, //ssd_y_start_pos
	{0x22, 0x8C, MSM_CAMERA_I2C_BYTE_DATA}, //ssd_blk_width
	{0x23, 0x9c, MSM_CAMERA_I2C_BYTE_DATA}, //ssd_blk_height
	{0x28, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x29, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x3B, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x3C, 0x8C, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x19, MSM_CAMERA_I2C_BYTE_DATA}, //SSD enable

	///////////////////////////////////////////

	///////////////////////////////////////////
	// C4 Page MCU AE
	///////////////////////////////////////////
	{0x03, 0xc4, MSM_CAMERA_I2C_BYTE_DATA},
	{0x11, 0x30, MSM_CAMERA_I2C_BYTE_DATA}, // ae speed B[7:6] 0 (SLOW) ~ 3 (FAST), 0x70 - 0x30
	{0x12, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x19, 0x3C, MSM_CAMERA_I2C_BURST_DATA}, // band0 3/120
	{0x1a, 0x5C, MSM_CAMERA_I2C_BURST_DATA}, // band1 gain 6/120
	{0x1b, 0x5c, MSM_CAMERA_I2C_BURST_DATA}, // band2 gain 15/120
	{0x1c, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x1d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x00, MSM_CAMERA_I2C_BURST_DATA}, // band1 min exposure time	1/40s // correction point
	{0x1f, 0x0d, MSM_CAMERA_I2C_BURST_DATA},
	{0x20, 0xbb, MSM_CAMERA_I2C_BURST_DATA},

	{0x21, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x00, MSM_CAMERA_I2C_BURST_DATA}, // band2 min exposure time	1/20s
	{0x23, 0x1b, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x25, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x00, MSM_CAMERA_I2C_BURST_DATA},// band3 min exposure time  1/12s
	{0x27, 0x2d, MSM_CAMERA_I2C_BURST_DATA},
	{0x28, 0xc6, MSM_CAMERA_I2C_BURST_DATA},

	{0x29, 0xc0, MSM_CAMERA_I2C_BURST_DATA},

	{0x36, 0x22, MSM_CAMERA_I2C_BYTE_DATA}, // AE Yth º¯°æ

	{0x03, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x12, 0x6d, MSM_CAMERA_I2C_BYTE_DATA}, //STEVE Dgain ON for Low Light Spec. (2d) MR

	///////////////////////////////////////////
	// c3 Page MCU AE Weight
	///////////////////////////////////////////
	{0x03, 0xc3, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x38, 0xFE, MSM_CAMERA_I2C_BURST_DATA},
	{0x39, 0x79, MSM_CAMERA_I2C_BURST_DATA},

	{0x3A, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x3B, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x3C, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x3D, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x3E, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x3F, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x40, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x41, 0x11, MSM_CAMERA_I2C_BURST_DATA},

	{0x42, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x43, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x45, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x47, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x48, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x49, 0x11, MSM_CAMERA_I2C_BURST_DATA},

	{0x4A, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x4B, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x4C, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x4D, 0x31, MSM_CAMERA_I2C_BURST_DATA},
	{0x4E, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x4F, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0x13, MSM_CAMERA_I2C_BURST_DATA},
	{0x51, 0x11, MSM_CAMERA_I2C_BURST_DATA},

	{0x52, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x53, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x23, MSM_CAMERA_I2C_BURST_DATA},
	{0x57, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0x66, MSM_CAMERA_I2C_BURST_DATA},

	{0x5A, 0x76, MSM_CAMERA_I2C_BURST_DATA},
	{0x5B, 0x67, MSM_CAMERA_I2C_BURST_DATA},
	{0x5C, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x5D, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x5E, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0x5F, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x76, MSM_CAMERA_I2C_BURST_DATA},
	{0x61, 0x67, MSM_CAMERA_I2C_BURST_DATA},

	{0x62, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x63, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x67, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x69, 0x12, MSM_CAMERA_I2C_BURST_DATA},

	{0x6A, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x6B, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x6C, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x6D, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x6E, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x6F, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x70, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x71, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x72, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x73, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x75, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x77, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x79, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x7A, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7B, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7C, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x7D, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x7E, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x7F, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x80, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x81, 0x11, MSM_CAMERA_I2C_BURST_DATA},

	{0x82, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x83, 0x71, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x85, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0x17, MSM_CAMERA_I2C_BURST_DATA},
	{0x87, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x89, 0x22, MSM_CAMERA_I2C_BURST_DATA},

	{0x8A, 0x86, MSM_CAMERA_I2C_BURST_DATA},
	{0x8B, 0x68, MSM_CAMERA_I2C_BURST_DATA},
	{0x8C, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x8D, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x8E, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0x8F, 0x65, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x91, 0x77, MSM_CAMERA_I2C_BURST_DATA},

	{0x92, 0x56, MSM_CAMERA_I2C_BURST_DATA},
	{0x93, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0x95, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x97, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x99, 0x12, MSM_CAMERA_I2C_BURST_DATA},

	{0x9A, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0x9B, 0x52, MSM_CAMERA_I2C_BURST_DATA},
	{0x9C, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x9D, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x9E, 0x25, MSM_CAMERA_I2C_BURST_DATA},
	{0x9F, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0xA0, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0xA1, 0x11, MSM_CAMERA_I2C_BURST_DATA},

	{0xA2, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0xA3, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0xA4, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0xA5, 0x11, MSM_CAMERA_I2C_BURST_DATA},

	{0x03, 0xc3, MSM_CAMERA_I2C_BYTE_DATA},
	{0xe1, 0x30, MSM_CAMERA_I2C_BYTE_DATA}, //STEVE OUT AG MAX
	{0xe2, 0x03, MSM_CAMERA_I2C_BYTE_DATA}, //flicker option

	///////////////////////////////////////////
	// Capture Setting
	///////////////////////////////////////////

	{0x03, 0xd5, MSM_CAMERA_I2C_BYTE_DATA},
	{0x11, 0xb1, MSM_CAMERA_I2C_BYTE_DATA}, //manual sleep onoff STEVE Y correnctio OFF 20120220 Dgain Lowlight
	{0x14, 0xfd, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE EXPMIN x2
	{0x1e, 0x02, MSM_CAMERA_I2C_BYTE_DATA}, //capture clock set
	{0x86, 0x02, MSM_CAMERA_I2C_BYTE_DATA}, //preview clock set
	//{0x1f, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, //
	//{0x20, 0x40, MSM_CAMERA_I2C_BYTE_DATA}, // Capture Hblank 320

	// STEVE When capture process, decrease Green
	{0x1f, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0x20, 0x20, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE Capture Hblank 288 -) 2180 + 288 one line 2468

	{0x21, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x22, 0xA4, MSM_CAMERA_I2C_BYTE_DATA}, // C4 -) A4 Capture Line unit 2468

	///////////////////////////////////////////
	// Capture Mode option D6
	///////////////////////////////////////////
	{0x03, 0xd6, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0xd6, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x28, MSM_CAMERA_I2C_BYTE_DATA}, // ISO 100
	{0x11, 0x38, MSM_CAMERA_I2C_BYTE_DATA}, // ISO 200
	{0x12, 0x78, MSM_CAMERA_I2C_BYTE_DATA}, // ISO 400
	{0x13, 0xa0, MSM_CAMERA_I2C_BYTE_DATA}, // ISO 800
	{0x14, 0xe0, MSM_CAMERA_I2C_BYTE_DATA}, // ISO 1600
	{0x15, 0xf0, MSM_CAMERA_I2C_BYTE_DATA}, // ISO 3200
	///////////////////////////////////////////
	// C0 Page Firmware system
	///////////////////////////////////////////
	{0x03, 0xc0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x16, 0x81, MSM_CAMERA_I2C_BYTE_DATA}, //MCU main roof holding on

	///////////////////////////////////////////
	// C5 Page AWB
	///////////////////////////////////////////

	{0x03, 0xc5, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0xb0, MSM_CAMERA_I2C_BURST_DATA}, //bCtl1_a00_n00
	{0x11, 0xa1, MSM_CAMERA_I2C_BURST_DATA}, // Steve [4] bit must 0 for MWB
	{0x12, 0x17, MSM_CAMERA_I2C_BURST_DATA}, // STEVE 97 -> 9f YNorm -> 1f -> 17 near pt chek, Ynorm OFF
	{0x13, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //bCtl4_a00_n00
	{0x14, 0x24, MSM_CAMERA_I2C_BURST_DATA}, //bLockTh_a00_n00
	{0x15, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x17, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //bBlkPtBndWdhTh_a00_n00

	{0x18, 0x28, MSM_CAMERA_I2C_BURST_DATA}, //bBlkPtBndCntTh_a00_n00
	{0x19, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0xa0, MSM_CAMERA_I2C_BURST_DATA},//awb max ylvl
	{0x1b, 0x18, MSM_CAMERA_I2C_BURST_DATA},//awb min ylvl
	{0x1c, 0x0a, MSM_CAMERA_I2C_BURST_DATA},//awb frame skip when min max
	{0x1d, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x1f, 0xfe, MSM_CAMERA_I2C_BURST_DATA},//sky limit

	{0x20, 0x00, MSM_CAMERA_I2C_BURST_DATA}, // out2 Angle MIN
	{0x21, 0x96, MSM_CAMERA_I2C_BURST_DATA}, // out2 Angle MIN steve outdoor awb angle min (for tree) 160
	{0x22, 0x01, MSM_CAMERA_I2C_BURST_DATA}, // out2 Anble Max
	{0x23, 0x02, MSM_CAMERA_I2C_BURST_DATA}, // out2 Anble Max              sky limit
	{0x24, 0x00, MSM_CAMERA_I2C_BURST_DATA}, // out1 Angle MIN
	{0x25, 0x8e, MSM_CAMERA_I2C_BURST_DATA}, // out1 Angle MIN //steve
	{0x26, 0x00, MSM_CAMERA_I2C_BURST_DATA}, // out1 Anble Max //iInAglMaxLmt_a00_n00
	{0x27, 0xf2, MSM_CAMERA_I2C_BURST_DATA}, // out1 Anble Max //iInAglMaxLmt_a00_n01

	{0x28, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x29, 0x64, MSM_CAMERA_I2C_BURST_DATA}, //iInAglMinLmt_a00_n01 STEVE for Inca white
	{0x2a, 0x01, MSM_CAMERA_I2C_BURST_DATA}, //iDakAglMaxLmt_a00_n00
	{0x2b, 0x04, MSM_CAMERA_I2C_BURST_DATA}, //iDakAglMaxLmt_a00_n01
	{0x2c, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //iDakAglMinLmt_a00_n00
	{0x2d, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //iDakAglMinLmt_a00_n01
	{0x2e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x2f, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x30, 0x4e, MSM_CAMERA_I2C_BURST_DATA},
	{0x31, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x33, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x52, MSM_CAMERA_I2C_BURST_DATA},
	{0x35, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x37, 0x02, MSM_CAMERA_I2C_BURST_DATA}, //dwOut1LmtTh_a00_n01

	{0x38, 0xbf, MSM_CAMERA_I2C_BURST_DATA},
	{0x39, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x3b, 0x03, MSM_CAMERA_I2C_BURST_DATA}, //dwOut1StrLmtTh_a00_n01
	{0x3c, 0x0d, MSM_CAMERA_I2C_BURST_DATA}, //dwOut1StrLmtTh_a00_n02
	{0x3d, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //dwOut1StrLmtTh_a00_n03
	{0x3e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x3f, 0xb7, MSM_CAMERA_I2C_BURST_DATA},

	{0x40, 0x1b, MSM_CAMERA_I2C_BURST_DATA},
	{0x41, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x42, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x43, 0xd5, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x9f, MSM_CAMERA_I2C_BURST_DATA}, //dwDakLmtTh_a00_n02
	{0x45, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x47, 0x03, MSM_CAMERA_I2C_BURST_DATA},

	{0x48, 0x0d, MSM_CAMERA_I2C_BURST_DATA},
	{0x49, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x00, MSM_CAMERA_I2C_BURST_DATA},  // steve H outdoor -> indoor(EV)
	{0x4b, 0x04, MSM_CAMERA_I2C_BURST_DATA},  // steve M1 outdoor -> indoor(EV)
	{0x4c, 0x93, MSM_CAMERA_I2C_BURST_DATA},  // steve M2 outdoor -> indoor(EV)
	{0x4d, 0xe0, MSM_CAMERA_I2C_BURST_DATA},  // steve L outdoor -) indoor(EV)
	{0x4e, 0x00, MSM_CAMERA_I2C_BURST_DATA},  // white region shift X
	{0x4f, 0x00, MSM_CAMERA_I2C_BURST_DATA},  // white region shift Y

	{0x50, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x51, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x53, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x57, 0x55, MSM_CAMERA_I2C_BURST_DATA},

	{0x58, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x5b, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x5d, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x5f, 0x55, MSM_CAMERA_I2C_BURST_DATA},

	{0x60, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x61, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x63, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x67, 0x55, MSM_CAMERA_I2C_BURST_DATA},

	{0x68, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x69, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x6b, 0x24, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a00_n00
	{0x6c, 0x2a, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a01_n00
	{0x6d, 0x31, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a02_n00
	{0x6e, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a03_n00
	{0x6f, 0x3e, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a04_n00
	{0x70, 0x42, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a05_n00
	{0x71, 0x4a, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a06_n00
	{0x72, 0x53, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a07_n00
	{0x73, 0x5c, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a08_n00
	{0x74, 0x69, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a09_n00
	{0x75, 0x75, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnBg_a10_n00
	{0x76, 0x86, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a00_n00
	{0x77, 0x79, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a01_n00
	{0x78, 0x69, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a02_n00
	{0x79, 0x5b, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a03_n00
	{0x7a, 0x53, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a04_n00
	{0x7b, 0x4e, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a05_n00
	{0x7c, 0x48, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a06_n00
	{0x7d, 0x43, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a07_n00
	{0x7e, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a08_n00
	{0x7f, 0x3c, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a09_n00
	{0x80, 0x3c, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgLeftLmt_a10_n00
	{0x81, 0x95, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a00_n00
	{0x82, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a01_n00
	{0x83, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a02_n00
	{0x84, 0x81, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a03_n00
	{0x85, 0x79, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a04_n00
	{0x86, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a05_n00
	{0x87, 0x6b, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a06_n00
	{0x88, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a07_n00
	{0x89, 0x5a, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a08_n00
	{0x8a, 0x52, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a09_n00
	{0x8b, 0x4e, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtRgnRgRightLmt_a10_n00
	{0x8c, 0x25, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a00_n00
	{0x8d, 0x30, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a01_n00
	{0x8e, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a02_n00
	{0x8f, 0x3b, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a03_n00
	{0x90, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a04_n00
	{0x91, 0x44, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a05_n00
	{0x92, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a06_n00
	{0x93, 0x55, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a07_n00
	{0x94, 0x60, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a08_n00
	{0x95, 0x69, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a09_n00
	{0x96, 0x75, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineBg_a10_n00
	{0x97, 0x8e, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a00_n00
	{0x98, 0x7c, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a01_n00
	{0x99, 0x73, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a02_n00
	{0x9a, 0x6a, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a03_n00
	{0x9b, 0x61, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a04_n00
	{0x9c, 0x5c, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a05_n00
	{0x9d, 0x55, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a06_n00
	{0x9e, 0x4f, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a07_n00
	{0x9f, 0x4a, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a08_n00
	{0xa0, 0x47, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a09_n00
	{0xa1, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //aInWhtLineRg_a10_n00
//AWB target angle
	{0xa2, 0x34, MSM_CAMERA_I2C_BURST_DATA}, //aInTgtAngle_a00_n00
	{0xa3, 0x3c, MSM_CAMERA_I2C_BURST_DATA}, //aInTgtAngle_a01_n00
	{0xa4, 0x41, MSM_CAMERA_I2C_BURST_DATA}, //aInTgtAngle_a02_n00
	{0xa5, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //aInTgtAngle_a03_n00
	{0xa6, 0x4b, MSM_CAMERA_I2C_BURST_DATA}, //aInTgtAngle_a04_n00
	{0xa7, 0x6e, MSM_CAMERA_I2C_BURST_DATA}, //aInTgtAngle_a05_n00
	{0xa8, 0x73, MSM_CAMERA_I2C_BURST_DATA}, //aInTgtAngle_a06_n00
	{0xa9, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //aInTgtAngle_a07_n00

	//AWB target RG angle
	{0xaa, 0x28, MSM_CAMERA_I2C_BYTE_DATA},//aInRgTgtOfs_a00_n00
	{0xab, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},//aInRgTgtOfs_a01_n00
	{0xac, 0x14, MSM_CAMERA_I2C_BYTE_DATA},//aInRgTgtOfs_a02_n00
	{0xad, 0x0a, MSM_CAMERA_I2C_BYTE_DATA},//aInRgTgtOfs_a03_n00
	{0xae, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//aInRgTgtOfs_a04_n00
	{0xaf, 0x81, MSM_CAMERA_I2C_BYTE_DATA},//aInRgTgtOfs_a05_n00
	{0xb0, 0x84, MSM_CAMERA_I2C_BYTE_DATA},//aInRgTgtOfs_a06_n00
	{0xb1, 0x85, MSM_CAMERA_I2C_BYTE_DATA},//aInRgTgtOfs_a07_n00

	//AWB target BG angle
	{0xb2, 0xa8, MSM_CAMERA_I2C_BYTE_DATA},//aInBgTgtOfs_a00_n00
	{0xb3, 0x9e, MSM_CAMERA_I2C_BYTE_DATA},//aInBgTgtOfs_a01_n00
	{0xb4, 0x94, MSM_CAMERA_I2C_BYTE_DATA},//aInBgTgtOfs_a02_n00
	{0xb5, 0x8a, MSM_CAMERA_I2C_BYTE_DATA},//aInBgTgtOfs_a03_n00
	{0xb6, 0x00, MSM_CAMERA_I2C_BYTE_DATA},//aInBgTgtOfs_a04_n00
	{0xb7, 0x01, MSM_CAMERA_I2C_BYTE_DATA},//aInBgTgtOfs_a05_n00
	{0xb8, 0x04, MSM_CAMERA_I2C_BYTE_DATA},//aInBgTgtOfs_a06_n00
	{0xb9, 0x05, MSM_CAMERA_I2C_BYTE_DATA},//aInBgTgtOfs_a07_n00


//AWB left target offset
	{0xba, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xbb, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xbd, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xbf, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0xc0, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc1, 0x00, MSM_CAMERA_I2C_BURST_DATA},
//AWB right target offset
	{0xc2, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc3, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc5, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc7, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0xc8, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xc9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xcb, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xcd, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xcf, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0xd0, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xd1, 0x00, MSM_CAMERA_I2C_BURST_DATA},
// Y wgt
	{0xd2, 0x01, MSM_CAMERA_I2C_BURST_DATA}, // STEVE 20120626
	{0xd3, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xd4, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0xd5, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xd6, 0x0E, MSM_CAMERA_I2C_BURST_DATA},
	{0xd7, 0x20, MSM_CAMERA_I2C_BURST_DATA},

	{0xd8, 0x2C, MSM_CAMERA_I2C_BURST_DATA},
	{0xd9, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xda, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xdb, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xdc, 0x2E, MSM_CAMERA_I2C_BURST_DATA},
	{0xdd, 0x2c, MSM_CAMERA_I2C_BURST_DATA},
	{0xde, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xdf, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xe0, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xe1, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xe2, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xe3, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xe4, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xe5, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xe6, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xe7, 0x24, MSM_CAMERA_I2C_BURST_DATA},

	{0xe8, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xe9, 0x1c, MSM_CAMERA_I2C_BURST_DATA},
	{0xea, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0xeb, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xec, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xed, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xee, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xef, 0x0a, MSM_CAMERA_I2C_BURST_DATA},

	{0xf0, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xf1, 0x09, MSM_CAMERA_I2C_BURST_DATA},
	{0xf2, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xf3, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0xf4, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0xf5, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0xf6, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0xf7, 0x05, MSM_CAMERA_I2C_BURST_DATA},

	{0xf8, 0x64, MSM_CAMERA_I2C_BURST_DATA}, //aInHiTmpWgtRatio_a00_n00
	{0xf9, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //bInDyAglDiffMin_a00_n00
	{0xfa, 0xc0, MSM_CAMERA_I2C_BURST_DATA}, //bInDyAglDiffMax_a00_n00
	{0xfb, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //bInDyMinMaxTempWgt_a00_n00
	{0xfc, 0xc8, MSM_CAMERA_I2C_BURST_DATA}, //96 (100(96) -> 200(c8)deg  //bInSplTmpAgl_a00_n00
	{0xfd, 0x0a, MSM_CAMERA_I2C_BURST_DATA}, //bInSplTmpAglOfs_a00_n00
	//{0xfe, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, // STEVE delete
	//{0xff, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //pt core STEVE delete

	{0x03, 0xc6, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //bInSplTmpBpCntTh_a00_n00
	{0x11, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //bInSplTmpPtCorWgt_a00_n00
	{0x12, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //bInSplTmpPtWgtRatio_a00_n00
	{0x13, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //bInSplTmpAglMinLmt_a00_n00
	{0x14, 0xb4, MSM_CAMERA_I2C_BURST_DATA}, //bInSplTmpAglMaxLmt_a00_n00
	{0x15, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x17, 0xf8, MSM_CAMERA_I2C_BURST_DATA},

	{0x18, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //bInRgainMin_a00_n00
	{0x19, 0xf0, MSM_CAMERA_I2C_BURST_DATA}, //bInRgainMax_a00_n00
	{0x1a, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x1b, 0xf0, MSM_CAMERA_I2C_BURST_DATA}, //bInBgainMax_a00_n00

	{0x1c, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x1d, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a00_n00
	{0x1f, 0x3a, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a01_n00
	{0x20, 0x3f, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a02_n00
	{0x21, 0x43, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a03_n00
	{0x22, 0x49, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a04_n00
	{0x23, 0x4f, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a05_n00
	{0x24, 0x55, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a06_n00
	{0x25, 0x5e, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a07_n00
	{0x26, 0x66, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a08_n00
	{0x27, 0x6e, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a09_n00
	{0x28, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnBg_a10_n00
	{0x29, 0x5f, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a00_n00
	{0x2a, 0x5a, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a01_n00
	{0x2b, 0x54, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a02_n00
	{0x2c, 0x4e, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a03_n00
	{0x2d, 0x4b, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a04_n00
	{0x2e, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a05_n00
	{0x2f, 0x43, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a06_n00
	{0x30, 0x3e, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a07_n00
	{0x31, 0x3d, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a08_n00
	{0x32, 0x3c, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a09_n00
	{0x33, 0x3b, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgLeftLmt_a10_n00
	{0x34, 0x6f, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a00_n00
	{0x35, 0x6b, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a01_n00
	{0x36, 0x68, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a02_n00
	{0x37, 0x63, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a03_n00
	{0x38, 0x60, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a04_n00
	{0x39, 0x5c, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a05_n00
	{0x3a, 0x58, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a06_n00
	{0x3b, 0x53, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a07_n00
	{0x3c, 0x4f, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a08_n00
	{0x3d, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a09_n00
	{0x3e, 0x4a, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtRgnRgRightLmt_a10_n00
	{0x3f, 0x34, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a00_n00
	{0x40, 0x3b, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a01_n00
	{0x41, 0x41, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a02_n00
	{0x42, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a03_n00
	{0x43, 0x4b, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a04_n00
	{0x44, 0x51, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a05_n00
	{0x45, 0x56, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a06_n00
	{0x46, 0x5f, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a07_n00
	{0x47, 0x6a, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a08_n00
	{0x48, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a09_n00
	{0x49, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineBg_a10_n00
	{0x4a, 0x66, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a00_n00
	{0x4b, 0x61, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a01_n00
	{0x4c, 0x5c, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a02_n00
	{0x4d, 0x59, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a03_n00
	{0x4e, 0x55, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a04_n00
	{0x4f, 0x51, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a05_n00
	{0x50, 0x4e, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a06_n00
	{0x51, 0x49, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a07_n00
	{0x52, 0x44, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a08_n00
	{0x53, 0x43, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a09_n00
	{0x54, 0x43, MSM_CAMERA_I2C_BURST_DATA}, //aOutWhtLineRg_a10_n00
	{0x55, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //aOutTgtAngle_a00_n00
	{0x56, 0x4b, MSM_CAMERA_I2C_BURST_DATA}, //aOutTgtAngle_a01_n00
	{0x57, 0x50, MSM_CAMERA_I2C_BURST_DATA}, //aOutTgtAngle_a02_n00

	{0x58, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0x69, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x6e, MSM_CAMERA_I2C_BURST_DATA},
	{0x5b, 0x73, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x78, MSM_CAMERA_I2C_BURST_DATA},
	{0x5d, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0x5f, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x60, 0x81, MSM_CAMERA_I2C_BURST_DATA}, //aOutRgTgtOfs_a03_n00
	{0x61, 0x81, MSM_CAMERA_I2C_BURST_DATA}, //aOutRgTgtOfs_a04_n00  // STEVE 20120626
	{0x62, 0x82, MSM_CAMERA_I2C_BURST_DATA}, //aOutRgTgtOfs_a05_n00
	{0x63, 0x85, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x8f, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0x8a, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x85, MSM_CAMERA_I2C_BURST_DATA},
	{0x67, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x68, 0x01, MSM_CAMERA_I2C_BURST_DATA}, //aOutBgTgtOfs_a03_n00
	{0x69, 0x01, MSM_CAMERA_I2C_BURST_DATA}, //aOutBgTgtOfs_a04_n00 // STEVE 20120626
	{0x6a, 0x02, MSM_CAMERA_I2C_BURST_DATA}, //aOutBgTgtOfs_a05_n00
	{0x6b, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x0f, MSM_CAMERA_I2C_BURST_DATA},
	{0x6d, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6f, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x70, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x71, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x73, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x75, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x77, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x78, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x79, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7b, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7d, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x7f, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x80, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x81, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x83, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x85, 0x01, MSM_CAMERA_I2C_BURST_DATA}, // STEVE 20120626
	{0x86, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x87, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x89, 0x0E, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x8b, 0x2C, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x8d, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x8f, 0x2E, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0x2c, MSM_CAMERA_I2C_BURST_DATA},
	{0x91, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x93, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x95, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //aOutHiTmpWgtHiLmt_a00_n00
	{0x96, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //01
	{0x97, 0x1c, MSM_CAMERA_I2C_BURST_DATA}, //02
	{0x98, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //03
	{0x99, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //04
	{0x9a, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //05
	{0x9b, 0x23, MSM_CAMERA_I2C_BURST_DATA}, //06
	{0x9c, 0x24, MSM_CAMERA_I2C_BURST_DATA}, //07
	{0x9d, 0x27, MSM_CAMERA_I2C_BURST_DATA}, //08
	{0x9e, 0x28, MSM_CAMERA_I2C_BURST_DATA}, //09
	{0x9f, 0x29, MSM_CAMERA_I2C_BURST_DATA}, //10

	{0xa0, 0x08, MSM_CAMERA_I2C_BURST_DATA}, //aOutHiTmpWgtLoLmt_a00_n00
	{0xa1, 0x08, MSM_CAMERA_I2C_BURST_DATA}, //01
	{0xa2, 0x08, MSM_CAMERA_I2C_BURST_DATA}, //02
	{0xa3, 0x0d, MSM_CAMERA_I2C_BURST_DATA}, //03
	{0xa4, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //04
	{0xa5, 0x12, MSM_CAMERA_I2C_BURST_DATA}, //05
	{0xa6, 0x12, MSM_CAMERA_I2C_BURST_DATA}, //06
	{0xa7, 0x12, MSM_CAMERA_I2C_BURST_DATA}, //07
	{0xa8, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //08
	{0xa9, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //09
	{0xaa, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //10
	{0xab, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0xad, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xaf, 0x64, MSM_CAMERA_I2C_BURST_DATA},//kjh out limit 64 -> 76 sky

	{0xb0, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xb1, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //50 -> 20 sky outdoor
	{0xb3, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //1e -> 32(50%)
	{0xb4, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xb5, 0x3c, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0xb7, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0xd2, MSM_CAMERA_I2C_BURST_DATA},

	{0xb9, 0x58, MSM_CAMERA_I2C_BURST_DATA}, // steve OutRgainMin
	{0xba, 0xf0, MSM_CAMERA_I2C_BURST_DATA}, // steve OutRgainMax
	{0xbb, 0x40, MSM_CAMERA_I2C_BURST_DATA}, // steve OutBgainMin
	{0xbc, 0x90, MSM_CAMERA_I2C_BURST_DATA}, // steve OutBgainMax

	{0xbd, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xbf, 0xcd, MSM_CAMERA_I2C_BURST_DATA},

	///////////////////////////////////////////
	// CD Page (Color ratio)
	///////////////////////////////////////////
	{0x03, 0xCD, MSM_CAMERA_I2C_BYTE_DATA},
	{0x47, 0x06, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0xB8, MSM_CAMERA_I2C_BYTE_DATA}, //STEVE 38 -) B8 disable

	///////////////////////////////////////////
	//Adaptive mode : Page Mode = 0xCF
	///////////////////////////////////////////
	{0x03, 0xcf, MSM_CAMERA_I2C_BYTE_DATA},

	{0x10, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x11, 0x84, MSM_CAMERA_I2C_BURST_DATA}, // STEVE 04 -> 84  //cmc + - , adaptive lsc
	{0x12, 0x01, MSM_CAMERA_I2C_BURST_DATA},

	{0x13, 0x02, MSM_CAMERA_I2C_BURST_DATA}, //STEVE //Y_LUM_MAX 10fps, DG : 0xd0, AG:0xf0 MAX
	{0x14, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x15, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x17, 0x00, MSM_CAMERA_I2C_BURST_DATA},  //Y_LUM middle 1 //72mhz 14.58fps, AG 0x4c
	{0x18, 0x3d, MSM_CAMERA_I2C_BURST_DATA},
	{0x19, 0x39, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0xd4, MSM_CAMERA_I2C_BURST_DATA},

	{0x1b, 0x00, MSM_CAMERA_I2C_BURST_DATA},  //Y_LUM middle 2 //72mhz 120fps, AG 0x30	 0.5(0x10) x 1(0x80) = 10,000(0x0186a0)
	{0x1c, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0x1d, 0xdd, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0xd0, MSM_CAMERA_I2C_BURST_DATA},

	{0x1f, 0x00, MSM_CAMERA_I2C_BURST_DATA},  //Y_LUM min //72mhz 6000fps,AG 0x30
	{0x20, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x21, 0x1d, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x4c, MSM_CAMERA_I2C_BURST_DATA},

	{0x23, 0x9a, MSM_CAMERA_I2C_BURST_DATA},  //CTEM high
	{0x24, 0x50, MSM_CAMERA_I2C_BURST_DATA},  //ctemp middler
	{0x25, 0x3e, MSM_CAMERA_I2C_BURST_DATA},  //CTEM low

	{0x26, 0x30, MSM_CAMERA_I2C_BURST_DATA},  //YCON high
	{0x27, 0x18, MSM_CAMERA_I2C_BURST_DATA},  //YCON middle
	{0x28, 0x08, MSM_CAMERA_I2C_BURST_DATA},  //YCON low

	{0x29, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //Y_LUM max_TH
	{0x2a, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x2b, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x2d, 0x00, MSM_CAMERA_I2C_BURST_DATA},  //Y_LUM middle1_TH
	{0x2e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x2f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x30, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x31, 0x00, MSM_CAMERA_I2C_BURST_DATA},  //Y_LUM middle_TH
	{0x32, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x33, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x35, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //Y_LUM min_TH
	{0x36, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x37, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x38, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x39, 0x00, MSM_CAMERA_I2C_BURST_DATA},  //CTEM high_TH
	{0x3a, 0x00, MSM_CAMERA_I2C_BURST_DATA},  //CTEM middle_TH
	{0x3b, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //CTEM low_TH

	{0x3c, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //YCON high_TH
	{0x3d, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //YCON middle_TH
	{0x3e, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //YCON low_TH

	/////////////MSM_CAMERA_I2C_BURST_DATA///////////////////////
	// CF Page Adaptive Y Target
	/////////////MSM_CAMERA_I2C_BURST_DATA//////////////////////

	{0x3f, 0x30, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_00
	{0x40, 0x30, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_01
	{0x41, 0x30, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_02
	{0x42, 0x34, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_03
	{0x43, 0x34, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_04
	{0x44, 0x34, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_05
	{0x45, 0x36, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_06
	{0x46, 0x36, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_07
	{0x47, 0x36, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_08
	{0x48, 0x34, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_09
	{0x49, 0x34, MSM_CAMERA_I2C_BURST_DATA},  //YLVL_10
	{0x4a, 0x34, MSM_CAMERA_I2C_BURST_DATA},  //36 YLVL_11

	{0x4b, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //YCON_00
	{0x4c, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //YCON_01
	{0x4d, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //YCON_02
	{0x4e, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //Contrast 3
	{0x4f, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //Contrast 4
	{0x50, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //Contrast 5
	{0x51, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //Contrast 6
	{0x52, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //Contrast 7
	{0x53, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //Contrast 8
	{0x54, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //Contrast 9
	{0x55, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //Contrast 10
	{0x56, 0x80, MSM_CAMERA_I2C_BURST_DATA},  //Contrast 11

	/////////////MSM_CAMERA_I2C_BURST_DATA//////////////////////
	// CF Page AdMSM_CAMERA_I2C_BURST_DATA OFFSET
	/////////////MSM_CAMERA_I2C_BURST_DATA//////////////////////

	{0x57, 0x08, MSM_CAMERA_I2C_BURST_DATA}, // dark offset for noise
	{0x58, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5b, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5d, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x5f, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x60, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x61, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	/////////////MSM_CAMERA_I2C_BURST_DATA//////////////////////
	// CF~D0~D1 PMSM_CAMERA_I2C_BURST_DATAtive GAMMA
	/////////////MSM_CAMERA_I2C_BURST_DATA//////////////////////
//LGE_CHANGE_S MR LOW Light Tuning

	{0x63, 0x00, MSM_CAMERA_I2C_BURST_DATA},//GMA00
	{0x64, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0x0F, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x17, MSM_CAMERA_I2C_BURST_DATA},
	{0x67, 0x1D, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x2A, MSM_CAMERA_I2C_BURST_DATA},
	{0x69, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x3E, MSM_CAMERA_I2C_BURST_DATA},
	{0x6b, 0x47, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x4E, MSM_CAMERA_I2C_BURST_DATA},
	{0x6d, 0x56, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x5D, MSM_CAMERA_I2C_BURST_DATA},
	{0x6f, 0x63, MSM_CAMERA_I2C_BURST_DATA},
	{0x70, 0x6A, MSM_CAMERA_I2C_BURST_DATA},
	{0x71, 0x70, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x75, MSM_CAMERA_I2C_BURST_DATA},
	{0x73, 0x7B, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x75, 0x85, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x8A, MSM_CAMERA_I2C_BURST_DATA},
	{0x77, 0x8F, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0x79, 0xA1, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0xA9, MSM_CAMERA_I2C_BURST_DATA},
	{0x7b, 0xB8, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0xC5, MSM_CAMERA_I2C_BURST_DATA},
	{0x7d, 0xD1, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0xDB, MSM_CAMERA_I2C_BURST_DATA},
	{0x7f, 0xE4, MSM_CAMERA_I2C_BURST_DATA},
	{0x80, 0xEB, MSM_CAMERA_I2C_BURST_DATA},
	{0x81, 0xF2, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
	{0x83, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

	{0x85, 0x00, MSM_CAMERA_I2C_BURST_DATA},//GMA01
	{0x86, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0x87, 0x0F, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0x17, MSM_CAMERA_I2C_BURST_DATA},
	{0x89, 0x1D, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0x2A, MSM_CAMERA_I2C_BURST_DATA},
	{0x8b, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x3E, MSM_CAMERA_I2C_BURST_DATA},
	{0x8d, 0x47, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x4E, MSM_CAMERA_I2C_BURST_DATA},
	{0x8f, 0x56, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0x5D, MSM_CAMERA_I2C_BURST_DATA},
	{0x91, 0x63, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0x6A, MSM_CAMERA_I2C_BURST_DATA},
	{0x93, 0x70, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0x75, MSM_CAMERA_I2C_BURST_DATA},
	{0x95, 0x7B, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x97, 0x85, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0x8A, MSM_CAMERA_I2C_BURST_DATA},
	{0x99, 0x8F, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0x9b, 0xA1, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0xA9, MSM_CAMERA_I2C_BURST_DATA},
	{0x9d, 0xB8, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0xC5, MSM_CAMERA_I2C_BURST_DATA},
	{0x9f, 0xD1, MSM_CAMERA_I2C_BURST_DATA},
	{0xa0, 0xDB, MSM_CAMERA_I2C_BURST_DATA},
	{0xa1, 0xE4, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0xEB, MSM_CAMERA_I2C_BURST_DATA},
	{0xa3, 0xF2, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
	{0xa5, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

	{0xa7, 0x00, MSM_CAMERA_I2C_BURST_DATA},//GMA02
	{0xa8, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0xa9, 0x0F, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0x17, MSM_CAMERA_I2C_BURST_DATA},
	{0xab, 0x1D, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0x2A, MSM_CAMERA_I2C_BURST_DATA},
	{0xad, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x3E, MSM_CAMERA_I2C_BURST_DATA},
	{0xaf, 0x47, MSM_CAMERA_I2C_BURST_DATA},
	{0xb0, 0x4E, MSM_CAMERA_I2C_BURST_DATA},
	{0xb1, 0x56, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x5D, MSM_CAMERA_I2C_BURST_DATA},
	{0xb3, 0x63, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x6A, MSM_CAMERA_I2C_BURST_DATA},
	{0xb5, 0x70, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x75, MSM_CAMERA_I2C_BURST_DATA},
	{0xb7, 0x7B, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xb9, 0x85, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x8A, MSM_CAMERA_I2C_BURST_DATA},
	{0xbb, 0x8F, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0xbd, 0xA1, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0xA9, MSM_CAMERA_I2C_BURST_DATA},
	{0xbf, 0xB8, MSM_CAMERA_I2C_BURST_DATA},
	{0xc0, 0xC5, MSM_CAMERA_I2C_BURST_DATA},
	{0xc1, 0xD1, MSM_CAMERA_I2C_BURST_DATA},
	{0xc2, 0xDB, MSM_CAMERA_I2C_BURST_DATA},
	{0xc3, 0xE4, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0xEB, MSM_CAMERA_I2C_BURST_DATA},
	{0xc5, 0xF2, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
	{0xc7, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
	{0xc8, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

  {0xc9, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GMA03
  {0xca, 0x02, MSM_CAMERA_I2C_BURST_DATA},
  {0xcb, 0x04, MSM_CAMERA_I2C_BURST_DATA},
  {0xcc, 0x08, MSM_CAMERA_I2C_BURST_DATA},
  {0xcd, 0x0C, MSM_CAMERA_I2C_BURST_DATA},
  {0xce, 0x1A, MSM_CAMERA_I2C_BURST_DATA},
  {0xcf, 0x2B, MSM_CAMERA_I2C_BURST_DATA},
  {0xd0, 0x3C, MSM_CAMERA_I2C_BURST_DATA},
  {0xd1, 0x49, MSM_CAMERA_I2C_BURST_DATA},
  {0xd2, 0x55, MSM_CAMERA_I2C_BURST_DATA},
  {0xd3, 0x5F, MSM_CAMERA_I2C_BURST_DATA},
  {0xd4, 0x67, MSM_CAMERA_I2C_BURST_DATA},
  {0xd5, 0x70, MSM_CAMERA_I2C_BURST_DATA},
  {0xd6, 0x78, MSM_CAMERA_I2C_BURST_DATA},
  {0xd7, 0x80, MSM_CAMERA_I2C_BURST_DATA},
  {0xd8, 0x86, MSM_CAMERA_I2C_BURST_DATA},
  {0xd9, 0x8C, MSM_CAMERA_I2C_BURST_DATA},
  {0xda, 0x92, MSM_CAMERA_I2C_BURST_DATA},
  {0xdb, 0x97, MSM_CAMERA_I2C_BURST_DATA},
  {0xdc, 0x9C, MSM_CAMERA_I2C_BURST_DATA},
  {0xdd, 0xA1, MSM_CAMERA_I2C_BURST_DATA},
  {0xde, 0xAA, MSM_CAMERA_I2C_BURST_DATA},
  {0xdf, 0xB2, MSM_CAMERA_I2C_BURST_DATA},
  {0xe0, 0xB9, MSM_CAMERA_I2C_BURST_DATA},
  {0xe1, 0xC6, MSM_CAMERA_I2C_BURST_DATA},
  {0xe2, 0xD0, MSM_CAMERA_I2C_BURST_DATA},
  {0xe3, 0xD8, MSM_CAMERA_I2C_BURST_DATA},
  {0xe4, 0xDF, MSM_CAMERA_I2C_BURST_DATA},
  {0xe5, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
  {0xe6, 0xEC, MSM_CAMERA_I2C_BURST_DATA},
  {0xe7, 0xF1, MSM_CAMERA_I2C_BURST_DATA},
  {0xe8, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
  {0xe9, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
  {0xea, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

  {0xeb, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GMA04
  {0xec, 0x02, MSM_CAMERA_I2C_BURST_DATA},
  {0xed, 0x04, MSM_CAMERA_I2C_BURST_DATA},
  {0xee, 0x0A, MSM_CAMERA_I2C_BURST_DATA},
  {0xef, 0x10, MSM_CAMERA_I2C_BURST_DATA},
  {0xf0, 0x1F, MSM_CAMERA_I2C_BURST_DATA},
  {0xf1, 0x2D, MSM_CAMERA_I2C_BURST_DATA},
  {0xf2, 0x3E, MSM_CAMERA_I2C_BURST_DATA},
  {0xf3, 0x4A, MSM_CAMERA_I2C_BURST_DATA},
  {0xf4, 0x55, MSM_CAMERA_I2C_BURST_DATA},
  {0xf5, 0x5F, MSM_CAMERA_I2C_BURST_DATA},
  {0xf6, 0x67, MSM_CAMERA_I2C_BURST_DATA},
  {0xf7, 0x70, MSM_CAMERA_I2C_BURST_DATA},
  {0xf8, 0x78, MSM_CAMERA_I2C_BURST_DATA},
  {0xf9, 0x80, MSM_CAMERA_I2C_BURST_DATA},
  {0xfa, 0x86, MSM_CAMERA_I2C_BURST_DATA},
  {0xfb, 0x8C, MSM_CAMERA_I2C_BURST_DATA},
  {0xfc, 0x92, MSM_CAMERA_I2C_BURST_DATA},
  {0xfd, 0x97, MSM_CAMERA_I2C_BURST_DATA},
  {0x03, 0xd0, MSM_CAMERA_I2C_BYTE_DATA}, //Page d0
  {0x10, 0x9C, MSM_CAMERA_I2C_BURST_DATA},
  {0x11, 0xA1, MSM_CAMERA_I2C_BURST_DATA},
  {0x12, 0xAA, MSM_CAMERA_I2C_BURST_DATA},
  {0x13, 0xB2, MSM_CAMERA_I2C_BURST_DATA},
  {0x14, 0xB9, MSM_CAMERA_I2C_BURST_DATA},
  {0x15, 0xC6, MSM_CAMERA_I2C_BURST_DATA},
  {0x16, 0xD0, MSM_CAMERA_I2C_BURST_DATA},
  {0x17, 0xD8, MSM_CAMERA_I2C_BURST_DATA},
  {0x18, 0xDF, MSM_CAMERA_I2C_BURST_DATA},
  {0x19, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
  {0x1a, 0xEC, MSM_CAMERA_I2C_BURST_DATA},
  {0x1b, 0xF1, MSM_CAMERA_I2C_BURST_DATA},
  {0x1c, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
  {0x1d, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
  {0x1e, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

  {0x1f, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GMA05
  {0x20, 0x03, MSM_CAMERA_I2C_BURST_DATA},
  {0x21, 0x08, MSM_CAMERA_I2C_BURST_DATA},
  {0x22, 0x12, MSM_CAMERA_I2C_BURST_DATA},
  {0x23, 0x19, MSM_CAMERA_I2C_BURST_DATA},
  {0x24, 0x25, MSM_CAMERA_I2C_BURST_DATA},
  {0x25, 0x32, MSM_CAMERA_I2C_BURST_DATA},
  {0x26, 0x3E, MSM_CAMERA_I2C_BURST_DATA},
  {0x27, 0x4B, MSM_CAMERA_I2C_BURST_DATA},
  {0x28, 0x56, MSM_CAMERA_I2C_BURST_DATA},
  {0x29, 0x62, MSM_CAMERA_I2C_BURST_DATA},
  {0x2a, 0x6A, MSM_CAMERA_I2C_BURST_DATA},
  {0x2b, 0x71, MSM_CAMERA_I2C_BURST_DATA},
  {0x2c, 0x78, MSM_CAMERA_I2C_BURST_DATA},
  {0x2d, 0x7F, MSM_CAMERA_I2C_BURST_DATA},
  {0x2e, 0x85, MSM_CAMERA_I2C_BURST_DATA},
  {0x2f, 0x8A, MSM_CAMERA_I2C_BURST_DATA},
  {0x30, 0x90, MSM_CAMERA_I2C_BURST_DATA},
  {0x31, 0x95, MSM_CAMERA_I2C_BURST_DATA},
  {0x32, 0x9A, MSM_CAMERA_I2C_BURST_DATA},
  {0x33, 0x9F, MSM_CAMERA_I2C_BURST_DATA},
  {0x34, 0xA9, MSM_CAMERA_I2C_BURST_DATA},
  {0x35, 0xB1, MSM_CAMERA_I2C_BURST_DATA},
  {0x36, 0xB9, MSM_CAMERA_I2C_BURST_DATA},
  {0x37, 0xC6, MSM_CAMERA_I2C_BURST_DATA},
  {0x38, 0xD0, MSM_CAMERA_I2C_BURST_DATA},
  {0x39, 0xD8, MSM_CAMERA_I2C_BURST_DATA},
  {0x3a, 0xDF, MSM_CAMERA_I2C_BURST_DATA},
  {0x3b, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
  {0x3c, 0xEC, MSM_CAMERA_I2C_BURST_DATA},
  {0x3d, 0xF1, MSM_CAMERA_I2C_BURST_DATA},
  {0x3e, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
  {0x3f, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
  {0x40, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

  {0x41, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GMA06
  {0x42, 0x02, MSM_CAMERA_I2C_BURST_DATA},
  {0x43, 0x04, MSM_CAMERA_I2C_BURST_DATA},
  {0x44, 0x08, MSM_CAMERA_I2C_BURST_DATA},
  {0x45, 0x0C, MSM_CAMERA_I2C_BURST_DATA},
  {0x46, 0x1A, MSM_CAMERA_I2C_BURST_DATA},
  {0x47, 0x2B, MSM_CAMERA_I2C_BURST_DATA},
  {0x48, 0x3C, MSM_CAMERA_I2C_BURST_DATA},
  {0x49, 0x49, MSM_CAMERA_I2C_BURST_DATA},
  {0x4a, 0x55, MSM_CAMERA_I2C_BURST_DATA},
  {0x4b, 0x5F, MSM_CAMERA_I2C_BURST_DATA},
  {0x4c, 0x67, MSM_CAMERA_I2C_BURST_DATA},
  {0x4d, 0x70, MSM_CAMERA_I2C_BURST_DATA},
  {0x4e, 0x78, MSM_CAMERA_I2C_BURST_DATA},
  {0x4f, 0x80, MSM_CAMERA_I2C_BURST_DATA},
  {0x50, 0x86, MSM_CAMERA_I2C_BURST_DATA},
  {0x51, 0x8C, MSM_CAMERA_I2C_BURST_DATA},
  {0x52, 0x92, MSM_CAMERA_I2C_BURST_DATA},
  {0x53, 0x97, MSM_CAMERA_I2C_BURST_DATA},
  {0x54, 0x9C, MSM_CAMERA_I2C_BURST_DATA},
  {0x55, 0xA1, MSM_CAMERA_I2C_BURST_DATA},
  {0x56, 0xAA, MSM_CAMERA_I2C_BURST_DATA},
  {0x57, 0xB2, MSM_CAMERA_I2C_BURST_DATA},
  {0x58, 0xB9, MSM_CAMERA_I2C_BURST_DATA},
  {0x59, 0xC6, MSM_CAMERA_I2C_BURST_DATA},
  {0x5a, 0xD0, MSM_CAMERA_I2C_BURST_DATA},
  {0x5b, 0xD8, MSM_CAMERA_I2C_BURST_DATA},
  {0x5c, 0xDF, MSM_CAMERA_I2C_BURST_DATA},
  {0x5d, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
  {0x5e, 0xEC, MSM_CAMERA_I2C_BURST_DATA},
  {0x5f, 0xF1, MSM_CAMERA_I2C_BURST_DATA},
  {0x60, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
  {0x61, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
  {0x62, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

  {0x63, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GMA07
  {0x64, 0x02, MSM_CAMERA_I2C_BURST_DATA},
  {0x65, 0x04, MSM_CAMERA_I2C_BURST_DATA},
  {0x66, 0x0A, MSM_CAMERA_I2C_BURST_DATA},
  {0x67, 0x10, MSM_CAMERA_I2C_BURST_DATA},
  {0x68, 0x1F, MSM_CAMERA_I2C_BURST_DATA},
  {0x69, 0x2D, MSM_CAMERA_I2C_BURST_DATA},
  {0x6a, 0x3E, MSM_CAMERA_I2C_BURST_DATA},
  {0x6b, 0x4A, MSM_CAMERA_I2C_BURST_DATA},
  {0x6c, 0x55, MSM_CAMERA_I2C_BURST_DATA},
  {0x6d, 0x5F, MSM_CAMERA_I2C_BURST_DATA},
  {0x6e, 0x67, MSM_CAMERA_I2C_BURST_DATA},
  {0x6f, 0x70, MSM_CAMERA_I2C_BURST_DATA},
  {0x70, 0x78, MSM_CAMERA_I2C_BURST_DATA},
  {0x71, 0x80, MSM_CAMERA_I2C_BURST_DATA},
  {0x72, 0x86, MSM_CAMERA_I2C_BURST_DATA},
  {0x73, 0x8C, MSM_CAMERA_I2C_BURST_DATA},
  {0x74, 0x92, MSM_CAMERA_I2C_BURST_DATA},
  {0x75, 0x97, MSM_CAMERA_I2C_BURST_DATA},
  {0x76, 0x9C, MSM_CAMERA_I2C_BURST_DATA},
  {0x77, 0xA1, MSM_CAMERA_I2C_BURST_DATA},
  {0x78, 0xAA, MSM_CAMERA_I2C_BURST_DATA},
  {0x79, 0xB2, MSM_CAMERA_I2C_BURST_DATA},
  {0x7a, 0xB9, MSM_CAMERA_I2C_BURST_DATA},
  {0x7b, 0xC6, MSM_CAMERA_I2C_BURST_DATA},
  {0x7c, 0xD0, MSM_CAMERA_I2C_BURST_DATA},
  {0x7d, 0xD8, MSM_CAMERA_I2C_BURST_DATA},
  {0x7e, 0xDF, MSM_CAMERA_I2C_BURST_DATA},
  {0x7f, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
  {0x80, 0xEC, MSM_CAMERA_I2C_BURST_DATA},
  {0x81, 0xF1, MSM_CAMERA_I2C_BURST_DATA},
  {0x82, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
  {0x83, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
  {0x84, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

  {0x85, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GMA08
  {0x86, 0x03, MSM_CAMERA_I2C_BURST_DATA},
  {0x87, 0x08, MSM_CAMERA_I2C_BURST_DATA},
  {0x88, 0x12, MSM_CAMERA_I2C_BURST_DATA},
  {0x89, 0x19, MSM_CAMERA_I2C_BURST_DATA},
  {0x8a, 0x25, MSM_CAMERA_I2C_BURST_DATA},
  {0x8b, 0x32, MSM_CAMERA_I2C_BURST_DATA},
  {0x8c, 0x3E, MSM_CAMERA_I2C_BURST_DATA},
  {0x8d, 0x4B, MSM_CAMERA_I2C_BURST_DATA},
  {0x8e, 0x56, MSM_CAMERA_I2C_BURST_DATA},
  {0x8f, 0x62, MSM_CAMERA_I2C_BURST_DATA},
  {0x90, 0x6A, MSM_CAMERA_I2C_BURST_DATA},
  {0x91, 0x71, MSM_CAMERA_I2C_BURST_DATA},
  {0x92, 0x78, MSM_CAMERA_I2C_BURST_DATA},
  {0x93, 0x7F, MSM_CAMERA_I2C_BURST_DATA},
  {0x94, 0x85, MSM_CAMERA_I2C_BURST_DATA},
  {0x95, 0x8A, MSM_CAMERA_I2C_BURST_DATA},
  {0x96, 0x90, MSM_CAMERA_I2C_BURST_DATA},
  {0x97, 0x95, MSM_CAMERA_I2C_BURST_DATA},
  {0x98, 0x9A, MSM_CAMERA_I2C_BURST_DATA},
  {0x99, 0x9F, MSM_CAMERA_I2C_BURST_DATA},
  {0x9a, 0xA9, MSM_CAMERA_I2C_BURST_DATA},
  {0x9b, 0xB1, MSM_CAMERA_I2C_BURST_DATA},
  {0x9c, 0xB9, MSM_CAMERA_I2C_BURST_DATA},
  {0x9d, 0xC6, MSM_CAMERA_I2C_BURST_DATA},
  {0x9e, 0xD0, MSM_CAMERA_I2C_BURST_DATA},
  {0x9f, 0xD8, MSM_CAMERA_I2C_BURST_DATA},
  {0xa0, 0xDF, MSM_CAMERA_I2C_BURST_DATA},
  {0xa1, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
  {0xa2, 0xEC, MSM_CAMERA_I2C_BURST_DATA},
  {0xa3, 0xF1, MSM_CAMERA_I2C_BURST_DATA},
  {0xa4, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
  {0xa5, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
  {0xa6, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

  {0xa7, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GMA09
  {0xa8, 0x02, MSM_CAMERA_I2C_BURST_DATA},
  {0xa9, 0x04, MSM_CAMERA_I2C_BURST_DATA},
  {0xaa, 0x08, MSM_CAMERA_I2C_BURST_DATA},
  {0xab, 0x0C, MSM_CAMERA_I2C_BURST_DATA},
  {0xac, 0x1A, MSM_CAMERA_I2C_BURST_DATA},
  {0xad, 0x2B, MSM_CAMERA_I2C_BURST_DATA},
  {0xae, 0x3C, MSM_CAMERA_I2C_BURST_DATA},
  {0xaf, 0x49, MSM_CAMERA_I2C_BURST_DATA},
  {0xb0, 0x55, MSM_CAMERA_I2C_BURST_DATA},
  {0xb1, 0x5F, MSM_CAMERA_I2C_BURST_DATA},
  {0xb2, 0x67, MSM_CAMERA_I2C_BURST_DATA},
  {0xb3, 0x70, MSM_CAMERA_I2C_BURST_DATA},
  {0xb4, 0x78, MSM_CAMERA_I2C_BURST_DATA},
  {0xb5, 0x80, MSM_CAMERA_I2C_BURST_DATA},
  {0xb6, 0x86, MSM_CAMERA_I2C_BURST_DATA},
  {0xb7, 0x8C, MSM_CAMERA_I2C_BURST_DATA},
  {0xb8, 0x92, MSM_CAMERA_I2C_BURST_DATA},
  {0xb9, 0x97, MSM_CAMERA_I2C_BURST_DATA},
  {0xba, 0x9C, MSM_CAMERA_I2C_BURST_DATA},
  {0xbb, 0xA1, MSM_CAMERA_I2C_BURST_DATA},
  {0xbc, 0xAA, MSM_CAMERA_I2C_BURST_DATA},
  {0xbd, 0xB2, MSM_CAMERA_I2C_BURST_DATA},
  {0xbe, 0xB9, MSM_CAMERA_I2C_BURST_DATA},
  {0xbf, 0xC6, MSM_CAMERA_I2C_BURST_DATA},
  {0xc0, 0xD0, MSM_CAMERA_I2C_BURST_DATA},
  {0xc1, 0xD8, MSM_CAMERA_I2C_BURST_DATA},
  {0xc2, 0xDF, MSM_CAMERA_I2C_BURST_DATA},
  {0xc3, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
  {0xc4, 0xEC, MSM_CAMERA_I2C_BURST_DATA},
  {0xc5, 0xF1, MSM_CAMERA_I2C_BURST_DATA},
  {0xc6, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
  {0xc7, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
  {0xc8, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

  {0xc9, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GMA10
  {0xca, 0x02, MSM_CAMERA_I2C_BURST_DATA},
  {0xcb, 0x04, MSM_CAMERA_I2C_BURST_DATA},
  {0xcc, 0x0A, MSM_CAMERA_I2C_BURST_DATA},
  {0xcd, 0x10, MSM_CAMERA_I2C_BURST_DATA},
  {0xce, 0x1F, MSM_CAMERA_I2C_BURST_DATA},
  {0xcf, 0x2D, MSM_CAMERA_I2C_BURST_DATA},
  {0xd0, 0x3E, MSM_CAMERA_I2C_BURST_DATA},
  {0xd1, 0x4A, MSM_CAMERA_I2C_BURST_DATA},
  {0xd2, 0x55, MSM_CAMERA_I2C_BURST_DATA},
  {0xd3, 0x5F, MSM_CAMERA_I2C_BURST_DATA},
  {0xd4, 0x67, MSM_CAMERA_I2C_BURST_DATA},
  {0xd5, 0x70, MSM_CAMERA_I2C_BURST_DATA},
  {0xd6, 0x78, MSM_CAMERA_I2C_BURST_DATA},
  {0xd7, 0x80, MSM_CAMERA_I2C_BURST_DATA},
  {0xd8, 0x86, MSM_CAMERA_I2C_BURST_DATA},
  {0xd9, 0x8C, MSM_CAMERA_I2C_BURST_DATA},
  {0xda, 0x92, MSM_CAMERA_I2C_BURST_DATA},
  {0xdb, 0x97, MSM_CAMERA_I2C_BURST_DATA},
  {0xdc, 0x9C, MSM_CAMERA_I2C_BURST_DATA},
  {0xdd, 0xA1, MSM_CAMERA_I2C_BURST_DATA},
  {0xde, 0xAA, MSM_CAMERA_I2C_BURST_DATA},
  {0xdf, 0xB2, MSM_CAMERA_I2C_BURST_DATA},
  {0xe0, 0xB9, MSM_CAMERA_I2C_BURST_DATA},
  {0xe1, 0xC6, MSM_CAMERA_I2C_BURST_DATA},
  {0xe2, 0xD0, MSM_CAMERA_I2C_BURST_DATA},
  {0xe3, 0xD8, MSM_CAMERA_I2C_BURST_DATA},
  {0xe4, 0xDF, MSM_CAMERA_I2C_BURST_DATA},
  {0xe5, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
  {0xe6, 0xEC, MSM_CAMERA_I2C_BURST_DATA},
  {0xe7, 0xF1, MSM_CAMERA_I2C_BURST_DATA},
  {0xe8, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
  {0xe9, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
  {0xea, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

  {0xeb, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GMA11
  {0xec, 0x03, MSM_CAMERA_I2C_BURST_DATA},
  {0xed, 0x08, MSM_CAMERA_I2C_BURST_DATA},
  {0xee, 0x12, MSM_CAMERA_I2C_BURST_DATA},
  {0xef, 0x19, MSM_CAMERA_I2C_BURST_DATA},
  {0xf0, 0x25, MSM_CAMERA_I2C_BURST_DATA},
  {0xf1, 0x32, MSM_CAMERA_I2C_BURST_DATA},
  {0xf2, 0x3E, MSM_CAMERA_I2C_BURST_DATA},
  {0xf3, 0x4B, MSM_CAMERA_I2C_BURST_DATA},
  {0xf4, 0x56, MSM_CAMERA_I2C_BURST_DATA},
  {0xf5, 0x62, MSM_CAMERA_I2C_BURST_DATA},
  {0xf6, 0x6A, MSM_CAMERA_I2C_BURST_DATA},
  {0xf7, 0x71, MSM_CAMERA_I2C_BURST_DATA},
  {0xf8, 0x78, MSM_CAMERA_I2C_BURST_DATA},
  {0xf9, 0x7F, MSM_CAMERA_I2C_BURST_DATA},
  {0xfa, 0x85, MSM_CAMERA_I2C_BURST_DATA},
  {0xfb, 0x8A, MSM_CAMERA_I2C_BURST_DATA},
  {0xfc, 0x90, MSM_CAMERA_I2C_BURST_DATA},
  {0xfd, 0x95, MSM_CAMERA_I2C_BURST_DATA},
  {0x03, 0xd1, MSM_CAMERA_I2C_BYTE_DATA}, //Page d1
  {0x10, 0x9A, MSM_CAMERA_I2C_BURST_DATA},
  {0x11, 0x9F, MSM_CAMERA_I2C_BURST_DATA},
  {0x12, 0xA9, MSM_CAMERA_I2C_BURST_DATA},
  {0x13, 0xB1, MSM_CAMERA_I2C_BURST_DATA},
  {0x14, 0xB9, MSM_CAMERA_I2C_BURST_DATA},
  {0x15, 0xC6, MSM_CAMERA_I2C_BURST_DATA},
  {0x16, 0xD0, MSM_CAMERA_I2C_BURST_DATA},
  {0x17, 0xD8, MSM_CAMERA_I2C_BURST_DATA},
  {0x18, 0xDF, MSM_CAMERA_I2C_BURST_DATA},
  {0x19, 0xE6, MSM_CAMERA_I2C_BURST_DATA},
  {0x1a, 0xEC, MSM_CAMERA_I2C_BURST_DATA},
  {0x1b, 0xF1, MSM_CAMERA_I2C_BURST_DATA},
  {0x1c, 0xF7, MSM_CAMERA_I2C_BURST_DATA},
  {0x1d, 0xFB, MSM_CAMERA_I2C_BURST_DATA},
  {0x1e, 0xFF, MSM_CAMERA_I2C_BURST_DATA},

	///////////////////////////////////////////
	// D1 Page Adaptive Y Target delta
	///////////////////////////////////////////
	{0x1f, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 0
	{0x20, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 1
	{0x21, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 2
	{0x22, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 3
	{0x23, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 4
	{0x24, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 5
	{0x25, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 6
	{0x26, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 7
	{0x27, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 8
	{0x28, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 9
	{0x29, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 10
	{0x2a, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Y target delta 11
	///////////////////////////////////////////
	// D1 Page Adaptive R/B saturation
	///////////////////////////////////////////
	{0x2b, 0x90, MSM_CAMERA_I2C_BURST_DATA},//SATB_00 STEVE for Low Light
	{0x2c, 0x90, MSM_CAMERA_I2C_BURST_DATA},//SATB_01 STEVE for Low Light
	{0x2d, 0x90, MSM_CAMERA_I2C_BURST_DATA},//SATB_02 STEVE for Low Light
	{0x2e, 0x98, MSM_CAMERA_I2C_BURST_DATA},//SATB_03
	{0x2f, 0x98, MSM_CAMERA_I2C_BURST_DATA},//SATB_04
	{0x30, 0x98, MSM_CAMERA_I2C_BURST_DATA},//SATB_05
	{0x31, 0xa0, MSM_CAMERA_I2C_BURST_DATA},//SATB_06
	{0x32, 0xa0, MSM_CAMERA_I2C_BURST_DATA},//SATB_07
	{0x33, 0xa0, MSM_CAMERA_I2C_BURST_DATA},//SATB_08
	{0x34, 0xa8, MSM_CAMERA_I2C_BURST_DATA},//SATB_09
	{0x35, 0xa8, MSM_CAMERA_I2C_BURST_DATA},//SATB_10
	{0x36, 0xa8, MSM_CAMERA_I2C_BURST_DATA},//SATB_11

	//Cr

	{0x37, 0x90, MSM_CAMERA_I2C_BURST_DATA},//SATR_00 STEVE for Low Light
	{0x38, 0x90, MSM_CAMERA_I2C_BURST_DATA},//SATR_01 STEVE for Low Light
	{0x39, 0x98, MSM_CAMERA_I2C_BURST_DATA},//SATR_02 STEVE for Low Light
	{0x3a, 0x98, MSM_CAMERA_I2C_BURST_DATA},//SATR_03
	{0x3b, 0x98, MSM_CAMERA_I2C_BURST_DATA},//SATR_04
	{0x3c, 0x98, MSM_CAMERA_I2C_BURST_DATA},//SATR_05
	{0x3d, 0xa0, MSM_CAMERA_I2C_BURST_DATA},//SATR_06
	{0x3e, 0xa0, MSM_CAMERA_I2C_BURST_DATA},//SATR_07
	{0x3f, 0xa0, MSM_CAMERA_I2C_BURST_DATA},//SATR_08
	{0x40, 0xa8, MSM_CAMERA_I2C_BURST_DATA},//SATR_09
	{0x41, 0xa8, MSM_CAMERA_I2C_BURST_DATA},//SATR_10
	{0x42, 0xa8, MSM_CAMERA_I2C_BURST_DATA},//SATR_11

//LGE_CHANGE_E
	///////////////////////////////////////////
	// D1 Page Adaptive CMC
	///////////////////////////////////////////

	{0x43, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_00
	{0x44, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x45, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x47, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0x48, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0x49, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x4b, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0x4d, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_01
	{0x4e, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0x4f, 0x2b, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0x51, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x70, MSM_CAMERA_I2C_BURST_DATA},
	{0x53, 0x0b, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0x2a, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0x57, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_02
	{0x58, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0x2b, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0x5b, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x70, MSM_CAMERA_I2C_BURST_DATA},
	{0x5d, 0x0b, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x5f, 0x2a, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0x61, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_03
	{0x62, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x63, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0x67, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x69, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0x6b, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_04
	{0x6c, 0x65, MSM_CAMERA_I2C_BURST_DATA},
	{0x6d, 0x2b, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0x6f, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0x70, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0x71, 0x13, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x09, MSM_CAMERA_I2C_BURST_DATA},
	{0x73, 0x2a, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0x75, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_05
	{0x76, 0x65, MSM_CAMERA_I2C_BURST_DATA},
	{0x77, 0x2b, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0x79, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0x7b, 0x13, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x09, MSM_CAMERA_I2C_BURST_DATA},
	{0x7d, 0x2a, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0x7f, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_06
	{0x80, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x81, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x83, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0x85, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x87, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0x89, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_07
	{0x8a, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x8b, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x8d, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0x8f, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x91, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0x93, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_08
	{0x94, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x95, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x97, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0x99, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x9b, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0x9d, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_09
	{0x9e, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x9f, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xa0, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xa1, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0xa3, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xa5, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0xa7, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_10
	{0xa8, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0xa9, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xab, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0xad, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xaf, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xb0, 0x73, MSM_CAMERA_I2C_BURST_DATA},

	{0xb1, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//CMC_11
	{0xb2, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0xb3, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xb5, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0xb7, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xb9, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x73, MSM_CAMERA_I2C_BURST_DATA},
	///////////////////////////////////////////
	// D1~D2~D3 Page Adaptive Multi-CMC
	///////////////////////////////////////////
//LGE_CHANGE_S MR LOW Light Tuning
	//MCMC_00
	{0xbb, 0x80, MSM_CAMERA_I2C_BURST_DATA},//GLB_GAIN
	{0xbc, 0x00, MSM_CAMERA_I2C_BURST_DATA},//GLB_HUE
	{0xbd, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0xbe, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0xbf, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0xc0, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0xc1, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0xc2, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0xc3, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0xc4, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0xc5, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0xc6, 0x07, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0xc7, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0xc8, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0xc9, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0xca, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0xcb, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0xcc, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0xcd, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0xce, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0xcf, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0xd0, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0xd1, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0xd2, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0xd3, 0x1b, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0xd4, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA
	//MCMC_01
	{0xd5, 0x80, MSM_CAMERA_I2C_BURST_DATA},//GLB_GAIN
	{0xd6, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0xd7, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0xd8, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0xd9, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0xda, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0xdb, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0xdc, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0xdd, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0xde, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0xdf, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0xe0, 0x07, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0xe1, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0xe2, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0xe3, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0xe4, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0xe5, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0xe6, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0xe7, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0xe8, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0xe9, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0xea, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0xeb, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0xec, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0xed, 0x1b, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0xee, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA
	//MCMC_02
	{0xef, 0x80, MSM_CAMERA_I2C_BURST_DATA},//GLB_GAIN
	{0xf0, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0xf1, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0xf2, 0x06, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0xf3, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0xf4, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0xf5, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0xf6, 0x16, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0xf7, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0xf8, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0xf9, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0xfa, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0xfb, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0xfc, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0xfd, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN

	{0x03, 0xd2, MSM_CAMERA_I2C_BYTE_DATA},//Page d2
	{0x10, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0x11, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0x12, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0x13, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0x14, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0x15, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0x16, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0x17, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0x18, 0x09, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0x19, 0x22, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0x1a, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA

	//MCMC_03
	{0x1b, 0x80, MSM_CAMERA_I2C_BURST_DATA},//GLB_GAIN
	{0x1c, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0x1d, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0x1e, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0x1f, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0x20, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0x21, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0x22, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0x23, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0x24, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0x25, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0x26, 0x07, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0x27, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0x28, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0x29, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0x2a, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0x2b, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0x2c, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0x2d, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0x2e, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0x2f, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0x30, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0x31, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0x32, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0x33, 0x1b, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0x34, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA

	//MCMC_04
	{0x35, 0x80, MSM_CAMERA_I2C_BURST_DATA},//GLB_GAIN
	{0x36, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0x37, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0x38, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0x39, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0x3a, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0x3b, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0x3c, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0x3d, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0x3e, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0x3f, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0x40, 0x07, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0x41, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0x42, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0x43, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0x44, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0x45, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0x46, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0x47, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0x48, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0x49, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0x4a, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0x4b, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0x4c, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0x4d, 0x1b, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0x4e, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA

	//MCMC_05
	{0x4f, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //GLB_GAIN
	{0x50, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0x51, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0x52, 0x06, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0x53, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0x54, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0x55, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0x56, 0x16, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0x57, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0x58, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0x59, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0x5a, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0x5b, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0x5c, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0x5d, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0x5e, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0x5f, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0x60, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0x61, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0x62, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0x63, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0x64, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0x65, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0x66, 0x09, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0x67, 0x22, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0x68, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA

	//MCMC_06
	{0x69, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //GLB_GAIN
	{0x6a, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0x6b, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0x6c, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0x6d, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0x6e, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0x6f, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0x70, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0x71, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0x72, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0x73, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0x74, 0x07, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0x75, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0x76, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0x77, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0x78, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0x79, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0x7a, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0x7b, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0x7c, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0x7d, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0x7e, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0x7f, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0x80, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0x81, 0x1b, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0x82, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA

	//MCMC_07
	{0x83, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //GLB_GAIN
	{0x84, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0x85, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0x86, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0x87, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0x88, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0x89, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0x8a, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0x8b, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0x8c, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0x8d, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0x8e, 0x07, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0x8f, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0x90, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0x91, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0x92, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0x93, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0x94, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0x95, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0x96, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0x97, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0x98, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0x99, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0x9a, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0x9b, 0x1b, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0x9c, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA

	//MCMC_08
	{0x9d, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //GLB_GAIN
	{0x9e, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0x9f, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0xa0, 0x06, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0xa1, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0xa2, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0xa3, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0xa4, 0x16, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0xa5, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0xa6, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0xa7, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0xa8, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0xa9, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0xaa, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0xab, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0xac, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0xad, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0xae, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0xaf, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0xb0, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0xb1, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0xb2, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0xb3, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0xb4, 0x09, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0xb5, 0x22, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0xb6, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA

	//MCMC_09
	{0xb7, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //GLB_GAIN
	{0xb8, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0xb9, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0xba, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0xbb, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0xbc, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0xbd, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0xbe, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0xbf, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0xc0, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0xc1, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0xc2, 0x07, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0xc3, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0xc4, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0xc5, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0xc6, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0xc7, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0xc8, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0xc9, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0xca, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0xcb, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0xcc, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0xcd, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0xce, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0xcf, 0x1b, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0xd0, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA

	//MCMC_10
	{0xd1, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //GLB_GAIN
	{0xd2, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0xd3, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0xd4, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0xd5, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0xd6, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0xd7, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0xd8, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0xd9, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0xda, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0xdb, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0xdc, 0x07, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0xdd, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0xde, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0xdf, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0xe0, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0xe1, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0xe2, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0xe3, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN
	{0xe4, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0xe5, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0xe6, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0xe7, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0xe8, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0xe9, 0x1b, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0xea, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA

	//MCMC_11
	{0xeb, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //GLB_GAIN
	{0xec, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //GLB_HUE
	{0xed, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //0_GAIN
	{0xee, 0x06, MSM_CAMERA_I2C_BURST_DATA}, //0_HUE
	{0xef, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //0_CENTER
	{0xf0, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //0_DELTA
	{0xf1, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //1_GAIN
	{0xf2, 0x16, MSM_CAMERA_I2C_BURST_DATA}, //1_HUE
	{0xf3, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //1_CENTER
	{0xf4, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //1_DELTA
	{0xf5, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //2_GAIN
	{0xf6, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, //2_HUE
	{0xf7, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //2_CENTER
	{0xf8, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //2_DELTA
	{0xf9, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //3_GAIN
	{0xfa, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //3_HUE
	{0xfb, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, //3_CENTER
	{0xfc, 0x0e, MSM_CAMERA_I2C_BURST_DATA}, //3_DELTA
	{0xfd, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //4_GAIN


	{0x03, 0xd3, MSM_CAMERA_I2C_BYTE_DATA},//Page d3

	{0x10, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //4_HUE
	{0x11, 0xa5, MSM_CAMERA_I2C_BURST_DATA}, //4_CENTER
	{0x12, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //4_DELTA
	{0x13, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //5_GAIN
	{0x14, 0x09, MSM_CAMERA_I2C_BURST_DATA}, //5_HUE
	{0x15, 0x22, MSM_CAMERA_I2C_BURST_DATA}, //5_CENTER
	{0x16, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //5_DELTA

	///////////////////////////////////////////
	// D3 Page Adaptive LSC
	///////////////////////////////////////////

	{0x17, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 00 ofs GB
	{0x18, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 00 ofs B
	{0x19, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 00 ofs R
	{0x1a, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 00 ofs GR

	{0x1b, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //LSC 00 Gain GB
	{0x1c, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //LSC 00 Gain B
	{0x1d, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //LSC 00 Gain R
	{0x1e, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //LSC 00 Gain GR

	{0x1f, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 01 ofs GB
	{0x20, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 01 ofs B
	{0x21, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 01 ofs R
	{0x22, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 01 ofs GR
	{0x23, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //LSC 01 Gain GB
	{0x24, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //LSC 01 Gain B
	{0x25, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //LSC 01 Gain R
	{0x26, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //LSC 01 Gain GR

	{0x27, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 02 ofs GB
	{0x28, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 02 ofs B
	{0x29, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 02 ofs R
	{0x2a, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 02 ofs GR
	{0x2b, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //LSC 02 Gain GB
	{0x2c, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //LSC 02 Gain B
	{0x2d, 0x48, MSM_CAMERA_I2C_BURST_DATA}, //LSC 02 Gain R
	{0x2e, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //LSC 02 Gain GR

	{0x2f, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 03 ofs GB
	{0x30, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 03 ofs B
	{0x31, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 03 ofs R
	{0x32, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 03 ofs GR
	{0x33, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //LSC 03 Gain GB
	{0x34, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //LSC 03 Gain B
	{0x35, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //LSC 03 Gain R
	{0x36, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //LSC 03 Gain GR

	{0x37, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 04 ofs GB
	{0x38, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 04 ofs B
	{0x39, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 04 ofs R
	{0x3a, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 04 ofs GR
	{0x3b, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //LSC 04 Gain GB
	{0x3c, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //LSC 04 Gain B
	{0x3d, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //LSC 04 Gain R
	{0x3e, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //LSC 04 Gain GR

	{0x3f, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 05 ofs GB
	{0x40, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 05 ofs B
	{0x41, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 05 ofs R
	{0x42, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 05 ofs GR
	{0x43, 0x80, MSM_CAMERA_I2C_BURST_DATA},//LSC 05 Gain GB
	{0x44, 0x78, MSM_CAMERA_I2C_BURST_DATA},//LSC 05 Gain B
	{0x45, 0xA0, MSM_CAMERA_I2C_BURST_DATA}, //LSC 05 Gain R
	{0x46, 0x80, MSM_CAMERA_I2C_BURST_DATA},//LSC 05 Gain GR

	{0x47, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 06 ofs GB
	{0x48, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 06 ofs B
	{0x49, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 06 ofs R
	{0x4a, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 06 ofs GR
	{0x4b, 0x80, MSM_CAMERA_I2C_BURST_DATA},//78 LSC 06 Gain GB
	{0x4c, 0x80, MSM_CAMERA_I2C_BURST_DATA},//7c LSC 06 Gain B
	{0x4d, 0x80, MSM_CAMERA_I2C_BURST_DATA},//80 LSC 06 Gain R
	{0x4e, 0x80, MSM_CAMERA_I2C_BURST_DATA},//78 LSC 06 Gain GR

	{0x4f, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 07 ofs GB
	{0x50, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 07 ofs B
	{0x51, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 07 ofs R
	{0x52, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 07 ofs GR
	{0x53, 0x80, MSM_CAMERA_I2C_BURST_DATA},//78 LSC 07 Gain GB
	{0x54, 0x80, MSM_CAMERA_I2C_BURST_DATA},//7c LSC 07 Gain B
	{0x55, 0x80, MSM_CAMERA_I2C_BURST_DATA},//80 LSC 07 Gain R
	{0x56, 0x80, MSM_CAMERA_I2C_BURST_DATA},//78 LSC 07 Gain GR

	{0x57, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 08 ofs GB
	{0x58, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 08 ofs B
	{0x59, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 08 ofs R
	{0x5a, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 08 ofs GR
	{0x5b, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //78 LSC 08 Gain GB
	{0x5c, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //7c LSC 08 Gain B
	{0x5d, 0xA0, MSM_CAMERA_I2C_BURST_DATA}, //80 LSC 08 Gain R
	{0x5e, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //78 LSC 08 Gain GR

	{0x5f, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 09 ofs GB
	{0x60, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 09 ofs B
	{0x61, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 09 ofs R
	{0x62, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 09 ofs GR
	{0x63, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //78 LSC 09 Gain GB
	{0x64, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //7c LSC 09 Gain B
	{0x65, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //80 LSC 09 Gain R
	{0x66, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //78 LSC 09 Gain GR

	{0x67, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 10 ofs GB
	{0x68, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 10 ofs B
	{0x69, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 10 ofs R
	{0x6a, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 10 ofs GR
	{0x6b, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //78 LSC 10 Gain GB
	{0x6c, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //7c LSC 10 Gain B
	{0x6d, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //80 LSC 10 Gain R
	{0x6e, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //78 LSC 10 Gain GR

	{0x6f, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 11 ofs GB
	{0x70, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 11 ofs B
	{0x71, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 11 ofs R
	{0x72, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC 11 ofs GR
	{0x73, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //78 LSC 11 Gain GB
	{0x74, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //7c LSC 11 Gain B
	{0x75, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //80 LSC 11 Gain R
	{0x76, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //78 LSC 11 Gain GR

//LGE_CHANGE_E
	///////////////////////////////////////////
	// D3 Page OTP, ROM Select TH
	///////////////////////////////////////////
	{0x77, 0x60, MSM_CAMERA_I2C_BURST_DATA}, //2 ROM High
	{0x78, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //2 ROM Low
	{0x79, 0x60, MSM_CAMERA_I2C_BURST_DATA}, //3 OTP High
	{0x7a, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //3 OTP Mid
	{0x7b, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //3 OTP Low
	///////////////////////////////////////////
	// D3 Page Adaptive DNP
	///////////////////////////////////////////
	{0x7c, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC EV max
	{0x7d, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0x7f, 0xf1, MSM_CAMERA_I2C_BURST_DATA},

	{0x80, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC EV min
	{0x81, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0x83, 0xf1, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //CTEM max
	{0x85, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //CTEM min
	{0x86, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //Y STD max
	{0x87, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //Y STD min

	{0x88, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //LSC offset
	{0x89, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x8b, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //LSC gain
	{0x8d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x8f, 0x80, MSM_CAMERA_I2C_BURST_DATA},

	{0x90, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //DNP CB
	{0x91, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //DNP CR

	///////////////////////////////////
	//Page 0xD9 DMA EXTRA
	///////////////////////////////////

	{0x03, 0xd9, MSM_CAMERA_I2C_BYTE_DATA},

	{0x10, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x11, 0x10, MSM_CAMERA_I2C_BURST_DATA},//Page 10
	{0x12, 0x61, MSM_CAMERA_I2C_BURST_DATA},
	{0x13, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0x15, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x17, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x18, 0x48, MSM_CAMERA_I2C_BURST_DATA},
	{0x19, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x03, MSM_CAMERA_I2C_BURST_DATA},//Page 16
	{0x1b, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x1d, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x31, MSM_CAMERA_I2C_BURST_DATA},
	{0x1f, 0x42, MSM_CAMERA_I2C_BURST_DATA},

	{0x20, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x21, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0x23, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0x25, 0x7b, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x35, MSM_CAMERA_I2C_BURST_DATA},
	{0x27, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0x28, 0x36, MSM_CAMERA_I2C_BURST_DATA},
	{0x29, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x37, MSM_CAMERA_I2C_BURST_DATA},
	{0x2b, 0x43, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0x2d, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x2e, 0x70, MSM_CAMERA_I2C_BURST_DATA},
	{0x2f, 0x80, MSM_CAMERA_I2C_BURST_DATA},

	{0x30, 0x71, MSM_CAMERA_I2C_BURST_DATA},
	{0x31, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x72, MSM_CAMERA_I2C_BURST_DATA},
	{0x33, 0x9b, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x73, MSM_CAMERA_I2C_BURST_DATA},
	{0x35, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x74, MSM_CAMERA_I2C_BURST_DATA},
	{0x37, 0x34, MSM_CAMERA_I2C_BURST_DATA},

	{0x38, 0x75, MSM_CAMERA_I2C_BURST_DATA},
	{0x39, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x76, MSM_CAMERA_I2C_BURST_DATA},
	{0x3b, 0xa6, MSM_CAMERA_I2C_BURST_DATA},
	{0x3c, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x3d, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x3e, 0x78, MSM_CAMERA_I2C_BURST_DATA},
	{0x3f, 0x69, MSM_CAMERA_I2C_BURST_DATA},

	{0x40, 0x79, MSM_CAMERA_I2C_BURST_DATA},
	{0x41, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0x42, 0x7a, MSM_CAMERA_I2C_BURST_DATA},
	{0x43, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x7b, MSM_CAMERA_I2C_BURST_DATA},
	{0x45, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x7c, MSM_CAMERA_I2C_BURST_DATA},
	{0x47, 0xad, MSM_CAMERA_I2C_BURST_DATA},

	{0x48, 0x7d, MSM_CAMERA_I2C_BURST_DATA},
	{0x49, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x7e, MSM_CAMERA_I2C_BURST_DATA},
	{0x4b, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x4d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x4f, 0x51, MSM_CAMERA_I2C_BURST_DATA},

	{0x50, 0x81, MSM_CAMERA_I2C_BURST_DATA},
	{0x51, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x82, MSM_CAMERA_I2C_BURST_DATA},
	{0x53, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x83, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0x0c, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x57, 0x23, MSM_CAMERA_I2C_BURST_DATA},

	{0x58, 0x85, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x86, MSM_CAMERA_I2C_BURST_DATA},
	{0x5b, 0xb3, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0x5d, 0x8a, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x88, MSM_CAMERA_I2C_BURST_DATA},
	{0x5f, 0x52, MSM_CAMERA_I2C_BURST_DATA},

	{0x60, 0x89, MSM_CAMERA_I2C_BURST_DATA},
	{0x61, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x03, MSM_CAMERA_I2C_BURST_DATA},//Page 17 Gamma
	{0x63, 0x17, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0x67, 0x02, MSM_CAMERA_I2C_BURST_DATA},

	{0x68, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x69, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x23, MSM_CAMERA_I2C_BURST_DATA},
	{0x6b, 0x09, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x6d, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x25, MSM_CAMERA_I2C_BURST_DATA},
	{0x6f, 0x23, MSM_CAMERA_I2C_BURST_DATA},

	{0x70, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0x71, 0x37, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0x73, 0x47, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x75, 0x57, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0x77, 0x61, MSM_CAMERA_I2C_BURST_DATA},

	{0x78, 0x2a, MSM_CAMERA_I2C_BURST_DATA},
	{0x79, 0x6b, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x2b, MSM_CAMERA_I2C_BURST_DATA},
	{0x7b, 0x71, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x2c, MSM_CAMERA_I2C_BURST_DATA},
	{0x7d, 0x76, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x2d, MSM_CAMERA_I2C_BURST_DATA},
	{0x7f, 0x7a, MSM_CAMERA_I2C_BURST_DATA},

	{0x80, 0x2e, MSM_CAMERA_I2C_BURST_DATA},
	{0x81, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x2f, MSM_CAMERA_I2C_BURST_DATA},
	{0x83, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x85, 0x88, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0x31, MSM_CAMERA_I2C_BURST_DATA},
	{0x87, 0x8c, MSM_CAMERA_I2C_BURST_DATA},

	{0x88, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x89, 0x91, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0x8b, 0x94, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0x8d, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x35, MSM_CAMERA_I2C_BURST_DATA},
	{0x8f, 0x9f, MSM_CAMERA_I2C_BURST_DATA},

	{0x90, 0x36, MSM_CAMERA_I2C_BURST_DATA},
	{0x91, 0xa6, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0x37, MSM_CAMERA_I2C_BURST_DATA},
	{0x93, 0xae, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0x95, 0xbb, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0x39, MSM_CAMERA_I2C_BURST_DATA},
	{0x97, 0xc9, MSM_CAMERA_I2C_BURST_DATA},

	{0x98, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x99, 0xd3, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0x3b, MSM_CAMERA_I2C_BURST_DATA},
	{0x9b, 0xdc, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0x3c, MSM_CAMERA_I2C_BURST_DATA},
	{0x9d, 0xe2, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0x3d, MSM_CAMERA_I2C_BURST_DATA},
	{0x9f, 0xe8, MSM_CAMERA_I2C_BURST_DATA},

	{0xa0, 0x3e, MSM_CAMERA_I2C_BURST_DATA},
	{0xa1, 0xed, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0xa3, 0xf4, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0xa5, 0xfa, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0x41, MSM_CAMERA_I2C_BURST_DATA},
	{0xa7, 0xff, MSM_CAMERA_I2C_BURST_DATA},

	{0xa8, 0x03, MSM_CAMERA_I2C_BURST_DATA},//page 20 AE
	{0xa9, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0x39, MSM_CAMERA_I2C_BURST_DATA},
	{0xab, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0x03, MSM_CAMERA_I2C_BURST_DATA},//Page 15 SHD
	{0xad, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0xaf, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0xb0, 0x25, MSM_CAMERA_I2C_BURST_DATA},
	{0xb1, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0xb3, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0xb5, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xb7, 0x80, MSM_CAMERA_I2C_BURST_DATA},

	{0xb8, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0xb9, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x2a, MSM_CAMERA_I2C_BURST_DATA},
	{0xbb, 0x7a, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x2b, MSM_CAMERA_I2C_BURST_DATA},
	{0xbd, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0xbf, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	///////////////////////////////////
	// Page 0xDA(DMA Outdoor)
	///////////////////////////////////
	{0x03, 0xda, MSM_CAMERA_I2C_BYTE_DATA},

	{0x10, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x11, 0x11, MSM_CAMERA_I2C_BURST_DATA},//11 page
	{0x12, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x13, 0x1f, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x15, 0x25, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x17, 0x22, MSM_CAMERA_I2C_BURST_DATA},

	{0x18, 0x13, MSM_CAMERA_I2C_BURST_DATA},
	{0x19, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x1b, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x1d, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x31, MSM_CAMERA_I2C_BURST_DATA},
	{0x1f, 0x20, MSM_CAMERA_I2C_BURST_DATA},

	{0x20, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1132 //STEVE Lum. Level. in DLPF
	{0x21, 0x8b, MSM_CAMERA_I2C_BURST_DATA}, //52
	{0x22, 0x33, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1133
	{0x23, 0x54, MSM_CAMERA_I2C_BURST_DATA}, //3b
	{0x24, 0x34, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1134
	{0x25, 0x2c, MSM_CAMERA_I2C_BURST_DATA}, //1d
	{0x26, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1135
	{0x27, 0x29, MSM_CAMERA_I2C_BURST_DATA},	//21
	{0x28, 0x36, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1136
	{0x29, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //1b
	{0x2a, 0x37, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1137
	{0x2b, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //21
	{0x2c, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1138
	{0x2d, 0x17, MSM_CAMERA_I2C_BURST_DATA}, //18

	{0x2e, 0x39, MSM_CAMERA_I2C_BURST_DATA},
	{0x2f, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x30, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x31, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x3b, MSM_CAMERA_I2C_BURST_DATA},
	{0x33, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x3c, MSM_CAMERA_I2C_BURST_DATA},
	{0x35, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x3d, MSM_CAMERA_I2C_BURST_DATA},
	{0x37, 0x28, MSM_CAMERA_I2C_BURST_DATA},

	{0x38, 0x3e, MSM_CAMERA_I2C_BURST_DATA},
	{0x39, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0x3b, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0x3c, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x3d, 0x3c, MSM_CAMERA_I2C_BURST_DATA},
	{0x3e, 0x41, MSM_CAMERA_I2C_BURST_DATA},
	{0x3f, 0x28, MSM_CAMERA_I2C_BURST_DATA},

	{0x40, 0x42, MSM_CAMERA_I2C_BURST_DATA},
	{0x41, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x42, 0x43, MSM_CAMERA_I2C_BURST_DATA},
	{0x43, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0x45, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x45, MSM_CAMERA_I2C_BURST_DATA},
	{0x47, 0x28, MSM_CAMERA_I2C_BURST_DATA},

	{0x48, 0x46, MSM_CAMERA_I2C_BURST_DATA},
	{0x49, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x47, MSM_CAMERA_I2C_BURST_DATA},
	{0x4b, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0x48, MSM_CAMERA_I2C_BURST_DATA},
	{0x4d, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0x49, MSM_CAMERA_I2C_BURST_DATA},
	{0x4f, 0xf0, MSM_CAMERA_I2C_BURST_DATA},

	{0x50, 0x4a, MSM_CAMERA_I2C_BURST_DATA},
	{0x51, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x4b, MSM_CAMERA_I2C_BURST_DATA},
	{0x53, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x4c, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x4d, MSM_CAMERA_I2C_BURST_DATA},
	{0x57, 0xf0, MSM_CAMERA_I2C_BURST_DATA},

	{0x58, 0x4e, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x4f, MSM_CAMERA_I2C_BURST_DATA},
	{0x5b, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0x5d, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x51, MSM_CAMERA_I2C_BURST_DATA},
	{0x5f, 0xf0, MSM_CAMERA_I2C_BURST_DATA},

	{0x60, 0x52, MSM_CAMERA_I2C_BURST_DATA},
	{0x61, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x53, MSM_CAMERA_I2C_BURST_DATA},
	{0x63, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x54, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x67, 0xf0, MSM_CAMERA_I2C_BURST_DATA},

	{0x68, 0x56, MSM_CAMERA_I2C_BURST_DATA},
	{0x69, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x57, MSM_CAMERA_I2C_BURST_DATA},
	{0x6b, 0xe8, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x58, MSM_CAMERA_I2C_BURST_DATA},
	{0x6d, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x59, MSM_CAMERA_I2C_BURST_DATA},
	{0x6f, 0xfc, MSM_CAMERA_I2C_BURST_DATA},

	{0x70, 0x5a, MSM_CAMERA_I2C_BURST_DATA},
	{0x71, 0xf8, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x5b, MSM_CAMERA_I2C_BURST_DATA},
	{0x73, 0xf2, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x5c, MSM_CAMERA_I2C_BURST_DATA},
	{0x75, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x5d, MSM_CAMERA_I2C_BURST_DATA},
	{0x77, 0xf0, MSM_CAMERA_I2C_BURST_DATA},

	{0x78, 0x5e, MSM_CAMERA_I2C_BURST_DATA},
	{0x79, 0xec, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x5f, MSM_CAMERA_I2C_BURST_DATA},
	{0x7b, 0xe8, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x7d, 0xe4, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x61, MSM_CAMERA_I2C_BURST_DATA},
	{0x7f, 0xf0, MSM_CAMERA_I2C_BURST_DATA},

	{0x80, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0x81, 0xfc, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x63, MSM_CAMERA_I2C_BURST_DATA},
	{0x83, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0x85, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0x65, MSM_CAMERA_I2C_BURST_DATA},
	{0x87, 0x30, MSM_CAMERA_I2C_BURST_DATA},

	{0x88, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x89, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0x67, MSM_CAMERA_I2C_BURST_DATA},
	{0x8b, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x68, MSM_CAMERA_I2C_BURST_DATA},
	{0x8d, 0x5a, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x69, MSM_CAMERA_I2C_BURST_DATA},
	{0x8f, 0x24, MSM_CAMERA_I2C_BURST_DATA},

	{0x90, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x91, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0x6b, MSM_CAMERA_I2C_BURST_DATA},
	{0x93, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0x95, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0x6d, MSM_CAMERA_I2C_BURST_DATA},
	{0x97, 0x5c, MSM_CAMERA_I2C_BURST_DATA},

	{0x98, 0x6e, MSM_CAMERA_I2C_BURST_DATA},
	{0x99, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0x6f, MSM_CAMERA_I2C_BURST_DATA},
	{0x9b, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0x70, MSM_CAMERA_I2C_BURST_DATA},
	{0x9d, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0x71, MSM_CAMERA_I2C_BURST_DATA},
	{0x9f, 0x20, MSM_CAMERA_I2C_BURST_DATA},

	{0xa0, 0x72, MSM_CAMERA_I2C_BURST_DATA},
	{0xa1, 0x5c, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0x73, MSM_CAMERA_I2C_BURST_DATA},
	{0xa3, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0x74, MSM_CAMERA_I2C_BURST_DATA},
	{0xa5, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0x75, MSM_CAMERA_I2C_BURST_DATA},
	{0xa7, 0x60, MSM_CAMERA_I2C_BURST_DATA},

	{0xa8, 0x76, MSM_CAMERA_I2C_BURST_DATA},
	{0xa9, 0x42, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0xab, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0x78, MSM_CAMERA_I2C_BURST_DATA},
	{0xad, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x79, MSM_CAMERA_I2C_BURST_DATA},
	{0xaf, 0x88, MSM_CAMERA_I2C_BURST_DATA},

	{0xb0, 0x7a, MSM_CAMERA_I2C_BURST_DATA},
	{0xb1, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x7b, MSM_CAMERA_I2C_BURST_DATA},
	{0xb3, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x7c, MSM_CAMERA_I2C_BURST_DATA},
	{0xb5, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x7d, MSM_CAMERA_I2C_BURST_DATA},
	{0xb7, 0x1c, MSM_CAMERA_I2C_BURST_DATA},

	{0xb8, 0x7e, MSM_CAMERA_I2C_BURST_DATA},
	{0xb9, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
	{0xbb, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xbd, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x81, MSM_CAMERA_I2C_BURST_DATA},
	{0xbf, 0x32, MSM_CAMERA_I2C_BURST_DATA},

	{0xc0, 0x82, MSM_CAMERA_I2C_BURST_DATA},
	{0xc1, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xc2, 0x83, MSM_CAMERA_I2C_BURST_DATA},
	{0xc3, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0xc5, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0x85, MSM_CAMERA_I2C_BURST_DATA},
	{0xc7, 0x10, MSM_CAMERA_I2C_BURST_DATA},

	{0xc8, 0x86, MSM_CAMERA_I2C_BURST_DATA},
	{0xc9, 0x1c, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0xcb, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x88, MSM_CAMERA_I2C_BURST_DATA},
	{0xcd, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x89, MSM_CAMERA_I2C_BURST_DATA},
	{0xcf, 0x34, MSM_CAMERA_I2C_BURST_DATA},

	{0xd0, 0x8a, MSM_CAMERA_I2C_BURST_DATA},
	{0xd1, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xd2, 0x90, MSM_CAMERA_I2C_BURST_DATA},
	{0xd3, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xd4, 0x91, MSM_CAMERA_I2C_BURST_DATA},
	{0xd5, 0x48, MSM_CAMERA_I2C_BURST_DATA},
	{0xd6, 0x92, MSM_CAMERA_I2C_BURST_DATA},
	{0xd7, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0xd8, 0x93, MSM_CAMERA_I2C_BURST_DATA},
	{0xd9, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xda, 0x94, MSM_CAMERA_I2C_BURST_DATA},
	{0xdb, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xdc, 0x95, MSM_CAMERA_I2C_BURST_DATA},
	{0xdd, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0xde, 0x96, MSM_CAMERA_I2C_BURST_DATA},
	{0xdf, 0x14, MSM_CAMERA_I2C_BURST_DATA},

	{0xe0, 0x97, MSM_CAMERA_I2C_BURST_DATA},
	{0xe1, 0x90, MSM_CAMERA_I2C_BURST_DATA},
	{0xe2, 0xb0, MSM_CAMERA_I2C_BURST_DATA},
	{0xe3, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0xe4, 0xb1, MSM_CAMERA_I2C_BURST_DATA},
	{0xe5, 0x90, MSM_CAMERA_I2C_BURST_DATA},
	{0xe6, 0xb2, MSM_CAMERA_I2C_BURST_DATA},
	{0xe7, 0x10, MSM_CAMERA_I2C_BURST_DATA},

	{0xe8, 0xb3, MSM_CAMERA_I2C_BURST_DATA},
	{0xe9, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xea, 0xb4, MSM_CAMERA_I2C_BURST_DATA},
	{0xeb, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xec, 0x03, MSM_CAMERA_I2C_BURST_DATA},//12 page
	{0xed, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0xee, 0x10, MSM_CAMERA_I2C_BURST_DATA},
  {0xef, 0x57, MSM_CAMERA_I2C_BURST_DATA}, //steve 1210 57

	{0xf0, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0xf1, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0xf2, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0xf3, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xf4, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0xf5, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0xf6, 0x41, MSM_CAMERA_I2C_BURST_DATA},
	{0xf7, 0x0a, MSM_CAMERA_I2C_BURST_DATA},

	{0xf8, 0x42, MSM_CAMERA_I2C_BURST_DATA},
	{0xf9, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0xfa, 0x43, MSM_CAMERA_I2C_BURST_DATA},
	{0xfb, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xfc, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0xfd, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	//{0xfe, 0x45, MSM_CAMERA_I2C_BURST_DATA}, // STEVE deleted
	//{0xff, 0x0a, MSM_CAMERA_I2C_BURST_DATA}, // STEVE deleted

	// Page 0xdb
	{0x03, 0xdb, MSM_CAMERA_I2C_BYTE_DATA},

	{0x10, 0x45, MSM_CAMERA_I2C_BURST_DATA},
	{0x11, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x46, MSM_CAMERA_I2C_BURST_DATA},
	{0x13, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x15, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x61, MSM_CAMERA_I2C_BURST_DATA},
	{0x17, 0x04, MSM_CAMERA_I2C_BURST_DATA},

	{0x18, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0x19, 0x4b, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x63, MSM_CAMERA_I2C_BURST_DATA},
	{0x1b, 0x41, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0x1d, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x65, MSM_CAMERA_I2C_BURST_DATA},
	{0x1f, 0x00, MSM_CAMERA_I2C_BURST_DATA},

	{0x20, 0x68, MSM_CAMERA_I2C_BURST_DATA},
	{0x21, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x69, MSM_CAMERA_I2C_BURST_DATA},
	{0x23, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x25, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x6b, MSM_CAMERA_I2C_BURST_DATA},
	{0x27, 0x0a, MSM_CAMERA_I2C_BURST_DATA},

	{0x28, 0x6c, MSM_CAMERA_I2C_BURST_DATA},
	{0x29, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x6d, MSM_CAMERA_I2C_BURST_DATA},
	{0x2b, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x70, MSM_CAMERA_I2C_BURST_DATA},
  {0x2d, 0x29, MSM_CAMERA_I2C_BURST_DATA}, //1270 29
  {0x2e, 0x71, MSM_CAMERA_I2C_BURST_DATA},
  {0x2f, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //1271 7f

  {0x30, 0x80, MSM_CAMERA_I2C_BURST_DATA},
  {0x31, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x32, 0x81, MSM_CAMERA_I2C_BURST_DATA},
  {0x33, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x34, 0x82, MSM_CAMERA_I2C_BURST_DATA},
  {0x35, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x36, 0x83, MSM_CAMERA_I2C_BURST_DATA},
  {0x37, 0x00, MSM_CAMERA_I2C_BURST_DATA},

  {0x38, 0x84, MSM_CAMERA_I2C_BURST_DATA},
  {0x39, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x3a, 0x85, MSM_CAMERA_I2C_BURST_DATA},
  {0x3b, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x3c, 0x86, MSM_CAMERA_I2C_BURST_DATA},
  {0x3d, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x3e, 0x87, MSM_CAMERA_I2C_BURST_DATA},
  {0x3f, 0x00, MSM_CAMERA_I2C_BURST_DATA},

  {0x40, 0x88, MSM_CAMERA_I2C_BURST_DATA},
  {0x41, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x42, 0x89, MSM_CAMERA_I2C_BURST_DATA},
  {0x43, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
  {0x44, 0x8a, MSM_CAMERA_I2C_BURST_DATA},
  {0x45, 0xb0, MSM_CAMERA_I2C_BURST_DATA},
  {0x46, 0x8b, MSM_CAMERA_I2C_BURST_DATA},
  {0x47, 0x08, MSM_CAMERA_I2C_BURST_DATA},

  {0x48, 0x8c, MSM_CAMERA_I2C_BURST_DATA},
  {0x49, 0x05, MSM_CAMERA_I2C_BURST_DATA},
  {0x4a, 0x8d, MSM_CAMERA_I2C_BURST_DATA},
  {0x4b, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0xe6, MSM_CAMERA_I2C_BURST_DATA},
	{0x4d, 0xff, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0xe7, MSM_CAMERA_I2C_BURST_DATA},
	{0x4f, 0x18, MSM_CAMERA_I2C_BURST_DATA},

	{0x50, 0xe8, MSM_CAMERA_I2C_BURST_DATA},
	{0x51, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0xe9, MSM_CAMERA_I2C_BURST_DATA},
	{0x53, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0x13, MSM_CAMERA_I2C_BURST_DATA},//13 page
	{0x56, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x57, 0x3f, MSM_CAMERA_I2C_BURST_DATA},

	{0x58, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0x5b, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x5d, 0x36, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x23, MSM_CAMERA_I2C_BURST_DATA},
	{0x5f, 0x6a, MSM_CAMERA_I2C_BURST_DATA},

	{0x60, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x61, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x25, MSM_CAMERA_I2C_BURST_DATA},
	{0x63, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0x67, 0x02, MSM_CAMERA_I2C_BURST_DATA},

	{0x68, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x69, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0x6b, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x2a, MSM_CAMERA_I2C_BURST_DATA},
	{0x6d, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x2b, MSM_CAMERA_I2C_BURST_DATA},
	{0x6f, 0x03, MSM_CAMERA_I2C_BURST_DATA},

	{0x70, 0x2c, MSM_CAMERA_I2C_BURST_DATA},
	{0x71, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x2d, MSM_CAMERA_I2C_BURST_DATA},
	{0x73, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x2e, MSM_CAMERA_I2C_BURST_DATA},
	{0x75, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x2f, MSM_CAMERA_I2C_BURST_DATA},
	{0x77, 0x04, MSM_CAMERA_I2C_BURST_DATA},

	{0x78, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x79, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x31, MSM_CAMERA_I2C_BURST_DATA},
	{0x7b, 0x78, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x7d, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0x7f, 0x40, MSM_CAMERA_I2C_BURST_DATA},

	{0x80, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0x81, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x35, MSM_CAMERA_I2C_BURST_DATA},
	{0x83, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x36, MSM_CAMERA_I2C_BURST_DATA},
	{0x85, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x87, 0x07, MSM_CAMERA_I2C_BURST_DATA},

	{0x88, 0xa8, MSM_CAMERA_I2C_BURST_DATA},
	{0x89, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0xa9, MSM_CAMERA_I2C_BURST_DATA},
	{0x8b, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0xaa, MSM_CAMERA_I2C_BURST_DATA},
	{0x8d, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0xab, MSM_CAMERA_I2C_BURST_DATA},
	{0x8f, 0x02, MSM_CAMERA_I2C_BURST_DATA},

	{0x90, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
	{0x91, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0xc2, MSM_CAMERA_I2C_BURST_DATA},
	{0x93, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0xc3, MSM_CAMERA_I2C_BURST_DATA},
	{0x95, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0xc4, MSM_CAMERA_I2C_BURST_DATA},
	{0x97, 0x46, MSM_CAMERA_I2C_BURST_DATA},

	{0x98, 0xc5, MSM_CAMERA_I2C_BURST_DATA},
	{0x99, 0x78, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0xc6, MSM_CAMERA_I2C_BURST_DATA},
	{0x9b, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0xc7, MSM_CAMERA_I2C_BURST_DATA},
	{0x9d, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0xc8, MSM_CAMERA_I2C_BURST_DATA},
	{0x9f, 0x44, MSM_CAMERA_I2C_BURST_DATA},

	{0xa0, 0xc9, MSM_CAMERA_I2C_BURST_DATA},
	{0xa1, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0xca, MSM_CAMERA_I2C_BURST_DATA},
	{0xa3, 0xff, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0xcb, MSM_CAMERA_I2C_BURST_DATA},
	{0xa5, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0xcc, MSM_CAMERA_I2C_BURST_DATA},
	{0xa7, 0x61, MSM_CAMERA_I2C_BURST_DATA},

	{0xa8, 0xcd, MSM_CAMERA_I2C_BURST_DATA},
	{0xa9, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0xce, MSM_CAMERA_I2C_BURST_DATA},
	{0xab, 0x8a, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0xcf, MSM_CAMERA_I2C_BURST_DATA},
	{0xad, 0xa5, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x03, MSM_CAMERA_I2C_BURST_DATA},//14 page
	{0xaf, 0x14, MSM_CAMERA_I2C_BURST_DATA},

	{0xb0, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xb1, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0xb3, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0xb5, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x13, MSM_CAMERA_I2C_BURST_DATA},
	{0xb7, 0x88, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xb9, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x15, MSM_CAMERA_I2C_BURST_DATA},
	{0xbb, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0xbd, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x17, MSM_CAMERA_I2C_BURST_DATA},
	{0xbf, 0x2d, MSM_CAMERA_I2C_BURST_DATA},
	{0xc0, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0xc1, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0xc2, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xc3, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0xc5, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xc7, 0x82, MSM_CAMERA_I2C_BURST_DATA},
	{0xc8, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0xc9, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0xcb, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x23, MSM_CAMERA_I2C_BURST_DATA},
	{0xcd, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0xcf, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xd0, 0x25, MSM_CAMERA_I2C_BURST_DATA},
	{0xd1, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0xd2, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0xd3, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xd4, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0xd5, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xd6, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xd7, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xd8, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0xd9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xda, 0x2a, MSM_CAMERA_I2C_BURST_DATA},
	{0xdb, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0xdc, 0x2b, MSM_CAMERA_I2C_BURST_DATA},
	{0xdd, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0xde, 0x2c, MSM_CAMERA_I2C_BURST_DATA},
	{0xdf, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0xe0, 0x2d, MSM_CAMERA_I2C_BURST_DATA},
	{0xe1, 0x4c, MSM_CAMERA_I2C_BURST_DATA},
	{0xe2, 0x2e, MSM_CAMERA_I2C_BURST_DATA},
	{0xe3, 0x4e, MSM_CAMERA_I2C_BURST_DATA},
	{0xe4, 0x2f, MSM_CAMERA_I2C_BURST_DATA},
	{0xe5, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0xe6, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xe7, 0x82, MSM_CAMERA_I2C_BURST_DATA},
	{0xe8, 0x31, MSM_CAMERA_I2C_BURST_DATA},
	{0xe9, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xea, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xeb, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xec, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0xed, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xee, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0xef, 0x0a, MSM_CAMERA_I2C_BURST_DATA},

	{0xf0, 0x35, MSM_CAMERA_I2C_BURST_DATA},
	{0xf1, 0x46, MSM_CAMERA_I2C_BURST_DATA},
	{0xf2, 0x36, MSM_CAMERA_I2C_BURST_DATA},
	{0xf3, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xf4, 0x37, MSM_CAMERA_I2C_BURST_DATA},
	{0xf5, 0x2c, MSM_CAMERA_I2C_BURST_DATA},
	{0xf6, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0xf7, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0xf8, 0x39, MSM_CAMERA_I2C_BURST_DATA},
	{0xf9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xfa, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0xfb, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xfc, 0x3b, MSM_CAMERA_I2C_BURST_DATA},
	{0xfd, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	//{0xfe, 0x3c, MSM_CAMERA_I2C_BURST_DATA}, // STEVE deleted
	//{0xff, 0x28, MSM_CAMERA_I2C_BURST_DATA}, // STEVE deleted

	{0x03, 0xdc, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x3c, MSM_CAMERA_I2C_BURST_DATA},
	{0x11, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x3d, MSM_CAMERA_I2C_BURST_DATA},
	{0x13, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x3e, MSM_CAMERA_I2C_BURST_DATA},
	{0x15, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0x17, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x19, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x41, MSM_CAMERA_I2C_BURST_DATA},
	{0x1b, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x42, MSM_CAMERA_I2C_BURST_DATA},
	{0x1d, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x43, MSM_CAMERA_I2C_BURST_DATA},
	{0x1f, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x20, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0x21, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x45, MSM_CAMERA_I2C_BURST_DATA},
	{0x23, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x46, MSM_CAMERA_I2C_BURST_DATA},
	{0x25, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x47, MSM_CAMERA_I2C_BURST_DATA},
	{0x27, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x28, 0x48, MSM_CAMERA_I2C_BURST_DATA},
	{0x29, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x49, MSM_CAMERA_I2C_BURST_DATA},
	{0x2b, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x50, MSM_CAMERA_I2C_BURST_DATA},
  {0x2d, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x2e, 0x51, MSM_CAMERA_I2C_BURST_DATA},
	{0x2f, 0x34, MSM_CAMERA_I2C_BURST_DATA},
	{0x30, 0x52, MSM_CAMERA_I2C_BURST_DATA},
	{0x31, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x53, MSM_CAMERA_I2C_BURST_DATA},
	{0x33, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x54, MSM_CAMERA_I2C_BURST_DATA},
	{0x35, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0x37, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x38, 0x56, MSM_CAMERA_I2C_BURST_DATA},
	{0x39, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x57, MSM_CAMERA_I2C_BURST_DATA},
	{0x3b, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x3c, 0x58, MSM_CAMERA_I2C_BURST_DATA},
	{0x3d, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x3e, 0x59, MSM_CAMERA_I2C_BURST_DATA},
	{0x3f, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x40, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x41, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x42, 0x61, MSM_CAMERA_I2C_BURST_DATA},
	{0x43, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0x45, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x63, MSM_CAMERA_I2C_BURST_DATA},
	{0x47, 0xe4, MSM_CAMERA_I2C_BURST_DATA},

	{0x48, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0x49, 0xa4, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x65, MSM_CAMERA_I2C_BURST_DATA},
	{0x4b, 0x7d, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0x4d, 0x4b, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0x70, MSM_CAMERA_I2C_BURST_DATA},
	{0x4f, 0x10, MSM_CAMERA_I2C_BURST_DATA},

	{0x50, 0x71, MSM_CAMERA_I2C_BURST_DATA},
	{0x51, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x72, MSM_CAMERA_I2C_BURST_DATA},
	{0x53, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x73, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x74, MSM_CAMERA_I2C_BURST_DATA},
	{0x57, 0x10, MSM_CAMERA_I2C_BURST_DATA},

	{0x58, 0x75, MSM_CAMERA_I2C_BURST_DATA},
	{0x59, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x76, MSM_CAMERA_I2C_BURST_DATA},
	{0x5b, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x77, MSM_CAMERA_I2C_BURST_DATA},
	{0x5d, 0x58, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x78, MSM_CAMERA_I2C_BURST_DATA},
	{0x5f, 0x5a, MSM_CAMERA_I2C_BURST_DATA},

	{0x60, 0x79, MSM_CAMERA_I2C_BURST_DATA},
	{0x61, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x7a, MSM_CAMERA_I2C_BURST_DATA},
	{0x63, 0x5a, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x7b, MSM_CAMERA_I2C_BURST_DATA},
	{0x65, 0x50, MSM_CAMERA_I2C_BURST_DATA},

	// STEVE Saturation control
	// CB/CR vs sat
//LGE_CHANGE_S ADD INTO MR ver. for low light condition
	{0x66, 0x03, MSM_CAMERA_I2C_BURST_DATA}, //page 10
	{0x67, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x68, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1070
	{0x69, 0x08, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6a, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1071
	{0x6b, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6c, 0x72, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1072
	{0x6d, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6e, 0x73, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1073
	{0x6f, 0x6d, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x70, 0x74, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1074
	{0x71, 0x36, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x72, 0x75, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1075
	{0x73, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x74, 0x76, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1076
	{0x75, 0x2e, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x76, 0x77, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1077
	{0x77, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x78, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1078
	{0x79, 0xc2, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7a, 0x79, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x1079
	{0x7b, 0x3a, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7c, 0x7a, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x107a
	{0x7d, 0xe1, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7e, 0x7b, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x107b
	{0x7f, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x80, 0x7c, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x107c
	{0x81, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x82, 0x7d, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x107d
	{0x83, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x84, 0x7e, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x107e
	{0x85, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x86, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //outdoor 0x107f
	{0x87, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //

	// Lum Vs Sat
	{0x88, 0x03, MSM_CAMERA_I2C_BURST_DATA}, // page 16
	{0x89, 0x16, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8A, 0x8a, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x168a
	{0x8B, 0x6d, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8C, 0x8b, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x168b
	{0x8D, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8E, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x168c
	{0x8F, 0x7d, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x90, 0x8d, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x168d
	{0x91, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x92, 0x8e, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x168e
	{0x93, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x94, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x168f
	{0x95, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x96, 0x90, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x1690
	{0x97, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x98, 0x91, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x1691
	{0x99, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9A, 0x92, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x1692
	{0x9B, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9C, 0x93, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x1693
	{0x9D, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9E, 0x94, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x1694
	{0x9F, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA0, 0x95, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x1695
	{0xA1, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA2, 0x96, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x1696
	{0xA3, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA4, 0x97, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x1697
	{0xA5, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA6, 0x98, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x1698
	{0xA7, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA8, 0x99, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x1699
	{0xA9, 0x7c, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xAA, 0x9a, MSM_CAMERA_I2C_BURST_DATA}, // outdoor 0x169a
	{0xAB, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //
//LGE_CHANGE_E ADD INTO MR ver. for low light condition

	//////////////////
	// dd Page (DMA Indoor)
	//////////////////
	{0x03, 0xdd, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x03, MSM_CAMERA_I2C_BURST_DATA},//Indoor Page11
	{0x11, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x10, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1110
	{0x13, 0x13, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x11, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1111
	{0x15, 0x0c, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x12, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1112
	{0x17, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x13, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1113
	{0x19, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x14, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1114
	{0x1b, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x30, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1130
	{0x1d, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x31, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1131
	{0x1f, 0x20, MSM_CAMERA_I2C_BURST_DATA},

	{0x20, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1132 //STEVE Lum. Level. in DLPF
	{0x21, 0x8b, MSM_CAMERA_I2C_BURST_DATA}, //52
	{0x22, 0x33, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1133
	{0x23, 0x54, MSM_CAMERA_I2C_BURST_DATA}, //3b
	{0x24, 0x34, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1134
	{0x25, 0x2c, MSM_CAMERA_I2C_BURST_DATA}, //1d
	{0x26, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1135
	{0x27, 0x29, MSM_CAMERA_I2C_BURST_DATA}, //21
	{0x28, 0x36, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1136
	{0x29, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //1b
	{0x2a, 0x37, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1137
	{0x2b, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //21
	{0x2c, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1138
	{0x2d, 0x17, MSM_CAMERA_I2C_BURST_DATA}, //18

	{0x2e, 0x39, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1139 gain 1
	{0x2f, 0x34, MSM_CAMERA_I2C_BURST_DATA},    //r2 1
	{0x30, 0x3a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x113a
	{0x31, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x3b, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x113b
	{0x33, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x3c, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x113c
	{0x35, 0x38, MSM_CAMERA_I2C_BURST_DATA},   //18
	{0x36, 0x3d, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x113d
	{0x37, 0x2a, MSM_CAMERA_I2C_BURST_DATA},   //18
	{0x38, 0x3e, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x113e
	{0x39, 0x26, MSM_CAMERA_I2C_BURST_DATA},   //18
	{0x3a, 0x3f, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x113f
	{0x3b, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x3c, 0x40, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1140 gain 8
	{0x3d, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x3e, 0x41, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1141
	{0x3f, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0x40, 0x42, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1142
	{0x41, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0x42, 0x43, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1143
	{0x43, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x44, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1144
	{0x45, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x45, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1145
	{0x47, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x48, 0x46, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1146
	{0x49, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x47, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1147
	{0x4b, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0x48, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1148
	{0x4d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0x49, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1149
	{0x4f, 0xfc, MSM_CAMERA_I2C_BURST_DATA}, //high_clip_start
	{0x50, 0x4a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x114a
	{0x51, 0xfc, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x4b, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x114b
	{0x53, 0xfc, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x4c, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x114c
	{0x55, 0xfc, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x4d, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x114d
	{0x57, 0xfc, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x4e, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x114e
	{0x59, 0xf0, MSM_CAMERA_I2C_BURST_DATA},   //Lv 6 h_clip
	{0x5a, 0x4f, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x114f
	{0x5b, 0xf0, MSM_CAMERA_I2C_BURST_DATA},   //Lv 7 h_clip
	{0x5c, 0x50, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1150 clip 8
	{0x5d, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x51, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1151
	{0x5f, 0x08, MSM_CAMERA_I2C_BURST_DATA}, //color gain start
	{0x60, 0x52, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1152
	{0x61, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x53, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1153
	{0x63, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x54, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1154
	{0x65, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x55, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1155
	{0x67, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x56, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1156
	{0x69, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x57, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1157
	{0x6b, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x58, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1158
	{0x6d, 0x08, MSM_CAMERA_I2C_BURST_DATA}, //color gain end
	{0x6e, 0x59, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1159
	{0x6f, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //color ofs lmt start
	{0x70, 0x5a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x115a
	{0x71, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x5b, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x115b
	{0x73, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x5c, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x115c
	{0x75, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x5d, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x115d
	{0x77, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x5e, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x115e
	{0x79, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x5f, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x115f
	{0x7b, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x60, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1160
	{0x7d, 0x10, MSM_CAMERA_I2C_BURST_DATA},//color ofs lmt end
	{0x7e, 0x61, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1161
	{0x7f, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
	{0x80, 0x62, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1162
	{0x81, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x63, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1163
	{0x83, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x64, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1164
	{0x85, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0x65, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1165
	{0x87, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0x66, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1166
	{0x89, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0x67, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1167
	{0x8b, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x68, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1168
	{0x8d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x69, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1169
	{0x8f, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0x6a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x116a	 //Imp Lv2 High Gain
	{0x91, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0x6b, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x116b	 //Imp Lv2 Middle Gain
	{0x93, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0x6c, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x116c	 //Imp Lv2 Low Gain
	{0x95, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0x6d, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x116d
	{0x97, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0x6e, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x116e
	{0x99, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0x6f, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x116f	//Imp Lv3 Hi Gain
	{0x9b, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0x70, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1170	//Imp Lv3 Middle Gain
	{0x9d, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0x71, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1171	//Imp Lv3 Low Gain
	{0x9f, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0xa0, 0x72, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1172
	{0xa1, 0x6e, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0x73, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1173
	{0xa3, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0x74, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1174	//Imp Lv4 Hi Gain
	{0xa5, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0x75, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1175	//Imp Lv4 Middle Gain
	{0xa7, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0xa8, 0x76, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1176	//Imp Lv4 Low Gain
	{0xa9, 0x60, MSM_CAMERA_I2C_BURST_DATA},//18
	{0xaa, 0x77, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1177	//Imp Lv5 Hi Th
	{0xab, 0x6e, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0x78, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1178	//Imp Lv5 Middle Th
	{0xad, 0x66, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x79, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1179	//Imp Lv5 Hi Gain
	{0xaf, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0xb0, 0x7a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x117a	//Imp Lv5 Middle Gain
	{0xb1, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x7b, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x117b	//Imp Lv5 Low Gain
	{0xb3, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x7c, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x117c	//Imp Lv6 Hi Th
	{0xb5, 0x5c, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x7d, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x117d	//Imp Lv6 Middle Th
	{0xb7, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0x7e, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x117e	//Imp Lv6 Hi Gain
	{0xb9, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x7f, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x117f	//Imp Lv6 Middle Gain
	{0xbb, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1180	//Imp Lv6 Low Gain
	{0xbd, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x81, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1181
	{0xbf, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0xc0, 0x82, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1182
	{0xc1, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0xc2, 0x83, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1183	//Imp Lv7 Hi Gain
	{0xc3, 0x3e, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0x84, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1184	//Imp Lv7 Middle Gain
	{0xc5, 0x3e, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0x85, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1185	//Imp Lv7 Low Gain
	{0xc7, 0x3e, MSM_CAMERA_I2C_BURST_DATA},
	{0xc8, 0x86, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1186
	{0xc9, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x87, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1187
	{0xcb, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x88, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1188
	{0xcd, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x89, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1189
	{0xcf, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xd0, 0x8a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x118a
	{0xd1, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xd2, 0x90, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1190
	{0xd3, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xd4, 0x91, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1191
	{0xd5, 0x4e, MSM_CAMERA_I2C_BURST_DATA},
	{0xd6, 0x92, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1192
	{0xd7, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xd8, 0x93, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1193
	{0xd9, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0xda, 0x94, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1194
	{0xdb, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0xdc, 0x95, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1195
	{0xdd, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xde, 0x96, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1196
	{0xdf, 0x55, MSM_CAMERA_I2C_BURST_DATA},
	{0xe0, 0x97, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1197
	{0xe1, 0x8d, MSM_CAMERA_I2C_BURST_DATA},
	{0xe2, 0xb0, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x11b0
	{0xe3, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0xe4, 0xb1, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x11b1
	{0xe5, 0x99, MSM_CAMERA_I2C_BURST_DATA},
	{0xe6, 0xb2, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x11b2
	{0xe7, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xe8, 0xb3, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x11b3
	{0xe9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xea, 0xb4, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x11b4
	{0xeb, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xec, 0x03, MSM_CAMERA_I2C_BURST_DATA}, //12 page
	{0xed, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0xee, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1210
	{0xef, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xf0, 0x11, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1211
	{0xf1, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0xf2, 0x12, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1212
	{0xf3, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xf4, 0x40, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1240
	{0xf5, 0x33, MSM_CAMERA_I2C_BURST_DATA}, //07
	{0xf6, 0x41, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1241
	{0xf7, 0x0a, MSM_CAMERA_I2C_BURST_DATA}, //32
	{0xf8, 0x42, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1242
	{0xf9, 0x6a, MSM_CAMERA_I2C_BURST_DATA}, //8c
	{0xfa, 0x43, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1243
	{0xfb, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xfc, 0x44, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1244
	{0xfd, 0x02, MSM_CAMERA_I2C_BURST_DATA},

	{0x03, 0xde, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x45, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1245
	{0x11, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1246
	{0x13, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x60, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1260
	{0x15, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x61, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1261
	{0x17, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1262
	{0x19, 0x4b, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x63, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1263
	{0x1b, 0x41, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x64, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1264
	{0x1d, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x65, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1265
	{0x1f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x20, 0x68, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1268
	{0x21, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x69, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1269
	{0x23, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x6a, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x126a
	{0x25, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x6b, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x126b
	{0x27, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x28, 0x6c, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x126c
	{0x29, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x6d, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x126d
	{0x2b, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x1270
  {0x2d, 0x29, MSM_CAMERA_I2C_BURST_DATA},
  {0x2e, 0x71, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1271
  {0x2f, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
  {0x30, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1280
  {0x31, 0x30, MSM_CAMERA_I2C_BURST_DATA},//88
  {0x32, 0x81, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1281
  {0x33, 0xa0, MSM_CAMERA_I2C_BURST_DATA}, //05
  {0x34, 0x82, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1282
  {0x35, 0xa0, MSM_CAMERA_I2C_BURST_DATA},//13
  {0x36, 0x83, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1283
  {0x37, 0x00, MSM_CAMERA_I2C_BURST_DATA},//40
  {0x38, 0x84, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1284
  {0x39, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x3a, 0x85, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1285
  {0x3b, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x3c, 0x86, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1286
  {0x3d, 0xa0, MSM_CAMERA_I2C_BURST_DATA},//15
  {0x3e, 0x87, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1287
  {0x3f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
  {0x40, 0x88, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1288
  {0x41, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x42, 0x89, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1289
  {0x43, 0xc0, MSM_CAMERA_I2C_BURST_DATA},//c0
  {0x44, 0x8a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x128a
  {0x45, 0xb0, MSM_CAMERA_I2C_BURST_DATA},//18
  {0x46, 0x8b, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x128b
  {0x47, 0x08, MSM_CAMERA_I2C_BURST_DATA},//05
  {0x48, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x128c
  {0x49, 0x05, MSM_CAMERA_I2C_BURST_DATA},
  {0x4a, 0x8d, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x128d
  {0x4b, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0xe6, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x12e6
	{0x4d, 0xff, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0xe7, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x12e7
	{0x4f, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0xe8, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x12e8
	{0x51, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0xe9, MSM_CAMERA_I2C_BURST_DATA}, //Indoor 0x12e9
	{0x53, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x03, MSM_CAMERA_I2C_BURST_DATA},//Indoor Page13
	{0x55, 0x13, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x10, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1310
  {0x57, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x20, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1320
	{0x59, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x21, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1321
	{0x5b, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x22, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1322
	{0x5d, 0x36, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x23, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1323
	{0x5f, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x24, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1324
	{0x61, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x25, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1325
	{0x63, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x26, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1326
	{0x65, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x27, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1327
	{0x67, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x28, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1328
	{0x69, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x29, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1329
	{0x6b, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x2a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x132a
	{0x6d, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x2b, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x132b
	{0x6f, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x70, 0x2c, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x132c
	{0x71, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x2d, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x132d
	{0x73, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x2e, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x132e
	{0x75, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x132f
	{0x77, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x30, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1330
	{0x79, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x31, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1331
	{0x7b, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x32, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1332
	{0x7d, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x33, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1333
	{0x7f, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x80, 0x34, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1334
	{0x81, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x35, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1335
	{0x83, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x36, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1336
	{0x85, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0xa0, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13a0
	{0x87, 0x0f, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0xa8, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13a8
	{0x89, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0xa9, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13a9
	{0x8b, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0xaa, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13aa
	{0x8d, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0xab, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13ab
	{0x8f, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0xc0, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13c0
	{0x91, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0xc2, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13c2
	{0x93, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0xc3, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13c3
	{0x95, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0xc4, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13c4
	{0x97, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0xc5, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13c5
	{0x99, 0x38, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0xc6, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13c6
	{0x9b, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0xc7, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13c7
	{0x9d, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0xc8, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13c8
	{0x9f, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0xa0, 0xc9, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13c9
	{0xa1, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0xca, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13ca
	{0xa3, 0xff, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0xcb, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13cb
	{0xa5, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0xcc, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13cc
	{0xa7, 0x61, MSM_CAMERA_I2C_BURST_DATA},
	{0xa8, 0xcd, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13cd
	{0xa9, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0xce, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13ce
	{0xab, 0x8a, MSM_CAMERA_I2C_BURST_DATA},//07
	{0xac, 0xcf, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x13cf
	{0xad, 0xa5, MSM_CAMERA_I2C_BURST_DATA},//07
	{0xae, 0x03, MSM_CAMERA_I2C_BURST_DATA},//Indoor Page14
	{0xaf, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xb0, 0x10, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1410
	{0xb1, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x11, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1411
	{0xb3, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x12, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1412
	{0xb5, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x13, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1413
	{0xb7, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0x14, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1414
	{0xb9, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x15, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1415
	{0xbb, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x16, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1416
	{0xbd, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x17, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1417
	{0xbf, 0x1a, MSM_CAMERA_I2C_BURST_DATA},
	{0xc0, 0x18, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1418	Negative High Gain
	{0xc1, 0x60, MSM_CAMERA_I2C_BURST_DATA},//3a
	{0xc2, 0x19, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1419	Negative Middle Gain
	{0xc3, 0x68, MSM_CAMERA_I2C_BURST_DATA},//3a
	{0xc4, 0x1a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x141a	Negative Low Gain
	{0xc5, 0x68, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xc6, 0x20, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1420
	{0xc7, 0x82, MSM_CAMERA_I2C_BURST_DATA},  // s_diff L_clip
	{0xc8, 0x21, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1421
	{0xc9, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x22, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1422
	{0xcb, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x23, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1423
	{0xcd, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x24, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1424
	{0xcf, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xd0, 0x25, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1425
	{0xd1, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //19
	{0xd2, 0x26, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1426
	{0xd3, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xd4, 0x27, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1427
	{0xd5, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0xd6, 0x28, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1428
	{0xd7, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xd8, 0x29, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1429
	{0xd9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xda, 0x2a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x142a
	{0xdb, 0x18, MSM_CAMERA_I2C_BURST_DATA},//40
	{0xdc, 0x2b, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x142b
	{0xdd, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0xde, 0x2c, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x142c
	{0xdf, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0xe0, 0x2d, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x142d
	{0xe1, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xe2, 0x2e, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x142e
	{0xe3, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xe4, 0x2f, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x142f
	{0xe5, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xe6, 0x30, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1430
	{0xe7, 0x82, MSM_CAMERA_I2C_BURST_DATA},   //Ldiff_L_cip
	{0xe8, 0x31, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1431
	{0xe9, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xea, 0x32, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1432
	{0xeb, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xec, 0x33, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1433
	{0xed, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xee, 0x34, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1434
	{0xef, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xf0, 0x35, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1435
	{0xf1, 0x46, MSM_CAMERA_I2C_BURST_DATA},//12
	{0xf2, 0x36, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1436
	{0xf3, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xf4, 0x37, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1437
	{0xf5, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xf6, 0x38, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1438
	{0xf7, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0xf8, 0x39, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1439
	{0xf9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xfa, 0x3a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x143a
	{0xfb, 0x48, MSM_CAMERA_I2C_BURST_DATA},
	{0xfc, 0x3b, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x143b
	{0xfd, 0x30, MSM_CAMERA_I2C_BURST_DATA},

	{0x03, 0xdf, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x3c, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x143c
	{0x11, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x3d, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x143d
	{0x13, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x3e, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x143e
	{0x15, 0x22, MSM_CAMERA_I2C_BURST_DATA},//12
	{0x16, 0x3f, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x143f
	{0x17, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x40, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1440
	{0x19, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x41, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1441
	{0x1b, 0x10, MSM_CAMERA_I2C_BURST_DATA},//20
	{0x1c, 0x42, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1442
	{0x1d, 0xb0, MSM_CAMERA_I2C_BURST_DATA},//20
	{0x1e, 0x43, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1443
	{0x1f, 0x40, MSM_CAMERA_I2C_BURST_DATA},//20
	{0x20, 0x44, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1444
	{0x21, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x45, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1445
	{0x23, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x46, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1446
	{0x25, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x47, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1447
	{0x27, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x28, 0x48, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1448
	{0x29, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x49, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1449
	{0x2b, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x50, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1450
	{0x2d, 0x84, MSM_CAMERA_I2C_BURST_DATA},//19
	{0x2e, 0x51, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1451
	{0x2f, 0x30, MSM_CAMERA_I2C_BURST_DATA},//60
	{0x30, 0x52, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1452
	{0x31, 0xb0, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x53, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1453
	{0x33, 0x37, MSM_CAMERA_I2C_BURST_DATA},//58
	{0x34, 0x54, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1454
	{0x35, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x55, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1455
	{0x37, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0x38, 0x56, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1456
	{0x39, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x57, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1457
	{0x3b, 0x10, MSM_CAMERA_I2C_BURST_DATA},//03
	{0x3c, 0x58, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1458
	{0x3d, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x3e, 0x59, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1459
	{0x3f, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x40, 0x60, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1460
  {0x41, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x42, 0x61, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1461
	{0x43, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x62, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1462
	{0x45, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x63, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1463
	{0x47, 0xe4, MSM_CAMERA_I2C_BURST_DATA},
	{0x48, 0x64, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1464
	{0x49, 0xa4, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x65, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1465
	{0x4b, 0x7d, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0x66, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1466
	{0x4d, 0x4b, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0x70, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1470
	{0x4f, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0x71, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1471
	{0x51, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x72, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1472
	{0x53, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x73, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1473
	{0x55, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x74, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1474
	{0x57, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x75, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1475
	{0x59, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x76, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1476	  //green sharp pos High
	{0x5b, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x77, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1477	  //green sharp pos Middle
	{0x5d, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x78, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1478	  //green sharp pos Low
	{0x5f, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x79, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x1479	   //green sharp nega High
	{0x61, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x7a, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x147a	   //green sharp nega Middle
	{0x63, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x7b, MSM_CAMERA_I2C_BURST_DATA},//Indoor 0x147b	   //green sharp nega Low
	{0x65, 0x60, MSM_CAMERA_I2C_BURST_DATA},

	// STEVE Saturation control
	// CB/CR vs sat
//LGE_CHANGE_S ADD INTO MR ver. for low light condition
	{0x66, 0x03, MSM_CAMERA_I2C_BURST_DATA}, //page 10
	{0x67, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x68, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x1070
	{0x69, 0x08, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6a, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x1071
	{0x6b, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6c, 0x72, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x1072
	{0x6d, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6e, 0x73, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x1073
	{0x6f, 0x6d, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x70, 0x74, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x1074
	{0x71, 0x36, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x72, 0x75, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x1075
	{0x73, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x74, 0x76, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x1076
	{0x75, 0x2e, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x76, 0x77, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x1077
	{0x77, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x78, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x1078
	{0x79, 0xc2, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7a, 0x79, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x1079
	{0x7b, 0x3a, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7c, 0x7a, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x107a
	{0x7d, 0xe1, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7e, 0x7b, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x107b
	{0x7f, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x80, 0x7c, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x107c
	{0x81, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x82, 0x7d, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x107d
	{0x83, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x84, 0x7e, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x107e
	{0x85, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x86, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //indoor 0x107f
	{0x87, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //

	// Lum Vs Sat
	{0x88, 0x03, MSM_CAMERA_I2C_BURST_DATA}, // page 16
	{0x89, 0x16, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8A, 0x8a, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x168a
	{0x8B, 0x6d, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8C, 0x8b, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x168b
	{0x8D, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8E, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x168c
	{0x8F, 0x7d, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x90, 0x8d, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x168d
	{0x91, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x92, 0x8e, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x168e
	{0x93, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x94, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x168f
	{0x95, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x96, 0x90, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x1690
	{0x97, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x98, 0x91, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x1691
	{0x99, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9A, 0x92, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x1692
	{0x9B, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9C, 0x93, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x1693
	{0x9D, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9E, 0x94, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x1694
	{0x9F, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA0, 0x95, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x1695
	{0xA1, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA2, 0x96, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x1696
	{0xA3, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA4, 0x97, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x1697
	{0xA5, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA6, 0x98, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x1698
	{0xA7, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA8, 0x99, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x1699
	{0xA9, 0x7c, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xAA, 0x9a, MSM_CAMERA_I2C_BURST_DATA}, // indoor 0x169a
	{0xAB, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //
//LGE_CHANGE_E ADD INTO MR ver. for low light condition

	//////////////////
	// e0 Page (DMA Dark1)
	//////////////////

	//Page 0xe0
	{0x03, 0xe0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x11, 0x11, MSM_CAMERA_I2C_BURST_DATA}, //11 page
	{0x12, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1110
	{0x13, 0x1f, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x11, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1111
	{0x15, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x12, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1112
	{0x17, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1113
	{0x19, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1114
	{0x1b, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x30, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1130
	{0x1d, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //20
	{0x1e, 0x31, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1131
	{0x1f, 0x24, MSM_CAMERA_I2C_BURST_DATA}, //20

	{0x20, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1132 //STEVE Lum. Level. in DLPF
	{0x21, 0x8b, MSM_CAMERA_I2C_BURST_DATA}, //52
	{0x22, 0x33, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1133
	{0x23, 0x54, MSM_CAMERA_I2C_BURST_DATA}, //3b
	{0x24, 0x34, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1134
	{0x25, 0x2c, MSM_CAMERA_I2C_BURST_DATA}, //1d
	{0x26, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1135
	{0x27, 0x29, MSM_CAMERA_I2C_BURST_DATA}, //21
	{0x28, 0x36, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1136
	{0x29, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //1b
	{0x2a, 0x37, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1137
	{0x2b, 0x1e, MSM_CAMERA_I2C_BURST_DATA}, //21
	{0x2c, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1138
	{0x2d, 0x17, MSM_CAMERA_I2C_BURST_DATA}, //18

	{0x2e, 0x39, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1139
	{0x2f, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x30, 0x3a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x113a
	{0x31, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x3b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x113b
	{0x33, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x3c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x113c
	{0x35, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x3d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x113d
	{0x37, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x38, 0x3e, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x113e
	{0x39, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x3f, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x113f
	{0x3b, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x3c, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1140
	{0x3d, 0x84, MSM_CAMERA_I2C_BURST_DATA},
	{0x3e, 0x41, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1141
	{0x3f, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x40, 0x42, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1142
	{0x41, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x42, 0x43, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1143
	{0x43, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x44, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1144
	{0x45, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x45, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1145
	{0x47, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x48, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1146
	{0x49, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x47, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1147
	{0x4b, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0x48, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1148
	{0x4d, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0x49, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1149
	{0x4f, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0x4a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x114a
	{0x51, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x4b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x114b
	{0x53, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x4c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x114c
	{0x55, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x4d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x114d
	{0x57, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x4e, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x114e
	{0x59, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x4f, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x114f
	{0x5b, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x50, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1150
	{0x5d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x51, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1151
	{0x5f, 0xd8, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x52, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1152
	{0x61, 0xd8, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x53, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1153
	{0x63, 0xd8, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x54, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1154
	{0x65, 0xd0, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x55, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1155
	{0x67, 0xd0, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x56, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1156
	{0x69, 0xc8, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x57, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1157
	{0x6b, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x58, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1158
	{0x6d, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x59, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1159
	{0x6f, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x70, 0x5a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x115a
	{0x71, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x5b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x115b
	{0x73, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x5c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x115c
	{0x75, 0xe8, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x5d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x115d
	{0x77, 0xe8, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x5e, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x115e
	{0x79, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x5f, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x115f
	{0x7b, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x60, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1160
	{0x7d, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x61, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1161
	{0x7f, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x80, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1162
	{0x81, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x63, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1163
	{0x83, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x64, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1164
	{0x85, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0x65, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1165
	{0x87, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0x66, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1166
	{0x89, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0x67, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1167
	{0x8b, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x68, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1168
	{0x8d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x69, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1169
	{0x8f, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0x6a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x116a
	{0x91, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0x6b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x116b
	{0x93, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0x6c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x116c
	{0x95, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0x6d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x116d
	{0x97, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0x6e, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x116e
	{0x99, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0x6f, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x116f
	{0x9b, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1170
	{0x9d, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1171
	{0x9f, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xa0, 0x72, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1172
	{0xa1, 0x6e, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0x73, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1173
	{0xa3, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0x74, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1174
	{0xa5, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0x75, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1175
	{0xa7, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xa8, 0x76, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1176
	{0xa9, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0x77, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1177
	{0xab, 0x6e, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1178
	{0xad, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x79, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1179
	{0xaf, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xb0, 0x7a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x117a
	{0xb1, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x7b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x117b
	{0xb3, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x7c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x117c
	{0xb5, 0x5c, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x7d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x117d
	{0xb7, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0x7e, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x117e
	{0xb9, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x117f
	{0xbb, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1180
	{0xbd, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x81, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1181
	{0xbf, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0xc0, 0x82, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1182
	{0xc1, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0xc2, 0x83, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1183
	{0xc3, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1184
	{0xc5, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0x85, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1185
	{0xc7, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xc8, 0x86, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1186
	{0xc9, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1187
	{0xcb, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1188
	{0xcd, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x89, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1189
	{0xcf, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xd0, 0x8a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x118a
	{0xd1, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xd2, 0x90, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1190
	{0xd3, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xd4, 0x91, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1191
	{0xd5, 0xff, MSM_CAMERA_I2C_BURST_DATA},
	{0xd6, 0x92, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1192
	{0xd7, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xd8, 0x93, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1193
	{0xd9, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xda, 0x94, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1194
	{0xdb, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xdc, 0x95, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1195
	{0xdd, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0xde, 0x96, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1196
	{0xdf, 0x90, MSM_CAMERA_I2C_BURST_DATA},
	{0xe0, 0x97, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1197
	{0xe1, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0xe2, 0xb0, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x11b0
	{0xe3, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0xe4, 0xb1, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x11b1
	{0xe5, 0xd8, MSM_CAMERA_I2C_BURST_DATA},
	{0xe6, 0xb2, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x11b2
	{0xe7, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0xe8, 0xb3, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x11b3
	{0xe9, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xea, 0xb4, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x11b4
	{0xeb, 0x03, MSM_CAMERA_I2C_BURST_DATA},

	{0xec, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xed, 0x12, MSM_CAMERA_I2C_BURST_DATA},//12 page
	{0xee, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1210
	{0xef, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xf0, 0x11, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1211
	{0xf1, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0xf2, 0x12, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1212
	{0xf3, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xf4, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1240
	{0xf5, 0x33, MSM_CAMERA_I2C_BURST_DATA}, //07
	{0xf6, 0x41, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1241
	{0xf7, 0x0a, MSM_CAMERA_I2C_BURST_DATA}, //32
	{0xf8, 0x42, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1242
	{0xf9, 0x6a, MSM_CAMERA_I2C_BURST_DATA}, //8c
	{0xfa, 0x43, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1243
	{0xfb, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xfc, 0x44, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1244
	{0xfd, 0x02, MSM_CAMERA_I2C_BURST_DATA},

	{0x03, 0xe1, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x45, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1245
	{0x11, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1246
	{0x13, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x60, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1260
	{0x15, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x61, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1261
	{0x17, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1262
	{0x19, 0x4b, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x63, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1263
	{0x1b, 0x41, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x64, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1264
	{0x1d, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x65, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1265
	{0x1f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x20, 0x68, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1268
	{0x21, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x69, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1269
	{0x23, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x6a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x126a
	{0x25, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x6b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x126b
	{0x27, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x28, 0x6c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x126c
	{0x29, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x6d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x126d
	{0x2b, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1270
  {0x2d, 0x29, MSM_CAMERA_I2C_BURST_DATA},
  {0x2e, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1271
  {0x2f, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
  {0x30, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1280
  {0x31, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x32, 0x81, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1281
  {0x33, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x34, 0x82, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1282
  {0x35, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x36, 0x83, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1283
  {0x37, 0x00, MSM_CAMERA_I2C_BURST_DATA},
  {0x38, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1284
  {0x39, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x3a, 0x85, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1285
  {0x3b, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x3c, 0x86, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1286
  {0x3d, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x3e, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1287
  {0x3f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
  {0x40, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1288
  {0x41, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x42, 0x89, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1289
  {0x43, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
  {0x44, 0x8a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x128a
  {0x45, 0xb0, MSM_CAMERA_I2C_BURST_DATA},
  {0x46, 0x8b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x128b
  {0x47, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x48, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x128c
	{0x49, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x8d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x128d
	{0x4b, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0xe6, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x12e6
	{0x4d, 0xff, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0xe7, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x12e7
	{0x4f, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0xe8, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x12e8
	{0x51, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0xe9, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x12e9
	{0x53, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0x13, MSM_CAMERA_I2C_BURST_DATA},//13 page
	{0x56, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1310
	{0x57, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1320
	{0x59, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x21, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1321
	{0x5b, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x22, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1322
	{0x5d, 0x36, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x23, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1323
	{0x5f, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x24, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1324
	{0x61, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x25, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1325
	{0x63, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x26, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1326
	{0x65, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x27, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1327
	{0x67, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x28, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1328
	{0x69, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x29, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1329
	{0x6b, 0x06, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x2a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x132a
	{0x6d, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x2b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x132b
	{0x6f, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x70, 0x2c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x132c
	{0x71, 0x0c, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x2d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x132d
	{0x73, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x2e, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x132e
	{0x75, 0x16, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x2f, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x132f	   //weight skin
	{0x77, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x30, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1330	   //weight blue
	{0x79, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x31, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1331	   //weight green
	{0x7b, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1332	   //weight strong color
	{0x7d, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x33, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1333
	{0x7f, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x80, 0x34, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1334
	{0x81, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1335
	{0x83, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x36, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1336
	{0x85, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0xa0, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13a0
	{0x87, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0xa8, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13a8	   //Dark1 Cb-filter 0x20
	{0x89, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0xa9, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13a9	   //Dark1 Cr-filter 0x20
	{0x8b, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0xaa, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13aa
	{0x8d, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0xab, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13ab
	{0x8f, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0xc0, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13c0
	{0x91, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0xc2, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13c2
	{0x93, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0xc3, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13c3
	{0x95, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0xc4, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13c4
	{0x97, 0x46, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0xc5, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13c5
	{0x99, 0x78, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0xc6, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13c6
	{0x9b, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0xc7, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13c7
	{0x9d, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0xc8, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13c8
	{0x9f, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0xa0, 0xc9, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13c9
	{0xa1, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0xca, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13ca
	{0xa3, 0xff, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0xcb, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13cb
	{0xa5, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0xcc, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13cc	   //skin range_cb_l
	{0xa7, 0x61, MSM_CAMERA_I2C_BURST_DATA},
	{0xa8, 0xcd, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13cd	   //skin range_cb_h
	{0xa9, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0xce, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13ce	   //skin range_cr_l
	{0xab, 0x8a, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0xcf, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x13cf	   //skin range_cr_h
	{0xad, 0xa5, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x03, MSM_CAMERA_I2C_BURST_DATA}, //14 page
	{0xaf, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xb0, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1410
  {0xb1, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x11, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1411
	{0xb3, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x12, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1412
	{0xb5, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //Top H_Clip
	{0xb6, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1413
	{0xb7, 0xc8, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1414
	{0xb9, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x15, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1415	   //sharp positive hi
	{0xbb, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x16, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1416	   //sharp positive mi
	{0xbd, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x17, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1417	   //sharp positive low
	{0xbf, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xc0, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1418	   //sharp negative hi
	{0xc1, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0xc2, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1419	   //sharp negative mi
	{0xc3, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0x1a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x141a	   //sharp negative low
	{0xc5, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1420
	{0xc7, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xc8, 0x21, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1421
	{0xc9, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x22, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1422
	{0xcb, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x23, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1423
	{0xcd, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x24, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1424
	{0xcf, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xd0, 0x25, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1425
	{0xd1, 0x46, MSM_CAMERA_I2C_BURST_DATA},
	{0xd2, 0x26, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1426
	{0xd3, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xd4, 0x27, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1427
	{0xd5, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0xd6, 0x28, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1428
	{0xd7, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xd8, 0x29, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1429
	{0xd9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xda, 0x2a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x142a
	{0xdb, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xdc, 0x2b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x142b
	{0xdd, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xde, 0x2c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x142c
	{0xdf, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xe0, 0x2d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x142d
	{0xe1, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xe2, 0x2e, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x142e
	{0xe3, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xe4, 0x2f, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x142f
	{0xe5, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xe6, 0x30, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1430
	{0xe7, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xe8, 0x31, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1431
	{0xe9, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xea, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1432
	{0xeb, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xec, 0x33, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1433
	{0xed, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xee, 0x34, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1434
	{0xef, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xf0, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1435
	{0xf1, 0x46, MSM_CAMERA_I2C_BURST_DATA},
	{0xf2, 0x36, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1436
	{0xf3, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xf4, 0x37, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1437
	{0xf5, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xf6, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1438
	{0xf7, 0x12, MSM_CAMERA_I2C_BURST_DATA},//2d
	{0xf8, 0x39, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1439
	{0xf9, 0x00, MSM_CAMERA_I2C_BURST_DATA},//23
	{0xfa, 0x3a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x143a
	{0xfb, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //dr gain
	{0xfc, 0x3b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x143b
	{0xfd, 0x20, MSM_CAMERA_I2C_BURST_DATA},

	{0x03, 0xe2, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x3c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x143c
	{0x11, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x3d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x143d
	{0x13, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //nor gain
	{0x14, 0x3e, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x143e
	{0x15, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x3f, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x143f
	{0x17, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1440
	{0x19, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x41, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1441
	{0x1b, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x42, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1442
	{0x1d, 0xb0, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x43, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1443
	{0x1f, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x20, 0x44, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1444
	{0x21, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x45, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1445
	{0x23, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1446
	{0x25, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x47, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1447
	{0x27, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x28, 0x48, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1448
	{0x29, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x49, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1449
	{0x2b, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x50, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1450
	{0x2d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x2e, 0x51, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1451
	{0x2f, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x30, 0x52, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1452
	{0x31, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x53, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1453
	{0x33, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x54, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1454
	{0x35, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x55, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1455
	{0x37, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x38, 0x56, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1456
	{0x39, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x57, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1457
	{0x3b, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x3c, 0x58, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1458
	{0x3d, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x3e, 0x59, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1459
	{0x3f, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x40, 0x60, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1460
	{0x41, 0x03, MSM_CAMERA_I2C_BURST_DATA}, //skin opt en
	{0x42, 0x61, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1461
	{0x43, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1462
	{0x45, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x63, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1463
	{0x47, 0xe4, MSM_CAMERA_I2C_BURST_DATA}, //skin_std_th_h
	{0x48, 0x64, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1464
	{0x49, 0xa4, MSM_CAMERA_I2C_BURST_DATA}, //skin_std_th_l
	{0x4a, 0x65, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1465
	{0x4b, 0x7d, MSM_CAMERA_I2C_BURST_DATA}, //sharp_std_th_h
	{0x4c, 0x66, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1466
	{0x4d, 0x4b, MSM_CAMERA_I2C_BURST_DATA}, //sharp_std_th_l
	{0x4e, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1470
	{0x4f, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1471
	{0x51, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x72, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1472
	{0x53, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x73, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1473
	{0x55, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x74, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1474
	{0x57, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x75, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1475
	{0x59, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x76, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1476
	{0x5b, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x77, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1477
	{0x5d, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1478
	{0x5f, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x79, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1479
	{0x61, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x7a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x147a
	{0x63, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x7b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x147b
	{0x65, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	// STEVE Saturation control
	// CB/CR vs sat
//LGE_CHANGE_S ADD INTO MR ver. for low light condition

	{0x66, 0x03, MSM_CAMERA_I2C_BURST_DATA}, //page 10
	{0x67, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x68, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1070
	{0x69, 0x08, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6a, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1071
	{0x6b, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6c, 0x72, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1072
	{0x6d, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6e, 0x73, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1073
	{0x6f, 0x6d, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x70, 0x74, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1074
	{0x71, 0x36, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x72, 0x75, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1075
	{0x73, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x74, 0x76, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1076
	{0x75, 0x2e, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x76, 0x77, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1077
	{0x77, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x78, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1078
	{0x79, 0xc2, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7a, 0x79, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x1079
	{0x7b, 0x3a, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7c, 0x7a, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x107a
	{0x7d, 0xe1, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7e, 0x7b, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x107b
	{0x7f, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x80, 0x7c, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x107c
	{0x81, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x82, 0x7d, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x107d
	{0x83, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x84, 0x7e, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x107e
	{0x85, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x86, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //Dark1 0x107f
	{0x87, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //

	// Lum Vs Sat
	{0x88, 0x03, MSM_CAMERA_I2C_BURST_DATA}, // page 16
	{0x89, 0x16, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8A, 0x8a, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x168a
	{0x8B, 0x6d, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8C, 0x8b, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x168b
	{0x8D, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8E, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x168c
	{0x8F, 0x7d, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x90, 0x8d, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x168d
	{0x91, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x92, 0x8e, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x168e
	{0x93, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x94, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x168f
	{0x95, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x96, 0x90, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x1690
	{0x97, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x98, 0x91, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x1691
	{0x99, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9A, 0x92, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x1692
	{0x9B, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9C, 0x93, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x1693
	{0x9D, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9E, 0x94, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x1694
	{0x9F, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA0, 0x95, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x1695
	{0xA1, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA2, 0x96, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x1696
	{0xA3, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA4, 0x97, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x1697
	{0xA5, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA6, 0x98, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x1698
	{0xA7, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA8, 0x99, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x1699
	{0xA9, 0x7c, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xAA, 0x9a, MSM_CAMERA_I2C_BURST_DATA}, // Dark1 0x169a
	{0xAB, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //
//LGE_CHANGE_E ADD INTO MR ver. for low light condition


	//////////////////
	// e3 Page (DMA Dark2)
	//////////////////

	{0x03, 0xe3, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x03, MSM_CAMERA_I2C_BURST_DATA},//Dark2 Page11
	{0x11, 0x11, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x10, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1110
	{0x13, 0x1f, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x11, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1111
	{0x15, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x12, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1112
	{0x17, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x13, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1113
	{0x19, 0x21, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x14, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1114
	{0x1b, 0x39, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x30, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1130
	{0x1d, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x31, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1131
	{0x1f, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x20, 0x32, MSM_CAMERA_I2C_BURST_DATA},  //Dark2 0x1132 //STEVE Lum. Level. in DLPF
	{0x21, 0x8b, MSM_CAMERA_I2C_BURST_DATA},  //52                    82
	{0x22, 0x33, MSM_CAMERA_I2C_BURST_DATA},  //Dark2 0x1133
	{0x23, 0x54, MSM_CAMERA_I2C_BURST_DATA},  //3b                    5d
	{0x24, 0x34, MSM_CAMERA_I2C_BURST_DATA},  //Dark2 0x1134
	{0x25, 0x2c, MSM_CAMERA_I2C_BURST_DATA},  //1d                    37
	{0x26, 0x35, MSM_CAMERA_I2C_BURST_DATA},  //Dark2 0x1135
	{0x27, 0x29, MSM_CAMERA_I2C_BURST_DATA},  //21                    30
	{0x28, 0x36, MSM_CAMERA_I2C_BURST_DATA},  //Dark2 0x1136
	{0x29, 0x18, MSM_CAMERA_I2C_BURST_DATA},  //1b                    18
	{0x2a, 0x37, MSM_CAMERA_I2C_BURST_DATA},  //Dark2 0x1137
	{0x2b, 0x1e, MSM_CAMERA_I2C_BURST_DATA},  //21                    24
	{0x2c, 0x38, MSM_CAMERA_I2C_BURST_DATA},  //Dark2 0x1138
	{0x2d, 0x17, MSM_CAMERA_I2C_BURST_DATA},  //18                    18
	{0x2e, 0x39, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1139 gain 1

	{0x2f, 0x80, MSM_CAMERA_I2C_BURST_DATA},    //r2 1
	{0x30, 0x3a, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x113a
	{0x31, 0x82, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x3b, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x113b
	{0x33, 0x82, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x3c, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x113c
	{0x35, 0x82, MSM_CAMERA_I2C_BURST_DATA},   //18
	{0x36, 0x3d, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x113d
	{0x37, 0x82, MSM_CAMERA_I2C_BURST_DATA},   //18
	{0x38, 0x3e, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x113e
	{0x39, 0x82, MSM_CAMERA_I2C_BURST_DATA},   //18
	{0x3a, 0x3f, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x113f
	{0x3b, 0x82, MSM_CAMERA_I2C_BURST_DATA},
	{0x3c, 0x40, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1140 gain 8
	{0x3d, 0x82, MSM_CAMERA_I2C_BURST_DATA},
	{0x3e, 0x41, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1141
	{0x3f, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x40, 0x42, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1142
	{0x41, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x42, 0x43, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1143
	{0x43, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x44, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1144
	{0x45, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x45, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1145
	{0x47, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x48, 0x46, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1146
	{0x49, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x47, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1147
	{0x4b, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0x48, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1148
	{0x4d, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0x49, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1149
	{0x4f, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //high_clip_start
	{0x50, 0x4a, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x114a
	{0x51, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x4b, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x114b
	{0x53, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x4c, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x114c
	{0x55, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x4d, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x114d
	{0x57, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x4e, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x114e
	{0x59, 0x10, MSM_CAMERA_I2C_BURST_DATA},   //Lv 6 h_clip
	{0x5a, 0x4f, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x114f
	{0x5b, 0x10, MSM_CAMERA_I2C_BURST_DATA},   //Lv 7 h_clip
	{0x5c, 0x50, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1150 clip 8
	{0x5d, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x51, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1151
	{0x5f, 0xd8, MSM_CAMERA_I2C_BURST_DATA}, //color gain start
	{0x60, 0x52, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1152
	{0x61, 0xd8, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x53, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1153
	{0x63, 0xd8, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x54, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1154
	{0x65, 0xd0, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x55, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1155
	{0x67, 0xd0, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x56, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1156
	{0x69, 0xc8, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x57, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1157
	{0x6b, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x58, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1158
	{0x6d, 0xc0, MSM_CAMERA_I2C_BURST_DATA}, //color gain end
	{0x6e, 0x59, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1159
	{0x6f, 0xf0, MSM_CAMERA_I2C_BURST_DATA}, //color ofs lmt start
	{0x70, 0x5a, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x115a
	{0x71, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x5b, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x115b
	{0x73, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x5c, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x115c
	{0x75, 0xe8, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x5d, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x115d
	{0x77, 0xe8, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x5e, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x115e
	{0x79, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x5f, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x115f
	{0x7b, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x60, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1160
	{0x7d, 0xe0, MSM_CAMERA_I2C_BURST_DATA},//color ofs lmt end
	{0x7e, 0x61, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1161
	{0x7f, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x80, 0x62, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1162
	{0x81, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x63, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1163
	{0x83, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x64, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1164
	{0x85, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0x65, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1165
	{0x87, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0x66, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1166
	{0x89, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0x67, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1167
	{0x8b, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0x68, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1168
	{0x8d, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0x69, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1169
	{0x8f, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0x6a, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x116a
	{0x91, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0x6b, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x116b
	{0x93, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0x6c, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x116c
	{0x95, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0x6d, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x116d
	{0x97, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0x6e, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x116e
	{0x99, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0x6f, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x116f
	{0x9b, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0x70, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1170
	{0x9d, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0x71, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1171
	{0x9f, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xa0, 0x72, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1172
	{0xa1, 0x6e, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0x73, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1173
	{0xa3, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0x74, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1174
	{0xa5, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0x75, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1175
	{0xa7, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xa8, 0x76, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1176
	{0xa9, 0x02, MSM_CAMERA_I2C_BURST_DATA},//18
	{0xaa, 0x77, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1177
	{0xab, 0x6e, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0x78, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1178
	{0xad, 0x3a, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x79, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1179
	{0xaf, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xb0, 0x7a, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x117a
	{0xb1, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x7b, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x117b
	{0xb3, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x7c, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x117c
	{0xb5, 0x5c, MSM_CAMERA_I2C_BURST_DATA},
	{0xb6, 0x7d, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x117d
	{0xb7, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0x7e, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x117e
	{0xb9, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x7f, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x117f
	{0xbb, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x80, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1180
	{0xbd, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x81, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1181
	{0xbf, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0xc0, 0x82, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1182
	{0xc1, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0xc2, 0x83, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1183
	{0xc3, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0x84, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1184
	{0xc5, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0x85, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1185
	{0xc7, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xc8, 0x86, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1186
	{0xc9, 0x62, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x87, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1187
	{0xcb, 0x26, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x88, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1188
	{0xcd, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x89, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1189
	{0xcf, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xd0, 0x8a, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x118a
	{0xd1, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xd2, 0x90, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1190
	{0xd3, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xd4, 0x91, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1191
	{0xd5, 0xff, MSM_CAMERA_I2C_BURST_DATA},
	{0xd6, 0x92, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1192
	{0xd7, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xd8, 0x93, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1193
	{0xd9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xda, 0x94, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1194
	{0xdb, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xdc, 0x95, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1195
	{0xdd, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0xde, 0x96, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1196
	{0xdf, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xe0, 0x97, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x1197
	{0xe1, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xe2, 0xb0, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x11b0
	{0xe3, 0x64, MSM_CAMERA_I2C_BURST_DATA},
	{0xe4, 0xb1, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x11b1
	{0xe5, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xe6, 0xb2, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x11b2
	{0xe7, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xe8, 0xb3, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x11b3
	{0xe9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xea, 0xb4, MSM_CAMERA_I2C_BURST_DATA},//Dark2 0x11b4
	{0xeb, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xec, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xed, 0x12, MSM_CAMERA_I2C_BURST_DATA},//12 page
	{0xee, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1210
	{0xef, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xf0, 0x11, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1211
	{0xf1, 0x29, MSM_CAMERA_I2C_BURST_DATA},
	{0xf2, 0x12, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1212
	{0xf3, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0xf4, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1240
	{0xf5, 0x33, MSM_CAMERA_I2C_BURST_DATA}, //07
	{0xf6, 0x41, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1241
	{0xf7, 0x0a, MSM_CAMERA_I2C_BURST_DATA}, //32
	{0xf8, 0x42, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1242
	{0xf9, 0x6a, MSM_CAMERA_I2C_BURST_DATA}, //8c
	{0xfa, 0x43, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1243
	{0xfb, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xfc, 0x44, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1244
	{0xfd, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	//{0xfe, 0x45, MSM_CAMERA_I2C_BURST_DATA}, // STEVE deleted
	//{0xff, 0x0a, MSM_CAMERA_I2C_BURST_DATA}, // STEVE deleted

	{0x03, 0xe4, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x45, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1245
	{0x11, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1246
	{0x13, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x14, 0x60, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1260
	{0x15, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x61, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1261
	{0x17, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1262
	{0x19, 0x4b, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x63, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1263
	{0x1b, 0x41, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x64, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1264
	{0x1d, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x65, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1265
	{0x1f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x20, 0x68, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1268
	{0x21, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x69, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1269
	{0x23, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x6a, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x126a
	{0x25, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x6b, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x126b
	{0x27, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x28, 0x6c, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x126c
	{0x29, 0x24, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x6d, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x126d
	{0x2b, 0x01, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1270
  {0x2d, 0x29, MSM_CAMERA_I2C_BURST_DATA},
  {0x2e, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1271
  {0x2f, 0x7f, MSM_CAMERA_I2C_BURST_DATA},
  {0x30, 0x80, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1280
  {0x31, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x32, 0x81, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1281
  {0x33, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x34, 0x82, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1282
  {0x35, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x36, 0x83, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1283
  {0x37, 0x00, MSM_CAMERA_I2C_BURST_DATA},
  {0x38, 0x84, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1284
  {0x39, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x3a, 0x85, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1285
  {0x3b, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x3c, 0x86, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1286
  {0x3d, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
  {0x3e, 0x87, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1287
  {0x3f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
  {0x40, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1288
  {0x41, 0x30, MSM_CAMERA_I2C_BURST_DATA},
  {0x42, 0x89, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1289
  {0x43, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
  {0x44, 0x8a, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x128a
  {0x45, 0xb0, MSM_CAMERA_I2C_BURST_DATA},
  {0x46, 0x8b, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x128b
  {0x47, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x48, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x128c
	{0x49, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0x4a, 0x8d, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x128d
	{0x4b, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x4c, 0xe6, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x12e6
	{0x4d, 0xff, MSM_CAMERA_I2C_BURST_DATA},
	{0x4e, 0xe7, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x12e7
	{0x4f, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0xe8, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x12e8
	{0x51, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0xe9, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x12e9
	{0x53, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0x55, 0x13, MSM_CAMERA_I2C_BURST_DATA},//13 page
	{0x56, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1310
	{0x57, 0x3f, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1320
	{0x59, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x21, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1321
	{0x5b, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x22, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1322
	{0x5d, 0x36, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x23, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1323
	{0x5f, 0x6a, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x24, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1324
	{0x61, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x25, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1325
	{0x63, 0xc0, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x26, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1326
	{0x65, 0xe0, MSM_CAMERA_I2C_BURST_DATA},
	{0x66, 0x27, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1327 lum 0
	{0x67, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0x68, 0x28, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1328
	{0x69, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6a, 0x29, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1329
	{0x6b, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6c, 0x2a, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x132a
	{0x6d, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x6e, 0x2b, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x132b
	{0x6f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x70, 0x2c, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x132c
	{0x71, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x72, 0x2d, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x132d
	{0x73, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x74, 0x2e, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x132e lum7
	{0x75, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x76, 0x2f, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x132f	   //weight skin
	{0x77, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x78, 0x30, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1330	   //weight blue
	{0x79, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7a, 0x31, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1331	   //weight green
	{0x7b, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7c, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1332	   //weight strong color
	{0x7d, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x7e, 0x33, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1333
	{0x7f, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x80, 0x34, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1334
	{0x81, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x82, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1335
	{0x83, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x84, 0x36, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1336
	{0x85, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x86, 0xa0, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13a0
	{0x87, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0x88, 0xa8, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13a8	   //Dark2 Cb-filter 0x20
	{0x89, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x8a, 0xa9, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13a9	   //Dark2 Cr-filter 0x20
	{0x8b, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x8c, 0xaa, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13aa
	{0x8d, 0x30, MSM_CAMERA_I2C_BURST_DATA},
	{0x8e, 0xab, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13ab
	{0x8f, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0x90, 0xc0, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13c0
	{0x91, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0x92, 0xc2, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13c2
	{0x93, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x94, 0xc3, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13c3
	{0x95, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x96, 0xc4, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13c4
	{0x97, 0x46, MSM_CAMERA_I2C_BURST_DATA},
	{0x98, 0xc5, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13c5
	{0x99, 0x78, MSM_CAMERA_I2C_BURST_DATA},
	{0x9a, 0xc6, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13c6
	{0x9b, 0xf0, MSM_CAMERA_I2C_BURST_DATA},
	{0x9c, 0xc7, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13c7
	{0x9d, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x9e, 0xc8, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13c8
	{0x9f, 0x44, MSM_CAMERA_I2C_BURST_DATA},
	{0xa0, 0xc9, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13c9
	{0xa1, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0xa2, 0xca, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13ca
	{0xa3, 0xff, MSM_CAMERA_I2C_BURST_DATA},
	{0xa4, 0xcb, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13cb
	{0xa5, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0xa6, 0xcc, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13cc	   //skin range_cb_l
	{0xa7, 0x61, MSM_CAMERA_I2C_BURST_DATA},
	{0xa8, 0xcd, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13cd	   //skin range_cb_h
	{0xa9, 0x87, MSM_CAMERA_I2C_BURST_DATA},
	{0xaa, 0xce, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13ce	   //skin range_cr_l
	{0xab, 0x8a, MSM_CAMERA_I2C_BURST_DATA},
	{0xac, 0xcf, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x13cf	   //skin range_cr_h
	{0xad, 0xa5, MSM_CAMERA_I2C_BURST_DATA},
	{0xae, 0x03, MSM_CAMERA_I2C_BURST_DATA}, //14 page
	{0xaf, 0x14, MSM_CAMERA_I2C_BURST_DATA},
	{0xb0, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1410
  {0xb1, 0x27, MSM_CAMERA_I2C_BURST_DATA},
	{0xb2, 0x11, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1411
	{0xb3, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xb4, 0x12, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1412
	{0xb5, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //Top H_Clip
	{0xb6, 0x13, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1413
	{0xb7, 0xc8, MSM_CAMERA_I2C_BURST_DATA},
	{0xb8, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1414
	{0xb9, 0x50, MSM_CAMERA_I2C_BURST_DATA},
	{0xba, 0x15, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1415	   //sharp positive hi
	{0xbb, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xbc, 0x16, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1416	   //sharp positive mi
	{0xbd, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xbe, 0x17, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1417	   //sharp positive low
	{0xbf, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xc0, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1418	   //sharp negative hi
	{0xc1, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0xc2, 0x19, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1419	   //sharp negative mi
	{0xc3, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0xc4, 0x1a, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x141a	   //sharp negative low
	{0xc5, 0x33, MSM_CAMERA_I2C_BURST_DATA},
	{0xc6, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1420
	{0xc7, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xc8, 0x21, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1421
	{0xc9, 0x03, MSM_CAMERA_I2C_BURST_DATA},
	{0xca, 0x22, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1422
	{0xcb, 0x05, MSM_CAMERA_I2C_BURST_DATA},
	{0xcc, 0x23, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1423
	{0xcd, 0x07, MSM_CAMERA_I2C_BURST_DATA},
	{0xce, 0x24, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1424
	{0xcf, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xd0, 0x25, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1425
	{0xd1, 0x46, MSM_CAMERA_I2C_BURST_DATA},
	{0xd2, 0x26, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1426
	{0xd3, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xd4, 0x27, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1427
	{0xd5, 0x1e, MSM_CAMERA_I2C_BURST_DATA},
	{0xd6, 0x28, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1428
	{0xd7, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0xd8, 0x29, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1429
	{0xd9, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0xda, 0x2a, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x142a
	{0xdb, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xdc, 0x2b, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x142b
	{0xdd, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xde, 0x2c, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x142c
	{0xdf, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0xe0, 0x2d, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x142d
	{0xe1, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xe2, 0x2e, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x142e
	{0xe3, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xe4, 0x2f, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x142f
	{0xe5, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xe6, 0x30, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1430
	{0xe7, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0xe8, 0x31, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1431
	{0xe9, 0x02, MSM_CAMERA_I2C_BURST_DATA},
	{0xea, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1432
	{0xeb, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xec, 0x33, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1433
	{0xed, 0x04, MSM_CAMERA_I2C_BURST_DATA},
	{0xee, 0x34, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1434
	{0xef, 0x0a, MSM_CAMERA_I2C_BURST_DATA},
	{0xf0, 0x35, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1435
	{0xf1, 0x46, MSM_CAMERA_I2C_BURST_DATA},
	{0xf2, 0x36, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1436
	{0xf3, 0x32, MSM_CAMERA_I2C_BURST_DATA},
	{0xf4, 0x37, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1437
	{0xf5, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0xf6, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1438
	{0xf7, 0x12, MSM_CAMERA_I2C_BURST_DATA},//2d
	{0xf8, 0x39, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1439
	{0xf9, 0x00, MSM_CAMERA_I2C_BURST_DATA},//23
	{0xfa, 0x3a, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x143a
	{0xfb, 0x18, MSM_CAMERA_I2C_BURST_DATA}, //dr gain
	{0xfc, 0x3b, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x143b
	{0xfd, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	//{0xfe, 0x3c, MSM_CAMERA_I2C_BURST_DATA}, // STEVE deleted
	//{0xff, 0x20, MSM_CAMERA_I2C_BURST_DATA}, // STEVE deleted

	{0x03, 0xe5, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x3c, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x143c
	{0x11, 0x18, MSM_CAMERA_I2C_BURST_DATA},
	{0x12, 0x3d, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x143d
	{0x13, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //nor gain
	{0x14, 0x3e, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x143e
	{0x15, 0x22, MSM_CAMERA_I2C_BURST_DATA},
	{0x16, 0x3f, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x143f
	{0x17, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x18, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1440
	{0x19, 0x80, MSM_CAMERA_I2C_BURST_DATA},
	{0x1a, 0x41, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1441
	{0x1b, 0x12, MSM_CAMERA_I2C_BURST_DATA},
	{0x1c, 0x42, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1442
	{0x1d, 0xb0, MSM_CAMERA_I2C_BURST_DATA},
	{0x1e, 0x43, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1443
	{0x1f, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x20, 0x44, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1444
	{0x21, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x22, 0x45, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1445
	{0x23, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x24, 0x46, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1446
	{0x25, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x26, 0x47, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1447
	{0x27, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x28, 0x48, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1448
	{0x29, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x2a, 0x49, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1449
	{0x2b, 0x08, MSM_CAMERA_I2C_BURST_DATA},
	{0x2c, 0x50, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1450
  {0x2d, 0x00, MSM_CAMERA_I2C_BURST_DATA},
	{0x2e, 0x51, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1451
	{0x2f, 0x32, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x30, 0x52, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1452
	{0x31, 0x40, MSM_CAMERA_I2C_BURST_DATA},
	{0x32, 0x53, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1453
	{0x33, 0x19, MSM_CAMERA_I2C_BURST_DATA},
	{0x34, 0x54, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1454
	{0x35, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x36, 0x55, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1455
	{0x37, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x38, 0x56, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1456
	{0x39, 0x60, MSM_CAMERA_I2C_BURST_DATA},
	{0x3a, 0x57, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1457
	{0x3b, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x3c, 0x58, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1458
	{0x3d, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x3e, 0x59, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1459
	{0x3f, 0x20, MSM_CAMERA_I2C_BURST_DATA},
	{0x40, 0x60, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1460
	{0x41, 0x03, MSM_CAMERA_I2C_BURST_DATA}, //skin opt en
	{0x42, 0x61, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1461
	{0x43, 0xa0, MSM_CAMERA_I2C_BURST_DATA},
	{0x44, 0x62, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1462
	{0x45, 0x98, MSM_CAMERA_I2C_BURST_DATA},
	{0x46, 0x63, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1463
	{0x47, 0xe4, MSM_CAMERA_I2C_BURST_DATA}, //skin_std_th_h
	{0x48, 0x64, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1464
	{0x49, 0xa4, MSM_CAMERA_I2C_BURST_DATA}, //skin_std_th_l
	{0x4a, 0x65, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1465
	{0x4b, 0x7d, MSM_CAMERA_I2C_BURST_DATA}, //sharp_std_th_h
	{0x4c, 0x66, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1466
	{0x4d, 0x4b, MSM_CAMERA_I2C_BURST_DATA}, //sharp_std_th_l
	{0x4e, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1470
	{0x4f, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x50, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1471
	{0x51, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x52, 0x72, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1472
	{0x53, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x54, 0x73, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1473
	{0x55, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x56, 0x74, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1474
	{0x57, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x58, 0x75, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1475
	{0x59, 0x10, MSM_CAMERA_I2C_BURST_DATA},
	{0x5a, 0x76, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1476
	{0x5b, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x5c, 0x77, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1477
	{0x5d, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x5e, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1478
	{0x5f, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x60, 0x79, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1479
	{0x61, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x62, 0x7a, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x147a
	{0x63, 0x28, MSM_CAMERA_I2C_BURST_DATA},
	{0x64, 0x7b, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x147b
	{0x65, 0x28, MSM_CAMERA_I2C_BURST_DATA},

	// STEVE Saturation control
	// CB/CR vs sat
//LGE_CHANGE_S ADD INTO MR ver. for low light condition

	{0x66, 0x03, MSM_CAMERA_I2C_BURST_DATA}, //page 10
	{0x67, 0x10, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x68, 0x70, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1070
	{0x69, 0x08, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6a, 0x71, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1071
	{0x6b, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6c, 0x72, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1072
	{0x6d, 0xbe, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x6e, 0x73, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1073
	{0x6f, 0x88, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x70, 0x74, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1074
	{0x71, 0x51, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x72, 0x75, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1075
	{0x73, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x74, 0x76, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1076
	{0x75, 0x23, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x76, 0x77, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1077
	{0x77, 0x31, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x78, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1078
	{0x79, 0xeb, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7a, 0x79, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x1079
	{0x7b, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7c, 0x7a, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x107a
	{0x7d, 0x51, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x7e, 0x7b, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x107b
	{0x7f, 0x40, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x80, 0x7c, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x107c
	{0x81, 0x00, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x82, 0x7d, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x107d
	{0x83, 0x14, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x84, 0x7e, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x107e
	{0x85, 0x20, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x86, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //Dark2 0x107f
	{0x87, 0x38, MSM_CAMERA_I2C_BURST_DATA}, //

	// Lum Vs Sat
	{0x88, 0x03, MSM_CAMERA_I2C_BURST_DATA}, // page 16
	{0x89, 0x16, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8A, 0x8a, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x168a
	{0x8B, 0x51, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8C, 0x8b, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x168b
	{0x8D, 0x5a, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x8E, 0x8c, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x168c
	{0x8F, 0x64, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x90, 0x8d, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x168d
	{0x91, 0x6e, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x92, 0x8e, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x168e
	{0x93, 0x7b, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x94, 0x8f, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x168f
	{0x95, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x96, 0x90, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x1690
	{0x97, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x98, 0x91, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x1691
	{0x99, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9A, 0x92, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x1692
	{0x9B, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9C, 0x93, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x1693
	{0x9D, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0x9E, 0x94, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x1694
	{0x9F, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA0, 0x95, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x1695
	{0xA1, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA2, 0x96, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x1696
	{0xA3, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA4, 0x97, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x1697
	{0xA5, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA6, 0x98, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x1698
	{0xA7, 0x7f, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xA8, 0x99, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x1699
	{0xA9, 0x7c, MSM_CAMERA_I2C_BURST_DATA}, //
	{0xAA, 0x9a, MSM_CAMERA_I2C_BURST_DATA}, // Dark2 0x169a
	{0xAB, 0x78, MSM_CAMERA_I2C_BURST_DATA}, //
	// DMA END
//LGE_CHANGE_E ADD INTO MR ver. for low light condition


	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x01, 0xF1, MSM_CAMERA_I2C_BYTE_DATA}, //Sleep mode on

	{0x03, 0xc0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x16, 0x80, MSM_CAMERA_I2C_BYTE_DATA}, //MCU main roof holding off

	{0x03, 0xC0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x33, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, //DMA hand shake mode set
	{0x32, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, //DMA off
	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x11, 0x04, MSM_CAMERA_I2C_BYTE_DATA}, //Bit[0]: MCU hold off

	{0x03, 0xc0, MSM_CAMERA_I2C_BYTE_DATA},
	{0xe1, 0x00, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0x25, 0x0e, MSM_CAMERA_I2C_BYTE_DATA},
	{0x25, 0x1e, MSM_CAMERA_I2C_BYTE_DATA},
	///////////////////////////////////////////
	// 1F Page SSD
	///////////////////////////////////////////
	{0x03, 0x1f, MSM_CAMERA_I2C_BYTE_DATA}, //1F page
	{0x11, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //bit[5:4]: debug mode
	{0x12, 0x60, MSM_CAMERA_I2C_BYTE_DATA},
	{0x13, 0x14, MSM_CAMERA_I2C_BYTE_DATA},
	{0x14, 0x10, MSM_CAMERA_I2C_BYTE_DATA},
	{0x15, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x20, 0x18, MSM_CAMERA_I2C_BYTE_DATA}, //ssd_x_start_pos
	{0x21, 0x14, MSM_CAMERA_I2C_BYTE_DATA}, //ssd_y_start_pos
	{0x22, 0x8C, MSM_CAMERA_I2C_BYTE_DATA}, //ssd_blk_width
	{0x23, 0x9C, MSM_CAMERA_I2C_BYTE_DATA}, //ssd_blk_height
	{0x28, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x29, 0x02, MSM_CAMERA_I2C_BYTE_DATA},
	{0x3B, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0x3C, 0x8C, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x19, MSM_CAMERA_I2C_BYTE_DATA}, //SSD enable

	{0x03, 0xc4, MSM_CAMERA_I2C_BYTE_DATA}, //AE en
#if defined (LGE_CAMERA_ANTIBAND_50HZ)	//Flicker 50Hz
	{0x10, 0xe9, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE AE ON    (50hz : 0xe9, 60hz : 0xe1)

#else
	{0x10, 0xe1, MSM_CAMERA_I2C_BYTE_DATA},

#endif

	{0x03, 0xc3, MSM_CAMERA_I2C_BYTE_DATA}, //AE Static en
	{0x10, 0x84, MSM_CAMERA_I2C_BYTE_DATA},

	///////////////////////////////////////////
	// 30 Page DMA address set
	///////////////////////////////////////////
	{0x03, 0x30, MSM_CAMERA_I2C_BYTE_DATA}, //DMA
	{0x7c, 0x2c, MSM_CAMERA_I2C_BYTE_DATA}, //Extra str
	{0x7d, 0xce, MSM_CAMERA_I2C_BYTE_DATA},
	{0x7e, 0x2d, MSM_CAMERA_I2C_BYTE_DATA}, //Extra end
	{0x7f, 0xbb, MSM_CAMERA_I2C_BYTE_DATA},
	{0x80, 0x24, MSM_CAMERA_I2C_BYTE_DATA}, //Outdoor str
	{0x81, 0x70, MSM_CAMERA_I2C_BYTE_DATA},
	{0x82, 0x27, MSM_CAMERA_I2C_BYTE_DATA}, //Outdoor end
	{0x83, 0x39, MSM_CAMERA_I2C_BYTE_DATA},
	{0x84, 0x21, MSM_CAMERA_I2C_BYTE_DATA}, //Indoor str
	{0x85, 0xa6, MSM_CAMERA_I2C_BYTE_DATA},
	{0x86, 0x24, MSM_CAMERA_I2C_BYTE_DATA}, //Indoor end
	{0x87, 0x6f, MSM_CAMERA_I2C_BYTE_DATA},
	{0x88, 0x27, MSM_CAMERA_I2C_BYTE_DATA}, //Dark1 str
	{0x89, 0x3a, MSM_CAMERA_I2C_BYTE_DATA},
	{0x8a, 0x2a, MSM_CAMERA_I2C_BYTE_DATA}, //Dark1 end
	{0x8b, 0x03, MSM_CAMERA_I2C_BYTE_DATA},
	{0x8c, 0x2a, MSM_CAMERA_I2C_BYTE_DATA}, //Dark2 str
	{0x8d, 0x04, MSM_CAMERA_I2C_BYTE_DATA},
	{0x8e, 0x2c, MSM_CAMERA_I2C_BYTE_DATA}, //Dark2 end
	{0x8f, 0xcd, MSM_CAMERA_I2C_BYTE_DATA},



	///////////////////////////////////////////
	// CD Page (Color ratio)
	///////////////////////////////////////////
	{0x03, 0xCD, MSM_CAMERA_I2C_BYTE_DATA},
	{0x10, 0x38, MSM_CAMERA_I2C_BYTE_DATA}, //ColorRatio disable ½ÃÅ´


	{0x03, 0xc9, MSM_CAMERA_I2C_BYTE_DATA}, //AWB Start Point
	{0x2a, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x2b, 0xb2, MSM_CAMERA_I2C_BYTE_DATA},
	{0x2c, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x2d, 0x82, MSM_CAMERA_I2C_BYTE_DATA},
	{0x2e, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x2f, 0xb2, MSM_CAMERA_I2C_BYTE_DATA},
	{0x30, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x31, 0x82, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0xc5, MSM_CAMERA_I2C_BYTE_DATA}, //AWB en
	{0x10, 0xb1, MSM_CAMERA_I2C_BYTE_DATA},

	{0x03, 0xcf, MSM_CAMERA_I2C_BYTE_DATA}, //Adative en
	{0x10, 0xaf, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE 8f -) af ALL ON :Ytar, Gam, CCM, Sat, LSC, MCMC , (Yoffs, Contrast)

	///////////////////////////////////////////
	// 48 Page MIPI setting
	///////////////////////////////////////////
	{0x03, 0x48, MSM_CAMERA_I2C_BYTE_DATA},
	{0x09, 0xa2, MSM_CAMERA_I2C_BYTE_DATA}, //MIPI CLK
	{0x10, 0x1C, MSM_CAMERA_I2C_BYTE_DATA}, //MIPI ON
	{0x11, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //Normal Mode
	{0x14, 0x50, MSM_CAMERA_I2C_BYTE_DATA}, //Skew
	{0x16, 0x04, MSM_CAMERA_I2C_BYTE_DATA},

	{0x1a, 0x11, MSM_CAMERA_I2C_BYTE_DATA},
	{0x1b, 0x0d, MSM_CAMERA_I2C_BYTE_DATA}, //Short Packet
	{0x1c, 0x0a, MSM_CAMERA_I2C_BYTE_DATA}, //Control DP
	{0x1d, 0x0f, MSM_CAMERA_I2C_BYTE_DATA}, //Control DN
	{0x1e, 0x09, MSM_CAMERA_I2C_BYTE_DATA},
	{0x1f, 0x07, MSM_CAMERA_I2C_BYTE_DATA},
	{0x20, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x24, 0x1e, MSM_CAMERA_I2C_BYTE_DATA}, //Bayer8 : 2a, Bayer10 : 2b, YUV : 1e

	{0x30, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, //1024*2
	{0x31, 0x08, MSM_CAMERA_I2C_BYTE_DATA}, // Steve for 1024x768
	{0x32, 0x0f, MSM_CAMERA_I2C_BYTE_DATA}, // Tclk zero
	{0x34, 0x06, MSM_CAMERA_I2C_BYTE_DATA}, // Tclk prepare

	{0x39, 0x03, MSM_CAMERA_I2C_BYTE_DATA}, //Drivability 00

	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x0c, 0xf0, MSM_CAMERA_I2C_BYTE_DATA}, //Parallel Line Off

	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x11, 0x83, MSM_CAMERA_I2C_BYTE_DATA}, // STEVE 0 skip Fix Frame Off, XY Flip

	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x01, 0xf0, MSM_CAMERA_I2C_BYTE_DATA}, //sleep off

	{0x03, 0xC0, MSM_CAMERA_I2C_BYTE_DATA},
	{0x33, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x32, 0x01, MSM_CAMERA_I2C_BYTE_DATA}, //DMA on

	//////////////////////////////////////////////
	// Delay
	//////////////////////////////////////////////
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},
	{0x03, 0x00, MSM_CAMERA_I2C_BYTE_DATA},



};


#endif	/* HI351_REG_H */
