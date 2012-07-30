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

#ifndef __LINUX_SND_WM9093_H
#define __LINUX_SND_WM9093_H

/* interface for control amp path */
#define ICODEC_AMP_OFF	0
#define ICODEC_HANDSET_RX	1
#define ICODEC_HEADSET_ST_RX	2
#define ICODEC_HEADSET_MN_RX	3
#define ICODEC_SPEAKER_RX	4
//LGE_UPDATE_S, jeremy.pi@lge.com, 2011-04-09, at&t 
#if defined(CONFIG_MACH_LGE_I_BOARD_ATNT) || defined(CONFIG_MACH_LGE_I_BOARD_DCM) || defined(CONFIG_MACH_LGE_I_BOARD_SKT)
#define ICODEC_HEADSET_ST_RX_SPEAKER_RX 5   //simultaneously Ringing Headset and SPK
#define ICODEC_TTY_RX 6 // TTY
#define ICODEC_SPEAKER_PLAYBACK_RX  7 // Playback not call
#define ICODEC_HEADSET_ST_PLAYBACK_RX  8 // Playback not call
#endif /* CONFIG_MACH_LGE_I_BOARD_ATNT */
//LGE_UPDATE_E, jeremy.pi@lge.com, 2011-04-09, at&t
/* interface for platform data while initializing wm9093 as platform device */
typedef struct {
	u8 wmaddress;
    u16 wmdata;	
} wmCodecCmd;

/* following information is for initializing platform device */
struct wm9093_CodecCmd_data {
	wmCodecCmd *amp_function;
	unsigned int amp_function_size;
};

struct wm9093_platform_data {
	struct wm9093_CodecCmd_data speaker_on;
	struct wm9093_CodecCmd_data hph_on;
//LGE_UPDATE_S, jeremy.pi@lge.com, 2011-04-09, at&t 
#if defined(CONFIG_MACH_LGE_I_BOARD_ATNT) || defined(CONFIG_MACH_LGE_I_BOARD_DCM) || defined(CONFIG_MACH_LGE_I_BOARD_SKT)
	struct wm9093_CodecCmd_data hph_spk_on; // simultaneously Ringing Headset and SPK
    struct wm9093_CodecCmd_data tty_on; // TTY 
	struct wm9093_CodecCmd_data speaker_playback_on;    // not call
	struct wm9093_CodecCmd_data hph_playback_on;    // not call
#endif /* CONFIG_MACH_LGE_I_BOARD_ATNT */
//LGE_UPDATE_E, jeremy.pi@lge.com, 2011-04-09, at&t
	struct wm9093_CodecCmd_data power_down;
	void (*set_amp_path)(int icodec_num); /* callback function which is initialized while probing */
};

#endif
