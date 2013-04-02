/*
 * Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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

#ifndef __ARCH_ARM_MACH_MSM_MINIABB_REGULATOR_H__
#define __ARCH_ARM_MACH_MSM_MINIABB_REGULATOR_H__

#include <linux/regulator/machine.h>

#define MINIABB_REGULATOR_DEV_NAME "miniabb-regulator"

/**
 * struct miniabb_regulator_info - A description of one miniabb regulator
 * @init_data:	Initialization data for the regulator.
 *		Must contain:
 *		- A list of struct regulator_consumer_supply indicating
 *		  supply names for the regulator
 *		- A filled out struct regulation_constraints containing:
 *		  - The name of the regulator
 *		  - The minimum and maximum voltages supported
 *		  - The supported modes (REGULATOR_MODE_NORMAL)
 *		  - The supported operations, currently limited to:
 *		    REGULATOR_CHANGE_STATUS
 *		    REGULATOR_CHANGE_VOLTAGE
 *		  - The input voltage, if the regulator is powered by another
 *		  - Properly set always_on, boot_on, and apply_uV flags
 *		- The name of the supply regulator, if applicable
 * @id:		The miniabb ID of this regulator.
 */
struct miniabb_regulator_info {
	struct regulator_init_data	init_data;
	int				id;
	int				negative;
};

/**
 * struct miniabb_regulator_platform_data - miniabb driver platform data.
 *
 * Contains a description of a set of miniabb-controlled regulators.
 * Pass this in the platform_data field when instantiating the driver.
 *
 * @regs:	An array of struct miniabb_regulator_info describing
 *		the regulators to register.
 * @nregs:	The number of regulators to register.
 */
struct miniabb_regulator_platform_data {
	struct miniabb_regulator_info	*regs;
	size_t				nregs;
};

#define VREG_SWITCH_ENABLE 1
#define VREG_SWITCH_DISABLE 0

#endif

