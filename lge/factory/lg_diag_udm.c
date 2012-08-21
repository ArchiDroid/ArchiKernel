/*
  Program : UDM 

  Author : khlee

  Date : 2010.01.26
*/
/* ==========================================================================
===========================================================================*/
#include <linux/module.h>
#include <lg_diag_udm.h>
#include <lg_diagcmd.h>
#include <lg_diag_testmode.h>

/* ==========================================================================
===========================================================================*/
extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);
/*==========================================================================*/

PACK (void *)LGF_Udm (
        PACK (void	*)req_pkt_ptr,	/* pointer to request packet  */
        uint16		pkt_len )		      /* length of request packet   */
{
  DIAG_UDM_MODE_req_type	*req_ptr = (DIAG_UDM_MODE_req_type *) req_pkt_ptr;
  DIAG_UDM_MODE_rsp_type	*rsp_ptr = NULL;
//  unsigned int rsp_len;

//  rsp_len = sizeof(DIAG_UDM_MODE_rsp_type);		
//  rsp_ptr = (DIAG_UDM_MODE_rsp_type *)diagpkt_alloc(DIAG_UDM_SMS_MODE, rsp_len);
//  rsp_ptr->nrb_udm_mode.sub_cmd_code = req_ptr->nrb_udm_mode.sub_cmd_code;
//  rsp_ptr->nrb_udm_mode.ret_stat_code = TEST_OK_S;

  printk(KERN_ERR "[UDM] SubCmd=<%d>\n",req_ptr->nrb_udm_mode.sub_cmd_code);

  switch( req_ptr->nrb_udm_mode.sub_cmd_code )
  {

    default:
//      rsp_ptr->nrb_udm_mode.ret_stat_code = TEST_FAIL_S;
      break;
  }

  return (rsp_ptr);	
}

EXPORT_SYMBOL(LGF_Udm); 
