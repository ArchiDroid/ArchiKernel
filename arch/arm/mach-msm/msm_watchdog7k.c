/* Copyright (c) 2010-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <asm/cacheflush.h>
#include "msm_watchdog.h"
#include "timer.h"
#include "mach/smem_log.h"

#define MODULE_NAME "msm_softwd"

static unsigned long delay_time;
static unsigned long long last_pet;


static void pet_softwatchdog_work(struct work_struct *work);
static DECLARE_DELAYED_WORK(dogsoftwork_struct, pet_softwatchdog_work);



static void pet_softwatchdog_work(struct work_struct *work)
{
#if !defined (CONFIG_MACH_MSM7X27A_U0)		// bohyun.jung@lge.com - it overwhelms all over kernel logs. 
/*	printk(KERN_INFO "MSM Soft Watchdog petting every 2 seconds"); */
#endif
	last_pet = sched_clock();
        smem_log_event(SMEM_LOG_PROC_ID_APPS |
				DEM_DEBUG,
				0xBEEFBEEF,
				(long)last_pet,
				0xB01DB01D);
	schedule_delayed_work_on(0, &dogsoftwork_struct, delay_time);
 	

}

static int init_softwatchdog(void)
{
	delay_time = msecs_to_jiffies(2000);
	printk(KERN_INFO "MSM Soft Watchdog Initialized\n");
	schedule_delayed_work_on(0, &dogsoftwork_struct, delay_time);
	return 0;	
}

module_init(init_softwatchdog);
MODULE_DESCRIPTION("MSM Soft Watchdog");
