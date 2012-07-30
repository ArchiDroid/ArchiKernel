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
 */

#ifndef __ASM__ARCH_OEM_RAPI_CLIENT_H
#define __ASM__ARCH_OEM_RAPI_CLIENT_H

/*
 * OEM RAPI CLIENT Driver header file
 */

#include <linux/types.h>
#include <mach/msm_rpcrouter.h>

enum {
	OEM_RAPI_CLIENT_EVENT_NONE = 0,

	/*
	 * list of oem rapi client events
	 */
/*LGE_CHANGE_S : seven.kim@lge.com kernel3.0 porting */
#if defined (CONFIG_LGE_SUPPORT_RAPI)
	LG_FW_RAPI_START = 100,
	LG_FW_RAPI_CLIENT_EVENT_GET_LINE_TYPE = LG_FW_RAPI_START,
	LG_FW_TESTMODE_EVENT_FROM_ARM11 = LG_FW_RAPI_START + 1,
	LG_FW_A2M_BATT_INFO_GET = LG_FW_RAPI_START + 2,
	LG_FW_A2M_PSEUDO_BATT_INFO_SET = LG_FW_RAPI_START + 3,
	LG_FW_MEID_GET = LG_FW_RAPI_START + 4,
	/* LGE_CHANGE_S
	 * SUPPORT TESTMODE FOR AIRPLAN MODE
	 * 2010-07-12 taehung.kim@lge.com
	 */
	LG_FW_SET_OPERATION_MODE = LG_FW_RAPI_START + 5,
	LG_FW_A2M_BLOCK_CHARGING_SET = LG_FW_RAPI_START + 6,
	LG_FW_MANUAL_TEST_MODE = LG_FW_RAPI_START + 8,
#ifdef CONFIG_LGE_SUPPORT_RAPI
	LGE_RPC_HANDLE_REQUEST = LG_FW_RAPI_START + 9,
#endif
#ifdef CONFIG_LGE_DLOAD_SRD
	LG_FW_REQUEST_SRD_RPC = LG_FW_RAPI_START + 10,  //kabjoo.choi
#endif

#ifdef CONFIG_LGE_DLOAD_SRD
	LG_FW_RAPI_ERI_DIAG_WRITE= LG_FW_RAPI_START + 11,   //uts dll
#endif
	LGE_REQUEST_ERI_RPC = LG_FW_RAPI_START + 12,

  OEM_RAPI_CLIENT_SET_ROOTING_NV_FROM_KERNEL =LG_FW_RAPI_START + 13,
  
  LG_FW_WEB_DLOAD_STATUS= LG_FW_RAPI_START + 14, 	
	LG_FW_SW_VERSION_GET = LG_FW_RAPI_START + 15,
	LG_FW_SUB_VERSION_GET = LG_FW_RAPI_START + 16,
//DID BACKUP  support   kabjoo.choi
	LG_FW_DID_BACKUP_REQUEST= LG_FW_RAPI_START + 17,
	
	LG_FW_OEM_RAPI_CLIENT_SRD_COMMAND = LG_FW_RAPI_START+20,  //send event 
	LG_FW_OEM_RAPI_CLIENT_SRD_COMMAND_WRITE,  //send event 
	LG_FW_OEM_RAPI_CLIENT_SRD_COMMAND_SYNC,

	LG_MSG_UNIFIEDMSGTOOL_FROM_ARM11 = 200, //#ifdef LG_SMS_PC_TEST 
#endif
/*LGE_CHANGE_E : seven.kim@lge.com kernel3.0 porting */
	OEM_RAPI_CLIENT_EVENT_MAX

};

struct oem_rapi_client_streaming_func_cb_arg {
	uint32_t  event;
	void      *handle;
	uint32_t  in_len;
	char      *input;
	uint32_t out_len_valid;
	uint32_t output_valid;
	uint32_t output_size;
};

struct oem_rapi_client_streaming_func_cb_ret {
	uint32_t *out_len;
	char *output;
};

struct oem_rapi_client_streaming_func_arg {
	uint32_t event;
	int (*cb_func)(struct oem_rapi_client_streaming_func_cb_arg *,
		       struct oem_rapi_client_streaming_func_cb_ret *);
	void *handle;
	uint32_t in_len;
	char *input;
	uint32_t out_len_valid;
	uint32_t output_valid;
	uint32_t output_size;
};

struct oem_rapi_client_streaming_func_ret {
	uint32_t *out_len;
	char *output;
};

int oem_rapi_client_streaming_function(
	struct msm_rpc_client *client,
	struct oem_rapi_client_streaming_func_arg *arg,
	struct oem_rapi_client_streaming_func_ret *ret);

int oem_rapi_client_close(void);

struct msm_rpc_client *oem_rapi_client_init(void);

#endif
