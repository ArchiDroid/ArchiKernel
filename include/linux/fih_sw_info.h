/*************************************************************************************
 *
 * FIH Project
 *
 * General Description
 *
 * Linux kernel: Definitions of SW related information for all project.
 *
 */
/*************************************************************************************/
#ifndef _LINUX_FIH_SW_INFO_H
#define _LINUX_FIH_SW_INFO_H

#include "fih_hw_info.h"
/*
 * This file has definitions of SW related information for SEMC.
 * SMEC naming rule: $PartNumber_$SwRevision
 */

//MTD-BSP-LC-SMEM-00 +[
/*===========================================================================
                        FIH Shared Memory OEM info
===========================================================================*/
struct smem_oem_info
{
    unsigned int hw_id;
    char   amss_version[32];    //MTD-BSP-LC-Get_Version-00 +
#ifndef CONFIG_FIH_NICKI_SS_MR2_MODEM
    char nonHLOS_git_head[64]; //BSP-REXER-GIT-00+
    unsigned int power_on_cause; //MTD-KERNEL-DL-POC-00 // //Reserve//
#endif
};
//MTD-BSP-LC-SMEM-00 +]

//MTD-BSP-KC-HWID-00+[
#define PROJECT_ID_SHIFT_MASK 0
#define PHASE_ID_SHIFT_MASK 8
#define BAND_ID_SHIFT_MASK 16
#define SIM_ID_SHIFT_MASK 24
//MTD-BSP-KC-HWID-00+]

//MTD-BSP-LC-Reserve_Memory-00 +[
/*===========================================================================
                        FIH Reserved Memory Definition
===========================================================================*/
#define MTD_MEMORY_RESERVE_BASE 0x88100000
#define MTD_MEMORY_RESERVE_SIZE 0x300000
#define MTD_SUSPEND_LOG_BASE MTD_MEMORY_RESERVE_BASE
#define MTD_RAM_CONSOLE_ADDR  MTD_MEMORY_RESERVE_BASE
#define MTD_RAM_CONSOLE_SIZE  0x00100000
#define MTD_MLOG_ADDR         (MTD_MEMORY_RESERVE_BASE + MTD_RAM_CONSOLE_SIZE
#define MTD_MLOG_SIZE         0x00100000
#define MTD_FB_SPARE_ADDR     (MTD_MLOG_ADDR + MTD_MLOG_SIZE)
#define MTD_FB_SPARE_SIZE     0x00100000

#define SEMC_PartNumber		"S_PartNumber"
#define SEMC_SwRevison		"S_SwRevision"
#define SEMC_ReleaseFlag	"S_ReleaseFlag"
#define SEMC_BlobVersion	"S_BlobVersion"
#define SEMC_BpVersion		"S_BpVersion"
#define SEMC_SvnVersion		"S_SvnVersion"
#define SEMC_ProductName	"S_ProductModelName"

//MTD-KERNEL-DL-PWRON_CAUSE-00 +[
/*===========================================================================
                        FIH PWRON CAUSE
===========================================================================*/
#define MTD_PWR_ON_EVENT_MODEM_FATAL_ERROR      0x10000000 // modem fatal error
#define MTD_PWR_ON_EVENT_KERNEL_PANIC           0x20000000 // host panic
#define MTD_PWR_ON_EVENT_MODEM_SW_WD_RESET      0x40000000 // modem software watchdog reset
#define MTD_PWR_ON_EVENT_MODEM_FW_WD_RESET      0x80000000 // modem firmware watchdog reset
#define MTD_PWR_ON_EVENT_ABNORMAL_RESET         0x01000000 // abnormal reset
#define MTD_PWR_ON_EVENT_HW_WD_RESET            0x00100000 // hardware watchdog reset
#define MTD_PWR_ON_EVENT_SOFTWARE_RESET         0x00200000 // software reset //CORE-DL-AddPocForSwReset-00
#define MTD_PWR_ON_EVENT_PWR_OFF_CHG_REBOOT     0x00400000 // power off charging reset system //CORE-DL-FixForcePowerOn-00
#define MTD_PWR_ON_EVENT_RPM_WD_RESET           0x00800000 // rpm watchdog reset //CORE-DL-AddPocForRPM-00
#define MTD_PWR_ON_EVENT_CLEAN_DATA             0x00000000 // data is clean
//MTD-KERNEL-DL-PWRON_CAUSE-00 +]

/*===========================================================================
                        FIH RAM console definition
===========================================================================*/
/*0x88100000*/
#define RAM_CONSOLE_PHYS MTD_RAM_CONSOLE_ADDR
#define RAM_CONSOLE_SIZE 0x00020000/*128KB */

/*===========================================================================
                        FIH Other definition
===========================================================================*/
#define FIH_OTHER_DATA_BASE	(RAM_CONSOLE_PHYS + MTD_RAM_CONSOLE_SIZE)
#define CONFIG_FEATURE_FIH_SW3_PANIC_FILE

/*0x881FFFF0 Ramdump Time*/
#define   CRASH_TIME_RAMDUMP_LEN	0x10 /*16bytes*/
#define   CRASH_TIME_RAMDUMP_ADDR	(FIH_OTHER_DATA_BASE - CRASH_TIME_RAMDUMP_LEN)

/*0x881FFFEC power off charging*/
#define   SECOND_POWER_ON_LEN	0x04 /*4bytes*/
#define   SECOND_POWER_ON_ADDR	(CRASH_TIME_RAMDUMP_ADDR - SECOND_POWER_ON_LEN)
#define   SECOND_POWER_ON_SIGN	0x65971228

/*0x881FFE80 RAM panic data*/
#define PANIC_RAM_DATA_SIZE		0x16C /* 364bytes */
#define PANIC_RAM_DATA_BEGIN	(SECOND_POWER_ON_ADDR - PANIC_RAM_DATA_SIZE)
#define PANIC_RAM_SIGNATURE		0x19761105
struct fih_panic_ram_data{
  unsigned int				signature;
  unsigned int				length;
  char						data[1];
} ;

/*0x881FFDB0 RAM STORE_FATAL_ERROR_REASON*/
#define DIAG_BUFFER_LEN          0xD0
#define STORE_FATAL_ERROR_REASON (PANIC_RAM_DATA_BEGIN - DIAG_BUFFER_LEN)

//MTD-KERNEL-DL-PWRON_CAUSE-00 +[
/*0x881FFFE8 Power on cause*/
#define FIH_PWRON_CAUSE_LEN      0x04 /*4bytes*/
#define FIH_PWRON_CAUSE_ADDR     (SECOND_POWER_ON_ADDR - FIH_PWRON_CAUSE_LEN)

/*0x881FFFE4 HW watch dog*/
#define FIH_HW_WD_LEN            0x04 /*4bytes*/
#define FIH_HW_WD_ADDR           (FIH_PWRON_CAUSE_ADDR - FIH_HW_WD_LEN)
#define FIH_HW_WD_SIGNATURE      0x19850118

#define SOFTWARE_RESET           99500 //CORE-DL-AddPocForSwReset-00
#define MODEM_FATAL_ERR          99501
#define MODEM_SW_WDOG_EXPIRED    99502
#define MODEM_FW_WDOG_EXPIRED    99503
#define HOST_KERNEL_PANIC        99504
#define PWR_OFF_CHG_REBOOT       99505 //CORE-DL-FixForcePowerOn-00
//MTD-KERNEL-DL-PWRON_CAUSE-00 +]

#endif
