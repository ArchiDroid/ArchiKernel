#ifndef __LGE_PROC_COMM_H__
#define __LGE_PROC_COMM_H__

/* External Functions Prototype */
unsigned lge_get_pif_info(void);
unsigned lge_get_lpm_info(void);
unsigned lge_get_batt_volt(void);
unsigned lge_get_chg_therm(void);
unsigned lge_get_pcb_version(void);
unsigned lge_get_chg_curr_volt(void);
unsigned lge_get_batt_therm(void);
// 2012-11-10 Jinhong Kim(miracle.kim@lge.com)  [V7][Power] read batt therm 8bit raw [START]
unsigned lge_get_batt_therm_8bit_raw(void);
// 2012-11-10 Jinhong Kim(miracle.kim@lge.com)  [V7][Power] read batt therm 8bit raw [END]
unsigned lge_get_batt_volt_raw(void);
unsigned lge_get_chg_stat_reg(void);
unsigned lge_get_chg_en_reg(void);
unsigned lge_set_elt_test(void);
unsigned lge_clear_elt_test(void);
unsigned lge_get_batt_id(void);
unsigned lge_get_cable_info(void);
unsigned lge_get_nv_qem(void);
unsigned lge_get_cable_info(void);
// 2012-11-05 Sonchiwon(chiwon.son@lge.com) [V3/V7][Hidden.Menu] HiddenMenu > Settings > Battery > Charging Bypass Boot [START]
unsigned lge_get_nv_charging_bypass_boot(void);
// 2012-11-05 Sonchiwon(chiwon.son@lge.com) [V3/V7][Hidden.Menu] HiddenMenu > Settings > Battery > Charging Bypass Boot [START]
unsigned lge_pm_low_vbatt_notify(void);
/*LGE_CHANGE_S: seunhang.lee@lge.com 14/01/2013:Factory Reset with AT Command*/
/* LGE_CHANGE hyungjoon.jeon@lge.com 12/02/13 */ 
//#if defined(CONFIG_MACH_MSM7X27A_U0)
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_M4)
unsigned lge_get_nv_frststatus(void);
void lge_set_nv_frststatus(unsigned int flag);
#endif
/*LGE_CHANGE_E: seunhang.lee@lge.com 14/01/2013:Factory Reset with AT Command*/
/*LGE_CHANGE_S: seunhang.lee@lge.com 21/01/2013:Send Sync*/
void lge_set_nv_send_sync(void);
/*LGE_CHANGE_E: seunhang.lee@lge.com 21/01/2013:Send Sync*/

/* [LGE_BSP_S] jihyun2.jang@lge.com, 20130130 - RT8966A LDO & Backlight control */
#ifdef CONFIG_MACH_MSM7X25A_V1
#ifndef CONFIG_MINIABB_REGULATOR
int lge_rt8966a_ldo_control( int ldo_id, int onoff );
int lge_rt8966a_ldo_set_level( int ldo_id, int level );
#endif /* !CONFIG_MINIABB_REGULATOR */
int lge_rt8966a_backlight_control( int onoff );
int lge_rt8966a_backlight_set_level( int scale, int level );
#endif /* CONFIG_MACH_MSM7X25A_V1 */
/* [LGE_BSP_E] jihyun2.jang@lge.com, 20130130 - RT8966A LDO & Backlight control */

#endif/*__LGE_PROC_COMM_H__*/
