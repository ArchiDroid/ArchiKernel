#ifndef LG_DIAG_ERI_H
#define LG_DIAG_ERI_H


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

#define ERI_READ_SUB_CMD_CODE		7
#define ERI_WRITE_SUB_CMD_CODE		8

typedef struct
{
	dword eri_size;
	byte user_eri_data[1400];
}NV_ERI_TYPE;

typedef struct
{
	byte cmd_code;
	word sub_cmd_code;
	NV_ERI_TYPE eri_data;
}PACKED eri_req_type;

typedef struct
{
	byte cmd_code;
	word sub_cmd_code;
}PACKED eri_read_req_type;

typedef struct
{
	byte cmd_code;
	word sub_cmd_code;
	byte status;
	NV_ERI_TYPE eri_data;
}PACKED eri_read_rsp_type;

typedef struct
{
	byte cmd_code;
	word sub_cmd_code;
	NV_ERI_TYPE eri_data;
}PACKED eri_write_req_type;

typedef struct
{
	byte cmd_code;
	word sub_cmd_code;
	byte status;
}PACKED eri_write_rsp_type;

extern PACK (void *)LGE_ERI (PACK (void *)req_pkt_ptr,uint16 pkt_len );
#endif
