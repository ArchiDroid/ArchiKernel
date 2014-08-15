/*
 * include/linux/exynos4_export_temp.h
 *
 * Author: Zane Zaminsky <cyxman@yahoo.com>
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

#ifndef _EXYNOS4_EXPORT_TEMP_H
#define _EXYNOS4_EXPORT_TEMP_H

/*
 * Function in tmu.c to export current cpu temperature via variable in
 * get_curr_temperature function. This variable will have the refresh interval
 * from the driver itself which monitors the cpu temperature every 10 seconds.
 */
unsigned int get_exynos4_temperature(void);
#endif