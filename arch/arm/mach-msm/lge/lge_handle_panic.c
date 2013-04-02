/*
 * arch/arm/mach-msm/lge/lge_handle_panic.c
 *
 * Copyright (C) 2010 LGE, Inc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/module.h> /*seven for struct kernel_param*/
#include <asm/setup.h>
#include CONFIG_LGE_BOARD_HEADER_FILE

#define PANIC_HANDLER_NAME "panic-handler"
#define VAILD_MAGICID			0xDADADADA
#define CRASH_REBOOT			0x618E1000
#define CRASH_REBOOT_FAKE		0x12121212
#define DLOAD_RESET				0x776655AA

struct crash_log_dump {
	unsigned vaild_magic_id;
	unsigned magic_key;
	unsigned int size;
	unsigned char buffer[0];
};

typedef struct
{	unsigned int	ExternalMagicNumber;
	unsigned int	ExternalBootReason;
	unsigned int	NotifyingRamCorruption;
	unsigned char	InteranlRamDump[2048];
} InternalRamCorruption;

static struct crash_log_dump *crash_dump_log;
static unsigned int crash_buf_size;
static int crash_store_flag;
static int kernel_crash_mode = 0;

#ifdef CONFIG_LGE_SILENCE_RESET
static int kernel_silence_mode = 1;
#endif

static DEFINE_SPINLOCK(lge_panic_lock);

static int dummy_arg;
static int gen_bug(const char *val, struct kernel_param *kp)
{
	BUG();

	return 0;
}
module_param_call(gen_bug, gen_bug, param_get_bool, &dummy_arg, S_IWUSR | S_IRUGO);

static int gen_panic(const char *val, struct kernel_param *kp)
{
	panic("generate test-panic");

	return 0;
}
module_param_call(gen_panic, gen_panic, param_get_bool, &dummy_arg, S_IWUSR | S_IRUGO);

#if 0
static int get_panic_info(char *buffer, struct kernel_param *kp)
{
	return sprintf(buffer, "%s", (kernel_crash_mode == 2) ? "1" : "0");
}
module_param_call(get_panic_info, NULL, get_panic_info, NULL, 0644);
#endif

static int crash_handle_enable = 1;
module_param_named(crash_handle_enable, crash_handle_enable,
				   int, S_IRUGO | S_IWUSR | S_IWGRP);


void set_kernel_panicmode(int val)
{
	printk(KERN_ERR "set_kernel_panicmode enter = %d\n", val);
	kernel_crash_mode = val;	
}

int get_kernel_panicmode(void)
{	
	return kernel_crash_mode;	
}


#ifdef CONFIG_LGE_SILENCE_RESET
void set_kernel_silencemode(int val)
{
	printk(KERN_ERR "set_kernel_silence enter = %d\n", val);
	kernel_silence_mode = val;	
}

int get_kernel_silencemode(void)
{	
	return kernel_silence_mode;	
}
#endif

void store_crash_log(char *p)
{
	if (!crash_store_flag)
		return;

	if (crash_dump_log->size == crash_buf_size)
		return;

	for ( ; *p; p++) {
		if (*p == '[') {
			for ( ; *p != ']'; p++)
				;
			p++;
			if (*p == ' ')
				p++;
		}

		if (*p == '<') {
			for ( ; *p != '>'; p++)
				;
			p++;
		}

		crash_dump_log->buffer[crash_dump_log->size] = *p;
		crash_dump_log->size++;
	}
	crash_dump_log->buffer[crash_dump_log->size] = 0;

	return;
}

static int restore_crash_log(struct notifier_block *this, unsigned long event,
		void *ptr)
{
	unsigned long flags;

	crash_store_flag = 0;

	spin_lock_irqsave(&lge_panic_lock, flags);

#ifdef CONFIG_LGE_SILENCE_RESET
	if(kernel_silence_mode == 0){
		lge_set_reboot_reason(CRASH_REBOOT_FAKE); // display kernel crash mode
	}
	else{
		lge_set_reboot_reason(CRASH_REBOOT); 	 // not display kernel crash mode, normal booting
	}

#else
	lge_set_reboot_reason(CRASH_REBOOT_FAKE); 	// display kernel crash mode
#endif

	spin_unlock_irqrestore(&lge_panic_lock, flags);

	return NOTIFY_DONE;
}

void set_crash_store_enable(void)
{
	crash_store_flag = 1;
}

void set_crash_store_disable(void)
{
	crash_store_flag = 0;
}

void set_magicnum_restart(unsigned reason)
{
	unsigned long flags;

	spin_lock_irqsave(&lge_panic_lock, flags);
	crash_dump_log->vaild_magic_id = VAILD_MAGICID;
	crash_dump_log->magic_key = reason;
	spin_unlock_irqrestore(&lge_panic_lock, flags);
}

static struct notifier_block panic_handler_block = {
	.notifier_call  = restore_crash_log,
};

static int __init lge_panic_handler_probe(struct platform_device *pdev)
{
	struct resource *res = pdev->resource;
	size_t start;
	size_t buffer_size;
	void *buffer;
	int ret = 0;

	if (res == NULL || pdev->num_resources != 1 ||
			!(res->flags & IORESOURCE_MEM)) {
		printk(KERN_ERR "lge_panic_handler: invalid resource, %p %d flags "
				"%lx\n", res, pdev->num_resources, res ? res->flags : 0);
		return -ENXIO;
	}

	buffer_size = res->end - res->start + 1;
	start = res->start;
	printk(KERN_INFO "lge_panic_handler: got buffer at %zx, size %zx\n",
			start, buffer_size);
	buffer = ioremap(res->start, buffer_size);
	if (buffer == NULL) {
		printk(KERN_ERR "lge_panic_handler: failed to map memory\n");
		return -ENOMEM;
	}

	{	InternalRamCorruption*	l_RamDump = (InternalRamCorruption*)buffer;
		if( l_RamDump->NotifyingRamCorruption == VAILD_MAGICID )
		{	int i = 0;
			printk(KERN_ERR "[BootMode] IRAM Corruption is detected.\n");
			for( i=0; i<256; i++ )
			{	unsigned int*	l_FourBytes = (unsigned int*)l_RamDump->InteranlRamDump + i;
				printk(KERN_ERR "%08x\n", *l_FourBytes);
			}
		}
	}

	crash_dump_log = (struct crash_log_dump *)buffer;
	memset(crash_dump_log, 0, buffer_size);
// 2012.11.30, chiwon.son, [Reboot.reason.in.SDRAM] To entirely replace boot reason in IRAM, the magic_key needs to be cleared to indicate normal power off. [START]
#if 1
	crash_dump_log->vaild_magic_id = 0;
	crash_dump_log->magic_key = 0;
#else	// Original
	crash_dump_log->vaild_magic_id = VAILD_MAGICID;
	crash_dump_log->magic_key = DLOAD_RESET;
#endif
// 2012.11.30, chiwon.son, [Reboot.reason.in.SDRAM] To entirely replace boot reason in IRAM, the magic_key needs to be cleared to indicate normal power off. [END]
	crash_dump_log->size = 0;
	crash_buf_size = buffer_size - offsetof(struct crash_log_dump, buffer);

	/* Setup panic notifier */
	atomic_notifier_chain_register(&panic_notifier_list, &panic_handler_block);

	return ret;
}

static int __devexit lge_panic_handler_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver panic_handler_driver __refdata = {
	.probe = lge_panic_handler_probe,
	.remove = __devexit_p(lge_panic_handler_remove),
	.driver = {
		.name = PANIC_HANDLER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init lge_panic_handler_init(void)
{
	return platform_driver_register(&panic_handler_driver);
}

static void __exit lge_panic_handler_exit(void)
{
	platform_driver_unregister(&panic_handler_driver);
}

module_init(lge_panic_handler_init);
module_exit(lge_panic_handler_exit);

MODULE_DESCRIPTION("LGE panic handler driver");
MODULE_AUTHOR("SungEun Kim <cleaneye.kim@lge.com>");
MODULE_LICENSE("GPL");
