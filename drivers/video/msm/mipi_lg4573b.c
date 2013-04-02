/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#include <asm/atomic.h>
#include <linux/workqueue.h>
#include "mdp.h"
/* LGE_CHANGE_E : LCD ESD Protection*/ 
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_lg4573b.h"
#include <linux/gpio.h>
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#include <linux/irq.h>
/* LGE_CHANGE_E : LCD ESD Protection*/ 
#include <mach/vreg.h>
#include CONFIG_LGE_BOARD_HEADER_FILE

#define LG4573B_CMD_DELAY  0
/*LGE_CHANGE_S, youngbae.choi@lge.com, 13-01-03, for V7 lcd backlight timing code*/
#if defined(CONFIG_MACH_MSM8X25_V7)
extern int lcd_on_completed;
#endif
/*LGE_CHANGE_E, youngbae.choi@lge.com, 13-01-03, for V7 lcd backlight timing code*/

#ifndef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
static boolean lglogo_firstboot = TRUE;
#endif
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
//unsigned int irq_just_onetime=true;
extern bool b_normal_wakeup_started;
extern bool b_normal_sleep_started;
/*LGE_START: Kiran.kanneganti@lge.com 25-2-2012*/
/*In case of ESD no delays required in power off*/
boolean is_esd_occured = false;
/*LGE_END: Kiran.kanneganti@lge.com*/
static struct platform_device *local_pdev_for_pwm;
static struct lcd_esd_detect *lcd_esd;
int pwm_pin = 0;
int pwm_pin_high_or_low(const char *val, struct kernel_param *kp);
module_param_call(pwm_pin, pwm_pin_high_or_low, param_get_int,
					&pwm_pin, 0664);
static int drive_pwm_pin_to_low(void);
static int drive_pwm_pin_to_high(void);
static void esd_dsi_reset(void);
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/
/*LGE_CHANGE_S: Kiran.kanneganti@lge.com 05-03-2012*/
/*LCD Reset After data pulled Down*/
void mipi_ldp_lcd_panel_poweroff(void);
/*LGE_CHANGE_E LCD Reset After Data Pulled Down*/

static struct msm_panel_common_pdata *mipi_lg4573b_pdata;

static struct dsi_buf lg4573b_tx_buf;
static struct dsi_buf lg4573b_rx_buf;

/*LGE_CHANGE_S [yoonsoo.kim@lge.com] 20111004: Cosmo Video Mode LCD porting*/

#if 1 /* LGE_CHANGE_S  [yoonsoo.kim@lge.com] 20111004 :  */
static char config_mipi[] = {0x03,0x00};	/* MIPI DSI config */
#endif /* LGE_CHANGE_E [yoonsoo.kim@lge.com] 20111004 */
static char config_display_inversion[] = {0x20};	/* Display Inversion */
static char config_set_address_mode[] = {0x36,0x00};	 /* set address mode  */
static char config_interface_pixel_format[] = {0x3A,0x70};	 /* interface pixel format  */
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
/* LGE_CHANGE_S  [jongyoung.koo@lge.com] 20120102 :  */
static char config_cabc_51[] = {0x51,0xFF};	 /* LCD cabc code  */
static char config_cabc_53[] = {0x53,0x24};	 /* LCD cabc code  */
static char config_cabc_55[] = {0x55,0x00};	 /* LCD cabc code  */
static char config_cabc_5e[] = {0x5E,0x55};	 /* LCD cabc code  */
/* LGE_CHANGE_E [jongyoung.koo@lge.com] 20120102 */
static char config_cabc_51_reset_val[] = {0x51,0x00};/* LCD cabc code after reset*/
static char config_cabc_53_reset_val[] = {0x53,0x00};/* LCD cabc code after reset*/
static char config_cabc_55_reset_val[] = {0x55,0x00};/* LCD cabc code after reset*/
static char config_cabc_5e_reset_val[] = {0x5E,0x00};/* LCD cabc code after reset*/
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/ 
static char config_rgb_interface_setting[] = {0xB1,0x06,0x43,0x0A};	 /* rgb interface setting */
static char config_panel_char_setting[] = {0xB2, 0x00, 0xC8};	 /* Panel Characteristics Setting */
/*LGE_CHANGE_S : LCD updating panel parameters.HW team request
* 2012-02-20, kiran.kanneganti@lge.com
* Added LCD RGB interface parameters*/
#if 0 /*New tuning*/
static char config_panel_drv_setting[] = {0xB3, 0x00};	 /* Panel Drive Setting */
#else
static char config_panel_drv_setting[] = {0xB3, 0x02};	 /* Panel Drive Setting */
#endif
/* LGE_CHANGE_E : LCD panel parameters update*/
static char config_display_mode_ctl[] = {0xB4, 0x04};	/* Display Mode Control */
/*LGE_CHANGE_S : LCD updating panel parameters.HW team request
* 2012-02-20, kiran.kanneganti@lge.com
* Added LCD RGB interface parameters*/
#if 0 /*New tuning*/
static char config_display_ctl_1[] = {0xB5, 0x40, 0x18, 0x02, 0x00, 0x01};	 /* Display Control 1 */
#else
static char config_display_ctl_1[] = {0xB5, 0x40, 0x10, 0x10, 0x00, 0x00};	 /* Display Control 1 */
#endif
/* LGE_CHANGE_E : LCD panel parameters update*/
static char config_display_ctl_2[] = {0xB6, 0x0B, 0x0F, 0x02, 0x40, 0x10, 0xE8};	 /* Display Control 2 */
static char config_pwr_ctl_3[] = {0xC3, 0x07, 0x0A, 0x0A, 0x0A, 0x02};	/* Power Control 3 */
/*LGE_CHANGE_S : LCD updating panel parameters.HW team request
* 2012-02-20, kiran.kanneganti@lge.com
* Added LCD RGB interface parameters*/
#if 0 /*New tuning*/
static char config_pwr_ctl_4[] = {0xC4, 0x12, 0x24, 0x18, 0x18, 0x04, 0x49};	/* Power Control 4 */
static char config_pwr_ctl_5[] = {0xC5, 0x6B};	/* Power Control 5 */
static char config_pwr_ctl_6[] = {0xC6, 0x41, 0x63, 0x03};	/* Power Control 6 */
static char config_gamma_r_pos[] = {0xD0,0x11,0x36,0x67,0x01,0x00,0x00,0x20,0x00,0x02}; /* Positive Gamma Curve for Red */
static char config_gamma_r_neg[] = {0xD1,0x11,0x36,0x67,0x01,0x00,0x00,0x20,0x00,0x02}; /* Negative Gamma Curve for Red */
static char config_gamma_g_pos[] = {0xD2,0x11,0x36,0x67,0x01,0x00,0x00,0x20,0x00,0x02}; /* Positive Gamma Curve for Green */
static char config_gamma_g_neg[] = {0xD3,0x11,0x36,0x67,0x01,0x00,0x00,0x20,0x00,0x02}; /* Negative Gamma Curve for Green */
static char config_gamma_b_pos[] = {0xD4,0x11,0x36,0x67,0x01,0x00,0x00,0x20,0x00,0x02}; /* Positive Gamma Curve for Blue */
static char config_gamma_b_neg[] = {0xD5,0x11,0x36,0x67,0x01,0x00,0x00,0x20,0x00,0x02}; /* Negative Gamma Curve for Blue */	
#else
static char config_pwr_ctl_4[] = {0xC4, 0x12, 0x24, 0x18, 0x18, 0x02, 0x49};	/* Power Control 4 */
static char config_pwr_ctl_5[] = {0xC5, 0x6D};	/* Power Control 5 */
static char config_pwr_ctl_6[] = {0xC6, 0x42, 0x63, 0x03};	/* Power Control 6 */
static char config_gamma_r_pos[] = {0xD0,0x22,0x05,0x65,0x03,0x00,0x04,0x21,0x00,0x02}; /* Positive Gamma Curve for Red */
static char config_gamma_r_neg[] = {0xD1,0x22,0x05,0x65,0x03,0x00,0x04,0x21,0x00,0x02}; /* Negative Gamma Curve for Red */
static char config_gamma_g_pos[] = {0xD2,0x22,0x05,0x65,0x03,0x00,0x04,0x21,0x00,0x02}; /* Positive Gamma Curve for Green */
static char config_gamma_g_neg[] = {0xD3,0x22,0x05,0x65,0x03,0x00,0x04,0x21,0x00,0x02}; /* Negative Gamma Curve for Green */
static char config_gamma_b_pos[] = {0xD4,0x22,0x05,0x65,0x03,0x00,0x04,0x21,0x00,0x02}; /* Positive Gamma Curve for Blue */
static char config_gamma_b_neg[] = {0xD5,0x22,0x05,0x65,0x03,0x00,0x04,0x21,0x00,0x02}; /* Negative Gamma Curve for Blue */	
#endif
/* LGE_CHANGE_E : LCD panel parameters update*/
/*---------------------- display_on ----------------------------*/
static char disp_sleep_out[1] = {0x11};
static char disp_display_on[1] = {0x29};

/*---------------------- sleep_mode_on ----------------------------*/
static char sleep_display_off[1] = {0x28};
static char sleep_mode_on[1] = {0x10};

#define DTYPE_DCS_WRITE		0x05	/* short write, 0 parameter */
#define DTYPE_DCS_WRITE1	0x15	/* short write, 1 parameter */
#define DTYPE_DCS_READ		0x06	/* read */
#define DTYPE_DCS_LWRITE	0x39	/* long write */


static struct dsi_cmd_desc lg4573b_init_on_cmds[] = {
#if 1 /* LGE_CHANGE_S  [yoonsoo.kim@lge.com] 20111004 :  */
	{DTYPE_GEN_WRITE2, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_mipi), config_mipi},
#endif /* LGE_CHANGE_E [yoonsoo.kim@lge.com] 20111004 */
	{DTYPE_DCS_WRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_display_inversion), config_display_inversion},
	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_set_address_mode), config_set_address_mode},
	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_interface_pixel_format), config_interface_pixel_format},
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
	/* LGE_CHANGE_S  [jongyoung.koo@lge.com] 20120102 :  */
 	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
 		sizeof(config_cabc_51), config_cabc_51},
 	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
 		sizeof(config_cabc_53), config_cabc_53},
 	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
 		sizeof(config_cabc_55), config_cabc_55},
 	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
 		sizeof(config_cabc_5e), config_cabc_5e},
	/* LGE_CHANGE_E [jongyoung.koo@lge.com] 20120102 */
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/ 
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_rgb_interface_setting), config_rgb_interface_setting},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_panel_char_setting), config_panel_char_setting},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_panel_drv_setting), config_panel_drv_setting},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_display_mode_ctl), config_display_mode_ctl},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_display_ctl_1), config_display_ctl_1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_display_ctl_2), config_display_ctl_2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_pwr_ctl_3), config_pwr_ctl_3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_pwr_ctl_4), config_pwr_ctl_4},
	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_pwr_ctl_5), config_pwr_ctl_5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,	
		sizeof(config_pwr_ctl_6), config_pwr_ctl_6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_gamma_r_pos), config_gamma_r_pos},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_gamma_r_neg), config_gamma_r_neg},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_gamma_g_pos), config_gamma_g_pos},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_gamma_g_neg), config_gamma_g_neg},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_gamma_b_pos), config_gamma_b_pos},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_gamma_b_neg), config_gamma_b_neg},
	{DTYPE_DCS_LWRITE, 1, 0, 0, LG4573B_CMD_DELAY,
		sizeof(config_gamma_b_neg), config_gamma_b_neg}
};

static struct dsi_cmd_desc lg4573b_disp_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 40,
		sizeof(disp_display_on), disp_display_on}
};

//LGE_CHANGE_S [Kiran] Change LCD sleep sequence
static struct dsi_cmd_desc lg4573b_disp_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 40,
		sizeof(sleep_display_off), sleep_display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, /*LG4573B_CMD_DELAY*/10,
		sizeof(sleep_mode_on), sleep_mode_on},
};
//LGE_CHANGE_E [Kiran] Change LCD sleep sequence


static struct dsi_cmd_desc lg4573b_sleep_out_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(disp_sleep_out), disp_sleep_out},
};
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
static struct dsi_cmd_desc lg4573b_pwm_pin_low_cmds[] = {
 	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
 		sizeof(config_cabc_51_reset_val), config_cabc_51_reset_val},
 	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
 		sizeof(config_cabc_53_reset_val), config_cabc_53_reset_val},
 	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
 		sizeof(config_cabc_55_reset_val), config_cabc_55_reset_val},
 	{DTYPE_DCS_WRITE1, 1, 0, 0, LG4573B_CMD_DELAY,
 		sizeof(config_cabc_5e_reset_val), config_cabc_5e_reset_val},
};
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/ 
static int mipi_lg4573b_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
	int result=0;
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
	bool int_en_wq_ret;
	if ( (!local_pdev_for_pwm) && (pdev) )
	{
		local_pdev_for_pwm = pdev;
	} 
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/ 

	mfd = platform_get_drvdata(pdev);
	mipi  = &mfd->panel_info.mipi;

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	printk( "mipi_lg4573b_lcd_on START\n");
	
#ifndef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
	if(!lglogo_firstboot)
#endif
	{	
	#if 1//LGE_CHANGE_S [changbum.lee] 20120128 
	//jangsu.lee
// LGE_S, bohyun.jung@lge.com, 12-11-28 without this U0 JB does not go 1.8mA
#if 1 //defined(CONFIG_MACH_MSM7X27A_U0)
	udelay(500);//mdelay(1);//1
	gpio_set_value(GPIO_U0_LCD_RESET, 1);	
	msleep(10);	//10	
#endif
// LGE_E, bohyun.jung@lge.com, 12-11-28 without this U0 JB does not go 1.8mA
    #endif//LGE_CHANGE_E [changbum.lee] 20120128 
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
	/*If any work pending flush it & disable irq */	

/* LGE_CHANGE_S jungrock.oh@lge.com 2013-02-02 delete for ESD signal*/
#if !defined(CONFIG_MACH_MSM7X27A_U0)
	int_en_wq_ret = cancel_delayed_work_sync(&lcd_esd->esd_det_work);
	int_en_wq_ret = cancel_delayed_work_sync(&lcd_esd->esd_dsi_panel_on);
#endif
/* LGE_CHANGE_E jungrock.oh@lge.com 2013-02-02 delete for ESD signal*/
	int_en_wq_ret = flush_delayed_work_sync(&lcd_esd->esd_int_en_work);

	if( true == int_en_wq_ret)
	{
		printk("Pending INTR EN work Finished \n");
	}
	if( 1 == atomic_read(&lcd_esd->esd_irq_state))
	{
		disable_irq(lcd_esd->esd_irq);
		printk("ESD irq Disabled \n");
		atomic_set(&lcd_esd->esd_irq_state,0);
	}
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/ 

	mipi_set_tx_power_mode(1);
	result=mipi_dsi_cmds_tx(&lg4573b_tx_buf, lg4573b_init_on_cmds,
			ARRAY_SIZE(lg4573b_init_on_cmds));

	mdelay(10);
	
	result=mipi_dsi_cmds_tx(&lg4573b_tx_buf, lg4573b_sleep_out_cmds,
			ARRAY_SIZE(lg4573b_sleep_out_cmds));

/*LGE_CHANGE_S, youngbae.choi@lge.com, 12-12-28, for V7 reduce the display time*/
/* LGE_CHANGE_S jungrock.oh@lge.com 2013-02-02 delete for ESD signal*/
#if !defined(CONFIG_MACH_MSM8X25_V7) && !defined(CONFIG_MACH_MSM7X27A_U0)
/* LGE_CHANGE_E jungrock.oh@lge.com 2013-02-02 delete for ESD signal*/
	/*[LGSI_SP4_BSP_BEGIN] [kiran.jainapure@lge.com]: Sometimes display is blank or distorted during bootlogo*/
	if(lglogo_firstboot){
		mdelay(120);
	}
	/*[LGSI_SP4_BSP_END] [kiran.jainapure@lge.com]*/
#endif
/*LGE_CHANGE_E, youngbae.choi@lge.com, 12-12-28, for V7 reduce the display time*/
	
	result=mipi_dsi_cmds_tx(&lg4573b_tx_buf, lg4573b_disp_on_cmds,
			ARRAY_SIZE(lg4573b_disp_on_cmds));

	mipi_set_tx_power_mode(0);
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
	is_esd_occured = false;
#endif
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */	
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
/*Make Panel power off state to ZERO. So that esd irq can be enabled in int wq handler*/
	if(1 == atomic_read(&lcd_esd->panel_poweroff))
	{
		atomic_set(&lcd_esd->panel_poweroff,0);
	}
	/*Schedule work after 1 sec to enable ESD interrupt*/
	schedule_delayed_work(&lcd_esd->esd_int_en_work,ESD_INT_EN_DELAY);
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/ 

	printk( "mipi_lg4573b_lcd_on FINISH\n");
	}

	/*LGE_CHANGE_S, youngbae.choi@lge.com, 13-01-03, for V7 lcd backlight timing code*/
	#if defined(CONFIG_MACH_MSM8X25_V7)
	lcd_on_completed = 1;
	#endif
	/*LGE_CHANGE_E, youngbae.choi@lge.com, 13-01-03, for V7 lcd backlight timing code*/
	return 0;
}

static int mipi_lg4573b_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#ifdef CONFIG_LGE_LCD_ESD_DETECTION	
	bool int_en_wq_ret;
	bool panel_power_on_wq_ret;
	if ( (!local_pdev_for_pwm) && (pdev) )
	{
		local_pdev_for_pwm = pdev;
	}
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/ 

/*LGE_CHANGE_S, youngbae.choi@lge.com, 12-12-28, for V7 sometimes booting animation is no display*/
/* LGE_CHANGE_S jungrock.oh@lge.com 2013-01-15 add featuring for booting animation sometimes no display*/
#if defined(CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X27A_U0)
/* LGE_CHANGE_E jungrock.oh@lge.com 2013-01-15 add featuring for booting animation sometimes no display*/
#ifndef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
	if(lglogo_firstboot){
		lglogo_firstboot = FALSE;
		return 0;
	}
#endif
#endif
/*LGE_CHANGE_E, youngbae.choi@lge.com, 12-12-28, for V7 sometimes booting animation is no display*/

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	printk(KERN_INFO "mipi_lg4573b_chip_lcd_off START\n");
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
/*Set panel power off state to TRUE & flush the INTR EN Work queue*/
/*Don't enable esd irq in wq handler as we are powering off panel */	
	if( 0 == atomic_read(&lcd_esd->panel_poweroff) ) 
	{
		atomic_set(&lcd_esd->panel_poweroff,1);
	}
	int_en_wq_ret = flush_delayed_work_sync(&lcd_esd->esd_int_en_work);
	if ( true == int_en_wq_ret )
		printk("Waited for intr work to finish \n");
	panel_power_on_wq_ret = flush_delayed_work_sync(&lcd_esd->esd_dsi_panel_on);
	if( true == panel_power_on_wq_ret )
		printk("Waited for Panel On work to finish \n");

/*Disable ESD interrupt while powering off*/
	if( 1 == atomic_read(&lcd_esd->esd_irq_state))
	{	
		disable_irq(lcd_esd->esd_irq); 
		printk("ESD irq Disabled \n");
		atomic_set(&lcd_esd->esd_irq_state,0);
	}
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/ 


//LGE_CHANGE_S [Kiran] Change LCD sleep sequence
	//display off
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
	if(false == is_esd_occured)
#endif
	{
		mipi_dsi_cmds_tx(&lg4573b_tx_buf, lg4573b_disp_off_cmds,
			ARRAY_SIZE(lg4573b_disp_off_cmds));

		msleep(40);
	}
//LGE_CHANGE_E [Kiran] Change LCD sleep sequence
/*LGE_CHANGE_S: Kiran.kanneganti@lge.com 05-03-2012*/
/*LCD Reset After data pulled Down*/
#if 0
	mipi_ldp_lcd_panel_poweroff();
#endif
/*LGE_CHANGE_E LCD Reset After Data Pulled Down*/
#ifndef CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO
	lglogo_firstboot = FALSE;
#endif
/* LGE_CHANGE_S jungrock.oh@lge.com 2013-02-02 add for ESD signal*/
/* LGE_CHANGE_ E jungrock.oh@lge.com 2013-02-02 add for ESD signal*/
	return 0;
}

ssize_t mipi_lg4573b_lcd_show_onoff(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	printk( "%s : start\n", __func__);
	return 0;
}

ssize_t mipi_lg4573b_lcd_store_onoff(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	/*struct platform_device dummy_pdev;*/
	int onoff;

	sscanf(buf, "%d", &onoff);
	printk( "%s: onoff : %d\n", __func__, onoff);
	if (onoff)
		mipi_lg4573b_lcd_on(NULL);
	else
		mipi_lg4573b_lcd_off(NULL);

	return count;
}

DEVICE_ATTR(lcd_onoff, 0664, mipi_lg4573b_lcd_show_onoff,
	mipi_lg4573b_lcd_store_onoff);
	
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
/********************************************************************
Function Name  :-  drive_pwm_pin_to_low
Arguments 	   :-  None
Return Value   :-  int / Always Success.
Functionality  :-  to drive PWM pin of lcd panel to LOW.  
dependencies   :-  Panel Should be already ON.
 Others        :-  By writing 1 to pwm_pin parameter (module/mipi_lg4573/)
 					we can simulate low on pwm pin.
			For Software ESD simulation Testing.		
*********************************************************************/
static int drive_pwm_pin_to_low()
{
	struct msm_fb_data_type *mfd;
	int ret = 0;
	printk("%s Enter \n",__func__);
	mfd = platform_get_drvdata(local_pdev_for_pwm);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
	mipi_dsi_op_mode_config(DSI_CMD_MODE);
	mipi_dsi_cmds_tx(&lg4573b_tx_buf, lg4573b_pwm_pin_low_cmds,
			ARRAY_SIZE(lg4573b_pwm_pin_low_cmds));
	mipi_dsi_op_mode_config(DSI_VIDEO_MODE);

	printk("%s Exit \n",__func__);
	return ret;
}

/********************************************************************
Function Name  :-  drive_pwm_pin_to_high
Arguments 	   :-  None
Return Value   :-  int / Always Success.
Functionality  :-  Power off / reset / power on / lcd panel init.
dependencies   :-  Panel Should be already ON.
 Others        :-  By writing 0 to pwm_pin parameter (module/mipi_lg4573/)
 					we can re-initialize panel.
 			For Software ESD simulation Testing.
*********************************************************************/
static int drive_pwm_pin_to_high()
{
	struct msm_fb_data_type *mfd;
	int ret = 0;
	printk("%s Enter \n",__func__);
	mfd = platform_get_drvdata(local_pdev_for_pwm);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
	mipi_dsi_op_mode_config(DSI_CMD_MODE);
	mipi_dsi_cmds_tx(&lg4573b_tx_buf, lg4573b_disp_off_cmds,
			ARRAY_SIZE(lg4573b_disp_off_cmds));

	mipi_ldp_lcd_panel_poweroff();//reset pin
	esd_sw_test_lcd_panel_power_off(); //power
	
	mdelay(500);

	esd_sw_test_lcd_panel_power_on(); //panel power on
	
	mipi_set_tx_power_mode(1);
	mipi_dsi_cmds_tx(&lg4573b_tx_buf, lg4573b_init_on_cmds,
			ARRAY_SIZE(lg4573b_init_on_cmds));
	mdelay(10);
	mipi_dsi_cmds_tx(&lg4573b_tx_buf, lg4573b_sleep_out_cmds,
			ARRAY_SIZE(lg4573b_sleep_out_cmds));
	mdelay(120);
	mipi_dsi_cmds_tx(&lg4573b_tx_buf, lg4573b_disp_on_cmds,
			ARRAY_SIZE(lg4573b_disp_on_cmds));
	mipi_set_tx_power_mode(0);
	
	mipi_dsi_op_mode_config(DSI_VIDEO_MODE); //back to video mode.

	printk("%s Exit \n",__func__);

	return ret;
}

/********************************************************************
Function Name  :-  pwm_pin_high_or_low
Arguments 	   :-  None
Return Value   :-  None/Always Success.
Functionality  :-  to Simulate ESD using Module Parameter.  
dependencies   :-  Should be called when lcd panel is on.
Others		   :- pwm_pin --> 1 ( PWM_PIN low & esd isr active )
			      pwm_pin --> 2 ( Dsi reset )
*********************************************************************/
int pwm_pin_high_or_low(const char *val, struct kernel_param *kp)
{
	unsigned long pwm_pin;
	int rc = 0;
	
	if (val == NULL)
	{
		printk("%s Null Buf \n",__func__);
		return -1;
	}
	printk("%s enter \n",__func__);
	pwm_pin = simple_strtoul(val,NULL,10);
	if ( 1 == pwm_pin)
	{
		rc = drive_pwm_pin_to_low();	//ESD Occur	
		if ( rc )
		{
			printk("Error while making pwm pin Low \n");
		}
	}
	else if ( 2 == pwm_pin )
	{
		esd_dsi_reset();
		printk("Dsi Reset Done \n");
	}
	else
	{
		rc = drive_pwm_pin_to_high(); //Normal
		if ( rc )
		{
			printk("Error while making pwm pin High \n");
		}
	}
        return 0;
}
/********************************************************************
Function Name  :-  esd_dsi_panel_on_wq_handler
Arguments 	   :-  Work Q
Return Value   :-  None
Functionality  :-  to power on panel.  
dependencies   :-  Panel should be already intialized.
*********************************************************************/
static void esd_dsi_panel_on_wq_handler(struct work_struct *w)
{
/*LGE_START: Kiran.kanneganti@lge.com 25-2-2012*/
/*In case of ESD no delays required in power off*/
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(local_pdev_for_pwm);
	/* LGE_CHANGE_S jungrock.oh@lge.com 2013-02-02 modify for ESD signal*/
	#if !defined(CONFIG_MACH_MSM7X27A_U0)
	if ( b_normal_wakeup_started || b_normal_sleep_started )
	#else
	if ( (atomic_read(&lcd_esd->panel_poweroff) == 0) || b_normal_wakeup_started || b_normal_sleep_started )
	#endif
	/* LGE_CHANGE_E jungrock.oh@lge.com 2013-02-02 modify for ESD signal*/
	{
		is_esd_occured = false;
		printk("Panel on/off started in FB !! \n");
		return;
	}	
	esd_dma_dsi_panel_on();
	mfd->panel_power_on = true;
	is_esd_occured = false;
/*LGE_END: Kiran.kanneganti@lge.com*/
}

/********************************************************************
Function Name  :-  esd_dsi_reset
Arguments 	   :-  None
Return Value   :-  None
Functionality  :-  to power off panel & schedule work to power on (1sec).  
dependencies   :-  Panel should be already intialized.
*********************************************************************/
static void esd_dsi_reset()
{
/*LGE_START: Kiran.kanneganti@lge.com 25-2-2012*/
/*In case of ESD no delays required in power off*/
	struct msm_fb_data_type *mfd;
	printk("Doing dma dsi reset \n");
	mfd = platform_get_drvdata(local_pdev_for_pwm);
	is_esd_occured = true;
	mfd->panel_power_on = false;
	esd_dma_dsi_panel_off();
	mdp_pipe_ctrl(MDP_MASTER_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
	msleep(20);
	schedule_delayed_work(&lcd_esd->esd_dsi_panel_on,ESD_PANEL_ON_DELAY);
/*LGE_END: Kiran.kanneganti@lge.com*/
}

/********************************************************************
Function Name  :-  esd_detect_handler
Arguments 	   :-  IRQ & device ID
Return Value   :-  IRQ status
Functionality  :-  Scheduled a work after 200ms for disabling IRQ &
					to RESET mdp dma,dsi & panel.  
dependencies   :-  Panel should be already intialized.
*********************************************************************/
static irqreturn_t esd_detect_handler(int irq, void *dev_id)
{
/*LGE_START: Kiran.kanneganti@lge.com 25-2-2012*/
/*In case of ESD no delays required in power off*/

/*LGE_CHANGE_S byungyong.hwang - firstbooting is not and panel should be power-on state excute irq-handler*/
	if((!lglogo_firstboot) && ( 0 == atomic_read(&lcd_esd->panel_poweroff))){
/*LGE_CHANGE_E byungyong.hwang - firstbooting is not and panel should be power-on state excute irq-handler*/
		schedule_delayed_work(&lcd_esd->esd_det_work,ESD_HANDLE_DELAY);
	}
	return IRQ_HANDLED;
/*LGE_END: Kiran.kanneganti@lge.com*/
}

/********************************************************************
Function Name  :-  esd_detect_wq_handler
Arguments 	   :-  Work queue
Return Value   :-  None
Functionality  :-  to disable ESD Irq & issue a dma reset.  
dependencies   :-  Panel should be already intialized.
Others 		   :-  This will be called after 200ms of interrupt occurrence.
*********************************************************************/
static void  esd_detect_wq_handler(struct work_struct *w)
{
	if(gpio_get_value_cansleep(lcd_esd->esd_detect_gpio) == GPIO_U0_LCD_ESD_DETECT_PIN_IS_LOW)
	{
		if( 1 == atomic_read(&lcd_esd->esd_irq_state))
		{

			disable_irq(lcd_esd->esd_irq);
			printk("ESD irq Disabled \n");
			atomic_set(&lcd_esd->esd_irq_state,0);
		}
		printk("%s Enter \n",__func__);
		esd_dsi_reset();	
	}
	else /*GPIO_U0_LCD_ESD_DETECT_PIN_IS_HIGH ??*/
	{
		printk("%s LCD_PWM pin is high - do nothing \n",__func__);
	}		
}

/********************************************************************
Function Name  :-  esd_int_en_wq_handler
Arguments 	   :-  Work queue
Return Value   :-  None
Functionality  :-  to enable ESD Irq.  
dependencies   :-  Panel should be already intialized.
Others 		   :-  This will be called after 1sec of lcd ON.
*********************************************************************/
static void esd_int_en_wq_handler(struct work_struct *w)
{
	printk("%s Enter \n",__func__);
	if( 1 == atomic_read(&lcd_esd->panel_poweroff))
	{
		/*Panel in power off state, No need to enable interrupt*/
		return;
	}
/*LGE_START: Kiran.kanneganti@lge.com 25-2-2012*/
/*In case of ESD no delays required in power off*/
	if ( b_normal_wakeup_started || b_normal_sleep_started )
	{
		/*In case of normal suspend/resume start don't enable interrupt*/
		return;
	}

	if( 0 == atomic_read(&lcd_esd->esd_irq_state))
	{
		printk("ESD irq Enabled \n");
		enable_irq(lcd_esd->esd_irq);
		atomic_set(&lcd_esd->esd_irq_state,1); 
	}
/*LGE_END: Kiran.kanneganti@lge.com*/
}
#endif

static int __devinit mipi_lg4573b_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;

	if (pdev->id == 0) {
		mipi_lg4573b_pdata = pdev->dev.platform_data;
		return 0;
	}
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */ 
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
    	lcd_esd = kzalloc(sizeof(struct lcd_esd_detect), GFP_KERNEL );
	if (!lcd_esd)
	{
		pr_err("Failed to allocate memory for LCD ESD info");
		goto skip_esd_detection;
	}

	lcd_esd->esd_detect_gpio = GPIO_U0_LCD_ESD_DETECT;
	lcd_esd->name = pdev->name;
	/*request to use pwm gpio*/
	rc = gpio_request(lcd_esd->esd_detect_gpio, lcd_esd->name);
	if (0 > rc)
	{
		pr_err("failed to request GPIO %d \n",lcd_esd->esd_detect_gpio);
		goto skip_esd_detection;
	}
	/*Configure as input*/
	rc = gpio_direction_input(lcd_esd->esd_detect_gpio);
	if (0 > rc)
	{
		pr_err("failed to set GPIO as input%d \n",lcd_esd->esd_detect_gpio);
		gpio_free(lcd_esd->esd_detect_gpio);
		goto skip_esd_detection;
	}
	
	rc = gpio_tlmm_config(GPIO_CFG(lcd_esd->esd_detect_gpio,0,GPIO_CFG_INPUT,GPIO_CFG_NO_PULL,GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if( 0 > rc)
	{
		pr_err("failed to configure tlmm for %d \n",lcd_esd->esd_detect_gpio);
		gpio_free(lcd_esd->esd_detect_gpio);
		goto skip_esd_detection;
	}
    /*Get IRQ number*/
	lcd_esd->esd_irq = gpio_to_irq(lcd_esd->esd_detect_gpio);
	if( 0 > lcd_esd->esd_irq )
	{
		pr_err("Failed to get interrupt \n");
		gpio_free(lcd_esd->esd_detect_gpio);
		goto skip_esd_detection;
	}
	/*request for interrupt on low level*/
	rc = request_irq(lcd_esd->esd_irq,esd_detect_handler,IRQF_TRIGGER_FALLING|IRQF_ONESHOT,lcd_esd->name,NULL);
	if ( 0 > rc )
	{
		pr_err("failed to register interrupt Handler %d \n", lcd_esd->esd_irq);
		gpio_free(lcd_esd->esd_detect_gpio);
		goto skip_esd_detection;
	}
	/*Enable Interrupt*/
#if 0 /*No  irq request in sleep */	
	rc = irq_set_irq_wake(lcd_esd->esd_irq, 1);
	if ( 0 > rc )
	{
		pr_err("Failed to set irq wake \n");
		free_irq(lcd_esd->esd_irq,0);
		gpio_free(lcd_esd->esd_detect_gpio);
		goto skip_esd_detection;
	}
#endif

	atomic_set(&lcd_esd->esd_irq_state, 1);
	atomic_set(&lcd_esd->panel_poweroff,0);
	/*Work queue to serve interrupt*/
	INIT_DELAYED_WORK(&lcd_esd->esd_det_work, esd_detect_wq_handler);	
	/*Work queue to enable interrupt*/
	INIT_DELAYED_WORK(&lcd_esd->esd_int_en_work, esd_int_en_wq_handler);
	/*Work Queue to power on MDP, DSI & Panel*/
	INIT_DELAYED_WORK(&lcd_esd->esd_dsi_panel_on, esd_dsi_panel_on_wq_handler);
	printk("ESD interrupts registered successfully \n");

skip_esd_detection:
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/ 
	msm_fb_add_device(pdev);
	/*this for AT Command*/
	rc = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);
	
	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_lg4573b_lcd_probe,
	.driver = {
		.name   = "mipi_lg4573b",
	},
};

static struct msm_fb_panel_data lg4573b_panel_data = {
	.on		= mipi_lg4573b_lcd_on,
	.off	= mipi_lg4573b_lcd_off,
};

static int ch_used[3];

int mipi_lg4573b_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;
	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_lg4573b", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	lg4573b_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &lg4573b_panel_data,
		sizeof(lg4573b_panel_data));
	if (ret) {
		pr_err("%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		pr_err("%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

#ifdef CONFIG_FB_MSM_MIPI_DSI_HX8379A
extern unsigned int maker_id;
#endif
static int __init mipi_lg4573b_lcd_init(void)
{
	int bufsize=0;
	
	bufsize=mipi_dsi_buf_alloc(&lg4573b_tx_buf, DSI_BUF_SIZE);
	
	bufsize=mipi_dsi_buf_alloc(&lg4573b_rx_buf, DSI_BUF_SIZE);
	
#ifdef CONFIG_FB_MSM_MIPI_DSI_HX8379A
	if( maker_id == 0) //Tovis
		return 0;
	else{
#endif
		return platform_driver_register(&this_driver);
#ifdef CONFIG_FB_MSM_MIPI_DSI_HX8379A
		}
#endif
}
/*LGE_CHANGE_S: Kiran.kanneganti@lge.com 05-03-2012*/
/*LCD Reset After data pulled Down*/
void mipi_ldp_lcd_panel_poweroff(void)
{
// LGE_S, bohyun.jung@lge.com, 12-11-28 without this U0 JB does not go 1.8mA
#if 1//defined(CONFIG_MACH_MSM7X27A_U0)
	gpio_set_value(GPIO_U0_LCD_RESET, 0);  /* LGE_CHANGE  [yoonsoo.kim@lge.com] 20110906: LCD Pinname */
#if defined(CONFIG_MACH_MSM8X25_V7)
	msleep(40);
#else
	msleep(20);
#endif
#endif
// LGE_E, bohyun.jung@lge.com, 12-11-28 without this U0 JB does not go 1.8mA
}
EXPORT_SYMBOL(mipi_ldp_lcd_panel_poweroff);
/*LGE_CHANGE_E LCD Reset After Data Pulled Down*/
module_init(mipi_lg4573b_lcd_init);
