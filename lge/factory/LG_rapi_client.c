/* LGE_CHANGES LGE_RAPI_COMMANDS  */
/* Created by khlee@lge.com  
 * arch/arm/mach-msm/lge/LG_rapi_client.c
 *
 * Copyright (C) 2009 LGE, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/err.h>
#include <mach/oem_rapi_client.h>
#include <lg_diag_testmode.h>
#include <lg_diag_unifiedmsgtool.h> //#ifdef LG_SMS_PC_TEST
#include <linux/slab.h>
#include <lge_diag_eri.h>
#include <LG_rapi_client.h>

#ifdef CONFIG_LGE_DLOAD_SRD  //kabjoo.choi
#include <userDataBackUpTypeDef.h>
#include <userDataBackUpDiag.h>

#endif 
#include <lge_diag_eri.h>
#if 0
#if defined(CONFIG_MACH_MSM7X27_ALOHAV)
#include <mach/msm_battery_alohav.h>
#elif defined(CONFIG_MACH_MSM7X27_THUNDERC)
/* ADD THUNERC feature to use VS740 BATT DRIVER
 * 2010--5-13, taehung.kim@lge.com
 */
#include <mach/msm_battery_thunderc.h>
#else
#include <mach/msm_battery.h>
#endif
#endif 


/* BEGIN: 0014166 jihoon.lee@lge.com 20110116 */
/* MOD 0014166: [KERNEL] send_to_arm9 work queue */
#include <linux/kmod.h>
#include <linux/workqueue.h>
/* END: 0014166 jihoon.lee@lge.com 20110116 */

#undef LOCAL_RAPI_DBG


/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* sync up with oem_rapi */
extern uint32_t get_oem_rapi_open_cnt(void);

//static uint32_t open_count = 0; // confirm
/* END: 0014110 jihoon.lee@lge.com 20110115 */

static struct msm_rpc_client *client;

/* BEGIN: 0014166 jihoon.lee@lge.com 20110116 */
/* MOD 0014166: [KERNEL] send_to_arm9 work queue */
extern void msleep(unsigned int msecs);

static struct __send_to_arm9 send_to_arm9_data;

extern int lge_mmc_scan_partitions(void);
extern const MmcPartition *lge_mmc_find_partition_by_name(const char *name);
extern int lge_read_block(unsigned int bytes_pos, unsigned char *buf, size_t size);
extern int lge_write_block(unsigned int bytes_pos, unsigned char *buf, size_t size);
//


static void send_to_arm9_wq_func(struct work_struct *work);


char *fs_err_to_string(int err_num, char *ret_buf)
{
	pr_info("%s: err_num : %d\n", __func__, err_num);
	switch (err_num)
	{
		case -EOP:
			sprintf(ret_buf, " %d, EOP", EOP);
			break;
		case -ENOENT:
			sprintf(ret_buf, " %d, ENOENT", ENOENT);
			break;
		case -EBADF:
			sprintf(ret_buf, " %d, EBADF", EBADF);
			break;
		case -ENOMEM:
			sprintf(ret_buf, " %d, ENOMEM", ENOMEM);
			break;
		// this will be a file size in case remote_eri_rpc
		default:
			sprintf(ret_buf, " %d, no matched", err_num);
			break;
	}

	pr_info("%s: err_str : %s\n", __func__, ret_buf);
	return ret_buf;
}
//#endif
/* END: 0013860 jihoon.lee@lge.com 20110111 */

int LG_rapi_init(void)
{
	client = oem_rapi_client_init();
	if (IS_ERR(client)) {
		pr_err("%s: couldn't open oem rapi client\n", __func__);
		return PTR_ERR(client);
	}
/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* sync up with oem_rapi */
	//open_count++;
/* END: 0014110 jihoon.lee@lge.com 20110115 */

	return 0;
}

void Open_check(void)
{
/*[LGE_CHANGE_S] khyun.kim@lge.com [V7] If open count has value, occur kernel panic . */
#if 0
/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* sync up with oem_rapi */
	uint32_t open_count;
	
	/* to double check re-open; */
	open_count = get_oem_rapi_open_cnt();
	
	if(open_count > 0)
	{
#ifdef LOCAL_RAPI_DBG
		pr_info("%s,  open_count : %d \r\n", __func__, open_count);
#endif
		return;
	}

/* END: 0014110 jihoon.lee@lge.com 20110115 */
#endif
/*[LGE_CHANGE_E] khyun.kim@lge.com [V7] If open count has value, occur kernel panic . */
	LG_rapi_init();
}

/* BEGIN: 0015327 jihoon.lee@lge.com 20110204 */
/* MOD 0015327: [KERNEL] LG RAPI validity check */
int lg_rapi_check_validity_and_copy_result(void* src, char* dest, uint32 size_expected)
{
	struct oem_rapi_client_streaming_func_ret* psrc = (struct oem_rapi_client_streaming_func_ret*)src;
	int result = -1;

	// error handling - if rpc timeout occurs, page fault will be invoked
	if((psrc->output != NULL) && (psrc->out_len != NULL) && (*(psrc->out_len) > 0))
	{
		// check size overflow or underflow
		if(*(psrc->out_len) == size_expected)
		{
			memcpy((void *)dest, psrc->output, *(psrc->out_len));
			result = LG_RAPI_SUCCESS;
		}
		else
		{
//			pr_err("%s, size overflow or underflow, expected : %d, returned : %d\r\n", __func__, size_expected, *(psrc->out_len));
			memcpy((void *)dest, psrc->output,size_expected >(*(psrc->out_len))?(*(psrc->out_len)):size_expected);
			result = LG_RAPI_OVER_UNDER_FLOW;
		}
	}
	else
	{
		pr_err("%s, invalid rpc results\n", __func__);
		result = LG_RAPI_INVALID_RESPONSE;
	}

	return result;

}
/* END: 0015327 jihoon.lee@lge.com 20110204 */

//#ifdef LG_SMS_PC_TEST
static void unifiedmsgtool_send_to_arm9(void*	pReq, void* pRsp, int flag)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;

	Open_check();
	printk(KERN_INFO "%s %s start\n", __func__, (flag==NORMAL_WORK_FLAG)?"[N]":"[WQ]");

	arg.event = LG_MSG_UNIFIEDMSGTOOL_FROM_ARM11;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = 160;
	arg.input = (char*)pReq;
	arg.out_len_valid = 1;
	arg.output_valid = 1;

	ret.output = NULL;
	ret.out_len = NULL;

	rc= oem_rapi_client_streaming_function(client, &arg, &ret);

	if (rc < 0)
	{
		pr_err("%s, rapi reqeust failed\r\n", __func__);
		((diag_sms_mode_rsp_type*)pRsp)->ret_stat_code = TEST_FAIL_S;
		
	}
	else
	{
		rc = lg_rapi_check_validity_and_copy_result((void*)&ret, (char*)pRsp, arg.output_size);
		if(rc == LG_RAPI_INVALID_RESPONSE)
			((diag_sms_mode_rsp_type*)pRsp)->ret_stat_code = TEST_FAIL_S;
	}

	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);

	printk(KERN_INFO "%s %s end\n", __func__, (flag==NORMAL_WORK_FLAG)?"[N]":"[WQ]");
}

void unifiedmsgtool_do_send_to_arm9(void*	pReq, void* pRsp)
{
	unifiedmsgtool_send_to_arm9(pReq, pRsp, NORMAL_WORK_FLAG);
}
//#endif


int msm_chg_LG_cable_type(void)
{
char output[LG_RAPI_CLIENT_MAX_OUT_BUFF_SIZE]={0,};
	#if 0
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
//	char output[LG_RAPI_CLIENT_MAX_OUT_BUFF_SIZE];
	int rc= -1;
	int errCount= 0;

	Open_check();

/* LGE_CHANGES_S [younsuk.song@lge.com] 2010-09-06, Add error control code. Repeat 3 times if error occurs*/

	do 
	{
		arg.event = LG_FW_RAPI_CLIENT_EVENT_GET_LINE_TYPE;
		arg.cb_func = NULL;
		arg.handle = (void*) 0;
		arg.in_len = 0;
		arg.input = NULL;
		arg.out_len_valid = 1;
		arg.output_valid = 1;
		arg.output_size = 4;

		ret.output = NULL;
		ret.out_len = NULL;

		rc= oem_rapi_client_streaming_function(client, &arg, &ret);
	
		if (rc < 0)
			pr_err("get LG_cable_type error \r\n");
		else
			pr_info("msm_chg_LG_cable_type: %d \r\n", GET_INT32(ret.output));

	} while (rc < 0 && errCount++ < 3);

/* LGE_CHANGES_E [younsuk.song@lge.com] */

/* BEGIN: 0015327 jihoon.lee@lge.com 20110204 */
/* MOD 0015327: [KERNEL] LG RAPI validity check */
	memset(output, 0, LG_RAPI_CLIENT_MAX_OUT_BUFF_SIZE);

	rc = lg_rapi_check_validity_and_copy_result((void*)&ret, output, arg.output_size);
/* END: 0015327 jihoon.lee@lge.com 20110204 */

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
/* END: 0014591 jihoon.lee@lge.com 2011022 */
#endif 
	return (GET_INT32(output));  


}

/* BEGIN: 0014166 jihoon.lee@lge.com 20110116 */
/* MOD 0014166: [KERNEL] send_to_arm9 work queue */
static void
do_send_to_arm9(void*	pReq, void* pRsp, int flag)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;

	Open_check();
	printk(KERN_INFO "%s %s start\n", __func__, (flag==NORMAL_WORK_FLAG)?"[N]":"[WQ]");

	arg.event = LG_FW_TESTMODE_EVENT_FROM_ARM11;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(DIAG_TEST_MODE_F_req_type);
	arg.input = (char*)pReq;
	arg.out_len_valid = 1;
	arg.output_valid = 1;

	// DIAG_TEST_MODE_F_rsp_type union type is greater than the actual size, decrease it in case sensitive items
	switch(((DIAG_TEST_MODE_F_req_type*)pReq)->sub_cmd_code)
	{
		case TEST_MODE_FACTORY_RESET_CHECK_TEST:
			arg.output_size = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type);
			break;

		case TEST_MODE_TEST_SCRIPT_MODE:
			arg.output_size = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_test_script_mode_type);
			break;
		//Added by jaeopark 110527 for XO Cal Backup
		case TEST_MODE_XO_CAL_DATA_COPY:
			arg.output_size = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_XOCalDataBackup_Type);
			break;

        case TEST_MODE_MANUAL_TEST_MODE:
            arg.output_size = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_manual_test_mode_type);
            break;

        case TEST_MODE_BLUETOOTH_RW:
            arg.output_size = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_bt_addr_type);
            break;

        case TEST_MODE_WIFI_MAC_RW:
            arg.output_size = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_wifi_addr_type);
            break;
            
		default:
			arg.output_size = sizeof(DIAG_TEST_MODE_F_rsp_type);
			break;
	}

	ret.output = NULL;
	ret.out_len = NULL;

/* BEGIN: 0015327 jihoon.lee@lge.com 20110204 */
/* MOD 0015327: [KERNEL] LG RAPI validity check */
	rc= oem_rapi_client_streaming_function(client, &arg, &ret);

	if (rc < 0)
	{
		pr_err("%s, rapi reqeust failed\r\n", __func__);
		((DIAG_TEST_MODE_F_rsp_type*)pRsp)->ret_stat_code = TEST_FAIL_S;
		
	}
	else
	{
		rc = lg_rapi_check_validity_and_copy_result((void*)&ret, (char*)pRsp, arg.output_size);
		if(rc == LG_RAPI_INVALID_RESPONSE)
			((DIAG_TEST_MODE_F_rsp_type*)pRsp)->ret_stat_code = TEST_FAIL_S;
	}
/* END: 0015327 jihoon.lee@lge.com 20110204 */

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty

	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);

/* END: 0014591 jihoon.lee@lge.com 2011022 */

	printk(KERN_INFO "%s %s end\n", __func__, (flag==NORMAL_WORK_FLAG)?"[N]":"[WQ]");
}

void wait_for_main_try_finished(void)
{
	int local_cnt = 0;
#ifdef LOCAL_RAPI_DBG
	printk(KERN_INFO "%s start\n", __func__);
#endif
	do
	{
		if(GET_COMPLETE_MAIN()==1)
			break;
		
		msleep(100);
		if(++local_cnt > 50)
		{
			printk(KERN_ERR "%s wait too long : %d sec\n", __func__, (local_cnt*100/1000));
			break;
		}
	}while(1);
#ifdef LOCAL_RAPI_DBG
	printk(KERN_INFO "%s end\n", __func__);
#endif
}

void wait_for_sub_try_finished(void)
{
	int local_cnt = 0;
#ifdef LOCAL_RAPI_DBG
	printk(KERN_INFO "%s start\n", __func__);
#endif
	do
	{
		if(GET_COMPLETE_SUB()==1)
			break;
		
		msleep(100);
		if(++local_cnt > 50)
		{
			printk(KERN_ERR "%s wait too long : %d sec\n", __func__, (local_cnt*100/1000));
			break;
		}
	}while(1);
#ifdef LOCAL_RAPI_DBG
	printk(KERN_INFO "%s end\n", __func__);
#endif
}

static void
send_to_arm9_wq_func(struct work_struct *work)
{
	printk(KERN_INFO "%s, flag : %d, handle work queue\n", __func__, GET_MUTEX_FLAG() );
	//INCREASE_MUTEX_FLAG(); //increase before getting into the work queue
	wait_for_main_try_finished();
	do_send_to_arm9(send_to_arm9_data.pReq, send_to_arm9_data.pRsp, WORK_QUEUE_FLAG);
	DECREASE_MUTEX_FLAG();
	SET_COMPLETE_SUB(1);
	return;
}

void send_to_arm9(void*	pReq, void* pRsp)
{
	// sleep some time to avoid normal and work queue overlap
	msleep(100);
	
	// initialize this work queue only once
	if(send_to_arm9_wq == NULL)
	{
		printk(KERN_INFO "%s initialize work queue\n", __func__);
		send_to_arm9_wq = create_singlethread_workqueue("send_to_arm9_wq");
		INIT_WORK(&send_to_arm9_data.work, send_to_arm9_wq_func);
		SET_MUTEX_FLAG(0); // initial flag is 0
		SET_COMPLETE_MAIN(1); // initial complete flag is 1
		SET_COMPLETE_SUB(1); // initial complete flag is 1
		
	}

       if(GET_MUTEX_FLAG() == 0)
       {
#ifdef LOCAL_RAPI_DBG
       	printk(KERN_INFO "%s, flag : %d, do normal work\n", __func__, GET_MUTEX_FLAG());
#endif
       	INCREASE_MUTEX_FLAG();
		SET_COMPLETE_MAIN(0);
		wait_for_sub_try_finished();
        	do_send_to_arm9(pReq, pRsp, NORMAL_WORK_FLAG);
		SET_COMPLETE_MAIN(1);
		DECREASE_MUTEX_FLAG();
       }
	else
	{
		// hardly ever comes here in normal case.
		// Previously, it took more than 30 secs for modem factory reset, and this wq was needed at that time
		printk(KERN_INFO "%s, flag : %d, activate work queue\n", __func__, GET_MUTEX_FLAG());
		INCREASE_MUTEX_FLAG(); // increase before getting into the work queue
		SET_COMPLETE_SUB(0);
		send_to_arm9_data.pReq = pReq;
		send_to_arm9_data.pRsp = pRsp;
		queue_work(send_to_arm9_wq, &send_to_arm9_data.work);
		wait_for_sub_try_finished();
	}

	return;
}
/* END: 0014166 jihoon.lee@lge.com 20110116 */

void set_operation_mode(boolean info)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;

	Open_check();

	arg.event = LG_FW_SET_OPERATION_MODE;	
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(boolean);
	arg.input = (char*) &info;
	arg.out_len_valid = 0;
	arg.output_valid = 0;
	arg.output_size = 0;

	ret.output = (char*) NULL;
	ret.out_len = 0;

	rc = oem_rapi_client_streaming_function(client,&arg, &ret);
	if (rc < 0)
	{
		pr_err("%s, rapi reqeust failed\r\n", __func__);
	}

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty

	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);

/* END: 0014591 jihoon.lee@lge.com 2011022 */
}



#ifdef CONFIG_MACH_MSM7X27_THUNDERC
void battery_info_get(struct batt_info* resp_buf)
{

	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	uint32_t out_len;
	int ret_val;
	struct batt_info rsp_buf;

	Open_check();

	arg.event = LG_FW_A2M_BATT_INFO_GET;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = 0;
	arg.input = NULL;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(rsp_buf);

	ret.output = (char*)&rsp_buf;
	ret.out_len = &out_len;

	ret_val = oem_rapi_client_streaming_function(client, &arg, &ret);
	if(ret_val == 0) {
		resp_buf->valid_batt_id = GET_U_INT32(&rsp_buf.valid_batt_id);
		resp_buf->batt_therm = GET_U_INT32(&rsp_buf.batt_therm);
		resp_buf->batt_temp = GET_INT32(&rsp_buf.batt_temp);
	} else { /* In case error */
		resp_buf->valid_batt_id = 1; /* authenticated battery id */
		resp_buf->batt_therm = 100;  /* 100 battery therm adc */
		resp_buf->batt_temp = 30;    /* 30 degree celcius */
	}
	return;
}


void pseudo_batt_info_set(struct pseudo_batt_info_type* info)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;

	Open_check();

	arg.event = LG_FW_A2M_PSEUDO_BATT_INFO_SET;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(struct pseudo_batt_info_type);
	arg.input = (char*)info;
	arg.out_len_valid = 0;
	arg.output_valid = 0;
	arg.output_size = 0;  /* alloc memory for response */

	ret.output = (char*)NULL;
	ret.out_len = 0;

	rc = oem_rapi_client_streaming_function(client, &arg, &ret);
	if (rc < 0)
	{
		pr_err("%s, rapi reqeust failed\r\n", __func__);
	}

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
/* END: 0014591 jihoon.lee@lge.com 2011022 */
	
	return;
}

void block_charging_set(int bypass)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;

	Open_check();
	arg.event = LG_FW_A2M_BLOCK_CHARGING_SET;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(int);
	arg.input = (char*) &bypass;
	arg.out_len_valid = 0;
	arg.output_valid = 0;
	arg.output_size = 0;

	ret.output = (char*)NULL;
	ret.out_len = 0;

	rc = oem_rapi_client_streaming_function(client,&arg,&ret);
	if (rc < 0)
	{
		pr_err("%s, rapi reqeust failed\r\n", __func__);
	}

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
/* END: 0014591 jihoon.lee@lge.com 2011022 */
	
	return;
}

#endif	/* CONFIG_MACH_MSM7X27_THUNDERC */

void msm_get_MEID_type(char* sMeid)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;
	
	char temp[16];
	memset(temp,0,16); // passing argument 2 of 'memset' makes integer from pointer without a cast, change NULL to 0
	
	Open_check();

	arg.event = LG_FW_MEID_GET;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	//FIX ME : RPC_ACCEPTSTAT_GARBAGE_ARGS rpc fail
	arg.in_len = sizeof(temp);
	arg.input = temp;
//	arg.in_len = 0;
//	arg.input = NULL;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = 15;

	ret.output = NULL;
	ret.out_len = NULL;

/* BEGIN: 0015327 jihoon.lee@lge.com 20110204 */
/* MOD 0015327: [KERNEL] LG RAPI validity check */
	rc = oem_rapi_client_streaming_function(client, &arg, &ret);
	if (rc < 0)
	{
		pr_err("%s, rapi reqeust failed\r\n", __func__);
		memset(sMeid,0,14);
	}
	else
	{
		rc = lg_rapi_check_validity_and_copy_result((void*)&ret, (char*)sMeid, 14); // returned MEID size is 14
		if(rc == LG_RAPI_INVALID_RESPONSE)
			memset(sMeid,0,14);
		else
			printk(KERN_INFO "meid from modem nv : '%s'\n", sMeid);
	}
/* END: 0015327 jihoon.lee@lge.com 20110204 */

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
/* END: 0014591 jihoon.lee@lge.com 2011022 */

	return;  
}



//#ifdef CONFIG_LGE_DIAG_ERI
int eri_send_to_arm9(void* pReq, void* pRsp, unsigned int output_length)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
//	int result; 
	
	Open_check();
	
	printk(KERN_ERR "%s,LG_FW_RAPI_ERI_DIAG_WRITE : %d \n",__func__, LG_FW_RAPI_ERI_DIAG_WRITE);
	printk(KERN_ERR "%s, output_length : %d\n",__func__, output_length );
	arg.event = LG_FW_RAPI_ERI_DIAG_WRITE;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(eri_write_req_type);
	arg.input = (char*)pReq;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = output_length;

	ret.output = NULL;
	ret.out_len = NULL;
	
	oem_rapi_client_streaming_function(client, &arg, &ret);
	printk(KERN_ERR "%s, finished OEM_RAPI\n",__func__);
	printk(KERN_ERR "%s, ret.out_len : %d\n",__func__, *ret.out_len);	

	if(ret.output == NULL || ret.out_len == NULL){
		printk(KERN_ERR "%s, output is NULL\n",__func__);
		return -1;
	} else {
		memcpy(pRsp,ret.output,*ret.out_len);
		return 0;
	}

	return 0;
	
	
}

//#endif



// request eri.bin in case factory reset  
//#ifndef CONFIG_LGE_ERI_DOWNLOAD
#if !defined(CONFIG_MACH_MSM7X27A_U0)
#define ERI_FILE_PATH 	"/data/eri/eri.bin"
char eri_data[1400];
extern int eri_factory_direct_write(const char *path , char* eri_data, int size );


void remote_eri_rpc(void)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;
	int errCount=0;
	int file_size=0;
	char fs_err_buf[20];

	memset(fs_err_buf, 0, sizeof(fs_err_buf));
	Open_check();

	do
	{
		arg.event = LGE_REQUEST_ERI_RPC;
		arg.cb_func = NULL;
		arg.handle = (void*) 0;
		arg.in_len =  sizeof(fs_err_buf);
		arg.input =fs_err_buf;
		arg.out_len_valid = 1;
		arg.output_valid = 1;
		arg.output_size = sizeof(eri_data);

		ret.output = NULL;
		ret.out_len = NULL;

		rc = oem_rapi_client_streaming_function(client,&arg,&ret);

		memset(fs_err_buf, 0, sizeof(fs_err_buf));
		//if ((rc < 0) || (GET_INT32(ret.output) <=0))
		if (rc < 0)
			pr_err("%s error \r\n", __func__);
		else
		{
			pr_info("%s succeeded, file size : %s\r\n",__func__, fs_err_to_string(GET_INT32(ret.output), fs_err_buf));
			pr_info("%s succeeded\r\n",__func__);
		}
	}while (rc < 0 && errCount++ < 3);

	if(ret.output == NULL || ret.out_len == NULL){ 
		printk(KERN_ERR "%s, output is NULL\n",__func__);
		return;
	}
	memcpy((void *)eri_data, ret.output, *ret.out_len);
	file_size=(int)*ret.out_len;
	//memcpy((void *)dest, psrc->output, *(psrc->out_len));

	eri_factory_direct_write(ERI_FILE_PATH, (char *)&eri_data,file_size );
	
		

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
/* END: 0014591 jihoon.lee@lge.com 2011022 */
	
	return;
}

#endif
/* END: 0013860 jihoon.lee@lge.com 20110111 */

//DID BACKUP for dload   kabjoo.choi@lge.com  20110806
unsigned char pbuf_emmc[192]; // size of test_mode_emmc_direct_type
unsigned char pbuf_emmc2[192]; // 
void remote_did_rpc(void)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;
	
	//int file_size=0;
	char fs_err_buf[20];
	//test_mode_emmc_direct_type did_info;

	const MmcPartition *pMisc_part; 
	unsigned int did_bytes_pos_in_emmc ;
	 int mtd_op_result ;
	

	printk(KERN_ERR "%s, start OEM_RAPI\n",__func__);
	//memset(&did_info,0x0,sizeof(test_mode_emmc_direct_type));
	
	//memset(fs_err_buf, 0, sizeof(fs_err_buf));

	Open_check();

		arg.event = LG_FW_DID_BACKUP_REQUEST;
		arg.cb_func = NULL;
		arg.handle = (void*) 0;
		arg.in_len =  sizeof(fs_err_buf);
		arg.input =fs_err_buf;
		arg.out_len_valid = 1;
		arg.output_valid = 1;
		arg.output_size = sizeof(pbuf_emmc);

		ret.output = NULL;
		ret.out_len = NULL;

		rc = oem_rapi_client_streaming_function(client,&arg,&ret);
		#if 0
		memset(fs_err_buf, 0, sizeof(fs_err_buf));
		//if ((rc < 0) || (GET_INT32(ret.output) <=0))
		if (rc < 0)
			pr_err("%s error \r\n", __func__);
		else
		{
			pr_info("%s succeeded, file size : %s\r\n",__func__, fs_err_to_string(GET_INT32(ret.output), fs_err_buf));
			pr_info("%s succeeded\r\n",__func__);
		}
		#endif 
		
	if(ret.output == NULL || ret.out_len == NULL){ 
		printk(KERN_ERR "%s, output is NULL\n",__func__);
		return;
	}	

	
	printk(KERN_ERR "%s, output lenght =%dis\n",__func__,*ret.out_len);	
	//memcpy(&did_info, ret.output, *ret.out_len);
	memcpy((void *)&pbuf_emmc2[0], ret.output, 192);  //form modem  length =192

	
//===================== emmc wirte routine =======================
	printk(KERN_ERR "%s, lge_mmc_scan_partitions\n",__func__);  
	lge_mmc_scan_partitions();
	pMisc_part = lge_mmc_find_partition_by_name("misc");
	printk(KERN_ERR "%s, lge_mmc_find_partition_by_name\n",__func__);  
	if (pMisc_part ==NULL)
	{
		if (ret.output)
		kfree(ret.output);
		if (ret.out_len)
		kfree(ret.out_len);
		return ; 
	}
	
	did_bytes_pos_in_emmc = (pMisc_part->dfirstsec)*512+0x300000;  //3M 
	memset((void *)pbuf_emmc, 0, sizeof(pbuf_emmc));  
	
	mtd_op_result = lge_read_block(did_bytes_pos_in_emmc, pbuf_emmc, 192);
	//printk(KERN_ERR "%s,lge_read_block\n",__func__);  
	//if(memcmp((void *)pbuf_emmc,(void *)ret.output, sizeof(pbuf_emmc)) != 0) 
	if(memcmp((void *)pbuf_emmc,(void *)pbuf_emmc2, sizeof(pbuf_emmc)) != 0) 
	{
	  //printk(KERN_ERR "%s,lge_write_block0\n",__func__);  
	  lge_write_block(did_bytes_pos_in_emmc, pbuf_emmc2, 192);	
	 // printk(KERN_ERR "%s,lge_write_block1\n",__func__);  
	}

//=============================================================			

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
/* END: 0014591 jihoon.lee@lge.com 2011022 */
	
	return;
}



/* BEGIN: 0016311 jihoon.lee@lge.com 20110217 */
/* ADD 0016311: [POWER OFF] CALL EFS_SYNC */
#ifdef CONFIG_LGE_SUPPORT_RAPI
int remote_rpc_request(uint32_t command)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	char output[4];
	int rc= -1;
	int request_cmd = command;

	Open_check();
	arg.event = LGE_RPC_HANDLE_REQUEST;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(request_cmd);
	arg.input = (char*)&request_cmd;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(output);

	ret.output = (char*)NULL;
	ret.out_len = 0;

	rc = oem_rapi_client_streaming_function(client,&arg,&ret);
	if (rc < 0)
	{
		pr_err("%s, rapi reqeust failed\r\n", __func__);
	}
	else
	{
		rc = lg_rapi_check_validity_and_copy_result((void*)&ret, (char*)output, arg.output_size);
		if(rc == LG_RAPI_INVALID_RESPONSE)
			memset(output,0,sizeof(output));
		
		switch(command)
		{
#ifdef CONFIG_LGE_SYNC_CMD
			case LGE_SYNC_REQUEST:
				pr_info("%s, sync retry count : %d\r\n", __func__, GET_INT32(output));
				if(rc != LG_RAPI_SUCCESS)
					rc = -1;
				break;
#endif			
			default :
				break;
		}
	}
	
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
	
	return rc;
}
#endif /*CONFIG_LGE_SUPPORT_RAPI*/
/* END: 0016311 jihoon.lee@lge.com 20110217 */


#ifdef CONFIG_LGE_DLOAD_SRD 
void remote_rpc_srd_cmmand(void*pReq, void* pRsp )  //kabjoo.choi
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	//char output[4];
	int rc= -1;
	//int request_cmd = command;

	Open_check();
	arg.event = LG_FW_REQUEST_SRD_RPC;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(udbp_req_type);
	arg.input = (char*)pReq;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(udbp_rsp_type);

	ret.output = (char*)NULL;
	ret.out_len = 0;

	rc = oem_rapi_client_streaming_function(client,&arg,&ret);
	if (rc < 0)
	{
		pr_err("%s, rapi reqeust failed\r\n", __func__);
		((udbp_rsp_type*)pRsp)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
		
	}

	
	else
	{
		rc = lg_rapi_check_validity_and_copy_result((void*)&ret, (char*)pRsp, arg.output_size);
		if(rc == LG_RAPI_INVALID_RESPONSE)
			((udbp_rsp_type*)pRsp)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
	}


	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
	
	return ;
}


//CSFB SRD
void remote_rpc_with_mdm(uint32 in_len, byte *input, uint32 *out_len, byte *output)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;

	printk(KERN_ERR "%s, start OEM_RAPI\n",__func__);

	Open_check();

	arg.event = LG_FW_OEM_RAPI_CLIENT_SRD_COMMAND;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len =  in_len;
	arg.input = input;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(pbuf_emmc);

	ret.output = NULL;
	ret.out_len = NULL;

	rc = oem_rapi_client_streaming_function(client,&arg,&ret);
	
	if(ret.output == NULL || ret.out_len == NULL)
	{ 
		printk(KERN_ERR "%s, output is NULL\n",__func__);
		return;
	}
	
	printk(KERN_ERR "%s, output lenght =%dis\n",__func__,*ret.out_len);
	*out_len = *ret.out_len;
	memcpy(output, ret.output, *ret.out_len);

//=============================================================			

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
/* END: 0014591 jihoon.lee@lge.com 2011022 */
	
	return;
}

void remote_rpc_with_mdm_nv_write(uint32 in_len, byte *input, uint32 *out_len, byte *output)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;

	printk(KERN_ERR "%s, start OEM_RAPI\n",__func__);

	Open_check();

	arg.event = LG_FW_OEM_RAPI_CLIENT_SRD_COMMAND_WRITE;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len =  in_len;
	arg.input = input;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(pbuf_emmc);

	ret.output = NULL;
	ret.out_len = NULL;

	rc = oem_rapi_client_streaming_function(client,&arg,&ret);
	
	if(ret.output == NULL || ret.out_len == NULL)
	{ 
		printk(KERN_ERR "%s, output is NULL\n",__func__);
		return;
	}
	
	printk(KERN_ERR "%s, output lenght =%dis\n",__func__,*ret.out_len);	
	memcpy(output, ret.output, *ret.out_len);

//=============================================================			

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
/* END: 0014591 jihoon.lee@lge.com 2011022 */
	
	return;
}

void remote_rpc_with_mdm_nv_sync(uint32 in_len, byte *input, uint32 *out_len, byte *output)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;

	printk(KERN_ERR "%s, start OEM_RAPI\n",__func__);

	Open_check();

	arg.event = LG_FW_OEM_RAPI_CLIENT_SRD_COMMAND_SYNC;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len =  in_len;
	arg.input = input;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(pbuf_emmc);

	ret.output = NULL;
	ret.out_len = NULL;

	rc = oem_rapi_client_streaming_function(client,&arg,&ret);
	
	if(ret.output == NULL || ret.out_len == NULL)
	{ 
		printk(KERN_ERR "%s, output is NULL\n",__func__);
		return;
	}
	
	printk(KERN_ERR "%s, output lenght =%dis\n",__func__,*ret.out_len);	
	memcpy(output, ret.output, *ret.out_len);

//=============================================================			

/* BEGIN: 0014591 jihoon.lee@lge.com 20110122 */
/* MOD 0014591: [LG_RAPI] rpc request heap leakage bug fix */
	// free received buffers if it is not empty
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
/* END: 0014591 jihoon.lee@lge.com 2011022 */
	
	return;
}

#endif 

// LGE_START 20121113 seonbeom.lee [Security] support NTCODE max 40 .
static int remote_large_data_ntcode_read(unsigned int pkt_no, void* data, int* recv_len)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;

	int retval = -1;

	rpc_large_read_pkt_recv *recv_pkt;
	rpc_large_read_pkt_send send_pkt;

	send_pkt.pkt_no = pkt_no;

	arg.event = LG_COMMON_LARGE_DATA_RPC_NTCODE_TX;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(send_pkt);
	arg.input = (void*) &send_pkt;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(rpc_large_read_pkt_recv);

	ret.output = NULL;
	ret.out_len = NULL;

	if (oem_rapi_client_streaming_function(client, &arg, &ret) != 0) {
		pr_err(	" *** oem_rapi_client_streaming_function error..!!\n");
		goto __exit__;
	}

	if (ret.output == NULL || ret.out_len == NULL) {
		pr_err(" *** output is NULL\n");
		goto __exit__;
	}

	recv_pkt = (rpc_large_read_pkt_recv*) ret.output;

	if (recv_pkt->pkt_no != pkt_no ||
	recv_pkt->flag == RPC_LARGE_READ_ERROR ||
	recv_pkt->data_len == 0 ||
	recv_pkt->data_len > 120) {
	pr_err(" *** recv_pkt (%d/%d/%d/%d)\n", recv_pkt->pkt_no, pkt_no, recv_pkt->flag, recv_pkt->data_len);
	goto __exit__;
	}

	memcpy(data, recv_pkt->data, recv_pkt->data_len);
	*recv_len = recv_pkt->data_len;
	retval = recv_pkt->flag;
	
__exit__:
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
	return retval;
	
}

int ntcode_rpc_read_string(char* data)
{
	int i, ret, status = 0;
	int total_len = 0;
	char* offset = data;

	Open_check();

	for (i = 0; status != RPC_LARGE_READ_FINISH; i++)
	{
		pr_info(" *** ntcode_rpc_read_string loop (%d)\n", i);
		status = remote_large_data_ntcode_read(i, offset, &ret);
		if (status < 0) {
			pr_err(" *** ntcode_rpc_read_string error..!!\n");
			return -1;
		}
		offset += ret;
		total_len +=ret;
	}

	data[total_len-1] = '\0';
	pr_info(" *** ntcode_rpc_read_string Done(%d bytes)..!!\n", total_len);
	return total_len;
}

static int remote_large_data_ntcode_write(unsigned int pkt_no,unsigned short flag, void* data, int* send_len)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;

	int retval = -1;

	rpc_large_write_pkt_recv *recv_pkt;
	rpc_large_write_pkt_send send_pkt;

	send_pkt.pkt_no = pkt_no;
	send_pkt.flag = flag;
	send_pkt.data_len = *send_len;
	memcpy(send_pkt.data, data, send_pkt.data_len);

	arg.event = LG_COMMON_LARGE_DATA_RPC_NTCODE_RX;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(send_pkt);
	arg.input = (void*) &send_pkt;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(rpc_large_write_pkt_recv);

	ret.output = NULL;
	ret.out_len = NULL;

	//debug
	pr_err(" *** send_pkt (%d,%d,%d)", pkt_no, flag, *send_len);
	if (oem_rapi_client_streaming_function(client, &arg, &ret) != 0) {
		pr_err(	" *** oem_rapi_client_streaming_function error..!!\n");
		goto __exit__;
	}

	if (ret.output == NULL || ret.out_len == NULL) {
		pr_err(" *** output is NULL\n");
		goto __exit__;
	}

	recv_pkt = (rpc_large_write_pkt_recv*) ret.output;

	if (recv_pkt->pkt_no != pkt_no )
	{
	pr_err(" *** recv_pkt (%d/%d)\n", recv_pkt->pkt_no, pkt_no);
	goto __exit__;
	}
	
	retval = 1;
	
__exit__:
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
	return retval;
	
}

int ntcode_rpc_write_string(char* data, int data_len)
{
	int i, ret, status = 0;
	int total_len = 0;
	char* offset = data;
	int total_pkt, last_pkt_len = 0;

	Open_check();

	total_len = data_len;
	total_pkt = (total_len / 120) + (total_len % 120 == 0 ? 0 : 1);
	last_pkt_len = (total_len % 120) + (total_len % 120 == 0 ? 120 : 0);
	
	for (i = 0; i < total_pkt; i++)
	{
		if(total_pkt-1 == i) //last pkt
		{
			ret = last_pkt_len;
			status = RPC_LARGE_READ_FINISH;
		}
		else
		{
			ret = 120;
			status = RPC_LARGE_READ_CONTINUE;
		}
			
		pr_info(" *** ntcode_rpc_read_string loop (%d / %d)\n", i, total_pkt);
		if(remote_large_data_ntcode_write(i, status, offset, &ret) <0 )
		{
			pr_err(" *** ntcode_rpc_read_string error..!!\n");
			return -1;
		}
		offset += ret;
	}

	pr_info(" *** ntcode_rpc_write_string Done(%d bytes)..!!\n", total_len);
	return 1;
}
static int remote_large_vslt_send_data(unsigned int pkt_no,unsigned short flag, void* data, int* send_len)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;

	int retval = -1;

	rpc_large_write_pkt_recv *recv_pkt;
	rpc_large_write_pkt_send send_pkt;

	send_pkt.pkt_no = pkt_no;
	send_pkt.flag = flag;
	send_pkt.data_len = *send_len;
	memcpy(send_pkt.data, data, send_pkt.data_len);

	arg.event = LG_COMMON_LARGE_DATA_RPC_VSLT_RX;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(send_pkt);
	arg.input = (void*) &send_pkt;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(rpc_large_write_pkt_recv);

	ret.output = NULL;
	ret.out_len = NULL;

	//debug
	pr_err(" *** send_pkt (%d,%d,%d)", pkt_no, flag, *send_len);
	if (oem_rapi_client_streaming_function(client, &arg, &ret) != 0) {
		pr_err(	" *** oem_rapi_client_streaming_function error..!!\n");
		goto __exit__;
	}

	if (ret.output == NULL || ret.out_len == NULL) {
		pr_err(" *** output is NULL\n");
		goto __exit__;
	}

	recv_pkt = (rpc_large_write_pkt_recv*) ret.output;

	if (recv_pkt->pkt_no != pkt_no )
	{
	pr_err(" *** recv_pkt (%d/%d)\n", recv_pkt->pkt_no, pkt_no);
	goto __exit__;
	}
	
	retval = 1;
	
__exit__:
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
	return retval;
	
}


static int remote_large_vslt_response_data(unsigned int pkt_no, void* data, int* recv_len)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;

	int retval = -1;

	rpc_large_read_pkt_recv *recv_pkt;
	rpc_large_read_pkt_send send_pkt;

	send_pkt.pkt_no = pkt_no;

	arg.event = LG_COMMON_LARGE_DATA_RPC_VSLT_TX;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len = sizeof(send_pkt);
	arg.input = (void*) &send_pkt;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(rpc_large_read_pkt_recv);

	ret.output = NULL;
	ret.out_len = NULL;

	if (oem_rapi_client_streaming_function(client, &arg, &ret) != 0) {
		pr_err(	" *** oem_rapi_client_streaming_function error..!!\n");
		goto __exit__;
	}

	if (ret.output == NULL || ret.out_len == NULL) {
		pr_err(" *** output is NULL\n");
		goto __exit__;
	}

	recv_pkt = (rpc_large_read_pkt_recv*) ret.output;

	if (recv_pkt->pkt_no != pkt_no ||
	recv_pkt->flag == RPC_LARGE_READ_ERROR ||
	recv_pkt->data_len == 0 ||
	recv_pkt->data_len > 120) {
	pr_err(" *** recv_pkt (%d/%d/%d/%d)\n", recv_pkt->pkt_no, pkt_no, recv_pkt->flag, recv_pkt->data_len);
	goto __exit__;
	}

	memcpy(data, recv_pkt->data, recv_pkt->data_len);
	*recv_len = recv_pkt->data_len;
	retval = recv_pkt->flag;
	
__exit__:
	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
	return retval;
	
}


int vslt_rpc_command_string(char* in_data, int data_len, char* out_data)
{
	int i, ret, status = 0;
	int total_len = 0;
	char* offset = in_data;
	int total_pkt, last_pkt_len = 0;

	Open_check();

	total_len = data_len;
	total_pkt = (total_len / 120) + (total_len % 120 == 0 ? 0 : 1);
	last_pkt_len = (total_len % 120) + (total_len % 120 == 0 ? 120 : 0);
	
	for (i = 0; i < total_pkt; i++)
	{
		if(total_pkt-1 == i) //last pkt
		{
			ret = last_pkt_len;
			status = RPC_LARGE_READ_FINISH;
		}
		else
		{
			ret = 120;
			status = RPC_LARGE_READ_CONTINUE;
		}
			
		pr_info(" *** vslt_rpc_command_string send loop (%d / %d)\n", i, total_pkt);
		if(remote_large_vslt_send_data(i, status, offset, &ret) <0 )
		{
			pr_err(" *** vslt_rpc_command_string send error..!!\n");
			return -1;
		}
		offset += ret;
	}
	status = 0;
	total_len = 0;
	ret =0;
	offset = out_data;
	
	for (i = 0; status != RPC_LARGE_READ_FINISH; i++)
	{
		pr_info(" *** vslt_rpc_command_string loop res (%d)\n", i);
		status = remote_large_vslt_response_data(i, offset, &ret);
		if (status < 0) {
			pr_err(" *** vslt_rpc_command_string res error..!!\n");
			return -1;
		}
		offset += ret;
		total_len +=ret;
	}

	out_data[total_len-1] = '\0';	

	pr_info(" *** vslt_rpc_command_string Done(%d bytes)..!!\n", total_len);
	return total_len;
}
// LGE_END 20121113 seonbeom.lee [Security] support NTCODE max 40 .

/*LGE_CHANGE_S 2012-11-28 khyun.kim@lge.com sw_version's value set to property via rapi.*/
unsigned char swv_buff[100];
char* remote_get_sw_version(void)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc= -1;

	char fs_err_buf[20];
	
	printk(KERN_ERR "%s, start OEM_RAPI\n",__func__);

	Open_check();

	arg.event = LG_FW_GET_SW_VERSION;
	arg.cb_func = NULL;
	arg.handle = (void*) 0;
	arg.in_len =  sizeof(fs_err_buf);
	arg.input =fs_err_buf;
	arg.out_len_valid = 1;
	arg.output_valid = 1;
	arg.output_size = sizeof(swv_buff);

	ret.output = NULL;
	ret.out_len = NULL;

	rc = oem_rapi_client_streaming_function(client,&arg,&ret);
		
	if(ret.output == NULL || ret.out_len == NULL){ 
		printk(KERN_ERR "%s, output is NULL\n",__func__);
		return 0;
	}	
	memcpy((void *)&swv_buff[0], ret.output, sizeof(swv_buff));
	
	printk(KERN_ERR "%s, output lenght =%dis\n",__func__,*ret.out_len);	

	if (ret.output)
		kfree(ret.output);
	if (ret.out_len)
		kfree(ret.out_len);
	
	return swv_buff;
}
/*LGE_CHANGE_E 2012-11-28 khyun.kim@lge.com sw_version's value set to property via rapi.*/

