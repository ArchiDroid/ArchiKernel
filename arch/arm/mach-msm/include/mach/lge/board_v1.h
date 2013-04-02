#ifndef __ASM_ARCH_MSM_BOARD_LGE_H
#define __ASM_ARCH_MSM_BOARD_LGE_H

#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include "lge_proc_comm.h"
#ifdef CONFIG_LGE_BOOT_MODE
#include "lge_boot_mode.h"
#endif
#if __GNUC__
#define __WEAK __attribute__((weak))
#endif

#ifdef CONFIG_ANDROID_RAM_CONSOLE
/* allocate 128K * 2 instead of ram_console's original size 128K
 * this is for storing kernel panic log which is used by lk loader
 * 2010-03-03, cleaneye.kim@lge.com
 */
#define MSM7X27_EBI1_CS0_BASE	PHYS_OFFSET
#define LGE_RAM_CONSOLE_SIZE    		(124 * SZ_1K * 2)
#endif

#ifdef CONFIG_LGE_HANDLE_PANIC
#define LGE_CRASH_LOG_SIZE              (4 * SZ_1K)
#endif
//LGE_CHANGE_S FTM boot mode
enum lge_fboot_mode_type {
	first_boot,
	second_boot
};
//LGE_CHANGE_E FTM boot mode
#if 0
#define PMEM_KERNEL_EBI1_SIZE	0x3A000
#define MSM_PMEM_AUDIO_SIZE	0x5B000
#endif

#ifdef CONFIG_LGE_HW_REVISION
/* board revision information */
typedef  enum {
	EVB = 0,
	LGE_REV_A,
	LGE_REV_B,
	LGE_REV_C,
	LGE_REV_D,
	LGE_REV_E,
	LGE_REV_F,
	LGE_REV_G,
	LGE_REV_10,
	LGE_REV_11,
	LGE_REV_12,
	LGE_REV_TOT_NUM,
}lge_pcb_rev;

#define REV_EVB	0
#define REV_A	1
#define REV_B	2
#define REV_C	3
#define REV_D	4
#define REV_E	5
#define REV_F	6
#define REV_G	7
#define REV_10	8
#define REV_11	9
#define REV_12	10
#endif

/* define gpio pin number of i2c-gpio */
struct gpio_i2c_pin {
	unsigned int sda_pin;
	unsigned int scl_pin;
	unsigned int reset_pin;
	unsigned int irq_pin;
};

/*LGE_CHANGE_S : seven.kim@lge.com kernel3.0 porting
 * camera flash device/driver naming match
 */
 #if defined (CONFIG_LEDS_AS364X)
#define LEDS_CAMERA_FLASH_NAME	"as3647"
#endif
/*LGE_CHANGE_E : seven.kim@lge.com kernel3.0 porting*/

#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
#define GPIO_SD_DETECT_N 40
#endif

#if defined (CONFIG_MACH_MSM7X25A_V1)
#define GPIO_SD_DETECT_N 40

#define SENSOR_GPIO_I2C_SCL		13
#define SENSOR_GPIO_I2C_SDA		35

/* accelerometer */
#define ACCEL_GPIO_INT			94	/* motion interrupt 1*/
#define ACCEL_GPIO_I2C_SCL		SENSOR_GPIO_I2C_SCL
#define ACCEL_GPIO_I2C_SDA		SENSOR_GPIO_I2C_SDA
#if defined (CONFIG_SENSORS_BMA2X2)
#define ACCEL_I2C_ADDRESS		0x18//0x10 LGE_CHANGE : [v1] jinseok.choi 2013-01-31 v1 uses BMA 255, change i2c address
#else
#define ACCEL_I2C_ADDRESS		0x18 
#endif

/* BOSCH Ecompass : Bosch compass+accelerometer are internally use two sensor */
#define ECOM_GPIO_I2C_SCL		SENSOR_GPIO_I2C_SCL
#define ECOM_GPIO_I2C_SDA		SENSOR_GPIO_I2C_SDA
#define ECOM_GPIO_INT			130 /* motion interrupt 2*/
#if defined (CONFIG_SENSORS_BMA2X2)
#define ECOM_I2C_ADDRESS		0x12
#else
#define ECOM_I2C_ADDRESS		0x10 
#endif

/* proximity sensor */
#define PROXI_GPIO_I2C_SCL		16
#define PROXI_GPIO_I2C_SDA		30
#define PROXI_GPIO_DOUT			17
#if defined (CONFIG_SENSOR_APDS9190) || defined(CONFIG_SENSOR_RPR0400) //LGE_CHANGE : 2013-02-04 jinseok.choi@lge.com V1 uses Rohm RPR0400 for Proximity Sensor
#define PROXI_I2C_ADDRESS		0x39 /* slave address 7bit - GP2AP002 */
#else
#define PROXI_I2C_ADDRESS		0x44 /* slave address 7bit - GP2AP002 */
#endif
#define PROXI_LDO_NO_VCC		1
#endif /* CONFIG_MACH_MSM7X25A_V1 */

#ifdef CONFIG_LEDS_LP5521
#define RGB_GPIO_I2C_SCL		77
#define RGB_GPIO_I2C_SDA		80

#define RGB_GPIO_RGB_EN 		115
#define RGB_I2C_ADDRESS			0x32
#endif	/* CONFIG_LEDS_LP5521 */

/* touch screen platform data */
#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS)
struct touch_platform_data {
	int ts_x_min;
	int ts_x_max;
	int ts_y_min;
	int ts_y_max;
	int (*power)(unsigned char onoff);
	int irq;
	int scl;
	int sda;
};
#endif

/* touch screen platform data */
#if defined(CONFIG_TOUCHSCREEN_MELFAS_MMS128S) || defined(CONFIG_TOUCHSCREEN_MELFAS_MMS128S_V1)
#define MELFAS_TS_NAME "melfas-ts"

#define TS_X_MIN				0
#define TS_X_MAX				240
#define TS_Y_MIN				0
#define TS_Y_MAX				320
#define TS_GPIO_I2C_SDA			10
#define TS_GPIO_I2C_SCL			9
#define TS_GPIO_IRQ				39
#define TS_I2C_SLAVE_ADDR		0x48
#define TS_TOUCH_LDO_EN			14
#define TS_TOUCH_ID				121

struct touch_platform_data {
	int ts_x_min;
	int ts_x_max;
	int ts_y_min;
	int ts_y_max;
	int (*power)(unsigned char onoff);
	int irq;
	int scl;
	int sda;
};
#endif

#if defined(CONFIG_LGE_TOUCHSCREEN_SYNAPTICS_I2C_RMI4)
#define MELFAS_TS_NAME "melfas-ts"

#define TS_X_MIN				0
#define TS_X_MAX				480
#define TS_Y_MIN				0
#define TS_Y_MAX				800
#define TS_GPIO_I2C_SDA			10
#define TS_GPIO_I2C_SCL			9
#define TS_GPIO_IRQ				39
#define TS_I2C_SLAVE_ADDR		0x48
#define TS_TOUCH_LDO_EN			14
#define TS_TOUCH_ID				121

struct melfas_tsi_platform_data {
	uint32_t version;
	int max_x;
	int max_y;
	int max_pressure;
	int max_width;
	int gpio_scl;
	int gpio_sda;
	int i2c_int_gpio;
	int (*power)(int on);	/* Only valid in first array entry */
	int (*power_enable)(int en, bool log_en);
};

#define SYNAPTICS_TS_I2C_SLAVE_ADDR            0x20
#define SYNAPTICS_TS_I2C_SDA                   10
#define SYNAPTICS_TS_I2C_SCL                   9
#define SYNAPTICS_TS_I2C_INT_GPIO              39
#endif

#if !defined(CONFIG_LGE_TOUCHSCREEN_SYNAPTICS_I2C_RMI4) && defined(CONFIG_TOUCHSCREEN_MELFAS_MMS136) 
#define MELFAS_TS_NAME "melfas-ts"

#define TS_X_MIN				0
#define TS_X_MAX				480
#define TS_Y_MIN				0
#define TS_Y_MAX				800
#define TS_GPIO_I2C_SDA			10
#define TS_GPIO_I2C_SCL			9
#define TS_GPIO_IRQ				39
#define TS_I2C_SLAVE_ADDR		0x48
#define TS_TOUCH_LDO_EN			14
#define TS_TOUCH_ID				121

struct melfas_tsi_platform_data {
	uint32_t version;
	int max_x;
	int max_y;
	int max_pressure;
	int max_width;
	int gpio_scl;
	int gpio_sda;
	int i2c_int_gpio;
	int (*power)(int on);	/* Only valid in first array entry */
	int (*power_enable)(int en, bool log_en);
};
#endif 

#if defined(CONFIG_TOUCHSCREEN_MCS8000)
struct touch_platform_data {
	int ts_x_min;
	int ts_x_max;
	int ts_y_min;
	int ts_y_max;
	int (*power)(unsigned char onoff);
	int irq;
	int scl;
	int sda;
};
#endif

#if defined(CONFIG_TOUCHSCREEN_MELFAS_TS)
#define TS_X_MIN				0
#define TS_X_MAX				320
#define TS_Y_MIN				0
#define TS_Y_MAX				480
#define TS_GPIO_I2C_SDA			10
#define TS_GPIO_I2C_SCL			9
#define TS_GPIO_IRQ				39
#define TS_I2C_SLAVE_ADDR		0x48
#define TS_TOUCH_LDO_EN			14
#define TS_TOUCH_ID				121

struct touch_platform_data {
	int ts_x_min;
	int ts_x_max;
	int ts_y_min;
	int ts_y_max;
	int (*power)(unsigned char onoff);
	int irq;
	int scl;
	int sda;
};
#endif
#if defined(CONFIG_TOUCHSCREEN_MXT140)
struct touch_platform_data {
	int ts_x_min;
	int ts_x_max;
	int ts_y_min;
	int ts_y_max;
	int ts_y_start;
	int ts_y_scrn_max;
	int (*power)(unsigned char onoff);
	int irq;
	int gpio_int;
	int hw_i2c;
	int scl;
	int sda;
	int ce;
};
#endif

/* acceleration platform data */
struct acceleration_platform_data {
	int irq_num;
	int (*power)(unsigned char onoff);
};

#if defined(CONFIG_SENSOR_K3DH)
/* k3dh acceleration platform data */
struct k3dh_platform_data {
	int poll_interval;
	int min_interval;

	u8 g_range;

	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;

	int (*kr_init)(void);
	void (*kr_exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);
	int sda_pin;
	int scl_pin;
	int pin_int;
	int (*gpio_config)(int);
};
#endif

/* ecompass platform data */
struct ecom_platform_data {
	int pin_int;
	int pin_rst;
	int (*power)(unsigned char onoff);
	char accelerator_name[20];
	int fdata_sign_x;
        int fdata_sign_y;
        int fdata_sign_z;
	int fdata_order0;
	int fdata_order1;
	int fdata_order2;
	int sensitivity1g;
	s16 *h_layout;
	s16 *a_layout;
	int drdy;
};

/* proximity platform data */
struct proximity_platform_data {
	int irq_num;
	int (*power)(struct device *dev, unsigned char onoff);
	int methods;
	int operation_mode;
	int debounce;
	u8 cycle;
};

#if defined(CONFIG_SENSORS_BH1721)
/* light ambient platform data */
struct light_ambient_platform_data {
	int power_state;
	int dvi_gpio;
	int (*power_on)(unsigned char onoff);
	int (*dvi_reset_ctrl)(unsigned char onoff);
};
#endif

/* backlight platform data*/
struct lge_backlight_platform_data {
	void (*platform_init)(void);
	int gpio;
	unsigned int mode;		     /* initial mode */
	int max_current;			 /* led max current(0-7F) */
	int initialized;			 /* flag which initialize on system boot */
	int version;				 /* Chip version number */
};

/* android vibrator platform data */
struct android_vibrator_platform_data {
	int enable_status;
	int (*power_set)(int enable); 		/* LDO Power Set Function */
	int (*pwm_set)(int enable, int gain); 		/* PWM Set Function */
	int (*ic_enable_set)(int enable); 	/* Motor IC Set Function */
	int (*gpio_request)(void);	/* gpio request */
	int amp_value;				/* PWM tuning value */
};

struct gpio_h2w_platform_data {
	int gpio_detect;
	int gpio_button_detect;
	int gpio_mic_mode;
};

/* gpio switch platform data */
struct lge_gpio_switch_platform_data {
	const char *name;
	unsigned *gpios;
	size_t num_gpios;
	unsigned long irqflags;
	unsigned int wakeup_flag;
	int (*work_func)(int *value);
	char *(*print_name)(int state);
	char *(*print_state)(int state);
	int (*sysfs_store)(const char *buf, size_t size);
	int (*additional_init)(void);

	/* especially to address gpio key */
	unsigned *key_gpios;
	size_t num_key_gpios;
	int (*key_work_func)(int *value);
};

/* LED flash platform data */
struct led_flash_platform_data {
	int gpio_flen;
	int gpio_en_set;
	int gpio_inh;
};

/* pp2106 qwerty platform data */
struct pp2106_platform_data {
	unsigned int reset_pin;
	unsigned int irq_pin;
	unsigned int sda_pin;
	unsigned int scl_pin;
	unsigned int keypad_row;
	unsigned int keypad_col;
	unsigned char *keycode;
	int (*power)(unsigned char onoff);
};

/* LCD panel */
enum {
	PANEL_ID_AUTO = 0,
	PANEL_ID_LGDISPLAY_1 = 1,
	PANEL_ID_TOVIS = 2,
	PANEL_ID_LGDISPLAY = 3,
	PANEL_ID_ILITEK =4,
};
/* LCD panel IL9486*/
enum {
	PANEL_ID_OLD_ILI9486 = 0,
	PANEL_ID_NEW_ILI9486 = 1,
};

struct msm_panel_ilitek_pdata {
	int gpio;
	int initialized;
	int maker_id;
	int (*lcd_power_save)(int);
};

void __init msm_msm7x2x_allocate_memory_regions(void);
#if 0
void __init msm7x27a_reserve(void);
#endif
/* lge API functions to register i2c devices */

/* LGE_CHANGE_S: murali.ramaiah@lge.com [2011-09-22]  */
#ifdef CONFIG_LGE_POWER_ON_STATUS_PATCH
void __init lge_board_pwr_on_status(void);
#endif
/* LGE_CHANGE_E: murali.ramaiah@lge.com [2011-09-22]  */

typedef void (gpio_i2c_init_func_t)(int bus_num);

void __init lge_add_gpio_i2c_device(gpio_i2c_init_func_t *init_func);
void __init lge_add_gpio_i2c_devices(void);
int __init lge_init_gpio_i2c_pin(struct i2c_gpio_platform_data *i2c_adap_pdata,
		struct gpio_i2c_pin gpio_i2c_pin,
		struct i2c_board_info *i2c_board_info_data);
int __init lge_init_gpio_i2c_pin_pullup(struct i2c_gpio_platform_data *i2c_adap_pdata,
		struct gpio_i2c_pin gpio_i2c_pin,
		struct i2c_board_info *i2c_board_info_data);

void __init msm_add_fb_device(void);
void __init msm_add_pmem_devices(void);

/* lge common functions to add devices */
void __init lge_add_input_devices(void);
void __init lge_add_misc_devices(void);
void __init lge_add_mmc_devices(void);
void __init lge_add_sound_devices(void);
void __init lge_add_lcd_devices(void);
void __init lge_add_camera_devices(void);
void __init lge_add_pm_devices(void);
void __init lge_add_usb_devices(void);
void __init lge_add_connectivity_devices(void);
/*LGE_CHANGE_S : NFC ,2011-11-15, siny@lge.com, Delete SW I2C NFC*/
void __init lge_add_nfc_devices(void);
/*LGE_CHANGE_E : NFC*/

void __init lge_add_gpio_i2c_device(gpio_i2c_init_func_t *init_func);

void __init lge_add_ramconsole_devices(void);
#if defined(CONFIG_ANDROID_RAM_CONSOLE) && defined(CONFIG_LGE_HANDLE_PANIC)
void __init lge_add_panic_handler_devices(void);
void lge_set_reboot_reason(unsigned reason);
#endif
int __init lge_get_uart_mode(void);
#ifdef CONFIG_LGE_HW_REVISION
lge_pcb_rev  lge_get_board_revno(void);
#endif

int get_reboot_mode(void);

//LGE_CHANGE_S FTM boot mode
enum lge_fboot_mode_type lge_get_fboot_mode(void);
unsigned lge_nv_manual_f(int val);
//LGE_CHANGE_E FTM boot mode
/* LGE_CHANGE_S, youngbae.choi@lge.com, for silence reset */
#ifdef CONFIG_LGE_SILENCE_RESET
unsigned lge_silence_reset_f(int val);
#endif
/* LGE_CHANGE_E, youngbae.choi@lge.com, for silence reset */

unsigned lge_smpl_counter_f(int val);
unsigned lge_charging_bypass_boot_f(int val);
unsigned lge_pseudo_battery_mode_f(int val);
/* LGE_CHANGE_S  : adiyoung.lee, FTM Mode and ManualModeCkeckComplete on RPC, 2012-12-12 */
#if defined(CONFIG_MACH_MSM7X25A_V1)
unsigned lge_aat_partial_f(int val);
unsigned lge_aat_full_f(int val);
unsigned lge_aat_partial_or_full_f(int val);
#endif
/* LGE_CHANGE_E  : adiyoung.lee, FTM Mode and ManualModeCkeckComplete on RPC, 2012-12-12 */
#endif

