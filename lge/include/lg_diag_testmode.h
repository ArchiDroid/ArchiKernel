#ifndef LG_DIAG_TESTMODE_H
#define LG_DIAG_TESTMODE_H
// LG_FW_DIAG_KERNEL_SERVICE

#include "lg_comdef.h"


/*********************** BEGIN PACK() Definition ***************************/
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
/********************** END PACK() Definition *****************************/


/* BEGIN: 0014654 jihoon.lee@lge.com 20110124 */
/* MOD 0014654: [TESTMODE] SYNC UP TESTMODE PACKET STRUCTURE TO KERNEL */
//#define MAX_KEY_BUFF_SIZE    200
#define MAX_KEY_BUFF_SIZE    201
/* END: 0014654 jihoon.lee@lge.com 2011024 */

typedef enum
{
    VER_SW=0,
    VER_DSP,
    VER_MMS,
    VER_CONTENTS,
    VER_PRL,
    VER_ERI,
    VER_BREW,
    VER_MODEL,
    VER_HW,
    REV_DSP=9,
    CONTENTS_SIZE,
    JAVA_FILE_CNT=13,
    JAVA_FILE_SIZE,
    VER_JAVA,
    BANK_ON_CNT=16,
    BANK_ON_SIZE,
    MODULE_FILE_CNT,
    MODULE_FILE_SIZE,
    MP3_DSP_OS_VER=21,
    VER_MODULE  ,
    VER_LCD_REVISION=24
} test_mode_req_version_type;

typedef enum
{
    EXTERNAL_SOCKET_MEMORY_CHECK,
    EXTERNAL_FLASH_MEMORY_SIZE,
    EXTERNAL_SOCKET_ERASE,
    EXTERNAL_FLASH_MEMORY_USED_SIZE = 4,
    EXTERNAL_FLASH_MEMORY_CONTENTS_CHECK,
    EXTERNAL_FLASH_MEMORY_ERASE,
    EXTERNAL_SOCKET_ERASE_SDCARD_ONLY = 0xE,
    EXTERNAL_SOCKET_ERASE_FAT_ONLY = 0xF,
}test_mode_req_socket_memory;

typedef enum
{
    FIRST_BOOTING_COMPLETE_CHECK,
    FIRST_BOOTING_CHG_MODE_CHECK=0xF,
}test_mode_req_fboot;

typedef enum
{
    FIRST_BOOTING_IN_CHG_MODE,
    FIRST_BOOTING_NOT_IN_CHG_MODE
}test_mode_first_booting_chg_mode_type;

typedef enum
{
    MEMORY_TOTAL_CAPA_TEST,
    MEMORY_USED_CAPA_TEST,
    MEMORY_REMAIN_CAPA_TEST
}test_mode_req_memory_capa_type;

typedef enum
{
    MEMORY_TOTAL_SIZE_TEST = 0 ,
    MEMORY_FORMAT_MEMORY_TEST = 1,
}test_mode_req_memory_size_type;

typedef enum
{
    FACTORY_RESET_CHECK,
    FACTORY_RESET_COMPLETE_CHECK,
    FACTORY_RESET_STATUS_CHECK,
    FACTORY_RESET_COLD_BOOT,
    FACTORY_RESET_ERASE_USERDATA = 0x0F,
}test_mode_req_factory_reset_mode_type;

typedef enum{
    FACTORY_RESET_START = 0,
    FACTORY_RESET_INITIAL = 1,
    FACTORY_RESET_ARM9_END = 2,
    FACTORY_RESET_COLD_BOOT_START = 3,
    FACTORY_RESET_COLD_BOOT_END = 5,
    FACTORY_RESET_HOME_SCREEN_END = 6,
    FACTORY_RESET_NA = 7,
}test_mode_factory_reset_status_type;

typedef enum
{
    SLEEP_MODE_ON,
    AIR_PLAIN_MODE_ON,
    FTM_BOOT_ON
} test_mode_sleep_mode_type;

typedef enum
{
    TEST_SCRIPT_ITEM_SET,
    TEST_SCRIPT_MODE_CHECK,
    CAL_DATA_BACKUP,
    CAL_DATA_RESTORE,
    CAL_DATA_ERASE,
    CAL_DATA_INFO
}test_mode_req_test_script_mode_type;

typedef enum
{
    PID_WRITE,
    PID_READ
}test_mode_req_pid_type;

typedef enum
{
    SW_VERSION,
    SW_OUTPUT_VERSION,
    SW_COMPLETE_VERSION,
    SW_VERSION_CHECK
} test_mode_req_sw_version_type;

typedef enum
{
    CAL_CHECK,
    CAL_DATA_CHECK,
} test_mode_req_cal_check_type;

typedef enum
{
    MULTIMODE,
    LTEONLY,
    CDMAONLY,
} test_mode_req_lte_mdoe_seletion_type;

typedef enum
{
    CALL_CONN,
    CALL_DISCONN,
} test_mode_req_lte_call_type;

typedef enum
{
    VIRTUAL_SIM_OFF,
    VIRTUAL_SIM_ON,
    VIRTUAL_SIM_STATUS,
    CAMP_CHECK = 3,
    AUTO_CAMP_REQ = 20,
    DETACH = 21,
}test_mode_req_virtual_sim_type;

typedef enum
{
	CHANGE_MODEM,
	CHANGE_MASS,
} test_mode_req_change_usb_driver_type;

typedef enum
{
    DB_INTEGRITY_CHECK=0,
    FPRI_CRC_CHECK=1,
    FILE_CRC_CHECK=2,
    CODE_PARTITION_CRC_CHECK=3,
    TOTAL_CRC_CHECK=4,
    DB_DUMP_CHECK=5,
    DB_COPY_CHECK=6,    
} test_mode_req_db_check;

typedef enum 
{
    IMEI_WRITE=0, 
    IMEI_READ=1,
    IMEI2_WRITE=2,
    IMEI2_READ=3
} test_mode_req_imei_req_type;

typedef enum
{
    VCO_SELF_TUNNING_ITEM_SET,
    VCO_SELF_TUNNING_ITEM_READ
}test_mode_req_vco_self_tunning_type;

typedef enum 
{
    BATTERY_THERM_ADC=0,	 
    BATTERY_VOLTAGE_LEVEL=1,
    BATTERY_CHARGING_COMPLETE,
    BATTERY_CHARGING_MODE_TEST,
    BATTERY_FUEL_GAUGE_RESET=5,
    BATTERY_FUEL_GAUGE_SOC=6,
#ifdef CONFIG_LGE_BATT_SOC_FOR_NPST
    BATTERY_FUEL_GAUGE_SOC_NPST =99,
#endif
} test_mode_req_batter_bar_type;

typedef enum 
{
    MANUAL_TEST_ON,
    MANUAL_TEST_OFF,
    MANUAL_MODE_CHECK
} test_mode_req_manual_test_mode_type;

typedef enum
{
    CDMA_WCDMA_MAX_POWER_ON = 0,
    CDMA_WCDMA_MAX_POWER_OFF =1,
    LTE_TESTMODE_ON = 2,
    LTE_RF_ON = 3,
    LTE_FAKE_SYNC = 4,
    LTE_RX_SETUP = 5,
    LTE_SCHEDULE = 6,
    LTE_TX_SETUP = 7,
    LTE_TX_POWER_SETUP = 8,
    LTE_MAX_POWER_OFF = 9,
    LTE_RF_OFF = 10,	
} test_mode_rep_max_current_check_type; 

typedef enum
{
    LTE_OFF_AND_CDMA_ON = 0,
    LTE_ON_AND_CDMA_OFF = 1,
    RF_MODE_CHECK = 2
} test_mode_rep_change_rf_cal_mode_type;

typedef enum
{
	DUAL_ANT = 0,
	SECONDARY_ANT_ONLY = 1,
	PRIMARY_ANT_ONLY = 2
} test_mode_rep_select_mimo_ant_type;

typedef enum
{
    MODE_OFF,
    MODE_ON,
    STATUS_CHECK,
    IRDA_AUTO_TEST_START,
    IRDA_AUTO_TEST_RESULT,
    EXT_CARD_AUTO_TEST,
}test_mode_req_irda_fmrt_finger_uim_type;

typedef enum
{
    RESET_FIRST_PRODUCTION,
    RESET_FIRST_PRODUCTION_CHECK,
    RESET_REFURBISH=2,
    RESET_REFURBISH_CHECK,
}test_mode_req_reset_production_type;

#define BT_ADDR_CNT 12

// +s LG_BTUI_DIAGCMD_DUTMODE munho2.lee@lge.com 110915
typedef enum
{
  BT_GET_ADDR, //no use anymore
  BT_TEST_MODE_1=1,
  BT_TEST_MODE_CHECK=2,
  BT_TEST_MODE_RELEASE=5,
  BT_TEST_MODE_11=11 // 11~42
}test_mode_req_bt_type;
// +e LG_BTUI_DIAGCMD_DUTMODE

typedef enum
{
    BLUETOOTH_ADDRESS_WRITE,
    BLUETOOTH_ADDRESS_READ,
}test_mode_req_bt_addr_req_type;

typedef struct {
    test_mode_req_bt_addr_req_type req_type;
    byte bt_addr[BT_ADDR_CNT];
}test_mode_req_bt_addr_type;

#define WIFI_MAC_ADDR_CNT 12
// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
typedef enum
{
  WLAN_TEST_MODE_54G_ON=4,
  WLAN_TEST_MODE_OFF,
  WLAN_TEST_MODE_RX_START,
  WLAN_TEST_MODE_RX_RESULT=9,
  WLAN_TEST_MODE_TX_START=10,
  WLAN_TEST_MODE_TXRX_STOP=13,
  WLAN_TEST_MODE_LF_RX_START=31,
  WLAN_TEST_MODE_MF_TX_START=44,
  WLAN_TEST_MODE_11B_ON=57,
  WLAN_TEST_MODE_11N_MIXED_LONG_GI_ON=69,
  WLAN_TEST_MODE_11N_MIXED_SHORT_GI_ON=77,
  WLAN_TEST_MODE_11N_GREEN_LONG_GI_ON=85,
  WLAN_TEST_MODE_11N_GREEN_SHORT_GI_ON=93,
  WLAN_TEST_MODE_11A_CH_RX_START=101, // not support
  WLAN_TEST_MODE_11BG_CH_TX_START=128,
  WLAN_TEST_MODE_11A_ON=155,
  WLAN_TEST_MODE_11AN_MIXED_LONG_GI_ON=163,
  WLAN_TEST_MODE_MAX=195,
}test_mode_req_wifi_type;

typedef enum
{
  WLAN_TEST_MODE_CTGRY_ON,
  WLAN_TEST_MODE_CTGRY_OFF,
  WLAN_TEST_MODE_CTGRY_RX_START,
  WLAN_TEST_MODE_CTGRY_RX_STOP,
  WLAN_TEST_MODE_CTGRY_TX_START,
  WLAN_TEST_MODE_CTGRY_TX_STOP,
  WLAN_TEST_MODE_CTGRY_NOT_SUPPORTED,
} test_mode_ret_wifi_ctgry_t;

// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]

typedef enum
{
    WIFI_MAC_ADDRESS_WRITE = 0,
    WIFI_MAC_ADDRESS_READ = 1,
}test_mode_req_wifi_addr_req_type;

typedef struct {
	//test_mode_req_wifi_addr_req_type req_type;
	byte req_type;
    byte wifi_mac_addr[WIFI_MAC_ADDR_CNT];
}test_mode_req_wifi_addr_type;

typedef enum {
    XO_FIELD_CALIBRATION,
    XO_FACTORY_CALIBRATION,
} test_mode_req_XOCalDataBackup_Type;

// LGE_UPDATE_FOTA_S M3 bryan.oh@lge.com 2011/10/18
typedef enum
{
    FOTA_ID_CHECK,
    FOTA_ID_READ
}test_mode_req_fota_id_check_type;

/* LGE_CHANGE_S [myunghwan.kim@lge.com] 2011-09-27 support test mode */
typedef enum
{
  LCD_INITIAL=0,
  LCD_TILT=2,
  LCD_COLOR,
  LCD_ON,
  LCD_OFF
}test_mode_req_lcd_type;

typedef enum
{
  MOTOR_OFF,
  MOTOR_ON
}test_mode_req_motor_type;

typedef enum
{
  ACOUSTIC_OFF=0,
  ACOUSTIC_ON,
  HEADSET_PATH_OPEN,
  HANDSET_PATH_OPEN,
  ACOUSTIC_LOOPBACK_ON,
  ACOUSTIC_LOOPBACK_OFF
}test_mode_req_acoustic_type;

typedef enum
{
	CAM_TEST_MODE_OFF = 0,
	CAM_TEST_MODE_ON,
	CAM_TEST_SHOT,
	CAM_TEST_SAVE_IMAGE,
	CAM_TEST_CALL_IMAGE,
	CAM_TEST_ERASE_IMAGE,
	CAM_TEST_FLASH_ON,
	CAM_TEST_FLASH_OFF = 9,
	CAM_TEST_CAMCORDER_MODE_OFF,
	CAM_TEST_CAMCORDER_MODE_ON,
	CAM_TEST_CAMCORDER_SHOT,
	CAM_TEST_CAMCORDER_SAVE_MOVING_FILE,
	CAM_TEST_CAMCORDER_PLAY_MOVING_FILE,
	CAM_TEST_CAMCORDER_ERASE_MOVING_FILE,
	CAM_TEST_CAMCORDER_FLASH_ON,
	CAM_TEST_CAMCORDER_FLASH_OFF,
	CAM_TEST_STROBE_LIGHT_ON,
	CAM_TEST_STROBE_LIGHT_OFF,
	CAM_TEST_CAMERA_SELECT = 22,
}test_mode_req_cam_type;

typedef enum
{
  MP3_128KHZ_0DB,
  MP3_128KHZ_0DB_L,
  MP3_128KHZ_0DB_R,
  MP3_MULTISINE_20KHZ,
  MP3_PLAYMODE_OFF,
  MP3_SAMPLE_FILE,
  MP3_NoSignal_LR_128k
}test_mode_req_mp3_test_type;

typedef enum
{
  SPEAKER_PHONE_OFF,
  SPEAKER_PHONE_ON,
  NOMAL_Mic1,
  NC_MODE_ON,
  ONLY_MIC2_ON_NC_ON,
  ONLY_MIC1_ON_NC_ON
}test_mode_req_speaker_phone_type;

typedef enum
{
  VOL_LEV_OFF,
  VOL_LEV_MIN,
  VOL_LEV_MEDIUM,
  VOL_LEV_MAX
}test_mode_req_volume_level_type;
/* LGE_CHANGE_E [myunghwan.kim@lge.com] 2011-09-27 support test mode */

#define MAX_NTCODE_SIZE (512-3)

typedef union
{
    test_mode_req_version_type version;
    test_mode_req_socket_memory esm;
    test_mode_req_fboot fboot;
    test_mode_req_memory_capa_type mem_capa;
    test_mode_req_memory_size_type memory_format;
    word key_data;
    boolean key_test_start;
    test_mode_req_factory_reset_mode_type factory_reset;
    test_mode_sleep_mode_type sleep_mode;
    test_mode_req_test_script_mode_type test_mode_test_scr_mode;
    test_mode_req_pid_type pid;
    test_mode_req_vco_self_tunning_type vco_self_tunning;
    test_mode_req_factory_reset_mode_type test_factory_mode;
    byte CGPSTest;
    test_mode_req_batter_bar_type batt;
    test_mode_req_manual_test_mode_type test_manual_mode; 
    test_mode_req_sw_version_type sw_version;
    test_mode_req_cal_check_type cal_check;
    test_mode_req_db_check db_check;
    test_mode_req_lte_mdoe_seletion_type mode_seletion;
    test_mode_req_lte_call_type lte_call;
    test_mode_req_virtual_sim_type lte_virtual_sim;
    test_mode_rep_max_current_check_type max_current;
    test_mode_rep_change_rf_cal_mode_type rf_mode;
    test_mode_rep_select_mimo_ant_type select_mimo;
    test_mode_req_change_usb_driver_type change_usb_driver;
    test_mode_req_irda_fmrt_finger_uim_type ext_device_cmd;
    test_mode_req_reset_production_type reset_production_cmd;
	// +s LG_BTUI_DIAGCMD_DUTMODE munho2.lee@lge.com 110915
	test_mode_req_bt_type bt;
	// +e LG_BTUI_DIAGCMD_DUTMODE
    test_mode_req_bt_addr_type bt_ad;
// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
    test_mode_req_wifi_type wifi;
// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]	
    test_mode_req_wifi_addr_type wifi_mac_ad;
    test_mode_req_XOCalDataBackup_Type XOCalDataBackup;

// LGE_UPDATE_FOTA_S M3 bryan.oh@lge.com 2011/10/18
    test_mode_req_fota_id_check_type fota_id_check;
// LGE_UPDATE_FOTA_E M3 bryan.oh@lge.com 2011/10/18



	/* LGE_CHANGE_S [myunghwan.kim@lge.com] 2011-09-27 support test mode */
    test_mode_req_lcd_type lcd;
    test_mode_req_motor_type motor;
    test_mode_req_acoustic_type acoustic;
    test_mode_req_cam_type camera;
    test_mode_req_mp3_test_type mp3_play;
    test_mode_req_speaker_phone_type speaker_phone;
    test_mode_req_volume_level_type volume_level;
    /* LGE_CHANGE_E [myunghwan.kim@lge.com] 2011-09-27 support test mode */

	byte NTCode[MAX_NTCODE_SIZE];
} test_mode_req_type;

typedef struct diagpkt_header
{
    byte opaque_header;
}PACKED diagpkt_header_type;

typedef struct DIAG_TEST_MODE_F_req_tag {
    diagpkt_header_type		xx_header;
    word					sub_cmd_code;
    test_mode_req_type		test_mode_req;
} PACKED DIAG_TEST_MODE_F_req_type;

typedef enum
{
    TEST_OK_S,
    TEST_FAIL_S,
    TEST_NOT_SUPPORTED_S
} PACKED (test_mode_ret_stat_type);

typedef struct
{
    byte SVState;
    uint8 SV;
    uint16 MeasuredCNo;
} PACKED CGPSResultType;

// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
typedef struct
{
	int packet;
	int per;
} PACKED WlRxResults;
// LGE_CHANGE_E, real-wifi@lge.com, 20110928, [WLAN TEST MODE]

/* BEGIN: 0014654 jihoon.lee@lge.com 20110124 */
/* MOD 0014654: [TESTMODE] SYNC UP TESTMODE PACKET STRUCTURE TO KERNEL */
typedef union
{
    test_mode_req_version_type version;
    byte str_buf[17];
    CGPSResultType TestResult[16];
    char key_pressed_buf[MAX_KEY_BUFF_SIZE];
    char memory_check;
    uint32 socket_memory_size;
    uint32 socket_memory_usedsize;
    unsigned int mem_capa;
    char batt_voltage[5];
  	byte	chg_stat;
  	//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-10-04
  	char	battery_soc[3];
  	//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-10-04
    int manual_test;
    test_mode_req_pid_type pid;
    test_mode_req_sw_version_type sw_version;
    byte hkadc_value;
    byte uim_state;
    byte vco_value;
// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
    byte wlan_status;
    WlRxResults wlan_rx_results;
// LGE_CHANGE_S, real-wifi@lge.com, 20110928, [WLAN TEST MODE]
    test_mode_req_cal_check_type cal_check;
    test_mode_req_factory_reset_mode_type factory_reset;
    test_mode_req_test_script_mode_type test_mode_test_scr_mode;
    byte read_bd_addr[BT_ADDR_CNT];
    byte read_wifi_mac_addr[WIFI_MAC_ADDR_CNT];    
    test_mode_req_XOCalDataBackup_Type XOCalDataBackup;

// LGE_UPDATE_FOTA_S M3 hyunbae.jun@lge.com 2011/10/19
   byte fota_id[15];
	
	/* LGE_CHANGE_S [myunghwan.kim@lge.com] 2011-09-27 support test mode */
    test_mode_req_lcd_type lcd;
    test_mode_req_motor_type motor;
    test_mode_req_acoustic_type acoustic;
    test_mode_req_cam_type camera;
    test_mode_req_mp3_test_type mp3_play;
    test_mode_req_speaker_phone_type speaker_phone;
    test_mode_req_volume_level_type volume_level;
    /* LGE_CHANGE_E [myunghwan.kim@lge.com] 2011-09-27 support test mode */

} PACKED test_mode_rsp_type;
/* END: 0014654 jihoon.lee@lge.com 2011024 */

typedef struct DIAG_TEST_MODE_F_rsp_tag {
    diagpkt_header_type		xx_header;
    word					sub_cmd_code;
    test_mode_ret_stat_type	ret_stat_code;
    test_mode_rsp_type		test_mode_rsp;
// [110919 kkh8318@lge.com M3_ALL]Added Factory Reset Test [START]
    byte					factory_flag;
// [110919 kkh8318@lge.com M3_ALL] [END]
} PACKED DIAG_TEST_MODE_F_rsp_type;

typedef enum
{
    TEST_MODE_VERSION=0,
    TEST_MODE_LCD=1,
    TEST_MODE_MOTOR=3,
    TEST_MODE_ACOUSTIC,
    TEST_MODE_CAM=7,
    TEST_MODE_EFS_INTEGRITY=11,
    TEST_MODE_IRDA_FMRT_FINGER_UIM_TEST=13,
    TEST_MODE_BREW_CNT=20,

    TEST_MODE_BREW_SIZE=21,
    TEST_MODE_KEY_TEST,
    TEST_MODE_EXT_SOCKET_TEST,
    TEST_MODE_BLUETOOTH_TEST,
    TEST_MODE_BATT_LEVEL_TEST,
    TEST_MODE_MP3_TEST=27,
    TEST_MODE_FM_TRANCEIVER_TEST,
    TEST_MODE_ISP_DOWNLOAD_TEST,
    TEST_MODE_COMPASS_SENSOR_TEST=30,

    TEST_MODE_ACCEL_SENSOR_TEST=31,
    TEST_MODE_ALCOHOL_SENSOR_TEST=32,
    TEST_MODE_TDMB_TEST=33,
    TEST_MODE_WIFI_TEST=33,
    TEST_MODE_TV_OUT_TEST=33,
    TEST_MODE_SDMB_TEST=33,
    TEST_MODE_MANUAL_TEST_MODE=36,
    TEST_MODE_FORMAT_MEMORY_TEST=38,
    TEST_MODE_3D_ACCELERATOR_SENSOR_TEST=39,
    TEST_MODE_KEY_DATA_TEST = 40,  

    TEST_MODE_MEMORY_CAPA_TEST = 41,  
    TEST_MODE_SLEEP_MODE_TEST,
    TEST_MODE_SPEAKER_PHONE_TEST,
    TEST_MODE_VIRTUAL_SIM_TEST = 44,
    TEST_MODE_PHOTO_SENSER_TEST,
    TEST_MODE_VCO_SELF_TUNNING_TEST,
    TEST_MODE_MRD_USB_TEST=47,
    TEST_MODE_TEST_SCRIPT_MODE = 48,
    TEST_MODE_PROXIMITY_SENSOR_TEST = 49,
    TEST_MODE_FACTORY_RESET_CHECK_TEST = 50,

    TEST_MODE_VOLUME_TEST=51,
    TEST_MODE_HANDSET_FREE_ACTIVATION_TEST,
    TEST_MODE_MOBILE_SYSTEM_CHANGE_TEST,
    TEST_MODE_STANDALONE_GPS_TEST,
    TEST_MODE_PRELOAD_INTEGRITY_TEST,
    TEST_MODE_FIRST_BOOT_COMPLETE_TEST = 58,
    TEST_MODE_MAX_CURRENT_CHECK = 59,
    TEST_MODE_LED_TEST = 60,

    TEST_MODE_CHANGE_RFCALMODE = 61,
    TEST_MODE_SELECT_MIMO_ANT = 62,
    TEST_MODE_LTE_MODE_SELECTION = 63,
    TEST_MODE_LTE_CALL = 64,
    TEST_MODE_CHANGE_USB_DRIVER = 65,
    TEST_MODE_GET_HKADC_VALUE = 66,
    TEST_MODE_PID_TEST = 70,

    TEST_MODE_SW_VERSION = 71,
    TEST_MODE_IME_TEST=72,
    TEST_MODE_IMPL_TEST,
    TEST_MODE_SIM_LOCK_TYPE_TEST,
    TEST_MODE_UNLOCK_CODE_TEST,
    TEST_MODE_IDDE_TEST,
    TEST_MODE_FULL_SIGNATURE_TEST,
    TEST_MODE_NT_CODE_TEST,
    TEST_MODE_SIM_ID_TEST = 79,

    TEST_MODE_CAL_CHECK= 82,
    TEST_MODE_BLUETOOTH_RW=83,
    TEST_MODE_SKIP_WELCOM_TEST = 87,
    TEST_MODE_WIFI_MAC_RW = 88,

    TEST_MODE_DB_INTEGRITY_CHECK=91,
    TEST_MODE_NVCRC_CHECK = 92,
    TEST_MODE_RESET_PRODUCTION = 96,

// LGE_UPDATE_FOTA_S M3 bryan.oh@lge.com 2011/10/18
    TEST_MODE_FOTA_ID_CHECK = 98,
// LGE_UPDATE_FOTA_E M3 bryan.oh@lge.com 2011/10/18

    TEST_MODE_XO_CAL_DATA_COPY=250,

    MAX_TEST_MODE_SUBCMD = 0xFFFF
} PACKED test_mode_sub_cmd_type;

#define TESTMODE_MSTR_TBL_SIZE   128

#define ARM9_PROCESSOR       0
#define ARM11_PROCESSOR     1

typedef void*(* testmode_func_type)(test_mode_req_type * , DIAG_TEST_MODE_F_rsp_type * );

typedef struct
{
    word cmd_code;
    testmode_func_type func_ptr;
    byte  which_procesor;             // to choose which processor will do act.
}testmode_user_table_entry_type;

/* LGE_CHANGES_S, [dongp.kim@lge.com], 2010-01-10, <LGE_FACTORY_TEST_MODE for WLAN RF Test > */
typedef struct
{
    uint16 countresult;
    uint16 wlan_status;
    uint16 g_wlan_status;
    uint16 rx_channel;
    uint16 rx_per;
    uint16 tx_channel;
    uint32 goodFrames;
    uint16 badFrames;
    uint16 rxFrames;
    uint16 wlan_data_rate;
    uint16 wlan_payload;
    uint16 wlan_data_rate_recent;
    unsigned long pktengrxducast_old;
    unsigned long pktengrxducast_new;
    unsigned long rxbadfcs_old;
    unsigned long rxbadfcs_new;
    unsigned long rxbadplcp_old;
    unsigned long rxbadplcp_new;
}wlan_status;
/* LGE_CHANGES_E, [dongp.kim@lge.com], 2010-01-10, <LGE_FACTORY_TEST_MODE for WLAN RF Test > */

typedef struct DIAG_TEST_MODE_KEY_F_rsp_tag {
    diagpkt_header_type xx_header;
    word sub_cmd_code;
    test_mode_ret_stat_type ret_stat_code;
    char key_pressed_buf[MAX_KEY_BUFF_SIZE];
} PACKED DIAG_TEST_MODE_KEY_F_rsp_type;

#endif /* LG_DIAG_TESTMODE_H */
