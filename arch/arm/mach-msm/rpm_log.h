/* Copyright (c) 2010, The Linux Foundation. All rights reserved.
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

#ifndef __ARCH_ARM_MACH_MSM_RPM_LOG_H
#define __ARCH_ARM_MACH_MSM_RPM_LOG_H

#include <linux/types.h>

//MTD-SD1-Power-BH-SuspendLog-01+[
#define SUSPEND_LOG_PHY_ADDR 0x881A0000
#define NPA_RES_FILTER_NUM	6
#define RPM_MASTER_NUM 6
#define NPA_RES_CLIENT_NUM 10

typedef enum
{
	NOT_MPM_REASON_NONE = 0x0000, //reset MPM reason by leaving MPM
	NOT_MPM_REASON_PENDING_INT = 0x0001, //not enter MPM because pending interrupt
	NOT_MPM_REASON_NO_TIME = 0x0002, //not enter MPM because no enough time 
	//CORE-BH-SuspendLog-07+[
	NOT_MPM_REASON_ENTER_XO_SHUTDOWN = 0x1000, //has entered XO Shutdown
	NOT_MPM_REASON_ENTER_VDD_MIN = 0x2000, //has entered Vdd Min
	//CORE-BH-SuspendLog-07+]
} pm_not_mpm_reason;

typedef enum
{
	MASTER_APSS = 0x0001,
	MASTER_MPSS_SW = 0x0002,
	MASTER_LPASS = 0x0004,
	MASTER_RIVA = 0x0008,
	MASTER_DSPS = 0x0010,
	MASTER_MPSS_FW = 0x0020
} rpm_master_type_bitmap;

typedef enum
{
	UBER_CXO = 0x0001,
	UBER_PXO = 0x0002,
	UBER_VDD_CX = 0x0004,
	UBER_VDD_MX = 0x0008
} sleep_uber_state_bitmap;  

typedef enum
{
	LPR_RPM_HALT = 0x0001,
	LPR_XO_SHUTDOWN = 0x0002,
	LPR_VDD_MIN = 0x0004,
} sleep_lpr_state_bitmap;  


typedef enum
{
	RES_SLEEP_UBER ,
	RES_SLEEP_LPR,
	RES_CXO,
	RES_PXO,
	RES_VDD_CX,
	RES_VDD_MX ,
	RES_MAX
} rpm_shared_resource_log_type;

typedef enum
{
  NPA_NO_CLIENT          = 0x7fffffff, /* Do not create a client */
  NPA_CLIENT_RESERVED1   = (1 << 0), /* Reserved client type */
  NPA_CLIENT_RESERVED2   = (1 << 1), /* Reserved client type */
  NPA_CLIENT_CUSTOM1     = (1 << 2), /* Custom client types - can be */
  NPA_CLIENT_CUSTOM2     = (1 << 3), /* customized by resource */
  NPA_CLIENT_CUSTOM3     = (1 << 4), 
  NPA_CLIENT_CUSTOM4     = (1 << 5), 
  NPA_CLIENT_REQUIRED    = (1 << 6), /* Request must honor request */
  NPA_CLIENT_ISOCHRONOUS = (1 << 7), /* Placeholder for isochronous work model */
  NPA_CLIENT_IMPULSE     = (1 << 8), /* Poke the resource - node defines the impulse response */
  NPA_CLIENT_LIMIT_MAX   = (1 << 9), /* Set maximum value for the resource */
  NPA_CLIENT_VECTOR      = (1 << 10), /* Vector request */
  NPA_CLIENT_SUPPRESSIBLE = (1 << 11), /* Suppressible Scalar request */
  NPA_CLIENT_SUPPRESSIBLE_VECTOR = (1 << 12), /* Suppressible Vector request */
} npa_client_type;

typedef struct npa_client_info
{
	char name[128];
	npa_client_type type;
	u32 request;
} rpm_npa_client_info;

typedef struct rpm_npa_log
{
	char name[128];
	u32 active_state;
	u32 request_state;
	rpm_npa_client_info client[NPA_RES_CLIENT_NUM];
} rpm_npa_resource_log;
	  
typedef struct  suspend_log_def
{
	u32 enable_log; //enable RPM Suspend Log //CORE-BH-SuspendLog-07*
	u32 not_mpm_reason; //not enter MPM reason
	u32 not_mpm_pending_int; //store the not MPM pending interrupt

	//Following fields are recorded in sleep_perform_lpm() when suspend
	u32 sleep_known_masters; //mask bitmap of sleep monitor SPM
	u32 sleep_idle_masters; //idle bitmap of sleep monitor SPM
	u32 sleep_uber_state; //resource request bitmap of sleep monitor NPA //use npa_query() to query state of "/sleep/uber"
	u32 sleep_lpr_state; //RPM LPRM enable bitmap //use npa_query() to query state of "/sleep/lpr"
	rpm_npa_resource_log npa_resource_log[NPA_RES_FILTER_NUM]; //log NPA resource state and their client info

	//Following fields are statistics during suspend
	u32 master_bringup_count[RPM_MASTER_NUM]; //Each master bringup count
	u64 master_awake_time[RPM_MASTER_NUM]; //Each master awake time(sclk)
	u32 xo_shutdown_time; //XO Shutdown time
	u32 xo_shutdown_count; //XO Shutdown count
	u32 vdd_min_time; //VDD Min time
	u32 vdd_min_count; //VDD Min count

	//Following field is recorded in leaving MPM
	u32 mpm_wakeup_pending_int_status1; //Pending MPM wakeup interrupt status1
	u32 mpm_wakeup_pending_int_status2; //Pending MPM wakeup interrupt status2

	u16 vdd_dig_sleep_voltage_mv; //CORE-BH-SuspendLog-07+

	//CORE-BH-SuspendLog-08+[
	u8 not_mpm_detect; //Trigger ramdump when not mpn too long
	u32 not_mpm_duration_threshold; //not mpm duration in SCLK for not_mpm_dectect
	//CORE-BH-SuspendLog-08+]
}rpm_suspend_log;
//MTD-SD1-Power-BH-SuspendLog-01+]

enum {
	MSM_RPM_LOG_PAGE_INDICES,
	MSM_RPM_LOG_PAGE_BUFFER,
	MSM_RPM_LOG_PAGE_COUNT
};

struct msm_rpm_log_platform_data {
	u32 reg_offsets[MSM_RPM_LOG_PAGE_COUNT];
	u32 log_len;
	u32 log_len_mask;
	phys_addr_t phys_addr_base;
	u32 phys_size;
	void __iomem *reg_base;
};

#endif /* __ARCH_ARM_MACH_MSM_RPM_LOG_H */
