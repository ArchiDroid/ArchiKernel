/*
 * drivers/media/video/msm/lm2759_flash.c
 *
 * Flash (LM2759) driver
 *
 * Copyright (C) 2010 LGE, Inc.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <../../../drivers/staging/android/timed_output.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <mach/board_lge.h>
#include <mach/camera.h>

#define CAMERA_LED_OFF                 0
#define CAMERA_LED_LOW                 1
#define CAMERA_LED_HIGH                2
#define CAMERA_LED_AGC_STATE           3
#define CAMERA_LED_TORCH               4

/*for isx006 sensor*/
#define AGC_THRESHOLD                  0x06CC

/* Register Descriptions */
#define LM2759_REG_ENABLE              0x10
#define LM2759_REG_GPIO                0x20
#define LM2759_REG_VLED_MONITOR        0x30
#define LM2759_REG_ADC_DELAY           0x31
#define LM2759_REG_VIN_MONITOR         0x80
#define LM2759_REG_LAST_FLASH          0x81
#define LM2759_REG_TORCH_BRIGHTNESS    0xA0
#define LM2759_REG_FLASH_BRIGHTNESS    0xB0
#define LM2759_REG_FLASH_DURATION      0xC0
#define LM2759_REG_FLAGS               0xD0
#define LM2759_REG_CONFIGURATION1      0xE0
#define LM2759_REG_CONFIGURATION2      0xF0
#define LM2759_REG_PRIVACY             0x11
#define LM2759_REG_MESSAGE_INDICATOR   0x12
#define LM2759_REG_INDICATOR_BLINKING  0x13
#define LM2759_REG_PRIVACY_PWM         0x14

#define LM2759_I2C_NAME  "lm2759"

struct led_flash_platform_data *lm2759_pdata;
struct i2c_client *lm2759_client;

static int32_t lm2759_write_reg(struct i2c_client *client, unsigned char* buf,
	int length)
{
	int32_t err = 0;

	struct i2c_msg	msg[] = {
		{
			.addr  = client->addr,
			.flags = 0,
			.len   = length,
			.buf   = buf,
		},
	};

	err = i2c_transfer(client->adapter, &msg[0], 1);
	if (err < 0)
		dev_err(&client->dev, "i2c write error [%d]\n", err);

	return err;
}

static int32_t lm2759_i2c_write(struct i2c_client *client, unsigned char addr,
	unsigned char data)
{
	unsigned char buf[2] = {0};
	int32_t rc = -EIO;

	if (client == NULL)
		return rc;


	buf[0] = addr;
	buf[1] = data;

	rc = lm2759_write_reg(client, &buf[0], 2);

	return rc;
}

static int32_t lm2759_read_reg(struct i2c_client *client, unsigned char* buf,
	int length)
{
	int32_t err = 0;

	struct i2c_msg	msgs[] = {
		{
			.addr  = client->addr,
			.flags = 0,
			.len   = 2,
			.buf   = buf,
		},
		{
			.addr  = client->addr,
			.flags = I2C_M_RD,
			.len   = length,
			.buf   = buf,
		},
	};

	err = i2c_transfer(client->adapter, msgs, 1);
	if (err < 0)
		dev_err(&client->dev, "i2c write error [%d]\n", err);

	return err;
}

static int32_t lm2759_i2c_read(struct i2c_client *client, unsigned char addr,
	unsigned char *data)
{
	unsigned char buf[2] = {0};
	int32_t rc = -EIO;

	if (client == NULL || data == NULL)
		return rc;

	buf[0] = addr;

	rc = lm2759_read_reg(client, &buf[0], 1);
	if (rc < 0)
		return rc;

	*data = buf[0];

	return rc;
}

void lm2759_led_shutdown(void)
{
	lm2759_i2c_write(lm2759_client, LM2759_REG_ENABLE, 0x00);
}

void lm2759_enable_torch_mode(int state)
{
	unsigned char data = 0;

	lm2759_i2c_read(lm2759_client, LM2759_REG_TORCH_BRIGHTNESS, &data);
	data &= 0xF8;

	if (state == CAMERA_LED_LOW) {
		/* 000 : 28.125 mA (56.25 mA total) */
		lm2759_i2c_write(lm2759_client,
			LM2759_REG_TORCH_BRIGHTNESS, data);
	} else {
		/* 110 : 196.875 mA (393.75 mA total) */
		data |= 0x06;
		lm2759_i2c_write(lm2759_client,
			LM2759_REG_TORCH_BRIGHTNESS, data);
	}

	lm2759_i2c_write(lm2759_client, LM2759_REG_ENABLE, 0x01);
}

void lm2759_enable_flash_mode(int state)
{
	unsigned char data = 0;

	lm2759_i2c_read(lm2759_client, LM2759_REG_FLASH_BRIGHTNESS, &data);
	printk(KERN_ERR "lm2759_enable_flash_mode 01: %d", data);
	data &= 0xF0;
	if (state == CAMERA_LED_LOW) {
		/* 0000 : 56.25 mA (112.5 mA total) */
		lm2759_i2c_write(lm2759_client,
			LM2759_REG_FLASH_BRIGHTNESS, data);
	} else {
		/* 470 mA Default */
		data |= 0x06;
		lm2759_i2c_write(lm2759_client,
			LM2759_REG_FLASH_BRIGHTNESS, data);
	}

	lm2759_i2c_write(lm2759_client, LM2759_REG_ENABLE, 0x03);
}

int lm2759_flash_set_led_state(int led_state)
{
	int current_agc = 0;
	int rc = 0;

	switch (led_state) {
	case CAMERA_LED_OFF:
		lm2759_led_shutdown();
		break;

	case CAMERA_LED_LOW:
		lm2759_enable_flash_mode(CAMERA_LED_LOW);
		break;

	case CAMERA_LED_HIGH:
		lm2759_enable_flash_mode(CAMERA_LED_HIGH);
		break;

	case CAMERA_LED_AGC_STATE:
		if (current_agc >= AGC_THRESHOLD)
			lm2759_enable_flash_mode(CAMERA_LED_HIGH);
		break;
	case CAMERA_LED_TORCH:
		lm2759_enable_torch_mode(CAMERA_LED_HIGH);
		break;
	default:
		rc = -EFAULT;
		break;
	}

	return rc;
}
EXPORT_SYMBOL(lm2759_flash_set_led_state);

static int __devinit lm2759_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	if (i2c_get_clientdata(client))
		return -EBUSY;

	lm2759_client = client;
	/*lm2759_i2c_write(lm2759_client,LM2759_REG_ENABLE,0x11);*/

	return 0;
}

static int lm2759_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id lm2759_ids[] = {
	{ LM2759_I2C_NAME, 0 },  /* lm2759 */
	{ /* end of list */ },
};

static struct i2c_driver lm2759_driver = {
	.probe    = lm2759_probe,
	.remove   = lm2759_remove,
	.id_table = lm2759_ids,
	.driver   = {
		.name  = LM2759_I2C_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init lm2759_init(void)
{
	return i2c_add_driver(&lm2759_driver);
}

static void __exit lm2759_exit(void)
{
	i2c_del_driver(&lm2759_driver);
}

module_init(lm2759_init);
module_exit(lm2759_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("LM2759 Flash Driver");
MODULE_LICENSE("GPL");
