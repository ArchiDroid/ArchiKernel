/* 
 * arch/arm/mach-msm/lge/lge_proc_comm.c
 *
 * Copyright (C) 2010 LGE, Inc
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/delay.h>
#include CONFIG_LGE_BOARD_HEADER_FILE
#include <mach/lge/lge_proc_comm.h>
#include <mach/proc_comm.h>

/* LGE_CHANGE_S, youngbae.choi@lge.com , for silence reset */
#ifdef CONFIG_LGE_SILENCE_RESET
static int silence_reset_f = 1;
#endif
/* LGE_CHANGE_E, youngbae.choi@lge.com , for silence reset */

/* 2011-01-26 by baborobo@lge.com
 * match with
 * /modem/AMSS/products/7x30/modem/rfa/pmic/common/app/src/pmapp_pcil.c
 */
#if 1 //#if defined(CONFIG_LGE_DETECT_PIF_PATCH)
unsigned lge_get_pif_info(void)
{
	int err;
	unsigned pif_value = -1;
	unsigned cmd_pif = 8;

	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &pif_value, &cmd_pif);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed\n",
		       __func__);
		return err;
	}
	
	return pif_value;
}
EXPORT_SYMBOL(lge_get_pif_info);

unsigned lge_get_cable_info(void)
{
	int err;
	unsigned pif_value = -1;
	unsigned cmd_pif = 0x13;

	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &pif_value, &cmd_pif);
	
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed\n",
		       __func__);
		return err;
	}
	
	return pif_value;
}
EXPORT_SYMBOL(lge_get_cable_info);

unsigned lge_get_lpm_info(void)
{
	int err;
	unsigned low_power_mode = 0;
	unsigned cmd_lpm = 7;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &low_power_mode, &cmd_lpm);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed\n",
		       __func__);
		return err;
	}

	return low_power_mode;
}
EXPORT_SYMBOL(lge_get_lpm_info);
#endif

#ifdef CONFIG_MACH_LGE
unsigned lge_get_batt_volt(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0x3;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_batt_volt);

unsigned lge_get_chg_therm(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0x9;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_chg_therm);

unsigned lge_get_pcb_version(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0xA;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_pcb_version);

unsigned lge_get_chg_curr_volt(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0xB;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_chg_curr_volt);

unsigned lge_get_batt_therm(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0xC;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_batt_therm);

// 2012-11-10 Jinhong Kim(miracle.kim@lge.com)  [V7][Power] read batt therm 8bit raw [START]
unsigned lge_get_batt_therm_8bit_raw(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0x17;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_batt_therm_8bit_raw);
// 2012-11-10 Jinhong Kim(miracle.kim@lge.com)  [V7][Power] read batt therm 8bit raw [END]

unsigned lge_get_batt_volt_raw(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0xD;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_batt_volt_raw);

#if 1 //#ifdef CONFIG_MACH_MSM7X27_GELATO
unsigned lge_get_chg_stat_reg(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0xE;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_chg_stat_reg);

unsigned lge_get_chg_en_reg(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0xF;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_chg_en_reg);


unsigned lge_set_elt_test(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0x10;

	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_set_elt_test);

unsigned lge_clear_elt_test(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0x11;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_clear_elt_test);


#endif
unsigned lge_get_nv_qem(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0x12;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_nv_qem);

//LGE_CHANGE_S FTM boot mode
#if (defined (CONFIG_MACH_MSM7X25A_V3) && !defined (CONFIG_MACH_MSM7X25A_M4)) || defined (CONFIG_MACH_MSM8X25_V7) || defined (CONFIG_MACH_MSM7X25A_V1)
unsigned lge_nv_manual_f(int val)
{
	int err;
	unsigned ret = val;
	unsigned cmd = 0x1c;

	printk(KERN_INFO"[FTM] lge_nv_manual_f [%d] \n", val);
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}
	return ret;
}
EXPORT_SYMBOL(lge_nv_manual_f);
#endif
//LGE_CHANGE_E FTM boot mode

//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-10-10
unsigned lge_get_batt_id(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0x4;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_batt_id);
//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-10-10

// 2012-11-05 Sonchiwon(chiwon.son@lge.com) [V3/V7][Hidden.Menu] HiddenMenu > Settings > Battery > Charging Bypass Boot [START]
unsigned lge_get_nv_charging_bypass_boot(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = 0x16;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_nv_charging_bypass_boot);
// 2012-11-05 Sonchiwon(chiwon.son@lge.com) [V3/V7][Hidden.Menu] HiddenMenu > Settings > Battery > Charging Bypass Boot [END]

/* LGE_CHANGE_S, youngbae.choi@lge.com , for silence reset */
#ifdef CONFIG_LGE_SILENCE_RESET
unsigned lge_silence_reset_f(int val)
{
    int err;
    unsigned ret = val;
    unsigned cmd = 0x18;

	if(silence_reset_f == ret)
		return 0;

	silence_reset_f = ret;	
    err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
    if (err < 0) {
        pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
               __func__, cmd);
        return err;
    }

    return ret;
}
EXPORT_SYMBOL(lge_silence_reset_f);
#endif
/* LGE_CHANGE_E, youngbae.choi@lge.com , for silence reset */

unsigned lge_smpl_counter_f(int val)
{
    int err;
    unsigned ret = val;
    unsigned cmd = 0x19;

    err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
    if (err < 0) {
        pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
               __func__, cmd);
        return err;
    }

    return ret;
}
EXPORT_SYMBOL(lge_smpl_counter_f);

unsigned lge_charging_bypass_boot_f(int val)
{
    int err;
    unsigned ret = val;
    unsigned cmd = 0x1a;

    err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
    if (err < 0) {
        pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
               __func__, cmd);
        return err;
    }

    return ret;
}
EXPORT_SYMBOL(lge_charging_bypass_boot_f);

unsigned lge_pseudo_battery_mode_f(int val)
{
    int err;
    unsigned ret = val;
    unsigned cmd = 0x1b;

    err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
    if (err < 0) {
        pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
               __func__, cmd);
        return err;
    }

    return ret;
}
EXPORT_SYMBOL(lge_pseudo_battery_mode_f);

unsigned lge_pm_low_vbatt_notify(void)
{
    int err;
    unsigned ret ;
    unsigned cmd = 0x01;

    err = msm_proc_comm(PCOM_CUSTOMER_CMD1, &ret, &cmd);
    if (err < 0) {
        pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD1) failed. cmd(%d)\n",
               __func__, cmd);
        return err;
    }

    return ret;
}
EXPORT_SYMBOL(lge_pm_low_vbatt_notify);

/* LGE_CHANGE_S  : adiyoung.lee, FTM Mode and ManualModeCkeckComplete on RPC, 2012-12-12 */
#if !defined(CONFIG_MACH_MSM7X25A_M4) && (defined (CONFIG_MACH_MSM7X25A_V3) || defined (CONFIG_MACH_MSM8X25_V7) || defined (CONFIG_MACH_MSM7X25A_V1))
unsigned lge_aat_partial_f(int val)

{
    int err;
    unsigned  ret = val;
    unsigned cmd = 0x1d;

   err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
    if (err < 0) {
        pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
               __func__, cmd);
        return err;
    }

    return ret;
}
EXPORT_SYMBOL(lge_aat_partial_f);

unsigned lge_aat_full_f(int val)

{
    int err;
    unsigned ret = val;
    unsigned cmd = 0x1e;

    err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
    if (err < 0) {
        pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
               __func__, cmd);
        return err;
    }

    return ret;
}
EXPORT_SYMBOL(lge_aat_full_f);

unsigned lge_aat_partial_or_full_f(int val)
{
    int err;
    unsigned ret = val;
    unsigned cmd = 0x1f;

    err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
    if (err < 0) {
        pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
               __func__, cmd);
        return err;
    }

    return ret;
}
EXPORT_SYMBOL(lge_aat_partial_or_full_f);
#endif
/* LGE_CHANGE_E  : adiyoung.lee, FTM Mode and ManualModeCkeckComplete on RPC, 2012-12-12 */

/*LGE_CHANGE_S: seunhang.lee@lge.com 14/01/2013:Factory Reset with AT Command*/
/* LGE_CHANGE hyungjoon.jeon@lge.com 12/02/13 */
//#if defined(CONFIG_MACH_MSM7X27A_U0)
#if defined(CONFIG_MACH_MSM7X27A_U0) ||  defined(CONFIG_MACH_MSM7X25A_M4)
#define CUSTOMER_CMD2_GET_FRSTSTATUS        0x20
#define CUSTOMER_CMD2_SET_FRSTSTATUS        0x21
#endif

/*LGE_CHANGE_S: seunhang.lee@lge.com 21/01/2013:Send Sync*/
#define CUSTOMER_CMD2_SEND_SYNC		        0x22
/*LGE_CHANGE_E: seunhang.lee@lge.com 21/01/2013:Send Sync*/

/* LGE_CHANGE hyungjoon.jeon@lge.com 12/02/13 */
//#if defined(CONFIG_MACH_MSM7X27A_U0)
#if defined(CONFIG_MACH_MSM7X27A_U0) ||  defined(CONFIG_MACH_MSM7X25A_M4)
unsigned lge_get_nv_frststatus(void) 
{
	int err;
	unsigned ret = 0;
	unsigned cmd = CUSTOMER_CMD2_GET_FRSTSTATUS;

	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) failed. cmd(%d)\n",
		__func__, cmd);
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_nv_frststatus);

void lge_set_nv_frststatus(unsigned int flag)
{
	int err;
	static unsigned req;
	unsigned cmd = CUSTOMER_CMD2_SET_FRSTSTATUS;

	req = flag;

/*LGE_CHANGE_S: seunhang.lee@lge.com 21/01/2013:Send Sync*/
	if ( req == 5 || req == 6 ) {
		err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &req, &cmd);

		if (err < 0) {
			pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2:CUSTOMER_CMD2_SET_FRSTSTATUS) failed.\n",
			__func__); 
		}
	}
	else {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2) req value (%d) failed.\n",
			__func__, req); 
	}
/*LGE_CHANGE_E: seunhang.lee@lge.com 21/01/2013:Send Sync*/
	return;
}       
EXPORT_SYMBOL(lge_set_nv_frststatus);
#endif
/*LGE_CHANGE_E: seunhang.lee@lge.com 14/01/2013:Factory Reset with AT Command*/

/*LGE_CHANGE_S: seunhang.lee@lge.com 21/01/2013:Send Sync*/
void lge_set_nv_send_sync()
{
	int err;
	int req;
	unsigned cmd;

	req = 0;
	cmd = CUSTOMER_CMD2_SEND_SYNC;
	err = msm_proc_comm(PCOM_CUSTOMER_CMD2, &req, &cmd);
		
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD2:CUSTOMER_CMD2_SEND_SYNC) failed.\n",
		__func__); 
	}
	return;
}       
EXPORT_SYMBOL(lge_set_nv_send_sync);
/*LGE_CHANGE_E: seunhang.lee@lge.com 21/01/2013:Send Sync*/

/* [LGE_BSP_S] jihyun2.jang@lge.com, 20130130 - RT8966A LDO & Backlight control */
#ifdef CONFIG_MACH_MSM7X25A_V1
#ifndef CONFIG_MINIABB_REGULATOR
/*********************************************************************************************************
	lge_rt8966a_ldo_control

	[input]

			ldo_id
				1 	: VREG_PROX_3.0V
				2 	: CAM_AVDD_2.8V
				3 	: CAM_DVDD_1.2V
				4 	: CAM_IOVDD_1.8V

			onoff
				0 	: off
				1 	: on

	[output]

				0 		: success
				-EIO 	: fail
*********************************************************************************************************/
int lge_rt8966a_ldo_control( int ldo_id, int onoff )
{
	int err;

	pr_info("%s : msm_proc_comm(PCOM_RT8966A_LDO_CONTROL) called. ldo_id=%d, onoff=%d\n", __func__, ldo_id, onoff);

	err = msm_proc_comm(PCOM_RT8966A_LDO_CONTROL, &ldo_id, &onoff);
	
	if (err < 0) {
		pr_err("%s : msm_proc_comm(PCOM_RT8966A_LDO_CONTROL) failed. err=%d\n", __func__, err);
		return err;
	}
	
	return 0;
}
EXPORT_SYMBOL(lge_rt8966a_ldo_control);

/*********************************************************************************************************
	boolean lge_rt8966a_ldo_set_level

	[input]

			ldo_id
				1 	: VREG_PROX_3.0V
				2 	: CAM_AVDD_2.8V
				3 	: CAM_DVDD_1.2V
				4 	: CAM_IOVDD_1.8V

			level
				0 	: 1.0V
				1 	: 1.2V
				2 	: 1.5V
				3 	: 1.8V
				4 	: 2.0V
				5 	: 2.1V
				6 	: 2.5V
				7 	: 2.6V
				8 	: 2.7V
				9 	: 2.8V
				10	: 2.9V
				11	: 3.0V
				12	: 3.1V
				13	: 3.3V

	[output]

				0 		: success
				-EIO 	: fail
*********************************************************************************************************/
int lge_rt8966a_ldo_set_level( int ldo_id, int level )
{
	int err;

	pr_info("%s : msm_proc_comm(PCOM_RT8966A_LDO_SET_LEVEL) called. ldo_id=%d, level=%d\n", __func__, ldo_id, level);

	err = msm_proc_comm(PCOM_RT8966A_LDO_SET_LEVEL, &ldo_id, &level);
	
	if (err < 0) {
		pr_err("%s : msm_proc_comm(PCOM_RT8966A_LDO_SET_LEVEL) failed. err=%d\n", __func__, err);
		return err;
	}

	return 0;
}
EXPORT_SYMBOL(lge_rt8966a_ldo_set_level);
#endif /* !CONFIG_MINIABB_REGULATOR */

/*********************************************************************************************************
	lge_rt8966a_backlight_control

	[input]

			onoff
					0 	: off
					1 	: on

	[output]
	
					0 		: success
					-EIO 	: fail
*********************************************************************************************************/
int lge_rt8966a_backlight_control( int onoff )
{
	int err;

	pr_info("%s : msm_proc_comm(PCOM_RT8966A_BACKLIGHT_CONTROL) called. onoff=%d\n", __func__, onoff);

	err = msm_proc_comm(PCOM_RT8966A_BACKLIGHT_CONTROL, &onoff, NULL);
	
	if (err < 0) {
		pr_err("%s : msm_proc_comm(PCOM_RT8966A_BACKLIGHT_CONTROL) failed. err=%d\n", __func__, err);
		return err;
	}
	
	return 0;
}
EXPORT_SYMBOL(lge_rt8966a_backlight_control);

/*********************************************************************************************************
	lge_rt8966a_backlight_set_level

	[input]

			scale
					0		: 2.5mA
					1		: 5mA
					2		: 10mA
					3		: 15mA
					4		: 20mA
					5		: 25mA
					6		: 30mA

			level
					0 ~ 0x1F

	[output]
	
					0 		: success
					-EIO 	: fail
*********************************************************************************************************/
int lge_rt8966a_backlight_set_level( int scale, int level )
{
	int err;

	pr_info("%s : msm_proc_comm(PCOM_RT8966A_BACKLIGHT_SET_LEVEL) called. scale =%d level=%d\n", __func__, scale, level);

	err = msm_proc_comm(PCOM_RT8966A_BACKLIGHT_SET_LEVEL, &scale, &level);
	
	if (err < 0) {
		pr_err("%s : msm_proc_comm(PCOM_RT8966A_BACKLIGHT_SET_LEVEL) failed. err=%d\n", __func__, err);
		return err;
	}
	
	return 0;
}
EXPORT_SYMBOL(lge_rt8966a_backlight_set_level);
#endif /* CONFIG_MACH_MSM7X25A_V1 */
/* [LGE_BSP_E] jihyun2.jang@lge.com, 20130130 - RT8966A LDO & Backlight control */

#endif
