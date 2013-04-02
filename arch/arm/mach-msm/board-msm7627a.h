/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __ARCH_ARM_MACH_MSM_BOARD_7627A__
#define __ARCH_ARM_MACH_MSM_BOARD_7627A__

#include "pm.h"
void __init msm7627a_init_mmc(void);

void __init msm_msm7627a_allocate_memory_regions(void);
void __init msm_fb_add_devices(void);

enum {
	GPIO_EXPANDER_IRQ_BASE  = NR_MSM_IRQS + NR_GPIO_IRQS,
	GPIO_EXPANDER_GPIO_BASE = NR_MSM_GPIOS,
	/* SURF expander */
	GPIO_CORE_EXPANDER_BASE = GPIO_EXPANDER_GPIO_BASE,
	GPIO_BT_SYS_REST_EN     = GPIO_CORE_EXPANDER_BASE,
	GPIO_WLAN_EXT_POR_N,
	GPIO_DISPLAY_PWR_EN,
	GPIO_BACKLIGHT_EN,
	GPIO_PRESSURE_XCLR,
	GPIO_VREG_S3_EXP,
	GPIO_UBM2M_PWRDWN,
	GPIO_ETM_MODE_CS_N,
	GPIO_HOST_VBUS_EN,
	GPIO_SPI_MOSI,
	GPIO_SPI_MISO,
	GPIO_SPI_CLK,
	GPIO_SPI_CS0_N,
	GPIO_CORE_EXPANDER_IO13,
	GPIO_CORE_EXPANDER_IO14,
	GPIO_CORE_EXPANDER_IO15,
	/* Camera expander */
	GPIO_CAM_EXPANDER_BASE  = GPIO_CORE_EXPANDER_BASE + 16,
	GPIO_CAM_GP_STROBE_READY	= GPIO_CAM_EXPANDER_BASE,
	GPIO_CAM_GP_AFBUSY,
	GPIO_CAM_GP_CAM_PWDN,
	GPIO_CAM_GP_CAM1MP_XCLR,
	GPIO_CAM_GP_CAMIF_RESET_N,
	GPIO_CAM_GP_STROBE_CE,
	GPIO_CAM_GP_LED_EN1,
	GPIO_CAM_GP_LED_EN2,
};
/* LGE_CHANGE  [yoonsoo.kim@lge.com] 20120131 : U0 700 Rev.F Base*/
#ifdef CONFIG_LGE_HEADSET_DETECTION_FSA8008
// 20120914 - error
/* LGE_CHANGE [ew0804.kim] fsa8008 gpio */
#define GPIO_EAR_SENSE_N          41
#define GPIO_EARPOL_DETECT        4
#define GPIO_EAR_MIC_EN           127
/* 2012-09-27 JongKwang-Lee(jongkwang.lee@lge.com) [V3] change HOOK Button  on V3 Rev.C [START] */
#ifdef CONFIG_MACH_MSM7X25A_V3BR_REV_C
#define GPIO_EAR_KEY_INT          28
#else
#define GPIO_EAR_KEY_INT          29
#endif
/* 2012-09-27 JongKwang-Lee(jongkwang.lee@lge.com) [V3] change HOOK Button  on V3 Rev.C [END] */
#else
#define GPIO_EAR_SENSE			41
#define GPIO_BUTTON_DETECT		29
#define GPIO_MIC_MODE			127
#endif


enum {
	QRD_GPIO_HOST_VBUS_EN       = 107,
	QRD_GPIO_BT_SYS_REST_EN     = 114,
	QRD_GPIO_WAKE_ON_WIRELESS,
	QRD_GPIO_BACKLIGHT_EN,
	QRD_GPIO_NC,
	QRD_GPIO_CAM_3MP_PWDN,      /* CAM_VGA */
	QRD_GPIO_WLAN_EN,
	QRD_GPIO_CAM_5MP_SHDN_EN,
	QRD_GPIO_CAM_5MP_RESET,
	QRD_GPIO_TP,
	QRD_GPIO_CAM_GP_CAMIF_RESET,
};

#define ADSP_RPC_PROG           0x3000000a
#if defined(CONFIG_BT) && defined(CONFIG_MARIMBA_CORE)

#define FPGA_MSM_CNTRL_REG2 0x90008010
#define BAHAMA_SLAVE_ID_FM_REG 0x02
#define BAHAMA_SLAVE_ID_FM_ADDR  0x2A
#define BAHAMA_SLAVE_ID_QMEMBIST_ADDR   0x7B
#define FM_GPIO 83
#define BT_PCM_BCLK_MODE  0x88
#define BT_PCM_DIN_MODE   0x89
#define BT_PCM_DOUT_MODE  0x8A
#define BT_PCM_SYNC_MODE  0x8B
#define FM_I2S_SD_MODE    0x8E
#define FM_I2S_WS_MODE    0x8F
#define FM_I2S_SCK_MODE   0x90
#define I2C_PIN_CTL       0x15
#define I2C_NORMAL	  0x40

/* LGE_CHANGE_S : 2012-09-15 sungmin.cho@lge.com flash led porting */
#ifdef CONFIG_LEDS_AS364X
#define GPIO_FLASH_I2C_SCL  	27
#define GPIO_FLASH_I2C_SDA  	26
#define FLASH_I2C_ADDRESS  	0x30
#endif
/* LGE_CHANGE_E : 2012-09-15 sungmin.cho@lge.com flash led porting */

struct bahama_config_register {
	u8 reg;
	u8 value;
	u8 mask;
};

struct bt_vreg_info {
	const char *name;
	unsigned int pmapp_id;
	unsigned int min_level;
	unsigned int max_level;
	unsigned int is_pin_controlled;
	struct regulator *reg;
};

void __init msm7627a_bt_power_init(void);
#endif

/*LGE_CHANGE_S : seven.kim@lge.com 2.0PL pre-cs merge , Fix build error*/
#if 0
extern struct platform_device msm_device_snd;
extern struct platform_device msm_device_adspdec;
extern struct platform_device msm_device_cad;
#endif 
/*LGE_CHANGE_E : seven.kim@lge.com 2.0PL pre-cs merge*/

void __init msm7627a_camera_init(void);
int lcd_camera_power_onoff(int on);

void __init msm7627a_add_io_devices(void);
void __init qrd7627a_add_io_devices(void);
#endif
