/*
 * arch/arm/mach-msm/include/mach/lge_boot_mode.h
 *
 * Copyright (C) 2012 LGE, Inc
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

#ifndef __LGE_BOOT_MODE_H__
#define __LGE_BOOT_MODE_H__

enum lge_boot_mode_type {
	LGE_BOOT_MODE_UNKOWN = 0,	/* Normal Booting Mode */
	LGE_BOOT_MODE_CHARGER,		/* Power-Off Charging Mode */
	LGE_BOOT_MODE_FACTORY,		/* AAT Test Mode */
	LGE_BOOT_MODE_FIRST,		/* AAT first boot Mode */
	LGE_BOOT_MODE_MINIOS,		/* MiniOS 2.0 Mode */
	LGE_BOOT_MODE_CRASH,		/* crash case - no sound*/
};

#define LGE_BOOT_MODE_DEVICE		"lge_boot_mode"

/* Exported symbols */
int get_lge_boot_mode(void);

void __init lge_add_boot_mode_devices(void);

#endif/*__LGE_BOOT_MODE_H__*/
