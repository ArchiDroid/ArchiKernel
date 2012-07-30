#ifndef __ASM_ARCH_MSM_BOARD_LGE_H
#define __ASM_ARCH_MSM_BOARD_LGE_H

#include <linux/i2c.h>
#include <linux/i2c-gpio.h>

#if __GNUC__
#define __WEAK __attribute__((weak))
#endif

#ifdef CONFIG_ANDROID_RAM_CONSOLE
/* allocate 128K * 2 instead of ram_console's original size 128K
 * this is for storing kernel panic log which is used by lk loader
 * 2010-03-03, cleaneye.kim@lge.com
 */
#define MSM7X27_EBI1_CS0_BASE	PHYS_OFFSET
#define LGE_RAM_CONSOLE_SIZE    (124 * SZ_1K * 2)
#endif

#ifdef CONFIG_LGE_HANDLE_PANIC
#define LGE_CRASH_LOG_SIZE              (4 * SZ_1K)
#endif

#define PMEM_KERNEL_EBI1_SIZE	0x3A000
#define MSM_PMEM_AUDIO_SIZE	0x5B000

/*LGE_CHANGE_S : seven.kim@lge.com kernel3.0 porting*/
#if 1	/*kernel3.0.8 code*/
#ifdef CONFIG_ARCH_MSM7X27A
#define MSM_PMEM_MDP_SIZE       			0x2300000
#define MSM7x25A_MSM_PMEM_MDP_SIZE		0x1500000

#define MSM_PMEM_ADSP_SIZE      			0x1100000
#define MSM7x25A_MSM_PMEM_ADSP_SIZE    0xB91000


#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
#define MSM_FB_SIZE							0x260000
#define MSM7x25A_MSM_FB_SIZE				0xE1000
#else
#define MSM_FB_SIZE							0x195000
#define MSM7x25A_MSM_FB_SIZE				0x96000

#endif

#endif

#else	/*kernel2.6.38 code*/
#ifdef CONFIG_ARCH_MSM7X27A
#define MSM_PMEM_MDP_SIZE       0x1DD1000
#define MSM_PMEM_ADSP_SIZE      0x1000000
#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
#define MSM_FB_SIZE             0x260000
#else
#define MSM_FB_SIZE             0x195000
#endif
#endif
#endif	/*end of kernel2.6.38 code*/
/*LGE_CHANGE_E : seven.kim@lge.com kernel3.0 porting*/

/* board revision information */
enum {
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
};

extern int lge_bd_rev;

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

#if defined(CONFIG_TOUCHSCREEN_MELFAS_MMS136) /*U0 Rev.b melfas touch*/
#define MELFAS_TS_NAME "melfas-ts"

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
#endif /*U0 Rev.b melfas touch*/

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
	int (*power)(unsigned char onoff);
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
	PANEL_ID_LGDISPLAY = 1,
};

struct msm_panel_ilitek_pdata {
	int gpio;
	int initialized;
	int maker_id;
	int (*lcd_power_save)(int);
};

void __init msm_msm7x2x_allocate_memory_regions(void);
void __init msm7x27a_reserve(void);

/* lge API functions to register i2c devices */

//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-07-28
unsigned lge_get_pif_info(void);
unsigned lge_get_lpm_info(void);

unsigned lge_get_batt_volt(void);
unsigned lge_get_chg_therm(void);
unsigned lge_get_pcb_version(void);
unsigned lge_get_chg_curr_volt(void);
unsigned lge_get_batt_therm(void);
unsigned lge_get_batt_volt_raw(void);
#if 1 //#ifdef CONFIG_MACH_MSM7X27_GELATO
unsigned lge_get_chg_stat_reg(void);
unsigned lge_get_chg_en_reg(void);
unsigned lge_set_elt_test(void);
unsigned lge_clear_elt_test(void);
//LGE_CHANGE_S, [hyo.park@lge.com] , 2011-10-10
unsigned lge_get_batt_id(void);
//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-10-10
#endif
//LGE_CHANGE_E, [hyo.park@lge.com] , 2011-07-28
unsigned lge_get_cable_info(void);
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
void lge_set_reboot_reason(unsigned int reason);
#endif
int __init lge_get_uart_mode(void);
#endif
