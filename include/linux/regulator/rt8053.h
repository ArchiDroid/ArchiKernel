/*
 * RICHTEK RT8053 PMIC chip client interface
 *
 * Based on lp3972.h
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __LINUX_REGULATOR_RT8053_H
#define __LINUX_REGULATOR_RT8053_H

#include <linux/regulator/machine.h>

#define RT8053_LDO1  0
#define RT8053_LDO2  1
#define RT8053_LDO3  2
#define RT8053_LDO4  3
#define RT8053_LDO5  4
#define RT8053_DCDC1 5
#define RT8053_NUM_REGULATORS 6

struct rt8053_regulator_subdev {
	int id;
	struct regulator_init_data initdata;
};

struct rt8053_platform_data {
	int num_regulators;
	int enable_gpio;
	struct rt8053_regulator_subdev *regulators;
};

#endif
