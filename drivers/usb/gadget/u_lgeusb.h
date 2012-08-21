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

#include <mach/board_lge.h>

#define FACTORY_PID		0x6000
#define MAX_IMEI_LEN 19

// cable type & NV MANUAL TESTMODE Mask
#define LGE_CABLE_TYPE_MASK					0x0000000F
#define LGE_CABLE_TYPE_56K					0x00000007
#define LGE_CABLE_TYPE_130K					0x00000008
#define LGE_CABLE_TYPE_910K					0x00000009
#define LGE_CABLE_TYPE_NV_MANUAL_TESTMODE	0x00001000

int android_lge_is_factory_cable(int *type);
void android_lge_set_factory_mode(int is_factory);
bool android_lge_get_factory_mode(void);

#endif /* __U_LGEUSB_H__ */
