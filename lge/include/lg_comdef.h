#ifndef LG_COMDEF_H
#define LG_COMDEF_H
// LG_FW_DIAG_KERNEL_SERVICE

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned char boolean;
typedef unsigned long qword[ 2 ];

typedef uint8 diagpkt_cmd_code_type;
typedef uint8 diagpkt_subsys_id_type;
typedef uint16 diagpkt_subsys_cmd_code_type;
typedef uint32 diagpkt_subsys_status_type;
typedef uint16 diagpkt_subsys_delayed_rsp_id_type;
typedef uint16 diagpkt_subsys_rsp_cnt;

#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0
/*LGE_CHANGE_S, 2010-10-08, [dongp.kim@lge.com], #define LG_FW_WLAN_TEST for WI-FI TestMode */
#define LG_FW_WLAN_TEST 1 
/*LGE_CHANGE_E, 2010-10-08, [dongp.kim@lge.com], #define LG_FW_WLAN_TEST for WI-FI TestMode */
#endif /* LG_COMDEF_H */
