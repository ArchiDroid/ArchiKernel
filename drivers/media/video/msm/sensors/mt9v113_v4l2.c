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
#define SENSOR_NAME "mt9v113"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9v113"
#define mt9v113_obj mt9v113_##obj


#ifdef CDBG
#undef CDBG
#define CDBG printk
#else
#define CDBG printk
#endif

static int mt9v113_polling_reg(struct msm_camera_i2c_client *client,unsigned short waddr, 
	unsigned short wcondition, unsigned short result, int delay, int time_out);
int32_t mt9v113_camera_i2c_write_tbl(struct msm_camera_i2c_client *client,
                                     struct msm_camera_i2c_reg_conf *reg_conf_tbl, uint16_t size,
                                     enum msm_camera_i2c_data_type data_type);
int mt9v113_check_sensor_mode(struct msm_sensor_ctrl_t *s_ctrl);
int mt9v113_msm_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value);
DEFINE_MUTEX(mt9v113_mut);
static struct msm_sensor_ctrl_t mt9v113_s_ctrl;

static int PREV_EFFECT = -1;
static int PREV_EXPOSURE = -1;
static int PREV_WB = -1;
static int PREV_FPS = -1;
static int PREV_NIGHT_MODE = -1;

typedef enum {
  MT9V113_60HZ,
  MT9V113_50HZ,
  MT9V113_HZ_MAX_NUM,
} MT9V113AntibandingType;

static int mt9v113_antibanding = MT9V113_50HZ;

static struct msm_camera_i2c_reg_conf mt9v113_start_settings[] = {
	{0x0018, 0x4028},	
	{0xFFFF, 0x0002},
};

static struct msm_camera_i2c_reg_conf mt9v113_stop_settings[] = {
	{0x0018, 0x4029},	
	{0xFFFF, 0x0005},
};

static struct msm_camera_i2c_reg_conf mt9v113_prev_settings[] = {
};

static struct msm_camera_i2c_reg_conf mt9v113_recommend_settings[MT9V113_HZ_MAX_NUM][507] = {
	// antibanding 60Hz
	{
		{0x0018, 0x4028}, // STANDBY_CONTROL  
		{0x0018, 0x4000},
		{0x301A, 0x0004},
		{0x001A, 0x0013}, // RESET_AND_MISC_CONTROL                                                                                  //DELAY=10           
		{0xFFFF, 0x000A},                                                                                          
		{0x001A, 0x0010}, // RESET_AND_MISC_CONTROL                                                                                                                                                                   
		//DELAY=10           
		{0xFFFF, 0x000A},                                                                                                                                                                                            
		{0x0018, 0x4028}, // STANDBY_CONTROL   
		{0x0018, 0x4000},
		{0x301A, 0x0004},
		//reduce_IO
		{0x098C, 0x02F0}, 	// MCU_ADDRESS
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0x02F2}, 	// MCU_ADDRESS
		{0x0990, 0x0210}, 	// MCU_DATA_0
		{0x098C, 0x02F4}, 	// MCU_ADDRESS
		{0x0990, 0x001A}, 	// MCU_DATA_0
		{0x098C, 0x2145}, 	// MCU_ADDRESS
		{0x0990, 0x02F4}, 	// MCU_DATA_0
		{0x098C, 0xA134}, 	// MCU_ADDRESS
		{0x0990, 0x0001}, 	// MCU_DATA_0
		{0x31E0, 0x0001}, 	// CORE_31E0
		{0xFFFF, 0x000A},
		{0x3400, 0x783C}, 	// MIPI_CONTROL	
		{0x001A, 0x0210}, 	// RESET_AND_MISC_CONTROL
		//{0x3400, 0x7A34}, 	// MIPI_CONTROL
		//{0x321C, 0x0003}, 	// OFIFO_CONTROL_STATUS
		{0x001E, 0x0400}, 	// PAD_SLEW
		{0x0016, 0x42DF}, 	// CLOCKS_CONTROL

		//pll
		{0x0014, 0x2145}, 	// PLL_CONTROL
		{0x0014, 0x2145}, 	// PLL_CONTROL
		{0x0010, 0x021C}, 	// PLL_DIVIDERS
		{0x0012, 0x0000}, 	// PLL_P_DIVIDERS
		{0x0014, 0x244B}, 	// PLL_CONTROL
		{0xFFFF, 0x000A},  	// DELAY=10 

		{0x0014, 0x304B}, 	// PLL_CONTROL
		{0xFFFF, 0x0032},  	// DELAY=50

		//{0x0014, 0x8000},
		//POLL_REG=0x0014, 0x8000, ==0, DELAY=50, TIMEOUT=20 // Verify PLL lock	//DELAY=10

		{0x0014, 0xB04A}, 	// PLL_CONTROL                          
		{0xFFFF, 0x0012}, 

		{0x3400, 0x7A3C}, 	// MIPI_CONTROL

		{0x001A, 0x0010}, 	
		{0x001A, 0x0018}, 	
		{0x321C, 0x0003}, 	// OFIFO_CONTROL_STATUS                   

		{0x098C, 0x2703}, 	// MCU_ADDRESS
		{0x0990, 0x0280}, 	// MCU_DATA_0
		{0x098C, 0x2705}, 	// MCU_ADDRESS
		{0x0990, 0x01E0}, 	// MCU_DATA_0
		{0x098C, 0x2707}, 	// MCU_ADDRESS
		{0x0990, 0x0280}, 	// MCU_DATA_0
		{0x098C, 0x2709}, 	// MCU_ADDRESS
		{0x0990, 0x01E0}, 	// MCU_DATA_0
		{0x098C, 0x270D}, 	// MCU_ADDRESS
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0x270F}, 	// MCU_ADDRESS
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0x2711}, 	// MCU_ADDRESS
		{0x0990, 0x01E7}, 	// MCU_DATA_0
		{0x098C, 0x2713}, 	// MCU_ADDRESS
		{0x0990, 0x0287}, 	// MCU_DATA_0
		{0x098C, 0x2715}, 	// MCU_ADDRESS
		{0x0990, 0x0001}, 	// MCU_DATA_0
		{0x098C, 0x2717}, 	// MCU_ADDRESS
		{0x0990, 0x0026}, 	// MCU_DATA_0
		{0x098C, 0x2719}, 	// MCU_ADDRESS
		{0x0990, 0x001A}, 	// MCU_DATA_0
		{0x098C, 0x271B}, 	// MCU_ADDRESS
		{0x0990, 0x006B}, 	// MCU_DATA_0
		{0x098C, 0x271D}, 	// MCU_ADDRESS
		{0x0990, 0x006B}, 	// MCU_DATA_0
		{0x098C, 0x271F}, 	// MCU_ADDRESS
		{0x0990, 0x01FB}, 	// MCU_DATA_0
		{0x098C, 0x2721}, 	// MCU_ADDRESS
		{0x0990, 0x0398}, 	// MCU_DATA_0
		{0x098C, 0x2723}, 	// MCU_ADDRESS
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0x2725}, 	// MCU_ADDRESS
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0x2727}, 	// MCU_ADDRESS
		{0x0990, 0x01E7}, 	// MCU_DATA_0
		{0x098C, 0x2729}, 	// MCU_ADDRESS
		{0x0990, 0x0287}, 	// MCU_DATA_0
		{0x098C, 0x272B}, 	// MCU_ADDRESS
		{0x0990, 0x0001}, 	// MCU_DATA_0
		{0x098C, 0x272D}, 	// MCU_ADDRESS
		{0x0990, 0x0026}, 	// MCU_DATA_0
		{0x098C, 0x272F}, 	// MCU_ADDRESS
		{0x0990, 0x001A}, 	// MCU_DATA_0
		{0x098C, 0x2731}, 	// MCU_ADDRESS
		{0x0990, 0x006B}, 	// MCU_DATA_0
		{0x098C, 0x2733}, 	// MCU_ADDRESS
		{0x0990, 0x006B}, 	// MCU_DATA_0
		{0x098C, 0x2735}, 	// MCU_ADDRESS
		{0x0990, 0x01FB}, 	// MCU_DATA_0
		{0x098C, 0x2737}, 	// MCU_ADDRESS
		{0x0990, 0x0398}, 	// MCU_DATA_0
		{0x098C, 0x2739}, 	// MCU_ADDRESS
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0x273B}, 	// MCU_ADDRESS
		{0x0990, 0x027F}, 	// MCU_DATA_0
		{0x098C, 0x273D}, 	// MCU_ADDRESS
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0x273F}, 	// MCU_ADDRESS
		{0x0990, 0x01DF}, 	// MCU_DATA_0
		{0x098C, 0x2747}, 	// MCU_ADDRESS
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0x2749}, 	// MCU_ADDRESS
		{0x0990, 0x027F}, 	// MCU_DATA_0
		{0x098C, 0x274B}, 	// MCU_ADDRESS
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0x274D}, 	// MCU_ADDRESS
		{0x0990, 0x01DF}, 	// MCU_DATA_0
		{0x098C, 0x222D}, 	// MCU_ADDRESS
		{0x0990, 0x007F}, 	// MCU_DATA_0
		{0x098C, 0xA408}, 	// MCU_ADDRESS
		{0x0990, 0x001E}, 	// MCU_DATA_0
		{0x098C, 0xA409}, 	// MCU_ADDRESS
		{0x0990, 0x0020}, 	// MCU_DATA_0
		{0x098C, 0xA40A}, 	// MCU_ADDRESS
		{0x0990, 0x0025}, 	// MCU_DATA_0
		{0x098C, 0xA40B}, 	// MCU_ADDRESS
		{0x0990, 0x0027}, 	// MCU_DATA_0
		{0x098C, 0x2411}, 	// MCU_ADDRESS
		{0x0990, 0x007F}, 	// MCU_DATA_0
		{0x098C, 0x2413}, 	// MCU_ADDRESS
		{0x0990, 0x0098}, 	// MCU_DATA_0
		{0x098C, 0x2415}, 	// MCU_ADDRESS
		{0x0990, 0x007F}, 	// MCU_DATA_0
		{0x098C, 0x2417}, 	// MCU_ADDRESS
		{0x0990, 0x0098}, 	// MCU_DATA_0
		{0x098C, 0xA404}, 	// MCU_ADDRESS
		{0x0990, 0x0010}, 	// MCU_DATA_0
		{0x098C, 0xA40D}, 	// MCU_ADDRESS
		{0x0990, 0x0002}, 	// MCU_DATA_0
		{0x098C, 0xA40E}, 	// MCU_ADDRESS
		{0x0990, 0x0003}, 	// MCU_DATA_0
		{0x098C, 0xA410}, 	// MCU_ADDRESS
		{0x0990, 0x000A}, 	// MCU_DATA_0

		//[Gamma]
		//0x098C, 0xAB37,  // MCU_ADDRESS    // 080723 ... .. .. .....
		//0x0990, 0x0001,  // gamma_table_A_0   // 080723 ... .. .. .....
		{0x098C, 0xAB37},  // MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
		{0x0990, 0x0001}, // MCU_DATA_0
		{0x098C, 0x2B38},  // MCU_ADDRESS [HG_GAMMASTARTMORPH]
		{0x0990, 0x1000},  // MCU_DATA_0
		{0x098C, 0x2B3A},  // MCU_ADDRESS [HG_GAMMASTOPMORPH]
		{0x0990, 0x2000},  // MCU_DATA_0
		{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
		{0x0990, 0x0017}, 	// MCU_DATA_0
		{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
		{0x0990, 0x0028}, 	// MCU_DATA_0
		{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
		{0x0990, 0x003D}, 	// MCU_DATA_0
		{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
		{0x0990, 0x005B}, 	// MCU_DATA_0
		{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
		{0x0990, 0x0074}, 	// MCU_DATA_0
		{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
		{0x0990, 0x0089}, 	// MCU_DATA_0
		{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
		{0x0990, 0x009B}, 	// MCU_DATA_0
		{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
		{0x0990, 0x00AA}, 	// MCU_DATA_0
		{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
		{0x0990, 0x00B7}, 	// MCU_DATA_0
		{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
		{0x0990, 0x00C3}, 	// MCU_DATA_0
		{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
		{0x0990, 0x00CD}, 	// MCU_DATA_0
		{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
		{0x0990, 0x00D6}, 	// MCU_DATA_0
		{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
		{0x0990, 0x00DE}, 	// MCU_DATA_0
		{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
		{0x0990, 0x00E6}, 	// MCU_DATA_0
		{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
		{0x0990, 0x00ED}, 	// MCU_DATA_0
		{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
		{0x0990, 0x00F3}, 	// MCU_DATA_0
		{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
		{0x0990, 0x00F9}, 	// MCU_DATA_0
		{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
		{0x0990, 0x00FF}, 	// MCU_DATA_0
		{0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
		{0x0990, 0x0017}, 	// MCU_DATA_0
		{0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
		{0x0990, 0x0028}, 	// MCU_DATA_0
		{0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
		{0x0990, 0x003D}, 	// MCU_DATA_0
		{0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
		{0x0990, 0x005B}, 	// MCU_DATA_0
		{0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
		{0x0990, 0x0074}, 	// MCU_DATA_0
		{0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
		{0x0990, 0x0089}, 	// MCU_DATA_0
		{0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
		{0x0990, 0x009B}, 	// MCU_DATA_0
		{0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
		{0x0990, 0x00AA}, 	// MCU_DATA_0
		{0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
		{0x0990, 0x00B7}, 	// MCU_DATA_0
		{0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
		{0x0990, 0x00C3}, 	// MCU_DATA_0
		{0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
		{0x0990, 0x00CD}, 	// MCU_DATA_0
		{0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
		{0x0990, 0x00D6}, 	// MCU_DATA_0
		{0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
		{0x0990, 0x00DE}, 	// MCU_DATA_0
		{0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
		{0x0990, 0x00E6}, 	// MCU_DATA_0
		{0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
		{0x0990, 0x00ED}, 	// MCU_DATA_0
		{0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
		{0x0990, 0x00F3}, 	// MCU_DATA_0
		{0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
		{0x0990, 0x00F9}, 	// MCU_DATA_0
		{0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
		{0x0990, 0x00FF}, 	// MCU_DATA_0
		  
		//[Lens Correction 85]                     
		{0x3658, 0x0130},  //  P_RD_P0Q0    
		{0x365A, 0x030D},  //  P_RD_P0Q1    
		{0x365C, 0x6B92},  //  P_RD_P0Q2    
		{0x365E, 0xE62E},  //  P_RD_P0Q3    
		{0x3660, 0x53B4},  //  P_RD_P0Q4    
		{0x3680, 0x1AED},  //  P_RD_P1Q0    
		{0x3682, 0x2A6A},  //  P_RD_P1Q1    
		{0x3684, 0xBA2B},  //  P_RD_P1Q2    
		{0x3686, 0x392F},  //  P_RD_P1Q3    
		{0x3688, 0xAD53},  //  P_RD_P1Q4    
		{0x36A8, 0x1453},  //  P_RD_P2Q0    
		{0x36AA, 0xC78D},  //  P_RD_P2Q1    
		{0x36AC, 0x4C35},  //  P_RD_P2Q2    
		{0x36AE, 0x7791},  //  P_RD_P2Q3    
		{0x36B0, 0x6797},  //  P_RD_P2Q4    
		{0x36D0, 0xE3F0},  //  P_RD_P3Q0    
		{0x36D2, 0xF8F0},  //  P_RD_P3Q1    
		{0x36D4, 0x5934},  //  P_RD_P3Q2    
		{0x36D6, 0x6AF2},  //  P_RD_P3Q3    
		{0x36D8, 0xFC98},  //  P_RD_P3Q4    
		{0x36F8, 0x2EB4},  //  P_RD_P4Q0    
		{0x36FA, 0x9972},  //  P_RD_P4Q1    
		{0x36FC, 0x5E33},  //  P_RD_P4Q2    
		{0x36FE, 0x07D8},  //  P_RD_P4Q3    
		{0x3700, 0x51DC},  //  P_RD_P4Q4    
		{0x364E, 0x0170},  //  P_GR_P0Q0    
		{0x3650, 0x570C},  //  P_GR_P0Q1    
		{0x3652, 0x6652},  //  P_GR_P0Q2    
		{0x3654, 0xB5B0},  //  P_GR_P0Q3    
		{0x3656, 0x3274},  //  P_GR_P0Q4    
		{0x3676, 0x214D},  //  P_GR_P1Q0    
		{0x3678, 0x3D46},  //  P_GR_P1Q1    
		{0x367A, 0x218F},  //  P_GR_P1Q2    
		{0x367C, 0x13D0},  //  P_GR_P1Q3    
		{0x367E, 0x8C34},  //  P_GR_P1Q4    
		{0x369E, 0x1253},  //  P_GR_P2Q0    
		{0x36A0, 0x98B0},  //  P_GR_P2Q1    
		{0x36A2, 0x3275},  //  P_GR_P2Q2    
		{0x36A4, 0x4FD0},  //  P_GR_P2Q3    
		{0x36A6, 0x0896},  //  P_GR_P2Q4    
		{0x36C6, 0x82D0},  //  P_GR_P3Q0    
		{0x36C8, 0x8292},  //  P_GR_P3Q1    
		{0x36CA, 0x0CD5},  //  P_GR_P3Q2    
		{0x36CC, 0x2632},  //  P_GR_P3Q3    
		{0x36CE, 0xC659},  //  P_GR_P3Q4    
		{0x36EE, 0x2374},  //  P_GR_P4Q0    
		{0x36F0, 0xDC32},  //  P_GR_P4Q1    
		{0x36F2, 0xCBF7},  //  P_GR_P4Q2    
		{0x36F4, 0x1918},  //  P_GR_P4Q3    
		{0x36F6, 0x675C},  //  P_GR_P4Q4    
		{0x3662, 0x00D0},  //  P_BL_P0Q0    
		{0x3664, 0x67AC},  //  P_BL_P0Q1    
		{0x3666, 0x6CD2},  //  P_BL_P0Q2    
		{0x3668, 0xB350},  //  P_BL_P0Q3    
		{0x366A, 0x0CD4},  //  P_BL_P0Q4    
		{0x368A, 0x400C},  //  P_BL_P1Q0    
		{0x368C, 0x07ED},  //  P_BL_P1Q1    
		{0x368E, 0x57CF},  //  P_BL_P1Q2    
		{0x3690, 0x3C50},  //  P_BL_P1Q3    
		{0x3692, 0xD654},  //  P_BL_P1Q4    
		{0x36B2, 0x0213},  //  P_BL_P2Q0    
		{0x36B4, 0x2CCF},  //  P_BL_P2Q1    
		{0x36B6, 0x1636},  //  P_BL_P2Q2    
		{0x36B8, 0x8EB5},  //  P_BL_P2Q3    
		{0x36BA, 0xF338},  //  P_BL_P2Q4    
		{0x36DA, 0xD310},  //  P_BL_P3Q0    
		{0x36DC, 0x22B0},  //  P_BL_P3Q1    
		{0x36DE, 0x3273},  //  P_BL_P3Q2    
		{0x36E0, 0x9BF6},  //  P_BL_P3Q3    
		{0x36E2, 0x82D8},  //  P_BL_P3Q4    
		{0x3702, 0x4854},  //  P_BL_P4Q0    
		{0x3704, 0x9AD5},  //  P_BL_P4Q1    
		{0x3706, 0xD519},  //  P_BL_P4Q2    
		{0x3708, 0x5D99},  //  P_BL_P4Q3    
		{0x370A, 0x685D},  //  P_BL_P4Q4    
		{0x366C, 0x00D0},  //  P_GB_P0Q0    
		{0x366E, 0x498C},  //  P_GB_P0Q1    
		{0x3670, 0x6B32},  //  P_GB_P0Q2    
		{0x3672, 0xA910},  //  P_GB_P0Q3    
		{0x3674, 0x3614},  //  P_GB_P0Q4    
		{0x3694, 0x3FAC},  //  P_GB_P1Q0    
		{0x3696, 0xB68A},  //  P_GB_P1Q1    
		{0x3698, 0x0FB0},  //  P_GB_P1Q2    
		{0x369A, 0x5E70},  //  P_GB_P1Q3    
		{0x369C, 0xD6B4},  //  P_GB_P1Q4    
		{0x36BC, 0x09F3},  //  P_GB_P2Q0    
		{0x36BE, 0xB9F1},  //  P_GB_P2Q1    
		{0x36C0, 0x3C55},  //  P_GB_P2Q2    
		{0x36C2, 0x2035},  //  P_GB_P2Q3    
		{0x36C4, 0x5B55},  //  P_GB_P2Q4    
		{0x36E4, 0x9450},  //  P_GB_P3Q0    
		{0x36E6, 0xF171},  //  P_GB_P3Q1    
		{0x36E8, 0x16D4},  //  P_GB_P3Q2    
		{0x36EA, 0x8234},  //  P_GB_P3Q3    
		{0x36EC, 0x98F9},  //  P_GB_P3Q4    
		{0x370C, 0x3854},  //  P_GB_P4Q0    
		{0x370E, 0x3754},  //  P_GB_P4Q1    
		{0x3710, 0xBED7},  //  P_GB_P4Q2    
		{0x3712, 0xF557},  //  P_GB_P4Q3    
		{0x3714, 0x4B7C},  //  P_GB_P4Q4    
		{0x3644, 0x0148},  //  POLY_ORIGIN_C
		{0x3642, 0x00F0},  //  POLY_ORIGIN_R         
		{0x3210, 0x09B8},  // COLOR_PIPELINE_CONTROL
		                                                                                                  
		//[AWB and CCMs ..2]                                                                                
		{0x098C, 0x2306}, 	// MCU_ADDRESS [AWB_CCM_L_0]
		{0x0990, 0x0133}, 	// MCU_DATA_0
		{0x098C, 0x2308}, 	// MCU_ADDRESS [AWB_CCM_L_1]
		{0x0990, 0xFFC4}, 	// MCU_DATA_0
		{0x098C, 0x230A}, 	// MCU_ADDRESS [AWB_CCM_L_2]
		{0x0990, 0x0014}, 	// MCU_DATA_0
		{0x098C, 0x230C}, 	// MCU_ADDRESS [AWB_CCM_L_3]
		{0x0990, 0xFF64}, 	// MCU_DATA_0
		{0x098C, 0x230E}, 	// MCU_ADDRESS [AWB_CCM_L_4]
		{0x0990, 0x01E3}, 	// MCU_DATA_0
		{0x098C, 0x2310}, 	// MCU_ADDRESS [AWB_CCM_L_5]
		{0x0990, 0xFFB2}, 	// MCU_DATA_0
		{0x098C, 0x2312}, 	// MCU_ADDRESS [AWB_CCM_L_6]
		{0x0990, 0xFF9A}, 	// MCU_DATA_0
		{0x098C, 0x2314}, 	// MCU_ADDRESS [AWB_CCM_L_7]
		{0x0990, 0xFEDB}, 	// MCU_DATA_0
		{0x098C, 0x2316}, 	// MCU_ADDRESS [AWB_CCM_L_8]
		{0x0990, 0x0213}, 	// MCU_DATA_0
		{0x098C, 0x2318}, 	// MCU_ADDRESS [AWB_CCM_L_9]
		{0x0990, 0x001C}, 	// MCU_DATA_0
		{0x098C, 0x231A}, 	// MCU_ADDRESS [AWB_CCM_L_10]
		{0x0990, 0x003A}, 	// MCU_DATA_0
		{0x098C, 0x231C}, 	// MCU_ADDRESS [AWB_CCM_RL_0]
		{0x0990, 0x0064}, 	// MCU_DATA_0
		{0x098C, 0x231E}, 	// MCU_ADDRESS [AWB_CCM_RL_1]
		{0x0990, 0xFF7D}, 	// MCU_DATA_0
		{0x098C, 0x2320}, 	// MCU_ADDRESS [AWB_CCM_RL_2]
		{0x0990, 0xFFFF}, 	// MCU_DATA_0
		{0x098C, 0x2322}, 	// MCU_ADDRESS [AWB_CCM_RL_3]
		{0x0990, 0x001A}, 	// MCU_DATA_0
		{0x098C, 0x2324}, 	// MCU_ADDRESS [AWB_CCM_RL_4]
		{0x0990, 0xFF94}, 	// MCU_DATA_0
		{0x098C, 0x2326}, 	// MCU_ADDRESS [AWB_CCM_RL_5]
		{0x0990, 0x0048}, 	// MCU_DATA_0
		{0x098C, 0x2328}, 	// MCU_ADDRESS [AWB_CCM_RL_6]
		{0x0990, 0x001B}, 	// MCU_DATA_0
		{0x098C, 0x232A}, 	// MCU_ADDRESS [AWB_CCM_RL_7]
		{0x0990, 0x0166}, 	// MCU_DATA_0
		{0x098C, 0x232C}, 	// MCU_ADDRESS [AWB_CCM_RL_8]
		{0x0990, 0xFEE3}, 	// MCU_DATA_0
		{0x098C, 0x232E}, 	// MCU_ADDRESS [AWB_CCM_RL_9]
		{0x0990, 0x0004}, 	// MCU_DATA_0
		{0x098C, 0x2330}, 	// MCU_ADDRESS [AWB_CCM_RL_10]
		{0x0990, 0xFFDC}, 	// MCU_DATA_0             
		{0x098C, 0xA348},  // MCU_ADDRESS                
		{0x0990, 0x0008},  // AWB_GAIN_BUFFER_SPEED      
		{0x098C, 0xA349},  // MCU_ADDRESS                
		{0x0990, 0x0002},  // AWB_JUMP_DIVISOR           
		{0x098C, 0xA34A},  // MCU_ADDRESS                
		{0x0990, 0x0059},  // AWB_GAINMIN_R               
		{0x098C, 0xA34B},  // MCU_ADDRESS                
		{0x0990, 0x00E6},  // AWB_GAINMAX_R
		{0x098C, 0xA34C},  // MCU_ADDRESS      
		{0x0990, 0x0059},  // AWB_GAINMIN_B                        
		{0x098C, 0xA34D},  // MCU_ADDRESS     
		{0x0990, 0x00A6},  // AWB_GAINMAX_B
		{0x098C, 0xA34E},  // MCU_ADDRESS     
		{0x0990, 0x0080},  // AWB_GAIN_R
		{0x098C, 0xA34F},  // MCU_ADDRESS     
		{0x0990, 0x0080},  // AWB_GAIN_G
		{0x098C, 0xA350},  // MCU_ADDRESS     
		{0x0990, 0x0080},  // AWB_GAIN_B        
		//[AWB MIN POS CHANGE BY DANIEL]
		{0x098C, 0xA351},  // MCU_ADDRESS [AWB_CCM_POSITION_MIN]
		{0x0990, 0x0020},  // MCU_DATA_0   
		{0x098C, 0xA352},  // MCU_ADDRESS                
		{0x0990, 0x007F},  // AWB_CCM_POSITION_MAX
		{0x098C, 0xA354},  // MCU_ADDRESS                
		{0x0990, 0x0060},  // AWB_CCM_POSITION
		{0x098C, 0xA354},  // MCU_ADDRESS                
		{0x0990, 0x0060},  // AWB_SATURATION             
		{0x098C, 0xA355},  // MCU_ADDRESS                
		{0x0990, 0x0001},  // AWB_MODE                   
		{0x098C, 0xA35D},  // MCU_ADDRESS                
		{0x0990, 0x0078},  // AWB_STEADY_BGAIN_OUT_MIN   
		{0x098C, 0xA35E},  // MCU_ADDRESS                
		{0x0990, 0x0086},  // AWB_STEADY_BGAIN_OUT_MAX   
		{0x098C, 0xA35F},  // MCU_ADDRESS                
		{0x0990, 0x007E},  // AWB_STEADY_BGAIN_IN_MIN    
		{0x098C, 0xA360},  // MCU_ADDRESS                
		{0x0990, 0x0082},  // AWB_STEADY_BGAIN_IN_MAX
		{0x098C, 0xA302},  // MCU_ADDRESS                
		{0x0990, 0x0000},  // AWB_WINDOW_POS             
		{0x098C, 0xA303},  // MCU_ADDRESS                
		{0x0990, 0x00EF},  // AWB_WINDOW_SIZE            
		//.. . ..(RED higher)
		{0x098C, 0xA364},  // MCU_ADDRESS // 20091217 add (110721 cayman newly updated)
		{0x0990, 0x00E4},  // MCU_DATA_0
		{0x098C, 0xA365},  // MCU_ADDRESS 
		{0x0990, 0x0000},  // AWB_X0 <-0x0010
		{0x098C, 0xA366},  // MCU_ADDRESS 
		{0x0990, 0x0080},  // AWB_KR_L
		{0x098C, 0xA367},  // MCU_ADDRESS 
		{0x0990, 0x0080},  // AWB_KG_L
		{0x098C, 0xA368},  // MCU_ADDRESS 
		{0x0990, 0x0080},  // AWB_KB_L
		{0x098C, 0xA369},  // MCU_ADDRESS 
		{0x0990, 0x0083},  // 8a AWB_KR_R <-0x0082  (110721 cayman updated)
		{0x098C, 0xA36A},  // MCU_ADDRESS 
		{0x0990, 0x0082},  // AWB_KG_R
		{0x098C, 0xA36B},  // MCU_ADDRESS 
		{0x0990, 0x007c},  // 82 AWB_KB_R  (110721 cayman updated)
		//[LL(Low Light) setting & NR(Noise Reduction)] 
		{0x098C, 0xAB1F},  // MCU_ADDRESS        
		{0x0990, 0x00C6},  // RESERVED_HG_1F                         
		{0x098C, 0xAB20},  // MCU_ADDRESS        
		{0x0990, 0x0060},  // RESERVED_HG_20(maximum saturation)(080731) 80->43                        
		{0x098C, 0xAB21},  // MCU_ADDRESS        
		{0x0990, 0x001F},  // RESERVED_HG_21                    
		{0x098C, 0xAB22},  // MCU_ADDRESS        
		{0x0990, 0x0003},  // RESERVED_HG_22                         
		{0x098C, 0xAB23},  // MCU_ADDRESS        
		{0x0990, 0x0005},  // RESERVED_HG_23                         
		{0x098C, 0xAB24},  // MCU_ADDRESS        
		{0x0990, 0x0030},  // 20 RESERVED_HG_24(minimum saturation)<-0x0030 (080731) 10->00  (110721 cayman updated) 
		{0x098C, 0xAB25},  // MCU_ADDRESS        
		{0x0990, 0x0060},  //35,  // RESERVED_HG_25(noise filter)<-0x0014                      
		{0x098C, 0xAB26},  // MCU_ADDRESS        
		{0x0990, 0x0000},  // RESERVED_HG_26                         
		{0x098C, 0xAB27},  // MCU_ADDRESS        
		{0x0990, 0x0006},  // RESERVED_HG_27                         
		{0x098C, 0x2B28},  // MCU_ADDRESS 
		{0x0990, 0x1800},  // HG_LL_BRIGHTNESSSTART <-0x1388                         
		{0x098C, 0x2B2A},  // MCU_ADDRESS  
		{0x0990, 0x3000},  // HG_LL_BRIGHTNESSSTOP <-0x4E20                        
		{0x098C, 0xAB2C},  // MCU_ADDRESS        
		{0x0990, 0x0006},  // RESERVED_HG_2C                       
		{0x098C, 0xAB2D},  // MCU_ADDRESS        
		{0x0990, 0x000A},  // RESERVED_HG_2D                         
		{0x098C, 0xAB2E},  // MCU_ADDRESS        
		{0x0990, 0x0006},  // RESERVED_HG_2E                         
		{0x098C, 0xAB2F},  // MCU_ADDRESS        
		{0x0990, 0x0006},  // RESERVED_HG_2F                       
		{0x098C, 0xAB30},  // MCU_ADDRESS        
		{0x0990, 0x001E},  // RESERVED_HG_30                         
		{0x098C, 0xAB31},  // MCU_ADDRESS        
		{0x0990, 0x000E},  // RESERVED_HG_31                        
		{0x098C, 0xAB32},  // MCU_ADDRESS        
		{0x0990, 0x001E},  // RESERVED_HG_32                    
		{0x098C, 0xAB33},  // MCU_ADDRESS        
		{0x0990, 0x001E},  // RESERVED_HG_33                      
		{0x098C, 0xAB34},  // MCU_ADDRESS        
		{0x0990, 0x0008},  // RESERVED_HG_34                     
		{0x098C, 0xAB35},  // MCU_ADDRESS        
		{0x0990, 0x0080},  // RESERVED_HG_35

		//[AE WINDOW SIZE POS CHANGE-CENTER]window wider 080827
		{0x098C, 0xA202},    // MCU_ADDRESS [AE_WINDOW_POS]  (080731) AE window change
		{0x0990, 0x0021},    //0x0043,    // MCU_DATA_0
		{0x098C, 0xA203},    // MCU_ADDRESS [AE_WINDOW_SIZE]
		{0x0990, 0x00dd},       //0x00B9,    // MCU_DATA_0
		                                                                                            
		//[20070806 tuned]
		{0x098C, 0xA11D},  // MCU_ADDRESS                
		{0x0990, 0x0002},  // SEQ_PREVIEW_1_AE                                                                                                                 
		{0x098C, 0xA208},                               
		{0x0990, 0x0003},         // 080723  AE speed ..  0x0004  (080731 speed ... 1->3)
		{0x098C, 0xA209},                               
		{0x0990, 0x0002},                               
		{0x098C, 0xA20A},                               
		{0x0990, 0x001F},                               
		{0x098C, 0xA216},                               
		{0x0990, 0x003A},
		{0x098C, 0xA244},  // MCU_ADDRESS                
		{0x0990, 0x0008},  // RESERVED_AE_44                                                              
		{0x098C, 0xA24F},  // MCU_ADDRESS  
		{0x0990, 0x0042}, // 080723 AE target 0x0045,  // AE_BASETARGET <-0x004A                        
		{0x098C, 0xA207},  // MCU_ADDRESS                                
		{0x0990, 0x000A},  // AE_GATE                        
		{0x098C, 0xA20D},  // MCU_ADDRESS                               
		{0x0990, 0x0020},  // AE_MinVirtGain(minimum allowed virtual gain)                       
		{0x098C, 0xA20E},  // MCU_ADDRESS                              // 080723 ... Gain
		{0x0990, 0x0080},  // a0->80  AE_MaxVirtGain(maximum allowed virtual gain)
		{0x098C, 0xAB04},                        
		{0x0990, 0x0014},                               
		{0x098C, 0x2361},  // protect the WB hunting                               
		{0x0990, 0x0a00},  // <-0x00X0                          
		{0x3244, 0x0310},                               
		//2008/01/16 Dgain higher
		{0x098C, 0x2212},  // MCU_ADDRESS          
		{0x0990, 0x00F0},  // RESERVED_AE_12(default:0x0080)  
		                                                                                                   
		//[Edge]                                                                                              
		{0x326C, 0x1305},                               
		{0x098C, 0xAB22},  // MCU_ADDRESS                                   
		{0x0990, 0x0003},  // MCU_DATA_0[RESERVED_HG_22]...  

		//[Mirror &flip]
		//[Mirror &flip_SKT]
		{0x098C, 0x2717}, 	// MCU_ADDRESS [MODE_SENSOR_READ_MODE_A]
		{0x0990, 0x0025},	/* LGE_CHANGE [donghyun.kwon@lge.com] 20111109 : MCU_DATA_0, Flip = OFF, Mirror = ON */
		//	{0x0990, 0x0026}, 	/* LGE_CHANGE : 2011-12-13, donghyun.kwon@lge.com : [INTERNAL TD : 106163] rotate 180 degree for captured image,  Flip = ON, Mirror = ON*/
		{0x098C, 0x272D}, 	// MCU_ADDRESS [MODE_SENSOR_READ_MODE_B]
		{0x0990, 0x0025},	/* LGE_CHANGE [donghyun.kwon@lge.com] 20111109 : MCU_DATA_0, Flip = OFF, Mirror = ON */
		//	{0x0990, 0x0026}, 	/* LGE_CHANGE : 2011-12-13, donghyun.kwon@lge.com : [INTERNAL TD : 106163] rotate 180 degree for captured image,  Flip = ON, Mirror = ON*/

		//[ min frame]
		{0x098C, 0xA20C}, 	// MCU_ADDRESS [AE_MAX_INDEX]
		{0x0990, 0x0009}, 	// MCU_DATA_0
		{0x098C, 0xA20B}, 	// MCU_ADDRESS [AE_MIN_INDEX]
		{0x0990, 0x0000}, 	// MCU_DATA_0

#if 1
		{0x098C, 0xA207}, 	// MCU_ADDRESS [AE_MIN_INDEX]
		{0x0990, 0x0004}, 	// MCU_DATA_0

		{0x098C, 0xA75D}, 	// MCU_ADDRESS [AE_MIN_INDEX]
		{0x0990, 0x0000}, 	// MCU_DATA_0
#endif
	},
	// antibanding 50Hz
	{	
		{0x0018, 0x4028}, // STANDBY_CONTROL  
		{0x0018, 0x4000},
		{0x301A, 0x0004},
		{0x001A, 0x0013}, // RESET_AND_MISC_CONTROL 																				 //DELAY=10 		  
		{0xFFFF, 0x000A},																						   
		{0x001A, 0x0010}, // RESET_AND_MISC_CONTROL 																																								  
		//DELAY=10			 
		{0xFFFF, 0x000A},																																															 
		{0x0018, 0x4028}, // STANDBY_CONTROL   
		{0x0018, 0x4000},
		{0x301A, 0x0004},
		//reduce_IO
		{0x098C, 0x02F0},	// MCU_ADDRESS
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0x02F2},	// MCU_ADDRESS
		{0x0990, 0x0210},	// MCU_DATA_0
		{0x098C, 0x02F4},	// MCU_ADDRESS
		{0x0990, 0x001A},	// MCU_DATA_0
		{0x098C, 0x2145},	// MCU_ADDRESS
		{0x0990, 0x02F4},	// MCU_DATA_0
		{0x098C, 0xA134},	// MCU_ADDRESS
		{0x0990, 0x0001},	// MCU_DATA_0
		{0x31E0, 0x0001},	// CORE_31E0
		{0xFFFF, 0x000A},
		{0x3400, 0x783C},	// MIPI_CONTROL 
		{0x001A, 0x0210},	// RESET_AND_MISC_CONTROL
		//{0x3400, 0x7A34}, 	// MIPI_CONTROL
		//{0x321C, 0x0003}, 	// OFIFO_CONTROL_STATUS
		{0x001E, 0x0400},	// PAD_SLEW
		{0x0016, 0x42DF},	// CLOCKS_CONTROL

		//pll
		{0x0014, 0x2145},	// PLL_CONTROL
		{0x0014, 0x2145},	// PLL_CONTROL
		{0x0010, 0x021C},	// PLL_DIVIDERS
		{0x0012, 0x0000},	// PLL_P_DIVIDERS
		{0x0014, 0x244B},	// PLL_CONTROL
		{0xFFFF, 0x000A},	// DELAY=10 

		{0x0014, 0x304B},	// PLL_CONTROL
		{0xFFFF, 0x0032},	// DELAY=50

		//{0x0014, 0x8000},
		//POLL_REG=0x0014, 0x8000, ==0, DELAY=50, TIMEOUT=20 // Verify PLL lock //DELAY=10

		{0x0014, 0xB04A},	// PLL_CONTROL							
		{0xFFFF, 0x0012}, 

		{0x3400, 0x7A3C},	// MIPI_CONTROL

		{0x001A, 0x0010},	
		{0x001A, 0x0018},	
		{0x321C, 0x0003},	// OFIFO_CONTROL_STATUS 				  

		{0x098C, 0x2703},	// MCU_ADDRESS
		{0x0990, 0x0280},	// MCU_DATA_0
		{0x098C, 0x2705},	// MCU_ADDRESS
		{0x0990, 0x01E0},	// MCU_DATA_0
		{0x098C, 0x2707},	// MCU_ADDRESS
		{0x0990, 0x0280},	// MCU_DATA_0
		{0x098C, 0x2709},	// MCU_ADDRESS
		{0x0990, 0x01E0},	// MCU_DATA_0
		{0x098C, 0x270D},	// MCU_ADDRESS
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0x270F},	// MCU_ADDRESS
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0x2711},	// MCU_ADDRESS
		{0x0990, 0x01E7},	// MCU_DATA_0
		{0x098C, 0x2713},	// MCU_ADDRESS
		{0x0990, 0x0287},	// MCU_DATA_0
		{0x098C, 0x2715},	// MCU_ADDRESS
		{0x0990, 0x0001},	// MCU_DATA_0
		{0x098C, 0x2717},	// MCU_ADDRESS
		{0x0990, 0x0026},	// MCU_DATA_0
		{0x098C, 0x2719},	// MCU_ADDRESS
		{0x0990, 0x001A},	// MCU_DATA_0
		{0x098C, 0x271B},	// MCU_ADDRESS
		{0x0990, 0x006B},	// MCU_DATA_0
		{0x098C, 0x271D},	// MCU_ADDRESS
		{0x0990, 0x006B},	// MCU_DATA_0
		{0x098C, 0x271F},	// MCU_ADDRESS
		{0x0990, 0x01FB},	// MCU_DATA_0
		{0x098C, 0x2721},	// MCU_ADDRESS
		{0x0990, 0x0398},	// MCU_DATA_0
		{0x098C, 0x2723},	// MCU_ADDRESS
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0x2725},	// MCU_ADDRESS
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0x2727},	// MCU_ADDRESS
		{0x0990, 0x01E7},	// MCU_DATA_0
		{0x098C, 0x2729},	// MCU_ADDRESS
		{0x0990, 0x0287},	// MCU_DATA_0
		{0x098C, 0x272B},	// MCU_ADDRESS
		{0x0990, 0x0001},	// MCU_DATA_0
		{0x098C, 0x272D},	// MCU_ADDRESS
		{0x0990, 0x0026},	// MCU_DATA_0
		{0x098C, 0x272F},	// MCU_ADDRESS
		{0x0990, 0x001A},	// MCU_DATA_0
		{0x098C, 0x2731},	// MCU_ADDRESS
		{0x0990, 0x006B},	// MCU_DATA_0
		{0x098C, 0x2733},	// MCU_ADDRESS
		{0x0990, 0x006B},	// MCU_DATA_0
		{0x098C, 0x2735},	// MCU_ADDRESS
		{0x0990, 0x01FB},	// MCU_DATA_0
		{0x098C, 0x2737},	// MCU_ADDRESS
		{0x0990, 0x0398},	// MCU_DATA_0
		{0x098C, 0x2739},	// MCU_ADDRESS
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0x273B},	// MCU_ADDRESS
		{0x0990, 0x027F},	// MCU_DATA_0
		{0x098C, 0x273D},	// MCU_ADDRESS
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0x273F},	// MCU_ADDRESS
		{0x0990, 0x01DF},	// MCU_DATA_0
		{0x098C, 0x2747},	// MCU_ADDRESS
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0x2749},	// MCU_ADDRESS
		{0x0990, 0x027F},	// MCU_DATA_0
		{0x098C, 0x274B},	// MCU_ADDRESS
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0x274D},	// MCU_ADDRESS
		{0x0990, 0x01DF},	// MCU_DATA_0
		{0x098C, 0x222D},	// MCU_ADDRESS
		{0x0990, 0x007F},	// MCU_DATA_0
		{0x098C, 0xA408},	// MCU_ADDRESS
		{0x0990, 0x001E},	// MCU_DATA_0
		{0x098C, 0xA409},	// MCU_ADDRESS
		{0x0990, 0x0020},	// MCU_DATA_0
		{0x098C, 0xA40A},	// MCU_ADDRESS
		{0x0990, 0x0025},	// MCU_DATA_0
		{0x098C, 0xA40B},	// MCU_ADDRESS
		{0x0990, 0x0027},	// MCU_DATA_0
		{0x098C, 0x2411},	// MCU_ADDRESS
		{0x0990, 0x007F},	// MCU_DATA_0
		{0x098C, 0x2413},	// MCU_ADDRESS
		{0x0990, 0x0098},	// MCU_DATA_0
		{0x098C, 0x2415},	// MCU_ADDRESS
		{0x0990, 0x007F},	// MCU_DATA_0
		{0x098C, 0x2417},	// MCU_ADDRESS
		{0x0990, 0x0098},	// MCU_DATA_0
		{0x098C, 0xA404},	// MCU_ADDRESS
		{0x0990, 0x0070},	// MCU_DATA_0
		{0x098C, 0xA40D},	// MCU_ADDRESS
		{0x0990, 0x0002},	// MCU_DATA_0
		{0x098C, 0xA40E},	// MCU_ADDRESS
		{0x0990, 0x0003},	// MCU_DATA_0
		{0x098C, 0xA410},	// MCU_ADDRESS
		{0x0990, 0x000A},	// MCU_DATA_0

		//[Gamma]
		//0x098C, 0xAB37,  // MCU_ADDRESS	 // 080723 ... .. .. .....
		//0x0990, 0x0001,  // gamma_table_A_0	// 080723 ... .. .. .....
		{0x098C, 0xAB37},  // MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
		{0x0990, 0x0001}, // MCU_DATA_0
		{0x098C, 0x2B38},  // MCU_ADDRESS [HG_GAMMASTARTMORPH]
		{0x0990, 0x1000},  // MCU_DATA_0
		{0x098C, 0x2B3A},  // MCU_ADDRESS [HG_GAMMASTOPMORPH]
		{0x0990, 0x2000},  // MCU_DATA_0
		{0x098C, 0xAB3C},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0xAB3D},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
		{0x0990, 0x0017},	// MCU_DATA_0
		{0x098C, 0xAB3E},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
		{0x0990, 0x0028},	// MCU_DATA_0
		{0x098C, 0xAB3F},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
		{0x0990, 0x003D},	// MCU_DATA_0
		{0x098C, 0xAB40},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
		{0x0990, 0x005B},	// MCU_DATA_0
		{0x098C, 0xAB41},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
		{0x0990, 0x0074},	// MCU_DATA_0
		{0x098C, 0xAB42},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
		{0x0990, 0x0089},	// MCU_DATA_0
		{0x098C, 0xAB43},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
		{0x0990, 0x009B},	// MCU_DATA_0
		{0x098C, 0xAB44},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
		{0x0990, 0x00AA},	// MCU_DATA_0
		{0x098C, 0xAB45},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
		{0x0990, 0x00B7},	// MCU_DATA_0
		{0x098C, 0xAB46},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
		{0x0990, 0x00C3},	// MCU_DATA_0
		{0x098C, 0xAB47},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
		{0x0990, 0x00CD},	// MCU_DATA_0
		{0x098C, 0xAB48},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
		{0x0990, 0x00D6},	// MCU_DATA_0
		{0x098C, 0xAB49},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
		{0x0990, 0x00DE},	// MCU_DATA_0
		{0x098C, 0xAB4A},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
		{0x0990, 0x00E6},	// MCU_DATA_0
		{0x098C, 0xAB4B},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
		{0x0990, 0x00ED},	// MCU_DATA_0
		{0x098C, 0xAB4C},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
		{0x0990, 0x00F3},	// MCU_DATA_0
		{0x098C, 0xAB4D},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
		{0x0990, 0x00F9},	// MCU_DATA_0
		{0x098C, 0xAB4E},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
		{0x0990, 0x00FF},	// MCU_DATA_0
		{0x098C, 0xAB3C},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
		{0x0990, 0x0000},	// MCU_DATA_0
		{0x098C, 0xAB3D},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
		{0x0990, 0x0017},	// MCU_DATA_0
		{0x098C, 0xAB3E},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
		{0x0990, 0x0028},	// MCU_DATA_0
		{0x098C, 0xAB3F},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
		{0x0990, 0x003D},	// MCU_DATA_0
		{0x098C, 0xAB40},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
		{0x0990, 0x005B},	// MCU_DATA_0
		{0x098C, 0xAB41},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
		{0x0990, 0x0074},	// MCU_DATA_0
		{0x098C, 0xAB42},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
		{0x0990, 0x0089},	// MCU_DATA_0
		{0x098C, 0xAB43},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
		{0x0990, 0x009B},	// MCU_DATA_0
		{0x098C, 0xAB44},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
		{0x0990, 0x00AA},	// MCU_DATA_0
		{0x098C, 0xAB45},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
		{0x0990, 0x00B7},	// MCU_DATA_0
		{0x098C, 0xAB46},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
		{0x0990, 0x00C3},	// MCU_DATA_0
		{0x098C, 0xAB47},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
		{0x0990, 0x00CD},	// MCU_DATA_0
		{0x098C, 0xAB48},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
		{0x0990, 0x00D6},	// MCU_DATA_0
		{0x098C, 0xAB49},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
		{0x0990, 0x00DE},	// MCU_DATA_0
		{0x098C, 0xAB4A},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
		{0x0990, 0x00E6},	// MCU_DATA_0
		{0x098C, 0xAB4B},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
		{0x0990, 0x00ED},	// MCU_DATA_0
		{0x098C, 0xAB4C},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
		{0x0990, 0x00F3},	// MCU_DATA_0
		{0x098C, 0xAB4D},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
		{0x0990, 0x00F9},	// MCU_DATA_0
		{0x098C, 0xAB4E},	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
		{0x0990, 0x00FF},	// MCU_DATA_0
		  
		//[Lens Correction 85]					   
		{0x3658, 0x0130},  //  P_RD_P0Q0	
		{0x365A, 0x030D},  //  P_RD_P0Q1	
		{0x365C, 0x6B92},  //  P_RD_P0Q2	
		{0x365E, 0xE62E},  //  P_RD_P0Q3	
		{0x3660, 0x53B4},  //  P_RD_P0Q4	
		{0x3680, 0x1AED},  //  P_RD_P1Q0	
		{0x3682, 0x2A6A},  //  P_RD_P1Q1	
		{0x3684, 0xBA2B},  //  P_RD_P1Q2	
		{0x3686, 0x392F},  //  P_RD_P1Q3	
		{0x3688, 0xAD53},  //  P_RD_P1Q4	
		{0x36A8, 0x1453},  //  P_RD_P2Q0	
		{0x36AA, 0xC78D},  //  P_RD_P2Q1	
		{0x36AC, 0x4C35},  //  P_RD_P2Q2	
		{0x36AE, 0x7791},  //  P_RD_P2Q3	
		{0x36B0, 0x6797},  //  P_RD_P2Q4	
		{0x36D0, 0xE3F0},  //  P_RD_P3Q0	
		{0x36D2, 0xF8F0},  //  P_RD_P3Q1	
		{0x36D4, 0x5934},  //  P_RD_P3Q2	
		{0x36D6, 0x6AF2},  //  P_RD_P3Q3	
		{0x36D8, 0xFC98},  //  P_RD_P3Q4	
		{0x36F8, 0x2EB4},  //  P_RD_P4Q0	
		{0x36FA, 0x9972},  //  P_RD_P4Q1	
		{0x36FC, 0x5E33},  //  P_RD_P4Q2	
		{0x36FE, 0x07D8},  //  P_RD_P4Q3	
		{0x3700, 0x51DC},  //  P_RD_P4Q4	
		{0x364E, 0x0170},  //  P_GR_P0Q0	
		{0x3650, 0x570C},  //  P_GR_P0Q1	
		{0x3652, 0x6652},  //  P_GR_P0Q2	
		{0x3654, 0xB5B0},  //  P_GR_P0Q3	
		{0x3656, 0x3274},  //  P_GR_P0Q4	
		{0x3676, 0x214D},  //  P_GR_P1Q0	
		{0x3678, 0x3D46},  //  P_GR_P1Q1	
		{0x367A, 0x218F},  //  P_GR_P1Q2	
		{0x367C, 0x13D0},  //  P_GR_P1Q3	
		{0x367E, 0x8C34},  //  P_GR_P1Q4	
		{0x369E, 0x1253},  //  P_GR_P2Q0	
		{0x36A0, 0x98B0},  //  P_GR_P2Q1	
		{0x36A2, 0x3275},  //  P_GR_P2Q2	
		{0x36A4, 0x4FD0},  //  P_GR_P2Q3	
		{0x36A6, 0x0896},  //  P_GR_P2Q4	
		{0x36C6, 0x82D0},  //  P_GR_P3Q0	
		{0x36C8, 0x8292},  //  P_GR_P3Q1	
		{0x36CA, 0x0CD5},  //  P_GR_P3Q2	
		{0x36CC, 0x2632},  //  P_GR_P3Q3	
		{0x36CE, 0xC659},  //  P_GR_P3Q4	
		{0x36EE, 0x2374},  //  P_GR_P4Q0	
		{0x36F0, 0xDC32},  //  P_GR_P4Q1	
		{0x36F2, 0xCBF7},  //  P_GR_P4Q2	
		{0x36F4, 0x1918},  //  P_GR_P4Q3	
		{0x36F6, 0x675C},  //  P_GR_P4Q4	
		{0x3662, 0x00D0},  //  P_BL_P0Q0	
		{0x3664, 0x67AC},  //  P_BL_P0Q1	
		{0x3666, 0x6CD2},  //  P_BL_P0Q2	
		{0x3668, 0xB350},  //  P_BL_P0Q3	
		{0x366A, 0x0CD4},  //  P_BL_P0Q4	
		{0x368A, 0x400C},  //  P_BL_P1Q0	
		{0x368C, 0x07ED},  //  P_BL_P1Q1	
		{0x368E, 0x57CF},  //  P_BL_P1Q2	
		{0x3690, 0x3C50},  //  P_BL_P1Q3	
		{0x3692, 0xD654},  //  P_BL_P1Q4	
		{0x36B2, 0x0213},  //  P_BL_P2Q0	
		{0x36B4, 0x2CCF},  //  P_BL_P2Q1	
		{0x36B6, 0x1636},  //  P_BL_P2Q2	
		{0x36B8, 0x8EB5},  //  P_BL_P2Q3	
		{0x36BA, 0xF338},  //  P_BL_P2Q4	
		{0x36DA, 0xD310},  //  P_BL_P3Q0	
		{0x36DC, 0x22B0},  //  P_BL_P3Q1	
		{0x36DE, 0x3273},  //  P_BL_P3Q2	
		{0x36E0, 0x9BF6},  //  P_BL_P3Q3	
		{0x36E2, 0x82D8},  //  P_BL_P3Q4	
		{0x3702, 0x4854},  //  P_BL_P4Q0	
		{0x3704, 0x9AD5},  //  P_BL_P4Q1	
		{0x3706, 0xD519},  //  P_BL_P4Q2	
		{0x3708, 0x5D99},  //  P_BL_P4Q3	
		{0x370A, 0x685D},  //  P_BL_P4Q4	
		{0x366C, 0x00D0},  //  P_GB_P0Q0	
		{0x366E, 0x498C},  //  P_GB_P0Q1	
		{0x3670, 0x6B32},  //  P_GB_P0Q2	
		{0x3672, 0xA910},  //  P_GB_P0Q3	
		{0x3674, 0x3614},  //  P_GB_P0Q4	
		{0x3694, 0x3FAC},  //  P_GB_P1Q0	
		{0x3696, 0xB68A},  //  P_GB_P1Q1	
		{0x3698, 0x0FB0},  //  P_GB_P1Q2	
		{0x369A, 0x5E70},  //  P_GB_P1Q3	
		{0x369C, 0xD6B4},  //  P_GB_P1Q4	
		{0x36BC, 0x09F3},  //  P_GB_P2Q0	
		{0x36BE, 0xB9F1},  //  P_GB_P2Q1	
		{0x36C0, 0x3C55},  //  P_GB_P2Q2	
		{0x36C2, 0x2035},  //  P_GB_P2Q3	
		{0x36C4, 0x5B55},  //  P_GB_P2Q4	
		{0x36E4, 0x9450},  //  P_GB_P3Q0	
		{0x36E6, 0xF171},  //  P_GB_P3Q1	
		{0x36E8, 0x16D4},  //  P_GB_P3Q2	
		{0x36EA, 0x8234},  //  P_GB_P3Q3	
		{0x36EC, 0x98F9},  //  P_GB_P3Q4	
		{0x370C, 0x3854},  //  P_GB_P4Q0	
		{0x370E, 0x3754},  //  P_GB_P4Q1	
		{0x3710, 0xBED7},  //  P_GB_P4Q2	
		{0x3712, 0xF557},  //  P_GB_P4Q3	
		{0x3714, 0x4B7C},  //  P_GB_P4Q4	
		{0x3644, 0x0148},  //  POLY_ORIGIN_C
		{0x3642, 0x00F0},  //  POLY_ORIGIN_R		 
		{0x3210, 0x09B8},  // COLOR_PIPELINE_CONTROL
																										  
		//[AWB and CCMs ..2]																				
		{0x098C, 0x2306},	// MCU_ADDRESS [AWB_CCM_L_0]
		{0x0990, 0x0133},	// MCU_DATA_0
		{0x098C, 0x2308},	// MCU_ADDRESS [AWB_CCM_L_1]
		{0x0990, 0xFFC4},	// MCU_DATA_0
		{0x098C, 0x230A},	// MCU_ADDRESS [AWB_CCM_L_2]
		{0x0990, 0x0014},	// MCU_DATA_0
		{0x098C, 0x230C},	// MCU_ADDRESS [AWB_CCM_L_3]
		{0x0990, 0xFF64},	// MCU_DATA_0
		{0x098C, 0x230E},	// MCU_ADDRESS [AWB_CCM_L_4]
		{0x0990, 0x01E3},	// MCU_DATA_0
		{0x098C, 0x2310},	// MCU_ADDRESS [AWB_CCM_L_5]
		{0x0990, 0xFFB2},	// MCU_DATA_0
		{0x098C, 0x2312},	// MCU_ADDRESS [AWB_CCM_L_6]
		{0x0990, 0xFF9A},	// MCU_DATA_0
		{0x098C, 0x2314},	// MCU_ADDRESS [AWB_CCM_L_7]
		{0x0990, 0xFEDB},	// MCU_DATA_0
		{0x098C, 0x2316},	// MCU_ADDRESS [AWB_CCM_L_8]
		{0x0990, 0x0213},	// MCU_DATA_0
		{0x098C, 0x2318},	// MCU_ADDRESS [AWB_CCM_L_9]
		{0x0990, 0x001C},	// MCU_DATA_0
		{0x098C, 0x231A},	// MCU_ADDRESS [AWB_CCM_L_10]
		{0x0990, 0x003A},	// MCU_DATA_0
		{0x098C, 0x231C},	// MCU_ADDRESS [AWB_CCM_RL_0]
		{0x0990, 0x0064},	// MCU_DATA_0
		{0x098C, 0x231E},	// MCU_ADDRESS [AWB_CCM_RL_1]
		{0x0990, 0xFF7D},	// MCU_DATA_0
		{0x098C, 0x2320},	// MCU_ADDRESS [AWB_CCM_RL_2]
		{0x0990, 0xFFFF},	// MCU_DATA_0
		{0x098C, 0x2322},	// MCU_ADDRESS [AWB_CCM_RL_3]
		{0x0990, 0x001A},	// MCU_DATA_0
		{0x098C, 0x2324},	// MCU_ADDRESS [AWB_CCM_RL_4]
		{0x0990, 0xFF94},	// MCU_DATA_0
		{0x098C, 0x2326},	// MCU_ADDRESS [AWB_CCM_RL_5]
		{0x0990, 0x0048},	// MCU_DATA_0
		{0x098C, 0x2328},	// MCU_ADDRESS [AWB_CCM_RL_6]
		{0x0990, 0x001B},	// MCU_DATA_0
		{0x098C, 0x232A},	// MCU_ADDRESS [AWB_CCM_RL_7]
		{0x0990, 0x0166},	// MCU_DATA_0
		{0x098C, 0x232C},	// MCU_ADDRESS [AWB_CCM_RL_8]
		{0x0990, 0xFEE3},	// MCU_DATA_0
		{0x098C, 0x232E},	// MCU_ADDRESS [AWB_CCM_RL_9]
		{0x0990, 0x0004},	// MCU_DATA_0
		{0x098C, 0x2330},	// MCU_ADDRESS [AWB_CCM_RL_10]
		{0x0990, 0xFFDC},	// MCU_DATA_0			  
		{0x098C, 0xA348},  // MCU_ADDRESS				 
		{0x0990, 0x0008},  // AWB_GAIN_BUFFER_SPEED 	 
		{0x098C, 0xA349},  // MCU_ADDRESS				 
		{0x0990, 0x0002},  // AWB_JUMP_DIVISOR			 
		{0x098C, 0xA34A},  // MCU_ADDRESS				 
		{0x0990, 0x0059},  // AWB_GAINMIN_R 			  
		{0x098C, 0xA34B},  // MCU_ADDRESS				 
		{0x0990, 0x00E6},  // AWB_GAINMAX_R
		{0x098C, 0xA34C},  // MCU_ADDRESS	   
		{0x0990, 0x0059},  // AWB_GAINMIN_B 					   
		{0x098C, 0xA34D},  // MCU_ADDRESS	  
		{0x0990, 0x00A6},  // AWB_GAINMAX_B
		{0x098C, 0xA34E},  // MCU_ADDRESS	  
		{0x0990, 0x0080},  // AWB_GAIN_R
		{0x098C, 0xA34F},  // MCU_ADDRESS	  
		{0x0990, 0x0080},  // AWB_GAIN_G
		{0x098C, 0xA350},  // MCU_ADDRESS	  
		{0x0990, 0x0080},  // AWB_GAIN_B		
		//[AWB MIN POS CHANGE BY DANIEL]
		{0x098C, 0xA351},  // MCU_ADDRESS [AWB_CCM_POSITION_MIN]
		{0x0990, 0x0020},  // MCU_DATA_0   
		{0x098C, 0xA352},  // MCU_ADDRESS				 
		{0x0990, 0x007F},  // AWB_CCM_POSITION_MAX
		{0x098C, 0xA354},  // MCU_ADDRESS				 
		{0x0990, 0x0060},  // AWB_CCM_POSITION
		{0x098C, 0xA354},  // MCU_ADDRESS				 
		{0x0990, 0x0060},  // AWB_SATURATION			 
		{0x098C, 0xA355},  // MCU_ADDRESS				 
		{0x0990, 0x0001},  // AWB_MODE					 
		{0x098C, 0xA35D},  // MCU_ADDRESS				 
		{0x0990, 0x0078},  // AWB_STEADY_BGAIN_OUT_MIN	 
		{0x098C, 0xA35E},  // MCU_ADDRESS				 
		{0x0990, 0x0086},  // AWB_STEADY_BGAIN_OUT_MAX	 
		{0x098C, 0xA35F},  // MCU_ADDRESS				 
		{0x0990, 0x007E},  // AWB_STEADY_BGAIN_IN_MIN	 
		{0x098C, 0xA360},  // MCU_ADDRESS				 
		{0x0990, 0x0082},  // AWB_STEADY_BGAIN_IN_MAX
		{0x098C, 0xA302},  // MCU_ADDRESS				 
		{0x0990, 0x0000},  // AWB_WINDOW_POS			 
		{0x098C, 0xA303},  // MCU_ADDRESS				 
		{0x0990, 0x00EF},  // AWB_WINDOW_SIZE			 
		//.. . ..(RED higher)
		{0x098C, 0xA364},  // MCU_ADDRESS // 20091217 add (110721 cayman newly updated)
		{0x0990, 0x00E4},  // MCU_DATA_0
		{0x098C, 0xA365},  // MCU_ADDRESS 
		{0x0990, 0x0000},  // AWB_X0 <-0x0010
		{0x098C, 0xA366},  // MCU_ADDRESS 
		{0x0990, 0x0080},  // AWB_KR_L
		{0x098C, 0xA367},  // MCU_ADDRESS 
		{0x0990, 0x0080},  // AWB_KG_L
		{0x098C, 0xA368},  // MCU_ADDRESS 
		{0x0990, 0x0080},  // AWB_KB_L
		{0x098C, 0xA369},  // MCU_ADDRESS 
		{0x0990, 0x0083},  // 8a AWB_KR_R <-0x0082	(110721 cayman updated)
		{0x098C, 0xA36A},  // MCU_ADDRESS 
		{0x0990, 0x0082},  // AWB_KG_R
		{0x098C, 0xA36B},  // MCU_ADDRESS 
		{0x0990, 0x007c},  // 82 AWB_KB_R  (110721 cayman updated)
		//[LL(Low Light) setting & NR(Noise Reduction)] 
		{0x098C, 0xAB1F},  // MCU_ADDRESS		 
		{0x0990, 0x00C6},  // RESERVED_HG_1F						 
		{0x098C, 0xAB20},  // MCU_ADDRESS		 
		{0x0990, 0x0060},  // RESERVED_HG_20(maximum saturation)(080731) 80->43 					   
		{0x098C, 0xAB21},  // MCU_ADDRESS		 
		{0x0990, 0x001F},  // RESERVED_HG_21					
		{0x098C, 0xAB22},  // MCU_ADDRESS		 
		{0x0990, 0x0003},  // RESERVED_HG_22						 
		{0x098C, 0xAB23},  // MCU_ADDRESS		 
		{0x0990, 0x0005},  // RESERVED_HG_23						 
		{0x098C, 0xAB24},  // MCU_ADDRESS		 
		{0x0990, 0x0030},  // 20 RESERVED_HG_24(minimum saturation)<-0x0030 (080731) 10->00  (110721 cayman updated) 
		{0x098C, 0xAB25},  // MCU_ADDRESS		 
		{0x0990, 0x0060},  //35,  // RESERVED_HG_25(noise filter)<-0x0014					   
		{0x098C, 0xAB26},  // MCU_ADDRESS		 
		{0x0990, 0x0000},  // RESERVED_HG_26						 
		{0x098C, 0xAB27},  // MCU_ADDRESS		 
		{0x0990, 0x0006},  // RESERVED_HG_27						 
		{0x098C, 0x2B28},  // MCU_ADDRESS 
		{0x0990, 0x1800},  // HG_LL_BRIGHTNESSSTART <-0x1388						 
		{0x098C, 0x2B2A},  // MCU_ADDRESS  
		{0x0990, 0x3000},  // HG_LL_BRIGHTNESSSTOP <-0x4E20 					   
		{0x098C, 0xAB2C},  // MCU_ADDRESS		 
		{0x0990, 0x0006},  // RESERVED_HG_2C					   
		{0x098C, 0xAB2D},  // MCU_ADDRESS		 
		{0x0990, 0x000A},  // RESERVED_HG_2D						 
		{0x098C, 0xAB2E},  // MCU_ADDRESS		 
		{0x0990, 0x0006},  // RESERVED_HG_2E						 
		{0x098C, 0xAB2F},  // MCU_ADDRESS		 
		{0x0990, 0x0006},  // RESERVED_HG_2F					   
		{0x098C, 0xAB30},  // MCU_ADDRESS		 
		{0x0990, 0x001E},  // RESERVED_HG_30						 
		{0x098C, 0xAB31},  // MCU_ADDRESS		 
		{0x0990, 0x000E},  // RESERVED_HG_31						
		{0x098C, 0xAB32},  // MCU_ADDRESS		 
		{0x0990, 0x001E},  // RESERVED_HG_32					
		{0x098C, 0xAB33},  // MCU_ADDRESS		 
		{0x0990, 0x001E},  // RESERVED_HG_33					  
		{0x098C, 0xAB34},  // MCU_ADDRESS		 
		{0x0990, 0x0008},  // RESERVED_HG_34					 
		{0x098C, 0xAB35},  // MCU_ADDRESS		 
		{0x0990, 0x0080},  // RESERVED_HG_35

		//[AE WINDOW SIZE POS CHANGE-CENTER]window wider 080827
		{0x098C, 0xA202},	 // MCU_ADDRESS [AE_WINDOW_POS]  (080731) AE window change
		{0x0990, 0x0021},	 //0x0043,	  // MCU_DATA_0
		{0x098C, 0xA203},	 // MCU_ADDRESS [AE_WINDOW_SIZE]
		{0x0990, 0x00dd},		//0x00B9,	 // MCU_DATA_0
																									
		//[20070806 tuned]
		{0x098C, 0xA11D},  // MCU_ADDRESS				 
		{0x0990, 0x0002},  // SEQ_PREVIEW_1_AE																												   
		{0x098C, 0xA208},								
		{0x0990, 0x0003},		  // 080723  AE speed ..  0x0004  (080731 speed ... 1->3)
		{0x098C, 0xA209},								
		{0x0990, 0x0002},								
		{0x098C, 0xA20A},								
		{0x0990, 0x001F},								
		{0x098C, 0xA216},								
		{0x0990, 0x003A},
		{0x098C, 0xA244},  // MCU_ADDRESS				 
		{0x0990, 0x0008},  // RESERVED_AE_44															  
		{0x098C, 0xA24F},  // MCU_ADDRESS  
		{0x0990, 0x0042}, // 080723 AE target 0x0045,  // AE_BASETARGET <-0x004A						
		{0x098C, 0xA207},  // MCU_ADDRESS								 
		{0x0990, 0x000A},  // AE_GATE						 
		{0x098C, 0xA20D},  // MCU_ADDRESS								
		{0x0990, 0x0020},  // AE_MinVirtGain(minimum allowed virtual gain)						 
		{0x098C, 0xA20E},  // MCU_ADDRESS							   // 080723 ... Gain
		{0x0990, 0x0080},  // a0->80  AE_MaxVirtGain(maximum allowed virtual gain)
		{0x098C, 0xAB04},						 
		{0x0990, 0x0014},								
		{0x098C, 0x2361},  // protect the WB hunting							   
		{0x0990, 0x0a00},  // <-0x00X0							
		{0x3244, 0x0310},								
		//2008/01/16 Dgain higher
		{0x098C, 0x2212},  // MCU_ADDRESS		   
		{0x0990, 0x00F0},  // RESERVED_AE_12(default:0x0080)  
																										   
		//[Edge]																							  
		{0x326C, 0x1305},								
		{0x098C, 0xAB22},  // MCU_ADDRESS									
		{0x0990, 0x0003},  // MCU_DATA_0[RESERVED_HG_22]...  

		//[Mirror &flip]
		//[Mirror &flip_SKT]
		{0x098C, 0x2717},	// MCU_ADDRESS [MODE_SENSOR_READ_MODE_A]
		{0x0990, 0x0025},	/* LGE_CHANGE [donghyun.kwon@lge.com] 20111109 : MCU_DATA_0, Flip = OFF, Mirror = ON */
		//	{0x0990, 0x0026},	/* LGE_CHANGE : 2011-12-13, donghyun.kwon@lge.com : [INTERNAL TD : 106163] rotate 180 degree for captured image,  Flip = ON, Mirror = ON*/
		{0x098C, 0x272D},	// MCU_ADDRESS [MODE_SENSOR_READ_MODE_B]
		{0x0990, 0x0025},	/* LGE_CHANGE [donghyun.kwon@lge.com] 20111109 : MCU_DATA_0, Flip = OFF, Mirror = ON */
		//	{0x0990, 0x0026},	/* LGE_CHANGE : 2011-12-13, donghyun.kwon@lge.com : [INTERNAL TD : 106163] rotate 180 degree for captured image,  Flip = ON, Mirror = ON*/

		//[ min frame]
		{0x098C, 0xA20C},	// MCU_ADDRESS [AE_MAX_INDEX]
		{0x0990, 0x0009},	// MCU_DATA_0
		{0x098C, 0xA20B},	// MCU_ADDRESS [AE_MIN_INDEX]
		{0x0990, 0x0000},	// MCU_DATA_0

#if 1
		{0x098C, 0xA207},	// MCU_ADDRESS [AE_MIN_INDEX]
		{0x0990, 0x0004},	// MCU_DATA_0

		{0x098C, 0xA75D},	// MCU_ADDRESS [AE_MIN_INDEX]
		{0x0990, 0x0000},	// MCU_DATA_0
#endif
	}
};
static struct msm_camera_i2c_reg_conf refreshmode_sub[] = {
    {0x098C, 0xA103},                                 
    {0x0990, 0x0006}, 
};
static struct msm_camera_i2c_reg_conf refresh_sub[] =
{   
    {0x098C, 0xA103},                                 
    {0x0990, 0x0005}, 
};

static struct v4l2_subdev_info mt9v113_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8, /* For YUV type sensor (YUV422) */
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order  = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array mt9v113_init_conf[] = {
	{&mt9v113_recommend_settings[MT9V113_60HZ][0],
	ARRAY_SIZE(mt9v113_recommend_settings[MT9V113_60HZ]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9v113_recommend_settings[MT9V113_50HZ][0],
	ARRAY_SIZE(mt9v113_recommend_settings[MT9V113_50HZ]), 0, MSM_CAMERA_I2C_WORD_DATA}
};

static struct msm_camera_i2c_conf_array mt9v113_confs[] = {
	{&mt9v113_prev_settings[0],
	ARRAY_SIZE(mt9v113_prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_sensor_output_info_t mt9v113_dimensions[] = {
	{
		.x_output = 0x280,
		.y_output = 0x1E0,
		.line_length_pclk = 0xD06,
		.frame_length_lines = 0x04ED,
		.vt_pixel_clk = 45600000,
		.op_pixel_clk = 45600000,
	},
};
static struct msm_camera_i2c_reg_conf mt9v113_exposure[][2] = {
// need to check
	{{0x098C, 0xA24F}, {0x0990, 0x001E},}, /*EXPOSURECOMPENSATIONN6*/
	{{0x098C, 0xA24F}, {0x0990, 0x0021},}, /*EXPOSURECOMPENSATIONN5*/
	{{0x098C, 0xA24F}, {0x0990, 0x0024},}, /*EXPOSURECOMPENSATIONN4*/
	{{0x098C, 0xA24F}, {0x0990, 0x0027},}, /*EXPOSURECOMPENSATIONN3*/
	{{0x098C, 0xA24F}, {0x0990, 0x002D},}, /*EXPOSURECOMPENSATIONN2*/
	{{0x098C, 0xA24F}, {0x0990, 0x002A},}, /*EXPOSURECOMPENSATIONN1*/
	{{0x098C, 0xA24F}, {0x0990, 0x0030},}, /*EXPOSURECOMPENSATIOND*/
	{{0x098C, 0xA24F}, {0x0990, 0x0033},}, /*EXPOSURECOMPENSATIONp1*/
	{{0x098C, 0xA24F}, {0x0990, 0x0036},}, /*EXPOSURECOMPENSATIONp2*/
	{{0x098C, 0xA24F}, {0x0990, 0x0039},}, /*EXPOSURECOMPENSATIONp3*/
	{{0x098C, 0xA24F}, {0x0990, 0x003C},}, /*EXPOSURECOMPENSATIONp4*/
	{{0x098C, 0xA24F}, {0x0990, 0x003F},}, /*EXPOSURECOMPENSATIONp5*/
	{{0x098C, 0xA24F}, {0x0990, 0x0042},}, /*EXPOSURECOMPENSATIONp6*/
};
// static int brightness_table[] = {0x000F, 0x0017, 0x0020, 0x0028, 0x0031, 0x0039, 0x0042, 0x004B, 0x0053, 0x005C, 0x0064, 0x006D, 0x0075};
// static int brightness_table[] = {0x001E, 0x0021, 0x0024, 0x0027, 0x002A, 0x002D, 0x0030, 0x0033, 0x0036, 0x0039, 0x003C, 0x003F, 0x0042, 0x0045, 0x0048, 0x004B, 0x004E, 0x0051, 0x0054, 0x0057, 0x005A, 0x005D, 0x0060, 0x0063, 0x0066};
static struct msm_camera_i2c_conf_array mt9v113_exposure_confs[][1] = {
	{{mt9v113_exposure[0], ARRAY_SIZE(mt9v113_exposure[0]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[1], ARRAY_SIZE(mt9v113_exposure[1]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[2], ARRAY_SIZE(mt9v113_exposure[2]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[3], ARRAY_SIZE(mt9v113_exposure[3]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[4], ARRAY_SIZE(mt9v113_exposure[4]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[5], ARRAY_SIZE(mt9v113_exposure[5]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[6], ARRAY_SIZE(mt9v113_exposure[6]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[7], ARRAY_SIZE(mt9v113_exposure[7]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[8], ARRAY_SIZE(mt9v113_exposure[8]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[9], ARRAY_SIZE(mt9v113_exposure[9]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[10], ARRAY_SIZE(mt9v113_exposure[10]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[11], ARRAY_SIZE(mt9v113_exposure[11]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[12], ARRAY_SIZE(mt9v113_exposure[12]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_exposure[13], ARRAY_SIZE(mt9v113_exposure[13]), 0,
		MSM_CAMERA_I2C_WORD_DATA},},
};

static int mt9v113_exposure_enum_map[] = {
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

static struct msm_camera_i2c_enum_conf_array mt9v113_exposure_enum_confs = {
	.conf = &mt9v113_exposure_confs[0][0],
	.conf_enum = mt9v113_exposure_enum_map,
	.num_enum = ARRAY_SIZE(mt9v113_exposure_enum_map),
	.num_index = ARRAY_SIZE(mt9v113_exposure_confs),
	.num_conf = ARRAY_SIZE(mt9v113_exposure_confs[0]),
	.data_type = MSM_CAMERA_I2C_WORD_DATA,
};

static struct msm_camera_i2c_reg_conf mt9v113_no_effect[] = {
	{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6440},	// MODE_A_SPECIAL_EFFECT_OFF 
	{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6440},	// MODE_B_SPECIAL_EFFECT_OFF
};

static struct msm_camera_i2c_conf_array mt9v113_no_effect_confs[] = {
	{&mt9v113_no_effect[0],
	ARRAY_SIZE(mt9v113_no_effect), 0,
	MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_reg_conf mt9v113_special_effect[][6] = {
		/*for special effect OFF*/
	{
		{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
		{0x0990, 0x6440},	// MODE_A_SPECIAL_EFFECT_OFF 
		{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
		{0x0990, 0x6440},	// MODE_B_SPECIAL_EFFECT_OFF
	},
		/*for special effect MONO*/
	{
		{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
		{0x0990, 0x6441},	// MODE_A_MONO_ON 
		{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
		{0x0990, 0x6441},	// MODE_B_MONO_ON
	},
		/*for special effect Negative*/
	{
		{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
		{0x0990, 0x6443},	// MODE_A_NEGATIVE_ON 
		{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
		{0x0990, 0x6443},	// MODE_B_NEGATIVE_ON
	},
		/* for special effect Sepia */
	{
		{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
		{0x0990, 0x6442},	// MODE_A_SEPIA_ON 
		{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
		{0x0990, 0x6442},	// MODE_B_SEPIA_ON
		{0x098C, 0x2763},	// LOGICAL_ADDRESS_ACCESS [MODE_COMMONMODESETTINGS_FX_SEPIA_SETTINGS]
		{0x0990, 0xE817},	// MCU_DATA_0
	},
};

static struct msm_camera_i2c_conf_array mt9v113_special_effect_confs[][1] = {
	{{mt9v113_special_effect[0],  ARRAY_SIZE(mt9v113_special_effect[0]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_special_effect[1],  ARRAY_SIZE(mt9v113_special_effect[1]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_special_effect[2],  ARRAY_SIZE(mt9v113_special_effect[2]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_special_effect[3],  ARRAY_SIZE(mt9v113_special_effect[3]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
};

static int mt9v113_special_effect_enum_map[] = {
	MSM_V4L2_EFFECT_OFF,
	MSM_V4L2_EFFECT_MONO,
	MSM_V4L2_EFFECT_NEGATIVE,
	MSM_V4L2_EFFECT_SEPIA,
};

static struct msm_camera_i2c_enum_conf_array
		 mt9v113_special_effect_enum_confs = {
	.conf = &mt9v113_special_effect_confs[0][0],
	.conf_enum = mt9v113_special_effect_enum_map,
	.num_enum = ARRAY_SIZE(mt9v113_special_effect_enum_map),
	.num_index = ARRAY_SIZE(mt9v113_special_effect_confs),
	.num_conf = ARRAY_SIZE(mt9v113_special_effect_confs[0]),
	.data_type = MSM_CAMERA_I2C_WORD_DATA,
};

static struct msm_camera_i2c_reg_conf mt9v113_wb_oem[][17] = {
	/* wb_default_tbl_sub */
	{
		{0x098C, 0xA34A},	  // MCU_ADDRESS [SEQ_MODE]
		{0x0990, 0x0059},	  // MCU_DATA_0
		{0x098C, 0xA34B},   // MCU_ADDRESS [AWB_GAIN_R]
		{0x0990, 0x00E6},	  // MCU_DATA_0
		{0x098C, 0xA34C},	  // MCU_ADDRESS [AWB_GAIN_G]
		{0x0990, 0x0059},	  // MCU_DATA_0
		{0x098C, 0xA34D},	  // MCU_ADDRESS [AWB_GAIN_B]
		{0x0990, 0x00A6},	  // MCU_DATA_0
		{0x098C, 0xA351},	  // MCU_ADDRESS [AWB_CCM_POSITION_MIN]
		{0x0990, 0x0020},	  // MCU_DATA_0
		{0x098C, 0xA352},	  // MCU_ADDRESS [AWB_CCM_POSITION_MAX]
		{0x0990, 0x007F},	  // MCU_DATA_0
	},
	/* wb_incandescent_tbl_sub */
	{
		{0x098C, 0xA34A}, 	// MCU_ADDRESS [AWB_GAIN_MIN]
		{0x0990, 0x0084}, 	// MCU_DATA_0
		{0x098C, 0xA34B}, 	// MCU_ADDRESS [AWB_GAIN_MAX]
		{0x0990, 0x0091}, 	// MCU_DATA_0
		{0x098C, 0xA34C}, 	// MCU_ADDRESS [AWB_GAINMIN_B]
		{0x0990, 0x0085}, 	// MCU_DATA_0       //0x0080~0x0085 test //0x0075
		{0x098C, 0xA34D}, 	// MCU_ADDRESS [AWB_GAINMAX_B]
		{0x0990, 0x008a}, 	// MCU_DATA_0
		{0x098C, 0xA351}, 	// MCU_ADDRESS [AWB_CCM_POSITION_MIN]
		{0x0990, 0x0000}, 	// MCU_DATA_0
		{0x098C, 0xA352}, 	// MCU_ADDRESS [AWB_CCM_POSITION_MAX]
		{0x0990, 0x0020}, 	// MCU_DATA_0
	},
	/* wb_sunny_tbl_sub */
	{
		{0x098C, 0xA34A}, 	// MCU_ADDRESS [AWB_GAIN_MIN]
		{0x0990, 0x00a5}, 	// MCU_DATA_0
		{0x098C, 0xA34B}, 	// MCU_ADDRESS [AWB_GAIN_MAX]
		{0x0990, 0x00ce}, 	// MCU_DATA_0
		{0x098C, 0xA34C}, 	// MCU_ADDRESS [AWB_GAINMIN_B]
		{0x0990, 0x0096}, 	// MCU_DATA_0
		{0x098C, 0xA34D}, 	// MCU_ADDRESS [AWB_GAINMAX_B]
		{0x0990, 0x00b0}, 	// MCU_DATA_0
		{0x098C, 0xA351}, 	// MCU_ADDRESS [AWB_CCM_POSITION_MIN]
		{0x0990, 0x0060}, 	// MCU_DATA_0
		{0x098C, 0xA352},	 // MCU_ADDRESS [AWB_CCM_POSITION_MAX]
		{0x0990, 0x007F}, 	// MCU_DATA_0
	},
	/* wb_fluorescent_tbl_sub */
	{
		{0x098C, 0xA34A},	// MCU_ADDRESS [AWB_GAIN_MIN]
		{0x0990, 0x0097},	// MCU_DATA_0
		{0x098C, 0xA34B},	// MCU_ADDRESS [AWB_GAIN_MAX]
		{0x0990, 0x00B6},	// MCU_DATA_0
		{0x098C, 0xA34C},	// MCU_ADDRESS [AWB_GAINMIN_B]
		{0x0990, 0x00A3},	// MCU_DATA_0
		{0x098C, 0xA34D},	// MCU_ADDRESS [AWB_GAINMAX_B]
		{0x0990, 0x00B9},	// MCU_DATA_0
		{0x098C, 0xA351},	// MCU_ADDRESS [AWB_CCM_POSITION_MIN]
		{0x0990, 0x0050},	// MCU_DATA_0
		{0x098C, 0xA352},	// MCU_ADDRESS [AWB_CCM_POSITION_MAX]
		{0x0990, 0x0065},	// MCU_DATA_0
	},
	/* wb_cloudy_tbl_sub */		
	{
		{0x098C, 0xA34A},	// MCU_ADDRESS [AWB_GAIN_MIN]
		{0x0990, 0x00d1},	// MCU_DATA_0
		{0x098C, 0xA34B},	// MCU_ADDRESS [AWB_GAIN_MAX]
		{0x0990, 0x00da},	// MCU_DATA_0
		{0x098C, 0xA34C},	// MCU_ADDRESS [AWB_GAINMIN_B]
		{0x0990, 0x0090},	// MCU_DATA_0
		{0x098C, 0xA34D},	// MCU_ADDRESS [AWB_GAINMAX_B]
		{0x0990, 0x00a1},	// MCU_DATA_0
		{0x098C, 0xA351},	// MCU_ADDRESS [AWB_CCM_POSITION_MIN]
		{0x0990, 0x0070},	// MCU_DATA_0
		{0x098C, 0xA352},	// MCU_ADDRESS [AWB_CCM_POSITION_MAX]
		{0x0990, 0x007F},	// MCU_DATA_0
	},	
};

static struct msm_camera_i2c_conf_array mt9v113_wb_oem_confs[][1] = {
	{{mt9v113_wb_oem[0], ARRAY_SIZE(mt9v113_wb_oem[0]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_wb_oem[1], ARRAY_SIZE(mt9v113_wb_oem[1]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_wb_oem[2], ARRAY_SIZE(mt9v113_wb_oem[2]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_wb_oem[3], ARRAY_SIZE(mt9v113_wb_oem[3]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_wb_oem[4], ARRAY_SIZE(mt9v113_wb_oem[4]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
};

static int mt9v113_wb_oem_enum_map[] = {
	MSM_V4L2_WB_AUTO,
	MSM_V4L2_WB_INCANDESCENT,
	MSM_V4L2_WB_DAYLIGHT,
	MSM_V4L2_WB_FLUORESCENT,
	MSM_V4L2_WB_CLOUDY_DAYLIGHT,	
};

static struct msm_camera_i2c_enum_conf_array mt9v113_wb_oem_enum_confs = {
	.conf = &mt9v113_wb_oem_confs[0][0],
	.conf_enum = mt9v113_wb_oem_enum_map,
	.num_enum = ARRAY_SIZE(mt9v113_wb_oem_enum_map),
	.num_index = ARRAY_SIZE(mt9v113_wb_oem_confs),
	.num_conf = ARRAY_SIZE(mt9v113_wb_oem_confs[0]),
	.data_type = MSM_CAMERA_I2C_WORD_DATA,
};

static struct msm_camera_i2c_reg_conf mt9v113_night_mode[][14] = {
	/* scene_normal_mode_tbl_sub */
	{	
		   {0x098C, 0xA20C},
		   {0x0990, 0x0009},
		   {0x098C, 0xA75D},
		   {0x0990, 0x0000},
	},
	/* scene_night_mode_tbl_sub */
	{	
		   {0x098C, 0xA20C},
		   {0x0990, 0x000E},	//adjust 0x000E 
		   {0x098C, 0xA75D},
		   {0x0990, 0x0005},
	},
};

static struct msm_camera_i2c_conf_array mt9v113_night_mode_confs[][1] = {
	{{mt9v113_night_mode[0], ARRAY_SIZE(mt9v113_night_mode[0]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_night_mode[1], ARRAY_SIZE(mt9v113_night_mode[1]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
};

static int mt9v113_night_mode_enum_map[] = {
	MSM_V4L2_NIGHT_MODE_OFF ,
	MSM_V4L2_NIGHT_MODE_ON,
};

static struct msm_camera_i2c_enum_conf_array mt9v113_night_mode_enum_confs = {
	.conf = &mt9v113_night_mode_confs[0][0],
	.conf_enum = mt9v113_night_mode_enum_map,
	.num_enum = ARRAY_SIZE(mt9v113_night_mode_enum_map),
	.num_index = ARRAY_SIZE(mt9v113_night_mode_confs),
	.num_conf = ARRAY_SIZE(mt9v113_night_mode_confs[0]),
	.data_type = MSM_CAMERA_I2C_WORD_DATA,
};

static struct msm_camera_i2c_reg_conf mt9v113_fps_range[][45] = {
	/* fps_fixed_15_tbl_sub */
	{	
		{0x098C, 0x2721},	   //Line Length (A)				  	
		{0x0990, 0x0730},	   //	   = 1840									   	
		{0x098C, 0x2737},	   //Line Length (B)				  	
		{0x0990, 0x0730},	   //	   = 1840					  	
		{0x098C, 0x222D},	   //R9 Step						  	
		{0x0990, 0x003F},	   //	   = 63 					  	
		{0x098C, 0x2411},	   //R9_Step_60 (A) 				  	
		{0x0990, 0x003F},	   //	   = 63 					  	
		{0x098C, 0x2413},	   //R9_Step_50 (A) 				  	
		{0x0990, 0x004C},	   //	   = 76 					  	
		{0x098C, 0x2415},	   //R9_Step_60 (B) 				  	
		{0x0990, 0x003F},	   //	   = 63 					  	
		{0x098C, 0x2417},	   //R9_Step_50 (B) 				  	
		{0x0990, 0x004C},	   //	   = 76 					  	
		{0x098C, 0xA20C},	 // MCU_ADDRESS [AE_MAX_INDEX]	  	
		{0x0990, 0x0008},	 // MCU_DATA_0					  	
		{0x098C, 0xA20B},	 // MCU_ADDRESS [AE_MIN_INDEX]	  	
		{0x0990, 0x0000},	 // MCU_DATA_0					  	
	},

	/* fps_fixed_30_tbl_sub */
	{	
		{0x098C, 0xA20C},// MCU_ADDRESS [AE_MAX_INDEX]       				
		{0x0990, 0x0005},	// MCU_DATA_0 								 	
		{0x098C, 0xA20B},// MCU_ADDRESS [AE_MIN_INDEX]  				
		{0x0990, 0x0005},	// MCU_DATA_0 	
		{0x098C, 0xA21B},	// MCU_ADDRESS [AE_INDEX]					
		{0x0990, 0x0003},	// MCU_DATA_0 					  	
	},
	/* fps_auto_1030_tbl_sub */
	{		
		{0x098C, 0x2721},	  //Line Length (A) 			   	
		{0x0990, 0x0398},	  //	  = 920 							  	
		{0x098C, 0x2737},	  //Line Length (B) 			   	
		{0x0990, 0x0398},	  //	  = 920	
		{0x098C, 0x222D},	 //R9 Step			
		{0x0990, 0x007F},	 // 	 = 127							
		{0x098C, 0x2411},	  //R9_Step_60 (A)				   	
		{0x0990, 0x007F},	  //	  = 127 				   	
		{0x098C, 0x2413},	  //R9_Step_50 (A)				   	
		{0x0990, 0x0098},	  //	  = 152 				   	
		{0x098C, 0x2415},	  //R9_Step_60 (B)				   	
		{0x0990, 0x007F},	  //	  = 127 				   	
		{0x098C, 0x2417},	  //R9_Step_50 (B)				   	
		{0x0990, 0x0098},	  //	  = 152 				   	
		{0x098C, 0xA20C},	// MCU_ADDRESS [AE_MAX_INDEX]	 	
		{0x0990, 0x000c},	// MCU_DATA_0					 	
		{0x098C, 0xA20B},	// MCU_ADDRESS [AE_MIN_INDEX]	 	
		{0x0990, 0x0000},	// MCU_DATA_0					 	
	},
	/* fps_auto_730_tbl_sub */
	{	
		
	 	{0x098C, 0x2721},	//Line Length (A)				 	
	 	{0x0990, 0x0398},	//		= 920								   	
	 	{0x098C, 0x2737},	//Line Length (B)				 	
	 	{0x0990, 0x0398},	//		= 920	
	 	{0x098C, 0x222D},	   //R9 Step	  	
	 	{0x0990, 0x007F},	   //	   = 127					  	
	 	{0x098C, 0x2411},	//R9_Step_60 (A)				 	
	 	{0x0990, 0x007F},	//		= 127					 	
	 	{0x098C, 0x2413},	//R9_Step_50 (A)				 	
	 	{0x0990, 0x0098},	//		= 152					 	
	 	{0x098C, 0x2415},	//R9_Step_60 (B)				 	
	 	{0x0990, 0x007F},	//		= 127					 	
	 	{0x098C, 0x2417},	//R9_Step_50 (B)				 	
	 	{0x0990, 0x0098},	//		= 152					 	
	 	{0x098C, 0xA20C},	// MCU_ADDRESS [AE_MAX_INDEX]	 	
	 	{0x0990, 0x0010},	// MCU_DATA_0					 	
	 	{0x098C, 0xA20B},	// MCU_ADDRESS [AE_MIN_INDEX]	 	
	 	{0x0990, 0x0000},	// MCU_DATA_0					 	
	},
};

static struct msm_camera_i2c_conf_array mt9v113_fps_range_confs[][1] = {
	{{mt9v113_fps_range[0],  ARRAY_SIZE(mt9v113_fps_range[0]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_fps_range[1],  ARRAY_SIZE(mt9v113_fps_range[1]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9v113_fps_range[2],  ARRAY_SIZE(mt9v113_fps_range[2]),  0,
		MSM_CAMERA_I2C_WORD_DATA},},
};

static int mt9v113_fps_range_enum_map[] = {
	MSM_V4L2_FPS_15_15, 
	MSM_V4L2_FPS_30_30,
	MSM_V4L2_FPS_7P5_30,	// 10 to 30
};

static struct msm_camera_i2c_enum_conf_array mt9v113_fps_range_enum_confs = {
	.conf = &mt9v113_fps_range_confs[0][0],
	.conf_enum = mt9v113_fps_range_enum_map,
	.num_enum = ARRAY_SIZE(mt9v113_fps_range_enum_map),
	.num_index = ARRAY_SIZE(mt9v113_fps_range_confs),
	.num_conf = ARRAY_SIZE(mt9v113_fps_range_confs[0]),
	.data_type = MSM_CAMERA_I2C_WORD_DATA,
};


int mt9v113_msm_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("%s is called enum num: %d , value = %d\n", __func__, ctrl_info->ctrl_id, value);
	switch(ctrl_info->ctrl_id) {
		case V4L2_CID_EXPOSURE:
			if(PREV_EXPOSURE == value || PREV_EXPOSURE == -1) {
				PREV_EXPOSURE = value;
				return rc;
			}else {
				PREV_EXPOSURE = value;
			}
			break;
			
		case V4L2_CID_SPECIAL_EFFECT:
			if(PREV_EFFECT == value || PREV_EFFECT == -1) {
				PREV_EFFECT = value;
				return rc;
			}else {
				PREV_EFFECT = value;
			}
			break;
			
		case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
			if(PREV_WB == value || PREV_WB == -1) {
				PREV_WB = value;
				return rc;
			}else {
				PREV_WB = value;
			}
			break;
			
		case V4L2_CID_NIGHT_MODE:
			if(PREV_NIGHT_MODE == value || PREV_NIGHT_MODE == -1) {
				PREV_NIGHT_MODE = value;
				return rc;
			}else {
				PREV_NIGHT_MODE = value;
			}
			break;
			
		case V4L2_CID_FPS_RANGE:
			if(PREV_FPS == value || PREV_FPS == -1) {
				PREV_FPS = value;
				return rc;
			}else {
				PREV_FPS = value;
			}
			break;
		
		default:
			break;
	}


	rc = msm_sensor_write_enum_conf_array(
		s_ctrl->sensor_i2c_client,
		ctrl_info->enum_cfg_settings, value);
	if (rc < 0) {
		CDBG("write faield\n");
		return rc;
	}

	if((ctrl_info->ctrl_id == V4L2_CID_NIGHT_MODE ) || (ctrl_info->ctrl_id == V4L2_CID_FPS_RANGE))
	{
		rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, refreshmode_sub,
			ARRAY_SIZE(refreshmode_sub), MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0)
			return rc;

		rc = mt9v113_check_sensor_mode(s_ctrl);
		if (rc<0)
		{
		   pr_err("###[ERROR]%s: %d failed to check sensor mode\n", __func__, __LINE__);
		   return rc;
		}
	}
	
	rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, refresh_sub,
			ARRAY_SIZE(refresh_sub), MSM_CAMERA_I2C_WORD_DATA);
	if (rc < 0)
		return rc;

	rc = mt9v113_check_sensor_mode(s_ctrl);

	if (rc<0)
	{
	   pr_err("###[ERROR]%s: %d failed to check sensor mode\n", __func__, __LINE__);
	   return rc;
	}
	return rc;
}

struct msm_sensor_v4l2_ctrl_info_t mt9v113_v4l2_ctrl_info[] = {
	{
		.ctrl_id = V4L2_CID_EXPOSURE,
		.min = MSM_V4L2_EXPOSURE_N6,
		.max = MSM_V4L2_EXPOSURE_P6,
		.step = 1,
		.enum_cfg_settings = &mt9v113_exposure_enum_confs,
		.s_v4l2_ctrl = mt9v113_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_SPECIAL_EFFECT,
		.min = MSM_V4L2_EFFECT_OFF,
		.max = MSM_V4L2_EFFECT_SEPIA,
		.step = 1,
		.enum_cfg_settings = &mt9v113_special_effect_enum_confs,
		.s_v4l2_ctrl = mt9v113_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_WHITE_BALANCE_TEMPERATURE,
		.min = MSM_V4L2_WB_AUTO,
		.max = MSM_V4L2_WB_CLOUDY_DAYLIGHT,
		.step = 1,
		.enum_cfg_settings = &mt9v113_wb_oem_enum_confs,
		.s_v4l2_ctrl = mt9v113_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_NIGHT_MODE,
		.min = MSM_V4L2_NIGHT_MODE_OFF,
		.max = MSM_V4L2_NIGHT_MODE_ON,
		.step = 1,
		.enum_cfg_settings = &mt9v113_night_mode_enum_confs,
		.s_v4l2_ctrl = mt9v113_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_FPS_RANGE,
		.min = MSM_V4L2_FPS_15_15,
		.max = MSM_V4L2_FPS_30_30,
		.step = 1,
		.enum_cfg_settings = &mt9v113_fps_range_enum_confs,
		.s_v4l2_ctrl = mt9v113_msm_sensor_s_ctrl_by_enum,
	},
};

static struct msm_camera_csi_params mt9v113_csic_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x14,
};

static struct msm_camera_csi_params *mt9v113_csic_params_array[] = {
	&mt9v113_csic_params,
};

// not used
static struct msm_sensor_output_reg_addr_t mt9v113_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t mt9v113_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x2280,
};

//static struct sensor_calib_data mt9v113_calib_data;
static const struct i2c_device_id mt9v113_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9v113_s_ctrl},
	{ }
};

static ssize_t mt9v113_antibanding_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	sscanf(buf,"%d",&val);
	printk("mt9v113: antibanding type [0x%x] \n",val);

	/* 1 : Antibanding 60Hz
	 * 2 : Antibanding 50Hz */
	switch(val)
	{
		case 1:
			mt9v113_antibanding = MT9V113_60HZ;
			break;
		case 2:
			mt9v113_antibanding = MT9V113_50HZ;
			break;
		default:
			pr_err("mt9v113: invalid antibanding type[%d] \n",val);
			mt9v113_antibanding = MT9V113_50HZ;			
			break;	
	}
	return n;	
}

static DEVICE_ATTR(antibanding, /*S_IRUGO|S_IWUGO*/ 0664, NULL, mt9v113_antibanding_store);

static struct attribute* mt9v113_sysfs_attrs[] = {
	&dev_attr_antibanding.attr,
};

static int mt9v113_sysfs_add(struct kobject* kobj)
{
	int i, n, ret;
	
	n = ARRAY_SIZE(mt9v113_sysfs_attrs);
	for(i = 0; i < n; i++){
		if(mt9v113_sysfs_attrs[i]){
			ret = sysfs_create_file(kobj, mt9v113_sysfs_attrs[i]);
			if(ret < 0){
				pr_err("mt9v113 sysfs is not created\n");
			}
		}
	}
	return 0;
}

int32_t mt9v113_i2c_probe(struct i2c_client *client,
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
	rc = mt9v113_sysfs_add(&client->dev.kobj);
	printk("%s : sysfs rc = %d\n", __func__, rc);
	if (rc < 0){
		pr_err("mt9v113: failed mt9v113_sysfs_add\n");
	}
/* LGE_CHANGE_E : 2013-02-20 sungmin.cho@lge.com for setting antibanding in runtime */
	return rc;
}

static struct i2c_driver mt9v113_i2c_driver = {
	.id_table = mt9v113_i2c_id,
	.probe  = mt9v113_i2c_probe, //msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9v113_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	int rc = 0;
	printk("msm_sensor_init_module mt9v113\n");

	rc = i2c_add_driver(&mt9v113_i2c_driver);

	return rc;
}

static struct v4l2_subdev_core_ops mt9v113_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

static int32_t mt9v113_reg_init(struct msm_sensor_ctrl_t *s_ctrl) {
	int32_t rc = 0;
	
	#if defined(CONFIG_MACH_MSM7X27A_U0) // kim.duhyung_130227_modified for flicker issue
	printk("%s : mt9v113_antibanding = %d \n", __func__, mt9v113_antibanding);
	rc = mt9v113_camera_i2c_write_tbl(
		             s_ctrl->sensor_i2c_client,
		             s_ctrl->msm_sensor_reg->init_settings[mt9v113_antibanding].conf,
		             s_ctrl->msm_sensor_reg->init_settings[mt9v113_antibanding].size,
		             s_ctrl->msm_sensor_reg->init_settings[mt9v113_antibanding].data_type);

	if (rc < 0)
	{
		pr_err("###[ERROR]%s: %d failed to sensor init setting \n", __func__, __LINE__);
		return rc;
	}
	#else
	for (i = 0; i < s_ctrl->msm_sensor_reg->init_size; i++) {
		rc = mt9v113_camera_i2c_write_tbl(
		             s_ctrl->sensor_i2c_client,
		             s_ctrl->msm_sensor_reg->init_settings[i].conf,
		             s_ctrl->msm_sensor_reg->init_settings[i].size,
		             s_ctrl->msm_sensor_reg->init_settings[i].data_type);

		if (rc < 0)
			break;
	}
	#endif
	rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, refreshmode_sub,
		ARRAY_SIZE(refreshmode_sub), MSM_CAMERA_I2C_WORD_DATA);
	if (rc < 0)
		return rc;

	rc = mt9v113_check_sensor_mode(s_ctrl);
	if (rc<0)
	{
	   pr_err("###[ERROR]%s: %d failed to check sensor mode\n", __func__, __LINE__);
	   return rc;
	}
	
	rc = mt9v113_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client, refresh_sub,
			ARRAY_SIZE(refresh_sub), MSM_CAMERA_I2C_WORD_DATA);
	if (rc < 0)
		return rc;

	rc = mt9v113_check_sensor_mode(s_ctrl);

	if (rc<0)
	{
	   pr_err("###[ERROR]%s: %d failed to check sensor mode\n", __func__, __LINE__);
	   return rc;
	}

	return rc;
}


int32_t mt9v113_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;

	if (update_type == MSM_SENSOR_REG_INIT) {
		pr_err("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		PREV_EFFECT = -1;
		PREV_EXPOSURE = -1;
		PREV_WB = -1;
		PREV_FPS = -1;
		PREV_NIGHT_MODE = -1;

		mt9v113_reg_init(s_ctrl);
		csi_config = 0;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		pr_err("PERIODIC : %d\n", res);
		if (!csi_config) {
			s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			pr_err("CSI config in progress\n");
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
				s_ctrl->curr_csic_params);
			pr_err("CSI config is done\n");
			mb();
			msleep(50);
			csi_config = 1;
			s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		}
		
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE,
			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

	}
	return rc;
}
int mt9v113_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
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

	if (data->sensor_reset_enable) {
		rc = gpio_request(data->sensor_reset, "VT_RESET");
		if (rc < 0)
			pr_err("%s: gpio_request:VT_RESET %d failed\n", __func__, data->sensor_reset);
		rc = gpio_direction_output(data->sensor_reset, 0);
		if (rc < 0)
			pr_err("%s: gpio:VT_RESET %d direction can't be set\n", __func__, data->sensor_reset);
	}
	

	rc = gpio_direction_output(data->sensor_reset, 1);
	if (rc < 0)
		pr_err("%s: gpio:VT_RESET %d direction can't be set\n", __func__, data->sensor_reset);
	mdelay(1);


	rc = msm_camera_config_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
		goto config_gpio_failed;
	}	
	mdelay(2);

	rc = gpio_direction_output(data->sensor_reset, 0);
	if (rc < 0)
		pr_err("%s: gpio:VT_RESET %d direction can't be set\n", __func__, data->sensor_reset);

	mdelay(10);
	

	if (s_ctrl->clk_rate != 0)
		cam_clk_info->clk_rate = s_ctrl->clk_rate;

	rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n", __func__);
		goto enable_clk_failed;
	}
	msleep(2);


	rc = gpio_direction_output(data->sensor_reset, 1);
	if (rc < 0)
		pr_err("%s: gpio:VT_RESET %d direction can't be set\n", __func__, data->sensor_reset);

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(1);
	mdelay(5);
	printk("%s: X %d\n", __func__, __LINE__);

	return rc;
enable_clk_failed:
	msm_camera_request_gpio_table(data, 0);
config_gpio_failed:
	kfree(s_ctrl->reg_ptr);
	return rc;
}

int mt9v113_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	int32_t rc = 0;
	printk("%s: E %d\n", __func__, __LINE__);
	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(0);


	if (data->sensor_reset_enable) {
		rc = gpio_direction_output(data->sensor_reset, 0);
		if (rc < 0)
			pr_err("%s: gpio:VT_RESET %d direction can't be set\n", __func__, data->sensor_reset);
		gpio_free(data->sensor_reset);
	}
	msleep(30);
	

	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);
	msleep(2);


	msm_camera_config_gpio_table(data, 0);
	kfree(s_ctrl->reg_ptr);
	printk("%s: X %d\n", __func__, __LINE__);
	return 0;
}

int32_t mt9v113_camera_i2c_write_tbl(struct msm_camera_i2c_client *client,
                                     struct msm_camera_i2c_reg_conf *reg_conf_tbl, uint16_t size,
                                     enum msm_camera_i2c_data_type data_type)
{
	int i;
	int32_t rc = -EIO;

	printk("%s: E %d\n", __func__, __LINE__);

	for (i = 0; i < size; i++) {		
		if (reg_conf_tbl->reg_addr == 0xFFFF) {
			msleep(reg_conf_tbl->reg_data);
			rc = 0;
		} else if ((reg_conf_tbl->reg_data == 0x4000) && (reg_conf_tbl->reg_addr == 0x0018)) {
			rc = mt9v113_polling_reg(client, reg_conf_tbl->reg_addr, reg_conf_tbl->reg_data, 0x0000, 10, 100);
			if (rc<0)
				return rc;
		} else if (reg_conf_tbl->reg_addr == 0x301A) {
			rc = mt9v113_polling_reg(client,reg_conf_tbl->reg_addr, reg_conf_tbl->reg_data, 0x0004, 50, 20);
			if (rc<0)
				return rc;
		} else {
			rc = msm_camera_i2c_write(client, reg_conf_tbl->reg_addr, reg_conf_tbl->reg_data, data_type);
			if (rc < 0)
				return rc;
		}

		if (rc < 0)
			break;
		reg_conf_tbl++;
	}

	printk("%s: X %d\n", __func__, __LINE__);

	return rc;
}

static int mt9v113_polling_reg(struct msm_camera_i2c_client *client,unsigned short waddr, 
	unsigned short wcondition, unsigned short result, int delay, int time_out)
{
	int rc = -EFAULT;
	int i=0;
	unsigned short wdata;
	CDBG("%s : E", __func__);
	for(i=0; i<time_out; i++){
		rc = msm_camera_i2c_read(client, waddr, &wdata, MSM_CAMERA_I2C_WORD_DATA);

		if (rc<0) {
			CDBG("polling reg failed to read\n");
			return rc;
		}

		CDBG("polling reg 0x%x ==> 0x%x\n", waddr, wdata);

		if ((wdata&wcondition)==result) {
			CDBG("polling register meets condition\n");
			break;
		}
		else
			mdelay(delay);
	}
	CDBG("%s : X", __func__);

	return rc;
}

int mt9v113_check_sensor_mode(struct msm_sensor_ctrl_t *s_ctrl)
{
	unsigned short mcu_address_sts =0, mcu_data =0;
	int i, rc=-EFAULT;

	CDBG("mt9v113 : check_sensor_mode E\n");

	for(i=0; i<50; i++){

		/* MCU_ADDRESS : check mcu_address */
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
				0x098C, &mcu_address_sts, MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0){
			CDBG("mt9v113: reading mcu_address_sts fail\n");
			return rc;
		}

		/* MCU_DATA_0 : check mcu_data */
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
				0x0990, &mcu_data, MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0){
			CDBG("mt9v113: reading mcu_data fail\n");
			return rc;
		}


		if( ((mcu_address_sts & 0xA103) == 0xA103) && (mcu_data == 0x00)){
			CDBG("mt9v113: sensor refresh mode success!!\n");
			break;
		}
		msleep(10);
	}

	CDBG("mt9v113: check_sensor_mode X\n");

	return rc;
}

void mt9v113_sensor_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	mt9v113_camera_i2c_write_tbl(
	        s_ctrl->sensor_i2c_client,
	        s_ctrl->msm_sensor_reg->start_stream_conf,
	        s_ctrl->msm_sensor_reg->start_stream_conf_size,
	        s_ctrl->msm_sensor_reg->default_data_type);
}

void mt9v113_sensor_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	mt9v113_camera_i2c_write_tbl(
	        s_ctrl->sensor_i2c_client,
	        s_ctrl->msm_sensor_reg->stop_stream_conf,
	        s_ctrl->msm_sensor_reg->stop_stream_conf_size,
	        s_ctrl->msm_sensor_reg->default_data_type);
}


static struct v4l2_subdev_video_ops mt9v113_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9v113_subdev_ops = {
	.core = &mt9v113_subdev_core_ops,
	.video  = &mt9v113_subdev_video_ops,
};

static struct msm_sensor_fn_t mt9v113_func_tbl = {
	.sensor_start_stream = mt9v113_sensor_start_stream,
	.sensor_stop_stream = mt9v113_sensor_stop_stream,
	.sensor_csi_setting = mt9v113_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = mt9v113_sensor_power_up,
	.sensor_power_down = mt9v113_sensor_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t mt9v113_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA,
	.start_stream_conf = mt9v113_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(mt9v113_start_settings),
	.stop_stream_conf = mt9v113_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(mt9v113_stop_settings),
	.init_settings = &mt9v113_init_conf[0],
	.init_size = ARRAY_SIZE(mt9v113_init_conf),
	.mode_settings = &mt9v113_confs[0],
	.no_effect_settings = &mt9v113_no_effect_confs[0],
	.output_settings = &mt9v113_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9v113_confs),
};

static struct msm_sensor_ctrl_t mt9v113_s_ctrl = {
	.msm_sensor_reg = &mt9v113_regs,
	.msm_sensor_v4l2_ctrl_info = mt9v113_v4l2_ctrl_info,
	.num_v4l2_ctrl = ARRAY_SIZE(mt9v113_v4l2_ctrl_info),
	.sensor_i2c_client = &mt9v113_sensor_i2c_client,
	.sensor_i2c_addr = 0x7A,
	.sensor_output_reg_addr = &mt9v113_reg_addr,
	.sensor_id_info = &mt9v113_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &mt9v113_csic_params_array[0],
	.msm_sensor_mutex = &mt9v113_mut,
	.sensor_i2c_driver = &mt9v113_i2c_driver,
	.sensor_v4l2_subdev_info = mt9v113_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9v113_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9v113_subdev_ops,
	.func_tbl = &mt9v113_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina VGA Soc Sensor");
MODULE_LICENSE("GPL v2");

