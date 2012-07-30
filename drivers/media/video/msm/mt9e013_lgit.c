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

#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include "mt9e013_lgit.h"

uint16_t mt9e013_lgit_step_position_table[MT9E013_LGIT_TOTAL_STEPS_NEAR_TO_FAR+1];
uint16_t af_infinity = 30;
uint16_t mt9e013_lgit_nl_region_boundary1 = 0;
uint16_t mt9e013_lgit_nl_region_code_per_step1 = 0;
uint16_t mt9e013_lgit_l_region_code_per_step = 5;
uint16_t mt9e013_lgit_vcm_step_time;
uint16_t mt9e013_lgit_sw_damping_time_wait;

static uint16_t mt9e013_lgit_linear_total_step = MT9E013_LGIT_TOTAL_STEPS_NEAR_TO_FAR;
static struct mt9e013_lgit_i2c_reg_conf *lsc_data;
static struct mt9e013_lgit_work_t *mt9e013_lgit_sensorw;
static struct i2c_client *mt9e013_lgit_client;
static bool CSI_CONFIG;
static struct mt9e013_lgit_ctrl_t *mt9e013_lgit_ctrl;

static int cam_debug_init(void);
static DECLARE_WAIT_QUEUE_HEAD(mt9e013_lgit_wait_queue);
DEFINE_MUTEX(mt9e013_lgit_mut);

static int mt9e013_lgit_log_enable = 1;
#define MT9E013_LGIT_ERR(fmt, args...) {printk("[%s] " fmt, __func__, ##args);}
#define MT9E013_LGIT_LOG(fmt, args...) {if(mt9e013_lgit_log_enable & 0x01) printk("[%s] " fmt, __func__, ##args);}
#define MT9E013_LGIT_REG(fmt, args...) {if(mt9e013_lgit_log_enable & 0x02) printk("[%s] " fmt, __func__, ##args);}
#define MT9E013_LGIT_MSG(fmt, args...) {if(mt9e013_lgit_log_enable & 0x04) printk("[%s] " fmt, __func__, ##args);}

static int mt9e013_lgit_i2c_rxdata(unsigned short saddr, unsigned char *rxdata, int length)
{
	int rc = 0;
	
	struct i2c_msg msgs[] = 
	{
		{
			.addr  = saddr,
			.flags = 0,
			.len   = 2,
			.buf   = rxdata,
		},
		{
			.addr  = saddr,
			.flags = I2C_M_RD,
			.len   = 2,
			.buf   = rxdata,
		},
	};

	rc = i2c_transfer(mt9e013_lgit_client->adapter, msgs, 2);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed: %d\n", rc);
		return rc;
	}

	return rc;
}

static int mt9e013_lgit_eeprom_i2c_rxdata(uint8_t saddr, uint8_t *rxdata, int length)
{
	int rc = 0;
	
	struct i2c_msg msgs[] = 
	{
		{
			.addr  = saddr,
			.flags = 0,
			.len   = 1,
			.buf   = rxdata,
		},
		{
			.addr  = saddr,
			.flags = I2C_M_RD,
			.len   = 1,
			.buf   = rxdata,
		},
	};

	rc = i2c_transfer(mt9e013_lgit_client->adapter, msgs, 2);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed: %d\n", rc);
		return rc;
	}
	
	return rc;
}

static int mt9e013_lgit_i2c_txdata(unsigned short saddr, unsigned char *txdata, int length)
{
	int rc = 0;
	
	struct i2c_msg msg[] = 
	{
		{
			.addr = saddr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		 },
	};

	rc = i2c_transfer(mt9e013_lgit_client->adapter, msg, 1);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed: %d\n", rc);
		return rc;
	}

	return rc;
}

static int mt9e013_lgit_i2c_read(unsigned short raddr, unsigned short *rdata, int rlen)
{
	int rc = 0;
	unsigned char buf[2];
	
	if(!rdata)
	{
		MT9E013_LGIT_ERR("invalid rdata\n");
		return -EINVAL;
	}
	
	memset(buf, 0, sizeof(buf));
	buf[0] = (raddr & 0xFF00) >> 8;
	buf[1] = (raddr & 0x00FF);
	
	rc = mt9e013_lgit_i2c_rxdata(mt9e013_lgit_client->addr, buf, rlen);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("reg: 0x%x read failed: %d\n", raddr, rc);
		return rc;
	}
	
	*rdata = (rlen == 2 ? buf[0] << 8 | buf[1] : buf[0]);
	MT9E013_LGIT_REG("reg: 0x%x, data: 0x%x\n", raddr, *rdata);
	
	return rc;
}

static int mt9e013_lgit_i2c_write_w_sensor(unsigned short waddr, uint16_t wdata)
{
	int rc = 0;
	unsigned char buf[4];
	
	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00) >> 8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = (wdata & 0xFF00) >> 8;
	buf[3] = (wdata & 0x00FF);
	
	MT9E013_LGIT_REG("reg: 0x%x, data: 0x%x\n", waddr, wdata);
	
	rc = mt9e013_lgit_i2c_txdata(mt9e013_lgit_client->addr, buf, 4);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("reg: 0x%x, data: 0x%x write failed: %d\n", waddr, wdata, rc);
		return rc;
	}
	
	return rc;
}

static int mt9e013_lgit_i2c_write_b_sensor(unsigned short waddr, uint8_t bdata)
{
	int rc = 0;
	unsigned char buf[3];
	
	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00) >> 8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = bdata;

	MT9E013_LGIT_REG("reg: 0x%x, data: 0x%x\n", waddr, bdata);

	rc = mt9e013_lgit_i2c_txdata(mt9e013_lgit_client->addr, buf, 3);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("reg: 0x%x, data: 0x%x write failed: %d\n", waddr, bdata, rc);
		return rc;
	}
	
	return rc;
}

static int mt9e013_lgit_i2c_write_w_table(struct mt9e013_lgit_i2c_reg_conf const *reg_conf_tbl, int num)
{
	int i, j;
	int rc = 0;
	unsigned short read_val = 0;
	
	for(i = 0; i < num; i++)
	{
		if(reg_conf_tbl->register_type == CMD_WRITE)
		{
			rc = mt9e013_lgit_i2c_write_w_sensor(reg_conf_tbl->register_address, reg_conf_tbl->register_data);
		}
		else if(reg_conf_tbl->register_type == CMD_POLL)
		{
			for(j=0; j<10; j++)
			{
				rc = mt9e013_lgit_i2c_read(reg_conf_tbl->register_address, &read_val, 2);
				if(read_val == reg_conf_tbl->register_data)
				{
					MT9E013_LGIT_LOG("CMD_POLL reg[%d] OK: 0x%x, val: 0x%x, read: 0x%x\n", j, reg_conf_tbl->register_address, reg_conf_tbl->register_data, read_val);
					break;
				}
				msleep(10);
			}
		}
		else if(reg_conf_tbl->register_type == CMD_DELAY)
		{
			msleep(reg_conf_tbl->register_data);
		}
		else
		{
			MT9E013_LGIT_ERR("invalide reg type: %d\n", reg_conf_tbl->register_type);
			break;
		}
		
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("reg: 0x%x, data: 0x%x write failed: %d\n", reg_conf_tbl->register_address, reg_conf_tbl->register_data, rc);
			break;
		}
		reg_conf_tbl++;
	}
	return rc;
}

static int mt9e013_lgit_i2c_read_w_eeprom(uint16_t raddr, uint16_t *rdata)
{
	int rc = 0;
	unsigned char buf;
	
	if(!rdata)
	{
		MT9E013_LGIT_ERR("invalid rdata\n");
		return -EINVAL;
	}
	
	//Read 2 bytes in sequence 
	//Big Endian address:
	buf = raddr;
	buf = (raddr & 0xFF00) >> 8;
	
	rc = mt9e013_lgit_eeprom_i2c_rxdata(MT9E013_LGIT_EEPROM_SLAVE_ADDR, &buf, 1);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("reg: 0x%x read failed[1]: %d\n", raddr, rc);
		return rc;
	}
	
	*rdata = buf;
	buf = (raddr & 0x00FF);
	
	rc = mt9e013_lgit_eeprom_i2c_rxdata(MT9E013_LGIT_EEPROM_SLAVE_ADDR, &buf, 1);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("reg: 0x%x read failed[2]: %d\n", raddr, rc);
		return rc;
	}
	
	*rdata = (*rdata<<8)|buf;

	MT9E013_LGIT_REG("reg: 0x%x, data: 0x%x\n", raddr, *rdata);
		
	return rc;
}

static int mt9e013_lgit_read_5100k_data(void)
{
	int32_t i, j, n, k;
	int rc = 0;
	uint16_t raddr = 0x0001;
	uint16_t waddr = 0x3600;
	uint16_t eepromdata = 0;
	int32_t data_size = 20;	// EEPROM data size

	k=0;
	lsc_data = kzalloc(sizeof(struct mt9e013_lgit_i2c_reg_conf)*106, GFP_KERNEL);

	MT9E013_LGIT_REG("raddr: 0x%x, waddr: 0x%x\n", raddr, waddr);
	
	n = 0x01;
	for(j=0; j<5; j++)
	{
		for(i=0; i<data_size; i++) 
		{
			rc = mt9e013_lgit_i2c_read_w_eeprom(raddr, &eepromdata);
			if(rc < 0)
			{
				MT9E013_LGIT_ERR("reg: 0x%x, i:%d, j:%d read failed: %d\n", raddr, i, j, rc);
				return rc;
			}
			
			MT9E013_LGIT_REG("raddr: 0x%x, waddr: 0x%x, data: 0x%x\n", raddr, waddr, eepromdata);

			lsc_data[k].register_type = CMD_WRITE;
			lsc_data[k].register_address = waddr;
			lsc_data[k].register_data = eepromdata;
			
			raddr++;
			waddr += 2;
			n += 2;
			k++;
			raddr = (raddr<<8)|n;
		}
		waddr = waddr + 0x0018;
	}

	// Second Phase
	data_size = 2;
	waddr = 0x3782;
	for(i=0; i<data_size; i++)
	{
		rc = mt9e013_lgit_i2c_read_w_eeprom(raddr, &eepromdata);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("reg: 0x%x, i:%d, j:%d read failed: %d\n", raddr, i, j, rc);
			return rc;
		}
		
		MT9E013_LGIT_REG("raddr: 0x%x, waddr: 0x%x, data: 0x%x\n", raddr, waddr, eepromdata);

		lsc_data[k].register_type = CMD_WRITE;
		lsc_data[k].register_address = waddr;
		lsc_data[k].register_data = eepromdata;
		
		raddr++;
		waddr += 2;
		n += 2;
		k++;
		raddr = (raddr<<8)|n;
	}

	// Third Phase
	data_size = 4;
	waddr = 0x37C0;
	for(i=0;i<data_size;i++)
	{
		rc = mt9e013_lgit_i2c_read_w_eeprom(raddr, &eepromdata);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("reg: 0x%x, i:%d, j:%d read failed: %d\n", raddr, i, j, rc);
			return rc;
		}

		MT9E013_LGIT_REG("raddr: 0x%x, waddr: 0x%x, data: 0x%x\n", raddr, waddr, eepromdata);

		lsc_data[k].register_type = CMD_WRITE;
		lsc_data[k].register_address = waddr;
		lsc_data[k].register_data = eepromdata;

		raddr++;
		waddr += 2;
		n += 2;
		k++;
		raddr = (raddr<<8)|n;
	}
	
	for(i=0; i<110; i++)
	{
		MT9E013_LGIT_REG("EEPROM(%03d) waddr: 0x%04x, wdata: 0x%04x\n", i, lsc_data[i].register_address, lsc_data[i].register_data);
	}

	return rc;
}

static int mt9e013_lgit_write_5100k_data(void)
{
	int rc = 0;

	rc = mt9e013_lgit_i2c_write_w_table(lsc_data, 106); // 106: LSC data length
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed: %d\n", rc);
		return rc;
	}
	
	return rc;
}

static int mt9e013_lgit_read_awb_data(struct sensor_cfg_data *cfg, bool bresult)
{
	int rc = 0;
	uint16_t raddr = 0;
	uint16_t eepromdata = 0;

	raddr = 0xD4D5; // R/G
	rc = mt9e013_lgit_i2c_read_w_eeprom(raddr, &eepromdata);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("reg: 0x%x read failed[1]: %d\n", raddr, rc);
		return rc;
	}
	
	// If there is no data in EEPROM, Apply static value.
	if(!bresult)
	{
		cfg->cfg.calib_info.r_over_g = 0x0300; //0x005E;
	} 
	else
	{
		cfg->cfg.calib_info.r_over_g = eepromdata;
	}
	
	MT9E013_LGIT_LOG("R/G: 0x%04x\n", cfg->cfg.calib_info.r_over_g);

	raddr = 0xD6D7; // B/G
	rc = mt9e013_lgit_i2c_read_w_eeprom(raddr, &eepromdata);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("reg: 0x%x read failed[2]: %d\n", raddr, rc);
		return rc;
	}
	
	// If there is no data in EEPROM, Apply static value.
	if(!bresult)
	{
		cfg->cfg.calib_info.b_over_g = 0x0289; //0x0051;
	}
	else
	{
		cfg->cfg.calib_info.b_over_g = eepromdata;
	}
	
	MT9E013_LGIT_LOG("B/G: 0x%04x\n", cfg->cfg.calib_info.b_over_g);

	raddr = 0xD4D5; // Gr/Gb
	rc = mt9e013_lgit_i2c_read_w_eeprom(raddr, &eepromdata);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("reg: 0x%x read failed[3]: %d\n", raddr, rc);
		return rc;
	}
	
	// If there is no data in EEPROM, Apply static value.
	if(!bresult)
	{
		cfg->cfg.calib_info.gr_over_gb = 0x0300; //0x005E;
	}
	else
	{
		cfg->cfg.calib_info.gr_over_gb = eepromdata;
	}
	
	MT9E013_LGIT_LOG("GR/GB: 0x%04x\n", cfg->cfg.calib_info.gr_over_gb);

	return rc;
}

static void mt9e013_lgit_group_hold_on(void)
{
	MT9E013_LGIT_MSG("start\n");

	mt9e013_lgit_i2c_write_b_sensor(REG_GROUPED_PARAMETER_HOLD, GROUPED_PARAMETER_HOLD);
}

static void mt9e013_lgit_group_hold_off(void)
{
	MT9E013_LGIT_MSG("start\n");
	
	mt9e013_lgit_i2c_write_b_sensor(REG_GROUPED_PARAMETER_HOLD, GROUPED_PARAMETER_HOLD_OFF);
}

static void mt9e013_lgit_start_stream(void)
{
	MT9E013_LGIT_LOG("start\n");

	mt9e013_lgit_i2c_write_w_sensor(0x301A, 0x8250);
	mt9e013_lgit_i2c_write_w_sensor(0x301A, 0x8650);
	mt9e013_lgit_i2c_write_w_sensor(0x301A, 0x8658);
	mt9e013_lgit_i2c_write_b_sensor(0x0104, 0x00);
	mt9e013_lgit_i2c_write_w_sensor(0x301A, 0x065C);
}

static void mt9e013_lgit_stop_stream(void)
{
	MT9E013_LGIT_LOG("start\n");
	
	mt9e013_lgit_i2c_write_w_sensor(0x301A, 0x0058); //Start_Streaming
	mt9e013_lgit_i2c_write_w_sensor(0x301A, 0x0050); //Lock_Register
	mt9e013_lgit_i2c_write_b_sensor(0x0104, 0x01); //GROUPED_PARAMETER_HOLD   ==== 8bit 
}

static void mt9e013_lgit_get_pict_fps(uint16_t fps, uint16_t *pfps)
{
	uint32_t divider, d1, d2;

	d1 = mt9e013_lgit_regs.reg_prev[E013_FRAME_LENGTH_LINES].register_data * 0x00000400 / mt9e013_lgit_regs.reg_snap[E013_FRAME_LENGTH_LINES].register_data;
	d2 = mt9e013_lgit_regs.reg_prev[E013_LINE_LENGTH_PCK].register_data * 0x00000400 / mt9e013_lgit_regs.reg_snap[E013_LINE_LENGTH_PCK].register_data;
	divider = d1 * d2 / 0x400;
	
	MT9E013_LGIT_LOG("divider: %d, d1: %d, d2: %d\n", divider, d1, d2);

	/* Verify PCLK settings and frame sizes */
	*pfps = (uint16_t) (fps * divider / 0x400);
	/* 2 is the ratio of no.of snapshot channels to number of preview channels */

	MT9E013_LGIT_LOG("fps: %d, pfps: %d\n", fps, *pfps);
}

static uint16_t mt9e013_lgit_get_prev_lines_pf(void)
{
	MT9E013_LGIT_LOG("prev_res: %d\n", mt9e013_lgit_ctrl->prev_res);

	if(mt9e013_lgit_ctrl->prev_res == QTR_SIZE)
		return mt9e013_lgit_regs.reg_prev[E013_FRAME_LENGTH_LINES].register_data;
	else if(mt9e013_lgit_ctrl->prev_res == FHD_SIZE)
		return mt9e013_lgit_regs.reg_FHD[E013_FRAME_LENGTH_LINES].register_data;
	else
		return mt9e013_lgit_regs.reg_snap[E013_FRAME_LENGTH_LINES].register_data;
}

static uint16_t mt9e013_lgit_get_prev_pixels_pl(void)
{
	MT9E013_LGIT_LOG("prev_res: %d\n", mt9e013_lgit_ctrl->prev_res);

	if(mt9e013_lgit_ctrl->prev_res == QTR_SIZE)
		return mt9e013_lgit_regs.reg_prev[E013_LINE_LENGTH_PCK].register_data;
	else if(mt9e013_lgit_ctrl->prev_res == FHD_SIZE)
		return mt9e013_lgit_regs.reg_FHD[E013_LINE_LENGTH_PCK].register_data;
	else
		return mt9e013_lgit_regs.reg_snap[E013_LINE_LENGTH_PCK].register_data;
}

static uint16_t mt9e013_lgit_get_pict_lines_pf(void)
{
	MT9E013_LGIT_LOG("pict_res: %d\n", mt9e013_lgit_ctrl->pict_res);

	if(mt9e013_lgit_ctrl->pict_res == QTR_SIZE)
		return mt9e013_lgit_regs.reg_prev[E013_FRAME_LENGTH_LINES].register_data;
	else if(mt9e013_lgit_ctrl->pict_res == FHD_SIZE)
		return mt9e013_lgit_regs.reg_FHD[E013_FRAME_LENGTH_LINES].register_data;
	else
		return mt9e013_lgit_regs.reg_snap[E013_FRAME_LENGTH_LINES].register_data;
}

static uint16_t mt9e013_lgit_get_pict_pixels_pl(void)
{
	MT9E013_LGIT_LOG("pict_res: %d\n", mt9e013_lgit_ctrl->pict_res);

	if(mt9e013_lgit_ctrl->pict_res == QTR_SIZE)
		return mt9e013_lgit_regs.reg_prev[E013_LINE_LENGTH_PCK].register_data;
	else if(mt9e013_lgit_ctrl->pict_res == FHD_SIZE)
		return mt9e013_lgit_regs.reg_FHD[E013_LINE_LENGTH_PCK].register_data;
	else
		return mt9e013_lgit_regs.reg_snap[E013_LINE_LENGTH_PCK].register_data;
}

static uint32_t mt9e013_lgit_get_pict_max_exp_lc(void)
{
	MT9E013_LGIT_LOG("pict_res: %d\n", mt9e013_lgit_ctrl->pict_res);

	if(mt9e013_lgit_ctrl->pict_res == QTR_SIZE)
		return mt9e013_lgit_regs.reg_prev[E013_FRAME_LENGTH_LINES].register_data * 24;
	else if(mt9e013_lgit_ctrl->pict_res == FHD_SIZE)
		return mt9e013_lgit_regs.reg_FHD[E013_FRAME_LENGTH_LINES].register_data * 24;
	else
		return mt9e013_lgit_regs.reg_snap[E013_FRAME_LENGTH_LINES].register_data * 24;
}

static int mt9e013_lgit_set_fps(struct fps_cfg *fps)
{
	uint16_t total_lines_per_frame;
	int rc = 0;

	MT9E013_LGIT_LOG("curr_res: %d\n", mt9e013_lgit_ctrl->curr_res);
	
	if(mt9e013_lgit_ctrl->curr_res == QTR_SIZE)
		total_lines_per_frame =	mt9e013_lgit_regs.reg_prev[E013_FRAME_LENGTH_LINES].register_data;
	else if (mt9e013_lgit_ctrl->curr_res == FHD_SIZE)
		total_lines_per_frame =	mt9e013_lgit_regs.reg_FHD[E013_FRAME_LENGTH_LINES].register_data;
	else
		total_lines_per_frame =	mt9e013_lgit_regs.reg_snap[E013_FRAME_LENGTH_LINES].register_data;

	mt9e013_lgit_ctrl->fps_divider = fps->fps_div;
	mt9e013_lgit_ctrl->pict_fps_divider = fps->pict_fps_div;
	
	MT9E013_LGIT_LOG("total_lines_per_frame: %d, fps->fps_div: %d, fps->pict_fps_div: %d\n", total_lines_per_frame, fps->fps_div, fps->pict_fps_div);

	if(mt9e013_lgit_ctrl->curr_res == FULL_SIZE)
	{
		total_lines_per_frame = (uint16_t)(total_lines_per_frame * mt9e013_lgit_ctrl->pict_fps_divider / 0x400);
	}
	else
	{
		total_lines_per_frame = (uint16_t)(total_lines_per_frame * mt9e013_lgit_ctrl->fps_divider / 0x400);
	}
	
	MT9E013_LGIT_LOG("total_lines_per_frame: %d, fps->fps_div: %d, fps->pict_fps_div: %d\n", total_lines_per_frame, fps->fps_div, fps->pict_fps_div);

	mt9e013_lgit_group_hold_on();
	rc = mt9e013_lgit_i2c_write_w_sensor(REG_FRAME_LENGTH_LINES, total_lines_per_frame);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed: %d\n", rc);
		return rc;
	}
		
	mt9e013_lgit_group_hold_off();
	
	return rc;
}

static int mt9e013_lgit_write_exp_gain(uint16_t gain, uint32_t line)
{
	uint16_t max_legal_gain = 0xE7F;
	int rc = 0;

	MT9E013_LGIT_MSG("gain: %d, line: %d\n", gain, line);
	
	if(mt9e013_lgit_ctrl->curr_res == SENSOR_PREVIEW_MODE)
	{
		mt9e013_lgit_ctrl->my_reg_gain = gain;
		mt9e013_lgit_ctrl->my_reg_line_count = (uint16_t) line;
	}

	if(gain > max_legal_gain)
	{
		MT9E013_LGIT_LOG("Max legal gain\n");
		gain = max_legal_gain;
	}

#if 0
	if(mt9e013_lgit_ctrl->curr_res == QTR_SIZE)
	{
		frame_length_lines = mt9e013_lgit_regs.reg_prev[E013_FRAME_LENGTH_LINES].register_data;
		frame_length_lines = frame_length_lines * mt9e013_lgit_ctrl->fps_divider / 0x400;
		line = (uint32_t) (line * mt9e013_lgit_ctrl->fps_divider / 0x00000400);		
	}
	else if(mt9e013_lgit_ctrl->curr_res == FHD_SIZE)
	{
		frame_length_lines = mt9e013_lgit_regs.reg_FHD[E013_FRAME_LENGTH_LINES].register_data;
		frame_length_lines = frame_length_lines * mt9e013_lgit_ctrl->fps_divider / 0x400;
		line = (uint32_t) (line * mt9e013_lgit_ctrl->fps_divider / 0x00000400);		
	}
	else //snapshot
	{
		frame_length_lines = mt9e013_lgit_regs.reg_snap[E013_FRAME_LENGTH_LINES].register_data;
		frame_length_lines = frame_length_lines * mt9e013_lgit_ctrl->pict_fps_divider / 0x400;
		mt9e013_lgit_ctrl->pict_fps_divider = mt9e013_lgit_ctrl->fps_divider * 2;
		line = (uint32_t) (line * mt9e013_lgit_ctrl->pict_fps_divider / 0x00000400);		
	}
#else
	if(mt9e013_lgit_ctrl->sensormode != SENSOR_SNAPSHOT_MODE)
	{
		//mt9e013_lgit_ctrl->my_reg_gain = gain;
		//mt9e013_lgit_ctrl->my_reg_line_count = (uint16_t) line;
		line = (uint32_t) (line * mt9e013_lgit_ctrl->fps_divider / 0x00000400);
	}
	else
	{
		//mt9e013_lgit_ctrl->pict_fps_divider = mt9e013_lgit_ctrl->fps_divider / 2;
		line = (uint32_t) (line * mt9e013_lgit_ctrl->pict_fps_divider / 0x00000400);
	}
#endif

	gain |= 0x1000;
	MT9E013_LGIT_MSG("fps_divider: %d, pict_fps_divider: %d\n", mt9e013_lgit_ctrl->fps_divider, mt9e013_lgit_ctrl->pict_fps_divider);	
	MT9E013_LGIT_MSG("line: %d, gain: %d\n",line, gain); 

	mt9e013_lgit_group_hold_on();
	rc = mt9e013_lgit_i2c_write_w_sensor(REG_GLOBAL_GAIN, gain);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[1]: %d\n", rc);
		return rc;
	}	
	rc = mt9e013_lgit_i2c_write_w_sensor(REG_COARSE_INTEGRATION_TIME, line);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[2]: %d\n", rc);
		return rc;
	}
	mt9e013_lgit_group_hold_off();

	return rc;
}

static int mt9e013_lgit_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
	int rc = 0;

	MT9E013_LGIT_LOG("gain: %d, line: %d\n", gain, line);

	rc = mt9e013_lgit_write_exp_gain(gain, line);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[1]: %d\n", rc);
		return rc;
	}
	
	rc = mt9e013_lgit_i2c_write_w_sensor(0x301A, 0x065C|0x2);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[2]: %d\n", rc);
		return rc;
	}
	
	return rc;
}

static int mt9e013_lgit_move_focus(int direction, int32_t num_steps)
{
	int rc = 0;
	int16_t step_direction, dest_lens_position, dest_step_position;

	MT9E013_LGIT_LOG("direction: %d, num_steps: %d\n", direction, num_steps);

	if(direction == MOVE_NEAR)
		step_direction = 1;
	else
		step_direction = -1;

	dest_step_position = mt9e013_lgit_ctrl->curr_step_pos + (step_direction * num_steps);

	if(dest_step_position < 0)
		dest_step_position = 0;
	else if(dest_step_position > mt9e013_lgit_linear_total_step)
		dest_step_position = mt9e013_lgit_linear_total_step;

	if(dest_step_position == mt9e013_lgit_ctrl->curr_step_pos)
	{
		MT9E013_LGIT_LOG("same position: %d\n", dest_step_position);
		return 0;
	}
	
	MT9E013_LGIT_LOG("dest_step_position: %d\n", dest_step_position);
	dest_lens_position = mt9e013_lgit_step_position_table[dest_step_position];

	if((dest_step_position <= 4) && (step_direction == 1))
	{
		rc = mt9e013_lgit_i2c_write_w_sensor(REG_VCM_STEP_TIME, 0x0000);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("failed[1]: %d\n", rc);
			return rc;
		}
		
		if(num_steps == 4)
		{
			MT9E013_LGIT_LOG("jumpvalue: %d\n", mt9e013_lgit_nl_region_boundary1 * mt9e013_lgit_nl_region_code_per_step1);
			rc = mt9e013_lgit_i2c_write_w_sensor(REG_VCM_NEW_CODE, mt9e013_lgit_nl_region_boundary1 * mt9e013_lgit_nl_region_code_per_step1);
			if(rc < 0)
			{
				MT9E013_LGIT_ERR("failed[2]: %d\n", rc);
				return rc;
			}
		}
		else
		{
			if(dest_step_position <= mt9e013_lgit_nl_region_boundary1)
			{
				MT9E013_LGIT_LOG("fine search: %d\n", dest_lens_position);
				rc = mt9e013_lgit_i2c_write_w_sensor(REG_VCM_NEW_CODE, dest_lens_position);
				if(rc < 0)
				{
					MT9E013_LGIT_ERR("failed[3]: %d\n", rc);
					return rc;
				}
				mt9e013_lgit_ctrl->curr_lens_pos = dest_lens_position;
				mt9e013_lgit_ctrl->curr_step_pos = dest_step_position;
				
				return 0;
			}
		}
	}

	if(step_direction < 0)
	{
		if(num_steps > 20)
		{
			/*macro to infinity*/
			mt9e013_lgit_vcm_step_time = 0x0050;
			mt9e013_lgit_sw_damping_time_wait = 5;
		}
		else if(num_steps <= 4)
		{
			/*reverse search fine step  dir - macro to infinity*/
			mt9e013_lgit_vcm_step_time = 0x0400;
			mt9e013_lgit_sw_damping_time_wait = 4;
		}
		else
		{
			/*reverse search Coarse Jump ( > 4) dir - macro to infinity*/
			mt9e013_lgit_vcm_step_time = 0x96;
			mt9e013_lgit_sw_damping_time_wait = 3;
		}
	}
	else
	{
		if(num_steps >= 4)
		{
			/*coarse jump  dir - infinity to macro*/
			mt9e013_lgit_vcm_step_time = 0x0200;
			mt9e013_lgit_sw_damping_time_wait = 2;
		}
		else
		{
			/*fine step  dir - infinity to macro*/
			mt9e013_lgit_vcm_step_time = 0x0400;
			mt9e013_lgit_sw_damping_time_wait = 4;
		}
	}

	rc = mt9e013_lgit_i2c_write_w_sensor(REG_VCM_STEP_TIME, mt9e013_lgit_vcm_step_time);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[4]: %d\n", rc);
		return rc;
	}
	
	MT9E013_LGIT_LOG("vcm_step_time: %d\n", mt9e013_lgit_vcm_step_time);
	MT9E013_LGIT_LOG("dest_lens_position: %d\n", dest_lens_position);
	if(mt9e013_lgit_ctrl->curr_lens_pos != dest_lens_position)
	{
		rc = mt9e013_lgit_i2c_write_w_sensor(REG_VCM_NEW_CODE, dest_lens_position);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("failed[5]: %d\n", rc);
			return rc;
		}
		usleep(mt9e013_lgit_sw_damping_time_wait * 1000);
	}
	mt9e013_lgit_ctrl->curr_lens_pos = dest_lens_position;
	mt9e013_lgit_ctrl->curr_step_pos = dest_step_position;
	
	return 0;
}

static int mt9e013_lgit_set_default_focus(uint8_t af_step)
{
	int rc = 0;

	MT9E013_LGIT_LOG("af_step: %d, curr_step_pos: %d\n", af_step, mt9e013_lgit_ctrl->curr_step_pos);

	if(mt9e013_lgit_ctrl->curr_step_pos != 0)
	{
		rc = mt9e013_lgit_move_focus(MOVE_FAR, mt9e013_lgit_ctrl->curr_step_pos);
	}
	else
	{
		rc = mt9e013_lgit_i2c_write_w_sensor(REG_VCM_NEW_CODE, 0x00);
	}

	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed: %d\n", rc);
		return rc;
	}

	mt9e013_lgit_ctrl->curr_lens_pos = 0;
	mt9e013_lgit_ctrl->curr_step_pos = 0;

	return rc;
}

static void mt9e013_lgit_init_focus(void)
{
	uint8_t i;

	MT9E013_LGIT_LOG("start\n");

	mt9e013_lgit_step_position_table[0] = af_infinity;
	MT9E013_LGIT_MSG("mt9e013_lgit_step_position_table[%02d]: %d\n", 0, mt9e013_lgit_step_position_table[0]);
	for(i = 1; i <= mt9e013_lgit_linear_total_step; i++)
	{
		if(i <= mt9e013_lgit_nl_region_boundary1)
		{
			mt9e013_lgit_step_position_table[i] = mt9e013_lgit_step_position_table[i-1] + mt9e013_lgit_nl_region_code_per_step1;
		}
		else
		{
			mt9e013_lgit_step_position_table[i] = mt9e013_lgit_step_position_table[i-1] + mt9e013_lgit_l_region_code_per_step;
		}

		if(mt9e013_lgit_step_position_table[i] > 255)
			mt9e013_lgit_step_position_table[i] = 255;

		MT9E013_LGIT_MSG("mt9e013_lgit_step_position_table[%02d]: %d\n", i, mt9e013_lgit_step_position_table[i]);
	}
	mt9e013_lgit_ctrl->curr_lens_pos = 0;
}

static int mt9e013_lgit_test(enum mt9e013_lgit_test_mode_t mo)
{
	int rc = 0;

	MT9E013_LGIT_LOG("mo: %d\n", mo);

	if(mo == TEST_OFF)
	{
		rc = 0;
		return rc;
	}
	else
	{
		/* REG_0x30D8[4] is TESBYPEN: 0: Normal Operation, 1: Bypass Signal Processing
		    REG_0x30D8[5] is EBDMASK: 0: Output Embedded data, 1: No output embedded data */
		rc = mt9e013_lgit_i2c_write_b_sensor(REG_TEST_PATTERN_MODE, (uint8_t) mo);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("failed: %d\n", rc);
			return rc;
		}
	}
	
	return rc;
}

static int mt9e013_lgit_sensor_setting(int update_type, int rt)
{
	int rc = 0;
	uint16_t total_lines_per_frame;
	struct msm_camera_csi_params mt9e013_lgit_csi_params;

	MT9E013_LGIT_LOG("update_type: %d, rt: %d\n", update_type, rt);
	
	mt9e013_lgit_stop_stream();
	msleep(5);

	if(update_type == REG_INIT)
	{
		MT9E013_LGIT_LOG("REG_INIT\n");
		rc = mt9e013_lgit_i2c_write_w_table(mt9e013_lgit_regs.reg_mipi, mt9e013_lgit_regs.reg_mipi_size);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("failed[1-1]: %d\n", rc);
			return rc;
		}
		msleep(5);
		
		rc = mt9e013_lgit_i2c_write_w_table(mt9e013_lgit_regs.reg_pll,	mt9e013_lgit_regs.reg_pll_size);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("failed[1-2]: %d\n", rc);
			return rc;
		}
		
		rc = mt9e013_lgit_i2c_write_w_table(mt9e013_lgit_regs.rec_settings, mt9e013_lgit_regs.rec_size);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("failed[1-3]: %d\n", rc);
			return rc;
		}

		CSI_CONFIG = 0;
	}
	else if(update_type == UPDATE_PERIODIC)
	{
		if(rt == QTR_SIZE)
		{
			//mt9e013_lgit_set_default_focus(0);
#if 0	
			rc = mt9e013_lgit_i2c_write_w_table(mt9e013_lgit_regs.reg_pll, mt9e013_lgit_regs.reg_pll_size);
			if(rc < 0)
			{
				MT9E013_LGIT_ERR("failed[2-1]: %d\n", rc);
				return rc;
			}
			msleep(1);
#endif	
			rc = mt9e013_lgit_i2c_write_w_table(mt9e013_lgit_regs.reg_prev, mt9e013_lgit_regs.reg_prev_size);
			if(rc < 0)
			{
				MT9E013_LGIT_ERR("failed[2-2]: %d\n", rc);
				return rc;
			}

			total_lines_per_frame =	mt9e013_lgit_regs.reg_prev[E013_FRAME_LENGTH_LINES].register_data;
			mt9e013_lgit_group_hold_on();
			rc = mt9e013_lgit_i2c_write_w_sensor(REG_FRAME_LENGTH_LINES, total_lines_per_frame);
			if(rc < 0)
			{
				MT9E013_LGIT_ERR("failed[2-3]: %d\n", rc);
				return rc;
			}
			mt9e013_lgit_group_hold_off();
		}
		else if(rt == FHD_SIZE)
		{
			//mt9e013_lgit_set_default_focus(0);
#if 0	
			rc = mt9e013_lgit_i2c_write_w_table(mt9e013_lgit_regs.reg_pll, mt9e013_lgit_regs.reg_pll_size);
			if(rc < 0)
			{
				MT9E013_LGIT_ERR("failed[3-1]: %d\n", rc);
				return rc;
			}
			msleep(1);
#endif
			rc = mt9e013_lgit_i2c_write_w_table(mt9e013_lgit_regs.reg_FHD, mt9e013_lgit_regs.reg_FHD_size);
			if(rc < 0)
			{
				MT9E013_LGIT_ERR("failed[3-2]: %d\n", rc);
				return rc;
			}

			total_lines_per_frame =	mt9e013_lgit_regs.reg_FHD[E013_FRAME_LENGTH_LINES].register_data;
			mt9e013_lgit_group_hold_on();
			rc = mt9e013_lgit_i2c_write_w_sensor(REG_FRAME_LENGTH_LINES, total_lines_per_frame);
			if(rc < 0)
			{
				MT9E013_LGIT_ERR("failed[3-3]: %d\n", rc);
				return rc;
			}
			mt9e013_lgit_group_hold_off();
		}
		else if(rt == FULL_SIZE)
		{
#if 0		
			rc = mt9e013_lgit_i2c_write_w_table(mt9e013_lgit_regs.reg_pll, mt9e013_lgit_regs.reg_pll_size);
			if(rc < 0)
			{
				MT9E013_LGIT_ERR("failed[4-1]: %d\n", rc);
				return rc;
			}
			msleep(1);
#endif			
			rc = mt9e013_lgit_i2c_write_w_table(mt9e013_lgit_regs.reg_snap, mt9e013_lgit_regs.reg_snap_size);
			if(rc < 0)
			{
				MT9E013_LGIT_ERR("failed[4-2]: %d\n", rc);
				return rc;
			}
		} 
		
		if(!CSI_CONFIG)
		{
			if(rt != FULL_SIZE)
			{
				msm_camio_vfe_clk_rate_set(192000000);
				mt9e013_lgit_csi_params.data_format = CSI_10BIT;
				mt9e013_lgit_csi_params.lane_cnt = 2;
				mt9e013_lgit_csi_params.lane_assign = 0xe4;
				mt9e013_lgit_csi_params.dpcm_scheme = 0;
				mt9e013_lgit_csi_params.settle_cnt = 0x18;
				msm_camio_csi_config(&mt9e013_lgit_csi_params);
				msleep(10);
			}
			CSI_CONFIG = 1;
		}
		mt9e013_lgit_start_stream();
	}
	
	return rc;
}

static int mt9e013_lgit_video_config(int mode)
{
	int rc = 0;

	MT9E013_LGIT_LOG("mode: %d, prev_res: %d\n", mode, mt9e013_lgit_ctrl->prev_res);

	/* change sensor resolution if needed */
	rc = mt9e013_lgit_sensor_setting(UPDATE_PERIODIC, mt9e013_lgit_ctrl->prev_res);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[1]: %d\n", rc);
		return rc;
	}
	
	if(mt9e013_lgit_ctrl->set_test)
	{
		rc = mt9e013_lgit_test(mt9e013_lgit_ctrl->set_test);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("failed[2]: %d\n", rc);
			return rc;
		}
	}

	mt9e013_lgit_ctrl->curr_res = mt9e013_lgit_ctrl->prev_res;
	mt9e013_lgit_ctrl->sensormode = mode;
	
	return rc;
}

static int mt9e013_lgit_snapshot_config(int mode)
{
	int rc = 0;

	MT9E013_LGIT_LOG("mode: %d, curr_res: %d, pict_res: %d\n", mode, mt9e013_lgit_ctrl->curr_res, mt9e013_lgit_ctrl->pict_res);
	
	/*change sensor resolution if needed */
	if(mt9e013_lgit_ctrl->curr_res != mt9e013_lgit_ctrl->pict_res)
	{
		rc = mt9e013_lgit_sensor_setting(UPDATE_PERIODIC, mt9e013_lgit_ctrl->pict_res);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("failed: %d\n", rc);
			return rc;
		}
	}
	else
	{
		rc = 0;
	}

	mt9e013_lgit_ctrl->curr_res = mt9e013_lgit_ctrl->pict_res;
	mt9e013_lgit_ctrl->sensormode = mode;
	
	return rc;
}

static int mt9e013_lgit_raw_snapshot_config(int mode)
{
	int rc = 0;

	MT9E013_LGIT_LOG("mode: %d, curr_res: %d, pict_res: %d\n", mode, mt9e013_lgit_ctrl->curr_res, mt9e013_lgit_ctrl->pict_res);

	/* change sensor resolution if needed */
	if(mt9e013_lgit_ctrl->curr_res != mt9e013_lgit_ctrl->pict_res)
	{
		rc = mt9e013_lgit_sensor_setting(UPDATE_PERIODIC, mt9e013_lgit_ctrl->pict_res);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("failed: %d\n", rc);
			return rc;
		}
	}
	else
	{
		rc = 0;
	}

	mt9e013_lgit_ctrl->curr_res = mt9e013_lgit_ctrl->pict_res;
	mt9e013_lgit_ctrl->sensormode = mode;
	
	return rc;
}

static int mt9e013_lgit_set_sensor_mode(int mode, int res)
{
	int rc = 0;

	MT9E013_LGIT_LOG("mode: %d, res: %d\n", mode, res);

	switch(mode)
	{
		case SENSOR_PREVIEW_MODE:
			mt9e013_lgit_ctrl->prev_res = res;
			rc = mt9e013_lgit_video_config(mode);
			break;
		case SENSOR_SNAPSHOT_MODE:
			mt9e013_lgit_ctrl->pict_res = res;
			rc = mt9e013_lgit_snapshot_config(mode);
			break;
		case SENSOR_RAW_SNAPSHOT_MODE:
			mt9e013_lgit_ctrl->pict_res = res;
			rc = mt9e013_lgit_raw_snapshot_config(mode);
			break;
		default:
			rc = -EINVAL;
			break;
	}
	
	return rc;
}

static int mt9e013_lgit_af_power_down(void)
{
	int rc = 0;
	
	MT9E013_LGIT_LOG("curr_lens_pos: %d\n", mt9e013_lgit_ctrl->curr_lens_pos);

	if(mt9e013_lgit_ctrl->curr_lens_pos != 0)
	{
		rc = mt9e013_lgit_set_default_focus(0);
		if(rc < 0)
		{
			MT9E013_LGIT_ERR("failed[1]: %d\n", rc);
			return rc;
		}
		msleep(40);
	}
	
	rc = mt9e013_lgit_i2c_write_w_sensor(REG_VCM_CONTROL, 0x00);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[2]: %d\n", rc);
		return rc;
	}
	
	return rc;
}

static int mt9e013_lgit_power_down(void)
{
	int rc = 0;
	
	MT9E013_LGIT_LOG("start\n");
	
	rc = mt9e013_lgit_af_power_down();
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed: %d\n", rc);
		return rc;
	}

	return rc;
}

static int mt9e013_lgit_probe_init_done(const struct msm_camera_sensor_info *data)
{
	MT9E013_LGIT_LOG("sensor_reset: %d\n", data->sensor_reset);
	
	gpio_free(data->sensor_reset);
	mt9e013_lgit_ctrl->sensordata->pdata->camera_power_off();

	return 0;
}

static int mt9e013_lgit_lens_shading_enable(uint8_t is_enable)
{
	int rc = 0;

	MT9E013_LGIT_LOG("is_enable: %d\n", is_enable);

	mt9e013_lgit_group_hold_on();

	rc = mt9e013_lgit_i2c_write_w_sensor(REG_LENS_SHADING, ((uint16_t) is_enable) << 15);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed: %d\n", rc);
		return rc;
	}

	mt9e013_lgit_group_hold_off();

	return rc;
}

static int mt9e013_lgit_read_eeprom_data(struct sensor_cfg_data *cfg)
{
	int rc = 0;
	uint16_t eepromdata = 0;
	uint16_t addr = 0;
	bool bresult = false;

	MT9E013_LGIT_LOG("start\n");	

	/*#1. Model ID for checking EEPROM READ*/
	addr = 0xFEFF;
	rc = mt9e013_lgit_i2c_read_w_eeprom(addr, &eepromdata);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[1]: %d\n", rc);
		return rc;
	}

	if(eepromdata == 0x0AFF)
		bresult = true;
	else
		bresult = false;

	MT9E013_LGIT_LOG("Reading EEPROM @ addr: 0x%04x, data: 0x%04x\n, bresult: %d", addr, eepromdata, bresult);
	
	/*#2. 5100K LSC : Read LSC table Data from EEPROM */
	rc = mt9e013_lgit_read_5100k_data(); // read LSC data
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[2]: %d\n", rc);
		return rc;
	}
	
	/*#3. 5100K AWB Data from EEPROM */
	rc = mt9e013_lgit_read_awb_data(cfg, bresult);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[3]: %d\n", rc);
		return rc;
	}
	
	MT9E013_LGIT_LOG("AWB: r/g:0x%04x, b/g:0x%04x, gr/gb:0x%04x\n", cfg->cfg.calib_info.r_over_g, cfg->cfg.calib_info.b_over_g, cfg->cfg.calib_info.gr_over_gb);
	
	/*#4. Write LSC data to sensor - it will be enabled in setting routine*/
	mt9e013_lgit_group_hold_on();

	//Write LSC table to the sensor
	rc = mt9e013_lgit_write_5100k_data();
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[4]: %d\n", rc);
		return rc;
	}

	mt9e013_lgit_group_hold_off();

	/*Enable Aptina Lens shading */
	rc = mt9e013_lgit_lens_shading_enable(LSC_ON);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed[5]: %d\n", rc);
		return rc;
	}

	return rc;
}

static int mt9e013_lgit_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
	int rc = 0;
	uint16_t chipid = 0;

	MT9E013_LGIT_LOG("start\n");
	
	rc = gpio_request(data->sensor_reset, "mt9e013_lgit");
	if(!rc)
	{
		MT9E013_LGIT_LOG("sensor_reset: %d\n", data->sensor_reset);
		
		gpio_direction_output(data->sensor_reset, 0);
		msleep(10);
		gpio_set_value_cansleep(data->sensor_reset, 1);
		msleep(10);
	}
	else
	{
		goto init_probe_done;
	}	

	rc = mt9e013_lgit_i2c_read(0x0000, &chipid, 2);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("failed: %d\n", rc);
		return rc;
	}
	
	MT9E013_LGIT_LOG("chip id: 0x%04x\n", chipid);

	// 4. Compare sensor ID to MT9E013_LGIT ID:
	if(chipid != 0x4B00)
	{
		rc = -ENODEV;
		MT9E013_LGIT_ERR("chip id doesnot match\n");
		goto init_probe_fail;
	}

	mt9e013_lgit_ctrl = kzalloc(sizeof(struct mt9e013_lgit_ctrl_t), GFP_KERNEL);
	if(!mt9e013_lgit_ctrl)
	{
		MT9E013_LGIT_ERR("invalid mt9e013_lgit_ctrl");
		rc = -ENOMEM;
	}
	
	mt9e013_lgit_ctrl->fps_divider = 1 * 0x00000400;
	mt9e013_lgit_ctrl->pict_fps_divider = 1 * 0x00000400;
	mt9e013_lgit_ctrl->set_test = TEST_OFF;
	mt9e013_lgit_ctrl->prev_res = QTR_SIZE;
	mt9e013_lgit_ctrl->pict_res = FULL_SIZE;

	if(data)
		mt9e013_lgit_ctrl->sensordata = data;

#if defined(CAM_TXT_TUNING)
	cam_txt_reg_init();
#endif

	goto init_probe_done;

init_probe_fail:
	MT9E013_LGIT_ERR("failed: %d\n",rc);
	gpio_set_value_cansleep(data->sensor_reset, 0);
	//mt9e013_lgit_probe_init_done(data);

init_probe_done:
	MT9E013_LGIT_LOG("finishes: %d\n", rc);
	
	return rc;
}

int mt9e013_lgit_sensor_open_init(const struct msm_camera_sensor_info *data)
{
	int rc = 0;

	MT9E013_LGIT_LOG("start\n");

	mt9e013_lgit_ctrl = kzalloc(sizeof(struct mt9e013_lgit_ctrl_t), GFP_KERNEL);
	if(!mt9e013_lgit_ctrl)
	{
		MT9E013_LGIT_ERR("alloc failed\n");
		rc = -ENOMEM;
		goto init_fail;
	}
	
	mt9e013_lgit_ctrl->fps_divider = 1 * 0x00000400;
	mt9e013_lgit_ctrl->pict_fps_divider = 1 * 0x00000400;
	mt9e013_lgit_ctrl->set_test = TEST_OFF;
	mt9e013_lgit_ctrl->prev_res = QTR_SIZE;
	mt9e013_lgit_ctrl->pict_res = FULL_SIZE;

	if(data)
		mt9e013_lgit_ctrl->sensordata = data;

	rc = data->pdata->camera_power_on();
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("camera_power_on failed: %d\n", rc);
		return rc;
	}

	/* enable mclk first */
	msm_camio_clk_rate_set(MT9E013_LGIT_MASTER_CLK_RATE);
	rc = mt9e013_lgit_probe_init_sensor(data);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("probe_init_sensor failed: %d\n", rc);
		goto init_fail;
	}
	
	rc = mt9e013_lgit_sensor_setting(REG_INIT, mt9e013_lgit_ctrl->prev_res);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("sensor_setting failed: %d\n", rc);
		goto init_fail;
	}
	
	mt9e013_lgit_ctrl->fps = 30*Q8;
	mt9e013_lgit_init_focus();
	
	if(rc < 0)
	{
		gpio_set_value_cansleep(data->sensor_reset, 0);
		goto init_fail;
	}
	else
	{
		goto init_done;
	}
	
init_fail:
	MT9E013_LGIT_ERR("failed: %d\n", rc);
	kfree(mt9e013_lgit_ctrl);
	mt9e013_lgit_ctrl = NULL;
	mt9e013_lgit_probe_init_done(data);
	
init_done:
	MT9E013_LGIT_LOG("finishes: %d\n", rc);
	
	return rc;
}

static int mt9e013_lgit_init_client(struct i2c_client *client)
{
	MT9E013_LGIT_LOG("start\n");
	
	init_waitqueue_head(&mt9e013_lgit_wait_queue);
	
	return 0;
}

static const struct i2c_device_id mt9e013_lgit_i2c_id[] = 
{
	{"mt9e013_lgit", 0},
	{}
};

static int mt9e013_lgit_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int rc = 0;
	
	MT9E013_LGIT_LOG("start\n");

	rc = i2c_check_functionality(client->adapter, I2C_FUNC_I2C);
	if(!rc)
	{
		MT9E013_LGIT_ERR("i2c_check_functionality failed\n");
		goto probe_failure;
	}

	mt9e013_lgit_sensorw = kzalloc(sizeof(struct mt9e013_lgit_work_t), GFP_KERNEL);
	if(!mt9e013_lgit_sensorw)
	{
		MT9E013_LGIT_ERR("kzalloc failed\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, mt9e013_lgit_sensorw);
	mt9e013_lgit_init_client(client);
	mt9e013_lgit_client = client;

	cam_debug_init();

	MT9E013_LGIT_LOG("finishes: %d\n", rc);
	
	return 0;

probe_failure:
	MT9E013_LGIT_ERR("failed: %d\n", rc);
	
	return rc;
}

static int mt9e013_lgit_send_wb_info(struct wb_info_cfg *wb)
{
	return 0;
}

static int __exit mt9e013_lgit_remove(struct i2c_client *client)
{
	struct mt9e013_lgit_work_t_t *sensorw = i2c_get_clientdata(client);

	MT9E013_LGIT_LOG("start\n");

	free_irq(client->irq, sensorw);
	mt9e013_lgit_client = NULL;
	kfree(sensorw);
	
	return 0;
}

static struct i2c_driver mt9e013_lgit_i2c_driver = 
{
	.id_table = mt9e013_lgit_i2c_id,
	.probe  = mt9e013_lgit_i2c_probe,
	.remove = __exit_p(mt9e013_lgit_i2c_remove),
	.driver = 
	{
		.name = "mt9e013_lgit",
	},
};

int mt9e013_lgit_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cdata;
	int rc = 0;
	
	if(copy_from_user(&cdata, (void *)argp, sizeof(struct sensor_cfg_data)))
	{
		MT9E013_LGIT_ERR("ioctl failed\n");
		return -EFAULT;
	}
	
	mutex_lock(&mt9e013_lgit_mut);
	
	MT9E013_LGIT_MSG("cfgtype: %d\n", cdata.cfgtype);
	
	switch(cdata.cfgtype)
	{
		case CFG_GET_PICT_FPS:
			mt9e013_lgit_get_pict_fps(cdata.cfg.gfps.prevfps, &(cdata.cfg.gfps.pictfps));
			if(copy_to_user((void *)argp, &cdata, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PREV_L_PF:
			cdata.cfg.prevl_pf = mt9e013_lgit_get_prev_lines_pf();
			if(copy_to_user((void *)argp, &cdata, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PREV_P_PL:
			cdata.cfg.prevp_pl = mt9e013_lgit_get_prev_pixels_pl();
			if(copy_to_user((void *)argp, &cdata, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PICT_L_PF:
			cdata.cfg.pictl_pf = mt9e013_lgit_get_pict_lines_pf();
			if(copy_to_user((void *)argp, &cdata, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PICT_P_PL:
			cdata.cfg.pictp_pl = mt9e013_lgit_get_pict_pixels_pl();
			if(copy_to_user((void *)argp, &cdata, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PICT_MAX_EXP_LC:
			cdata.cfg.pict_max_exp_lc = mt9e013_lgit_get_pict_max_exp_lc();
			if(copy_to_user((void *)argp, &cdata, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_SET_FPS:
		case CFG_SET_PICT_FPS:
			rc = mt9e013_lgit_set_fps(&(cdata.cfg.fps));
			break;

		case CFG_SET_EXP_GAIN:
			rc = mt9e013_lgit_write_exp_gain(cdata.cfg.exp_gain.gain, cdata.cfg.exp_gain.line);
			break;

		case CFG_SET_PICT_EXP_GAIN:
			rc = mt9e013_lgit_set_pict_exp_gain(cdata.cfg.exp_gain.gain, cdata.cfg.exp_gain.line);
			break;

		case CFG_SET_MODE:
			rc = mt9e013_lgit_set_sensor_mode(cdata.mode, cdata.rs);
			break;

		case CFG_PWR_DOWN:
			rc = mt9e013_lgit_power_down();
			break;

		case CFG_MOVE_FOCUS:
			rc = mt9e013_lgit_move_focus(cdata.cfg.focus.dir, cdata.cfg.focus.steps);
			break;

		case CFG_SET_DEFAULT_FOCUS:
			rc = mt9e013_lgit_set_default_focus(cdata.cfg.focus.steps);
			break;

		case CFG_GET_CALIB_DATA:
			rc = mt9e013_lgit_read_eeprom_data(&cdata);
			if(rc < 0)
				break;

			if(copy_to_user((void *)argp, &cdata, sizeof(cdata)))
				rc = -EFAULT;			
			break;

		case CFG_GET_AF_MAX_STEPS:
			cdata.max_steps = mt9e013_lgit_linear_total_step;
			if(copy_to_user((void *)argp, &cdata, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_SET_EFFECT:
			rc = mt9e013_lgit_set_default_focus(cdata.cfg.effect);
			break;

		case CFG_SEND_WB_INFO:
			rc = mt9e013_lgit_send_wb_info(&(cdata.cfg.wb_info));
			break;

		default:
			rc = -EFAULT;
			break;
	}

	mutex_unlock(&mt9e013_lgit_mut);

	if(rc < 0)
	{
		MT9E013_LGIT_ERR("cfgtype: %d,  failed: %d\n", cdata.cfgtype, rc);
	}

	return rc;
}

static int mt9e013_lgit_sensor_release(void)
{
	int rc = 0;
	
	mutex_lock(&mt9e013_lgit_mut);
	
	rc = mt9e013_lgit_power_down();

	gpio_set_value_cansleep(mt9e013_lgit_ctrl->sensordata->sensor_reset, 0);
	msleep(5);
	gpio_free(mt9e013_lgit_ctrl->sensordata->sensor_reset);
	
	mt9e013_lgit_ctrl->sensordata->pdata->camera_power_off();
	
	kfree(mt9e013_lgit_ctrl);
	kfree(lsc_data);
	
	mt9e013_lgit_ctrl = NULL;

	mutex_unlock(&mt9e013_lgit_mut);

	MT9E013_LGIT_LOG("finishes: %d\n", rc);

	return rc;
}

static int mt9e013_lgit_sensor_probe(const struct msm_camera_sensor_info *info, struct msm_sensor_ctrl *s)
{
	int rc = 0;
	
	rc = i2c_add_driver(&mt9e013_lgit_i2c_driver);
	if(rc < 0)
	{
		MT9E013_LGIT_ERR("i2c failed: %d\n", rc);
		goto probe_fail;
	}

	if(mt9e013_lgit_client == NULL)
	{
		MT9E013_LGIT_ERR("invalid mt9e013_lgit_client\n");
		goto probe_fail;
	}
	
	s->s_init = mt9e013_lgit_sensor_open_init;
	s->s_release = mt9e013_lgit_sensor_release;
	s->s_config  = mt9e013_lgit_sensor_config;
	s->s_mount_angle  = info->sensor_platform_info->mount_angle;
	
	gpio_set_value_cansleep(info->sensor_reset, 0);

	return rc;

probe_fail:
	MT9E013_LGIT_ERR("sensor_probe failed\n");
	
	return rc;
}

static int __mt9e013_lgit_probe(struct platform_device *pdev)
{
	MT9E013_LGIT_LOG("start\n");
	
	return msm_camera_drv_start(pdev, mt9e013_lgit_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
	.probe = __mt9e013_lgit_probe,
	.driver =
	{
		.name = "msm_camera_mt9e013_lgit",
		.owner = THIS_MODULE,
	},
};

static int __init mt9e013_lgit_init(void)
{
	MT9E013_LGIT_LOG("start\n");
	
	return platform_driver_register(&msm_camera_driver);
}

void mt9e013_lgit_exit(void)
{
	i2c_del_driver(&mt9e013_lgit_i2c_driver);
}

module_init(mt9e013_lgit_init);

MODULE_DESCRIPTION("Aptina 8 MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");

static bool streaming = 1;

static int mt9e013_lgit_set_af_codestep(void *data, u64 val)
{
	mt9e013_lgit_l_region_code_per_step = val;
	mt9e013_lgit_init_focus();
	return 0;
}

static int mt9e013_lgit_get_af_codestep(void *data, u64 *val)
{
	*val = mt9e013_lgit_l_region_code_per_step;
	return 0;
}

static int mt9e013_lgit_set_linear_total_step(void *data, u64 val)
{
	mt9e013_lgit_linear_total_step = val;
	return 0;
}

static int mt9e013_lgit_af_linearity_test(void *data, u64 *val)
{
	int i = 0;

	mt9e013_lgit_set_default_focus(0);
	msleep(3000);
	for(i = 0; i < mt9e013_lgit_linear_total_step; i++)
	{
		mt9e013_lgit_move_focus(MOVE_NEAR, 1);
		MT9E013_LGIT_LOG("MOVE_NEAR moved to index: %d\n", i);
		msleep(1000);
	}

	for (i = 0; i < mt9e013_lgit_linear_total_step; i++)
	{
		mt9e013_lgit_move_focus(MOVE_FAR, 1);
		MT9E013_LGIT_LOG("MOVE_FAR moved to index: %d\n", i);
		msleep(1000);
	}
	
	return 0;
}

static uint16_t mt9e013_lgit_step_jump = 4;
static uint8_t mt9e013_lgit_step_dir = MOVE_NEAR;
static int mt9e013_lgit_af_step_config(void *data, u64 val)
{
	mt9e013_lgit_step_jump = val & 0xFFFF;
	mt9e013_lgit_step_dir = (val >> 16) & 0x1;
	
	return 0;
}

static int mt9e013_lgit_af_step(void *data, u64 *val)
{
	int i = 0;
	int dir = MOVE_NEAR;
	
	mt9e013_lgit_set_default_focus(0);
	
	if(mt9e013_lgit_step_dir == 1)
		dir = MOVE_FAR;

	for(i = 1; i < MT9E013_LGIT_TOTAL_STEPS_NEAR_TO_FAR; i+=mt9e013_lgit_step_jump)
	{
		mt9e013_lgit_move_focus(dir, mt9e013_lgit_step_jump);
		msleep(1000);
	}
	mt9e013_lgit_set_default_focus(0);
	
	return 0;
}

static int mt9e013_lgit_af_set_slew(void *data, u64 val)
{
	mt9e013_lgit_vcm_step_time = val & 0xFFFF;
	return 0;
}

static int mt9e013_lgit_af_get_slew(void *data, u64 *val)
{
	*val = mt9e013_lgit_vcm_step_time;
	return 0;
}

static int mt9e013_lgit_set_sw_damping(void *data, u64 val)
{
	mt9e013_lgit_sw_damping_time_wait = val;
	return 0;
}

static int mt9e013_lgit_get_sw_damping(void *data, u64 *val)
{
	*val = mt9e013_lgit_sw_damping_time_wait;
	return 0;
}

static int mt9e013_lgit_focus_test(void *data, u64 *val)
{
	int i = 0;
	
	mt9e013_lgit_set_default_focus(0);

	for(i = 90; i < 256; i++)
	{
		mt9e013_lgit_i2c_write_w_sensor(REG_VCM_NEW_CODE, i);
		msleep(5000);
	}
	msleep(5000);
	for(i = 255; i > 90; i--)
	{
		mt9e013_lgit_i2c_write_w_sensor(REG_VCM_NEW_CODE, i);
		msleep(5000);
	}
	
	return 0;
}

static int mt9e013_lgit_step_test(void *data, u64 *val)
{
	int i = 0;
	mt9e013_lgit_set_default_focus(0);

	for(i = 0; i < MT9E013_LGIT_TOTAL_STEPS_NEAR_TO_FAR; i++)
	{
		mt9e013_lgit_move_focus(MOVE_NEAR, 1);
		msleep(5000);
	}

	mt9e013_lgit_move_focus(MOVE_FAR, MT9E013_LGIT_TOTAL_STEPS_NEAR_TO_FAR);
	msleep(5000);
	
	return 0;
}

static int cam_debug_stream_set(void *data, u64 val)
{
	int rc = 0;

	if(val)
	{
		mt9e013_lgit_start_stream();
		streaming = 1;
	} 
	else
	{
		mt9e013_lgit_stop_stream();
		streaming = 0;
	}

	return rc;
}

static int cam_debug_stream_get(void *data, u64 *val)
{
	*val = streaming;

	return 0;
}

static int mt9e013_lgit_log_set(void *data, u64 val)
{
	int rc = 0;

	mt9e013_lgit_log_enable = (int)val;

	return rc;
}

static int mt9e013_lgit_log_get(void *data, u64 *val)
{
	*val = mt9e013_lgit_log_enable;

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(af_damping, mt9e013_lgit_get_sw_damping, mt9e013_lgit_set_sw_damping, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(af_codeperstep, mt9e013_lgit_get_af_codestep, mt9e013_lgit_set_af_codestep, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(af_linear, mt9e013_lgit_af_linearity_test, mt9e013_lgit_set_linear_total_step, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(af_step, mt9e013_lgit_af_step, mt9e013_lgit_af_step_config, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(af_slew, mt9e013_lgit_af_get_slew, mt9e013_lgit_af_set_slew, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(cam_focus, mt9e013_lgit_focus_test, NULL, "%lld\n");
DEFINE_SIMPLE_ATTRIBUTE(cam_step, mt9e013_lgit_step_test, NULL, "%lld\n");
DEFINE_SIMPLE_ATTRIBUTE(cam_stream, cam_debug_stream_get, cam_debug_stream_set, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(log_enable, mt9e013_lgit_log_get, mt9e013_lgit_log_set, "%llu\n");

static int cam_debug_init(void)
{
	struct dentry *cam_dir;
	
	cam_dir = debugfs_create_dir("mt9e013_lgit", NULL);
	if(!cam_dir)
		return -ENOMEM;

	if(!debugfs_create_file("af_codeperstep", S_IRUGO | S_IWUSR, cam_dir, NULL, &af_codeperstep))
		return -ENOMEM;
	
	if(!debugfs_create_file("af_linear", S_IRUGO | S_IWUSR, cam_dir, NULL, &af_linear))
		return -ENOMEM;
	
	if(!debugfs_create_file("af_step", S_IRUGO | S_IWUSR, cam_dir, NULL, &af_step))
		return -ENOMEM;
	
	if(!debugfs_create_file("af_slew", S_IRUGO | S_IWUSR, cam_dir, NULL, &af_slew))
		return -ENOMEM;
	
	if(!debugfs_create_file("af_damping", S_IRUGO | S_IWUSR, cam_dir, NULL, &af_damping))
		return -ENOMEM;
	
	if(!debugfs_create_file("stream", S_IRUGO | S_IWUSR, cam_dir, NULL, &cam_stream))
		return -ENOMEM;

	if(!debugfs_create_file("log", S_IRUGO | S_IWUSR, cam_dir, NULL, &log_enable))
		return -ENOMEM;

	return 0;
}

#if defined(CAM_TXT_TUNING)
static unsigned short cam_txt_convert(char ch)
{
	unsigned short value;
	
	value = (unsigned short)ch;
	if(48<= value && value<=57) /* 0-9 */
		return (value-48);
	if(65<= value && value<=90) /* A-Z */       
		return (value-55);
	if(97<= value && value<=122) /* a- z */     
		return (value-87);  

	return 0;
}

static unsigned int cam_txt_to_hex(unsigned char N, char *cmd)
{
	unsigned char i = 0;
	unsigned int digit=0; 
	unsigned int value=0;

	for(i=0; i<N; i++)
	{       
		digit = cam_txt_convert(*(cmd+i));
		value  += (digit&0xFF)<<((N-i-1)*4);
	}

	return value;
}

static unsigned int cam_txt_to_dec(unsigned char N, char *cmd)
{
	char i = 0;
	char j = 0;
	unsigned int digit = 0;
	unsigned int value = 0;
	unsigned int power = 0;

	for(i=0; i<N; i++)
	{
		digit = cam_txt_convert(*(cmd+i));
		power = 1;
		for(j=N-i-1; j>0; j--)
			power = power*10;
		
		value  += digit*power;
	}
	
	return value;
}

static int cam_txt_check_type(unsigned char *type)
{
	int ret_value = -1;

	if(strncmp(type, "CMD", 3) == 0)
	{
		if(strncmp(type, "CMD_WRITE", 9) == 0) ret_value = CMD_WRITE;
		else if(strncmp(type, "CMD_POLL", 8) == 0) ret_value = CMD_POLL;
		else if(strncmp(type, "CMD_DELAY", 9) == 0) ret_value = CMD_DELAY;
		else ret_value = -1;
	}
	else
		ret_value = -1;

	return ret_value;
}

static int cam_txt_check_token(unsigned char *type, char *token)
{
	int ret_value = TOKEN_NONE;
	char start_token[TOKEN_BUF_MAX_LENGTH];
	char end_token[TOKEN_BUF_MAX_LENGTH];

	if(strncmp(type, CAM_TXT_TOK, strlen(CAM_TXT_TOK)) == 0)
	{
		memset(start_token, 0x00, TOKEN_BUF_MAX_LENGTH);
		memset(end_token, 0x00, TOKEN_BUF_MAX_LENGTH);
		memcpy(start_token, token, strlen(token));
		strcat(start_token, START_TAG);
		memcpy(end_token, token, strlen(token));
		strcat(end_token, END_TAG);
		
		if(strncmp(type, start_token, strlen(start_token)) == 0) ret_value = TOKEN_START;
		else if(strncmp(type, end_token, strlen(end_token)) == 0) ret_value = TOKEN_END;
		else ret_value = TOKEN_NONE;
	}
	else
		ret_value = TOKEN_NONE;

	return ret_value;
}

static int cam_txt_is_digit(unsigned char data)
{
	int ret_value = 0;
	static int prev_value = -1;
	
	if(data >= '0' && data <= '9') ret_value = DEC_VAL;
	else if(data >= 'a' && data <= 'f') ret_value = HEX_VAL;
	else if(data >= 'A' && data <= 'F') ret_value = HEX_VAL;
	else if(data == 'x' || data == 'X') ret_value = HEX_VAL;
	else ret_value = 0;

	if(prev_value == 0 && ret_value == HEX_VAL) ret_value = 0;
	else prev_value = ret_value;

	return ret_value;
}

static int cam_txt_get_data1(unsigned char *buf, int pos, int *reg, int *data, int *len)
{
	int ret_value = 0;
	int check_digit = 0;
	int dec_cnt = 0;
	int hex_cnt =0;
	int i =0;
	unsigned char temp[6];

	while(!cam_txt_is_digit(buf[pos++]));
	pos--;

	memset(temp, 0x00, 6);
	check_digit = 0;
	dec_cnt = 0;
	hex_cnt = 0;
	i = 0;
	while((check_digit = cam_txt_is_digit(buf[pos])) && (i <6))
	{
		if(check_digit == DEC_VAL) dec_cnt++;
		else if(check_digit == HEX_VAL) hex_cnt++;

		temp[i++] = buf[pos++];
	}

	if(hex_cnt > 0 && dec_cnt > 0)
		*reg = cam_txt_to_hex((i-2), &(temp[2]));
	else 
	{
		MT9E013_LGIT_ERR("hex_cnt: %d, dec_cnt: %d\n", hex_cnt, dec_cnt);
		return -1;
	}

	while(!cam_txt_is_digit(buf[pos++]));
	pos--;

	memset(temp, 0x00, 6);
	check_digit = 0;
	dec_cnt = 0;
	hex_cnt = 0;
	i = 0;
	while((check_digit = cam_txt_is_digit(buf[pos])) && (i <6))
	{
		if(check_digit == DEC_VAL) dec_cnt++;
		else if(check_digit == HEX_VAL) hex_cnt++;

		temp[i++] = buf[pos++];
	}
	
	if(hex_cnt > 0 && dec_cnt > 0)
		*data = cam_txt_to_hex((i-2), &(temp[2]));
	else 
	{
		MT9E013_LGIT_ERR("hex_cnt: %d, dec_cnt: %d\n", hex_cnt, dec_cnt);
		return -2;
	};

	*len = (i-2)/2;

	return ret_value;
}

static int cam_txt_get_data2(unsigned char *buf, int pos, int *reg, int *data, int *count)
{
	int ret_value = 0;
	int check_digit = 0;
	int dec_cnt = 0;
	int hex_cnt =0;
	int i =0;
	unsigned char temp[6];

	while(!cam_txt_is_digit(buf[pos++]));
	pos--;

	memset(temp, 0x00, 6);
	check_digit = 0;
	dec_cnt = 0;
	hex_cnt = 0;
	i = 0;
	while((check_digit = cam_txt_is_digit(buf[pos])) && (i <6))
	{
		if(check_digit == DEC_VAL) dec_cnt++;
		else if(check_digit == HEX_VAL) hex_cnt++;

		temp[i++] = buf[pos++];
	}

	if(hex_cnt > 0 && dec_cnt > 0)
		*reg = cam_txt_to_hex((i-2), &(temp[2]));
	else 
	{
		MT9E013_LGIT_ERR("hex_cnt: %d, dec_cnt: %d\n", hex_cnt, dec_cnt);
		return -1;
	}

	while(!cam_txt_is_digit(buf[pos++]));
	pos--;

	memset(temp, 0x00, 6);
	check_digit = 0;
	dec_cnt = 0;
	hex_cnt = 0;
	i = 0;
	while((check_digit = cam_txt_is_digit(buf[pos])) && (i <6))
	{
		if(check_digit == DEC_VAL) dec_cnt++;
		else if(check_digit == HEX_VAL) hex_cnt++;

		temp[i++] = buf[pos++];
	}
	
	if(hex_cnt > 0 && dec_cnt > 0)
		*data = cam_txt_to_hex((i-2), &(temp[2]));
	else if(dec_cnt > 0 && hex_cnt == 0)
		*data = cam_txt_to_dec(i, temp);
	else 
	{
		MT9E013_LGIT_ERR("hex_cnt: %d, dec_cnt: %d\n", hex_cnt, dec_cnt);
		return -2;
	}

	while(!cam_txt_is_digit(buf[pos++]));
	pos--;

	memset(temp, 0x00, 6);
	check_digit = 0;
	dec_cnt = 0;
	hex_cnt = 0;
	i = 0;
	while((check_digit = cam_txt_is_digit(buf[pos])) && (i <6))
	{
		if(check_digit == DEC_VAL) dec_cnt++;
		else if(check_digit == HEX_VAL) hex_cnt++;

		temp[i++] = buf[pos++];
	}
	
	if(hex_cnt > 0 && dec_cnt > 0)
		*count = cam_txt_to_hex((i-2), &(temp[2]));
	else if(dec_cnt > 0 && hex_cnt == 0)
		*count = cam_txt_to_dec(i, temp);
	else 
	{
		MT9E013_LGIT_ERR("hex_cnt: %d, dec_cnt: %d\n", hex_cnt, dec_cnt);
		return -3;
	}

	return ret_value;
}

static void cam_txt_shift(unsigned char* data)
{
	int i;
	
	for(i=0; i<CMD_BUF_MAX_LENGTH-1; i++)
		*(data+i) = *(data+i+1);
}

static void cam_txt_shift_token(unsigned char* data)
{
	int i;
	
	for(i=0; i<TOKEN_BUF_MAX_LENGTH-1; i++)
		*(data+i) = *(data+i+1);
}

int cam_txt_from_file(char *token, struct mt9e013_lgit_i2c_reg_conf **data, unsigned short *size)
{
	struct file *flip;
	mm_segment_t old_fs;
	
	int read_size;
	unsigned char *buffer = NULL;
	unsigned char *cmd_buffer = NULL;
  
	int i = 0;
	int ret_val = 0;
	int cmd_type = -1;
	int cmd_reg = 0;
	int cmd_data = 0;
	int cmd_len = 0;

	unsigned char cmd_buffer_str[CMD_BUF_MAX_LENGTH];
	unsigned char token_buffer_str[TOKEN_BUF_MAX_LENGTH];
	unsigned char line_skip_flag = 0;
	unsigned char area_skip_flag = 0;
	int token_find_flag = TOKEN_NONE;
	int data_start = 0;
	int data_end = 0;
	int data_count = 0;

	int cam_txt_file_size = 0;
	int cam_txt_count = 0;
	struct mt9e013_lgit_i2c_reg_conf *cam_txt_file_data = NULL;

	static int cam_txt_in_use = 0;

	while((cam_txt_in_use != 0) && (i<10))
	{
		msleep(100);
		i++;
	}
	i=0;

	cam_txt_in_use = 1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	
	flip = filp_open(CAM_TXT_FILE, O_RDONLY |O_LARGEFILE, S_IRUSR);
	if(IS_ERR(flip))
	{
		MT9E013_LGIT_ERR("%s open failed: %d\n", CAM_TXT_FILE, (int)IS_ERR(flip));
		set_fs(old_fs);
		cam_txt_in_use = 0;
		return -EIO;
	}

	cam_txt_file_size = flip->f_op->llseek(flip, (loff_t)0, SEEK_END);
	MT9E013_LGIT_LOG("%s: cam_txt_file_size: %d, token: %s\n", CAM_TXT_FILE, cam_txt_file_size, token);

	buffer = (unsigned char *)kmalloc(cam_txt_file_size, GFP_KERNEL);
	if(!buffer)
	{
		MT9E013_LGIT_ERR("alloc failed: %d\n", cam_txt_file_size);
		filp_close(flip, NULL);
		set_fs(old_fs);
		cam_txt_file_size = 0;
		cam_txt_in_use = 0;
		return -ENOMEM;
	}

	flip->f_pos = 0;
	read_size = flip->f_op->read(flip, buffer, cam_txt_file_size, &flip->f_pos);
	MT9E013_LGIT_LOG("read size: %d\n", read_size);

	filp_close(flip, NULL);
	set_fs(old_fs);

	if((strlen(token)+strlen(START_TAG) > TOKEN_BUF_MAX_LENGTH-1)
		|| (strlen(token)+strlen(END_TAG) > TOKEN_BUF_MAX_LENGTH-1))
	{
		MT9E013_LGIT_ERR("token length: %s invalid\n", token);
		return -EFAULT;
	}
		
	i = 0;
	line_skip_flag = 0;
	area_skip_flag = 0;
	memset(cmd_buffer_str, 0x00, CMD_BUF_MAX_LENGTH);
	memset(token_buffer_str, 0x00, TOKEN_BUF_MAX_LENGTH);

	while(i < cam_txt_file_size)
	{
		cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] = *(buffer+i);
		token_buffer_str[TOKEN_BUF_MAX_LENGTH-1] = *(buffer+i);
		token_find_flag = cam_txt_check_token(token_buffer_str, token);
		if(token_find_flag == TOKEN_START)
		{
			data_start = i;
			MT9E013_LGIT_LOG("%s: start token found, data_start: %d\n", token, data_start);
			cam_txt_shift(cmd_buffer_str);
			cam_txt_shift_token(token_buffer_str);
			i++;
			break;
		}
		cam_txt_shift(cmd_buffer_str);
		cam_txt_shift_token(token_buffer_str);
		i++;
	}

	if(token_find_flag == TOKEN_START)
	{
		while(i < cam_txt_file_size)
		{
			cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] = *(buffer+i);
			token_buffer_str[TOKEN_BUF_MAX_LENGTH-1] = *(buffer+i);
			token_find_flag = cam_txt_check_token(token_buffer_str, token);

			if(token_find_flag != TOKEN_END)
			{
				if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '/') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '/'))
					line_skip_flag = 1;

				if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '\r') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '\n'))
					line_skip_flag = 0;

				if(cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '\n')
					line_skip_flag = 0;

				if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '/') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '*'))
					area_skip_flag = 1;

				if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '*') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '/'))
					area_skip_flag = 0;
					
				if((line_skip_flag == 0) && (area_skip_flag == 0) && (cam_txt_check_type(cmd_buffer_str) > 0))
				{
					data_count++;
					memset(cmd_buffer_str, 0x00, CMD_BUF_MAX_LENGTH);
				}
			}
			else
			{
				data_end = i;
				MT9E013_LGIT_LOG("%s: end token found, data_count: %d, data_end: %d\n", token, data_count, data_end);
				break;
			}
			cam_txt_shift(cmd_buffer_str);
			cam_txt_shift_token(token_buffer_str);
			i++;
		}	
	}
	else
	{
		MT9E013_LGIT_ERR("%s: token found fail\n", token);
		return -EFAULT;
	}

	cam_txt_count = data_count;

	cmd_buffer = (unsigned char *)kmalloc(cam_txt_count*sizeof(struct mt9e013_lgit_i2c_reg_conf), GFP_KERNEL);
	if(!cmd_buffer)
	{
		MT9E013_LGIT_ERR("alloc failed: %d\n", (cam_txt_count*sizeof(struct mt9e013_lgit_i2c_reg_conf)));
		kfree(buffer);
		cam_txt_count = 0;
		cam_txt_file_size = 0;
		cam_txt_in_use = 0;
		token_find_flag = TOKEN_NONE;
		data_start = 0;
		data_end = 0;
		return -ENOMEM;
	}
		
	cam_txt_file_data = (struct mt9e013_lgit_i2c_reg_conf*)cmd_buffer;

	i = data_start;
	line_skip_flag = 0;
	area_skip_flag = 0;
	data_count = 0;
	memset(cmd_buffer_str, 0x00, CMD_BUF_MAX_LENGTH);
		
	while((i < data_end) && (data_count < cam_txt_count))
	{
		cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] = *(buffer+i);
		i++;

		if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '/') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '/'))
			line_skip_flag = 1;

		if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '\r') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '\n'))
			line_skip_flag = 0;

		if(cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '\n')
			line_skip_flag = 0;

		if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '/') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '*'))
			area_skip_flag = 1;

		if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '*') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '/'))
			area_skip_flag = 0;

		cmd_type = cam_txt_check_type(cmd_buffer_str);

		if((line_skip_flag == 0) && (area_skip_flag == 0) && (cmd_type == CMD_WRITE))
		{
			ret_val = cam_txt_get_data1(buffer, i, &cmd_reg, &cmd_data, &cmd_len);

			if(ret_val < 0)
			{
				MT9E013_LGIT_ERR("invalid data format[1]: %d\n", ret_val);
				kfree(buffer);
				kfree(cmd_buffer);
				cam_txt_file_data = NULL;
				cam_txt_count = 0;
				cam_txt_file_size = 0;
				cam_txt_in_use = 0;
				token_find_flag = TOKEN_NONE;
				data_start = 0;
				data_end = 0;
				*data = NULL;
				*size = 0;
				return -EFAULT;
			}

			MT9E013_LGIT_REG("(%d) type: %d, reg: 0x%04x, data: 0x%04x, count: %d\n", data_count, cmd_type, cmd_reg, cmd_data, cmd_len);

			cam_txt_file_data[data_count].register_type = cmd_type;
			cam_txt_file_data[data_count].register_address = (uint16_t)cmd_reg;
			cam_txt_file_data[data_count].register_data = (uint32_t)cmd_data;
				
			data_count++;
			memset(cmd_buffer_str, 0x00, CMD_BUF_MAX_LENGTH);
		}
		else if((line_skip_flag == 0) && (area_skip_flag == 0) && ((cmd_type == CMD_POLL) || (cmd_type == CMD_DELAY)))
		{
			ret_val = cam_txt_get_data2(buffer, i, &cmd_reg, &cmd_data, &cmd_len);

			if(ret_val < 0)
			{
				MT9E013_LGIT_ERR("invalid data format[2]: %d, %d\n", ret_val, cmd_type);
				kfree(buffer);
				kfree(cmd_buffer);
				cam_txt_file_data = NULL;
				cam_txt_count = 0;
				cam_txt_file_size = 0;
				cam_txt_in_use = 0;
				token_find_flag = TOKEN_NONE;
				data_start = 0;
				data_end = 0;
				*data = NULL;
				*size = 0;
				return -EFAULT;
			}

			MT9E013_LGIT_REG("(%d) type: %d, reg: 0x%04x, data: 0x%04x, count: %d\n", data_count, cmd_type, cmd_reg, cmd_data, cmd_len);

			cam_txt_file_data[data_count].register_type = cmd_type;
			cam_txt_file_data[data_count].register_address = (uint16_t)cmd_reg;
			cam_txt_file_data[data_count].register_data = (uint32_t)cmd_data;
				
			data_count++;
			memset(cmd_buffer_str, 0x00, CMD_BUF_MAX_LENGTH);
		}
		cam_txt_shift(cmd_buffer_str);
	}

	kfree(buffer);

#if defined(CAM_TXT_DIRECT_WRITE)
	ret_val = mt9e013_lgit_i2c_write_w_table(cam_txt_file_data, cam_txt_count);
	if(ret_val < 0)
	{
		MT9E013_LGIT_ERR("cam_txt i2c write failed: %d\n", ret_val);
		kfree(cmd_buffer);
		cam_txt_file_data = NULL;
		cam_txt_count = 0;
		cam_txt_file_size = 0;
		cam_txt_in_use = 0;
		token_find_flag = TOKEN_NONE;
		data_start = 0;
		data_end = 0;
		*data = NULL;
		*size = 0;
		return -EFAULT;
	}

	if(cam_txt_file_data != NULL)
	{
		kfree((unsigned char *)cam_txt_file_data);
		cam_txt_file_data = NULL;
		cam_txt_count = 0;
		cam_txt_file_size = 0;
		token_find_flag = TOKEN_NONE;
		data_start = 0;
		data_end = 0;
		*data = NULL;
		*size = 0;
	}
#else
	MT9E013_LGIT_LOG("update %s reg table: data: %p, count: %d\n", token, cam_txt_file_data, cam_txt_count);

	*data = cam_txt_file_data;
	*size = cam_txt_count;
	cam_txt_count = 0;
	cam_txt_file_size = 0;
	token_find_flag = TOKEN_NONE;
	data_start = 0;
	data_end = 0;
#endif	

	cam_txt_in_use = 0;
	
	return ret_val;
}

#if !defined(CAM_TXT_DIRECT_WRITE)
struct mt9e013_lgit_i2c_reg_conf *g_reg_mipi = NULL;
unsigned short g_reg_mipi_size = 0;
struct mt9e013_lgit_i2c_reg_conf *g_rec_settings = NULL;
unsigned short g_rec_size = 0;
struct mt9e013_lgit_i2c_reg_conf *g_reg_pll = NULL;
unsigned short g_reg_pll_size = 0;
struct mt9e013_lgit_i2c_reg_conf *g_reg_prev = NULL;
unsigned short g_reg_prev_size = 0;
struct mt9e013_lgit_i2c_reg_conf *g_reg_snap = NULL;
unsigned short g_reg_snap_size = 0;
struct mt9e013_lgit_i2c_reg_conf *g_reg_FHD = NULL;
unsigned short g_reg_FHD_size = 0;
void cam_txt_reg_init(void)
{
	int rc = 0;
	
	if(g_reg_mipi != NULL)
	{
		MT9E013_LGIT_LOG("free memory: %p\n", g_reg_mipi);
		kfree(g_reg_mipi);
		g_reg_mipi = NULL;
		g_reg_mipi_size = 0;
	}
	rc = cam_txt_from_file(MIPI_TOKEN, &g_reg_mipi, &g_reg_mipi_size);
	if(rc < 0 || g_reg_mipi == NULL || g_reg_mipi_size == 0)
	{
		MT9E013_LGIT_ERR("failed: %d, %p, %d\n", rc, g_reg_mipi, g_reg_mipi_size);
	}
	else
	{
		mt9e013_lgit_regs.reg_mipi = g_reg_mipi;
		mt9e013_lgit_regs.reg_mipi_size = g_reg_mipi_size;
		MT9E013_LGIT_LOG("g_reg_mipi: %p, g_reg_mipi_size: %d\n", g_reg_mipi, g_reg_mipi_size);
	}

	if(g_rec_settings != NULL)
	{
		MT9E013_LGIT_LOG("free memory: %p\n", g_rec_settings);
		kfree(g_rec_settings);
		g_rec_settings = NULL;
		g_rec_size = 0;
	}
	rc = cam_txt_from_file(RECOMMEND_TOKEN, &g_rec_settings, &g_rec_size);
	if(rc < 0 || g_rec_settings == NULL || g_rec_size == 0)
	{
		MT9E013_LGIT_ERR("failed: %d, %p, %d\n", rc, g_rec_settings, g_rec_size);
	}
	else
	{
		mt9e013_lgit_regs.rec_settings = g_rec_settings;
		mt9e013_lgit_regs.rec_size = g_rec_size;
		MT9E013_LGIT_LOG("g_rec_settings: %p, g_rec_size: %d\n", g_rec_settings, g_rec_size);
	}

	if(g_reg_pll != NULL)
	{
		MT9E013_LGIT_LOG("free memory: %p\n", g_reg_pll);
		kfree(g_reg_pll);
		g_reg_pll = NULL;
		g_reg_pll_size = 0;
	}
	rc = cam_txt_from_file(PLL_TOKEN, &g_reg_pll, &g_reg_pll_size);
	if(rc < 0 || g_reg_pll == NULL || g_reg_pll_size == 0)
	{
		MT9E013_LGIT_ERR("failed: %d, %p, %d\n", rc, g_reg_pll, g_reg_pll_size);
	}
	else
	{
		mt9e013_lgit_regs.reg_pll = g_reg_pll;
		mt9e013_lgit_regs.reg_pll_size = g_reg_pll_size;
		MT9E013_LGIT_LOG("g_reg_pll: %p, g_reg_pll_size: %d\n", g_reg_pll, g_reg_pll_size);
	}

	if(g_reg_prev != NULL)
	{
		MT9E013_LGIT_LOG("free memory: %p\n", g_reg_prev);
		kfree(g_reg_prev);
		g_reg_prev = NULL;
		g_reg_prev_size = 0;
	}
	rc = cam_txt_from_file(PREVIEW_TOKEN, &g_reg_prev, &g_reg_prev_size);
	if(rc < 0 || g_reg_prev == NULL || g_reg_prev_size == 0)
	{
		MT9E013_LGIT_ERR("failed: %d, %p, %d\n", rc, g_reg_prev, g_reg_prev_size);
	}
	else
	{
		mt9e013_lgit_regs.reg_prev = g_reg_prev;
		mt9e013_lgit_regs.reg_prev_size = g_reg_prev_size;
		MT9E013_LGIT_LOG("g_reg_prev: %p, g_reg_prev_size: %d\n", g_reg_prev, g_reg_prev_size);
	}

	if(g_reg_snap != NULL)
	{
		MT9E013_LGIT_LOG("free memory: %p\n", g_reg_snap);
		kfree(g_reg_snap);
		g_reg_snap = NULL;
		g_reg_snap_size = 0;
	}
	rc = cam_txt_from_file(SNAPSHOT_TOKEN, &g_reg_snap, &g_reg_snap_size);
	if(rc < 0 || g_reg_snap == NULL || g_reg_snap_size == 0)
	{
		MT9E013_LGIT_ERR("failed: %d, %p, %d\n", rc, g_reg_snap, g_reg_snap_size);
	}
	else
	{
		mt9e013_lgit_regs.reg_snap = g_reg_snap;
		mt9e013_lgit_regs.reg_snap_size = g_reg_snap_size;
		MT9E013_LGIT_LOG("g_reg_snap: %p, g_reg_snap_size: %d\n", g_reg_snap, g_reg_snap_size);
	}

	if(g_reg_FHD != NULL)
	{
		MT9E013_LGIT_LOG("free memory: %p\n", g_reg_FHD);
		kfree(g_reg_FHD);
		g_reg_FHD = NULL;
		g_reg_FHD_size = 0;
	}
	rc = cam_txt_from_file(FHD_TOKEN, &g_reg_FHD, &g_reg_FHD_size);
	if(rc < 0 || g_reg_FHD == NULL || g_reg_FHD_size == 0)
	{
		MT9E013_LGIT_ERR("failed: %d, %p, %d\n", rc, g_reg_FHD, g_reg_FHD_size);
	}
	else
	{
		mt9e013_lgit_regs.reg_FHD = g_reg_FHD;
		mt9e013_lgit_regs.reg_FHD_size = g_reg_FHD_size;
		MT9E013_LGIT_LOG("g_reg_FHD: %p, g_reg_FHD_size: %d\n", g_reg_FHD, g_reg_FHD_size);
	}
}
#endif
#endif

