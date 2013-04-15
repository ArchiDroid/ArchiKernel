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

#if defined(CONFIG_MACH_MSM8X25_V7)
#define DEBUG 1
#else
#define DEBUG 0
#endif

#include <linux/slab.h>
#include <linux/earlysuspend.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#include <asm/atomic.h>

#include <mach/msm_rpcrouter.h>
#include <mach/msm_battery.h>
#if defined(CONFIG_MACH_MSM7X27A_U0)
/* LGE_CHANGE_S : U0 Heating and DoU Issue
 * 2012-01-26, yoonsoo.kim@lge.com,
 * When user enter the streaming service, change the charging current
 */
#include <linux/delay.h>
#include <linux/timer.h>
#include <mach/pmic.h>
/* LGE_CHANGE_E : U0 Heating and DoU Issue*/
#endif
#if defined (CONFIG_LGE_CHARGER_TYPE_DETECTION) || defined (CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
#include <mach/msm_hsusb.h>
#include <mach/lge/lge_boot_mode.h>
#endif
#include CONFIG_LGE_BOARD_HEADER_FILE

#if defined(LGE_CHG_DONE_NOTIFICATION)
#include <mach/rpc_server_handset.h>
#endif

#define BATTERY_RPC_PROG	0x30000089
#define BATTERY_RPC_VER_1_1	0x00010001
#define BATTERY_RPC_VER_2_1	0x00020001
#define BATTERY_RPC_VER_4_1     0x00040001
#define BATTERY_RPC_VER_5_1     0x00050001

#define BATTERY_RPC_CB_PROG	(BATTERY_RPC_PROG | 0x01000000)

#define CHG_RPC_PROG		0x3000001a
#define CHG_RPC_VER_1_1		0x00010001
#define CHG_RPC_VER_1_3		0x00010003
#define CHG_RPC_VER_2_2		0x00020002
#define CHG_RPC_VER_3_1         0x00030001
#define CHG_RPC_VER_4_1         0x00040001

#define BATTERY_REGISTER_PROC				2
#define BATTERY_MODIFY_CLIENT_PROC			4
#define BATTERY_DEREGISTER_CLIENT_PROC			5
#define BATTERY_READ_MV_PROC				12
#define BATTERY_ENABLE_DISABLE_FILTER_PROC		14
#ifdef CONFIG_LGE_FUEL_GAUGE
#define BATTERY_READ_SOC_PROC				17
#endif

#define VBATT_FILTER			2

#define BATTERY_CB_TYPE_PROC		1
#define BATTERY_CB_ID_ALL_ACTIV		1
#define BATTERY_CB_ID_LOW_VOL		2

#define BATTERY_LOW		3200
#define BATTERY_HIGH		4300

#define ONCRPC_CHG_GET_GENERAL_STATUS_PROC	12
#define ONCRPC_CHARGER_API_VERSIONS_PROC	0xffffffff

#define BATT_RPC_TIMEOUT    5000	/* 5 sec */

#define INVALID_BATT_HANDLE    -1

#define RPC_TYPE_REQ     0
#define RPC_TYPE_REPLY   1
#define RPC_REQ_REPLY_COMMON_HEADER_SIZE   (3 * sizeof(uint32_t))


#if DEBUG
#define DBG_LIMIT(x...) do {if (printk_ratelimit()) pr_debug(x); } while (0)
#else
#define DBG_LIMIT(x...) do {} while (0)
#endif


enum {
	BATTERY_REGISTRATION_SUCCESSFUL = 0,
	BATTERY_DEREGISTRATION_SUCCESSFUL = BATTERY_REGISTRATION_SUCCESSFUL,
	BATTERY_MODIFICATION_SUCCESSFUL = BATTERY_REGISTRATION_SUCCESSFUL,
	BATTERY_INTERROGATION_SUCCESSFUL = BATTERY_REGISTRATION_SUCCESSFUL,
	BATTERY_CLIENT_TABLE_FULL = 1,
	BATTERY_REG_PARAMS_WRONG = 2,
	BATTERY_DEREGISTRATION_FAILED = 4,
	BATTERY_MODIFICATION_FAILED = 8,
	BATTERY_INTERROGATION_FAILED = 16,
	/* Client's filter could not be set because perhaps it does not exist */
	BATTERY_SET_FILTER_FAILED         = 32,
	/* Client's could not be found for enabling or disabling the individual
	 * client */
	BATTERY_ENABLE_DISABLE_INDIVIDUAL_CLIENT_FAILED  = 64,
	BATTERY_LAST_ERROR = 128,
};

enum {
	BATTERY_VOLTAGE_UP = 0,
	BATTERY_VOLTAGE_DOWN,
	BATTERY_VOLTAGE_ABOVE_THIS_LEVEL,
	BATTERY_VOLTAGE_BELOW_THIS_LEVEL,
	BATTERY_VOLTAGE_LEVEL,
	BATTERY_ALL_ACTIVITY,
	VBATT_CHG_EVENTS,
	BATTERY_VOLTAGE_UNKNOWN,
};

/*
 * This enum contains defintions of the charger hardware status
 */
enum chg_charger_status_type {
	/* The charger is good      */
	CHARGER_STATUS_GOOD,
	/* The charger is bad       */
	CHARGER_STATUS_BAD,
	/* The charger is weak      */
	CHARGER_STATUS_WEAK,
	/* Invalid charger status.  */
	CHARGER_STATUS_INVALID
};

/*
 *This enum contains defintions of the charger hardware type
 */
enum chg_charger_hardware_type {
	/* The charger is removed                 */
	CHARGER_TYPE_NONE,
	/* The charger is a regular wall charger   */
	CHARGER_TYPE_WALL,
	/* The charger is a PC USB                 */
	CHARGER_TYPE_USB_PC,
	/* The charger is a wall USB charger       */
	CHARGER_TYPE_USB_WALL,
	/* The charger is a USB carkit             */
	CHARGER_TYPE_USB_CARKIT,
	/* Invalid charger hardware status.        */
	CHARGER_TYPE_INVALID
};

/*
 *  This enum contains defintions of the battery status
 */
enum chg_battery_status_type {
	/* The battery is good        */
	BATTERY_STATUS_GOOD,
	/* The battery is cold/hot    */
	BATTERY_STATUS_BAD_TEMP,
	/* The battery is bad         */
	BATTERY_STATUS_BAD,
	/* The battery is removed     */
	BATTERY_STATUS_REMOVED,		/* on v2.2 only */
	BATTERY_STATUS_INVALID_v1 = BATTERY_STATUS_REMOVED,
	/* Invalid battery status.    */
	BATTERY_STATUS_INVALID
};

/*
 *This enum contains defintions of the battery voltage level
 */
enum chg_battery_level_type {
	/* The battery voltage is dead/very low (less than 3.2V) */
	BATTERY_LEVEL_DEAD,
	/* The battery voltage is weak/low (between 3.2V and 3.4V) */
	BATTERY_LEVEL_WEAK,
	/* The battery voltage is good/normal(between 3.4V and 4.2V) */
	BATTERY_LEVEL_GOOD,
	/* The battery voltage is up to full (close to 4.2V) */
	BATTERY_LEVEL_FULL,
	/* Invalid battery voltage level. */
	BATTERY_LEVEL_INVALID
};

#ifndef CONFIG_BATTERY_MSM_FAKE
struct rpc_reply_batt_chg_v1 {
	struct rpc_reply_hdr hdr;
	u32 	more_data;

	u32	charger_status;
	u32	charger_type;
	u32	battery_status;
	u32	battery_level;
	u32     battery_voltage;
	u32	battery_temp;
#ifdef CONFIG_LGE_FUEL_GAUGE
	u32	battery_capacity;
#endif
#if defined(LGE_DEV_CHARGING_CURRENT)
	u32	charging_current;
#endif
};

struct rpc_reply_batt_chg_v2 {
	struct rpc_reply_batt_chg_v1	v1;

	u32	is_charger_valid;
	u32	is_charging;
	u32	is_battery_valid;
	u32	ui_event;
};

union rpc_reply_batt_chg {
	struct rpc_reply_batt_chg_v1	v1;
	struct rpc_reply_batt_chg_v2	v2;
};

static union rpc_reply_batt_chg rep_batt_chg;
#endif

struct msm_battery_info {
	u32 voltage_max_design;
	u32 voltage_min_design;
	u32 voltage_fail_safe;
	u32 chg_api_version;
	u32 batt_technology;
	u32 batt_api_version;

	u32 avail_chg_sources;
	u32 current_chg_source;

	u32 batt_status;
	u32 batt_health;
	u32 charger_valid;
	u32 batt_valid;
	u32 batt_capacity; /* in percentage */

	u32 charger_status;
	u32 charger_type;
	u32 battery_status;
	u32 battery_level;
	u32 battery_voltage; /* in millie volts */
	u32 battery_temp;  /* in celsius */

	u32(*calculate_capacity) (u32 voltage);

#ifdef CONFIG_LGE_LOW_VOLTAGE_BATTERY_CHECK
	bool low_vbatt_check;
	void  (*power_off_device)(void);
#endif

	s32 batt_handle;

	struct power_supply *msm_psy_ac;
	struct power_supply *msm_psy_usb;
	struct power_supply *msm_psy_batt;
	struct power_supply *current_ps;

	struct msm_rpc_client *batt_client;
	struct msm_rpc_endpoint *chg_ep;

	wait_queue_head_t wait_q;

	u32 vbatt_modify_reply_avail;

#if defined(CONFIG_MACH_LGE)
	bool is_run_batt_update;
#ifdef LGE_CHG_DONE_NOTIFICATION
	bool wakeup_signal;
#endif
#endif
	struct early_suspend early_suspend;
};

static struct msm_battery_info msm_batt_info = {
	.batt_handle = INVALID_BATT_HANDLE,
	.charger_status = CHARGER_STATUS_BAD,
	.charger_type = CHARGER_TYPE_INVALID,
	.battery_status = BATTERY_STATUS_GOOD,
	.battery_level = BATTERY_LEVEL_FULL,
	.battery_voltage = BATTERY_HIGH,
	.batt_capacity = 100,
	.batt_status = POWER_SUPPLY_STATUS_DISCHARGING,
	.batt_health = POWER_SUPPLY_HEALTH_GOOD,
	.batt_valid  = 1,
	.battery_temp = 23,
	.vbatt_modify_reply_avail = 0,
#ifdef LGE_CHG_DONE_NOTIFICATION
	.wakeup_signal = false,
#endif
};

static enum power_supply_property msm_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static char *msm_power_supplied_to[] = {
	"battery",
};
#if defined(CONFIG_MACH_MSM7X27A_U0)
/* LGE_CHANGE_S : U0 Heating and DoU Issue
 * 2012-01-26, yoonsoo.kim@lge.com,
 * When user enter the streaming service, change the charging current
 */
static unsigned int i_chg_current_change;
static unsigned int i_chg_current_change_back_up;
/* LGE_CHANGE_E : U0 Heating and DoU Issue*/
#endif
#ifdef CONFIG_LGE_CHARGER_TYPE_DETECTION

extern int get_charger_type(void);
static int msm_power_get_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
int chg_type = get_charger_type();
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if (psy->type == POWER_SUPPLY_TYPE_MAINS) {
			val->intval = (LGE_BOOT_MODE_CHARGER == get_lge_boot_mode())
						?((chg_type == USB_CHG_TYPE__WALLCHARGER) ? 1 : 0)
						:((msm_batt_info.current_chg_source & AC_CHG)? 1 : 0);
		}
		if (psy->type == POWER_SUPPLY_TYPE_USB) {
			val->intval =(LGE_BOOT_MODE_CHARGER == get_lge_boot_mode())
						?((chg_type == USB_CHG_TYPE__SDP)? 1 : 0)
						:((msm_batt_info.current_chg_source & USB_CHG)? 1 : 0);
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
#else
static int msm_power_get_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if (psy->type == POWER_SUPPLY_TYPE_MAINS) {
			val->intval = msm_batt_info.current_chg_source & AC_CHG
			    ? 1 : 0;
		}
		if (psy->type == POWER_SUPPLY_TYPE_USB) {
			val->intval = msm_batt_info.current_chg_source & USB_CHG
			    ? 1 : 0;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
#endif
static struct power_supply msm_psy_ac = {
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.supplied_to = msm_power_supplied_to,
	.num_supplicants = ARRAY_SIZE(msm_power_supplied_to),
	.properties = msm_power_props,
	.num_properties = ARRAY_SIZE(msm_power_props),
	.get_property = msm_power_get_property,
};

static struct power_supply msm_psy_usb = {
	.name = "usb",
	.type = POWER_SUPPLY_TYPE_USB,
	.supplied_to = msm_power_supplied_to,
	.num_supplicants = ARRAY_SIZE(msm_power_supplied_to),
	.properties = msm_power_props,
	.num_properties = ARRAY_SIZE(msm_power_props),
	.get_property = msm_power_get_property,
};

static enum power_supply_property msm_batt_power_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
// LGE_CHANGE_S,narasimha.chikka@lge.com,Add Vbatt temp prop
/*LGE_CHANGE_S : 121110 [fe.kim@lge.com] to write battery temperature info on Android Native hidden menu*/
#if defined (CONFIG_LGE_BATTERY_TEMP) || defined (CONFIG_MACH_MSM7X27A_U0)
/*LGE_CHANGE_E : 121110 [fe.kim@lge.com] to write battery temperature info on Android Native hidden menu*/
	POWER_SUPPLY_PROP_TEMP,
#endif
// LGE_CHANGE_E,narasimha.chikka@lge.com,Add Vbatt temp prop
};

static int msm_batt_power_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = msm_batt_info.batt_status;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = msm_batt_info.batt_health;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = msm_batt_info.batt_valid;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = msm_batt_info.batt_technology;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = msm_batt_info.voltage_max_design;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = msm_batt_info.voltage_min_design;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		/* 2012-11-01 dajin.kim@lge.com change unit mV -> uV */
		val->intval = msm_batt_info.battery_voltage * 1000;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = msm_batt_info.batt_capacity;
		break;
// LGE_CHANGE_S,narasimha.chikka@lge.com,Add Vbatt temp prop
/*LGE_CHANGE_S : 121110 [fe.kim@lge.com] to write battery temperature info on Android Native hidden menu*/
#if defined (CONFIG_LGE_BATTERY_TEMP)
	case POWER_SUPPLY_PROP_TEMP:
#if defined (CONFIG_MACH_MSM7X25A_V1)
	     	val->intval = msm_batt_info.battery_temp * 10;
#else
		val->intval = msm_batt_info.battery_temp;
#endif
		break;
#elif defined (CONFIG_MACH_MSM7X27A_U0)
    case POWER_SUPPLY_PROP_TEMP:
        val->intval = msm_batt_info.battery_temp * 10;
		break;
#endif
/*LGE_CHANGE_E : 121110 [fe.kim@lge.com] to write battery temperature info on Android Native hidden menu*/
// LGE_CHANGE_E,narasimha.chikka@lge.com,Add Vbatt temp prop
	default:
		return -EINVAL;
	}
	return 0;
}

static struct power_supply msm_psy_batt = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = msm_batt_power_props,
	.num_properties = ARRAY_SIZE(msm_batt_power_props),
	.get_property = msm_batt_power_get_property,
};

/*2012-11-13 sungchul.jung@lge.com did not used 'CONFIG_BATTERY_MSM_FAKE' flag [START] */
#ifdef CONFIG_LGE_BATTERY_SUSPEND_RESUME
//#ifndef CONFIG_BATTERY_MSM_FAKE
#endif /*CONFIG_LGE_BATTERY_SUSPEND_RESUME*/
/*2012-11-13 sungchul.jung@lge.com did not used 'CONFIG_BATTERY_MSM_FAKE' flag [END] */

struct msm_batt_get_volt_ret_data {
	u32 battery_voltage;
};

static int msm_batt_get_volt_ret_func(struct msm_rpc_client *batt_client,
				       void *buf, void *data)
{
	struct msm_batt_get_volt_ret_data *data_ptr, *buf_ptr;

	data_ptr = (struct msm_batt_get_volt_ret_data *)data;
	buf_ptr = (struct msm_batt_get_volt_ret_data *)buf;

	data_ptr->battery_voltage = be32_to_cpu(buf_ptr->battery_voltage);

	return 0;
}

static u32 msm_batt_get_vbatt_voltage(void)
{
	int rc;

	struct msm_batt_get_volt_ret_data rep;

	rc = msm_rpc_client_req(msm_batt_info.batt_client,
			BATTERY_READ_MV_PROC,
			NULL, NULL,
			msm_batt_get_volt_ret_func, &rep,
			msecs_to_jiffies(BATT_RPC_TIMEOUT));

	if (rc < 0) {
		pr_err("%s: FAIL: vbatt get volt. rc=%d\n", __func__, rc);
		return 0;
	}

	return rep.battery_voltage;
}

#ifdef CONFIG_LGE_FUEL_GAUGE
struct msm_batt_get_soc_ret_data {
	u32 battery_capacity;
};

static int msm_batt_get_soc_ret_func(struct msm_rpc_client *batt_client,
				       void *buf, void *data)
{
	struct msm_batt_get_soc_ret_data *data_ptr, *buf_ptr;

	data_ptr = (struct msm_batt_get_soc_ret_data *)data;
	buf_ptr = (struct msm_batt_get_soc_ret_data *)buf;

	data_ptr->battery_capacity = be32_to_cpu(buf_ptr->battery_capacity);

	return 0;
}

u32 msm_batt_get_vbatt_capacity(void)
{
	int rc;

	struct msm_batt_get_soc_ret_data rep;

	rc = msm_rpc_client_req(msm_batt_info.batt_client,
			BATTERY_READ_SOC_PROC,
			NULL, NULL,
			msm_batt_get_soc_ret_func, &rep,
			msecs_to_jiffies(BATT_RPC_TIMEOUT));

	if (rc < 0) {
		pr_err("%s: FAIL: vbatt get volt. rc=%d\n", __func__, rc);
		return 0;
	}

	return rep.battery_capacity;
}
EXPORT_SYMBOL(msm_batt_get_vbatt_capacity);
#endif

u32 msm_batt_get_vbatt_level(void)
{
	return msm_batt_info.battery_level;
}
EXPORT_SYMBOL(msm_batt_get_vbatt_level);
#define	be32_to_cpu_self(v)	(v = be32_to_cpu(v))

static int msm_batt_get_batt_chg_status(void)
{
	int rc;

	struct rpc_req_batt_chg {
		struct rpc_request_hdr hdr;
		u32 more_data;
	} req_batt_chg;
	struct rpc_reply_batt_chg_v1 *v1p;

	req_batt_chg.more_data = cpu_to_be32(1);

	memset(&rep_batt_chg, 0, sizeof(rep_batt_chg));

	v1p = &rep_batt_chg.v1;
	rc = msm_rpc_call_reply(msm_batt_info.chg_ep,
				ONCRPC_CHG_GET_GENERAL_STATUS_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_batt_chg, sizeof(rep_batt_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		pr_err("%s: ERROR. msm_rpc_call_reply failed! proc=%d rc=%d\n",
		       __func__, ONCRPC_CHG_GET_GENERAL_STATUS_PROC, rc);
		return rc;
	} else if (be32_to_cpu(v1p->more_data)) {
		be32_to_cpu_self(v1p->charger_status);
		be32_to_cpu_self(v1p->charger_type);
		be32_to_cpu_self(v1p->battery_status);
		be32_to_cpu_self(v1p->battery_level);
		be32_to_cpu_self(v1p->battery_voltage);
		be32_to_cpu_self(v1p->battery_temp);
#ifdef CONFIG_LGE_FUEL_GAUGE
		be32_to_cpu_self(v1p->battery_capacity);
#endif
#if defined(LGE_DEV_CHARGING_CURRENT)
		be32_to_cpu_self(v1p->charging_current);
#endif
	} else {
		pr_err("%s: No battery/charger data in RPC reply\n", __func__);
		return -EIO;
	}

	return 0;
}

static void msm_batt_update_psy_status(void)
{
	static u32 unnecessary_event_count;
	u32	charger_status;
	u32	charger_type;
	u32	battery_status;
	u32	battery_level;
	u32     battery_voltage;
	u32	battery_temp;
#ifdef CONFIG_LGE_FUEL_GAUGE
	u32	battery_capacity;
#endif
/* 2012-12-18, jikhwan.jeong@lge.com, [V7][Power][Common] Add battery_charging_current kernel debug log. [START] */
#ifdef LGE_DEV_CHARGING_CURRENT
	u32	battery_charging_current;
#endif
/* 2012-12-18, jikhwan.jeong@lge.com, [V7][Power][Common] Add battery_charging_current kernel debug log. [END] */
	struct	power_supply	*supp;

        pr_debug("%s: enter\n", __func__);

#if defined(CONFIG_MACH_LGE)
  /* 2010-12-14 by baborobo@lge.com
   * to check the updating-status
   */
        if (msm_batt_info.is_run_batt_update == true){
		  printk(KERN_ERR "BATT:EXIT %s \n",__func__);
                return;
        }

        msm_batt_info.is_run_batt_update = true;
        if (msm_batt_get_batt_chg_status())     {
                msm_batt_info.is_run_batt_update = false;
                return;
        }
#else
	if (msm_batt_get_batt_chg_status())
		return;
#endif

	charger_status = rep_batt_chg.v1.charger_status;
	charger_type = rep_batt_chg.v1.charger_type;
	battery_status = rep_batt_chg.v1.battery_status;
	battery_level = rep_batt_chg.v1.battery_level;
	battery_voltage = rep_batt_chg.v1.battery_voltage;
	battery_temp = rep_batt_chg.v1.battery_temp;
#ifdef CONFIG_LGE_FUEL_GAUGE
	//battery_capacity = msm_batt_get_vbatt_capacity();
	battery_capacity = rep_batt_chg.v1.battery_capacity;
#endif
/* 2012-12-18, jikhwan.jeong@lge.com, [V7][Power][Common] Add battery_charging_current kernel debug log. [START] */
#ifdef LGE_DEV_CHARGING_CURRENT
	battery_charging_current = rep_batt_chg.v1.charging_current;
#endif
/* 2012-12-18, jikhwan.jeong@lge.com, [V7][Power][Common] Add battery_charging_current kernel debug log. [END] */

#ifdef CONFIG_LGE_LOW_VOLTAGE_BATTERY_CHECK
	if ((msm_batt_info.low_vbatt_check )
		&& (battery_status != BATTERY_STATUS_INVALID_v1)
		&& (battery_voltage <= msm_batt_info.voltage_min_design)
		&& (battery_voltage < msm_batt_info.battery_voltage)
		&& (!msm_batt_info.current_chg_source)) {

		printk(KERN_ERR "BATT: MIN VOLATGE,POWER OFF %d %d %d %d\n",
				battery_status,
				battery_voltage,
				msm_batt_info.battery_voltage,
				msm_batt_info.current_chg_source);

		msm_batt_info.power_off_device();
		return;
	}	
	msm_batt_info.low_vbatt_check=false;
#endif

#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_V1)
        if (battery_status == BATTERY_STATUS_REMOVED) {
                /* LGE_CHANGE : 2011-02-17 baborobo@lge.com
                 * for Battery Remove status
                 */
                battery_status = BATTERY_STATUS_INVALID;
                battery_level = BATTERY_LEVEL_INVALID;
        }
#else
	/* Make correction for battery status */
	if (battery_status == BATTERY_STATUS_INVALID_v1) {
		if (msm_batt_info.chg_api_version < CHG_RPC_VER_3_1)
			battery_status = BATTERY_STATUS_INVALID;
	}
#endif
	if (charger_status == msm_batt_info.charger_status &&
	    charger_type == msm_batt_info.charger_type &&
	    battery_status == msm_batt_info.battery_status &&
	    battery_level == msm_batt_info.battery_level &&
	    battery_voltage == msm_batt_info.battery_voltage &&
#ifdef CONFIG_LGE_FUEL_GAUGE
		battery_capacity == msm_batt_info.batt_capacity &&
#endif
	    battery_temp == msm_batt_info.battery_temp) {
		/* Got unnecessary event from Modem PMIC VBATT driver.
		 * Nothing changed in Battery or charger status.
		 */
		unnecessary_event_count++;
		pr_debug("BATT: same event count = %u\n",
				 unnecessary_event_count);
#if defined(CONFIG_MACH_LGE)
    /* 2010-12-14 by baborobo@lge.com
     * to check the updating-status
     */
          msm_batt_info.is_run_batt_update = false;
#endif
		return;
	}

	unnecessary_event_count = 0;

#ifdef CONFIG_LGE_FUEL_GAUGE
/* 2012-12-18, jikhwan.jeong@lge.com, [V7][Power][Common] Add battery_charging_current kernel debug log. [START] */
	#ifdef LGE_DEV_CHARGING_CURRENT
	pr_info("BATT: rcvd: %d, %d, %d, %d; %d, %d, %d, %d\n",
		 charger_status, charger_type, battery_status,
		 battery_level, battery_voltage, battery_temp, battery_capacity, battery_charging_current);
/* 2012-12-18, jikhwan.jeong@lge.com, [V7][Power][Common] Add battery_charging_current kernel debug log. [END] */
	#endif
#else
/* 2012-12-18, jikhwan.jeong@lge.com, [V7][Power][Common] Add battery_charging_current kernel debug log. [START] */
	#ifdef LGE_DEV_CHARGING_CURRENT
	pr_info("BATT: rcvd: %d, %d, %d, %d; %d, %d, %d\n",
		 charger_status, charger_type, battery_status,
		 battery_level, battery_voltage, battery_temp, battery_charging_current);
/* 2012-12-18, jikhwan.jeong@lge.com, [V7][Power][Common] Add battery_charging_current kernel debug log. [END] */
	#endif
#endif

	if (battery_status == BATTERY_STATUS_INVALID &&
	    battery_level != BATTERY_LEVEL_INVALID) {
		DBG_LIMIT("BATT: change status(%d) to (%d) for level=%d\n",
			 battery_status, BATTERY_STATUS_GOOD, battery_level);
		battery_status = BATTERY_STATUS_GOOD;
	}

	if (msm_batt_info.charger_type != charger_type) {
		if (charger_type == CHARGER_TYPE_USB_WALL ||
		    charger_type == CHARGER_TYPE_USB_PC ||
		    charger_type == CHARGER_TYPE_USB_CARKIT) {
			DBG_LIMIT("BATT: USB charger plugged in\n");
			msm_batt_info.current_chg_source = USB_CHG;
			supp = &msm_psy_usb;
		} else if (charger_type == CHARGER_TYPE_WALL) {
			DBG_LIMIT("BATT: AC Wall changer plugged in\n");
			msm_batt_info.current_chg_source = AC_CHG;
			supp = &msm_psy_ac;
#if defined(CONFIG_MACH_MSM7X27A_U0)
/* LGE_CHANGE_S : U0 Heating and DoU Issue
 * 2012-01-26, yoonsoo.kim@lge.com,
 * When user enter the streaming service, change the charging current
 */
                	if( (1 == i_chg_current_change_back_up) && ( 0 == i_chg_current_change ) )
                	{
                   	/*Entered You tube & connected TA*/
                        	i_chg_current_change = 1;
                        	pmic_miniabb_charging_current_change(1);
                        	i_chg_current_change_back_up = 0;
                        	printk("Changing Chg I to 400mA. Youtube ON !! \n");
                	}
/* LGE_CHANGE_E : U0 Heating and DoU Issue*/
#endif
		} else {
			if (msm_batt_info.current_chg_source & AC_CHG)
#if defined(CONFIG_MACH_MSM7X27A_U0)
                        {
#endif
				DBG_LIMIT("BATT: AC Wall charger removed\n");
#if defined(CONFIG_MACH_MSM7X27A_U0)
/* LGE_CHANGE_S : U0 Heating and DoU Issue
 * 2012-01-26, yoonsoo.kim@lge.com,
 * When user enter the streaming service, change the charging current
 */
                                if( 1 == i_chg_current_change )
                                {
                                        /*Entered Youtube with TA & removed TA*/
 /*Keep BackUp & Use it to change chg I to 400mA if TA connected again*/
                                        printk("AC Chg removed in youtube \n");
                                        i_chg_current_change = 0;
                                        i_chg_current_change_back_up = 1;
                                }
/* LGE_CHANGE_E : U0 Heating and DoU Issue*/
			}
#endif
			else if (msm_batt_info.current_chg_source & USB_CHG)
				DBG_LIMIT("BATT: USB charger removed\n");
			else
				DBG_LIMIT("BATT: No charger present\n");
			msm_batt_info.current_chg_source = 0;
			supp = &msm_psy_batt;

			/* Correct charger status */
			if (charger_status != CHARGER_STATUS_INVALID) {
				DBG_LIMIT("BATT: No charging!\n");
				charger_status = CHARGER_STATUS_INVALID;
				msm_batt_info.batt_status =
					POWER_SUPPLY_STATUS_NOT_CHARGING;
			}
		}
	} else
		supp = NULL;

	if (msm_batt_info.charger_status != charger_status) {
		if (charger_status == CHARGER_STATUS_GOOD ||
		    charger_status == CHARGER_STATUS_WEAK) {
			if (msm_batt_info.current_chg_source) {
// LGE_CHANGE_S,narasimha.chikka@lge.com, Add Full charging condition
#ifdef CONFIG_LGE_FUEL_GAUGE
				if (battery_capacity >= 100) {
					DBG_LIMIT("BATT: FULL \n");
					msm_batt_info.batt_status = POWER_SUPPLY_STATUS_FULL;
				} else {
					DBG_LIMIT("BATT: Charging.\n");
					msm_batt_info.batt_status =POWER_SUPPLY_STATUS_CHARGING;
				}

#else
				DBG_LIMIT("BATT: Charging.\n");
				msm_batt_info.batt_status =
					POWER_SUPPLY_STATUS_CHARGING;
#endif
// LGE_CHANGE_S,narasimha.chikka@lge.com, Add Full charging condition

				/* Correct when supp==NULL */
				if (msm_batt_info.current_chg_source & AC_CHG)
					supp = &msm_psy_ac;
				else
					supp = &msm_psy_usb;
			}
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_V1)
                        /* LGE_CHANGE
                        * add for unpluged status of battery
                        * 2010-04-28 baborobo@lge.com
                        */
                        if (battery_status == BATTERY_STATUS_INVALID
                                && battery_level == BATTERY_LEVEL_INVALID) {
                                DBG_LIMIT("BATT: No Battery.\n");
                                msm_batt_info.batt_status =
                            POWER_SUPPLY_STATUS_UNKNOWN;
                        }
#endif
		} else {
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_V1)
                        /* LGE_CHANGE
                        * add for unpluged status of battery
                        * 2011-02-17 baborobo@lge.com
                        */
                        if (battery_status == BATTERY_STATUS_INVALID
                                && battery_level == BATTERY_LEVEL_INVALID) {
                                DBG_LIMIT("BATT: No Battery.\n");
                                msm_batt_info.batt_status =
                            POWER_SUPPLY_STATUS_UNKNOWN;
                } else {
                        DBG_LIMIT("BATT: No charging.\n");
                        msm_batt_info.batt_status =
                                POWER_SUPPLY_STATUS_NOT_CHARGING;
                        }
#else
			DBG_LIMIT("BATT: No charging.\n");
			msm_batt_info.batt_status =
				POWER_SUPPLY_STATUS_NOT_CHARGING;
#endif
			supp = &msm_psy_batt;
		}
	} else {
		/* Correct charger status */
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_V1)
                /* LGE_CHANGE
                 * add for unpluged status of battery
                 * 2010-04-07 baborobo@lge.com
                 */
                if (battery_status == BATTERY_STATUS_INVALID
                        && battery_level == BATTERY_LEVEL_INVALID) {
                        DBG_LIMIT("BATT: No Battery\n");
                        msm_batt_info.batt_status =
                                        POWER_SUPPLY_STATUS_UNKNOWN;
                } else
#endif
		if (charger_type != CHARGER_TYPE_INVALID &&
		    charger_status == CHARGER_STATUS_GOOD) {

// LGE_CHANGE_S,narasimha.chikka@lge.com, Add Full charging condition
#ifdef CONFIG_LGE_FUEL_GAUGE
			if (battery_capacity >= 100) {
				DBG_LIMIT("BATT: FULL \n");
				msm_batt_info.batt_status = POWER_SUPPLY_STATUS_FULL;
			} else {
				DBG_LIMIT("BATT: In charging\n");
				msm_batt_info.batt_status =POWER_SUPPLY_STATUS_CHARGING;
			}

#else
			DBG_LIMIT("BATT: In charging\n");
			msm_batt_info.batt_status =
				POWER_SUPPLY_STATUS_CHARGING;
#endif
// LGE_CHANGE_E,narasimha.chikka@lge.com, Add Full charging condition

		}
	}

	/* Correct battery voltage and status */
	if (!battery_voltage) {
		if (charger_status == CHARGER_STATUS_INVALID) {
			DBG_LIMIT("BATT: Read VBATT\n");
			battery_voltage = msm_batt_get_vbatt_voltage();
		} else
			/* Use previous */
			battery_voltage = msm_batt_info.battery_voltage;
	}
	if (battery_status == BATTERY_STATUS_INVALID) {
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_V1)	
                if (battery_level != BATTERY_LEVEL_INVALID
                  && battery_voltage >= msm_batt_info.voltage_min_design
                  && battery_voltage <= msm_batt_info.voltage_max_design) {
                        DBG_LIMIT("BATT: Battery valid\n");
                        msm_batt_info.batt_valid = 1;
                        battery_status = BATTERY_STATUS_GOOD;
                }
#else
		if (battery_voltage >= msm_batt_info.voltage_min_design &&
		    battery_voltage <= msm_batt_info.voltage_max_design) {
			DBG_LIMIT("BATT: Battery valid\n");
			msm_batt_info.batt_valid = 1;
			battery_status = BATTERY_STATUS_GOOD;
		}
#endif
	}

	if (msm_batt_info.battery_status != battery_status) {
		if (battery_status != BATTERY_STATUS_INVALID) {
			msm_batt_info.batt_valid = 1;

			if (battery_status == BATTERY_STATUS_BAD) {
				DBG_LIMIT("BATT: Battery bad.\n");
				msm_batt_info.batt_health =
					POWER_SUPPLY_HEALTH_DEAD;
			} else if (battery_status == BATTERY_STATUS_BAD_TEMP) {
				DBG_LIMIT("BATT: Battery overheat.\n");
				msm_batt_info.batt_health =
					POWER_SUPPLY_HEALTH_OVERHEAT;
			} else {
				DBG_LIMIT("BATT: Battery good.\n");
				msm_batt_info.batt_health =
					POWER_SUPPLY_HEALTH_GOOD;
			}
		} else {
			msm_batt_info.batt_valid = 0;
			DBG_LIMIT("BATT: Battery invalid.\n");
			msm_batt_info.batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;
		}

#ifdef CONFIG_LGE_FUEL_GAUGE
		if (msm_batt_info.batt_status != POWER_SUPPLY_STATUS_CHARGING
			&& msm_batt_info.batt_status != POWER_SUPPLY_STATUS_FULL) {
#else
		if (msm_batt_info.batt_status != POWER_SUPPLY_STATUS_CHARGING) {
#endif
			if (battery_status == BATTERY_STATUS_INVALID) {
				DBG_LIMIT("BATT: Battery -> unknown\n");
				msm_batt_info.batt_status =
					POWER_SUPPLY_STATUS_UNKNOWN;
			} else {
				DBG_LIMIT("BATT: Battery -> discharging\n");
				msm_batt_info.batt_status =
					POWER_SUPPLY_STATUS_DISCHARGING;
			}
		}

		if (!supp) {
			if (msm_batt_info.current_chg_source) {
				if (msm_batt_info.current_chg_source & AC_CHG)
					supp = &msm_psy_ac;
				else
					supp = &msm_psy_usb;
			} else
				supp = &msm_psy_batt;
		}
	}

	msm_batt_info.charger_status = charger_status;
	msm_batt_info.charger_type = charger_type;
	msm_batt_info.battery_status = battery_status;
	msm_batt_info.battery_level = battery_level;
	msm_batt_info.battery_temp = battery_temp;

#if defined(CONFIG_MACH_MSM7X25A_V1)	
    pr_info("******* battery info *******\n");
	pr_info(" . charger_status   : %d\n", msm_batt_info.charger_status);
	pr_info(" . charger_type     : %d\n", msm_batt_info.charger_type);
	pr_info(" . battery_capacity : %d\n", msm_batt_info.batt_capacity);
	pr_info(" . battery_voltage  : %d\n", msm_batt_info.battery_voltage);
    pr_info(" . battery_temp     : %d\n", msm_batt_info.battery_temp);
	pr_info(" . battery_status     : %d\n", msm_batt_info.batt_status);
	pr_info(" . batt_health     : %d\n", msm_batt_info.batt_health);
	pr_info("****************************\n");
#endif	
#ifdef CONFIG_LGE_FUEL_GAUGE
	if (msm_batt_info.battery_voltage != battery_voltage) {
		msm_batt_info.battery_voltage = battery_voltage;
		if (!supp)
			supp = msm_batt_info.current_ps;
	}
	if (msm_batt_info.batt_capacity != battery_capacity) {
		msm_batt_info.batt_capacity = battery_capacity;
		if (!supp)
			supp = msm_batt_info.current_ps;
	}
#else
	if (msm_batt_info.battery_voltage != battery_voltage) {

		/* Android doesn't initiate shutdown even if voltage is less
		 * than minimum batt level if USB is connected. Hence report
		 * fake USB disconnection, in such scenario. Do this only when
		 * the charger is present but battery is discharging faster.
		 */
		if (battery_voltage <= msm_batt_info.voltage_min_design &&
			battery_voltage < msm_batt_info.battery_voltage &&
			msm_batt_info.charger_status == CHARGER_STATUS_GOOD) {
			pr_err("BATT: send fake USB unplug, start shutdown\n");
			msm_batt_info.current_chg_source = 0;
			supp = &msm_psy_batt;
		}

		msm_batt_info.battery_voltage  	= battery_voltage;
		msm_batt_info.batt_capacity =
			msm_batt_info.calculate_capacity(battery_voltage);
		DBG_LIMIT("BATT: voltage = %u mV [capacity = %d%%]\n",
			 battery_voltage, msm_batt_info.batt_capacity);

		if (!supp)
			supp = msm_batt_info.current_ps;
	}
#endif

#if defined(CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
	/* 2012-11-26, sungchul.jung@lge.com, fixed cut off voltage setting and update level */
	if(msm_batt_info.batt_capacity == 0 && msm_batt_info.battery_voltage >= 3400)
	{
		printk("[JSC] cut off voltage check ========================\n");
		msm_batt_info.batt_capacity = 1;
	}
	/* 2012-11-26, sungchul.jung@lge.com, fixed cut off voltage setting and update level */
#endif /* CONFIG_MACH_MSM7X25A_V3 */

	if (supp) {
		msm_batt_info.current_ps = supp;
		DBG_LIMIT("BATT: Supply = %s\n", supp->name);
		power_supply_changed(supp);
	}

#ifdef LGE_CHG_DONE_NOTIFICATION
	if(battery_capacity >= 100){
		if(false == msm_batt_info.wakeup_signal
#ifdef CONFIG_MACH_MSM8X25_V7
			&& BATTERY_LEVEL_FULL == msm_batt_info.battery_level
#endif
			){
			pr_info("BATT:FULL Soc\n");
			msm_batt_info.wakeup_signal = true;
			report_power_key();
		}
	}
	else
		msm_batt_info.wakeup_signal = false;
		
#endif
		
	pr_debug("%s: exit\n", __func__);

#if defined(CONFIG_MACH_LGE)
  /* 2010-12-14 by baborobo@lge.com
   * to check the updating-status
   */
        msm_batt_info.is_run_batt_update = false;
#endif
}

#if defined (CONFIG_LGE_CHARGER_TYPE_DETECTION) || defined (CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
void update_power_supply(int chg_type)
{
	struct	power_supply	*supp=NULL;
	
	if(chg_type == USB_CHG_TYPE__SDP)
		supp=&msm_psy_usb;
		
	else if(chg_type == USB_CHG_TYPE__WALLCHARGER)
		supp=&msm_psy_ac;
#if defined(CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
	else 
		supp = &msm_psy_batt;
#endif //CONFIG_MACH_MSM7X25A_V3	
	if(supp && supp->registered)
		power_supply_changed(supp);
}
EXPORT_SYMBOL(update_power_supply);

#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
struct batt_modify_client_req {

	u32 client_handle;

	/* The voltage at which callback (CB) should be called. */
	u32 desired_batt_voltage;

	/* The direction when the CB should be called. */
	u32 voltage_direction;

	/* The registered callback to be called when voltage and
	 * direction specs are met. */
	u32 batt_cb_id;

	/* The call back data */
	u32 cb_data;
};

struct batt_modify_client_rep {
	u32 result;
};

static int msm_batt_modify_client_arg_func(struct msm_rpc_client *batt_client,
				       void *buf, void *data)
{
	struct batt_modify_client_req *batt_modify_client_req =
		(struct batt_modify_client_req *)data;
	u32 *req = (u32 *)buf;
	int size = 0;

	*req = cpu_to_be32(batt_modify_client_req->client_handle);
	size += sizeof(u32);
	req++;

	*req = cpu_to_be32(batt_modify_client_req->desired_batt_voltage);
	size += sizeof(u32);
	req++;

	*req = cpu_to_be32(batt_modify_client_req->voltage_direction);
	size += sizeof(u32);
	req++;

	*req = cpu_to_be32(batt_modify_client_req->batt_cb_id);
	size += sizeof(u32);
	req++;

	*req = cpu_to_be32(batt_modify_client_req->cb_data);
	size += sizeof(u32);

	return size;
}

static int msm_batt_modify_client_ret_func(struct msm_rpc_client *batt_client,
				       void *buf, void *data)
{
	struct  batt_modify_client_rep *data_ptr, *buf_ptr;

	data_ptr = (struct batt_modify_client_rep *)data;
	buf_ptr = (struct batt_modify_client_rep *)buf;

	data_ptr->result = be32_to_cpu(buf_ptr->result);

	return 0;
}

static int msm_batt_modify_client(u32 client_handle, u32 desired_batt_voltage,
	     u32 voltage_direction, u32 batt_cb_id, u32 cb_data)
{
	int rc;

	struct batt_modify_client_req  req;
	struct batt_modify_client_rep rep;

	req.client_handle = client_handle;
	req.desired_batt_voltage = desired_batt_voltage;
	req.voltage_direction = voltage_direction;
	req.batt_cb_id = batt_cb_id;
	req.cb_data = cb_data;

	rc = msm_rpc_client_req(msm_batt_info.batt_client,
			BATTERY_MODIFY_CLIENT_PROC,
			msm_batt_modify_client_arg_func, &req,
			msm_batt_modify_client_ret_func, &rep,
			msecs_to_jiffies(BATT_RPC_TIMEOUT));

	if (rc < 0) {
		pr_err("%s: ERROR. failed to modify  Vbatt client\n",
		       __func__);
		return rc;
	}

	if (rep.result != BATTERY_MODIFICATION_SUCCESSFUL) {
		pr_err("%s: ERROR. modify client failed. result = %u\n",
		       __func__, rep.result);
		return -EIO;
	}

	return 0;
}

/*2012-11-13 sungchul.jung@lge.com registration suspend and resume for v3 [START] */

/* by baborobo@lge.com
* it is notthing at early-suspend / msm_batt_late_resume
* the rpc_client-setting is executed at suspend/resume
* when the phone is wake up by charger(USB or Wall-chager),
* the screen must be on-status. */

#ifdef CONFIG_LGE_BATTERY_SUSPEND_RESUME
static int msm_batt_suspend(struct platform_device *pdev, pm_message_t state)
{
	int rc;

	pr_debug("%s: enter\n", __func__);

	if (msm_batt_info.batt_handle != INVALID_BATT_HANDLE) {
		rc = msm_batt_modify_client(msm_batt_info.batt_handle,
				msm_batt_info.voltage_fail_safe,
				BATTERY_VOLTAGE_BELOW_THIS_LEVEL,
				BATTERY_CB_ID_LOW_VOL,
				msm_batt_info.voltage_fail_safe);

		if (rc < 0) {
			pr_err("%s: msm_batt_modify_client. rc=%d\n",
			       __func__, rc);
			return 0;
		}
	} else {
		pr_err("%s: ERROR. invalid batt_handle\n", __func__);
		return 0;
	}

	pr_debug("%s: exit\n", __func__);
	return 0;
}

static int msm_batt_resume(struct platform_device *pdev)
{
	int rc;

	pr_debug("%s: enter\n", __func__);

	if (msm_batt_info.batt_handle != INVALID_BATT_HANDLE) {
		rc = msm_batt_modify_client(msm_batt_info.batt_handle,
				msm_batt_info.voltage_fail_safe,
				BATTERY_ALL_ACTIVITY,
			    BATTERY_CB_ID_ALL_ACTIV,
				BATTERY_ALL_ACTIVITY);
		if (rc < 0) {
			pr_err("%s: msm_batt_modify_client FAIL rc=%d\n",
			       __func__, rc);
			return 0;
		}
	} else {
		pr_err("%s: ERROR. invalid batt_handle\n", __func__);
		return 0;
	}

/*2013-02-06 miracle.kim@lge.com add battery info update func. in resume [START] */
#if defined(CONFIG_MACH_MSM8X25_V7)
	msm_batt_update_psy_status();
#endif
/*2013-02-06 miracle.kim@lge.com add battery info update func. in resume [END] */

	pr_debug("%s: exit\n", __func__);
	return 0;
}
#else
void msm_batt_early_suspend(struct early_suspend *h)
{
	int rc;

	pr_debug("%s: enter\n", __func__);

	if (msm_batt_info.batt_handle != INVALID_BATT_HANDLE) {
		rc = msm_batt_modify_client(msm_batt_info.batt_handle,
				msm_batt_info.voltage_fail_safe,
				BATTERY_VOLTAGE_BELOW_THIS_LEVEL,
				BATTERY_CB_ID_LOW_VOL,
				msm_batt_info.voltage_fail_safe);

		if (rc < 0) {
			pr_err("%s: msm_batt_modify_client. rc=%d\n",
			       __func__, rc);
			return;
		}
	} else {
		pr_err("%s: ERROR. invalid batt_handle\n", __func__);
		return;
	}

	pr_debug("%s: exit\n", __func__);
}

void msm_batt_late_resume(struct early_suspend *h)
{
	int rc;

	pr_debug("%s: enter\n", __func__);

	if (msm_batt_info.batt_handle != INVALID_BATT_HANDLE) {
		rc = msm_batt_modify_client(msm_batt_info.batt_handle,
				msm_batt_info.voltage_fail_safe,
				BATTERY_ALL_ACTIVITY,
				BATTERY_CB_ID_ALL_ACTIV, BATTERY_ALL_ACTIVITY);
		if (rc < 0) {
			pr_err("%s: msm_batt_modify_client FAIL rc=%d\n",
			       __func__, rc);
			return;
		}
	} else {
		pr_err("%s: ERROR. invalid batt_handle\n", __func__);
		return;
	}

	msm_batt_update_psy_status();
	pr_debug("%s: exit\n", __func__);
}
#endif /*CONFIG_LGE_BATTERY_SUSPEND_RESUME*/
/*2012-11-13 sungchul.jung@lge.com registration suspend and resume for v3 [END] */


struct msm_batt_vbatt_filter_req {
	u32 batt_handle;
	u32 enable_filter;
	u32 vbatt_filter;
};

struct msm_batt_vbatt_filter_rep {
	u32 result;
};

static int msm_batt_filter_arg_func(struct msm_rpc_client *batt_client,

		void *buf, void *data)
{
	struct msm_batt_vbatt_filter_req *vbatt_filter_req =
		(struct msm_batt_vbatt_filter_req *)data;
	u32 *req = (u32 *)buf;
	int size = 0;

	*req = cpu_to_be32(vbatt_filter_req->batt_handle);
	size += sizeof(u32);
	req++;

	*req = cpu_to_be32(vbatt_filter_req->enable_filter);
	size += sizeof(u32);
	req++;

	*req = cpu_to_be32(vbatt_filter_req->vbatt_filter);
	size += sizeof(u32);
	return size;
}

static int msm_batt_filter_ret_func(struct msm_rpc_client *batt_client,
				       void *buf, void *data)
{

	struct msm_batt_vbatt_filter_rep *data_ptr, *buf_ptr;

	data_ptr = (struct msm_batt_vbatt_filter_rep *)data;
	buf_ptr = (struct msm_batt_vbatt_filter_rep *)buf;

	data_ptr->result = be32_to_cpu(buf_ptr->result);
	return 0;
}

static int msm_batt_enable_filter(u32 vbatt_filter)
{
	int rc;
	struct  msm_batt_vbatt_filter_req  vbatt_filter_req;
	struct  msm_batt_vbatt_filter_rep  vbatt_filter_rep;

	vbatt_filter_req.batt_handle = msm_batt_info.batt_handle;
	vbatt_filter_req.enable_filter = 1;
	vbatt_filter_req.vbatt_filter = vbatt_filter;

	rc = msm_rpc_client_req(msm_batt_info.batt_client,
			BATTERY_ENABLE_DISABLE_FILTER_PROC,
			msm_batt_filter_arg_func, &vbatt_filter_req,
			msm_batt_filter_ret_func, &vbatt_filter_rep,
			msecs_to_jiffies(BATT_RPC_TIMEOUT));

	if (rc < 0) {
		pr_err("%s: FAIL: enable vbatt filter. rc=%d\n",
		       __func__, rc);
		return rc;
	}

	if (vbatt_filter_rep.result != BATTERY_DEREGISTRATION_SUCCESSFUL) {
		pr_err("%s: FAIL: enable vbatt filter: result=%d\n",
		       __func__, vbatt_filter_rep.result);
		return -EIO;
	}

	pr_debug("%s: enable vbatt filter: OK\n", __func__);
	return rc;
}

struct batt_client_registration_req {
	/* The voltage at which callback (CB) should be called. */
	u32 desired_batt_voltage;

	/* The direction when the CB should be called. */
	u32 voltage_direction;

	/* The registered callback to be called when voltage and
	 * direction specs are met. */
	u32 batt_cb_id;

	/* The call back data */
	u32 cb_data;
	u32 more_data;
	u32 batt_error;
};

struct batt_client_registration_req_4_1 {
	/* The voltage at which callback (CB) should be called. */
	u32 desired_batt_voltage;

	/* The direction when the CB should be called. */
	u32 voltage_direction;

	/* The registered callback to be called when voltage and
	 * direction specs are met. */
	u32 batt_cb_id;

	/* The call back data */
	u32 cb_data;
	u32 batt_error;
};

struct batt_client_registration_rep {
	u32 batt_handle;
};

struct batt_client_registration_rep_4_1 {
	u32 batt_handle;
	u32 more_data;
	u32 err;
};

static int msm_batt_register_arg_func(struct msm_rpc_client *batt_client,
				       void *buf, void *data)
{
	struct batt_client_registration_req *batt_reg_req =
		(struct batt_client_registration_req *)data;

	u32 *req = (u32 *)buf;
	int size = 0;


	if (msm_batt_info.batt_api_version == BATTERY_RPC_VER_4_1) {
		*req = cpu_to_be32(batt_reg_req->desired_batt_voltage);
		size += sizeof(u32);
		req++;

		*req = cpu_to_be32(batt_reg_req->voltage_direction);
		size += sizeof(u32);
		req++;

		*req = cpu_to_be32(batt_reg_req->batt_cb_id);
		size += sizeof(u32);
		req++;

		*req = cpu_to_be32(batt_reg_req->cb_data);
		size += sizeof(u32);
		req++;

		*req = cpu_to_be32(batt_reg_req->batt_error);
		size += sizeof(u32);

		return size;
	} else {
		*req = cpu_to_be32(batt_reg_req->desired_batt_voltage);
		size += sizeof(u32);
		req++;

		*req = cpu_to_be32(batt_reg_req->voltage_direction);
		size += sizeof(u32);
		req++;

		*req = cpu_to_be32(batt_reg_req->batt_cb_id);
		size += sizeof(u32);
		req++;

		*req = cpu_to_be32(batt_reg_req->cb_data);
		size += sizeof(u32);
		req++;

		*req = cpu_to_be32(batt_reg_req->more_data);
		size += sizeof(u32);
		req++;

		*req = cpu_to_be32(batt_reg_req->batt_error);
		size += sizeof(u32);

		return size;
	}

}

static int msm_batt_register_ret_func(struct msm_rpc_client *batt_client,
				       void *buf, void *data)
{
	struct batt_client_registration_rep *data_ptr, *buf_ptr;
	struct batt_client_registration_rep_4_1 *data_ptr_4_1, *buf_ptr_4_1;

	if (msm_batt_info.batt_api_version == BATTERY_RPC_VER_4_1) {
		data_ptr_4_1 = (struct batt_client_registration_rep_4_1 *)data;
		buf_ptr_4_1 = (struct batt_client_registration_rep_4_1 *)buf;

		data_ptr_4_1->batt_handle
			= be32_to_cpu(buf_ptr_4_1->batt_handle);
		data_ptr_4_1->more_data
			= be32_to_cpu(buf_ptr_4_1->more_data);
		data_ptr_4_1->err = be32_to_cpu(buf_ptr_4_1->err);
		return 0;
	} else {
		data_ptr = (struct batt_client_registration_rep *)data;
		buf_ptr = (struct batt_client_registration_rep *)buf;

		data_ptr->batt_handle = be32_to_cpu(buf_ptr->batt_handle);
		return 0;
	}
}

static int msm_batt_register(u32 desired_batt_voltage,
			     u32 voltage_direction, u32 batt_cb_id, u32 cb_data)
{
	struct batt_client_registration_req batt_reg_req;
	struct batt_client_registration_req_4_1 batt_reg_req_4_1;
	struct batt_client_registration_rep batt_reg_rep;
	struct batt_client_registration_rep_4_1 batt_reg_rep_4_1;
	void *request;
	void *reply;
	int rc;

	if (msm_batt_info.batt_api_version == BATTERY_RPC_VER_4_1) {
		batt_reg_req_4_1.desired_batt_voltage = desired_batt_voltage;
		batt_reg_req_4_1.voltage_direction = voltage_direction;
		batt_reg_req_4_1.batt_cb_id = batt_cb_id;
		batt_reg_req_4_1.cb_data = cb_data;
		batt_reg_req_4_1.batt_error = 1;
		request = &batt_reg_req_4_1;
	} else {
		batt_reg_req.desired_batt_voltage = desired_batt_voltage;
		batt_reg_req.voltage_direction = voltage_direction;
		batt_reg_req.batt_cb_id = batt_cb_id;
		batt_reg_req.cb_data = cb_data;
		batt_reg_req.more_data = 1;
		batt_reg_req.batt_error = 0;
		request = &batt_reg_req;
	}

	if (msm_batt_info.batt_api_version == BATTERY_RPC_VER_4_1)
		reply = &batt_reg_rep_4_1;
	else
		reply = &batt_reg_rep;

	rc = msm_rpc_client_req(msm_batt_info.batt_client,
			BATTERY_REGISTER_PROC,
			msm_batt_register_arg_func, request,
			msm_batt_register_ret_func, reply,
			msecs_to_jiffies(BATT_RPC_TIMEOUT));

	if (rc < 0) {
		pr_err("%s: FAIL: vbatt register. rc=%d\n", __func__, rc);
		return rc;
	}

	if (msm_batt_info.batt_api_version == BATTERY_RPC_VER_4_1) {
		if (batt_reg_rep_4_1.more_data != 0
			&& batt_reg_rep_4_1.err
				!= BATTERY_REGISTRATION_SUCCESSFUL) {
			pr_err("%s: vBatt Registration Failed proc_num=%d\n"
					, __func__, BATTERY_REGISTER_PROC);
			return -EIO;
		}
		msm_batt_info.batt_handle = batt_reg_rep_4_1.batt_handle;
	} else
		msm_batt_info.batt_handle = batt_reg_rep.batt_handle;

	return 0;
}

struct batt_client_deregister_req {
	u32 batt_handle;
};

struct batt_client_deregister_rep {
	u32 batt_error;
};

static int msm_batt_deregister_arg_func(struct msm_rpc_client *batt_client,
				       void *buf, void *data)
{
	struct batt_client_deregister_req *deregister_req =
		(struct  batt_client_deregister_req *)data;
	u32 *req = (u32 *)buf;
	int size = 0;

	*req = cpu_to_be32(deregister_req->batt_handle);
	size += sizeof(u32);

	return size;
}

static int msm_batt_deregister_ret_func(struct msm_rpc_client *batt_client,
				       void *buf, void *data)
{
	struct batt_client_deregister_rep *data_ptr, *buf_ptr;

	data_ptr = (struct batt_client_deregister_rep *)data;
	buf_ptr = (struct batt_client_deregister_rep *)buf;

	data_ptr->batt_error = be32_to_cpu(buf_ptr->batt_error);

	return 0;
}

static int msm_batt_deregister(u32 batt_handle)
{
	int rc;
	struct batt_client_deregister_req req;
	struct batt_client_deregister_rep rep;

	req.batt_handle = batt_handle;

	rc = msm_rpc_client_req(msm_batt_info.batt_client,
			BATTERY_DEREGISTER_CLIENT_PROC,
			msm_batt_deregister_arg_func, &req,
			msm_batt_deregister_ret_func, &rep,
			msecs_to_jiffies(BATT_RPC_TIMEOUT));

	if (rc < 0) {
		pr_err("%s: FAIL: vbatt deregister. rc=%d\n", __func__, rc);
		return rc;
	}

	if (rep.batt_error != BATTERY_DEREGISTRATION_SUCCESSFUL) {
		pr_err("%s: vbatt deregistration FAIL. error=%d, handle=%d\n",
		       __func__, rep.batt_error, batt_handle);
		return -EIO;
	}

	return 0;
}
#endif  /* CONFIG_BATTERY_MSM_FAKE */

static int msm_batt_cleanup(void)
{
	int rc = 0;

#ifndef CONFIG_BATTERY_MSM_FAKE
	if (msm_batt_info.batt_handle != INVALID_BATT_HANDLE) {

		rc = msm_batt_deregister(msm_batt_info.batt_handle);
		if (rc < 0)
			pr_err("%s: FAIL: msm_batt_deregister. rc=%d\n",
			       __func__, rc);
	}

	msm_batt_info.batt_handle = INVALID_BATT_HANDLE;

	if (msm_batt_info.batt_client)
		msm_rpc_unregister_client(msm_batt_info.batt_client);
#endif  /* CONFIG_BATTERY_MSM_FAKE */

	if (msm_batt_info.msm_psy_ac)
		power_supply_unregister(msm_batt_info.msm_psy_ac);

	if (msm_batt_info.msm_psy_usb)
		power_supply_unregister(msm_batt_info.msm_psy_usb);
	if (msm_batt_info.msm_psy_batt)
		power_supply_unregister(msm_batt_info.msm_psy_batt);

#ifndef CONFIG_BATTERY_MSM_FAKE
	if (msm_batt_info.chg_ep) {
		rc = msm_rpc_close(msm_batt_info.chg_ep);
		if (rc < 0) {
			pr_err("%s: FAIL. msm_rpc_close(chg_ep). rc=%d\n",
			       __func__, rc);
		}
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
/*2012-11-13 sungchul.jung@lge.com did not used early_suspend and late_resume  [START] */
#ifdef CONFIG_LGE_BATTERY_SUSPEND_RESUME
//	if (msm_batt_info.early_suspend.suspend == msm_batt_early_suspend)
#else
	if (msm_batt_info.early_suspend.suspend == msm_batt_early_suspend)
#endif /*CONFIG_LGE_BATTERY_SUSPEND_RESUME*/
/*2012-11-13 sungchul.jung@lge.com did not used early_suspend and late_resume [END] */
		unregister_early_suspend(&msm_batt_info.early_suspend);
#endif
#endif
	return rc;
}

static u32 msm_batt_capacity(u32 current_voltage)
{
	u32 low_voltage = msm_batt_info.voltage_min_design;
	u32 high_voltage = msm_batt_info.voltage_max_design;

	if (current_voltage <= low_voltage)
		return 0;
	else if (current_voltage >= high_voltage)
		return 100;
	else
		return (current_voltage - low_voltage) * 100
			/ (high_voltage - low_voltage);
}

#ifndef CONFIG_BATTERY_MSM_FAKE
int msm_batt_get_charger_api_version(void)
{
	int rc ;
	struct rpc_reply_hdr *reply;

	struct rpc_req_chg_api_ver {
		struct rpc_request_hdr hdr;
		u32 more_data;
	} req_chg_api_ver;

	struct rpc_rep_chg_api_ver {
		struct rpc_reply_hdr hdr;
		u32 num_of_chg_api_versions;
		u32 *chg_api_versions;
	};

	u32 num_of_versions;

	struct rpc_rep_chg_api_ver *rep_chg_api_ver;


	req_chg_api_ver.more_data = cpu_to_be32(1);

	msm_rpc_setup_req(&req_chg_api_ver.hdr, CHG_RPC_PROG, CHG_RPC_VER_1_1,
			  ONCRPC_CHARGER_API_VERSIONS_PROC);

	rc = msm_rpc_write(msm_batt_info.chg_ep, &req_chg_api_ver,
			sizeof(req_chg_api_ver));
	if (rc < 0) {
		pr_err("%s: FAIL: msm_rpc_write. proc=0x%08x, rc=%d\n",
		       __func__, ONCRPC_CHARGER_API_VERSIONS_PROC, rc);
		return rc;
	}

	for (;;) {
		rc = msm_rpc_read(msm_batt_info.chg_ep, (void *) &reply, -1,
				BATT_RPC_TIMEOUT);
		if (rc < 0)
			return rc;
		if (rc < RPC_REQ_REPLY_COMMON_HEADER_SIZE) {
			pr_err("%s: LENGTH ERR: msm_rpc_read. rc=%d (<%d)\n",
			       __func__, rc, RPC_REQ_REPLY_COMMON_HEADER_SIZE);

			rc = -EIO;
			break;
		}
		/* we should not get RPC REQ or call packets -- ignore them */
		if (reply->type == RPC_TYPE_REQ) {
			pr_err("%s: TYPE ERR: type=%d (!=%d)\n",
			       __func__, reply->type, RPC_TYPE_REQ);
			kfree(reply);
			continue;
		}

		/* If an earlier call timed out, we could get the (no
		 * longer wanted) reply for it.	 Ignore replies that
		 * we don't expect
		 */
		if (reply->xid != req_chg_api_ver.hdr.xid) {
			pr_err("%s: XID ERR: xid=%d (!=%d)\n", __func__,
			       reply->xid, req_chg_api_ver.hdr.xid);
			kfree(reply);
			continue;
		}
		if (reply->reply_stat != RPCMSG_REPLYSTAT_ACCEPTED) {
			rc = -EPERM;
			break;
		}
		if (reply->data.acc_hdr.accept_stat !=
				RPC_ACCEPTSTAT_SUCCESS) {
			rc = -EINVAL;
			break;
		}

		rep_chg_api_ver = (struct rpc_rep_chg_api_ver *)reply;

		num_of_versions =
			be32_to_cpu(rep_chg_api_ver->num_of_chg_api_versions);

		rep_chg_api_ver->chg_api_versions =  (u32 *)
			((u8 *) reply + sizeof(struct rpc_reply_hdr) +
			sizeof(rep_chg_api_ver->num_of_chg_api_versions));

		rc = be32_to_cpu(
			rep_chg_api_ver->chg_api_versions[num_of_versions - 1]);

		pr_debug("%s: num_of_chg_api_versions = %u. "
			"The chg api version = 0x%08x\n", __func__,
			num_of_versions, rc);
		break;
	}
	kfree(reply);
	return rc;
}

static int msm_batt_cb_func(struct msm_rpc_client *client,
			    void *buffer, int in_size)
{
	int rc = 0;
	struct rpc_request_hdr *req;
	u32 procedure;
	u32 accept_status;

	req = (struct rpc_request_hdr *)buffer;
	procedure = be32_to_cpu(req->procedure);

	switch (procedure) {
	case BATTERY_CB_TYPE_PROC:
		accept_status = RPC_ACCEPTSTAT_SUCCESS;
		break;

	default:
		accept_status = RPC_ACCEPTSTAT_PROC_UNAVAIL;
		pr_err("%s: ERROR. procedure (%d) not supported\n",
		       __func__, procedure);
		break;
	}

//LGE_CHANGE_S [panchaxari.t@lge.com][to prevent vbatt dog timeout leading to modem crash][SR# 1049898]	
#ifdef LGE_VBATT_MODEM_CRASH_FIX
	msm_rpc_start_accepted_reply(client,
			be32_to_cpu(req->xid), accept_status);

	rc = msm_rpc_send_accepted_reply(client, 0);
#else
	msm_rpc_start_accepted_reply(msm_batt_info.batt_client,
			be32_to_cpu(req->xid), accept_status);

	rc = msm_rpc_send_accepted_reply(msm_batt_info.batt_client, 0);
#endif	
//LGE_CHANGE_E [panchaxari.t@lge.com][to prevent vbatt dog timeout leading to modem crash][SR# 1049898]	
	if (rc)
		pr_err("%s: FAIL: sending reply. rc=%d\n", __func__, rc);

	if (accept_status == RPC_ACCEPTSTAT_SUCCESS){
		msm_batt_update_psy_status();
#ifdef CONFIG_LGE_LOW_VOLTAGE_BATTERY_CHECK
		msm_batt_info.low_vbatt_check=true;
#endif 
	}
	return rc;
}
#endif  /* CONFIG_BATTERY_MSM_FAKE */

#if defined(CONFIG_MACH_MSM7X27A_U0)
ssize_t msm_chg_current_change_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf,"%s\n", (i_chg_current_change == 1) ? "1":"0");
}

ssize_t msm_chg_current_change_store(struct device *dev, struct device_attribute *attr, const char *buf,size_t count)
{
#if defined (CONFIG_MT9E013_LGIT)
	if(!msm_batt_info.current_chg_source)
#else
	if(AC_CHG != msm_batt_info.current_chg_source)
#endif
	{   /*If Chg source is not TA keep back up & return*/
		/*In between if TA connects use backup & change chg I to 500mA*/
		sscanf(buf, "%d", &i_chg_current_change_back_up);
		if( 0 == i_chg_current_change_back_up )
		{ /*If No CHG source,intialize chg_current_change to 0*/
			i_chg_current_change = 0;
			pmic_miniabb_charging_current_change(0);/* LGE_CHANGE  [yoonsoo.kim@lge.com]  20120508  : Charging Current Change Patch */
		}
		printk("Not a TA charger.Returning!! \n");
		return count;
	}
	sscanf(buf, "%d", &i_chg_current_change);
	if( 1 == i_chg_current_change)
	{ /* If CHG source is TA , Directly change CHH I to 500mA*/
		printk("Changing Chg I to 400mA \n");
		pmic_miniabb_charging_current_change(1);
	}
	else if ( 0 == i_chg_current_change )
	{  /* While exiting intialize back up & change it back to 700mA */
		printk("Changing Chg I to 700mA \n");
		i_chg_current_change_back_up = 0;
		pmic_miniabb_charging_current_change(0);

	}
	return count;
}
DEVICE_ATTR(chg_current_change,S_IRUGO | S_IWUSR, msm_chg_current_change_show,msm_chg_current_change_store);
/*LGE_CHANGE_E : U0 Heating and DoU Issue*/

/* LGE_CHANGE_S: [murali.ramaiah@lge.com] 2012-01-19
   sysfs interface is added to check the type of usb cable is connected to Handset.
   0 - Unknown or No cable
   1 - Normal Charger cable(180k)
   2 - Factory USB cable (56k)
   3 - Factory UART cable(130k)
*/
#ifdef CONFIG_LGE_DETECT_USB_CABLE_TYPE
static unsigned char cable_type;
static ssize_t msm_batt_cable_type_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	cable_type = lge_get_cable_info();
	return sprintf(buf,"%d\n", cable_type);
}

static DEVICE_ATTR(usb_cable, S_IRUGO, msm_batt_cable_type_show, NULL);

#endif /* CONFIG_LGE_DETECT_USB_CABLE_TYPE */
/* LGE_CHANGE_E: [murali.ramaiah@lge.com] 2012-01-19 */

//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-07-28
static unsigned pif_value;
//static unsigned low_power_mode;

static ssize_t msm_batt_pif_show(struct device* dev, struct device_attribute* attr, char* buf)
{
//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-10-20
	pif_value = lge_get_pif_info();
//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-10-20
	return sprintf(buf,"%d\n", pif_value);
}

static DEVICE_ATTR(pif, S_IRUGO, msm_batt_pif_show, NULL);

static struct attribute* dev_attrs[] = {
	&dev_attr_pif.attr,
#ifdef CONFIG_LGE_DETECT_USB_CABLE_TYPE
	&dev_attr_usb_cable.attr,
#endif
/* LGE_CHANGE_S : U0 Heating and DoU Issue
 * 2012-01-26, yoonsoo.kim@lge.com,
 * When user enter the streaming service, change the charging current
 */
	&dev_attr_chg_current_change.attr,
/*LGE_CHANGE_E : U0 Heating and DoU Issue*/
	NULL,
};

static struct attribute_group dev_attr_grp = {
	.attrs = dev_attrs,
};

#ifdef CONFIG_MACH_LGE
static unsigned batt_volt;
static unsigned chg_therm;
static unsigned pcb_version;
static unsigned chg_curr_volt;
static unsigned batt_therm;
static unsigned batt_volt_raw;
static unsigned chg_stat_reg;
static unsigned chg_en_reg;
//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-10-10
static unsigned batt_id;
//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-10-10


static ssize_t msm_batt_batt_volt_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	batt_volt = lge_get_batt_volt();
	return sprintf(buf,"%d\n", batt_volt);
}
static DEVICE_ATTR(batt_volt, S_IRUGO, msm_batt_batt_volt_show, NULL);

static ssize_t msm_batt_chg_therm_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	chg_therm = lge_get_chg_therm();
	return sprintf(buf,"%d\n", chg_therm);
}
static DEVICE_ATTR(chg_therm, S_IRUGO, msm_batt_chg_therm_show, NULL);

static ssize_t msm_batt_pcb_version_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	pcb_version = lge_get_pcb_version();
	return sprintf(buf,"%d\n", pcb_version);
}
static DEVICE_ATTR(pcb_version, S_IRUGO, msm_batt_pcb_version_show, NULL);

static ssize_t msm_batt_chg_curr_volt_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	chg_curr_volt = lge_get_chg_curr_volt();
	return sprintf(buf,"%d\n", chg_curr_volt);
}
static DEVICE_ATTR(chg_curr_volt, S_IRUGO, msm_batt_chg_curr_volt_show, NULL);

static ssize_t msm_batt_batt_therm_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	batt_therm = lge_get_batt_therm();
	return sprintf(buf,"%d\n", batt_therm);
}
static DEVICE_ATTR(batt_therm, S_IRUGO, msm_batt_batt_therm_show, NULL);

static ssize_t msm_batt_batt_volt_raw_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	batt_volt_raw = lge_get_batt_volt_raw();
	return sprintf(buf,"%d\n", batt_volt_raw);
}
static DEVICE_ATTR(batt_volt_raw, S_IRUGO, msm_batt_batt_volt_raw_show, NULL);

static ssize_t msm_batt_chg_stat_reg_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	chg_stat_reg = lge_get_chg_stat_reg();
	return sprintf(buf,"%d\n", chg_stat_reg);
}
static DEVICE_ATTR(chg_stat_reg, S_IRUGO, msm_batt_chg_stat_reg_show, NULL);


static ssize_t msm_batt_chg_en_reg_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	chg_en_reg = lge_get_chg_en_reg();
	return sprintf(buf,"%d\n", chg_en_reg);
}
static DEVICE_ATTR(chg_en_reg, S_IRUGO, msm_batt_chg_en_reg_show, NULL);

//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-10-10
static ssize_t msm_batt_batt_id_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	batt_id = lge_get_batt_id();
	return sprintf(buf,"%d\n", batt_id);
}
static DEVICE_ATTR(batt_id, S_IRUGO, msm_batt_batt_id_show, NULL);
//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-10-10

extern char* get_frst_mode(void);
static ssize_t msm_batt_frst_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	return sprintf(buf,"%s\n", get_frst_mode());
}
static DEVICE_ATTR(frst, S_IRUGO, msm_batt_frst_show, NULL);
static struct attribute* dev_attrs_lge_batt_info[] = {
	&dev_attr_batt_volt.attr,
	&dev_attr_chg_therm.attr,
	&dev_attr_pcb_version.attr,
	&dev_attr_chg_curr_volt.attr,
	&dev_attr_batt_therm.attr,
	&dev_attr_batt_volt_raw.attr,
	&dev_attr_chg_stat_reg.attr,
	&dev_attr_chg_en_reg.attr,
	//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-10-10
	&dev_attr_batt_id.attr,
	//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-10-10
	&dev_attr_frst.attr,
	NULL,
};

static struct attribute_group dev_attr_grp_lge_batt_info = {
	.attrs = dev_attrs_lge_batt_info,
};

#endif
//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-07-28
#endif //CONFIG_MACH_MSM7X27A_U0

static int __devinit msm_batt_probe(struct platform_device *pdev)
{
	int rc;
	struct msm_psy_batt_pdata *pdata = pdev->dev.platform_data;

	if (pdev->id != -1) {
		dev_err(&pdev->dev,
			"%s: MSM chipsets Can only support one"
			" battery ", __func__);
		return -EINVAL;
	}

#ifndef CONFIG_BATTERY_MSM_FAKE
	if (pdata->avail_chg_sources & AC_CHG) {
#else
	{
#endif
		rc = power_supply_register(&pdev->dev, &msm_psy_ac);
		if (rc < 0) {
			dev_err(&pdev->dev,
				"%s: power_supply_register failed"
				" rc = %d\n", __func__, rc);
			msm_batt_cleanup();
			return rc;
		}
		msm_batt_info.msm_psy_ac = &msm_psy_ac;
		msm_batt_info.avail_chg_sources |= AC_CHG;
        /* LGE_CHANGE,narasimha.chikka@lge.com,Add power supply register flag */
		msm_psy_ac.registered = true;
	}

	if (pdata->avail_chg_sources & USB_CHG) {
		rc = power_supply_register(&pdev->dev, &msm_psy_usb);
		if (rc < 0) {
			dev_err(&pdev->dev,
				"%s: power_supply_register failed"
				" rc = %d\n", __func__, rc);
			msm_batt_cleanup();
			return rc;
		}
		msm_batt_info.msm_psy_usb = &msm_psy_usb;
		msm_batt_info.avail_chg_sources |= USB_CHG;
        /* LGE_CHANGE,narasimha.chikka@lge.com,Add power supply register flag */
		msm_psy_usb.registered = true;
	}

	if (!msm_batt_info.msm_psy_ac && !msm_batt_info.msm_psy_usb) {

		dev_err(&pdev->dev,
			"%s: No external Power supply(AC or USB)"
			"is avilable\n", __func__);
		msm_batt_cleanup();
		return -ENODEV;
	}

#if defined(CONFIG_MACH_LGE)
	msm_batt_info.is_run_batt_update = false;
#endif
#ifdef CONFIG_LGE_LOW_VOLTAGE_BATTERY_CHECK
	msm_batt_info.low_vbatt_check = false;
#endif 
	msm_batt_info.voltage_max_design = pdata->voltage_max_design;
	msm_batt_info.voltage_min_design = pdata->voltage_min_design;
	msm_batt_info.voltage_fail_safe  = pdata->voltage_fail_safe;

	msm_batt_info.batt_technology = pdata->batt_technology;
	msm_batt_info.calculate_capacity = pdata->calculate_capacity;
#ifdef CONFIG_LGE_LOW_VOLTAGE_BATTERY_CHECK
	msm_batt_info.power_off_device = pdata->power_off_device;
#endif
	if (!msm_batt_info.voltage_min_design)
		msm_batt_info.voltage_min_design = BATTERY_LOW;
	if (!msm_batt_info.voltage_max_design)
		msm_batt_info.voltage_max_design = BATTERY_HIGH;
	if (!msm_batt_info.voltage_fail_safe)
		msm_batt_info.voltage_fail_safe  = BATTERY_LOW;

	if (msm_batt_info.batt_technology == POWER_SUPPLY_TECHNOLOGY_UNKNOWN)
		msm_batt_info.batt_technology = POWER_SUPPLY_TECHNOLOGY_LION;

	if (!msm_batt_info.calculate_capacity)
		msm_batt_info.calculate_capacity = msm_batt_capacity;

	rc = power_supply_register(&pdev->dev, &msm_psy_batt);
	if (rc < 0) {
		dev_err(&pdev->dev, "%s: power_supply_register failed"
			" rc=%d\n", __func__, rc);
		msm_batt_cleanup();
		return rc;
	}
	msm_batt_info.msm_psy_batt = &msm_psy_batt;

#ifndef CONFIG_BATTERY_MSM_FAKE
	rc = msm_batt_register(msm_batt_info.voltage_fail_safe,
			       BATTERY_ALL_ACTIVITY,
			       BATTERY_CB_ID_ALL_ACTIV,
			       BATTERY_ALL_ACTIVITY);
	if (rc < 0) {
		dev_err(&pdev->dev,
			"%s: msm_batt_register failed rc = %d\n", __func__, rc);
		msm_batt_cleanup();
		return rc;
	}
#if defined(CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
	msm_psy_batt.registered = true;
#endif //CONFIG_MACH_MSM7X25A_V3
	rc =  msm_batt_enable_filter(VBATT_FILTER);

	if (rc < 0) {
		dev_err(&pdev->dev,
			"%s: msm_batt_enable_filter failed rc = %d\n",
			__func__, rc);
		msm_batt_cleanup();
		return rc;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	msm_batt_info.early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
/*2012-11-13 sungchul.jung@lge.com did not used early_suspend and late_resume [START] */
#ifdef CONFIG_LGE_BATTERY_SUSPEND_RESUME
//	msm_batt_info.early_suspend.suspend = msm_batt_early_suspend;
//	msm_batt_info.early_suspend.resume = msm_batt_late_resume;
#else
	msm_batt_info.early_suspend.suspend = msm_batt_early_suspend;
	msm_batt_info.early_suspend.resume = msm_batt_late_resume;
#endif /*CONFIG_LGE_BATTERY_SUSPEND_RESUME*/
/*2012-11-13 sungchul.jung@lge.com did not used early_suspend and late_resume [END] */
	register_early_suspend(&msm_batt_info.early_suspend);
#endif
	msm_batt_update_psy_status();

#else
	power_supply_changed(&msm_psy_ac);
#endif  /* CONFIG_BATTERY_MSM_FAKE */

#if defined(CONFIG_MACH_MSM7X27A_U0)
//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-07-28
        rc = sysfs_create_group(&pdev->dev.kobj, &dev_attr_grp);
        if(rc < 0) {
                dev_err(&pdev->dev,
                        "%s: msm_batt_cleanup  failed rc=%d\n", __func__, rc);
        } else {
                pif_value = lge_get_pif_info();
        }

        dev_info(&pdev->dev, "%s : Using PIF ZIG (%d)\n", __func__, pif_value);

#ifdef CONFIG_MACH_LGE
                rc = sysfs_create_group(&pdev->dev.kobj, &dev_attr_grp_lge_batt_info);
                if(rc < 0) {
                        dev_err(&pdev->dev,
                                "%s: fail to create sysfs for lge batt info rc=%d\n", __func__, rc);
                }
#endif
//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-07-28
#endif
	return 0;
}

static int __devexit msm_batt_remove(struct platform_device *pdev)
{
	int rc;

#if defined(CONFIG_MACH_MSM7X27A_U0)
//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-07-28
        sysfs_remove_group(&pdev->dev.kobj,&dev_attr_grp);
#ifdef CONFIG_MACH_LGE
        sysfs_remove_group(&pdev->dev.kobj,&dev_attr_grp_lge_batt_info);
#endif
//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-07-28
#endif
	rc = msm_batt_cleanup();

	if (rc < 0) {
		dev_err(&pdev->dev,
			"%s: msm_batt_cleanup  failed rc=%d\n", __func__, rc);
		return rc;
	}
	return 0;
}

static struct platform_driver msm_batt_driver = {
	.probe = msm_batt_probe,
	.remove = __devexit_p(msm_batt_remove),
/*2012-11-13 sungchul.jung@lge.com registration suspend and resume for v3 [START] */
#ifdef CONFIG_LGE_BATTERY_SUSPEND_RESUME
	.suspend = msm_batt_suspend,
	.resume = msm_batt_resume,
#else
	/*nothing*/
#endif /*CONFIG_LGE_BATTERY_SUSPEND_RESUME*/
/*2012-11-13 sungchul.jung@lge.com registration suspend and resume for v3 [END] */

	.driver = {
		   .name = "msm-battery",
		   .owner = THIS_MODULE,
		   },
};

static int __devinit msm_batt_init_rpc(void)
{
	int rc;

#ifdef CONFIG_BATTERY_MSM_FAKE
	pr_info("Faking MSM battery\n");
#else

	msm_batt_info.chg_ep =
		msm_rpc_connect_compatible(CHG_RPC_PROG, CHG_RPC_VER_4_1, 0);
	msm_batt_info.chg_api_version =  CHG_RPC_VER_4_1;
	if (msm_batt_info.chg_ep == NULL) {
		pr_err("%s: rpc connect CHG_RPC_PROG = NULL\n", __func__);
		return -ENODEV;
	}

	if (IS_ERR(msm_batt_info.chg_ep)) {
		msm_batt_info.chg_ep = msm_rpc_connect_compatible(
				CHG_RPC_PROG, CHG_RPC_VER_3_1, 0);
		msm_batt_info.chg_api_version =  CHG_RPC_VER_3_1;
	}
	if (IS_ERR(msm_batt_info.chg_ep)) {
		msm_batt_info.chg_ep = msm_rpc_connect_compatible(
				CHG_RPC_PROG, CHG_RPC_VER_1_1, 0);
		msm_batt_info.chg_api_version =  CHG_RPC_VER_1_1;
	}
	if (IS_ERR(msm_batt_info.chg_ep)) {
		msm_batt_info.chg_ep = msm_rpc_connect_compatible(
				CHG_RPC_PROG, CHG_RPC_VER_1_3, 0);
		msm_batt_info.chg_api_version =  CHG_RPC_VER_1_3;
	}
	if (IS_ERR(msm_batt_info.chg_ep)) {
		msm_batt_info.chg_ep = msm_rpc_connect_compatible(
				CHG_RPC_PROG, CHG_RPC_VER_2_2, 0);
		msm_batt_info.chg_api_version =  CHG_RPC_VER_2_2;
	}
	if (IS_ERR(msm_batt_info.chg_ep)) {
		rc = PTR_ERR(msm_batt_info.chg_ep);
		pr_err("%s: FAIL: rpc connect for CHG_RPC_PROG. rc=%d\n",
		       __func__, rc);
		msm_batt_info.chg_ep = NULL;
		return rc;
	}

	/* Get the real 1.x version */
	if (msm_batt_info.chg_api_version == CHG_RPC_VER_1_1)
		msm_batt_info.chg_api_version =
			msm_batt_get_charger_api_version();

	/* Fall back to 1.1 for default */
	if (msm_batt_info.chg_api_version < 0)
		msm_batt_info.chg_api_version = CHG_RPC_VER_1_1;
#ifdef CONFIG_MACH_LGE
        // LGE_CHANGE_S, set default vbatt rpc version 5.1
	msm_batt_info.batt_api_version =  BATTERY_RPC_VER_5_1;

	msm_batt_info.batt_client =
		msm_rpc_register_client("battery", BATTERY_RPC_PROG,
					BATTERY_RPC_VER_5_1,
					1, msm_batt_cb_func);
#else /*qct original*/
	msm_batt_info.batt_api_version =  BATTERY_RPC_VER_4_1;

	msm_batt_info.batt_client =
		msm_rpc_register_client("battery", BATTERY_RPC_PROG,
					BATTERY_RPC_VER_4_1,
					1, msm_batt_cb_func);
#endif /*CONFIG_MACH_LGE*/

	if (msm_batt_info.batt_client == NULL) {
		pr_err("%s: FAIL: rpc_register_client. batt_client=NULL\n",
		       __func__);
		return -ENODEV;
	}
	if (IS_ERR(msm_batt_info.batt_client)) {
		msm_batt_info.batt_client =
			msm_rpc_register_client("battery", BATTERY_RPC_PROG,
						BATTERY_RPC_VER_1_1,
						1, msm_batt_cb_func);
		msm_batt_info.batt_api_version =  BATTERY_RPC_VER_1_1;
	}
	if (IS_ERR(msm_batt_info.batt_client)) {
		msm_batt_info.batt_client =
			msm_rpc_register_client("battery", BATTERY_RPC_PROG,
						BATTERY_RPC_VER_2_1,
						1, msm_batt_cb_func);
		msm_batt_info.batt_api_version =  BATTERY_RPC_VER_2_1;
	}
	if (IS_ERR(msm_batt_info.batt_client)) {
#ifdef CONFIG_MACH_LGE
		msm_batt_info.batt_client =
			msm_rpc_register_client("battery", BATTERY_RPC_PROG,
						BATTERY_RPC_VER_4_1,
						1, msm_batt_cb_func);
		msm_batt_info.batt_api_version =  BATTERY_RPC_VER_4_1;
#else /*qct original*/
		msm_batt_info.batt_client =
			msm_rpc_register_client("battery", BATTERY_RPC_PROG,
						BATTERY_RPC_VER_5_1,
						1, msm_batt_cb_func);
		msm_batt_info.batt_api_version =  BATTERY_RPC_VER_5_1;
#endif /*CONFIG_MACH_LGE*/
	}
	if (IS_ERR(msm_batt_info.batt_client)) {
		rc = PTR_ERR(msm_batt_info.batt_client);
		pr_err("%s: ERROR: rpc_register_client: rc = %d\n ",
		       __func__, rc);
		msm_batt_info.batt_client = NULL;
		return rc;
	}
#endif  /* CONFIG_BATTERY_MSM_FAKE */

	rc = platform_driver_register(&msm_batt_driver);

	if (rc < 0)
		pr_err("%s: FAIL: platform_driver_register. rc = %d\n",
		       __func__, rc);

	return rc;
}

static int __init msm_batt_init(void)
{
	int rc;

	pr_debug("%s: enter\n", __func__);

	rc = msm_batt_init_rpc();

	if (rc < 0) {
		pr_err("%s: FAIL: msm_batt_init_rpc.  rc=%d\n", __func__, rc);
		msm_batt_cleanup();
		return rc;
	}

	pr_info("%s: Charger/Battery = 0x%08x/0x%08x (RPC version)\n",
		__func__, msm_batt_info.chg_api_version,
		msm_batt_info.batt_api_version);

	return 0;
}

static void __exit msm_batt_exit(void)
{
	platform_driver_unregister(&msm_batt_driver);
}

module_init(msm_batt_init);
module_exit(msm_batt_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Kiran Kandi, Qualcomm Innovation Center, Inc.");
MODULE_DESCRIPTION("Battery driver for Qualcomm MSM chipsets.");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:msm_battery");
