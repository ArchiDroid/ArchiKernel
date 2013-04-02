/* lge/include/nfc_nxp_pn544pn65n.h
 *
 * Copyright (C) 2010 NXP Semiconductors
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

#define PN544_MAGIC	0xE9

#define PN544_DRV_NAME			"pn544"
#define NFC_GPIO_VEN			12
#define NFC_GPIO_IRQ			42
#define NFC_GPIO_FIRM			11
#define NFC_GPIO_I2C_SDA		132
#define NFC_GPIO_I2C_SCL		131
#define NFC_I2C_SLAVE_ADDR		0x28

/*
 * PN544 power control via ioctl
 * PN544_SET_PWR(0): power off
 * PN544_SET_PWR(1): power on
 * PN544_SET_PWR(2): reset and power on with firmware download enabled
 */
 
#define PN544_SET_PWR	_IOW(PN544_MAGIC, 0x01, unsigned int)
#define PN544_HW_REVISION _IOR(PN544_MAGIC, 0x02, unsigned int)

struct pn544_i2c_platform_data {
	unsigned int sda_gpio;
	unsigned int scl_gpio;
	unsigned int irq_gpio;
	unsigned int ven_gpio;
	unsigned int firm_gpio;
};
