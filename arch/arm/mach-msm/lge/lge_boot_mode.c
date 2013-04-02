/*
 * arch/arm/mach-msm/lge/lge_boot_mode.c
 *
 * Copyright (C) 2012 LGE, Inc
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

#include <linux/module.h>
#include <linux/platform_device.h>
#include <mach/msm_smsm.h>
#include <asm/processor.h>
#include <mach/lge/lge_proc_comm.h>
#include <mach/lge/lge_boot_mode.h>
#include CONFIG_LGE_BOARD_HEADER_FILE

/* in MSM7k, Defines must sync with
 * modem/modem_proc/modem/rfa/pmic/devices/drivers/inc/pm_pwron.h
 */
#define  PM_PWR_ON_EVENT_KEYPAD		0x01
#define  PM_PWR_ON_EVENT_RTC       	0x02
#define  PM_PWR_ON_EVENT_CABLE		0x04
#define  PM_PWR_ON_EVENT_SMPL		0x08
#define  PM_PWR_ON_EVENT_WDOG		0x10
#define  PM_PWR_ON_EVENT_USB_CHG	0x20
#define  PM_PWR_ON_EVENT_WALL_CHG	0x40
#define  PM_PWR_ON_EVENT_FACTORY_USB	0x80
#define  PM_PWR_ON_EVENT_HARD_RESET	0x100

typedef enum
{
    USB_UNKOWN  = 0x00,
    USB_56K     = 0x02,
    USB_130K    = 0x04,
    USB_180K    = 0x05,
    USB_620K    = 0x09,
    USB_910K    = 0x0A,
    USB_OPEN    = 0x0B,
} cable_type_t;

/* LGE BOOT MODE Strings */
#define LGE_BOOT_MODE_UNKNOWN_STR		"unknown"
#define LGE_BOOT_MODE_CHARGER_STR		"charger"
#define	LGE_BOOT_MODE_FACTORY_STR		"factory"
//LGE_CHANGE_S FTM boot mode
#define LGE_BOOT_MODE_FIRST_STR		"first"
//LGE_CHANGE_E FTM boot mode
#define	LGE_BOOT_MODE_MINIOS_STR		"miniOS"

#define LGE_BOOT_MODE_CRASHNOSOUND_STR	"panic"
//LGE_CHANGE_S FTM boot mode
static const char *boot_mode_str[] = {
	LGE_BOOT_MODE_UNKNOWN_STR,
	LGE_BOOT_MODE_CHARGER_STR,
	LGE_BOOT_MODE_FACTORY_STR,
	LGE_BOOT_MODE_FIRST_STR,
	LGE_BOOT_MODE_MINIOS_STR,
	LGE_BOOT_MODE_CRASHNOSOUND_STR,
};
//LGE_CHANGE_E FTM boot mode

#define APPS_BOOT_MODE_UNKWOWN					0x77665501
#define APPS_BOOT_MODE_RECOVERY					0x77665502
#define APPS_BOOT_MODE_BNR_RECOVERY				0x77665555
#define APPS_BOOT_MODE_IDLE						0x776655AA
#define RESET_REASON_CRASH_NOSOUND				0x6480E000
#define REASON_APPS_UART_ENABLE      				0x618E0001

static int boot_mode = LGE_BOOT_MODE_UNKOWN;
static int apps_boot_mode = 0x0;

int get_lge_boot_mode(void)
{
	return boot_mode;
}
EXPORT_SYMBOL(get_lge_boot_mode);

static int update_apps_boot_mode(void)
{
	static int smem_apps_boot_mode;
// LGE_CHANGE_S, youngbae.choi@lge.com, Don't get the smem data. boot reason is received at lk.
#if 1
	smem_apps_boot_mode = get_reboot_mode();
#else
	unsigned smem_size;

	smem_apps_boot_mode = *(unsigned int *)
		(smem_get_entry(SMEM_APPS_BOOT_MODE, &smem_size));
#endif
// LGE_CHANGE_E, youngbae.choi@lge.com, Don't get the smem data. boot reason is received at lk.

	apps_boot_mode = smem_apps_boot_mode;

	printk(KERN_INFO "[BootMode] apps_boot_mode = 0x%08x\n", apps_boot_mode);

	return apps_boot_mode;
}

//LGE_CHANGE_S FTM boot mode
#if (defined (CONFIG_MACH_MSM7X25A_V3) && !defined (CONFIG_MACH_MSM7X25A_M4)) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
extern unsigned lge_nv_manual_f(int val);
#endif
//LGE_CHANGE_E FTM boot mode
static int update_lge_boot_mode(void)
{
	unsigned smem_size;
// 2012-11-05 Sonchiwon(chiwon.son@lge.com) [V3/V7][Hidden.Menu] HiddenMenu > Settings > Battery > Charging Bypass Boot [START]
    int test_mode, factory_cable, charger, charging_bypass;
//LGE_CHANGE_S FTM boot mode
#if (defined (CONFIG_MACH_MSM7X25A_V3) && !defined (CONFIG_MACH_MSM7X25A_M4)) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
    int first_boot_check = 0;
#endif
//LGE_CHANGE_E FTM boot mode
// 2012-11-05 Sonchiwon(chiwon.son@lge.com) [V3/V7][Hidden.Menu] HiddenMenu > Settings > Battery > Charging Bypass Boot [END]
#ifdef CONFIG_LGE_SUPPORT_MINIOS
	int cable_type = lge_get_cable_info();
#endif

// LGE_CHANGE_S, youngbae.choi@lge.com, Because of crash boot, don't check the other reason.
	if (apps_boot_mode == RESET_REASON_CRASH_NOSOUND) {
		boot_mode = LGE_BOOT_MODE_CRASH;
		printk(KERN_INFO "LGE boot mode is %s\n", boot_mode_str[boot_mode]);
		return boot_mode;
	}
// LGE_CHANGE_E, youngbae.choi@lge.com, Because of crash boot, don't check the other reason.

	/* Update boot reason */
	boot_reason = *(unsigned int *)
		(smem_get_entry(SMEM_POWER_ON_STATUS_INFO, &smem_size));
	/* filter pwr on status byte */
	boot_reason &= 0xFF;

//LGE_CHANGE_S FTM boot mode
#if (defined (CONFIG_MACH_MSM7X25A_V3) && !defined (CONFIG_MACH_MSM7X25A_M4)) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
	if (lge_get_fboot_mode() == first_boot) {
		lge_nv_manual_f(1);
		first_boot_check = 1;
		printk(KERN_INFO "[FTM] first boot -> change manual mode '1' \n");
	}
	test_mode = lge_get_nv_qem();
	printk(KERN_INFO "[FTM] check manual mode [%d] \n", test_mode);
#else
	test_mode = lge_get_nv_qem();
#endif
//LGE_CHANGE_E FTM boot mode
	factory_cable = boot_reason & PM_PWR_ON_EVENT_FACTORY_USB;
	charger = boot_reason & (PM_PWR_ON_EVENT_USB_CHG | PM_PWR_ON_EVENT_WALL_CHG);
// 2012-11-05 Sonchiwon(chiwon.son@lge.com) [V3/V7][Hidden.Menu] HiddenMenu > Settings > Battery > Charging Bypass Boot [START]
    charging_bypass = lge_get_nv_charging_bypass_boot();
// 2012-11-05 Sonchiwon(chiwon.son@lge.com) [V3/V7][Hidden.Menu] HiddenMenu > Settings > Battery > Charging Bypass Boot [END]

    if (charging_bypass == 1) {
        printk(KERN_INFO "charging_bypass is set\n");
        boot_mode = LGE_BOOT_MODE_UNKOWN;
    }
    else if (factory_cable) {
        printk(KERN_INFO "factory_cable is detected\n");
	if (test_mode) {
#ifdef CONFIG_LGE_SUPPORT_MINIOS
			if (cable_type == USB_130K) {
				boot_mode = LGE_BOOT_MODE_MINIOS;
			} else {
				boot_mode = LGE_BOOT_MODE_FACTORY;
			}
#else
			boot_mode = LGE_BOOT_MODE_UNKOWN;
#endif
	} else {

#ifdef CONFIG_LGE_SUPPORT_MINIOS
			if (cable_type == USB_130K) {
				boot_mode = LGE_BOOT_MODE_MINIOS;
			} else {
				boot_mode = LGE_BOOT_MODE_UNKOWN;
			}
#else
			boot_mode = LGE_BOOT_MODE_UNKOWN;
#endif
	}
	} 
	else if (charger && (apps_boot_mode == 0 || apps_boot_mode == REASON_APPS_UART_ENABLE)) {
        printk(KERN_INFO "charger and RESET_REASON_NONE\n");
		boot_mode = LGE_BOOT_MODE_CHARGER;
	} else if (test_mode) {
//LGE_CHANGE_S FTM boot mode
#if (defined (CONFIG_MACH_MSM7X25A_V3) && !defined (CONFIG_MACH_MSM7X25A_M4)) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
		if (first_boot_check == 1) {
		        printk(KERN_INFO "[FTM] first boot is set --> FTM boot\n");
			boot_mode = LGE_BOOT_MODE_FIRST;
		} else {
		        printk(KERN_INFO "[FTM] manual mode is set\n");
			boot_mode = LGE_BOOT_MODE_FACTORY;
		}
#else
        printk(KERN_INFO "test_mode is set\n");
			boot_mode = LGE_BOOT_MODE_FACTORY;
#endif
//LGE_CHANGE_E FTM boot mode
	} else {
        printk(KERN_INFO "All above check routines are passed.\n");
		boot_mode = LGE_BOOT_MODE_UNKOWN;
	}

	printk(KERN_INFO "[BootMode] boot_mode = %s\n", boot_mode_str[boot_mode]);
	return boot_mode;
}

/*LGE_CHANGE_S: seunhang.lee@lge.com 14/01/2013:Factory Reset with AT Command*/
/* LGE_CHANGE hyungjoon.jeon@lge.com 12/02/13 */
//#if defined(CONFIG_MACH_MSM7X27A_U0)
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_M4)
static ssize_t msm_nv_frststatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned char frststatus;

	frststatus = (unsigned char)lge_get_nv_frststatus();

	return sprintf(buf, "%d\n", frststatus);
}

static ssize_t msm_nv_frststatus_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int frststatus;

	sscanf(buf, "%d", &frststatus);

	pr_info("%s: do rmt_storage sync value (%d)!\n", __func__, frststatus);
	lge_set_nv_frststatus(frststatus);

	return count;
}

#endif
/*LGE_CHANGE_E: seunhang.lee@lge.com 14/01/2013:Factory Reset with AT Command*/

/*LGE_CHANGE_S: seunhang.lee@lge.com 21/01/2013:Send Sync*/
static ssize_t msm_nv_send_sync(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int sync_flag;

	sscanf(buf, "%d", &sync_flag);

	if ( sync_flag == 1 ) {
		pr_info("%s: do rmt_storage send sync!\n", __func__);
		lge_set_nv_send_sync();
	}
	return count;
}
/*LGE_CHANGE_E: seunhang.lee@lge.com 21/01/2013:Send Sync*/

/* 31/01/13,narasimha.chikka@lge.com,Add Boot chargertype Syfs */
static const char* get_boot_factory_cable(void)
{
	if(boot_reason & PM_PWR_ON_EVENT_FACTORY_USB)
		return "Y";
	else
		return "N";
}
/* 31/01/13,narasimha.chikka@lge.com,Add Boot chargertype Syfs */

/* sysfs */
static ssize_t cable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_cable_info());
}
static DEVICE_ATTR(cable, 0400, cable_show, NULL);

static ssize_t qem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_nv_qem());
}
static DEVICE_ATTR(qem, 0400, qem_show, NULL);

static ssize_t manual_testmode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lge_get_nv_qem());
}
static DEVICE_ATTR(manual_testmode, 0400, manual_testmode_show, NULL);

static ssize_t reset_reason_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", boot_mode_str[boot_mode]);
}
static DEVICE_ATTR(reset_reason, 0400, reset_reason_show, NULL);

static ssize_t boot_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", boot_mode_str[boot_mode]);
}
static DEVICE_ATTR(boot_mode, 0400, boot_mode_show, NULL);

/*LGE_CHANGE_S: seunhang.lee@lge.com 14/01/2013:Factory Reset with AT Command*/
/* LGE_CHANGE hyungjoon.jeon@lge.com 12/02/13 */
//#if defined(CONFIG_MACH_MSM7X27A_U0)
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_M4)
static DEVICE_ATTR(frststatus, 0660, msm_nv_frststatus_show, msm_nv_frststatus_store);
#endif
/*LGE_CHANGE_E: seunhang.lee@lge.com 14/01/2013:Factory Reset with AT Command*/	

/*LGE_CHANGE_S: seunhang.lee@lge.com 21/01/2013:Send Sync*/
static DEVICE_ATTR(send_sync, 0660, NULL, msm_nv_send_sync);
/*LGE_CHANGE_E: seunhang.lee@lge.com 21/01/2013:Send Sync*/

static ssize_t boot_factory_cable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", get_boot_factory_cable());
}
static DEVICE_ATTR(boot_factory_cable, 0400, boot_factory_cable_show, NULL);

static struct attribute* dev_attrs_bootmode_info[] = {
	&dev_attr_cable.attr,
	&dev_attr_qem.attr,
	&dev_attr_manual_testmode.attr,
	&dev_attr_reset_reason.attr,
	&dev_attr_boot_mode.attr,
/*LGE_CHANGE_S: seunhang.lee@lge.com 14/01/2013:Factory Reset with AT Command*/
/* LGE_CHANGE hyungjoon.jeon@lge.com 12/02/13 */	
//#if defined(CONFIG_MACH_MSM7X27A_U0)
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_M4)
    &dev_attr_frststatus.attr,
#endif
/*LGE_CHANGE_E: seunhang.lee@lge.com 14/01/2013:Factory Reset with AT Command*/	
/*LGE_CHANGE_S: seunhang.lee@lge.com 21/01/2013:Send Sync*/
    &dev_attr_send_sync.attr,
/*LGE_CHANGE_E: seunhang.lee@lge.com 21/01/2013:Send Sync*/
	&dev_attr_boot_factory_cable.attr,
	NULL,
};

static struct attribute_group dev_attr_grp_bootmode_info = {
	.attrs = dev_attrs_bootmode_info,
};

static int __devinit lge_boot_mode_probe(struct platform_device *pdev)
{
	int rc;

	update_apps_boot_mode();
	update_lge_boot_mode();

	rc = sysfs_create_group(&pdev->dev.kobj, &dev_attr_grp_bootmode_info);
	if(rc < 0) {
		dev_err(&pdev->dev,
			"%s: fail to create sysfs for lge_boot_mode rc=%d\n", __func__, rc);
	}

	return rc;
}

static int __devexit lge_boot_mode_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj,&dev_attr_grp_bootmode_info);

	return 0;
}

static struct platform_driver lge_boot_mode_driver = {
	.probe = lge_boot_mode_probe,
	.remove = __devexit_p(lge_boot_mode_remove),
	.driver = {
		.name = LGE_BOOT_MODE_DEVICE,
		.owner = THIS_MODULE,
	},
};

static int __init lge_boot_mode_init(void)
{
	return platform_driver_register(&lge_boot_mode_driver);
}
static void __exit lge_boot_mode_exit(void)
{
	platform_driver_unregister(&lge_boot_mode_driver);
}

late_initcall(lge_boot_mode_init);
module_exit(lge_boot_mode_exit);
