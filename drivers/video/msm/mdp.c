/* drivers/video/msm_fb/mdp.c
 *
 * MSM MDP Interface (used by framebuffer core)
 *
 * Copyright (c) 2007-2012, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2007 Google Incorporated
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/hrtimer.h>
#include <linux/clk.h>
#include <mach/hardware.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <mach/clk.h>
#include "mdp.h"
#include "msm_fb.h"
#ifdef CONFIG_FB_MSM_MDP40
#include "mdp4.h"
#endif
#include "mipi_dsi.h"

uint32 mdp4_extn_disp;

static struct clk *mdp_clk;
static struct clk *mdp_pclk;
static struct clk *mdp_lut_clk;
int mdp_rev;

static struct regulator *footswitch;
static unsigned int mdp_footswitch_on;

struct completion mdp_ppp_comp;
struct semaphore mdp_ppp_mutex;
struct semaphore mdp_pipe_ctrl_mutex;

unsigned long mdp_timer_duration = (HZ/20);   /* 50 msecond */

boolean mdp_ppp_waiting = FALSE;
uint32 mdp_tv_underflow_cnt;
uint32 mdp_lcdc_underflow_cnt;

boolean mdp_current_clk_on = FALSE;
boolean mdp_is_in_isr = FALSE;

/*
 * legacy mdp_in_processing is only for DMA2-MDDI
 * this applies to DMA2 block only
 */
uint32 mdp_in_processing = FALSE;

#ifdef CONFIG_FB_MSM_MDP40
uint32 mdp_intr_mask = MDP4_ANY_INTR_MASK;
#else
uint32 mdp_intr_mask = MDP_ANY_INTR_MASK;
#endif

MDP_BLOCK_TYPE mdp_debug[MDP_MAX_BLOCK];

atomic_t mdp_block_power_cnt[MDP_MAX_BLOCK];

spinlock_t mdp_spin_lock;
struct workqueue_struct *mdp_dma_wq;	/*mdp dma wq */
struct workqueue_struct *mdp_vsync_wq;	/*mdp vsync wq */

struct workqueue_struct *mdp_hist_wq;	/*mdp histogram wq */
struct work_struct mdp_histogram_worker;

static struct workqueue_struct *mdp_pipe_ctrl_wq; /* mdp mdp pipe ctrl wq */
static struct delayed_work mdp_pipe_ctrl_worker;

static boolean mdp_suspended = FALSE;
DEFINE_MUTEX(mdp_suspend_mutex);

#ifdef CONFIG_FB_MSM_MDP40
struct mdp_dma_data dma2_data;
struct mdp_dma_data dma_s_data;
struct mdp_dma_data dma_e_data;
ulong mdp4_display_intf;
#else
static struct mdp_dma_data dma2_data;
static struct mdp_dma_data dma_s_data;
#ifndef CONFIG_FB_MSM_MDP303
static struct mdp_dma_data dma_e_data;
#endif
#endif

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
struct mdp_dma_data dma_wb_data;
#endif

static struct mdp_dma_data dma3_data;

extern ktime_t mdp_dma2_last_update_time;

extern uint32 mdp_dma2_update_time_in_usec;
extern int mdp_lcd_rd_cnt_offset_slow;
extern int mdp_lcd_rd_cnt_offset_fast;
extern int mdp_usec_diff_threshold;

extern int first_pixel_start_x;
extern int first_pixel_start_y;

#ifdef MSM_FB_ENABLE_DBGFS
struct dentry *mdp_dir;
#endif

#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
static int mdp_suspend(struct platform_device *pdev, pm_message_t state);
#else
#define mdp_suspend NULL
#endif

struct timeval mdp_dma2_timeval;
struct timeval mdp_ppp_timeval;

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend early_suspend;
#endif

static u32 mdp_irq;

static uint32 mdp_prim_panel_type = NO_PANEL;
#ifndef CONFIG_FB_MSM_MDP22

struct list_head mdp_hist_lut_list;
DEFINE_MUTEX(mdp_hist_lut_list_mutex);

uint32_t mdp_block2base(uint32_t block)
{
	uint32_t base = 0x0;
	switch (block) {
	case MDP_BLOCK_DMA_P:
		base = 0x90000;
		break;
	case MDP_BLOCK_DMA_S:
		base = 0xA0000;
		break;
	case MDP_BLOCK_VG_1:
		base = 0x20000;
		break;
	case MDP_BLOCK_VG_2:
		base = 0x30000;
		break;
	case MDP_BLOCK_RGB_1:
		base = 0x40000;
		break;
	case MDP_BLOCK_RGB_2:
		base = 0x50000;
		break;
	case MDP_BLOCK_OVERLAY_0:
		base = 0x10000;
		break;
	case MDP_BLOCK_OVERLAY_1:
		base = 0x18000;
		break;
	default:
		break;
	}
	return base;
}

static uint32_t mdp_pp_block2hist_lut(uint32_t block)
{
	uint32_t valid = 0;
	switch (block) {
	case MDP_BLOCK_DMA_P:
		valid = (mdp_rev >= MDP_REV_40) ? 1 : 0;
		break;
	case MDP_BLOCK_DMA_S:
		valid = (mdp_rev >= MDP_REV_40) ? 1 : 0;
		break;
	case MDP_BLOCK_VG_1:
		valid = (mdp_rev >= MDP_REV_40) ? 1 : 0;
		break;
	case MDP_BLOCK_VG_2:
		valid = (mdp_rev >= MDP_REV_40) ? 1 : 0;
		break;
	default:
		break;
	}
	return valid;
}

static void mdp_hist_lut_init_mgmt(struct mdp_hist_lut_mgmt *mgmt,
		uint32_t block)
{
	mutex_init(&mgmt->lock);
	mgmt->block = block;

	mutex_lock(&mdp_hist_lut_list_mutex);
	list_add(&mgmt->list, &mdp_hist_lut_list);
	mutex_unlock(&mdp_hist_lut_list_mutex);
}

static int mdp_hist_lut_init(void)
{
	struct mdp_hist_lut_mgmt *temp;
	struct list_head *pos, *q;
	INIT_LIST_HEAD(&mdp_hist_lut_list);

	if (mdp_rev >= MDP_REV_30) {
		temp = kmalloc(sizeof(struct mdp_hist_lut_mgmt), GFP_KERNEL);
		if (!temp)
			goto exit;
		mdp_hist_lut_init_mgmt(temp, MDP_BLOCK_DMA_P);
	}

	if (mdp_rev >= MDP_REV_40) {
		temp = kmalloc(sizeof(struct mdp_hist_lut_mgmt), GFP_KERNEL);
		if (!temp)
			goto exit_list;
		mdp_hist_lut_init_mgmt(temp, MDP_BLOCK_VG_1);

		temp = kmalloc(sizeof(struct mdp_hist_lut_mgmt), GFP_KERNEL);
		if (!temp)
			goto exit_list;
		mdp_hist_lut_init_mgmt(temp, MDP_BLOCK_VG_2);
	}

	if (mdp_rev > MDP_REV_42) {
		temp = kmalloc(sizeof(struct mdp_hist_lut_mgmt), GFP_KERNEL);
		if (!temp)
			goto exit_list;
		mdp_hist_lut_init_mgmt(temp, MDP_BLOCK_DMA_S);
	}
	return 0;

exit_list:
	mutex_lock(&mdp_hist_lut_list_mutex);
	list_for_each_safe(pos, q, &mdp_hist_lut_list) {
		temp = list_entry(pos, struct mdp_hist_lut_mgmt, list);
		list_del(pos);
		kfree(temp);
	}
	mutex_unlock(&mdp_hist_lut_list_mutex);
exit:
	pr_err("Failed initializing histogram LUT memory\n");
	return -ENOMEM;
}

static int mdp_hist_lut_block2mgmt(uint32_t block,
		struct mdp_hist_lut_mgmt **mgmt)
{
	struct mdp_hist_lut_mgmt *temp, *output;
	int ret = 0;

	output = NULL;

	mutex_lock(&mdp_hist_lut_list_mutex);
	list_for_each_entry(temp, &mdp_hist_lut_list, list) {
		if (temp->block == block)
			output = temp;
	}
	mutex_unlock(&mdp_hist_lut_list_mutex);

	if (output == NULL)
		ret = -EINVAL;
	else
		*mgmt = output;

	return ret;
}

#define MDP_HIST_LUT_SIZE (256)
static int mdp_hist_lut_write_off(struct mdp_hist_lut_data *data,
		struct mdp_hist_lut_info *info, uint32_t offset)
{
	int i;
	uint32_t element[MDP_HIST_LUT_SIZE];
	uint32_t base = mdp_block2base(info->block);
	uint32_t sel = info->bank_sel;


	if (data->len != MDP_HIST_LUT_SIZE) {
		pr_err("%s: data->len != %d", __func__, MDP_HIST_LUT_SIZE);
		return -EINVAL;
	}

	if (copy_from_user(&element, data->data,
				MDP_HIST_LUT_SIZE * sizeof(uint32_t))) {
		pr_err("%s: Error copying histogram data", __func__);
		return -ENOMEM;
	}
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	for (i = 0; i < MDP_HIST_LUT_SIZE; i++)
		MDP_OUTP(MDP_BASE + base + offset + (0x400*(sel)) + (4*i),
				element[i]);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	return 0;
}

static int mdp_hist_lut_write(struct mdp_hist_lut_data *data,
						struct mdp_hist_lut_info *info)
{
	int ret = 0;

	if (data->block != info->block) {
		ret = -1;
		pr_err("%s, data/info mdp_block mismatch! %d != %d\n",
				__func__, data->block, info->block);
		goto error;
	}

	switch (data->block) {
	case MDP_BLOCK_VG_1:
	case MDP_BLOCK_VG_2:
		ret = mdp_hist_lut_write_off(data, info, 0x3400);
		break;
	case MDP_BLOCK_DMA_P:
	case MDP_BLOCK_DMA_S:
		ret = mdp_hist_lut_write_off(data, info, 0x4800);
		break;
	default:
		ret = -EINVAL;
		goto error;
	}

error:
	return ret;
}

#define MDP_HIST_LUT_VG_EN_MASK (0x20000)
#define MDP_HIST_LUT_VG_EN_SHIFT (17)
#define MDP_HIST_LUT_VG_EN_OFFSET (0x0058)
#define MDP_HIST_LUT_VG_SEL_OFFSET (0x0064)
static void mdp_hist_lut_commit_vg(struct mdp_hist_lut_info *info)
{
	uint32_t out_en, temp_en;
	uint32_t base = mdp_block2base(info->block);
	temp_en = (info->is_enabled) ? (1 << MDP_HIST_LUT_VG_EN_SHIFT) : 0x0;

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	out_en = inpdw(MDP_BASE + base + MDP_HIST_LUT_VG_EN_OFFSET) &
						~MDP_HIST_LUT_VG_EN_MASK;
	MDP_OUTP(MDP_BASE + base + MDP_HIST_LUT_VG_EN_OFFSET, out_en | temp_en);

	if (info->has_sel_update)
		MDP_OUTP(MDP_BASE + base + MDP_HIST_LUT_VG_SEL_OFFSET,
								info->bank_sel);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
}

#define MDP_HIST_LUT_DMA_EN_MASK (0x7)
#define MDP_HIST_LUT_DMA_SEL_MASK (0x400)
#define MDP_HIST_LUT_DMA_SEL_SHIFT (10)
#define MDP_HIST_LUT_DMA_P_OFFSET (0x0070)
#define MDP_HIST_LUT_DMA_S_OFFSET (0x0028)
static void mdp_hist_lut_commit_dma(struct mdp_hist_lut_info *info)
{
	uint32_t out, temp, mask;
	uint32_t base = mdp_block2base(info->block);
	uint32_t offset = (info->block == MDP_BLOCK_DMA_P) ?
		MDP_HIST_LUT_DMA_P_OFFSET : MDP_HIST_LUT_DMA_S_OFFSET;

	mask = MDP_HIST_LUT_DMA_EN_MASK;
	temp = (info->is_enabled) ? 0x7 : 0x0;

	if (info->has_sel_update) {
		mask |= MDP_HIST_LUT_DMA_SEL_MASK;
		temp |=  ((info->bank_sel & 0x1) << MDP_HIST_LUT_DMA_SEL_SHIFT);
	}

	out = inpdw(MDP_BASE + base + offset) & ~mask;
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	MDP_OUTP(MDP_BASE + base + offset, out | temp);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
}

static void mdp_hist_lut_commit_info(struct mdp_hist_lut_info *info)
{
	switch (info->block) {
	case MDP_BLOCK_VG_1:
	case MDP_BLOCK_VG_2:
		mdp_hist_lut_commit_vg(info);
		break;
	case MDP_BLOCK_DMA_P:
	case MDP_BLOCK_DMA_S:
		mdp_hist_lut_commit_dma(info);
		break;
	default:
		goto error;
	}

error:
	return;
}

static void mdp_hist_lut_update_info(struct mdp_hist_lut_info *info, int ops)
{
	info->bank_sel = (ops & 0x8) >> 3;
	info->is_enabled = (ops & 0x1) ? TRUE : FALSE;
	info->has_sel_update = (ops & 0x10) ? TRUE : FALSE;
}

int mdp_hist_lut_config(struct mdp_hist_lut_data *data)
{
	struct mdp_hist_lut_mgmt *mgmt = NULL;
	struct mdp_hist_lut_info info;
	int ret = 0;

	if (!mdp_pp_block2hist_lut(data->block)) {
		ret = -ENOTTY;
		goto error;
	}

	ret = mdp_hist_lut_block2mgmt(data->block, &mgmt);
	if (ret)
		goto error;

	mutex_lock(&mgmt->lock);

	info.block = mgmt->block;

	mdp_hist_lut_update_info(&info, data->ops);

	switch ((data->ops & 0x6) >> 1) {
	case 0x1:
		pr_info("%s: histogram LUT read not supported\n", __func__);
		break;
	case 0x2:
		ret = mdp_hist_lut_write(data, &info);
		if (ret)
			goto error_lock;
		break;
	default:
		break;
	}

	mdp_hist_lut_commit_info(&info);

error_lock:
	mutex_unlock(&mgmt->lock);
error:
	return ret;
}


DEFINE_MUTEX(mdp_lut_push_sem);
static int mdp_lut_i;
/* LGE_CHANGE_S: jongyoung.koo@lge.com [2012-01-16] 
: To display white screen */
#ifdef CONFIG_FB_MSM_MDP_LUT_ENABLE
extern int g_kcal[6];

uint32 lge_gc_lut[] = {	 /* non linear */
/* The base of M3
	0x00000000,0x00010101,0x00020202,0x00020202,0x00030303,0x00040404,0x00050505,0x00050505,
	0x00060606,0x00070707,0x00080808,0x00080808,0x00090909,0x000A0A0A,0x000B0B0B,0x000B0B0B,
	0x000C0C0C,0x000D0D0D,0x000E0E0E,0x000E0E0E,0x000F0F0F,0x00101010,0x00111111,0x00121212,
	0x00121212,0x00131313,0x00141414,0x00151515,0x00161616,0x00161616,0x00171717,0x00181818,
	0x00191919,0x001A1A1A,0x001B1B1B,0x001B1B1B,0x001C1C1C,0x001D1D1D,0x001E1E1E,0x001F1F1F,
	0x00202020,0x00212121,0x00212121,0x00222222,0x00232323,0x00242424,0x00252525,0x00262626,
	0x00272727,0x00282828,0x00292929,0x00292929,0x002A2A2A,0x002B2B2B,0x002C2C2C,0x002D2D2D,
	0x002E2E2E,0x002F2F2F,0x00303030,0x00313131,0x00323232,0x00333333,0x00343434,0x00353535,
	0x00363636,0x00373737,0x00373737,0x00383838,0x00393939,0x003A3A3A,0x003A3A3A,0x003B3B3B,
	0x003C3C3C,0x003D3D3D,0x003E3E3E,0x003F3F3F,0x00404040,0x00414141,0x00424242,0x00434343,
	0x00444444,0x00454545,0x00464646,0x00474747,0x00494949,0x004A4A4A,0x004B4B4B,0x004C4C4C,
	0x004E4E4E,0x004F4F4F,0x00515151,0x00525252,0x00535353,0x00555555,0x00565656,0x00585858,
	0x00595959,0x005A5A5A,0x005B5B5B,0x005D5D5D,0x005E5E5E,0x005F5F5F,0x00606060,0x00616161,
	0x00636363,0x00646464,0x00656565,0x00666666,0x00676767,0x00696969,0x006A6A6A,0x006B6B6B,
	0x006C6C6C,0x006E6E6E,0x006F6F6F,0x00707070,0x00717171,0x00737373,0x00747474,0x00757575,
	0x00767676,0x00787878,0x00797979,0x007A7A7A,0x007B7B7B,0x007D7D7D,0x007E7E7E,0x007F7F7F,
	0x00808080,0x00818181,0x00828282,0x00848484,0x00858585,0x00868686,0x00878787,0x00898989,
	0x008A8A8A,0x008B8B8B,0x008C8C8C,0x008E8E8E,0x008F8F8F,0x00909090,0x00919191,0x00939393,
	0x00949494,0x00959595,0x00969696,0x00989898,0x00999999,0x009A9A9A,0x009B9B9B,0x009C9C9C,
	0x009E9E9E,0x009F9F9F,0x00A0A0A0,0x00A1A1A1,0x00A2A2A2,0x00A4A4A4,0x00A5A5A5,0x00A6A6A6,
	0x00A7A7A7,0x00A8A8A8,0x00A9A9A9,0x00ABABAB,0x00ACACAC,0x00ADADAD,0x00AEAEAE,0x00AFAFAF,
	0x00B0B0B0,0x00B1B1B1,0x00B3B3B3,0x00B4B4B4,0x00B5B5B5,0x00B6B6B6,0x00B7B7B7,0x00B8B8B8,
	0x00B9B9B9,0x00BABABA,0x00BBBBBB,0x00BDBDBD,0x00BEBEBE,0x00BFBFBF,0x00C0C0C0,0x00C1C1C1,
	0x00C2C2C2,0x00C3C3C3,0x00C4C4C4,0x00C5C5C5,0x00C6C6C6,0x00C7C7C7,0x00C8C8C8,0x00C9C9C9,
	0x00CACACA,0x00CBCBCB,0x00CCCCCC,0x00CECECE,0x00CFCFCF,0x00D0D0D0,0x00D1D1D1,0x00D2D2D2,
	0x00D3D3D3,0x00D4D4D4,0x00D5D5D5,0x00D6D6D6,0x00D7D7D7,0x00D8D8D8,0x00D9D9D9,0x00DADADA,
	0x00DBDBDB,0x00DCDCDC,0x00DDDDDD,0x00DEDEDE,0x00DFDFDF,0x00DFDFDF,0x00E0E0E0,0x00E1E1E1,
	0x00E2E2E2,0x00E2E2E2,0x00E3E3E3,0x00E4E4E4,0x00E4E4E4,0x00E5E5E5,0x00E5E5E5,0x00E6E6E6,
	0x00E7E7E7,0x00E8E8E8,0x00E8E8E8,0x00E9E9E9,0x00EAEAEA,0x00EBEBEB,0x00EBEBEB,0x00ECECEC,
	0x00EDEDED,0x00EDEDED,0x00EEEEEE,0x00EFEFEF,0x00F0F0F0,0x00F0F0F0,0x00F1F1F1,0x00F2F2F2,
	0x00F3F3F3,0x00F3F3F3,0x00F4F4F4,0x00F5F5F5,0x00F6F6F6,0x00F7F7F7,0x00F7F7F7,0x00F8F8F8,
	0x00F9F9F9,0x00FAFAFA,0x00FBFBFB,0x00FCFCFC,0x00FCFCFC,0x00FDFDFD,0x00FEFEFE,0x00FFFFFF,
*/
 // U0


#if 1  //  yangwook.lim@lge.com [2012-02-20] 
0X00000000,
0X00010101,
0X00020202,
0X00030302,
0X00030303,
0X00040404,
0X00050505,
0X00060606,
0X00070706,
0X00080807,
0X00090908,
0X00090909,
0X000A0A0A,
0X000B0B0A,
0X000C0C0B,
0X000D0D0C,
0X000E0E0D,
0X000E0F0D,
0X000F0F0E,
0X0010100F,
0X00111110,
0X00121210,
0X00121311,
0X00131312,
0X00141413,
0X00151513,
0X00161614,
0X00171715,
0X00171816,
0X00181817,
0X00191917,
0X001A1A18,
0X001B1B19,
0X001C1C1A,
0X001D1D1B,
0X001D1E1B,
0X001E1F1C,
0X001F1F1D,
0X0020201E,
0X0021211F,
0X00222220,
0X00232320,
0X00242421,
0X00252522,
0X00262623,
0X00272724,
0X00272825,
0X00282926,
0X00292A27,
0X002A2B27,
0X002B2C28,
0X002C2D29,
0X002D2E2A,
0X002E2F2B,
0X002F302C,
0X0030312D,
0X0031322E,
0X0032332F,
0X00333430,
0X00343531,
0X00353632,
0X00373733,
0X00383834,
0X00393935,
0X003A3A36,
0X003B3B36,
0X003C3C37,
0X003D3D38,
0X003E3E39,
0X003F3F3A,
0X0040403B,
0X0041413C,
0X0042423D,
0X0043433E,
0X0044443F,
0X00454540,
0X00464641,
0X00474742,
0X00484843,
0X00494944,
0X004A4A45,
0X004B4C46,
0X004C4D47,
0X004D4E47,
0X004E4F48,
0X004F5049,
0X0050514A,
0X0051524B,
0X0052534C,
0X0053544D,
0X0054554E,
0X0055564F,
0X00565750,
0X00575851,
0X00595952,
0X005A5A53,
0X005B5C54,
0X005C5D55,
0X005D5E56,
0X005E5F57,
0X005F6058,
0X00606159,
0X0061625A,
0X0062635B,
0X0063645C,
0X0064655D,
0X0066675E,
0X00676860,
0X00686961,
0X00696A62,
0X006A6B63,
0X006B6C64,
0X006C6D65,
0X006D6E66,
0X006E7067,
0X00707168,
0X00717269,
0X0072736A,
0X0073746B,
0X0074756C,
0X0075766D,
0X0076776E,
0X0077796F,
0X00797A70,
0X007A7B71,
0X007B7C72,
0X007C7D73,
0X007D7E74,
0X007E7F75,
0X007F8076,
0X00808177,
0X00818378,
0X00828479,
0X0083857A,
0X0085867B,
0X0086877C,
0X0087887D,
0X0088897E,
0X00898A7F,
0X008A8B80,
0X008B8D81,
0X008C8E82,
0X008D8F83,
0X008E9085,
0X00909186,
0X00919287,
0X00929388,
0X00939489,
0X0094958A,
0X0095978B,
0X0096988C,
0X0097998D,
0X00989A8E,
0X00999B8F,
0X009B9C90,
0X009C9D91,
0X009D9E92,
0X009E9F93,
0X009FA094,
0X00A0A295,
0X00A1A396,
0X00A2A497,
0X00A3A598,
0X00A4A699,
0X00A5A79A,
0X00A6A89B,
0X00A7A99C,
0X00A9AA9D,
0X00AAAB9E,
0X00ABAC9F,
0X00ACADA0,
0X00ADAFA1,
0X00AEB0A2,
0X00AFB1A3,
0X00B0B2A4,
0X00B1B3A5,
0X00B2B4A6,
0X00B3B5A7,
0X00B4B6A8,
0X00B5B7A9,
0X00B6B8A9,
0X00B7B9AA,
0X00B8BAAB,
0X00B9BBAC,
0X00BABCAD,
0X00BBBDAE,
0X00BCBEAF,
0X00BDBFB0,
0X00BFC0B1,
0X00C0C1B2,
0X00C1C2B3,
0X00C2C4B4,
0X00C3C5B5,
0X00C4C6B6,
0X00C5C7B7,
0X00C6C8B8,
0X00C7C9B9,
0X00C8CABA,
0X00C9CBBB,
0X00CACCBC,
0X00CBCDBC,
0X00CCCEBD,
0X00CDCFBE,
0X00CED0BF,
0X00CFD1C0,
0X00D0D2C1,
0X00D1D3C2,
0X00D2D4C3,
0X00D3D5C4,
0X00D4D6C5,
0X00D5D7C6,
0X00D6D8C7,
0X00D6D9C7,
0X00D7DAC8,
0X00D8DBC9,
0X00D9DCCA,
0X00DADDCB,
0X00DBDECC,
0X00DCDECD,
0X00DDDFCE,
0X00DEE0CF,
0X00DFE1D0,
0X00E0E2D0,
0X00E1E3D1,
0X00E2E4D2,
0X00E3E5D3,
0X00E4E6D4,
0X00E4E7D4,
0X00E5E7D5,
0X00E6E8D6,
0X00E7E9D6,
0X00E7EAD7,
0X00E8EAD8,
0X00E9EBD9,
0X00EAECD9,
0X00EBEDDA,
0X00EBEEDB,
0X00ECEFDC,
0X00EDF0DD,
0X00EEF1DD,
0X00EFF1DE,
0X00F0F2DF,
0X00F1F3E0,
0X00F2F4E1,
0X00F3F5E2,
0X00F4F6E3,
0X00F5F8E4,
0X00F6F9E5,
0X00F7FAE6,
0X00F8FBE7,
0X00F9FCE8,
0X00FBFDE9,
0X00FCFEEA,
0X00FDFFEB,
0X00FEFFEC,
0X00FFFFED
#else
0X00000000,
0X00010101,
0X00020202,
0X00020202,
0X00030303,
0X00040404,
0X00050505,
0X00060606,
0X00060606,
0X00070707,
0X00080808,
0X00090909,
0X000A0A0A,
0X000A0A0A,
0X000B0B0B,
0X000C0C0C,
0X000D0D0D,
0X000E0E0E,
0X000F0F0F,
0X000F0F0F,
0X00101010,
0X00111111,
0X00121212,
0X00131313,
0X00141414,
0X00141414,
0X00151515,
0X00161616,
0X00171717,
0X00181818,
0X00191919,
0X00191919,
0X001A1A1A,
0X001B1B1B,
0X001C1C1C,
0X001D1D1D,
0X001E1E1E,
0X001F1F1F,
0X00202020,
0X00202020,
0X00212121,
0X00222222,
0X00232323,
0X00242424,
0X00252525,
0X00262626,
0X00272727,
0X00282828,
0X00292929,
0X002A2A2A,
0X002A2A2A,
0X002B2B2B,
0X002C2C2C,
0X002D2D2D,
0X002E2E2E,
0X002F2F2F,
0X00303030,
0X00313131,
0X00323232,
0X00333333,
0X00343434,
0X00353535,
0X00363636,
0X00373737,
0X00383838,
0X00393939,
0X003A3A3A,
0X003B3B3B,
0X003C3C3C,
0X003E3E3E,
0X003F3F3F,
0X00404040,
0X00414141,
0X00424242,
0X00434343,
0X00444444,
0X00464646,
0X00474747,
0X00484848,
0X00494949,
0X004A4A4A,
0X004B4B4B,
0X004D4D4D,
0X004E4E4E,
0X004F4F4F,
0X00505050,
0X00525252,
0X00535353,
0X00545454,
0X00565656,
0X00575757,
0X00585858,
0X00595959,
0X005B5B5B,
0X005C5C5C,
0X005D5D5D,
0X005E5E5E,
0X005F5F5F,
0X00616161,
0X00626262,
0X00636363,
0X00646464,
0X00656565,
0X00666666,
0X00686868,
0X00696969,
0X006A6A6A,
0X006B6B6B,
0X006C6C6C,
0X006D6D6D,
0X006E6E6E,
0X006F6F6F,
0X00707070,
0X00717171,
0X00727272,
0X00737373,
0X00747474,
0X00757575,
0X00767676,
0X00777777,
0X00787878,
0X00797979,
0X007A7A7A,
0X007B7B7B,
0X007C7C7C,
0X007D7D7D,
0X007E7E7E,
0X007F7F7F,
0X00808080,
0X00818181,
0X00838383,
0X00848484,
0X00868686,
0X00878787,
0X00888888,
0X008A8A8A,
0X008B8B8B,
0X008C8C8C,
0X008E8E8E,
0X008F8F8F,
0X00909090,
0X00929292,
0X00939393,
0X00949494,
0X00969696,
0X00979797,
0X00989898,
0X009A9A9A,
0X009B9B9B,
0X009D9D9D,
0X009E9E9E,
0X009F9F9F,
0X00A1A1A1,
0X00A2A2A2,
0X00A3A3A3,
0X00A4A4A4,
0X00A6A6A6,
0X00A7A7A7,
0X00A8A8A8,
0X00A9A9A9,
0X00ABABAB,
0X00ACACAC,
0X00ADADAD,
0X00AEAEAE,
0X00AFAFAF,
0X00B0B0B0,
0X00B1B1B1,
0X00B2B2B2,
0X00B3B3B3,
0X00B4B4B4,
0X00B5B5B5,
0X00B6B6B6,
0X00B7B7B7,
0X00B8B8B8,
0X00B9B9B9,
0X00BABABA,
0X00BBBBBB,
0X00BCBCBC,
0X00BDBDBD,
0X00BEBEBE,
0X00BFBFBF,
0X00C0C0C0,
0X00C1C1C1,
0X00C2C2C2,
0X00C3C3C3,
0X00C4C4C4,
0X00C5C5C5,
0X00C5C5C5,
0X00C6C6C6,
0X00C7C7C7,
0X00C8C8C8,
0X00C9C9C9,
0X00CACACA,
0X00CBCBCB,
0X00CCCCCC,
0X00CDCDCD,
0X00CFCFCF,
0X00D0D0D0,
0X00D1D1D1,
0X00D2D2D2,
0X00D3D3D3,
0X00D4D4D4,
0X00D5D5D5,
0X00D6D6D6,
0X00D7D7D7,
0X00D8D8D8,
0X00D9D9D9,
0X00DADADA,
0X00DBDBDB,
0X00DCDCDC,
0X00DDDDDD,
0X00DEDEDE,
0X00DFDFDF,
0X00E0E0E0,
0X00E1E1E1,
0X00E2E2E2,
0X00E3E3E3,
0X00E4E4E4,
0X00E5E5E5,
0X00E6E6E6,
0X00E7E7E7,
0X00E7E7E7,
0X00E8E8E8,
0X00E9E9E9,
0X00EAEAEA,
0X00EBEBEB,
0X00ECECEC,
0X00ECECEC,
0X00EDEDED,
0X00EEEEEE,
0X00EFEFEF,
0X00EFEFEF,
0X00F0F0F0,
0X00F1F1F1,
0X00F2F2F2,
0X00F2F2F2,
0X00F3F3F3,
0X00F4F4F4,
0X00F4F4F4,
0X00F5F5F5,
0X00F6F6F6,
0X00F6F6F6,
0X00F7F7F7,
0X00F8F8F8,
0X00F8F8F8,
0X00F9F9F9,
0X00FAFAFA,
0X00FAFAFA,
0X00FBFBFB,
0X00FBFBFB,
0X00FCFCFC,
0X00FDFDFD,
0X00FDFDFD,
0X00FEFEFE,
0X00FEFEFE,
0X00FFFFFF
#endif

};
/* LGE_CHANGE_E: jongyoung.koo@lge.com [2012-01-16] 
: To display white screen */


/* LGE_CHANGE_S: E0 jongyoung.koo@lge.com [2012-01-16]  
: For the calibration of LCD Color temperature */

static int mdp_lut_initial_hw_update(void )
{
	int i;
	uint32 r, g, b;
	uint32 cal_R, cal_G, cal_B;
	uint32 sign_0, sign_1, sign_2;
	uint32 gain_R,gain_G,gain_B;

	printk("### mdp_lut_hw_updae++: mdp_lut_i = %d\n", mdp_lut_i);

	cal_R = g_kcal[0]; cal_G = g_kcal[1]; cal_B = g_kcal[2];
	sign_0 = g_kcal[3]; sign_1 = g_kcal[4]; sign_2 = g_kcal[5];

	if( (sign_0 == 'c') && (sign_1 == 'a') && (sign_2 = 'l')){
		gain_R = (uint32)((cal_R * 100000)/255);
		gain_G = (uint32)((cal_G * 100000)/255);
		gain_B = (uint32)((cal_B * 100000)/255);

		printk("#### Sign is Matched %d,%d,%d Gain RGB : %d,%d,%d\n",sign_0,sign_1,sign_2,gain_R,gain_G,gain_B);
	}
	else{
		printk("#### Sign is not Matched %d,%d,%d\n",sign_0,sign_1,sign_2);
		gain_R = gain_G = gain_B = 100000;
	}

	for (i = 0; i < 256; i++) {

		r = lge_gc_lut[i];
		g = lge_gc_lut[i];
		b = lge_gc_lut[i];

		cal_R = (uint32)((((r & 0xff0000) >> 16)  * gain_R) /100000);
		cal_G = (uint32)((((g & 0xff00) >> 8)  * gain_G) /100000);
		cal_B = (uint32)(((b & 0xff) * gain_B) /100000);

#ifdef CONFIG_FB_MSM_MDP40
	MDP_OUTP(MDP_BASE + 0x94800 +
#else
	MDP_OUTP(MDP_BASE + 0x93800 +
#endif
		(0x400*mdp_lut_i) + i*4,
			((cal_G & 0xff) |
			 ((cal_B & 0xff) << 8) |
			 (cal_R << 16)));

	}

	printk("#### Cal value R=%d G=%d B=%d\n",cal_R,cal_G,cal_B);

	return 0;
}
#endif //CONFIG_FB_MSM_MDP_LUT_ENABLE

/* LGE_CHANGE_E: jongyoung.koo@lge.com [2012-01-16] 
: For the calibration of LCD Color temperature */
static int mdp_lut_hw_update(struct fb_cmap *cmap)
{
	int i;
	u16 *c[3];
	u16 r, g, b;

	c[0] = cmap->green;
	c[1] = cmap->blue;
	c[2] = cmap->red;

	for (i = 0; i < cmap->len; i++) {
		if (copy_from_user(&r, cmap->red++, sizeof(r)) ||
		    copy_from_user(&g, cmap->green++, sizeof(g)) ||
		    copy_from_user(&b, cmap->blue++, sizeof(b)))
			return -EFAULT;

#ifdef CONFIG_FB_MSM_MDP40
		MDP_OUTP(MDP_BASE + 0x94800 +
#else
		MDP_OUTP(MDP_BASE + 0x93800 +
#endif
			(0x400*mdp_lut_i) + cmap->start*4 + i*4,
				((g & 0xff) |
				 ((b & 0xff) << 8) |
				 ((r & 0xff) << 16)));
	}

	return 0;
}

static int mdp_lut_push;
static int mdp_lut_push_i;
static int mdp_lut_update_nonlcdc(struct fb_info *info, struct fb_cmap *cmap)
{
	int ret;

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	ret = mdp_lut_hw_update(cmap);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	if (ret)
		return ret;

	mutex_lock(&mdp_lut_push_sem);
	mdp_lut_push = 1;
	mdp_lut_push_i = mdp_lut_i;
	mutex_unlock(&mdp_lut_push_sem);

	mdp_lut_i = (mdp_lut_i + 1)%2;

	return 0;
}
/* LGE_CHANGE_S: jongyoung.koo@lge.com [2012-01-16]  
: For the calibration of LCD Color temperature */
#ifdef CONFIG_FB_MSM_MDP_LUT_ENABLE
static int mdp_lut_init_update_lcdc(void )
{
	int ret;

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	//ret = mdp_lut_hw_update(cmap);

  ret = mdp_lut_initial_hw_update();
	if (ret) {
		mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
		return ret;
	}

	MDP_OUTP(MDP_BASE + 0x90070, (mdp_lut_i << 10) | 0x17);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
	mdp_lut_i = (mdp_lut_i + 1)%2;

	return 0;
}

#endif //CONFIG_FB_MSM_MDP_LUT_ENABLE
/* LGE_CHANGE_E: jongyoung.koo@lge.com [2012-01-16]  
: For the calibration of LCD Color temperature */

static int mdp_lut_update_lcdc(struct fb_info *info, struct fb_cmap *cmap)
{
	int ret;
	uint32_t out;

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	ret = mdp_lut_hw_update(cmap);

	if (ret) {
		mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
		return ret;
	}

	/*mask off non LUT select bits*/
	out = inpdw(MDP_BASE + 0x90070) & ~((0x1 << 10) | 0x7);
	MDP_OUTP(MDP_BASE + 0x90070, (mdp_lut_i << 10) | 0x7 | out);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
	mdp_lut_i = (mdp_lut_i + 1)%2;

	return 0;
}

static void mdp_lut_enable(void)
{
	uint32_t out;
	if (mdp_lut_push) {
		mutex_lock(&mdp_lut_push_sem);
		mdp_lut_push = 0;
		out = inpdw(MDP_BASE + 0x90070) & ~((0x1 << 10) | 0x7);
		MDP_OUTP(MDP_BASE + 0x90070,
				(mdp_lut_push_i << 10) | 0x7 | out);
		mutex_unlock(&mdp_lut_push_sem);
	}
}

#define MDP_REV42_HIST_MAX_BIN 128
#define MDP_REV41_HIST_MAX_BIN 32

#ifdef CONFIG_FB_MSM_MDP40
unsigned int mdp_hist_frame_cnt;
#else
static unsigned int mdp_hist_frame_cnt;
#endif
struct completion mdp_hist_comp;
static DEFINE_MUTEX(mdp_hist_mutex);
static boolean mdp_is_hist_data = FALSE;
static boolean mdp_is_hist_start = FALSE;
boolean mdp_is_hist_valid = FALSE;
static boolean mdp_is_hist_init = FALSE;
static uint32 mdp_hist_r[128];
static uint32 mdp_hist_g[128];
static uint32 mdp_hist_b[128];

void __mdp_histogram_kickoff()
{
	char *mdp_hist_base;

	if (mdp_rev >= MDP_REV_40)
		mdp_hist_base = MDP_BASE + 0x95000;
	else if (mdp_rev >= MDP_REV_30 && mdp_rev <= MDP_REV_31)
		mdp_hist_base = MDP_BASE + 0x94000;
	else {
		pr_err("%s(): Unsupported MDP rev. %u\n", __func__, mdp_rev);
		return ;
	}

	if (mdp_is_hist_data == TRUE) {
		MDP_OUTP(mdp_hist_base + 0x004,	mdp_hist_frame_cnt);
		MDP_OUTP(mdp_hist_base, 1);
	}
}

void __mdp_histogram_reset()
{
	char *mdp_hist_base;

	if (mdp_rev >= MDP_REV_40)
		mdp_hist_base = MDP_BASE + 0x95000;
	else if (mdp_rev >= MDP_REV_30 && mdp_rev <= MDP_REV_31)
		mdp_hist_base = MDP_BASE + 0x94000;
	else {
		pr_err("%s(): Unsupported MDP rev %u\n", __func__, mdp_rev);
		return ;
	}

	MDP_OUTP(mdp_hist_base + 0x00C, 1);
}

static void mdp_hist_read_work(struct work_struct *data)
{
	char *mdp_hist_base;
	uint32 r_data_offset = 0x100, g_data_offset = 0x200;
	uint32 b_data_offset = 0x300;
	int num_bins, i = 0;

	if (mdp_rev >= MDP_REV_42) {
		mdp_hist_base = MDP_BASE + 0x95000;
		r_data_offset = 0x400;
		g_data_offset = 0x800;
		b_data_offset = 0xc00;
		num_bins = 128;
	} else if (mdp_rev >= MDP_REV_40 && mdp_rev <= MDP_REV_41) {
		mdp_hist_base = MDP_BASE + 0x95000;
		num_bins = 32;
	} else if (mdp_rev >= MDP_REV_30 && mdp_rev <= MDP_REV_31) {
		mdp_hist_base = MDP_BASE + 0x94000;
		num_bins = 32;
	} else {
		pr_err("%s(): Unsupported MDP rev %u\n", __func__, mdp_rev);
		return ;
	}

	mutex_lock(&mdp_hist_mutex);
	if (mdp_is_hist_data == FALSE) {
		pr_debug("%s, Histogram disabled before read.\n", __func__);
		goto error;
	}

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	for (i = 0; i < num_bins; i++) {
		mdp_hist_r[i] = inpdw(mdp_hist_base + r_data_offset + (4*i));
		mdp_hist_g[i] = inpdw(mdp_hist_base + g_data_offset + (4*i));
		mdp_hist_b[i] = inpdw(mdp_hist_base + b_data_offset + (4*i));
	}

	__mdp_histogram_kickoff();

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	/* if read was triggered by an underrun, don't wake up readers*/
	if (mdp_is_hist_valid && mdp_is_hist_init) {
		complete(&mdp_hist_comp);
	} else {
		if (mdp_is_hist_valid == FALSE)
			mdp_is_hist_valid = TRUE;

		if (mdp_is_hist_init == FALSE)
			mdp_is_hist_init = TRUE;
	}
error:
	mutex_unlock(&mdp_hist_mutex);
}

/*should hold mdp_hist_mutex before calling this function*/
int _mdp_histogram_ctrl(boolean en)
{
	unsigned long hist_base;
	uint32_t status;

	if (mdp_rev >= MDP_REV_40)
		hist_base = 0x95000;
	else
		hist_base = 0x94000;

	if (en == TRUE) {
		if (mdp_is_hist_data)
			return -EINVAL;

		mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
		mdp_hist_frame_cnt = 1;
		mdp_enable_irq(MDP_HISTOGRAM_TERM);
		INIT_COMPLETION(mdp_hist_comp);

		/*Clear the interrupts before enabling them*/
		MDP_OUTP(MDP_BASE + hist_base + 0x18, INTR_HIST_DONE |
						INTR_HIST_RESET_SEQ_DONE);
		MDP_OUTP(MDP_BASE + hist_base + 0x10, 1);
		MDP_OUTP(MDP_BASE + hist_base + 0x1c, INTR_HIST_DONE |
						INTR_HIST_RESET_SEQ_DONE);

		mdp_is_hist_data = TRUE;
		mdp_is_hist_valid = TRUE;
		mdp_is_hist_init = FALSE;

		__mdp_histogram_reset();
		mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	} else {
		if (!mdp_is_hist_data)
			return -EINVAL;

		mdp_is_hist_data = FALSE;
		mdp_is_hist_valid = FALSE;
		mdp_is_hist_init = FALSE;

		mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
		status = inpdw(MDP_BASE + hist_base + 0x1C);
		status &= ~(INTR_HIST_DONE | INTR_HIST_RESET_SEQ_DONE);
		MDP_OUTP(MDP_BASE + hist_base + 0x1C, status);
		MDP_OUTP(MDP_BASE + hist_base + 0x18, INTR_HIST_DONE |
						INTR_HIST_RESET_SEQ_DONE);
		mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

		complete(&mdp_hist_comp);

		mdp_disable_irq(MDP_HISTOGRAM_TERM);
	}

	return 0;
}

int mdp_histogram_ctrl(boolean en)
{
	int ret = 0;
	mutex_lock(&mdp_hist_mutex);
	if (mdp_is_hist_start)
		ret = _mdp_histogram_ctrl(en);
	mutex_unlock(&mdp_hist_mutex);

	if (en == false)
		flush_workqueue(mdp_hist_wq);

	return ret;
}

int mdp_start_histogram(struct fb_info *info)
{
	unsigned long flag;

	int ret = 0;
	mutex_lock(&mdp_hist_mutex);
	if (mdp_is_hist_start == TRUE) {
		printk(KERN_ERR "%s histogram already started\n", __func__);
		ret = -EPERM;
		goto mdp_hist_start_err;
	}

	ret = _mdp_histogram_ctrl(TRUE);

	spin_lock_irqsave(&mdp_spin_lock, flag);
	mdp_is_hist_start = TRUE;
	spin_unlock_irqrestore(&mdp_spin_lock, flag);

mdp_hist_start_err:
	mutex_unlock(&mdp_hist_mutex);
	return ret;

}

int mdp_stop_histogram(struct fb_info *info)
{
	unsigned long flag;
	int ret = 0;
	struct msm_fb_data_type *mfd = (struct msm_fb_data_type *)info->par;

	mutex_lock(&mdp_hist_mutex);
	if (!mdp_is_hist_start) {
		printk(KERN_ERR "%s histogram already stopped\n", __func__);
		ret = -EPERM;
		goto mdp_hist_stop_err;
	}

	spin_lock_irqsave(&mdp_spin_lock, flag);
	mdp_is_hist_start = FALSE;
	spin_unlock_irqrestore(&mdp_spin_lock, flag);

	if (!mfd->panel_power_on) {

		mdp_is_hist_data = FALSE;
		complete(&mdp_hist_comp);
		ret = -EINVAL;
		goto mdp_hist_stop_err;
	}

	ret = _mdp_histogram_ctrl(FALSE);

	mutex_unlock(&mdp_hist_mutex);
	flush_workqueue(mdp_hist_wq);
	return ret;

mdp_hist_stop_err:
	mutex_unlock(&mdp_hist_mutex);
	return ret;
}

/*call from within mdp_hist_mutex*/
static int _mdp_copy_hist_data(struct mdp_histogram *hist)
{
	int ret = 0;

	if (hist->r) {
		ret = copy_to_user(hist->r, mdp_hist_r, hist->bin_cnt * 4);
		if (ret)
			goto hist_err;
	}
	if (hist->g) {
		ret = copy_to_user(hist->g, mdp_hist_g, hist->bin_cnt * 4);
		if (ret)
			goto hist_err;
	}
	if (hist->b) {
		ret = copy_to_user(hist->b, mdp_hist_b, hist->bin_cnt * 4);
		if (ret)
			goto hist_err;
	}
	return 0;
hist_err:
	pr_err("%s: invalid hist buffer\n", __func__);
	return ret;
}

static int mdp_do_histogram(struct fb_info *info, struct mdp_histogram *hist)
{
	int ret = 0;

	if (!hist->frame_cnt || (hist->bin_cnt == 0))
		return -EINVAL;

	if ((mdp_rev <= MDP_REV_41 && hist->bin_cnt > MDP_REV41_HIST_MAX_BIN)
		|| (mdp_rev == MDP_REV_42 &&
		hist->bin_cnt > MDP_REV42_HIST_MAX_BIN))
		return -EINVAL;

	mutex_lock(&mdp_hist_mutex);
	if (!mdp_is_hist_data) {
		pr_err("%s - histogram not ready\n", __func__);
		ret = -EPERM;
		goto error;
	}

	if (!mdp_is_hist_start) {
		pr_err("%s histogram not started\n", __func__);
		ret = -EPERM;
		goto error;
	}

	mdp_hist_frame_cnt = hist->frame_cnt;
	mutex_unlock(&mdp_hist_mutex);

	if (wait_for_completion_killable(&mdp_hist_comp)) {
		pr_err("%s(): histogram bin collection killed", __func__);
		return -EINVAL;
	}

	mutex_lock(&mdp_hist_mutex);
	if (mdp_is_hist_data && mdp_is_hist_init)
		ret =  _mdp_copy_hist_data(hist);

error:
	mutex_unlock(&mdp_hist_mutex);
	return ret;
}
#endif

/* Returns < 0 on error, 0 on timeout, or > 0 on successful wait */

int mdp_ppp_pipe_wait(void)
{
	int ret = 1;

	/* wait 5 seconds for the operation to complete before declaring
	the MDP hung */

	if (mdp_ppp_waiting == TRUE) {
		ret = wait_for_completion_interruptible_timeout(&mdp_ppp_comp,
								5 * HZ);

		if (!ret)
			printk(KERN_ERR "%s: Timed out waiting for the MDP.\n",
					__func__);
	}

	return ret;
}

static DEFINE_SPINLOCK(mdp_lock);
static int mdp_irq_mask;
static int mdp_irq_enabled;

/*
 * mdp_enable_irq: can not be called from isr
 */
void mdp_enable_irq(uint32 term)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&mdp_lock, irq_flags);
	if (mdp_irq_mask & term) {
		printk(KERN_ERR "%s: MDP IRQ term-0x%x is already set, mask=%x irq=%d\n",
				__func__, term, mdp_irq_mask, mdp_irq_enabled);
	} else {
		mdp_irq_mask |= term;
		if (mdp_irq_mask && !mdp_irq_enabled) {
			mdp_irq_enabled = 1;
			enable_irq(mdp_irq);
		}
	}
	spin_unlock_irqrestore(&mdp_lock, irq_flags);
}

/*
 * mdp_disable_irq: can not be called from isr
 */
void mdp_disable_irq(uint32 term)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&mdp_lock, irq_flags);
	if (!(mdp_irq_mask & term)) {
		printk(KERN_ERR "%s: MDP IRQ term-0x%x is NOT set, mask=%x irq=%d\n",
				__func__, term, mdp_irq_mask, mdp_irq_enabled);
	} else {
		mdp_irq_mask &= ~term;
		if (!mdp_irq_mask && mdp_irq_enabled) {
			mdp_irq_enabled = 0;
			disable_irq(mdp_irq);
		}
	}
	spin_unlock_irqrestore(&mdp_lock, irq_flags);
}

void mdp_disable_irq_nosync(uint32 term)
{
	spin_lock(&mdp_lock);
	if (!(mdp_irq_mask & term)) {
		printk(KERN_ERR "%s: MDP IRQ term-0x%x is NOT set, mask=%x irq=%d\n",
				__func__, term, mdp_irq_mask, mdp_irq_enabled);
	} else {
		mdp_irq_mask &= ~term;
		if (!mdp_irq_mask && mdp_irq_enabled) {
			mdp_irq_enabled = 0;
			disable_irq_nosync(mdp_irq);
		}
	}
	spin_unlock(&mdp_lock);
}

void mdp_pipe_kickoff(uint32 term, struct msm_fb_data_type *mfd)
{
	/* complete all the writes before starting */
	wmb();

	/* kick off PPP engine */
	if (term == MDP_PPP_TERM) {
		if (mdp_debug[MDP_PPP_BLOCK])
			jiffies_to_timeval(jiffies, &mdp_ppp_timeval);

		/* let's turn on PPP block */
		mdp_pipe_ctrl(MDP_PPP_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

		mdp_enable_irq(term);
		INIT_COMPLETION(mdp_ppp_comp);
		mdp_ppp_waiting = TRUE;
		outpdw(MDP_BASE + 0x30, 0x1000);
		wait_for_completion_killable(&mdp_ppp_comp);
		mdp_disable_irq(term);

		if (mdp_debug[MDP_PPP_BLOCK]) {
			struct timeval now;

			jiffies_to_timeval(jiffies, &now);
			mdp_ppp_timeval.tv_usec =
			    now.tv_usec - mdp_ppp_timeval.tv_usec;
			MSM_FB_DEBUG("MDP-PPP: %d\n",
				    (int)mdp_ppp_timeval.tv_usec);
		}
	} else if (term == MDP_DMA2_TERM) {
		if (mdp_debug[MDP_DMA2_BLOCK]) {
			MSM_FB_DEBUG("MDP-DMA2: %d\n",
				    (int)mdp_dma2_timeval.tv_usec);
			jiffies_to_timeval(jiffies, &mdp_dma2_timeval);
		}
		/* DMA update timestamp */
		mdp_dma2_last_update_time = ktime_get_real();
		/* let's turn on DMA2 block */
#if 0
		mdp_pipe_ctrl(MDP_DMA2_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
#endif
#ifdef CONFIG_FB_MSM_MDP22
		outpdw(MDP_CMD_DEBUG_ACCESS_BASE + 0x0044, 0x0);/* start DMA */
#else
		mdp_lut_enable();

#ifdef CONFIG_FB_MSM_MDP40
		outpdw(MDP_BASE + 0x000c, 0x0);	/* start DMA */
#else
		outpdw(MDP_BASE + 0x0044, 0x0);	/* start DMA */

#ifdef CONFIG_FB_MSM_MDP303

#ifdef CONFIG_FB_MSM_MIPI_DSI
		mipi_dsi_cmd_mdp_start();
#endif

#endif

#endif
#endif
#ifdef CONFIG_FB_MSM_MDP40
	} else if (term == MDP_DMA_S_TERM) {
		mdp_pipe_ctrl(MDP_DMA_S_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
		outpdw(MDP_BASE + 0x0010, 0x0);	/* start DMA */
	} else if (term == MDP_DMA_E_TERM) {
		mdp_pipe_ctrl(MDP_DMA_E_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
		outpdw(MDP_BASE + 0x0014, 0x0);	/* start DMA */
	} else if (term == MDP_OVERLAY0_TERM) {
		mdp_pipe_ctrl(MDP_OVERLAY0_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
		mdp_lut_enable();
		outpdw(MDP_BASE + 0x0004, 0);
	} else if (term == MDP_OVERLAY1_TERM) {
		mdp_pipe_ctrl(MDP_OVERLAY1_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
		mdp_lut_enable();
		outpdw(MDP_BASE + 0x0008, 0);
	} else if (term == MDP_OVERLAY2_TERM) {
		mdp_pipe_ctrl(MDP_OVERLAY2_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
		mdp_lut_enable();
		outpdw(MDP_BASE + 0x00D0, 0);
	}
#else
	} else if (term == MDP_DMA_S_TERM) {
		mdp_pipe_ctrl(MDP_DMA_S_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
		outpdw(MDP_BASE + 0x0048, 0x0);	/* start DMA */
	} else if (term == MDP_DMA_E_TERM) {
		mdp_pipe_ctrl(MDP_DMA_E_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
		outpdw(MDP_BASE + 0x004C, 0x0);
	}
#endif
}

static int mdp_clk_rate;
static struct platform_device *pdev_list[MSM_FB_MAX_DEV_LIST];
static int pdev_list_cnt;

static void mdp_pipe_ctrl_workqueue_handler(struct work_struct *work)
{
	mdp_pipe_ctrl(MDP_MASTER_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
}
void mdp_pipe_ctrl(MDP_BLOCK_TYPE block, MDP_BLOCK_POWER_STATE state,
		   boolean isr)
{
	boolean mdp_all_blocks_off = TRUE;
	int i;
	unsigned long flag;
	struct msm_fb_panel_data *pdata;

	/*
	 * It is assumed that if isr = TRUE then start = OFF
	 * if start = ON when isr = TRUE it could happen that the usercontext
	 * could turn off the clocks while the interrupt is updating the
	 * power to ON
	 */
	WARN_ON(isr == TRUE && state == MDP_BLOCK_POWER_ON);

	spin_lock_irqsave(&mdp_spin_lock, flag);
	if (MDP_BLOCK_POWER_ON == state) {
		atomic_inc(&mdp_block_power_cnt[block]);

		if (MDP_DMA2_BLOCK == block)
			mdp_in_processing = TRUE;
	} else {
		atomic_dec(&mdp_block_power_cnt[block]);

		if (atomic_read(&mdp_block_power_cnt[block]) < 0) {
			/*
			* Master has to serve a request to power off MDP always
			* It also has a timer to power off.  So, in case of
			* timer expires first and DMA2 finishes later,
			* master has to power off two times
			* There shouldn't be multiple power-off request for
			* other blocks
			*/
			if (block != MDP_MASTER_BLOCK) {
				MSM_FB_INFO("mdp_block_power_cnt[block=%d] \
				multiple power-off request\n", block);
			}
			atomic_set(&mdp_block_power_cnt[block], 0);
		}

		if (MDP_DMA2_BLOCK == block)
			mdp_in_processing = FALSE;
	}
	spin_unlock_irqrestore(&mdp_spin_lock, flag);

	/*
	 * If it's in isr, we send our request to workqueue.
	 * Otherwise, processing happens in the current context
	 */
	if (isr) {
		if (mdp_current_clk_on) {
			/* checking all blocks power state */
			for (i = 0; i < MDP_MAX_BLOCK; i++) {
				if (atomic_read(&mdp_block_power_cnt[i]) > 0) {
					mdp_all_blocks_off = FALSE;
					break;
				}
			}

			if (mdp_all_blocks_off) {
				/* send workqueue to turn off mdp power */
				queue_delayed_work(mdp_pipe_ctrl_wq,
						   &mdp_pipe_ctrl_worker,
						   mdp_timer_duration);
			}
		}
	} else {
		down(&mdp_pipe_ctrl_mutex);
		/* checking all blocks power state */
		for (i = 0; i < MDP_MAX_BLOCK; i++) {
			if (atomic_read(&mdp_block_power_cnt[i]) > 0) {
				mdp_all_blocks_off = FALSE;
				break;
			}
		}

		/*
		 * find out whether a delayable work item is currently
		 * pending
		 */

		if (delayed_work_pending(&mdp_pipe_ctrl_worker)) {
			/*
			 * try to cancel the current work if it fails to
			 * stop (which means del_timer can't delete it
			 * from the list, it's about to expire and run),
			 * we have to let it run. queue_delayed_work won't
			 * accept the next job which is same as
			 * queue_delayed_work(mdp_timer_duration = 0)
			 */
			cancel_delayed_work(&mdp_pipe_ctrl_worker);
		}

		if ((mdp_all_blocks_off) && (mdp_current_clk_on)) {
			mutex_lock(&mdp_suspend_mutex);
			if (block == MDP_MASTER_BLOCK || mdp_suspended) {
				mdp_current_clk_on = FALSE;
				mb();
				/* turn off MDP clks */
				mdp_vsync_clk_disable();
				for (i = 0; i < pdev_list_cnt; i++) {
					pdata = (struct msm_fb_panel_data *)
						pdev_list[i]->dev.platform_data;
					if (pdata && pdata->clk_func)
						pdata->clk_func(0);
				}
				if (mdp_clk != NULL) {
					mdp_clk_rate = clk_get_rate(mdp_clk);
					clk_disable(mdp_clk);
					if (mdp_hw_revision <=
						MDP4_REVISION_V2_1 &&
						mdp_clk_rate > 122880000) {
						clk_set_rate(mdp_clk,
							 122880000);
					}
					MSM_FB_DEBUG("MDP CLK OFF\n");
				}
				if (mdp_pclk != NULL) {
					clk_disable(mdp_pclk);
					MSM_FB_DEBUG("MDP PCLK OFF\n");
				}
				if (mdp_lut_clk != NULL)
					clk_disable(mdp_lut_clk);
			} else {
				/* send workqueue to turn off mdp power */
				queue_delayed_work(mdp_pipe_ctrl_wq,
						   &mdp_pipe_ctrl_worker,
						   mdp_timer_duration);
			}
			mutex_unlock(&mdp_suspend_mutex);
		} else if ((!mdp_all_blocks_off) && (!mdp_current_clk_on)) {
			mdp_current_clk_on = TRUE;
			/* turn on MDP clks */
			for (i = 0; i < pdev_list_cnt; i++) {
				pdata = (struct msm_fb_panel_data *)
					pdev_list[i]->dev.platform_data;
				if (pdata && pdata->clk_func)
					pdata->clk_func(1);
			}
			if (mdp_clk != NULL) {
				if (mdp_hw_revision <=
					MDP4_REVISION_V2_1 &&
					mdp_clk_rate > 122880000) {
					clk_set_rate(mdp_clk,
						 mdp_clk_rate);
				}
				clk_enable(mdp_clk);
				MSM_FB_DEBUG("MDP CLK ON\n");
			}
			if (mdp_pclk != NULL) {
				clk_enable(mdp_pclk);
				MSM_FB_DEBUG("MDP PCLK ON\n");
			}
			if (mdp_lut_clk != NULL)
				clk_enable(mdp_lut_clk);
			mdp_vsync_clk_enable();
		}
		up(&mdp_pipe_ctrl_mutex);
	}
}

#ifndef CONFIG_FB_MSM_MDP40
irqreturn_t mdp_isr(int irq, void *ptr)
{
	uint32 hist_interrupt, mdp_interrupt = 0;
	struct mdp_dma_data *dma;

	mdp_is_in_isr = TRUE;
	do {
		mdp_interrupt = inp32(MDP_INTR_STATUS);
		outp32(MDP_INTR_CLEAR, mdp_interrupt);

		mdp_interrupt &= mdp_intr_mask;

		if (mdp_interrupt & TV_ENC_UNDERRUN) {
			mdp_interrupt &= ~(TV_ENC_UNDERRUN);
			mdp_tv_underflow_cnt++;
		}

		if (!mdp_interrupt)
			break;

		/* DMA3 TV-Out Start */
		if (mdp_interrupt & TV_OUT_DMA3_START) {
			/* let's disable TV out interrupt */
			mdp_intr_mask &= ~TV_OUT_DMA3_START;
			outp32(MDP_INTR_ENABLE, mdp_intr_mask);

			dma = &dma3_data;
			if (dma->waiting) {
				dma->waiting = FALSE;
				complete(&dma->comp);
			}
		}
#ifndef CONFIG_FB_MSM_MDP22
		if (mdp_interrupt & MDP_HIST_DONE) {
			hist_interrupt = inp32(MDP_DMA_P_HIST_INTR_STATUS);
			outp32(MDP_BASE + 0x94018, 0x3);
			outp32(MDP_INTR_CLEAR, MDP_HIST_DONE);
			if (hist_interrupt & INTR_HIST_RESET_SEQ_DONE)
				__mdp_histogram_kickoff();

			if (hist_interrupt & INTR_HIST_DONE) {
				if (waitqueue_active(&mdp_hist_comp.wait)) {
					if (!queue_work(mdp_hist_wq,
						&mdp_histogram_worker)) {
						pr_err(
						"%s: can't queue hist_read\n",
								__func__);
					}
				} else
					__mdp_histogram_reset();
			}
		}

		/* LCDC UnderFlow */
		if (mdp_interrupt & LCDC_UNDERFLOW) {
			mdp_lcdc_underflow_cnt++;
			/*when underflow happens HW resets all the histogram
			  registers that were set before so restore them back
			  to normal.*/
			MDP_OUTP(MDP_BASE + 0x94010, 1);
			MDP_OUTP(MDP_BASE + 0x9401c, INTR_HIST_DONE);
			mdp_is_hist_valid = FALSE;
			__mdp_histogram_reset();
		}

		/* LCDC Frame Start */
		if (mdp_interrupt & LCDC_FRAME_START) {
			/* let's disable LCDC interrupt */
			mdp_intr_mask &= ~LCDC_FRAME_START;
			outp32(MDP_INTR_ENABLE, mdp_intr_mask);

			dma = &dma2_data;
			if (dma->waiting) {
				dma->waiting = FALSE;
				complete(&dma->comp);
			}
		}

		/* DMA2 LCD-Out Complete */
		if (mdp_interrupt & MDP_DMA_S_DONE) {
			dma = &dma_s_data;
			dma->busy = FALSE;
			mdp_pipe_ctrl(MDP_DMA_S_BLOCK, MDP_BLOCK_POWER_OFF,
				      TRUE);
			complete(&dma->comp);
		}
		/* DMA_E LCD-Out Complete */
		if (mdp_interrupt & MDP_DMA_E_DONE) {
			dma = &dma_s_data;
			dma->busy = FALSE;
			mdp_pipe_ctrl(MDP_DMA_E_BLOCK, MDP_BLOCK_POWER_OFF,
				TRUE);
			complete(&dma->comp);
		}

#endif

		/* DMA2 LCD-Out Complete */
		if (mdp_interrupt & MDP_DMA_P_DONE) {
			struct timeval now;

			mdp_dma2_last_update_time = ktime_sub(ktime_get_real(),
				mdp_dma2_last_update_time);
			if (mdp_debug[MDP_DMA2_BLOCK]) {
				jiffies_to_timeval(jiffies, &now);
				mdp_dma2_timeval.tv_usec =
				    now.tv_usec - mdp_dma2_timeval.tv_usec;
			}
#ifndef CONFIG_FB_MSM_MDP303
			dma = &dma2_data;
			dma->busy = FALSE;
			mdp_pipe_ctrl(MDP_DMA2_BLOCK, MDP_BLOCK_POWER_OFF,
				      TRUE);
			complete(&dma->comp);
#else
			if (mdp_prim_panel_type == MIPI_CMD_PANEL) {
				dma = &dma2_data;
				dma->busy = FALSE;
				mdp_pipe_ctrl(MDP_DMA2_BLOCK,
					MDP_BLOCK_POWER_OFF, TRUE);
				complete(&dma->comp);
			}
#endif
		}
		/* PPP Complete */
		if (mdp_interrupt & MDP_PPP_DONE) {
#ifdef	CONFIG_FB_MSM_MDP31
			MDP_OUTP(MDP_BASE + 0x00100, 0xFFFF);
#endif
			mdp_pipe_ctrl(MDP_PPP_BLOCK, MDP_BLOCK_POWER_OFF, TRUE);
			if (mdp_ppp_waiting) {
				mdp_ppp_waiting = FALSE;
				complete(&mdp_ppp_comp);
			}
		}
	} while (1);

	mdp_is_in_isr = FALSE;

	return IRQ_HANDLED;
}
#endif

static void mdp_drv_init(void)
{
	int i;

	for (i = 0; i < MDP_MAX_BLOCK; i++) {
		mdp_debug[i] = 0;
	}

	/* initialize spin lock and workqueue */
	spin_lock_init(&mdp_spin_lock);
	mdp_dma_wq = create_singlethread_workqueue("mdp_dma_wq");
	mdp_vsync_wq = create_singlethread_workqueue("mdp_vsync_wq");
	mdp_hist_wq = create_singlethread_workqueue("mdp_hist_wq");
	INIT_WORK(&mdp_histogram_worker, mdp_hist_read_work);
	mdp_pipe_ctrl_wq = create_singlethread_workqueue("mdp_pipe_ctrl_wq");
	INIT_DELAYED_WORK(&mdp_pipe_ctrl_worker,
			  mdp_pipe_ctrl_workqueue_handler);

	/* initialize semaphore */
	init_completion(&mdp_ppp_comp);
	sema_init(&mdp_ppp_mutex, 1);
	sema_init(&mdp_pipe_ctrl_mutex, 1);

	dma2_data.busy = FALSE;
	dma2_data.dmap_busy = FALSE;
	dma2_data.waiting = FALSE;
	init_completion(&dma2_data.comp);
	init_completion(&dma2_data.dmap_comp);
	sema_init(&dma2_data.mutex, 1);
	mutex_init(&dma2_data.ov_mutex);

	dma3_data.busy = FALSE;
	dma3_data.waiting = FALSE;
	init_completion(&dma3_data.comp);
	sema_init(&dma3_data.mutex, 1);

	dma_s_data.busy = FALSE;
	dma_s_data.waiting = FALSE;
	init_completion(&dma_s_data.comp);
	sema_init(&dma_s_data.mutex, 1);

#ifndef CONFIG_FB_MSM_MDP303
	dma_e_data.busy = FALSE;
	dma_e_data.waiting = FALSE;
	init_completion(&dma_e_data.comp);
	mutex_init(&dma_e_data.ov_mutex);
#endif
#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
	dma_wb_data.busy = FALSE;
	dma_wb_data.waiting = FALSE;
	init_completion(&dma_wb_data.comp);
	mutex_init(&dma_wb_data.ov_mutex);
#endif

#ifndef CONFIG_FB_MSM_MDP22
	init_completion(&mdp_hist_comp);
#endif

	/* initializing mdp power block counter to 0 */
	for (i = 0; i < MDP_MAX_BLOCK; i++) {
		atomic_set(&mdp_block_power_cnt[i], 0);
	}

#ifdef MSM_FB_ENABLE_DBGFS
	{
		struct dentry *root;
		char sub_name[] = "mdp";

		root = msm_fb_get_debugfs_root();
		if (root != NULL) {
			mdp_dir = debugfs_create_dir(sub_name, root);

			if (mdp_dir) {
				msm_fb_debugfs_file_create(mdp_dir,
					"dma2_update_time_in_usec",
					(u32 *) &mdp_dma2_update_time_in_usec);
				msm_fb_debugfs_file_create(mdp_dir,
					"vs_rdcnt_slow",
					(u32 *) &mdp_lcd_rd_cnt_offset_slow);
				msm_fb_debugfs_file_create(mdp_dir,
					"vs_rdcnt_fast",
					(u32 *) &mdp_lcd_rd_cnt_offset_fast);
				msm_fb_debugfs_file_create(mdp_dir,
					"mdp_usec_diff_threshold",
					(u32 *) &mdp_usec_diff_threshold);
				msm_fb_debugfs_file_create(mdp_dir,
					"mdp_current_clk_on",
					(u32 *) &mdp_current_clk_on);
#ifdef CONFIG_FB_MSM_LCDC
				msm_fb_debugfs_file_create(mdp_dir,
					"lcdc_start_x",
					(u32 *) &first_pixel_start_x);
				msm_fb_debugfs_file_create(mdp_dir,
					"lcdc_start_y",
					(u32 *) &first_pixel_start_y);
#endif
			}
		}
	}
#endif
}

static int mdp_probe(struct platform_device *pdev);
static int mdp_remove(struct platform_device *pdev);

static int mdp_runtime_suspend(struct device *dev)
{
	dev_dbg(dev, "pm_runtime: suspending...\n");
	return 0;
}

static int mdp_runtime_resume(struct device *dev)
{
	dev_dbg(dev, "pm_runtime: resuming...\n");
	return 0;
}

static struct dev_pm_ops mdp_dev_pm_ops = {
	.runtime_suspend = mdp_runtime_suspend,
	.runtime_resume = mdp_runtime_resume,
};


static struct platform_driver mdp_driver = {
	.probe = mdp_probe,
	.remove = mdp_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = mdp_suspend,
	.resume = NULL,
#endif
	.shutdown = NULL,
	.driver = {
		/*
		 * Driver name must match the device name added in
		 * platform.c.
		 */
		.name = "mdp",
		.pm = &mdp_dev_pm_ops,
	},
};

static int mdp_off(struct platform_device *pdev)
{
	int ret = 0;
	struct msm_fb_data_type *mfd = platform_get_drvdata(pdev);

	mdp_histogram_ctrl(FALSE);

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	ret = panel_next_off(pdev);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	if (mdp_rev >= MDP_REV_41 && mfd->panel.type == MIPI_CMD_PANEL)
		mdp_dsi_cmd_overlay_suspend();
	return ret;
}

static int mdp_on(struct platform_device *pdev)
{
	int ret = 0;

#ifdef CONFIG_FB_MSM_MDP40
	struct msm_fb_data_type *mfd;
	mdp4_overlay_ctrl_db_reset();

	mfd = platform_get_drvdata(pdev);

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	if (is_mdp4_hw_reset()) {
		mdp_vsync_cfg_regs(mfd, FALSE);
		mdp4_hw_init();
		outpdw(MDP_BASE + 0x0038, mdp4_display_intf);
	}
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
#endif

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	ret = panel_next_on(pdev);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	mdp_histogram_ctrl(TRUE);
#ifdef CONFIG_FB_MSM_MDP40
	if (mfd->panel.type == MIPI_CMD_PANEL)
		mdp4_dsi_cmd_overlay_restore();
	else if (mfd->panel.type == MDDI_PANEL)
		mdp4_mddi_overlay_restore();
#endif

	return ret;
}

static int mdp_resource_initialized;
static struct msm_panel_common_pdata *mdp_pdata;

uint32 mdp_hw_revision;

/*
 * mdp_hw_revision:
 * 0 == V1
 * 1 == V2
 * 2 == V2.1
 *
 */
void mdp_hw_version(void)
{
	char *cp;
	uint32 *hp;

	if (mdp_pdata == NULL)
		return;

	mdp_hw_revision = MDP4_REVISION_NONE;
	if (mdp_pdata->hw_revision_addr == 0)
		return;

	/* tlmmgpio2 shadow */
	cp = (char *)ioremap(mdp_pdata->hw_revision_addr, 0x16);

	if (cp == NULL)
		return;

	hp = (uint32 *)cp;	/* HW_REVISION_NUMBER */
	mdp_hw_revision = *hp;
	iounmap(cp);

	mdp_hw_revision >>= 28;	/* bit 31:28 */
	mdp_hw_revision &= 0x0f;

	MSM_FB_DEBUG("%s: mdp_hw_revision=%x\n",
				__func__, mdp_hw_revision);
}

#ifdef CONFIG_FB_MSM_MDP40
static void configure_mdp_core_clk_table(uint32 min_clk_rate)
{
	uint8 count;
	uint32 current_rate;
	if (mdp_clk && mdp_pdata && mdp_pdata->mdp_core_clk_table) {
		min_clk_rate = clk_round_rate(mdp_clk, min_clk_rate);
		if (clk_set_rate(mdp_clk, min_clk_rate) < 0)
			printk(KERN_ERR "%s: clk_set_rate failed\n",
							 __func__);
		else {
			count = 0;
			current_rate = clk_get_rate(mdp_clk);
			while (count < mdp_pdata->num_mdp_clk) {
				if (mdp_pdata->mdp_core_clk_table[count]
						< current_rate) {
					mdp_pdata->
					mdp_core_clk_table[count] =
							current_rate;
				}
				count++;
			}
		}
	}
}
#endif

#ifdef CONFIG_MSM_BUS_SCALING
static uint32_t mdp_bus_scale_handle;
int mdp_bus_scale_update_request(uint32_t index)
{
	if (!mdp_pdata && (!mdp_pdata->mdp_bus_scale_table
	     || index > (mdp_pdata->mdp_bus_scale_table->num_usecases - 1))) {
		printk(KERN_ERR "%s invalid table or index\n", __func__);
		return -EINVAL;
	}
	if (mdp_bus_scale_handle < 1) {
		printk(KERN_ERR "%s invalid bus handle\n", __func__);
		return -EINVAL;
	}
	return msm_bus_scale_client_update_request(mdp_bus_scale_handle,
							index);
}
#endif
DEFINE_MUTEX(mdp_clk_lock);
int mdp_set_core_clk(uint16 perf_level)
{
	int ret = -EINVAL;
	if (mdp_clk && mdp_pdata
		 && mdp_pdata->mdp_core_clk_table) {
		if (perf_level > mdp_pdata->num_mdp_clk)
			printk(KERN_ERR "%s invalid perf level\n", __func__);
		else {
			mutex_lock(&mdp_clk_lock);
			ret = clk_set_rate(mdp_clk,
				mdp_pdata->
				mdp_core_clk_table[mdp_pdata->num_mdp_clk
						 - perf_level]);
			mutex_unlock(&mdp_clk_lock);
			if (ret) {
				printk(KERN_ERR "%s unable to set mdp_core_clk rate\n",
					__func__);
			}
		}
	}
	return ret;
}

unsigned long mdp_get_core_clk(void)
{
	unsigned long clk_rate = 0;
	if (mdp_clk) {
		mutex_lock(&mdp_clk_lock);
		clk_rate = clk_get_rate(mdp_clk);
		mutex_unlock(&mdp_clk_lock);
	}

	return clk_rate;
}

unsigned long mdp_perf_level2clk_rate(uint32 perf_level)
{
	unsigned long clk_rate = 0;

	if (mdp_pdata && mdp_pdata->mdp_core_clk_table) {
		if (perf_level > mdp_pdata->num_mdp_clk) {
			printk(KERN_ERR "%s invalid perf level\n", __func__);
			clk_rate = mdp_get_core_clk();
		} else {
			clk_rate = mdp_pdata->
				mdp_core_clk_table[mdp_pdata->num_mdp_clk
					- perf_level];
		}
	} else
		clk_rate = mdp_get_core_clk();

	return clk_rate;
}

static int mdp_irq_clk_setup(char cont_splashScreen)
{
	int ret;

#ifdef CONFIG_FB_MSM_MDP40
	ret = request_irq(mdp_irq, mdp4_isr, IRQF_DISABLED, "MDP", 0);
#else
	ret = request_irq(mdp_irq, mdp_isr, IRQF_DISABLED, "MDP", 0);
#endif
	if (ret) {
		printk(KERN_ERR "mdp request_irq() failed!\n");
		return ret;
	}
	disable_irq(mdp_irq);

	footswitch = regulator_get(NULL, "fs_mdp");
	if (IS_ERR(footswitch))
		footswitch = NULL;
	else {
		regulator_enable(footswitch);
		mdp_footswitch_on = 1;
	}

	mdp_clk = clk_get(NULL, "mdp_clk");
	if (IS_ERR(mdp_clk)) {
		ret = PTR_ERR(mdp_clk);
		printk(KERN_ERR "can't get mdp_clk error:%d!\n", ret);
		free_irq(mdp_irq, 0);
		return ret;
	}

	mdp_pclk = clk_get(NULL, "mdp_pclk");
	if (IS_ERR(mdp_pclk))
		mdp_pclk = NULL;

	if (mdp_rev == MDP_REV_42) {
		mdp_lut_clk = clk_get(NULL, "lut_mdp");
		if (IS_ERR(mdp_lut_clk)) {
			ret = PTR_ERR(mdp_lut_clk);
			pr_err("can't get mdp_clk error:%d!\n", ret);
			clk_put(mdp_clk);
			free_irq(mdp_irq, 0);
			return ret;
		}
	} else {
		mdp_lut_clk = NULL;
	}

#ifdef CONFIG_FB_MSM_MDP40
	/*
	 * mdp_clk should greater than mdp_pclk always
	 */
	if (mdp_pdata && mdp_pdata->mdp_core_clk_rate) {
		if (cont_splashScreen)
			mdp_clk_rate = clk_get_rate(mdp_clk);
		else
			mdp_clk_rate = mdp_pdata->mdp_core_clk_rate;

		mutex_lock(&mdp_clk_lock);
		clk_set_rate(mdp_clk, mdp_clk_rate);
		if (mdp_lut_clk != NULL)
			clk_set_rate(mdp_lut_clk, mdp_clk_rate);
		mutex_unlock(&mdp_clk_lock);
	}

	MSM_FB_DEBUG("mdp_clk: mdp_clk=%d\n", (int)clk_get_rate(mdp_clk));
#endif
	return 0;
}
/* LGE_CHANGE_S: jongyoung.koo@lge.com [2012-01-16]  
: For the calibration of LCD Color temperature */
#ifdef CONFIG_FB_MSM_MDP_LUT_ENABLE //sohyun.nam
int mdp_write_kcal_reg(const char* buf)
{
	int iRed, iGreen, iBlue;
	uint32 cal_R, cal_G, cal_B;
	uint32 gain_R, gain_G, gain_B;
	uint32 r, g, b;
	int i;
	sscanf(buf, "%d,%d,%d", &iRed, &iGreen, &iBlue);

	printk(KERN_ERR "================mdp_write_kcal_reg function starts ~~~~~~!\n");
	printk(KERN_ERR "================Param 1 : %d, Param 2 : %d, Param 3: %d ~~~~~~!\n", iRed, iGreen, iBlue );

#if 1

	cal_R = (unsigned int)iRed;
	cal_G = (unsigned int)iGreen;
	cal_B = (unsigned int)iBlue;


	gain_R = (uint32)((cal_R * 100000)/255);
	gain_G = (uint32)((cal_G * 100000)/255);
	gain_B = (uint32)((cal_B * 100000)/255);

	printk("#########mdp_write_kcal color : R=%d, G=%d, B=%d AND gain : R=%d, G=%d, B=%d\n###########",cal_R,cal_G,cal_B,gain_R,gain_G,gain_B);
	printk("#### mdp_lut_init_update_lcdc++: mdp_lut_i = %d\n", mdp_lut_i);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	for (i = 0; i < 256; i++) {
		r = lge_gc_lut[i];
		g = lge_gc_lut[i];
		b = lge_gc_lut[i];

		cal_R = (uint32)((((r & 0xff0000) >> 16)  * gain_R) /100000);
		cal_G = (uint32)((((g & 0xff00) >> 8)  * gain_G) /100000);
		cal_B = (uint32)(((b & 0xff) * gain_B) /100000);
		
#ifdef CONFIG_FB_MSM_MDP40
		MDP_OUTP(MDP_BASE + 0x94800 +
#else
		MDP_OUTP(MDP_BASE + 0x93800 +
#endif
			(0x400*mdp_lut_i) + i*4,
				((cal_G & 0xff) |
				 ((cal_B & 0xff) << 8) |
				 (cal_R << 16)));
	}

	MDP_OUTP(MDP_BASE + 0x90070, (mdp_lut_i << 10) | 0x17);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	mdp_lut_i = (mdp_lut_i + 1)%2;
#endif

	return 1;
}

EXPORT_SYMBOL(mdp_write_kcal_reg);
#endif //CONFIG_FB_MSM_MDP_LUT_ENABLE

/* LGE_CHANGE_E: jongyoung.koo@lge.com [2012-01-16]  
: For the calibration of LCD Color temperature */

static int mdp_probe(struct platform_device *pdev)
{
	struct platform_device *msm_fb_dev = NULL;
	struct msm_fb_data_type *mfd;
	struct msm_fb_panel_data *pdata = NULL;
	int rc;
	resource_size_t  size ;
#ifdef CONFIG_FB_MSM_MDP40
	int intf, if_no;
#else
	unsigned long flag;
#endif
#if defined(CONFIG_FB_MSM_MIPI_DSI) && defined(CONFIG_FB_MSM_MDP40)
	struct mipi_panel_info *mipi;
#endif
	static int contSplash_update_done;

	if ((pdev->id == 0) && (pdev->num_resources > 0)) {

		mdp_pdata = pdev->dev.platform_data;

		size =  resource_size(&pdev->resource[0]);
		msm_mdp_base = ioremap(pdev->resource[0].start, size);

		MSM_FB_DEBUG("MDP HW Base phy_Address = 0x%x virt = 0x%x\n",
			(int)pdev->resource[0].start, (int)msm_mdp_base);

		if (unlikely(!msm_mdp_base))
			return -ENOMEM;

		mdp_irq = platform_get_irq(pdev, 0);
		if (mdp_irq < 0) {
			pr_err("mdp: can not get mdp irq\n");
			return -ENOMEM;
		}

		mdp_rev = mdp_pdata->mdp_rev;

		rc = mdp_irq_clk_setup(mdp_pdata->cont_splash_enabled);

		if (rc)
			return rc;

		mdp_hw_version();

		/* initializing mdp hw */
#ifdef CONFIG_FB_MSM_MDP40
		if (!(mdp_pdata->cont_splash_enabled))
			mdp4_hw_init();
#else
		mdp_hw_init();
#endif

#ifdef CONFIG_FB_MSM_OVERLAY
		mdp_hw_cursor_init();
#endif
		mdp_resource_initialized = 1;
		return 0;
	}

	if (!mdp_resource_initialized)
		return -EPERM;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;

	if (mfd->key != MFD_KEY)
		return -EINVAL;

	if (pdev_list_cnt >= MSM_FB_MAX_DEV_LIST)
		return -ENOMEM;

	msm_fb_dev = platform_device_alloc("msm_fb", pdev->id);
	if (!msm_fb_dev)
		return -ENOMEM;

	/* link to the latest pdev */
	mfd->pdev = msm_fb_dev;
	mfd->mdp_rev = mdp_rev;

	if (mdp_pdata) {
		if (mdp_pdata->cont_splash_enabled) {
			mfd->cont_splash_done = 0;
			if (!contSplash_update_done) {
				mdp_pipe_ctrl(MDP_CMD_BLOCK,
					MDP_BLOCK_POWER_ON, FALSE);
				contSplash_update_done = 1;
			}
		} else
			mfd->cont_splash_done = 1;
	}

	mfd->ov0_wb_buf = MDP_ALLOC(sizeof(struct mdp_buf_type));
	mfd->ov1_wb_buf = MDP_ALLOC(sizeof(struct mdp_buf_type));
	memset((void *)mfd->ov0_wb_buf, 0, sizeof(struct mdp_buf_type));
	memset((void *)mfd->ov1_wb_buf, 0, sizeof(struct mdp_buf_type));

	if (mdp_pdata) {
		mfd->ov0_wb_buf->size = mdp_pdata->ov0_wb_size;
		mfd->ov1_wb_buf->size = mdp_pdata->ov1_wb_size;
		mfd->mem_hid = mdp_pdata->mem_hid;
	} else {
		mfd->ov0_wb_buf->size = 0;
		mfd->ov1_wb_buf->size = 0;
		mfd->mem_hid = 0;
	}
	mfd->ov0_blt_state  = 0;
	mfd->use_ov0_blt = 0 ;

    /* initialize Post Processing data*/
	mdp_hist_lut_init();

	/* add panel data */
	if (platform_device_add_data
	    (msm_fb_dev, pdev->dev.platform_data,
	     sizeof(struct msm_fb_panel_data))) {
		printk(KERN_ERR "mdp_probe: platform_device_add_data failed!\n");
		rc = -ENOMEM;
		goto mdp_probe_err;
	}
	/* data chain */
	pdata = msm_fb_dev->dev.platform_data;
	pdata->on = mdp_on;
	pdata->off = mdp_off;
	pdata->next = pdev;

	mdp_prim_panel_type = mfd->panel.type;
	switch (mfd->panel.type) {
	case EXT_MDDI_PANEL:
	case MDDI_PANEL:
	case EBI2_PANEL:
		INIT_WORK(&mfd->dma_update_worker,
			  mdp_lcd_update_workqueue_handler);
		INIT_WORK(&mfd->vsync_resync_worker,
			  mdp_vsync_resync_workqueue_handler);
		mfd->hw_refresh = FALSE;

		if (mfd->panel.type == EXT_MDDI_PANEL) {
			/* 15 fps -> 66 msec */
			mfd->refresh_timer_duration = (66 * HZ / 1000);
		} else {
			/* 24 fps -> 42 msec */
			mfd->refresh_timer_duration = (42 * HZ / 1000);
		}

#ifdef CONFIG_FB_MSM_MDP22
		mfd->dma_fnc = mdp_dma2_update;
		mfd->dma = &dma2_data;
#else
		if (mfd->panel_info.pdest == DISPLAY_1) {
#if defined(CONFIG_FB_MSM_OVERLAY) && defined(CONFIG_FB_MSM_MDDI)
			mfd->dma_fnc = mdp4_mddi_overlay;
			mfd->cursor_update = mdp4_mddi_overlay_cursor;
#else
			mfd->dma_fnc = mdp_dma2_update;
#endif
			mfd->dma = &dma2_data;
			mfd->lut_update = mdp_lut_update_nonlcdc;
			mfd->do_histogram = mdp_do_histogram;
		} else {
			mfd->dma_fnc = mdp_dma_s_update;
			mfd->dma = &dma_s_data;
		}
#endif
		if (mdp_pdata)
			mfd->vsync_gpio = mdp_pdata->gpio;
		else
			mfd->vsync_gpio = -1;

#ifdef CONFIG_FB_MSM_MDP40
		if (mfd->panel.type == EBI2_PANEL)
			intf = EBI2_INTF;
		else
			intf = MDDI_INTF;

		if (mfd->panel_info.pdest == DISPLAY_1)
			if_no = PRIMARY_INTF_SEL;
		else
			if_no = SECONDARY_INTF_SEL;

		mdp4_display_intf_sel(if_no, intf);
#endif
		mdp_config_vsync(mfd);
		break;

#ifdef CONFIG_FB_MSM_MIPI_DSI
	case MIPI_VIDEO_PANEL:
#ifndef CONFIG_FB_MSM_MDP303
		pdata->on = mdp4_dsi_video_on;
		pdata->off = mdp4_dsi_video_off;
		mfd->hw_refresh = TRUE;
		mfd->dma_fnc = mdp4_dsi_video_overlay;
		mfd->lut_update = mdp_lut_update_lcdc;
		mfd->do_histogram = mdp_do_histogram;
		if (mfd->panel_info.pdest == DISPLAY_1) {
			if_no = PRIMARY_INTF_SEL;
			mfd->dma = &dma2_data;
		} else {
			if_no = EXTERNAL_INTF_SEL;
			mfd->dma = &dma_e_data;
		}
		mdp4_display_intf_sel(if_no, DSI_VIDEO_INTF);
#else
		pdata->on = mdp_dsi_video_on;
		pdata->off = mdp_dsi_video_off;
		mfd->hw_refresh = TRUE;
		mfd->dma_fnc = mdp_dsi_video_update;
		mfd->do_histogram = mdp_do_histogram;
		if (mfd->panel_info.pdest == DISPLAY_1)
			mfd->dma = &dma2_data;
		else {
			printk(KERN_ERR "Invalid Selection of destination panel\n");
			rc = -ENODEV;
			goto mdp_probe_err;
		}

#endif
		if (mdp_rev >= MDP_REV_40)
			mfd->cursor_update = mdp_hw_cursor_sync_update;
		else
			mfd->cursor_update = mdp_hw_cursor_update;
		break;

	case MIPI_CMD_PANEL:
#ifndef CONFIG_FB_MSM_MDP303
		mfd->dma_fnc = mdp4_dsi_cmd_overlay;
#ifdef CONFIG_FB_MSM_MDP40
		mipi = &mfd->panel_info.mipi;
		configure_mdp_core_clk_table((mipi->dsi_pclk_rate) * 3 / 2);
#endif
		if (mfd->panel_info.pdest == DISPLAY_1) {
			if_no = PRIMARY_INTF_SEL;
			mfd->dma = &dma2_data;
		} else {
			if_no = SECONDARY_INTF_SEL;
			mfd->dma = &dma_s_data;
		}
		mfd->lut_update = mdp_lut_update_nonlcdc;
		mfd->do_histogram = mdp_do_histogram;
		mdp4_display_intf_sel(if_no, DSI_CMD_INTF);
#else
		mfd->dma_fnc = mdp_dma2_update;
/*LGE_CHANGE_S : jongyoung.koo@lge.com [2012-01-16] at%kcal*/
#ifndef CONFIG_FB_MSM_MDP_LUT_ENABLE
		mfd->do_histogram = mdp_do_histogram;
#endif 
/*LGE_CHANGE_E : jongyoung.koo@lge.com [2012-01-16] at%kcal*/
		if (mfd->panel_info.pdest == DISPLAY_1)
			mfd->dma = &dma2_data;
		else {
			printk(KERN_ERR "Invalid Selection of destination panel\n");
			rc = -ENODEV;
			goto mdp_probe_err;
		}
#endif
		mdp_config_vsync(mfd);
		break;
#endif

#ifdef CONFIG_FB_MSM_DTV
	case DTV_PANEL:
		pdata->on = mdp4_dtv_on;
		pdata->off = mdp4_dtv_off;
		mfd->hw_refresh = TRUE;
		mfd->cursor_update = mdp_hw_cursor_update;
		mfd->dma_fnc = mdp4_dtv_overlay;
		mfd->dma = &dma_e_data;
		mdp4_display_intf_sel(EXTERNAL_INTF_SEL, DTV_INTF);
		break;
#endif
	case HDMI_PANEL:
	case LCDC_PANEL:
		pdata->on = mdp_lcdc_on;
		pdata->off = mdp_lcdc_off;
		mfd->hw_refresh = TRUE;
#if	defined(CONFIG_FB_MSM_OVERLAY) && defined(CONFIG_FB_MSM_MDP40)
		mfd->cursor_update = mdp_hw_cursor_sync_update;
#else
		mfd->cursor_update = mdp_hw_cursor_update;
#endif
#ifndef CONFIG_FB_MSM_MDP22
		mfd->lut_update = mdp_lut_update_lcdc;
		mfd->do_histogram = mdp_do_histogram;
#endif
#ifdef CONFIG_FB_MSM_OVERLAY
		mfd->dma_fnc = mdp4_lcdc_overlay;
#else
		mfd->dma_fnc = mdp_lcdc_update;
#endif

#ifdef CONFIG_FB_MSM_MDP40
		configure_mdp_core_clk_table((mfd->panel_info.clk_rate)
								* 23 / 20);
		if (mfd->panel.type == HDMI_PANEL) {
			mfd->dma = &dma_e_data;
			mdp4_display_intf_sel(EXTERNAL_INTF_SEL, LCDC_RGB_INTF);
		} else {
			mfd->dma = &dma2_data;
			mdp4_display_intf_sel(PRIMARY_INTF_SEL, LCDC_RGB_INTF);
		}
#else
		mfd->dma = &dma2_data;
		spin_lock_irqsave(&mdp_spin_lock, flag);
		mdp_intr_mask &= ~MDP_DMA_P_DONE;
		outp32(MDP_INTR_ENABLE, mdp_intr_mask);
		spin_unlock_irqrestore(&mdp_spin_lock, flag);
#endif
		break;

	case TV_PANEL:
#if defined(CONFIG_FB_MSM_OVERLAY) && defined(CONFIG_FB_MSM_TVOUT)
		pdata->on = mdp4_atv_on;
		pdata->off = mdp4_atv_off;
		mfd->dma_fnc = mdp4_atv_overlay;
		mfd->dma = &dma_e_data;
		mdp4_display_intf_sel(EXTERNAL_INTF_SEL, TV_INTF);
#else
		pdata->on = mdp_dma3_on;
		pdata->off = mdp_dma3_off;
		mfd->hw_refresh = TRUE;
		mfd->dma_fnc = mdp_dma3_update;
		mfd->dma = &dma3_data;
#endif
		break;

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
	case WRITEBACK_PANEL:
		{
			unsigned int mdp_version;
			mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON,
						 FALSE);
			mdp_version = inpdw(MDP_BASE + 0x0);
			mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF,
						FALSE);
			if (mdp_version < 0x04030303) {
				pr_err("%s: writeback panel not supprted\n",
					 __func__);
				platform_device_put(msm_fb_dev);
				return -ENODEV;
			}
			pdata->on = mdp4_overlay_writeback_on;
			pdata->off = mdp4_overlay_writeback_off;
			mfd->dma_fnc = mdp4_writeback_overlay;
			mfd->dma = &dma_wb_data;
			mdp4_display_intf_sel(EXTERNAL_INTF_SEL, DTV_INTF);
		}
		break;
#endif
	default:
		printk(KERN_ERR "mdp_probe: unknown device type!\n");
		rc = -ENODEV;
		goto mdp_probe_err;
	}
#ifdef CONFIG_FB_MSM_MDP40
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	mdp4_display_intf = inpdw(MDP_BASE + 0x0038);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
#endif

#ifdef CONFIG_MSM_BUS_SCALING
	if (!mdp_bus_scale_handle && mdp_pdata &&
		mdp_pdata->mdp_bus_scale_table) {
		mdp_bus_scale_handle =
			msm_bus_scale_register_client(
					mdp_pdata->mdp_bus_scale_table);
		if (!mdp_bus_scale_handle) {
			printk(KERN_ERR "%s not able to get bus scale\n",
				__func__);
			return -ENOMEM;
		}
	}

	/* req bus bandwidth immediately */
	if (!(mfd->cont_splash_done))
		mdp_bus_scale_update_request(5);

#endif

/*LGE_CHANGE_S : jongyoung.koo@lge.com [2012-01-16] at%kcal*/
#ifdef CONFIG_FB_MSM_MDP_LUT_ENABLE 
			printk("### call the initial lut update routine\n");
			mdp_lut_init_update_lcdc();
#endif 
/*LGE_CHANGE_E : jongyoung.koo@lge.com [2012-01-16] at%kcal*/

	/* set driver data */
	platform_set_drvdata(msm_fb_dev, mfd);

	rc = platform_device_add(msm_fb_dev);
	if (rc) {
		goto mdp_probe_err;
	}

	pm_runtime_set_active(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	pdev_list[pdev_list_cnt++] = pdev;
	mdp4_extn_disp = 0;
	return 0;

      mdp_probe_err:
	platform_device_put(msm_fb_dev);
#ifdef CONFIG_MSM_BUS_SCALING
	if (mdp_pdata && mdp_pdata->mdp_bus_scale_table &&
		mdp_bus_scale_handle > 0)
		msm_bus_scale_unregister_client(mdp_bus_scale_handle);
#endif
	return rc;
}

void mdp_footswitch_ctrl(boolean on)
{
	mutex_lock(&mdp_suspend_mutex);
	if (!mdp_suspended || mdp4_extn_disp || !footswitch ||
		mdp_rev <= MDP_REV_41) {
		mutex_unlock(&mdp_suspend_mutex);
		return;
	}

	if (on && !mdp_footswitch_on) {
		pr_debug("Enable MDP FS\n");
		regulator_enable(footswitch);
		mdp_footswitch_on = 1;
	} else if (!on && mdp_footswitch_on) {
		pr_debug("Disable MDP FS\n");
		regulator_disable(footswitch);
		mdp_footswitch_on = 0;
	}

	mutex_unlock(&mdp_suspend_mutex);
}

#ifdef CONFIG_PM
static void mdp_suspend_sub(void)
{
	/* cancel pipe ctrl worker */
	cancel_delayed_work(&mdp_pipe_ctrl_worker);

	/* for workder can't be cancelled... */
	flush_workqueue(mdp_pipe_ctrl_wq);

	/* let's wait for PPP completion */
	while (atomic_read(&mdp_block_power_cnt[MDP_PPP_BLOCK]) > 0)
		cpu_relax();

	/* try to power down */
	mdp_pipe_ctrl(MDP_MASTER_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	mutex_lock(&mdp_suspend_mutex);
	mdp_suspended = TRUE;
	mutex_unlock(&mdp_suspend_mutex);
}
#endif

#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
static int mdp_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (pdev->id == 0) {
		mdp_suspend_sub();
		if (mdp_current_clk_on) {
			printk(KERN_WARNING"MDP suspend failed\n");
			return -EBUSY;
		}
	}

	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mdp_early_suspend(struct early_suspend *h)
{
	mdp_suspend_sub();
#ifdef CONFIG_FB_MSM_DTV
	mdp4_dtv_set_black_screen();
#endif
	mdp_footswitch_ctrl(FALSE);
}

static void mdp_early_resume(struct early_suspend *h)
{
	mdp_footswitch_ctrl(TRUE);
	mutex_lock(&mdp_suspend_mutex);
	mdp_suspended = FALSE;
	mutex_unlock(&mdp_suspend_mutex);
}
#endif

static int mdp_remove(struct platform_device *pdev)
{
	if (footswitch != NULL)
		regulator_put(footswitch);
	iounmap(msm_mdp_base);
	pm_runtime_disable(&pdev->dev);
#ifdef CONFIG_MSM_BUS_SCALING
	if (mdp_pdata && mdp_pdata->mdp_bus_scale_table &&
		mdp_bus_scale_handle > 0)
		msm_bus_scale_unregister_client(mdp_bus_scale_handle);
#endif
	return 0;
}

static int mdp_register_driver(void)
{
#ifdef CONFIG_HAS_EARLYSUSPEND
	early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1;
	early_suspend.suspend = mdp_early_suspend;
	early_suspend.resume = mdp_early_resume;
	register_early_suspend(&early_suspend);
#endif

	return platform_driver_register(&mdp_driver);
}

static int __init mdp_driver_init(void)
{
	int ret;

	mdp_drv_init();

	ret = mdp_register_driver();
	if (ret) {
		printk(KERN_ERR "mdp_register_driver() failed!\n");
		return ret;
	}

#if defined(CONFIG_DEBUG_FS)
	mdp_debugfs_init();
#endif

	return 0;

}

module_init(mdp_driver_init);
