/*
 * drivers/i2c/chips/SenseTek/stk_ps31xx_lux_threshold_table.h
 *
 * $Id: stk_defines.h,v 1.0 2011/03/06 14:35:24 jsgood Exp $
 *
 * Copyright (C) 2011 Patrick Chang <patrick_chang@sitronix.com.tw>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	SenseTek/Sitronix Proximity / Ambient Light Sensor Driver
 */
#ifndef __STK_PS31XX_LUX_THRESHOLD_TABLE_H
#define __STK_PS31XX_LUX_THRESHOLD_TABLE_H

static uint32_t lux_threshold_table[] =
{
    3,
    10,
    40,
    65,
    145,
    300,
    550,
    930,
};

#define LUX_THD_TABLE_SIZE (sizeof(lux_threshold_table)/sizeof(uint32_t)+1)
static uint32_t nLuxIndex;

static uint16_t code_threshold_table[LUX_THD_TABLE_SIZE+1];



#endif // __STK_PS31XX_LUX_THRESHOLD_TABLE_H
