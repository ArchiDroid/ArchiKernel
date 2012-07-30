#include <linux/module.h>
#include <lg_diagcmd.h>
#include <linux/input.h>
#include <linux/syscalls.h>

#include <lg_diag_unifiedmsgtool.h>
/*BEGIN: 0011452 kiran.kanneganti@lge.com 2010-11-26*/
/*ADD 0011452: Noice cancellation check support for testmode*/
#include <mach/qdsp5v2/audio_def.h>
/* END: 0011452 kiran.kanneganti@lge.com 2010-11-26 */
#include <linux/delay.h>

#ifndef SKW_TEST
#include <linux/fcntl.h> 
#include <linux/fs.h>
#include <linux/uaccess.h>
#endif

//#include <mach/lg_backup_items.h>

// BEGIN : munho.lee@lge.com 2011-01-15
// ADD: 0013541: 0014142: [Test_Mode] To remove Internal memory information in External memory test when SD-card is not exist 
#include <linux/gpio.h>

extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);
extern PACK(void *) diagpkt_free (PACK(void *)pkt);
extern void unifiedmsgtool_do_send_to_arm9( void*	pReq, void	*pRsp);
/* ==========================================================================
===========================================================================*/

//#ifdef LG_SMS_PC_TEST
typedef enum {
	DIAG_SMS_TEST_TYPE_L2_PAGING_CHANNEL_WITH_CRC = 0x00,
	DIAG_SMS_TEST_TYPE_L2_TRAFFIC_CHANNEL_WITH_CRC = 0x01,
	DIAG_SMS_TEST_TYPE_L3 = 0x02,
	DIAG_SMS_TEST_TYPE_VOICEMAIL = 0x03,
	DIAG_SMS_TEST_MMSDBFULL = 0x04,
	DIAG_SMS_TEST_WAPPUSH_EXPIRED_TEST = 0x05, //spint
	DIAG_SMS_TEST_WAPPUSH_EXPIRED_1MIN_LATER_TEST = 0x06, //spint
	DIAG_SMS_TEST_MSG_COUNT = 0x07,
	DIAG_SMS_TEST_TYPE_IMS_WMS_MT = 0x08,
	DIAG_SMS_TEST_TYPE_IMS_WMS_MO = 0x09,
	DIAG_MMS_TEST_TYPE_CONFMSG = 0x0A,
	DIAG_MMS_TEST_TYPE_LOG = 0x10,
	DIAG_SMS_TEST_KEYPROCESS
}diagSmsTestType;

PACK (void *)LGF_MsgTest (
        PACK (void	*)req_pkt,	/* pointer to request packet  */
        uint16		pkt_len )		      /* length of request packet   */
{
	diag_sms_mode_rsp_type *rsp;
	byte				*req_ptr = (byte *) req_pkt;
	byte						smsTestType;  
	const unsigned int rsp_len = sizeof( diag_sms_mode_rsp_type );

	rsp = (diag_sms_mode_rsp_type *)diagpkt_alloc (DIAG_SMS_TEST_F, rsp_len);
	
	memcpy(&smsTestType, req_ptr + 1, 1);

	rsp->sub_cmd_code = 0;
	rsp->ret_stat_code = 0;
	rsp->sms_mode_rsp.retvalue = 0;

	switch(smsTestType)
	{
		case DIAG_SMS_TEST_TYPE_L2_PAGING_CHANNEL_WITH_CRC:   //  Layer2 SMS Page LOG with CRC
		case DIAG_SMS_TEST_TYPE_L2_TRAFFIC_CHANNEL_WITH_CRC:		  //  Layer2 SMS Traffic LOG with CRC
		case DIAG_SMS_TEST_TYPE_L3: //	Layer3	Normal Messagae
		case DIAG_SMS_TEST_TYPE_VOICEMAIL:	//	VoiceMail Messagae
			
			unifiedmsgtool_do_send_to_arm9((void*)req_ptr, (void*)rsp);
			break;
			
#ifdef SPRINT_MODEL
		case DIAG_SMS_TEST_WAPPUSH_EXPIRED_TEST: // SMS Wap push Expire Test
		{
			BalsmsMTWAPPushExpireTest(req_ptr,rsp, 0);
			break;				
		}
		case DIAG_SMS_TEST_WAPPUSH_EXPIRED_1MIN_LATER_TEST:
		{
			BalsmsMTWAPPushExpireTest(req_ptr,rsp, 1);
			break;
		}
#endif
	}

	return (rsp);
}
EXPORT_SYMBOL(LGF_MsgTest);


