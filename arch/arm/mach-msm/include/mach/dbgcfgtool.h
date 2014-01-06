/**+===========================================================================
  File: dbgcfgtool.h

  Description:


  Note:


  Author:
        Hebbel Chao Oct-20-2009

-------------------------------------------------------------------------------
** FIHSPEC CONFIDENTIAL
** Copyright(c) 2009 FIHSPEC Corporation. All Rights Reserved.
**
** The source code contained or described herein and all documents related
** to the source code (Material) are owned by Mobinnova Technology Corporation.
** The Material is protected by worldwide copyright and trade secret laws and
** treaty provisions. No part of the Material may be used, copied, reproduced,
** modified, published, uploaded, posted, transmitted, distributed, or disclosed
** in any way without Mobinnova prior express written permission.
============================================================================+*/
#ifndef __DBGCFGTOOL_H__
#define __DBGCFGTOOL_H__

#define DEV_IOCTLID 0xD1
#define DBG_IOCTL_CMD_HANDLE_DBGCFG _IOWR(DEV_IOCTLID, 0, dbgcfg_ioctl_arg)

typedef enum
{
    DBG_CMDLINE_PARAMETER_START = 0,
    DBG_UARTMSG_CFG = DBG_CMDLINE_PARAMETER_START,
    DBG_PRINTF_UARTMSG_CFG = 1,
    DBG_ANDROID_UARTMSG_CFG = 2,
    DBG_ANDROID_UARTMSG_MAIN_CFG = 3,
    DBG_ANDROID_UARTMSG_RADIO_CFG = 4,
    DBG_ANDROID_UARTMSG_EVENTS_CFG = 5,
    DBG_ANDROID_UARTMSG_SYSTEM_CFG = 6,
    DBG_FORCE_TRIGGER_PANIC_CFG = 7,
    DBG_RAMDUMP_TO_SDCARD_CFG = 8,  /*CORE-HC-RAMDUMP-00+*/
    DBG_SUBSYSTEM_RESTART_CFG = 9,	/*CORE-TH-SubSystemRestart-00+*/
    DBG_CMDLINE_PARAMETER_MAX = 31,
    
    DBG_XML_PARAMETER_START = 32,
    DBG_CPU_USAGE_CFG = DBG_XML_PARAMETER_START,
    DBG_TCXOSD_DISPLAY_ON_CFG = 33,
    DBG_XML_PARAMETER_MAX = 63
}dbgcfg_id;

typedef enum
{
    DBGCFG_READ = 0,
    DBGCFG_WRITE = 1
}dbgcfg_action;

typedef struct
{
    dbgcfg_id id;
    dbgcfg_action action;
    unsigned int value;
}dbgcfg_ioctl_arg;

/* Referenced from system/core/include/cutils/log.h */
typedef enum {
    LOG_ID_MAIN = 0,
    LOG_ID_RADIO = 1,
    LOG_ID_EVENTS = 2,
    LOG_ID_SYSTEM = 3,
    LOG_ID_MAX
} log_id_t;

#endif	//__DBGCFGTOOL_H__