/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                      User Data Backup File
PROJECT
      VN251

GENERAL DESCRIPTION
   

Copyright (c) 2010 LG Electronics. All right reserved.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*============================================================================

                      EDIT HISTORY FOR FILE

 This section contains comments describing changes made to this file.
 Notice that changes are listed in reverse chronological order.

 when      who    what, where, why
 --------  -----  ----------------------------------------------------------
 10/04/09  bkshin Initial Release
============================================================================*/


/*============================================================================
                          INCLUDE FILES
============================================================================*/

#include <linux/module.h>
#include <lg_diagcmd.h>
#include <linux/input.h>
#include <linux/syscalls.h>

#include <userDataBackUpDiag.h>
#include <userDataBackUpTypeDef.h> 

#include <linux/fcntl.h> 
#include <linux/fs.h>
#include <linux/uaccess.h>
//#include "Diagcomm.h"
//#include "diagBuf.h"
//#include "UserDataBackUp.h"
#include <linux/slab.h>

// LG_FW : 2011.07.06 moon.yongho : saving webdload status variable to eMMC. ----------[[
#include "lg_diag_cfg.h"
// LG_FW : 2011.07.06 moon.yongho -----------------------------------------------------]]

/*============================================================================
              CONSTANTS, MACROS AND STRUCTURE TYPES FOR MODULE
============================================================================*/
/*============================================================================
                        EXTERNAL FUNCTIONS
============================================================================*/
#ifdef CONFIG_LGE_DLOAD_SRD  //kabjoo.choi


user_data_backup_info userDataBackUpInfo;

char nv_item_buffer[MAX_NV_ITEM_SIZE*2];	//CSFB SRD
extern unsigned char * load_srd_base;		//CSFB SRD
word  ud_page_index=0;						//CSFB SRD
	
#define EMMC_SECTOR_UNIT 512
#define SRD_NV_ITEM_WRITE_BYTE_SIZE (EMMC_SECTOR_UNIT/2)
#define SRD_1MBYTE_AREA  (EMMC_SECTOR_UNIT*2048)
#define SRD_FIRST_AREA_INPARTITION 1  //nv
#define SRD_SECOND_AREA_INPARTITION 2  //extra nv
#define SRD_THIRD_AREA_INPARTITION 3  //prl 

#define PTN_FRST_PERSIST_OFFSET_IN_MISC_PARTITION 1    // 8192 개 nv 지원  (1Mbyte /128 byte)
#define PTN_EXTRA_NV_PERSIST_OFFSET_IN_MISC_PARTITION 2048 //start sector number
#define PTN_PRL_PERSIST_OFFSET_IN_MISC_PARTITION 4096  //start sector number

//extern const MmcPartition *lge_mmc_find_partition_by_name(const char *name);




typedef struct  {
    char *device_index;
    char *filesystem;
    char *name;
    unsigned dstatus;
    unsigned dtype ;
    unsigned dfirstsec;
    unsigned dsize;
}MmcPartition ;


//typedef struct MmcPartition pMisc_part;


extern int lge_erase_block(int bytes_pos, size_t erase_size);
extern int lge_write_block(unsigned int bytes_pos, unsigned char *buf, size_t size);
extern int lge_read_block(unsigned int bytes_pos, unsigned char *buf, size_t size);
extern int lge_mmc_scan_partitions(void);
extern void userDataBackUpCopyBuffer_MDM(uint16 nEnum, int nNam, char * nType);	//CSFB SRD
extern void remote_rpc_with_mdm(uint32 in_len, byte *input, uint32 *out_len, byte *output);	//CSFB SRD
//extern const MmcPartition *lge_mmc_find_partition_by_name(const char *name);
//#define PTN_FRST_PERSIST_POSITION_IN_MISC_PARTITION      (512*PTN_FRST_PERSIST_OFFSET_IN_MISC_PARTITION)
//#define PTN_EXTRA_NV_PERSIST_POSITION_IN_MISC_PARTITION      (512*PTN_EXTRA_NV_PERSIST_OFFSET_IN_MISC_PARTITION)
//#define PTN_PRL_PERSIST_POSITION_IN_MISC_PARTITION     (512*PTN_PRL_PERSIST_OFFSET_IN_MISC_PARTITION)
/*============================================================================
                        EXTERNAL VARIABLES
============================================================================*/


/*============================================================================
                        GLOBAL VARIABLES
============================================================================*/

/*============================================================================
                        Functions
============================================================================*/

/*===========================================================================

FUNCTION userDataBackUpBadCalc

DESCRIPTION
DEPENDENCIES
  None.

RETURN VALUE
  Pointer to response packet.

SIDE EFFECTS
None. 
===========================================================================*/
extern const MmcPartition *lge_mmc_find_partition_by_name(const char *name);
unsigned int srd_bytes_pos_in_emmc ;
unsigned int write_counter_sector=0;

boolean userDataBackUpBadCalc(void)
{
	const MmcPartition *pMisc_part; 
	
	pMisc_part = lge_mmc_find_partition_by_name("misc");
	if (pMisc_part ==NULL)
	{
		return FALSE;
	}
	srd_bytes_pos_in_emmc = (pMisc_part->dfirstsec)*512;

	printk(KERN_ERR "MISC dfirstsec = %d", pMisc_part->dfirstsec);
	
	return TRUE;
	#if 0
	int i;  
  	int start_blk;
  	int end_blk;
  	int max_badBlk;
  	int total_bad_count = 0;


	if(userDataBackupFlashInit()==FALSE)
		return FALSE;
	  

			   
	start_blk = flash_nand_partition_start_block(MIBIB_DIDBACKUP_PARTI_NAME);
	end_blk = flash_nand_partition_end_block(MIBIB_DIDBACKUP_PARTI_NAME);

	max_badBlk = (int)(end_blk - start_blk);
	  
	for (i=start_blk; i<= end_blk; i++)		
	{
		if ( nand_device->bad_block_check(nand_device, i) == FS_DEVICE_BAD_BLOCK)
		{
		  total_bad_count++;	
		}
		else
		{
          uBadBlockAddress[i - start_blk - total_bad_count] = i;
		}
	}
	
	if (total_bad_count > MIN_NEED_BLOCK_SRD)
		return FALSE;
	else
		return TRUE;
	#endif 	
}	

boolean userDataBackUpPartitionErase(void )
{
	 int mtd_op_result ;
	 mtd_op_result = lge_erase_block(srd_bytes_pos_in_emmc, (size_t)(SRD_1MBYTE_AREA*SRD_FIRST_AREA_INPARTITION));
	  if(mtd_op_result != (SRD_1MBYTE_AREA*SRD_FIRST_AREA_INPARTITION))
	 {
          printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
         //  rsp_pkt->header.err_code = TEST_FAIL_S;        
         return FALSE;
        }

	//CSFB SRD  MDM 밖에 없음
	// srd_bytes_pos_in_emmc+0x400000  for  MDM 
	  /*mtd_op_result = lge_erase_block(srd_bytes_pos_in_emmc+0x400000, (size_t)(SRD_1MBYTE_AREA));
	  if(mtd_op_result != (SRD_1MBYTE_AREA*SRD_FIRST_AREA_INPARTITION))
	 {
          printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
         //  rsp_pkt->header.err_code = TEST_FAIL_S;        
         return FALSE;
        }*/
	 //현재 1mbyte 만 지움. ..
#if 0
	int i;  
  	int start_blk;
  	int end_blk;
  
	if(userDataBackupFlashInit()==FALSE)
		return FALSE;

	start_blk = flash_nand_partition_start_block(MIBIB_DIDBACKUP_PARTI_NAME);
	end_blk = flash_nand_partition_end_block(MIBIB_DIDBACKUP_PARTI_NAME);

	for (i=start_blk; i<= end_blk; i++)		
	{
		if(nand_device->bad_block_check(nand_device, i) == FS_DEVICE_OK)
		{
		    if(nand_device->erase_block(nand_device,i) != FS_DEVICE_DONE)
			  return FALSE;
		}		
	}
#endif 
	return TRUE;
}	

boolean writeBackUpCommand(script_process_type MDM_MODEM ) 
{
 
	int mtd_op_result = 0;
		
	
	if(userDataBackUpInfo.dl_info_cnt<MAX_INFO_COUNT)
		userDataBackUpInfo.dl_info_cnt++;
	else 
		userDataBackUpInfo.dl_info_cnt=1;

	if(MDM_MODEM==MODEM_BACKUP)
	 mtd_op_result = lge_write_block(srd_bytes_pos_in_emmc, (char*)&userDataBackUpInfo, sizeof(user_data_backup_info));
	else
	mtd_op_result = lge_write_block(srd_bytes_pos_in_emmc+0x400000, (char*)&userDataBackUpInfo, sizeof(user_data_backup_info));	
	
        if(mtd_op_result!= sizeof(user_data_backup_info))
        {
		return FALSE;
                   
        }

	 write_counter_sector ++;

	return TRUE;
}

boolean writeBackUpNVdata( char * ram_start_address , unsigned int size)
{
int mtd_op_result = 0;
unsigned char *buf = (char *)ram_start_address;	
	
	
	 mtd_op_result = lge_write_block(srd_bytes_pos_in_emmc+(write_counter_sector*512), buf, size);
        if(mtd_op_result!= size)
        {
		return FALSE;
                   
        }

	 write_counter_sector =0;
//todo  ??  srd_bytes_pos_in_emmc=srd_bytes_pos_in_emmc +512   ,  app 에서는 info만 write ,,
	
	return TRUE;
	
}

/*===========================================================================
	
FUNCTION diag_userDataBackUp_entrySet
	
DESCRIPTION
	
	
DEPENDENCIES
  None.
	
RETURN VALUE
  Pointer to response packet.
	
SIDE EFFECTS
  None.
	 
===========================================================================*/
void diag_userDataBackUp_entrySet(udbp_req_type *req_pkt, udbp_rsp_type *rsp_pkt, script_process_type MODEM_MDM )
{
	userDataBackUpInfo.info.backup_used = req_pkt->req_data.do_dl_entry.backup_used;
	userDataBackUpInfo.info.binary_class = req_pkt->req_data.do_dl_entry.binary_class;
	userDataBackUpInfo.info.factory_reset_required = req_pkt->req_data.do_dl_entry.factory_reset_required;
	userDataBackUpInfo.info.device_srd_reset_required = req_pkt->req_data.do_dl_entry.device_srd_reset_required;
	userDataBackUpInfo.info.information = req_pkt->req_data.do_dl_entry.information;
	userDataBackUpInfo.info.srd_backup_nv_counter = rsp_pkt->rsp_data.write_sector_counter;
		
	if(writeBackUpCommand(MODEM_MDM)!= TRUE)
	{
		printk(KERN_ERR "UDBU_ERROR_CANNOT_COMPLETE : diag_userDataBackUp_entrySet");
		rsp_pkt->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
	}
	else
	{
		printk(KERN_ERR "UDBU_ERROR_SUCCESS : diag_userDataBackUp_entrySet");
		rsp_pkt->header.err_code = UDBU_ERROR_SUCCESS;
	}
	return;
}
EXPORT_SYMBOL(diag_userDataBackUp_entrySet);

/*===========================================================================
	
FUNCTION diag_userDataBackUp_data
	
DESCRIPTION
	
	
DEPENDENCIES
  None.
	
RETURN VALUE
  Pointer to response packet.
	
SIDE EFFECTS
  None.
	 
===========================================================================*/
//CSFB SRD
//#include "../../../vendor/qcom/proprietary/data/dss_new/src/platform/inc/nv_items.h"
#include "SU640_nv_backup_list.h"
void diag_userDataBackUp_data(udbp_req_type *req_pkt, udbp_rsp_type *rsp_pkt)
{
	int i;
	int nItemCnt = sizeof(nv_backup_list)/sizeof(word);

	rsp_pkt->header.cmd_code = req_pkt->header.cmd_code;
	rsp_pkt->header.sub_cmd = req_pkt->header.sub_cmd;
	rsp_pkt->header.packet_version = req_pkt->header.packet_version;
	rsp_pkt->header.dwsize = req_pkt->header.dwsize;
	rsp_pkt->header.err_code = UDBU_ERROR_SUCCESS;

	for(i=0; i<nItemCnt; i++)
	{
		userDataBackUpCopyBuffer_MDM(nv_backup_list[i], 0, "BinDL");
	}

	rsp_pkt->header.err_code = UDBU_ERROR_SUCCESS;
	rsp_pkt->rsp_data.write_sector_counter = ud_page_index;
	printk(KERN_WARNING "SRD ud_page_index = %d", ud_page_index);
		
//	scriptManagerDispError("Script Write Complete", SCM_SUCCESS);
//	strcpy(strTemp, "Script Write Complete");
//	scriptManagerDispError(strTemp, SCM_SUCCESS); //SRD_ERROR

	return;
}
EXPORT_SYMBOL(diag_userDataBackUp_data);

#ifdef LG_FW_SRD_EXTRA_NV

/*===========================================================================
	
FUNCTION diag_extraNv_entrySet
	
DESCRIPTION

	
DEPENDENCIES
  None.
	
RETURN VALUE
  Pointer to response packet.
	
SIDE EFFECTS
  None.
	 
===========================================================================*/

void diag_extraNv_entrySet(udbp_req_type *req_pkt, udbp_rsp_type *rsp_pkt)
{

    rsp_pkt->header.cmd_code = req_pkt->header.cmd_code;
	rsp_pkt->header.sub_cmd = req_pkt->header.sub_cmd;
	rsp_pkt->header.packet_version = req_pkt->header.packet_version;
	rsp_pkt->header.dwsize = req_pkt->header.dwsize;
	rsp_pkt->header.err_code = UDBU_ERROR_SUCCESS;
	
		switch(req_pkt->req_data.extra_nv_operation.bOperation)
  		{
			case EXTRANV_BACKUP :
				if(!extraNvBackUp(&(req_pkt->req_data.extra_nv_operation)))
				    rsp_pkt->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
				break;

			case EXTRANV_INIT :
				if(!extraNvInit(&(req_pkt->req_data.extra_nv_operation)))
					rsp_pkt->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
				break;

			case EXTRANV_DELETE :
				if(!extraNvDelete(&(req_pkt->req_data.extra_nv_operation)))
					rsp_pkt->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
				break;

			case EXTRANV_BACKUP_END :
				if(!extraNvBackUpEnd())
				    rsp_pkt->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
				break;

			default :
  				rsp_pkt->header.err_code = UDBU_ERROR_BAD_COMMAND;
				break;
		
		}
}
EXPORT_SYMBOL(diag_extraNv_entrySet);

#endif

#ifdef LG_FW_SRD_PRL

/*===========================================================================
	
FUNCTION diag_PRL_entrySet
	
DESCRIPTION

	
DEPENDENCIES
  None.
	
RETURN VALUE
  Pointer to response packet.
	
SIDE EFFECTS
  None.
	 
===========================================================================*/

 void diag_PRL_entrySet(udbp_req_type *req_pkt, udbp_rsp_type *rsp_pkt)
{

    rsp_pkt->header.cmd_code = req_pkt->header.cmd_code;
	rsp_pkt->header.sub_cmd = req_pkt->header.sub_cmd;
	rsp_pkt->header.packet_version = req_pkt->header.packet_version;
	rsp_pkt->header.dwsize = req_pkt->header.dwsize;
	rsp_pkt->header.err_code = UDBU_ERROR_SUCCESS;
	

	if(!PRLBackUp(&(req_pkt->req_data.extra_prl_operation)))
	    rsp_pkt->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;

}
EXPORT_SYMBOL(diag_PRL_entrySet);
#endif

/*===========================================================================
	
FUNCTION diag_PRL_entrySet
	
DESCRIPTION

	
DEPENDENCIES
  None.
	
RETURN VALUE
  Pointer to response packet.
	
SIDE EFFECTS
  None.
	 
===========================================================================*/
 void diag_SRD_Init(udbp_req_type *req_pkt, udbp_rsp_type *rsp_pkt)
{
	int nPartionCount;
   	rsp_pkt->header.cmd_code = req_pkt->header.cmd_code;
	rsp_pkt->header.sub_cmd = req_pkt->header.sub_cmd;
	rsp_pkt->header.packet_version = req_pkt->header.packet_version;
	rsp_pkt->header.dwsize = req_pkt->header.dwsize;
	rsp_pkt->header.err_code = UDBU_ERROR_SUCCESS;

 	//rsp_pkt->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
	nPartionCount = lge_mmc_scan_partitions();
  	printk(KERN_ERR "diag_SRD_Init nPartionCount = %d", nPartionCount);

	if(!userDataBackUpBadCalc())
	{
		printk(KERN_ERR "diag_SRD_Init Err : userDataBackUpBadCalc");
		goto Error;
	}

	if(!userDataBackUpPartitionErase())
	{
		printk(KERN_ERR "diag_SRD_Init Err : userDataBackUpPartitionErase");
		goto Error;
	}
//	if(!userDataBackUpAndRecoverInit())
//		goto Error;
#ifdef LG_FW_SRD_EXTRA_NV
	if(!extraNvBackUpAndRecoverInit())
		goto Error;
#endif
#ifdef LG_FW_SRD_PRL
	if(!PRLBackUpAndRecoverInit())
		goto Error;
#endif
	printk(KERN_ERR "diag_SRD_Init Success");
	return;

Error:
   rsp_pkt->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
}
EXPORT_SYMBOL(diag_SRD_Init);

#endif //#ifdef CONFIG_LGE_DLOAD_SRD  //kabjoo.choi


// LG_FW : 2011.07.07 moon.yongho : saving webdload status variable to eMMC. ----------[[
#ifdef LG_FW_WEB_DOWNLOAD	
unsigned int web_status_bytes_pos_in_emmc = 0;
boolean userWebDnBadCalc(void)
{

        const MmcPartition *pMisc_part; 
	
	pMisc_part = lge_mmc_find_partition_by_name("blb");/* web status variable saving region */
	if (pMisc_part ==NULL)
	{
		return FALSE;
	}
	web_status_bytes_pos_in_emmc = (pMisc_part->dfirstsec)*512;
	
	return TRUE;

}


 boolean diag_WebDL_SRD_Init(void)
{

	if(lge_mmc_scan_partitions() ==-1)
		return FALSE;
  	
	if(!userWebDnBadCalc())
		return FALSE;


#if 0 /* It is not necerssary to erase web memory part in eMMC. */
	if(!userDataBackUpPartitionErase())  
		return FALSE;
#endif 

	return TRUE;
}
EXPORT_SYMBOL(diag_WebDL_SRD_Init);	
#endif /*LG_FW_WEB_DOWNLOAD*/	
// LG_FW : 2011.07.07 moon.yongho ----------------------------------------------------]]	



#if 0
/*===========================================================================
	
FUNCTION diag_userDataBackUp_handler
	
DESCRIPTION
	
	
DEPENDENCIES
  None.
	
RETURN VALUE
  Pointer to response packet.
	
SIDE EFFECTS
  None.
	 
===========================================================================*/
PACKED void * diag_userDataBackUp_handler(PACKED void *req_pkt_ptr,uint16 pkt_len)
{
	udbp_req_type		*req_ptr = (udbp_req_type *) req_pkt_ptr;
	udbp_rsp_type	  	*rsp_ptr = NULL;

	const rsp_len = sizeof( udbp_rsp_type ); 
	
	rsp_ptr = (udbp_rsp_type *) diagpkt_alloc (DIAG_USET_DATA_BACKUP, rsp_len);

    if(rsp_ptr)
	{
		switch(req_ptr->header.sub_cmd)
  		{
  			case SRD_INIT_OPERATION :
				diag_SRD_Init(req_ptr,rsp_ptr);							
				break;
				
			case USERDATA_BACKUP_REQUEST :
				diag_userDataBackUp_entrySet(req_ptr,rsp_ptr);
				break;

			case GET_DOWNLOAD_INFO :
				break;

			case EXTRA_NV_OPERATION :
#ifdef LG_FW_SRD_EXTRA_NV				
				diag_extraNv_entrySet(req_ptr,rsp_ptr);
#endif
				break;
				
			case PRL_OPERATION :
#ifdef LG_FW_SRD_PRL				
				diag_PRL_entrySet(req_ptr,rsp_ptr);
#endif
				break;
				
			default :
  				rsp_ptr = (void *) diagpkt_err_rsp (DIAG_BAD_PARM_F, req_ptr, pkt_len);
				break;
		
		}
	}
	/* Execption*/	
	if (rsp_ptr == NULL){
		return NULL;
	}
	if (!diagcomm_status()){
		diagpkt_free(rsp_ptr);
		return NULL;
	}

	return rsp_ptr;
}
#endif 


//CSFB SRD
void userDataBackUpCopyBuffer_MDM(uint16 nEnum, int nNam, char * nType)
{

	nv_item_type remote_nvi;
	//nv_stat_enum_type nv_stat;
	nv_item_info_type	nv_backuptype;
	
	xml_parser_nv_item_info	nv_item_info;

	unsigned int    i;

	uint32 out_len = 0;
	
#ifdef LG_FW_SRD_POSTPAY_PREPAY
	int pay_phone_type;
#endif

	memset(&nv_item_info,0x00,sizeof(xml_parser_nv_item_info));   //clear buffer 
	
	if(strncmp(nType,"Upgrade",7) == 0)
		nv_backuptype = NV_UPGRADE;
	else if(strncmp(nType,"BinDL",5) == 0)
		nv_backuptype = NV_BINDL;
	else if(strncmp(nType,"SKIP",4) == 0) 
		nv_backuptype = NV_SKIP;
	else if(strncmp(nType,"Refurbish",9) == 0)
		nv_backuptype = NV_REFURBISH;
	else if(strncmp(nType,"END",3) == 0)
		nv_backuptype = NV_END;
	else
		nv_backuptype = NV_END;
	
	nv_item_info.nv_enum = nEnum;
	nv_item_info.nv_nam = nNam;
	nv_item_info.nv_type = nv_backuptype;

	if (nNam >= 2)
	{
		for(i=0; i<=nNam-1; i++)
		{
		nv_item_info.nam_cnt = i;
		


		remote_rpc_with_mdm(sizeof(nv_item_info), (byte *)&nv_item_info, &out_len, (byte *)&remote_nvi);
		//nv_item_nam_info_set(nEnum,&remote_nvi,(byte)i);	
			
		if(out_len!=0)   //out len is zero means not active nv or fail
		{
			memset(nv_item_buffer,0xff,MAX_NV_ITEM_SIZE*2);   // 256 buffer 

			
				//			256			      nv enum,nam,cnt,type			
				memcpy(nv_item_buffer,(void *)&nv_item_info,sizeof(xml_parser_nv_item_info));
																							// 256 -xml_parser_nv_item_info
				memcpy(&nv_item_buffer[0]+sizeof(xml_parser_nv_item_info),(void *)&remote_nvi,256);
																							
				memcpy( ((load_srd_base)+ud_page_index*256),nv_item_buffer,sizeof(nv_item_buffer));
				//memset(nv_item_buffer,0xff,sizeof(nv_item_buffer));
				
				ud_page_index++;
				
				#ifdef LG_FW_USER_DATA_BACKUP_DEGUG
				
				active_nv_item[nv_active]=nEnum;
				nv_active++;
				#endif 
				//buffer_index =0;
		}
			#ifdef LG_FW_USER_DATA_BACKUP_DEGUG
			else
			{

				
				
				no_active_nv_item[nv_no_active]=nEnum;
				//no_active_nv_item_return[nv_no_active]=nv_stat; 
				nv_no_active++;
			
			}
			#endif 			
		}
	}





	else
	{
	nv_item_info.nam_cnt = 0;

	remote_rpc_with_mdm(sizeof(nv_item_info),(byte *)&nv_item_info, &out_len,(byte *)&remote_nvi);
	if(out_len!=0)   //out len is zero means not active nv or fail
	{

			memset(nv_item_buffer,0xff,MAX_NV_ITEM_SIZE*2);
					//			256			      nv enum,nam,cnt,type			
			memcpy(nv_item_buffer,(void *)&nv_item_info,sizeof(xml_parser_nv_item_info));
																							// 256 -xml_parser_nv_item_info
			memcpy(&nv_item_buffer[0]+sizeof(xml_parser_nv_item_info),(void *)&remote_nvi,128);
																							
			memcpy( ((load_srd_base)+ud_page_index*256),nv_item_buffer,sizeof(nv_item_buffer));
			//memset(nv_item_buffer,0xff,sizeof(nv_item_buffer));
				
			ud_page_index++;
			#ifdef LG_FW_USER_DATA_BACKUP_DEGUG
			active_nv_item[nv_active]=nEnum;
			nv_active++;
			#endif 

			
	}	
		
		#ifdef LG_FW_USER_DATA_BACKUP_DEGUG
		else
		{
				
				
				no_active_nv_item[nv_no_active]=nEnum;
				//no_active_nv_item_return[nv_no_active]=nv_stat;
				nv_no_active++;
				 

		}
		#endif	

  }
}
