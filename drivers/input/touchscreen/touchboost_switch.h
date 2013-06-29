/*
 * Author: andip71
 *
 * Added sysfs for touchboost frequency : Jean-Pierre Rasquin <yank555.lu@gmail.com>
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

extern int tb_switch;
extern int tb_freq;
extern int tb_freq_level;

#define TOUCHBOOST_OFF	0
#define TOUCHBOOST_ON	1

#define TOUCHBOOST_DEFAULT_FREQ 800000

#define TOUCHBOOST_FREQ_UNDEFINED -1
