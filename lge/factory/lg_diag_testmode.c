#include <linux/module.h>
#include <lg_diagcmd.h>
#include <linux/input.h>
#include <linux/syscalls.h>

#include <lg_fw_diag_communication.h>
#include <lg_diag_testmode.h>
#include <mach/qdsp5v2/audio_def.h>
#include <linux/delay.h>

#ifndef SKW_TEST
#include <linux/fcntl.h> 
#include <linux/fs.h>
#include <linux/uaccess.h>
#endif

#ifdef CONFIG_LGE_DLOAD_SRD
#include <userDataBackUpDiag.h>
#include <userDataBackUpTypeDef.h> 
#include <../../kernel/arch/arm/mach-msm/smd_private.h>
#include <linux/slab.h>
#endif 

// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
#include <linux/parser.h>
// LGE_CHANGE_E, real-wifi@lge.com, 20110928, [WLAN TEST MODE]

#include <mach/board_lge.h>
#include <lg_backup_items.h>

#include <linux/gpio.h>
#include <linux/mfd/pmic8058.h>
#include <mach/irqs.h>

#if 0 // M3 use Internal SD, not External SD
// m3 use Internal SD, so we dont use this
#else
#define SYS_GPIO_SD_DET 40
#endif
#define PM8058_GPIO_BASE NR_MSM_GPIOS
#define PM8058_GPIO_PM_TO_SYS(pm_gpio) (pm_gpio + PM8058_GPIO_BASE)
// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
#define WL_IS_WITHIN(min,max,expr)         (((min)<=(expr))&&((max)>(expr)))
// LGE_CHANGE_E, real-wifi@lge.com, 20110928, [WLAN TEST MODE]

static struct diagcmd_dev *diagpdev;

extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);
extern PACK(void *) diagpkt_free (PACK(void *)pkt);
extern void send_to_arm9( void * pReq, void * pRsp);
extern testmode_user_table_entry_type testmode_mstr_tbl[TESTMODE_MSTR_TBL_SIZE];
extern int diag_event_log_start(void);
extern int diag_event_log_end(void);
extern void set_operation_mode(boolean isOnline);
extern struct input_dev* get_ats_input_dev(void);

#ifdef CONFIG_LGE_DIAG_KEYPRESS
extern unsigned int LGF_KeycodeTrans(word input);
extern void LGF_SendKey(word keycode);
#endif

extern int boot_info;

extern void remote_rpc_srd_cmmand(void * pReq, void * pRsp );
extern void *smem_alloc(unsigned id, unsigned size);


extern PACK (void *)LGE_Dload_SRD (PACK (void *)req_pkt_ptr, uint16 pkg_len);
extern void diag_SRD_Init(udbp_req_type * req_pkt, udbp_rsp_type * rsp_pkt);
extern void diag_userDataBackUp_entrySet(udbp_req_type * req_pkt, udbp_rsp_type * rsp_pkt, script_process_type MODEM_MDM );
extern boolean writeBackUpNVdata( char * ram_start_address , unsigned int size);
extern void diag_userDataBackUp_data(udbp_req_type *req_pkt, udbp_rsp_type *rsp_pkt);	//CSFB SRD

#ifdef CONFIG_LGE_DLOAD_SRD  //kabjoo.choi
#define SIZE_OF_SHARD_RAM  0x60000  //384K

extern int lge_erase_block(int secnum, size_t size);
extern int lge_write_block(int secnum, unsigned char *buf, size_t size);
extern int lge_read_block(int secnum, unsigned char *buf, size_t size);
extern int lge_mmc_scan_partitions(void);

extern unsigned int srd_bytes_pos_in_emmc ;
unsigned char * load_srd_base;	//CSFB SRD
//CSFB SRD	unsigned char * load_srd_shard_base;
//CSFB SRD	unsigned char * load_srd_kernel_base;
#endif 

/* ==========================================================================
===========================================================================*/

struct statfs_local {
 __u32 f_type;
 __u32 f_bsize;
 __u32 f_blocks;
 __u32 f_bfree;
 __u32 f_bavail;
 __u32 f_files;
 __u32 f_ffree;
 __kernel_fsid_t f_fsid;
 __u32 f_namelen;
 __u32 f_frsize;
 __u32 f_spare[5];
};

/* ==========================================================================
===========================================================================*/

extern int lge_bd_rev;

// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
typedef struct _rx_packet_info 
{
	int goodpacket;
	int badpacket;
} rx_packet_info_t;

enum {
	Param_none = -1,
	Param_goodpacket,
	Param_badpacket,
	Param_end,
	Param_err,
};

static const match_table_t param_tokens = {
	{Param_goodpacket, "good=%d"},
	{Param_badpacket, "bad=%d"},
	{Param_end,	"END"},
	{Param_err, NULL}
};
// LGE_CHANGE_E, real-wifi@lge.com, 20110928, [WLAN TEST MODE]


void CheckHWRev(byte *pStr)
{
    char *rev_str[] = {"evb1", "evb2", "A", "B", "C", "D",
        "E", "F", "G", "1.0", "1.1", "1.2",
        "revserved"};

    strcpy((char *)pStr ,(char *)rev_str[lge_bd_rev]);
}

PACK (void *)LGF_TestMode (
        PACK (void	*)req_pkt_ptr, /* pointer to request packet */
        uint16 pkt_len )        /* length of request packet */
{
    DIAG_TEST_MODE_F_req_type *req_ptr = (DIAG_TEST_MODE_F_req_type *) req_pkt_ptr;
    DIAG_TEST_MODE_F_rsp_type *rsp_ptr;
    unsigned int rsp_len=0;
    testmode_func_type func_ptr= NULL;
    int nIndex = 0;

    diagpdev = diagcmd_get_dev();

    // DIAG_TEST_MODE_F_rsp_type union type is greater than the actual size, decrease it in case sensitive items
    switch(req_ptr->sub_cmd_code)
    {
        case TEST_MODE_FACTORY_RESET_CHECK_TEST:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type);
            break;

        case TEST_MODE_TEST_SCRIPT_MODE:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_test_script_mode_type);
            break;

        //REMOVE UNNECESSARY RESPONSE PACKET FOR EXTERNEL SOCKET ERASE
        case TEST_MODE_EXT_SOCKET_TEST:
            if((req_ptr->test_mode_req.esm == EXTERNAL_SOCKET_ERASE) || (req_ptr->test_mode_req.esm == EXTERNAL_SOCKET_ERASE_SDCARD_ONLY) \
                    || (req_ptr->test_mode_req.esm == EXTERNAL_SOCKET_ERASE_FAT_ONLY))
                rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type);
            else
                rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type);
            break;

        //Added by jaeopark 110527 for XO Cal Backup
        case TEST_MODE_XO_CAL_DATA_COPY:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_XOCalDataBackup_Type);
            break;

        case TEST_MODE_MANUAL_TEST_MODE:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_manual_test_mode_type);
            break;

        case TEST_MODE_BLUETOOTH_RW:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_bt_addr_type);
            break;

        case TEST_MODE_WIFI_MAC_RW:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_wifi_addr_type);
            break;

/* 2011.07.22 woochang.chun@lge.com, to ignore key, touch event on sleep mode (250-42-0) */
#ifdef CONFIG_LGE_DIAG_DISABLE_INPUT_DEVICES_ON_SLEEP_MODE
        case TEST_MODE_SLEEP_MODE_TEST:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_sleep_mode_type);
            break;
#endif

        default :
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type);
            break;
    }

    rsp_ptr = (DIAG_TEST_MODE_F_rsp_type *)diagpkt_alloc(DIAG_TEST_MODE_F, rsp_len);

    printk(KERN_ERR "[LGF_TestMode] rsp_len: %d, sub_cmd_code: %d \n", rsp_len, req_ptr->sub_cmd_code);

    if (!rsp_ptr)
        return 0;

    rsp_ptr->sub_cmd_code = req_ptr->sub_cmd_code;
    rsp_ptr->ret_stat_code = TEST_OK_S; // test ok

    for( nIndex = 0 ; nIndex < TESTMODE_MSTR_TBL_SIZE  ; nIndex++)
    {
        if( testmode_mstr_tbl[nIndex].cmd_code == req_ptr->sub_cmd_code)
        {
            if( testmode_mstr_tbl[nIndex].which_procesor == ARM11_PROCESSOR)
                func_ptr = testmode_mstr_tbl[nIndex].func_ptr;
            break;
        }
    }
    
    printk(KERN_ERR "[LGF_TestMode] testmode_mstr_tbl Index : %d \n", nIndex);

    if( func_ptr != NULL)
    {
    		printk(KERN_ERR "[LGF_TestMode] inner if(func_ptr!=NULL) \n");
        return func_ptr( &(req_ptr->test_mode_req), rsp_ptr);
    }
    else
    {
        if(req_ptr->test_mode_req.version == VER_HW)
        {
        		printk(KERN_ERR "[LGF_TestMode] inner if(req_ptr->test_mode_req.version == VER_HW) \n");
            CheckHWRev((byte *)rsp_ptr->test_mode_rsp.str_buf);
        }
        else
            send_to_arm9((void*)req_ptr, (void*)rsp_ptr);
    }

    return (rsp_ptr);
}
EXPORT_SYMBOL(LGF_TestMode);

void* linux_app_handler(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    diagpkt_free(pRsp);
    return 0;
}

void* not_supported_command_handler(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
    return pRsp;
}

char external_memory_copy_test(void)
{
    char return_value = TEST_FAIL_S;
    char *src = (void *)0;
    char *dest = (void *)0;
    off_t fd_offset;
    int fd;
    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

    if ( (fd = sys_open((const char __user *) "/sdcard/_ExternalSD/SDTest.txt", O_CREAT | O_RDWR, 0) ) < 0 )
    {
        printk(KERN_ERR "[Testmode Memory Test] Can not access SD card\n");
        goto file_fail;
    }

    if ( (src = kmalloc(10, GFP_KERNEL)) )
    {
        sprintf(src,"TEST");
        if ((sys_write(fd, (const char __user *) src, 5)) < 0)
        {
            printk(KERN_ERR "[Testmode Memory Test] Can not write SD card \n");
            goto file_fail;
        }

        fd_offset = sys_lseek(fd, 0, 0);
    }

    if ( (dest = kmalloc(10, GFP_KERNEL)) )
    {
        if ((sys_read(fd, (char __user *) dest, 5)) < 0)
        {
            printk(KERN_ERR "[Testmode Memory Test] Can not read SD card \n");
            goto file_fail;
        }

        if ((memcmp(src, dest, 4)) == 0)
            return_value = TEST_OK_S;
        else
            return_value = TEST_FAIL_S;
    }

    kfree(src);
    kfree(dest);

file_fail:
    sys_close(fd);
    set_fs(old_fs);
    sys_unlink((const char __user *)"/sdcard/_ExternalSD/SDTest.txt");

    return return_value;
}

extern int external_memory_test_diag;

void* LGF_ExternalSocketMemory(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    struct statfs_local sf;
    pRsp->ret_stat_code = TEST_FAIL_S;

    // ADD: 0013541: 0014142: [Test_Mode] To remove Internal memory information in External memory test when SD-card is not exist
#if 0
// m3 use Internal SD, so we dont use this
#else
//    if(gpio_get_value(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SDC3_DET - 1)))
    if(gpio_get_value(SYS_GPIO_SD_DET)) //dy.lee
    {
        if (pReq->esm == EXTERNAL_SOCKET_MEMORY_CHECK)
        {
            pRsp->test_mode_rsp.memory_check = TEST_FAIL_S;
            pRsp->ret_stat_code = TEST_OK_S;
        }
        
        printk(KERN_ERR "[Testmode Memory Test] Can not detect SD card\n");
        return pRsp;
    }
#endif

    switch( pReq->esm){
        case EXTERNAL_SOCKET_MEMORY_CHECK:
            pRsp->test_mode_rsp.memory_check = external_memory_copy_test();
            pRsp->ret_stat_code = TEST_OK_S;
            break;

        case EXTERNAL_FLASH_MEMORY_SIZE:
            if (sys_statfs("/sdcard/_ExternalSD", (struct statfs *)&sf) != 0)
            {
                printk(KERN_ERR "[Testmode Memory Test] can not get sdcard infomation \n");
                break;
            }

            pRsp->test_mode_rsp.socket_memory_size = ((long long)sf.f_blocks * (long long)sf.f_bsize) >> 20; // needs Mb.
            pRsp->ret_stat_code = TEST_OK_S;
            break;

        case EXTERNAL_SOCKET_ERASE:
            if (diagpdev == NULL){
                  diagpdev = diagcmd_get_dev();
                  printk("\n[%s] diagpdev is Null", __func__ );
            }
            
            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "MMCFORMAT", 1);
                msleep(5000);
                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                printk("\n[%s] error FACTORY_RESET", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
            }
            break;

        case EXTERNAL_FLASH_MEMORY_USED_SIZE:
            external_memory_test_diag = -1;
            update_diagcmd_state(diagpdev, "CALCUSEDSIZE", 0);
            msleep(1000);

            if(external_memory_test_diag != -1)
            {
                pRsp->test_mode_rsp.socket_memory_usedsize = external_memory_test_diag;
                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                pRsp->ret_stat_code = TEST_FAIL_S;
                printk(KERN_ERR "[CALCUSEDSIZE] DiagCommandObserver returned fail or didn't return in 100ms.\n");
            }

            break;

        case EXTERNAL_FLASH_MEMORY_CONTENTS_CHECK:
            external_memory_test_diag = -1;
            update_diagcmd_state(diagpdev, "CHECKCONTENTS", 0);
            msleep(1000);
            
            if(external_memory_test_diag != -1)
            {
                if(external_memory_test_diag == 1)
                    pRsp->test_mode_rsp.memory_check = TEST_OK_S;
                else 
                    pRsp->test_mode_rsp.memory_check = TEST_FAIL_S;

                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                pRsp->ret_stat_code = TEST_FAIL_S;
                printk(KERN_ERR "[CHECKCONTENTS] DiagCommandObserver returned fail or didn't return in 1000ms.\n");
            }
            
            break;

        case EXTERNAL_FLASH_MEMORY_ERASE:
            external_memory_test_diag = -1;
            update_diagcmd_state(diagpdev, "ERASEMEMORY", 0);
            msleep(5000);
            
            if(external_memory_test_diag != -1)
            {
                if(external_memory_test_diag == 1)
                    pRsp->test_mode_rsp.memory_check = TEST_OK_S;
                else
                    pRsp->test_mode_rsp.memory_check = TEST_FAIL_S;

                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                pRsp->ret_stat_code = TEST_FAIL_S;
                printk(KERN_ERR "[ERASEMEMORY] DiagCommandObserver returned fail or didn't return in 5000ms.\n");
            }
            
            break;

        case EXTERNAL_SOCKET_ERASE_SDCARD_ONLY: /*0xE*/
            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "MMCFORMAT", EXTERNAL_SOCKET_ERASE_SDCARD_ONLY);
                msleep(5000);
                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                printk("\n[%s] error EXTERNAL_SOCKET_ERASE_SDCARD_ONLY", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
            }
            break;

        case EXTERNAL_SOCKET_ERASE_FAT_ONLY: /*0xF*/
            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "MMCFORMAT", EXTERNAL_SOCKET_ERASE_FAT_ONLY);
                msleep(5000);
                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                printk("\n[%s] error EXTERNAL_SOCKET_ERASE_FAT_ONLY", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
            }
            break;

        default:
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            break;
    }

    return pRsp;
}

void* LGF_TestModeBattLevel(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-10-03

	DIAG_TEST_MODE_F_req_type req_ptr;

  	req_ptr.sub_cmd_code = TEST_MODE_BATT_LEVEL_TEST;
  	req_ptr.test_mode_req.batt = pReq->batt;

 	pRsp->ret_stat_code = TEST_FAIL_S;

//#ifdef CONFIG_LGE_BATT_SOC_FOR_NPST
#if 0
    int battery_soc = 0;
    extern int max17040_get_battery_capacity_percent(void);

    pRsp->ret_stat_code = TEST_OK_S;

    printk(KERN_ERR "%s, pRsp->ret_stat_code : %d\n", __func__, pReq->batt);
    if(pReq->batt == BATTERY_FUEL_GAUGE_SOC_NPST)
    {
        battery_soc = (int)max17040_get_battery_capacity_percent();
    }
    else
    {
        pRsp->ret_stat_code = TEST_FAIL_S;
    }

    if(battery_soc > 100)
        battery_soc = 100;
    else if (battery_soc < 0)
        battery_soc = 0;

    printk(KERN_ERR "%s, battery_soc : %d\n", __func__, battery_soc);

    sprintf((char *)pRsp->test_mode_rsp.batt_voltage, "%d", battery_soc);

    printk(KERN_ERR "%s, battery_soc : %s\n", __func__, (char *)pRsp->test_mode_rsp.batt_voltage);
#endif

	send_to_arm9((void*)&req_ptr, (void*)pRsp);
	//printk(KERN_INFO "%s, result : %s\n", __func__, pRsp->ret_stat_code==TEST_OK_S?"OK":"FALSE");

	if(pRsp->ret_stat_code == TEST_FAIL_S)
	{
		printk(KERN_ERR "[Testmode]send_to_arm9 response : %d\n", pRsp->ret_stat_code);
		pRsp->ret_stat_code = TEST_FAIL_S;
	}
	else if(pRsp->ret_stat_code == TEST_OK_S)
	{
	  	printk(KERN_ERR "[Testmode]send_to_arm9 response : %d\n", pRsp->ret_stat_code);
        pRsp->ret_stat_code = TEST_OK_S;
	}
//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-10-03
    return pRsp;
}

void* LGF_TestModeKeyData(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{

    pRsp->ret_stat_code = TEST_OK_S;

#ifdef CONFIG_LGE_DIAG_KEYPRESS
    LGF_SendKey(LGF_KeycodeTrans(pReq->key_data));
#endif

    return pRsp;
}


/* 2011.07.22 woochang.chun@lge.com, to ignore key, touch event on sleep mode (250-42-0) */
#ifdef CONFIG_LGE_DIAG_DISABLE_INPUT_DEVICES_ON_SLEEP_MODE 
static int test_mode_disable_input_devices = 0;
void LGF_TestModeSetDisableInputDevices(int value)
{
    test_mode_disable_input_devices = value;
}
int LGF_TestModeGetDisableInputDevices(void)
{
    return test_mode_disable_input_devices;
}
EXPORT_SYMBOL(LGF_TestModeGetDisableInputDevices);
#endif

void* LGF_TestModeSleepMode(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    pRsp->ret_stat_code = TEST_FAIL_S;

#ifdef CONFIG_LGE_DIAG_DISABLE_INPUT_DEVICES_ON_SLEEP_MODE 
    switch(pReq->sleep_mode)
    {
        /* ignore touch, key events on this mode */
        case SLEEP_MODE_ON:
            printk(KERN_INFO "%s, disable input devices..\n", __func__);
            LGF_TestModeSetDisableInputDevices(1);
            pRsp->ret_stat_code = TEST_OK_S;
            break;

        case AIR_PLAIN_MODE_ON:
        case FTM_BOOT_ON:
            break;

        default:
            break;
    }
#endif

    return pRsp;
}

void* LGF_TestModeVirtualSimTest(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{

    pRsp->ret_stat_code = TEST_OK_S;
    return pRsp;
}

//extern int boot_info;

void* LGF_TestModeFBoot(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    switch( pReq->fboot)
    {
        case FIRST_BOOTING_COMPLETE_CHECK:
            if (boot_info)
                pRsp->ret_stat_code = TEST_OK_S;
            else
                pRsp->ret_stat_code = TEST_FAIL_S;
            break;

#if 0
        case FIRST_BOOTING_CHG_MODE_CHECK:
            if(get_first_booting_chg_mode_status() == 1)
                pRsp->ret_stat_code = FIRST_BOOTING_IN_CHG_MODE;
            else
                pRsp->ret_stat_code = FIRST_BOOTING_NOT_IN_CHG_MODE;
            break;
#endif

        default:
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            break;
    }
    return pRsp;
}

extern int db_integrity_ready;
extern int fpri_crc_ready;
extern int file_crc_ready;
extern int db_dump_ready;
extern int db_copy_ready;

typedef struct {
    char ret[32];
} testmode_rsp_from_diag_type;

extern testmode_rsp_from_diag_type integrity_ret;
void* LGF_TestModeDBIntegrityCheck(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    unsigned int crc_val;

    memset(integrity_ret.ret, 0, 32);

    if (diagpdev != NULL)
    {
        update_diagcmd_state(diagpdev, "DBCHECK", pReq->db_check);

        switch(pReq->db_check)
        {
            case DB_INTEGRITY_CHECK:
                while ( !db_integrity_ready )
                    msleep(10);

                db_integrity_ready = 0;

                msleep(100); // wait until the return value is written to the file

                crc_val = (unsigned int)simple_strtoul(integrity_ret.ret+1,NULL,16);
                sprintf(pRsp->test_mode_rsp.str_buf, "0x%08X", crc_val);

                printk(KERN_INFO "%s\n", integrity_ret.ret);
                printk(KERN_INFO "%d\n", crc_val);
                printk(KERN_INFO "%s\n", pRsp->test_mode_rsp.str_buf);

                pRsp->ret_stat_code = TEST_OK_S;
                break;

            case FPRI_CRC_CHECK:
                while ( !fpri_crc_ready )
                    msleep(10);

                fpri_crc_ready = 0;

                msleep(100); // wait until the return value is written to the file

                crc_val = (unsigned int)simple_strtoul(integrity_ret.ret+1,NULL,16);
                sprintf(pRsp->test_mode_rsp.str_buf, "0x%08X", crc_val);

                printk(KERN_INFO "%s\n", integrity_ret.ret);
                printk(KERN_INFO "%d\n", crc_val);
                printk(KERN_INFO "%s\n", pRsp->test_mode_rsp.str_buf);

                pRsp->ret_stat_code = TEST_OK_S;
                break;

            case FILE_CRC_CHECK:
                while ( !file_crc_ready )
                    msleep(10);

                file_crc_ready = 0;

                msleep(100); // wait until the return value is written to the file

                crc_val = (unsigned int)simple_strtoul(integrity_ret.ret+1,NULL,16);
                sprintf(pRsp->test_mode_rsp.str_buf, "0x%08X", crc_val);

                printk(KERN_INFO "%s\n", integrity_ret.ret);
                printk(KERN_INFO "%d\n", crc_val);
                printk(KERN_INFO "%s\n", pRsp->test_mode_rsp.str_buf);

                pRsp->ret_stat_code = TEST_OK_S;
                break;

            case CODE_PARTITION_CRC_CHECK:
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                break;

            case TOTAL_CRC_CHECK:
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                break;

            case DB_DUMP_CHECK:
                while ( !db_dump_ready )
                    msleep(10);

                db_dump_ready = 0;

                msleep(100); // wait until the return value is written to the file

                if (integrity_ret.ret[0] == '0')
                    pRsp->ret_stat_code = TEST_OK_S;
                else
                    pRsp->ret_stat_code = TEST_FAIL_S;

                break;

            case DB_COPY_CHECK:
                while ( !db_copy_ready )
                    msleep(10);

                db_copy_ready = 0;

                msleep(100); // wait until the return value is written to the file

                if (integrity_ret.ret[0] == '0')
                    pRsp->ret_stat_code = TEST_OK_S;
                else
                    pRsp->ret_stat_code = TEST_FAIL_S;

                break;

            default :
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                break;
        }
    }
    else
    {
        printk("\n[%s] error DBCHECK", __func__ );
        pRsp->ret_stat_code = TEST_FAIL_S;
    }

    printk(KERN_ERR "[_DBCHECK_] [%s:%d] DBCHECK Result=<%s>\n", __func__, __LINE__, integrity_ret.ret);

    return pRsp;
}

// LGE_UPDATE_FOTA_S M3 bryan.oh@lge.com 2011/09/29
 #define fota_id_length 15
void* LGF_TestModeFotaIDCheck(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
	int fd = -1;
	int i = 0;
	char fota_id_read[fota_id_length] = {0,};
	char *src = (void *)0;	
	mm_segment_t old_fs=get_fs();
    set_fs(get_ds());
	
    if (diagpdev != NULL)
    {
        switch( pReq->fota_id_check)
        {
            case FOTA_ID_CHECK:
                update_diagcmd_state(diagpdev, "FOTAIDCHECK", 0);
                msleep(500);

				if ( (fd = sys_open((const char __user *) "/sys/module/lge_emmc_direct_access/parameters/fota_id_check", O_CREAT | O_RDWR, 0777) ) < 0 )
			    {
			    	printk(KERN_ERR "[FOTA_TEST_MODE] Can not open file .\n");
					pRsp->ret_stat_code = TEST_FAIL_S;
					goto fota_fail;
			    }
				if ( (src = kmalloc(20, GFP_KERNEL)) )
				{
					if ((sys_read(fd, (char __user *) src, 2)) < 0)
					{
						printk(KERN_ERR "[FOTA_TEST_MODE] Can not read file.\n");
						pRsp->ret_stat_code = TEST_FAIL_S;
						goto fota_fail;
					}
			        if ((memcmp(src, "0", 1)) == 0)
			        {
			        	kfree(src);
			        	sys_unlink((const char __user *)"/sys/module/lge_emmc_direct_access/parameters/fota_id_check");	
			       		pRsp->ret_stat_code = TEST_OK_S;
						printk(KERN_ERR "[##LMH_TEST] TEST_OK \n");	
						return pRsp; 
			        }	
			        else
			        {
			        	kfree(src);
						sys_unlink((const char __user *)"/sys/module/lge_emmc_direct_access/parameters/fota_id_check");	
			       		pRsp->ret_stat_code = TEST_FAIL_S;
						printk(KERN_ERR "[##LMH_TEST] TEST_FAIL \n");	
						return pRsp;
			        }	
				}
				
                break;
				
            case FOTA_ID_READ:
                update_diagcmd_state(diagpdev, "FOTAIDREAD", 0);
                msleep(500);

				if ( (fd = sys_open((const char __user *) "/sys/module/lge_emmc_direct_access/parameters/fota_id_read", O_CREAT | O_RDWR, 0777) ) < 0 )
			    {
			    	printk(KERN_ERR "[FOTA_TEST_MODE] Can not open file .\n");
					pRsp->ret_stat_code = TEST_FAIL_S;
					goto fota_fail;
			    }
				printk(KERN_ERR "[##LMH_TEST] fota_id_check is %s \n", fota_id_read);

				{
					if (sys_read(fd, (char __user *) fota_id_read, fota_id_length) < 0)
					{
						printk(KERN_ERR "[FOTA_TEST_MODE] Can not read file.\n");
						pRsp->ret_stat_code = TEST_FAIL_S;
						goto fota_fail;
					}
			        if ((memcmp((void*)fota_id_read, "fail", 4)) != 0)	//f is fail, and f is not 0x
			        {

			        	sys_unlink((const char __user *)"/sys/module/lge_emmc_direct_access/parameters/fota_id_read");	
						printk(KERN_ERR "[##LMH_TEST] fota_id_check is %s \n", fota_id_read);
			       		pRsp->ret_stat_code = TEST_OK_S;
		
						for(i=0;i<fota_id_length;i++){
							pRsp->test_mode_rsp.fota_id[i] = fota_id_read[i];
							printk(KERN_ERR "[##LMH_TEST] fota_id_check is %d \n", fota_id_read[i]);
						}
						printk(KERN_ERR "[##LMH_TEST] TEST_OK \n");	
						return pRsp; 
			        }	
				   
			        else
			        {
						sys_unlink((const char __user *)"/sys/module/lge_emmc_direct_access/parameters/fota_id_read");	
			       		pRsp->ret_stat_code = TEST_FAIL_S;
						printk(KERN_ERR "[##LMH_TEST] TEST_FAIL \n");	
						return pRsp;
			        }	
			        
				}
				
                break;

            default:
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                break;
        }
    }
    else
        pRsp->ret_stat_code = TEST_FAIL_S;

fota_fail:
		kfree(src);
		sys_close(fd);
		set_fs(old_fs); 
		sys_unlink((const char __user *)"/sys/module/lge_emmc_direct_access/parameters/fota_id_check");
		sys_unlink((const char __user *)"/sys/module/lge_emmc_direct_access/parameters/fota_id_read");

    return pRsp;
}
// LGE_UPDATE_FOTA_E M3 bryan.oh@lge.com 2011/09/29

// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
static char wifi_get_rx_packet_info(rx_packet_info_t* rx_info)
{
	const char* src = "/data/misc/wifi/diag_wifi_result";
	char return_value = TEST_FAIL_S;
	char *dest = (void *)0;
	char buf[30];
	off_t fd_offset;
	int fd;
	char *tok, *holder = NULL;
	char *delimiter = ":\r\n";
	substring_t args[MAX_OPT_ARGS];	
	int token;	
	char tmpstr[10];

    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

	if (rx_info == NULL) {
		goto file_fail;
	}
	
	memset(buf, 0x00, sizeof(buf));

    if ( (fd = sys_open((const char __user *)src, O_CREAT | O_RDWR, 0) ) < 0 )
    {
        printk(KERN_ERR "[Testmode Wi-Fi] sys_open() failed!!\n");
        goto file_fail;
    }

    if ( (dest = kmalloc(30, GFP_KERNEL)) )
    {
        fd_offset = sys_lseek(fd, 0, 0);

        if ((sys_read(fd, (char __user *) dest, 30)) < 0)
        {
            printk(KERN_ERR "[Testmode Wi-Fi] can't read path %s \n", src);
            goto file_fail;
        }

#if 0 
		/*	sscanf(dest, "%d:%d", &(rx_info->goodpacket), &(rx_info->badpacket));    */
		strncpy(buf, (const char *)dest, sizeof(buf) - 1) ;

		tok = strtok_r(dest, delimiter, &holder);

		if ( holder != NULL && tok != NULL)
		{
			rx_info->goodpacket = simple_strtoul(tok, (char**)NULL, 10);
			tok = strtok_r(NULL, delimiter, &holder);
			rx_info->badpacket = simple_strtoul(tok, (char**)NULL, 10);
			printk(KERN_ERR "[Testmode Wi-Fi] rx_info->goodpacket %lu, rx_info->badpacket = %lu \n",
				rx_info->goodpacket, rx_info->badpacket);
			return_value = TEST_OK_S;
		}
#else
		if ((memcmp(dest, "30", 2)) == 0) {
			printk(KERN_INFO "rx_packet_cnt read error \n");
			goto file_fail;
		}

		strncpy(buf, (const char *)dest, sizeof(buf) - 1);
		buf[sizeof(buf)-1] = 0;
		holder = &(buf[2]); // skip index, result
		
		while (holder != NULL) {
			tok = strsep(&holder, delimiter);
			
			if (!*tok)
				continue;

			token = match_token(tok, param_tokens, args);
			switch (token) {
			case Param_goodpacket:
				memset(tmpstr, 0x00, sizeof(tmpstr));
				if (0 == match_strlcpy(tmpstr, &args[0], sizeof(tmpstr)))
				{
					printk(KERN_ERR "Error GoodPacket %s", args[0].from);
					continue;
				}
				rx_info->goodpacket = simple_strtol(tmpstr, NULL, 0);
				printk(KERN_INFO "[Testmode Wi-Fi] rx_info->goodpacket = %d", rx_info->goodpacket);
				break;

			case Param_badpacket:
				memset(tmpstr, 0x00, sizeof(tmpstr));
				if (0 == match_strlcpy(tmpstr, &args[0], sizeof(tmpstr)))
				{
					printk(KERN_ERR "Error BadPacket %s\n", args[0].from);
					continue;
				}

				rx_info->badpacket = simple_strtol(tmpstr, NULL, 0);
				printk(KERN_INFO "[Testmode Wi-Fi] rx_info->badpacket = %d", rx_info->badpacket);
				return_value = TEST_OK_S;
				break;

			case Param_end:
			case Param_err:
			default:
				/* silently ignore unknown settings */
				printk(KERN_ERR "[Testmode Wi-Fi] ignore unknown token %s\n", tok);
				break;
			}
		}
#endif
    }

	printk(KERN_INFO "[Testmode Wi-Fi] return_value %d!!\n", return_value);
	
file_fail:    
    kfree(dest);
    sys_close(fd);
    set_fs(old_fs);
    sys_unlink((const char __user *)src);
    return return_value;
}


static char wifi_get_test_results(int index)
{
	const char* src = "/data/misc/wifi/diag_wifi_result";
    char return_value = TEST_FAIL_S;
    char *dest = (void *)0;
	char buf[4]={0};
    off_t fd_offset;
    int fd;
    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

    if ( (fd = sys_open((const char __user *)src, O_CREAT | O_RDWR, 0) ) < 0 )
    {
        printk(KERN_ERR "[Testmode Wi-Fi] sys_open() failed!!\n");
        goto file_fail;
    }

    if ( (dest = kmalloc(20, GFP_KERNEL)) )
    {
        fd_offset = sys_lseek(fd, 0, 0);

        if ((sys_read(fd, (char __user *) dest, 20)) < 0)
        {
            printk(KERN_ERR "[Testmode Wi-Fi] can't read path %s \n", src);
            goto file_fail;
        }

		sprintf(buf, "%d""1", index);
		buf[3]='\0';
        printk(KERN_INFO "[Testmode Wi-Fi] result %s!!\n", buf);

        if ((memcmp(dest, buf, 2)) == 0)
            return_value = TEST_OK_S;
        else
            return_value = TEST_FAIL_S;
		
        printk(KERN_ERR "[Testmode Wi-Fi] return_value %d!!\n", return_value);

    }
	
file_fail:
    kfree(dest);
    sys_close(fd);
    set_fs(old_fs);
    sys_unlink((const char __user *)src);

    return return_value;
}


static test_mode_ret_wifi_ctgry_t divide_into_wifi_category(test_mode_req_wifi_type input)
{
	test_mode_ret_wifi_ctgry_t sub_category = WLAN_TEST_MODE_CTGRY_NOT_SUPPORTED;
	
	if ( input == WLAN_TEST_MODE_54G_ON || 
		WL_IS_WITHIN(WLAN_TEST_MODE_11B_ON, WLAN_TEST_MODE_11A_CH_RX_START, input)) {
		sub_category = WLAN_TEST_MODE_CTGRY_ON;
	} else if ( input == WLAN_TEST_MODE_OFF ) {
		sub_category = WLAN_TEST_MODE_CTGRY_OFF;
	} else if ( input == WLAN_TEST_MODE_RX_RESULT ) {
		sub_category = WLAN_TEST_MODE_CTGRY_RX_STOP;
	} else if ( WL_IS_WITHIN(WLAN_TEST_MODE_RX_START, WLAN_TEST_MODE_RX_RESULT, input) || 
			WL_IS_WITHIN(WLAN_TEST_MODE_LF_RX_START, WLAN_TEST_MODE_MF_TX_START, input)) {
        sub_category = WLAN_TEST_MODE_CTGRY_RX_START;
	} else if ( WL_IS_WITHIN(WLAN_TEST_MODE_TX_START, WLAN_TEST_MODE_TXRX_STOP, input) || 
			WL_IS_WITHIN( WLAN_TEST_MODE_MF_TX_START, WLAN_TEST_MODE_11B_ON, input)) {
		sub_category = WLAN_TEST_MODE_CTGRY_TX_START;
	} else if ( input == WLAN_TEST_MODE_TXRX_STOP) {
		sub_category = WLAN_TEST_MODE_CTGRY_TX_STOP;
	}
	
	printk(KERN_INFO "[divide_into_wifi_category] input = %d, sub_category = %d!!\n", input, sub_category );
	
	return sub_category;	
}


void* LGF_TestModeWLAN(
        test_mode_req_type*	pReq,
        DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int i;
	static int first_on_try = 10;
	test_mode_ret_wifi_ctgry_t wl_category;

	if (diagpdev != NULL)
	{
		update_diagcmd_state(diagpdev, "WIFI_TEST_MODE", pReq->wifi);

		printk(KERN_ERR "[WI-FI] [%s:%d] WiFiSubCmd=<%d>\n", __func__, __LINE__, pReq->wifi);

		wl_category = divide_into_wifi_category(pReq->wifi);

		/* Set Test Mode */
		switch (wl_category) {

			case WLAN_TEST_MODE_CTGRY_ON:
				//[10sec timeout] when wifi turns on, it takes about 9seconds to bring up FTM mode.
				for (i = 0; i< first_on_try ; i++) {
					msleep(1000);
				}
				
				first_on_try = 5;

				pRsp->ret_stat_code = wifi_get_test_results(wl_category);
				pRsp->test_mode_rsp.wlan_status = !(pRsp->ret_stat_code);
				break;

			case WLAN_TEST_MODE_CTGRY_OFF:
				//5sec timeout
				for (i = 0; i< 3; i++)
					msleep(1000);
				pRsp->ret_stat_code = wifi_get_test_results(wl_category);
				break;

			case WLAN_TEST_MODE_CTGRY_RX_START:
				for (i = 0; i< 2; i++)
					msleep(1000);
				pRsp->ret_stat_code = wifi_get_test_results(wl_category);
				pRsp->test_mode_rsp.wlan_status = !(pRsp->ret_stat_code);
				break;

			case WLAN_TEST_MODE_CTGRY_RX_STOP:
			{
				rx_packet_info_t rx_info;
				int total_packet = 0;
				int m_rx_per = 0;
				// init
				rx_info.goodpacket = 0;
				rx_info.badpacket = 0;
				// wait 3 sec
				for (i = 0; i< 3; i++)
					msleep(1000);
				
				pRsp->test_mode_rsp.wlan_rx_results.packet = 0;
				pRsp->test_mode_rsp.wlan_rx_results.per = 0;

				pRsp->ret_stat_code = wifi_get_rx_packet_info(&rx_info);
				if (pRsp->ret_stat_code == TEST_OK_S) {
					total_packet = rx_info.badpacket + rx_info.goodpacket;
					if(total_packet > 0) {
						m_rx_per = (rx_info.badpacket * 1000 / total_packet);
						printk(KERN_INFO "[WI-FI] per = %d, rx_info.goodpacket = %d, rx_info.badpacket = %d ",
							m_rx_per, rx_info.goodpacket, rx_info.badpacket);
					}
					pRsp->test_mode_rsp.wlan_rx_results.packet = rx_info.goodpacket;
					pRsp->test_mode_rsp.wlan_rx_results.per = m_rx_per;
				}				
				break;
			}

			case WLAN_TEST_MODE_CTGRY_TX_START:
				for (i = 0; i< 2; i++)
					msleep(1000);
				pRsp->ret_stat_code = wifi_get_test_results(wl_category);
				pRsp->test_mode_rsp.wlan_status = !(pRsp->ret_stat_code);
				break;

			case WLAN_TEST_MODE_CTGRY_TX_STOP:
				for (i = 0; i< 2; i++)
					msleep(1000);
				pRsp->ret_stat_code = wifi_get_test_results(wl_category);
				pRsp->test_mode_rsp.wlan_status = !(pRsp->ret_stat_code);
				break;

			default:
				pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
				break;
		}
	}
	else
	{
		printk(KERN_ERR "[WI-FI] [%s:%d] diagpdev %d ERROR\n", __func__, __LINE__, pReq->wifi);
		pRsp->ret_stat_code = TEST_FAIL_S;
	}

	return pRsp;
}

// LGE_CHANGE_E, real-wifi@lge.com, 20110928, [WLAN TEST MODE]



// LGE_CHANGE_S, bill.jung@lge.com, 20110808, WiFi MAC R/W Function by DIAG
void* LGF_TestModeWiFiMACRW(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
	DIAG_TEST_MODE_F_req_type req_ptr;

	req_ptr.sub_cmd_code = TEST_MODE_WIFI_MAC_RW;
	printk(KERN_ERR "[LGF_TestModeWiFiMACRW] req_type=%d, wifi_mac_addr=[%s]\n", pReq->wifi_mac_ad.req_type, pReq->wifi_mac_ad.wifi_mac_addr);

	if (diagpdev != NULL)
	{
		pRsp->ret_stat_code = TEST_FAIL_S;
		if( pReq->wifi_mac_ad.req_type == 0) {
			req_ptr.test_mode_req.wifi_mac_ad.req_type = 0;
			memcpy(req_ptr.test_mode_req.wifi_mac_ad.wifi_mac_addr, (void*)(pReq->wifi_mac_ad.wifi_mac_addr), WIFI_MAC_ADDR_CNT);
			send_to_arm9((void*)&req_ptr, (void*)pRsp);
			printk(KERN_INFO "[Wi-Fi] %s, result : %s\n", __func__, pRsp->ret_stat_code==TEST_OK_S?"OK":"FAILURE");
		} else if ( pReq->wifi_mac_ad.req_type == 1) {
			req_ptr.test_mode_req.wifi_mac_ad.req_type = 1;
			send_to_arm9((void*)&req_ptr, (void*)pRsp);
			printk(KERN_INFO "[Wi-Fi] %s, result : %s\n", __func__, pRsp->ret_stat_code==TEST_OK_S?"OK":"FAILURE");
		}
		else{
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		}
	}
	else
	{
		printk(KERN_ERR "[WI-FI] [%s:%d] diagpdev %d ERROR\n", __func__, __LINE__, pReq->wifi_mac_ad.req_type);
		pRsp->ret_stat_code = TEST_FAIL_S;
	}

	return pRsp;
// LGE_CHANGE_E, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
#if 0
	int fd=0; 
	int i=0;
    char *src = (void *)0;	
    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

	printk(KERN_ERR "[LGF_TestModeWiFiMACRW] req_type=%d, wifi_mac_addr=[%s]\n", pReq->wifi_mac_ad.req_type, pReq->wifi_mac_ad.wifi_mac_addr);

	if (diagpdev != NULL)
	{
		if( pReq->wifi_mac_ad.req_type == 0 )
		{
			printk(KERN_ERR "[LGF_TestModeWiFiMACRW] WIFI_MAC_ADDRESS_WRITE.\n");
			
			if ( (fd = sys_open((const char __user *) "/data/misc/wifi/diag_mac", O_CREAT | O_RDWR, 0777) ) < 0 )
		    {
		    	printk(KERN_ERR "[LGF_TestModeWiFiMACRW] Can not open file.\n");
				pRsp->ret_stat_code = TEST_FAIL_S;
				goto file_fail;
		    }
				
			if ( (src = kmalloc(20, GFP_KERNEL)) )
			{
				sprintf( src,"%c%c%c%c%c%c%c%c%c%c%c%c", pReq->wifi_mac_ad.wifi_mac_addr[0],
					pReq->wifi_mac_ad.wifi_mac_addr[1], pReq->wifi_mac_ad.wifi_mac_addr[2],
					pReq->wifi_mac_ad.wifi_mac_addr[3], pReq->wifi_mac_ad.wifi_mac_addr[4],
					pReq->wifi_mac_ad.wifi_mac_addr[5], pReq->wifi_mac_ad.wifi_mac_addr[6],
					pReq->wifi_mac_ad.wifi_mac_addr[7], pReq->wifi_mac_ad.wifi_mac_addr[8],
					pReq->wifi_mac_ad.wifi_mac_addr[9], pReq->wifi_mac_ad.wifi_mac_addr[10],
					pReq->wifi_mac_ad.wifi_mac_addr[11]
					);
					
				if ((sys_write(fd, (const char __user *) src, 12)) < 0)
				{
					printk(KERN_ERR "[LGF_TestModeWiFiMACRW] Can not write file.\n");
					pRsp->ret_stat_code = TEST_FAIL_S;
					goto file_fail;
				}
			}

			msleep(500);
				
			update_diagcmd_state(diagpdev, "WIFIMACWRITE", 0);
				
			pRsp->ret_stat_code = TEST_OK_S;

		}
		else if(  pReq->wifi_mac_ad.req_type == 1 )
		{
			printk(KERN_ERR "[LGF_TestModeWiFiMACRW] WIFI_MAC_ADDRESS_READ.\n");
			
			update_diagcmd_state(diagpdev, "WIFIMACREAD", 0);

			for( i=0; i< 2; i++ )
			{
				msleep(500);
			}					

			if ( (fd = sys_open((const char __user *) "/data/misc/wifi/diag_mac", O_CREAT | O_RDWR, 0777) ) < 0 )
		    {
		    	printk(KERN_ERR "[LGF_TestModeWiFiMACRW] Can not open file.\n");
				pRsp->ret_stat_code = TEST_FAIL_S;
				goto file_fail;
		    }
			
			if ( (src = kmalloc(20, GFP_KERNEL)) )
			{
				if ((sys_read(fd, (char __user *) src, 12)) < 0)
				{
					printk(KERN_ERR "[LGF_TestModeWiFiMACRW] Can not read file.\n");
					pRsp->ret_stat_code = TEST_FAIL_S;
					goto file_fail;
				}
			}

			for( i=0; i<14; i++)
			{
				pRsp->test_mode_rsp.key_pressed_buf[i] = 0;
			}

			for( i=0; i< 12; i++ )
			{
				pRsp->test_mode_rsp.read_wifi_mac_addr[i] = src[i];
			}

			sys_unlink((const char __user *)"/data/misc/wifi/diag_mac");
					
			pRsp->ret_stat_code = TEST_OK_S;
		}				
		else
		{
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		}
	}
	else
	{
		pRsp->ret_stat_code = TEST_FAIL_S;
	}

file_fail:
	kfree(src);
	
	sys_close(fd);
	set_fs(old_fs); 
	
	return pRsp;
#endif
}
// LGE_CHANGE_E, bill.jung@lge.com, 20110808, WiFi MAC R/W Function by DIAG



#ifndef SKW_TEST
// [110919 kkh8318@lge.com M3_ALL]Added Factory Reset Test [START]
//static unsigned char test_mode_factory_reset_status = FACTORY_RESET_START;
// [110919 kkh8318@lge.com M3_ALL] [END]
#define BUF_PAGE_SIZE 2048
// BEGIN: 0010090 sehyuny.kim@lge.com 2010-10-21
// MOD 0010090: [FactoryReset] Enable Recovery mode FactoryReset

#define FACTORY_RESET_STR       "FACT_RESET_"
#define FACTORY_RESET_STR_SIZE	11
#define FACTORY_RESET_BLK 1 // read / write on the first block

#define MSLEEP_CNT 100

typedef struct MmcPartition MmcPartition;

struct MmcPartition {
    char *device_index;
    char *filesystem;
    char *name;
    unsigned dstatus;
    unsigned dtype ;
    unsigned dfirstsec;
    unsigned dsize;
};
// END: 0010090 sehyuny.kim@lge.com 2010-10-21
#endif

// +s LG_BTUI_DIAGCMD_DUTMODE munho2.lee@lge.com 110915
void* LGF_TestModeBlueTooth(
        test_mode_req_type*	pReq,
        DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
    int fd;
	char *src = (void *)0;		
    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

	printk(KERN_ERR "[_BTUI_] [%s:%d] BTSubCmd=<%d>\n", __func__, __LINE__, pReq->bt);

	if (diagpdev != NULL)
	{
		update_diagcmd_state(diagpdev, "BT_TEST_MODE", pReq->bt);

		printk(KERN_ERR "[_BTUI_] [%s:%d] BTSubCmd=<%d>\n", __func__, __LINE__, pReq->bt);

		/* Set Test Mode */
		if(pReq->bt==1 || (pReq->bt>=11 && pReq->bt<=42)) 
		{			
			msleep(5900); //6sec timeout
		}
		/*Test Mode Check*/
		else if(pReq->bt==2) 
		{
			ssleep(1);
			if ( (fd = sys_open((const char __user *) "/data/bt_dut_test.txt", O_CREAT | O_RDWR, 0777) ) < 0 )
		    {
		    	printk(KERN_ERR "[BT_TEST_MODE] Can not open file .\n");
				pRsp->ret_stat_code = TEST_FAIL_S;
				goto file_fail;
		    }
			if ( (src = kmalloc(20, GFP_KERNEL)) )
			{
				if ((sys_read(fd, (char __user *) src, 3)) < 0)
				{
					printk(KERN_ERR "[BT_TEST_MODE] Can not read file.\n");
					pRsp->ret_stat_code = TEST_FAIL_S;
					goto file_fail;
				}
		        if ((memcmp(src, "on", 2)) == 0)
		        {
		        	kfree(src);
		        	sys_unlink((const char __user *)"/data/bt_dut_test.txt");	
		       		pRsp->ret_stat_code = TEST_OK_S;
					printk(KERN_ERR "[##LMH_TEST] TEST_OK \n");	
					return pRsp; 
		        }	
		        else
		        {
		        	kfree(src);
					sys_unlink((const char __user *)"/data/bt_dut_test.txt");	
		       		pRsp->ret_stat_code = TEST_FAIL_S;
					printk(KERN_ERR "[##LMH_TEST] TEST_FAIL \n");	
					return pRsp;
		        }	
			}		
		}	
		/*Test Mode Release*/
		else if(pReq->bt==5)
		{
			ssleep(3);
		}
		/*Test Mode Not Supported*/
		else
		{
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			return pRsp;
		}

		pRsp->ret_stat_code = TEST_OK_S;
		return pRsp;
	}
	else
	{
		printk(KERN_ERR "[_BTUI_] [%s:%d] BTSubCmd=<%d> ERROR\n", __func__, __LINE__, pReq->bt);
		pRsp->ret_stat_code = TEST_FAIL_S;
		return pRsp;
	}  
file_fail:
	kfree(src);
	
	sys_close(fd);
	set_fs(old_fs); 
	sys_unlink((const char __user *)"/data/bt_dut_test.txt");	
	return pRsp;	
}
// +e LG_BTUI_DIAGCMD_DUTMODE

/* LGE_CHANGE_S [myunghwan.kim@lge.com] 2011-09-27 support test mode */
void* LGF_TestMotor(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		update_diagcmd_state(diagpdev, "MOTOR", pReq->motor);
	}
	else
	{
		printk("\n[%s] error MOTOR", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;
}

void* LGF_TestLCD(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		update_diagcmd_state(diagpdev, "LCD", pReq->lcd);
	}
	else
	{
		printk("\n[%s] error LCD", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;
}


void* LGF_TestAcoustic(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		update_diagcmd_state(diagpdev, "ACOUSTIC", pReq->acoustic);
	}
	else
	{
		printk("\n[%s] error ACOUSTIC", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;
}

byte key_buf[MAX_KEY_BUFF_SIZE];
int count_key_buf = 0;

boolean lgf_factor_key_test_rsp (char key_code)
{
	/* sanity check */
	if (count_key_buf>=MAX_KEY_BUFF_SIZE)
		return FALSE;

	key_buf[count_key_buf++] = key_code;
	return TRUE;
}
EXPORT_SYMBOL(lgf_factor_key_test_rsp);
void* LGT_TestModeKeyTest(test_mode_req_type* pReq, DIAG_TEST_MODE_F_rsp_type *pRsp)
{
  pRsp->ret_stat_code = TEST_OK_S;

  if(pReq->key_test_start){
	memset((void *)key_buf,0x00,MAX_KEY_BUFF_SIZE);
	count_key_buf=0;
	diag_event_log_start();
  }
  else
  {
	memcpy((void *)((DIAG_TEST_MODE_KEY_F_rsp_type *)pRsp)->key_pressed_buf, (void *)key_buf, MAX_KEY_BUFF_SIZE);
	memset((void *)key_buf,0x00,MAX_KEY_BUFF_SIZE);
	diag_event_log_end();
  }  
  return pRsp;
}
void* LGF_TestCam(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;

	switch(pReq->camera)
	{
		case CAM_TEST_CAMERA_SELECT:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			break;

		default:
			if (diagpdev != NULL){

				update_diagcmd_state(diagpdev, "CAMERA", pReq->camera);
			}
			else
			{
				printk("\n[%s] error CAMERA", __func__ );
				pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			}
			break;
	}
	return pRsp;
}

void* LGF_TestModeMP3 (
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		if(pReq->mp3_play == MP3_SAMPLE_FILE)
		{
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		}
		else
		{
			update_diagcmd_state(diagpdev, "MP3", pReq->mp3_play);
		}
	}
	else
	{
		printk("\n[%s] error MP3", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;
}

void* LGF_MemoryVolumeCheck (
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	struct statfs_local  sf;
	unsigned int total = 0;
	unsigned int used = 0;
	unsigned int remained = 0;
	pRsp->ret_stat_code = TEST_OK_S;

	if (sys_statfs("/data", (struct statfs *)&sf) != 0)
	{
		printk(KERN_ERR "[Testmode]can not get sdcard infomation \n");
		pRsp->ret_stat_code = TEST_FAIL_S;
	}
	else
	{

		total = (sf.f_blocks * sf.f_bsize) >> 20;
		remained = (sf.f_bavail * sf.f_bsize) >> 20;
		used = total - remained;

		switch(pReq->mem_capa)
		{
			case MEMORY_TOTAL_CAPA_TEST:
				pRsp->test_mode_rsp.mem_capa = total;
				break;

			case MEMORY_USED_CAPA_TEST:
				pRsp->test_mode_rsp.mem_capa = used;
				break;

			case MEMORY_REMAIN_CAPA_TEST:
				pRsp->test_mode_rsp.mem_capa = remained;
				break;

			default :
				pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
				break;
		}
	}
	return pRsp;
}

void* LGF_TestModeSpeakerPhone(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		if((pReq->speaker_phone == NOMAL_Mic1) || (pReq->speaker_phone == NC_MODE_ON)
				|| (pReq->speaker_phone == ONLY_MIC2_ON_NC_ON) || (pReq->speaker_phone == ONLY_MIC1_ON_NC_ON)
		  )
		{
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		}
		else
		{
			update_diagcmd_state(diagpdev, "SPEAKERPHONE", pReq->speaker_phone);
		}
	}
	else
	{
		printk("\n[%s] error SPEAKERPHONE", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;
}

void* LGT_TestModeVolumeLevel (
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type *pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		update_diagcmd_state(diagpdev, "VOLUMELEVEL", pReq->volume_level);
	}
	else
	{
		printk("\n[%s] error VOLUMELEVEL", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;
}
/* LGE_CHANGE_E [myunghwan.kim@lge.com] 2011-09-27 support test mode */

extern const MmcPartition *lge_mmc_find_partition_by_name(const char *name);

void* LGF_TestModeFactoryReset(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
// [111004 kkh8318@lge.com M3_ALL]Added Factory Reset Test [START]
/* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
/* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
  DIAG_TEST_MODE_F_req_type req_ptr;

  req_ptr.sub_cmd_code = TEST_MODE_FACTORY_RESET_CHECK_TEST;
  req_ptr.test_mode_req.factory_reset = pReq->factory_reset;
/* END: 0014656 jihoon.lee@lge.com 2011024 */

/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
  pRsp->ret_stat_code = TEST_FAIL_S;
/* END: 0014110 jihoon.lee@lge.com 20110115 */

#if 0
  unsigned char pbuf[50]; //no need to have huge size, this is only for the flag
  const MmcPartition *pMisc_part; 
  unsigned char startStatus = FACTORY_RESET_NA; 
  int mtd_op_result = 0;
  unsigned long factoryreset_bytes_pos_in_emmc = 0;
/* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
/* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
  DIAG_TEST_MODE_F_req_type req_ptr;

  req_ptr.sub_cmd_code = TEST_MODE_FACTORY_RESET_CHECK_TEST;
  req_ptr.test_mode_req.factory_reset = pReq->factory_reset;
/* END: 0014656 jihoon.lee@lge.com 2011024 */
  
/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
  pRsp->ret_stat_code = TEST_FAIL_S;
/* END: 0014110 jihoon.lee@lge.com 20110115 */
  
  lge_mmc_scan_partitions();
  pMisc_part = lge_mmc_find_partition_by_name("misc");
  factoryreset_bytes_pos_in_emmc = (pMisc_part->dfirstsec*512)+PTN_FRST_PERSIST_POSITION_IN_MISC_PARTITION;
  
  printk("LGF_TestModeFactoryReset> mmc info sec : 0x%x, size : 0x%x type : 0x%x frst sec: 0x%lx\n", pMisc_part->dfirstsec, pMisc_part->dsize, pMisc_part->dtype, factoryreset_bytes_pos_in_emmc);
#endif
// [111004 kkh8318@lge.com M3_ALL] [END]

/* BEGIN: 0013861 jihoon.lee@lge.com 20110111 */
/* MOD 0013861: [FACTORY RESET] emmc_direct_access factory reset flag access */
/* add carriage return and change flag size for the platform access */
/* END: 0013861 jihoon.lee@lge.com 20110111 */
  switch(pReq->factory_reset)
  {
    case FACTORY_RESET_CHECK :
// [111004 kkh8318@lge.com M3_ALL]Added Factory Reset Test [START]
#if 0  // def CONFIG_LGE_MTD_DIRECT_ACCESS
/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
      memset((void*)pbuf, 0, sizeof(pbuf));
      mtd_op_result = lge_read_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2);

      if( mtd_op_result != (FACTORY_RESET_STR_SIZE+2) )
      {
        printk(KERN_ERR "[Testmode]lge_read_block, read data  = %d \n", mtd_op_result);
        pRsp->ret_stat_code = TEST_FAIL_S;
        break;
      }
      else
      {
        //printk(KERN_INFO "\n[Testmode]factory reset memcmp\n");
        if(memcmp(pbuf, FACTORY_RESET_STR, FACTORY_RESET_STR_SIZE) == 0) // tag read sucess
        {
          startStatus = pbuf[FACTORY_RESET_STR_SIZE] - '0';
          printk(KERN_INFO "[Testmode]factory reset backup status = %d \n", startStatus);
        }
        else
        {
          // if the flag storage is erased this will be called, start from the initial state
          printk(KERN_ERR "[Testmode] tag read failed :  %s \n", pbuf);
        }
      }  
/* END: 0014110 jihoon.lee@lge.com 20110115 */

      test_mode_factory_reset_status = FACTORY_RESET_INITIAL;
      memset((void *)pbuf, 0, sizeof(pbuf));
      sprintf(pbuf, "%s%d\n",FACTORY_RESET_STR, test_mode_factory_reset_status);
      printk(KERN_INFO "[Testmode]factory reset status = %d\n", test_mode_factory_reset_status);

      mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc, FACTORY_RESET_STR_SIZE+2);	
/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
      if(mtd_op_result!= (FACTORY_RESET_STR_SIZE+2))
      {
        printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
        pRsp->ret_stat_code = TEST_FAIL_S;
        break;
      }
      else
      {
        mtd_op_result = lge_write_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2);
        if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
        {
          printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
          pRsp->ret_stat_code = TEST_FAIL_S;
          break;
        }
      }
/* END: 0014110 jihoon.lee@lge.com 20110115 */

/* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
/* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
      //send_to_arm9((void*)(((byte*)pReq) -sizeof(diagpkt_header_type) - sizeof(word)) , pRsp);
      send_to_arm9((void*)&req_ptr, (void*)pRsp);
/* END: 0014656 jihoon.lee@lge.com 2011024 */

/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
      if(pRsp->ret_stat_code != TEST_OK_S)
      {
        printk(KERN_ERR "[Testmode]send_to_arm9 response : %d\n", pRsp->ret_stat_code);
        pRsp->ret_stat_code = TEST_FAIL_S;
        break;
      }
/* END: 0014110 jihoon.lee@lge.com 20110115 */

      /*LG_FW khlee 2010.03.04 -If we start at 5, we have to go to APP reset state(3) directly */
      if((startStatus == FACTORY_RESET_COLD_BOOT_END) || (startStatus == FACTORY_RESET_HOME_SCREEN_END))
        test_mode_factory_reset_status = FACTORY_RESET_COLD_BOOT_START;
      else
        test_mode_factory_reset_status = FACTORY_RESET_ARM9_END;

      memset((void *)pbuf, 0, sizeof(pbuf));
      sprintf(pbuf, "%s%d\n",FACTORY_RESET_STR, test_mode_factory_reset_status);
      printk(KERN_INFO "[Testmode]factory reset status = %d\n", test_mode_factory_reset_status);

      mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc, FACTORY_RESET_STR_SIZE+2);
/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
      if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
      {
        printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
        pRsp->ret_stat_code = TEST_FAIL_S;
        break;
      }
      else
      {
         mtd_op_result = lge_write_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2);
         if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
         {
          printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
          pRsp->ret_stat_code = TEST_FAIL_S;
          break;
         }
      }
/* END: 0014110 jihoon.lee@lge.com 20110115 */

#else /**/
      //send_to_arm9((void*)(((byte*)pReq) -sizeof(diagpkt_header_type) - sizeof(word)) , pRsp);
      send_to_arm9((void*)&req_ptr, (void*)pRsp);

/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
      if(pRsp->ret_stat_code != TEST_OK_S)
      {
        printk(KERN_ERR "[Testmode]send_to_arm9 response : %d\n", pRsp->ret_stat_code);
        pRsp->ret_stat_code = TEST_FAIL_S;
      }
	else
	{
	  printk(KERN_ERR "[Testmode]send_to_arm9 response : %d\n", pRsp->ret_stat_code);
         pRsp->ret_stat_code = TEST_OK_S;
	}
/* END: 0014110 jihoon.lee@lge.com 20110115 */
#endif /*CONFIG_LGE_MTD_DIRECT_ACCESS*/

#if 0
      if((startStatus == FACTORY_RESET_COLD_BOOT_END) || (startStatus == FACTORY_RESET_HOME_SCREEN_END))
      {
        if (diagpdev != NULL)
          update_diagcmd_state(diagpdev, "REBOOT", 0);
        else
        {
          printk(KERN_INFO "%s, factory reset reboot failed \n", __func__);
          pRsp->ret_stat_code = TEST_FAIL_S;
          break;
        }
      }
#endif

      //printk(KERN_INFO "%s, factory reset check completed \n", __func__);
      //pRsp->ret_stat_code = TEST_OK_S;
      break;
// [111004 kkh8318@lge.com M3_ALL] [END]
    case FACTORY_RESET_COMPLETE_CHECK:

	 		send_to_arm9((void*)&req_ptr, (void*)pRsp);
      if(pRsp->ret_stat_code != TEST_OK_S)
      {
        printk(KERN_ERR "[Testmode]send_to_arm9 response : %d\n", pRsp->ret_stat_code);
        break;
      }
      break;
    case FACTORY_RESET_STATUS_CHECK:
#if 0 // def CONFIG_LGE_MTD_DIRECT_ACCESS
      memset((void*)pbuf, 0, sizeof(pbuf));
      mtd_op_result = lge_read_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2 );
/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
      if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
      {
      	 printk(KERN_ERR "[Testmode]lge_read_block, error num = %d \n", mtd_op_result);
      	 pRsp->ret_stat_code = TEST_FAIL_S;
      	 break;
      }
      else
      {
      	 if(memcmp(pbuf, FACTORY_RESET_STR, FACTORY_RESET_STR_SIZE) == 0) // tag read sucess
      	 {
      	   test_mode_factory_reset_status = pbuf[FACTORY_RESET_STR_SIZE] - '0';
      	   printk(KERN_INFO "[Testmode]factory reset status = %d \n", test_mode_factory_reset_status);
      	   pRsp->ret_stat_code = test_mode_factory_reset_status;
      	 }
      	 else
      	 {
      	   printk(KERN_ERR "[Testmode]factory reset tag fail, set initial state\n");
      	   test_mode_factory_reset_status = FACTORY_RESET_START;
      	   pRsp->ret_stat_code = test_mode_factory_reset_status;
      	   break;
      	 }
      }  
/* END: 0014110 jihoon.lee@lge.com 20110115 */
#endif /*CONFIG_LGE_MTD_DIRECT_ACCESS*/
// [111004 kkh8318@lge.com M3_ALL]Added Factory Reset Test [START]
			send_to_arm9((void*)&req_ptr, (void*)pRsp);
// [111004 kkh8318@lge.com M3_ALL] [END]
      break;
    case FACTORY_RESET_COLD_BOOT:
// remove requesting sync to CP as all sync will be guaranteed on their own.
// [111004 kkh8318@lge.com M3_ALL]Added Factory Reset Test [START]
			send_to_arm9((void*)&req_ptr, (void*)pRsp);
      if(pRsp->ret_stat_code == TEST_OK_S)
			{
				printk(KERN_INFO "[Testmode]pRsp->ret_stat_code : TEST_OK_S\n");
// [111018 kkh8318@lge.com M3_ALL]Added Factory Reset Test [START]
				/*
				if (diagpdev != NULL)
          update_diagcmd_state(diagpdev, "REBOOT", 0);
        else
        {
          printk(KERN_INFO "%s, factory reset reboot failed \n", __func__);
          pRsp->ret_stat_code = TEST_FAIL_S;
          break;
        }
        */
// [111018 kkh8318@lge.com M3_ALL]Added Factory Reset Test [START]
			}
			else
				printk(KERN_INFO "[Testmode]pRsp->ret_stat_code : TEST_FAIL_S\n");			
// [111004 kkh8318@lge.com M3_ALL] [END]
#if 0 // def CONFIG_LGE_MTD_DIRECT_ACCESS
      test_mode_factory_reset_status = FACTORY_RESET_COLD_BOOT_START;
      memset((void *)pbuf, 0, sizeof(pbuf));
      sprintf(pbuf, "%s%d",FACTORY_RESET_STR, test_mode_factory_reset_status);
      printk(KERN_INFO "[Testmode]factory reset status = %d\n", test_mode_factory_reset_status);
      mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc,  FACTORY_RESET_STR_SIZE+2);
/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
      if(mtd_op_result!=( FACTORY_RESET_STR_SIZE+2))
      {
        printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
        pRsp->ret_stat_code = TEST_FAIL_S;
        break;
      }
      else
      {
        mtd_op_result = lge_write_block(factoryreset_bytes_pos_in_emmc, pbuf,  FACTORY_RESET_STR_SIZE+2);
        if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
        {
          printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
          pRsp->ret_stat_code = TEST_FAIL_S;
        }
      }
/* END: 0014110 jihoon.lee@lge.com 20110115 */
#endif /*CONFIG_LGE_MTD_DIRECT_ACCESS*/
      break;

    case FACTORY_RESET_ERASE_USERDATA:
#if 0 // def CONFIG_LGE_MTD_DIRECT_ACCESS
      test_mode_factory_reset_status = FACTORY_RESET_COLD_BOOT_START;
      memset((void *)pbuf, 0, sizeof(pbuf));
      sprintf(pbuf, "%s%d",FACTORY_RESET_STR, test_mode_factory_reset_status);
      printk(KERN_INFO "[Testmode-erase userdata]factory reset status = %d\n", test_mode_factory_reset_status);
      mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc , FACTORY_RESET_STR_SIZE+2);
/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
      if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
      {
        printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
        pRsp->ret_stat_code = TEST_FAIL_S;
        break;
      }
      else
      {
        mtd_op_result = lge_write_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2);
        if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
        {
          printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
          pRsp->ret_stat_code = TEST_FAIL_S;
          break;
        }
      }
/* END: 0014110 jihoon.lee@lge.com 20110115 */
#endif /*CONFIG_LGE_MTD_DIRECT_ACCESS*/
    pRsp->ret_stat_code = TEST_OK_S;
    break;

     default:
        pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
        break;
    }
 
  return pRsp;

}

void* LGF_TestScriptItemSet(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
// [111004 kkh8318@lge.com M3_ALL]Added Factory Reset Test [START]
#if 1
/* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
/* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
  DIAG_TEST_MODE_F_req_type req_ptr;
  //int mtd_op_result = 0;
  //const MmcPartition *pMisc_part; 
  //unsigned long factoryreset_bytes_pos_in_emmc = 0; 
//jaeo.park@lge.com [[
  udbp_req_type udbReqType;
  memset(&udbReqType,0x0,sizeof(udbp_req_type));
//]]
  

  req_ptr.sub_cmd_code = TEST_MODE_TEST_SCRIPT_MODE;
  req_ptr.test_mode_req.test_mode_test_scr_mode = pReq->test_mode_test_scr_mode;
/* END: 0014656 jihoon.lee@lge.com 2011024 */

  //lge_mmc_scan_partitions();
  //pMisc_part = lge_mmc_find_partition_by_name("misc");
  //factoryreset_bytes_pos_in_emmc = (pMisc_part->dfirstsec*512)+PTN_FRST_PERSIST_POSITION_IN_MISC_PARTITION;
//  printk("LGF_TestScriptItemSet> mmc info sec : 0x%x, size : 0x%x type : 0x%x frst sec: 0x%lx\n", pMisc_part->dfirstsec, pMisc_part->dsize, pMisc_part->dtype, factoryreset_bytes_pos_in_emmc);

  switch(pReq->test_mode_test_scr_mode)
  {
    case TEST_SCRIPT_ITEM_SET:
 // [111004 kkh8318@lge.com M3_ALL]Added Factory Reset Test [START]   	
    	/*
	mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc, (FACTORY_RESET_STR_SIZE+1) );	
	if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+1))
      {
      	 printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
      	 pRsp->ret_stat_code = TEST_FAIL_S;
      	 break;
	 }
	 */
// [111004 kkh8318@lge.com M3_ALL] [END]
	 
/* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
/* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
      	 //send_to_arm9((void*)(((byte*)pReq) -sizeof(diagpkt_header_type) - sizeof(word)) , pRsp);
      	 send_to_arm9((void*)&req_ptr, (void*)pRsp);
        printk(KERN_INFO "%s, result : %s\n", __func__, pRsp->ret_stat_code==TEST_OK_S?"OK":"FALSE");
/* END: 0014656 jihoon.lee@lge.com 2011024 */
      break;

//jaeo.park@lge.com for SRD cal backup
    case CAL_DATA_BACKUP:
		udbReqType.header.sub_cmd = SRD_INIT_OPERATION;
		LGE_Dload_SRD((udbp_req_type *)&udbReqType,sizeof(udbReqType));//SRD_INIT_OPERATION
		udbReqType.header.sub_cmd = USERDATA_BACKUP_REQUEST;
		LGE_Dload_SRD((udbp_req_type *)&udbReqType,sizeof(udbReqType));//USERDATA_BACKUP_REQUEST
//		printk(KERN_INFO "%s,backup_nv_counter %d\n", __func__,userDataBackUpInfo.info.srd_backup_nv_counter);
		udbReqType.header.sub_cmd = USERDATA_BACKUP_REQUEST_MDM;
		LGE_Dload_SRD((udbp_req_type *)&udbReqType,sizeof(udbReqType));//USERDATA_BACKUP_REQUEST_MDM
//		printk(KERN_INFO "%s,backup_nv_counter %d\n", __func__,userDataBackUpInfo.info.srd_backup_nv_counter);
		break;
    
    case CAL_DATA_RESTORE:
		send_to_arm9((void*)&req_ptr, (void*)pRsp);
		printk(KERN_INFO "%s, result : %s\n", __func__, pRsp->ret_stat_code==TEST_OK_S?"OK":"FALSE");
		break;
/*
  	case CAL_DATA_ERASE:
  	case CAL_DATA_INFO:
  		diagpkt_free(pRsp);
  		return 0;			
  		break;
  */			
    default:
/* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
/* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
      //send_to_arm9((void*)(((byte*)pReq) -sizeof(diagpkt_header_type) - sizeof(word)) , pRsp);
      send_to_arm9((void*)&req_ptr, (void*)pRsp);
      printk(KERN_INFO "%s, cmd : %d, result : %s\n", __func__, pReq->test_mode_test_scr_mode, \
	  										pRsp->ret_stat_code==TEST_OK_S?"OK":"FALSE");
      if(pReq->test_mode_test_scr_mode == TEST_SCRIPT_MODE_CHECK)
      {
        switch(pRsp->test_mode_rsp.test_mode_test_scr_mode)
        {
          case 0:
            printk(KERN_INFO "%s, mode : %s\n", __func__, "USER SCRIPT");
            break;
          case 1:
            printk(KERN_INFO "%s, mode : %s\n", __func__, "TEST SCRIPT");
            break;
          default:
            printk(KERN_INFO "%s, mode : %s, returned %d\n", __func__, "NO PRL", pRsp->test_mode_rsp.test_mode_test_scr_mode);
            break;
        }
      }
/* END: 0014656 jihoon.lee@lge.com 2011024 */
      break;

  }  
        

// END: 0009720 sehyuny.kim@lge.com 2010-10-06

#else
// BEGIN: 0009720 sehyuny.kim@lge.com 2010-10-06
// MOD 0009720: [Modem] It add RF X-Backup feature
  int mtd_op_result = 0;

  const MmcPartition *pMisc_part; 
  unsigned long factoryreset_bytes_pos_in_emmc = 0;
/* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
/* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
  DIAG_TEST_MODE_F_req_type req_ptr;

  req_ptr.sub_cmd_code = TEST_MODE_TEST_SCRIPT_MODE;
  req_ptr.test_mode_req.test_mode_test_scr_mode = pReq->test_mode_test_scr_mode;
/* END: 0014656 jihoon.lee@lge.com 2011024 */

  lge_mmc_scan_partitions();
  pMisc_part = lge_mmc_find_partition_by_name("misc");
  factoryreset_bytes_pos_in_emmc = (pMisc_part->dfirstsec*512)+PTN_FRST_PERSIST_POSITION_IN_MISC_PARTITION;

  printk("LGF_TestScriptItemSet> mmc info sec : 0x%x, size : 0x%x type : 0x%x frst sec: 0x%lx\n", pMisc_part->dfirstsec, pMisc_part->dsize, pMisc_part->dtype, factoryreset_bytes_pos_in_emmc);

  switch(pReq->test_mode_test_scr_mode)
  {
    case TEST_SCRIPT_ITEM_SET:
  #if 1 // def CONFIG_LGE_MTD_DIRECT_ACCESS
      mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc, (FACTORY_RESET_STR_SIZE+1) );
/* BEGIN: 0014110 jihoon.lee@lge.com 20110115 */
/* MOD 0014110: [FACTORY RESET] stability */
/* handle operation or rpc failure as well */
      if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+1))
      {
      	 printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
      	 pRsp->ret_stat_code = TEST_FAIL_S;
      	 break;
/* END: 0014110 jihoon.lee@lge.com 20110115 */
      } else
  #endif /*CONFIG_LGE_MTD_DIRECT_ACCESS*/
      // LG_FW khlee 2010.03.16 - They want to ACL on state in test script state.
      {
      	 update_diagcmd_state(diagpdev, "ALC", 1);
/* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
/* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
      	 //send_to_arm9((void*)(((byte*)pReq) -sizeof(diagpkt_header_type) - sizeof(word)) , pRsp);
      	 send_to_arm9((void*)&req_ptr, (void*)pRsp);
        printk(KERN_INFO "%s, result : %s\n", __func__, pRsp->ret_stat_code==TEST_OK_S?"OK":"FALSE");
/* END: 0014656 jihoon.lee@lge.com 2011024 */
      }
      break;
  /*			
  	case CAL_DATA_BACKUP:
  	case CAL_DATA_RESTORE:
  	case CAL_DATA_ERASE:
  	case CAL_DATA_INFO:
  		diagpkt_free(pRsp);
  		return 0;			
  		break;
  */			
    default:
/* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
/* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
      //send_to_arm9((void*)(((byte*)pReq) -sizeof(diagpkt_header_type) - sizeof(word)) , pRsp);
      send_to_arm9((void*)&req_ptr, (void*)pRsp);
      printk(KERN_INFO "%s, cmd : %d, result : %s\n", __func__, pReq->test_mode_test_scr_mode, \
	  										pRsp->ret_stat_code==TEST_OK_S?"OK":"FALSE");
      if(pReq->test_mode_test_scr_mode == TEST_SCRIPT_MODE_CHECK)
      {
        switch(pRsp->test_mode_rsp.test_mode_test_scr_mode)
        {
          case 0:
            printk(KERN_INFO "%s, mode : %s\n", __func__, "USER SCRIPT");
            break;
          case 1:
            printk(KERN_INFO "%s, mode : %s\n", __func__, "TEST SCRIPT");
            break;
          default:
            printk(KERN_INFO "%s, mode : %s, returned %d\n", __func__, "NO PRL", pRsp->test_mode_rsp.test_mode_test_scr_mode);
            break;
        }
      }
/* END: 0014656 jihoon.lee@lge.com 2011024 */
      break;

  }  
// END: 0009720 sehyuny.kim@lge.com 2010-10-06
#endif 
// [111004 kkh8318@lge.com M3_ALL] [END]
  return pRsp;

}

//====================================================================
// Self Recovery Download Support  diag command 249-XX
//====================================================================
#ifdef CONFIG_LGE_DLOAD_SRD  //kabjoo.choi
PACK (void *)LGE_Dload_SRD (PACK (void *)req_pkt_ptr, uint16 pkg_len)
{

  	udbp_req_type		*req_ptr = (udbp_req_type *) req_pkt_ptr;
	udbp_rsp_type	  	*rsp_ptr = NULL;
	uint16 rsp_len = pkg_len;
	int write_size=0 , mtd_op_result=0;
	rsp_ptr = (udbp_rsp_type *)diagpkt_alloc(DIAG_USET_DATA_BACKUP, rsp_len);

  	// DIAG_TEST_MODE_F_rsp_type union type is greater than the actual size, decrease it in case sensitive items
  		switch(req_ptr->header.sub_cmd)
      		{
  			case  SRD_INIT_OPERATION:				
				diag_SRD_Init(req_ptr,rsp_ptr);							
				break;
				
			case USERDATA_BACKUP_REQUEST:
				printk(KERN_WARNING "USERDATA_BACKUP_REQUEST");
				//CSFB SRD remote 제거 -> diag_userDataBackUp_data 에서 처리함
				//remote_rpc_srd_cmmand(req_ptr, rsp_ptr);  //userDataBackUpStart() 여기서 ... shared ram 저장 하도록. .. 
				diag_userDataBackUp_entrySet(req_ptr,rsp_ptr,0);  //write info data ,  after rpc respons include write_sector_counter  

				//CSFB SRD
				load_srd_base=kmalloc(SIZE_OF_SHARD_RAM, GFP_KERNEL);  //384K byte 
				diag_userDataBackUp_data(req_ptr, rsp_ptr);

				//todo ..  rsp_prt->header.write_sector_counter,  how about checking  no active nv item  ; 
				// write ram data to emmc misc partition  as many as retruned setor counters 
				 
				 if (load_srd_base ==NULL)
				 {
				 	printk(KERN_ERR "UDBU_ERROR_CANNOT_COMPLETE : load_srd_baseis NULL");
				 	((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;	
					break;
				 	// return rsp_ptr;
				 }					
				  
				 write_size= rsp_ptr->rsp_data.write_sector_counter *256;	 //return nv backup counters  

				 if( write_size >SIZE_OF_SHARD_RAM)
				 {
				 	printk(KERN_ERR "UDBU_ERROR_CANNOT_COMPLETE : write_size %d >SIZE_OF_SHARD_RAM %d", write_size, SIZE_OF_SHARD_RAM);
				 	((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;  //hue..
				 	break;
				 }

				 //srd_bytes_pos_in_emmc+512 means that info data already writed at emmc first sector 
				 mtd_op_result = lge_write_block(srd_bytes_pos_in_emmc+512, load_srd_base, write_size);  //512 info data

				
        			if(mtd_op_result!= write_size)
        			{
						printk(KERN_ERR "UDBU_ERROR_CANNOT_COMPLETE : mtd_op_result!= write_size");
				((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;	
				kfree(load_srd_base);	//CSFB SRD
				break;
				//return rsp_ptr;
                   
        			}
				kfree(load_srd_base);	//CSFB SRD
				#if 0
			  	if ( !writeBackUpNVdata( load_srd_base , write_size))
			  	{
					((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;	
				 	 return rsp_ptr;
			  	}
				#endif 

				 
				break;

			case USERDATA_BACKUP_REQUEST_MDM:
				//MDM backup 
//CSFB SRD
/*				((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_SUCCESS;	
				load_srd_shard_base=smem_alloc(SMEM_ERR_CRASH_LOG, SIZE_OF_SHARD_RAM);  //384K byte 
				
				if (load_srd_shard_base ==NULL)
				 {
				 	((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;	
					break;
				 	// return rsp_ptr;
				 }	
				load_srd_shard_base+=1200*256 ; //mdm ram offset 
				
				// CSFB remote 제거 -> diag_userDataBackUp_data 에서 처리함
				//remote_rpc_srd_cmmand(req_ptr, rsp_ptr);  //userDataBackUpStart() 여기서 ... ram 저장 하도록. .. 
				diag_userDataBackUp_entrySet(req_ptr,rsp_ptr,1);  //write info data ,  after rpc respons include write_sector_counter  remote_rpc_srd_cmmand(req_ptr, rsp_ptr);  //userDataBackUpStart() 여기서 ... ram 저장 하도록. .. 
				write_size= rsp_ptr->rsp_data.write_sector_counter *256;	 //return nv backup counters  

				 if( write_size >0x15000)  //384K = mode ram (300K) + mdm (80K)
				 {
				 	((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;  //hue..
				 	break;
				 }
				  load_srd_kernel_base=kmalloc((size_t)write_size, GFP_KERNEL);
				  memcpy(load_srd_kernel_base,load_srd_shard_base,write_size);	
				  
				 mtd_op_result = lge_write_block(srd_bytes_pos_in_emmc+0x400000+512, load_srd_kernel_base, write_size);  //not sector address > 4M byte offset  

				if(mtd_op_result!= write_size)
        			{
				((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;	
				kfree(load_srd_kernel_base);
				break;
				//return rsp_ptr;
                   
        			}
				kfree(load_srd_kernel_base);*/
				break;
			

			case GET_DOWNLOAD_INFO :
				break;

			case EXTRA_NV_OPERATION :
			#ifdef LG_FW_SRD_EXTRA_NV				
				diag_extraNv_entrySet(req_ptr,rsp_ptr);
			#endif
				break;
				
			case PRL_OPERATION :
			#ifdef LG_FW_SRD_PRL				
				diag_PRL_entrySet(req_ptr,rsp_ptr);
			#endif
				break;
				
			default :
  				rsp_ptr =NULL; //(void *) diagpkt_err_rsp (DIAG_BAD_PARM_F, req_ptr, pkt_len);
				break;
		
		}

	/* Execption*/	
	if (rsp_ptr == NULL){
		return NULL;
	}

  return rsp_ptr;
}
EXPORT_SYMBOL(LGE_Dload_SRD);
#endif 

/*  USAGE
 *  1. If you want to handle at ARM9 side, you have to insert fun_ptr as NULL and mark ARM9_PROCESSOR
 *  2. If you want to handle at ARM11 side , you have to insert fun_ptr as you want and mark AMR11_PROCESSOR.
 */

testmode_user_table_entry_type testmode_mstr_tbl[TESTMODE_MSTR_TBL_SIZE] =
{
    /* sub_command                          fun_ptr                           which procesor*/
    /* 0 ~ 10 */
    {TEST_MODE_VERSION,                     NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_LCD,                         LGF_TestLCD,   					 ARM11_PROCESSOR},
    {TEST_MODE_MOTOR,                       LGF_TestMotor,                    ARM11_PROCESSOR},
    {TEST_MODE_ACOUSTIC,                    LGF_TestAcoustic,                 ARM11_PROCESSOR},
    {TEST_MODE_CAM,                         LGF_TestCam,                      ARM11_PROCESSOR},
    /* 11 ~ 20 */
    /* 21 ~ 30 */
    {TEST_MODE_KEY_TEST,                    LGT_TestModeKeyTest,              ARM11_PROCESSOR},
    {TEST_MODE_EXT_SOCKET_TEST,             LGF_ExternalSocketMemory,         ARM11_PROCESSOR},
	// *s LG_BTUI_DIAGCMD_DUTMODE munho2.lee@lge.com 110915
	/* Original
    {TEST_MODE_BLUETOOTH_TEST,              not_supported_command_handler,    ARM11_PROCESSOR},
	*/
	{TEST_MODE_BLUETOOTH_TEST,				LGF_TestModeBlueTooth,	          ARM11_PROCESSOR},
	// *e LG_BTUI_DIAGCMD_DUTMODE
    {TEST_MODE_BATT_LEVEL_TEST,             LGF_TestModeBattLevel,            ARM11_PROCESSOR},
    {TEST_MODE_MP3_TEST,                    LGF_TestModeMP3,                  ARM11_PROCESSOR},
    /* 31 ~ 40 */
    {TEST_MODE_ACCEL_SENSOR_TEST,   linux_app_handler,    ARM11_PROCESSOR},
    {TEST_MODE_WIFI_TEST,                   LGF_TestModeWLAN,                 ARM11_PROCESSOR},
    {TEST_MODE_MANUAL_TEST_MODE,            NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_FORMAT_MEMORY_TEST,          not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_KEY_DATA_TEST,               linux_app_handler,                ARM11_PROCESSOR},
    /* 41 ~ 50 */
    {TEST_MODE_MEMORY_CAPA_TEST,            LGF_MemoryVolumeCheck,            ARM11_PROCESSOR},
    {TEST_MODE_SLEEP_MODE_TEST,             LGF_TestModeSleepMode,            ARM11_PROCESSOR},
    {TEST_MODE_SPEAKER_PHONE_TEST,          LGF_TestModeSpeakerPhone,         ARM11_PROCESSOR},
    {TEST_MODE_VIRTUAL_SIM_TEST,            LGF_TestModeVirtualSimTest,       ARM11_PROCESSOR},
    {TEST_MODE_PHOTO_SENSER_TEST,           not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_MRD_USB_TEST,                NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_PROXIMITY_SENSOR_TEST,       linux_app_handler,    ARM11_PROCESSOR},
    {TEST_MODE_TEST_SCRIPT_MODE,            LGF_TestScriptItemSet,            ARM11_PROCESSOR},
    {TEST_MODE_FACTORY_RESET_CHECK_TEST,    LGF_TestModeFactoryReset,         ARM11_PROCESSOR},
    /* 51 ~60 */
    {TEST_MODE_VOLUME_TEST,                 LGT_TestModeVolumeLevel,          ARM11_PROCESSOR},
    {TEST_MODE_FIRST_BOOT_COMPLETE_TEST,    LGF_TestModeFBoot,                ARM11_PROCESSOR},
    {TEST_MODE_MAX_CURRENT_CHECK,           NULL,                             ARM9_PROCESSOR},
    /* 61 ~70 */
    {TEST_MODE_CHANGE_RFCALMODE,            NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_SELECT_MIMO_ANT,             NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_LTE_MODE_SELECTION,          not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_LTE_CALL,                    not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_CHANGE_USB_DRIVER,           not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_GET_HKADC_VALUE,             NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_LED_TEST,                    linux_app_handler,                ARM11_PROCESSOR},
    {TEST_MODE_PID_TEST,                    NULL,                             ARM9_PROCESSOR},
    /* 71 ~ 80 */
    {TEST_MODE_SW_VERSION,                  NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_IME_TEST,                    NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_IMPL_TEST,                   NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_SIM_LOCK_TYPE_TEST,          NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_UNLOCK_CODE_TEST,            NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_IDDE_TEST,                   NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_FULL_SIGNATURE_TEST,         NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_NT_CODE_TEST,                NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_SIM_ID_TEST,                 NULL,                             ARM9_PROCESSOR},
    /* 81 ~ 90*/
    {TEST_MODE_CAL_CHECK,                   NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_BLUETOOTH_RW,                NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_SKIP_WELCOM_TEST,            not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_WIFI_MAC_RW,                 LGF_TestModeWiFiMACRW,            ARM11_PROCESSOR},
    /* 91 ~ */
    {TEST_MODE_DB_INTEGRITY_CHECK,          LGF_TestModeDBIntegrityCheck,     ARM11_PROCESSOR},
    {TEST_MODE_NVCRC_CHECK,                 NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_RESET_PRODUCTION,            NULL,                             ARM9_PROCESSOR},

// LGE_UPDATE_FOTA_S M3 bryan.oh@lge.com 2011/10/18
    {TEST_MODE_FOTA_ID_CHECK,               LGF_TestModeFotaIDCheck,      ARM11_PROCESSOR},
// LGE_UPDATE_FOTA_E M3 bryan.oh@lge.com 2011/10/18
		

    {TEST_MODE_XO_CAL_DATA_COPY,            NULL,                             ARM9_PROCESSOR}
};
