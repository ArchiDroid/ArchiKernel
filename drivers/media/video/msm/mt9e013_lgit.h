/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef MT9E013_LGIT_H
#define MT9E013_LGIT_H
#include <linux/types.h>
#include <mach/board.h>

#define REG_GROUPED_PARAMETER_HOLD 0x0104
#define GROUPED_PARAMETER_HOLD_OFF 0x00
#define GROUPED_PARAMETER_HOLD 0x01
/* Integration Time */
#define REG_COARSE_INTEGRATION_TIME 0x3012
/* Gain */
#define REG_GLOBAL_GAIN 0x305E
/* PLL registers */
#define REG_FRAME_LENGTH_LINES 0x0340
/* Test Pattern */
#define REG_TEST_PATTERN_MODE	 0x0601
#define REG_VCM_CONTROL 0x30F0
#define REG_VCM_NEW_CODE 0x30F2
#define REG_VCM_STEP_TIME 0x30F4
#define REG_LENS_SHADING 0x3780
#define LSC_ON 1
#define LSC_OFF 0

#define MT9E013_LGIT_EEPROM_SLAVE_ADDR (0xA0>>1) //EEPROM Slave Address for 5100K(Page #1)

#define Q8 0x00000100
#define Q10 0x00000400
#define MT9E013_LGIT_MASTER_CLK_RATE 24000000

#define MT9E013_LGIT_TOTAL_STEPS_NEAR_TO_FAR 42 //32

extern struct mt9e013_lgit_reg mt9e013_lgit_regs;

enum mt9e013_lgit_reg_update
{
	/* Sensor egisters that need to be updated during initialization */
	REG_INIT,
	/* Sensor egisters that needs periodic I2C writes */
	UPDATE_PERIODIC,
	/* All the sensor Registers will be updated */
	UPDATE_ALL,
	/* Not valid update */
	UPDATE_INVALID
};

enum mt9e013_lgit_reg_pll
{
	E013_VT_PIX_CLK_DIV,
	E013_VT_SYS_CLK_DIV,
	E013_PRE_PLL_CLK_DIV,
	E013_PLL_MULTIPLIER,
	E013_OP_PIX_CLK_DIV,
	E013_OP_SYS_CLK_DIV
};

enum mt9e013_lgit_reg_mode
{
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

enum mt9e013_lgit_test_mode_t
{
	TEST_OFF,
	TEST_1,
	TEST_2,
	TEST_3
};

enum mt9e013_lgit_resolution_t
{
	QTR_SIZE,
	FULL_SIZE,
	FHD_SIZE,
	INVALID_SIZE
};

enum mt9e013_lgit_setting
{
	RES_PREVIEW,
	RES_CAPTURE,
	RES_FHD
};

enum mt9e013_lgit_type
{
	CMD_WRITE = 1,
	CMD_POLL = 2,
	CMD_DELAY = 3,	
};

struct reg_struct_init
{
	uint8_t reg_0x0112; /* 0x0112*/
	uint8_t reg_0x0113; /* 0x0113*/
	uint8_t vt_pix_clk_div; /* 0x0301*/
	uint8_t pre_pll_clk_div; /* 0x0305*/
	uint8_t pll_multiplier; /* 0x0307*/
	uint8_t op_pix_clk_div; /* 0x0309*/
	uint8_t reg_0x3030; /*0x3030*/
	uint8_t reg_0x0111; /*0x0111*/
	uint8_t reg_0x0b00; /*0x0b00*/
	uint8_t reg_0x3001; /*0x3001*/
	uint8_t reg_0x3004; /*0x3004*/
	uint8_t reg_0x3007; /*0x3007*/
	uint8_t reg_0x3016; /*0x3016*/
	uint8_t reg_0x301d; /*0x301d*/
	uint8_t reg_0x317e; /*0x317E*/
	uint8_t reg_0x317f; /*0x317F*/
	uint8_t reg_0x3400; /*0x3400*/
	uint8_t reg_0x0b06; /*0x0b06*/
	uint8_t reg_0x0b07; /*0x0b07*/
	uint8_t reg_0x0b08; /*0x0b08*/
	uint8_t reg_0x0b09; /*0x0b09*/
	uint8_t reg_0x0136;
	uint8_t reg_0x0137;
	/* Edof */
	uint8_t reg_0x0b83; /*0x0b83*/
	uint8_t reg_0x0b84; /*0x0b84*/
	uint8_t reg_0x0b85; /*0x0b85*/
	uint8_t reg_0x0b88; /*0x0b88*/
	uint8_t reg_0x0b89; /*0x0b89*/
	uint8_t reg_0x0b8a; /*0x0b8a*/
};

struct reg_struct
{
	uint8_t coarse_integration_time_hi; /*REG_COARSE_INTEGRATION_TIME_HI*/
	uint8_t coarse_integration_time_lo; /*REG_COARSE_INTEGRATION_TIME_LO*/
	uint8_t analogue_gain_code_global;
	uint8_t frame_length_lines_hi; /* 0x0340*/
	uint8_t frame_length_lines_lo; /* 0x0341*/
	uint8_t line_length_pck_hi; /* 0x0342*/
	uint8_t line_length_pck_lo; /* 0x0343*/
	uint8_t reg_0x3005; /* 0x3005*/
	uint8_t reg_0x3010; /* 0x3010*/
	uint8_t reg_0x3011; /* 0x3011*/
	uint8_t reg_0x301a; /* 0x301a*/
	uint8_t reg_0x3035; /* 0x3035*/
	uint8_t reg_0x3036; /* 0x3036*/
	uint8_t reg_0x3041; /*0x3041*/
	uint8_t reg_0x3042; /*0x3042*/
	uint8_t reg_0x3045; /*0x3045*/
	uint8_t reg_0x0b80; /* 0x0b80*/
	uint8_t reg_0x0900; /*0x0900*/
	uint8_t reg_0x0901; /* 0x0901*/
	uint8_t reg_0x0902; /*0x0902*/
	uint8_t reg_0x0383; /*0x0383*/
	uint8_t reg_0x0387; /* 0x0387*/
	uint8_t reg_0x034c; /* 0x034c*/
	uint8_t reg_0x034d; /*0x034d*/
	uint8_t reg_0x034e; /* 0x034e*/
	uint8_t reg_0x034f; /* 0x034f*/
	uint8_t reg_0x1716; /*0x1716*/
	uint8_t reg_0x1717; /*0x1717*/
	uint8_t reg_0x1718; /*0x1718*/
	uint8_t reg_0x1719; /*0x1719*/
	uint8_t reg_0x3210; /*0x3210*/
	uint8_t reg_0x111; /*0x111*/
	uint8_t reg_0x3410; /*0x3410*/
	uint8_t reg_0x3098;
	uint8_t reg_0x309D;
	uint8_t reg_0x0200;
	uint8_t reg_0x0201;
};

struct mt9e013_lgit_i2c_reg_conf
{
	enum mt9e013_lgit_type register_type;
	unsigned short register_address;
	unsigned short register_data;
};

struct mt9e013_lgit_work_t
{
	struct work_struct work;
};

struct mt9e013_lgit_ctrl_t
{
	const struct  msm_camera_sensor_info *sensordata;

	uint32_t sensormode;
	uint32_t fps_divider; /* init to 1 * 0x00000400 */
	uint32_t pict_fps_divider; /* init to 1 * 0x00000400 */
	uint16_t fps;

	uint16_t curr_lens_pos;
	uint16_t curr_step_pos;
	uint16_t my_reg_gain;
	uint32_t my_reg_line_count;
	uint16_t total_lines_per_frame;

	enum mt9e013_lgit_resolution_t prev_res;
	enum mt9e013_lgit_resolution_t pict_res;
	enum mt9e013_lgit_resolution_t curr_res;
	enum mt9e013_lgit_test_mode_t  set_test;
};

struct mt9e013_lgit_reg
{
	struct mt9e013_lgit_i2c_reg_conf *reg_mipi;
	unsigned short reg_mipi_size;
	struct mt9e013_lgit_i2c_reg_conf *rec_settings;
	unsigned short rec_size;
	struct mt9e013_lgit_i2c_reg_conf *reg_pll;
	unsigned short reg_pll_size;
	struct mt9e013_lgit_i2c_reg_conf *reg_prev;
	unsigned short reg_prev_size;
	struct mt9e013_lgit_i2c_reg_conf *reg_snap;
	unsigned short reg_snap_size;
	struct mt9e013_lgit_i2c_reg_conf *reg_FHD;
	unsigned short reg_FHD_size;
};

//#define CAM_TXT_TUNING // Enable this define for camera sensor tuning via SD card
#if defined(CAM_TXT_TUNING)
//#define CAM_TXT_DIRECT_WRITE

#define CMD_BUF_MAX_LENGTH (10)
#define TOKEN_BUF_MAX_LENGTH (32)
#define DEC_VAL (1)
#define HEX_VAL (2)
#define TOKEN_START (1)
#define TOKEN_END (0)
#define TOKEN_NONE (-1)
#define CAM_TXT_TOK "@"
#define START_TAG "#start"
#define END_TAG "#end"

// NOTE: Token length available up to 24, and start with CAM_TXT_TOK
#define MIPI_TOKEN CAM_TXT_TOK"mipi"
#define PLL_TOKEN CAM_TXT_TOK"pll"
#define PREVIEW_TOKEN CAM_TXT_TOK"preview"
#define SNAPSHOT_TOKEN CAM_TXT_TOK"snapshot"
#define FHD_TOKEN CAM_TXT_TOK"FHD"
#define RECOMMEND_TOKEN CAM_TXT_TOK"recommend"

#define CAM_TXT_PATH "/sdcard"
#define CAM_TXT_NAME "mt9e013_lgit_reg.c"
#define CAM_TXT_FILE CAM_TXT_PATH"/"CAM_TXT_NAME

int cam_txt_from_file(char *token, struct mt9e013_lgit_i2c_reg_conf **data, unsigned short *size);
void cam_txt_reg_init(void);
#endif

#endif /* MT9E013_LGIT_H */

