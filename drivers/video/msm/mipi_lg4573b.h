/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef MIPI_LG4573B_H
#define MIPI_LG4573B_H

/*[LGSI_SP4_BSP_BEGIN] [kiran.jainapure@lge.com] - Support not required - as QCT is supporting this feature. 
For details check board file.*/
//#define CONFIG_FB_MSM_MIPI_DSI_LG4573B_BOOT_LOGO 1
/*[LGSI_SP4_BSP_END] [kiran.jainapure@lge.com] */

#define GPIO_U0_LCD_RESET 125
/* LGE_CHANGE_S : LCD ESD Protection 
 * 2012-01-30, yoonsoo@lge.com
 * LCD ESD Protection
 */
#ifdef CONFIG_LGE_LCD_ESD_DETECTION
#define GPIO_U0_LCD_ESD_DETECT 86

enum {
	GPIO_U0_LCD_ESD_DETECT_PIN_IS_LOW=0,
	GPIO_U0_LCD_ESD_DETECT_PIN_IS_HIGH=1,
};
/*LGE_START: Kiran.kanneganti@lge.com 05-03-2012*/
/*In case of ESD no delays required in power off*/
/*Change Interrupt enable & panel on timings to avoid display corruption*/
/* LGE_CHANGE_S jungrock.oh@lge.com 2013-02-02 reduce lcd off->on delay time*/
#if !defined(CONFIG_MACH_MSM7X27A_U0)
#define ESD_HANDLE_DELAY (3*HZ)
#else
#define ESD_HANDLE_DELAY msecs_to_jiffies(0) 
#endif
#define ESD_INT_EN_DELAY (3*HZ)
#if !defined(CONFIG_MACH_MSM7X27A_U0)
#define ESD_PANEL_ON_DELAY msecs_to_jiffies(4000)
#else
#define ESD_PANEL_ON_DELAY msecs_to_jiffies(1000)
#endif
/* LGE_CHANGE_E jungrock.oh@lge.com 2013-02-02 reduce lcd off->on delay time*/
/*LGE_END: Kiran.kanneganti@lge.com*/
struct lcd_esd_detect {
 const char *name;
 unsigned esd_detect_gpio;
 atomic_t esd_irq_state;
 atomic_t panel_poweroff;
 unsigned int esd_irq;
 struct delayed_work esd_det_work;
 struct delayed_work esd_int_en_work;
 struct delayed_work esd_dsi_panel_on;
};
#endif
/* LGE_CHANGE_E : LCD ESD Protection*/ 
int mipi_lg4573b_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel);

#endif  /* MIPI_MAGNACHIP_H */
