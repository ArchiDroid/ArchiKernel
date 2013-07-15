/*
 * Author: Jean-Pierre Rasquin <yank555.lu@gmail.com>
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

#ifndef _LINUX_SUSPDEL_H
#define _LINUX_SUSPDEL_H
extern int early_suspend_delay;

#define EARLY_SUSPEND_DISABLED 0
#define EARLY_SUSPEND_DEFAULT 100	/* default is 100 ms */
#define EARLY_SUSPEND_MAX 700

#endif
