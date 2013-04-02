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

#include <linux/crc16.h>
#include <linux/string.h>
#include CONFIG_LGE_BOARD_HEADER_FILE
//LGE_CHANGE_S FTM boot mode
#include <mach/lge/board_v1.h>
#include <mach/proc_comm.h>
#include <lg_diag_testmode.h>
#if (defined (CONFIG_MACH_MSM7X25A_V3) && !defined (CONFIG_MACH_MSM7X25A_M4)) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
#include <mach/proc_comm.h>
#include <mach/lge/lge_proc_comm.h>
#include <lg_diag_testmode.h>
#endif
//LGE_CHANGE_E FTM boot mode

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

/*LGE_CHANGE_S 2012-10-26 khyun.kim@lge.com [V7] misc partition FS API for LGE*/
#ifdef CONFIG_MACH_MSM8X25_V7
#define LG_MISC_DATA
#endif
#ifdef LG_MISC_DATA
#define LG_MISC_DATA_PAGE_SIZE 2048
#define LG_MISC_DATA_SIZE 2040
#define LG_MISC_DATA_MAGIC 0xBEEF
typedef struct{
	unsigned int misc_magic;
	unsigned int misc_crc;
	char* misc_data;
}lg_misc_data_type;

typedef enum{
	LG_MISC_IO_RESERVED1,				/* DO NOT USE*/
	LG_MISC_IO_RESERVED2,				/* DO NOT USE*/
	LG_MISC_IO_RESERVED3,				/* DO NOT USE*/
	LG_MISC_IO_RESERVED4,				/* DO NOT USE*/
	LG_MISC_IO_RESERVED5,				/* DO NOT USE*/
	LG_MISC_IO_FRST_FLAG,				/* 5 */
	LG_MISC_IO_DEVICETEST_RESULT,
	LG_MISC_IO_AAT_RESULT,
	LG_MISC_IO_AATSET,
	LG_MISC_IO_SILENT_RESET,
	LG_MISC_IO_TEST_REGION,			/* 10 */
	LG_MISC_IO_DISPLAY_KCAL,
	LG_MISC_IO_BOOT_REASON_CARVING,
	LG_MISC_IO_ITEM_MAX,	
}misc_io_allocation;

#endif
/*LGE_CHANGE_E 2012-10-26 khyun.kim@lge.com [V7] misc partition FS API for LGE*/

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

// LGE_CHANGE_S, sohyun.nam@lge.com 
#ifdef CONFIG_LGE_FB_MSM_MDP_LUT_ENABLE
#define LCD_K_CAL_SIZE 13
static unsigned char lcd_buf[LCD_K_CAL_SIZE]={0,};
#endif /* CONFIG_LGE_FB_MSM_MDP_LUT_ENABLE */
// LGE_CHANGE_S, sohyun.nam@lge.com 

/*LGE_CHANGE_S, 2012-06-29, sohyun.nam@lge.com add silence reset to enable controling in hidden menu*/
#ifdef CONFIG_LGE_SILENCE_RESET
#define SILENT_RESET_SIZE 2
static unsigned char silent_reset_buf[SILENT_RESET_SIZE]={0,};
#endif /* CONFIG_LGE_FB_MSM_MDP_LUT_ENABLE */
// LGE_CHANGE_S, sohyun.nam@lge.com 

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


#ifdef CONFIG_LGE_DID_BACKUP   
//DID BACKUP
static struct workqueue_struct *did_dload_wq;
struct __did_data {
    unsigned long flag;
    struct work_struct work;
};
static struct __did_data did_dload_data;

static void did_dload_func(struct work_struct *work);
#endif

#if 1//!defined(CONFIG_MACH_MSM7X27A_U0)
/*LGE_CHANGE_S 2012-11-28 khyun.kim@lge.com sw_version's value set to property via rapi.*/
//LG_SW_VERSION
extern char* remote_get_sw_version(void);
void swv_get_func(struct work_struct *work);
static struct workqueue_struct *swv_dload_wq;
struct work_struct swv_work;
/*LGE_CHANGE_E 2012-11-28 khyun.kim@lge.com sw_version's value set to property via rapi.*/
#endif

#ifdef LG_MISC_DATA
int lge_emmc_misc_write_crc(unsigned int blockNo, unsigned int magickey, const char* buffer, unsigned int size, int pos);
int lge_emmc_misc_read_crc(unsigned int blockNo, char* buffer, int size);
#endif
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
module_param_call(boot_info, boot_info_write, param_get_int, &boot_info, S_IWUSR | S_IRUGO);
//[END] LGE_BOOTCOMPLETE_INFO

/* ys.seong@lge.com nfc boot compelete check [start] */
#ifdef CONFIG_LGE_NFC
int boot_info_nfc = 0;
module_param(boot_info_nfc, int, S_IWUSR | S_IRUGO);
#endif //CONFIG_LGE_NFC
/* ys.seong@lge.com nfc boot compelete check [end] */


// LGE_CHANGE_S, myunghwan.kim@lge.com
int is_factory=0;
//int android_lge_is_factory_cable(int *type);
static int get_is_factory(char *buffer, struct kernel_param *kp)
{
	//is_factory = android_lge_is_factory_cable(NULL);
	return sprintf(buffer, "%s", is_factory ? "yes" : "no");
}
module_param_call(is_factory, NULL, get_is_factory, &is_factory, 0444);
// LGE_CHANGE_E, myunghwan.kim@lge.com
#if 0
// LGE_CHANGE_S, myunghwan.kim@lge.com, miniOS controller
static int is_miniOS = -1;
unsigned lge_get_is_miniOS(void);
static int update_is_miniOS(const char *val, struct kernel_param *kp)
{
	if (is_miniOS == -1)
		is_miniOS = lge_get_is_miniOS();
	return 1;
}
static int get_is_miniOS(char *buffer, struct kernel_param *kp)
{

	return sprintf(buffer, "%s", (is_miniOS==1) ? "yes" : "no");
}
module_param_call(is_miniOS, update_is_miniOS, get_is_miniOS, &is_miniOS, 0644);
// LGE_CHANGE_E, myunghwan.kim@lge.com
#endif

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
#ifdef LG_MISC_DATA
int misc_io_blk_no = 0;
module_param(misc_io_blk_no, int, S_IWUSR | S_IRUGO);

int misc_io_size = 0;
module_param(misc_io_size, int, S_IWUSR | S_IRUGO);
#endif

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

int lge_emmc_misc_write(unsigned int blockNo, const char* buffer, int size);
int lge_emmc_misc_write_pos(unsigned int blockNo, const char* buffer, int size, int pos);
int lge_emmc_misc_read(unsigned int blockNo, char* buffer, int size);

// LGE_CHANGE_S, sohyun.nam@lge.com
#ifdef CONFIG_LGE_FB_MSM_MDP_LUT_ENABLE
static int write_lcd_k_cal(const char *val, struct kernel_param *kp)
{
	int size;

	memcpy(lcd_buf, val, LCD_K_CAL_SIZE);
	memcpy(lcd_buf+9, "cal", 4);
	lcd_buf[LCD_K_CAL_SIZE-1] = '\0';
	printk("write_lcd_k_cal :[%s] - [%s]:\n", val, lcd_buf);
#if 0//def LG_MISC_DATA
	size = lge_emmc_misc_write_crc(LG_MISC_IO_DISPLAY_KCAL,LG_MISC_DATA_MAGIC, lcd_buf, LCD_K_CAL_SIZE,0);
#else
	size = lge_emmc_misc_write(44, lcd_buf, LCD_K_CAL_SIZE);
#endif
	if (size == LCD_K_CAL_SIZE)
		printk("<6>" "write %d block\n", size);
	else
		printk("<6>" "write fail (%d)\n", size);

	return size;
}

static int read_lcd_k_cal(char *buf, struct kernel_param *kp)
{
#if 0//def LG_MISC_DATA
	int size = lge_emmc_misc_read_crc(LG_MISC_IO_DISPLAY_KCAL, buf, LCD_K_CAL_SIZE);
#else
	int size = lge_emmc_misc_read(44, buf, LCD_K_CAL_SIZE);
#endif
	buf[LCD_K_CAL_SIZE-1] = '\0';
	return size;
}
module_param_call(lcd_k_cal, write_lcd_k_cal, read_lcd_k_cal, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);  
#endif /* CONFIG_LGE_FB_MSM_MDP_LUT_ENABLE */
// LGE_CHANGE_E, sohyun.nam@lge.com

/*LGE_CHANGE_S, 2012-06-29, sohyun.nam@lge.com add silence reset to enable controling in hidden menu*/
#ifdef CONFIG_LGE_SILENCE_RESET
int lge_emmc_misc_write(unsigned int blockNo, const char* buffer, int size);
int lge_emmc_misc_read(unsigned int blockNo, char* buffer, int size);

static int write_SilentReset_flag(const char *val, struct kernel_param *kp)
{
	int size;

	memcpy(silent_reset_buf, val, SILENT_RESET_SIZE);
	silent_reset_buf[SILENT_RESET_SIZE-1] = '\0';
	printk("%s val[%s] -> silent_reset_buf[%s] \n", __func__, val, silent_reset_buf);
#if 0//def LG_MISC_DATA
	size = lge_emmc_misc_write_crc(LG_MISC_IO_SILENT_RESET,LG_MISC_DATA_MAGIC, silent_reset_buf, SILENT_RESET_SIZE);
#else
	size = lge_emmc_misc_write(36, silent_reset_buf, SILENT_RESET_SIZE);
#endif
	if (size == SILENT_RESET_SIZE)
		printk("<6>" "write %d block\n", size);
	else
		printk("<6>" "write fail (%d)\n", size);

	if(silent_reset_buf[0] == '1'){
		set_kernel_silencemode(0);
		lge_silence_reset_f(0);
	}
	else{
		set_kernel_silencemode(1);
		lge_silence_reset_f(1);
	}

	printk("%s,lge_silent_reset_flag=%d \n", __func__, get_kernel_silencemode());
	return 0;
}

static int read_SilentReset_flag(char *buf, struct kernel_param *kp)
{
	printk("%s,lge_silent_reset_flag=%d \n", __func__, get_kernel_silencemode());

	if(get_kernel_silencemode() == 0)
		sprintf(buf, "1"); // panic display mode
	else	
		sprintf(buf, "0"); // silence reset mode		
		
	return strlen(buf)+1;
}
module_param_call(silent_reset, write_SilentReset_flag, read_SilentReset_flag, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);  
#endif 
/*LGE_CHANGE_S, 2012-06-29, sohyun.nam@lge.com add silence reset to enable controling in hidden menu*/


static int write_SMPL_flag(const char *val, struct kernel_param *kp)
{
	lge_smpl_counter_f(0);
	return 0;
}

static int read_SMPL_flag(char *buf, struct kernel_param *kp)
{
	sprintf(buf, "%d", lge_smpl_counter_f(-1));
	return strlen(buf)+1;
}
module_param_call(smpl_counter, write_SMPL_flag, read_SMPL_flag, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);  


//LGE_CHANGE_S FTM boot mode
#if (defined (CONFIG_MACH_MSM7X25A_V3) && !defined (CONFIG_MACH_MSM7X25A_M4)) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
extern unsigned lge_nv_manual_f(int val);
extern void send_to_arm9( void * pReq, void * pRsp);
test_mode_req_type manual;
void ManualTestOn_Local(int manual_mode)
{
           DIAG_TEST_MODE_F_req_type req_ptr;
           DIAG_TEST_MODE_F_rsp_type resp;

           manual.test_manual_mode = (test_mode_req_manual_test_mode_type)manual_mode;
           req_ptr.sub_cmd_code = TEST_MODE_MANUAL_TEST_MODE;
           req_ptr.test_mode_req = manual;

           send_to_arm9((void*) &req_ptr, (void*) &resp);
           if (resp.ret_stat_code != TEST_OK_S) {
                     printk(" *** LGF_TestModeManualTestMode return error \n");
                     return;
           }
}
EXPORT_SYMBOL(ManualTestOn_Local);

static int write_ftm_boot_mode(const char *val, struct kernel_param *kp)
{
	if(*val == '1')
	{
		printk(KERN_INFO"[FTM] writing write_ftm_boot_mode [1] \n");
		ManualTestOn_Local(MANUAL_TEST_ON);
		return 0;
	}
	else
	{
		printk(KERN_INFO"[FTM] writing write_ftm_boot_mode [0] \n");
		ManualTestOn_Local(MANUAL_TEST_OFF);
		return 0;
	}
	return 0;
}

static int read_ftm_boot_mode(char *buf, struct kernel_param *kp)
{
	sprintf(buf, "%d", lge_nv_manual_f(-1));
	return strlen(buf)+1;
}
module_param_call(ftm_boot_mode, write_ftm_boot_mode, read_ftm_boot_mode, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
#endif
//LGE_CHANGE_E FTM boot mode

static int write_ChargingBypassBoot_flag(const char *val, struct kernel_param *kp)
{
	if (*val == '1')
	{
		lge_charging_bypass_boot_f(1);
	}
	else
	{
		lge_charging_bypass_boot_f(0);
	}
	return 0;
}

static int read_ChargingBypassBoot_flag(char *buf, struct kernel_param *kp)
{
	sprintf(buf, "%d", lge_charging_bypass_boot_f(-1));
	return strlen(buf)+1;
}
module_param_call(charging_bypass_boot, write_ChargingBypassBoot_flag, read_ChargingBypassBoot_flag, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);  

static int write_PseudoBatteryMode_flag(const char *val, struct kernel_param *kp)
{
	if(*val == '1')
	{
		lge_pseudo_battery_mode_f(1);
	}
	else
	{
		lge_pseudo_battery_mode_f(0);
	}
	return 0;
}

static int read_PseudoBatteryMode_flag(char *buf, struct kernel_param *kp)
{
	sprintf(buf, "%d", lge_pseudo_battery_mode_f(-1));
	return strlen(buf)+1;
}
module_param_call(pseudo_battery_mode, write_PseudoBatteryMode_flag, read_PseudoBatteryMode_flag, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);  

int lge_get_frst_flag(void);
int lge_set_frst_flag(int flag);
int lge_get_esd_flag(void);
int lge_set_esd_flag(int flag);

int frst_write_block(const char *buf, struct kernel_param *kp)
{
	int expected = -1;
	int changeto = -1;

	// skip null
	if (buf == NULL)
		return -1;

	// skip invalid format
	if (buf[0] < '0' || buf[0] > '7')
		return -1;

	changeto = buf[0]-'0';

	// check expected value format
	if (buf[1] == '-') {
		if (buf[2] < '0' || buf[2] > '7')
			return -1;

		expected = changeto;
		changeto = buf[2]-'0';

		// compare expected flag and current flag
		if (lge_get_frst_flag() != expected)
			return -1;
	}

	// set frst flag
	lge_set_frst_flag(changeto);
	return 1;
}

int frst_read_block(char *buf, struct kernel_param *kp)
{
	int flag;

	if (buf == NULL)
		return -1;

	flag = lge_get_frst_flag();
	if (flag == -1)
		return -1;

	buf[0] = '0' + flag;
	buf[1] = '\0';
	return 2;
}
module_param_call(frst_flag, frst_write_block, frst_read_block, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);


#if !defined(CONFIG_MACH_MSM7X27A_U0)//#[jinseok.choi@lge.com]2012-11-22 U0 emmc access via kernel [START]
//[LGE_UPDATE_S] 20120215 minwoo.jung
int HiddenMenu_FactoryReset(void);

int do_hiddenmenu_frst(char *buf, struct kernel_param *kp)
{
	buf[0] = '1'+HiddenMenu_FactoryReset();	
	buf[1] = '\0';
	return 2;
}
module_param_call(hiddenmenu_factory_reset, NULL, do_hiddenmenu_frst, NULL, S_IRUSR | S_IRGRP );
//[LGE_UPDATE_E] 20120215 minwoo.jung
#endif
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

#ifdef CONFIG_LGE_DID_BACKUP   
extern void remote_did_rpc(void);
static void
did_dload_func(struct work_struct *work)
{
	printk(KERN_INFO "%s, flag : %ld\n", __func__, did_dload_data.flag);	
	//printk("%s [WQ] pressed: %d, keycode: %d\n", __func__, eta_gpio_matrix_data.pressed, eta_gpio_matrix_data.keycode);
#ifdef CONFIG_LGE_SUPPORT_RAPI 
	//remote_did_rpc();
#endif
	return;
}
#endif

/* BEGIN: 0013860 jihoon.lee@lge.com 20110111 */
/* ADD 0013860: [FACTORY RESET] ERI file save */
#ifdef CONFIG_LGE_ERI_DOWNLOAD
static void
eri_dload_func(struct work_struct *work)
{
	printk(KERN_INFO "%s, flag : %ld\n", __func__, eri_dload_data.flag);	
	//printk("%s [WQ] pressed: %d, keycode: %d\n", __func__, eta_gpio_matrix_data.pressed, eta_gpio_matrix_data.keycode);
#ifdef CONFIG_LGE_SUPPORT_RAPI
	//remote_eri_rpc();
#endif
	return;
}
#endif
/* END: 0013860 jihoon.lee@lge.com 20110111 */

int lge_emmc_misc_write_pos(unsigned int blockNo, const char* buffer, int size, int pos)
{
	struct file *fp_misc = NULL;
	mm_segment_t old_fs;
	int write_bytes = -1;

	// exception handling
	if ((buffer == NULL) || size <= 0) {
		printk(KERN_ERR "%s, NULL buffer or NULL size : %d\n", __func__, size);
		return -1;
	}

	old_fs=get_fs();
	set_fs(get_ds());

	// try to open
	fp_misc = filp_open("/dev/block/platform/msm_sdcc.3/by-num/p8", O_WRONLY | O_SYNC, 0);
	if (IS_ERR(fp_misc)) {
		printk(KERN_ERR "%s, Can not access MISC\n", __func__);
		goto misc_write_fail;
	}

	fp_misc->f_pos = (loff_t) (512 * blockNo + pos);
	write_bytes = fp_misc->f_op->write(fp_misc, buffer, size, &fp_misc->f_pos);

	if (write_bytes <= 0) {
		printk(KERN_ERR "%s, Can not write (MISC) \n", __func__);
		goto misc_write_fail;
	}

misc_write_fail:
	if (!IS_ERR(fp_misc))
		filp_close(fp_misc, NULL);

	set_fs(old_fs);	
	return write_bytes;
}
EXPORT_SYMBOL(lge_emmc_misc_write_pos);

int lge_emmc_misc_write(unsigned int blockNo, const char* buffer, int size)
{
	return lge_emmc_misc_write_pos(blockNo, buffer, size, 0);
}
EXPORT_SYMBOL(lge_emmc_misc_write);

int lge_emmc_misc_read(unsigned int blockNo, char* buffer, int size)
{
	struct file *fp_misc = NULL;
	mm_segment_t old_fs;
	int read_bytes = -1;

	// exception handling
	if ((buffer == NULL) || size <= 0) {
		printk(KERN_ERR "%s, NULL buffer or NULL size : %d\n", __func__, size);
		return 0;
	}

	old_fs=get_fs();
	set_fs(get_ds());

	// try to open
	fp_misc = filp_open("/dev/block/platform/msm_sdcc.3/by-num/p8", O_RDONLY | O_SYNC, 0);
	if(IS_ERR(fp_misc)) {
		printk(KERN_ERR "%s, Can not access MISC\n", __func__);
		goto misc_read_fail;
	}

	fp_misc->f_pos = (loff_t) (512 * blockNo);
	read_bytes = fp_misc->f_op->read(fp_misc, buffer, size, &fp_misc->f_pos);

	if (read_bytes <= 0) {
		printk(KERN_ERR "%s, Can not read (MISC) \n", __func__);
		goto misc_read_fail;
	}

misc_read_fail:
	if (!IS_ERR(fp_misc))
		filp_close(fp_misc, NULL);

	set_fs(old_fs);	
	return read_bytes;
}
EXPORT_SYMBOL(lge_emmc_misc_read);

int lge_get_frst_flag(void)
{
	unsigned char buffer[16]={0,};
#if 0 //def LG_MISC_DATA
	if (lge_emmc_misc_read_crc(LG_MISC_IO_FRST_FLAG, buffer, sizeof(buffer)) <= 0) {
		pr_info(" *** (MISC) FRST Flag read fail\n");
		return -1;
	}
#else
	if (lge_emmc_misc_read(8, buffer, sizeof(buffer)) <= 0) {
		pr_info(" *** (MISC) FRST Flag read fail\n");
		return -1;
	}
#endif
	if (buffer[0] == 0xff || buffer[0] == 0)
		buffer[0] = '0';

	pr_info(" *** (MISC) FRST Flag read: %d\n", buffer[0]-'0');

	if (buffer[0] < '0' || buffer[0] > '6')
		return -1;

	return buffer[0] - '0';
}
EXPORT_SYMBOL(lge_get_frst_flag);

int lge_set_frst_flag(int flag)
{
	char buffer[4]={0,};
	if (flag < 0 || flag > 6)
		return -1;

	buffer[0] = '0' + flag;
#if 0 //def LG_MISC_DATA
	if (lge_emmc_misc_write_crc(LG_MISC_IO_FRST_FLAG, LG_MISC_DATA_MAGIC, buffer, sizeof(buffer),0) <= 0)
		return -1;
#else
	if (lge_emmc_misc_write(8, buffer, sizeof(buffer)) <= 0)
		return -1;
#endif
	pr_info(" *** (MISC) FRST Flag write: %c\n", buffer[0]);
	return 0;
}
EXPORT_SYMBOL(lge_set_frst_flag);

int lge_get_esd_flag(void)
{
	unsigned char buffer[8]={0,};

	if (lge_emmc_misc_read(36, buffer, sizeof(buffer)) <= 0) {
		pr_info(" *** (MISC) lge_get_esd_flag read fail\n");
		return -1;
	}

	pr_info(" *** (MISC) lge_get_esd_flag: %d\n", buffer[0]-'0');
	return buffer[5] - '0';
}
EXPORT_SYMBOL(lge_get_esd_flag);

int lge_set_esd_flag(int flag)
{
	char buffer[8]={0,};	

	buffer[0] = '0' + flag;
	if (lge_emmc_misc_write_pos(36, buffer, sizeof(buffer), 5) <= 0)
		return -1;

	pr_info(" *** (MISC) lge_set_esd_flag write: %c\n", buffer[0]);
	return 0;
}
EXPORT_SYMBOL(lge_set_esd_flag);
// LGE_START 20121101 seonbeom.lee [Security] porting security code.

int lge_emmc_wallpaper_write_pos(unsigned int blockNo, const char* buffer, int size, int pos)
{
	struct file *fp_wallpaper = NULL;
	mm_segment_t old_fs;
	int write_bytes = -1;

	// exception handling
	if ((buffer == NULL) || size <= 0) {
		printk(KERN_ERR "%s, NULL buffer or NULL size : %d\n", __func__, size);
		return -1;
	}

	old_fs=get_fs();
	set_fs(get_ds());

	// try to open
	fp_wallpaper = filp_open("/dev/block/mmcblk0p6", O_RDWR | O_SYNC, 0);
	if (IS_ERR(fp_wallpaper)) {
		printk(KERN_ERR "%s, Can not access WALLPAPER\n", __func__);
		goto wallpaper_write_fail;
	}

	fp_wallpaper->f_pos = (loff_t) (512 * blockNo + pos);
	write_bytes = fp_wallpaper->f_op->write(fp_wallpaper, buffer, size, &fp_wallpaper->f_pos);

	if (write_bytes <= 0) {
		printk(KERN_ERR "[sunny], write_bytes = [%d]\n", write_bytes );
		printk(KERN_ERR "%s, Can not write (WALLPAPER) \n", __func__);
		goto wallpaper_write_fail;
	}

wallpaper_write_fail:
	if (!IS_ERR(fp_wallpaper))
		filp_close(fp_wallpaper, NULL);

	set_fs(old_fs);	
	return write_bytes;
}
EXPORT_SYMBOL(lge_emmc_wallpaper_write_pos);


int lge_emmc_wallpaper_write(unsigned int blockNo, const char* buffer, int size)
{
	return lge_emmc_wallpaper_write_pos(blockNo, buffer, size, 0);
}
EXPORT_SYMBOL(lge_emmc_wallpaper_write);

// LGE_END 20121101 seonbeom.lee [Security] porting security code.

/*LGE_CHANGE_S 2012-10-26 khyun.kim@lge.com [V7] misc partition FS API for LGE*/
#ifdef LG_MISC_DATA
extern u16 crc16(u16 crc, u8 const *buffer, size_t len);

int lge_emmc_misc_write_crc(unsigned int blockNo, unsigned int magickey, const char* buffer, unsigned int size, int pos)
{
	struct file *fp_misc = NULL;
	mm_segment_t old_fs;
	int write_bytes = -1;
	lg_misc_data_type misc_type;

	//debug
	printk(KERN_ERR "%s, blockno = %d, magickey = %d,size = %d\n", __func__, blockNo, magickey, size);
	// exception handling
	if ((buffer == NULL) || size <= 0) {
		printk(KERN_ERR "%s, NULL buffer or NULL size : %d\n", __func__, size);
		return -1;
	}

	//magic key
	misc_type.misc_magic = magickey;
	if (misc_type.misc_magic != LG_MISC_DATA_MAGIC)
	{
		printk(KERN_ERR "%s, MAGIC key is not matched. key = %x\n", __func__,misc_type.misc_magic);
		return -1;
	}
	
	//data
	misc_type.misc_data = kmalloc(LG_MISC_DATA_SIZE,GFP_KERNEL);
	memset(misc_type.misc_data,0x00,LG_MISC_DATA_SIZE);
	memcpy(misc_type.misc_data,buffer,size);

	//CRC
	misc_type.misc_crc = crc16(0,misc_type.misc_data,LG_MISC_DATA_SIZE);
	printk(KERN_ERR "%s, CRC = %x\n", __func__, misc_type.misc_crc);
	
	old_fs=get_fs();
	set_fs(get_ds());

	// try to open
	fp_misc = filp_open("/dev/block/platform/msm_sdcc.3/by-num/p8", O_WRONLY | O_SYNC, 0);
	if (IS_ERR(fp_misc)) {
		printk(KERN_ERR "%s, Can not access MISC\n", __func__);
		goto misc_io_write_fail;
	}

	fp_misc->f_pos = (loff_t) (LG_MISC_DATA_PAGE_SIZE * blockNo + pos);
	write_bytes = fp_misc->f_op->write(fp_misc, (char*)&misc_type, 8, &fp_misc->f_pos);
	write_bytes += fp_misc->f_op->write(fp_misc, misc_type.misc_data, LG_MISC_DATA_SIZE, &fp_misc->f_pos);
	
	if (write_bytes <= 0) {
		printk(KERN_ERR "%s, Can not write (MISC) \n", __func__);
		goto misc_io_write_fail;
	}
	
	if (write_bytes == LG_MISC_DATA_PAGE_SIZE)
	{
		write_bytes = size;
	}
	else
	{
		printk(KERN_ERR "PAGE write fail!, written bytes are %d bytes.\n",write_bytes );
		kfree(misc_type.misc_data);
		return write_bytes;
	}

misc_io_write_fail:
	if (!IS_ERR(fp_misc))
		filp_close(fp_misc, NULL);

	kfree(misc_type.misc_data);
	
	set_fs(old_fs);	
	return write_bytes;
}
EXPORT_SYMBOL(lge_emmc_misc_write_crc);

int lge_emmc_misc_read_crc(unsigned int blockNo, char* buffer, int size)
{
	struct file *fp_misc = NULL;
	mm_segment_t old_fs;
	int read_bytes = -1;
	unsigned int m_magic = 0;
	unsigned int m_crc = 0;
	unsigned int c_crc = 0;
	char* read_for_c;
	
	//debug
	printk(KERN_ERR "%s, blockno = %d, size = %d\n", __func__, blockNo, size);
	// exception handling
	if ((buffer == NULL) || size <= 0) {
		printk(KERN_ERR "%s, NULL buffer or NULL size : %d\n", __func__, size);
		return 0;
	}

	old_fs=get_fs();
	set_fs(get_ds());

	read_for_c = kzalloc(LG_MISC_DATA_PAGE_SIZE,GFP_KERNEL);
	// try to open
	fp_misc = filp_open("/dev/block/platform/msm_sdcc.3/by-num/p8", O_RDONLY | O_SYNC, 0);
	if(IS_ERR(fp_misc)) {
		printk(KERN_ERR "%s, Can not access MISC\n", __func__);
		goto misc_io_read_fail;
	}

	fp_misc->f_pos = (loff_t) (LG_MISC_DATA_PAGE_SIZE * blockNo);
	read_bytes = fp_misc->f_op->read(fp_misc, read_for_c, LG_MISC_DATA_PAGE_SIZE, &fp_misc->f_pos);

	if (read_bytes <= 0) {
		printk(KERN_ERR "%s, Can not read (MISC) \n", __func__);
		goto misc_io_read_fail;
	}

	if (read_bytes == LG_MISC_DATA_PAGE_SIZE)
		{
			read_bytes = size;
		}
	else
		{
			printk(KERN_ERR "PAGE read fail!, readed bytes are %d bytes.\n",read_bytes );
			kfree(read_for_c);
			return read_bytes;
		}
		
	
	//magic key check
	memcpy(&m_magic,read_for_c,sizeof(unsigned int));
	if ((m_magic != LG_MISC_DATA_MAGIC) && (m_magic != 0))
	{
		printk(KERN_ERR "%s, MAGIC key is not matched. key = %x\n", __func__,m_magic);
		kfree(read_for_c);
		return -1;
	}

	//crc check
	memcpy(&m_crc,read_for_c+(sizeof(unsigned int)),sizeof(unsigned int));
	c_crc = crc16(0,read_for_c+(2*sizeof(unsigned int)),LG_MISC_DATA_SIZE);
	if (m_crc != c_crc)
	{
		printk(KERN_ERR "%s, CRC is not matched. saved CRC = %x, caculated CRC = %x\n", __func__,m_crc,c_crc);
		kfree(read_for_c);
		return -1;
	}

	memcpy(buffer,read_for_c+8,size);

misc_io_read_fail:
	if (!IS_ERR(fp_misc))
		filp_close(fp_misc, NULL);
	
	kfree(read_for_c);
	set_fs(old_fs);	
	return read_bytes;
}
EXPORT_SYMBOL(lge_emmc_misc_read_crc);

/*
If you want to write data to misc partition(by using the below funtion), must chagne values of following specific sysfs files.
misc_io_blk_no, misc_io_size.
These function for misc partition accessing by user space.
*/
int lge_emmc_misc_io_write(const char *val, struct kernel_param *kp)
{
	unsigned int m_blkno = misc_io_blk_no;
	unsigned int m_size = misc_io_size;
	unsigned int m_magic = LG_MISC_DATA_MAGIC;
	printk(KERN_ERR "%s, blockno = %d, size = %d\n", __func__, misc_io_blk_no,misc_io_size);
	return lge_emmc_misc_write_crc(m_blkno,m_magic,val,m_size,0);
}

int lge_emmc_misc_io_read(char *buf, struct kernel_param *kp)
{
	int size = 0;
	printk(KERN_ERR "%s, blockno = %d, size = %d\n", __func__, misc_io_blk_no, misc_io_size);
	size = lge_emmc_misc_read_crc(misc_io_blk_no,buf,misc_io_size);
	return size;
}

module_param_call(misc_io, lge_emmc_misc_io_write, lge_emmc_misc_io_read, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

#endif
/*LGE_CHANGE_E 2012-10-26 khyun.kim@lge.com [V7] misc partition FS API for LGE*/
/* LGE_CHANGE_S  : adiyoung.lee, FTM Mode and ManualModeCkeckComplete on RPC, 2012-12-12 */
#if !defined(CONFIG_MACH_MSM7X25A_M4) && (defined (CONFIG_MACH_MSM7X25A_V3) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1))
extern void send_to_arm9( void * pReq, void * pRsp);
test_mode_req_type manual;
void AAT_Local(int manual_mode)
{
       DIAG_TEST_MODE_F_req_type req_ptr;
       DIAG_TEST_MODE_F_rsp_type resp;

       manual.test_manual_mode = (test_mode_req_manual_test_mode_type)manual_mode;
       req_ptr.sub_cmd_code = TEST_MODE_MANUAL_TEST_MODE;
       req_ptr.test_mode_req = manual;

       send_to_arm9((void*) &req_ptr, (void*) &resp);
       if (resp.ret_stat_code != TEST_OK_S) {
                 printk(" *** AAT_Local return error \n");
                 return;
       }
}
EXPORT_SYMBOL(AAT_Local);

static int write_aat_partial(const char *val, struct kernel_param *kp)
{

    if (*val == '1')
    {
        lge_aat_partial_f(1);
        AAT_Local(MANUAL_EFS_SYNC);
    }
    else
    {
        lge_aat_partial_f(0);
        AAT_Local(MANUAL_EFS_SYNC);
    }

	return 0;
}

static int read_aat_partial(char *buf, struct kernel_param *kp)
{
	sprintf(buf, "%d", lge_aat_partial_f(-1));
	return strlen(buf)+1;
}
module_param_call(aat_partial, write_aat_partial, read_aat_partial, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);


static int write_aat_full(const char *val, struct kernel_param *kp)
{
	if (*val == '1')
	{
		lge_aat_full_f(1);
        AAT_Local(MANUAL_EFS_SYNC);
	}
	else
	{
		lge_aat_full_f(0);
        AAT_Local(MANUAL_EFS_SYNC);
	}

    return 0;
}

static int read_aat_full(char *buf, struct kernel_param *kp)
{
	sprintf(buf, "%d", lge_aat_full_f(-1));
	return strlen(buf)+1;
}
module_param_call(aat_full, write_aat_full, read_aat_full, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);


static int write_aat_partial_or_full(const char *val, struct kernel_param *kp)
{
	if (*val == '1')
	{
		lge_aat_partial_or_full_f(1);
        AAT_Local(MANUAL_EFS_SYNC);
	}
	else
	{
		lge_aat_partial_or_full_f(0);
        AAT_Local(MANUAL_EFS_SYNC);
	}
    return 0;
}

static int read_aat_partial_or_full(char *buf, struct kernel_param *kp)
{
	sprintf(buf, "%d", lge_aat_partial_or_full_f(-1));
	return strlen(buf)+1;
}
module_param_call(aat_partial_or_full, write_aat_partial_or_full, read_aat_partial_or_full, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
#endif
/* LGE_CHANGE_E  : adiyoung.lee, FTM Mode and ManualModeCkeckComplete on RPC, 2012-12-12 */

#if 1//!defined(CONFIG_MACH_MSM7X27A_U0)
/*LGE_CHANGE_S 2012-11-28 khyun.kim@lge.com sw_version's value set to property via rapi.*/
extern unsigned char swv_buff[100];
void swv_get_func(struct work_struct *work)
{
	char *envp[] = {
		"HOME=/",
		"TERM=linux",
		NULL,
	};

	unsigned* swv_buffer;
	char fac_version[50];
	char sw_version[50];
	unsigned ret;
	swv_buffer = kzalloc(100, GFP_KERNEL);

	remote_get_sw_version();

	memcpy(&sw_version[0],swv_buff,50);
	//printk(KERN_INFO "sw_version VALUE = %s\n",sw_version);
	memcpy(&fac_version[0],(&swv_buff[0])+50,50);
	//printk(KERN_INFO "fac_version VALUE = %s\n",fac_version);
	sw_version[49] = '\0';
	fac_version[49] = '\0';
	{
		char *argv[] = {
		"setprop",
		"lge.version.factorysw",
		fac_version,
		NULL,
		};
		if ((ret = call_usermodehelper("/system/bin/setprop", argv, envp, UMH_WAIT_PROC)) != 0) {
			printk(KERN_ERR "%s lge.version.factorysw set failed to run \": %i\n",__func__, ret);
		}
		else{
			printk(KERN_INFO "%s lge.version.factorysw set execute ok\n", __func__);
		}
	}
	{
		char *argv[] = {
		"setprop",
		"ro.lge.factoryversion",
		fac_version,
		NULL,
		};
		if ((ret = call_usermodehelper("/system/bin/setprop", argv, envp, UMH_WAIT_PROC)) != 0) {
			printk(KERN_ERR "%s ro.lge.factoryversion set failed to run \": %i\n",__func__, ret);
		}
		else{
			printk(KERN_INFO "%s ro.lge.factoryversion set execute ok\n", __func__);
		}
	}	{
		char *argv[] = {
		"setprop",
		"lge.version.sw",
		sw_version,
		NULL,
		};
		if ((ret = call_usermodehelper("/system/bin/setprop", argv, envp, UMH_WAIT_PROC)) != 0) {
			printk(KERN_ERR "%s lge.version.sw set failed to run \": %i\n",__func__, ret);
		}
		else{
			printk(KERN_INFO "%s  lge.version.sw set execute ok\n", __func__);
		}
	}
	kfree(swv_buffer);
}

int lge_sw_version_read(const char *val, struct kernel_param *kp)
{
	printk(KERN_INFO"%s: started\n", __func__);
	
	queue_work(swv_dload_wq, &swv_work);

	return 0;
}
module_param_call(lge_swv,lge_sw_version_read, NULL, NULL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
/*LGE_CHANGE_E 2012-11-28 khyun.kim@lge.com sw_version's value set to property via rapi.*/
#endif

static int __init lge_emmc_direct_access_init(void)
{
	printk(KERN_INFO"%s: started\n", __func__);

/* BEGIN: 0013860 jihoon.lee@lge.com 20110111 */
/* ADD 0013860: [FACTORY RESET] ERI file save */
#ifdef CONFIG_LGE_ERI_DOWNLOAD
	eri_dload_wq = create_singlethread_workqueue("eri_dload_wq");
	INIT_WORK(&eri_dload_data.work, eri_dload_func);
#endif
/* END: 0013860 jihoon.lee@lge.com 20110111 */

//kabjoo.choi 20110806
#ifdef CONFIG_LGE_DID_BACKUP   
	did_dload_wq = create_singlethread_workqueue("did_dload_wq");
	INIT_WORK(&did_dload_data.work, did_dload_func);
#endif

#if 1//!defined(CONFIG_MACH_MSM7X27A_U0)
/*LGE_CHANGE_S 2012-11-28 khyun.kim@lge.com sw_version's value set to property via rapi.*/
	swv_dload_wq = create_singlethread_workqueue("swv_dload_wq");
	if (swv_dload_wq == NULL)
	{
		printk(KERN_INFO"%s: swv wq error\n", __func__);
	}
	INIT_WORK(&swv_work,swv_get_func);
	printk(KERN_INFO"%s: finished\n", __func__);
/*LGE_CHANGE_E 2012-11-28 khyun.kim@lge.com sw_version's value set to property via rapi.*/
#endif
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
