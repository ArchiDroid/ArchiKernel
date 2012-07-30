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
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/i2c-gpio.h>
#include <mach/board_lge.h>
#include "melfas_ts.h"
#define TS_MAX_Z_TOUCH			255
#define TS_MAX_W_TOUCH		30


#define TS_MAX_X_COORD 		320 
#define TS_MAX_Y_COORD 		480 

#define FW_VERSION				0x14

#define TS_READ_START_ADDR 	0x0F
#define TS_READ_START_ADDR2 	0x10
#define TS_READ_VERSION_ADDR	0xF0
#define TSP_REVISION		0xF0
#define HARDWARE_REVISION	0xF1
#define COMPATIBILITY_GROUP	0xF2
#define CORE_FIRMWARE_VERSION   0xF3

#define TS_LATEST_FW_VERSION_A	0x18
#define TS_LATEST_FW_VERSION_B	0x1e //0x1b
#define TS_READ_REGS_LEN 		66
#define MELFAS_MAX_TOUCH		5

#define DEBUG_PRINT 			0

#define SET_DOWNLOAD_BY_GPIO	1


#if SET_DOWNLOAD_BY_GPIO
#include <mcs8000_download.h>
#endif // SET_DOWNLOAD_BY_GPIO

enum {
	None = 0,
	TOUCH_SCREEN,
	TOUCH_KEY
};

struct muti_touch_info
{
	int strength;
	int width;	
	int posX;
	int posY;
};

struct melfas_ts_data 
{
	uint16_t addr;
	struct i2c_client *client; 
	struct input_dev *input_dev;
	struct work_struct  work;
	uint32_t flags;
	int num_irq;
	int intr_gpio;
	int scl_gpio;
	int sda_gpio;
	int (*power)(unsigned char onoff);
	struct early_suspend early_suspend;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif

static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];
unsigned char ex_fw_ver;


void Send_Touch(unsigned int x, unsigned int y)
{
/*	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs8000_ts_dev.input_dev);
	input_sync(mcs8000_ts_dev.input_dev);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs8000_ts_dev.input_dev);
	input_sync(mcs8000_ts_dev.input_dev);
*/
}
EXPORT_SYMBOL(Send_Touch);

/*
static int melfas_init_panel(struct melfas_ts_data *ts)
{
	//int buf = 0x00;
	uint8_t buf[1];
	int ret;
	
	buf[0]=0x00;
	ret = i2c_master_send(ts->client, buf, 1);

	if(ret <0)
	{
		printk(KERN_ERR "melfas_ts_probe: i2c_master_send() failed\n [%d]", ret);	
		return 0;
	}

	return true;
}
*/



static void release_all_finger(struct melfas_ts_data *ts)
{
	int i;
		
	
	for (i = 0; i < MELFAS_MAX_TOUCH; i++) {
		if (g_Mtouch_info[i].strength == -1){
			g_Mtouch_info[i].posX = 0;
			g_Mtouch_info[i].posY = 0;
			continue;
		}
		g_Mtouch_info[i].width = 0;
		g_Mtouch_info[i].strength = 0;

		input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength);
		input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);
		input_mt_sync(ts->input_dev);
		
		if (g_Mtouch_info[i].strength == 0)
			g_Mtouch_info[i].strength = -1;
			g_Mtouch_info[i].posX = 0;
			g_Mtouch_info[i].posY = 0;	
	}
	input_sync(ts->input_dev);	 	
}

static void melfas_ts_work_func(struct work_struct *work)
{
	struct melfas_ts_data *ts = container_of(work, struct melfas_ts_data, work);
	int ret = 0, i;
	int touchType = 0, keyID = 0, touchState=0;
	uint8_t buf[TS_READ_REGS_LEN];
	int read_num, FingerID;


#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_work_func\n");

	if(ts ==NULL)
			printk(KERN_ERR "melfas_ts_work_func : TS NULL\n");
#endif 

	buf[0] = TS_READ_START_ADDR;

	ret = i2c_master_send(ts->client, buf, 1);
	if(ret < 0){
#if DEBUG_PRINT
		printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
		enable_irq(ts->client->irq);
		return ;	
#endif 
	}
	
	ret = i2c_master_recv(ts->client, buf, 1);
	if(ret < 0){
#if DEBUG_PRINT
		printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
		enable_irq(ts->client->irq);
		return ;	
#endif 
	}

	read_num = buf[0];
//	printk("mcs800 read_num=%d\n",read_num);
	if(read_num>0){
		buf[0] = TS_READ_START_ADDR2;

		ret = i2c_master_send(ts->client, buf, 1);
		if(ret < 0){
#if DEBUG_PRINT
			printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
			enable_irq(ts->client->irq);
			return ;	
#endif 
		}
		ret = i2c_master_recv(ts->client, buf, read_num);
		if(ret < 0){
#if DEBUG_PRINT
			printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
			enable_irq(ts->client->irq);
			return ;	
#endif 
		}
	
		for(i=0; i<read_num; i=i+6){
			FingerID = (buf[i] & 0x0F)-1;

			g_Mtouch_info[FingerID].posX= (uint16_t)(buf[i+1] & 0x0F) << 8 | buf[i+2];
			g_Mtouch_info[FingerID].posY= (uint16_t)(buf[i+1] & 0xF0) << 4 | buf[i+3];	
			
			if((buf[i] & 0x80)==0)
				g_Mtouch_info[FingerID].strength = 0;
			else
				g_Mtouch_info[FingerID].strength = buf[i+4];
			
			g_Mtouch_info[FingerID].width= buf[i+5];					
		}
	
	}

	if (ret < 0){
		printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
		enable_irq(ts->client->irq);
		return ;	
	}
	else{
		if( buf[0] == 0x0f ){
			release_all_finger(ts);
			ts->power(0);
			msleep(100);
			ts->power(1);
			enable_irq(ts->client->irq);
			return;
		}	
	touchType  = (buf[0]>>5)&0x03;
	touchState = buf[0]&0x80;
	  if(touchType == TOUCH_SCREEN) {
		for(i=0; i<MELFAS_MAX_TOUCH; i++){
			if(g_Mtouch_info[i].strength== -1)
				continue;
			
			input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength );
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);      				
			input_mt_sync(ts->input_dev);          
#if DEBUG_PRINT
			printk(KERN_ERR "melfas_ts_work_func: Touch ID: %d, State : %d, x: %d, y: %d, z: %d w: %d\n", 
				i, (g_Mtouch_info[i].strength>0), g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength, g_Mtouch_info[i].width);
#endif	

			if(g_Mtouch_info[i].strength == 0)
				g_Mtouch_info[i].strength = -1;
		}
	}
	else if (touchType == TOUCH_KEY){
		
		keyID = (buf[0]&0x0f);
			if (keyID == 0x1)
				input_report_key(ts->input_dev, KEY_MENU, touchState ? 1 : 0);
			if (keyID == 0x2)
				input_report_key(ts->input_dev, KEY_HOME, touchState ? 1 : 0);
			if (keyID == 0x3)
				input_report_key(ts->input_dev, KEY_BACK, touchState ? 1 : 0);
			if (keyID == 0x4)
				input_report_key(ts->input_dev, KEY_SEARCH, touchState ? 1 : 0);
	}		
		input_sync(ts->input_dev);
	}
			
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
void melfas_firmware_info(struct melfas_ts_data *ts,unsigned char *fw_ver, unsigned char *hw_ver, unsigned char *comp_ver)
{
	unsigned char data;

	msleep(200); 

	i2c_smbus_write_byte(ts->client, 0x30);
	data = i2c_smbus_read_byte(ts->client);
	msleep(10);
	printk(KERN_INFO "HARDWARE REVISION [0x%x]\n", data);
	*hw_ver = data;

	i2c_smbus_write_byte(ts->client, 0x32);
	data = i2c_smbus_read_byte(ts->client);
	msleep(10);
	printk(KERN_INFO "COMPATIBILITY_GROUP [0x%x]\n", data);
	*comp_ver = data;
	

	i2c_smbus_write_byte(ts->client, 0x31);
	data = i2c_smbus_read_byte(ts->client);
	msleep(10);
	printk(KERN_INFO "CORE_FIRMWARE_VERSION [0x%x]\n", data);
	*fw_ver = data;
	
	ex_fw_ver=*fw_ver;
}

static ssize_t read_touch_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "M3 Touch Ver:%02x\n",ex_fw_ver);
}

static DEVICE_ATTR(version, S_IRUGO /*| S_IWUSR*/, read_touch_version, NULL);

static int melfas_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct melfas_ts_data *ts;
	struct touch_platform_data *ts_pdata;
	int ret = 0, i; 
	unsigned char fw_ver, hw_ver, comp_ver;
	uint8_t buf[4] = {0,};
	
#if DEBUG_PRINT
	printk(KERN_ERR "kim ms : melfas_ts_probe\n");
#endif
	ts_pdata = client->dev.platform_data;
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        printk(KERN_ERR "melfas_ts_probe: need I2C_FUNC_I2C\n");
        ret = -ENODEV;
        goto err_check_functionality_failed;
    }

    ts = kmalloc(sizeof(struct melfas_ts_data), GFP_KERNEL);
    if (ts == NULL)
    {
        printk(KERN_ERR "melfas_ts_probe: failed to create a state of melfas-ts\n");
        ret = -ENOMEM;
        goto err_alloc_data_failed;
    }

   	INIT_WORK(&ts->work, melfas_ts_work_func);
	ts->power = ts_pdata->power;
	ts->num_irq = client->irq;
	ts->intr_gpio	= (client->irq) - NR_MSM_IRQS ;
	ts->sda_gpio = ts_pdata->sda;
	ts->scl_gpio  = ts_pdata->scl;
	ts->client = client;
    	i2c_set_clientdata(client, ts);
    	ret = i2c_master_send(ts->client, buf, 1);

#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_probe: i2c_master_send() [%d], Add[%d]\n", ret, ts->client->addr);
#endif
	
	ts->input_dev = input_allocate_device();
	
	if (!ts->input_dev){
		printk(KERN_ERR "melfas_ts_probe: Not enough memory\n");
		ret = -ENOMEM;
		goto err_input_dev_alloc_failed;
	} 

	ts->input_dev->name = "touch_mcs8000" ;

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
	if (ret){
        printk(KERN_ERR "melfas_ts_probe: Failed to register device\n");
        ret = -ENOMEM;
        goto err_input_register_device_failed;
	}

	ret = gpio_request(ts->intr_gpio, "touch_mcs8000");
	if (ret < 0) {
		printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
		return ret;
	}

	ret = gpio_direction_input(ts->intr_gpio);
	if (ret < 0) {
		printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
		return ret;
	}
	if (ts->num_irq){
#if DEBUG_PRINT
        printk(KERN_ERR "melfas_ts_probe: trying to request irq: %s-%d\n", ts->client->name, ts->num_irq);
#endif
        ret = request_irq(ts->num_irq, melfas_ts_irq_handler, IRQF_TRIGGER_FALLING, ts->client->name, ts);
        if (ret > 0){
            printk(KERN_ERR "melfas_ts_probe: Can't allocate irq %d, ret %d\n", ts->client->irq, ret);
            ret = -EBUSY;
            goto err_request_irq;
        }
	}

	disable_irq(ts->num_irq);
	ts->power(0);
	mdelay(10);
	ts->power(1);
	
	melfas_firmware_info(ts,&fw_ver, &hw_ver, &comp_ver);
#if SET_DOWNLOAD_BY_GPIO
	buf[0] = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(ts->client, buf, 1);
	if(ret < 0){
		printk(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);			
	}

	ret = i2c_master_recv(ts->client, buf, 4);
	if(ret < 0){
		printk(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]\n", ret);			
	}
#if defined(CONFIG_MACH_MSM7X27A_M3EU) 	
	mcsdl_download_binary_data(1, 1,hw_ver,0x01);
#elif defined(CONFIG_MACH_MSM7X27A_M3MPCS)
	if (fw_ver !=TS_LATEST_FW_VERSION_A && fw_ver !=TS_LATEST_FW_VERSION_B) {
		mcsdl_download_binary_data(1, 1,hw_ver,comp_ver);
	} 
#else 	
	mcsdl_download_binary_data(1, 1,hw_ver,0x00);
#endif
#endif // SET_DOWNLOAD_BY_GPIO
	
	enable_irq(ts->num_irq);
	
	melfas_firmware_info(ts,&fw_ver, &hw_ver, &comp_ver);
	
	ret = device_create_file(&ts->input_dev->dev, &dev_attr_version);


	for (i = 0; i < MELFAS_MAX_TOUCH ; i++)  /* _SUPPORT_MULTITOUCH_ */
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
//err_detect_failed:
//	printk(KERN_ERR "melfas-ts: err_detect failed\n");
//	kfree(ts);
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

static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
    
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

    	release_all_finger(ts);
	disable_irq(client->irq);
	ts->power(0);
	msleep(10);
	return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

//	melfas_init_panel(ts);
	ts->power(1);
	msleep(10);
	enable_irq(client->irq); // scl wave

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

static const struct i2c_device_id melfas_ts_id[] =
{
    { MELFAS_TS_NAME, 1 },
    { }
};

static struct i2c_driver melfas_ts_driver =
{
    .driver = {
    .name = MELFAS_TS_NAME,
    },
    .id_table = melfas_ts_id,
    .probe = melfas_ts_probe,
    .remove = __devexit_p(melfas_ts_remove),
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

module_init(melfas_ts_init);
module_exit(melfas_ts_exit);
