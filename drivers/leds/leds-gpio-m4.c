/*
 * Phoenix Keypad LED Driver for the OMAP4430 SDP
 *
 * Copyright (C) 2010 Texas Instruments
 *
 * Author: Dan Murphy <DMurphy@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/gpio.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <mach/board_lge.h>

#define KP_LEDS_GPIO 23
int 	kp_leds_gp = 0;

struct keypad_led_data {
	struct led_classdev keypad_led_class_dev;
};

static void m4_keypad_led_store(struct led_classdev *led_cdev,
				enum led_brightness value)
{
	if (value > LED_OFF) {
		gpio_set_value(kp_leds_gp, 1);
	} else {
		gpio_set_value(kp_leds_gp, 0);
	}
}

static int __devinit m4_keypad_led_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct keypad_led_data *info;

	kp_leds_gp = KP_LEDS_GPIO;
	
	ret = gpio_request(kp_leds_gp, "kp_leds_gpio"); 
	if(ret){
		printk(KERN_ERR "[kp_led]: request gpio %d failed!\n", KP_LEDS_GPIO);
		return ret;
	}
		
	ret = gpio_direction_output(kp_leds_gp, 0); 
	if(ret){
		printk(KERN_ERR"[kp_led]: set gpio %d direction out error!\n", KP_LEDS_GPIO);
		return ret;
	}
	
	info = kzalloc(sizeof(struct keypad_led_data), GFP_KERNEL);
	if (info == NULL) {
		ret = -ENOMEM;
		return ret;
	}

	platform_set_drvdata(pdev, info);

	info->keypad_led_class_dev.name = "button-backlight";
	info->keypad_led_class_dev.brightness_set =
			m4_keypad_led_store;
	info->keypad_led_class_dev.max_brightness = LED_FULL;

	ret = led_classdev_register(&pdev->dev,
				    &info->keypad_led_class_dev);
	if (ret < 0) {
		pr_err("[kp_led]: %s: Register led class failed\n", __func__);
		kfree(info);
		return ret;
	}
	return ret;
}

static int m4_keypad_led_remove(struct platform_device *pdev)
{
	struct keypad_led_data *info = platform_get_drvdata(pdev);
	led_classdev_unregister(&info->keypad_led_class_dev);
	return 0;
}

static struct platform_driver m4_keypad_led_driver = {
	.probe = m4_keypad_led_probe,
	.remove = m4_keypad_led_remove,
	.driver = {
		.name = "keypad_led",
		.owner = THIS_MODULE,
	},
};

static int __init m4_keypad_led_init(void)
{
	return platform_driver_register(&m4_keypad_led_driver);
}

static void __exit m4_keypad_led_exit(void)
{
	platform_driver_unregister(&m4_keypad_led_driver);
}

module_init(m4_keypad_led_init);
module_exit(m4_keypad_led_exit);

MODULE_DESCRIPTION("M4 Keypad Lighting driver");
MODULE_AUTHOR("xuming <xuming.chen@agreeya.com");
MODULE_LICENSE("GPL");