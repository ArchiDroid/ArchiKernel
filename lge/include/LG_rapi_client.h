/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                      LG RPC Client File
PROJECT
   P715

GENERAL DESCRIPTION
   

Copyright (c) 2010 LG Electronics. All right reserved.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*============================================================================

                      EDIT HISTORY FOR FILE

 This section contains comments describing changes made to this file.
 Notice that changes are listed in reverse chronological order.

 when      who    what, where, why
 --------  -----  ----------------------------------------------------------
12/10/16  khyun.kim Initial Release
============================================================================*/
#ifndef _LG_RAPI_CLIENT_H_
#define _LG_RAPI_CLIENT_H_


/*===========================================================================
                      MACRO DECLARATIONS
===========================================================================*/

/*===========================================================================
                      TYPE DEFINITIONS
===========================================================================*/

#define GET_INT32(buf)       (int32_t)be32_to_cpu(*((uint32_t*)(buf)))
#define PUT_INT32(buf, v)        (*((uint32_t*)buf) = (int32_t)be32_to_cpu((uint32_t)(v)))
#define GET_U_INT32(buf)         ((uint32_t)GET_INT32(buf))
#define PUT_U_INT32(buf, v)      PUT_INT32(buf, (int32_t)(v))

#define GET_LONG(buf)            ((long)GET_INT32(buf))
#define PUT_LONG(buf, v) \
	(*((u_long*)buf) = (long)be32_to_cpu((u_long)(v)))

#define GET_U_LONG(buf)	      ((u_long)GET_LONG(buf))
#define PUT_U_LONG(buf, v)	      PUT_LONG(buf, (long)(v))


#define GET_BOOL(buf)            ((bool_t)GET_LONG(buf))
#define GET_ENUM(buf, t)         ((t)GET_LONG(buf))
#define GET_SHORT(buf)           ((short)GET_LONG(buf))
#define GET_U_SHORT(buf)         ((u_short)GET_LONG(buf))

#define PUT_ENUM(buf, v)         PUT_LONG(buf, (long)(v))
#define PUT_SHORT(buf, v)        PUT_LONG(buf, (long)(v))
#define PUT_U_SHORT(buf, v)      PUT_LONG(buf, (long)(v))

#define LG_RAPI_CLIENT_MAX_OUT_BUFF_SIZE 128
#define LG_RAPI_CLIENT_MAX_IN_BUFF_SIZE 128

typedef enum {
    NORMAL_WORK_FLAG = 0,
    WORK_QUEUE_FLAG = 1,
}send_to_arm9_wq_type;

#define INCREASE_MUTEX_FLAG()         (++send_to_arm9_data.mutex_flag)
#define DECREASE_MUTEX_FLAG()         ((send_to_arm9_data.mutex_flag==0)?(send_to_arm9_data.mutex_flag=0):(send_to_arm9_data.mutex_flag--))
#define GET_MUTEX_FLAG()	(send_to_arm9_data.mutex_flag)
#define SET_MUTEX_FLAG(v)	(send_to_arm9_data.mutex_flag = v)

#define GET_COMPLETE_MAIN()	(send_to_arm9_data.complete_main==0)?0:1
#define SET_COMPLETE_MAIN(v)	(send_to_arm9_data.complete_main = v)
#define GET_COMPLETE_SUB()		(send_to_arm9_data.complete_sub==0)?0:1
#define SET_COMPLETE_SUB(v)	(send_to_arm9_data.complete_sub = v)

typedef enum{
	LG_RAPI_SUCCESS = 0,
	LG_RAPI_OVER_UNDER_FLOW = 1,
	LG_RAPI_INVALID_RESPONSE = 2,
	
}lg_rapi_status_type;

#define EOP 1 			/*operation failed*/
#define ENOENT 2		/**< No such file or directory */
#define EBADF 9		/**< Bad file descriptor */
#define ENOMEM 12         /**< Out of memory. */


static struct workqueue_struct *send_to_arm9_wq = NULL;
struct __send_to_arm9 {
    int mutex_flag;
    boolean complete_main;
    boolean complete_sub;
    void*	pReq;
    void* pRsp;
    struct work_struct work;
};

#define DID_MAGIC_CODE "AABBCCDD"

//DID BACKUP  kabjoo.choi
typedef  struct
{
  int pid_magic_code;  //AABBCCDD
  char pid[60];
  int imei_magic_code;
  char imei[60];
  int swv_magic_code;
  char swv[60];
} test_mode_emmc_direct_type;

struct MmcPartition {
    char *device_index;
    char *filesystem;
    char *name;
    unsigned dstatus;
    unsigned dtype ;
    unsigned dfirstsec;
    unsigned dsize;
};
typedef struct MmcPartition MmcPartition;


/*121015 khyun.kim@lge.com [V7] rpc api for NTCODE [START]*/
// LGE_START 20121113 seonbeom.lee [Security] support NTCODE max 40 .
typedef struct
{
	unsigned int	pkt_no;
	unsigned short	data_len;
	unsigned short	flag;
	unsigned char	data[120];
} PACKED rpc_large_read_pkt_recv;

typedef struct
{
	unsigned int	pkt_no;
	unsigned short	flag;
} PACKED rpc_large_read_pkt_send;

typedef struct
{
	unsigned int	pkt_no;
} PACKED rpc_large_write_pkt_recv;

typedef struct
{
	unsigned int	pkt_no;
	unsigned short	flag;
	unsigned short	data_len;
	unsigned char	data[120];
} PACKED rpc_large_write_pkt_send;

#define RPC_LARGE_READ_CONTINUE		1
#define RPC_LARGE_READ_FINISH			2
#define RPC_LARGE_READ_ERROR			3

// LGE_END 20121113 seonbeom.lee [Security] support NTCODE max 40 .
/*121015 khyun.kim@lge.com [V7] rpc api for NTCODE [END]*/

#endif
