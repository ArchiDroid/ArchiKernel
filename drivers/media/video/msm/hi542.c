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
 */

#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/bitops.h>
#include <mach/camera.h>
#include <media/msm_camera.h>
#include "hi542.h"

/* 16bit address - 8 bit context register structure */
#define Q8	0x00000100
#define Q10	0x00000400

/* MCLK */
#define HI542_MASTER_CLK_RATE 24000000

/*****To Do:  Start *****/ 
#define HI542_REG_PREV_FRAME_LEN_1	31 /*To Do*/
#define HI542_REG_PREV_FRAME_LEN_2	32
#define HI542_REG_PREV_LINE_LEN_1	33
#define HI542_REG_PREV_LINE_LEN_2	34

#define HI542_REG_SNAP_FRAME_LEN_1	15
#define HI542_REG_SNAP_FRAME_LEN_2	16
#define  HI542_REG_SNAP_LINE_LEN_1	17
#define HI542_REG_SNAP_LINE_LEN_2	18
/*****To Do:  End *****/ 

#define HI542_OFFSET                                     5

/* QTR Size */
#define HI542_5M_Bayer_QTR_SIZE_WIDTH                    1288 // in pixels
#define HI542_5M_Bayer_QTR_SIZE_HEIGHT                   968  // in lines
#define HI542_5M_Bayer_QTR_SIZE_BLANKING_PIXELS  1503 //        1496 // in pixels
#define HI542_5M_Bayer_QTR_SIZE_BLANKING_LINES           35   // in lines
#define HI542_5M_Bayer_QTR_SIZE_X_DECIMATION             2
#define HI542_5M_Bayer_QTR_SIZE_Y_DECIMATION             2
#define HI542_5M_Bayer_QTR_SIZE_FIRST_IMAGE_LINE         0   // Set to 2 if meta-data is output
#define HI542_5M_Bayer_QTR_SIZE_FIRST_IMAGE_PIXEL_CLOCK  0
#define HI542_5M_Bayer_QTR_SIZE_FRAME_RATE               30

/* FULL Size */
#define HI542_5M_Bayer_FULL_SIZE_WIDTH                   2576//2608 // in pixels
#define HI542_5M_Bayer_FULL_SIZE_HEIGHT                  1936//1960 // in lines
#define HI542_5M_Bayer_FULL_SIZE_BLANKING_PIXELS		215//182
#define HI542_5M_Bayer_FULL_SIZE_BLANKING_LINES          59//15  // in lines
#define HI542_5M_Bayer_FULL_SIZE_X_DECIMATION            1
#define HI542_5M_Bayer_FULL_SIZE_Y_DECIMATION            1
#define HI542_5M_Bayer_FULL_SIZE_FIRST_IMAGE_LINE        0    // Set to 2 if meta-data is output
#define HI542_5M_Bayer_FULL_SIZE_FIRST_IMAGE_PIXEL_CLOCK 0
#define HI542_5M_Bayer_FULL_SIZE_FRAME_RATE              15

#define HI542_5M_Bayer_MAX_SIZE_WIDTH                    HI542_5M_Bayer_FULL_SIZE_WIDTH
#define HI542_5M_Bayer_MAX_SIZE_HEIGHT                   HI542_5M_Bayer_FULL_SIZE_HEIGHT

/* Grouped Parameter */
#define REG_GROUPED_PARAMETER_HOLD                       0x0104
#define GROUPED_PARAMETER_HOLD_OFF                       0x00
#define GROUPED_PARAMETER_HOLD                           0x01

/* Mode Select */
#define REG_MODE_SELECT                                  0x0001
#define MODE_SELECT_STANDBY_MODE                         0x01
#define MODE_SELECT_STREAM                               0x00


/* Integration Time */
#define REG_COARSE_INTEGRATION_TIME_LB                   0x0118
#define REG_COARSE_INTEGRATION_TIME_MB1                  0x0117
#define REG_COARSE_INTEGRATION_TIME_MB2                  0x0116
#define REG_COARSE_INTEGRATION_TIME_HB                   0x0115


/* Gain */
#define REG_ANALOGUE_GAIN_CODE_GLOBAL                    0x0129

#define MSB                             1
#define LSB                             0


#define HI542_REG_MODEL_ID 		 0x0004 /*Chip ID read register*/
#define HI542_MODEL_ID     		 0xB1 /*Hynix HI542 Chip ID*/ 


//Start : shchang@qualcomm.com
/* AF Total steps parameters */
//#define HI542_TOTAL_STEPS_NEAR_TO_FAR    32

#define HI542_STEPS_NEAR_TO_CLOSEST_INF  42	//37/* Total Steps */
#define HI542_TOTAL_STEPS_NEAR_TO_FAR    42 //37

#if 0 // donghyun.kwon (20120318) : change AF table to LGIT
uint16_t hi542_step_position_table[HI542_TOTAL_STEPS_NEAR_TO_FAR+1]= {
  0,
216,
227,
233,
240,
246,
263,
285,
304,
320,
336,
349,
362,
375,
388,
402,
413,
425,
436,
444,
452,
460,
468,
478,
488,
498,
509,
514,
519,
524,
529,
534,
540,
547,
553,
559,
565,
571,
577,
583,
590,
596,
602,
};
#else
uint16_t hi542_step_position_table[HI542_TOTAL_STEPS_NEAR_TO_FAR+1];
#endif

uint16_t hi542_nl_region_boundary1 = 2;
uint16_t hi542_nl_region_boundary2 = 1;
uint16_t hi542_nl_region_code_per_step1 = 90; //32;
uint16_t hi542_nl_region_code_per_step2 = 0; // 0
uint16_t hi542_nl_region_code_per_step = 11; ///4;/* Num Gross Steps Between Stat Points */
uint16_t hi542_damping_threshold = 10;
uint16_t hi542_sw_damping_time_wait = 1;
//End : shchang@qualcomm.com

struct hi542_work_t {
	struct work_struct work;
};

static struct hi542_work_t *hi542_sensorw;
static struct hi542_work_t *hi542_af_sensorw;
static struct i2c_client *hi542_af_client;
static struct i2c_client *hi542_client;

struct hi542_ctrl_t {
	const struct  msm_camera_sensor_info *sensordata;

	uint32_t sensormode;
	uint32_t fps_divider;/* init to 1 * 0x00000400 */
	uint32_t pict_fps_divider;/* init to 1 * 0x00000400 */
	uint16_t fps;

	uint16_t curr_lens_pos;
	uint16_t curr_step_pos;
	uint16_t my_reg_gain;
	uint32_t my_reg_line_count;
	uint16_t total_lines_per_frame;

	enum hi542_resolution_t prev_res;
	enum hi542_resolution_t pict_res;
	enum hi542_resolution_t curr_res;
	enum hi542_test_mode_t  set_test;
};

static bool CSI_CONFIG;
static struct hi542_ctrl_t *hi542_ctrl;

static DECLARE_WAIT_QUEUE_HEAD(hi542_wait_queue);
static DECLARE_WAIT_QUEUE_HEAD(hi542_af_wait_queue);

DEFINE_MUTEX(hi542_mut);

static uint16_t prev_line_length_pck;
static uint16_t prev_frame_length_lines;
static uint16_t snap_line_length_pck;
static uint16_t snap_frame_length_lines;

static int hi542_i2c_rxdata(unsigned short saddr,
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
			.len   = 1,
			.buf   = rxdata,
		},
	};
	if (i2c_transfer(hi542_client->adapter, msgs, 2) < 0) {
		pr_err("hi542_i2c_rxdata faild 0x%x\n", saddr);
		return -EIO;
	}
	return 0;
}

static int32_t hi542_i2c_txdata(unsigned short saddr,
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
	if (i2c_transfer(hi542_client->adapter, msg, 1) < 0) {
		pr_err("hi542_i2c_txdata faild 0x%x\n", saddr);
		return -EIO;
	}

	return 0;
}

static int32_t hi542_i2c_read(unsigned short saddr,
	unsigned short raddr, unsigned short *rdata)
{
	int32_t rc = 0;
	unsigned char buf[4];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	buf[0] = (raddr & 0xFF00)>>8;
	buf[1] = (raddr & 0x00FF);

	rc = hi542_i2c_rxdata(saddr, buf, 2);
	if (rc < 0)
		return rc;


	*rdata = buf[0];
	//*rdata = buf[0] << 8 | buf[1];


	if (rc < 0)
		pr_err("hi542_i2c_read failed!\n");

	return rc;
}


static int32_t hi542_i2c_write_b_sensor(unsigned short waddr, uint8_t bdata)
{
	int32_t rc = -EFAULT;
	unsigned char buf[3];
	int loop;
	CDBG("%s: %d  Enter \n",__func__, __LINE__);
	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00) >> 8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = bdata;
	CDBG("i2c_write_b addr = 0x%x, val = 0x%x\n", waddr, bdata);
	for (loop = 0; loop < 3; loop++)
	{
		rc = hi542_i2c_txdata(hi542_client->addr, buf, 3);
		if((waddr == 0x0001) && (bdata == 0x02) && rc < 0) {
			CDBG("hynix SW reset called message: addr = 0x%x, val = 0x%x\n", waddr, bdata);
			continue;
		}
		if (rc < 0) {
			printk("i2c_write_b failed, addr = 0x%x, val = 0x%x\n", waddr, bdata);
		}
		else 
			break;
	}
	CDBG("%s: %d  Exit \n",__func__, __LINE__);
	return rc;
}

static int32_t hi542_i2c_write_b_table(struct hi542_i2c_reg_conf const
		*reg_conf_tbl, int num)
{
	int i;
	int32_t rc = -EIO;
	CDBG("%s: %d  Enter num: %d\n",__func__,__LINE__,num);
	for (i = 0; i < num; i++) {
		rc = hi542_i2c_write_b_sensor(reg_conf_tbl->waddr,
				reg_conf_tbl->wdata);
		if (rc < 0)
			break;
		reg_conf_tbl++;
	}
	CDBG("%s: %d  Exit \n",__func__, __LINE__);
	return rc;
}

static int32_t hi542_af_i2c_txdata(unsigned short saddr,
		unsigned char *txdata, int length)
{
	struct i2c_msg msg[] = {
		{
			.addr = saddr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		},
	};

	if (i2c_transfer(hi542_af_client->adapter, msg, 1) < 0) {
		pr_err("hi542_af_i2c_txdata faild 0x%x\n", saddr);
		return -EIO;
	}

	return 0;
}

static int32_t hi542_af_i2c_write_b_sensor(uint8_t waddr, uint8_t bdata)
{
	int32_t rc = -EFAULT;
	unsigned char buf[2];

	memset(buf, 0, sizeof(buf));
	buf[0] = waddr;
	buf[1] = bdata;
	CDBG("i2c_write_b addr = 0x%x, val = 0x%x\n", waddr, bdata);
	rc = hi542_af_i2c_txdata(hi542_af_client->addr << 1, buf, 2);
	if (rc < 0) {
		pr_err("i2c_write_b failed, addr = 0x%x, val = 0x%x!\n",
				waddr, bdata);
	}
	return rc;
}

static void hi542_start_stream(void)
{
	CDBG("%s: %d  Enter \n",__func__, __LINE__);
	hi542_i2c_write_b_sensor(0x0001, 0x00);/* streaming on */
//	hi542_i2c_write_b_sensor(0x0616, 0x01);/* mipi reset disable => samjinjang 20120415 reg file add*/
//	hi542_i2c_write_b_sensor(0x03d3, 0x08);/* mipi divider bypass */  //20120416 ryu
//  msleep(20);
	CDBG("%s: %d  Exit \n",__func__, __LINE__);
}

static void hi542_stop_stream(void)
{
	CDBG("%s: %d  Enter \n",__func__, __LINE__);
	hi542_i2c_write_b_sensor(0x0632, 0x00); //ryu add
	hi542_i2c_write_b_sensor(0x0633, 0x00);
	hi542_i2c_write_b_sensor(0x0001, 0x01);
        /* LGE_CHANGE_S : donghyun.kwon@lge.com, skip frame patch by qct */
//	hi542_i2c_write_b_sensor(0x0632, 0x0);
//	hi542_i2c_write_b_sensor(0x0633, 0x0);
//	hi542_i2c_write_b_sensor(0x0674, 0xFF);
        /* LGE_CHANGE_E : donghyun.kwon@lge.com, skip frame patch by qct */
	hi542_i2c_write_b_sensor(0x0617, 0x01);
	hi542_i2c_write_b_sensor(0x0001, 0x00);
	hi542_i2c_write_b_sensor(0x0001, 0x01);
	hi542_i2c_write_b_sensor(0x0617, 0x00);
	CDBG("%s: %d  Exit \n",__func__, __LINE__);
}

static void hi542_get_pict_fps(uint16_t fps, uint16_t *pfps)
{
	/* input fps is preview fps in Q8 format */
	uint32_t divider, d1, d2;
	CDBG("%s: %d  Enter \n",__func__, __LINE__);
	d1 = (prev_frame_length_lines * 0x00000400) / snap_frame_length_lines;
	d2 = (prev_line_length_pck * 0x00000400) / snap_line_length_pck;
	divider = (d1 * d2) / 0x400;

	/*Verify PCLK settings and frame sizes.*/
	*pfps = (uint16_t) (fps * divider / 0x400);
	CDBG("%s: %d  Exit \n",__func__, __LINE__);
}

static uint16_t hi542_get_prev_lines_pf(void)
{
	CDBG("%s: %d  Enter \n",__func__, __LINE__);
	if (hi542_ctrl->prev_res == QTR_SIZE)
		return prev_frame_length_lines;
	else
		return snap_frame_length_lines;
}

static uint16_t hi542_get_prev_pixels_pl(void)
{
	if (hi542_ctrl->prev_res == QTR_SIZE)
		return prev_line_length_pck;
	else
		return snap_line_length_pck;
}

static uint16_t hi542_get_pict_lines_pf(void)
{
	if (hi542_ctrl->pict_res == QTR_SIZE)
		return prev_frame_length_lines;
	else
		return snap_frame_length_lines;
}

static uint16_t hi542_get_pict_pixels_pl(void)
{
	if (hi542_ctrl->pict_res == QTR_SIZE)
		return prev_line_length_pck;
	else
		return snap_line_length_pck;
}

static uint32_t hi542_get_pict_max_exp_lc(void)
{
	return snap_frame_length_lines * 24;
}

static int32_t hi542_set_fps(struct fps_cfg   *fps)
{
	uint32_t total_lines_per_frame;
	int32_t rc = 0;

	hi542_ctrl->fps_divider = fps->fps_div;
	hi542_ctrl->pict_fps_divider = fps->pict_fps_div;

	if (hi542_ctrl->sensormode == SENSOR_PREVIEW_MODE) {
		total_lines_per_frame = (uint32_t)
		((prev_frame_length_lines * prev_line_length_pck * hi542_ctrl->fps_divider) / 0x400);
	CDBG("%s : sensormode = %d, prev_frame_length_lines = %d, prev_line_length_pck = %d, fps_divider = %d, total_lines_per_frame = %d", __func__,
		hi542_ctrl->sensormode, prev_frame_length_lines, prev_line_length_pck, hi542_ctrl->fps_divider, total_lines_per_frame);

	} else {
		total_lines_per_frame = (uint32_t)
		((snap_frame_length_lines * snap_line_length_pck * hi542_ctrl->fps_divider) / 0x400);
	CDBG("%s : sensormode = %d, snap_frame_length_lines = %d, snap_line_length_pck = %d, fps_divider = %d, total_lines_per_frame = %d", __func__,
		hi542_ctrl->sensormode, snap_frame_length_lines, snap_line_length_pck, hi542_ctrl->fps_divider, total_lines_per_frame);
	}

	return rc;
}

static inline uint8_t hi542_byte(uint16_t word, uint8_t offset)
{
	return word >> (offset * BITS_PER_BYTE);
}

static int32_t hi542_write_exp_gain(uint16_t gain, uint32_t line)
{
	uint32_t pixels_line = 0;
	uint8_t i = 0, mask = 0xFF;
	uint8_t values[] = { 0, 0, 0, 0, 0 };
	int rc;
	CDBG("%s: %d  Enter \n",__func__, __LINE__);

	if (hi542_ctrl->sensormode == SENSOR_PREVIEW_MODE) {
		pixels_line = line *
	   		(HI542_5M_Bayer_QTR_SIZE_WIDTH + HI542_5M_Bayer_QTR_SIZE_BLANKING_PIXELS);
	} else {
		pixels_line = line *
	   		(HI542_5M_Bayer_FULL_SIZE_WIDTH + HI542_5M_Bayer_FULL_SIZE_BLANKING_PIXELS);
	}
//	pixels_line = line *
//	   (HI542_5M_Bayer_FULL_SIZE_WIDTH + HI542_5M_Bayer_FULL_SIZE_BLANKING_PIXELS);

	CDBG("%s: sensormode = %d, gain = %d line = %d, pixels_line = %d", __func__, hi542_ctrl->sensormode, gain, line, pixels_line);

	for ( i = 1 ; i < 5; i++ ) {
	   values[i]  = ( mask & pixels_line );
	   pixels_line >>= 8;
	}
	values[0] = gain;

	rc = hi542_i2c_write_b_sensor(REG_ANALOGUE_GAIN_CODE_GLOBAL, values[0]);
	rc = hi542_i2c_write_b_sensor(REG_COARSE_INTEGRATION_TIME_LB, values[1]);
	rc = hi542_i2c_write_b_sensor(REG_COARSE_INTEGRATION_TIME_MB1, values[2]);
	rc = hi542_i2c_write_b_sensor(REG_COARSE_INTEGRATION_TIME_MB2, values[3]);
	rc = hi542_i2c_write_b_sensor(REG_COARSE_INTEGRATION_TIME_HB, values[4]);

	CDBG("%s: %d  Exit \n",__func__, __LINE__);
	return rc;
}

static int32_t hi542_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
	 int32_t rc = 0;

       /* LGE_CHANGE_S : donghyun.kwon@lge.com, skip frame patch by qct */
       if (gain == 0xFFFF && line == 0xFFFFFFFF) {
	   	printk("%s E: gain = %d, line = %d\n", __func__, gain, line);
		hi542_start_stream();
       } else {
       /* LGE_CHANGE_E : donghyun.kwon@lge.com, skip frame patch by qct */
      	hi542_stop_stream();		
	hi542_i2c_write_b_table(hi542_regs.reg_snap, hi542_regs.reg_snap_size);
	msleep(10);
	rc = hi542_write_exp_gain(gain, line);
	printk("%s X: hi542_start_stream() is triggered\n", __func__);
       }  /* LGE_CHANGE : donghyun.kwon@lge.com, skip frame patch by qct */
	return rc;
}

/* LGE_CHANGE_S : Camera AF config (by QCT)
 * 2011-12-20, donghyun.kwon@lge.com, 
 * Camera AF configuration has been changed for HI542 (Hynix) by QCT
 */
static int32_t hi542_move_focus(int direction, int32_t num_steps)
{
	//Start : shchang@qualcomm.com
	int32_t rc = 0;
	int16_t step_direction, dest_lens_position, dest_step_position;
//	 int16_t target_dist, small_step, next_lens_position;		 //Remove this part if you use slew mode.
	uint8_t code_val_msb, code_val_lsb;
	uint8_t hw_damping;

	CDBG("hi542_move_focus entering.... \n");
	if (direction == MOVE_NEAR){
		step_direction = 1;
	}
	else if (direction == MOVE_FAR){
		step_direction = -1;
	}
	else{
		printk("Illegal focus direction \n");
		return -EINVAL;
	}

	CDBG("hi542p017_move_focus calculating dest_step_position \n");

	dest_step_position = hi542_ctrl->curr_step_pos + (step_direction * num_steps);

	if (dest_step_position < 0){
		dest_step_position = 0;
	}
	else if (dest_step_position > HI542_TOTAL_STEPS_NEAR_TO_FAR){
		dest_step_position = HI542_TOTAL_STEPS_NEAR_TO_FAR;
	}				

	if(dest_step_position == hi542_ctrl->curr_step_pos){
		CDBG("hi542_move_focus ==  hi542_ctrl->curr_step_pos No Move exit \n");
		return rc;
	}
	CDBG("Cur Step: %hd Step Direction: %hd Dest Step Pos: %hd Num Step: %hd\n", hi542_ctrl->curr_step_pos, step_direction, dest_step_position, num_steps);

	dest_lens_position = hi542_step_position_table[dest_step_position];

	hw_damping = 0xF;	//0x5;//0xB;	/* LGE_CHANGED donghyun.kwon(20120317) : add workaround rutines for dampper */
	hi542_sw_damping_time_wait = 4500;// //4.5 msec

#if 0
 	if (dest_step_position <= hi542_nl_region_boundary1) {
		CDBG("__debug:MoveFocus, fine search:%d \n",
									dest_lens_position);
		/* LGE_CHANGED_S donghyun.kwon(20120317) : add workaround rutines for dampper */
		if (hi542_ctrl->curr_step_pos >= 300) {
			rc = hi542_af_i2c_write_b_sensor(0x12, 0xCF);
		}
		/* LGE_CHANGED_E donghyun.kwon(20120317) : add workaround rutines for dampper */
		
		dest_lens_position = (dest_lens_position << 4) | hw_damping;
		code_val_msb = (dest_lens_position & 0x3F00) >> 8;

		code_val_lsb = (dest_lens_position & 0x00FF);

		 rc = hi542_af_i2c_write_b_sensor(code_val_msb, code_val_lsb);
		usleep(hi542_sw_damping_time_wait);
		hi542_ctrl->curr_lens_pos = dest_lens_position;
		hi542_ctrl->curr_step_pos = dest_step_position;
		
		return 0;
	}
#endif		 
	if(hi542_ctrl->curr_lens_pos != dest_lens_position){
		CDBG("hi542_move_focus writing i2c at line %d ...\n", __LINE__);
		CDBG("hi542_move_focus curr_lens_pos = %d  dest_lens_position = %d ...\n", hi542_ctrl->curr_lens_pos, dest_lens_position);

		dest_lens_position = (dest_lens_position << 4) | hw_damping;//Slew mode enable with 50us
		code_val_msb = (dest_lens_position & 0x3F00) >> 8;
		code_val_lsb = (dest_lens_position & 0x00FF);

		rc = hi542_af_i2c_write_b_sensor(code_val_msb, code_val_lsb);

		if (rc < 0){
			CDBG("hi542_move_focus failed writing i2c at line %d ...\n", __LINE__);
			return rc;
		}
		CDBG("hi542_move_focus writing Success i2c at line %d ...\n", __LINE__);

		usleep(hi542_sw_damping_time_wait);//*1000);
	}

	hi542_ctrl->curr_lens_pos = dest_lens_position;
	hi542_ctrl->curr_step_pos = dest_step_position;

	CDBG("hi542_move_focus exit.... \n");

	return rc;
	//End : shchang@qualcomm.com
}

//Start : shchang@qualcomm.com
static void hi542_af_init(void)
{
#if 1 // donghyun.kwon (20120318) : change AF table to LGIT

    uint8_t i;
    hi542_step_position_table[0] = 0;
    for(i=1; i <= HI542_TOTAL_STEPS_NEAR_TO_FAR; i++){
        if ( i <= hi542_nl_region_boundary1){
	        hi542_step_position_table[i] = hi542_step_position_table[i-1] + hi542_nl_region_code_per_step1;
        }
//        else if ( i <= hi542_nl_region_boundary2){
//	        hi542_step_position_table[i] = hi542_step_position_table[i-1] + hi542_nl_region_code_per_step2;
//       }
        else{
	        hi542_step_position_table[i] = hi542_step_position_table[i-1] + hi542_nl_region_code_per_step;
        }
        if (hi542_step_position_table[i] > 620)
            hi542_step_position_table[i] = 620;
    }
#endif
}
//End : shchang@qualcomm.com
/* LGE_CHANGE_E : Camera AF config (by QCT) */

static int32_t hi542_set_default_focus(uint8_t af_step)
{
	int32_t rc = 0;

	if (hi542_ctrl->curr_step_pos != 0) {
		rc = hi542_move_focus(MOVE_FAR,	hi542_ctrl->curr_step_pos);
	} else {
		hi542_af_i2c_write_b_sensor(0x00, 0x0F);	/* LGE_CHANGED donghyun.kwon(20120317) : add workaround rutines for dampper */
	}

	hi542_ctrl->curr_lens_pos = 0x00;
	hi542_ctrl->curr_step_pos = 0x00;

	return rc;
}
/* LGE_CHANGE_E : Camera AF config */

static int32_t hi542_sensor_setting(int update_type, int rt)
{

	int32_t rc = 0;
	struct msm_camera_csi_params hi542_csi_params;
	pr_info("###### %s: %d Enter - update_type[%d] rt[%d]\n",__func__, __LINE__, update_type, rt);
/*LGE_CHANGE_S : Hi542 sensor stabilization
  2011-12-28, suk.kitak@lge.com, 
  apply X-shutdown mode,sensor reset stabilization*/
	//hi542_stop_stream();
/* LGE_CHANGE_E : Hi542 sensor stabilization */	
	/* TODO change msleep API */
//	msleep(10);

	if (update_type == REG_INIT) {

/*LGE_CHANGE_S : Hi542 sensor stabilization
  2011-12-28, suk.kitak@lge.com, 
  apply X-shutdown mode,sensor reset stabilization*/
	hi542_i2c_write_b_sensor(0x0001, 0x02);/* SW reset */
	hi542_i2c_write_b_sensor(0x0001, 0x01);/* SW sleep */
	hi542_i2c_write_b_sensor(0x03d4, 0x18);//20120611 LDO level change : 0x28 -> 0x18
	msleep(3);
/* LGE_CHANGE_E : Hi542 sensor stabilization */	
     
		hi542_i2c_write_b_table(hi542_regs.rec_settings,
				hi542_regs.rec_size);
	} 
	else if (update_type == UPDATE_PERIODIC) 
	{
/*LGE_CHANGE_S : Hi542 sensor stabilization
  2011-12-28, suk.kitak@lge.com, 
  apply X-shutdown mode,sensor reset stabilization*/
//        	hi542_stop_stream();
/* LGE_CHANGE_E : Hi542 sensor stabilization */	
	
		if (rt == RES_PREVIEW)
		{		
			if (!CSI_CONFIG) 
			{
				msleep(50);
				msm_camio_vfe_clk_rate_set(192000000);
				hi542_csi_params.data_format = CSI_10BIT;
				hi542_csi_params.lane_cnt = 2;
				hi542_csi_params.lane_assign = 0xe4;
				hi542_csi_params.dpcm_scheme = 0;
				hi542_csi_params.settle_cnt = 0x21; //14; settle count 20120418 ryu add
				rc = msm_camio_csi_config(&hi542_csi_params);
				pr_info("########### %s E:CSI_CONFIG \n", __func__);
				
				if (rc < 0)
					pr_err("config csi controller failed \n");
				msleep(20);
				CSI_CONFIG = 1;
			}
			else
			{
		        hi542_stop_stream();		
				msleep(10);		/* LGE_CHANGE_E : Hi542 sensor stabilization */
			}
			
			hi542_i2c_write_b_table(hi542_regs.reg_prev,
					hi542_regs.reg_prev_size);
			hi542_start_stream();
			msleep(10);
		}
	CDBG("%s: %d  Exit \n",__func__, __LINE__);
	}
	return rc;	
}

static int32_t hi542_video_config(int mode)
{

	int32_t rc = 0;
	int rt;
	CDBG("video config\n");
	/* change sensor resolution if needed */
	if (hi542_ctrl->prev_res == QTR_SIZE)
		rt = RES_PREVIEW;
	else
		rt = RES_CAPTURE;
	if (hi542_sensor_setting(UPDATE_PERIODIC, rt) < 0)
		return rc;

	hi542_ctrl->curr_res = hi542_ctrl->prev_res;
	hi542_ctrl->sensormode = mode;
	return rc;
}

static int32_t hi542_snapshot_config(int mode)
{
	int32_t rc = 0;
	int rt;

	/*change sensor resolution if needed */
	if (hi542_ctrl->curr_res != hi542_ctrl->pict_res) {
		if (hi542_ctrl->pict_res == QTR_SIZE)
			rt = RES_PREVIEW;
		else
			rt = RES_CAPTURE;
		if (hi542_sensor_setting(UPDATE_PERIODIC, rt) < 0)
			return rc;
	}

	hi542_ctrl->curr_res = hi542_ctrl->pict_res;
	hi542_ctrl->sensormode = mode;
	return rc;
}

static int32_t hi542_raw_snapshot_config(int mode)
{
	int32_t rc = 0;
	int rt;

	/* change sensor resolution if needed */
	if (hi542_ctrl->curr_res != hi542_ctrl->pict_res) {
		if (hi542_ctrl->pict_res == QTR_SIZE)
			rt = RES_PREVIEW;
		else
			rt = RES_CAPTURE;
		if (hi542_sensor_setting(UPDATE_PERIODIC, rt) < 0) {
			printk("%s: %d  Enter \n",__func__, __LINE__);
			return rc;
		}
	}

	hi542_ctrl->curr_res = hi542_ctrl->pict_res;
	hi542_ctrl->sensormode = mode;
	return rc;
}

static int32_t hi542_set_sensor_mode(int mode,
		int res)
{
	int32_t rc = 0;
	CDBG("%s: %d  Enter \n",__func__, __LINE__);
	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		rc = hi542_video_config(mode);
		break;
	case SENSOR_SNAPSHOT_MODE:
		rc = hi542_snapshot_config(mode);
		break;
	case SENSOR_RAW_SNAPSHOT_MODE:
		rc = hi542_raw_snapshot_config(mode);
		break;
	default:
		rc = -EINVAL;
		break;
	}
	pr_info("###### hi542_set_sensor_mode() mode=%d rc=%d\n", mode, rc);
	CDBG("%s: %d  Exit \n",__func__, __LINE__);
	return rc;
}

static int32_t hi542_power_down(void)
{
	int32_t rc = 0;

	CDBG("%s: %d  Enter \n",__func__, __LINE__);
	hi542_stop_stream();
	
	if (hi542_ctrl->curr_step_pos != 0) {
		if(hi542_ctrl->curr_step_pos > hi542_nl_region_boundary2) {
			rc = hi542_move_focus(MOVE_FAR, (hi542_ctrl->curr_step_pos - hi542_nl_region_boundary2));
			mdelay(10);
			rc = hi542_move_focus(MOVE_FAR, hi542_nl_region_boundary2);
		}
		else {
			rc = hi542_move_focus(MOVE_FAR,	hi542_ctrl->curr_step_pos);
		}
	}
	CDBG("%s: %d  Exit \n",__func__, __LINE__);

	return 0;
}

static int hi542_probe_init_done(const struct msm_camera_sensor_info *data)
{
	CDBG("probe done\n");
	CDBG("%s: %d  Enter \n",__func__, __LINE__);
//	gpio_free(data->sensor_reset);
	CDBG("%s: %d  Exit \n",__func__, __LINE__);
	return 0;
}

static int hi542_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
	int32_t rc = 0;
	unsigned short chipid1 = 0;

	CDBG("%s: %d\n", __func__, __LINE__);
	CDBG(" hi542_probe_init_sensor is called: Address - 0x%x\n",hi542_client->addr);
	
	rc = hi542_i2c_read(hi542_client->addr,HI542_REG_MODEL_ID, &chipid1);
	if (rc < 0)
	{
		printk(KERN_ERR "hi542_probe_init_sensor: 0x%x\n",chipid1);
		goto init_probe_fail;
	}

	if (chipid1 != HI542_MODEL_ID) {
		rc = -ENODEV;
		printk(KERN_ERR "hi542_probe_init_sensor Chip ID not equal: 0x%x\n",chipid1);
		goto init_probe_fail;
	}
	CDBG("ID: %d\n", chipid1);
	CDBG("%s: %d  Exit Success \n",__func__, __LINE__);
	return rc;

init_probe_fail:
	pr_err(" hi542_probe_init_sensor fails\n");
	gpio_set_value_cansleep(data->sensor_reset, 0);
	hi542_probe_init_done(data);

#if 0 // test_0228
	if (data->vcm_enable) {
		int ret = gpio_request(data->vcm_pwd, "hi542_af");
		if (!ret) {
			gpio_direction_output(data->vcm_pwd, 0);
			msleep(20);
			gpio_free(data->vcm_pwd);
		}
	}
#endif	
	return rc;
}

int hi542_sensor_open_init(const struct msm_camera_sensor_info *data)
{
	int32_t rc = 0;

	hi542_ctrl = kzalloc(sizeof(struct hi542_ctrl_t), GFP_KERNEL);
	if (!hi542_ctrl) {
		pr_err("hi542_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}
	hi542_ctrl->fps_divider = 1 * 0x00000400;
	hi542_ctrl->pict_fps_divider = 1 * 0x00000400;
	hi542_ctrl->set_test = TEST_OFF;
	hi542_ctrl->prev_res = QTR_SIZE;
	hi542_ctrl->pict_res = FULL_SIZE;
	CSI_CONFIG = 0;
	if (data)
		hi542_ctrl->sensordata = data;

	prev_frame_length_lines = HI542_5M_Bayer_QTR_SIZE_HEIGHT + HI542_5M_Bayer_QTR_SIZE_BLANKING_LINES - HI542_OFFSET;
	prev_line_length_pck = HI542_5M_Bayer_QTR_SIZE_WIDTH + HI542_5M_Bayer_QTR_SIZE_BLANKING_PIXELS;

	snap_frame_length_lines = HI542_5M_Bayer_FULL_SIZE_HEIGHT + HI542_5M_Bayer_FULL_SIZE_BLANKING_LINES - HI542_OFFSET;
	snap_line_length_pck = HI542_5M_Bayer_FULL_SIZE_WIDTH + HI542_5M_Bayer_FULL_SIZE_BLANKING_PIXELS;

	/* enable mclk first */
#if 0 // test_0228
	msm_camio_clk_rate_set(HI542_MASTER_CLK_RATE);

	// power on
	data->pdata->camera_power_on();
#endif	

	rc = hi542_probe_init_sensor(data);
	if (rc < 0)
		goto init_fail;

 	/* LGE_CHANGE_S : Camera AF config (by QCT)
  	 * 2011-12-20, donghyun.kwon@lge.com, 
 	 * Camera AF configuration has been changed for HI542 (Hynix) by QCT
  	 */
	//Start : shchang@qualcomm.com
	CDBG("Calling hi542_af_init \n");
	hi542_af_init();
	CDBG("%s: %d\n", __func__, __LINE__);	
	//End : shchang@qualcomm.com
 	/* LGE_CHANGE_E : Camera AF config (by QCT) */

	CDBG("init settings\n");
	if (hi542_ctrl->prev_res == QTR_SIZE)
		rc = hi542_sensor_setting(REG_INIT, RES_PREVIEW);
	else
		rc = hi542_sensor_setting(REG_INIT, RES_CAPTURE);
	hi542_ctrl->fps = 30 * Q8;

	if (hi542_ctrl->sensordata->vcm_enable) {
		CDBG("enable AF actuator, gpio = %d\n",
			 hi542_ctrl->sensordata->vcm_pwd);
		rc = gpio_request(hi542_ctrl->sensordata->vcm_pwd, "hi542_af");
		if (!rc) {
			gpio_direction_output(
				hi542_ctrl->sensordata->vcm_pwd, 1);
		} else {
			pr_err("hi542_ctrl gpio request failed!\n");
			goto init_fail;
		}
#if 0 // test_0228
		msleep(20);
		rc = hi542_set_default_focus(0);
		if (rc < 0) {
			gpio_direction_output(hi542_ctrl->sensordata->vcm_pwd, 0);
			gpio_free(hi542_ctrl->sensordata->vcm_pwd);
		}
#endif		
	}

	if (rc < 0)
		goto init_fail;
	else
		goto init_done;
init_fail:
	CSI_CONFIG = 1;
	pr_err("%s: %d Exit init_fail\n",__func__,__LINE__);
	hi542_probe_init_done(data);
	if (hi542_ctrl) {
		kfree(hi542_ctrl);
		hi542_ctrl = NULL;
	}
	return rc;
init_done:
	CSI_CONFIG = 0;
	pr_err("%s: %d Exit init_done\n", __func__,__LINE__);
	return rc;
}

static int hi542_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&hi542_wait_queue);
	return 0;
}

static int hi542_af_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&hi542_af_wait_queue);
	return 0;
}

static const struct i2c_device_id hi542_af_i2c_id[] = {
	{"hi542_af", 0},
	{ }
};

static int hi542_af_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int rc = 0;
	CDBG("hi542_af_probe called!\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("i2c_check_functionality failed\n");
		goto probe_failure;
	}

	hi542_af_sensorw = kzalloc(sizeof(struct hi542_work_t), GFP_KERNEL);
	if (!hi542_af_sensorw) {
		pr_err("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, hi542_af_sensorw);
	hi542_af_init_client(client);
	hi542_af_client = client;

	//msleep(50);	// test_0228

	CDBG("hi542_af_probe successed! rc = %d\n", rc);
	return 0;

probe_failure:
	pr_err("hi542_af_probe failed! rc = %d\n", rc);
	return rc;
}

static const struct i2c_device_id hi542_i2c_id[] = {
	{"hi542", 0},
	{ }
};

static int hi542_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int rc = 0;
	CDBG("hi542_probe called!\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("i2c_check_functionality failed\n");
		goto probe_failure;
	}

	hi542_sensorw = kzalloc(sizeof(struct hi542_work_t), GFP_KERNEL);
	if (!hi542_sensorw) {
		pr_err("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, hi542_sensorw);
	hi542_init_client(client);
	hi542_client = client;

	//msleep(50);	// test_0228

	CDBG("hi542_probe successed! rc = %d\n", rc);
	return 0;

probe_failure:
	pr_err("hi542_probe failed! rc = %d\n", rc);
	return rc;
}

static int __devexit hi542_remove(struct i2c_client *client)
{
	struct hi542_work_t *sensorw = i2c_get_clientdata(client);
	free_irq(client->irq, sensorw);
	hi542_client = NULL;
	kfree(sensorw);
	return 0;
}

static struct i2c_driver hi542_i2c_driver = {
	.id_table = hi542_i2c_id,
	.probe  = hi542_i2c_probe,
	.remove = __exit_p(hi542_i2c_remove),
	.driver = {
		.name = "hi542",
	},
};

static int __devexit hi542_af_remove(struct i2c_client *client)
{
	struct hi542_work_t *hi542_af = i2c_get_clientdata(client);

	free_irq(client->irq, hi542_af);
	hi542_af_client = NULL;
	kfree(hi542_af);
	return 0;
}


static struct i2c_driver hi542_af_i2c_driver = {
	.id_table = hi542_af_i2c_id,
	.probe  = hi542_af_i2c_probe,
	.remove = __exit_p(hi542_af_i2c_remove),
	.driver = {
		.name = "hi542_af",
	},
};

int hi542_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cdata;
	long   rc = 0;
	if (copy_from_user(&cdata,
				(void *)argp,
				sizeof(struct sensor_cfg_data)))
		return -EFAULT;
	mutex_lock(&hi542_mut);
	CDBG("hi542_sensor_config: cfgtype = %d\n",
			cdata.cfgtype);
	switch (cdata.cfgtype) {
	case CFG_GET_PICT_FPS:
		hi542_get_pict_fps(
			cdata.cfg.gfps.prevfps,
			&(cdata.cfg.gfps.pictfps));

		if (copy_to_user((void *)argp,
			&cdata,
			sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;
	case CFG_GET_PREV_L_PF:
		cdata.cfg.prevl_pf =
			hi542_get_prev_lines_pf();

		if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;
	case CFG_GET_PREV_P_PL:
		cdata.cfg.prevp_pl =
			hi542_get_prev_pixels_pl();

		if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;
	case CFG_GET_PICT_L_PF:
		cdata.cfg.pictl_pf =
			hi542_get_pict_lines_pf();

		if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;
	case CFG_GET_PICT_P_PL:
		cdata.cfg.pictp_pl =
			hi542_get_pict_pixels_pl();
		if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;
	case CFG_GET_PICT_MAX_EXP_LC:
		cdata.cfg.pict_max_exp_lc =
			hi542_get_pict_max_exp_lc();

		if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;
	case CFG_SET_FPS:
	case CFG_SET_PICT_FPS:
		rc = hi542_set_fps(&(cdata.cfg.fps));
		break;
	case CFG_SET_EXP_GAIN:
		rc = hi542_write_exp_gain(cdata.cfg.exp_gain.gain,
				cdata.cfg.exp_gain.line);
		break;
	case CFG_SET_PICT_EXP_GAIN:
		rc = hi542_set_pict_exp_gain(cdata.cfg.exp_gain.gain,
				cdata.cfg.exp_gain.line);
		break;
	case CFG_SET_MODE:
		rc = hi542_set_sensor_mode(cdata.mode, cdata.rs);
		break;
	case CFG_PWR_DOWN:
		rc = hi542_power_down();
		break;
	case CFG_MOVE_FOCUS:
		rc = hi542_move_focus(cdata.cfg.focus.dir, cdata.cfg.focus.steps);
		break;
	case CFG_SET_DEFAULT_FOCUS:
		rc = hi542_set_default_focus(cdata.cfg.focus.steps);
		break;
	case CFG_GET_AF_MAX_STEPS:
		cdata.max_steps = HI542_TOTAL_STEPS_NEAR_TO_FAR;
		if (copy_to_user((void *)argp,
					&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;
	default:
		rc = -EFAULT;
		break;
	}
	mutex_unlock(&hi542_mut);

	return rc;
}

static int hi542_sensor_release(void)
{
	int rc = -EBADF;

	mutex_lock(&hi542_mut);
	hi542_power_down();
	msleep(2);
//	gpio_set_value_cansleep(hi542_ctrl->sensordata->sensor_reset, 0);
//	usleep_range(5000, 5100);
//	gpio_free(hi542_ctrl->sensordata->sensor_reset);
	if (hi542_ctrl->sensordata->vcm_enable) {
		gpio_set_value_cansleep(hi542_ctrl->sensordata->vcm_pwd, 0);
		gpio_free(hi542_ctrl->sensordata->vcm_pwd);
	}
	if (hi542_ctrl) {
		kfree(hi542_ctrl);
		hi542_ctrl = NULL;
	}
	pr_info("###### hi542_release completed\n");
	mutex_unlock(&hi542_mut);

	return rc;
}

static int hi542_sensor_stop(void)  
{  
	hi542_stop_stream();  
	return 0;  
}  

static int hi542_sensor_probe(const struct msm_camera_sensor_info *info,
		struct msm_sensor_ctrl *s)
{
	int rc = 0;

	rc = i2c_add_driver(&hi542_i2c_driver);
	if (rc < 0 || hi542_client == NULL) {
		rc = -ENOTSUPP;
		pr_err("I2C add driver failed");
		goto probe_fail_1;
	}

	rc = i2c_add_driver(&hi542_af_i2c_driver);
	if (rc < 0 || hi542_af_client == NULL) {
		rc = -ENOTSUPP;
		pr_err("I2C add driver failed");
		goto probe_fail_2;
	}

	s->s_init = hi542_sensor_open_init;
	s->s_release = hi542_sensor_release;
	s->s_stop = hi542_sensor_stop; // QCT patch sensor stop stream //patch 6
	s->s_config  = hi542_sensor_config;
	s->s_camera_type = BACK_CAMERA_2D;
	s->s_mount_angle = info->sensor_platform_info->mount_angle;

#if 0 // test_0228
	/* Keep vcm_pwd to OUT Low */
	if (info->vcm_enable) {
		rc = gpio_request(info->vcm_pwd, "hi542_af");
		if (!rc) {
			gpio_direction_output(info->vcm_pwd, 0);
			msleep(20);
			gpio_free(info->vcm_pwd);
		} else
			return rc;
	}
#endif
	return rc;

probe_fail_2:
	i2c_del_driver(&hi542_i2c_driver);

probe_fail_1:
	pr_err("hi542_sensor_probe: SENSOR PROBE FAILS!\n");
	return rc;
}

static int __devinit hi542_probe(struct platform_device *pdev)
{
	return msm_camera_drv_start(pdev, hi542_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = hi542_probe,
	.driver = {
		.name = "msm_camera_hi542",
		.owner = THIS_MODULE,
	},
};

static int __init hi542_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

module_init(hi542_init);
MODULE_DESCRIPTION("Hynix 5 MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
