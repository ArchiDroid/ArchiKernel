/* arch/arm/mach-msm/lge/lg_fw_diag_eri.c
 *
 * Copyright (C) 2009,2010 LGE, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/fcntl.h> 
#include <linux/fs.h>
#include <lg_diagcmd.h>
#include <linux/uaccess.h>
#include <lge_diag_eri.h>


#include <linux/syscalls.h>
#include <linux/fcntl.h> 
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/module.h>
#include <linux/input.h>
#include <linux/syscalls.h>
#include <linux/delay.h>


#define ERI_FILE_PATH 	"/data/eri/eri.bin"
//#define DEBUG_ERI

/*
 * EXTERNAL FUNCTION AND VARIABLE DEFINITIONS
 */
extern PACK(void *) diagpkt_alloc(diagpkt_cmd_code_type code,unsigned int length);
extern PACK(void *) diagpkt_free(PACK(void *)pkt);
extern int eri_send_to_arm9(void* pReq, void* pRsp, unsigned int output_length);
//extern int eri_send_to_arm9(void* pReq, void* pRsp, unsigned int output_length);

/* LGE_CHANGE_ [jaekyung83.lee@lge.com] 2011-04-06. ERI UTS AP Write Test  [START]*/
static int write_eri(const char *path, eri_write_req_type* write_req_ptr, eri_write_rsp_type* write_rsp_ptr)
{
	int write;
	int err;
	mm_segment_t oldfs;
#ifdef DEBUG_ERI
	printk(KERN_INFO "%s, UTS ERI Write Test START\n",__func__);
#endif
	oldfs = get_fs();
	set_fs(KERNEL_DS);

#ifdef DEBUG_ERI	
	printk(KERN_ERR "%s, PATH : %s\n",__func__, path);
#endif

	write = sys_open((const char __user *) path, O_WRONLY | O_CREAT | O_TRUNC , 0);

	if(write < 0) {
		printk(KERN_ERR "%s, ERI File Open Fail\n",__func__);
		return -1;
	}else {
		printk(KERN_ERR "%s, ERI File Open Success\n",__func__);
	}

	err = sys_write(write, write_req_ptr->eri_data.user_eri_data, write_req_ptr->eri_data.eri_size);

	if(err < 0){
#ifdef DEBUG_ERI		
		printk(KERN_INFO "%s, ERI File Write Fail\n",__func__);
#endif
		write_rsp_ptr->cmd_code = write_req_ptr->cmd_code;
		write_rsp_ptr->sub_cmd_code = write_req_ptr->sub_cmd_code;
		write_rsp_ptr->status = 1;		
	}
	else {
#ifdef DEBUG_ERI		
		printk(KERN_INFO "%s, UTS ERI WRITE Test Make Rsp PACK\n",__func__);
#endif
		write_rsp_ptr->cmd_code = write_req_ptr->cmd_code;
		write_rsp_ptr->sub_cmd_code = write_req_ptr->sub_cmd_code;
		write_rsp_ptr->status = 0;
	}
	sys_close(write);
	set_fs(oldfs);	

	return 1;
}
/* LGE_CHANGE_ [jaekyung83.lee@lge.com] 2011-04-06. ERI UTS AP Write Test  [END]*/
/* LGE_CHANGE_ [jaekyung83.lee@lge.com] 2011-04-06. ERI UTS AP Read Test  [START]*/
byte eri_buf[1400];
static int read_eri(const char *path, eri_read_req_type* read_req_ptr, eri_read_rsp_type* read_rsp_ptr)
{
	int read;
//	size_t count;
	int read_size;
//	int ret;
	
	mm_segment_t oldfs;
	
#ifdef DEBUG_ERI
	printk(KERN_ERR "%s, UTS ERI READ Test START\n",__func__);	
#endif

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	
	memset(eri_buf,0,sizeof(byte) * 1400);
	
	read = sys_open((const char __user *)path, O_RDONLY , 0);

	if(read < 0) {
#ifdef DEBUG_ERI		
		printk(KERN_ERR "%s, ERI File Open Fail\n",__func__);
#endif
		return -1;
	}else {
#ifdef DEBUG_ERI
		printk(KERN_ERR "%s, ERI File Open Success\n",__func__);
#endif
	}

	read_size = 0;

	while(sys_read(read, &eri_buf[read_size++], 1) == 1){}
		
	memcpy(read_rsp_ptr->eri_data.user_eri_data, eri_buf, read_size);
	
	if(read_size <= 0){
#ifdef DEBUG_ERI		
		printk(KERN_ERR "%s, ERI File Read Fail\n",__func__);
#endif
		read_rsp_ptr->cmd_code = read_req_ptr->cmd_code;
		read_rsp_ptr->sub_cmd_code = read_req_ptr->sub_cmd_code;
		read_rsp_ptr->eri_data.eri_size = 0;
		read_rsp_ptr->status = 1;		
	} else {
#ifdef DEBUG_ERI	
		printk(KERN_ERR "%s, UTS ERI READ Test Make Rsp PACK\n",__func__);
#endif
		read_rsp_ptr->cmd_code = read_req_ptr->cmd_code;
		read_rsp_ptr->sub_cmd_code = read_req_ptr->sub_cmd_code;
		read_rsp_ptr->eri_data.eri_size = read_size - 1;
		read_rsp_ptr->status = 0;
	}
	set_fs(oldfs);
	sys_close(read);
	return read_size;
}
/* LGE_CHANGE_ [jaekyung83.lee@lge.com] 2011-04-06. ERI UTS AP Read Test  [END]*/
/* LGE_CHANGE_ [jaekyung83.lee@lge.com] 2011-04-06. ERI UTS Test  [START]*/
PACK (void *)LGE_ERI (
        PACK (void	*)req_pkt_ptr,	/* pointer to request packet  */
        uint16		pkt_len )		      /* length of request packet   */
{
	eri_req_type* req_ptr = (eri_req_type *)req_pkt_ptr;
	eri_read_rsp_type* read_rsp_ptr = NULL;
	eri_write_rsp_type* write_rsp_ptr = NULL;	
	eri_read_req_type* read_req_ptr = NULL;
	eri_write_req_type* write_req_ptr = NULL;	
	unsigned int rsp_ptr_len;
	int err;

	switch(req_ptr->sub_cmd_code)
	{
		case ERI_READ_SUB_CMD_CODE:
			rsp_ptr_len = sizeof(eri_read_rsp_type);
			read_req_ptr = (eri_read_req_type *)req_pkt_ptr;
			read_rsp_ptr = (eri_read_rsp_type*) diagpkt_alloc (DIAG_ERI_CMD_F, sizeof(eri_read_rsp_type));
			err = read_eri(ERI_FILE_PATH, read_req_ptr ,read_rsp_ptr);

			if(err < 0){
				read_rsp_ptr->eri_data.eri_size = 0;
				read_rsp_ptr->status = 1;
				memset(read_rsp_ptr->eri_data.user_eri_data, 0, sizeof(read_rsp_ptr->eri_data.user_eri_data));
			}
			return(read_rsp_ptr);
		break;

		case ERI_WRITE_SUB_CMD_CODE:
			rsp_ptr_len = sizeof(eri_write_rsp_type);
			write_req_ptr = (eri_write_req_type *)req_pkt_ptr;
			write_rsp_ptr = (eri_write_rsp_type*) diagpkt_alloc (DIAG_ERI_CMD_F, sizeof(eri_write_rsp_type));
			//for writting EFS
			/* LGE_CHANGE_ [jaekyung83.lee@lge.com] 2011-04-06. ERI UTS Write Test EFS /eri/eri.bin   [START]*/
			eri_send_to_arm9((void *)write_req_ptr, (void *)write_rsp_ptr, rsp_ptr_len);
			/* LGE_CHANGE_ [jaekyung83.lee@lge.com] 2011-04-06. ERI UTS Write Test EFS /eri/eri.bin   [END]*/
			//AP /data/eri/eri.bin write
			err = write_eri(ERI_FILE_PATH, write_req_ptr, write_rsp_ptr);
			if(err < 0) {
				write_rsp_ptr->cmd_code = write_req_ptr->cmd_code;
				write_rsp_ptr->sub_cmd_code = write_req_ptr->sub_cmd_code;
				write_rsp_ptr->status = 1;
			}else {
				write_rsp_ptr->cmd_code = write_req_ptr->cmd_code;
				write_rsp_ptr->sub_cmd_code = write_req_ptr->sub_cmd_code;
				write_rsp_ptr->status = 0;
			}
			return (write_rsp_ptr);
		break;	
	}
	return NULL;
}


//restore eri.bin  after factory reset command 

int eri_factory_direct_write(const char *path , char *eri_data, int size )
{
	int fd;
	int err;
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);


	fd = sys_open((const char __user *) path, O_WRONLY | O_CREAT | O_TRUNC , 0);

	if(fd < 0) {
		printk(KERN_ERR "%s, ERI File Open Fail\n",__func__);
		return -1;
	}else {
		printk(KERN_ERR "%s, ERI File Open Success\n",__func__);
	}

	err = sys_write(fd, eri_data, size);

	if(err < 0)
		
	sys_close(fd);
	set_fs(oldfs);	

	return 1;
}
/* LGE_CHANGE_ [jaekyung83.lee@lge.com] 2011-04-06. ERI UTS Test  [END]*/