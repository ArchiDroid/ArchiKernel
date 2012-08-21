#include "lg_diag_dload.h"

// LG_FW : 2011.07.07 moon.yongho : saving webdload status variable to eMMC. ----------[[
#ifdef LG_FW_WEB_DOWNLOAD
extern void webDload_rpc_srd_cmmand(void*pReq, void* pRsp);
extern  boolean diag_WebDL_SRD_Init(void);
// chinsu.ko@lge.com
//extern int is_batt_lvl_present(void);


extern int lge_erase_block(int secnum, size_t size);
extern int lge_write_block(int secnum, unsigned char *buf, size_t size);
extern int lge_read_block(int secnum, unsigned char *buf, size_t size);
extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);


boolean diag_Flag_Data_Init(void);



webdload_type webdload;
unsigned char si_page_buffer[BACKUP_BYTE_MAX_SIZE];
extern unsigned int web_status_bytes_pos_in_emmc;
unsigned int select_sector = (512 * 32765);			// BLB : 32768 block

boolean diag_Flag_Data_Init(void)
{
    int mtd_op_result = 0;

    /* ==========> eMMC data read   ============ */
    mtd_op_result = lge_read_block(web_status_bytes_pos_in_emmc + select_sector, si_page_buffer, BACKUP_BYTE_MAX_SIZE);

    if( mtd_op_result != BACKUP_BYTE_MAX_SIZE )
    {
		printk("%s, emmc Read error, normal return value size = 2048 : %d\n", __func__, mtd_op_result);

		return FALSE;
    }
	
    return true;
}

PACK (void *)LGE_WebDload_SRD(PACK (void *)req_pkt_ptr, uint16 pkg_len)
{
    uint16 rsp_len =0;
    byte *req_ptr = (byte*)req_pkt_ptr;
    byte *rsp_ptr = NULL;

    static char exe_flag=0;

    rsp_len = sizeof(diag_webdload_rsp_type); /* 무조건 이 size로 보낸다는 가정임 */
    rsp_ptr = (byte*)diagpkt_alloc(DIAG_WEBDLOAD_COMMON_F  , rsp_len);

    if( rsp_ptr == NULL )
		return rsp_ptr;

	if(exe_flag == 0) /* one-time execution */
    {
		if(diag_WebDL_SRD_Init()== TRUE)   /* once intializing eMMC */
		{
		    exe_flag = 1;/* once normal operation  */
		}
		else
		{/* eMMc initial operation ERROR */
		    return rsp_ptr;
		}
    }

    webdload_common_cmd((byte*)req_ptr, (byte*)rsp_ptr, CONNECT_MODE_DIAG);

    return rsp_ptr;
}

void webdload_memset(byte* buff, byte val, int len)
{
    int i;

    for( i=0; i<len; i++)    buff[i] = (byte)val;
}

/*===========================================================================
FUNCTION WEBDLOAD_COMMON_CMD
DESCRIPTION
===========================================================================*/
void webdload_common_cmd
(
 byte* cmd_buf,
 byte* rsp_buf,
 byte  mode
 )
{
    static char data_read_flag = 0;
    boolean  result = TRUE;
    int mtd_op_result = 0;
    // chinsu.ko@lge.com
    //int batt_state = 0;
    webdload_sub_cmd_type sub_cmd;

    diag_webdload_rsp_type* rsp_ptr;
    diag_webdload_req_type* req_ptr;

    req_ptr = (diag_webdload_req_type*)cmd_buf;
    rsp_ptr = (diag_webdload_rsp_type*)rsp_buf;

    webdload_memset((byte*)rsp_buf, 0, (int)sizeof(diag_webdload_rsp_type));

    sub_cmd = (webdload_sub_cmd_type)req_ptr->sub_cmd;

    rsp_ptr->cmd_code = DIAG_WEBDLOAD_COMMON_F;
    rsp_ptr->sub_cmd  = sub_cmd;
    rsp_ptr->success  = TRUE;

    if(data_read_flag == 0)
    {
		result = diag_Flag_Data_Init();

		if(result == FALSE)
		{
		    rsp_ptr->success = FALSE;
		    return;
		}

		data_read_flag = 1;
    }

    switch( sub_cmd )
    {

	case WEBDLOAD_READ_STEP_END:
	    rsp_ptr->rsp_data.current_step = (word)((status_info_type*)si_page_buffer)->current_step;
	    webdload.step_real = (word)((status_info_type*)si_page_buffer)->current_step; /* update webdload_step variable for later use */
	    break;

	case WEBDLOAD_READ_BATT_LVL:  /* battery level response, See Msm_charger.c (kernel\drivers\power)  */
// chinsu.ko@lge.com
/*
#if 1
	    batt_state = is_batt_lvl_present();
	    if(batt_state >= 25)
		rsp_ptr->rsp_data.batt_lvl = 5;
	    else
		rsp_ptr->rsp_data.batt_lvl = 2;
#else
	    rsp_ptr->rsp_data.batt_lvl = is_batt_lvl_present();// range 0~10 확인 하고 수정 할것  
#endif
*/
		// [111018 dong.kim@lge.com M3_ALL] WEB Download[START]
		rsp_ptr->rsp_data.batt_lvl = 5;
		// [111018 dong.kim@lge.com M3_ALL] [END]
		break;	  

	case WEBDLOAD_READ_CNT:
	    rsp_ptr->rsp_data.webdload_cnt = (word)((status_info_type*)si_page_buffer)->webdload_cnt; 
	    break;

	case WEBDLOAD_READ_BIN_VER: 
	    memcpy( (void*)rsp_ptr->rsp_data.bin_ver, (void*)&((status_info_type*)si_page_buffer)->bin_ver, VER_NAME_LEN );
	    break;

	case WEBDLOAD_READ_SUB_REV:
	    memcpy( (void*)rsp_ptr->rsp_data.sub_rev, (void*)&((status_info_type*)si_page_buffer)->sub_rev, SUB_REV_LEN );
	    break;

	case WEBDLOAD_READ_RSVD_VAL1: 
	    rsp_ptr->rsp_data.reserved_val1 = (word)((status_info_type*)si_page_buffer)->reserved_val1;
	    break;

	case WEBDLOAD_READ_RSVD_VAL2:
	    rsp_ptr->rsp_data.reserved_val2 = (word)((status_info_type*)si_page_buffer)->reserved_val2;
	    break;

	case WEBDLOAD_READ_RSVD_ARRAY1:
	    memcpy( (void*)rsp_ptr->rsp_data.reserved_array1, (void*)&((status_info_type*)si_page_buffer)->reserved_array1, RSVD_ARRAY_LEN );
	    break;

	case WEBDLOAD_READ_RSVD_ARRAY2: 
	    memcpy( (void*)rsp_ptr->rsp_data.reserved_array2, (void*)&((status_info_type*)si_page_buffer)->reserved_array2, RSVD_ARRAY_LEN );
	    break;

	case WEBDLOAD_READ_LANGUAGE: 
	    rsp_ptr->rsp_data.language = (word)((status_info_type*)si_page_buffer)->language;
	    webdload.language = rsp_ptr->rsp_data.language;
	    break;

	case WEBDLOAD_CONNECT_MODE: 
	    rsp_ptr->rsp_data.connection_mode = (byte)mode;
	    break;

	case WEBDLOAD_CLEAR_SIB:
	    /* ==========> eMMC data erase   ============ */
	    mtd_op_result = lge_erase_block(web_status_bytes_pos_in_emmc + select_sector, (size_t)BACKUP_BYTE_MAX_SIZE);

	    if( mtd_op_result != (BACKUP_BYTE_MAX_SIZE) )
	    {
			printk("%s, emmc erase error, normal return value size = 2048 : %d\n", __func__, mtd_op_result);
			rsp_ptr->success = FALSE;
			return;
	    }/* else noraml */
	    break;

	case WEBDLOAD_DISP_IMAGE:  
	    webdload.step_ctrl = WEBDLOAD_INIT_VAL;
	    break;

	case WEBDLOAD_SET_RSP_NEED: 
	    webdload.rsp_needed = req_ptr->req_data.rsp_needed;
	    break;

	case WEBDLOAD_GET_AUTH_MARK:
	    rsp_ptr->rsp_data.auth_mark = (word)((status_info_type*)si_page_buffer)->auth_mark;
		/* LGE_CHANGE_S [jaehoon1024.jung] 20120315 : [WEBDOWNLOAD] if M4, Don't response CDMA authmark */
		/* LGE_CHANGE_S [jongkwang.lee@lge.com] 20111124 : [WEBDOWNLOAD] if E0, Don't response CDMA authmark */
		//#ifdef CONFIG_MACH_MSM7X25A_E0EU
		rsp_ptr->success = FALSE;
		//#endif
		/* LGE_CHANGE_E [jongkwang.lee@lge.com] 20111124 */
		/* LGE_CHANGE_E [jaehoon1024.jung] 20120315 */
	    break;

	case WEBDLOAD_GET_USB_STRM_INFO:
	    //#ifdef BUILD_BOOT_CHAIN_OEMSBL
	    //	rsp_ptr->rsp_data.curr_strm = LS_STREAM;
	    //#else
	    rsp_ptr->rsp_data.curr_strm = HS_STREAM;
	    //#endif
	    break;

	case WEBDLOAD_SET_AUTH_MARK:			case WEBDLOAD_WRITE_STEP_END:		case WEBDLOAD_WRITE_CNT:
	case WEBDLOAD_WRITE_BIN_VER:			case WEBDLOAD_WRITE_SUB_REV:		case WEBDLOAD_WRITE_LANGUAGE:
	case WEBDLOAD_WRITE_RSVD_VAL1:			case WEBDLOAD_WRITE_RSVD_VAL2:		case WEBDLOAD_WRITE_RSVD_ARRAY1:
	case WEBDLOAD_WRITE_RSVD_ARRAY2:		case WEBDLOAD_DL_RESET:
	    {
		if( sub_cmd == WEBDLOAD_SET_AUTH_MARK )
		{
		    ((status_info_type*)si_page_buffer)->auth_mark = (word)(sib_step_type)req_ptr->req_data.auth_mark;
		}
		else if( sub_cmd == WEBDLOAD_WRITE_STEP_END )
		{
		    ((status_info_type*)si_page_buffer)->current_step = (word)(sib_step_type)req_ptr->req_data.current_step;
		    webdload.step_real = (word)(((status_info_type*)si_page_buffer)->current_step); /* update webdload_step variable for later use */
		}
		else if( sub_cmd == WEBDLOAD_WRITE_CNT )
		{
		    ((status_info_type*)si_page_buffer)->webdload_cnt = (word)req_ptr->req_data.webdload_cnt;
		}
		else if( sub_cmd == WEBDLOAD_WRITE_BIN_VER )
		{
		    memcpy( (void*)&((status_info_type*)si_page_buffer)->bin_ver, (void*)req_ptr->req_data.bin_ver, VER_NAME_LEN );
		}
		else if( sub_cmd == WEBDLOAD_WRITE_SUB_REV )
		{
		    memcpy( (void*)&((status_info_type*)si_page_buffer)->sub_rev, (void*)req_ptr->req_data.sub_rev, SUB_REV_LEN );
		}
		else if( sub_cmd == WEBDLOAD_WRITE_LANGUAGE )
		{
		    word *lang=NULL;

		    ((status_info_type*)si_page_buffer)->language = (word)req_ptr->req_data.language;

		    *lang = ((word)((status_info_type*)si_page_buffer)->language);  /* 값 확인 할것 */

		    if( *lang == WEBDLOAD_LANG_TRUST_PHONE )
		    {
			//#ifndef BUILD_BOOT_CHAIN_OEMSBL
			//tjc411 -	*lang = g_webdload_char_set;
			*lang = 0;//tjc411
			//#endif
		    }
		    webdload.language = *lang;
		}
		else if( sub_cmd == WEBDLOAD_WRITE_RSVD_VAL1 )
		{
		    ((status_info_type*)si_page_buffer)->reserved_val1 = (word)(sib_step_type)req_ptr->req_data.reserved_val1;
		}
		else if( sub_cmd == WEBDLOAD_WRITE_RSVD_VAL2 )
		{
		    ((status_info_type*)si_page_buffer)->reserved_val2 = (word)(sib_step_type)req_ptr->req_data.reserved_val2;
		}
		else if( sub_cmd == WEBDLOAD_WRITE_RSVD_ARRAY1 )
		{
		    memcpy( (void*)&((status_info_type*)si_page_buffer)->reserved_array1, (void*)req_ptr->req_data.reserved_array1, RSVD_ARRAY_LEN );
		}
		else if( sub_cmd == WEBDLOAD_WRITE_RSVD_ARRAY2 )
		{
		    memcpy( (void*)&((status_info_type*)si_page_buffer)->reserved_array2, (void*)req_ptr->req_data.reserved_array2, RSVD_ARRAY_LEN );
		}
		else if( sub_cmd == WEBDLOAD_DL_RESET )
		{
		    ((status_info_type*)si_page_buffer)->web_dload_entry = WEB_DLOAD_ENTRY; /* webdload status save */
			
		    /* ==========> eMMC data write   ============ */
		    mtd_op_result = lge_write_block(web_status_bytes_pos_in_emmc + select_sector, si_page_buffer, BACKUP_BYTE_MAX_SIZE);

		    if( mtd_op_result != (BACKUP_BYTE_MAX_SIZE) )
		    {
				printk("%s, emmc write error, normal return value size = 2048 : %d\n", __func__, mtd_op_result);
				rsp_ptr->success = FALSE;
				return;
		    }/* else noraml */


		    /* RPC로 reset 요청 */
		    //webDload_rpc_srd_cmmand(req_ptr, rsp_ptr); /* send req packet to  server & receive rsp_ptr  packe form server.*/

		}
		break;
	    }

#if 0 //def BUILD_BOOT_CHAIN_OEMSBL
	case WEBDLOAD_READ_BL_VER:
	    memcpy( (void*)rsp_ptr->rsp_data.bin_ver, (void*)cs_ver_dir, CS_VER_LEN);
	    webdload_memset((byte*)&rsp_ptr->rsp_data.bin_ver[CS_VER_LEN], 0, (VER_NAME_LEN-CS_VER_LEN));
	    break;
#endif
	case WEBDLOAD_GET_MEID:			/*0x31*/
	    /* read successful, copy the meid */
	    rsp_ptr->rsp_data.meid[0] = ((status_info_type*)si_page_buffer)->meid[0];
	    rsp_ptr->rsp_data.meid[1] = ((status_info_type*)si_page_buffer)->meid[1];
	    break;

	case 0x30:	case 0x61:		/* We don't use command But other device use command, so we don't delete command.  */
	    break;
	case WEBDLOAD_MODEL_NAME:
		{
			diag_webdload_name_req_type* name_req_ptr;
			name_req_ptr = (diag_webdload_name_req_type*)cmd_buf;
			memcpy((void*)&((status_info_type*)si_page_buffer)->model_name, (void*)name_req_ptr->model_name, MODEL_NAME_LEN);
			
			rsp_ptr->success = TRUE;
		}
		break;
	default:
	    rsp_ptr->success = FALSE;
	    break;
    }/*  end ==>  switch( sub_cmd )*/

    return;

}

#endif /*LG_FW_WEB_DOWNLOAD*/	
// LG_FW : 2011.07.07 moon.yongho -----------------------------------------------------]]

