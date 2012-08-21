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

//[[ NFCDiag wongab.jeon@lge.com
#define NFC_RESULT_PATH 	"/data/nfc/nfc_testmode_result"
//]] NFCDiag

#if 0 // M3 use Internal SD, not External SD
// m3 use Internal SD, so we dont use this
#else
#define SYS_GPIO_SD_DET 40
#endif

#define EXTERNALSD_MOUNT_POINT "/mnt/sdcard/external_sd"
#define EXTERNALSD_TEST_FILE "/mnt/sdcard/external_sd/SDTest.txt"
#define LED_BUTTON_BACKLIGHT "/sys/class/leds/button-backlight/brightness"

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
static int disable_check=0;
//extern void disable_touch_key(int);

//[[ NFCDiag wongab.jeon@lge.com
void* LGF_TestModeNFC( test_mode_req_type*	pReq, DIAG_TEST_MODE_F_rsp_type	*pRsp);
//]] NFCDiag
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-02-07 support test mode 8.9*/
#define BUF_LEN 125
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-02-07 support test mode 8.9*/
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
/* LGE_CHANGE_S : PCB_Revision
 * 2012-04-04, jikhwan.jeong@lge.com
 * [M4][PCB_Revision][Common] Modify PCB Revision.
 */
    char *rev_str[] = {"evb", "A", "B", "C", "D",
        "E", "F", "G", "1.0", "1.1", "1.2", "1.3",
        "revserved"};
/* LGE_CHANGE_E : PCB_Revision */

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
#if 1 //def CONFIG_LGE_DIAG_DISABLE_INPUT_DEVICES_ON_SLEEP_MODE
        case TEST_MODE_SLEEP_MODE_TEST:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_sleep_mode_type);
            break;
#endif

/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-01-19 support test mode 8.9*/
#if 0
	       case TEST_MODE_ORIENTATION_SENSOR:
       	     	rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(int);
			break;
		case TEST_MODE_ACCEL_SENSOR_TEST:
	            	rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_accel_rsp_type);
			break;
		case TEST_MODE_COMPASS_SENSOR_TEST:
			rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_compass_rsp_type);
			break;
		case TEST_MODE_SENSOR_CALIBRATION_TEST:
			rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(int);
			break;
		case TEST_MODE_PROXIMITY_SENSOR_TEST:
			rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(int);
			break;
		case TEST_MODE_PROXIMITY_MFT_SENSOR_TEST:
			rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(int);
			break;
#endif			
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-01-19 support test mode 8.9*/
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

    if ( (fd = sys_open((const char __user *) EXTERNALSD_TEST_FILE, O_CREAT | O_RDWR, 0) ) < 0 )
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
    sys_unlink((const char __user *)EXTERNALSD_TEST_FILE);

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
            if (sys_statfs(EXTERNALSD_MOUNT_POINT, (struct statfs *)&sf) != 0)
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

void* LGF_TestModeLED(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{

    char *src = (void *)0;
    int fd;
	DIAG_TEST_MODE_F_req_type req_ptr;
	
	req_ptr.sub_cmd_code =TEST_MODE_LED_TEST;
	pRsp->ret_stat_code = TEST_OK_S;
	req_ptr.test_mode_req.led=pReq->led;
	
	if ( (fd = sys_open((const char __user *) LED_BUTTON_BACKLIGHT, O_CREAT | O_RDWR, 0777) ) < 0 )
		{
			printk(KERN_ERR "[Testmode LED Test] Can not open  LED sysfs\n");
			goto file_fail;
		}
	 
	if (!(src =kmalloc(1, GFP_KERNEL))) {
		printk(KERN_ERR "[Testmode LED Test]Fail to allocate addr space\n");
		goto file_fail;
	}
	
	switch(req_ptr.test_mode_req.led){
		case 0:
			sprintf(src,"0");
			if ((sys_write(fd, (const char __user *) src, 1)) < 0)
			{
				printk(KERN_ERR "[Testmode LED Test] Can not write LED sysfs \n");
				goto file_fail;
			}
			break;
		case 1:	
			sprintf(src,"255");
			if ((sys_write(fd, (const char __user *) src, 1)) < 0)
			{
				printk(KERN_ERR "[Testmode LED Test] Can not write LED sysfs \n");
				goto file_fail;
			}
			break;
		default:
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            break;
	}	

    kfree(src);

file_fail:
    sys_close(fd);
    sys_unlink((const char __user *)LED_BUTTON_BACKLIGHT);
	
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

extern void LGF_SendKey(word keycode);
extern void set_operation_mode(boolean info);
extern int lm3530_get_state(void);

/* LGE_CHANGE_S : flight test mode
 * 2012-02-14, hyo.park@lge.com, 
 * modification for flight test mode
*/
static boolean testMode_sleepMode = FALSE;
boolean LGF_TestMode_Is_SleepMode(void)
{
	return testMode_sleepMode;
}
EXPORT_SYMBOL(LGF_TestMode_Is_SleepMode);
/* LGE_CHANGE_E : flight test mode */

void* LGF_TestModeSleepMode(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
#if 1
	DIAG_TEST_MODE_F_req_type req_ptr;

  	req_ptr.sub_cmd_code = TEST_MODE_SLEEP_MODE_TEST;

	pRsp->ret_stat_code = TEST_OK_S;
	req_ptr.test_mode_req.sleep_mode = (pReq->sleep_mode & 0x00FF); 	// 2011.06.21 biglake for power test after cal

	switch(req_ptr.test_mode_req.sleep_mode){	
		case SLEEP_MODE_ON:
            if(lm3530_get_state() == 1 /* NORMAL_STATE */)
			    LGF_SendKey(KEY_POWER);
            
            //send_to_arm9((void*)&req_ptr, (void*)pRsp);
            /* LGE_CHANGE_S : flight test mode
			 * 2012-02-14, hyo.park@lge.com, 
			 * modification for flight test mode
			*/
            testMode_sleepMode = FALSE;
            /* LGE_CHANGE_E : flight test mode */
			break;            
		case FLIGHT_MODE_ON:
	  	case FLIGHT_KERNEL_MODE_ON:                        
            if(lm3530_get_state() == 2 /* SLEEP_STATE */)
			    LGF_SendKey(KEY_POWER);
            
            set_operation_mode(FALSE);
            /* LGE_CHANGE_S : flight test mode
			 * 2012-02-14, hyo.park@lge.com, 
			 * modification for flight test mode
			*/
			testMode_sleepMode = TRUE;
			/* LGE_CHANGE_E : flight test mode */
            //send_to_arm9((void*)&req_ptr, (void*)pRsp);
			break;

		case FLIGHT_MODE_OFF:            
            if(lm3530_get_state() == 2 /* SLEEP_STATE */)
			    LGF_SendKey(KEY_POWER);
            
            set_operation_mode(TRUE);
            /* LGE_CHANGE_S : flight test mode
			 * 2012-02-14, hyo.park@lge.com, 
			 * modification for flight test mode
			*/
            testMode_sleepMode = FALSE;
            /* LGE_CHANGE_E : flight test mode */
            //send_to_arm9((void*)&req_ptr, (void*)pRsp);           
			break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			/* LGE_CHANGE_S : flight test mode
			 * 2012-02-14, hyo.park@lge.com, 
			 * modification for flight test mode
			*/
            testMode_sleepMode = FALSE;
            /* LGE_CHANGE_E : flight test mode */
            break;
	}
    #if 0
	switch(pReq->sleep_mode){	
		case SLEEP_MODE_ON:
			LGF_SendKey(KEY_POWER);
			break;		
		case FLIGHT_MODE_ON:
			LGF_SendKey(KEY_POWER);
			//if_condition_is_on_air_plain_mode = 1;
			set_operation_mode(FALSE);
			break;
	  	case FLIGHT_KERNEL_MODE_ON:
			break;
		case FLIGHT_MODE_OFF:
			set_operation_mode(TRUE);
			break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            break;
	}
    #endif

	return pRsp;
#else
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

        case FLIGHT_MODE_ON:
            break;            
        case FLIGHT_KERNEL_MODE_ON:
            break;
        case FLIGHT_MODE_OFF:
            break;

        default:
            break;
    }
#endif

    return pRsp;
#endif
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
//LGE_CHANGE_S, moon-wifi@lge.com by wonho.ki 2012-05-03, "Patch for FM Radio Test : delay 10sec -> 5sec"
	static int first_on_try = 5;
//LGE_CHANGE_E, moon-wifi@lge.com by wonho.ki 2012-05-03, "Patch for FM Radio Test : delay 10sec -> 5sec"
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
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-01-19 support test mode 8.9*/
#if 0
extern void bma222_diag_reult_get(float* x, float* y, float* z);
typedef unsigned long int float32;
typedef unsigned long long float64;
typedef unsigned int bits32;
typedef signed int sbits32;
typedef unsigned short int bits16;
typedef unsigned long long int bits64;
typedef signed char int8;
typedef char flag;
typedef int int16;
typedef signed int int32;

struct roundingData {
    int8 mode;
    int8 precision;
    signed char exception;
};

enum {
    float_round_nearest_even = 0,
    float_round_to_zero      = 1,
    float_round_down         = 2,
    float_round_up           = 3
};

enum {
    float_flag_invalid   =  1,
    float_flag_divbyzero =  2,
    float_flag_overflow  =  4,
    float_flag_underflow =  8,
    float_flag_inexact   = 16
};

enum {
    float_tininess_after_rounding  = 0,
    float_tininess_before_rounding = 1
};

float32 packFloat32(flag zSign, int16 zExp, bits32 zSig)
{
	return (((bits32) zSign) << 31) + (((bits32) zExp) << 23) + zSig;
}

void shift32RightJamming(bits32 a, int16 count, bits32 * zPtr)
{
	bits32 z;
	if (count == 0) {
		z = a;
	} else if (count < 32) {
		z = (a >> count) | ((a << ((-count) & 31)) != 0);
	} else {
		z = (a != 0);
	}
	*zPtr = z;
}

static float32 roundAndPackFloat32( struct roundingData *roundData, flag zSign, int16 zExp, bits32 zSig )
{
    int8 roundingMode;
    flag roundNearestEven;
    int8 roundIncrement, roundBits;
    flag isTiny;

    roundingMode = roundData->mode;
    roundNearestEven = ( roundingMode == float_round_nearest_even );
    roundIncrement = 0x40;
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            roundIncrement = 0;
        }
        else {
            roundIncrement = 0x7F;
            if ( zSign ) {
                if ( roundingMode == float_round_up ) roundIncrement = 0;
            }
            else {
                if ( roundingMode == float_round_down ) roundIncrement = 0;
            }
        }
    }
    roundBits = zSig & 0x7F;
    if ( 0xFD <= (bits16) zExp ) {
        if (    ( 0xFD < zExp )
             || (    ( zExp == 0xFD )
                  && ( (sbits32) ( zSig + roundIncrement ) < 0 ) )
           ) {
            roundData->exception |= float_flag_overflow | float_flag_inexact;
            return packFloat32( zSign, 0xFF, 0 ) - ( roundIncrement == 0 );
        }
        if ( zExp < 0 ) {
            isTiny = ( zExp < -1 ) || ( zSig + roundIncrement < 0x80000000 );
            shift32RightJamming( zSig, - zExp, &zSig );
            zExp = 0;
            roundBits = zSig & 0x7F;
            if ( isTiny && roundBits ) roundData->exception |= float_flag_underflow;
        }
    }
    if ( roundBits ) roundData->exception |= float_flag_inexact;
    zSig = ( zSig + roundIncrement )>>7;
    zSig &= ~ ( ( ( roundBits ^ 0x40 ) == 0 ) & roundNearestEven );
    if ( zSig == 0 ) zExp = 0;
    return packFloat32( zSign, zExp, zSig );

}

static int8 countLeadingZeros32(bits32 a)
{
	static const int8 countLeadingZerosHigh[] = {
		8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	int8 shiftCount;

	shiftCount = 0;
	if (a < 0x10000) {
		shiftCount += 16;
		a <<= 16;
	}
	if (a < 0x1000000) {
		shiftCount += 8;
		a <<= 8;
	}
	shiftCount += countLeadingZerosHigh[a >> 24];
	return shiftCount;

}

float32 normalizeRoundAndPackFloat32( struct roundingData *roundData, flag zSign, int16 zExp, bits32 zSig )
{
    int8 shiftCount;

    shiftCount = countLeadingZeros32( zSig ) - 1;
    return roundAndPackFloat32( roundData, zSign, zExp - shiftCount, zSig<<shiftCount );

}

float32 int32_to_float32(struct roundingData *roundData, int32 a)
{
    flag zSign;

    if ( a == 0 ) return 0;
    if ( a == 0x80000000 ) return packFloat32( 1, 0x9E, 0 );
    zSign = ( a < 0 );
    return normalizeRoundAndPackFloat32( roundData, zSign, 0x9C, zSign);

}
static float32 roundAndPackFloat32(flag zSign, int16 zExp, bits32 zSig)
{
	flag roundNearestEven;
	int8 roundIncrement, roundBits;
	flag isTiny;

	/* SH4 has only 2 rounding modes - round to nearest and round to zero */
	roundNearestEven = (float_rounding_mode() == FPSCR_RM_NEAREST);
	roundIncrement = 0x40;
	if (!roundNearestEven) {
		roundIncrement = 0;
	}
	roundBits = zSig & 0x7F;
	if (0xFD <= (bits16) zExp) {
		if ((0xFD < zExp)
		    || ((zExp == 0xFD)
			&& ((sbits32) (zSig + roundIncrement) < 0))
		    ) {
			float_raise(FPSCR_CAUSE_OVERFLOW | FPSCR_CAUSE_INEXACT);
			return packFloat32(zSign, 0xFF,
					   0) - (roundIncrement == 0);
		}
		if (zExp < 0) {
			isTiny = (zExp < -1)
			    || (zSig + roundIncrement < 0x80000000);
			shift32RightJamming(zSig, -zExp, &zSig);
			zExp = 0;
			roundBits = zSig & 0x7F;
			if (isTiny && roundBits)
				float_raise(FPSCR_CAUSE_UNDERFLOW);
		}
	}
	if (roundBits)
		float_raise(FPSCR_CAUSE_INEXACT);
	zSig = (zSig + roundIncrement) >> 7;
	zSig &= ~(((roundBits ^ 0x40) == 0) & roundNearestEven);
	if (zSig == 0)
		zExp = 0;
	return packFloat32(zSign, zExp, zSig);

}
void* LGF_TestModeAccel(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int destfile;
	const char* src = "/data/misc/diag_sensor_result";
	char buf[BUF_LEN];
	int cnt,x,y,z;
	u64 xx=0,yy=0,zz=0;
	DIAG_TEST_MODE_F_req_type req_ptr;

	mm_segment_t old_fs=get_fs();
	set_fs(get_ds());

	memset(&pRsp->test_mode_rsp.accel, 0x0, sizeof(pRsp->test_mode_rsp.accel));

	req_ptr.sub_cmd_code = TEST_MODE_ACCEL_SENSOR_TEST;
	req_ptr.test_mode_req.accel = pReq->accel;
	pRsp->ret_stat_code = TEST_FAIL_S;

	printk(KERN_ERR "\n[%s]", __func__ );


	destfile = sys_open((const char __user *)src, O_CREAT | O_RDONLY, 0666) ;
	if(destfile<0){
		printk(KERN_ERR "\n [Testmode Accel ] sys_open() failed. cannot create\n");
		goto file_fail;
	}

	if (diagpdev != NULL){
		printk(KERN_ERR "\n[%s] ACCEL test start:%d", __func__ ,pReq->accel);
		update_diagcmd_state(diagpdev, "ACCEL_TEST_MODE", pReq->accel);
		pRsp->ret_stat_code = TEST_OK_S;

	}else{
		printk(KERN_ERR "\n[%s] error ACCEL", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		goto file_fail;
	}
	switch(pReq->accel){
		case ACCEL_SENSOR_OFF:
		case ACCEL_SENSOR_ON:
			pRsp->ret_stat_code = TEST_OK_S;
			break;
		case ACCEL_SENSOR_SENSORDATA:
			msleep(800);		//result file is read after application write .
			//buf = kmalloc(BUF_LEN, GFP_KERNEL);
			memset(&buf,0x0,BUF_LEN);
			if ((sys_read(destfile, buf, BUF_LEN -1)) < 0)
			{
				printk(KERN_ERR "[%s] Can not read file.\n", __func__ );
				pRsp->ret_stat_code = TEST_FAIL_S;
				goto file_fail;
			}else{
				printk(KERN_ERR "[%s] sys_read:%s \n", __func__,buf );
			}
			if(strlen(buf)>0){
				float64 temp;
				sscanf(buf,"%d %d %d %d", &cnt,&x,&y,&z);
				printk(KERN_ERR "[%s] x:%d(%d)\n", __func__,x,(x%100000));
				printk(KERN_ERR "[%s] y:%d(%d)\n", __func__,y,(y%100000));
				printk(KERN_ERR "[%s] z:%d(%d)\n", __func__,z,(z%100000));
				memcpy(&xx,&x,sizeof(x));
				memcpy(&yy,&y,sizeof(y));
				memcpy(&zz,&z,sizeof(z));	


				//temp = int32_to_float32(&roundData,x);
				if(x<0)
				{
					flag zSign = -1;
					int16 zExp = x - (x%100000);
					bits32 zSig = x%100000;
					temp = roundAndPackFloat32( zSign,  zExp,  zSig);
				}

				

				pRsp->test_mode_rsp.accel.count=cnt;
				pRsp->test_mode_rsp.accel.x_data=temp;		//do_div(xx,100000);
				pRsp->test_mode_rsp.accel.y_data=do_div(yy,100000);
				pRsp->test_mode_rsp.accel.z_data=do_div(zz,100000);

				if(pRsp->ret_stat_code != TEST_OK_S)
					pRsp->ret_stat_code = TEST_FAIL_S;
				else
					pRsp->ret_stat_code = TEST_OK_S;
			}else
				pRsp->ret_stat_code = TEST_FAIL_S;
			//kfree(buf);
			break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			break;
	}
		
file_fail:
	sys_close(destfile);
	set_fs(old_fs);
	sys_unlink((const char __user *)src);
	return pRsp;
}
#else
void* LGF_TestModeAccel(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int destfile;
	const char* src = "/data/misc/diag_sensor_result";
	char buf[BUF_LEN];
	mm_segment_t old_fs;
	DIAG_TEST_MODE_F_req_type req_ptr;
	
	memset(&pRsp->test_mode_rsp.accel, 0x0, sizeof(pRsp->test_mode_rsp.accel));
	memset(&buf,0x0,sizeof(buf));
	memset(&req_ptr.test_mode_req.sensor_data,0x0,sizeof(req_ptr.test_mode_req.sensor_data));

	req_ptr.sub_cmd_code = TEST_MODE_ACCEL_SENSOR_TEST;
	req_ptr.test_mode_req.accel = pReq->accel;
	pRsp->ret_stat_code = TEST_FAIL_S;

	printk(KERN_ERR "\n[%s]", __func__ );

	old_fs=get_fs();
	set_fs(get_ds());

	destfile = sys_open((const char __user *)src, O_CREAT | O_RDONLY, 0666) ;
	if(destfile<0){
		printk(KERN_ERR "\n [Testmode Accel ] sys_open() failed. cannot create %s\n",src);
		goto file_fail;
	}

	if (diagpdev != NULL){
		printk(KERN_ERR "\n[%s] ACCEL test start:%d", __func__ ,pReq->accel);
		update_diagcmd_state(diagpdev, "ACCEL_TEST_MODE", pReq->accel);
		pRsp->ret_stat_code = TEST_OK_S;

	}else{
		printk(KERN_ERR "\n[%s] error ACCEL", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		goto file_fail;
	}
	switch(pReq->accel){
		case ACCEL_SENSOR_OFF:
		case ACCEL_SENSOR_ON:
			pRsp->ret_stat_code = TEST_OK_S;
			break;
		case ACCEL_SENSOR_SENSORDATA:
			msleep(800);		//result file is read after application write .
			memset(&buf,0x0,BUF_LEN);
			if ((sys_read(destfile, buf, BUF_LEN -1)) < 0)
			{
				printk(KERN_ERR "[%s] Can not read file.\n", __func__ );
				pRsp->ret_stat_code = TEST_FAIL_S;
				goto file_fail;
			}else{
				printk(KERN_ERR "[%s] sys_read:%s \n", __func__,buf );
			}
			strncpy(req_ptr.test_mode_req.sensor_data,buf,strlen(buf));
			printk(KERN_ERR "[%s] sensor_data:%s (len:%d)\n", __func__,req_ptr.test_mode_req.sensor_data,strlen(buf) );
			if(strlen(buf)>0){
				send_to_arm9((void*)&req_ptr, (void*)pRsp);

				if(pRsp->ret_stat_code != TEST_OK_S)
					pRsp->ret_stat_code = TEST_FAIL_S;
				else
					pRsp->ret_stat_code = TEST_OK_S;
			}else
				pRsp->ret_stat_code = TEST_FAIL_S;
			break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			break;
	}
		
file_fail:
	sys_close(destfile);
	set_fs(old_fs);
	sys_unlink((const char __user *)src);
	return pRsp;
}
#endif

void* LGF_TestModeProximity(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int p_e,p_s,pw_s;

	char buf;
	char pw_buf[4];
	const char* prox_enable =  "/sys/devices/platform/i2c-gpio.4/i2c-4/4-0044/enable";
	const char* prox_show= "/sys/devices/platform/i2c-gpio.4/i2c-4/4-0044/show";
	const char* pw_state = "/sys/power/state";

	p_e = sys_open((const char __user *)prox_enable, O_RDWR, 0666) ;
	p_s = sys_open((const char __user *)prox_show, O_RDWR, 0666) ;
	pw_s= sys_open((const char __user *)pw_state, O_RDWR, 0666) ;

	if(p_e<0||p_s<0||pw_s<0){
		printk(KERN_ERR "\n [Testmode Proximity2] sys_open() failed.");
		if(p_e<0)
			printk(KERN_ERR "\n [Testmode Proximity2] sys_open() failed.: p_e");
		if(p_s<0)
			printk(KERN_ERR "\n [Testmode Proximity2] sys_open() failed.: p_s");
		if(pw_s<0)
			printk(KERN_ERR "\n [Testmode Proximity2] sys_open() failed.: pw_s");
		goto file_fail;
	}

	pRsp->test_mode_rsp.proximity= 0;
	pRsp->ret_stat_code = TEST_FAIL_S;
	
	printk(KERN_ERR "\n[%s]", __func__ );
	switch(pReq->proximity){
		case PROXI_SENSOR_START:
			sys_write(p_e, "1", 1);
			sys_read(pw_s, pw_buf, 2);
			sys_read(p_s, &buf, 1);
			if(strncmp(&buf,"1",1)==0){
				/*proximity sensor :FAR*/
				pRsp->test_mode_rsp.proximity=1;
				sys_write(pw_s, "on", 2);
			}else{
				/*proximity sensor :NEAR*/
				pRsp->test_mode_rsp.proximity=0;
				sys_write(pw_s, "mem", 3);
			}

			pRsp->ret_stat_code = TEST_OK_S;
			break;
		case PROXI_SENSOR_STOP:
			sys_write(p_e, "0", 1);
			sys_write(pw_s, "on", 2);
			pRsp->ret_stat_code = TEST_OK_S;
			break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			break;
	}
file_fail:
	sys_close(p_e);
	sys_close(p_s);
	sys_close(pw_s);	
	sys_unlink((const char __user *)prox_enable);
	sys_unlink((const char __user *)prox_show);
	sys_unlink((const char __user *)pw_state);
	return pRsp;
}
void* LGF_TestModeProximityMFT(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int p_e,p_s;

	char buf;
	const char* prox_enable =  "/sys/devices/platform/i2c-gpio.4/i2c-4/4-0044/enable";
	const char* prox_show= "/sys/devices/platform/i2c-gpio.4/i2c-4/4-0044/show";

	p_e = sys_open((const char __user *)prox_enable, O_RDWR, 0666) ;
	p_s = sys_open((const char __user *)prox_show, O_RDWR, 0666) ;

	if(p_e<0||p_s<0){
		printk(KERN_ERR "\n [Testmode ProximityMFT] sys_open() failed.");
		goto file_fail;
	}

	pRsp->test_mode_rsp.proximity= 0;
	pRsp->ret_stat_code = TEST_FAIL_S;
	
	printk(KERN_ERR "\n[%s]", __func__ );
	switch(pReq->proximity){
		case PROXI_SENSOR_ON:
			sys_write(p_e, "1", 1);
			pRsp->ret_stat_code = TEST_OK_S;
			break;
		case PROXI_SENSOR_OFF:
			sys_write(p_e, "0", 1);
			pRsp->ret_stat_code = TEST_OK_S;
			break;
		case PROXI_SENSOR_SENSORDATA:
			sys_read(p_s, &buf, 1);
			if(strncmp(&buf,"1",1)==0){
				/*proximity sensor :FAR*/
				pRsp->test_mode_rsp.proximity=1;
			}else{
				/*proximity sensor :NEAR*/
				pRsp->test_mode_rsp.proximity=0;
			}
			pRsp->ret_stat_code = TEST_OK_S;
			break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			break;
	}
file_fail:
	sys_close(p_e);
	sys_close(p_s);
	sys_unlink((const char __user *)prox_enable);
	sys_unlink((const char __user *)prox_show);
	return pRsp;
}
void* LGF_TestModeAccelCal(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int f_x,f_y,f_z,e_w,c_r,enable,mode,enable_flag=0;

	char buf_enable,buf_mode,buf_result;
	const char* fast_x =  "/sys/class/input/input4/fast_calibration_x";
	const char* fast_y =  "/sys/class/input/input4/fast_calibration_y";
	const char* fast_z =  "/sys/class/input/input4/fast_calibration_z";
	const char* eeprom_writing =  "/sys/class/input/input4/eeprom_writing";
	const char* sensor_enable =  "/sys/class/input/input4/enable";
	const char* sensor_mode =  "/sys/class/input/input4/mode";
	const char* cal_result =  "/sys/class/input/input4/cal_result";

	f_x = sys_open((const char __user *)fast_x, O_RDWR, 0666) ;
	f_y = sys_open((const char __user *)fast_y, O_RDWR, 0666) ;
	f_z= sys_open((const char __user *)fast_z, O_RDWR, 0666) ;
	e_w= sys_open((const char __user *)eeprom_writing, O_RDWR, 0666) ;
	enable= sys_open((const char __user *)sensor_enable, O_RDWR, 0666) ;
	mode= sys_open((const char __user *)sensor_mode, O_RDWR, 0666) ;
	c_r= sys_open((const char __user *)cal_result, O_RDWR, 0666) ;

	if(f_x<0||f_y<0||f_z<0||e_w<0||enable<0||c_r<0){
		printk(KERN_ERR "\n [Testmode LGF_TestModeAccelCal] sys_open() failed.");
		goto file_fail;
	}
	sys_read(enable, &buf_enable, 1);
	if(strncmp(&buf_enable,"0",1)==0)
	{
		sys_write(enable, "1", 1);
		enable_flag = 1;
	}
	sys_read(mode, &buf_mode, 1);
	if(strncmp(&buf_mode,"2",1)==0)
	{
		sys_write(mode, "0", 1);
		
	}
	pRsp->test_mode_rsp.accel_calibration= 0;
	pRsp->ret_stat_code = TEST_FAIL_S;
	
	printk(KERN_ERR "\n[%s] lge_bd_rev:%d", __func__ ,lge_bd_rev);
	switch(pReq->sensor_calibration){
		case SENSOR_CALIBRATION_START:
			if (lge_bd_rev >= LGE_REV_10) {
				if(sys_write(f_x, "0", 1)>0 &&sys_write(f_y, "0", 1)>0&&sys_write(f_z, "2", 1)>0)
				{
					sys_write(e_w, "1", 1);
				}else{
					printk(KERN_ERR "\n[%s] calibration fail.", __func__ );
				}
			}else{

				if(sys_write(f_x, "0", 1)>0 &&sys_write(f_y, "0", 1)>0&&sys_write(f_z, "1", 1)>0)
				{
					sys_write(e_w, "1", 1);
				}else{
					printk(KERN_ERR "\n[%s] calibration fail.", __func__ );
				}
			}
			pRsp->ret_stat_code = TEST_OK_S;
			break;
		case SENSOR_CALIBRATION_RESULT:
			sys_read(c_r, &buf_result, 1);
			if(strncmp(&buf_result,"1",1)==0){
				pRsp->test_mode_rsp.accel_calibration=1;
				pRsp->ret_stat_code = TEST_OK_S;
			}else{
				pRsp->test_mode_rsp.accel_calibration=0;
				pRsp->ret_stat_code = TEST_FAIL_S;
			}
			break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			break;
	}
file_fail:
	sys_close(f_x);
	sys_close(f_y);
	sys_close(f_z);	
	sys_close(e_w);	
	sys_close(enable);	
	sys_close(mode);	
	sys_close(c_r);	
	sys_unlink((const char __user *)fast_x);
	sys_unlink((const char __user *)fast_y);
	sys_unlink((const char __user *)fast_z);
	sys_unlink((const char __user *)eeprom_writing);
	sys_unlink((const char __user *)sensor_enable);
	sys_unlink((const char __user *)sensor_mode);
	sys_unlink((const char __user *)cal_result);
	return pRsp;
}
void* LGF_TestModeOrientatioin(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int destfile;
	const char* src = "/data/misc/diag_sensor_result";
	char buf [256];

	mm_segment_t old_fs=get_fs();
	set_fs(get_ds());
	memset(&buf,0x0,sizeof(buf));

	destfile = sys_open((const char __user *)src, O_CREAT | O_RDWR, 0666) ;
	if(destfile<0){
		printk(KERN_ERR "\n [Testmode Orientation ] sys_open() failed. cannot create %s",src);
		goto file_fail;
	}

	pRsp->test_mode_rsp.orientation = 0;
	pRsp->ret_stat_code = TEST_FAIL_S;
	
	printk(KERN_ERR "\n[%s]", __func__ );

	if (diagpdev != NULL){
		printk(KERN_ERR "\n[%s] ORIENTATION test start:%d", __func__ ,pReq->orientation);
		update_diagcmd_state(diagpdev, "ORIENTATION_TEST_MODE", pReq->orientation);
		pRsp->ret_stat_code = TEST_OK_S;

	}else{
		printk(KERN_ERR "\n[%s] error ORIENTATION", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		goto file_fail;
	}
	msleep(1500);		//result file is read after application write .
	if ((sys_read(destfile,buf, sizeof(buf)-1)) < 0)
	{
		printk(KERN_ERR "[%s] Can not read file.\n", __func__ );
		pRsp->ret_stat_code = TEST_FAIL_S;
		goto file_fail;
	}
	printk(KERN_ERR "[%s] sys_read:%s \n", __func__, buf );
	sscanf(buf,"%d", &pRsp->test_mode_rsp.orientation);
file_fail:
	sys_close(destfile);
	set_fs(old_fs);
	sys_unlink((const char __user *)src);
	return pRsp;
}
void* LGF_TestModeCompass(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int destfile;
	const char* src = "/data/misc/diag_sensor_result";
	char buf [256];
	mm_segment_t old_fs;
	DIAG_TEST_MODE_F_req_type req_ptr;
	
	memset(&pRsp->test_mode_rsp.compass, 0x0, sizeof(pRsp->test_mode_rsp.compass));
	memset(&buf,0x0,sizeof(buf));
	memset(&req_ptr.test_mode_req.sensor_data,0x0,sizeof(req_ptr.test_mode_req.sensor_data));

	req_ptr.sub_cmd_code = TEST_MODE_COMPASS_SENSOR_TEST;
	req_ptr.test_mode_req.compass = pReq->compass;
	pRsp->ret_stat_code = TEST_FAIL_S;

	printk(KERN_ERR "\n[%s]", __func__ );

	old_fs=get_fs();
	set_fs(get_ds());

	destfile = sys_open((const char __user *)src, O_CREAT | O_RDONLY, 0666) ;
	if(destfile<0){
		printk(KERN_ERR "\n [Testmode Compass ] sys_open() failed. cannot create %s\n",src);
		goto file_fail;
	}

	if (diagpdev != NULL){
		printk(KERN_ERR "\n[%s] Compass test start:%d", __func__ ,pReq->compass);
		update_diagcmd_state(diagpdev, "COMPASS_TEST_MODE", pReq->compass);
		pRsp->ret_stat_code = TEST_OK_S;

	}else{
		printk(KERN_ERR "\n[%s] error COMPASS", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		goto file_fail;
	}
	switch(pReq->compass){
		case COMPS_SENSOR_OFF:
		case COMPS_SENSOR_ON:
			pRsp->ret_stat_code = TEST_OK_S;
			break;
		case COMPS_SENSOR_SENSORDATA:
			msleep(800);		//result file is read after application write .
			if ((sys_read(destfile, buf, sizeof(buf)-1)) < 0)
			{
				printk(KERN_ERR "[%s] Can not read file.\n", __func__ );
				pRsp->ret_stat_code = TEST_FAIL_S;
				goto file_fail;
			}else{
				printk(KERN_ERR "[%s] sys_read:%s \n", __func__,buf );
			}
			strncpy(req_ptr.test_mode_req.sensor_data,buf,strlen(buf));
			printk(KERN_ERR "[%s] sensor_data:%s (len:%d)\n", __func__,req_ptr.test_mode_req.sensor_data,strlen(buf) );
			if(strlen(buf)>0){
				//send_to_arm9((void*)&req_ptr, (void*)pRsp);
				if(pRsp->ret_stat_code != TEST_OK_S)
					pRsp->ret_stat_code = TEST_FAIL_S;
				else
					pRsp->ret_stat_code = TEST_OK_S;
			}
			else
				pRsp->ret_stat_code = TEST_FAIL_S;
			break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			break;
	}
		
file_fail:
	sys_close(destfile);
	set_fs(old_fs);
	sys_unlink((const char __user *)src);
	return pRsp;
}
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-01-19 support test mode 8.9*/
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
	char temp[32];
	test_mode_req_mft_lcd_type* mft_lcd = &pReq->mft_lcd;
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev == NULL) {
		pr_info("[%s] error LCD\n", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		return pRsp;
	}

	pr_info(" *** pReq->lcd: %d\n", mft_lcd->type_subcmd);
	switch (mft_lcd->type_subcmd)
	{
	case LCD_GET_INFO:
		sprintf(pRsp->test_mode_rsp.lcd_getinfo, "320,480,24");
		break;

	case LCD_DISPLAY_CHART:
		pr_info(" data[] = { %d (%c) }\n", mft_lcd->data_chart, mft_lcd->data_chart);
		sprintf(temp, "LCD,%c", mft_lcd->data_chart);
		update_diagcmd_state(diagpdev, temp, mft_lcd->type_subcmd);
		break;

	case LCD_DISLPAY_PATTERN_CHART:
		pr_info(" data[] = { %d, %d, %d } -> { %c, %c, %c }\n",
			mft_lcd->data_pattern[0], mft_lcd->data_pattern[1], mft_lcd->data_pattern[2],
			mft_lcd->data_pattern[0], mft_lcd->data_pattern[1], mft_lcd->data_pattern[2]);
		sprintf(temp, "LCD,%c,%c,%c",
			mft_lcd->data_pattern[0], mft_lcd->data_pattern[1], mft_lcd->data_pattern[2]);
		update_diagcmd_state(diagpdev, temp, mft_lcd->type_subcmd);
		break;

	case LCD_GET:
		pr_info(" data[] = { %d, %d, %d, %d }\n",
			mft_lcd->data_lcd_get[0], mft_lcd->data_lcd_get[1], mft_lcd->data_lcd_get[2], mft_lcd->data_lcd_get[3]);
		sprintf(temp, "LCD,%d,%d,%d,%d",
			mft_lcd->data_lcd_get[0], mft_lcd->data_lcd_get[1], mft_lcd->data_lcd_get[2], mft_lcd->data_lcd_get[3]);
		update_diagcmd_state(diagpdev, temp, mft_lcd->type_subcmd);
		pRsp->test_mode_rsp.lcd_get = mft_lcd->data_lcd_get[2]*mft_lcd->data_lcd_get[3]*3;
		break;

	case LCD_INITIAL:
	case LCD_ON:
	case LCD_OFF:
		update_diagcmd_state(diagpdev, "LCD", mft_lcd->type_subcmd);
		break;

	default:
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		return pRsp;
	}

	return pRsp;
}

void* LGF_TestLCD_Cal(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	char ptr[30];

	if (diagpdev == NULL) {
		printk("\n[%s] error LCD_cal", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		return pRsp;
	}

	pRsp->ret_stat_code = TEST_OK_S;
	printk("<6>" "pReq->lcd_cal:%d, (%x,%x)\n", pReq->lcd_cal, pReq->MaxRGB[0], pReq->MaxRGB[1]);

	if (pReq->MaxRGB[0] != 5) {
		update_diagcmd_state(diagpdev, "LCD_Cal", pReq->MaxRGB[0]);
	}
	else {
		printk("<6>" "pReq->MaxRGB string type : %s\n",pReq->MaxRGB);
		sprintf(ptr,"LCD_Cal,%s",&pReq->MaxRGB[1]);
		printk("<6>" "%s \n", ptr);
		update_diagcmd_state(diagpdev, ptr, pReq->MaxRGB[0]);
	}

	return pRsp;
}

void* LGF_TestTouchDraw(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	DIAG_TEST_MODE_F_req_type req_ptr;
	
	req_ptr.sub_cmd_code =TEST_MODE_TOUCH_DRAW_FOR_MFT;
	pRsp->ret_stat_code = TEST_OK_S;
	req_ptr.test_mode_req.mft_touch_draw=pReq->mft_touch_draw;
	
	if (diagpdev == NULL) {
		pr_info("[%s] error Touch Draw\n", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		return pRsp;
	}

	
	switch (req_ptr.test_mode_req.mft_touch_draw)
	{
		case 0:
			update_diagcmd_state(diagpdev, "TOUCHDRAW",req_ptr.test_mode_req.mft_touch_draw);
			break;
		case 1:
			update_diagcmd_state(diagpdev, "TOUCHDRAW",req_ptr.test_mode_req.mft_touch_draw);
			break;
		case 2:
			update_diagcmd_state(diagpdev, "TOUCHDRAW",req_ptr.test_mode_req.mft_touch_draw-2);
			break;		
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			return pRsp;
	}

	return pRsp;
}

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
void* LGF_TestModeKeyLockUnlock(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	DIAG_TEST_MODE_F_req_type req_ptr;
	
	req_ptr.sub_cmd_code =TEST_MODE_KEY_LOCK_UNLOCK;
	pRsp->ret_stat_code = TEST_OK_S;
	req_ptr.test_mode_req.key_lock_unlock=pReq->key_lock_unlock;
	
	if (diagpdev == NULL) {
		pr_info("[%s] error Key lock unlock\n", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		return pRsp;
	}
	 
	switch (req_ptr.test_mode_req.key_lock_unlock)
	{
		case 0://key lock
			if(disable_check ==0){
				LGF_TestModeSetDisableInputDevices(1);
				update_diagcmd_state(diagpdev, "KEYLOCK",req_ptr.test_mode_req.key_lock_unlock);
				//disable_touch_key(0);
				disable_check=1;
			}
			break;
		case 1:
			if(disable_check ==1){
				LGF_TestModeSetDisableInputDevices(0);
				update_diagcmd_state(diagpdev, "KEYLOCK",req_ptr.test_mode_req.key_lock_unlock);
				//disable_touch_key(1);
				disable_check=0;
			}
			break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			return pRsp;
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
void* LGF_TestModeKeyMFT(test_mode_req_type* pReq, DIAG_TEST_MODE_F_rsp_type *pRsp)
{
	byte buf_temp[100];
	test_mode_req_keytest_mft* key_test_mft = &pReq->key_test_mft;
	pRsp->ret_stat_code = TEST_OK_S;
		
	if (diagpdev == NULL) {
		pr_info("[%s] Key test error\n", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		return pRsp;
	}
	pr_info(" *** pReq->key_test_mft : %d\n",key_test_mft->type_subcmd);
	
	switch (key_test_mft->type_subcmd)
	{
		case 0:
				memset((void*)key_buf,0x00,MAX_KEY_BUFF_SIZE);
				count_key_buf=0;
				diag_event_log_start();
				break;
		case 1:
				pRsp->ret_stat_code = TEST_OK_S;
				break;
		case 2: 
				snprintf(buf_temp, 2*(2*key_test_mft->data_key_num+1),"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",key_test_mft->data_key_num,key_buf[0], key_buf[1], key_buf[2],key_buf[3],key_buf[4],key_buf[5]
									,key_buf[6], key_buf[7], key_buf[8],key_buf[9],key_buf[10],key_buf[11],key_buf[12], key_buf[13], key_buf[14]);
				sprintf((void *)((DIAG_TEST_MODE_KEY_F_rsp_type *)pRsp)->key_pressed_buf, (void *)buf_temp);
				memset((void *)key_buf,0x00,key_test_mft->data_key_num);
				diag_event_log_end();
				break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			return pRsp;
	}
  return pRsp;
}

void* LGF_TestMFTCamera(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
//		printk("\n[%s] yschoi CAMERA for MFT cmd[%d]\n", __func__ , pReq->mft_camera);
		update_diagcmd_state(diagpdev, "MFTCAMERA", pReq->mft_camera);
	}
	else
	{
		printk("\n[%s] error CAMERA for MFT\n", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;
}

void* LGF_TestMFTCamcorder(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
//		printk("\n[%s] yschoi CAMCORDER for MFT cmd[%d]\n", __func__ , pReq->mft_camcorder);
		update_diagcmd_state(diagpdev, "MFTCAMCORDER", pReq->mft_camcorder);
	}
	else
	{
		printk("\n[%s] error CAMCORDER for MFT\n", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
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

void* LGF_FmRadioTest (
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	DIAG_TEST_MODE_F_req_type req_ptr;
	req_ptr.sub_cmd_code = TEST_MODE_FMRADIO_TEST;
	req_ptr.test_mode_req.fm_transceiver = pReq->fm_transceiver;

    if (diagpdev == NULL) {
        pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
        return pRsp;
	}

	switch (pReq->fm_transceiver.type_subcmd)
	{
	case 5:
	case 6:
		send_to_arm9((void*) &req_ptr, (void*) pRsp);
		if (pRsp->ret_stat_code != TEST_OK_S)
			return pRsp;

		update_diagcmd_state(diagpdev, "FMTEST", pReq->fm_transceiver.type_subcmd);
		pRsp->ret_stat_code = TEST_OK_S;
		break;

	default:
        pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		break;
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

// LGE_CHANGE_S, myunghwan.kim@lge.com, process ap manual mode
void* LGF_TestModeManualTestMode(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
	DIAG_TEST_MODE_F_req_type req_ptr;

	req_ptr.sub_cmd_code = TEST_MODE_MANUAL_TEST_MODE;
	req_ptr.test_mode_req = *pReq;

	send_to_arm9((void*) &req_ptr, (void*) pRsp);
	if (pRsp->ret_stat_code != TEST_OK_S) {
		pr_info(" *** LGF_TestModeManualTestMode return error \n");
		return pRsp;
	}

	switch (pReq->test_manual_mode) {
	case MANUAL_TEST_ON:	/* lge_set_ap_manual_mode(1); */	break;
	case MANUAL_TEST_OFF:	/* lge_set_ap_manual_mode(0); */	break;
	default:
		break;
	}

	pr_info(" *** call Manualmode item : %d\n", pReq->test_manual_mode);
	return pRsp;
}
// LGE_CHANGE_E, myunghwan.kim@lge.com, process ap manual mode

// emmc interface
int lge_get_frst_flag(void);
int lge_set_frst_flag(int flag);

void* LGF_TestModeFactoryReset(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
	DIAG_TEST_MODE_F_req_type req_ptr;

	req_ptr.sub_cmd_code = TEST_MODE_FACTORY_RESET_CHECK_TEST;
	req_ptr.test_mode_req.factory_reset = pReq->factory_reset;
	pRsp->ret_stat_code = TEST_FAIL_S;

	switch(pReq->factory_reset)
	{
	case FACTORY_RESET_CHECK :
		send_to_arm9((void*)&req_ptr, (void*)pRsp);
		pr_info("[Testmode]send_to_arm9 response : %d\n", pRsp->ret_stat_code);

		if (pRsp->ret_stat_code == TEST_OK_S)
			lge_set_frst_flag(2);
		break;

	case FACTORY_RESET_COMPLETE_CHECK:
		send_to_arm9((void*)&req_ptr, (void*)pRsp);
		pr_info("[Testmode]send_to_arm9 response : %d\n", pRsp->ret_stat_code);
		break;

	case FACTORY_RESET_STATUS_CHECK:
		pRsp->ret_stat_code = lge_get_frst_flag();
        if (pRsp->ret_stat_code == 4)
            pRsp->ret_stat_code = 3;
		break;

	case FACTORY_RESET_COLD_BOOT:
		lge_set_frst_flag(3);
		pRsp->ret_stat_code = TEST_OK_S;
		break;

	default:
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		break;
	}

	return pRsp;
}
//[LGE_UPDATE_S] 20120215 minwoo.jung CP frst for HiddenMenu
int HiddenMenu_FactoryReset(void)
{
	DIAG_TEST_MODE_F_req_type req_ptr;
	DIAG_TEST_MODE_F_rsp_type rsp_ptr; 

	//NV_INIT
	req_ptr.sub_cmd_code = TEST_MODE_FACTORY_RESET_CHECK_TEST;
	req_ptr.test_mode_req.factory_reset = FACTORY_RESET_CHECK;
	send_to_arm9((void*)&req_ptr, (void*)&rsp_ptr);
	
    if (rsp_ptr.ret_stat_code != TEST_OK_S)
		return -1;

	lge_set_frst_flag(2);
	req_ptr.test_mode_req.factory_reset = FACTORY_RESET_COMPLETE_CHECK;
	send_to_arm9((void*)&req_ptr, (void*)&rsp_ptr);	

    if (rsp_ptr.ret_stat_code != TEST_OK_S)
		return -1;

    //COLD_BOOT		
	lge_set_frst_flag(3);
	return 0;
}
//[LGE_UPDATE_E] 20120215 minwoo.jung CP frst for HiddenMenu

void* LGF_TestScriptItemSet(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
	DIAG_TEST_MODE_F_req_type req_ptr;
	udbp_req_type udbReqType;
	memset(&udbReqType,0x0,sizeof(udbp_req_type));

	req_ptr.sub_cmd_code = TEST_MODE_TEST_SCRIPT_MODE;
	req_ptr.test_mode_req.test_mode_test_scr_mode = pReq->test_mode_test_scr_mode;

	switch(pReq->test_mode_test_scr_mode)
	{
	case TEST_SCRIPT_ITEM_SET:
		send_to_arm9((void*)&req_ptr, (void*)pRsp);
		pr_info("%s, result : %s\n", __func__, pRsp->ret_stat_code==TEST_OK_S ? "OK" : "FALSE");

		if (pRsp->ret_stat_code == TEST_OK_S)
			lge_set_frst_flag(0);
		break;

	case TEST_SCRIPT_MODE_CHECK:
		pRsp->test_mode_rsp.test_mode_test_scr_mode = 2;
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		break;

	case CAL_DATA_BACKUP:
		udbReqType.header.sub_cmd = SRD_INIT_OPERATION;
		LGE_Dload_SRD((udbp_req_type *)&udbReqType,sizeof(udbReqType));//SRD_INIT_OPERATION
		udbReqType.header.sub_cmd = USERDATA_BACKUP_REQUEST;
		LGE_Dload_SRD((udbp_req_type *)&udbReqType,sizeof(udbReqType));//USERDATA_BACKUP_REQUEST
		udbReqType.header.sub_cmd = USERDATA_BACKUP_REQUEST_MDM;
		LGE_Dload_SRD((udbp_req_type *)&udbReqType,sizeof(udbReqType));//USERDATA_BACKUP_REQUEST_MDM
		break;

	default:
		send_to_arm9((void*)&req_ptr, (void*)pRsp);
		printk(KERN_INFO "%s, cmd : %d, result : %s\n", __func__, pReq->test_mode_test_scr_mode, \
				pRsp->ret_stat_code==TEST_OK_S?"OK":"FALSE");
		break;
	}  

	return pRsp;
}

//[[ NFCDiag wongab.jeon@lge.com
static int lg_diag_nfc_result_file_read(int i_testcode ,int *ptrRenCode, char *sz_extra_buff, DIAG_TEST_MODE_F_rsp_type *pRsp)
{
	int read;
	int read_size;
	char buf[64] = {0,};
	
	mm_segment_t oldfs;	

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	printk("[NFC] HELLO START FILE READ");
	read = sys_open((const char __user *)NFC_RESULT_PATH, O_RDONLY , 0);

	if(read < 0) {
		printk(KERN_ERR "%s, NFC Result File Open Fail\n",__func__);
		goto nfc_read_err;
		
	}else {
		printk(KERN_ERR "%s, NFC Result File Open Success\n",__func__);
		 
	}

	read_size = 0;
	printk("[_NFC_] copy read to buf variable From read\n");
	while( sys_read(read, &buf[read_size], 1) == 1){
		printk("[_NFC_] READ  buf[%d]:%c \n",read_size,buf[read_size]);
		read_size++;
	}	

	printk("[_NFC_] READ char %d\n",buf[0]-48);
	
	*ptrRenCode = buf[0]-48; //change ASCII Code to int Number
	
	printk("[_NFC_] lg_diag_nfc_result_file_read : i_result_status == %d\n",*ptrRenCode);

	if(strlen(buf) > 1)
	{
		if(buf == NULL)
		{
			printk(KERN_ERR "ADDY.KIM@lge.com : [_NFC_] BUFF is NULL\n");
			goto nfc_read_err;
			
		}
		
		switch(i_testcode)
		{
			case 2:	//swp
				if((buf[1] - 48) == 1){ pRsp->test_mode_rsp.nfc_swp.swio_result = true; pRsp->test_mode_rsp.nfc_swp.pmuvcc_result = true;} 
				else if((buf[1] - 48) == 2){ pRsp->test_mode_rsp.nfc_swp.swio_result = false; pRsp->test_mode_rsp.nfc_swp.pmuvcc_result = true;} 
				else if((buf[1] - 48) == 3){ pRsp->test_mode_rsp.nfc_swp.swio_result = true; pRsp->test_mode_rsp.nfc_swp.pmuvcc_result = false;} 
				else if((buf[1] - 48) == 4){ pRsp->test_mode_rsp.nfc_swp.swio_result = false; pRsp->test_mode_rsp.nfc_swp.pmuvcc_result = false;} 
				break;
			case 3:	//antenna
				pRsp->test_mode_rsp.nfc_ant.loop1 = buf[1];
				pRsp->test_mode_rsp.nfc_ant.loop2 = buf[2];
				pRsp->test_mode_rsp.nfc_ant.loop3 = buf[3];
				break;
			case 5:	//f/w file
			case 7:	//f/w
				printk("[_NFC_] lg_diag_nfc_result_file_read : Start Copy From szExtraData -> buf buf\n");
				strcpy( sz_extra_buff,(char*)(&buf[1]) );
				break;
		}
	}	
		
	//sscanf(buf,"%d",&result);
	
	set_fs(oldfs);
	sys_close(read);

	return 1;

	nfc_read_err:
		set_fs(oldfs);
		sys_close(read);
		return 0;
		
}


void* LGF_TestModeNFC(
		test_mode_req_type*	pReq,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int nfc_result = 0;
	
	char szExtraData[64] = {0,};
//	char szExtra
	printk(KERN_ERR "ADDY.KIM@lge.com : [_NFC_] [%s:%d] SubCmd=<%d>\n", __func__, __LINE__, pReq->nfc);

	if (diagpdev != NULL){ 
		update_diagcmd_state(diagpdev, "NFC_TEST_MODE", pReq->nfc);
		
		if(pReq->nfc == 0)//on
			msleep(3000);
		else if(pReq->nfc == 1)//off
			msleep(5000);
		else if(pReq->nfc == 2)//SWP
			msleep(4000);
		else if(pReq->nfc == 3)//Antenna
			msleep(4000);
		else if(pReq->nfc == 4)//Reader
			msleep(5000);
		else if(pReq->nfc == 5)//f/w file version
			msleep(3000);
		else if(pReq->nfc == 6)//f/w update
			msleep(13000);
		else if(pReq->nfc == 7)//f/w version
			msleep(3000);
		else
			msleep(3000);		
	
		if( (lg_diag_nfc_result_file_read(pReq->nfc,&nfc_result, szExtraData, pRsp)) == 0 ){
			pRsp->ret_stat_code = TEST_FAIL_S;
			printk(KERN_ERR "addy.kim@lge.com , [NFC] FIle Open Error");
			goto nfc_test_err;
		}

		printk(KERN_ERR "ADDY.KIM@lge.com : [_NFC_] [%s:%d] Result Value=<%d>\n", __func__, __LINE__, nfc_result);

		if( nfc_result == 0			//normal OK 
			|| nfc_result == 9 ){	//OK with comment
			pRsp->ret_stat_code = TEST_OK_S;
			printk(KERN_ERR "ADDY.KIM@lge.com : [_NFC_] [%s:%d] DAIG RETURN VALUE=<%s>\n", __func__, __LINE__, "TEST_OK");
		}else if(nfc_result == 1){
			pRsp->ret_stat_code = TEST_FAIL_S;
			printk(KERN_ERR "ADDY.KIM@lge.com : [_NFC_] [%s:%d] DAIG RETURN VALUE=<%s>\n", __func__, __LINE__, "TEST_FAIL");
			goto nfc_test_err;
		}else{
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			printk(KERN_ERR "ADDY.KIM@lge.com : [_NFC_] [%s:%d] DAIG RETURN VALUE=<%s>\n", __func__, __LINE__, "NOT_SUPPORT");
			goto nfc_test_err;
		}

		if(pReq->nfc == 5 	//f/w file version
			|| pReq->nfc == 7 )	//f/w version
		{
			if(szExtraData == NULL ){
				printk(KERN_ERR "[_NFC_] [%s:%d] response Data is NULL \n", __func__, __LINE__);
				pRsp->ret_stat_code = TEST_FAIL_S;
				goto nfc_test_err;
			}

			printk("[_NFC_] Start Copy From szExtraData : [%s] -> test_mode_rsp buf\n",szExtraData);


			//save data to response to Diag
			//strcat(szExtraData,"0");
			strcpy(pRsp->test_mode_rsp.str_buf,(byte*)szExtraData);
	
			printk(KERN_INFO "%s\n", pRsp->test_mode_rsp.str_buf);
			pRsp->ret_stat_code = TEST_OK_S;
		}
	}
	else
	{
		printk(KERN_ERR "[_NFC_] [%s:%d] SubCmd=<%d> ERROR\n", __func__, __LINE__, pReq->nfc);
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;

	nfc_test_err:

		return pRsp;		
}
//]] NFCDiag

// LGE_CHANGE_S, hoseong.kang@lge.com
void* LGF_TestModeVolumeMftTest(
        test_mode_req_type* pReq ,
        DIAG_TEST_MODE_F_rsp_type   *pRsp)
{
	DIAG_TEST_MODE_F_req_type req_ptr;
	req_ptr.sub_cmd_code = TEST_MODE_VOLUME_TEST_FOR_MFT;
	req_ptr.test_mode_req.mft_volume = pReq->mft_volume;

    if (diagpdev == NULL) {
        pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
        return pRsp;
	}

	pr_info(" *** LGF_TestModeVolumeTest: sub(%d), argument(%u)", pReq->mft_volume.type_subcmd, pReq->mft_volume.data_volume);

	switch (pReq->mft_volume.type_subcmd)
	{
	case VOLUME_READ:
	case VOLUME_CHANGE:
		send_to_arm9((void*) &req_ptr, (void*) pRsp);
		break;

	default:
        pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		break;
	}

    return pRsp;
}
// LGE_CHANGE_E, hoseong.kang@lge.com

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

// [LGE_UPDATE_S] minwoo.jung 20120207
void* LGF_TestModeMLTEnableSet(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    char *src = (void *)0;
    char *dest = (void *)0;
    off_t fd_offset;
    int fd;

    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

    pRsp->ret_stat_code = TEST_FAIL_S;

    if (diagpdev != NULL)
    {
        if ( (fd = sys_open((const char __user *) "/mpt/enable", O_CREAT | O_RDWR, 0) ) < 0 )
        {
            printk(KERN_ERR "[Testmode MPT] Can not access MPT\n");
            goto file_fail;
        }
#if 0
		if(pReq->mlt_enable == 2)
		{
			if ( (dest = kmalloc(5, GFP_KERNEL)) )
			{
				if ((sys_read(fd, (char __user *) dest, 2)) < 0)
				{
					printk(KERN_ERR "[Testmode MPT] Can not read MPT \n");
					goto file_fail;
				}

				if ((memcmp("1", dest, 2)) == 0)
				{
					pRsp->test_mode_rsp.mlt_enable = 1;
					pRsp->ret_stat_code = TEST_OK_S;
				}
				else if ((memcmp("0", dest, 2)) == 0)
				{
					pRsp->test_mode_rsp.mlt_enable = 0;
					pRsp->ret_stat_code = TEST_OK_S;
				}
				else
				{
					//pRsp->test_mode_rsp = 1;
					pRsp->ret_stat_code = TEST_FAIL_S;
				}
			}
		}
		else
#endif
		{
			if ( (src = kmalloc(5, GFP_KERNEL)) )
			{
				sprintf(src, "%d", pReq->mlt_enable);
				if ((sys_write(fd, (const char __user *) src, 2)) < 0)
				{
					printk(KERN_ERR "[Testmode MPT] Can not write MPT \n");
					goto file_fail;
				}

				fd_offset = sys_lseek(fd, 0, 0);
			}

			if ( (dest = kmalloc(5, GFP_KERNEL)) )
			{
				if ((sys_read(fd, (char __user *) dest, 2)) < 0)
				{
					printk(KERN_ERR "[Testmode MPT] Can not read MPT \n");
					goto file_fail;
				}

				if ((memcmp(src, dest, 2)) == 0)
					pRsp->ret_stat_code = TEST_OK_S;
				else
					pRsp->ret_stat_code = TEST_FAIL_S;
			}
		}
			
        file_fail:
          kfree(src);
          kfree(dest);
          sys_close(fd);
          set_fs(old_fs);
//          sys_unlink((const char __user *)"/mpt/enable");
    }

    return pRsp;
}

// [LGE_UPDATE_E] minwoo.jung 20120207 

/*  USAGE
 *  1. If you want to handle at ARM9 side, you have to insert fun_ptr as NULL and mark ARM9_PROCESSOR
 *  2. If you want to handle at ARM11 side , you have to insert fun_ptr as you want and mark AMR11_PROCESSOR.
 */

testmode_user_table_entry_type testmode_mstr_tbl[TESTMODE_MSTR_TBL_SIZE] =
{
    /* sub_command                          fun_ptr                           which procesor*/
    /* 0 ~ 10 */
    {TEST_MODE_VERSION,                     NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_LCD_CAL,                     LGF_TestLCD_Cal,                  ARM11_PROCESSOR},
    {TEST_MODE_MOTOR,                       LGF_TestMotor,                    ARM11_PROCESSOR},
    {TEST_MODE_ACOUSTIC,                    LGF_TestAcoustic,                 ARM11_PROCESSOR},
    /* 11 ~ 20 */
    /* 21 ~ 30 */
    {TEST_MODE_KEY_TEST,                    LGT_TestModeKeyTest,              ARM11_PROCESSOR},
    {TEST_MODE_EXT_SOCKET_TEST,             LGF_ExternalSocketMemory,         ARM11_PROCESSOR},
	// *s LG_BTUI_DIAGCMD_DUTMODE munho2.lee@lge.com 110915
	/* Original
    {TEST_MODE_BLUETOOTH_TEST,              not_supported_command_handler,    ARM11_PROCESSOR},
	*/
	{TEST_MODE_BLUETOOTH_TEST,              LGF_TestModeBlueTooth,	          ARM11_PROCESSOR},
	// *e LG_BTUI_DIAGCMD_DUTMODE
    {TEST_MODE_BATT_LEVEL_TEST,             NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_MP3_TEST,                    LGF_TestModeMP3,                  ARM11_PROCESSOR},
	{TEST_MODE_FMRADIO_TEST,                LGF_FmRadioTest,                  ARM11_PROCESSOR},
    
    /* 31 ~ 40 */
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-01-19 support test mode 8.9*/
    {TEST_MODE_ORIENTATION_SENSOR,           linux_app_handler,               ARM11_PROCESSOR},
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-01-19 support test mode 8.9*/
    //[[ NFCDiag wongab.jeon@lge.com
    {TEST_MODE_NFC_TEST,                    LGF_TestModeNFC,                  ARM11_PROCESSOR},
    //]] NFCDiag
    {TEST_MODE_WIFI_TEST,                   LGF_TestModeWLAN,                 ARM11_PROCESSOR},
    {TEST_MODE_MANUAL_TEST_MODE,            LGF_TestModeManualTestMode,       ARM11_PROCESSOR},
    {TEST_MODE_FORMAT_MEMORY_TEST,          not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_KEY_DATA_TEST,               LGF_TestModeKeyData,              ARM11_PROCESSOR},
    /* 41 ~ 50 */
    {TEST_MODE_MEMORY_CAPA_TEST,            LGF_MemoryVolumeCheck,            ARM11_PROCESSOR},
    {TEST_MODE_SLEEP_MODE_TEST,             LGF_TestModeSleepMode,            ARM11_PROCESSOR},
    {TEST_MODE_SPEAKER_PHONE_TEST,          LGF_TestModeSpeakerPhone,         ARM11_PROCESSOR},
	/* LGE_CHANGE_S, [juok.kim@lge.com], 2012-01-17, < DIAG Virtual SIM > */
    {TEST_MODE_VIRTUAL_SIM_TEST,            NULL,                             ARM9_PROCESSOR},	//LGF_TestModeVirtualSimTest,       ARM11_PROCESSOR},  
	/* LGE_CHANGE_E, [juok.kim@lge.com], 2012-01-17, < DIAG Virtual SIM > */
    {TEST_MODE_PHOTO_SENSER_TEST,           not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_MRD_USB_TEST,                NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_PROXIMITY_SENSOR_TEST,       LGF_TestModeProximity,            ARM11_PROCESSOR},
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
    {TEST_MODE_LED_TEST,                    LGF_TestModeLED,                  ARM11_PROCESSOR},
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
    {TEST_MODE_FOTA_ID_CHECK,               LGF_TestModeFotaIDCheck,          ARM11_PROCESSOR},
// LGE_UPDATE_FOTA_E M3 bryan.oh@lge.com 2011/10/18
	{TEST_MODE_KEY_LOCK_UNLOCK,               LGF_TestModeKeyLockUnlock,          ARM11_PROCESSOR},
// [LGE_UPDATE_S] minwoo.jung 20120207
    {TEST_MODE_MLT_ENABLE,                  LGF_TestModeMLTEnableSet,         ARM11_PROCESSOR},
// [LGE_UPDATE_E] minwoo.jung 20120207
// LGE_CHANGE_S, soolim.you@lge.com, 20120113, [GPS DIAG TEST MODE] 
	{TEST_MODE_GNSS_TEST, 					NULL, 							  ARM9_PROCESSOR},   
// LGE_CHANGE_E, soolim.you@lge.com, 20120113, [GPS DIAG TEST MODE] 
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-01-19 support test mode 8.9*/
    {TEST_MODE_SENSOR_CALIBRATION_TEST,     LGF_TestModeAccelCal,             ARM11_PROCESSOR},
    {TEST_MODE_ACCEL_SENSOR_TEST,           linux_app_handler,                ARM11_PROCESSOR},
    {TEST_MODE_COMPASS_SENSOR_TEST,         linux_app_handler,    ARM11_PROCESSOR},
    {TEST_MODE_LCD,                         LGF_TestLCD,                      ARM11_PROCESSOR},
    {TEST_MODE_MFT_CAMERA,                  LGF_TestMFTCamera,                ARM11_PROCESSOR},
    {TEST_MODE_MFT_CAMCORDER,               LGF_TestMFTCamcorder,             ARM11_PROCESSOR},
    {TEST_MODE_PROXIMITY_MFT_SENSOR_TEST,   LGF_TestModeProximityMFT,         ARM11_PROCESSOR},
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-01-19 support test mode 8.9*/
    {TEST_MODE_KEY_TEST_FOR_MFT,                         LGF_TestModeKeyMFT,                      ARM11_PROCESSOR},

// LGE_CHANGE_S, hoseong.kang@lge.com
	{TEST_MODE_VOLUME_TEST_FOR_MFT,			LGF_TestModeVolumeMftTest,		  ARM11_PROCESSOR},
// LGE_CHANGE_E, hoseong.kang@lge.com
	{TEST_MODE_TOUCH_DRAW_FOR_MFT,			LGF_TestTouchDraw,				  ARM11_PROCESSOR},

    {TEST_MODE_XO_CAL_DATA_COPY,            NULL,                             ARM9_PROCESSOR}
};
