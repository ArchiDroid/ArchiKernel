#ifndef LG_DIAG_DLOAD_H
#define LG_DIAG_DLOAD_H

#include <linux/kernel.h>
#include <linux/fs.h>

#include "lg_diag_cfg.h"
#include "lg_comdef.h"
#include "lg_diagcmd.h"


/*********************** BEGIN PACK() Definition ***************************/
#if defined __GNUC__
  #define PACK(x)       x __attribute__((__packed__))
  #define PACKED        __attribute__((__packed__))
#elif defined __arm
  #define PACK(x)       __packed x
  #define PACKED        __packed
#else
  #error No PACK() macro defined for this compiler
#endif
/********************** END PACK() Definition *****************************/


// LG_FW : 2011.07.07 moon.yongho : saving webdload status variable to eMMC. ----------]]
#ifdef LG_FW_WEB_DOWNLOAD	

/* mode */
#define CONNECT_MODE_DIAG		0x01
#define CONNECT_MODE_DLOAD		0x02
#define CONNECT_MODE_PMDIAG		0x03
#define CONNECT_MODE_BOGUS		0xFF


#define WEBDLOAD_MAX_RSP_LEN	20
#define VER_NAME_LEN			8
#define SUB_REV_LEN				2
#define RSVD_ARRAY_LEN			8
#define CS_VER_LEN				6
#define USB_STRM_CMD_NUM		4 // USB STREM Change Command 의 개수
#define MODEL_NAME_LEN			10

#define BACKUP_BYTE_MAX_SIZE		512//2048    //#define FLASH_NAND_PAGE_NUM_BYTES 2048
#define WEBDLOAD_INIT_VAL ((word)(~STEP_CLEARED))
#define WEB_DLOAD_ENTRY 0xDEADDEAD


#define WEBDLOAD_LANG_UNKNOWN		0xFFFF // 양산 초기값(Flash 가 Erase 된 상태)
#define WEBDLOAD_LANG_TRUST_PHONE	0xFFFE
#define WEBDLOAD_LANG_KOREAN		0x0000
#define WEBDLOAD_LANG_ENGLISH		0x0001

//Current Stream 정의
typedef enum
{
  LS_STREAM		= 0,
  HS_STREAM
} usbdc_curr_diag_strm_type;

typedef struct
{
  word step_real;
  word step_ctrl;
  word language;
  byte rsp_needed;
} webdload_type;

/*---------------------------------------------------------------------------
  WebDownload 시 Stage 3 의 각 Step 을 정의.
  Enum 의 중간에 끼워 넣기 하지 마시고 새로 추가 되는 것은 항상 제일 뒤에
  넣기 바랍니다.

                                          : Step 1. Battery Level Check.
                                              -. status info 에 Phone Binary Version 기입
 STEP_C_DRV_ERASE           : Step 2. C Drive Erase
 STEP_DLOAD_CHANGE        : Step 3. Dload Chagne
 STEP_BIN_RAM_WR             : Step 4. Main Binary Download to SDRAM (PC to Phone)
 STEP_BIN_FLASH_WR          : Step 5. Code 영역 Erase, Flash Burning (SDRAM to Flash)
 STEP_BURN_END_RESET      : Step 6. Reset Phone
 STEP_C_DRV_WR                : Step 7. C Driver Write (PMAgent Diag)
 STEP_DLOAD_CHANGE_DSP : Step 8. Dload Change for DSP Binary Burning
 STEP_CLEARED                    : Step 9. Reset Phone
                                               -. Clear status info.
 ---------------------------------------------------------------------------*/
typedef  enum
{
  STEP_C_DRV_ERASE		= 0x0002,
  STEP_DLOAD_CHANGE		= 0x0003,
  STEP_BIN_RAM_WR		= 0x0004,
  STEP_BIN_FLASH_WR		= 0x0005,
  STEP_BURN_END_RESET		= 0x0006,
  STEP_C_DRV_WR			= 0x0007,
  STEP_DLOAD_CHANGE_DSP		= 0x0008,
  STEP_CLEARED			= 0xFFFF, // 양산 초기값(Flash 가 Erase 된 상태)
}PACKED sib_step_type;

typedef  struct
{
  sib_step_type current_step;
  word webdload_cnt;
  char bin_ver[VER_NAME_LEN];
  word auth_mark;
  char sub_rev[SUB_REV_LEN];
  word language;
  qword meid;
  char model_name[MODEL_NAME_LEN];
  /*--------------------*/
  word reserved_val1;
  word reserved_val2;
  char reserved_array1[RSVD_ARRAY_LEN];
  char reserved_array2[RSVD_ARRAY_LEN];
  /*--------------------*/
  dword web_dload_entry;		//webdownload entry 에 사용함
}PACKED status_info_type;


typedef enum
{
  WEBDLOAD_CONNECT_MODE			= 0x00, /*  0 */
  WEBDLOAD_READ_STEP_END		= 0x01, /*  1 */
  WEBDLOAD_WRITE_STEP_END		= 0x02, /*  2 */
  WEBDLOAD_READ_CNT			= 0x03, /*  3 */
  WEBDLOAD_WRITE_CNT			= 0x04, /*  4 */
  WEBDLOAD_READ_BIN_VER			= 0x05, /*  5 */
  WEBDLOAD_WRITE_BIN_VER		= 0x06, /*  6 */
  WEBDLOAD_CLEAR_SIB			= 0x07, /*  7 */
  WEBDLOAD_READ_BATT_LVL		= 0x08, /*  8 */
  WEBDLOAD_VAL_READ_STEP_END		= 0x09, /*  9 */
  WEBDLOAD_VAL_READ_CNT			= 0x0A, /* 10 */
  WEBDLOAD_DISP_IMAGE			= 0x0B, /* 11 */
  WEBDLOAD_SET_RSP_NEED			= 0x0C, /* 12 */
  WEBDLOAD_GET_USB_STRM_INFO		= 0x0D, /* 13 */
  WEBDLOAD_SET_AUTH_MARK		= 0x0E, /* 14 */
  WEBDLOAD_GET_AUTH_MARK		= 0x0F, /* 15 */
  WEBDLOAD_READ_SUB_REV			= 0x10, /* 16 */
  WEBDLOAD_WRITE_SUB_REV		= 0x11, /* 17 */
  WEBDLOAD_READ_LANGUAGE		= 0x12, /* 18 */
  WEBDLOAD_WRITE_LANGUAGE		= 0x13, /* 19 */
  WEBDLOAD_DEL_C_DRV			= 0x14, /* 20 */
  WEBDLOAD_DEL_IMG_DRV			= 0x15, /* 21 */
  WEBDLOAD_GET_USB_STRM_CMD		= 0x16, /* 22 */
  WEBDLOAD_READ_BL_VER			= 0x17, /* 23 */
  WEBDLOAD_DL_RESET			= 0x18, /* 24 added by obd...for change dload and reset*/
  /*--------------------*/
//#ifdef LG_FW_WEB_DOWNLOAD_MEID_INFO
  WEBDLOAD_GET_MEID			= 0x31, /*49*/ // Get MEID
//#endif /*LG_FW_WEB_DOWNLOAD_MEID_INFO*/
  WEBDLOAD_READ_RSVD_VAL1		= 0x40, /* 64 */
  WEBDLOAD_WRITE_RSVD_VAL1		= 0x41, /* 65 */
  WEBDLOAD_READ_RSVD_VAL2		= 0x42, /* 66 */
  WEBDLOAD_WRITE_RSVD_VAL2		= 0x43, /* 67 */
  WEBDLOAD_READ_RSVD_ARRAY1		= 0x50, /* 80 */
  WEBDLOAD_WRITE_RSVD_ARRAY1		= 0x51, /* 81 */
  WEBDLOAD_READ_RSVD_ARRAY2		= 0x52, /* 82 */
  WEBDLOAD_WRITE_RSVD_ARRAY2		= 0x53, /* 83 */

  WEBDLOAD_2CHIP_EMERG_CEHCK = 0x54, /* 84 */
  WEBDLOAD_2CHIP_EMERG_IMEI  = 0x60, /* 96 */
  WEBDLOAD_LTE_TYPE_CHECK  = 0x61, /* 97 */

  WEBDLOAD_MODEL_NAME			= 0x80, /* 128 */
  /*--------------------*/
  WEBDLOAD_SUB_CMD_END			= 0x7F
} webdload_sub_cmd_type;



/* WebDownload RPC request & response  packet structure  
      diag_webdload_req_type , diag_webdload_rsp_type */
/* Please !  See Modem source In Webdload.c (modem_proc\firmware\itf\webdload) */
/*============= request packet ==================*/
typedef union
{
  byte connection_mode;
  word current_step;
  word webdload_cnt;
  char bin_ver[VER_NAME_LEN/*8*/];
  byte rsp_needed;
  word auth_mark;
  char sub_rev[SUB_REV_LEN/*2*/];
  word language;
  /*--------------------*/
  word reserved_val1;
  word reserved_val2;
  char reserved_array1[RSVD_ARRAY_LEN/* 8 */];
  char reserved_array2[RSVD_ARRAY_LEN];
  /*--------------------*/
}PACKED webdload_req_type;

typedef struct
{
  byte cmd_code;               /* Command code */
  byte sub_cmd;                /* Use webdload_sub_cmd_type */
  webdload_req_type       req_data; 
}PACKED diag_webdload_req_type;

typedef struct
{
  byte cmd_code;
  byte sub_cmd;
  char model_name[MODEL_NAME_LEN];			/* model_name[9] =  NULL */
}PACKED diag_webdload_name_req_type;

/*============= response packet ==================*/
typedef union
{
  byte    connection_mode;
  word    current_step;
  word    webdload_cnt;
  char    bin_ver[VER_NAME_LEN/*8*/];
  byte    batt_lvl;
  byte    curr_strm;
  word    auth_mark;
  char    sub_rev[SUB_REV_LEN/*2*/];
  word    language;
  byte    usb_strm_cmd[USB_STRM_CMD_NUM/*4*/];
  /*--------------------*/
//#ifdef LG_FW_WEB_DOWNLOAD_MEID_INFO-------------------> modem s/w에   나중에 명시할 것 
  qword    meid;
//#endif /*LG_FW_WEB_DOWNLOAD_MEID_INFO*/
  word    reserved_val1;
  word    reserved_val2;
  char    reserved_array1[RSVD_ARRAY_LEN/* 8 */];
  char    reserved_array2[RSVD_ARRAY_LEN];
}PACKED webdload_rsp_type;

typedef struct
{
  byte cmd_code;
  byte sub_cmd;                /* Use webdload_sub_cmd_type */
  byte success;
  webdload_rsp_type   rsp_data;   
}PACKED diag_webdload_rsp_type;



/*=========== FUNCTION define ==================*/
void webdload_common_cmd
(
  byte* cmd_buf,
  byte* rsp_buf,
  byte  mode
);

void webdload_memset(byte* buff, byte val, int len);


#endif /*LG_FW_WEB_DOWNLOAD*/	
// LG_FW : 2011.07.07 moon.yongho -----------------------------------------------------]]

#endif /* LG_DIAG_DLOAD_H */
