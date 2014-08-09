/*
 *  Copyright (C) 1999,2000 Arm Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *  Copyright (C) 2002 Shane Nay (shane@minirl.com)
 *  Copyright 2005-2007 Freescale Semiconductor, Inc. All Rights Reserved.
 *    - add MX31 specific definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/mm.h>
#include <linux/init.h>
#include <linux/err.h>

#include <asm/pgtable.h>
#include <asm/mach/map.h>

#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/iomux-v3.h>
#include <mach/irqs.h>

static struct map_desc mx31_io_desc[] __initdata = {
	imx_map_entry(MX31, X_MEMC, MT_DEVICE),
	imx_map_entry(MX31, AVIC, MT_DEVICE_NONSHARED),
	imx_map_entry(MX31, AIPS1, MT_DEVICE_NONSHARED),
	imx_map_entry(MX31, AIPS2, MT_DEVICE_NONSHARED),
	imx_map_entry(MX31, SPBA0, MT_DEVICE_NONSHARED),
};

/*
 * This function initializes the memory map. It is called during the
 * system startup to create static physical to virtual memory mappings
 * for the IO modules.
 */
void __init mx31_map_io(void)
{
	iotable_init(mx31_io_desc, ARRAY_SIZE(mx31_io_desc));
}

void __init imx31_init_early(void)
{
	mxc_set_cpu_type(MXC_CPU_MX31);
	mxc_arch_reset_init(MX31_IO_ADDRESS(MX31_WDOG_BASE_ADDR));
}

void __init mx31_init_irq(void)
{
	mxc_init_irq(MX31_IO_ADDRESS(MX31_AVIC_BASE_ADDR));
}

void __init imx31_soc_init(void)
{
	mxc_register_gpio(0, MX31_GPIO1_BASE_ADDR, SZ_16K, MX31_INT_GPIO1, 0);
	mxc_register_gpio(1, MX31_GPIO2_BASE_ADDR, SZ_16K, MX31_INT_GPIO2, 0);
	mxc_register_gpio(2, MX31_GPIO3_BASE_ADDR, SZ_16K, MX31_INT_GPIO3, 0);
}
