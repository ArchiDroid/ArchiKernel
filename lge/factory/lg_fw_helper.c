#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <mach/oem_rapi_client.h>

#include <linux/kmod.h>
#include <linux/workqueue.h>

#include <lg_fw_helper.h>
#include <linux/module.h>

static struct msm_rpc_client *client;

int lg_fw_helper_send_to_cp(unsigned int cmd,
							void* data_in, int data_in_len,
							void** data_out, int* data_out_len)
{
	struct oem_rapi_client_streaming_func_arg arg;
	struct oem_rapi_client_streaming_func_ret ret;
	int rc = -1;

	memset(&arg, 0x00, sizeof(struct oem_rapi_client_streaming_func_arg));
	memset(&ret, 0x00, sizeof(struct oem_rapi_client_streaming_func_ret));

	if (data_in_len < 0 || data_in == NULL) {
		data_in_len = 0;
		data_in = NULL;
	}

	if (data_out_len == NULL || *data_out_len < 0 || data_out == NULL) {
		if (data_out_len != NULL)
			*data_out_len = -1;
		data_out_len = NULL;
		data_out = NULL;
	}

	// make request structure
	arg.event			= LG_FW_HELPER_OEM_RAPI;
	arg.in_len			= data_in_len;
	arg.input			= data_in;
	arg.out_len_valid	= (data_out_len != NULL) ? 1 : 0;
	arg.output_valid	= (data_out != NULL) ? 1 : 0;
	arg.output_size		= (data_out_len != NULL) ? *data_out_len : 0;

	// start communication
	rc = oem_rapi_client_streaming_function(client, &arg, &ret);
	if (rc < 0 || (data_out != NULL && ret.output == NULL) || (data_out_len != NULL && ret.out_len == NULL)) {
		if (ret.output)
			kfree(ret.output);
		if (ret.out_len)
			kfree(ret.out_len);
		pr_err("  *** failed to communicate with CP\n");
		return rc;
	}

	// copy result
	if (data_out_len != NULL) {
		*data_out_len = *ret.out_len;
		kfree(ret.out_len);
	}

	if (data_out != NULL)
		*data_out = ret.output;

	return rc;
}
EXPORT_SYMBOL(lg_fw_helper_send_to_cp);

int lg_fw_helper_send_to_cp2(struct lg_fw_rpc* rpc)
{
	int ret;

	if (rpc->cmd == 0)
		return -1;

	if ((rpc->data_in_len < 0) ||
		(rpc->data_in != NULL && rpc->data_in_len == 0) ||
		(rpc->data_in_len > 0 && rpc->data_in == NULL))
		return -1;

	if (rpc->data_out_len < 0)
		return -1;

	rpc->data_out = NULL;
	ret = lg_fw_helper_send_to_cp(rpc->cmd,
				rpc->data_in, rpc->data_in_len,
				&rpc->data_out, &rpc->data_out_len);
	return ret;
}
EXPORT_SYMBOL(lg_fw_helper_send_to_cp2);

// lg fw rpc server handler
enum {
	LG_FW_HELPER_NULL			= 0,

	LG_FW_HELPER_MISC_BLK_READ	= 10,
	LG_FW_HELPER_MISC_BLK_WRITE,

	LG_FW_HELPER_RPC_MAX
};

struct lg_fw_helper_misc_blk_rpc {
	uint32_t block_no;
	uint32_t *data;
	uint32_t size;
};

extern int lge_emmc_wallpaper_write(unsigned int blockNo, const char* buffer, int size);
static int handle_lg_fw_helper_misc_blk_write(struct msm_rpc_xdr *xdr)
{
	int ret;
	struct lg_fw_helper_misc_blk_rpc arg;

	// get block number
	ret = xdr_recv_uint32(xdr, &arg.block_no);
	if (ret != 0)
		return ret;

	// get block data
	ret = xdr_recv_array(xdr, (void **)&arg.data, &arg.size, 128,
		       sizeof(uint32_t), (void *)xdr_recv_uint32);
	if (ret != 0)
		return ret;

	// write block data
	print_hex_dump(KERN_INFO, "data: ", DUMP_PREFIX_OFFSET, 16, 1, arg.data, 512, true);
	printk(KERN_ERR "[sunny] block num = %d \n", arg.block_no);
	
	ret = lge_emmc_wallpaper_write(arg.block_no, (unsigned char*) arg.data, 512);

	ret = xdr_start_accepted_reply(xdr, RPC_ACCEPTSTAT_SUCCESS);
	if (ret != 0)
		goto handle_lg_fw_helper_misc_blk_write_exit;

	ret = 0xabcdef12; // for debugging
	ret = xdr_send_uint32(xdr, &ret);
	if (ret != 0)
		goto handle_lg_fw_helper_misc_blk_write_exit;

	ret = xdr_send_msg(xdr);
	if (ret < 0)
		 pr_err("%s: sending reply failed\n", __func__);

handle_lg_fw_helper_misc_blk_write_exit:
	if (arg.data)
		kfree(arg.data);
	return 0;
}

static int lg_fw_helper_rpc_server_call(struct msm_rpc_server *server,
										struct rpc_request_hdr *req,
										struct msm_rpc_xdr *xdr)
{
	switch (req->procedure)
	{
	case LG_FW_HELPER_MISC_BLK_WRITE:
		return handle_lg_fw_helper_misc_blk_write(xdr);

	case LG_FW_HELPER_NULL:
		pr_debug("helper rpc received NULL cmd\n");
		return 0;

	default:
		return -ENODEV;
	}
}

static struct msm_rpc_server rpc_server =
{
	.prog		= LG_FW_HELPER_RPC_APISPROG,
	.vers		= LG_FW_HELPER_RPC_APISVERS,
	.rpc_call2	= lg_fw_helper_rpc_server_call,
};

static int __init lg_fw_helper_init(void)
{
	int ret;

	// RPC server
	ret = msm_rpc_create_server2(&rpc_server);
	if (ret < 0) {
		pr_err("unable to init fw helper rpc server\n");
		return ret;
	}

	// OEM RAPI client
	client = oem_rapi_client_init();
	if (IS_ERR(client)) {
		pr_err("unable to init rapi client\n");
		return PTR_ERR(client);
	}

	return 0;
}

static void __exit lg_fw_helper_exit(void)
{
	oem_rapi_client_close();
}

module_init(lg_fw_helper_init);
module_exit(lg_fw_helper_exit);
  
MODULE_DESCRIPTION("LGE RPC Helper Module");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LGE P2 SW3");

MODULE_ALIAS("rpc:helper");
