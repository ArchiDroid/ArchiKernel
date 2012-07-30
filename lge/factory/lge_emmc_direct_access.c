/*
 * arch/arm/mach-msm/lge/lge_emmc_direct_access.c
 *
 * Copyright (C) 2010 LGE, Inc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <asm/div64.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

/* BEGIN: 0013860 jihoon.lee@lge.com 20110111 */
/* ADD 0013860: [FACTORY RESET] ERI file save */
#ifdef CONFIG_LGE_ERI_DOWNLOAD
#include <linux/kmod.h>
#include <linux/workqueue.h>
#endif
/* END: 0013860 jihoon.lee@lge.com 20110111 */

#include "lg_backup_items.h"

/* Some useful define used to access the MBR/EBR table */
//#define BLOCK_SIZE                0x200
#define TABLE_ENTRY_0             0x1BE
#define TABLE_ENTRY_1             0x1CE
#define TABLE_ENTRY_2             0x1DE
#define TABLE_ENTRY_3             0x1EE
#define TABLE_SIGNATURE           0x1FE
#define TABLE_ENTRY_SIZE          0x010

#define OFFSET_STATUS             0x00
#define OFFSET_TYPE               0x04
#define OFFSET_FIRST_SEC          0x08
#define OFFSET_SIZE               0x0C
#define COPYBUFF_SIZE             (1024 * 16)
#define BINARY_IN_TABLE_SIZE      (16 * 512)
#define MAX_FILE_ENTRIES          20

#define MMC_BOOT_TYPE 0x48
#define MMC_SYSTEM_TYPE 0x78
#define MMC_USERDATA_TYPE 0x79

#define MMC_RCA 2

#define MAX_PARTITIONS 64

#define GET_LWORD_FROM_BYTE(x)    ((unsigned)*(x) | \
        ((unsigned)*((x)+1) << 8) | \
        ((unsigned)*((x)+2) << 16) | \
        ((unsigned)*((x)+3) << 24))

#define PUT_LWORD_TO_BYTE(x, y)   do{*(x) = (y) & 0xff;     \
    *((x)+1) = ((y) >> 8) & 0xff;     \
    *((x)+2) = ((y) >> 16) & 0xff;     \
    *((x)+3) = ((y) >> 24) & 0xff; }while(0)

#define GET_PAR_NUM_FROM_POS(x) ((((x) & 0x0000FF00) >> 8) + ((x) & 0x000000FF))

#define MMC_BOOT_TYPE 0x48
#define MMC_EXT3_TYPE 0x83
#define MMC_VFAT_TYPE 0xC

// BEGIN: 0010090 sehyuny.kim@lge.com 2010-10-21
// MOD 0010090: [FactoryReset] Enable Recovery mode FactoryReset
#define MMC_RECOVERY_TYPE		0x60
#define MMC_MISC_TYPE 0x77
#define MMC_XCALBACKUP_TYPE 0x6E
// END: 0010090 sehyuny.kim@lge.com 2010-10-21


typedef struct MmcPartition MmcPartition;

static unsigned ext3_count = 0;
static char *ext3_partitions[] = {"persist", "bsp", "blb", "tombstones", "drm", "fota", "system", "cache", "wallpaper" , "userdata", "NONE"};

static unsigned vfat_count = 0;
static char *vfat_partitions[] = {"modem", "mdm",  "NONE"};



struct MmcPartition {
    char *device_index;
    char *filesystem;
    char *name;
    unsigned dstatus;
    unsigned dtype ;
    unsigned dfirstsec;
    unsigned dsize;
};

typedef struct {
    MmcPartition *partitions;
    int partitions_allocd;
    int partition_count;
} MmcState;

static MmcState g_mmc_state = {
    NULL,   // partitions
    0,      // partitions_allocd
    -1      // partition_count
};

typedef struct {
	char ret[32];
} testmode_rsp_from_diag_type;

// BEGIN: 0009484 sehyuny.kim@lge.com 2010-09-24
// MOD 0009484: [FactoryReset] Enable FactoryReset
#define FACTORY_RESET_STR_SIZE 11
#define FACTORY_RESET_STR "FACT_RESET_"
// END: 0009484 sehyuny.kim@lge.com 2010-09-24
#define MMC_DEVICENAME "/dev/block/mmcblk0"

// chinsu.ko@lge.com
//static unsigned char global_buf[FACTORY_RESET_STR_SIZE+2];

/* BEGIN: 0013860 jihoon.lee@lge.com 20110111 */
/* ADD 0013860: [FACTORY RESET] ERI file save */
/* make work queue so that rpc for eri does not affect to the factory reset */
#ifdef CONFIG_LGE_ERI_DOWNLOAD
extern void remote_eri_rpc(void);

static struct workqueue_struct *eri_dload_wq;
struct __eri_data {
    unsigned long flag;
    struct work_struct work;
};
static struct __eri_data eri_dload_data;

static void eri_dload_func(struct work_struct *work);
#endif
/* END: 0013860 jihoon.lee@lge.com 20110111 */



//DID BACKUP
static struct workqueue_struct *did_dload_wq;
struct __did_data {
    unsigned long flag;
    struct work_struct work;
};
static struct __did_data did_dload_data;

static void did_dload_func(struct work_struct *work);



int lge_erase_block(int secnum, size_t size);
int lge_write_block(unsigned int secnum, unsigned char *buf, size_t size);
int lge_read_block(unsigned int secnum, unsigned char *buf, size_t size);


static int dummy_arg;

int boot_info = 0;
//2011.08.09 younghoon.jeung temporary flag for MDM error dump
//extern char mdm_error_fatal_for_did;

//[START] LGE_BOOTCOMPLETE_INFO
//2011.07.21 jihoon.lee change module_param to module_param_call to see the log
static int boot_info_write(const char *val, struct kernel_param *kp)
{
	unsigned long flag=0;

	if(val == NULL)
	{
		printk(KERN_ERR "%s, NULL buf\n", __func__);
		return -1;
	}

	flag = simple_strtoul(val,NULL,10);
	boot_info = (int)flag;
	printk(KERN_INFO "%s, flag : %d\n", __func__, boot_info);


    //2011.08.09 younghoon.jeung temporary flag for MDM error dump
    //if(!mdm_error_fatal_for_did)
	  //  queue_work(did_dload_wq, &did_dload_data.work); //DID BACKUP support to DLOD Mode
//
	return 0;
}

//module_param(boot_info, int, S_IWUSR | S_IRUGO);
module_param_call(boot_info, boot_info_write, param_get_int, &boot_info, S_IWUSR | S_IRUGO);
//[END] LGE_BOOTCOMPLETE_INFO

int lg_manual_test_mode = 0;
int manual_test_mode =0;
extern int msm_get_manual_test_mode(void);

static int android_get_manual_test_mode(char *buffer, struct kernel_param *kp)
{
	int ret;
    lg_manual_test_mode = msm_get_manual_test_mode();
    printk(KERN_ERR "[%s] get manual test mode - %d\n", __func__, lg_manual_test_mode);
	ret = sprintf(buffer, "%d", lg_manual_test_mode);
	return ret;
}

module_param_call(manual_test_mode, NULL, android_get_manual_test_mode, &manual_test_mode, 0444);

int db_integrity_ready = 0;
module_param(db_integrity_ready, int, S_IWUSR | S_IRUGO);

int fpri_crc_ready = 0;
module_param(fpri_crc_ready, int, S_IWUSR | S_IRUGO);

int file_crc_ready = 0;
module_param(file_crc_ready, int, S_IWUSR | S_IRUGO);

int db_dump_ready = 0;
module_param(db_dump_ready, int, S_IWUSR | S_IRUGO);

int db_copy_ready = 0;
module_param(db_copy_ready, int, S_IWUSR | S_IRUGO);

int external_memory_test_diag = 0;
module_param(external_memory_test_diag, int, S_IWUSR | S_IRUGO);

// LGE_UPDATE_FOTA_S M3 bryan.oh@lge.com 2011/10/18
int fota_id_check = 0;
module_param(fota_id_check, int, S_IWUSR | S_IRUGO);

static char *fota_id_read = "fail";
module_param(fota_id_read, charp, S_IWUSR | S_IRUGO);

// LGE_UPDATE_FOTA_E M3 bryan.oh@lge.com 2011/10/18


testmode_rsp_from_diag_type integrity_ret;
static int integrity_ret_write(const char *val, struct kernel_param *kp)
{
	memcpy(integrity_ret.ret, val, 32);
	return 0;
}
static int integrity_ret_read(char *buf, struct kernel_param *kp)
{
	memcpy(buf, integrity_ret.ret, 32);
	return 0;
}

module_param_call(integrity_ret, integrity_ret_write, integrity_ret_read, &dummy_arg, S_IWUSR | S_IRUGO);

static char *lge_strdup(const char *str)
{
	size_t len;
	char *copy;
	
	len = strlen(str) + 1;
	copy = kmalloc(len, GFP_KERNEL);
	if (copy == NULL)
		return NULL;
	memcpy(copy, str, len);
	return copy;
}

int lge_erase_block(int bytes_pos, size_t erase_size)
{
	unsigned char *erasebuf;
	unsigned written = 0;
	erasebuf = kmalloc(erase_size, GFP_KERNEL);
	// allocation exception handling
	if(!erasebuf)
	{
		printk("%s, allocation failed, expected size : %d\n", __func__, erase_size);
		return 0;
	}
	memset(erasebuf, 0xff, erase_size);
	written += lge_write_block(bytes_pos, erasebuf, erase_size);

	kfree(erasebuf);

	return written;
}
EXPORT_SYMBOL(lge_erase_block);

/* BEGIN: 0014570 jihoon.lee@lge.com 20110122 */
/* MOD 0014570: [FACTORY RESET] change system call to filp function for handling the flag */
int lge_write_block(unsigned int bytes_pos, unsigned char *buf, size_t size)
{
	struct file *phMscd_Filp = NULL;
	mm_segment_t old_fs;
	unsigned int write_bytes = 0;

	// exception handling
	if((buf == NULL) || size <= 0)
	{
		printk(KERN_ERR "%s, NULL buffer or NULL size : %d\n", __func__, size);
		return 0;
	}
		
	old_fs=get_fs();
	set_fs(get_ds());

	// change from sys operation to flip operation, do not use system call since this routine is also system call service.
	// set O_SYNC for synchronous file io
	phMscd_Filp = filp_open(MMC_DEVICENAME, O_RDWR | O_SYNC, 0);
	if( !phMscd_Filp)
	{
		printk(KERN_ERR "%s, Can not access 0x%x bytes postition\n", __func__, bytes_pos );
		goto write_fail;
	}

	phMscd_Filp->f_pos = (loff_t)bytes_pos;
	write_bytes = phMscd_Filp->f_op->write(phMscd_Filp, buf, size, &phMscd_Filp->f_pos);

	if(write_bytes <= 0)
	{
		printk(KERN_ERR "%s, Can not write 0x%x bytes postition %d size \n", __func__, bytes_pos, size);
		goto write_fail;
	}

write_fail:
	if(phMscd_Filp != NULL)
		filp_close(phMscd_Filp,NULL);
	set_fs(old_fs); 
	return write_bytes;
	
}
/* END: 0014570 jihoon.lee@lge.com 2011022 */

EXPORT_SYMBOL(lge_write_block);

/* BEGIN: 0014570 jihoon.lee@lge.com 20110122 */
/* MOD 0014570: [FACTORY RESET] change system call to filp function for handling the flag */
int lge_read_block(unsigned int bytes_pos, unsigned char *buf, size_t size)
{
	struct file *phMscd_Filp = NULL;
	mm_segment_t old_fs;
	unsigned int read_bytes = 0;

	// exception handling
	if((buf == NULL) || size <= 0)
	{
		printk(KERN_ERR "%s, NULL buffer or NULL size : %d\n", __func__, size);
		return 0;
	}
		
	old_fs=get_fs();
	set_fs(get_ds());

	// change from sys operation to flip operation, do not use system call since this routine is also system call service.
	phMscd_Filp = filp_open(MMC_DEVICENAME, O_RDONLY, 0);
	if( !phMscd_Filp)
	{
		printk(KERN_ERR "%s, Can not access 0x%x bytes postition\n", __func__, bytes_pos );
		goto read_fail;
	}

	phMscd_Filp->f_pos = (loff_t)bytes_pos;
	read_bytes = phMscd_Filp->f_op->read(phMscd_Filp, buf, size, &phMscd_Filp->f_pos);

	if(read_bytes <= 0)
	{
		printk(KERN_ERR "%s, Can not read 0x%x bytes postition %d size \n", __func__, bytes_pos, size);
		goto read_fail;
	}

read_fail:
	if(phMscd_Filp != NULL)
		filp_close(phMscd_Filp,NULL);
	set_fs(old_fs); 
	return read_bytes;
}
/* END: 0014570 jihoon.lee@lge.com 2011022 */
EXPORT_SYMBOL(lge_read_block);

const MmcPartition *lge_mmc_find_partition_by_name(const char *name)
{
    if (g_mmc_state.partitions != NULL) {
        int i;
        for (i = 0; i < g_mmc_state.partitions_allocd; i++) {
            MmcPartition *p = &g_mmc_state.partitions[i];
            if (p->device_index !=NULL && p->name != NULL) {
                if (strcmp(p->name, name) == 0) {
                    return p;
                }
            }
        }
    }
    return NULL;
}
EXPORT_SYMBOL(lge_mmc_find_partition_by_name);

void lge_mmc_print_partition_status(void)
{
    if (g_mmc_state.partitions != NULL) 
    {
        int i;
        for (i = 0; i < g_mmc_state.partitions_allocd; i++) 
        {
            MmcPartition *p = &g_mmc_state.partitions[i];
            if (p->device_index !=NULL && p->name != NULL) {
                printk(KERN_INFO"Partition Name: %s\n",p->name);
                printk(KERN_INFO"Partition Name: %s\n",p->device_index);
            }
        }
    }
    return;
}
EXPORT_SYMBOL(lge_mmc_print_partition_status);


static void lge_mmc_partition_name (MmcPartition *mbr, unsigned int type) {
	char *name;
	name = kmalloc(64, GFP_KERNEL);
    switch(type)
    {
		case MMC_MISC_TYPE:
            sprintf(name,"misc");
            mbr->name = lge_strdup(name);
			break;
		case MMC_RECOVERY_TYPE:
            sprintf(name,"recovery");
            mbr->name = lge_strdup(name);
			break;
		case MMC_XCALBACKUP_TYPE:
            sprintf(name,"xcalbackup");
            mbr->name = lge_strdup(name);
			break;
        case MMC_BOOT_TYPE:
            sprintf(name,"boot");
            mbr->name = lge_strdup(name);
            break;
        case MMC_EXT3_TYPE:
            if (strcmp("NONE", ext3_partitions[ext3_count])) {
                strcpy((char *)name,(const char *)ext3_partitions[ext3_count]);
                mbr->name = lge_strdup(name);
                ext3_count++;
            }
            mbr->filesystem = lge_strdup("ext3");
            break;
        case MMC_VFAT_TYPE:
            if (strcmp("NONE", vfat_partitions[vfat_count])) {
                strcpy((char *)name,(const char *)vfat_partitions[vfat_count]);
                mbr->name = lge_strdup(name);
                vfat_count++;
            }
            mbr->filesystem = lge_strdup("vfat");
            break;
    };
	kfree(name);
}


//static int lge_mmc_read_mbr (MmcPartition *mbr) {
/* BEGIN: 0014570 jihoon.lee@lge.com 20110122 */
/* MOD 0014570: [FACTORY RESET] change system call to filp function for handling the flag */
int lge_mmc_read_mbr (MmcPartition *mbr) {
	//int fd;
	unsigned char *buffer = NULL;
	char *device_index = NULL;
	int idx, i;
	unsigned mmc_partition_count = 0;
	unsigned int dtype;
	unsigned int dfirstsec;
	unsigned int EBR_first_sec;
	unsigned int EBR_current_sec;
	int ret = -1;

	struct file *phMscd_Filp = NULL;
	mm_segment_t old_fs;

	old_fs=get_fs();
	set_fs(get_ds());

	buffer = kmalloc(512, GFP_KERNEL);
	device_index = kmalloc(128, GFP_KERNEL);
	if((buffer == NULL) || (device_index == NULL))
	{
		printk("%s, allocation failed\n", __func__);
		goto ERROR2;
	}

	// change from sys operation to flip operation, do not use system call since this routine is also system call service.
	phMscd_Filp = filp_open(MMC_DEVICENAME, O_RDONLY, 0);
	if( !phMscd_Filp)
	{
		printk(KERN_ERR "%s, Can't open device\n", __func__ );
		goto ERROR2;
	}

	phMscd_Filp->f_pos = (loff_t)0;
	if (phMscd_Filp->f_op->read(phMscd_Filp, buffer, 512, &phMscd_Filp->f_pos) != 512)
	{
		printk(KERN_ERR "%s, Can't read device: \"%s\"\n", __func__, MMC_DEVICENAME);
		goto ERROR1;
	}

	/* Check to see if signature exists */
	if ((buffer[TABLE_SIGNATURE] != 0x55) || \
		(buffer[TABLE_SIGNATURE + 1] != 0xAA))
	{
		printk(KERN_ERR "Incorrect mbr signatures!\n");
		goto ERROR1;
	}
	idx = TABLE_ENTRY_0;
	for (i = 0; i < 4; i++)
	{
		//char device_index[128];

		mbr[mmc_partition_count].dstatus = \
		            buffer[idx + i * TABLE_ENTRY_SIZE + OFFSET_STATUS];
		mbr[mmc_partition_count].dtype   = \
		            buffer[idx + i * TABLE_ENTRY_SIZE + OFFSET_TYPE];
		mbr[mmc_partition_count].dfirstsec = \
		            GET_LWORD_FROM_BYTE(&buffer[idx + \
		                                i * TABLE_ENTRY_SIZE + \
		                                OFFSET_FIRST_SEC]);
		mbr[mmc_partition_count].dsize  = \
		            GET_LWORD_FROM_BYTE(&buffer[idx + \
		                                i * TABLE_ENTRY_SIZE + \
		                                OFFSET_SIZE]);
		dtype  = mbr[mmc_partition_count].dtype;
		dfirstsec = mbr[mmc_partition_count].dfirstsec;
		lge_mmc_partition_name(&mbr[mmc_partition_count], \
		                mbr[mmc_partition_count].dtype);

		sprintf(device_index, "%sp%d", MMC_DEVICENAME, (mmc_partition_count+1));
		mbr[mmc_partition_count].device_index = lge_strdup(device_index);

		mmc_partition_count++;
		if (mmc_partition_count == MAX_PARTITIONS)
			goto SUCCESS;
	}

	/* See if the last partition is EBR, if not, parsing is done */
	if (dtype != 0x05)
	{
		goto SUCCESS;
	}

	EBR_first_sec = dfirstsec;
	EBR_current_sec = dfirstsec;

	phMscd_Filp->f_pos = (loff_t)(EBR_first_sec * 512);
	if (phMscd_Filp->f_op->read(phMscd_Filp, buffer, 512, &phMscd_Filp->f_pos) != 512)
	{
		printk(KERN_ERR "%s, Can't read device: \"%s\"\n", __func__, MMC_DEVICENAME);
		goto ERROR1;
	}

	/* Loop to parse the EBR */
	for (i = 0;; i++)
	{

		if ((buffer[TABLE_SIGNATURE] != 0x55) || (buffer[TABLE_SIGNATURE + 1] != 0xAA))
		{
		break;
		}
		mbr[mmc_partition_count].dstatus = \
                    buffer[TABLE_ENTRY_0 + OFFSET_STATUS];
		mbr[mmc_partition_count].dtype   = \
                    buffer[TABLE_ENTRY_0 + OFFSET_TYPE];
		mbr[mmc_partition_count].dfirstsec = \
                    GET_LWORD_FROM_BYTE(&buffer[TABLE_ENTRY_0 + \
                                        OFFSET_FIRST_SEC])    + \
                                        EBR_current_sec;
		mbr[mmc_partition_count].dsize = \
                    GET_LWORD_FROM_BYTE(&buffer[TABLE_ENTRY_0 + \
                                        OFFSET_SIZE]);
		lge_mmc_partition_name(&mbr[mmc_partition_count], \
                        mbr[mmc_partition_count].dtype);

		sprintf(device_index, "%sp%d", MMC_DEVICENAME, (mmc_partition_count+1));
		mbr[mmc_partition_count].device_index = lge_strdup(device_index);

		mmc_partition_count++;
		if (mmc_partition_count == MAX_PARTITIONS)
		goto SUCCESS;

		dfirstsec = GET_LWORD_FROM_BYTE(&buffer[TABLE_ENTRY_1 + OFFSET_FIRST_SEC]);
		if(dfirstsec == 0)
		{
			/* Getting to the end of the EBR tables */
			break;
		}
		
		 /* More EBR to follow - read in the next EBR sector */
		 phMscd_Filp->f_pos = (loff_t)((EBR_first_sec + dfirstsec) * 512);
		 if (phMscd_Filp->f_op->read(phMscd_Filp, buffer, 512, &phMscd_Filp->f_pos) != 512)
		 {
			 printk(KERN_ERR "%s, Can't read device: \"%s\"\n", __func__, MMC_DEVICENAME);
			 goto ERROR1;
		 }

		EBR_current_sec = EBR_first_sec + dfirstsec;
	}

SUCCESS:
    ret = mmc_partition_count;
ERROR1:
    if(phMscd_Filp != NULL)
		filp_close(phMscd_Filp,NULL);
ERROR2:
	set_fs(old_fs);
	if(buffer != NULL)
		kfree(buffer);
	if(device_index != NULL)
		kfree(device_index);
    return ret;
}
/* END: 0014570 jihoon.lee@lge.com 2011022 */

static int lge_mmc_partition_initialied = 0;
int lge_mmc_scan_partitions(void) {
    int i;
    //ssize_t nbytes;

	if ( lge_mmc_partition_initialied )
		return g_mmc_state.partition_count;
	
    if (g_mmc_state.partitions == NULL) {
        const int nump = MAX_PARTITIONS;
        MmcPartition *partitions = kmalloc(nump * sizeof(*partitions), GFP_KERNEL);
        if (partitions == NULL) {
            return -1;
        }
        g_mmc_state.partitions = partitions;
        g_mmc_state.partitions_allocd = nump;
        memset(partitions, 0, nump * sizeof(*partitions));
    }
    g_mmc_state.partition_count = 0;
    ext3_count = 0;
    vfat_count = 0;

    /* Initialize all of the entries to make things easier later.
     * (Lets us handle sparsely-numbered partitions, which
     * may not even be possible.)
     */
    for (i = 0; i < g_mmc_state.partitions_allocd; i++) {
        MmcPartition *p = &g_mmc_state.partitions[i];
        if (p->device_index != NULL) {
            kfree(p->device_index);
            p->device_index = NULL;
        }
        if (p->name != NULL) {
            kfree(p->name);
            p->name = NULL;
        }
        if (p->filesystem != NULL) {
            kfree(p->filesystem);
            p->filesystem = NULL;
        }
    }

    g_mmc_state.partition_count = lge_mmc_read_mbr(g_mmc_state.partitions);
    if(g_mmc_state.partition_count == -1)
    {
        printk(KERN_ERR"Error in reading mbr!\n");
        // keep "partitions" around so we can free the names on a rescan.
        g_mmc_state.partition_count = -1;
    }
	if ( g_mmc_state.partition_count != -1 )
		lge_mmc_partition_initialied = 1;
    return g_mmc_state.partition_count;
}

EXPORT_SYMBOL(lge_mmc_scan_partitions);


/* BEGIN: 0013861 jihoon.lee@lge.com 20110111 */
/* MOD 0013861: [FACTORY RESET] emmc_direct_access factory reset flag access */
/* add carriage return and change flag size in each functions for the platform access */
/* END: 0013861 jihoon.lee@lge.com 20110111 */

static int test_write_block(const char *val, struct kernel_param *kp)
{

	int i = 0;
	int err;
	//int normal_block_seq = 0;
	int mtd_op_result = 0;
	const MmcPartition *pMisc_part;
	unsigned long factoryreset_bytes_pos_in_emmc = 0;
	unsigned long flag=0;

	unsigned char *test_string;

	test_string = kmalloc(FACTORY_RESET_STR_SIZE+2, GFP_KERNEL);
	// allocation exception handling
	if(!test_string)
	{
		printk(KERN_ERR "allocation failed, return\n");
		return 0;
	}

	printk(KERN_INFO"write block1\n");

	flag = simple_strtoul(val,NULL,10);
//	if (flag == 5 || flag == 6 )
//	{
/* BEGIN: 0014076 jihoon.lee@lge.com 20110114 */
/* MOD 0014076: [FACTORY RESET] Android factory reset flag bug fix */
/* make sure to store only 13 bytes, string 11, flag 1, carriage 1 */
		//sprintf(test_string,"FACT_RESET_%d\n", flag);
		sprintf(test_string,"FACT_RESET_%d\n", (char)flag);
/* END: 0014076 jihoon.lee@lge.com 20110114 */
//	} else {
//		kfree(test_string);
//		return -1;
//	}

	lge_mmc_scan_partitions();
	pMisc_part = lge_mmc_find_partition_by_name("misc");
	if ( pMisc_part == NULL )
	{
		printk(KERN_INFO"NO MISC\n");
		return 0;
	}

	factoryreset_bytes_pos_in_emmc = (pMisc_part->dfirstsec*512)+PTN_FRST_PERSIST_POSITION_IN_MISC_PARTITION;


	printk(KERN_INFO"writing block\n");


	mtd_op_result = lge_write_block(factoryreset_bytes_pos_in_emmc, test_string, FACTORY_RESET_STR_SIZE+2);
	if ( mtd_op_result != (FACTORY_RESET_STR_SIZE+2) ) {
		printk(KERN_INFO"%s: write %u block fail\n", __func__, i);
		kfree(test_string);
		return err;
	}

/* BEGIN: 0013860 jihoon.lee@lge.com 20110111 */
/* ADD 0013860: [FACTORY RESET] ERI file save */
/* request rpc for eri file when the factory reset completes */
#ifdef CONFIG_LGE_ERI_DOWNLOAD
	if (flag == 5)
	{
		printk(KERN_INFO "%s, received flag : %ld, activate work queue\n", __func__, flag);
		eri_dload_data.flag = flag;
		queue_work(eri_dload_wq, &eri_dload_data.work);
	}
#endif
/* END: 0013860 jihoon.lee@lge.com 20110111 */

	printk(KERN_INFO"write %d block\n", i);
	kfree(test_string);
	return 0;
}
module_param_call(write_block, test_write_block, param_get_bool, &dummy_arg, S_IWUSR | S_IRUGO);

// chinsu.ko@lge.com
/*
static int test_read_block( char *buf, struct kernel_param *kp)
{
	//int i;
	int err=0;
	int mtd_op_result = 0;

	const MmcPartition *pMisc_part;
	unsigned long factoryreset_bytes_pos_in_emmc = 0;

	printk(KERN_INFO"read block1\n");

	lge_mmc_scan_partitions();
//	lge_mmc_partition_initialied = 4;

	pMisc_part = lge_mmc_find_partition_by_name("misc");
	if ( pMisc_part == NULL )
	{
		printk(KERN_INFO"NO MISC\n");
		return 0;
	}
//	lge_mmc_partition_initialied = 5;
	factoryreset_bytes_pos_in_emmc = (pMisc_part->dfirstsec*512)+PTN_FRST_PERSIST_POSITION_IN_MISC_PARTITION;

	printk(KERN_INFO"read block\n");
//	lge_mmc_partition_initialied = 6;
	memset(global_buf, 0 , FACTORY_RESET_STR_SIZE+2);

	mtd_op_result = lge_read_block(factoryreset_bytes_pos_in_emmc, global_buf, FACTORY_RESET_STR_SIZE+2);
//	lge_mmc_partition_initialied = 7;

	if (mtd_op_result != (FACTORY_RESET_STR_SIZE+2) ) {
		printk(KERN_INFO" read %ld block fail\n", factoryreset_bytes_pos_in_emmc);
		return err;
	}
//	lge_mmc_partition_initialied = 8;

	printk(KERN_INFO"read %ld block\n", factoryreset_bytes_pos_in_emmc);
//	printk(KERN_INFO"%s\n", __func__, global_buf);

// BEGIN: 0009484 sehyuny.kim@lge.com 2010-09-24
// MOD 0009484: [FactoryReset] Enable FactoryReset
	if(memcmp(global_buf, FACTORY_RESET_STR, FACTORY_RESET_STR_SIZE)==0){
		err = sprintf(buf,"%s",global_buf+FACTORY_RESET_STR_SIZE);
//		err = sprintf(buf,"123456789");
		return err;
	}
	else{
//error:
		err = sprintf(buf,"1");
		return err;
	}
// END: 0009484 sehyuny.kim@lge.com 2010-09-24
}
module_param_call(read_block, param_set_bool, test_read_block, &dummy_arg, S_IWUSR | S_IRUGO);
*/

// Begin: hyechan.lee 2011-04-06
// 0018768: [fota]Fix an issue that when VZW logo is displayed, remove battery it won¡¯t enter recovery mode 
static int fota_write_block(const char *val, struct kernel_param *kp)
{

	const MmcPartition *pMisc_part; 
	unsigned long bootmsg_bytes_pos = 0;
	unsigned long fotamsg_bytes_pos = 0;

	
	unsigned int bootmsg_size, fotamsg_size;
	unsigned int fota_reboot_result;

	lge_mmc_scan_partitions();
	pMisc_part = lge_mmc_find_partition_by_name("misc");
	if ( pMisc_part == NULL )
	{
		printk(KERN_INFO"NO MISC\n");
		return -1;
	}
	
	bootmsg_bytes_pos = (pMisc_part->dfirstsec*512);

	printk(KERN_INFO"writing block\n");

	bootmsg_size= sizeof("boot-recovery");

	fota_reboot_result = lge_write_block(bootmsg_bytes_pos, "boot-recovery", bootmsg_size);

	if ( fota_reboot_result != bootmsg_size ) 
	{
		printk(KERN_INFO"%s: write block fail\n", __func__);
		return -1;
	}

	fotamsg_bytes_pos = ((pMisc_part->dfirstsec+3)*512);

	printk(KERN_INFO"%lx writing block\n", fotamsg_bytes_pos);

	fotamsg_size= sizeof("fota-recovery");

	fota_reboot_result = lge_write_block(fotamsg_bytes_pos, "fota-recovery", fotamsg_size);

	if ( fota_reboot_result != fotamsg_size ) 
	{
		printk(KERN_INFO"%s: write block fail\n", __func__);
		return -1;
	}

	printk(KERN_INFO"fota write block\n");
	return 0;
}
module_param_call(fota_recovery_reboot, fota_write_block, param_get_bool, &dummy_arg, S_IWUSR | S_IRUGO);


// end: 0018768

//#ifndef CONFIG_LGE_ERI_DOWNLOAD
extern void remote_did_rpc(void);
static void
did_dload_func(struct work_struct *work)
{
	printk(KERN_INFO "%s, flag : %ld\n", __func__, did_dload_data.flag);	
	//printk("%s [WQ] pressed: %d, keycode: %d\n", __func__, eta_gpio_matrix_data.pressed, eta_gpio_matrix_data.keycode);
#ifdef CONFIG_LGE_SUPPORT_RAPI 
	remote_did_rpc();
#endif
	return;
}
//#endif



/* BEGIN: 0013860 jihoon.lee@lge.com 20110111 */
/* ADD 0013860: [FACTORY RESET] ERI file save */


#ifdef CONFIG_LGE_ERI_DOWNLOAD
static void
eri_dload_func(struct work_struct *work)
{
	printk(KERN_INFO "%s, flag : %ld\n", __func__, eri_dload_data.flag);	
	//printk("%s [WQ] pressed: %d, keycode: %d\n", __func__, eta_gpio_matrix_data.pressed, eta_gpio_matrix_data.keycode);
#ifdef CONFIG_LGE_SUPPORT_RAPI
	remote_eri_rpc();
#endif
	return;
}
#endif
/* END: 0013860 jihoon.lee@lge.com 20110111 */

static int __init lge_emmc_direct_access_init(void)
{
	printk(KERN_INFO"%s: finished\n", __func__);

/* BEGIN: 0013860 jihoon.lee@lge.com 20110111 */
/* ADD 0013860: [FACTORY RESET] ERI file save */
#ifdef CONFIG_LGE_ERI_DOWNLOAD
	eri_dload_wq = create_singlethread_workqueue("eri_dload_wq");
	INIT_WORK(&eri_dload_data.work, eri_dload_func);
#endif
/* END: 0013860 jihoon.lee@lge.com 20110111 */

//kabjoo.choi 20110806
//#ifndef CONFIG_LGE_DID_BACKUP   
	did_dload_wq = create_singlethread_workqueue("did_dload_wq");
	INIT_WORK(&did_dload_data.work, did_dload_func);
//#endif
	return 0;
}

static void __exit lge_emmc_direct_access_exit(void)
{
	return;
}

module_init(lge_emmc_direct_access_init);
module_exit(lge_emmc_direct_access_exit);

MODULE_DESCRIPTION("LGE emmc direct access apis");
MODULE_AUTHOR("SeHyun Kim <sehyuny.kim@lge.com>");
MODULE_LICENSE("GPL");
