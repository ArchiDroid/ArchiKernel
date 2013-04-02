/*
 * leds-msm-pmic.c - MSM PMIC LEDs driver.
 *
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/module.h>

#include <mach/pmic.h>

#define MAX_KEYPAD_BL_LEVEL	16

static void msm_keypad_bl_led_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
	
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined (CONFIG_MACH_MSM7X25A_M4)
	int on_off;
	int brightness;

	/* debug mesg */
	printk(KERN_INFO " LED DEBUG: Value is %d\n", value);
	if (value == 0)
		on_off = (int)PM_MPP__I_SINK__SWITCH_DIS;
	else
		on_off = (int)PM_MPP__I_SINK__SWITCH_ENA;

#if defined (CONFIG_MACH_MSM7X25A_M4)
	brightness = PM_MPP__I_SINK__LEVEL_10mA;
#else
	brightness = PM_MPP__I_SINK__LEVEL_5mA;
#endif	
#if 0
	switch (value) {
	case 5:
		brightness = PM_MPP__I_SINK__LEVEL_5mA;
		break;
	case 10:
		brightness = PM_MPP__I_SINK__LEVEL_10mA;
		break;
	case 15:
		brightness = PM_MPP__I_SINK__LEVEL_15mA;
		break;
	case 20:
		brightness = PM_MPP__I_SINK__LEVEL_20mA;
		break;
	case 25:
		brightness = PM_MPP__I_SINK__LEVEL_25mA;
		break;
	case 30:
		brightness = PM_MPP__I_SINK__LEVEL_30mA;
		break;
	case 35:
		brightness = PM_MPP__I_SINK__LEVEL_35mA;
		break;
	case 40:
		brightness = PM_MPP__I_SINK__LEVEL_40mA;
		break;
	default:/* LGE_CHANGE  [yoonsoo.kim@lge.com]  20120223  : LED Current Reduce 25 -> 5mA */
		brightness = PM_MPP__I_SINK__LEVEL_5mA;
		break;
	}
	if (lge_bd_rev == LGE_REV_A) {
	/* LED power(MPP pin) use
	*REV.A
	*EU	:MPP3, MPP4
	*MPCS	:MPP4
	*/
	pmic_secure_mpp_config_i_sink((enum mpp_which)PM_MPP_4, brightness, (enum mpp_i_sink_switch)on_off);
	} else if (lge_bd_rev >= LGE_REV_B) {
	/* LED power(MPP pin) use
	*REV.B
	*EU	:MPP3
	*MPCS	:MPP3
	*/
#endif
#if defined (CONFIG_MACH_MSM7X25A_M4)
	pmic_secure_mpp_config_i_sink((enum mpp_which)PM_MPP_7, brightness, (enum mpp_i_sink_switch)on_off);
#else
	pmic_secure_mpp_config_i_sink((enum mpp_which)PM_MPP_3, brightness, (enum mpp_i_sink_switch)on_off);
#endif	
//	}

#else	//CONFIG_MACH_MSM7X27A_U0
	int ret;
	ret = pmic_set_led_intensity(LED_KEYPAD, value / MAX_KEYPAD_BL_LEVEL);
	if (ret)
		dev_err(led_cdev->dev, "can't set keypad backlight\n");
#endif
}

static struct led_classdev msm_kp_bl_led = {
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined (CONFIG_MACH_MSM7X25A_M4)
	.name			= "button-backlight",
#else
	.name			= "keyboard-backlight",
#endif
	.brightness_set		= msm_keypad_bl_led_set,
	.brightness		= LED_OFF,
};

static int msm_pmic_led_probe(struct platform_device *pdev)
{
	int rc;

	rc = led_classdev_register(&pdev->dev, &msm_kp_bl_led);
	if (rc) {
		dev_err(&pdev->dev, "unable to register led class driver\n");
		return rc;
	}
#if 0//defined(CONFIG_MACH_MSM7X27A_U0)
	/* LGE_CHANGE_S: [murali.ramaiah@lge.com]: 2012-03-23,
	Enabled key backlight leds till idle screen */
	msm_keypad_bl_led_set(&msm_kp_bl_led, 5); /* 5mA Brightness */
#else//CONFIG_MACH_MSM7X27A_U0
	msm_keypad_bl_led_set(&msm_kp_bl_led, LED_OFF);
#endif
	return rc;
}

static int __devexit msm_pmic_led_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&msm_kp_bl_led);

	return 0;
}

#ifdef CONFIG_PM
static int msm_pmic_led_suspend(struct platform_device *dev,
		pm_message_t state)
{
	led_classdev_suspend(&msm_kp_bl_led);

	return 0;
}

static int msm_pmic_led_resume(struct platform_device *dev)
{
	led_classdev_resume(&msm_kp_bl_led);

	return 0;
}
#else
#define msm_pmic_led_suspend NULL
#define msm_pmic_led_resume NULL
#endif

static struct platform_driver msm_pmic_led_driver = {
	.probe		= msm_pmic_led_probe,
	.remove		= __devexit_p(msm_pmic_led_remove),
	.suspend	= msm_pmic_led_suspend,
	.resume		= msm_pmic_led_resume,
	.driver		= {
		.name	= "pmic-leds",
		.owner	= THIS_MODULE,
	},
};

static int __init msm_pmic_led_init(void)
{
	return platform_driver_register(&msm_pmic_led_driver);
}
module_init(msm_pmic_led_init);

static void __exit msm_pmic_led_exit(void)
{
	platform_driver_unregister(&msm_pmic_led_driver);
}
module_exit(msm_pmic_led_exit);

MODULE_DESCRIPTION("MSM PMIC LEDs driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:pmic-leds");
