/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
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

#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <mach/clk.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <asm/fiq.h>
#include <asm/hardware/gic.h>
#include <asm/cacheflush.h>
#include <mach/irqs-8625.h>
#include <linux/irq.h>

#include "msm_watchdog.h"

#define MODULE_NAME "msm7k_fiq_handler"

struct msm_watchdog_dump msm_dump_cpu_ctx;
int fiq_counter = 0;

/* Called from the FIQ bark handler */
void msm_7k_bark_fin(void)
{
       fiq_counter++;
       // local_irq_disable();
	//clear_pending_spi(MSM8625_INT_A9_M2A_2);
	gic_clear_irq_pending(MSM8625_INT_A9_M2A_2);
       //gic_clear_spi_pending(MSM8625_INT_A9_M2A_2);
       //local_irq_enable();
       flush_cache_all();
       outer_flush_all();
       return;
}

struct fiq_handler msm_7k_fh = {
       .name = MODULE_NAME,
};

extern unsigned int msm_7k_fiq_start, msm_7k_fiq_length;
//extern void msm_7k_fiq_setup(void *stack);
void *msm_fiq_stack;
int msm_setup_fiq_handler(void)
{
       int ret = 0;
       //void *stack = NULL;
       claim_fiq(&msm_7k_fh);
       set_fiq_handler(&msm_7k_fiq_start, msm_7k_fiq_length);
       msm_fiq_stack = (void *)__get_free_pages(GFP_KERNEL, THREAD_SIZE_ORDER);
       printk(" %s : free pages available -%p ::\n",__func__,msm_fiq_stack);
       if (msm_fiq_stack == NULL) {
              printk("No free pages available - %s fails\n",
                     __func__);
	      panic("FIQ STACK SETUP IS NOT SUCCESSFUL");	
              return -ENOMEM;
       }
       //msm_7k_fiq_setup(stack);
       irq_set_irq_type(MSM8625_INT_A9_M2A_2, IRQF_TRIGGER_RISING);
       gic_set_irq_secure(MSM8625_INT_A9_M2A_2);
       enable_irq(MSM8625_INT_A9_M2A_2);	
       printk("%s : setup_fiq_handler --done \n", __func__);
       return ret;
}

static int init_7k_fiq(void)
{
#ifndef CONFIG_MACH_MSM8X25_V7
       printk("MSM Init FIQ\n");
       if(msm_setup_fiq_handler())
	panic("CHECK FIQ INIT");
#endif	   
       return 0;
}
late_initcall(init_7k_fiq);
