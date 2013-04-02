/* lge/include/wm9093.h
 *
 * Copyright (C) 2010 LGE, Inc.
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

#ifndef __LINUX_SND_FSA8008_H
#define __LINUX_SND_FSA8008_H

struct fsa8008_platform_data {
	const char *switch_name;            /* switch device name */
	const char *keypad_name;			/* keypad device name */

	unsigned int key_code;				/* key code for hook */

	unsigned int gpio_detect;	/* DET : to detect jack inserted or not */
	unsigned int gpio_mic_en;	/* EN : to enable mic */
	unsigned int gpio_jpole;	/* JPOLE : 3pole or 4pole */
	unsigned int gpio_key;		/* S/E button */

	void (*set_headset_mic_bias)(int enable); /* callback function which is initialized while probing */

	unsigned int latency_for_detection; /* latency for pole (3 or 4)detection (in ms) */
};

#endif

