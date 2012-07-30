#ifndef LG_DIAG_MSGTEST_H
#define LG_DIAG_MSGTEST_H
// LG_FW_DIAG_KERNEL_SERVICE

#include "lg_comdef.h"


/*********************** BEGIN PACK() Definition ***************************/
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

//#ifdef LG_SMS_PC_TEST
typedef union
{
  boolean retvalue;
} PACKED sms_mode_rsp_type;

typedef struct
{
  word sub_cmd_code;	/* Use test_mode_sub_cmd_type. */
  byte ret_stat_code; /* Status to return. Use diag_test_mode_ret_stat_type. */
  sms_mode_rsp_type sms_mode_rsp;
} PACKED diag_sms_mode_rsp_type;

typedef struct
{
  byte testtype;
  byte typeofmmsDb;
  word numofmsgs;
  byte totalsegment;
  byte currentsegment;
  word lengthofconfdata;
  char confdata[500];
} diag_mms_mode_conf_type; 
//#endif


#endif /* LG_DIAG_MSGTEST_H */
