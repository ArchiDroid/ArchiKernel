/* lge/include/nfc_nxp_pn544pn65n.h
 *
 * Copyright (C) 2010 NXP Semiconductors
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#ifndef __NFC_NXP_PN544PN65N_H
#define __NFC_NXP_PN544PN65N_H

#define PN544_MAGIC	0xE9

/*
 * PN544 power control via ioctl
 * PN544_SET_PWR(0): power off
 * PN544_SET_PWR(1): power on
 * PN544_SET_PWR(2): reset and power on with firmware download enabled
 */
#define PN544_SET_PWR	_IOW(PN544_MAGIC, 0x01, unsigned int)

struct nxp_pn544pn65n_platform_data {
	unsigned int sda_gpio; // 2011.02.15 jaejoon.park@lge.com added for LGE structure
	unsigned int scl_gpio; // 2011.02.15 jaejoon.park@lge.com added for LGE structure
	unsigned int irq_gpio;
	unsigned int ven_gpio;
	unsigned int firm_gpio;
};

#endif	//__NFC_NXP_PN544PN65N_H

