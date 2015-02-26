/*
 *  linux/drivers/cpufreq/cpufreq_powersave.c
 *
 *  Copyright (C) 2002 - 2003 Dominik Brodowski <linux@brodo.de>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/init.h>

#include <linux/cpu.h>
#include <linux/cpumask.h>

static void cpu_up_work()
{
	int cpu;

	for_each_cpu_not(cpu, cpu_online_mask) {
		if (cpu == 0)
			continue;
		cpu_up(cpu);
	}
}

static void cpu_down_work()
{
	int cpu;

	for_each_online_cpu(cpu) {
		if (cpu == 0)
			continue;
		cpu_down(cpu);
	}
}

static DECLARE_WORK(powersave_up_work, cpu_down_work);
static DECLARE_WORK(powersave_down_work, cpu_up_work);

static int cpufreq_governor_powersave(struct cpufreq_policy *policy,
					unsigned int event)
{
	switch (event) {
	case CPUFREQ_GOV_START:
		schedule_work_on(0, &powersave_up_work);
	case CPUFREQ_GOV_LIMITS:
		pr_debug("setting to %u kHz because of event %u\n",
							policy->min, event);
		__cpufreq_driver_target(policy, policy->min,
						CPUFREQ_RELATION_L);
		break;
	case CPUFREQ_GOV_STOP:
		schedule_work_on(0, &powersave_down_work);
		break;
	default:
		break;
	}
	return 0;
}

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_POWERSAVE
static
#endif
struct cpufreq_governor cpufreq_gov_powersave = {
	.name		= "powersave",
	.governor	= cpufreq_governor_powersave,
	.owner		= THIS_MODULE,
};

static int __init cpufreq_gov_powersave_init(void)
{
	return cpufreq_register_governor(&cpufreq_gov_powersave);
}


static void __exit cpufreq_gov_powersave_exit(void)
{
	cpufreq_unregister_governor(&cpufreq_gov_powersave);
}


MODULE_AUTHOR("Dominik Brodowski <linux@brodo.de>");
MODULE_DESCRIPTION("CPUfreq policy governor 'powersave'");
MODULE_LICENSE("GPL");

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_POWERSAVE
fs_initcall(cpufreq_gov_powersave_init);
#else
module_init(cpufreq_gov_powersave_init);
#endif
module_exit(cpufreq_gov_powersave_exit);
