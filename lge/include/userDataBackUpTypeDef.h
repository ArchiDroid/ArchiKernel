#ifndef _USER_DATA_BACK_UP_TYPE_DEF_H_
#define _USER_DATA_BACK_UP_TYPE_DEF_H_

#include <lg_diag_testmode.h>

/*===========================================================================
                      MACRO DECLARATIONS
===========================================================================*/
		// NV backup use type
#define BACKUP_USE_CS             1  // nv backup for end user  - CS Download
#define BACKUP_USE_FACTORY     2  // nv backup for factory mode - Factory Download
#define BACKUP_USE_REFURBISH   3  // nv backup for refurbish(Huntsvile, World factory)

// Binary Class
#define BINARY_CLASS_NORMAL_BINARY      1  // normal binary *.bin
#define BINARY_CLASS_MASTER_BINARY         2  // Master binary *.tot
	
	// bOperation type
#define NV_OPERATION_BACKUP      1  // nv backup
#define NV_OPERATION_INIT           2  // nv item initialize by bData
#define NV_OPERATION_DELETE       3  // nv item initialize(no active)


#define MAX_DLL_VER_SIZE       16//32
#define MAX_FW_VER_SIZE        16//32
#define MAX_PRL_VER_SIZE       16//32
#define MAX_SCRIPT_VER_SIZE    16//32
#define MAX_RESERVED_SIZE      16//32

#define MAX_NV_ITEM_SIZE            128
#define MAX_USER_DATA_IN_PAGE		16
#define MAX_USER_DATA_IN_BLOCK		1024
#define MAX_PRL_ITEM_SIZE			128//2048   //kabjoo.choi  rpc fail.
#define MAX_INFO_COUNT		50
#define MAX_BLOCK_IN_DID	 	 7 //DID Partition Data + Padding
#define MIN_NEED_BLOCK_SRD	  	 2 //DID Partition Padding

#define SIZE_OF_SHARD_RAM_SIZE  0x60000	//CSFB SRD

/*===========================================================================
                     DIAG TYPE DEFINITIONS
===========================================================================*/

typedef  struct {
       byte    cmd_code;         // DIAG_PST_API_F = 249
       byte    sub_cmd;           // sub command
       dword   err_code;         // error code
       dword   packet_version;  // version information for this packet. currently, 0x1000
       dword   dwsize;            // size of packet - header
       
} PACKED udbp_header_type;


// System time is represented with the following structure:
//
typedef  struct {
    word wYear;
    word wMonth;
    word wDay;
    word wHour;
    word wMinute;
    word wSecond;
} PACKED udbp_time_type;

//
// Download Information is represented with the following structure:
//
typedef  struct {
    char dll_version[MAX_DLL_VER_SIZE];             // dll version ex) 1,0,0,0
    char fw_version[MAX_FW_VER_SIZE];          // new binary version for download
    char prl_version[MAX_PRL_VER_SIZE];         // new prl version for download
    char script_version[MAX_SCRIPT_VER_SIZE];  // new script version for download
    udbp_time_type dl_time;                         // download start time for log
    char reserved1[MAX_RESERVED_SIZE];         // reserved 1 for extra use
    char reserved2[MAX_RESERVED_SIZE];         // reserved 2 for extra use
    char reserved3[MAX_RESERVED_SIZE];        // reserved 3 for extra use
} PACKED udbp_dl_information_type;


//
// Download entry command is represented with the following structure:
//
typedef  struct {
       byte backup_used;          			 // CS, Factory, World Factory - refer to PST_BACKUP_USE_XXXX
       byte binary_class;            		 // Normal, Master Binary - refer to PST_BINARY_CLASS_XXX
       byte factory_reset_required;  		 // Is required factory reset ? after flashing completed.
       byte device_srd_reset_required;       // Is required reset ?
       word srd_backup_nv_counter ; 		//backup write couter .
       udbp_dl_information_type information;
}PACKED udbp_entry_req_type;

//
// Extra NV Operation command is represented with the following structure:
//
typedef  struct {
    byte    bOperation;
    dword 	dwNvEnumValue;
	dword	dwNam;
    byte    bData[MAX_NV_ITEM_SIZE];  // same as DIAG_NV_ITEM_SIZE
} PACKED udbp_extra_nv_operation_req_type;

typedef  struct {
        byte    bMore;
        dword   dwNam;
		dword   dwseqNum;
        dword   dwTotalSizeOfPrlFile;
        dword   dwSizeOfPrlData;
        byte    bData[MAX_PRL_ITEM_SIZE];
} PACKED udbp_extra_prl_operation_req_type;


typedef  struct {
    udbp_dl_information_type information;
    byte backup_used;             // CS, Factory, World Factory refer to PST_BACKUP_USE_XXXX
}PACKED udbp_get_information_req_type;

typedef  union {
	udbp_entry_req_type       			 do_dl_entry;
	udbp_get_information_req_type        get_dl_information;
	udbp_extra_nv_operation_req_type     extra_nv_operation;
	udbp_extra_prl_operation_req_type    extra_prl_operation;
	
}PACKED udbp_req_data_type;

typedef  struct {
	udbp_header_type header; // common header
	udbp_req_data_type req_data;
}PACKED udbp_req_type;

// RESPONSE --------------------------------------------------------------------------------------

typedef  union {
	udbp_entry_req_type       			 do_dl_entry;
	udbp_get_information_req_type        get_dl_information;
	udbp_extra_nv_operation_req_type     extra_nv_operation;
	udbp_extra_prl_operation_req_type    extra_prl_operation;
	word 	write_sector_counter;  //kabjoo.choi
}PACKED udbp_rsp_data_type;

typedef  struct {
    	udbp_header_type header; // common header
	udbp_rsp_data_type rsp_data;
}PACKED udbp_rsp_type;

typedef enum {
	MODEM_BACKUP=0,
	MDM_BACKUP=1,
	
} script_process_type;

/*===========================================================================
                     BACKUP DATA TYPE DEFINITIONS
===========================================================================*/
typedef  struct {
	udbp_entry_req_type info;
	int dl_info_cnt;
	dword nv_esn;
	qword nv_meid;
	//nv_bd_addr_type nv_bt_address;
}PACKED user_data_backup_info;

typedef  struct 
{
	user_data_backup_info	info_table;
//	user_data_backup_table	nv_table;
}PACKED user_data_backup_item;

#endif
