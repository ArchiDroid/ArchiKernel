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
#include CONFIG_LGE_BOARD_HEADER_FILE

/*LGE_CHANGE_S : seven.kim@lge.com touch firmware manual download using ADB */
#include <linux/miscdevice.h>
#include <linux/wakelock.h>
#include "melfas_mms136_ioctl.h"
/*LGE_CHANGE_E : seven.kim@lge.com touch firmware manual download using ADB */

/*LGE_CHANGE_S : mystery184.kim@lge.com */
#include <linux/timer.h>
#include "melfas_mms136_download_porting.h"
#include <asm/atomic.h>
/*LGE_CHANGE_E : mystery184.kim@lge.com */
#define MIP_ENABLE 1

#define TS_MAX_Z_TOUCH   255
#define TS_MAX_W_TOUCH   255

#define TS_MAX_X_COORD   480 
#define TS_MAX_Y_COORD   800 
// #define FW_VERSION       0x00

#define TS_READ_START_ADDR  0x10
/* LGE_CHANGE_S : 
 * 2011-12-29, yuseok.kim@lge.com 
 * Added define HW_VER and changed FW_VER addr value. 
*/

#define TS_READ_HW_VERSION_ADDR  0xF0
#define TS_READ_FW_VERSION_ADDR  0xF5
/* LGE_CHANGE_E : */
#ifndef MIP_ENABLE
#define TS_READ_REGS_LEN 5
#else
#define TS_READ_REGS_LEN 100
#endif

#define MELFAS_MAX_TOUCH		10

#define I2C_RETRY_CNT			10

#define PRESS_KEY   1
#define RELEASE_KEY 0
/* LGE_CHANGE_S mystery184.kim@lge.com */
#define CANCEL_KEY 0xff
/* LGE_CHANGE_E mystery184.kim@lge.com */

#define DEBUG_PRINT 0
#define	SET_DOWNLOAD_BY_GPIO	0
/* LGE_CHANGE_S : 
 * 2011-12-29, yuseok.kim@lge.com 
 * Added Melfas_FW_Download define value and MELFAS_ESD define value  
*/
#define	Melfas_FW_Download		1
#define MELFAS_ESD	1
static int ESD_check_flag = 0;
/* LGE_CHANGE_E : */
#if SET_DOWNLOAD_BY_GPIO
#include <linux/melfas_download.h>
/* LGE_CHANGE_S : 
 * 2011-12-29, yuseok.kim@lge.com 
 * Defined FW_VERSION as 0x05 for check the early FW_VERSION and current FW-VERSION
*/
#endif
#if Melfas_FW_Download
#define MELFAS_MMS136_TOUCH_ID		121
#define MELFAS_MMS136_TOVIS_PANEL	0x5
#define MELFAS_MMS136_LGIT_PANEL		0x0
unsigned char g_touchLogEnable = 0;
unsigned char g_melfasHWVer = 0;
unsigned char g_melfasFWVer = 0;
extern void SetManual(void);
extern void ResetManual(void);
extern unsigned char 	tovis_fw_ver;
extern unsigned char 	lgit_fw_ver;
extern int mms100_download(unsigned char hw_ver, unsigned char fw_ver,int is_probe);
/* LGE_CHANGE_E : */
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
/* LGE_CHANGE_S mystery184.kim@lge.com fix multi-touch protocol */
	int status;
/* LGE_CHANGE_E mystery184.kim@lge.com fix multi-touch protocol */
};

struct melfas_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct melfas_tsi_platform_data *pdata;
	struct work_struct  work;
	uint32_t flags;
	/*LGE_CHANGE_S : seven.kim@lge.com touch firmware manual download using ADB */
	struct wake_lock wakelock;
	struct delayed_work init_delayed_work;
	/*LGE_CHANGE_E : seven.kim@lge.com touch firmware manual download using ADB */
	int (*power)(int on);
	int (*power_enable)(int en, bool log_en);
	struct early_suspend early_suspend;
};
/*LGE_CHANGE_S : mystery184.kim@lge.com */

static int is_key_pressed = 0;
static int is_touch_pressed = 0;
static int pressed_keycode = 0;
static int esd_delay_ms = 200;
static int esd_flag = 0;
static int esd_count = 0;
static int irq_flag = 0;
static int is_fw_sleep = 0;
static int before_touch_time = 0;
static int current_key_time = 0;
static int key_press_count = 0;

static atomic_t mtx;
/*LGE_CHANGE_E : mystery184.kim@lge.com */

/*LGE_CHANGE_S : seven.kim@lge.com , at%touchfwver */
static int is_downloading = 0;
static int is_touch_suspend = 0;
static struct melfas_ts_data mms136_ts_data;
/*LGE_CHANGE_E : seven.kim@lge.com , at%touchfwver */

/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-12-07] : [U0] at%touchfwver
: To check out the touchscreen version in AT command method */
void mms136_firmware_info(unsigned char *fw_ver, unsigned char *hw_ver, unsigned char *comp_ver)
{
	unsigned char ucTXBuf[1] = {0};
	unsigned char ucRXBuf[1] = {0};
	unsigned char fw_version;
	unsigned char  hw_version;
	int iRet = 0;
	struct melfas_ts_data *dev = NULL;
	dev = &mms136_ts_data;

	ucTXBuf[0] = TS_READ_HW_VERSION_ADDR;
	iRet = i2c_master_send(dev->client, ucTXBuf, 1);
	if (iRet < 0) {
		printk(KERN_ERR "<MELFAS> HW VER : FW download : i2c_master_send [%d]\n", iRet);
	}

	iRet = i2c_master_recv(dev->client, ucRXBuf, 1); 
	if (iRet < 0)	{
		printk(KERN_ERR "<MELFAS> HW VER : FW download : i2c_master_recv [%d]\n", iRet);
	}
	hw_version = ucRXBuf[0];
	*hw_ver = ucRXBuf[0];
	printk(KERN_INFO "<MELFAS> melfas HW version  :: 0x%x\n", ucRXBuf[0]);
	
	ucTXBuf[0] = 0;
	ucRXBuf[0] = 0;
	
	ucTXBuf[0] = TS_READ_FW_VERSION_ADDR;
	iRet = i2c_master_send(dev->client, ucTXBuf, 1);
	if(iRet < 0)
	{
		printk(KERN_ERR "mms136_firmware_info: i2c failed\n");
		return ;	
	}

	iRet = i2c_master_recv(dev->client, ucRXBuf, 1);
	if(iRet < 0)
	{
		printk(KERN_ERR "mms136_firmware_info: i2c failed\n");
		return ;	
	}
	fw_version = ucRXBuf[0];
	*fw_ver = ucRXBuf[0];
	printk(KERN_INFO "<MELFAS> melfas FW version  :: 0x%x\n", ucRXBuf[0]);
}


static ssize_t read_touch_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	int iRet = 0;
	unsigned char hw_ver, fw_ver, comp_ver;

	if(is_touch_suspend)
	{
		iRet = mms136_ts_data.power_enable(1, true); /* TOUCH POWER ON */
		msleep(1000);
	}
	
	printk(KERN_DEBUG "TOUCHSCREEN FW VERSION Starts \n");

	mms136_firmware_info(&fw_ver, &hw_ver, &comp_ver);

	iRet = sprintf(buf, "%02x \n", fw_ver);
	printk(KERN_DEBUG "TOUCHSCREEN FW VERSION : %x \n", fw_ver);

	if(is_touch_suspend)
	{
		iRet = mms136_ts_data.power_enable(0, true); /* TOUCH POWER OFF */
	}
	return iRet;
}

static DEVICE_ATTR(version, S_IRUGO , read_touch_version, NULL);

int mms136_create_file(struct input_dev *pdev)
{
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_version);
	
	if (ret) {
		printk(KERN_DEBUG "LG_FW : dev_attr_version create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_version);
		return ret;
	}
	
	return ret;
}
/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-12-07] 
: To check out the touchscreen version in AT command method */

/*LGE_CHANGE_S : seven.kim@lge.com touch firmware manual download using ADB */
static int misc_opened = 0;

static int mms136_touch_id(void)
{
	return (unsigned char)gpio_get_value(MELFAS_MMS136_TOUCH_ID);
}

static unsigned char mms136_touch_fw_ver(void)
{
	uint8_t 	buf;
	int 		ret;
	struct melfas_ts_data *ts = &mms136_ts_data;
	
	buf = TS_READ_FW_VERSION_ADDR;
	ret = i2c_master_send(ts->client, &buf, 1);
	if (ret < 0) {
		printk(KERN_ERR "<MELFAS> melfas_probe : i2c_master_send [%d]\n", ret);
	}

	buf =0;
	ret = i2c_master_recv(ts->client, &buf, 1);
	if (ret < 0)	{
		printk(KERN_ERR "<MELFAS> melfas_probe : i2c_master_recv [%d]\n", ret);
	}

	printk(KERN_ERR "<MELFAS> mms136_touch_fw_ver : 0x%x \n", buf);
	
	return buf;
}

static long mms136_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  	long lRet;
	int err = 1;
	unsigned char fw_ver = 0, hw_ver = 0, comp_ver = 0;
	int pret = 0;
	struct melfas_ts_data *ts = &mms136_ts_data;

	printk("%s : start !!!\n",__func__);

	switch (_IOC_TYPE(cmd)) 
	{
		case MMS136_TS_IOCTL_MAGIC :
			switch(cmd) 
			{
				case MMS136_TS_IOCTL_FW_VER	:
				{
						mms136_firmware_info(&fw_ver, &hw_ver, &comp_ver);
						printk(KERN_INFO "Firmware ver : [%d],HW ver : [%d] \n", fw_ver, hw_ver);
						break;
				}
				case MMS136_TS_IOCTL_DEBUG : /*seven.kim@lge.com : touch log enable/disable*/
				{
						if (g_touchLogEnable)
							g_touchLogEnable = 0;
						else
							g_touchLogEnable = 1;
						
						err = g_touchLogEnable;
						printk(KERN_INFO "<MELFAS> : Touch Log : %s \n", g_touchLogEnable ? "ENABLE" : "DISABLE");
						break;
				}
			   case MMS136_TS_IOCTL_KERNEL_DOWN	:
			   {
			   	printk("%s : touch firmware download start!!!\n",__func__);
			   	if(is_touch_suspend)
					{
						err = mms136_ts_data.power_enable(1, true); /* TOUCH POWER ON */
						msleep(1000);
					}

					if(irq_flag == 0){
						disable_irq_nosync(ts->client->irq);
						irq_flag = 1;
					}
					
			 		mms100_download(g_melfasHWVer, g_melfasFWVer, 0);

					
					pret = ts->power_enable(0, true);
					msleep(20);
					pret = ts->power_enable(1, true);
					
					if (irq_flag == 1) {
						printk("[MELFAS] enable_irq\n");
						enable_irq(ts->client->irq);
						irq_flag = 0;
					}

					break;					
				}
				case MMS136_TS_IOCTL_KERNEL_DOWN_MANUAL	:
			   {
			   	printk("%s : manual touch firmware download start!!!\n",__func__);
			   	if(is_touch_suspend)
					{
						err = mms136_ts_data.power_enable(1, true); /* TOUCH POWER ON */
						msleep(1000);
					}	
							   	
			   	SetManual();

					
					if (irq_flag == 0) {
						printk("[MELFAS] disable_irq_nosync\n");
						disable_irq_nosync(ts->client->irq);
						irq_flag = 1;
					}	
			 		mms100_download(g_melfasHWVer, g_melfasFWVer, 0);
			 		ResetManual();

					
					pret = ts->power_enable(0, true);
					msleep(20);
					pret = ts->power_enable(1, true);


					if (irq_flag == 1) {
						printk("[MELFAS] enable_irq\n");
						enable_irq(ts->client->irq);
						irq_flag = 0;
					}
					
					break;					
				}
			}
			break;
			
		default:
			printk(KERN_ERR "mms136_ts_ioctl: unknown ioctl\n");
			err = -EINVAL;
			break;
	}
	lRet = (long)err;
 
	return lRet;
}

static int mms136_open(struct inode *inode, struct file *file) 
{
	struct melfas_ts_data *ts = &mms136_ts_data;

	if (ts == (void *)NULL)
		return -EIO;

	if (misc_opened)
		return -EBUSY;

	disable_irq(ts->client->irq);

	misc_opened = 1;

	file->private_data = ts;
	is_downloading = 1;
	wake_lock(&ts->wakelock);

	return 0;
}

static int mms136_release(struct inode *inode, struct file *file) 
{
	struct melfas_ts_data *ts = &mms136_ts_data;

	if (ts == (void *)NULL)
		return -EIO;	

	enable_irq(ts->client->irq);

	misc_opened = 0;
	is_downloading = 0;
	wake_unlock(&ts->wakelock);

	return 0;
}


static struct file_operations mms136_ts_ioctl_fops = {
	.owner 						= THIS_MODULE, 
	.unlocked_ioctl 			= mms136_ioctl, 
	.open 						= mms136_open, 
	.release						= mms136_release,
};

static struct miscdevice mms136_ts_misc_dev = {
	.minor 						= MISC_DYNAMIC_MINOR,
	.name 						= "mms136",
	.fops 						= &mms136_ts_ioctl_fops,
};
/*LGE_CHANGE_E : seven.kim@lge.com touch firmware manual download using ADB */


/* LGE_CHANGE_S : 
 * 2011-12-29, yuseok.kim@lge.com 
 * Added release_all_fingers
*/
static void release_all_fingers(struct melfas_ts_data *ts);
/* LGE_CHANGE_E : */
#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif

static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];

int (*g_power_enable) (int en, bool log_en);

/* LGE_CHANGE_S : 
 * 2011-12-29, yuseok.kim@lge.com 
 * Added ESD check flag and ESD check Function
*/
#ifdef MELFAS_ESD
static void melfas_data_clear(void) 
{	
	int i;	
	for(i=0; i<MELFAS_MAX_TOUCH; i++)	 
	{	  
		if(g_Mtouch_info[i].status != -1){	   	  
			g_Mtouch_info[i].status = -1;	   	
		}	
	}
}
static void melfas_ts_restart_ex(struct work_struct *work)
{
	struct melfas_ts_data *ts = &mms136_ts_data;
	int ret;
	
	melfas_data_clear();
	printk(KERN_ERR "<MELFAS> melfas_ts_restart_ex start \n");

	ret = ts->power_enable(0, true);
	msleep(esd_delay_ms + esd_count * 200);

	ret = ts->power_enable(1, true);
	msleep(100);

	ESD_check_flag = 0;
		
	esd_count++;
	if(esd_count >= 10) esd_count = 0;
	printk(KERN_ERR "<MELFAS> melfas_ts_restart_ex end \n");
	atomic_set(&mtx ,0);
}
/* static void melfas_ts_restart(struct work_struct *work)
{
	struct melfas_ts_data *ts = &mms136_ts_data;
	unsigned long flags;
	int ret;
	
	if (ESD_check_flag == 1)
	{
		printk(KERN_ERR "<MELFAS> melfas_ts_restart start \n");
		local_irq_save(flags);
		
		ret = ts->power_enable(0, true);
		msleep(100);
		ret = ts->power_enable(1, true);
		msleep(100);
		
		ESD_check_flag = 0;
		local_irq_restore(flags);
		
		printk(KERN_ERR "<MELFAS> melfas_ts_restart end \n");
	}

	schedule_delayed_work(&ts->init_delayed_work, msecs_to_jiffies(HZ * 40)); 
} */
#endif
/* LGE_CHANGE_E : */

/*LGE_CHANGE_S mystery184.kim@lge.com timer handler */
static void send_key_event_forced(int keyID, unsigned int eventType)
{
	struct melfas_ts_data *ts = &mms136_ts_data;
	if(g_touchLogEnable)
		printk(KERN_ERR "<MELFAS> send_key_event_forced : %d \n",eventType);
	
	if(keyID == 0x01)
		input_report_key(ts->input_dev, KEY_BACK, eventType);
	else if(keyID == 0x02)
		input_report_key(ts->input_dev, KEY_MENU, eventType);
	else if(keyID == 0x03)
		input_report_key(ts->input_dev, KEY_HOME, eventType);
	else if(keyID == 0x04)
		input_report_key(ts->input_dev, KEY_SEARCH, eventType);


		pressed_keycode = 0;
}

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
/* LGE_CHANGE_S mystery184.kim@lge.com fix multi-touch protocol */
	int press_count = 0;
	int is_touch_mix = 0;
/* LGE_CHANGE_E mystery184.kim@lge.com fix multi-touch protocol */


#if DEBUG_PRINT
	printk(KERN_ERR "<MELFAS> melfas_ts_work_func\n");

	if (ts == NULL)
			printk(KERN_ERR "<MELFAS> melfas_ts_work_func : TS NULL\n");
#endif

#define MIP_INPUT_EVENT_PACKET_SIZE	0x0F
#define MIP_INPUT_EVENT_INFORMATION	0x10

	buf[0] = MIP_INPUT_EVENT_PACKET_SIZE;
	ret = i2c_master_send(ts->client, buf, 1);
	if ( ret < 0 )
	{
		printk(KERN_ERR "<MELFAS> melfas ESD recovery enable: i2c_master_send(1)\n");
		esd_flag = -1;
	}
	ret = i2c_master_recv(ts->client, &read_num, 1);

	if ( ret < 0 )
	{
		printk(KERN_ERR "<MELFAS> melfas ESD recovery enable: i2c_master_recv(1)\n");
		esd_flag = -2;
		if(atomic_read(&mtx) == 0){
			atomic_set(&mtx, 1);
			schedule_delayed_work(&ts->init_delayed_work, 0);			
		}
		enable_irq(ts->client->irq);
		return;
	}
	if (read_num == 0) {
		//printk(KERN_ERR "<MELFAS> read number 0 error!!!!\n");
		enable_irq(ts->client->irq);
		return;
	}

	buf[0] = MIP_INPUT_EVENT_INFORMATION;
	ret = i2c_master_send(ts->client, buf, 1);
	if ( ret < 0 )
	{
		printk(KERN_ERR "<MELFAS> melfas ESD recovery enable: i2c_master_send(2)\n");
		esd_flag = -3;
	}
	buf[0] = 0;
	ret = i2c_master_recv(ts->client, &buf[0], read_num);
	if ( ret < 0 )
	{
		printk(KERN_ERR "<MELFAS> melfas ESD recovery enable: i2c_master_recv(2)\n");
		esd_flag = -4;
	}

#ifdef MELFAS_ESD
	if((buf[0] & 0x0f) == 0x0f)
	{
		printk(KERN_ERR "<MELFAS> MIP_INPUT_EVENT_INFORMATION : %x \n", buf[0]);
		printk(KERN_ERR "<MELFAS> melfas ESD recovery enable: MIP_INPUT_EVENT_INFORMATION\n");
		ESD_check_flag = 1;
		printk(KERN_ERR "<MELFAS> melfas ESD flag: %d\n",esd_flag);
		esd_flag = 0;
		if(atomic_read(&mtx) == 0){
			atomic_set(&mtx, 1);
			schedule_delayed_work(&ts->init_delayed_work, 0);			
		}
		enable_irq(ts->client->irq);
		return;
	}
#endif
	if(ESD_check_flag == 0)
			esd_count=0;

	for (i = 0; i < read_num; i = i + 6) {
		if (ret < 0) {
			printk(KERN_ERR "<MELFAS> melfas_ts_work_func: i2c failed\n");
			enable_irq(ts->client->irq);
			return ;
		} else {
			touchType  =  ((buf[i] & 0x60) >> 5);
			touchState = ((buf[i] & 0x80) == 0x80);
			reportID = (buf[i] & 0x0F);
			posX = (uint16_t) (buf[i + 1] & 0x0F) << 8 | buf[i + 2];
			posY = (uint16_t) (buf[i + 1] & 0xF0) << 4 | buf[i + 3];
			width = buf[i + 4];
				keyID = reportID;

			touchID = reportID-1;

			if (touchID > MELFAS_MAX_TOUCH-1) {
			    enable_irq(ts->client->irq);
			    return;
			}

			if (touchType == TOUCH_SCREEN) {
				g_Mtouch_info[touchID].posX = posX;
				g_Mtouch_info[touchID].posY = posY;
				g_Mtouch_info[touchID].width = width;
/* LGE_CHANGE_S mystery184.kim@lge.com fix multi-touch protocol */
				g_Mtouch_info[touchID].status = touchState;
/* LGE_CHANGE_E mystery184.kim@lge.com fix multi-touch protocol */

				if (touchState)
					g_Mtouch_info[touchID].strength = strength;
				else {
					g_Mtouch_info[touchID].strength = 0;
					tmp_flag[touchID] = 1;
				}

				if(is_key_pressed == PRESS_KEY){					
					send_key_event_forced(pressed_keycode, CANCEL_KEY);
					input_sync(ts->input_dev);
					if(g_touchLogEnable)
						printk(KERN_ERR "<MELFAS> melfas_ts_work_func: SEND CANCEL EVENT \n");
					is_key_pressed = CANCEL_KEY;
				}

				is_touch_mix = 1;
				

			} else if (touchType == TOUCH_KEY) {
				/* LGE_CHANGE_S :
				 * 2012-03-29,mystery184.kim@lge.com
				 * For Debugging when discussing touch no action
				 */
				key_press_count++;
				if(key_press_count >= 5)
				{
					printk(KERN_ERR "<MELFAS> Key Event Dispatch!\n");
					key_press_count = 0;
				}
				/* LGE_CHANGE_E mystery184.kim@lge.com */

				/* LGE_CHANGE_S :
				 * 2012-03-29,mystery184.kim@lge.com
				 * Add key event ignore condition(100ms) after touch event drivened
				 */				
				current_key_time = jiffies_to_msecs(jiffies);

				if(before_touch_time > 0)
				{
					if(g_touchLogEnable)
							printk(KERN_ERR "<MELFAS> touch time : %d, key time : %d\n", before_touch_time, current_key_time);
					if(current_key_time - before_touch_time > 100)
					{
						is_touch_pressed = 0;
						if(g_touchLogEnable)
							printk(KERN_ERR "<MELFAS> ENABLE KEY EVENT OVER 200MS\n");
					}
					else{
						if(g_touchLogEnable)
							printk(KERN_ERR "<MELFAS> IGNORE KEY EVENT(LESS 200MS) \n");
						continue;
					}
				}
				before_touch_time = 0;
				current_key_time = 0;
				/* LGE_CHANGE_E mystery184.kim@lge.com */

			
				// Ignore Key event during touch event actioned
				if(is_touch_mix || is_touch_pressed){
					if(g_touchLogEnable)
						printk(KERN_ERR "<MELFAS> IGNORE KEY EVENT \n");
					continue;
				}
				if (keyID == 0x1)
					input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);
				if (keyID == 0x2) /* LGE_CHANGE_S [yoonsoo.kim@lge.com] 20111109 : U0 Rev.B Touch Back Key Fixed */
					input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);
				if (keyID == 0x3)
					input_report_key(ts->input_dev, KEY_HOME, touchState ? PRESS_KEY : RELEASE_KEY);
				if (keyID == 0x4)/* LGE_CHANGE_E  [yoonsoo.kim@lge.com]  20111109  : U0 Rev.B Touch Back Key Fixed */
					input_report_key(ts->input_dev, KEY_SEARCH, touchState ? PRESS_KEY : RELEASE_KEY);
				pressed_keycode = keyID;
				if(touchState) is_key_pressed = PRESS_KEY;
				else is_key_pressed = RELEASE_KEY;
				
				if(g_touchLogEnable)
					printk(KERN_ERR "<MELFAS> melfas_ts_work_func: keyID : %d, touchState: %s\n", keyID, touchState? "KEY PRESS" : "KEY RELEASE");
				
				//break;
			}

		}
	}

	press_count = 0;
	//if (touchType == TOUCH_SCREEN) {
	if(is_touch_mix){
		for (i = 0; i < MELFAS_MAX_TOUCH; i++) {
			if (g_Mtouch_info[i].strength == -1)
				continue;
/* LGE_CHANGE_S mystery184.kim@lge.com fix multi-touch protocol */
			if (g_Mtouch_info[i].status == 0){
				is_touch_pressed = 0;
				g_Mtouch_info[i].status = -1;
				continue;
			}

			if(g_Mtouch_info[i].status == 1){
/* LGE_CHANGE_E mystery184.kim@lge.com fix multi-touch protocol */

			input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].width);
/* LGE_CHANGE_S mystery184.kim@lge.com fix multi-touch protocol */
			input_report_abs(ts->input_dev, ABS_MT_PRESSURE, g_Mtouch_info[i].strength);
			input_mt_sync(ts->input_dev);
			is_touch_pressed = 1;
				press_count++;
			}
/* LGE_CHANGE_E mystery184.kim@lge.com fix multi-touch protocol */
			
			if(g_touchLogEnable)
			{
				printk(KERN_ERR "<MELFAS> Touch ID: %d, State : %d, x: %d, y: %d, z: %d w: %d\n",
						   i, g_Mtouch_info[i].status, g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength, g_Mtouch_info[i].width);
			}
				
#if DEBUG_PRINT			
			{
				if ((g_Mtouch_info[i].status == 1 && tmp_flag[touchID] == 1) || (g_Mtouch_info[i].status == 0 && tmp_flag[touchID] == 1)) {
					printk(KERN_ERR "<MELFAS> Touch ID: %d, State : %d, x: %d, y: %d, z: %d w: %d\n",
						   i, touchState, g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength, g_Mtouch_info[i].width);
					if (g_Mtouch_info[i].status == 1)
						tmp_flag[touchID] = 0;
				}
			}
#endif			
			if (g_Mtouch_info[i].strength == 0)
				g_Mtouch_info[i].strength = -1;

		}
/* LGE_CHANGE_S mystery184.kim@lge.com fix multi-touch protocol */
		if(press_count == 0) 
			input_mt_sync(ts->input_dev);

/* LGE_CHANGE_E mystery184.kim@lge.com fix multi-touch protocol */

		/* LGE_CHANGE_S :
		 * 2012-03-29,mystery184.kim@lge.com
		 * Add key event ignore condition(100ms) after touch event drivened
		 */	
		before_touch_time = jiffies_to_msecs(jiffies);			
		/* LGE_CHANGE_E mystery184.kim@lge.com */
		
	}
	is_touch_mix = 0;

	input_sync(ts->input_dev);

	enable_irq(ts->client->irq);
}

static irqreturn_t melfas_ts_irq_handler(int irq, void *handle)
{
	struct melfas_ts_data *ts = (struct melfas_ts_data *)handle;
	
	if(g_touchLogEnable)
		printk(KERN_ERR "<MELFAS> melfas_ts_irq_handler\n");

	irq_flag = 1;
	if(likely(!is_downloading)){

	disable_irq_nosync(ts->client->irq);
	schedule_work(&ts->work);
	}
	irq_flag = 0;

	return IRQ_HANDLED;
}

static int melfas_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct melfas_ts_data *ts;
	int ret = 0, i;

	uint8_t buf;

#if DEBUG_PRINT
	printk(KERN_ERR "<MELFAS> melfas_ts_probe Start!!!\n");
#endif

	memset(&tmp_flag[0], 0x01, sizeof(tmp_flag));


	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "<MELFAS> melfas_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	/*LGE_CHANGE_S : seven.kim@lge.com , at%touchfwver */
	#if 0
	ts = kmalloc(sizeof(struct melfas_ts_data), GFP_KERNEL);
	#else
	ts = &mms136_ts_data;
	#endif
	/*LGE_CHANGE_E : seven.kim@lge.com , at%touchfwver */
	
	if (ts == NULL) {
		printk(KERN_ERR "<MELFAS> melfas_ts_probe: failed to create a state of melfas-ts\n");
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
	ts->pdata = client->dev.platform_data;
	/*LGE_CHANGE_S : mystery184.kim@lge.com
	  * Gpio setting before f/w downloading check
	  */
	gpio_request(GPIO_TOUCH_SDA, "Melfas_I2C_SDA");
	gpio_request(GPIO_TOUCH_SCL, "Melfas_I2C_SCL");
	/*LGE_CHANGE_E : mystery184.kim@lge.com */ 
	if (ts->pdata->power_enable)
		ts->power_enable = ts->pdata->power_enable;

	ret = ts->pdata->power_enable(1, true);
	mdelay(500);

	g_power_enable = ts->pdata->power_enable;

	INIT_WORK(&ts->work, melfas_ts_work_func);
	/*LGE_START_S : seven.kim@lge.com work around for ESD detection function*/
	#ifdef MELFAS_ESD
	INIT_DELAYED_WORK(&ts->init_delayed_work, melfas_ts_restart_ex);
	#endif
	/*LGE_START_E : seven.kim@lge.com work around for ESD detection function*/

	ts->client = client;
	i2c_set_clientdata(client, ts);

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_master_send(ts->client, &buf, 1);
		if (ret >= 0) {
			printk(KERN_ERR "<MELFAS> melfas_ts_probe: i2c_master_send() ok [%d]\n", ret);
			break;
		} else{
			printk(KERN_ERR "<MELFAS> melfas_ts_probe: i2c_master_send() failed[%d]\n", ret);
			/*LGE_CHANGE_S : mystery184.kim@lge.com
		            * Check No LCD and No touch status and FW abnormal case
		            */
			if(i == I2C_RETRY_CNT - 1){		
				g_melfasHWVer = mms136_touch_id();
				/* generate crash */
				ret = mms100_download(g_melfasHWVer, 0, 2);
				
				if(ret == 0){
					ret = mms100_download(g_melfasHWVer, 0, 1);
					printk(KERN_ERR "<MELFAS> melfas_ts_probe: i2c_master_send() recovery-download[%d]\n", ret);
				}
				else{	
					printk(KERN_ERR "<MELFAS> melfas_ts_probe: i2c_master_send() ENODEV[%d]\n", ret);
					return -ENODEV;				
				}
			}			
			/*LGE_CHANGE_E : mystery184.kim@lge.com */     
		}
	}
	/*LGE_CHANGE_S : seven.kim@lge.com
            * Check No LCD and No touch status
            */
           //if (ret < 0 && i >= 10)
           //{
                   /*  printk("%s : No Touch !!!\n",__func__); */
             //     return -ENODEV;
           //}
            /*LGE_CHANGE_S : seven.kim@lge.com */           

/* LGE_CHANGE_S : 
 * 2011-12-29, yuseok.kim@lge.com, 
 * Read TS_READ_FW_VERSION_ADDR buffer address
*/
	buf = TS_READ_FW_VERSION_ADDR;
/* LGE_CHANGE_E : */
	ret = i2c_master_send(ts->client, &buf, 1);
	buf = 0;
	ret = i2c_master_recv(ts->client, &buf, 1); 

	printk(KERN_INFO "<MELFAS> melfas FW version :: 0x%2X\n", buf);

#if DEBUG_PRINT
	printk(KERN_ERR "<MELFAS> melfas_ts_probe: i2c_master_send() [%d], Add[%d]\n", ret, ts->client->addr);
#endif



#if Melfas_FW_Download
	g_melfasHWVer = mms136_touch_id();
	printk(KERN_INFO "<MELFAS> melfas HW version  :: %s\n", g_melfasHWVer ? "LGIT" : "TOVIS");
	
	g_melfasFWVer = mms136_touch_fw_ver();
		printk(KERN_ERR "<MELFAS> melfas_probe : buf %x \n", g_melfasFWVer);

	if (g_melfasHWVer) /* LGIT PANEL */
	{
		if(g_melfasFWVer < lgit_fw_ver)
		{
			/*download tovis fw image*/
			printk(KERN_ERR "<MELFAS> melfas_probe : LGIT FW Download START\n");
			is_downloading = 1;
			ret = mms100_download(g_melfasHWVer, g_melfasFWVer, 1);
		}
		else{
			/* LGE_CHANGE_S :
			 * 2012-03-29 mystery184.kim@lge.com 
			 * check abnormal touch fw ver 
			 */
			if(g_melfasFWVer > lgit_fw_ver){
				if(g_melfasFWVer >= 80 && lgit_fw_ver < 80){
					/*download tovis fw image*/
					printk(KERN_ERR "<MELFAS> melfas_probe : LGIT FW Download START(Test FW)\n");
					
					is_downloading = 1;
					SetManual();
					ret = mms100_download(g_melfasHWVer, g_melfasFWVer, 1);
					ResetManual();
				}
				else
					printk(KERN_ERR "<MELFAS> melfas_probe : No Need LGIT FW Download(Test FW) \n");
			}
			else			
			/* LGE_CHANGE_E mystery184.kim@lge.com */
				printk(KERN_ERR "<MELFAS> melfas_probe : No Need LGIT FW Download \n");
		}
	}
	else /* TOVIS PANEL */
	{
		if(g_melfasFWVer < tovis_fw_ver)
		{
			/*download tovis fw image*/
			printk(KERN_ERR "<MELFAS> melfas_probe : TOVIS FW Download START\n");
			is_downloading = 1;
			ret = mms100_download(g_melfasHWVer, g_melfasFWVer, 1);
		}
		else{
			/* LGE_CHANGE_S :
			 * 2012-03-29 mystery184.kim@lge.com 
			 * check abnormal touch fw ver 
			 */
			if(g_melfasFWVer > tovis_fw_ver){
				if(g_melfasFWVer >= 80 && tovis_fw_ver < 80){
					/*download tovis fw image*/
					printk(KERN_ERR "<MELFAS> melfas_probe : TOVIS FW Download START(Test FW)\n");
					is_downloading = 1;
					SetManual();
					ret = mms100_download(g_melfasHWVer, g_melfasFWVer, 1);
					ResetManual();
				}
				else
					printk(KERN_ERR "<MELFAS> melfas_probe : No Need TOVIS FW Download(Test FW) \n");
			}
			else			
			/* LGE_CHANGE_E mystery184.kim@lge.com  */
			printk(KERN_ERR "<MELFAS> melfas_probe : No Need TOVIS FW Download \n");
		}
	} 

	if(is_downloading)
	{
		ret = ts->pdata->power_enable(0, true);
		msleep(2000); //2s
		ret = ts->pdata->power_enable(1, true);
		is_downloading = 0;
	}
#endif

	ts->input_dev = input_allocate_device();
	if (!ts->input_dev) {
		printk(KERN_ERR "<MELFAS> melfas_ts_probe: Not enough memory\n");
		ret = -ENOMEM;
		goto err_input_dev_alloc_failed;
	}

	ts->input_dev->name = "melfas-ts" ;

	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	/*set_bit(BTN_TOUCH, ts->input_dev->keybit); */
	/* LGE_CHANGE_S : add touch device propbit
	 * 2012-01-17, mystery184.kim@lge.com
	 * not initialize propbit 
	 */
	set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
	/* LGE_CHANGE_E : add touch device propbit */ 
	ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);


	ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
	ts->input_dev->keybit[BIT_WORD(KEY_HOME)] |= BIT_MASK(KEY_HOME);
	ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);
	ts->input_dev->keybit[BIT_WORD(KEY_SEARCH)] |= BIT_MASK(KEY_SEARCH);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, TS_MAX_X_COORD, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, TS_MAX_Y_COORD, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, MELFAS_MAX_TOUCH-1, 0, 0);
/* LGE_CHANGE_S mystery184.kim@lge.com fix multi-touch protocol */
	input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0, 255, 0, 0);
/* LGE_CHANGE_E mystery184.kim@lge.com fix multi-touch protocol */
	/* input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0); */

	ret = input_register_device(ts->input_dev);
	if (ret) {
		printk(KERN_ERR "<MELFAS> melfas_ts_probe: Failed to register device\n");
		ret = -ENOMEM;
		goto err_input_register_device_failed;
	}

	ret = gpio_request( (client->irq) - NR_MSM_IRQS, "melfas-ts");
	if (ret < 0) {
		printk(KERN_ERR "<MELFAS> %s: gpio input direction fail\n", __FUNCTION__);
		return ret;
	}

	ret = gpio_direction_input( (client->irq) - NR_MSM_IRQS);
	if (ret < 0) {
		printk(KERN_ERR "<MELFAS> %s: gpio input direction fail\n", __FUNCTION__);
		return ret;
	}
	ret = request_threaded_irq(client->irq, NULL, melfas_ts_irq_handler,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT, ts->client->name, ts);

	if (ret < 0) {
		printk(KERN_ERR "<MELFAS> %s: request_irq failed\n", __FUNCTION__);
		ret = -EBUSY;
		goto err_request_irq;
	}

	/*LGE_CHANGE_S : seven.kim@lge.com touch firmware manual download using ADB */	
	wake_lock_init(&ts->wakelock, WAKE_LOCK_SUSPEND, "mms136");
	
	ret = misc_register(&mms136_ts_misc_dev);
	if (ret < 0) {
		printk(KERN_ERR "mcs8000_probe_ts: misc register failed\n");
		return ret;
	}
	/*LGE_CHANGE_S : seven.kim@lge.com touch firmware manual download using ADB */	
		
	for (i = 0; i < MELFAS_MAX_TOUCH ; i++)
		g_Mtouch_info[i].strength = -1;

	/*LGE_CHANGE_S : seven.kim@lge.com at%touchfwver */
	mms136_create_file(ts->input_dev);
	/*LGE_CHANGE_E : seven.kim@lge.com at%touchfwver */
	
	/*LGE_CHANGE_S : seven.kim@lge.com ESD Detection and recovery */
	/* #ifdef MELFAS_ESD
	schedule_delayed_work(&ts->init_delayed_work, msecs_to_jiffies(HZ * 40)); 8
	#endif */
	/*LGE_CHANGE_E : seven.kim@lge.com ESD Detection and recovery */

	atomic_set(&mtx, 0);
	
#if DEBUG_PRINT
	printk(KERN_ERR "<MELFAS> melfas_ts_probe: succeed to register input device\n");
#endif

#if CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = melfas_ts_early_suspend;
	ts->early_suspend.resume = melfas_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

#if DEBUG_PRINT
	printk(KERN_INFO "<MELFAS> melfas_ts_probe: Start touchscreen. name: %s, irq: %d\n", ts->client->name, ts->client->irq);
#endif
	return 0;

err_request_irq:
	printk(KERN_ERR "<MELFAS> melfas-ts: err_request_irq failed\n");
	free_irq(client->irq, ts);
err_input_register_device_failed:
	printk(KERN_ERR "<MELFAS> melfas-ts: err_input_register_device failed\n");
	/*LGE_CHANGE_S : mystery184.kim@lge.com
	  * Gpio setting before f/w downloading check
	  */
	gpio_free(GPIO_TOUCH_SDA);
	gpio_free(GPIO_TOUCH_SCL);
	gpio_free(GPIO_TOUCH_INT);	
	/*LGE_CHANGE_E : mystery184.kim@lge.com  */
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
	printk(KERN_ERR "<MELFAS> melfas-ts: err_input_dev_alloc failed\n");
err_alloc_data_failed:
	printk(KERN_ERR "<MELFAS> melfas-ts: err_alloc_data failed_\n");
err_check_functionality_failed:
	printk(KERN_ERR "<MELFAS> melfas-ts: err_check_functionality failed_\n");

	return ret;
}

static int melfas_ts_remove(struct i2c_client *client)
{
	int ret;
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

	unregister_early_suspend(&ts->early_suspend);
	free_irq(client->irq, ts);

	flush_work_sync(&ts->work);
	ret = ts->power_enable(0, true);
	/*LGE_CHANGE_S : mystery184.kim@lge.com
	  * Gpio setting before f/w downloading check
	  */
	gpio_free(GPIO_TOUCH_SDA);
	gpio_free(GPIO_TOUCH_SCL);
	gpio_free(GPIO_TOUCH_INT);	
	/*LGE_CHANGE_E : mystery184.kim@lge.com  */
	input_unregister_device(ts->input_dev);

/*	kfree(ts);*/
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
	/*	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength); 
		input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width); 
		input_report_key(ts->input_dev, BTN_TOUCH, 0);  */
/* LGE_CHANGE_S mystery184.kim@lge.com fix multi-touch protocol */
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].width);
		input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 0);
/* LGE_CHANGE_E mystery184.kim@lge.com fix multi-touch protocol */
		input_mt_sync(ts->input_dev);

		g_Mtouch_info[i].posX = 0;
		g_Mtouch_info[i].posY = 0;

		if (0 == g_Mtouch_info[i].strength)
			g_Mtouch_info[i].strength = -1;
	}
/* LGE_CHANGE_S mystery184.kim@lge.com add release all finger */
	input_sync(ts->input_dev);
	/* LGE_CHANGE_E mystery184.kim@lge.com add release all finger */
}


static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

	if (is_downloading == 0) {
		printk(KERN_ERR "<MELFAS> melfas_ts_suspend start \n");
		if(work_pending(&ts->work))
		{
			//schedule_work(&ts->work);
			flush_work_sync(&ts->work);
			msleep(10);
			printk(KERN_ERR "<MELFAS> %s : schedule_work end\n",__func__);
		}
		release_all_fingers(ts);
		disable_irq(client->irq);
	
		//ret = cancel_work_sync(&ts->work);
		while(atomic_read(&mtx) == 1){
			msleep(100);
		}
		
		#ifdef MELFAS_ESD
		ret = flush_delayed_work_sync(&ts->init_delayed_work); 
		#endif
		
		/*if (ret) //if work was pending disable-count is now 2
			enable_irq(client->irq);
	
		ret = i2c_smbus_write_byte_data(client, 0x01, 0x00); // deep sleep*/
	
		ret = ts->power_enable(0, true);
	
		if (ret < 0)
			printk(KERN_ERR "<MELFAS> melfas_ts_suspend: i2c_smbus_write_byte_data failed\n");
	}
	/*LGE_CHANGE_S : seven.kim@lge.com at%touchfwver */
	is_touch_suspend = 1;
	/*LGE_CHANGE_E : seven.kim@lge.com at%touchfwver */
	
	return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);
	int ret;
	if (is_downloading == 0) {
		printk(KERN_ERR "<MELFAS> melfas_ts_resume start \n");
	
		ret = ts->power_enable(1, true);
		msleep(50);
/* LGE_CHANGE_S mystery184.kim@lge.com add release all finger */
		release_all_fingers(ts);
		/* LGE_CHANGE_E mystery184.kim@lge.com add release all finger */
		    enable_irq(client->irq);
	}
	
	/*LGE_CHANGE_S : seven.kim@lge.com at%touchfwver */
	is_touch_suspend = 0;
	/*LGE_CHANGE_E : seven.kim@lge.com at%touchfwver */

	/* #ifdef MELFAS_ESD
	schedule_delayed_work(&ts->init_delayed_work, msecs_to_jiffies(HZ * 40)); 
	#endif */

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h)
{
	struct melfas_ts_data *ts;
	ts = container_of(h, struct melfas_ts_data, early_suspend);
	if(is_downloading == 0){
		melfas_ts_suspend(ts->client, PMSG_SUSPEND);
		is_fw_sleep = 0;
	}
	else
		is_fw_sleep = 1;
}

static void melfas_ts_late_resume(struct early_suspend *h)
{
	struct melfas_ts_data *ts;
	ts = container_of(h, struct melfas_ts_data, early_suspend);
	if(is_downloading == 0 && is_fw_sleep == 0)
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
	int i2c_add_driver_result;
	printk("<MELFAS> melfas_ts_init start \n");
	i2c_add_driver_result = i2c_add_driver(&melfas_ts_driver);

	printk("<MELFAS> melfas_ts_init i2c_add_driver_result = %d \n", i2c_add_driver_result);

	return i2c_add_driver_result;
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
