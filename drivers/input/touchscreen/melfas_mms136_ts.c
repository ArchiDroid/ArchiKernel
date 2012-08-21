/* drivers/input/touchscreen/melfas_ts.c
 *
 * Copyright (C) 2010 Melfas, Inc.
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

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/gpio.h> 
#include <mach/board_lge.h>

#define MIP_ENABLE 1

#define TS_MAX_Z_TOUCH   255
#define TS_MAX_W_TOUCH   255

#define TS_MAX_X_COORD   480 
#define TS_MAX_Y_COORD   800 
#define FW_VERSION       0x00

#define TS_READ_START_ADDR  0x10
#define TS_READ_VERSION_ADDR  0x31
#ifndef MIP_ENABLE
#define TS_READ_REGS_LEN 5
#else
#define TS_READ_REGS_LEN 100
#endif

#define MELFAS_MAX_TOUCH		10

#define I2C_RETRY_CNT			10

#define PRESS_KEY   1
#define RELEASE_KEY 0
#define DEBUG_PRINT 0
#define	SET_DOWNLOAD_BY_GPIO	0

#if SET_DOWNLOAD_BY_GPIO
#include <linux/melfas_download.h>
#endif

enum {
	None = 0,
	TOUCH_SCREEN,
	TOUCH_KEY
};

struct muti_touch_info {
	int strength;
	int width;
	int posX;
	int posY;
};

struct melfas_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct melfas_tsi_platform_data *pdata;
	struct work_struct  work;
	uint32_t flags;
	int (*power)(int on);
	int (*power_enable)(int en, bool log_en);
	struct early_suspend early_suspend;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif

static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];

int (*g_power_enable) (int en, bool log_en);


static char tmp_flag[10];

static void melfas_ts_work_func(struct work_struct *work)
{
	struct melfas_ts_data *ts =
		container_of(work, struct melfas_ts_data, work);
	int ret = 0, i;
	uint8_t buf[TS_READ_REGS_LEN];
	int touchType = 0, touchState = 0, touchID = 0;
	int posX = 0, posY = 0, width = 0, strength = 10;
	int keyID = 0, reportID = 0;
	uint8_t read_num = 0;


#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_work_func\n");

	if (ts == NULL)
			printk(KERN_ERR "melfas_ts_work_func : TS NULL\n");
#endif

#ifndef MIP_ENABLE
	buf[0] = TS_READ_START_ADDR;
	for (i = 0; i < I2C_RETRY_CNT; i++)	{
		ret = i2c_master_send(ts->client, buf, 1);
#if DEBUG_PRINT
		printk(KERN_ERR "%s : i2c_master_send [%d]\n", __func__, ret);
#endif
		if (ret >= 0) {
			ret = i2c_master_recv(ts->client, buf, TS_READ_REGS_LEN);
#if DEBUG_PRINT
			printk(KERN_ERR "%s : i2c_master_recv [%d]\n", __func__, ret);
			printk(KERN_ERR "%d %d %d %d %d\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
#endif
			if (ret >= 0)
				break;
		}
	}

	if (ret < 0) {
		printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
		return ;
	} else {
		touchType = (buf[0]>>5)&0x03;
		touchState = (buf[0]>>4)&0x01;
		reportID = (buf[0]&0x0f);
		posX = ((buf[1] & 0x0F) << (8)) + buf[2];
		posY = (((buf[1] & 0xF0) >> 4) << (8)) + buf[3];
		width = buf[4];
		if (touchType == 2)
			keyID = reportID;
		else
			keyID = reportID;

		touchID = reportID-1;

		if (touchID > MELFAS_MAX_TOUCH-1) {
#if DEBUG_PRINT
			printk(KERN_ERR "%s : Touch ID:%d\n", __func__, touchID);
#endif
			return ;
		}

	if (touchType == TOUCH_SCREEN) {
		g_Mtouch_info[touchID].posX = posX;
		g_Mtouch_info[touchID].posY = posY;
		g_Mtouch_info[touchID].width = width;

		if (touchState)
			g_Mtouch_info[touchID].strength = strength;
		else {
			g_Mtouch_info[touchID].strength = 0;
			tmp_flag[touchID] = 1;
		}

		for (i = 0; i < MELFAS_MAX_TOUCH; i++) {
			if (g_Mtouch_info[i].strength == -1)
				continue;

			input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength);
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);
			input_mt_sync(ts->input_dev);
			input_report_key(ts->input_dev, BTN_TOUCH, touchState);
#if DEBUG_PRINT
			if ((touchState == 1 && tmp_flag[touchID] == 1) || (touchState == 0 && tmp_flag[touchID] == 1)) {
				printk(KERN_ERR "Touch ID: %d, State : %d, x: %d, y: %d, z: %d w: %d\n",
					   i, touchState, g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength, g_Mtouch_info[i].width);
				if (touchState == 1)
					tmp_flag[touchID] = 0;
			}
#endif
			if (g_Mtouch_info[i].strength == 0)
				g_Mtouch_info[i].strength = -1;
		}
	} else if (touchType == TOUCH_KEY) {
		if (keyID == 0x1)
			input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);
		if (keyID == 0x2)/* LGE_CHANGE_S  [yoonsoo.kim@lge.com]  20111109  : U0 Rev.B Touch Back Key Fixed*/
			input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);
		if (keyID == 0x3)
			input_report_key(ts->input_dev, KEY_HOME, touchState ? PRESS_KEY : RELEASE_KEY);
		if (keyID == 0x4) /* LGE_CHANGE_E  [yoonsoo.kim@lge.com]  20111109  : U0 Rev.B Touch Back Key Fixed */
			input_report_key(ts->input_dev, KEY_SEARCH, touchState ? PRESS_KEY : RELEASE_KEY);
#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_work_func: keyID : %d, touchState: %d\n", keyID, touchState);
#endif
	}

	input_sync(ts->input_dev);
}



#else

#define MIP_INPUT_EVENT_PACKET_SIZE	0x0F
#define MIP_INPUT_EVENT_INFORMATION	0x10

	buf[0] = MIP_INPUT_EVENT_PACKET_SIZE;
	ret = i2c_master_send(ts->client, buf, 1);
	ret = i2c_master_recv(ts->client, &read_num, 1);

	if (read_num == 0) {
		printk(KERN_ERR "read number 0 error!!!!\n");
		enable_irq(ts->client->irq);
		return;
	}

	buf[0] = MIP_INPUT_EVENT_INFORMATION;
	ret = i2c_master_send(ts->client, buf, 1);
	ret = i2c_master_recv(ts->client, &buf[0], read_num);

	for (i = 0; i < read_num; i = i + 6) {
		if (ret < 0) {
			printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
			enable_irq(ts->client->irq);
			return ;
		} else {
			touchType  =  ((buf[i] & 0x60) >> 5);
			touchState = ((buf[i] & 0x80) == 0x80);
			reportID = (buf[i] & 0x0F);
			posX = (uint16_t) (buf[i + 1] & 0x0F) << 8 | buf[i + 2];
			posY = (uint16_t) (buf[i + 1] & 0xF0) << 4 | buf[i + 3];
			width = buf[i + 4];
			if (touchType == 2)
				keyID = reportID;
			else
				keyID = reportID;

			touchID = reportID-1;

			if (touchID > MELFAS_MAX_TOUCH-1) {
			    return;
				enable_irq(ts->client->irq);
			}

			if (touchType == TOUCH_SCREEN) {
				g_Mtouch_info[touchID].posX = posX;
				g_Mtouch_info[touchID].posY = posY;
				g_Mtouch_info[touchID].width = width;

				if (touchState)
					g_Mtouch_info[touchID].strength = strength;
				else {
					g_Mtouch_info[touchID].strength = 0;
					tmp_flag[touchID] = 1;
				}
			} else if (touchType == TOUCH_KEY) {
				if (keyID == 0x1)
					input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);
				if (keyID == 0x2) /* LGE_CHANGE_S [yoonsoo.kim@lge.com] 20111109 : U0 Rev.B Touch Back Key Fixed */
					input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);
				if (keyID == 0x3)
					input_report_key(ts->input_dev, KEY_HOME, touchState ? PRESS_KEY : RELEASE_KEY);
				if (keyID == 0x4)/* LGE_CHANGE_E  [yoonsoo.kim@lge.com]  20111109  : U0 Rev.B Touch Back Key Fixed */
					input_report_key(ts->input_dev, KEY_SEARCH, touchState ? PRESS_KEY : RELEASE_KEY);
#if DEBUG_PRINT
				printk(KERN_ERR "melfas_ts_work_func: keyID : %d, touchState: %d\n", keyID, touchState);
#endif
				break;
			}

		}
	}

	if (touchType == TOUCH_SCREEN) {
		for (i = 0; i < MELFAS_MAX_TOUCH; i++) {
			if (g_Mtouch_info[i].strength == -1)
				continue;

			input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength);
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);
			input_mt_sync(ts->input_dev);
			input_report_key(ts->input_dev, BTN_TOUCH, touchState);
#if DEBUG_PRINT
			if ((touchState == 1 && tmp_flag[touchID] == 1) || (touchState == 0 && tmp_flag[touchID] == 1)) {
				printk(KERN_ERR "Touch ID: %d, State : %d, x: %d, y: %d, z: %d w: %d\n",
					   i, touchState, g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength, g_Mtouch_info[i].width);
				if (touchState == 1)
					tmp_flag[touchID] = 0;
			}
#endif
			if (g_Mtouch_info[i].strength == 0)
				g_Mtouch_info[i].strength = -1;

		}
	}

	input_sync(ts->input_dev);
#endif

	enable_irq(ts->client->irq);
}

static irqreturn_t melfas_ts_irq_handler(int irq, void *handle)
{
	struct melfas_ts_data *ts = (struct melfas_ts_data *)handle;
#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_irq_handler\n");
#endif

	disable_irq_nosync(ts->client->irq);
	schedule_work(&ts->work);

	return IRQ_HANDLED;
}

static int melfas_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct melfas_ts_data *ts;
	int ret = 0, i;

	uint8_t buf;

#if DEBUG_PRINT
	printk(KERN_ERR "Touch : melfas_ts_probe Start!!!\n");
#endif

	memset(&tmp_flag[0], 0x01, sizeof(tmp_flag));


	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "melfas_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	ts = kmalloc(sizeof(struct melfas_ts_data), GFP_KERNEL);
	if (ts == NULL) {
		printk(KERN_ERR "melfas_ts_probe: failed to create a state of melfas-ts\n");
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
	ts->pdata = client->dev.platform_data;
	if (ts->pdata->power_enable)
		ts->power_enable = ts->pdata->power_enable;

	disable_irq(client->irq);
	ret = ts->pdata->power_enable(1, true);
	enable_irq(client->irq);
	mdelay(100);

	g_power_enable = ts->pdata->power_enable;

	INIT_WORK(&ts->work, melfas_ts_work_func);

	ts->client = client;
	i2c_set_clientdata(client, ts);

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_master_send(ts->client, &buf, 1);
		if (ret >= 0) {
			printk(KERN_ERR "melfas_ts_probe: i2c_master_send() ok [%d]\n", ret);
			break;
		} else
			printk(KERN_ERR "melfas_ts_probe: i2c_master_send() failed[%d]\n", ret);
	}

	buf = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(ts->client, &buf, 1);
	ret = i2c_master_recv(ts->client, &buf, 1); 

	printk(KERN_INFO "version :: %d\n", buf);

#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_probe: i2c_master_send() [%d], Add[%d]\n", ret, ts->client->addr);
#endif



#if SET_DOWNLOAD_BY_GPIO
	buf = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(ts->client, &buf, 1);
	if (ret < 0) {
		printk(KERN_ERR "melfas_probe : i2c_master_send [%d]\n", ret);
	}

	ret = i2c_master_recv(ts->client, &buf, 1);
	if (ret < 0)	{
		printk(KERN_ERR "melfas_probe : i2c_master_recv [%d]\n", ret);
	}

	printk(KERN_ERR "melfas_probe : buf %d \n", buf);
	if (buf > FW_VERSION) {
		printk(KERN_ERR "melfas_probe : download start \n");
		mms100_download();
		if (ret > 0)
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
		}

	ret = ts->pdata->power_enable(0, true);
	mdelay(50);
	ret = ts->pdata->power_enable(1, true);

#endif

	ts->input_dev = input_allocate_device();
	if (!ts->input_dev) {
		printk(KERN_ERR "melfas_ts_probe: Not enough memory\n");
		ret = -ENOMEM;
		goto err_input_dev_alloc_failed;
	}

	ts->input_dev->name = "melfas-ts" ;

	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);


	ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
	ts->input_dev->keybit[BIT_WORD(KEY_HOME)] |= BIT_MASK(KEY_HOME);
	ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);
	ts->input_dev->keybit[BIT_WORD(KEY_SEARCH)] |= BIT_MASK(KEY_SEARCH);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, TS_MAX_X_COORD, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, TS_MAX_Y_COORD, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, MELFAS_MAX_TOUCH-1, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0);

	ret = input_register_device(ts->input_dev);
	if (ret) {
		printk(KERN_ERR "melfas_ts_probe: Failed to register device\n");
		ret = -ENOMEM;
		goto err_input_register_device_failed;
	}

	ret = gpio_request( (client->irq) - NR_MSM_IRQS, "melfas-ts");
	if (ret < 0) {
		printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
		return ret;
	}

	ret = gpio_direction_input( (client->irq) - NR_MSM_IRQS);
	if (ret < 0) {
		printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
		return ret;
	}
	ret = request_threaded_irq(client->irq, NULL, melfas_ts_irq_handler,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT, ts->client->name, ts);

	if (ret < 0) {
		printk(KERN_ERR "%s: request_irq failed\n", __FUNCTION__);
		ret = -EBUSY;
		goto err_request_irq;
	}

	
	for (i = 0; i < MELFAS_MAX_TOUCH ; i++)
		g_Mtouch_info[i].strength = -1;

#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_probe: succeed to register input device\n");
#endif

#if CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = melfas_ts_early_suspend;
	ts->early_suspend.resume = melfas_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

#if DEBUG_PRINT
	printk(KERN_INFO "melfas_ts_probe: Start touchscreen. name: %s, irq: %d\n", ts->client->name, ts->client->irq);
#endif
	return 0;

err_request_irq:
	printk(KERN_ERR "melfas-ts: err_request_irq failed\n");
	free_irq(client->irq, ts);
err_input_register_device_failed:
	printk(KERN_ERR "melfas-ts: err_input_register_device failed\n");
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
	printk(KERN_ERR "melfas-ts: err_input_dev_alloc failed\n");
err_alloc_data_failed:
	printk(KERN_ERR "melfas-ts: err_alloc_data failed_\n");
err_check_functionality_failed:
	printk(KERN_ERR "melfas-ts: err_check_functionality failed_\n");

	return ret;
}

static int melfas_ts_remove(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

	unregister_early_suspend(&ts->early_suspend);
	free_irq(client->irq, ts);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static void release_all_fingers(struct melfas_ts_data *ts)
{
	int i;
	for (i = 0; i < MELFAS_MAX_TOUCH; i++) {
		if (-1 == g_Mtouch_info[i].strength) {
			g_Mtouch_info[i].posX = 0;
			g_Mtouch_info[i].posY = 0;
			continue;
		}

		g_Mtouch_info[i].strength = 0;

		input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength);
		input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);
		input_report_key(ts->input_dev, BTN_TOUCH, 0);
		input_mt_sync(ts->input_dev);

		g_Mtouch_info[i].posX = 0;
		g_Mtouch_info[i].posY = 0;

		if (0 == g_Mtouch_info[i].strength)
			g_Mtouch_info[i].strength = -1;
	}
}


static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

	printk(KERN_ERR "melfas_ts_suspend start \n");
	release_all_fingers(ts);
	disable_irq(client->irq);

	ret = cancel_work_sync(&ts->work);
	/*if (ret) //if work was pending disable-count is now 2
		enable_irq(client->irq);

	ret = i2c_smbus_write_byte_data(client, 0x01, 0x00); // deep sleep*/

	ret = ts->power_enable(0, true);

	if (ret < 0)
		printk(KERN_ERR "melfas_ts_suspend: i2c_smbus_write_byte_data failed\n");

	printk(KERN_ERR "melfas_ts_suspend end \n");
	return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);
	int ret;

	printk(KERN_ERR "melfas_ts_resume start \n");

	ret = ts->power_enable(1, true);
	mdelay(50);
	enable_irq(client->irq);
	printk(KERN_ERR "melfas_ts_resume end \n");

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h)
{
	struct melfas_ts_data *ts;
	ts = container_of(h, struct melfas_ts_data, early_suspend);
	melfas_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void melfas_ts_late_resume(struct early_suspend *h)
{
	struct melfas_ts_data *ts;
	ts = container_of(h, struct melfas_ts_data, early_suspend);
	melfas_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id melfas_ts_id[] = {
	{ MELFAS_TS_NAME, 0 },
	{ }
};

static struct i2c_driver melfas_ts_driver = {
	.driver		= {
		.name	= MELFAS_TS_NAME,
	},
	.id_table		= melfas_ts_id,
	.probe		= melfas_ts_probe,
	.remove		= __devexit_p (melfas_ts_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend		= melfas_ts_suspend,
	.resume		= melfas_ts_resume,
#endif
};

static int __devinit melfas_ts_init(void)
{
	return i2c_add_driver(&melfas_ts_driver);
}

static void __exit melfas_ts_exit(void)
{
	i2c_del_driver(&melfas_ts_driver);
}

MODULE_DESCRIPTION("Driver for Melfas MTSI Touchscreen Controller");
MODULE_AUTHOR("MinSang, Kim <kimms@melfas.com>");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

late_initcall(melfas_ts_init);
module_exit(melfas_ts_exit);
