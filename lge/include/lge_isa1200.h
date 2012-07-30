/* lge/include/lge_isa1200.h
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
#ifndef __LGE_ISA1200_H__
#define __LGE_ISA1200_H__

#define LGE_ISA1200_SCTRL		0x00

#define LGE_ISA1200_HCTRL0		0x30
#define LGE_ISA1200_HCTRL1		0x31
#define LGE_ISA1200_HCTRL2		0x32
#define LGE_ISA1200_HCTRL3		0x33
#define LGE_ISA1200_HCTRL4		0x34
#define LGE_ISA1200_HCTRL5		0x35
#define LGE_ISA1200_HCTRL6		0x36
#define LGE_ISA1200_HCTRL7		0x37
#define LGE_ISA1200_HCTRL8		0x38
#define LGE_ISA1200_HCTRL9		0x39
#define LGE_ISA1200_HCTRLA		0x3A
#define LGE_ISA1200_HCTRLB		0x3B
#define LGE_ISA1200_HCTRLC		0x3C
#define LGE_ISA1200_HCTRLD		0x3D
#define LGE_ISA1200_HCTRLE		0x3E
#define LGE_ISA1200_HCTRLF		0x3F

struct isa1200_reg_cmd {
	u8 addr;
    u8 data;	
};

/* following information is for initializing platform device */
struct isa1200_reg_seq {
	struct isa1200_reg_cmd *reg_cmd_list;
	unsigned int number_of_reg_cmd_list;
};

struct lge_isa1200_platform_data {
	const char *vibrator_name;	/* vibrator device name */

	/* following information is about hw configuration */
	unsigned int gpio_hen;		/* Haptic Motor Driver ENable */
	unsigned int gpio_len;		/* LDO ENable */
	//int (*power)(int enable);		/* power control VBAT. NULL assumes power is always-on.  */
	int (*clock)(int enable);		/* clock control for PWM/CLK. NULL assumes clock is always-on. */

	int max_timeout; /* maximum available timeout in millisec */
	int default_vib_strength; /* default vibrator strength : register LGE_ISA1200_HCTRL5 value */

	struct isa1200_reg_seq *init_seq; /* initialization sequence */
};

#endif


