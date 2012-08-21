/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "mt9v113.h"  

//=======================================================================
//    VGA Camera init
//=======================================================================

// vga size 382M 30fps to 15fps
struct mt9v113_i2c_reg_conf const init_tbl_sub[] =
{
	{0x0018, 0x4028}, // STANDBY_CONTROL  
};

struct mt9v113_i2c_reg_conf const init_tbl1_sub[] =
{
	{0x001A, 0x0013}, // RESET_AND_MISC_CONTROL                                                                                  //DELAY=10           
	{0xFFFF, 0x000A},                                                                                          
	{0x001A, 0x0010}, // RESET_AND_MISC_CONTROL                                                                                                                                                                   
	//DELAY=10           
	{0xFFFF, 0x000A},                                                                                                                                                                                            
	{0x0018, 0x4028}, // STANDBY_CONTROL   
};

struct mt9v113_i2c_reg_conf const init_tbl2_sub[] =
{                                                                                                                           
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
//DELAY=10
	{0xFFFF, 0x000A},
	{0x001A, 0x0210}, 	// RESET_AND_MISC_CONTROL
	{0x3400, 0xAA24}, 	// MIPI_CONTROL
	{0x321C, 0x0003}, 	// OFIFO_CONTROL_STATUS
	{0x001E, 0x0400}, 	// PAD_SLEW
	{0x0016, 0x42DF}, 	// CLOCKS_CONTROL
	{0x0014, 0x2145}, 	// PLL_CONTROL
	{0x0014, 0x2145}, 	// PLL_CONTROL
	{0x0010, 0x0110}, 	// 21c PLL_DIVIDERS
	{0x0012, 0x0000}, 	// PLL_P_DIVIDERS
	{0x0014, 0x244B}, 	// PLL_CONTROL
//DELAY=10
	{0xFFFF, 0x000A},
	{0x0014, 0x304B}, 	// PLL_CONTROL
//DELAY=10
	{0xFFFF, 0x000A},
	{0x0014, 0xB04A}, 	// PLL_CONTROL
//DELAY=10                                
                     
	{0xFFFF, 0x000A},                                                                                                                                                                                                    
	{0x001A, 0x0018},
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
//	{0x0990, 0x0026}, 	// MCU_DATA_0   //0x0025: mirror, 0x0026: flip, 0x0027: flip & mirror
	{0x098C, 0x272D}, 	// MCU_ADDRESS [MODE_SENSOR_READ_MODE_B]
	{0x0990, 0x0025},	/* LGE_CHANGE [donghyun.kwon@lge.com] 20111109 : MCU_DATA_0, Flip = OFF, Mirror = ON */
//	{0x0990, 0x0026}, 	// MCU_DATA_0      
//[ min frame]
	{0x098C, 0xA20C}, 	// MCU_ADDRESS [AE_MAX_INDEX]
	{0x0990, 0x0009}, 	// MCU_DATA_0
	{0x098C, 0xA20B}, 	// MCU_ADDRESS [AE_MIN_INDEX]
	{0x0990, 0x0000}, 	// MCU_DATA_0

   {0x098C, 0xA103},                                 
   {0x0990, 0x0006},        //refresh mode   
   {0xFFFF, 0x012c},       //DELAY=300     300: 0x012C, 200:0x00C8, 100:0x0064                                       
   {0x098C, 0xA103},                                 	
   {0x0990, 0x0005},        //refresh
//   {0xFFFF, 0x012c},       //DELAY=300      	 
};


//=======================================================================
//    Color Effect
//=======================================================================
/* Effect register settings */
struct mt9v113_i2c_reg_conf const effect_default_tbl_sub[]=
{
	{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6440},	// MODE_A_SPECIAL_EFFECT_OFF 
	{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6440},	// MODE_B_SPECIAL_EFFECT_OFF
	{0x098C, 0xA103}, 	// MCU_ADDRESS
	{0x0990, 0x0005}, 	// MCU_DATA_0
};
struct mt9v113_i2c_reg_conf const effect_mono_tbl_sub[]=
{
	{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6441},	// MODE_A_MONO_ON 
	{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6441},	// MODE_B_MONO_ON
	{0x098C, 0xA103}, 	// MCU_ADDRESS
	{0x0990, 0x0005}, 	// MCU_DATA_0
};
struct mt9v113_i2c_reg_conf const effect_sepia_tbl_sub[]=
{
	{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6442},	// MODE_A_SEPIA_ON 
	{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6442},	// MODE_B_SEPIA_ON
//Start LGE_BSP_CAMERA::elin.lee@lge.com 2011-07-14  Fix the Sepia effect
	{0x098C, 0x2763},	// LOGICAL_ADDRESS_ACCESS [MODE_COMMONMODESETTINGS_FX_SEPIA_SETTINGS]
	{0x0990, 0xE817},	// MCU_DATA_0
//End LGE_BSP_CAMERA::elin.lee@lge.com 2011-07-14  Fix the Sepia effect
	{0x098C, 0xA103}, 	// MCU_ADDRESS
	{0x0990, 0x0005}, 	// MCU_DATA_0
};
struct mt9v113_i2c_reg_conf const effect_aqua_tbl_sub[]=
{
	{0x098C, 0x2763}, 		   // MCU_ADDRESS [MODE_COMMONMODESETTINGS_FX_SEPIA_SETTINGS]
	{0x0990, 0x2492},		   // MCU_DATA_0
	{0x098C, 0x2759}, 		   // MCU_ADDRESS [MODE_SPEC_EFFECTS_A]
	{0x0990, 0x6442}, 		   // MCU_DATA_0
	{0x098C, 0x275B}, 		  // MCU_ADDRESS [MODE_SPEC_EFFECTS_B]
	{0x0990, 0x6442}, 		   // MCU_DATA_0
	{0x098C, 0xA103}, 		  // MCU_ADDRESS [SEQ_CMD]
	{0x0990, 0x0005}, 		   // MCU_DATA_0
	
};
struct mt9v113_i2c_reg_conf const effect_negative_tbl_sub[]=
{
	{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6443},	// MODE_A_NEGATIVE_ON 
	{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6443},	// MODE_B_NEGATIVE_ON
	{0x098C, 0xA103}, 	// MCU_ADDRESS
	{0x0990, 0x0005}, 	// MCU_DATA_0	
};
struct mt9v113_i2c_reg_conf const effect_solarization_tbl_sub[]=
{
	{0x098C, 0x2759},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6444},	// MODE_A_SOLARIZATION_ON 
	{0x098C, 0x275B},	// LOGICAL_ADDRESS_ACCESS			===== 8
	{0x0990, 0x6444},	// MODE_B_SOLARIZATION_ON
	{0x098C, 0xA103}, 	// MCU_ADDRESS
	{0x0990, 0x0005}, 	// MCU_DATA_0
};


//=======================================================================
//    White Balance
//=======================================================================

/* White balance register settings */
struct mt9v113_i2c_reg_conf const wb_default_tbl_sub[]=
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
	{0x098C, 0xA103}, 	// MCU_ADDRESS
	{0x0990, 0x0005}, 	// MCU_DATA_0	

};
struct mt9v113_i2c_reg_conf const wb_sunny_tbl_sub[]=
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
	{0x098C, 0xA103}, 	// MCU_ADDRESS
	{0x0990, 0x0005}, 	// MCU_DATA_0
};
struct mt9v113_i2c_reg_conf const wb_cloudy_tbl_sub[]=
{
	{0x098C, 0xA34A}, 	// MCU_ADDRESS [AWB_GAIN_MIN]
	{0x0990, 0x00d1}, 	// MCU_DATA_0
	{0x098C, 0xA34B}, 	// MCU_ADDRESS [AWB_GAIN_MAX]
	{0x0990, 0x00da}, 	// MCU_DATA_0
	{0x098C, 0xA34C}, 	// MCU_ADDRESS [AWB_GAINMIN_B]
	{0x0990, 0x0090}, 	// MCU_DATA_0
	{0x098C, 0xA34D}, 	// MCU_ADDRESS [AWB_GAINMAX_B]
	{0x0990, 0x00a1}, 	// MCU_DATA_0
	{0x098C, 0xA351}, 	// MCU_ADDRESS [AWB_CCM_POSITION_MIN]
	{0x0990, 0x0070}, 	// MCU_DATA_0
	{0x098C, 0xA352}, 	// MCU_ADDRESS [AWB_CCM_POSITION_MAX]
	{0x0990, 0x007F}, 	// MCU_DATA_0
  {0x098C, 0xA103}, 	// MCU_ADDRESS
	{0x0990, 0x0005}, 	// MCU_DATA_0
};
struct mt9v113_i2c_reg_conf const wb_fluorescent_tbl_sub[]=
{
	{0x098C, 0xA34A}, 	// MCU_ADDRESS [AWB_GAIN_MIN]
	{0x0990, 0x0097}, 	// MCU_DATA_0
	{0x098C, 0xA34B}, 	// MCU_ADDRESS [AWB_GAIN_MAX]
	{0x0990, 0x00B6}, 	// MCU_DATA_0
	{0x098C, 0xA34C}, 	// MCU_ADDRESS [AWB_GAINMIN_B]
	{0x0990, 0x00A3}, 	// MCU_DATA_0
	{0x098C, 0xA34D}, 	// MCU_ADDRESS [AWB_GAINMAX_B]
	{0x0990, 0x00B9}, 	// MCU_DATA_0
	{0x098C, 0xA351}, 	// MCU_ADDRESS [AWB_CCM_POSITION_MIN]
	{0x0990, 0x0050}, 	// MCU_DATA_0
	{0x098C, 0xA352},	// MCU_ADDRESS [AWB_CCM_POSITION_MAX]
	{0x0990, 0x0065}, 	// MCU_DATA_0
  {0x098C, 0xA103}, 	// MCU_ADDRESS
	{0x0990, 0x0005}, 	// MCU_DATA_0
};
struct mt9v113_i2c_reg_conf const wb_incandescent_tbl_sub[]=
{
	{0x098C, 0xA34A}, 	// MCU_ADDRESS [AWB_GAIN_MIN]
	{0x0990, 0x0084}, 	// MCU_DATA_0
	{0x098C, 0xA34B}, 	// MCU_ADDRESS [AWB_GAIN_MAX]
	{0x0990, 0x0091}, 	// MCU_DATA_0
	{0x098C, 0xA34C}, 	// MCU_ADDRESS [AWB_GAINMIN_B]
	{0x0990, 0x0075}, 	// MCU_DATA_0
	{0x098C, 0xA34D}, 	// MCU_ADDRESS [AWB_GAINMAX_B]
	{0x0990, 0x008a}, 	// MCU_DATA_0
	{0x098C, 0xA351}, 	// MCU_ADDRESS [AWB_CCM_POSITION_MIN]
	{0x0990, 0x0000}, 	// MCU_DATA_0
	{0x098C, 0xA352}, 	// MCU_ADDRESS [AWB_CCM_POSITION_MAX]
	{0x0990, 0x0020}, 	// MCU_DATA_0
	{0x098C, 0xA103}, 	// MCU_ADDRESS
	{0x0990, 0x0005}, 	// MCU_DATA_0	
};


//=======================================================================
//    FPS setting
//=======================================================================

struct mt9v113_i2c_reg_conf const fps_fixed_15_tbl_sub[]=
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
	{0x098C, 0xA103},										
	{0x0990, 0x0006},		  //refresh mode			  
	{0xffff, 0x0064},								
	{0x098C, 0xA103},									 
	{0x0990, 0x0005},		 //refresh					 
	{0xffff, 0x0064},										
};
struct mt9v113_i2c_reg_conf const fps_fixed_30_tbl_sub[]=
{	
	
	{0x098C, 0x2721},	//Line Length (A)				
	{0x0990, 0x0398},	//		= 920								 
	{0x098C, 0x2737},	//Line Length (B)				
	{0x0990, 0x0398},	//		= 920
	{0x098C, 0x222D},	//R9 Step							
	{0x0990, 0x007F},	//		= 127					  
	{0x098C, 0x2411},	//R9_Step_60 (A)				
	{0x0990, 0x007F},	//		= 127					
	{0x098C, 0x2413},	//R9_Step_50 (A)				
	{0x0990, 0x0098},	//		= 152					
	{0x098C, 0x2415},	//R9_Step_60 (B)				
	{0x0990, 0x007F},	//		= 127					
	{0x098C, 0x2417},	//R9_Step_50 (B)				
	{0x0990, 0x0098},	//		= 152					
	{0x098C, 0xA20C},	// MCU_ADDRESS [AE_MAX_INDEX]  
	{0x0990, 0x0004},	// MCU_DATA_0					 
	{0x098C, 0xA20B},	// MCU_ADDRESS [AE_MIN_INDEX]	 
	{0x0990, 0x0000},	// MCU_DATA_0					 
	{0x098C, 0xA103},									
	{0x0990, 0x0006},	 //refresh mode 			 
	{0xffff, 0x0064},									  
	{0x098C, 0xA103},								  
	{0x0990, 0x0005},	  //refresh 				  
	{0xffff, 0x0064},										 
	
};
struct mt9v113_i2c_reg_conf const fps_auto_1030_tbl_sub[]=
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
	{0x098C, 0xA103},									 
	{0x0990, 0x0006},		 //refresh mode 			 
	{0xffff, 0x0064},										 
	{0x098C, 0xA103},									 
	{0x0990, 0x0005},		 //refresh					 
	{0xffff, 0x0064},									   
	
};
struct mt9v113_i2c_reg_conf const fps_auto_730_tbl_sub[]=
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
	{0x098C, 0xA103},									 
	{0x0990, 0x0006},		 //refresh mode 			 
	{0xffff, 0x0064},								
	{0x098C, 0xA103},									 
	{0x0990, 0x0005},		 //refresh					 
	{0xffff, 0x0064},										
};


//=======================================================================
//   initial code table
//=======================================================================

/* Change-config register settings only used for change brightness*/
struct mt9v113_i2c_reg_conf const change_config_tbl_sub[]=
{
//	{0x098C, 0xA103},	// LOGICAL_ADDRESS_ACCESS			===== 8
//    {0x0990, 0x0006},   // AE_TRACK_AE_TRACKING_DAMPENING_SPEED
//    {0xFFFF, 0x012C},  LGE_BSP_CAMERA   miracle.kim@lge.com    delete for reducing camera start time
	{0x098C, 0xA103},   // COMMAND_REGISTER
    {0x0990, 0x0005},
//	{0xFFFF, 0x012C},    LGE_BSP_CAMERA miralce.kim@lge.com delete for reducing camera start time
};

struct mt9v113_reg mt9v113_regs = {

	/* init register settings */

	.init_tbl = &init_tbl_sub[0],
	.inittbl_size = ARRAY_SIZE(init_tbl_sub),
	
	.init_tbl1 = &init_tbl1_sub[0],
	.inittbl1_size = ARRAY_SIZE(init_tbl1_sub), 	

	.init_tbl2 = &init_tbl2_sub[0],
	.inittbl2_size = ARRAY_SIZE(init_tbl2_sub), 

	/* Effect register settings */
	.effect_default_tbl = &effect_default_tbl_sub[0],
	.effect_default_tbl_size = ARRAY_SIZE(effect_default_tbl_sub),
	.effect_mono_tbl = &effect_mono_tbl_sub[0],
	.effect_mono_tbl_size = ARRAY_SIZE(effect_mono_tbl_sub),
	.effect_sepia_tbl = &effect_sepia_tbl_sub[0],
	.effect_sepia_tbl_size = ARRAY_SIZE(effect_sepia_tbl_sub),
	.effect_aqua_tbl = &effect_aqua_tbl_sub[0],
	.effect_aqua_tbl_size = ARRAY_SIZE(effect_aqua_tbl_sub),
	.effect_negative_tbl = &effect_negative_tbl_sub[0],
	.effect_negative_tbl_size = ARRAY_SIZE(effect_negative_tbl_sub),
	.effect_solarization_tbl = &effect_solarization_tbl_sub[0],
	.effect_solarization_tbl_size = ARRAY_SIZE(effect_solarization_tbl_sub),
	/* White balance register settings */
	.wb_default_tbl = &wb_default_tbl_sub[0],
	.wb_default_tbl_size = ARRAY_SIZE(wb_default_tbl_sub),	
	.wb_sunny_tbl = &wb_sunny_tbl_sub[0],
	.wb_sunny_tbl_size = ARRAY_SIZE(wb_sunny_tbl_sub),	
	.wb_cloudy_tbl = &wb_cloudy_tbl_sub[0],
	.wb_cloudy_tbl_size = ARRAY_SIZE(wb_cloudy_tbl_sub),	
	.wb_fluorescent_tbl = &wb_fluorescent_tbl_sub[0],
	.wb_fluorescent_tbl_size = ARRAY_SIZE(wb_fluorescent_tbl_sub),	
	.wb_incandescent_tbl = &wb_incandescent_tbl_sub[0],
	.wb_incandescent_tbl_size = ARRAY_SIZE(wb_incandescent_tbl_sub),	
	/* FPS register settings */
	.fps_fixed_15_tbl = &fps_fixed_15_tbl_sub[0],
	.fps_fixed_15_tbl_size = ARRAY_SIZE(fps_fixed_15_tbl_sub),
	.fps_fixed_30_tbl = &fps_fixed_30_tbl_sub[0],
	.fps_fixed_30_tbl_size = ARRAY_SIZE(fps_fixed_30_tbl_sub),	
	.fps_auto_1030_tbl = &fps_auto_1030_tbl_sub[0],
	.fps_auto_1030_tbl_size = ARRAY_SIZE(fps_auto_1030_tbl_sub),	
	.fps_auto_730_tbl = &fps_auto_730_tbl_sub[0],
	.fps_auto_730_tbl_size = ARRAY_SIZE(fps_auto_730_tbl_sub),
	
	/* Change-config register settings */
	.change_config_tbl = &change_config_tbl_sub[0],
	.change_config_tbl_size = ARRAY_SIZE(change_config_tbl_sub),	
};


