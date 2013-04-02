/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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
#define SENSOR_NAME "t8ev4"
#define PLATFORM_DRIVER_NAME "msm_camera_t8ev4"
#define t8ev4_obj t8ev4_##obj

DEFINE_MUTEX(t8ev4_mut);
static struct msm_sensor_ctrl_t t8ev4_s_ctrl;

static struct msm_camera_i2c_reg_conf t8ev4_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf t8ev4_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf t8ev4_groupon_settings[] = {
	{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf t8ev4_groupoff_settings[] = {
	{0x104, 0x00},
};

static struct msm_camera_i2c_reg_conf t8ev4_prev_settings[] = {
	{0x0340, 0x04},//,FR_LENGTH_LINES[15:8]  ;
	{0x0341, 0xF4},//,FR_LENGTH_LINES[7:0]	;
	{0x0342, 0x0D},//,LINE_LENGTH_PCK[15:8]  ;
	{0x0343, 0x7C},//,LINE_LENGTH_PCK[7:0]	;
	{0x0344, 0x00},//,X_ADDR_START[15:8]  ;
	{0x0345, 0x00},//,X_ADDR_START[7:0]  ;
	{0x0346, 0x00},//,Y_ADDR_START[15:8]  ;
	{0x0347, 0x00},//,Y_ADDR_START[7:0]  ;
	{0x0348, 0x0C},//,X_ADDR_END[15:8]	;
	{0x0349, 0xCF},//,X_ADDR_END[7:0]  ;
	{0x034A, 0x09},//,Y_ADDR_END[15:8]	;
	{0x034B, 0x9F},//,Y_ADDR_END[7:0]  ;
	{0x034C, 0x06},//,X_OUTPUT_SIZE[15:8]  ;
	{0x034D, 0x60},//,X_OUTPUT_SIZE[7:0]  ;
	{0x034E, 0x04},//,Y_OUTPUT_SIZE[15:8]  ;
	{0x034F, 0xD0},//,Y_OUTPUT_SIZE[7:0]  ;
	{0x0401, 0x02},//,-/-/-/-/-/-/SCALING_MODE[1:0]  ;
	{0x0405, 0x10},//,SCALE_M[7:0]	;
	{0x0408, 0x00},//,-/-/-/-/DCROP_XOFS[11:8]	;
	{0x0409, 0x00},//,DCROP_XOFS[7:0]  ;
	{0x040A, 0x00},//,-/-/-/-/DCROP_YOFS[11:8]	;
	{0x040B, 0x00},//,DCROP_YOFS[7:0]  ;
	{0x040C, 0x06},//,-/-/-/-/DCROP_WIDTH[11:8]  ;
	{0x040D, 0x60},//,DCROP_WIDTH[7:0]	;
	{0x040E, 0x04},//,-/-/-/-/DCROP_HIGT[11:8]	;
	{0x040F, 0xD0},//,DCROP_HIGT[7:0]  ;
	{0x0820, 0x05},//,CSI_BITRATE[15:8]  ;
	{0x0821, 0x10},//,CSI_BITRATE[7:0]	;
	{0x0900, 0x01},//,-/-/-/-/-/-/-/BINNING_MODE  ;
	{0x0901, 0x22},//,-/-/H_ANABIN[1:0]/V_MONI[3:0]  ;
};

static struct msm_camera_i2c_reg_conf t8ev4_snap_settings[] = {
	{0x0340, 0x09},//,FR_LENGTH_LINES[15:8]  ;
	{0x0341, 0xC7},//,FR_LENGTH_LINES[7:0]	;
	{0x0342, 0x0D},//,LINE_LENGTH_PCK[15:8]  ;
	{0x0343, 0x7C},//,LINE_LENGTH_PCK[7:0]	;
	{0x0344, 0x00},//,X_ADDR_START[15:8]  ;
	{0x0345, 0x00},//,X_ADDR_START[7:0]  ;
	{0x0346, 0x00},//,Y_ADDR_START[15:8]  ;
	{0x0347, 0x00},//,Y_ADDR_START[7:0]  ;
	{0x0348, 0x0C},//,X_ADDR_END[15:8]	;
	{0x0349, 0xCF},//,X_ADDR_END[7:0]  ;
	{0x034A, 0x09},//,Y_ADDR_END[15:8]	;
	{0x034B, 0x9F},//,Y_ADDR_END[7:0]  ;
	{0x034C, 0x0C},//,X_OUTPUT_SIZE[15:8]  ;
	{0x034D, 0xD0},//,X_OUTPUT_SIZE[7:0]  ;
	{0x034E, 0x09},//,Y_OUTPUT_SIZE[15:8]  ;
	{0x034F, 0xA0},//,Y_OUTPUT_SIZE[7:0]  ;
	{0x0401, 0x02},//,-/-/-/-/-/-/SCALING_MODE[1:0]  ;
	{0x0405, 0x10},//,SCALE_M[7:0]	;
	{0x0408, 0x00},//,-/-/-/-/DCROP_XOFS[11:8]	;
	{0x0409, 0x00},//,DCROP_XOFS[7:0]  ;
	{0x040A, 0x00},//,-/-/-/-/DCROP_YOFS[11:8]	;
	{0x040B, 0x00},//,DCROP_YOFS[7:0]  ;
	{0x040C, 0x0C},//,-/-/-/-/DCROP_WIDTH[11:8]  ;
	{0x040D, 0xD0},//,DCROP_WIDTH[7:0]	;
	{0x040E, 0x09},//,-/-/-/-/DCROP_HIGT[11:8]	;
	{0x040F, 0xA0},//,DCROP_HIGT[7:0]  ;
	{0x0820, 0x05},//,CSI_BITRATE[15:8]  ;
	{0x0821, 0x10},//,CSI_BITRATE[7:0]	;
	{0x0900, 0x01},//,-/-/-/-/-/-/-/BINNING_MODE  ;
	{0x0901, 0x11},//,-/-/H_ANABIN[1:0]/V_MONI[3:0]  ;
};

static struct msm_camera_i2c_reg_conf t8ev4_recommend_settings[] = {	
	{0x0000, 0x14},//,[RO] VERNUM[15:8]  ;
	{0x0001, 0x03},//,[RO] VERNUM[7:0] ;
	{0x0101, 0x03},// xy flip //,-/-/-/-/-/-/IMAGE_ORIENT[1:0]  ;
	{0x0103, 0x00},//,-/-/-/-/-/-/-/SOFTWARE_RESET	;
	{0x0104, 0x00},//,-/-/-/-/-/-/-/GROUP_PARA_HOLD  ;
	{0x0105, 0x00},//,-/-/-/-/-/-/-/MSK_CORRUPT_FR	;
	{0x0110, 0x00},//,-/-/-/-/-/CSI_CHAN_IDNTF[2:0]  ;
	{0x0111, 0x02},//,-/-/-/-/-/-/CSI_SIGNAL_MOD[1:0]  ;
	{0x0112, 0x0A},//,CSI_DATA_FORMAT[15:8]  ;
	{0x0113, 0x0A},//,CSI_DATA_FORMAT[7:0]	;
	{0x0114, 0x01},//,-/-/-/-/-/-/-/CSI_LANE_MODE  ;
	{0x0115, 0x30},//,-/-/CSI_10TO8_DT[5:0]  ;
	{0x0117, 0x32},//,-/-/CSI_10TO6_DT[5:0]  ;
	{0x0202, 0x04},//,INTEGR_TIM[15:8]	;
	{0x0203, 0xAD},//,INTEGR_TIM[7:0]  ;
	{0x0204, 0x00},//,ANA_GA_CODE_GL[15:8]	;
	{0x0205, 0x48},//,ANA_GA_CODE_GL[7:0]  ;
	{0x020E, 0x01},//,-/-/-/-/-/-/DG_GA_GREENR[9:8]  ;
	{0x020F, 0x00},//,DG_GA_GREENR[7:0]  ;
	{0x0210, 0x01},//,-/-/-/-/-/-/DG_GA_RED[9:8]  ;
	{0x0211, 0x00},//,DG_GA_RED[7:0]  ;
	{0x0212, 0x01},//,-/-/-/-/-/-/DG_GA_BLUE[9:8]  ;
	{0x0213, 0x00},//,DG_GA_BLUE[7:0]  ;
	{0x0214, 0x01},//,-/-/-/-/-/-/DG_GA_GREENB[9:8]  ;
	{0x0215, 0x00},//,DG_GA_GREENB[7:0]  ;
	{0x0301, 0x05},//,-/-/-/-/VT_PIX_CLK_DIV[3:0]  ;
	{0x0303, 0x01},//,-/-/-/-/VT_SYS_CLK_DIV[3:0]  ;
	{0x0305, 0x02},//,-/-/-/-/-/PRE_PLL_CLK_DIV[2:0]  ;
	{0x0306, 0x00},//,-/-/-/-/-/-/-/PLL_MULTIPLIER[8]  ;
	{0x0307, 0x36},//,PLL_MULTIPLIER[7:0]  ;
	{0x0309, 0x08},//,-/-/-/-/-/-/-/-;
	{0x030B, 0x01},//,-/-/-/-/OP_SYS_CLK_DIV[3:0]  ;
	{0x0340, 0x04},//,FR_LENGTH_LINES[15:8]  ;
	{0x0341, 0xF4},//,FR_LENGTH_LINES[7:0]	;
	{0x0342, 0x0D},//,LINE_LENGTH_PCK[15:8]  ;
	{0x0343, 0x7C},//,LINE_LENGTH_PCK[7:0]	;
	{0x0344, 0x00},//,X_ADDR_START[15:8]  ;
	{0x0345, 0x00},//,X_ADDR_START[7:0]  ;
	{0x0346, 0x00},//,Y_ADDR_START[15:8]  ;
	{0x0347, 0x00},//,Y_ADDR_START[7:0]  ;
	{0x0348, 0x0C},//,X_ADDR_END[15:8]	;
	{0x0349, 0xCF},//,X_ADDR_END[7:0]  ;
	{0x034A, 0x09},//,Y_ADDR_END[15:8]	;
	{0x034B, 0x9F},//,Y_ADDR_END[7:0]  ;
	{0x034C, 0x06},//,X_OUTPUT_SIZE[15:8]  ;
	{0x034D, 0x60},//,X_OUTPUT_SIZE[7:0]  ;
	{0x034E, 0x04},//,Y_OUTPUT_SIZE[15:8]  ;
	{0x034F, 0xD0},//,Y_OUTPUT_SIZE[7:0]  ;
	{0x0381, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x0383, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x0401, 0x02},//,-/-/-/-/-/-/SCALING_MODE[1:0]  ;
	{0x0403, 0x00},//,-/-/-/-/-/-/-/SPATIAL_SAMPLING  ;
	{0x0405, 0x10},//,SCALE_M[7:0]	;
	{0x0408, 0x00},//,-/-/-/-/DCROP_XOFS[11:8]	;
	{0x0409, 0x00},//,DCROP_XOFS[7:0]  ;
	{0x040A, 0x00},//,-/-/-/-/DCROP_YOFS[11:8]	;
	{0x040B, 0x00},//,DCROP_YOFS[7:0]  ;
	{0x040C, 0x06},//,-/-/-/-/DCROP_WIDTH[11:8]  ;
	{0x040D, 0x60},//,DCROP_WIDTH[7:0]	;
	{0x040E, 0x04},//,-/-/-/-/DCROP_HIGT[11:8]	;
	{0x040F, 0xD0},//,DCROP_HIGT[7:0]  ;
	{0x0601, 0x00},//,TEST_PATT_MODE[7:0]  ;
	{0x0602, 0x02},//,-/-/-/-/-/-/TEST_DATA_RED[9:8]  ;
	{0x0603, 0xC0},//,TEST_DATA_RED[7:0]  ;
	{0x0604, 0x02},//,-/-/-/-/-/-/TEST_DATA_GREENR[9:8]  ;
	{0x0605, 0xC0},//,TEST_DATA_GREENR[7:0]  ;
	{0x0606, 0x02},//,-/-/-/-/-/-/TEST_DATA_BLUE[9:8]  ;
	{0x0607, 0xC0},//,TEST_DATA_BLUE[7:0]  ;
	{0x0608, 0x02},//,-/-/-/-/-/-/TEST_DATA_GREENB[9:8]  ;
	{0x0609, 0xC0},//,TEST_DATA_GREENB[7:0]  ;
	{0x060A, 0x00},//,HO_CURS_WIDTH[15:8]  ;
	{0x060B, 0x00},//,HO_CURS_WIDTH[7:0]  ;
	{0x060C, 0x00},//,HO_CURS_POSITION[15:8]  ;
	{0x060D, 0x00},//,HO_CURS_POSITION[7:0]  ;
	{0x060E, 0x00},//,VE_CURS_WIDTH[15:8]  ;
	{0x060F, 0x00},//,VE_CURS_WIDTH[7:0]  ;
	{0x0610, 0x00},//,VE_CURS_POSITION[15:8]  ;
	{0x0611, 0x00},//,VE_CURS_POSITION[7:0]  ;
	{0x0800, 0x68},//,TCLK_POST[4:0]/-/-/- ;
	{0x0801, 0x28},//,THS_PREPARE[4:0]/-/-/- ;
	{0x0802, 0x60},//,THS_ZERO[4:0]/-/-/- ;
	{0x0803, 0x38},//,THS_TRAIL[7:3]/-/-/- ;
	{0x0804, 0x38},//,TCLK_TRAIL[7:3]/-/-/- ;
	{0x0805, 0x28},//,TCLK_PREPARE[7:3]/-/-/- ;
	{0x0806, 0xF8},//,TCLK_ZERO[7:3]/-/-/- ;
	{0x0807, 0x30},//,TLPX[7:3]/-/-/- ;
	{0x0808, 0x01},//,-/-/-/-/-/-/DPHY_CTRL[1:0]  ;
	{0x0820, 0x05},//,CSI_BITRATE[15:8]  ;
	{0x0821, 0x10},//,CSI_BITRATE[7:0]	;
	{0x0900, 0x01},//,-/-/-/-/-/-/-/BINNING_MODE  ;
	{0x0901, 0x22},//,-/-/H_ANABIN[1:0]/V_MONI[3:0]  ;
	{0x0902, 0x00},//,-/-/-/-/-/-/BINNING_WEIGHTING[1:0]  ;
	{0x0B00, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x0B01, 0x80},//,-/-/-/-/-/-/-/-  ;
	{0x0B05, 0x00},//,-/-/-/-/-/-/-/-  ;
	{0x0B06, 0x01},//,-/-/-/-/-/-/-/-  ;
	{0x0B07, 0x40},//,-/-/-/-/-/-/-/-  ;
	{0x0B0A, 0x00},//,-/-/-/-/-/-/-/-  ;
	{0x0B0B, 0x40},//,-/-/-/-/-/-/-/-  ;
	{0x3001, 0x00},//,-/-/-/-/-/-/CLKULPS/ESCREQ ;
	{0x3005, 0x78},//,ESCDATA[7:0] ;
	{0x3006, 0x00},//,LVDS_D1_DELAY[3:0]/LVDS_D2_DELAY[3:0]  ;
	{0x3007, 0x00},//,-/-/-/-/LVDS_CLK_DELAY[3:0]  ;
	{0x3008, 0x40},//,-/-/-/-/-/-/-/- ;
	{0x3009, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x300A, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x300B, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x300C, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x300D, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x300E, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3010, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3011, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3012, 0x01},//,-/-/-/-/-/-/-/LNKBTWK_ON ;
	{0x3013, 0xA7},//,NUMWAKE[7:0] ;
	{0x3020, 0x27},//,-/-/-/-/-/-/-/- ;
	{0x3021, 0x07},//,-/-/-/-/-/-/-/- ;
	{0x3030, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3031, 0xCD},//,-/-/-/-/-/-/-/EQ_MONI  ;
	{0x3032, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3033, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x30A2, 0xF0},//,DCLK_DRVUP/DOUT_DRVUP/SDA_DRVUP/-/-/-/-/- ;
	{0x30A3, 0xA0},//,PARA_HZ/DCLK_POL/-/-/-/-/-/- ;
	{0x30A4, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x30A5, 0x01},//,-/-/PHY_PWRON_SW/-/-/-/PHY_PWRON_MN/-  ;
	{0x30A6, 0x03},//,-/-/-/-/-/-/-/- ;
	{0x30A7, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x30A8, 0x21},//,-/-/-/-/-/-/-/- ;
	{0x30A9, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x30AA, 0x8B},//,-/-/-/-/-/-/-/- ;
	{0x30AB, 0x13},//,-/-/-/-/-/-/-/- ;
	{0x30AC, 0x18},//,EXTCLK_FRQ_MHZ[15:8] ;
	{0x30AD, 0x00},//,EXTCLK_FRQ_MHZ[7:0] ;
	{0x3100, 0x07},//,OTP_RWT/OTP_RNUM[1:0]/-/-/OTP_PCLK[2:0]  ;
	{0x3101, 0x00},//,-/-/-/OTP_VE/-/-/-/-	;
	{0x3102, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3104, 0x00},//,OTP_LD_FEND/OTP_LD_RELD/-/-/-/OTP_LD_STS[1:0]/OTP_LD_ING	;
	{0x3120, 0x00},//,DFCT_TYP0[1:0]/-/-/DFCT_XADR0[11:8]  ;
	{0x3121, 0x00},//,DFCT_XADR0[7:0]  ;
	{0x3122, 0x00},//,-/-/-/-/DFCT_YADR0[11:8]	;
	{0x3123, 0x00},//,DFCT_YADR0[7:0]  ;
	{0x3124, 0x00},//,DFCT_TYP1[1:0]/-/-/DFCT_XADR1[11:8]  ;
	{0x3125, 0x00},//,DFCT_XADR1[7:0]  ;
	{0x3126, 0x00},//,-/-/-/-/DFCT_YADR1[11:8]	;
	{0x3127, 0x00},//,DFCT_YADR1[7:0]  ;
	{0x3128, 0x00},//,DFCT_TYP2[1:0]/-/-/DFCT_XADR2[11:8]  ;
	{0x3129, 0x00},//,DFCT_XADR2[7:0]  ;
	{0x312A, 0x00},//,-/-/-/-/DFCT_YADR2[11:8]	;
	{0x312B, 0x00},//,DFCT_YADR2[7:0]  ;
	{0x312C, 0x00},//,DFCT_TYP3[1:0]/-/-/DFCT_XADR3[11:8]  ;
	{0x312D, 0x00},//,DFCT_XADR3[7:0]  ;
	{0x312E, 0x00},//,-/-/-/-/DFCT_YADR3[11:8]	;
	{0x312F, 0x00},//,DFCT_YADR3[7:0]  ;
	{0x3130, 0x00},//,DFCT_TYP4[1:0]/-/-/DFCT_XADR4[11:8]  ;
	{0x3131, 0x00},//,DFCT_XADR4[7:0]  ;
	{0x3132, 0x00},//,-/-/-/-/DFCT_YADR4[11:8]	;
	{0x3133, 0x00},//,DFCT_YADR4[7:0]  ;
	{0x3134, 0x00},//,DFCT_TYP5[1:0]/-/-/DFCT_XADR5[11:8]  ;
	{0x3135, 0x00},//,DFCT_XADR5[7:0]  ;
	{0x3136, 0x00},//,-/-/-/-/DFCT_YADR5[11:8]	;
	{0x3137, 0x00},//,DFCT_YADR5[7:0]  ;
	{0x3138, 0x00},//,DFCT_TYP6[1:0]/-/-/DFCT_XADR6[11:8]  ;
	{0x3139, 0x00},//,DFCT_XADR6[7:0]  ;
	{0x313A, 0x00},//,-/-/-/-/DFCT_YADR6[11:8]	;
	{0x313B, 0x00},//,DFCT_YADR6[7:0]  ;
	{0x313C, 0x00},//,DFCT_TYP7[1:0]/-/-/DFCT_XADR7[11:8]  ;
	{0x313D, 0x00},//,DFCT_XADR7[7:0]  ;
	{0x313E, 0x00},//,-/-/-/-/DFCT_YADR7[11:8]	;
	{0x313F, 0x00},//,DFCT_YADR7[7:0]  ;
	{0x3200, 0x18},//,-/-/-/-/-/-/-/- ;
	{0x3201, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x3202, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3203, 0x20},//,-/-/-/-/-/-/-/- ;
	{0x3204, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x3205, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3206, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x321D, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x321E, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x321F, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3220, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3221, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3222, 0x0C},//,-/-/-/-/-/-/-/- ;
	{0x3223, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3225, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3226, 0x81},//,-/-/-/-/-/-/-/- ;
	{0x3227, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3228, 0x13},//,-/-/-/-/-/-/-/- ;
	{0x3229, 0x11},//,-/-/-/-/-/-/-/- ;
	{0x322A, 0x02},//,-/-/-/-/-/-/-/- ;
	{0x322B, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x322C, 0x22},//,-/-/-/-/-/-/-/- ;
	{0x322D, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x322E, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x322F, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3230, 0x0D},//,-/-/-/-/-/-/-/- ;
	{0x3231, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3232, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3233, 0x0D},//,-/-/-/-/-/-/-/- ;
	{0x3234, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3235, 0x17},//,-/-/-/-/-/-/-/- ;
	{0x3236, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3237, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x3238, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3239, 0x00},//,AG_LIMSW/-/-/-/-/-/-/- ;
	{0x323A, 0x33},//,AG_MIN[7:0]  ;
	{0x323B, 0x48},//,AG_MAX[7:0]  ;
	{0x323C, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x323D, 0x0E},//,-/-/-/-/-/-/-/- ;
	{0x323E, 0x03},//,-/-/-/-/-/-/-/- ;
	{0x323F, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3240, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3241, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3242, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3243, 0x12},//,-/-/-/-/-/-/-/- ;
	{0x3244, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x3245, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x3246, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x3247, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x3248, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3249, 0x87},//,-/-/-/-/-/-/-/- ;
	{0x3257, 0xC1},//,-/-/-/-/-/-/-/- ;
	{0x3258, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x3259, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x325A, 0x30},//,-/-/-/-/-/-/-/- ;
	{0x325B, 0x60},//,-/-/-/-/-/-/-/- ;
	{0x325C, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x325D, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x325E, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x325F, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x3260, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x3269, 0x24},//,-/-/-/-/-/-/-/- ;
	{0x326A, 0x82},//,-/-/-/-/-/-/-/- ;
	{0x326B, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x326C, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x326D, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x326E, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x326F, 0xAB},//,-/-/-/-/-/-/-/- ;
	{0x3270, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3273, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x3276, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3277, 0x80},//,PWB_WLRG[7:0]  ;
	{0x3278, 0x80},//,PWB_WLGRG[7:0]  ;
	{0x3279, 0x80},//,PWB_WLGBG[7:0]  ;
	{0x327A, 0x80},//,PWB_WLBG[7:0]  ;
	{0x3280, 0x01},//,LSSC_SW/-/-/-/-/-/-/- ;
	{0x3281, 0x53},//,-/-/-/-/-/-/-/- ;
	{0x3282, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x3283, 0x80},//,LSSC_HCEN_ADJ[7:0]  ;
	{0x3284, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x3285, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3286, 0x81},//,-/-/-/-/-/-/-/- ;
	{0x3287, 0x00},//,LSSC_VCEN_ADJ[7:0]  ;
	{0x3288, 0x02},//,-/-/-/-/-/-/LSSC_VCEN_ADJ[9:8]  ;
	{0x3289, 0x00},//,LSSC_TOPL_PM1RG[7:0]	;
	{0x328A, 0x00},//,LSSC_TOPL_PM1GRG[7:0]  ;
	{0x328B, 0x00},//,LSSC_TOPL_PM1GBG[7:0]  ;
	{0x328C, 0x00},//,LSSC_TOPL_PM1BG[7:0]	;
	{0x328D, 0x00},//,LSSC_TOPR_PM1RG[7:0]	;
	{0x328E, 0x00},//,LSSC_TOPR_PM1GRG[7:0]  ;
	{0x328F, 0x00},//,LSSC_TOPR_PM1GBG[7:0]  ;
	{0x3290, 0x00},//,LSSC_TOPR_PM1BG[7:0]	;
	{0x3291, 0x00},//,LSSC_BOTL_PM1RG[7:0]	;
	{0x3292, 0x00},//,LSSC_BOTL_PM1GRG[7:0]  ;
	{0x3293, 0x00},//,LSSC_BOTL_PM1GBG[7:0]  ;
	{0x3294, 0x00},//,LSSC_BOTL_PM1BG[7:0]	;
	{0x3295, 0x00},//,LSSC_BOTR_PM1RG[7:0]	;
	{0x3296, 0x00},//,LSSC_BOTR_PM1GRG[7:0]  ;
	{0x3297, 0x00},//,LSSC_BOTR_PM1GBG[7:0]  ;
	{0x3298, 0x00},//,LSSC_BOTR_PM1BG[7:0]	;
	{0x3299, 0x00},//,-/-/-/-/LSSC1BG_PMSW/LSSC1GBG_PMSW/LSSC1GRG_PMSW/LSSC1RG_PMSW  ;
	{0x329A, 0xFF},//,LSSC_LEFT_P2RG[7:0]  ;
	{0x329B, 0xFF},//,LSSC_LEFT_P2GRG[7:0]	;
	{0x329C, 0xFF},//,LSSC_LEFT_P2GBG[7:0]	;
	{0x329D, 0xFF},//,LSSC_LEFT_P2BG[7:0]  ;
	{0x329E, 0xFF},//,LSSC_RIGHT_P2RG[7:0]	;
	{0x329F, 0xFF},//,LSSC_RIGHT_P2GRG[7:0]  ;
	{0x32A0, 0xFF},//,LSSC_RIGHT_P2GBG[7:0]  ;
	{0x32A1, 0xFF},//,LSSC_RIGHT_P2BG[7:0]	;
	{0x32A2, 0xFF},//,LSSC_TOP_P2RG[7:0]  ;
	{0x32A3, 0xFF},//,LSSC_TOP_P2GRG[7:0]  ;
	{0x32A4, 0xFF},//,LSSC_TOP_P2GBG[7:0]  ;
	{0x32A5, 0xFF},//,LSSC_TOP_P2BG[7:0]  ;
	{0x32A6, 0xFF},//,LSSC_BOTTOM_P2RG[7:0]  ;
	{0x32A7, 0xFF},//,LSSC_BOTTOM_P2GRG[7:0]  ;
	{0x32A8, 0xFF},//,LSSC_BOTTOM_P2GBG[7:0]  ;
	{0x32A9, 0xFF},//,LSSC_BOTTOM_P2BG[7:0]  ;
	{0x32AA, 0x00},//,LSSC_LEFT_PM4RG[7:0]	;
	{0x32AB, 0x00},//,LSSC_LEFT_PM4GRG[7:0]  ;
	{0x32AC, 0x00},//,LSSC_LEFT_PM4GBG[7:0]  ;
	{0x32AD, 0x00},//,LSSC_LEFT_PM4BG[7:0]	;
	{0x32AE, 0x00},//,LSSC_RIGHT_PM4RG[7:0]  ;
	{0x32AF, 0x00},//,LSSC_RIGHT_PM4GRG[7:0]  ;
	{0x32B0, 0x00},//,LSSC_RIGHT_PM4GBG[7:0]  ;
	{0x32B1, 0x00},//,LSSC_RIGHT_PM4BG[7:0]  ;
	{0x32B2, 0x00},//,LSSC_TOP_PM4RG[7:0]  ;
	{0x32B3, 0x00},//,LSSC_TOP_PM4GRG[7:0]	;
	{0x32B4, 0x00},//,LSSC_TOP_PM4GBG[7:0]	;
	{0x32B5, 0x00},//,LSSC_TOP_PM4BG[7:0]  ;
	{0x32B6, 0x00},//,LSSC_BOTTOM_PM4RG[7:0]  ;
	{0x32B7, 0x00},//,LSSC_BOTTOM_PM4GRG[7:0]  ;
	{0x32B8, 0x00},//,LSSC_BOTTOM_PM4GBG[7:0]  ;
	{0x32B9, 0x00},//,LSSC_BOTTOM_PM4BG[7:0]  ;
	{0x32BA, 0x80},//,LSSC_MGSEL[1:0]/-/-/LSSC4BG_PMSW/LSSC4GBG_PMSW/LSSC4GRG_PMSW/LSSC4;
	{0x32BB, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x32BC, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x32BD, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x32BE, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x32C1, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x32C2, 0x89},//,-/-/-/-/-/-/-/- ;
	{0x32C4, 0x60},//,-/-/-/-/-/-/-/- ;
	{0x32C5, 0x7F},//,-/-/-/-/-/-/-/- ;
	{0x32C6, 0x7F},//,-/-/-/-/-/-/-/- ;
	{0x32C7, 0x01},//,-/-/-/-/-/ABPC_MAP_EN/-/- ;
	{0x32C8, 0x08},//,-/-/-/-/-/-/-/- ;
	{0x32C9, 0x08},//,-/-/-/-/-/-/-/- ;
	{0x32CA, 0x94},//,-/-/-/-/-/-/-/- ;
	{0x32D3, 0x21},//,-/-/-/-/-/-/-/- ;
	{0x32D4, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x32D5, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x32D6, 0x20},//,-/-/-/-/-/-/-/- ;
	{0x32D7, 0x30},//,-/-/-/-/-/-/-/- ;
	{0x32D8, 0x30},//,-/-/-/-/-/-/-/- ;
	{0x32D9, 0x3F},//,-/-/-/-/-/-/-/- ;
	{0x32DA, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x32DB, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x32DC, 0xC0},//,-/-/-/-/-/-/-/- ;
	{0x32DD, 0xF0},//,-/-/-/-/-/-/-/- ;
	{0x32DE, 0xF0},//,-/-/-/-/-/-/-/- ;
	{0x32DF, 0xFF},//,-/-/-/-/-/-/-/- ;
	{0x32E0, 0x02},//,-/-/-/-/-/-/-/- ;
	{0x32E2, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3300, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x3301, 0x06},//,-/-/-/-/-/-/-/- ;
	{0x3302, 0x06},//,-/-/-/-/-/-/-/- ;
	{0x3303, 0x35},//,-/-/-/-/-/-/-/- ;
	{0x3304, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3305, 0x87},//,-/-/-/-/-/-/-/- ;
	{0x3306, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3307, 0x33},//,-/-/-/-/-/-/-/- ;
	{0x3308, 0x0C},//,-/-/-/-/-/-/-/- ;
	{0x3309, 0x0C},//,-/-/-/-/-/-/-/- ;
	{0x330A, 0x20},//,-/-/-/-/-/-/-/- ;
	{0x330B, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x330C, 0x07},//,-/-/-/-/-/-/-/- ;
	{0x330E, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x330F, 0x0D},//,-/-/-/-/-/-/-/- ;
	{0x3310, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x3312, 0x0D},//,-/-/-/-/-/-/-/- ;
	{0x3314, 0x5C},//,-/-/-/-/-/-/-/- ;
	{0x3315, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3316, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3317, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3318, 0x18},//,-/-/-/-/-/-/-/- ;
	{0x3319, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x331A, 0x08},//,-/-/-/-/-/-/-/- ;
	{0x331B, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x331C, 0x2E},//,-/-/-/-/-/-/-/- ;
	{0x331D, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3320, 0x03},//,-/-/-/-/-/-/-/- ;
	{0x3321, 0x8B},//,-/-/-/-/-/-/-/- ;
	{0x3322, 0x6F},//,-/-/-/-/-/-/-/- ;
	{0x3323, 0x83},//,-/-/-/-/-/-/-/- ;
	{0x3324, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x3325, 0x3D},//,-/-/-/-/-/-/-/- ;
	{0x3326, 0x83},//,-/-/-/-/-/-/-/- ;
	{0x3327, 0x24},//,-/-/-/-/-/-/-/- ;
	{0x3328, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3329, 0xF7},//,-/-/-/-/-/-/-/- ;
	{0x332A, 0x0F},//,-/-/-/-/-/-/-/- ;
	{0x332B, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x332C, 0xE0},//,-/-/-/-/-/-/-/- ;
	{0x332D, 0x4C},//,-/-/-/-/-/-/-/- ;
	{0x332E, 0x3F},//,-/-/-/-/-/-/-/- ;
	{0x332F, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3330, 0x0B},//,-/-/-/-/-/-/-/- ;
	{0x3331, 0x1B},//,-/-/-/-/-/-/-/- ;
	{0x3332, 0x06},//,-/-/-/-/-/-/-/- ;
	{0x3333, 0x06},//,-/-/-/-/-/-/-/- ;
	{0x3334, 0x03},//,-/-/-/-/-/-/-/- ;
	{0x3335, 0x06},//,-/-/-/-/-/-/-/- ;
	{0x3336, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x3337, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x3338, 0x20},//,-/-/-/-/-/-/-/- ;
	{0x3339, 0x20},//,-/-/-/-/-/-/-/- ;
	{0x333A, 0x30},//,-/-/-/-/-/-/-/- ;
	{0x333B, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x333C, 0x30},//,-/-/-/-/-/-/-/- ;
	{0x333D, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x333E, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x333F, 0x1F},//,-/-/-/-/-/-/-/- ;
	{0x3340, 0x30},//,-/-/-/-/-/-/-/- ;
	{0x3341, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3342, 0x30},//,-/-/-/-/-/-/-/- ;
	{0x3343, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3344, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3350, 0x08},//,-/-/-/-/-/-/-/- ;
	{0x3351, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3352, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3353, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x3354, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x3355, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3356, 0x20},//,-/-/-/-/-/-/-/- ;
	{0x3357, 0x08},//,-/-/-/-/-/-/-/- ;
	{0x3358, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3359, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x335A, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x335B, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x335C, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x335D, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x335E, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x335F, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3360, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x3361, 0x03},//,-/-/-/-/-/-/-/- ;
	{0x3364, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3365, 0xAE},//,-/-/-/-/-/-/-/- ;
	{0x3366, 0x70},//,-/-/-/-/-/-/-/- ;
	{0x3367, 0x03},//,-/-/-/-/-/-/-/- ;
	{0x3368, 0x3B},//,-/-/-/-/-/-/-/- ;
	{0x3369, 0x3B},//,-/-/-/-/-/-/-/- ;
	{0x336A, 0x59},//,-/-/-/-/-/-/-/- ;
	{0x336B, 0x1D},//,-/-/-/-/-/-/-/- ;
	{0x336C, 0x3B},//,-/-/-/-/-/-/-/- ;
	{0x336D, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x336E, 0x21},//,-/-/-/-/-/-/-/- ;
	{0x336F, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x3370, 0x19},//,-/-/-/-/-/-/-/- ;
	{0x3371, 0x11},//,-/-/-/-/-/-/-/- ;
	{0x3372, 0x1E},//,-/-/-/-/-/-/-/- ;
	{0x3373, 0x2B},//,-/-/-/-/-/-/-/- ;
	{0x3374, 0x2E},//,-/-/-/-/-/-/-/- ;
	{0x3375, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x3376, 0x19},//,-/-/-/-/-/-/-/- ;
	{0x3377, 0x0F},//,-/-/-/-/-/-/-/- ;
	{0x3378, 0x5F},//,-/-/-/-/-/-/-/- ;
	{0x3379, 0x20},//,-/-/-/-/-/-/-/- ;
	{0x337A, 0x5E},//,-/-/-/-/-/-/-/- ;
	{0x337B, 0x0D},//,-/-/-/-/-/-/-/- ;
	{0x337C, 0x97},//,-/-/-/-/-/-/-/- ;
	{0x337D, 0x0D},//,-/-/-/-/-/-/-/- ;
	{0x337E, 0x03},//,-/-/-/-/-/-/-/- ;
	{0x337F, 0x71},//,-/-/-/-/-/-/-/- ;
	{0x3380, 0x54},//,-/-/-/-/-/-/-/- ;
	{0x3381, 0xB5},//,-/-/-/-/-/-/-/- ;
	{0x3382, 0x03},//,-/-/-/-/-/-/-/- ;
	{0x3383, 0x0D},//,-/-/-/-/-/-/-/- ;
	{0x3384, 0x03},//,-/-/-/-/-/-/-/- ;
	{0x3385, 0x38},//,-/-/-/-/-/-/-/- ;
	{0x3386, 0x3D},//,-/-/-/-/-/-/-/- ;
	{0x3387, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3388, 0x0B},//,-/-/-/-/-/-/-/- ;
	{0x3389, 0x0C},//,-/-/-/-/-/-/-/- ;
	{0x338A, 0x0B},//,-/-/-/-/-/-/-/- ;
	{0x338B, 0x0C},//,-/-/-/-/-/-/-/- ;
	{0x338C, 0x09},//,-/-/-/-/-/-/-/- ;
	{0x338D, 0x0C},//,-/-/-/-/-/-/-/- ;
	{0x338E, 0x09},//,-/-/-/-/-/-/-/- ;
	{0x338F, 0x0C},//,-/-/-/-/-/-/-/- ;
	{0x3390, 0x08},//,-/-/-/-/-/-/-/- ;
	{0x3391, 0x14},//,-/-/-/-/-/-/-/- ;
	{0x3392, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3393, 0x28},//,-/-/-/-/-/-/-/- ;
	{0x3394, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3397, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x3398, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x3399, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x339A, 0x1B},//,-/-/-/-/-/-/-/- ;
	{0x339B, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x339C, 0x3C},//,-/-/-/-/-/-/-/- ;
	{0x339D, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x339E, 0x1E},//,-/-/-/-/-/-/-/- ;
	{0x339F, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33A0, 0x78},//,-/-/-/-/-/-/-/- ;
	{0x33A1, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33A2, 0x3C},//,-/-/-/-/-/-/-/- ;
	{0x33A3, 0x06},//,-/-/-/-/-/-/-/- ;
	{0x33A4, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33A5, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33A6, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33A7, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33A8, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33A9, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33AA, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33AB, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33AE, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33AF, 0xFF},//,-/-/-/-/-/-/-/- ;
	{0x33B0, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33B1, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x33B2, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33B3, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33B4, 0xFF},//,-/-/-/-/-/-/-/- ;
	{0x33B5, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x33B6, 0x80},//,-/-/-/-/-/-/-/- ;
	{0x33B7, 0x60},//,-/-/-/-/-/-/-/- ;
	{0x33B9, 0x82},//,-/-/-/-/-/-/-/- ;
	{0x33BA, 0x06},//,-/-/-/-/-/-/-/- ;
	{0x33BD, 0x01},//,-/-/-/-/-/-/-/- ;
	{0x33BE, 0xA0},//,-/-/-/-/-/-/-/- ;
	{0x33BF, 0x20},//,-/-/-/-/-/-/-/- ;
	{0x33C0, 0xC7},//,-/-/-/-/-/-/-/- ;
	{0x33C1, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33C2, 0x10},//,-/-/-/-/-/-/-/- ;
	{0x33C3, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33C4, 0x03},//,-/-/-/-/-/-/-/- ;
	{0x33C5, 0x41},//,-/-/-/-/-/-/-/- ;
	{0x33C6, 0x18},//,-/-/-/-/-/-/-/- ;
	{0x33C7, 0x04},//,-/-/-/-/-/-/-/- ;
	{0x33C8, 0x08},//,-/-/-/-/-/-/-/- ;
	{0x33C9, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33CB, 0x00},//,-/-/-/-/-/-/-/- ;
	{0x33CE, 0x40},//,-/-/-/-/-/-/-/- ;
	{0x33CF, 0x66},//,-/-/-/-/-/-/-/- ;
	{0x3400, 0x00},//,OTP_STA[3:0]/OTP_CONT[3:0] ;
	{0x3402, 0x00},//,OTP_PSEL[7:0]  ;
	{0x3404, 0x00},//,OTP_DATA0[7:0]  ;
	{0x3405, 0x00},//,OTP_DATA1[7:0]  ;
	{0x3406, 0x00},//,OTP_DATA2[7:0]  ;
	{0x3407, 0x00},//,OTP_DATA3[7:0]  ;
	{0x3408, 0x00},//,OTP_DATA4[7:0]  ;
	{0x3409, 0x00},//,OTP_DATA5[7:0]  ;
	{0x340A, 0x00},//,OTP_DATA6[7:0]  ;
	{0x340B, 0x00},//,OTP_DATA7[7:0]  ;
	{0x340C, 0x00},//,OTP_DATA8[7:0]  ;
	{0x340D, 0x00},//,OTP_DATA9[7:0]  ;
	{0x340E, 0x00},//,OTP_DATA10[7:0]  ;
	{0x340F, 0x00},//,OTP_DATA11[7:0]  ;
	{0x3410, 0x00},//,OTP_DATA12[7:0]  ;
	{0x3411, 0x00},//,OTP_DATA13[7:0]  ;
	{0x3412, 0x00},//,OTP_DATA14[7:0]  ;
	{0x3413, 0x00},//,OTP_DATA15[7:0]  ;
	{0x3414, 0x00},//,OTP_DATA16[7:0]  ;
	{0x3415, 0x00},//,OTP_DATA17[7:0]  ;
	{0x3416, 0x00},//,OTP_DATA18[7:0]  ;
	{0x3417, 0x00},//,OTP_DATA19[7:0]  ;
	{0x3418, 0x00},//,OTP_DATA20[7:0]  ;
	{0x3419, 0x00},//,OTP_DATA21[7:0]  ;
	{0x341A, 0x00},//,OTP_DATA22[7:0]  ;
	{0x341B, 0x00},//,OTP_DATA23[7:0]  ;
	{0x341C, 0x00},//,OTP_DATA24[7:0]  ;
	{0x341D, 0x00},//,OTP_DATA25[7:0]  ;
	{0x341E, 0x00},//,OTP_DATA26[7:0]  ;
	{0x341F, 0x00},//,OTP_DATA27[7:0]  ;
	{0x3420, 0x00},//,OTP_DATA28[7:0]  ;
	{0x3421, 0x00},//,OTP_DATA29[7:0]  ;
	{0x3422, 0x00},//,OTP_DATA30[7:0]  ;
	{0x3423, 0x00},//,OTP_DATA31[7:0]  ;
	{0x3424, 0x00},//,OTP_DATA32[7:0]  ;
	{0x3425, 0x00},//,OTP_DATA33[7:0]  ;
	{0x3426, 0x00},//,OTP_DATA34[7:0]  ;
	{0x3427, 0x00},//,OTP_DATA35[7:0]  ;
	{0x3428, 0x00},//,OTP_DATA36[7:0]  ;
	{0x3429, 0x00},//,OTP_DATA37[7:0]  ;
	{0x342A, 0x00},//,OTP_DATA38[7:0]  ;
	{0x342B, 0x00},//,OTP_DATA39[7:0]  ;
	{0x342C, 0x00},//,OTP_DATA40[7:0]  ;
	{0x342D, 0x00},//,OTP_DATA41[7:0]  ;
	{0x342E, 0x00},//,OTP_DATA42[7:0]  ;
	{0x342F, 0x00},//,OTP_DATA43[7:0]  ;
	{0x3430, 0x00},//,OTP_DATA44[7:0]  ;
	{0x3431, 0x00},//,OTP_DATA45[7:0]  ;
	{0x3432, 0x00},//,OTP_DATA46[7:0]  ;
	{0x3433, 0x00},//,OTP_DATA47[7:0]  ;
	{0x3434, 0x00},//,OTP_DATA48[7:0]  ;
	{0x3435, 0x00},//,OTP_DATA49[7:0]  ;
	{0x3436, 0x00},//,OTP_DATA50[7:0]  ;
	{0x3437, 0x00},//,OTP_DATA51[7:0]  ;
	{0x3438, 0x00},//,OTP_DATA52[7:0]  ;
	{0x3439, 0x00},//,OTP_DATA53[7:0]  ;
	{0x343A, 0x00},//,OTP_DATA54[7:0]  ;
	{0x343B, 0x00},//,OTP_DATA55[7:0]  ;
	{0x343C, 0x00},//,OTP_DATA56[7:0]  ;
	{0x343D, 0x00},//,OTP_DATA57[7:0]  ;
	{0x343E, 0x00},//,OTP_DATA58[7:0]  ;
	{0x343F, 0x00},//,OTP_DATA59[7:0]  ;
	{0x3440, 0x00},//,OTP_DATA60[7:0]  ;
	{0x3441, 0x00},//,OTP_DATA61[7:0]  ;
	{0x3442, 0x00},//,OTP_DATA62[7:0]  ;
	{0x3443, 0x00},//,OTP_DATA63[7:0]  ;
	{0x0100, 0x01},//,-/-/-/-/-/-/-/MODE_SELECT  ;
};

static struct v4l2_subdev_info t8ev4_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array t8ev4_init_conf[] = {
	{&t8ev4_recommend_settings[0],
	ARRAY_SIZE(t8ev4_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array t8ev4_confs[] = {
	{&t8ev4_snap_settings[0],
	ARRAY_SIZE(t8ev4_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&t8ev4_prev_settings[0],
	ARRAY_SIZE(t8ev4_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t t8ev4_dimensions[] = {
	{
		.x_output = 0xCD0, // 3280
		.y_output = 0x9A0, // 2464
		.line_length_pclk = 0xD7C, 
		.frame_length_lines = 0x9C7, 
		.vt_pixel_clk = 129600000,
		.op_pixel_clk = 648000000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x660, // 1632
		.y_output = 0x4D0, // 1232
		.line_length_pclk = 0x0D7C,
		.frame_length_lines = 0x04F4,
		.vt_pixel_clk = 129600000,
		.op_pixel_clk = 648000000,
		.binning_factor = 1,
	},
};

static struct msm_camera_csi_params t8ev4_csic_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x14,
};

static struct msm_camera_csi_params *t8ev4_csic_params_array[] = {
	&t8ev4_csic_params,
	&t8ev4_csic_params,
};

static struct msm_camera_csid_vc_cfg t8ev4_cid_cfg[] = {
	{0, CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
	{2, CSI_RESERVED_DATA_0, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params t8ev4_csi_params = {
	.csid_params = {
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = ARRAY_SIZE(t8ev4_cid_cfg),
			.vc_cfg = t8ev4_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		.settle_cnt = 0x14,
	},
};

static struct msm_camera_csi2_params *t8ev4_csi_params_array[] = {
	&t8ev4_csi_params,
	&t8ev4_csi_params,
};

static struct msm_sensor_output_reg_addr_t t8ev4_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t t8ev4_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x1403,
};

static struct msm_sensor_exp_gain_info_t t8ev4_exp_gain_info = {
	.coarse_int_time_addr = 0x202,
	.global_gain_addr = 0x204,
	.vert_offset = 8,
};

static const struct i2c_device_id t8ev4_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&t8ev4_s_ctrl},
	{ }
};

static struct i2c_driver t8ev4_i2c_driver = {
	.id_table = t8ev4_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client t8ev4_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	printk("msm_sensor_init_module t8ev4 \n");
	return i2c_add_driver(&t8ev4_i2c_driver);
}

static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

static struct v4l2_subdev_core_ops t8ev4_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops t8ev4_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops t8ev4_subdev_ops = {
	.core = &t8ev4_subdev_core_ops,
	.video  = &t8ev4_subdev_video_ops,
};

int32_t t8ev4_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
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

int32_t t8ev4_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
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

static struct msm_sensor_fn_t t8ev4_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = msm_sensor_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = msm_sensor_write_exp_gain1,
	.sensor_setting = msm_sensor_setting,
	.sensor_csi_setting = msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = t8ev4_sensor_power_up,
	.sensor_power_down = t8ev4_sensor_power_down,
	.sensor_adjust_frame_lines = msm_sensor_adjust_frame_lines,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t t8ev4_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = t8ev4_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(t8ev4_start_settings),
	.stop_stream_conf = t8ev4_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(t8ev4_stop_settings),
	.group_hold_on_conf = t8ev4_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(t8ev4_groupon_settings),
	.group_hold_off_conf = t8ev4_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(t8ev4_groupoff_settings),
	.init_settings = &t8ev4_init_conf[0],
	.init_size = ARRAY_SIZE(t8ev4_init_conf),
	.mode_settings = &t8ev4_confs[0],
	.output_settings = &t8ev4_dimensions[0],
	.num_conf = ARRAY_SIZE(t8ev4_confs),
};

static struct msm_sensor_ctrl_t t8ev4_s_ctrl = {
	.msm_sensor_reg = &t8ev4_regs,
	.sensor_i2c_client = &t8ev4_sensor_i2c_client,
	.sensor_i2c_addr = 0x6E,
	.sensor_output_reg_addr = &t8ev4_reg_addr,
	.sensor_id_info = &t8ev4_id_info,
	.sensor_exp_gain_info = &t8ev4_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &t8ev4_csic_params_array[0],
	.csi_params = &t8ev4_csi_params_array[0],
	.msm_sensor_mutex = &t8ev4_mut,
	.sensor_i2c_driver = &t8ev4_i2c_driver,
	.sensor_v4l2_subdev_info = t8ev4_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(t8ev4_subdev_info),
	.sensor_v4l2_subdev_ops = &t8ev4_subdev_ops,
	.func_tbl = &t8ev4_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Toshiba 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
