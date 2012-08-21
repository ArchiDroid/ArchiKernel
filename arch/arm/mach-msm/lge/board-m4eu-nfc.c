/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/init.h>
#include <linux/gpio_event.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/i2c-gpio.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mfd/marimba.h>

#include <mach/rpc_pmapp.h>

#include "devices.h"
#include "timer.h"
#include "devices-msm7x2xa.h"

#include <mach/board_lge.h>

#include "board-m4eu.h"

#include <linux/nfc/pn544_lge.h> // 2011.06.24 kiwon.jeon@lge.com NFC


static struct pn544_i2c_platform_data nfc_pdata = 
{
	.ven_gpio 	= NFC_GPIO_VEN,
	.irq_gpio 	 	= NFC_GPIO_IRQ,
//	.scl_gpio		= NFC_GPIO_I2C_SCL,
//	.sda_gpio		= NFC_GPIO_I2C_SDA,
	.firm_gpio	= NFC_GPIO_FIRM,
};

static struct i2c_board_info nfc_i2c_bdinfo[] = 
{
	[0] = {
		I2C_BOARD_INFO("pn544", 0x28),
		.irq = MSM_GPIO_TO_INT(NFC_GPIO_IRQ),
		.platform_data = &nfc_pdata,
	},
};

void __init lge_add_nfc_devices(void)
{
  gpio_tlmm_config(GPIO_CFG(NFC_GPIO_FIRM, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

  gpio_tlmm_config(GPIO_CFG(NFC_GPIO_VEN, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
		gpio_set_value(NFC_GPIO_VEN, 1);

    gpio_tlmm_config(GPIO_CFG(NFC_GPIO_IRQ, 0,
			GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
		nfc_i2c_bdinfo->irq =
			MSM_GPIO_TO_INT(NFC_GPIO_IRQ);

   i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,  /* suhui.kim@lge.com, MSM7x27A HDK : MSM_GSBI0_QUP_I2C_BUS_ID */
				nfc_i2c_bdinfo,
				ARRAY_SIZE(nfc_i2c_bdinfo));   
}