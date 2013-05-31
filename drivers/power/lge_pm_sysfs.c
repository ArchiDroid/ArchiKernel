/* Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
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
 */

/*
 * this needs to be before <linux/kernel.h> is loaded,
 * and <linux/sched.h> loads <linux/kernel.h>
 */
 
#include <linux/module.h>
#include <linux/platform_device.h>
#include <mach/lge/lge_proc_comm.h>
#include CONFIG_LGE_BOARD_HEADER_FILE
#include <mach/lge/lge_pm.h>

#include "../../kernel/power/power.h"
#include <mach/msm_smsm.h>
#if defined(CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
#include <mach/msm_hsusb.h>
#endif
#ifdef CONFIG_LGE_LOW_VOLTAGE_BATTERY_CHECK
#include <linux/workqueue.h>
#include <linux/reboot.h>
#include <linux/cpumask.h>
#endif
/* LGE_CHANGE_S [jongjin7.park@lge.com] 20130122 Added direct read capacity sysfs */
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM8X25_V7) || defined (CONFIG_MACH_MSM7X25A_V1)
#include <mach/msm_battery.h>
#endif
/* LGE_CHANGE_E [jongjin7.park@lge.com] 20130122 Added direct read capacity sysfs */

#if defined(CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
extern int get_charger_type(void); /* defined in msm72k_udc.c */
#endif
extern u32 msm_batt_get_vbatt_level(void);
static ssize_t batt_volt_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_batt_volt());
}
static ssize_t chg_therm_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_chg_therm());
}
static ssize_t pcb_version_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_pcb_version());
}
static ssize_t chg_curr_volt_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_chg_curr_volt());
}
static ssize_t batt_therm_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_batt_therm());
}
// 2012-11-10 Jinhong Kim(miracle.kim@lge.com)	[V7][Power] read batt therm 8bit raw [START]
static ssize_t batt_therm_8bit_raw_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_batt_therm_8bit_raw());
}
// 2012-11-10 Jinhong Kim(miracle.kim@lge.com)	[V7][Power] read batt therm 8bit raw [END]
static ssize_t batt_volt_raw_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_batt_volt_raw());
}
static ssize_t chg_stat_reg_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_chg_stat_reg());
}
static ssize_t chg_en_reg_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_chg_en_reg());
}
static ssize_t batt_id_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_batt_id());
}

static ssize_t pm_suspend_state_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", check_suspend_state());
}

#ifdef CONFIG_LGE_PM_BATT_ID_DETECTION
static ssize_t lge_battery_id_info_show(struct device* dev,struct device_attribute* attr,char* buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", lge_get_battery_id());
}
#endif

/* LGE_CHANGE_S: [murali.ramaiah@lge.com] 2013-01-07 */
#if defined(CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
static ssize_t msm_batt_chgr_status_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	int chg_type;
	chg_type = get_charger_type();
	return sprintf(buf,"%d\n", (chg_type == USB_CHG_TYPE__WALLCHARGER) ? 1 : (chg_type == USB_CHG_TYPE__SDP) ? 1 : 0);
}
#endif /* CONFIG_MACH_MSM7X25A_V3 */
/* LGE_CHANGE_S: [murali.ramaiah@lge.com] 2013-01-07 */
/* LGE_CHANGE_S [jongjin7.park@lge.com] 20130122 Added direct read capacity sysfs */
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM8X25_V7) || defined (CONFIG_MACH_MSM7X25A_V1)
static ssize_t msm_batt_capacity_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	return snprintf(buf, PAGE_SIZE,"%d\n", msm_batt_get_vbatt_capacity());
}
#endif
/* LGE_CHANGE_E [jongjin7.park@lge.com] 20130122 Added direct read capacity sysfs */

static ssize_t msm_batt_level_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	return snprintf(buf, PAGE_SIZE,"%d\n", msm_batt_get_vbatt_level());
}
static DEVICE_ATTR(batt_volt, S_IRUGO, batt_volt_show, NULL);
static DEVICE_ATTR(chg_therm, S_IRUGO, chg_therm_show, NULL);
static DEVICE_ATTR(pcb_version, S_IRUGO, pcb_version_show, NULL);
static DEVICE_ATTR(chg_curr_volt, S_IRUGO, chg_curr_volt_show, NULL);
static DEVICE_ATTR(batt_therm, S_IRUGO, batt_therm_show, NULL);
// 2012-11-10 Jinhong Kim(miracle.kim@lge.com)	[V7][Power] read batt therm 8bit raw [START]
static DEVICE_ATTR(batt_therm_8bit_raw, S_IRUGO, batt_therm_8bit_raw_show, NULL);
// 2012-11-10 Jinhong Kim(miracle.kim@lge.com)	[V7][Power] read batt therm 8bit raw [END]
static DEVICE_ATTR(batt_volt_raw, S_IRUGO, batt_volt_raw_show, NULL);
static DEVICE_ATTR(chg_stat_reg, S_IRUGO, chg_stat_reg_show, NULL);
static DEVICE_ATTR(chg_en_reg, S_IRUGO, chg_en_reg_show, NULL);
static DEVICE_ATTR(batt_id, S_IRUGO, batt_id_show, NULL);
static DEVICE_ATTR(pm_suspend_state, S_IRUGO, pm_suspend_state_show, NULL);
#ifdef CONFIG_LGE_PM_BATT_ID_DETECTION
static DEVICE_ATTR(lge_battery_id_info, S_IRUGO, lge_battery_id_info_show, NULL);
#endif
#if defined(CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
static DEVICE_ATTR(chgr_status, S_IRUGO, msm_batt_chgr_status_show, NULL);
#endif
/* LGE_CHANGE_S [jongjin7.park@lge.com] 20130122 Added direct read capacity sysfs */
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM8X25_V7) || defined (CONFIG_MACH_MSM7X25A_V1)
static DEVICE_ATTR(batt_capacity, S_IRUGO, msm_batt_capacity_show, NULL);
#endif
/* LGE_CHANGE_E [jongjin7.park@lge.com] 20130122 Added direct read capacity sysfs */

static DEVICE_ATTR(msm_batt_level, S_IRUGO, msm_batt_level_show, NULL);

static struct attribute* dev_attrs_lge_pm_info[] = {
	&dev_attr_batt_volt.attr,
	&dev_attr_chg_therm.attr,
	&dev_attr_pcb_version.attr,
	&dev_attr_chg_curr_volt.attr,
	&dev_attr_batt_therm.attr,
// 2012-11-10 Jinhong Kim(miracle.kim@lge.com)	[V7][Power] read batt therm 8bit raw [START]
	&dev_attr_batt_therm_8bit_raw.attr,
// 2012-11-10 Jinhong Kim(miracle.kim@lge.com)	[V7][Power] read batt therm 8bit raw [END]
	&dev_attr_batt_volt_raw.attr,	
	&dev_attr_chg_stat_reg.attr,
	&dev_attr_chg_en_reg.attr,
	&dev_attr_batt_id.attr,
	&dev_attr_pm_suspend_state.attr,
#ifdef CONFIG_LGE_PM_BATT_ID_DETECTION
	&dev_attr_lge_battery_id_info.attr,
#endif
#if defined(CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
	&dev_attr_chgr_status.attr,
#endif
/* LGE_CHANGE_S [jongjin7.park@lge.com] 20130122 Added direct read capacity sysfs */
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM8X25_V7) || defined (CONFIG_MACH_MSM7X25A_V1)
	&dev_attr_batt_capacity.attr,
#endif
/* LGE_CHANGE_E [jongjin7.park@lge.com] 20130122 Added direct read capacity sysfs */
	&dev_attr_msm_batt_level.attr,
	NULL,
};

static struct attribute_group dev_attr_grp_lge_pm_info = {
	.attrs = dev_attrs_lge_pm_info,
};

static int __devinit lge_pm_probe(struct platform_device *pdev)
{
	int rc;
	
	rc = sysfs_create_group(&pdev->dev.kobj, &dev_attr_grp_lge_pm_info);
	if(rc < 0)
	{
		dev_err(&pdev->dev,"%s: fail to create sysfs for lge_pm rc=%d\n", __func__, rc);
	}
	return rc;
}

static int __devexit lge_pm_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj,&dev_attr_grp_lge_pm_info);
	return 0;
}

static struct platform_driver lge_pm_driver = {
	.probe  = lge_pm_probe,
	.remove = __devexit_p(lge_pm_remove),
	.driver = {
			 .name  = LGE_PM_DEVICE,
			 .owner = THIS_MODULE,
			 },
};

// LGE_CHANGE_S,narasimha.chikka@lge.com,Add BATT_ID Check
#if defined(CONFIG_LGE_PM_BATT_ID_DETECTION)
static void  __init  lge_pm_boot_batt_id_check(void)
{
	u32 *smem_batt_id = NULL;
	u32  batt_id = BATT_UNKNOWN;

	smem_batt_id = (u32*)smem_alloc(SMEM_BATT_INFO,sizeof(u32));

	if(smem_batt_id != NULL){

		batt_id = *smem_batt_id;

		if((batt_id == BATT_ISL6296_L) || (batt_id == BATT_DS2704_L)
		   || (batt_id == BATT_DS2704_C) || (batt_id == BATT_ISL6296_C)){
			printk(KERN_INFO "%s, High Voltage Battery Detected \n",__func__);
		}
		else if(batt_id == BATT_NORMAL){
			printk(KERN_INFO "%s, Normal Battery Detected \n",__func__);
		}
		else{
			printk(KERN_INFO "%s, Unknow Battery [Strange!!] \n",__func__);
		}	
	}
	else{
		printk(KERN_INFO "%s, BATT ID DETECTION FAILS \n",__func__);
	}
}
#endif
// LGE_CHANGE_E,narasimha.chikka@lge.com,Add BATT_ID Check

#ifdef CONFIG_LGE_LOW_VOLTAGE_BATTERY_CHECK
static void pm_do_poweroff(struct work_struct *dummy)
{
	kernel_power_off();
}

static DECLARE_WORK(poweroff_work, pm_do_poweroff);

void lge_pm_handle_poweroff(void)
{
#if 1
	lge_pm_low_vbatt_notify();
#else
	schedule_work_on(cpumask_first(cpu_online_mask), &poweroff_work);
#endif
}
EXPORT_SYMBOL(lge_pm_handle_poweroff);
#endif

static int __init lge_pm_init(void)
{

#if defined(CONFIG_LGE_PM_BATT_ID_DETECTION)
	lge_pm_boot_batt_id_check();
#endif

	return platform_driver_register(&lge_pm_driver);
}
static void __exit lge_pm_exit(void)
{
	platform_driver_unregister(&lge_pm_driver);
}

module_init(lge_pm_init);
module_exit(lge_pm_exit);
