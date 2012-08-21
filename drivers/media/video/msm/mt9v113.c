/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
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

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include "mt9v113.h"
#ifdef CONFIG_VTCAM_SD_IMAGE_TUNNING_FOR_MT9V113
#include "register_common_init.h"
#endif

/* Micron MT9V113 Registers and their values */

#define MT9V113_MODEL_ID 0x2280
#define REG_MT9V113_MODEL_ID 0x0000

#define REG_STANDBY_CONTROL 0x0018
#define ENABLE_STANDBY_MODE 0x4029
#define DISABLE_STANDBY_MODE 0x4028

#define SENSOR_DEBUG 0
#if SENSOR_DEBUG
#undef CDBG
#define CDBG(fmt, args...) printk(KERN_INFO "mt9v113: " fmt, ##args)
#endif 

struct mt9v113_work {
	struct work_struct work;
};

static struct  mt9v113_work *mt9v113_sensorw;
static struct  i2c_client *mt9v113_client;
static int32_t config_csi;

static uint8_t mt9v113_delay_msecs_stdby = 2;
static uint16_t mt9v113_delay_msecs_stream = 5;

struct mt9v113_ctrl_t {
	const struct msm_camera_sensor_info *sensordata;
};

static struct mt9v113_ctrl_t *mt9v113_ctrl;

static DECLARE_WAIT_QUEUE_HEAD(mt9v113_wait_queue);
DEFINE_MUTEX(mt9v113_sem);

static int prev_effect_mode;
static int prev_balance_mode;
static int prev_iso_mode;
static int prev_sensor_mode;
static int prev_brightness_mode;
#if 0 // donghyun.kwon(20111019) : build_error_fixed
//LGE_CAMERA_S : FPS - jonghwan.ko@lge.com
static int prev_fps_mode;
//LGE_CAMERA_E : FPS - jonghwan.ko@lge.com
#endif

/*=============================================================
	EXTERNAL DECLARATIONS
==============================================================*/
extern struct mt9v113_reg mt9v113_regs;


/*=============================================================*/

static int mt9v113_probe_init_done(const struct msm_camera_sensor_info *data)
{
	CDBG("%s : mt9v113 sensor_reset 0\n", __func__);
	gpio_set_value_cansleep(data->sensor_reset, 0);
	gpio_free(data->sensor_reset);

	CDBG("%s : mt9v113 MCLK OFF\n", __func__);
#if 0 // donghyun.kwon(20111019) : build_error_fixed
	gpio_set_value_cansleep(data->vt_mclk_enable, 0);
	gpio_free(data->vt_mclk_enable);
#else
	gpio_set_value_cansleep(15, 0);  // MCLK disable
	gpio_free(15);                   // MCLK free?
#endif
	mt9v113_ctrl->sensordata->pdata->camera_power_off();	

	return 0;
}


static int32_t mt9v113_i2c_txdata(unsigned short saddr,
	unsigned char *txdata, int length)
{
	struct i2c_msg msg[] = {
		{
			.addr = saddr ,
			.flags = 0,
			.len = length,
			.buf = txdata,
		},
	};

	if (i2c_transfer(mt9v113_client->adapter, msg, 1) < 0) {
		CDBG("mt9v113_i2c_txdata failed\n");
		return -EIO;
	}

	return 0;
}


static int32_t mt9v113_i2c_write_w_sensor(unsigned short waddr, unsigned short wdata)
{
	int32_t rc = -EIO;
	unsigned char buf[4];
	memset(buf, 0, sizeof(buf));

	buf[0] = (waddr & 0xFF00) >> 8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = (wdata & 0xFF00) >> 8;
	buf[3] = (wdata & 0x00FF);
	rc = mt9v113_i2c_txdata(mt9v113_client->addr, buf, 4);
       CDBG("[### WORD_LEN check] i2c_write , addr = 0x%04x, val = 0x%04x!\n", waddr, wdata);            

	if (rc < 0)
		CDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n", waddr, wdata);

	return rc;
}

static int32_t mt9v113_i2c_write_table(struct mt9v113_i2c_reg_conf const *reg_conf_tbl, int num_of_items_in_table)
{
	int i;
	int32_t rc = -EIO;

	for (i = 0; i < num_of_items_in_table; i++) {
		if(reg_conf_tbl->waddr == 0xFFFF)
		{
			msleep(reg_conf_tbl->wdata);
			rc = 0;
		}
		else
		{
			rc = mt9v113_i2c_write_w_sensor(reg_conf_tbl->waddr, reg_conf_tbl->wdata);
			if(rc < 0)
				return rc;
//			rc = mt9v113_i2c_write(mt9v113_client->addr, reg_conf_tbl->waddr, reg_conf_tbl->wdata, reg_conf_tbl->width);
		}

		if (rc < 0)
			break;

		reg_conf_tbl++;
	}

	return rc;
}

static int mt9v113_i2c_rxdata(unsigned short saddr,
	unsigned char *rxdata, int length)
{
	struct i2c_msg msgs[] = {
		{
			.addr   = saddr ,
			.flags = 0,
			.len   = 2,
			.buf   = rxdata,
		},
		{
			.addr   = saddr ,
			.flags = I2C_M_RD,
			.len   = length,
			.buf   = rxdata,
		},
	};

	if (i2c_transfer(mt9v113_client->adapter, msgs, 2) < 0) {
		CDBG("mt9v113_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
}

static int32_t mt9v113_i2c_read(unsigned short   saddr,
	unsigned short raddr, unsigned short *rdata, enum mt9v113_width width)
{
	int32_t rc = 0;
	unsigned char buf[4];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	switch (width) 
	{
		case BYTE_LEN:
			buf[0] = (raddr & 0xFF00) >> 8;
			buf[1] = (raddr & 0x00FF);
			rc = mt9v113_i2c_rxdata(saddr, buf, 2);
			if (rc < 0)
				return rc;
			*rdata = buf[0];
			break;

		case WORD_LEN: 
			buf[0] = (raddr & 0xFF00)>>8;
			buf[1] = (raddr & 0x00FF);

			rc = mt9v113_i2c_rxdata(saddr, buf, 2);
			if (rc < 0)
				return rc;

			*rdata = buf[0] << 8 | buf[1];
			break;

		default:
			break;
	}

	if (rc < 0)
		CDBG("mt9v113_i2c_read failed!\n");

	return rc;
}

static int mt9v113_polling_reg(unsigned short waddr, unsigned short wcondition, unsigned short result, int delay, int time_out)
{
	int rc = -EFAULT;
	int i=0; 
	unsigned short wdata;

	for(i=0; i<time_out; i++){
		rc = mt9v113_i2c_read(mt9v113_client->addr, waddr, &wdata, WORD_LEN);

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
	
	return rc;
}

int mt9v113_check_sensor_mode(void)
{
	unsigned short mcu_address_sts =0, mcu_data =0;
	int i, rc=-EFAULT;

	CDBG("mt9v113 : check_sensor_mode E\n");
	
	for(i=0; i<50; i++){

		/* MCU_ADDRESS : check mcu_address */
		rc = mt9v113_i2c_read(mt9v113_client->addr,
				0x098C, &mcu_address_sts, WORD_LEN);
		if (rc < 0){
			CDBG("mt9v113: reading mcu_address_sts fail\n");
			return rc;
		}

		/* MCU_DATA_0 : check mcu_data */
		rc = mt9v113_i2c_read(mt9v113_client->addr,
				0x0990, &mcu_data, WORD_LEN);
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

static long mt9v113_reg_init(void)
{
	int32_t rc = 0;

#ifdef CONFIG_VTCAM_SD_IMAGE_TUNNING_FOR_MT9V113
		common_reg_list_type* pstRegisterList = NULL, *ptr_list;
		int loop;
	
		CDBG(KERN_ERR "### %s: Register init\n", __func__);
		common_register_init(COMMON_REG_MEM, &pstRegisterList);
		if (!pstRegisterList)
		{
			CDBG("[miracle.kim] param file open failed");
			rc = mt9v113_i2c_write_table(mt9v113_regs.init_tbl, mt9v113_regs.inittbl_size);
		}
		else
		{
			ptr_list = pstRegisterList;
	
			for (loop = 0; loop < ptr_list->num_regs; loop++)
			{
				if (ptr_list->list_regs[loop].mem.addr == 0xFFFF)
				{
					msleep(ptr_list->list_regs[loop].mem.val);
					CDBG("[icebox]%s:msleep addr = %d, val = %d\n", __func__, ptr_list->list_regs[loop].mem.addr, ptr_list->list_regs[loop].mem.val);
					rc = 0;
				}
				else
				//isx006_i2c_write(isx006_client->addr, ptr_list->list_regs[loop].mem_var4.addr, ptr_list->list_regs[loop].mem_var4.vals.val32, ptr_list->list_regs[loop].mem_var4.len);
				rc = mt9v113_i2c_write_w_sensor(ptr_list->list_regs[loop].mem.addr, ptr_list->list_regs[loop].mem.val);
				CDBG("[icebox]%s: addr = %d, val = %d\n", __func__, ptr_list->list_regs[loop].mem.addr, ptr_list->list_regs[loop].mem.val);
	
				if (rc < 0)
				break;
			}
		}
	
		if (pstRegisterList)
			kfree(pstRegisterList);
#else
	rc = mt9v113_i2c_write_table(mt9v113_regs.init_tbl, mt9v113_regs.inittbl_size);
	if (rc < 0)
		return rc;


	rc = mt9v113_polling_reg(0x0018, 0x4000, 0x0000, 10, 100);
	if (rc<0)
		return rc;

	rc = mt9v113_polling_reg(0x301A, 0x0004, 0x0004, 50, 20);
	if (rc<0)
		return rc;

	rc = mt9v113_i2c_write_table(mt9v113_regs.init_tbl1, mt9v113_regs.inittbl1_size);
	if (rc < 0)
		return rc;

	rc = mt9v113_polling_reg(0x0018, 0x4000, 0x0000, 10, 100);
	if (rc<0)
		return rc;

	rc = mt9v113_polling_reg(0x301A, 0x0004, 0x0004, 50, 20);
	if (rc<0)
		return rc;

	rc = mt9v113_i2c_write_table(mt9v113_regs.init_tbl2, mt9v113_regs.inittbl2_size);
	if (rc < 0)
		return rc;

#endif
    rc = mt9v113_check_sensor_mode();

    if (rc<0)
    {
        CDBG("###[ERROR]%s: failed to check sensor mode\n", __func__);
        return rc;
	}

	return 0;
}

static int mt9v113_set_effect(int effect)
{
	int rc = 0;

	if(prev_effect_mode == effect)
	{
		CDBG("### ; [CHECK]%s: skip this function, effect_mode -> %d\n", __func__, effect);
		return rc;
	}

       CDBG("###  ; [CHECK]%s: effect -> %d\n", __func__, effect);

	switch (effect) {
	case CAMERA_EFFECT_OFF:
	 	rc = mt9v113_i2c_write_table(mt9v113_regs.effect_default_tbl, mt9v113_regs.effect_default_tbl_size);
		if (rc < 0)
			return rc;
		break;
	case CAMERA_EFFECT_MONO:
	 	rc = mt9v113_i2c_write_table(mt9v113_regs.effect_mono_tbl, mt9v113_regs.effect_mono_tbl_size);
		if (rc < 0)
			return rc;
		break;
	case CAMERA_EFFECT_NEGATIVE:
	 	rc = mt9v113_i2c_write_table(mt9v113_regs.effect_negative_tbl, mt9v113_regs.effect_negative_tbl_size);
		if (rc < 0)
			return rc;
		break;
	case CAMERA_EFFECT_SOLARIZE:
	 	rc = mt9v113_i2c_write_table(mt9v113_regs.effect_solarization_tbl, mt9v113_regs.effect_solarization_tbl_size);
		if (rc < 0)
			return rc;
		break;
	case CAMERA_EFFECT_SEPIA:
	 	rc = mt9v113_i2c_write_table(mt9v113_regs.effect_sepia_tbl, mt9v113_regs.effect_sepia_tbl_size);
		if (rc < 0)
			return rc;
		break;
	case CAMERA_EFFECT_AQUA:
	 	rc = mt9v113_i2c_write_table(mt9v113_regs.effect_aqua_tbl, mt9v113_regs.effect_aqua_tbl_size);
		if (rc < 0)
			return rc;
		break;
	case CAMERA_EFFECT_POSTERIZE : // effect off code
	 	rc = mt9v113_i2c_write_table(mt9v113_regs.effect_default_tbl, mt9v113_regs.effect_default_tbl_size);
		if (rc < 0)
			return rc;
		break;
	default:
		return -EINVAL;
	}

	rc = mt9v113_check_sensor_mode();

	if (rc<0)
	{
		CDBG("###[ERROR]%s: failed to check sensor mode\n", __func__);
		return rc;
	}

	prev_effect_mode = effect;
	return rc;
}

static int mt9v113_set_wb(int mode)
{
	int32_t rc = 0;

	if(prev_balance_mode == mode)
	{
		CDBG(KERN_ERR "###  [CHECK]%s: skip this function, wb_mode -> %d\n", __func__, mode);
		return rc;
	}
       CDBG(KERN_ERR "###  [CHECK]%s: mode -> %d\n", __func__, mode);

	switch (mode) {
		case CAMERA_WB_AUTO:
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.wb_default_tbl, mt9v113_regs.wb_default_tbl_size);
			if (rc < 0)
				return rc;			
			break;
		case CAMERA_WB_DAYLIGHT:	// sunny
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.wb_sunny_tbl, mt9v113_regs.wb_sunny_tbl_size);
			if (rc < 0)
				return rc;		
			break;
		case CAMERA_WB_CLOUDY_DAYLIGHT:  // cloudy
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.wb_cloudy_tbl, mt9v113_regs.wb_cloudy_tbl_size);
			if (rc < 0)
				return rc;
			break;
		case CAMERA_WB_FLUORESCENT:
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.wb_fluorescent_tbl, mt9v113_regs.wb_fluorescent_tbl_size);
			if (rc < 0)
				return rc;
			break;			
		case CAMERA_WB_INCANDESCENT:
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.wb_incandescent_tbl, mt9v113_regs.wb_incandescent_tbl_size);
			if (rc < 0)
				return rc;
			break;
		default:
			return -EINVAL;
	}

	rc = mt9v113_check_sensor_mode();

	if (rc<0)
	{
		CDBG("###[ERROR]%s: failed to check sensor mode\n", __func__);
		return rc;
	}

	prev_balance_mode = mode;
	return rc;
}

#if 0 // donghyun.kwon(20111019) : build_error_fixed
static int mt9v113_set_iso(int mode)
{
	int32_t rc = 0;

	if(prev_iso_mode == mode)
	{
		CDBG(KERN_ERR "###  [CHECK]%s: skip this function, iso_mode -> %d\n", __func__, mode);
		return rc;
	}
       CDBG(KERN_ERR "###  [CHECK]%s: mode -> %d\n", __func__, mode);

	switch (mode) {
		case CAMERA_ISO_AUTO:
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.iso_default_tbl, mt9v113_regs.iso_default_tbl_size);
			if (rc < 0)
				return rc;
			break;
		case CAMERA_ISO_100:
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.iso_160_tbl, mt9v113_regs.iso_160_tbl_size);
			if (rc < 0)
				return rc;		
			break;
		case CAMERA_ISO_200:  
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.iso_200_tbl, mt9v113_regs.iso_200_tbl_size);
			if (rc < 0)
				return rc;
			break;
		case CAMERA_ISO_400:
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.iso_400_tbl, mt9v113_regs.iso_400_tbl_size);
			if (rc < 0)
				return rc;
			break;			
		default:
			return -EINVAL;
	}

	rc = mt9v113_check_sensor_mode();

	if (rc<0)
	{
		CDBG("###[ERROR]%s: failed to check sensor mode\n", __func__);
		return rc;
	}
	
	prev_iso_mode = mode;
	return rc;
}
#endif

#if 0 // donghyun.kwon(20111019) : build_error_fixed
// START LGE_BSP_CAMERA miracle.kim@lge.com 2011-07-29 for CTS test, focus distance
int32_t mt9v113_get_focus_distances(struct focus_distances_type *distances)
{
	int32_t rc = 0;
	
	// maximum distance
	distances->near_focus = 0.1; 
	distances->current_focus = 0.6;
	distances->far_focus = 0.6;
	
	return rc;
}
// END LGE_BSP_CAMERA miracle.kim@lge.com 2011-07-29 for CTS test, focus distance
#endif

static int brightness_table[] = {0x0015, 0x001C, 0x0022, 0x0029, 0x0032, 0x0039, 0x0042,  0x0049, 0x0052, 0x0059, 0x0062, 0x0069, 0x0072};
//{0x0000, 0x0002, 0x0009, 0x001A, 0x0028, 0x0034, 0x0042,  0x005C, 0x0067, 0x0070, 0x007E, 0x0088, 0x00A0}; // 13 step
static int mt9v113_set_brightness(int mode)
{
	int32_t rc = 0;

       printk(KERN_ERR "###  [CHECK]%s: mode -> %d\n", __func__, mode);

	if (prev_brightness_mode==mode){
		CDBG("###%s: skip mt9v113_set_brightness\n", __func__);
		return rc;
	}
	mode = mode + 6 ;		// donghyun.kwon(20111022) : VGA brightness test
	if(mode < 0 || mode > 12){
		CDBG("###[ERROR]%s: Invalid Mode value\n", __func__);
		return -EINVAL;	
	}
	rc = mt9v113_i2c_write_w_sensor(0x098C, 0xA24F);
	if (rc < 0)
		return rc;	
	rc = mt9v113_i2c_write_w_sensor(0x0990, brightness_table[mode]);
	if (rc < 0)
		return rc;

 	rc = mt9v113_i2c_write_table(mt9v113_regs.change_config_tbl, mt9v113_regs.change_config_tbl_size);
	if (rc < 0)
		return rc;	

	rc = mt9v113_check_sensor_mode();

	if (rc<0)
	{
		CDBG("###[ERROR]%s: failed to check sensor mode\n", __func__);
		return rc;
	}
	prev_brightness_mode = mode;

	return rc;
}

static int mt9v113_reg_preview(void)
{
	int rc = 0;

	msleep(200); //miracle.kim@lge.com 2011-07-21 added for stability

	return rc;
}

static int mt9v113_reg_snapshot(void)
{
	int rc = 0;

	return rc;
}

static long mt9v113_set_sensor_mode(int mode)
{
	int32_t rc = 0;

	int retry = 0;
	struct msm_camera_csi_params mt9v113_csi_params;

	if (prev_sensor_mode == mode)
		return rc;

	/* config mipi csi controller */

	CDBG("%s: config mipi csi controller\n", __func__);
	if (config_csi == 0) {

		/* stop streaming */
		rc = mt9v113_i2c_write_w_sensor(REG_STANDBY_CONTROL, ENABLE_STANDBY_MODE);
		msleep(mt9v113_delay_msecs_stdby);

		mt9v113_csi_params.lane_cnt = 1;
		mt9v113_csi_params.data_format = CSI_8BIT;
		mt9v113_csi_params.lane_assign = 0xe4;
		mt9v113_csi_params.dpcm_scheme = 0;
		mt9v113_csi_params.settle_cnt = 0x14;

	       CDBG("%s: config mipi enter \n", __func__);
		rc = msm_camio_csi_config(&mt9v113_csi_params);
		if (rc < 0)
			CDBG("config csi controller failed \n");

		msleep(50);
		config_csi = 1;

		/* start streaming */ 
		rc = mt9v113_i2c_write_w_sensor(REG_STANDBY_CONTROL, DISABLE_STANDBY_MODE);
		msleep(mt9v113_delay_msecs_stream);

	}

	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		for (retry = 0; retry < 3; ++retry) {
			CDBG("[ERROR]%s:Sensor Preview Mode In\n", __func__);
			rc = mt9v113_reg_preview();
			if (rc < 0)
				CDBG("[ERROR]%s:Sensor Preview Mode Fail\n", __func__);
			else {
				prev_sensor_mode = SENSOR_PREVIEW_MODE;
				break;
			}
		}
		break;
	case SENSOR_SNAPSHOT_MODE:
	case SENSOR_RAW_SNAPSHOT_MODE:
		for (retry = 0; retry < 3; ++retry) {
			CDBG("[ERROR]%s:Sensor Snapshot Mode In\n", __func__);
			rc = mt9v113_reg_snapshot();
			if (rc < 0)
				CDBG("[ERROR]%s:Sensor Snapshot Mode Fail\n", __func__);
			else {
				prev_sensor_mode = SENSOR_RAW_SNAPSHOT_MODE;
				break;
			}
		}
		break;		
	default:
		rc = -EINVAL;
		break;
	}
		
	return rc;
}

#if 0 // donghyun.kwon(20111019) : build_error_fixed
// LGE_CAMERA_S : Adjust VT Cam frame rate - jonghwan.ko@lge.com
static int mt9v113_set_Fps(int mode)
{
	int32_t rc = 0;
	
	CDBG(KERN_ERR "mt9v113_set_Fps mode = %d, prev_fps_mode=%d \n ",mode, prev_fps_mode);

	if(prev_fps_mode == mode)
	{
		CDBG(KERN_ERR "###  [CHECK]%s: skip this function, prev_fps_mode -> %d\n", __func__, mode);
		return rc;
	}
	switch (mode) {		
		case SENSOR_FIXED_FPS_15://CAMERA_FPS_15:
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.fps_fixed_15_tbl, mt9v113_regs.fps_fixed_15_tbl_size);
			if (rc < 0)
				return rc;
			break;
			
		case SENSOR_FIXED_FPS_30: //CAMERA_FPS_30:	// sunny
			// 30 fps fixed 
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.fps_fixed_30_tbl, mt9v113_regs.fps_fixed_30_tbl_size);
			if (rc < 0)
				return rc;
			break;
			
		case SENSOR_AUTO_FPS_1030: //CAMERA_FPS_10to30:
			// 10 fps ~ 30 fps variable 
		 	rc = mt9v113_i2c_write_table(mt9v113_regs.fps_auto_1030_tbl, mt9v113_regs.fps_auto_1030_tbl_size);
			if (rc < 0)
				return rc;
			break;
			
		case SENSOR_AUTO_FPS_0730://CAMERA_FPS_7to30:  // cloudy
			 // 7.5 fps ~ 30 fps variable
			 rc = mt9v113_i2c_write_table(mt9v113_regs.fps_auto_730_tbl, mt9v113_regs.fps_auto_730_tbl_size);
			 if (rc < 0)
				 return rc;
			 break;
		default:
		       CDBG(KERN_ERR "mt9v113_set_Fps wrong value : %d \n ",mode);
		       rc =0;
		       return rc;
	}
	
	prev_fps_mode = mode;
	CDBG(KERN_ERR "mt9v113_set_Fps Change Frame rate \n ");

       if(prev_brightness_mode != 6) // Set again when mode is not center 
	{	
		CDBG(KERN_ERR "### Set again when mode is not center, value is %d\n ",prev_brightness_mode);
		rc = mt9v113_i2c_write_w_sensor(0x098C, 0xA24F);
		if (rc < 0)
		   return rc;  
		
		rc = mt9v113_i2c_write_w_sensor(0x0990, brightness_table[prev_brightness_mode]);
		if (rc < 0)
		   return rc;
		
		rc = mt9v113_i2c_write_table(mt9v113_regs.change_config_tbl, mt9v113_regs.change_config_tbl_size);
		if (rc < 0)
			return rc;	
		
		rc = mt9v113_check_sensor_mode();
		if (rc<0)
		{
		   CDBG("###[ERROR]%s: failed to check sensor mode\n", __func__);
		   return rc;
		}
       }
	   
	return rc;
}
// LGE_CAMERA_E : Adjust VT Cam frame rate - jonghwan.ko@lge.com
#endif

int mt9v113_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cfg_data;
	long   rc = 0;

	if (copy_from_user(&cfg_data,
			(void *)argp,
			sizeof(struct sensor_cfg_data)))
		return -EFAULT;

	mutex_lock(&mt9v113_sem);

	CDBG("mt9v113_ioctl, cfgtype = %d, mode = %d\n",
		cfg_data.cfgtype, cfg_data.mode);

		switch (cfg_data.cfgtype) {
		case CFG_SET_MODE:
			rc = mt9v113_set_sensor_mode(cfg_data.mode);
			break;

		case CFG_SET_EFFECT:
			rc = mt9v113_set_effect(cfg_data.mode);
			break;

		case CFG_SET_WB:
			rc = mt9v113_set_wb(cfg_data.mode);			
			break;

#if 0 // donghyun.kwon(20111019) : build_error_fixed
		case CFG_SET_ISO:
			rc = mt9v113_set_iso(cfg_data.mode);
			
			break;
#endif

		case CFG_SET_ANTIBANDING:
//			rc = mt9v113_set_antibanding(cfg_data.mode);
			
			break;
			
		case CFG_SET_BRIGHTNESS:
			rc = mt9v113_set_brightness(cfg_data.mode);
			
			break;

#if 0 // donghyun.kwon(20111019) : build_error_fixed
 // START LGE_BSP_CAMERA miracle.kim@lge.com 2011-07-29 for CTS test, focus distance
	 case CFG_GET_FOCUS_DISTANCES:			 
		 rc = mt9v113_get_focus_distances(&cfg_data.focus_distances);
		 if (copy_to_user((void *)argp, &cfg_data,sizeof(struct sensor_cfg_data)))
			 rc = -EFAULT;
		 break;
 // END LGE_BSP_CAMERA miracle.kim@lge.com 
 // LGE_CAMERA_S : Adjust VT Cam frame rate - jonghwan.ko@lge.com		 
		 case CFG_FIXED_FPS:
		   rc = mt9v113_set_Fps(cfg_data.mode);
		   break;
 // LGE_CAMERA_E : Adjust VT Cam frame rate - jonghwan.ko@lge.com			 
#endif

		default:
			rc = -EINVAL;
			break;
		}

	mutex_unlock(&mt9v113_sem);

	return rc;
}

static int mt9v113_sensor_init_probe(const struct msm_camera_sensor_info *data)
{
	int rc = 0;
	unsigned short model_id = 0;
	
	CDBG("%s in :%d\n",__func__, __LINE__);
	
	rc = gpio_request(data->sensor_reset, "mt9v113");

	if(rc<0){
		CDBG(" mt9v113_sensor_init_probe fails\n");
		return rc;
	}
	
#if 0 // donghyun.kwon(20111019) : build_error_fixed
	rc = gpio_request(data->vt_mclk_enable, "mt9v113");

	if(rc<0){
		CDBG(" mt9v113_sensor_init_probe fails\n");
		return rc;
	}
	
	rc = gpio_direction_output(data->vt_mclk_enable, 0);
	CDBG("[elin.lee] Cam MCLK OFF\n");
	
	if (rc < 0)
		goto init_probe_fail;
#else
	rc = gpio_request(15, "mt9v113");

	if(rc<0){
		CDBG(" mt9v113_sensor_init_probe fails\n");
		return rc;
	}
	
	rc = gpio_direction_output(15, 0);
	CDBG("[elin.lee] Cam MCLK OFF\n");
	
	if (rc < 0)
		goto init_probe_fail;
#endif

	rc = gpio_direction_output(data->sensor_reset, 1);
	CDBG("[elin.lee] Cam Reset 1\n");
	
	if (rc < 0)
		goto init_probe_fail;
	
	data->pdata->camera_power_on();
	mdelay(2);
	
	rc = gpio_direction_output(data->sensor_reset, 0);
	CDBG("[elin.lee] Cam Reset 0\n");

	if (rc < 0)
		goto init_probe_fail;

	mdelay(10);

#if 0 // donghyun.kwon(20111019) : build_error_fixed
	rc = gpio_direction_output(data->vt_mclk_enable, 1);
	CDBG("[elin.lee] Cam MCLK ON\n");
	
	if (rc < 0)
		goto init_probe_fail;
#else
	rc = gpio_direction_output(15, 1);
	CDBG("[elin.lee] Cam MCLK ON\n");
	
	if (rc < 0)
		goto init_probe_fail;
#endif

#if 0
	//msleep(500);

	/* enable mclk first */
	printk("[elin.lee]  Enable main Mclk  \n");
	msm_camio_clk_rate_set(MT9V113_DEFAULT_MASTER_CLK_RATE);
	msleep(2);
	
	CDBG("init entry \n");

	rc = mt9v113_reset(data);
	if (rc < 0) {
		CDBG("reset failed!\n");
		goto init_probe_fail;
	}
#endif

	mdelay(1);

	rc = gpio_direction_output(data->sensor_reset, 1);
	CDBG("[elin.lee] Cam Reset 1\n");

	if (rc < 0)
		goto init_probe_fail;
	
	mdelay(1);

	/* Micron suggested Power up block Start:
	* Put MCU into Reset - Stop MCU */
#if 0	
	rc = mt9v113_i2c_write(mt9v113_client->addr,
		REG_MT9V113_MCU_BOOT, 0x0501, WORD_LEN);
	if (rc < 0)
		goto init_probe_fail;
#endif
	/* Micron suggested Power up block End */
	/* Read the Model ID of the sensor */
	rc = mt9v113_i2c_read(mt9v113_client->addr,
		REG_MT9V113_MODEL_ID, &model_id, WORD_LEN);
	if (rc < 0)
		goto init_probe_fail;

	/* Check if it matches it with the value in Datasheet */
	if (model_id != MT9V113_MODEL_ID) {
		rc = -EINVAL;
		goto init_probe_fail;
	}

	CDBG("mt9v113 model id = 0x%x", model_id);

	rc = mt9v113_reg_init();
	if (rc < 0)
		goto init_probe_fail;

	CDBG("mt9v113_sensor_init_probe done\n");
	return rc;

init_probe_fail:
	mt9v113_probe_init_done(data);
	return rc;
}

int mt9v113_sensor_init(const struct msm_camera_sensor_info *data)
{
	int rc = 0;

	mt9v113_ctrl = kzalloc(sizeof(struct mt9v113_ctrl_t), GFP_KERNEL);
	if (!mt9v113_ctrl) {
		CDBG("mt9v113_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}

	config_csi = 0;
	
	if (data)
		mt9v113_ctrl->sensordata = data;


#ifdef CONFIG_MT9V113
//	msm_camio_camif_pad_reg_reset();
#endif

	mutex_lock(&mt9v113_sem);

	rc = mt9v113_sensor_init_probe(data);

	mutex_unlock(&mt9v113_sem);

	if (rc < 0) {
		CDBG("mt9v113_sensor_init failed!\n");
		goto init_fail;
	}

	prev_effect_mode = -1;	
	prev_balance_mode = -1;
	prev_iso_mode = -1;	
	prev_sensor_mode = -1;
	prev_brightness_mode = -1;

init_done:
	return rc;

init_fail:
	kfree(mt9v113_ctrl);
	return rc;
}

static int mt9v113_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&mt9v113_wait_queue);
	return 0;
}

int mt9v113_sensor_release(void)
{
	int rc = 0;
	CDBG("mt9v113_entering Sensor_release\n");

	mutex_lock(&mt9v113_sem);
	
	//gpio_direction_output(mt9v113_ctrl->sensordata->sensor_reset, 0);
	gpio_set_value_cansleep(mt9v113_ctrl->sensordata->sensor_reset, 0);
#if 0 // donghyun.kwon(20111019) : build_error_fixed
	gpio_set_value_cansleep(mt9v113_ctrl->sensordata->vt_mclk_enable, 0);
	gpio_free(mt9v113_ctrl->sensordata->sensor_reset);
	gpio_free(mt9v113_ctrl->sensordata->vt_mclk_enable);
#else
	gpio_set_value_cansleep(15, 0);
	gpio_free(mt9v113_ctrl->sensordata->sensor_reset);
	gpio_free(15);
#endif
	mt9v113_ctrl->sensordata->pdata->camera_power_off();

	kfree(mt9v113_ctrl);
	mt9v113_ctrl = NULL;
	CDBG("mt9v113_release completed\n");

	mutex_unlock(&mt9v113_sem);

	return rc;
}

static int mt9v113_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	CDBG("mt9v113_i2c_probe called!\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		CDBG("i2c_check_functionality failed\n");
		goto probe_failure;
	}

	mt9v113_sensorw =
		kzalloc(sizeof(struct mt9v113_work), GFP_KERNEL);

	if (!mt9v113_sensorw) {
		rc = -ENOMEM;
		CDBG("kzalloc failed.\n");
		goto probe_failure;
	}

	i2c_set_clientdata(client, mt9v113_sensorw);
	mt9v113_init_client(client);
	mt9v113_client = client;

	CDBG("mt9v113_probe succeeded!\n");

	return 0;

probe_failure:
	kfree(mt9v113_sensorw);
	mt9v113_sensorw = NULL;
	CDBG("mt9v113_probe failed!\n");
	return rc;
}

static const struct i2c_device_id mt9v113_i2c_id[] = {
	{ "mt9v113", 0},
	{ }
};

static struct i2c_driver mt9v113_i2c_driver = {
	.id_table = mt9v113_i2c_id,
	.probe  = mt9v113_i2c_probe,
	.remove = __exit_p(mt9v113_i2c_remove),
	.driver = {
		.name = "mt9v113",
	},
};

static int mt9v113_sensor_probe(const struct msm_camera_sensor_info *info,
				struct msm_sensor_ctrl *s)
{
	int rc = i2c_add_driver(&mt9v113_i2c_driver);
	if (rc < 0 || mt9v113_client == NULL) {
		CDBG("%s: ret =%d\n",__func__,rc);
		rc = -ENOTSUPP;
		goto probe_done;
	}

	s->s_init = mt9v113_sensor_init;
	s->s_release = mt9v113_sensor_release;
	s->s_config  = mt9v113_sensor_config;
	s->s_camera_type = FRONT_CAMERA_2D;
	s->s_mount_angle  = 270; // miracle.kim@lge.com 2011-07-19, 270 for flip, 90 for mirror
probe_done:
	CDBG("%s %s:%d\n", __FILE__, __func__, __LINE__);
	return rc;
}

static int __mt9v113_probe(struct platform_device *pdev)
{
	return msm_camera_drv_start(pdev, mt9v113_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __mt9v113_probe,
	.driver = {
		.name = "msm_camera_mt9v113",
		.owner = THIS_MODULE,
	},
};

static int __init mt9v113_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

module_init(mt9v113_init);
