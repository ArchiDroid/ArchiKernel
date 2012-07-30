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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include "mt9e013_lgit.h"

// @mipi#start
static struct mt9e013_lgit_i2c_reg_conf mipi_settings[] =
{
	{CMD_WRITE, 0x3064, 0x7800}, // embedded_data_enable
	{CMD_WRITE, 0x31AE, 0x0202},	// 2-lane MIPI SERIAL_FORMAT
//	{CMD_WRITE, 0x31B8, 0x0E3F}, //MIPI_timing
	/*set data to RAW10 format*/
	{CMD_WRITE, 0x0112, 0x0A0A},	/*CCP_DATA_FORMAT*/
	{CMD_WRITE, 0x30F0, 0x800D},	/*VCM CONTROL : Enable Power collapse, vcm_slew =5*/
};
// @mipi#end

/*PLL Configuration
   (Ext=24MHz, vt_pix_clk=174MHz, op_pix_clk=69.6MHz)*/
// @pll#start
static struct mt9e013_lgit_i2c_reg_conf pll_settings[] = 
{
	{CMD_WRITE, 0x0300, 0x0004}, //VT_PIX_CLK_DIV=4
	{CMD_WRITE, 0x0302, 0x0001}, //VT_SYS_CLK_DIV=1
	{CMD_WRITE, 0x0304, 0x0002},	//PRE_PLL_CLK_DIV=2 //Note: 24MHz/2=12MHz
	{CMD_WRITE, 0x0306, 0x0040},	//PLL_MULTIPLIER=64 //Note: Running at 768MHz
	{CMD_WRITE, 0x0308, 0x000A},	//OP_PIX_CLK_DIV=10
	{CMD_WRITE, 0x030A, 0x0001},	//OP_SYS_CLK_DIV=1
};
// @pll#end

// @preview#start
static struct mt9e013_lgit_i2c_reg_conf prev_settings[] = 
{
	/*Output Size (1640x1232)*/
	{CMD_WRITE, 0x0344, 0x0000}, /*X_ADDR_START*/
	{CMD_WRITE, 0x0348, 0x0CCF}, /*X_ADDR_END*/
	{CMD_WRITE, 0x0346, 0x0000}, /*Y_ADDR_START*/
	{CMD_WRITE, 0x034A, 0x09A1}, /*Y_ADDR_END*/
	{CMD_WRITE, 0x034C, 0x0668}, /*X_OUTPUT_SIZE*/
	{CMD_WRITE, 0x034E, 0x04D0}, /*Y_OUTPUT_SIZE*/
	{CMD_WRITE, 0x306E, 0xFCB0}, /*DATAPATH_SELECT*/
	{CMD_WRITE, 0x3040, 0x04C3}, /*READ_MODE*/
	{CMD_WRITE, 0x3178, 0x0000}, /*ANALOG_CONTROL5*/
	{CMD_WRITE, 0x3ED0, 0x1E24}, /*DAC_LD_4_5*/
	{CMD_WRITE, 0x0400, 0x0002}, /*SCALING_MODE*/
	{CMD_WRITE, 0x0404, 0x0010}, /*SCALE_M*/
	/*Timing configuration*/
	{CMD_WRITE, 0x0342, 0x1280}, /*LINE_LENGTH_PCK*/
	{CMD_WRITE, 0x0340, 0x0563}, /*FRAME_LENGTH_LINES*/
	{CMD_WRITE, 0x0202, 0x055F}, /*COARSE_INTEGRATION_TIME*/
	{CMD_WRITE, 0x3014, 0x0846}, /*FINE_INTEGRATION_TIME_*/
	{CMD_WRITE, 0x3010, 0x0130}, /*FINE_CORRECTION*/
};
// @preview#end

// @snapshot#start
static struct mt9e013_lgit_i2c_reg_conf snap_settings[] = 
{
	/*Output Size (3280x2464)*/
	//[2-lane MIPI 3280x2464 14.8FPS 67.6ms RAW10 Ext=24MHz Vt_pix_clk=192MHz Op_pix_clk=76.8MHz FOV=3280x2464] 
	{CMD_WRITE, 0x0344, 0x0000}, //X_ADDR_START 0
	{CMD_WRITE, 0x0348, 0x0CCF}, //X_ADDR_END 3279
	{CMD_WRITE, 0x0346, 0x0000},	//Y_ADDR_START 0
	{CMD_WRITE, 0x034A, 0x099F},	//Y_ADDR_END 2463
	{CMD_WRITE, 0x034C, 0x0CD0},	//X_OUTPUT_SIZE 3280
	{CMD_WRITE, 0x034E, 0x09A0},	//Y_OUTPUT_SIZE 2464
	{CMD_WRITE, 0x306E, 0xFC80}, /*DATAPATH_SELECT*/
	{CMD_WRITE, 0x3040, 0x0041}, /*READ_MODE*/
	{CMD_WRITE, 0x3178, 0x0000}, /*ANALOG_CONTROL5*/
	{CMD_WRITE, 0x3ED0, 0x1E24}, /*DAC_LD_4_5*/
	{CMD_WRITE, 0x0400, 0x0000}, /*SCALING_MODE*/
	{CMD_WRITE, 0x0404, 0x0010}, /*SCALE_M*/

	/*Timing configuration*/
	{CMD_WRITE, 0x0342, 0x1370}, /*LINE_LENGTH_PCK*/
	{CMD_WRITE, 0x0340, 0x0A2F}, /*FRAME_LENGTH_LINES*/
	//{0x0202, 0x0A2F}, /*COARSE_INTEGRATION_TIME*/
	{CMD_WRITE, 0x3014, 0x03F6}, /*FINE_INTEGRATION_TIME_ */
	{CMD_WRITE, 0x3010, 0x0078}, /*FINE_CORRECTION*/
};
// @snapshot#end

// @FHD#start
static struct mt9e013_lgit_i2c_reg_conf FHD_settings[] = 
{
	/*Output Size (2640x1486)*/
	{CMD_WRITE, 0x0344, 0x0140},	//X_ADDR_START 320
	{CMD_WRITE, 0x0348, 0x0B8F},	//X_ADDR_END 2959
	{CMD_WRITE, 0x0346, 0x01EA},	//Y_ADDR_START 490
	{CMD_WRITE, 0x034A, 0x07B7},	//Y_ADDR_END 1975
	{CMD_WRITE, 0x034C, 0x0A50},	//X_OUTPUT_SIZE 2640
	{CMD_WRITE, 0x034E, 0x05CE},	//Y_OUTPUT_SIZE 1486

	{CMD_WRITE, 0x306E, 0xFC80}, /*DATAPATH_SELECT*/
	{CMD_WRITE, 0x3040, 0x0041}, /*READ_MODE*/
	{CMD_WRITE, 0x3178, 0x0000}, /*ANALOG_CONTROL5*/
	{CMD_WRITE, 0x3ED0, 0x1E24}, /*DAC_LD_4_5*/
	{CMD_WRITE, 0x0400, 0x0000}, /*SCALING_MODE*/
	{CMD_WRITE, 0x0404, 0x0010}, /*SCALE_M*/
	/*Timing configuration*/
	{CMD_WRITE, 0x0342, 0x0FD8}, /*LINE_LENGTH_PCK*/	
	{CMD_WRITE, 0x0340, 0x065D}, /*FRAME_LENGTH_LINES*/
	{CMD_WRITE, 0x0202, 0x0629}, /*COARSE_INTEGRATION_TIME*/
	{CMD_WRITE, 0x3014, 0x0C82}, /*FINE_INTEGRATION_TIME_ */
	{CMD_WRITE, 0x3010, 0x0078}, /*FINE_CORRECTION*/
};
// @FHD#end

// @recommend#start
static struct mt9e013_lgit_i2c_reg_conf recommend_settings[] = 
{
	//mipi timing setting
	{CMD_WRITE, 0x31B0, 0x0083},
	{CMD_WRITE, 0x31B2, 0x004D},
	{CMD_WRITE, 0x31B4, 0x0E67},
	{CMD_WRITE, 0x31B6, 0x0D24},
	{CMD_WRITE, 0x31B8, 0x020E},
	{CMD_WRITE, 0x31BA, 0x0710},
	{CMD_WRITE, 0x31BC, 0x2A0D},
	{CMD_WRITE, 0x31BE, 0xC007},	

	//Recommended Settings
	{CMD_WRITE, 0x3044, 0x0590},
	{CMD_WRITE, 0x306E, 0xFC80},
	{CMD_WRITE, 0x30B2, 0xC000},
	{CMD_WRITE, 0x30D6, 0x0800},
	{CMD_WRITE, 0x316C, 0xB42A},
	{CMD_WRITE, 0x316E, 0x869C},
	{CMD_WRITE, 0x3170, 0x210E},
	{CMD_WRITE, 0x317A, 0x010E},
	{CMD_WRITE, 0x31E0, 0x1FB9},
	{CMD_WRITE, 0x31E6, 0x07FC},
	{CMD_WRITE, 0x37C0, 0x0000},
	{CMD_WRITE, 0x37C2, 0x0000},
	{CMD_WRITE, 0x37C4, 0x0000},
	{CMD_WRITE, 0x37C6, 0x0000},
	{CMD_WRITE, 0x3E00, 0x0011},
	{CMD_WRITE, 0x3E02, 0x8801},
	{CMD_WRITE, 0x3E04, 0x2801},
	{CMD_WRITE, 0x3E06, 0x8449},
	{CMD_WRITE, 0x3E08, 0x6841},
	{CMD_WRITE, 0x3E0A, 0x400C},
	{CMD_WRITE, 0x3E0C, 0x1001},
	{CMD_WRITE, 0x3E0E, 0x2603},
	{CMD_WRITE, 0x3E10, 0x4B41},
	{CMD_WRITE, 0x3E12, 0x4B24},
	{CMD_WRITE, 0x3E14, 0xA3CF},
	{CMD_WRITE, 0x3E16, 0x8802},
	{CMD_WRITE, 0x3E18, 0x8401},
	{CMD_WRITE, 0x3E1A, 0x8601},
	{CMD_WRITE, 0x3E1C, 0x8401},
	{CMD_WRITE, 0x3E1E, 0x840A},
	{CMD_WRITE, 0x3E20, 0xFF00},
	{CMD_WRITE, 0x3E22, 0x8401},
	{CMD_WRITE, 0x3E24, 0x00FF},
	{CMD_WRITE, 0x3E26, 0x0088},
	{CMD_WRITE, 0x3E28, 0x2E8A},
	{CMD_WRITE, 0x3E30, 0x0000},
	{CMD_WRITE, 0x3E32, 0x00FF},
	{CMD_WRITE, 0x3E34, 0x4029},
	{CMD_WRITE, 0x3E36, 0x00FF},
	{CMD_WRITE, 0x3E38, 0x8469},
	{CMD_WRITE, 0x3E3A, 0x00FF},
	{CMD_WRITE, 0x3E3C, 0x2801},
	{CMD_WRITE, 0x3E3E, 0x3E2A},
	{CMD_WRITE, 0x3E40, 0x1C01},
	{CMD_WRITE, 0x3E42, 0xFF84},
	{CMD_WRITE, 0x3E44, 0x8401},
	{CMD_WRITE, 0x3E46, 0x0C01},
	{CMD_WRITE, 0x3E48, 0x8401},
	{CMD_WRITE, 0x3E4A, 0x00FF},
	{CMD_WRITE, 0x3E4C, 0x8402},
	{CMD_WRITE, 0x3E4E, 0x8984},
	{CMD_WRITE, 0x3E50, 0x6628},
	{CMD_WRITE, 0x3E52, 0x8340},
	{CMD_WRITE, 0x3E54, 0x00FF},
	{CMD_WRITE, 0x3E56, 0x4A42},
	{CMD_WRITE, 0x3E58, 0x2703},
	{CMD_WRITE, 0x3E5A, 0x6752},
	{CMD_WRITE, 0x3E5C, 0x3F2A},
	{CMD_WRITE, 0x3E5E, 0x846A},
	{CMD_WRITE, 0x3E60, 0x4C01},
	{CMD_WRITE, 0x3E62, 0x8401},
	{CMD_WRITE, 0x3E66, 0x3901},
	{CMD_WRITE, 0x3E90, 0x2C01},
	{CMD_WRITE, 0x3E98, 0x2B02},
	{CMD_WRITE, 0x3E92, 0x2A04},
	{CMD_WRITE, 0x3E94, 0x2509},
	{CMD_WRITE, 0x3E96, 0xF000},
	{CMD_WRITE, 0x3E9A, 0x2905},
	{CMD_WRITE, 0x3E9C, 0x00FF},
	{CMD_WRITE, 0x3ECC, 0x00D8},
	{CMD_WRITE, 0x3ED0, 0x1E24},
	{CMD_WRITE, 0x3ED4, 0xFAA4},
	{CMD_WRITE, 0x3ED6, 0x909B},
	{CMD_WRITE, 0x3EE0, 0x2424},
	{CMD_WRITE, 0x3EE4, 0xC100},
	{CMD_WRITE, 0x3EE6, 0x0540},
	{CMD_WRITE, 0x3174, 0x8000},
};
// @recommend#end

struct mt9e013_lgit_reg mt9e013_lgit_regs = 
{
	.reg_mipi = &mipi_settings[0],
	.reg_mipi_size = ARRAY_SIZE(mipi_settings),
	.rec_settings = &recommend_settings[0],
	.rec_size = ARRAY_SIZE(recommend_settings),
	.reg_pll = &pll_settings[0],
	.reg_pll_size = ARRAY_SIZE(pll_settings),
	.reg_prev = &prev_settings[0],
	.reg_prev_size = ARRAY_SIZE(prev_settings),
	.reg_snap = &snap_settings[0],
	.reg_snap_size = ARRAY_SIZE(snap_settings),
	.reg_FHD = &FHD_settings[0],
	.reg_FHD_size = ARRAY_SIZE(FHD_settings),	
};

