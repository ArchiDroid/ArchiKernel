#ifndef LG_DIAG_UDM_H
#define LG_DIAG_UDM_H
// LG_FW_DIAG_UDM

#include "lg_comdef.h"
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
#define SMS_MAX_USER_DATA_LEN			220 
#define SMS_UTS_MAX_USERDATA_LEN		SMS_MAX_USER_DATA_LEN+1
#define SMS_UTS_MAX_ADDR_LEN 			20
#define SMS_DATA 1024

typedef enum {
	UDM_SUB_CMD_GET_STATUS = 0,
	UDM_SUB_CMD_GET_SMS = 4,
	UDM_SUB_CMD_SET_SMS = 5,
	UDM_SUB_CMD_MAX
} udm_sub_cmd_type;

typedef enum
{
	UDM_OK_S,
	UDM_FAIL_S,
	UDM_NOT_SUPPORTED_S
} PACKED udm_mode_ret_stat_type;

typedef struct {
  char add_number[20]; 
  char user_data[SMS_MAX_USER_DATA_LEN+1];
}PACKED udm_mode_sms_req_struct_type;

typedef  struct 
{
	char address[SMS_UTS_MAX_ADDR_LEN];
	char messagebody[SMS_UTS_MAX_USERDATA_LEN];
}PACKED EXT_SMS_COMPOSE_PARAMETER_INFO;

typedef union
{
//	udm_mode_sms_req_struct_type text_msg;
	EXT_SMS_COMPOSE_PARAMETER_INFO sms_send_data;
//	EXT_SMS_COMPOSE_PARAMETER_INFO sms_get_data;
}PACKED  udm_mode_req_type;

typedef  struct
{
  word sub_cmd_code;		   /* Use test_mode_sub_cmd_type. */
  udm_mode_req_type udm_mode_req;
}PACKED diag_udm_mode_req_type;


typedef struct DIAG_UDM_MODE_req_tag {
  byte		xx_header;
  diag_udm_mode_req_type  nrb_udm_mode;
} PACKED DIAG_UDM_MODE_req_type;



typedef struct {
  byte sms_state;
  char user_data[SMS_MAX_USER_DATA_LEN+1];
}PACKED udm_mode_sms_rsp_struct_type;

typedef struct 
{
	char address[SMS_UTS_MAX_ADDR_LEN];
	char messagebody[SMS_UTS_MAX_USERDATA_LEN];
}PACKED EXT_SMS_PARAMETER_INFO;

typedef union
{
//  udm_mode_sms_rsp_struct_type text_msg;
  EXT_SMS_PARAMETER_INFO sms_get_data;
  dword annun; 
}PACKED udm_mode_rsp_type;

typedef struct
{
  word sub_cmd_code;  /* Use test_mode_sub_cmd_type. */
  udm_mode_ret_stat_type ret_stat_code; /* Status to return. Use diag_test_mode_ret_stat_type. */
  udm_mode_rsp_type udm_mode_rsp;
}PACKED diag_udm_mode_rsp_type;

typedef struct DIAG_UDM_MODE_rsp_tag {
  byte		xx_header;
  diag_udm_mode_rsp_type  nrb_udm_mode;
} PACKED DIAG_UDM_MODE_rsp_type;

typedef struct
{
	uint16 set_data[SMS_DATA];
}udm_sms_status_new;

#endif /* LG_DIAG_UDM_H */
