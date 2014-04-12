/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 * Copyright(C) 2013 Foxconn International Holdings, Ltd. All rights.
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
#define SENSOR_NAME "s5k4e1"
#define PLATFORM_DRIVER_NAME "msm_camera_s5k4e1"
#define MSB                             1
#define LSB                             0

DEFINE_MUTEX(s5k4e1_mut);
static struct msm_sensor_ctrl_t s5k4e1_s_ctrl;

static struct msm_camera_i2c_reg_conf s5k4e1_start_settings[] = {
	{0x0100, 0x01},
    //{0x0601, 0x05},//Test patten 0: black, 5: white
};

static struct msm_camera_i2c_reg_conf s5k4e1_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k4e1_groupon_settings[] = {
	{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k4e1_groupoff_settings[] = {
	{0x0104, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k4e1_prev_settings[] = {
    {0x0100, 0x00},

    /* [20120321][4E1EVT3][MIPI2lane][Mclk=24Mhz-612Mbps][1304x980][30fps][non continuous] */
    // Integration setting ...
    /*Timing configuration*/
    {0x0202, 0x01}, //coarse_integration_time_H
    {0x0203, 0x11}, //coarse_integration_time_L
    {0x0204, 0x02}, //analogue_gain_code_global_H
    {0x0205, 0x00}, //analogue_gain_code_global_L
    //Frame Length
    {0x0340, 0x04},
    {0x0341, 0xC1},
    //Line Length
    {0x0342, 0x0D}, //3352
    {0x0343, 0x18},

    // PLL setting ...
    //// input clock 24MHz
    ////// (3) MIPI 1-lane Serial(TST = 0000b or TST = 0010b), 30 fps
    {0x0305, 0x04},
    {0x0306, 0x00},
    {0x0307, 0x66},
    {0x30B5, 0x01},
    {0x30E2, 0x02}, //num lanes[1:0] = 2
    {0x30F1, 0xA0},
    {0x30e8, 0x07}, //default =0f continuous ; 07= non-continuous

    // MIPI Size Setting ...
    // 1304 x 980
    {0x30A9, 0x02}, //Horizontal Binning On
    {0x300E, 0xEB}, //Vertical Binning On
    {0x0387, 0x03}, //y_odd_inc 03(10b AVG)
    {0x0344, 0x00}, //x_addr_start 0
    {0x0345, 0x00},
    {0x0348, 0x0A}, //x_addr_end 2607
    {0x0349, 0x2F},
    {0x0346, 0x00}, //y_addr_start 0
    {0x0347, 0x00},
    {0x034A, 0x07}, //y_addr_end 1959
    {0x034B, 0xA7},
    {0x0380, 0x00}, //x_even_inc 1
    {0x0381, 0x01},
    {0x0382, 0x00}, //x_odd_inc 1
    {0x0383, 0x01},
    {0x0384, 0x00}, //y_even_inc 1
    {0x0385, 0x01},
    {0x0386, 0x00}, //y_odd_inc 3
    {0x0387, 0x03},
    {0x034C, 0x05}, //x_output_size 1304
    {0x034D, 0x18},
    {0x034E, 0x03}, //y_output_size 980
    {0x034F, 0xd4},
    {0x30BF, 0xAB}, //outif_enable[7], data_type[5:0](2Bh = bayer 10bit)
    {0x30C0, 0xA0}, //video_offset[7:4] 3260%12
    {0x30C8, 0x06}, //video_data_length 1600 = 1304 * 1.25
    {0x30C9, 0x5E},
    {0x301B, 0x83}, //full size =77 , binning size = 83 greg0315 
    {0x3017, 0x84}, //full size =94 , binning size = 84 greg0315

    //MM-MC-UpdateTuningForColorShadingIn20130222-00+{
    {0x3098, 0x3e},     //sh4ch_blk_height = 123
    {0x309b, 0x04},     //sh4ch_step_y msb (sh4ch_step_y = 533)
    {0x309c, 0x21},     //sh4ch_step_y lsb
    //MM-MC-UpdateTuningForColorShadingIn20130222-00++}
    //{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k4e1_snap_settings[] = {
    {0x0100, 0x00},
        
    /*[20120314][4E1EVT3][MIPI2lane][Mclk=24Mhz-440Mbps][2608x1960][15fps][non continuous]*/
    // Integration setting ... 
    /*Timing configuration*/
    {0x0202, 0x04},
    {0x0203, 0x12},
    {0x0204, 0x00},
    {0x0205, 0x80},
    // Frame Length
    {0x0340, 0x07},
    {0x0341, 0xef},
    // Line Length
    {0x0342, 0x0A}, //2738
    {0x0343, 0xF0},

    // PLL setting ...
    //// input clock 24MHz
    ////// (3) MIPI 1-lane Serial(TST = 0000b or TST = 0010b), 15 fps
    {0x0305, 0x04},
    {0x0306, 0x00},
    {0x0307, 0x49}, //66:612MHz, 46:440MHz
    {0x30B5, 0x01},
    {0x30E2, 0x02}, //num lanes[1:0] = 2
    {0x30F1, 0xA0},
    {0x30e8, 0x07}, //default =0f continuous ; 07= non-continuous

    // MIPI Size Setting
    //2608 x 1960
    {0x30A9, 0x01}, //03greg0315 //Horizontal Binning Off 
    {0x300E, 0xE9}, //E8greg0315  //Vertical Binning Off  
    {0x0387, 0x01}, //y_odd_inc
    {0x0344, 0x00}, //x_addr_start 0
    {0x0345, 0x00},
    {0x0348, 0x0A}, //x_addr_end 2607
    {0x0349, 0x2F},
    {0x0346, 0x00}, //y_addr_start 0
    {0x0347, 0x00},
    {0x034A, 0x07}, //y_addr_end 1959
    {0x034B, 0xA7},
    {0x0380, 0x00}, //x_even_inc 1
    {0x0381, 0x01},
    {0x0382, 0x00}, //x_odd_inc 1
    {0x0383, 0x01},
    {0x0384, 0x00}, //y_even_inc 1
    {0x0385, 0x01},
    {0x0386, 0x00}, //y_odd_inc 1
    {0x0387, 0x01},
    {0x034C, 0x0A}, //x_output_size 2608
    {0x034D, 0x30},
    {0x034E, 0x07}, //y_output_size 1960
    {0x034F, 0xA8},
    {0x30BF, 0xAB}, //outif_enable[7], data_type[5:0](2Bh = bayer 10bit)
    {0x30C0, 0x80}, //video_offset[7:4] 3260%12
    {0x30C8, 0x0C}, //video_data_length 3260 = 2608 * 1.25
    {0x30C9, 0xBC},
    {0x301B, 0x77}, //full size =77 , binning size = 83 greg0315 
    {0x3017, 0x94}, //full size =94 , binning size = 84 greg0315 

    //MM-MC-UpdateTuningForColorShadingIn20130222-00+{
    {0x3098, 0x7B},     //sh4ch_blk_height = 123
    {0x309b, 0x02},     //sh4ch_step_y msb (sh4ch_step_y = 533)
    {0x309c, 0x15},     //sh4ch_step_y lsb
    //MM-MC-UpdateTuningForColorShadingIn20130222-00+}
    //{0x0100, 0x01},
};

/* MM-MC-ImplementRegSwitchMechanismForShading-00+{ */
static struct msm_camera_i2c_reg_conf s5k4e1_recommend_settings1[] = {
    {0x0100, 0x00},
    {0x3030, 0x06},
    
    /* [4E1EVT3][GlobalV1][MIPI2lane][Mclk=24Mhz] */
    /* Reset setting */
    //{0x0103, 0x01},

    // Analog Setting
    //// CDS timing setting ...                                    
    {0x3000, 0x05},
    {0x3001, 0x03},
    {0x3002, 0x08},
    {0x3003, 0x09},
    {0x3004, 0x2E},
    {0x3005, 0x06},
    {0x3006, 0x34},
    {0x3007, 0x00},
    {0x3008, 0x3C},
    {0x3009, 0x3C},
    {0x300A, 0x28},
    {0x300B, 0x04},
    {0x300C, 0x0A},
    {0x300D, 0x02},
    {0x300F, 0x82},
    {0x3010, 0x00},
    {0x3011, 0x4C},
    {0x3012, 0x30},
    {0x3013, 0xC0},
    {0x3014, 0x00},
    {0x3015, 0x00},
    {0x3016, 0x2C},
    {0x3017, 0x94},
    {0x3018, 0x78},
    //{0x301B, 0x77},	 //full size =77 , binning size = 83
    {0x301C, 0x04},
    {0x301D, 0xD4},
    {0x3021, 0x02},
    {0x3022, 0x24},
    {0x3024, 0x40},
    {0x3027, 0x08},
    {0x3029, 0xC6},
    {0x30BC, 0x98},
    {0x302B, 0x01},
    {0x30D8, 0x3F},

    // ADLC setting ...
    {0x3070, 0x5F},
    {0x3071, 0x00},
    {0x3080, 0x04},
    {0x3081, 0x38},

    // MIPI setting
    {0x30BD, 0x00}, //SEL_CCP[0]
    {0x3084, 0x15}, //SYNC Mode
    {0x30BE, 0x1A}, //M_PCLKDIV_AUTO[4], M_DIV_PCLK[3:0]
    {0x30C1, 0x01}, //pack video enable [0]
    {0x3111, 0x86}, //Embedded data off [5]

    //For MIPI T8 T9 add by greg0809 
    {0x30E3, 0x38}, //outif_mld_ulpm_rxinit_limit[15:8]                                          
    {0x30E4, 0x40}, //outif_mld_ulpm_rxinit_limit[7:0]                                           
    {0x3113, 0x70}, //outif_enable_time[15:8]                                                    
    {0x3114, 0x80}, //outif_enable_time[7:0]                                                     
    {0x3115, 0x7B}, //streaming_enalbe_time[15:8]                                                
    {0x3116, 0xC0}, //streaming_enalbe_time[7:0]                                                 
    {0x30EE, 0x12}, //[5:4]esc_ref_div, [3] dphy_ulps_auto, [1]dphy_enable  

    /* MM-MC-UpdateTuningForColorShadingIn20130411-00*{ */
    /* golden */
    {0x3096, 0x40},
 
    {0x3097, 0x52}, //sh4ch_blk_width = 82
    {0x3098, 0x3e}, //sh4ch_blk_height = 123
    {0x3099, 0x03}, //sh4ch_step_x msb (sh4ch_step_x = 799)
    {0x309a, 0x1f}, //sh4ch_step_x lsb
    {0x309b, 0x04}, //sh4ch_step_y msb (sh4ch_step_y = 533)
    {0x309c, 0x21}, //sh4ch_step_y lsb
    {0x309d, 0x00}, //sh4ch_start_blk_cnt_x = 0
    {0x309e, 0x00}, //sh4ch_start_int_cnt_x = 0
    {0x309f, 0x00}, //sh4ch_start_frac_cnt_x msb (sh4ch_start_frac_cnt_x = 0)
    {0x30a0, 0x00}, //sh4ch_start_frac_cnt_x lsb
    {0x30a1, 0x00}, //sh4ch_start_blk_cnt_y = 0
    {0x30a2, 0x00}, //sh4ch_start_int_cnt_y = 0
    {0x30a3, 0x00}, //sh4ch_start_frac_cnt_y msb (sh4ch_start_frac_cnt_x = 0)
    {0x30a4, 0x00}, //sh4ch_start_frac_cnt_y lsb
     
    {0x30a5, 0x01},
    {0x30a6, 0x00}, // gs_pedestal	= 64
     
    {0x3200, 0x00},
	/* MM-UW-UpdateTuningForColorShading-00*{ */
    {0x3201, 0x79},
    {0x3202, 0x8a},
    {0x3203, 0x0f},
    {0x3204, 0xf0},
    {0x3205, 0x28},
    {0x3206, 0x0f},
    {0x3207, 0xf8},
    {0x3208, 0xbb},
    {0x3209, 0x00},
    {0x320a, 0x1c},
    {0x320b, 0x6a},
    {0x320c, 0x0f},
    {0x320d, 0xec},
    {0x320e, 0xa4},
    {0x320f, 0x0f},
    {0x3210, 0xff},
    {0x3211, 0xc4},
    {0x3212, 0x0f},
    {0x3213, 0xe7},
    {0x3214, 0x34},
    {0x3215, 0x0f},
    {0x3216, 0xf8},
    {0x3217, 0x9c},
    {0x3218, 0x00},
    {0x3219, 0x06},
    {0x321a, 0x08},
    {0x321b, 0x0f},
    {0x321c, 0xf9},
    {0x321d, 0x09},
    {0x321e, 0x00},
    {0x321f, 0x0a},
    {0x3220, 0x99},
    {0x3221, 0x0f},
    {0x3222, 0xf9},
    {0x3223, 0x2b},
    {0x3224, 0x00},
    {0x3225, 0x05},
    {0x3226, 0x53},
    {0x3227, 0x00},
    {0x3228, 0x01},
    {0x3229, 0x39},
    {0x322a, 0x00},
    {0x322b, 0x0f},
    {0x322c, 0x21},
    {0x322d, 0x0f},
    {0x322e, 0xec},
    {0x322f, 0xa3},
    {0x3230, 0x00},
    {0x3231, 0x00},
    {0x3232, 0xaa},
    {0x3233, 0x00},
    {0x3234, 0x09},
    {0x3235, 0xf5},
    {0x3236, 0x00},
    {0x3237, 0x10},
    {0x3238, 0xcc},
    {0x3239, 0x00},
    {0x323a, 0x00},
    {0x323b, 0xd5},
    {0x323c, 0x0f},
    {0x323d, 0xe6},
    {0x323e, 0x9f},
    {0x323f, 0x00},
    {0x3240, 0x1d},
    {0x3241, 0x06},
    {0x3242, 0x00},
    {0x3243, 0x02},
    {0x3244, 0xd8},
    {0x3245, 0x0f},
    {0x3246, 0xf1},
    {0x3247, 0x74},
    {0x3248, 0x0f},
    {0x3249, 0xea},
    {0x324a, 0xf3},
    {0x324b, 0x00},
    {0x324c, 0x0c},
    {0x324d, 0xf5},
    {0x324e, 0x00},
    {0x324f, 0x03},
    {0x3250, 0xaf},
    {0x3251, 0x0f},
    {0x3252, 0xf8},
    {0x3253, 0x67},
    {0x3254, 0x0f},
    {0x3255, 0xfc},
    {0x3256, 0x44},
    {0x3257, 0x0f},
    {0x3258, 0xf7},
    {0x3259, 0x96},
    {0x325a, 0x00},
    {0x325b, 0x10},
    {0x325c, 0xe9},
    {0x325d, 0x0f},
    {0x325e, 0xe9},
    {0x325f, 0xc8},
    {0x3260, 0x00},
    {0x3261, 0x15},
    {0x3262, 0x40},
    {0x3263, 0x0f},
    {0x3264, 0xf0},
    {0x3265, 0xce},
    {0x3266, 0x0f},
    {0x3267, 0xfa},
    {0x3268, 0xe3},
    {0x3269, 0x00},
    {0x326a, 0x20},
    {0x326b, 0xee},
    {0x326c, 0x00},
    {0x326d, 0x7c},
    {0x326e, 0x53},
    {0x326f, 0x0f},
    {0x3270, 0xee},
    {0x3271, 0x4b},
    {0x3272, 0x0f},
    {0x3273, 0xfd},
    {0x3274, 0x7e},
    {0x3275, 0x00},
    {0x3276, 0x16},
    {0x3277, 0xcd},
    {0x3278, 0x0f},
    {0x3279, 0xef},
    {0x327a, 0x21},
    {0x327b, 0x00},
    {0x327c, 0x00},
    {0x327d, 0x9b},
    {0x327e, 0x0f},
    {0x327f, 0xea},
    {0x3280, 0x1b},
    {0x3281, 0x0f},
    {0x3282, 0xf5},
    {0x3283, 0x36},
    {0x3284, 0x00},
    {0x3285, 0x01},
    {0x3286, 0x26},
    {0x3287, 0x0f},
    {0x3288, 0xfb},
    {0x3289, 0x7e},
    {0x328a, 0x00},
    {0x328b, 0x18},
    {0x328c, 0xf2},
    {0x328d, 0x0f},
    {0x328e, 0xe8},
    {0x328f, 0x45},
    {0x3290, 0x0f},
    {0x3291, 0xff},
    {0x3292, 0xd2},
    {0x3293, 0x00},
    {0x3294, 0x04},
    {0x3295, 0x6a},
    {0x3296, 0x00},
    {0x3297, 0x1d},
    {0x3298, 0xbf},
    {0x3299, 0x0f},
    {0x329a, 0xe2},
    {0x329b, 0xe4},
    {0x329c, 0x0f},
    {0x329d, 0xe7},
    {0x329e, 0x28},
    {0x329f, 0x00},
    {0x32a0, 0x28},
    {0x32a1, 0xd7},
    {0x32a2, 0x00},
    {0x32a3, 0x16},
    {0x32a4, 0x95},
    {0x32a5, 0x00},
    {0x32a6, 0x00},
    {0x32a7, 0xba},
    {0x32a8, 0x0f},
    {0x32a9, 0xce},
    {0x32aa, 0x50},
    {0x32ab, 0x00},
    {0x32ac, 0x33},
    {0x32ad, 0x27},
    {0x32ae, 0x00},
    {0x32af, 0x0d},
    {0x32b0, 0x5c},
    {0x32b1, 0x0f},
    {0x32b2, 0xe3},
    {0x32b3, 0xf9},
    {0x32b4, 0x0f},
    {0x32b5, 0xe9},
    {0x32b6, 0xa0},
    {0x32b7, 0x00},
    {0x32b8, 0x0c},
    {0x32b9, 0x58},
    {0x32ba, 0x00},
    {0x32bb, 0x0c},
    {0x32bc, 0x6c},
    {0x32bd, 0x0f},
    {0x32be, 0xf4},
    {0x32bf, 0x3e},
    {0x32c0, 0x00},
    {0x32c1, 0x04},
    {0x32c2, 0x6f},
    {0x32c3, 0x0f},
    {0x32c4, 0xdb},
    {0x32c5, 0x6c},
    {0x32c6, 0x00},
    {0x32c7, 0x0f},
    {0x32c8, 0x0e},
    {0x32c9, 0x0f},
    {0x32ca, 0xe8},
    {0x32cb, 0x86},
    {0x32cc, 0x00},
    {0x32cd, 0x23},
    {0x32ce, 0x41},
    {0x32cf, 0x0f},
    {0x32d0, 0xd8},
    {0x32d1, 0xd2},
    {0x32d2, 0x0f},
    {0x32d3, 0xf8},
    {0x32d4, 0x4a},
    {0x32d5, 0x00},
    {0x32d6, 0x43},
    {0x32d7, 0xe9},
    {0x32d8, 0x00},
    {0x32d9, 0x6b},
    {0x32da, 0x50},
    {0x32db, 0x0f},
    {0x32dc, 0xf0},
    {0x32dd, 0xd7},
    {0x32de, 0x00},
    {0x32df, 0x01},
    {0x32e0, 0x2f},
    {0x32e1, 0x00},
    {0x32e2, 0x09},
    {0x32e3, 0xee},
    {0x32e4, 0x00},
    {0x32e5, 0x02},
    {0x32e6, 0x4b},
    {0x32e7, 0x0f},
    {0x32e8, 0xf3},
    {0x32e9, 0x5e},
    {0x32ea, 0x0f},
    {0x32eb, 0xed},
    {0x32ec, 0x49},
    {0x32ed, 0x0f},
    {0x32ee, 0xfa},
    {0x32ef, 0xb8},
    {0x32f0, 0x0f},
    {0x32f1, 0xfd},
    {0x32f2, 0x53},
    {0x32f3, 0x00},
    {0x32f4, 0x05},
    {0x32f5, 0x92},
    {0x32f6, 0x00},
    {0x32f7, 0x04},
    {0x32f8, 0x99},
    {0x32f9, 0x0f},
    {0x32fa, 0xf7},
    {0x32fb, 0x34},
    {0x32fc, 0x00},
    {0x32fd, 0x02},
    {0x32fe, 0xe5},
    {0x32ff, 0x00},
    {0x3300, 0x06},
    {0x3301, 0x60},
    {0x3302, 0x00},
    {0x3303, 0x0e},
    {0x3304, 0xc2},
    {0x3305, 0x0f},
    {0x3306, 0xed},
    {0x3307, 0x87},
    {0x3308, 0x0f},
    {0x3309, 0xf2},
    {0x330a, 0x83},
    {0x330b, 0x00},
    {0x330c, 0x1b},
    {0x330d, 0x7e},
    {0x330e, 0x00},
    {0x330f, 0x11},
    {0x3310, 0xbb},
    {0x3311, 0x0f},
    {0x3312, 0xf8},
    {0x3313, 0xb9},
    {0x3314, 0x0f},
    {0x3315, 0xe8},
    {0x3316, 0xa7},
    {0x3317, 0x00},
    {0x3318, 0x1c},
    {0x3319, 0xfa},
    {0x331a, 0x00},
    {0x331b, 0x0b},
    {0x331c, 0xe0},
    {0x331d, 0x0f},
    {0x331e, 0xe4},
    {0x331f, 0x3c},
    {0x3320, 0x0f},
    {0x3321, 0xeb},
    {0x3322, 0xfc},
    {0x3323, 0x00},
    {0x3324, 0x0f},
    {0x3325, 0x20},
    {0x3326, 0x0f},
    {0x3327, 0xff},
    {0x3328, 0x05},
    {0x3329, 0x0f},
    {0x332a, 0xfd},
    {0x332b, 0xe4},
    {0x332c, 0x0f},
    {0x332d, 0xf7},
    {0x332e, 0xb2},
    {0x332f, 0x0f},
    {0x3330, 0xfc},
    {0x3331, 0xe0},
    {0x3332, 0x00},
    {0x3333, 0x0d},
    {0x3334, 0x80},
    {0x3335, 0x0f},
    {0x3336, 0xec},
    {0x3337, 0xf2},
    {0x3338, 0x00},
    {0x3339, 0x1c},
    {0x333a, 0xea},
    {0x333b, 0x0f},
    {0x333c, 0xe1},
    {0x333d, 0xde},
    {0x333e, 0x00},
    {0x333f, 0x06},
    {0x3340, 0x3a},
    {0x3341, 0x00},
    {0x3342, 0x1b},
    {0x3343, 0x22},
    {0x3344, 0x00},
    {0x3345, 0x78},
    {0x3346, 0x21},
    {0x3347, 0x0f},
    {0x3348, 0xec},
    {0x3349, 0xd7},
    {0x334a, 0x00},
    {0x334b, 0x02},
    {0x334c, 0xa0},
    {0x334d, 0x00},
    {0x334e, 0x0b},
    {0x334f, 0xad},
    {0x3350, 0x00},
    {0x3351, 0x01},
    {0x3352, 0xb4},
    {0x3353, 0x0f},
    {0x3354, 0xf2},
    {0x3355, 0x14},
    {0x3356, 0x0f},
    {0x3357, 0xe5},
    {0x3358, 0xe4},
    {0x3359, 0x0f},
    {0x335a, 0xf8},
    {0x335b, 0x35},
    {0x335c, 0x00},
    {0x335d, 0x0b},
    {0x335e, 0x2d},
    {0x335f, 0x0f},
    {0x3360, 0xf1},
    {0x3361, 0x7c},
    {0x3362, 0x00},
    {0x3363, 0x0f},
    {0x3364, 0x1a},
    {0x3365, 0x0f},
    {0x3366, 0xf8},
    {0x3367, 0xb8},
    {0x3368, 0x00},
    {0x3369, 0x07},
    {0x336a, 0xe5},
    {0x336b, 0x00},
    {0x336c, 0x04},
    {0x336d, 0xfe},
    {0x336e, 0x0f},
    {0x336f, 0xff},
    {0x3370, 0xe4},
    {0x3371, 0x00},
    {0x3372, 0x01},
    {0x3373, 0xdb},
    {0x3374, 0x0f},
    {0x3375, 0xf0},
    {0x3376, 0x10},
    {0x3377, 0x00},
    {0x3378, 0x11},
    {0x3379, 0x8d},
    {0x337a, 0x00},
    {0x337b, 0x0f},
    {0x337c, 0xa3},
    {0x337d, 0x0f},
    {0x337e, 0xfd},
    {0x337f, 0x1e},
    {0x3380, 0x0f},
    {0x3381, 0xee},
    {0x3382, 0xf0},
    {0x3383, 0x00},
    {0x3384, 0x16},
    {0x3385, 0xc6},
    {0x3386, 0x00},
    {0x3387, 0x00},
    {0x3388, 0xc9},
    {0x3389, 0x0f},
    {0x338a, 0xf3},
    {0x338b, 0x2c},
    {0x338c, 0x0f},
    {0x338d, 0xec},
    {0x338e, 0xf7},
    {0x338f, 0x00},
    {0x3390, 0x0e},
    {0x3391, 0xf7},
    {0x3392, 0x00},
    {0x3393, 0x04},
    {0x3394, 0x82},
    {0x3395, 0x0f},
    {0x3396, 0xf2},
    {0x3397, 0x60},
    {0x3398, 0x00},
    {0x3399, 0x05},
    {0x339a, 0x69},
    {0x339b, 0x0f},
    {0x339c, 0xf6},
    {0x339d, 0x77},
    {0x339e, 0x00},
    {0x339f, 0x0e},
    {0x33a0, 0xad},
    {0x33a1, 0x0f},
    {0x33a2, 0xe7},
    {0x33a3, 0xec},
    {0x33a4, 0x00},
    {0x33a5, 0x15},
    {0x33a6, 0xe8},
    {0x33a7, 0x0f},
    {0x33a8, 0xee},
    {0x33a9, 0xb1},
    {0x33aa, 0x00},
    {0x33ab, 0x02},
    {0x33ac, 0x30},
    {0x33ad, 0x00},
    {0x33ae, 0x16},
    {0x33af, 0xae},
    /* MM-UW-UpdateTuningForColorShading-00*} */               
    {0x3096, 0x60},
    {0x3096, 0x40},
    /* MM-MC-UpdateTuningForColorShadingIn20130411-00*} */
};

static struct msm_camera_i2c_reg_conf s5k4e1_recommend_settings2[] = {
    {0x0100, 0x00},
    {0x3030, 0x06},
    
    /* [4E1EVT3][GlobalV1][MIPI2lane][Mclk=24Mhz] */
    /* Reset setting */
    //{0x0103, 0x01},

    // Analog Setting
    //// CDS timing setting ...                                    
    {0x3000, 0x05},
    {0x3001, 0x03},
    {0x3002, 0x08},
    {0x3003, 0x09},
    {0x3004, 0x2E},
    {0x3005, 0x06},
    {0x3006, 0x34},
    {0x3007, 0x00},
    {0x3008, 0x3C},
    {0x3009, 0x3C},
    {0x300A, 0x28},
    {0x300B, 0x04},
    {0x300C, 0x0A},
    {0x300D, 0x02},
    {0x300F, 0x82},
    {0x3010, 0x00},
    {0x3011, 0x4C},
    {0x3012, 0x30},
    {0x3013, 0xC0},
    {0x3014, 0x00},
    {0x3015, 0x00},
    {0x3016, 0x2C},
    {0x3017, 0x94},
    {0x3018, 0x78},
    //{0x301B, 0x77},	 //full size =77 , binning size = 83
    {0x301C, 0x04},
    {0x301D, 0xD4},
    {0x3021, 0x02},
    {0x3022, 0x24},
    {0x3024, 0x40},
    {0x3027, 0x08},
    {0x3029, 0xC6},
    {0x30BC, 0x98},
    {0x302B, 0x01},
    {0x30D8, 0x3F},

    // ADLC setting ...
    {0x3070, 0x5F},
    {0x3071, 0x00},
    {0x3080, 0x04},
    {0x3081, 0x38},

    // MIPI setting
    {0x30BD, 0x00}, //SEL_CCP[0]
    {0x3084, 0x15}, //SYNC Mode
    {0x30BE, 0x1A}, //M_PCLKDIV_AUTO[4], M_DIV_PCLK[3:0]
    {0x30C1, 0x01}, //pack video enable [0]
    {0x3111, 0x86}, //Embedded data off [5]

    //For MIPI T8 T9 add by greg0809 
    {0x30E3, 0x38}, //outif_mld_ulpm_rxinit_limit[15:8]                                          
    {0x30E4, 0x40}, //outif_mld_ulpm_rxinit_limit[7:0]                                           
    {0x3113, 0x70}, //outif_enable_time[15:8]                                                    
    {0x3114, 0x80}, //outif_enable_time[7:0]                                                     
    {0x3115, 0x7B}, //streaming_enalbe_time[15:8]                                                
    {0x3116, 0xC0}, //streaming_enalbe_time[7:0]                                                 
    {0x30EE, 0x12}, //[5:4]esc_ref_div, [3] dphy_ulps_auto, [1]dphy_enable  

    /* MM-MC-UpdateTuningForColorShadingIn20130411-00*{ */
    /* corner_60% */
    {0x3096, 0x40},
 
    {0x3097, 0x52}, //sh4ch_blk_width = 82
    {0x3098, 0x3e}, //sh4ch_blk_height = 123
    {0x3099, 0x03}, //sh4ch_step_x msb (sh4ch_step_x = 799)
    {0x309a, 0x1f}, //sh4ch_step_x lsb
    {0x309b, 0x04}, //sh4ch_step_y msb (sh4ch_step_y = 533)
    {0x309c, 0x21}, //sh4ch_step_y lsb
    {0x309d, 0x00}, //sh4ch_start_blk_cnt_x = 0
    {0x309e, 0x00}, //sh4ch_start_int_cnt_x = 0
    {0x309f, 0x00}, //sh4ch_start_frac_cnt_x msb (sh4ch_start_frac_cnt_x = 0)
    {0x30a0, 0x00}, //sh4ch_start_frac_cnt_x lsb
    {0x30a1, 0x00}, //sh4ch_start_blk_cnt_y = 0
    {0x30a2, 0x00}, //sh4ch_start_int_cnt_y = 0
    {0x30a3, 0x00}, //sh4ch_start_frac_cnt_y msb (sh4ch_start_frac_cnt_x = 0)
    {0x30a4, 0x00}, //sh4ch_start_frac_cnt_y lsb
     
    {0x30a5, 0x01},
    {0x30a6, 0x00}, // gs_pedestal	= 64
     
    {0x3200, 0x00},
	/* MM-UW-UpdateTuningForColorShading-00*{ */  
    {0x3201, 0x75},
    {0x3202, 0x2f},
    {0x3203, 0x0f},
    {0x3204, 0xf1},
    {0x3205, 0xe9},
    {0x3206, 0x00},
    {0x3207, 0x01},
    {0x3208, 0x04},
    {0x3209, 0x00},
    {0x320a, 0x09},
    {0x320b, 0x06},
    {0x320c, 0x0f},
    {0x320d, 0xfc},
    {0x320e, 0xc1},
    {0x320f, 0x0f},
    {0x3210, 0xff},
    {0x3211, 0x89},
    {0x3212, 0x0f},
    {0x3213, 0xef},
    {0x3214, 0x07},
    {0x3215, 0x0f},
    {0x3216, 0xf0},
    {0x3217, 0x96},
    {0x3218, 0x00},
    {0x3219, 0x0c},
    {0x321a, 0xa3},
    {0x321b, 0x0f},
    {0x321c, 0xfb},
    {0x321d, 0x3b},
    {0x321e, 0x0f},
    {0x321f, 0xff},
    {0x3220, 0x33},
    {0x3221, 0x00},
    {0x3222, 0x01},
    {0x3223, 0x97},
    {0x3224, 0x0f},
    {0x3225, 0xfb},
    {0x3226, 0x0a},
    {0x3227, 0x00},
    {0x3228, 0x0a},
    {0x3229, 0x8c},
    {0x322a, 0x0f},
    {0x322b, 0xff},
    {0x322c, 0x6a},
    {0x322d, 0x0f},
    {0x322e, 0xf7},
    {0x322f, 0x19},
    {0x3230, 0x00},
    {0x3231, 0x07},
    {0x3232, 0x60},
    {0x3233, 0x0f},
    {0x3234, 0xfe},
    {0x3235, 0x47},
    {0x3236, 0x00},
    {0x3237, 0x14},
    {0x3238, 0xe1},
    {0x3239, 0x00},
    {0x323a, 0x01},
    {0x323b, 0x86},
    {0x323c, 0x0f},
    {0x323d, 0xf7},
    {0x323e, 0x6c},
    {0x323f, 0x00},
    {0x3240, 0x08},
    {0x3241, 0xac},
    {0x3242, 0x00},
    {0x3243, 0x01},
    {0x3244, 0x07},
    {0x3245, 0x0f},
    {0x3246, 0xfc},
    {0x3247, 0xc1},
    {0x3248, 0x0f},
    {0x3249, 0xf4},
    {0x324a, 0x61},
    {0x324b, 0x0f},
    {0x324c, 0xf5},
    {0x324d, 0x49},
    {0x324e, 0x00},
    {0x324f, 0x07},
    {0x3250, 0x1d},
    {0x3251, 0x00},
    {0x3252, 0x02},
    {0x3253, 0xd1},
    {0x3254, 0x0f},
    {0x3255, 0xf8},
    {0x3256, 0xaa},
    {0x3257, 0x0f},
    {0x3258, 0xfe},
    {0x3259, 0x58},
    {0x325a, 0x00},
    {0x325b, 0x00},
    {0x325c, 0x5b},
    {0x325d, 0x00},
    {0x325e, 0x09},
    {0x325f, 0xcc},
    {0x3260, 0x0f},
    {0x3261, 0xfd},
    {0x3262, 0xa0},
    {0x3263, 0x0f},
    {0x3264, 0xfb},
    {0x3265, 0x0f},
    {0x3266, 0x0f},
    {0x3267, 0xff},
    {0x3268, 0x47},
    {0x3269, 0x00},
    {0x326a, 0x0a},
    {0x326b, 0xb6},
    {0x326c, 0x00},
    {0x326d, 0x78},
    {0x326e, 0x8a},
    {0x326f, 0x0f},
    {0x3270, 0xef},
    {0x3271, 0x1d},
    {0x3272, 0x00},
    {0x3273, 0x04},
    {0x3274, 0xee},
    {0x3275, 0x00},
    {0x3276, 0x08},
    {0x3277, 0x03},
    {0x3278, 0x0f},
    {0x3279, 0xf8},
    {0x327a, 0x58},
    {0x327b, 0x00},
    {0x327c, 0x04},
    {0x327d, 0x9f},
    {0x327e, 0x0f},
    {0x327f, 0xeb},
    {0x3280, 0x66},
    {0x3281, 0x0f},
    {0x3282, 0xf1},
    {0x3283, 0xf1},
    {0x3284, 0x00},
    {0x3285, 0x06},
    {0x3286, 0x26},
    {0x3287, 0x0f},
    {0x3288, 0xfe},
    {0x3289, 0xeb},
    {0x328a, 0x00},
    {0x328b, 0x09},
    {0x328c, 0x30},
    {0x328d, 0x0f},
    {0x328e, 0xf3},
    {0x328f, 0xef},
    {0x3290, 0x0f},
    {0x3291, 0xff},
    {0x3292, 0x3d},
    {0x3293, 0x00},
    {0x3294, 0x0e},
    {0x3295, 0x32},
    {0x3296, 0x00},
    {0x3297, 0x05},
    {0x3298, 0x0d},
    {0x3299, 0x0f},
    {0x329a, 0xf2},
    {0x329b, 0x4d},
    {0x329c, 0x0f},
    {0x329d, 0xf5},
    {0x329e, 0xe8},
    {0x329f, 0x00},
    {0x32a0, 0x16},
    {0x32a1, 0xd7},
    {0x32a2, 0x00},
    {0x32a3, 0x0f},
    {0x32a4, 0x6a},
    {0x32a5, 0x0f},
    {0x32a6, 0xf7},
    {0x32a7, 0x72},
    {0x32a8, 0x0f},
    {0x32a9, 0xf5},
    {0x32aa, 0xfd},
    {0x32ab, 0x00},
    {0x32ac, 0x10},
    {0x32ad, 0x3b},
    {0x32ae, 0x00},
    {0x32af, 0x0b},
    {0x32b0, 0xfa},
    {0x32b1, 0x0f},
    {0x32b2, 0xe8},
    {0x32b3, 0x27},
    {0x32b4, 0x0f},
    {0x32b5, 0xfd},
    {0x32b6, 0x1f},
    {0x32b7, 0x00},
    {0x32b8, 0x01},
    {0x32b9, 0xaf},
    {0x32ba, 0x0f},
    {0x32bb, 0xfd},
    {0x32bc, 0x43},
    {0x32bd, 0x00},
    {0x32be, 0x03},
    {0x32bf, 0x74},
    {0x32c0, 0x0f},
    {0x32c1, 0xfa},
    {0x32c2, 0xed},
    {0x32c3, 0x0f},
    {0x32c4, 0xfd},
    {0x32c5, 0xdc},
    {0x32c6, 0x0f},
    {0x32c7, 0xf9},
    {0x32c8, 0x1b},
    {0x32c9, 0x00},
    {0x32ca, 0x02},
    {0x32cb, 0xc8},
    {0x32cc, 0x00},
    {0x32cd, 0x0b},
    {0x32ce, 0x91},
    {0x32cf, 0x0f},
    {0x32d0, 0xf1},
    {0x32d1, 0x73},
    {0x32d2, 0x0f},
    {0x32d3, 0xf8},
    {0x32d4, 0xe8},
    {0x32d5, 0x00},
    {0x32d6, 0x17},
    {0x32d7, 0xdf},
    {0x32d8, 0x00},
    {0x32d9, 0x66},
    {0x32da, 0x64},
    {0x32db, 0x0f},
    {0x32dc, 0xfa},
    {0x32dd, 0x10},
    {0x32de, 0x0f},
    {0x32df, 0xf7},
    {0x32e0, 0xc4},
    {0x32e1, 0x00},
    {0x32e2, 0x12},
    {0x32e3, 0x55},
    {0x32e4, 0x0f},
    {0x32e5, 0xf7},
    {0x32e6, 0xab},
    {0x32e7, 0x0f},
    {0x32e8, 0xff},
    {0x32e9, 0x31},
    {0x32ea, 0x0f},
    {0x32eb, 0xf5},
    {0x32ec, 0xf4},
    {0x32ed, 0x0f},
    {0x32ee, 0xe9},
    {0x32ef, 0xeb},
    {0x32f0, 0x00},
    {0x32f1, 0x17},
    {0x32f2, 0x92},
    {0x32f3, 0x0f},
    {0x32f4, 0xec},
    {0x32f5, 0xce},
    {0x32f6, 0x00},
    {0x32f7, 0x0b},
    {0x32f8, 0xcd},
    {0x32f9, 0x0f},
    {0x32fa, 0xfd},
    {0x32fb, 0x47},
    {0x32fc, 0x0f},
    {0x32fd, 0xf6},
    {0x32fe, 0xf9},
    {0x32ff, 0x00},
    {0x3300, 0x18},
    {0x3301, 0x27},
    {0x3302, 0x0f},
    {0x3303, 0xee},
    {0x3304, 0x7e},
    {0x3305, 0x00},
    {0x3306, 0x0a},
    {0x3307, 0x7b},
    {0x3308, 0x0f},
    {0x3309, 0xf7},
    {0x330a, 0x70},
    {0x330b, 0x00},
    {0x330c, 0x02},
    {0x330d, 0x72},
    {0x330e, 0x00},
    {0x330f, 0x18},
    {0x3310, 0x19},
    {0x3311, 0x0f},
    {0x3312, 0xf5},
    {0x3313, 0x4f},
    {0x3314, 0x0f},
    {0x3315, 0xfd},
    {0x3316, 0xd5},
    {0x3317, 0x00},
    {0x3318, 0x05},
    {0x3319, 0xc6},
    {0x331a, 0x0f},
    {0x331b, 0xfd},
    {0x331c, 0x1b},
    {0x331d, 0x00},
    {0x331e, 0x07},
    {0x331f, 0xc7},
    {0x3320, 0x0f},
    {0x3321, 0xf2},
    {0x3322, 0xc4},
    {0x3323, 0x0f},
    {0x3324, 0xf6},
    {0x3325, 0x16},
    {0x3326, 0x00},
    {0x3327, 0x0a},
    {0x3328, 0x99},
    {0x3329, 0x00},
    {0x332a, 0x00},
    {0x332b, 0xc4},
    {0x332c, 0x00},
    {0x332d, 0x01},
    {0x332e, 0x34},
    {0x332f, 0x0f},
    {0x3330, 0xed},
    {0x3331, 0x50},
    {0x3332, 0x0f},
    {0x3333, 0xff},
    {0x3334, 0x7f},
    {0x3335, 0x00},
    {0x3336, 0x0f},
    {0x3337, 0x5b},
    {0x3338, 0x0f},
    {0x3339, 0xfc},
    {0x333a, 0x39},
    {0x333b, 0x0f},
    {0x333c, 0xf2},
    {0x333d, 0xec},
    {0x333e, 0x00},
    {0x333f, 0x05},
    {0x3340, 0xf7},
    {0x3341, 0x00},
    {0x3342, 0x0f},
    {0x3343, 0x5f},
    {0x3344, 0x00},
    {0x3345, 0x72},
    {0x3346, 0x4c},
    {0x3347, 0x0f},
    {0x3348, 0xf2},
    {0x3349, 0xf1},
    {0x334a, 0x00},
    {0x334b, 0x02},
    {0x334c, 0x80},
    {0x334d, 0x00},
    {0x334e, 0x06},
    {0x334f, 0x6c},
    {0x3350, 0x00},
    {0x3351, 0x00},
    {0x3352, 0xe9},
    {0x3353, 0x0f},
    {0x3354, 0xfc},
    {0x3355, 0x0b},
    {0x3356, 0x0f},
    {0x3357, 0xef},
    {0x3358, 0x1f},
    {0x3359, 0x0f},
    {0x335a, 0xec},
    {0x335b, 0xf5},
    {0x335c, 0x00},
    {0x335d, 0x15},
    {0x335e, 0x78},
    {0x335f, 0x0f},
    {0x3360, 0xf0},
    {0x3361, 0x72},
    {0x3362, 0x00},
    {0x3363, 0x05},
    {0x3364, 0x39},
    {0x3365, 0x00},
    {0x3366, 0x01},
    {0x3367, 0x72},
    {0x3368, 0x0f},
    {0x3369, 0xfb},
    {0x336a, 0xed},
    {0x336b, 0x00},
    {0x336c, 0x13},
    {0x336d, 0x4b},
    {0x336e, 0x0f},
    {0x336f, 0xeb},
    {0x3370, 0x1f},
    {0x3371, 0x00},
    {0x3372, 0x0a},
    {0x3373, 0xfc},
    {0x3374, 0x00},
    {0x3375, 0x03},
    {0x3376, 0x3a},
    {0x3377, 0x0f},
    {0x3378, 0xf7},
    {0x3379, 0xae},
    {0x337a, 0x00},
    {0x337b, 0x14},
    {0x337c, 0x72},
    {0x337d, 0x0f},
    {0x337e, 0xf8},
    {0x337f, 0x33},
    {0x3380, 0x00},
    {0x3381, 0x08},
    {0x3382, 0xa9},
    {0x3383, 0x0f},
    {0x3384, 0xfc},
    {0x3385, 0xd6},
    {0x3386, 0x0f},
    {0x3387, 0xf6},
    {0x3388, 0xfe},
    {0x3389, 0x00},
    {0x338a, 0x0f},
    {0x338b, 0x30},
    {0x338c, 0x0f},
    {0x338d, 0xf8},
    {0x338e, 0xb3},
    {0x338f, 0x0f},
    {0x3390, 0xf7},
    {0x3391, 0x72},
    {0x3392, 0x00},
    {0x3393, 0x03},
    {0x3394, 0xa1},
    {0x3395, 0x00},
    {0x3396, 0x06},
    {0x3397, 0x63},
    {0x3398, 0x0f},
    {0x3399, 0xfc},
    {0x339a, 0x4b},
    {0x339b, 0x0f},
    {0x339c, 0xf5},
    {0x339d, 0x88},
    {0x339e, 0x0f},
    {0x339f, 0xfa},
    {0x33a0, 0x83},
    {0x33a1, 0x00},
    {0x33a2, 0x0d},
    {0x33a3, 0x89},
    {0x33a4, 0x0f},
    {0x33a5, 0xf8},
    {0x33a6, 0xa6},
    {0x33a7, 0x0f},
    {0x33a8, 0xf8},
    {0x33a9, 0x51},
    {0x33aa, 0x00},
    {0x33ab, 0x0b},
    {0x33ac, 0xd3},
    {0x33ad, 0x00},
    {0x33ae, 0x01},
    {0x33af, 0x6f},
    /* MM-UW-UpdateTuningForColorShading-00*} */     
    {0x3096, 0x60},
    {0x3096, 0x40},
    /* MM-MC-UpdateTuningForColorShadingIn20130411-00*} */
};
/* MM-MC-ImplementRegSwitchMechanismForShading-00+} */

static struct v4l2_subdev_info s5k4e1_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array s5k4e1_init_conf[] = {
	{&s5k4e1_recommend_settings2[0],/* MM-MC-ImplementRegSwitchMechanismForShading-00* */
	ARRAY_SIZE(s5k4e1_recommend_settings2), 0, MSM_CAMERA_I2C_BYTE_DATA}/* MM-MC-ImplementRegSwitchMechanismForShading-00* */
};

static struct msm_camera_i2c_conf_array s5k4e1_confs[] = {
	{&s5k4e1_snap_settings[0],
	ARRAY_SIZE(s5k4e1_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&s5k4e1_prev_settings[0],
	ARRAY_SIZE(s5k4e1_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

//FIH-SW-MM-MC-UpdateCameraSettingForS5K4E1-00*{
static struct msm_sensor_output_info_t s5k4e1_dimensions[] = {
	{
		.x_output = 0xA30,//2608
		.y_output = 0x7A8,//1960
		.line_length_pclk = 0xAF0,//2800
		.frame_length_lines = 0x7EF,//2031
		//vt_pixel_clk* op_pixel_clk*fps
		.vt_pixel_clk = 85300000,//85302000
		.op_pixel_clk = 85300000,//85302000
		.binning_factor = 0,
	},
	{
		.x_output = 0x518,//1304
		.y_output = 0x3D4,//980
		.line_length_pclk = 0xD18,//3352
		.frame_length_lines = 0x4C1,//1217
		.vt_pixel_clk = 122400000,//
		.op_pixel_clk = 122400000,//
		.binning_factor = 1,
	},
};
//FIH-SW-MM-MC-UpdateCameraSettingForS5K4E1-00*}

static struct msm_sensor_output_reg_addr_t s5k4e1_reg_addr = {
	.x_output = 0x034C,
	.y_output = 0x034E,
	.line_length_pclk = 0x0342,
	.frame_length_lines = 0x0340,
};

static struct msm_sensor_id_info_t s5k4e1_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x4E10,
};

static struct msm_sensor_exp_gain_info_t s5k4e1_exp_gain_info = {
	.coarse_int_time_addr = 0x0202,
	.global_gain_addr = 0x0204,
	.vert_offset = 4,
};

static inline uint8_t s5k4e1_byte(uint16_t word, uint8_t offset)
{
	return word >> (offset * BITS_PER_BYTE);
}

static int32_t s5k4e1_write_prev_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
						uint16_t gain, uint32_t line, int32_t luma_avg, uint16_t fgain)
{
	uint16_t max_legal_gain = 0x0200;
	int32_t rc = 0;
	static uint32_t fl_lines, offset;

	pr_info("s5k4e1_write_prev_exp_gain :%d %d\n", gain, line);

       //line = (line * s_ctrl->fps_divider)/ Q10; /* MM-UW-fix MMS recording fail-00+{ */

	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (gain > max_legal_gain) {
		CDBG("Max legal gain Line:%d\n", __LINE__);
		gain = max_legal_gain;
	}

	/* Analogue Gain */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		s5k4e1_byte(gain, MSB),
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		s5k4e1_byte(gain, LSB),
		MSM_CAMERA_I2C_BYTE_DATA);

	if (line > (s_ctrl->curr_frame_length_lines - offset)) {
		fl_lines = line + offset;
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
			  /* MM-SL-FixMMSCantNotRecord23s-00*{ */
              /* MM-UW-Fix MMS can't record 23s-00+{ */
              if(s_ctrl->fps_divider > 2000){
                 fl_lines = fl_lines * 23/10;  /*let fps lower than 15*/
				 line = line * 23/10;
              }
              /* MM-UW-Fix MMS can't record 23s-00+} */
			  /* MM-SL-FixMMSCantNotRecord23s-00*} */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			s5k4e1_byte(fl_lines, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			s5k4e1_byte(fl_lines, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			s5k4e1_byte(line, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
			s5k4e1_byte(line, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	} else if (line < (fl_lines - offset)) {
		fl_lines = line + offset; 
		if (fl_lines < s_ctrl->curr_frame_length_lines)
			fl_lines = s_ctrl->curr_frame_length_lines;  
			  /* MM-SL-FixMMSCantNotRecord23s-00*{ */
              /* MM-UW-Fix MMS can't record 23s-00+{ */
              if(s_ctrl->fps_divider > 2000){
                 fl_lines = fl_lines * 23/10;  /*let fps lower than 15*/
			     line = line * 23/10;
			  }
              /* MM-UW-Fix MMS can't record 23s-00+} */
			  /* MM-SL-FixMMSCantNotRecord23s-00*} */
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			s5k4e1_byte(line, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
			s5k4e1_byte(line, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			s5k4e1_byte(fl_lines, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			s5k4e1_byte(fl_lines, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	} else {
		fl_lines = line+4;
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			s5k4e1_byte(line, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
			s5k4e1_byte(line, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	}
	return rc;
}

static int32_t s5k4e1_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line, int32_t luma_avg, uint16_t fgain)
{
	uint16_t max_legal_gain = 0x0200;
	uint16_t min_ll_pck = 0x0AB2;
	uint32_t ll_pck, fl_lines;
	uint32_t ll_ratio;
	uint8_t gain_msb, gain_lsb;
	uint8_t intg_time_msb, intg_time_lsb;
	uint8_t ll_pck_msb, ll_pck_lsb;

	if (gain > max_legal_gain) {
		CDBG("Max legal gain Line:%d\n", __LINE__);
		gain = max_legal_gain;
	}

	pr_info("s5k4e1_write_exp_gain : gain = %d line = %d\n", gain, line);
	line = (uint32_t) (line * s_ctrl->fps_divider);
	fl_lines = s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider / Q10;
	ll_pck = s_ctrl->curr_line_length_pclk;

	if (fl_lines < (line / Q10))
		ll_ratio = (line / (fl_lines - 4));
	else
		ll_ratio = Q10;

	ll_pck = ll_pck * ll_ratio / Q10;
	line = line / ll_ratio;
	if (ll_pck < min_ll_pck)
		ll_pck = min_ll_pck;

	gain_msb = (uint8_t) ((gain & 0xFF00) >> 8);
	gain_lsb = (uint8_t) (gain & 0x00FF);

	intg_time_msb = (uint8_t) ((line & 0xFF00) >> 8);
	intg_time_lsb = (uint8_t) (line & 0x00FF);

	ll_pck_msb = (uint8_t) ((ll_pck & 0xFF00) >> 8);
	ll_pck_lsb = (uint8_t) (ll_pck & 0x00FF);

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		gain_msb,
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		gain_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->line_length_pclk,
		ll_pck_msb,
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->line_length_pclk + 1,
		ll_pck_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* Coarse Integration Time */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		intg_time_msb,
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
		intg_time_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

	return 0;
}

//FIH-SW-MM-MC-BringUpCameraRawSensorS5k4e1-00+{
static struct msm_cam_clk_info cam_8960_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

static struct msm_cam_clk_info cam_8974_clk_info[] = {
	{"cam_src_clk", 19200000},
	{"cam_clk", -1},
};

int32_t s5k4e1_power_up(struct msm_sensor_ctrl_t *s_ctrl)
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

int32_t s5k4e1_power_down(struct msm_sensor_ctrl_t *s_ctrl)
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

//MM-UW-Change slave address-00+{
int32_t s5k4e1_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *s_info  = NULL;
    struct msm_sensor_ctrl_t *s_ctrl       = NULL;

    //MM-MC-ImplementReadOtpDataFeature-00*{
    if (g_IsFihMainCamProbe == 1)
    {
        printk("s5k4e1_sensor_i2c_probe: Main camera has probe success before !\n");
        return -EFAULT;
    }
    
    s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);
    rc = msm_sensor_i2c_probe(client, id);//Probe for 0x6C
    if(rc < 0){
#if 0/* MM-MC-FixMainCameraI2cBusy-00-{ */
        s_ctrl->sensor_i2c_addr = 0x6E;
        rc = msm_sensor_i2c_probe(client, id);//Probe for 0x6E
        if(rc < 0)
            return rc;
#else
        pr_err("%s %s_i2c_probe failed\n", __func__, client->name);
        return rc;
#endif/* MM-MC-FixMainCameraI2cBusy-00-} */
    }
    g_IsFihMainCamProbe = 1;
    printk("s5k4e1_sensor_i2c_probe: sensor_i2c_addr(0x6C/0x6E) = 0x%x, g_IsFihMainCamProbe = %d.\n", s_ctrl->sensor_i2c_addr, g_IsFihMainCamProbe);
    //MM-MC-ImplementReadOtpDataFeature-00*}

	s_info = client->dev.platform_data;
	if (s_info == NULL) {
		pr_err("%s %s NULL sensor data\n", __func__, client->name);
		return -EFAULT;
	}

	if (s_info->actuator_info->vcm_enable) {
		rc = gpio_request(s_info->actuator_info->vcm_pwd,
				"msm_actuator");
		if (rc < 0)
			pr_err("%s: gpio_request:msm_actuator %d failed\n",
				__func__, s_info->actuator_info->vcm_pwd);
		rc = gpio_direction_output(s_info->actuator_info->vcm_pwd, 0);
		if (rc < 0)
			pr_err("%s: gpio:msm_actuator %d direction can't be set\n",
				__func__, s_info->actuator_info->vcm_pwd);
		gpio_free(s_info->actuator_info->vcm_pwd);
	}
	return rc;
}
//MM-UW-Change slave address-00+}

/* MM-UW-Improve camera open performance-00+{ */
//MM-MC-ImplementReadOtpDataFeature-00+{
int32_t S5k4e1_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
      int32_t rc = 0;
      int32_t productid = 0;
      int32_t shading_index = 0;/* MM-MC-ImplementRegSwitchMechanismForShading-00+ */
      uint16_t chipid = 0;
    
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
		pr_err("S5k4e1_match_id: chip id doesnot match\n");
		return -ENODEV;
	}

    if (s_ctrl->sensor_i2c_addr == 0x6C)
    {
        if(!g_IsOtpInitDone)
        {
            rc = fih_init_otp(s_ctrl);
            if (rc < 0)
                return rc;
        
            rc = fih_get_otp_data(OTP_MI_PID , &productid);
            if (rc < 0)
                return rc;
            
            if (productid != PID_S5K4E1)
            {
                pr_err("S5k4e1_match_id: Product id doesnot match\n");
                return -ENODEV;
            }
        }

        /* MM-MC-ImplementRegSwitchMechanismForShading-00+{ */
        rc = fih_get_otp_data(OTP_SHADING_INDEX , &shading_index);
        if (rc < 0)
            return rc;
        
        if (shading_index == OTP_SHADING_1)
        {
            s5k4e1_init_conf[0].conf = &s5k4e1_recommend_settings1[0];
            s5k4e1_init_conf[0].size = ARRAY_SIZE(s5k4e1_recommend_settings1);
            s5k4e1_init_conf[0].delay = 0;
            s5k4e1_init_conf[0].data_type = MSM_CAMERA_I2C_BYTE_DATA;
            printk("S5k4e1_match_id: Load shading setting for golden module !\n");
        }
        else
            printk("S5k4e1_match_id: shading_index = %d, Load shading setting for corner module !\n", shading_index);
        /* MM-MC-ImplementRegSwitchMechanismForShading-00+} */
    }
    
	return rc;
}
//MM-MC-ImplementReadOtpDataFeature-00+}
/* MM-UW-Improve camera open performance-00+} */

/* MM-MC-ImplementRegSwitchMechanismForShading-00+{ */
int32_t S5k4e1_get_output_info(struct msm_sensor_ctrl_t *s_ctrl,
		struct sensor_output_info_t *sensor_output_info)
{
	int rc = 0;
    int32_t shading_index = 0;
    
    rc = fih_get_otp_data(OTP_SHADING_INDEX , &shading_index);
    if (rc < 0)
        return rc;

    sensor_output_info->shading_index = shading_index;
    printk("S5k4e1_get_output_info: shading_index = %d !\n", shading_index);
    
	sensor_output_info->num_info = s_ctrl->msm_sensor_reg->num_conf;
	if (copy_to_user((void *)sensor_output_info->output_info,
		s_ctrl->msm_sensor_reg->output_settings,
		sizeof(struct msm_sensor_output_info_t) *
		s_ctrl->msm_sensor_reg->num_conf))
		rc = -EFAULT;

	return rc;
}
/* MM-MC-ImplementRegSwitchMechanismForShading-00+} */

static enum msm_camera_vreg_name_t s5k4e1_veg_seq[] = {
	CAM_VDIG,
	CAM_VIO,
	CAM_VANA,
	CAM_VAF,
};

static const struct i2c_device_id s5k4e1_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&s5k4e1_s_ctrl},
	{ }
};

static struct i2c_driver s5k4e1_i2c_driver = {
	.id_table = s5k4e1_i2c_id,
	.probe  = s5k4e1_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client s5k4e1_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&s5k4e1_i2c_driver);
}

static struct v4l2_subdev_core_ops s5k4e1_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops s5k4e1_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops s5k4e1_subdev_ops = {
	.core = &s5k4e1_subdev_core_ops,
	.video  = &s5k4e1_subdev_video_ops,
};

static struct msm_sensor_fn_t s5k4e1_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = s5k4e1_write_prev_exp_gain,
	.sensor_write_snapshot_exp_gain = s5k4e1_write_pict_exp_gain,
	.sensor_setting = msm_sensor_setting,  //v
	.sensor_csi_setting = msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = S5k4e1_get_output_info,/* MM-MC-ImplementRegSwitchMechanismForShading-00+ */  //v
	.sensor_config = msm_sensor_config,
	.sensor_power_up = s5k4e1_power_up,//,msm_sensor_power_up,//FIH-SW-MM-MC-BringUpCameraRawSensorS5k4e1-00*                   //v
	.sensor_power_down = s5k4e1_power_down,//msm_sensor_power_down,//FIH-SW-MM-MC-BringUpCameraRawSensorS5k4e1-00*         //v
	.sensor_adjust_frame_lines = msm_sensor_adjust_frame_lines1,                                //v
	.sensor_get_csi_params = msm_sensor_get_csi_params,                                           //v
	.sensor_match_id = S5k4e1_match_id,//MM-MC-ImplementReadOtpDataFeature-00+              //v
};

static struct msm_sensor_reg_t s5k4e1_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = s5k4e1_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(s5k4e1_start_settings),
	.stop_stream_conf = s5k4e1_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(s5k4e1_stop_settings),
	.group_hold_on_conf = s5k4e1_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(s5k4e1_groupon_settings),
	.group_hold_off_conf = s5k4e1_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(s5k4e1_groupoff_settings),
	.init_settings = &s5k4e1_init_conf[0],
	.init_size = ARRAY_SIZE(s5k4e1_init_conf),
	.mode_settings = &s5k4e1_confs[0],
	.output_settings = &s5k4e1_dimensions[0],
	.num_conf = ARRAY_SIZE(s5k4e1_confs),
};

static struct msm_sensor_ctrl_t s5k4e1_s_ctrl = {
	.msm_sensor_reg = &s5k4e1_regs,
	.sensor_i2c_client = &s5k4e1_sensor_i2c_client,
	.sensor_i2c_addr = 0x6C,
	.vreg_seq = s5k4e1_veg_seq,
	.num_vreg_seq = ARRAY_SIZE(s5k4e1_veg_seq),
	.sensor_output_reg_addr = &s5k4e1_reg_addr,
	.sensor_id_info = &s5k4e1_id_info,
	.sensor_exp_gain_info = &s5k4e1_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.msm_sensor_mutex = &s5k4e1_mut,
	.sensor_i2c_driver = &s5k4e1_i2c_driver,
	.sensor_v4l2_subdev_info = s5k4e1_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k4e1_subdev_info),
	.sensor_v4l2_subdev_ops = &s5k4e1_subdev_ops,
	.func_tbl = &s5k4e1_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Samsung 5MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
