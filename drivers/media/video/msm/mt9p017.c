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

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include <asm/mach-types.h>
#include "mt9p017.h"

#define CAMDBG(fmt, args...) printk(KERN_INFO "msm_camera: " fmt, ##args)

#define MT9P017_REG_MODEL_ID       0x0000
#define MT9P017_MODEL_ID           0x4800
#define REG_GROUPED_PARAMETER_HOLD 0x0104
#define GROUPED_PARAMETER_HOLD_OFF 0x00
#define GROUPED_PARAMETER_HOLD     0x01
#define REG_MODE_SELECT            0x0100
#define MODE_SELECT_STANDBY_MODE   0x00
#define MODE_SELECT_STREAM         0x01

/* mode setting */
#define REG_FRAME_LENGTH_LINES     0x300A
#define REG_COARSE_INT_TIME        0x3012
#define REG_GLOBAL_GAIN            0x305E
#define REG_LENS_SHADING           0x3780
#define REG_NOISE_MODEL_COEFF      0x3102

#define REG_RESET_REGISTER         0x301A
#define RESET_STREAM_READY         0x8652
#define RESET_STREAM_STOP          0x0050

#define REG_VCM_NEW_CODE           0x30F2
#define REG_VCM_CONTROL            0x30F0
#define REG_VCM_ANALOG_POWER       0x317A

#define REG_VCM_CONTROL_POWER_EN   0x8010
#define REG_VCM_ANALOG_POWER_EN    0x2000

/* TYPE DECLARATIONS */
/* 16bit address - 16 bit context register structure */
#define Q8 0x00000100
#define MT9P017_DEFAULT_MASTER_CLK_RATE 24000000

/* Full Size */
#define MT9P017_FULL_SIZE_WIDTH 2608
#define MT9P017_FULL_SIZE_HEIGHT 1960
#define MT9P017_FULL_SIZE_DUMMY_PIXELS 0
#define MT9P017_FULL_SIZE_DUMMY_LINES 0
/* Quarter Size */
#define MT9P017_QTR_SIZE_WIDTH 1300
#define MT9P017_QTR_SIZE_HEIGHT 980
#define MT9P017_QTR_SIZE_DUMMY_PIXELS 0
#define MT9P017_QTR_SIZE_DUMMY_LINES 0

/* Full Size */
#define MT9P017_HRZ_FULL_BLK_PIXELS 2672
#define MT9P017_VER_FULL_BLK_LINES 80
/* Quarter Size */
#define MT9P017_HRZ_QTR_BLK_PIXELS 2104
#define MT9P017_VER_QTR_BLK_LINES 76

/* AF Total steps parameters */
#define MT9P017_STEPS_NEAR_TO_CLOSEST_INF  32
#define MT9P017_TOTAL_STEPS_NEAR_TO_FAR    32

uint16_t mt9p017_step_position_table[MT9P017_TOTAL_STEPS_NEAR_TO_FAR+1];
uint16_t mt9p017_nl_region_boundary1 = 2;
uint16_t mt9p017_nl_region_boundary2;
uint16_t mt9p017_nl_region_code_per_step1 = 32;
uint16_t mt9p017_nl_region_code_per_step2;
uint16_t mt9p017_l_region_code_per_step = 4;
uint16_t mt9p017_damping_threshold = 10;
uint16_t mt9p017_sw_damping_time_wait = 1;

enum mt9p017_move_focus_dir {
	CAMSENSOR_MOVE_FOCUS_NEAR,
	CAMSENSOR_MOVE_FOCUS_FAR
};

struct mt9p017_work_t {
	struct work_struct work;
};

static struct mt9p017_work_t *mt9p017_sensorw;
static struct i2c_client *mt9p017_client;
static int32_t config_csi;

struct mt9p017_ctrl_t {
	const struct  msm_camera_sensor_info *sensordata;

	uint32_t sensormode;
	uint32_t fps_divider; /* init to 1 * 0x00000400 */
	uint32_t pict_fps_divider; /* init to 1 * 0x00000400 */
	uint16_t fps;

	int16_t curr_lens_pos;
	uint16_t curr_step_pos;
	uint16_t my_reg_gain;
	uint32_t my_reg_line_count;
	uint16_t total_lines_per_frame;

	enum mt9p017_resolution_t prev_res;
	enum mt9p017_resolution_t pict_res;
	enum mt9p017_resolution_t curr_res;
	enum mt9p017_test_mode_t  set_test;

	unsigned short imgaddr;
};

static uint8_t mt9p017_delay_msecs_stdby = 20;
static uint16_t mt9p017_delay_msecs_stream = 60;

/* QCTK_20110111_LSC & DPC Flag */
static int lc_option;
static int nr_option = 1;

static struct mt9p017_ctrl_t *mt9p017_ctrl;
static DECLARE_WAIT_QUEUE_HEAD(mt9p017_wait_queue);
DEFINE_MUTEX(mt9p017_mut);

static int mt9p017_i2c_rxdata(unsigned short saddr,
	unsigned char *rxdata, int length)
{
	struct i2c_msg msgs[] = {
		{
			.addr  = saddr << 1,
			.flags = 0,
			.len   = 2,
			.buf   = rxdata,
		},
		{
			.addr  = saddr << 1,
			.flags = I2C_M_RD,
			.len   = 2,
			.buf   = rxdata,
		},
	};
	if (i2c_transfer(mt9p017_client->adapter, msgs, 2) < 0) {
		CAMDBG("mt9p017_i2c_rxdata failed!\n");
		return -EIO;
	}
	return 0;
}
static int32_t mt9p017_i2c_txdata(unsigned short saddr,
				unsigned char *txdata, int length)
{
	struct i2c_msg msg[] = {
		{
			.addr = saddr << 1,
			.flags = 0,
			.len = length,
			.buf = txdata,
		 },
	};
	if (i2c_transfer(mt9p017_client->adapter, msg, 1) < 0) {
		CAMDBG("mt9p017_i2c_txdata faild 0x%x\n", mt9p017_client->addr);
		return -EIO;
	}

	return 0;
}

static int32_t mt9p017_i2c_read_w(unsigned short saddr,
	unsigned short raddr, unsigned short *rdata)
{
	int32_t rc = 0;
	unsigned char buf[4];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	buf[0] = (raddr & 0xFF00) >> 8;
	buf[1] = (raddr & 0x00FF);

	rc = mt9p017_i2c_rxdata(saddr, buf, 2);
	if (rc < 0)
		return rc;

	*rdata = buf[0] << 8 | buf[1];

	if (rc < 0)
		pr_err("mt9p017_i2c_read failed!\n");

	return rc;
}

static int32_t mt9p017_i2c_write_b_sensor(unsigned short waddr, uint8_t bdata)
{
	int32_t rc = -EFAULT;
	unsigned char buf[3];
	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00) >> 8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = bdata;
	rc = mt9p017_i2c_txdata(mt9p017_client->addr, buf, 3);
	if (rc < 0) {
		CAMDBG(" mt9p017 i2c_write_b failed, addr = 0x%x, val = 0x%x!\n",
			waddr, bdata);
	}
	return rc;
}

static int32_t mt9p017_i2c_write_w_sensor(unsigned short waddr,
	unsigned short wdata)
{
	int32_t rc = -EFAULT;
	unsigned char buf[4];
	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00) >> 8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = (wdata & 0xFF00)>>8;
	buf[3] = (wdata & 0x00FF);
	rc = mt9p017_i2c_txdata(mt9p017_client->addr, buf, 4);
	if (rc < 0) {
		CAMDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n",
			waddr, wdata);
	}
	return rc;
}
static int32_t mt9p017_i2c_write_w(unsigned short saddr,
	unsigned short waddr, unsigned short wdata)
{
	int32_t rc = -EIO;
	unsigned char buf[4];

	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00)>>8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = (wdata & 0xFF00)>>8;
	buf[3] = (wdata & 0x00FF);

	rc = mt9p017_i2c_txdata(saddr, buf, 4);
	if (rc < 0)
		pr_err("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n",
		waddr, wdata);
	return rc;
}

static int32_t mt9p017_i2c_write_w_table(
	struct mt9p017_i2c_reg_conf const *reg_conf_tbl,
	int num_of_items_in_table)
{
	int i;
	int32_t rc = -EIO;

	for (i = 0; i < num_of_items_in_table; i++) {
		rc = mt9p017_i2c_write_w(mt9p017_client->addr,
			reg_conf_tbl->waddr, reg_conf_tbl->wdata);
		if (rc < 0)
			break;
		reg_conf_tbl++;
	}

	return rc;
}


static void mt9p017_get_pict_fps(uint16_t fps, uint16_t *pfps)
{
	/* input fps is preview fps in Q8 format */
	uint16_t preview_frame_length_lines, snapshot_frame_length_lines;
	uint16_t preview_line_length_pck, snapshot_line_length_pck;
	uint32_t divider, d1, d2;
	/* Total frame_length_lines and line_length_pck for preview */
	preview_frame_length_lines = MT9P017_QTR_SIZE_HEIGHT +
		MT9P017_VER_QTR_BLK_LINES;
	preview_line_length_pck = MT9P017_QTR_SIZE_WIDTH +
		MT9P017_HRZ_QTR_BLK_PIXELS;
	/* Total frame_length_lines and line_length_pck for snapshot */
	snapshot_frame_length_lines = MT9P017_FULL_SIZE_HEIGHT +
		MT9P017_VER_FULL_BLK_LINES;
	snapshot_line_length_pck = MT9P017_FULL_SIZE_WIDTH +
		MT9P017_HRZ_FULL_BLK_PIXELS;

	d1 = preview_frame_length_lines * 0x00000400/
		snapshot_frame_length_lines;
	d2 = preview_line_length_pck * 0x00000400/
		snapshot_line_length_pck;
	divider = d1 * d2 / 0x400;
	/*Verify PCLK settings and frame sizes.*/
	*pfps = (uint16_t) (fps * divider / 0x400);
	/* 2 is the ratio of no.of snapshot channels
	to number of preview channels */
}

static int32_t mt9p017_lens_shading_enable(uint8_t is_enable)
{
	int32_t rc = 0;

	CAMDBG("%s: entered. enable = %d\n", __func__, is_enable);

	rc = mt9p017_i2c_write_b_sensor(REG_GROUPED_PARAMETER_HOLD,
			GROUPED_PARAMETER_HOLD);
	if (rc < 0)
		return rc;

	rc = mt9p017_i2c_write_w_sensor(REG_LENS_SHADING, 0x0000);
	if (rc < 0)
		return rc;

	rc = mt9p017_i2c_write_b_sensor(REG_GROUPED_PARAMETER_HOLD,
			GROUPED_PARAMETER_HOLD_OFF);

	CAMDBG("%s: exiting. rc = %d\n", __func__, rc);
	return rc;
}

static uint16_t mt9p017_get_prev_lines_pf(void)
{
	if (mt9p017_ctrl->prev_res == QTR_SIZE)
		return MT9P017_QTR_SIZE_HEIGHT + MT9P017_VER_QTR_BLK_LINES;
	else
		return MT9P017_FULL_SIZE_HEIGHT + MT9P017_VER_FULL_BLK_LINES;

}

static uint16_t mt9p017_get_prev_pixels_pl(void)
{
	if (mt9p017_ctrl->prev_res == QTR_SIZE)
		return MT9P017_QTR_SIZE_WIDTH + MT9P017_HRZ_QTR_BLK_PIXELS;
	else
		return MT9P017_FULL_SIZE_WIDTH + MT9P017_HRZ_FULL_BLK_PIXELS;
}

static uint16_t mt9p017_get_pict_lines_pf(void)
{
		if (mt9p017_ctrl->pict_res == QTR_SIZE)
			return MT9P017_QTR_SIZE_HEIGHT +
				MT9P017_VER_QTR_BLK_LINES;
		else
			return MT9P017_FULL_SIZE_HEIGHT +
				MT9P017_VER_FULL_BLK_LINES;
}

static uint16_t mt9p017_get_pict_pixels_pl(void)
{
	if (mt9p017_ctrl->pict_res == QTR_SIZE)
		return MT9P017_QTR_SIZE_WIDTH +
			MT9P017_HRZ_QTR_BLK_PIXELS;
	else
		return MT9P017_FULL_SIZE_WIDTH +
			MT9P017_HRZ_FULL_BLK_PIXELS;
}

static uint32_t mt9p017_get_pict_max_exp_lc(void)
{
	if (mt9p017_ctrl->pict_res == QTR_SIZE)
		return (MT9P017_QTR_SIZE_HEIGHT +
			MT9P017_VER_QTR_BLK_LINES)*24;
	else
		return (MT9P017_FULL_SIZE_HEIGHT +
			MT9P017_VER_FULL_BLK_LINES)*24;
}

static int32_t mt9p017_set_fps(struct fps_cfg	*fps)
{
	uint16_t total_lines_per_frame;
	int32_t rc = 0;
	total_lines_per_frame = (uint16_t)(((MT9P017_QTR_SIZE_HEIGHT +
		MT9P017_VER_QTR_BLK_LINES) * mt9p017_ctrl->fps_divider)/0x400);

	if (mt9p017_i2c_write_w_sensor(REG_FRAME_LENGTH_LINES,
		total_lines_per_frame) < 0)
		return rc;

	return rc;
}

static int32_t mt9p017_write_exp_gain(uint16_t gain, uint32_t line)
{
	uint16_t max_legal_gain = 0x0E7F;
	int32_t rc = 0;
/* LGE_UPDATE_S jeonghoon.cho@lge.com : modification QCTK */
	uint16_t gain_NR = 0;
	uint16_t line_NR = 0;
/* LGE_UPDATE_E jeonghoon.cho@lge.com : modification QCTK */

	CDBG("mt9p017_write_exp_gain entering....\n");
	if (mt9p017_ctrl->sensormode == SENSOR_PREVIEW_MODE) {
		mt9p017_ctrl->my_reg_gain = gain;
		mt9p017_ctrl->my_reg_line_count = (uint16_t) line;
	}

	if (gain > max_legal_gain)
		gain = max_legal_gain;

/* LGE_UPDATE_S jeonghoon.cho@lge.com : modification QCTK */
	gain_NR = gain;
	line_NR = line;
/* LGE_UPDATE_E jeonghoon.cho@lge.com : modification QCTK */

	if (mt9p017_ctrl->sensormode != SENSOR_SNAPSHOT_MODE)
		line = (uint32_t) (line * mt9p017_ctrl->fps_divider /
				   0x00000400);
	else
		line = (uint32_t) (line * mt9p017_ctrl->pict_fps_divider /
				   0x00000400);

	/*Set digital gain to 1 */
	gain |= 0x1000;
	CDBG("[QCTK].mt9p017_write_exp_gain Func. Gain = %d, Line = %d\n",
		gain, line);
	rc = mt9p017_i2c_write_b_sensor(REG_GROUPED_PARAMETER_HOLD,
						GROUPED_PARAMETER_HOLD);
	if (rc < 0)
		return rc;
	CDBG("mt9p017_write_exp_gain REG_GLOBAL_GAIN ....\n");

	rc = mt9p017_i2c_write_w_sensor(REG_GLOBAL_GAIN, gain);
	if (rc < 0)
		return rc;
	CDBG("mt9p017_write_exp_gain REG_COARSE_INT_TIME ....\n");

	rc = mt9p017_i2c_write_w_sensor(REG_COARSE_INT_TIME, line);
	if (rc < 0)
		return rc;

/* LGE_UPDATE_S jeonghoon.cho@lge.com : modification QCTK */
	CDBG("[mt9p017_write_NoiseReduction]Gain = %d, Line = %d\n", gain_NR,
		line_NR);

	if (gain_NR < 2135 && line_NR < 900) {
		/* around 3.0x  Indoor & Outdoor */
		rc = mt9p017_i2c_write_w_sensor(0x3100, 0x0002);
		rc = mt9p017_i2c_write_w_sensor(0x3102, 0x28); /* 25 */
		CDBG("[NOISE] Outdoor/Bright Indoor...\n");
	} else if (gain_NR < 2135) {
		/* 5.4  Indoor */
		rc = mt9p017_i2c_write_w_sensor(0x3100, 0x0002);
		rc = mt9p017_i2c_write_w_sensor(0x3102, 0x38); /* 82 */
		CDBG("[NOISE] Indoor...\n");
	} else if (gain_NR < 2172) {
		/* 7.7x  ND2 */
		rc = mt9p017_i2c_write_w_sensor(0x3100, 0x0002);
		rc = mt9p017_i2c_write_w_sensor(0x3102, 0x48); /* E0 */
		CDBG("[NOISE] Dark[ND2] Indoor...\n");
	} else {
		/* over 7.7x to 12x  ND8 */
		rc = mt9p017_i2c_write_w_sensor(0x3100, 0x0002);
		rc = mt9p017_i2c_write_w_sensor(0x3102, 0x58); /* 112 */
		CDBG("[NOISE] Low light[ND8]Indoor...\n");
	}

	if (rc < 0) {
		CAMDBG("[QCTK]Fail to write NR...\n");
		return rc;
	}
/* LGE_UPDATE_E jeonghoon.cho@lge.com : modification QCTK */

	rc = mt9p017_i2c_write_b_sensor(REG_GROUPED_PARAMETER_HOLD,
		GROUPED_PARAMETER_HOLD_OFF);
	if (rc < 0)
		return rc;
	CDBG("mt9p017_write_exp_gain exit....\n");
	return rc;
}

static int32_t mt9p017_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
	int32_t rc = 0;

	rc = mt9p017_write_exp_gain(gain, line);
	if (rc < 0)
		return rc;
	mdelay(5);

	return rc;
}

static int32_t mt9p017_set_lc(void)
{
	int32_t rc;

	rc = mt9p017_i2c_write_w_table(mt9p017_regs.lensroff_tbl,
		mt9p017_regs.lensroff_size);

	return rc;
}

static int32_t mt9p017_move_focus(int direction, int32_t num_steps)
{
	int32_t rc = 0;
	int16_t step_direction, dest_lens_position, dest_step_position;
	int16_t target_dist, small_step, next_lens_position;

	CDBG("mt9p017_move_focus entering....\n");
	if (direction == CAMSENSOR_MOVE_FOCUS_NEAR) {
		step_direction = 1;
	} else if (direction == CAMSENSOR_MOVE_FOCUS_FAR) {
		step_direction = -1;
	} else {
		pr_err("Illegal focus direction\n");
		return -EINVAL;
	}

	CDBG("mt9p017_move_focus calculating dest_step_position\n");
	dest_step_position = mt9p017_ctrl->curr_step_pos +
		(step_direction * num_steps);
	if (dest_step_position < 0)
		dest_step_position = 0;
	else if (dest_step_position > MT9P017_TOTAL_STEPS_NEAR_TO_FAR)
		dest_step_position = MT9P017_TOTAL_STEPS_NEAR_TO_FAR;

	if (dest_step_position == mt9p017_ctrl->curr_step_pos) {
		CDBG("mt9p017_move_focus ==  mt9p017_ctrl->curr_step_pos "
			"No Move exit\n");
		return rc;
	}

	CDBG("Cur Step: %hd Step Direction: %hd Dest Step Pos: %hd Num Step: "
		"%hd\n", mt9p017_ctrl->curr_step_pos, step_direction,
		dest_step_position, num_steps);

	dest_lens_position = mt9p017_step_position_table[dest_step_position];
	target_dist = step_direction *
		(dest_lens_position - mt9p017_ctrl->curr_lens_pos);
	CDBG("Target Dist: %hd\n", target_dist);

	if (step_direction < 0 && (target_dist >=
		mt9p017_step_position_table[mt9p017_damping_threshold])) {
		small_step = (uint16_t)(target_dist / 10);
		if (small_step == 0)
			small_step = 1;
		mt9p017_sw_damping_time_wait = 1;
	} else {
		small_step = (uint16_t)(target_dist / 4);
		if (small_step == 0)
			small_step = 1;
		mt9p017_sw_damping_time_wait = 4;
	}
	CDBG("mt9p017_move_focus small_step %d ...\n", small_step);

	for (next_lens_position = mt9p017_ctrl->curr_lens_pos +
		(step_direction * small_step);
		(step_direction * next_lens_position) <=
		(step_direction * dest_lens_position);
		next_lens_position += (step_direction * small_step)) {
		CDBG("mt9p017_move_focus next_lens_position %d ...\n",
			next_lens_position);
		CDBG("mt9p017_move_focus writing i2c at line %d b...\n",
			__LINE__);
		rc = mt9p017_i2c_write_w_sensor(REG_VCM_NEW_CODE,
			next_lens_position);
		if (rc < 0) {
			CAMDBG("mt9p017_move_focus failed writing i2c at %d\n",
				__LINE__);
			return rc;
		}
		CDBG("mt9p017_move_focus writing Success i2c at line %d ...\n",
			__LINE__);
		mt9p017_ctrl->curr_lens_pos = next_lens_position;
		usleep(mt9p017_sw_damping_time_wait*10);
	}

	if (mt9p017_ctrl->curr_lens_pos != dest_lens_position) {
		CDBG("mt9p017_move_focus writing i2c at line %d ...\n",
			__LINE__);
		CDBG("mt9p017_move_focus curr_lens_pos = %d "
			"dest_lens_position = %d ...\n",
			mt9p017_ctrl->curr_lens_pos, dest_lens_position);

		rc = mt9p017_i2c_write_w_sensor(REG_VCM_NEW_CODE,
			 dest_lens_position);
		if (rc < 0) {
			CAMDBG("mt9p017_move_focus failed writing i2c at %d\n",
				__LINE__);
			return rc;
		}
		CDBG("mt9p017_move_focus writing Success i2c at line %d\n",
			__LINE__);

		usleep(mt9p017_sw_damping_time_wait*10);
	}

	mt9p017_ctrl->curr_lens_pos = dest_lens_position;
	mt9p017_ctrl->curr_step_pos = dest_step_position;
	CDBG("mt9p017_move_focus exit....\n");
	return rc;
}

static void mt9p017_af_init(void)
{
	uint8_t i;
	mt9p017_step_position_table[0] = 0;
	for (i = 1; i <= MT9P017_TOTAL_STEPS_NEAR_TO_FAR; i++) {
		if (i <= mt9p017_nl_region_boundary1)
			mt9p017_step_position_table[i] =
				mt9p017_step_position_table[i-1] +
				mt9p017_nl_region_code_per_step1;
		else if (i <= mt9p017_nl_region_boundary2)
			mt9p017_step_position_table[i] =
				mt9p017_step_position_table[i-1] +
				mt9p017_nl_region_code_per_step2;
		else
			mt9p017_step_position_table[i] =
				mt9p017_step_position_table[i-1] +
				mt9p017_l_region_code_per_step;

		if (mt9p017_step_position_table[i] > 255)
			mt9p017_step_position_table[i] = 255;
	}
}

static int32_t mt9p017_set_default_focus(uint8_t af_step)
{
	int32_t rc = 0;

	CDBG("mt9p017_set_default_focus entering....\n");

	if (mt9p017_ctrl->curr_step_pos != 0) {
		rc = mt9p017_move_focus(CAMSENSOR_MOVE_FOCUS_FAR,
			mt9p017_ctrl->curr_step_pos);
		if (rc < 0)
			return rc;
	} else {
		CDBG("%s writing REG_VCM_NEW_CODE with 00\n", __func__);
		rc = mt9p017_i2c_write_w_sensor(REG_VCM_NEW_CODE, 0x0000);
		if (rc < 0) {
			CAMDBG("%s failed writing i2c at line %d ...\n",
				__func__, __LINE__);
			return rc;
		}
	}

	mt9p017_ctrl->curr_lens_pos = 0x00;
	mt9p017_ctrl->curr_step_pos = 0x00;
	CDBG("mt9p017_set_default_focus exit....\n");
	return rc;
}

static int32_t mt9p017_sensor_setting(int update_type, int rt)
{
	int32_t rc = 0;
	struct msm_camera_csi_params mt9p017_csi_params;
	switch (update_type) {
	case REG_INIT:
		if (rt == RES_PREVIEW || rt == RES_CAPTURE) {
			CAMDBG("Sensor setting Init = %d\n", rt);
			/* reset fps_divider */
			mt9p017_ctrl->fps = 30.8 * Q8;
			mt9p017_ctrl->fps_divider = 1 * 0x400;
			CAMDBG("%s: %d\n", __func__, __LINE__);

			/* stop streaming */
			rc = mt9p017_i2c_write_b_sensor(REG_MODE_SELECT,
				MODE_SELECT_STANDBY_MODE);
			if (rc < 0)
				return rc;

			msleep(mt9p017_delay_msecs_stdby);
			rc = mt9p017_i2c_write_b_sensor(
				REG_GROUPED_PARAMETER_HOLD,
				GROUPED_PARAMETER_HOLD);
				if (rc < 0)
					return rc;

			rc = mt9p017_i2c_write_w_table(mt9p017_regs.pll_tbl,
				mt9p017_regs.plltbl_size);
				if (rc < 0)
					return rc;
			msleep(10);

			rc = mt9p017_i2c_write_w_table(mt9p017_regs.init_tbl,
				mt9p017_regs.inittbl_size);
			if (rc < 0)
				return rc;

			if (lc_option) { /* QCTK_20110111_Load_LSC */
				rc = mt9p017_i2c_write_w_table(
					mt9p017_regs.lensroff_tbl,
					mt9p017_regs.lensroff_size);
				if (rc < 0)
					return rc;
			}
/* LGE_UPDATE_S jeonghoon.cho@lge.com : modification QCTK */
			if (nr_option) { /* QCTK_20110124_Setting for NR */
				rc = mt9p017_i2c_write_w_sensor(0x3100, 0x0002);
				rc = mt9p017_i2c_write_w_sensor(0x3102, 0x0064);

				rc = mt9p017_i2c_write_w_sensor(0x3104, 0x0B6D);
				rc = mt9p017_i2c_write_w_sensor(0x3106, 0x0402);
				rc = mt9p017_i2c_write_w_sensor(0x3108, 0x0b09);
				rc = mt9p017_i2c_write_w_sensor(0x310A, 0x002A);
				rc = mt9p017_i2c_write_w_sensor(0x310C, 0x0080);
				rc = mt9p017_i2c_write_w_sensor(0x310E, 0x0100);
				rc = mt9p017_i2c_write_w_sensor(0x3110, 0x0200);

				/* <DPC> */
				rc = mt9p017_i2c_write_w_sensor(0x31E0, 0x1F01);
				rc = mt9p017_i2c_write_w_sensor(0x3F02, 0x0030);
				rc = mt9p017_i2c_write_w_sensor(0x3F04, 0x0120);
				rc = mt9p017_i2c_write_w_sensor(0x3F06, 0x00F0);
				rc = mt9p017_i2c_write_w_sensor(0x3F08, 0x0170);

				if (rc < 0)
					return rc;

				CAMDBG("mt9p017_NR_Default setting is done\n");
			}
/* LGE_UPDATE_E jeonghoon.cho@lge.com : modification QCTK */
			rc = mt9p017_i2c_write_b_sensor(
				REG_GROUPED_PARAMETER_HOLD,
				GROUPED_PARAMETER_HOLD_OFF);
			if (rc < 0)
				return rc;

/* LGE_UPDATE_S jeonghoon.cho@lge.com : modification QCTK */
			CAMDBG(" MT9P017 Turn on streaming\n");
			/* turn on streaming */
			rc = mt9p017_i2c_write_b_sensor(REG_MODE_SELECT,
				MODE_SELECT_STREAM);
			if (rc < 0)
				return rc;
/* LGE_UPDATE_E jeonghoon.cho@lge.com : modification QCTK */

			CAMDBG("%s: %d\n", __func__, __LINE__);

			msleep(mt9p017_delay_msecs_stdby);
		}
		break;

	case UPDATE_PERIODIC:
		if (rt == RES_PREVIEW || rt == RES_CAPTURE) {
			CAMDBG("%s: %d\n", __func__, __LINE__);

			/* config mipi csi controller */
			if (config_csi == 0) {
				mt9p017_csi_params.lane_cnt = 2;
				mt9p017_csi_params.data_format = CSI_10BIT;
				mt9p017_csi_params.lane_assign = 0xe4;
				mt9p017_csi_params.dpcm_scheme = 0;
				mt9p017_csi_params.settle_cnt = 0x14;
				/* msm_camio_clk_rate_set(
					MT9P017_DEFAULT_MASTER_CLK_RATE); */
				CAMDBG("mt9p017 configuring csi controller\n");
				rc = msm_camio_csi_config(&mt9p017_csi_params);
				if (rc < 0)
					CAMDBG("config csi controller failed\n");
				msleep(5);
				config_csi = 1;
			}
			rc = mt9p017_i2c_write_w_sensor(REG_VCM_CONTROL,
				REG_VCM_CONTROL_POWER_EN);
			if (rc < 0)
				return rc;
			rc = mt9p017_i2c_write_w_sensor(REG_VCM_ANALOG_POWER,
				REG_VCM_ANALOG_POWER_EN);
			if (rc < 0)
				return rc;

			/* stop streaming */
			CAMDBG("Sensor setting snap or preview = %d\n", rt);

			rc = mt9p017_i2c_write_b_sensor(REG_MODE_SELECT,
				MODE_SELECT_STANDBY_MODE);
			if (rc < 0)
				return rc;

			msleep(mt9p017_delay_msecs_stdby);

			rc = mt9p017_i2c_write_b_sensor(
				REG_GROUPED_PARAMETER_HOLD,
				GROUPED_PARAMETER_HOLD);
				if (rc < 0)
					return rc;

			/* load lens shading */
			rc = mt9p017_set_lc();
			if (rc < 0)
				return rc;

			/* write mode settings */
			if (rt == RES_PREVIEW) {
				rc = mt9p017_i2c_write_w_table(
					mt9p017_regs.prev_tbl,
					mt9p017_regs.prevtbl_size);
				CAMDBG(" MT9P017 Preview configs done\n");
				if (rc < 0)
					return rc;
			} else {
				rc = mt9p017_i2c_write_w_table(
					mt9p017_regs.snap_tbl,
					mt9p017_regs.snaptbl_size);
				if (rc < 0)
					return rc;
				CAMDBG(" MT9P017 Snapshot configs done\n");
			}
			rc = mt9p017_i2c_write_w_sensor(REG_RESET_REGISTER,
				RESET_STREAM_READY);
			if (rc < 0)
				return rc;
			rc = mt9p017_i2c_write_b_sensor(
				REG_GROUPED_PARAMETER_HOLD,
				GROUPED_PARAMETER_HOLD_OFF);
			if (rc < 0)
				return rc;
			CAMDBG(" MT9P017 Turn on streaming\n");
			/* turn on streaming */
			rc = mt9p017_i2c_write_b_sensor(REG_MODE_SELECT,
				MODE_SELECT_STREAM);
			if (rc < 0)
				return rc;
			msleep(mt9p017_delay_msecs_stream);
		}
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int32_t mt9p017_video_config(int mode)
{
	int32_t	rc = 0;
	int	rt;
	CAMDBG("mt9p017_video_config entering....\n");

	/* change sensor resolution if needed */
	if (mt9p017_ctrl->prev_res == QTR_SIZE) {
		rt = RES_PREVIEW;
		mt9p017_delay_msecs_stdby =
			(((2 * 1000 * Q8 * mt9p017_ctrl->fps_divider) / 0x400)
			/ mt9p017_ctrl->fps) + 1;
	} else {
		rt = RES_CAPTURE;
		mt9p017_delay_msecs_stdby =
			(((1000 * Q8 * mt9p017_ctrl->fps_divider) / 0x400) /
			mt9p017_ctrl->fps) + 1;
	}
	CAMDBG("mt9p017_video_config mt9p017_delay_msecs_stdby = %d\n",
		mt9p017_delay_msecs_stdby);

	if (mt9p017_sensor_setting(UPDATE_PERIODIC, rt) < 0)
		return rc;

	mt9p017_ctrl->curr_res = mt9p017_ctrl->prev_res;
	mt9p017_ctrl->sensormode = mode;
	CAMDBG("mt9p017_video_config exit....\n");
	return rc;
}

static int32_t mt9p017_snapshot_config(int mode)
{
	int32_t rc = 0;
	int rt;
	CAMDBG("mt9p017_snapshot_config entering....\n");

	/* change sensor resolution if needed */
	if (mt9p017_ctrl->curr_res != mt9p017_ctrl->pict_res) {
		if (mt9p017_ctrl->pict_res == QTR_SIZE) {
			rt = RES_PREVIEW;
			mt9p017_delay_msecs_stdby =
				(((2 * 1000 * Q8 * mt9p017_ctrl->fps_divider) /
				0x400) / mt9p017_ctrl->fps) + 1;
		} else {
			rt = RES_CAPTURE;
			mt9p017_delay_msecs_stdby =
				(((1000 * Q8 * mt9p017_ctrl->fps_divider) /
				0x400) / mt9p017_ctrl->fps) + 1;
		}

		CAMDBG("Calling mt9p017_snapshot_config\n");
		if (mt9p017_sensor_setting(UPDATE_PERIODIC, rt) < 0)
			return rc;
	}
	CAMDBG("mt9p017_snapshot_config mt9p017_delay_msecs_stdby = %d\n",
		mt9p017_delay_msecs_stdby);
	mt9p017_ctrl->curr_res = mt9p017_ctrl->pict_res;
	mt9p017_ctrl->sensormode = mode;
	CAMDBG("mt9p017_snapshot_config exit....\n");
	return rc;
}

static int32_t mt9p017_raw_snapshot_config(int mode)
{
	int32_t rc = 0;
	int rt;
	CAMDBG("mt9p017_raw_snapshot_config entering....\n");
	/* change sensor resolution if needed */
	if (mt9p017_ctrl->curr_res != mt9p017_ctrl->pict_res) {
		if (mt9p017_ctrl->pict_res == QTR_SIZE) {
			rt = RES_PREVIEW;
			mt9p017_delay_msecs_stdby =
				((2 * 1000 * Q8 *
				mt9p017_ctrl->fps_divider) /
				mt9p017_ctrl->fps) + 1;
		} else {
			rt = RES_CAPTURE;
			mt9p017_delay_msecs_stdby =
				((1000 * Q8 * mt9p017_ctrl->fps_divider) /
				mt9p017_ctrl->fps) + 1;
		}
		if (mt9p017_sensor_setting(UPDATE_PERIODIC, rt) < 0)
			return rc;
	}
	mt9p017_ctrl->curr_res = mt9p017_ctrl->pict_res;
	mt9p017_ctrl->sensormode = mode;
	CAMDBG("mt9p017_raw_snapshot_config exit....\n");

	return rc;
}

static int32_t mt9p017_set_sensor_mode(int mode, int res)
{
	int32_t rc = 0;
	CDBG("mt9p017_set_sensor_mode entering....\n");

	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		rc = mt9p017_video_config(mode);
		break;
	case SENSOR_SNAPSHOT_MODE:
		rc = mt9p017_snapshot_config(mode);
		break;
	case SENSOR_RAW_SNAPSHOT_MODE:
		rc = mt9p017_raw_snapshot_config(mode);
		break;
	default:
		rc = -EINVAL;
		break;
	}
	CDBG("mt9p017_set_sensor_mode exit....\n");

	return rc;
}

static int32_t mt9p017_power_down(void)
{
	CAMDBG("mt9p017_entering power_down\n");

	mt9p017_i2c_write_w_sensor(REG_RESET_REGISTER, RESET_STREAM_STOP);
	mt9p017_i2c_write_w_sensor(REG_VCM_CONTROL, 0x0000);
	mt9p017_i2c_write_b_sensor(REG_VCM_ANALOG_POWER, 0x0000);
	msleep(mt9p017_delay_msecs_stdby);

	if (mt9p017_ctrl)
		mt9p017_ctrl->sensordata->pdata->camera_power_off();

	CAMDBG("mt9p017_entering power_down completed\n");
	return 0;
}

static int mt9p017_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
	int32_t rc = 0;
	unsigned short chipid;
	CAMDBG("%s: %d\n", __func__, __LINE__);

	CAMDBG(" mt9p017_probe_init_sensor MT9P017_REG_MODEL_ID is read\n");

	/* Read sensor Model ID: */
	rc = mt9p017_i2c_read_w(mt9p017_client->addr, MT9P017_REG_MODEL_ID,
		&chipid);
	if (rc < 0)
		goto init_probe_fail;

	CAMDBG("mt9p017 model_id = 0x%x\n", chipid);

	/* 4. Compare sensor ID to MT9P017 ID: */
	if (chipid != MT9P017_MODEL_ID) {
		rc = -ENODEV;
		printk(KERN_ERR "mt9p017 model_id = 0x%x\n", chipid);
		goto init_probe_fail;
	}

	goto init_probe_done;
init_probe_fail:
	CAMDBG(" mt9p017_probe_init_sensor fails\n");
init_probe_done:
	CAMDBG(" mt9p017_probe_init_sensor finishes\n");
	return rc;
}

int mt9p017_sensor_open_init(const struct msm_camera_sensor_info *data)
{
	int32_t rc = 0;

	CAMDBG("Calling mt9p017_sensor_open_init\n");
	mt9p017_ctrl = kzalloc(sizeof(struct mt9p017_ctrl_t), GFP_KERNEL);
	if (!mt9p017_ctrl) {
		CAMDBG("mt9p017_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}

	mt9p017_ctrl->fps_divider = 1 * 0x00000400;
	mt9p017_ctrl->pict_fps_divider = 1 * 0x00000400;
	mt9p017_ctrl->fps = 30.8 * Q8;
	mt9p017_ctrl->set_test = TEST_OFF;
	mt9p017_ctrl->prev_res = QTR_SIZE;
	mt9p017_ctrl->pict_res = FULL_SIZE;
	mt9p017_ctrl->curr_res = INVALID_SIZE;
	config_csi = 0;

	if (data)
		mt9p017_ctrl->sensordata = data;

	msm_camio_clk_rate_set(MT9P017_DEFAULT_MASTER_CLK_RATE);

	/* power on */
	data->pdata->camera_power_on();

	rc = mt9p017_probe_init_sensor(data);
	if (rc < 0)
		goto init_fail;

	CAMDBG("Calling mt9p017_af_init\n");
	mt9p017_af_init();
	CAMDBG("%s: %d\n", __func__, __LINE__);
	rc = mt9p017_sensor_setting(REG_INIT, RES_PREVIEW);
	CAMDBG("%s: %d\n", __func__, __LINE__);
	if (rc < 0)
		goto init_fail;
	else
		goto init_done;
init_fail:
	CAMDBG(" mt9p017_sensor_open_init fail\n");
	kfree(mt9p017_ctrl);
/* LGE_UPDATE_S jeonghoon.cho@lge.com : modification QCTK */
	mt9p017_ctrl = NULL;
/* LGE_UPDATE_E jeonghoon.cho@lge.com : modification QCTK */
init_done:
	CAMDBG("mt9p017_sensor_open_init done\n");
	return rc;
}

static int mt9p017_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&mt9p017_wait_queue);
	return 0;
}

static const struct i2c_device_id mt9p017_i2c_id[] = {
	{"mt9p017", 0},
	{ }
};

static int mt9p017_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CAMDBG("mt9p017 i2c_check_functionality failed\n");
		goto probe_failure;
	}

	mt9p017_sensorw = kzalloc(sizeof(struct mt9p017_work_t), GFP_KERNEL);
	if (!mt9p017_sensorw) {
		CAMDBG("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, mt9p017_sensorw);
	mt9p017_init_client(client);
	mt9p017_client = client;

	msleep(50);

	CAMDBG("mt9p017_probe successed! rc = %d\n", rc);
	return 0;

probe_failure:
	CAMDBG("mt9p017_probe failed! rc = %d\n", rc);
	return rc;
}

static int __exit mt9p017_remove(struct i2c_client *client)
{
	struct mt9p017_work_t_t *sensorw = i2c_get_clientdata(client);
	free_irq(client->irq, sensorw);
	mt9p017_client = NULL;
	kfree(sensorw);
	return 0;
}

static struct i2c_driver mt9p017_i2c_driver = {
	.id_table = mt9p017_i2c_id,
	.probe  = mt9p017_i2c_probe,
	.remove = __exit_p(mt9p017_i2c_remove),
	.driver = {
		.name = "mt9p017",
	},
};

int mt9p017_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cdata;
	long   rc = 0;
	if (copy_from_user(&cdata,
		(void *)argp,
		sizeof(struct sensor_cfg_data)))
		return -EFAULT;
	mutex_lock(&mt9p017_mut);
	CDBG("mt9p017_sensor_config: cfgtype = %d\n",
	cdata.cfgtype);
		switch (cdata.cfgtype) {
		case CFG_GET_PICT_FPS:
			mt9p017_get_pict_fps(
				cdata.cfg.gfps.prevfps,
				&(cdata.cfg.gfps.pictfps));

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PREV_L_PF:
			cdata.cfg.prevl_pf =
			mt9p017_get_prev_lines_pf();

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PREV_P_PL:
			cdata.cfg.prevp_pl =
				mt9p017_get_prev_pixels_pl();

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PICT_L_PF:
			cdata.cfg.pictl_pf =
				mt9p017_get_pict_lines_pf();

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PICT_P_PL:
			cdata.cfg.pictp_pl =
				mt9p017_get_pict_pixels_pl();

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PICT_MAX_EXP_LC:
			cdata.cfg.pict_max_exp_lc =
				mt9p017_get_pict_max_exp_lc();

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_SET_FPS:
		case CFG_SET_PICT_FPS:
			rc = mt9p017_set_fps(&(cdata.cfg.fps));
			break;

		case CFG_SET_EXP_GAIN:
			rc =
				mt9p017_write_exp_gain(
					cdata.cfg.exp_gain.gain,
					cdata.cfg.exp_gain.line);
			break;

		case CFG_SET_PICT_EXP_GAIN:
			rc =
				mt9p017_set_pict_exp_gain(
				cdata.cfg.exp_gain.gain,
				cdata.cfg.exp_gain.line);
			break;

		case CFG_SET_MODE:
			rc = mt9p017_set_sensor_mode(cdata.mode,
					cdata.rs);
			break;

		case CFG_PWR_DOWN:
			rc = mt9p017_power_down();
			break;

		case CFG_MOVE_FOCUS:
			rc =
				mt9p017_move_focus(
				cdata.cfg.focus.dir,
				cdata.cfg.focus.steps);
			break;

		case CFG_SET_DEFAULT_FOCUS:
			rc =
				mt9p017_set_default_focus(
				cdata.cfg.focus.steps);
			break;
		case CFG_SET_LENS_SHADING:
			rc = mt9p017_lens_shading_enable(
				cdata.cfg.lens_shading);
			break;
		case CFG_GET_AF_MAX_STEPS:
			cdata.max_steps = MT9P017_STEPS_NEAR_TO_CLOSEST_INF;
			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;
		case CFG_SET_EFFECT:
		default:
			rc = -EFAULT;
			break;
		}

	mutex_unlock(&mt9p017_mut);

	return rc;
}

static int mt9p017_sensor_release(void)
{
	int rc = -EBADF;
	CAMDBG("mt9p017_entering Sensor_release\n");
	mutex_lock(&mt9p017_mut);
	mt9p017_power_down();

/* LGE_UPDATE_S jeonghoon.cho@lge.com : modification QCTK */
	if (mt9p017_ctrl) {
		kfree(mt9p017_ctrl);
		mt9p017_ctrl = NULL;
	}
/* LGE_UPDATE_E jeonghoon.cho@lge.com : modification QCTK */
	CAMDBG("mt9p017_release completed\n");
	mutex_unlock(&mt9p017_mut);

	return rc;
}

static int mt9p017_sensor_probe(const struct msm_camera_sensor_info *info,
		struct msm_sensor_ctrl *s)
{
	int rc = 0;

	CAMDBG("mt9p017_sensor_probe: SENSOR PROBE entered !\n");
	rc = i2c_add_driver(&mt9p017_i2c_driver);
	if (rc < 0 || mt9p017_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_fail;
	}

	s->s_init    = mt9p017_sensor_open_init;
	s->s_release = mt9p017_sensor_release;
	s->s_config  = mt9p017_sensor_config;
	s->s_camera_type = BACK_CAMERA_2D;
	s->s_mount_angle = info->sensor_platform_info->mount_angle;

	CAMDBG("mt9p017_sensor_probe: SENSOR PROBE completed !\n");
	return rc;

probe_fail:
	CAMDBG("mt9p017_sensor_probe: SENSOR PROBE FAILS!\n");
	return rc;
}

static int __mt9p017_probe(struct platform_device *pdev)
{
	return msm_camera_drv_start(pdev, mt9p017_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __mt9p017_probe,
	.driver = {
		.name = "msm_camera_mt9p017",
		.owner = THIS_MODULE,
	},
};

static int __init mt9p017_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

module_init(mt9p017_init);

void mt9p017_exit(void)
{
	i2c_del_driver(&mt9p017_i2c_driver);
}
