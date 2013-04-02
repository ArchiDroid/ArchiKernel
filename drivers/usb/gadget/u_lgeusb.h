/* linux/drivers/usb/gadget/u_lgeusb.h
 *
 * Copyright (C) 2008 Google, Inc.
 * Copyright (C) 2011 LGE.
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

#ifndef __U_LGEUSB_H__
#define __U_LGEUSB_H__

#include CONFIG_LGE_BOARD_HEADER_FILE


#define FACTORY_PID		0x6000

/* LGE_CHANGE_S jongjin7.park 2012.12.08 U0 used the same value as V3/V7 */
#if defined(CONFIG_LGE_PMIC_CABLE_DETECTION) || defined(CONFIG_MACH_MSM7X27A_U0)
/* LGE_CHANGE_E jongjin7.park 2012.12.08 U0 used the same value as V3/V7 */
/*  Below enum must match with ECableUsbType in 
	 modem/MODEL/adaptation/cable_detection/cable_detection.h   */

typedef enum
{
	USB_UNKOWN  = 0x00,
	USB_56K          = 0x02,
	USB_130K        = 0x04,
	USB_180K        = 0x05,
	USB_620K        = 0x09,
	USB_910K        = 0x0A,
	USB_OPEN       = 0x0B,
} ECableUsbType;

#else

#define LGE_FACTORY_CABLE_TYPE 1
#define LGE_PIF_CABLE 2
#define LGE_130K_CABLE 3

#endif	//CONFIG_LGE_PMIC_CABLE_DETECTION

int android_set_factory_mode(void);
bool android_get_factory_mode(void);
void android_factory_desc(int enable);



#endif /* __U_LGEUSB_H__ */
