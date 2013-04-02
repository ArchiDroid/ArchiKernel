/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include "msm_fb.h"

#include <linux/memory.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include "linux/proc_fs.h"

#include <linux/delay.h>

#include <mach/hardware.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <mach/vreg.h>
#include CONFIG_LGE_BOARD_HEADER_FILE
#include <linux/pm_qos_params.h>

#ifdef CONFIG_MACH_MSM7X25A_V1	
	#include <mach/lge/lge_proc_comm.h>
	
#endif

#define QVGA_WIDTH        240
#define QVGA_HEIGHT       320

#define  GPIO_LCD_TID      126

static void *DISP_CMD_PORT;
static void *DISP_DATA_PORT;

#define EBI2_WRITE16C(x, y) outpw(x, y)
#define EBI2_WRITE16D(x, y) outpw(x, y)
#define EBI2_READ16(x) inpw(x)

static boolean disp_initialized = FALSE;
struct msm_fb_panel_data tovis_qvga_panel_data;
struct msm_panel_ilitek_pdata *tovis_qvga_panel_pdata;
struct pm_qos_request_list *tovis_pm_qos_req;

/* For some reason the contrast set at init time is not good. Need to do
* it again
*/

/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
#if 0
static boolean display_on = FALSE;
#else
#ifdef CONFIG_MACH_MSM7X25A_V1	

extern int display_on; 
#else
int display_on = FALSE; 
#endif
#endif
/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */

/*[2012-12-20][junghoon79.kim@lge.com] boot time reduction [START]*/
#define LCD_INIT_SKIP_FOR_BOOT_TIME
/*[2012-12-20][junghoon79.kim@lge.com] boot time reduction [END]*/
#ifdef LCD_INIT_SKIP_FOR_BOOT_TIME
int lcd_init_skip_cnt = 0;
#endif


/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-07] :SE 85591 remove white screen during power on */
#define LCD_RESET_SKIP 1
int IsFirstDisplayOn = LCD_RESET_SKIP; 
/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-07] :SE 85591 remove white screen during power on */

#define DISP_SET_RECT(csp, cep, psp, pep) \
	{ \
		EBI2_WRITE16C(DISP_CMD_PORT, 0x2a);			\
		EBI2_WRITE16D(DISP_DATA_PORT,(csp)>>8);		\
		EBI2_WRITE16D(DISP_DATA_PORT,(csp)&0xFF);	\
		EBI2_WRITE16D(DISP_DATA_PORT,(cep)>>8);		\
		EBI2_WRITE16D(DISP_DATA_PORT,(cep)&0xFF);	\
		EBI2_WRITE16C(DISP_CMD_PORT, 0x2b);			\
		EBI2_WRITE16D(DISP_DATA_PORT,(psp)>>8);		\
		EBI2_WRITE16D(DISP_DATA_PORT,(psp)&0xFF);	\
		EBI2_WRITE16D(DISP_DATA_PORT,(pep)>>8);		\
		EBI2_WRITE16D(DISP_DATA_PORT,(pep)&0xFF);	\
	}


#ifdef TUNING_INITCODE
module_param(te_lines, uint, 0644);
module_param(mactl, uint, 0644);
#endif

#ifdef CONFIG_MACH_MSM7X25A_V1
extern	int lge_rt8966a_backlight_control( int onoff );
#endif

static void tovis_qvga_disp_init(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	if (disp_initialized)
		return;

	mfd = platform_get_drvdata(pdev);

	DISP_CMD_PORT = mfd->cmd_port;
	DISP_DATA_PORT = mfd->data_port;

	disp_initialized = TRUE;
}

static void msm_fb_ebi2_power_save(int on)
{
	struct msm_panel_ilitek_pdata *pdata = tovis_qvga_panel_pdata;

	if(pdata && pdata->lcd_power_save)
		pdata->lcd_power_save(on);
}

#ifdef CONFIG_MACH_MSM7X25A_V1	
int StatusBacklightOnOff = 1;
#endif

static int ilitek_qvga_disp_off(struct platform_device *pdev)
{

/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
#if 1
	struct msm_panel_ilitek_pdata *pdata = tovis_qvga_panel_pdata;
#endif
/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */

#ifdef CONFIG_MACH_MSM7X25A_V1	
	//do not control when suspend after just working chargerlogo
		if(StatusBacklightOnOff==1) {
	printk("%s: display off...\n", __func__);
			lge_rt8966a_backlight_control(0);
			StatusBacklightOnOff = 0;
			IsFirstDisplayOn = 0;
		}
#endif

	if (!disp_initialized)
		tovis_qvga_disp_init(pdev);

#ifndef CONFIG_ARCH_MSM7X27A
	pm_qos_update_request(tovis_pm_qos_req, PM_QOS_DEFAULT_VALUE);
#endif

	EBI2_WRITE16C(DISP_CMD_PORT, 0x28);
	msleep(50);
	EBI2_WRITE16C(DISP_CMD_PORT, 0x10); // SPLIN
	msleep(120);

/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
#if 1 
	if(pdata->gpio)
		gpio_set_value(pdata->gpio, 0);
#endif	
/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */

	msm_fb_ebi2_power_save(0);
	display_on = FALSE;

	return 0;
}

static void ilitek_qvga_disp_set_rect(int x, int y, int xres, int yres) // xres = width, yres - height
{
	if (!disp_initialized)
		return;

	DISP_SET_RECT(x, x+xres-1, y, y+yres-1);
	EBI2_WRITE16C(DISP_CMD_PORT,0x2c); // Write memory start
}

#ifdef CONFIG_MACH_MSM7X25A_V1
static void panel_CMIDisplay_init(void)
{


	EBI2_WRITE16C(DISP_CMD_PORT, 0x11); //Exit Sleep
	msleep(120);

    EBI2_WRITE16C(DISP_CMD_PORT,0x3A);
                EBI2_WRITE16D(DISP_DATA_PORT,0x55);

				
	EBI2_WRITE16C(DISP_CMD_PORT, 0xCF); //EXTC option
	EBI2_WRITE16D(DISP_DATA_PORT, 0x00);
	EBI2_WRITE16D(DISP_DATA_PORT, 0xA1);
	EBI2_WRITE16C(DISP_CMD_PORT, 0xB1); //
	EBI2_WRITE16D(DISP_DATA_PORT, 0x00);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x1E); //frame rate control
	EBI2_WRITE16C(DISP_CMD_PORT, 0XB4); //inversion control
	EBI2_WRITE16D(DISP_DATA_PORT, 0x02); //2 dots inversion
	EBI2_WRITE16C(DISP_CMD_PORT, 0XB6);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x02);
	EBI2_WRITE16C(DISP_CMD_PORT, 0XC0); //power control
	EBI2_WRITE16D(DISP_DATA_PORT, 0x0F); //
	EBI2_WRITE16D(DISP_DATA_PORT, 0x0D); //
	EBI2_WRITE16C(DISP_CMD_PORT, 0xC1); //power control
	EBI2_WRITE16D(DISP_DATA_PORT, 0x01); //VGH =6x VCI VGL= -5xVCI
	EBI2_WRITE16C(DISP_CMD_PORT, 0xC2); //power control
	EBI2_WRITE16D(DISP_DATA_PORT, 0x33);
	EBI2_WRITE16C(DISP_CMD_PORT, 0xC5); // VCOMDC=-1
	EBI2_WRITE16D(DISP_DATA_PORT, 0xE7);

	/* Blanking Porch control*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xb5);
	EBI2_WRITE16D(DISP_DATA_PORT,0x02);
	EBI2_WRITE16D(DISP_DATA_PORT,0x02);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a);
	EBI2_WRITE16D(DISP_DATA_PORT,0x14);

	
	/* Tearing Effect Line On */
	EBI2_WRITE16C(DISP_CMD_PORT, 0x35);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);

	/* Tearing effect Control Parameter */
	EBI2_WRITE16C(DISP_CMD_PORT, 0x44);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0xef);

	EBI2_WRITE16C(DISP_CMD_PORT, 0xE0); //Set Gamma
	EBI2_WRITE16D(DISP_DATA_PORT, 0x05);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x08);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x0D);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x07);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x10);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x08);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x33);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x35);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x45);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x04);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x0B);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x08);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x1A);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x1D);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x0F);

	EBI2_WRITE16C(DISP_CMD_PORT, 0XE1); //Set Gamma
	EBI2_WRITE16D(DISP_DATA_PORT, 0x06);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x23);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x26);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x00);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x0C);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x01);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x39);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x02);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x4A);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x02);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x0C);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x07);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x31);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x36);
	EBI2_WRITE16D(DISP_DATA_PORT, 0x0F);
	EBI2_WRITE16C(DISP_CMD_PORT, 0x29); //display on

}

static void panel_tianmaDisplay_init(void)
{
	
	
	 EBI2_WRITE16C(DISP_CMD_PORT, 0xCB);
                EBI2_WRITE16D(DISP_DATA_PORT, 0x39);
                EBI2_WRITE16D(DISP_DATA_PORT, 0x2C);
                EBI2_WRITE16D(DISP_DATA_PORT, 0x00);
                EBI2_WRITE16D(DISP_DATA_PORT, 0x34);   //Vcore 1.6V
                EBI2_WRITE16D(DISP_DATA_PORT, 0x02);   //AVDD 5.8V

	EBI2_WRITE16C(DISP_CMD_PORT,0xCF);
                EBI2_WRITE16D(DISP_DATA_PORT,0x00);
                EBI2_WRITE16D(DISP_DATA_PORT,0xd9);//83
                EBI2_WRITE16D(DISP_DATA_PORT,0X30);


    EBI2_WRITE16C(DISP_CMD_PORT,0xE8);
                EBI2_WRITE16D(DISP_DATA_PORT,0x85); 
                EBI2_WRITE16D(DISP_DATA_PORT,0x10); 	   //00
                EBI2_WRITE16D(DISP_DATA_PORT,0x78); 

    EBI2_WRITE16C(DISP_CMD_PORT,0xEA);
                EBI2_WRITE16D(DISP_DATA_PORT,0x00);
                EBI2_WRITE16D(DISP_DATA_PORT,0x00);

    EBI2_WRITE16C(DISP_CMD_PORT,0xED);
                EBI2_WRITE16D(DISP_DATA_PORT,0x64);
                EBI2_WRITE16D(DISP_DATA_PORT,0x03);
                EBI2_WRITE16D(DISP_DATA_PORT,0X12);
                EBI2_WRITE16D(DISP_DATA_PORT,0x81);

    EBI2_WRITE16C(DISP_CMD_PORT,0xF2);
                EBI2_WRITE16D(DISP_DATA_PORT,0x00);

    EBI2_WRITE16C(DISP_CMD_PORT,0xF7);
                EBI2_WRITE16D(DISP_DATA_PORT,0x20);

    EBI2_WRITE16C(DISP_CMD_PORT,0xC0);
                EBI2_WRITE16D(DISP_DATA_PORT,0x1B);	//	GVDD 4.20V

    EBI2_WRITE16C(DISP_CMD_PORT,0xC1);
                EBI2_WRITE16D(DISP_DATA_PORT,0x10);	//VGH X7   VGL X -4	   AVDD X2

    EBI2_WRITE16C(DISP_CMD_PORT,0xC5);
                EBI2_WRITE16D(DISP_DATA_PORT,0x3f);	  //VCOMH 4.275V
                EBI2_WRITE16D(DISP_DATA_PORT,0x48);	  //VCOML -0.7V

    EBI2_WRITE16C(DISP_CMD_PORT,0xC7);
                EBI2_WRITE16D(DISP_DATA_PORT,0xb9);	 //8F	//VCOM OFFSET	

    EBI2_WRITE16C(DISP_CMD_PORT,0x36);
                EBI2_WRITE16D(DISP_DATA_PORT,0x00);   //80 -> 00 бзб■?воиби╧во 	RGB

    EBI2_WRITE16C(DISP_CMD_PORT,0x3A);
                EBI2_WRITE16D(DISP_DATA_PORT,0x55);

    EBI2_WRITE16C(DISP_CMD_PORT,0xB1);           
                EBI2_WRITE16D(DISP_DATA_PORT,0x00);
                EBI2_WRITE16D(DISP_DATA_PORT,0x1a);

    EBI2_WRITE16C(DISP_CMD_PORT,0xb6);
                EBI2_WRITE16D(DISP_DATA_PORT,0x0a);
				EBI2_WRITE16D(DISP_DATA_PORT,0xa2);
				
				/* Blanking Porch control*/
				EBI2_WRITE16C(DISP_CMD_PORT, 0xb5);
				EBI2_WRITE16D(DISP_DATA_PORT,0x02);
				EBI2_WRITE16D(DISP_DATA_PORT,0x02);
				EBI2_WRITE16D(DISP_DATA_PORT,0x0a);
				EBI2_WRITE16D(DISP_DATA_PORT,0x14);
				
				
				/* Tearing Effect Line On */
				EBI2_WRITE16C(DISP_CMD_PORT, 0x35);
				EBI2_WRITE16D(DISP_DATA_PORT,0x00);
				
				/* Tearing effect Control Parameter */
				EBI2_WRITE16C(DISP_CMD_PORT, 0x44);
				EBI2_WRITE16D(DISP_DATA_PORT,0x00);
				EBI2_WRITE16D(DISP_DATA_PORT,0xef);

    EBI2_WRITE16C(DISP_CMD_PORT,0xE0);
                EBI2_WRITE16D(DISP_DATA_PORT,0x0F);
                EBI2_WRITE16D(DISP_DATA_PORT,0x22); 
                EBI2_WRITE16D(DISP_DATA_PORT,0x1e);
                EBI2_WRITE16D(DISP_DATA_PORT,0x0c); 
                EBI2_WRITE16D(DISP_DATA_PORT,0x0d);
                EBI2_WRITE16D(DISP_DATA_PORT,0x07);
                EBI2_WRITE16D(DISP_DATA_PORT,0x4c);
                EBI2_WRITE16D(DISP_DATA_PORT,0x85);
                EBI2_WRITE16D(DISP_DATA_PORT,0x3d);
                EBI2_WRITE16D(DISP_DATA_PORT,0x07);
                EBI2_WRITE16D(DISP_DATA_PORT,0x12);
                EBI2_WRITE16D(DISP_DATA_PORT,0x05); 
                EBI2_WRITE16D(DISP_DATA_PORT,0x1d); 
                EBI2_WRITE16D(DISP_DATA_PORT,0x1a);
                EBI2_WRITE16D(DISP_DATA_PORT,0x00);

    EBI2_WRITE16C(DISP_CMD_PORT,0xE1);
                EBI2_WRITE16D(DISP_DATA_PORT,0x00);
                EBI2_WRITE16D(DISP_DATA_PORT,0x1a);
                EBI2_WRITE16D(DISP_DATA_PORT,0x1e);
                EBI2_WRITE16D(DISP_DATA_PORT,0x04);	
                EBI2_WRITE16D(DISP_DATA_PORT,0x10);
                EBI2_WRITE16D(DISP_DATA_PORT,0x05);
                EBI2_WRITE16D(DISP_DATA_PORT,0x33);
                EBI2_WRITE16D(DISP_DATA_PORT,0x24);
                EBI2_WRITE16D(DISP_DATA_PORT,0x43);
                EBI2_WRITE16D(DISP_DATA_PORT,0x04);
                EBI2_WRITE16D(DISP_DATA_PORT,0x0c);
                EBI2_WRITE16D(DISP_DATA_PORT,0x0b); 
                EBI2_WRITE16D(DISP_DATA_PORT,0x24);
                EBI2_WRITE16D(DISP_DATA_PORT,0x28);
                EBI2_WRITE16D(DISP_DATA_PORT,0x0f);

    EBI2_WRITE16C(DISP_CMD_PORT,0x11);
	
	 msleep(120);
	EBI2_WRITE16C(DISP_CMD_PORT, 0x29);


	EBI2_WRITE16C(DISP_CMD_PORT, 0x2A);
					   EBI2_WRITE16D(DISP_DATA_PORT,0x00);
					   EBI2_WRITE16D(DISP_DATA_PORT,0x00);
					   EBI2_WRITE16D(DISP_DATA_PORT,0x00);
					   EBI2_WRITE16D(DISP_DATA_PORT,0xEF);

	EBI2_WRITE16C(DISP_CMD_PORT, 0x2B);
					   EBI2_WRITE16D(DISP_DATA_PORT,0x00);
					   EBI2_WRITE16D(DISP_DATA_PORT,0x00);
					   EBI2_WRITE16D(DISP_DATA_PORT,0x01);
					   EBI2_WRITE16D(DISP_DATA_PORT,0x3F);

				   
	 EBI2_WRITE16C(DISP_CMD_PORT,0x2c); // Write memory start
	 {
		 int x, y;
		 for(y = 0; y < QVGA_HEIGHT; y++) {
			 for(x = 0; x < QVGA_WIDTH; x++) {
				 EBI2_WRITE16D(DISP_DATA_PORT, 0);
			 }
		 }
	 }
	 msleep(80);
	
	 EBI2_WRITE16C(DISP_CMD_PORT,0x29); // Display On

}
#endif

#ifdef CONFIG_MACH_MSM7X25A_V3

static void panel_lgdisplay_init(void)
{

	/* SET EXTC */
	EBI2_WRITE16C(DISP_CMD_PORT, 0xcf);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0x21);

	/* Inversion Control */
	EBI2_WRITE16C(DISP_CMD_PORT, 0xb4);
	EBI2_WRITE16D(DISP_DATA_PORT,0x02);

	/* Power control 1 */
	EBI2_WRITE16C(DISP_CMD_PORT, 0xc0);
	EBI2_WRITE16D(DISP_DATA_PORT,0x14);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0f);

	/* Power control 2*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xc1);
	EBI2_WRITE16D(DISP_DATA_PORT,0x04);

	/* Power control 3*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xc2);
	EBI2_WRITE16D(DISP_DATA_PORT,0x32);

	/* Vcom control 1*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xc5);
	EBI2_WRITE16D(DISP_DATA_PORT,0xfc);
	
	/* Interface control*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xf6);
	EBI2_WRITE16D(DISP_DATA_PORT,0x41);
	EBI2_WRITE16D(DISP_DATA_PORT,0x30);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);

	/* Entry Mode Set*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xb7);
	EBI2_WRITE16D(DISP_DATA_PORT,0x06);

	/* Frame Rate control*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xb1);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0x1f);

	/* Memory Access control*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0x36);
	EBI2_WRITE16D(DISP_DATA_PORT,0x08);

	/* Blanking Porch control*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xb5);
	EBI2_WRITE16D(DISP_DATA_PORT,0x02);
	EBI2_WRITE16D(DISP_DATA_PORT,0x02);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a);
	EBI2_WRITE16D(DISP_DATA_PORT,0x14);

	/* Display Function control*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xb6);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a);
	EBI2_WRITE16D(DISP_DATA_PORT,0x02);
	EBI2_WRITE16D(DISP_DATA_PORT,0x27);
	EBI2_WRITE16D(DISP_DATA_PORT,0x04);

	/* Pixel Format */
	EBI2_WRITE16C(DISP_CMD_PORT, 0x3a);
	EBI2_WRITE16D(DISP_DATA_PORT,0x55);

	/* Tearing Effect Line On */
	EBI2_WRITE16C(DISP_CMD_PORT, 0x35);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);

	/* Tearing effect Control Parameter */
	EBI2_WRITE16C(DISP_CMD_PORT, 0x44);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0xef);

   /* Positive Gamma Correction*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xe0);
	EBI2_WRITE16D(DISP_DATA_PORT,0x08);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0e);
	EBI2_WRITE16D(DISP_DATA_PORT,0x12);
	EBI2_WRITE16D(DISP_DATA_PORT,0x04);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0f);
	EBI2_WRITE16D(DISP_DATA_PORT,0x05);
	EBI2_WRITE16D(DISP_DATA_PORT,0x35);
	EBI2_WRITE16D(DISP_DATA_PORT,0x32);
	EBI2_WRITE16D(DISP_DATA_PORT,0x4f);
	EBI2_WRITE16D(DISP_DATA_PORT,0x03);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0c);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a);
	EBI2_WRITE16D(DISP_DATA_PORT,0x2f);
	EBI2_WRITE16D(DISP_DATA_PORT,0x35);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0f);

	/* Negative Gamma Correction*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xe1);
	EBI2_WRITE16D(DISP_DATA_PORT,0x08);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0e);
	EBI2_WRITE16D(DISP_DATA_PORT,0x12);
	EBI2_WRITE16D(DISP_DATA_PORT,0x03);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0e);
	EBI2_WRITE16D(DISP_DATA_PORT,0x03);
	EBI2_WRITE16D(DISP_DATA_PORT,0x35);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0x4d);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a);
	EBI2_WRITE16D(DISP_DATA_PORT,0x12);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0f);
	EBI2_WRITE16D(DISP_DATA_PORT,0x31);
	EBI2_WRITE16D(DISP_DATA_PORT,0x38);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0f);

	/* Column address*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0x2a); // Set_column_address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0xef);

	/* Page address*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0x2b); // Set_Page_address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0x01);
	EBI2_WRITE16D(DISP_DATA_PORT,0x3f);

	/* Charge Sharing control*/
	EBI2_WRITE16C(DISP_CMD_PORT, 0xe8); // Charge Sharing Control
	EBI2_WRITE16D(DISP_DATA_PORT,0x84);
	EBI2_WRITE16D(DISP_DATA_PORT,0x1a);
	EBI2_WRITE16D(DISP_DATA_PORT,0x38);

	/* Exit Sleep - This command should be only used at Power on sequence*/
	EBI2_WRITE16C(DISP_CMD_PORT,0x11); // Exit Sleep

	msleep(120);

	EBI2_WRITE16C(DISP_CMD_PORT,0x2c); // Write memory start
	{
		int x, y;
	for(y = 0; y < QVGA_HEIGHT; y++) {
		for(x = 0; x < QVGA_WIDTH; x++) {
			EBI2_WRITE16D(DISP_DATA_PORT, 0);
		}
	}
	}
	msleep(80);

	EBI2_WRITE16C(DISP_CMD_PORT,0x29); // Display On
}
#endif



#if 0/*2012-09-26 junghoon-kim(junghoon79.kim@lge.com) V3 not use [START]*/
/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
extern int Is_Backlight_Set ; 
#ifdef CONFIG_BACKLIGHT_RT9396
extern int rt9396_force_set(void);
#else
extern int bu61800_force_set(void);
#endif
//extern int mcs8000_ts_on(void);//dajiniv

/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
#endif/*2012-09-26 junghoon-kim(junghoon79.kim@lge.com) V3 not use [END]*/



static int ilitek_qvga_disp_on(struct platform_device *pdev)
{
#ifdef CONFIG_MACH_MSM7X25A_V1
	int readport;
#endif

	struct msm_panel_ilitek_pdata *pdata = tovis_qvga_panel_pdata;

	printk("%s: display on... \n", __func__);
		
	if (!disp_initialized)
		tovis_qvga_disp_init(pdev);

#ifdef LCD_INIT_SKIP_FOR_BOOT_TIME
   if((pdata->initialized && system_state == SYSTEM_BOOTING) || lcd_init_skip_cnt < 1) {
   
      lcd_init_skip_cnt =1;
      printk("%s: display on...Skip!!!!!! and back light off charger logo mode\n", __func__);
	  
#else
	if(pdata->initialized && system_state == SYSTEM_BOOTING) {
		/* Do not hw initialize */      
#endif
	} else {

		/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2012-02-01] 
		: For the Wakeup Issue */
		//mcs8000_ts_on();//dajiniv
		/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2012-02-01] 
		: For the Wakeup Issue */
	
		msm_fb_ebi2_power_save(1);

		/* use pdata->maker_id to detect panel */
		
#ifdef CONFIG_MACH_MSM7X25A_V1
		gpio_tlmm_config(GPIO_CFG(GPIO_LCD_TID, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		readport = gpio_get_value(GPIO_LCD_TID);

		if(readport==0)
		{
			if(pdata->gpio) {
				mdelay(10);	// prevent stop to listen to music with BT
				gpio_set_value(pdata->gpio, 1);
				mdelay(10);
				gpio_set_value(pdata->gpio, 0);
				mdelay(10);
				gpio_set_value(pdata->gpio, 1);
				msleep(120);
			}
			
			panel_tianmaDisplay_init();
			printk("----------tianma Enabled-----------\n");

			
		}else {
			if(pdata->gpio) {
				mdelay(10);	// prevent stop to listen to music with BT
				gpio_set_value(pdata->gpio, 1);
				mdelay(10);
				gpio_set_value(pdata->gpio, 0);
				mdelay(10);
				gpio_set_value(pdata->gpio, 1);
				msleep(120);
			}
			panel_CMIDisplay_init();
			printk("----------CMI Enabled--------------\n");
			
		}
#else
		if(pdata->gpio) {
			//mdelay(10);	// prevent stop to listen to music with BT
			gpio_set_value(pdata->gpio, 1);
			mdelay(1);
			gpio_set_value(pdata->gpio, 0);
			mdelay(10);
			gpio_set_value(pdata->gpio, 1);
			msleep(1);
		}

#endif

#ifdef CONFIG_MACH_MSM7X25A_V3
		panel_lgdisplay_init();
#endif

	}

#ifdef CONFIG_MACH_MSM7X25A_V1	
	if(StatusBacklightOnOff==0) {
		if(IsFirstDisplayOn==0){
			lge_rt8966a_backlight_control(1);
			IsFirstDisplayOn = 1;
			StatusBacklightOnOff = 1;
		}
	}

#endif
	pm_qos_update_request(tovis_pm_qos_req, 65000);
	display_on = TRUE;

#if 0 /*2012-09-26 junghoon-kim(junghoon79.kim@lge.com) V3 not use [START]*/
/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
	if(!Is_Backlight_Set)
	{
		msleep(50);
      #ifdef CONFIG_BACKLIGHT_RT9396
		rt9396_force_set();    //backlight current level force setting here
		#else
      bu61800_force_set();    //backlight current level force setting here
      #endif
	}
   /* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
#endif/*2012-09-26 junghoon-kim(junghoon79.kim@lge.com) V3 not use [END]*/

	  
	return 0;
}

ssize_t tovis_qvga_show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", display_on);
}

ssize_t tovis_qvga_store_onoff(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int onoff;
	struct msm_fb_panel_data *pdata = dev_get_platdata(dev);
	struct platform_device *pd = to_platform_device(dev);

	sscanf(buf, "%d", &onoff);

	if (onoff) {
		pdata->on(pd);
	} else {
		pdata->off(pd);
	}

	return count;
}

DEVICE_ATTR(lcd_onoff, 0664, tovis_qvga_show_onoff, tovis_qvga_store_onoff);

static int __init tovis_qvga_probe(struct platform_device *pdev)
{
	int ret;

	if (pdev->id == 0) {
		tovis_qvga_panel_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);

	ret = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);
	if (ret) {
		printk("tovis_qvga_probe device_creat_file failed!!!\n");
	}

#ifndef CONFIG_ARCH_MSM7X27A
	tovis_pm_qos_req = pm_qos_add_request(PM_QOS_SYSTEM_BUS_FREQ, PM_QOS_DEFAULT_VALUE);
#endif
	return 0;
}

struct msm_fb_panel_data tovis_qvga_panel_data = {
	.on = ilitek_qvga_disp_on,
	.off = ilitek_qvga_disp_off,
	.set_backlight = NULL,
	.set_rect = ilitek_qvga_disp_set_rect,
};

static struct platform_device this_device = {
	.name   = "ebi2_tovis_qvga",
	.id	= 1,
	.dev	= {
		.platform_data = &tovis_qvga_panel_data,
	}
};

static struct platform_driver __refdata this_driver = {
	.probe  = tovis_qvga_probe,
	.driver = {
		.name   = "ebi2_tovis_qvga",
	},
};

static int __init tovis_qvga_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;

	ret = platform_driver_register(&this_driver);
	if (!ret) {
		pinfo = &tovis_qvga_panel_data.panel_info;
		pinfo->xres = QVGA_WIDTH;
		pinfo->yres = QVGA_HEIGHT;
		pinfo->type = EBI2_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->wait_cycle = 0x428000; // 0x908000; /* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-30] : LCD write timing matching */

		pinfo->bpp = 16;
#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
		pinfo->fb_num = 3;
#else
		pinfo->fb_num = 2;
#endif
		pinfo->lcd.vsync_enable = TRUE;
		pinfo->lcd.refx100 = 6000;
/*[2012-12-22][junghoon79.kim@lge.com] V3 camera tearing issue(QCT SR: 01031535) [START]*/
#if defined(CONFIG_MACH_MSM7X25A_V3) || defined(CONFIG_MACH_MSM7X25A_V1)
		pinfo->lcd.v_back_porch = 150;
		pinfo->lcd.v_front_porch = 140;
		pinfo->lcd.v_pulse_width = 40;
#else
		pinfo->lcd.v_back_porch = 0x06;
		pinfo->lcd.v_front_porch = 0x0a;
		pinfo->lcd.v_pulse_width = 2;
#endif
/*[2012-12-22][junghoon79.kim@lge.com] V3 camera tearing issue(QCT SR: 01031535) [END]*/
		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_notifier_period = 0;

		ret = platform_device_register(&this_device);
		if (ret)
			platform_driver_unregister(&this_driver);
	}

	return ret;
}
module_init(tovis_qvga_init);
