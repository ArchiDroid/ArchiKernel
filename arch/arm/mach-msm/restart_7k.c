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
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/reboot.h>
#include <linux/pm.h>
#include <asm/system_misc.h>
#include <mach/proc_comm.h>

#include "devices-msm7x2xa.h"
#include "smd_rpcrouter.h"

#ifdef CONFIG_LGE_WAIT_FOR_EFS_SYNC_COMPLETE
/*LGE_CHANGE_S: seven.kim@lge.com  21/03/2012*/
/*Wait for EFS sync comeplete while power off/ reset*/
#include "smd_private.h"
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/delay.h>
/*Wait for EFS sync comeplete while power off/ reset*/
/*LGE_CHANGE_E: seven.kim@lge.com  21/03/2012*/
#endif

static uint32_t restart_reason = 0x776655AA;

extern int lge_emmc_misc_write_crc(unsigned int blockNo, unsigned int magickey, const char* buffer, unsigned int size, int pos);

#ifdef CONFIG_LGE_WAIT_FOR_EFS_SYNC_COMPLETE
/*LGE_CHANGE_S: seven.kim@lge.com  21/03/2012*/
/*Wait for EFS sync comeplete while power off/ reset*/
struct pm_rmt_clnt_wait_info *pm_rmt_wait;
/*Wait for EFS sync comeplete while power off/ reset*/
/*LGE_CHANGE_E: seven.kim@lge.com  21/03/2012*/
#endif

static void msm_pm_power_off(void)
{
#ifdef CONFIG_LGE_WAIT_FOR_EFS_SYNC_COMPLETE
/*LGE_CHANGE_S: seven.kim@lge.com  21/03/2012*/
/*Wait for EFS sync comeplete while power off/ reset*/
	int ret = -1;
	if(pm_rmt_wait)
	{
		if(atomic_read(&pm_rmt_wait->wait_for_RPC_close) != 0)
		{
			printk("Waiting For RMT sync :: Start \n");
			atomic_set(&pm_rmt_wait->waiting_for_rmt, 1);
			ret = wait_event_interruptible_timeout(pm_rmt_wait->pm_event_q,
					(atomic_read(&pm_rmt_wait->wait_for_RPC_close) == 0), 6 * HZ);
			printk("Waiting For RMT sync :: Done \n");
		}
		if( (0 >= ret) && ( -1 != ret ))
		{
			msleep(200);
			printk("200msec wait for rmt RPC complete \n");
		}
		atomic_set(&pm_rmt_wait->waiting_for_rmt, 0);
	}
/*Wait for EFS sync comeplete while power off/ reset*/
/*LGE_CHANGE_E: seven.kim@lge.com  21/03/2012*/
#endif	

	msm_proc_comm(PCOM_POWER_DOWN, 0, 0);
	for (;;)
		;
}

static void msm_pm_restart(char str, const char *cmd)
{
#ifdef CONFIG_LGE_WAIT_FOR_EFS_SYNC_COMPLETE
/*LGE_CHANGE_S: seven.kim@lge.com  21/03/2012*/
/*Wait for EFS sync comeplete while power off/ reset*/
	int ret = -1;
	if(pm_rmt_wait)
	{
		if(atomic_read(&pm_rmt_wait->wait_for_RPC_close) != 0)
		{
			printk("Waiting For RMT sync :: Start \n");
			atomic_set(&pm_rmt_wait->waiting_for_rmt, 1);
			ret = wait_event_interruptible_timeout(pm_rmt_wait->pm_event_q,
					(atomic_read(&pm_rmt_wait->wait_for_RPC_close) == 0), 6 * HZ);
			printk("Waiting For RMT sync :: Done \n");
		}
		if( (0 >= ret) && ( -1 != ret ))
		{
			msleep(200);
			printk("200msec wait for rmt RPC complete \n");
		}
		atomic_set(&pm_rmt_wait->waiting_for_rmt, 0);
	}
/*Wait for EFS sync comeplete while power off/ reset*/
/*LGE_CHANGE_E: seven.kim@lge.com  21/03/2012*/
#endif


	pr_debug("The reset reason is %x\n", restart_reason);

	set_magicnum_restart(restart_reason);

	/* Disable interrupts */
	local_irq_disable();
	local_fiq_disable();

	/*
	 * Take out a flat memory mapping  and will
	 * insert a 1:1 mapping in place of
	 * the user-mode pages to ensure predictable results
	 * This function takes care of flushing the caches
	 * and flushing the TLB.
	 */
	setup_mm_for_reboot();

/* LGE_CHANGE_S : seven.kim@lge.com demigot crash handler*/
#ifdef CONFIG_LGE_HANDLE_PANIC
#define CRASH_REBOOT    	 0x618E1000
#define CRASH_REBOOT_FAKE    0x12121212
	if ((restart_reason == CRASH_REBOOT) || (restart_reason == CRASH_REBOOT_FAKE)){
		msm_proc_comm(PCOM_RESET_CHIP_IMM, &restart_reason, 0);
	}
	else{
/* LGE_CHANGE_S : youngbae.choi@lge.com RESET_CHIP command change*/
	msm_proc_comm(PCOM_RESET_CHIP_IMM, &restart_reason, 0);
/* LGE_CHANGE_E : youngbae.choi@lge.com RESET_CHIP command change*/
       }
#else /*qct original*/
	msm_proc_comm(PCOM_RESET_CHIP, &restart_reason, 0);
#endif
/* LGE_CHANGE_E : seven.kim@lge.com demigot crash handler */

	for (;;)
		;
}

static int msm_reboot_call
	(struct notifier_block *this, unsigned long code, void *_cmd)
{
#ifdef CONFIG_LGE_REBOOT_REASON_IN_EMMC
	int reason_carving_result = 0;
#endif
	if ((code == SYS_RESTART) && _cmd) {
		char *cmd = _cmd;
		if (!strncmp(cmd, "bootloader", 10)) {
			restart_reason = 0x77665500;
		} else if (!strncmp(cmd, "recovery", 8)) {
			restart_reason = 0x77665502;
		} else if (!strncmp(cmd, "eraseflash", 10)) {
			restart_reason = 0x776655EF;
		} else if (!strncmp(cmd, "oem-", 4)) {
			unsigned long code;
			int res;
			res = kstrtoul(cmd + 4, 16, &code);
			code &= 0xff;
			restart_reason = 0x6f656d00 | code;
	/* LGE_CHANGE_S : (youngbae.choi@lge.com) for bnr_recovery mode from pc sync [START]*/
                } else if (!strcmp(cmd, "--bnr_recovery")) {
                        restart_reason = 0x77665555;
	/* LGE_CHANGE_E : (youngbae.choi@lge.com) for bnr_recovery mode from pc sync [END]*/
// LGE_CHANGE_S, for reboot command from chargerlogo process
		} else if (!strcmp(cmd, "charge_reset")) {
			restart_reason = 0x776655AA;
		} else if (!strncmp(cmd, "", 1)) {
			restart_reason = 0x776655AA;
// LGE_CHANGE_E, for reboot command from chargerlogo process			
		} else {
			restart_reason = 0x77665501;
		}

//LGE_CHANGE_S, khyun.kim@lge.com [V7] Modify for the issue that erased reboot reason in internal ram because of hw problem [start].
#ifdef CONFIG_LGE_REBOOT_REASON_IN_EMMC
	reason_carving_result = lge_emmc_misc_write_crc(12, 0xBEEF,(char*)&restart_reason,4,0);
	if (reason_carving_result != 4){
		pr_err("Failed to carve reboot reason to misc\n");
	}
//LGE_CHANGE_E, khyun.kim@lge.com [V7] Modify for the issue that erased reboot reason in internal ram because of hw problem [end].
#endif
	}
	return NOTIFY_DONE;
}

static struct notifier_block msm_reboot_notifier = {
	.notifier_call = msm_reboot_call,
};

/* LGE_CHANGE_S : seven.kim@lge.com demigot crash handler porting */
#if defined(CONFIG_MACH_LGE)
void lge_set_reboot_reason(unsigned reason)
{
	restart_reason = reason;
}
#endif
/* LGE_CHANGE_S : seven.kim@lge.com demigot crash handler porting */


static int __init msm_pm_restart_init(void)
{
	int ret;

	pm_power_off = msm_pm_power_off;
	arm_pm_restart = msm_pm_restart;

	ret = register_reboot_notifier(&msm_reboot_notifier);
	if (ret)
		pr_err("Failed to register reboot notifier\n");

	return ret;
}
late_initcall(msm_pm_restart_init);
