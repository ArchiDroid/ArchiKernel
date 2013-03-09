/*
 * include/linux/melfas_ts.h - platform data structure for MCS Series sensor
 *
 * Copyright (C) 2010 Melfas, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_MELFAS_TS_H
#define _LINUX_MELFAS_TS_H

/* LGE_CHANGE_S : RECENT_APPS_KEY (Bell Operator in Canada) */
#if defined(CONFIG_MACH_MSM7X25A_M4CA_BELL_REV_B)
#define MELFAS_TS_NAME "touch_mcs8000_bell"
#else
#define MELFAS_TS_NAME "touch_mcs8000"
#endif
/* LGE_CHANGE_E : RECENT_APPS_KEY */

struct melfas_tsi_platform_data {
	int x_size;
	int y_size;
	int  version;
};

#endif /* _LINUX_MELFAS_TS_H */
