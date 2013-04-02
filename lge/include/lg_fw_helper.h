#ifndef _LG_FW_HELPER_H_
#define _LG_FW_HELPER_H_

#define LG_FW_HELPER_RPC_APISPROG	0x30001432
#define LG_FW_HELPER_RPC_APISVERS	0

struct lg_fw_rpc
{
	unsigned int cmd;
	unsigned int flag;
	void* data_in;
	int data_in_len;
	void* data_out;
	int data_out_len;
};

int lg_fw_helper_send_to_cp(unsigned int cmd,
							void* data_in, int data_in_len,
							void** data_out, int* data_out_len);

int lg_fw_helper_send_to_cp2(struct lg_fw_rpc* rpc);
#endif /* _LG_FW_HELPER_H_ */
