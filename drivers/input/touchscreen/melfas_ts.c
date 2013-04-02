/*
 * MELFAS mcs8000 touchscreen driver
 *
 * Copyright (C) 2011 LGE, Inc.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <linux/timer.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

/* To enable FW Upgrade */
#include "mcs8000_download.h"

#include <linux/i2c-gpio.h>
#include CONFIG_LGE_BOARD_HEADER_FILE


#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
static struct early_suspend ts_early_suspend;
static void mcs8000_early_suspend(struct early_suspend *h);
static void mcs8000_late_resume(struct early_suspend *h);
#endif


//static int mcs8000_ts_off(void);
int mcs8000_ts_on(void);
static void mcs8000_Data_Clear(void);
static void ResetTS(void);
static void Release_All_Fingers(void);

#define TS_POLLING_TIME 0 /* msec */


#define DEBUG_TS 0 /* enable or disable debug message */

#if DEBUG_TS
#define DMSG(fmt, args...) printk(KERN_DEBUG fmt, ##args)
#else
#define DMSG(fmt, args...) do {} while (0)
#endif

#define ON	1
#define OFF	0
#define PRESSED		1
#define RELEASED	0

/* melfas data */
#define TS_MAX_Z_TOUCH											255
#define TS_MAX_W_TOUCH											30
#define MTSI_VERSION												0x07    /* 0x05 */
#define TS_MAX_X_COORD											320
#define TS_MAX_Y_COORD											480
#define FW_VERSION													0x00	

#define TS_READ_START_ADDR 								0x0F
#define TS_READ_START_ADDR2 							0x10

#if 1	//  woden@lge.com [2013-02-26] => New TS Firmware was applied.
#define TS_LATEST_FW_VERSION_EU_SUN	0x16
#define TS_LATEST_FW_VERSION_EU_INO	0x20
#else
#define TS_LATEST_FW_VERSION_EU_SUN	0x15
#define TS_LATEST_FW_VERSION_EU_INO	0x18
#endif

#define TS_READ_REGS_LEN 									66
#define MELFAS_MAX_TOUCH									11

#define I2C_RETRY_CNT											10
#define PRESS_KEY													1
#define RELEASE_KEY												0
#define DEBUG_PRINT 												0



#define	SET_DOWNLOAD_BY_GPIO							1
#define GPIO_TOUCH_ID 121

#if defined(CONFIG_MACH_MSM7X25A_V3_DS) || defined(CONFIG_MACH_MSM7X25A_V1)
#define KEY_SIM_SWITCH 223
#endif

int power_flag=0;
static int irq_flag;

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


struct mcs8000_ts_device {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct delayed_work work; 
	//struct work_struct  work;
	int num_irq;
	int intr_gpio;
	int scl_gpio;
	int sda_gpio;
	bool pendown;
	int (*power)(unsigned char onoff);
	struct workqueue_struct *ts_wq;

	/*20110607 seven.kim@lge.com for touch frimware download [START] */
	//struct wake_lock wakelock;
	int irq_sync;
	int fw_version;
	int hw_version;
	int status;
	int tsp_type;
	/*20110607 seven.kim@lge.com for touch frimware download [END] */
};

static struct input_dev *mcs8000_ts_input = NULL;

struct mcs8000_ts_device mcs8000_ts_dev;

int Is_Release_Error[MELFAS_MAX_TOUCH]={0}; /* for touch stable */

static unsigned char ucSensedInfo = 0;

#define READ_NUM 8 /* now, just using two finger data */

static unsigned char g_touchLogEnable = 0;

unsigned char ex_fw_ver;
unsigned char ex_hw_ver;
unsigned char ex_touch_id;

void mcs8000_firmware_info(unsigned char* fw_ver, unsigned char* hw_ver, unsigned char *comp_ver);
	
//static int misc_opened = 0;
//static unsigned int mcs8000_debug_mask = MCS8000_DM_TRACE_NO;

//static struct mcs8000_ts_device *mcs8000_ext_ts = (void *)NULL; 


void Send_Touch(unsigned int x, unsigned int y)
{
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs8000_ts_dev.input_dev);
	input_sync(mcs8000_ts_dev.input_dev);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs8000_ts_dev.input_dev);
	input_sync(mcs8000_ts_dev.input_dev);
}
EXPORT_SYMBOL(Send_Touch);

/* #define to_delayed_work(_work) container_of(_work, struct delayed_work, work ) */


static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];
/*
static int melfas_init_panel(struct mcs8000_ts_device *ts)
{
	int ret ;
	int buf = 0;
	ret = i2c_master_send(ts->client, &buf, 1);
	
	ret = i2c_master_send(ts->client, &buf, 1);

	if (ret <0)
	{
		printk(KERN_ERR "melfas_ts_probe: i2c_master_send() failed\n [%d]", ret);
		return 0;
	}


	return true;
}
*/

/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-12-07] 
: For an abnormal condition after getting ESD */
static void ResetTS(void)
{
	struct mcs8000_ts_device *dev;
	dev = &mcs8000_ts_dev;

	mcs8000_Data_Clear();

	//disable_irq(dev->num_irq);


	/* add Touch power flag eungjin.kim@lge.com [2012-02-06]*/	
	if(power_flag==1){
			power_flag--;
			dev->power(OFF);
	}	
	/* add Touch power flag eungjin.kim@lge.com [2012-02-06]*/	



	mdelay(20);

	mcs8000_ts_on();	

	printk(KERN_DEBUG "Reset TS For ESD\n");

	//enable_irq(dev->num_irq);
}
/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-12-07] 
: For an abnormal condition after getting ESD */ 


/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-12-07] 
: For an abnormal condition after getting ESD */
int CheckTSForESD(unsigned char ucData)
{
	unsigned char ucStatus;
	ucStatus = ucData&0x0f;

	if (ucStatus == 0x0f) //Abnormal condition
	{
		ResetTS();
		return TRUE;
	} else {
		return FALSE;
	}
}
/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-12-07] 
: For an abnormal condition after getting ESD */ 

static void Release_All_Fingers(void)
{
		struct mcs8000_ts_device *dev;
		
		int i=0;
		dev = &mcs8000_ts_dev;
		for(i=0; i<MELFAS_MAX_TOUCH; i++) {
			if(-1 == g_Mtouch_info[i].strength) {
				g_Mtouch_info[i].posX = 0;
				g_Mtouch_info[i].posY = 0;
				continue;
			}
	
			g_Mtouch_info[i].strength = 0;
	
			input_report_abs(dev->input_dev, ABS_MT_TRACKING_ID, i);
			input_report_abs(dev->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
			input_report_abs(dev->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
			input_report_abs(dev->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength );
			input_report_abs(dev->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);
			input_mt_sync(dev->input_dev);
	
			g_Mtouch_info[i].posX = 0;
			g_Mtouch_info[i].posY = 0;
	
			if(0 == g_Mtouch_info[i].strength)
				g_Mtouch_info[i].strength = -1;
		}
		input_sync(dev->input_dev);		
}


static void mcs8000_work(struct work_struct *work)
{
	int read_num, FingerID;
	int touchType = 0, touchState = 0;
	struct mcs8000_ts_device *ts = container_of(to_delayed_work(work), struct mcs8000_ts_device, work);
	int ret 		= 0;
	int i = 0, j = 0;
	uint8_t buf[TS_READ_REGS_LEN];
  int keyID 	= 0;
	int iTouchedCnt;
	
	/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-11-23] 
	: For an abnormal condition of touchscreen after the phone sleeps on and off*/
	int Is_Touch_Valid = 0;
	/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-11-23] 
	: For an abnormal condition of touchscreen after the phone sleeps on and off*/ 
	
#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_work_func\n");

	if (ts == NULL)
			printk(KERN_ERR "melfas_ts_work_func : TS NULL\n");
#endif

/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-11-09] : 
TD1416085584 :  After sleep on and off while sensing a touchscreen,
Touchscreen doesn't work*/
	
	
/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-11-09]*/


	buf[0] = TS_READ_START_ADDR;

	ret = i2c_master_send(ts->client, buf, 1);
	if(ret < 0)
	{
#if DEBUG_PRINT
		printk(KERN_ERR "i2c_master_send: i2c failed\n");
		if (irq_flag == 0) {
			irq_flag++;
			enable_irq(ts->client->irq);
		}

		return ;	
#endif 
	}
	ret = i2c_master_recv(ts->client, buf, 1);
	if(ret < 0)
	{
#if DEBUG_PRINT
		printk(KERN_ERR "i2c_master_recv: i2c failed\n");
		if (irq_flag == 0) {
			irq_flag++;
			enable_irq(ts->client->irq);
		}

		return ;	
#endif 
	}

	read_num = buf[0];
			
/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-10-17] : 
TD1416085584 :  After sleeping on and off while sensing a touchscreen,
Touchscreen doesn't work*/

	iTouchedCnt = 6*5;

	if(read_num > iTouchedCnt)
	{	
		if (irq_flag == 0) {
			irq_flag++;
			enable_irq(ts->client->irq);
		}
		return ;
	}
/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-11-09]*/

	if(read_num>0)
	{
/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-11-23] 
: For an abnormal condition of touchscreen after the phone sleeps on and off*/	
	    Is_Touch_Valid = 1;
/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-11-23] 
: For an abnormal condition of touchscreen after the phone sleeps on and off*/ 
		
		buf[0] = TS_READ_START_ADDR2;

		ret = i2c_master_send(ts->client, buf, 1);
		if(ret < 0)
		{
#if DEBUG_PRINT
			printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
			if (irq_flag == 0) {
			irq_flag++;

			enable_irq(ts->client->irq);
}
			return ;	
#endif 
		}
		ret = i2c_master_recv(ts->client, buf, read_num);
		if(ret < 0)
		{
#if DEBUG_PRINT
			printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
			if (irq_flag == 0) {
			irq_flag++;
			enable_irq(ts->client->irq);
}
			return ;	
#endif 
		}

	/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-11-28] 
	: For an abnormal condition after getting ESD */
		ucSensedInfo  = buf[0];
		if (CheckTSForESD(ucSensedInfo))
		{
			if (irq_flag == 0) {
			irq_flag++;
			enable_irq(ts->client->irq);
			}
			
			return;
		}
	/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-11-28] 
	: For an abnormal condition after getting ESD */ 

		for(i = 0; i < read_num; i = i + 6)
		{
			touchType  = (buf[i] >> 5) & 0x03;				
			#if DEBUG_PRINT
			printk(KERN_INFO "TouchType  : [%d]\n", touchType);
			#endif

			/* Touch Type is Screen */
			if (touchType == TOUCH_SCREEN) 
			{
				FingerID = (buf[i] & 0x0F) -1;
				touchState = (buf[i] & 0x80);

				if((FingerID >=0) && (FingerID < MELFAS_MAX_TOUCH)) {
				g_Mtouch_info[FingerID].posX= (uint16_t)(buf[i + 1] & 0x0F) << 8 | buf[i + 2];
				g_Mtouch_info[FingerID].posY= (uint16_t)(buf[i + 1] & 0xF0) << 4 | buf[i + 3];				
				g_Mtouch_info[FingerID].width = buf[i + 4];

				if (touchState)
					g_Mtouch_info[FingerID].strength = buf[i + 5];
				else
					g_Mtouch_info[FingerID].strength = 0;
				}
			}
			/* Touch type is key */
			else if (touchType == TOUCH_KEY)
			{

				keyID = (buf[i] & 0x0F);
				touchState = (buf[i] & 0x80);

				/* 2012-10-08 JongWook-Park(blood9874@lge.com) [V3] Melfas mms-128s TSD patch [START] */
				#if 1
				if(g_touchLogEnable)
					printk(KERN_INFO "keyID    : [%d]\n", keyID);
				#endif
				/* 2012-10-08 JongWook-Park(blood9874@lge.com) [V3] Melfas mms-128s TSD patch [END] */

#if defined(CONFIG_MACH_MSM7X25A_V3_DS) || defined(CONFIG_MACH_MSM7X25A_V1)
				/* 2012-09-25 JongWook-Park(blood9874@lge.com) [V3] Bring up V3 mms-128s touch [START] */ 
				switch(keyID)
				{
					case 0x1:
						input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);
						break;
					case 0x2:
						input_report_key(ts->input_dev, KEY_HOMEPAGE/*KEY_HOME*/, touchState ? PRESS_KEY : RELEASE_KEY);
						break;
					case 0x3:
						input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);
						break;
					case 0x4:
						input_report_key(ts->input_dev, KEY_SIM_SWITCH, touchState ? PRESS_KEY : RELEASE_KEY);
						break;
					default:
						//make exception to debug
						//or printk msg
						break;					
				}
				/* 2012-09-25 JongWook-Park(blood9874@lge.com) [V3] Bring up V3 mms-128s touch [END] */ 
#else
				/* 2012-10-23 JongWook-Park(blood9874@lge.com) [V3] Single Touch Bring Up [START] */ 
				#if 0
				if (keyID == 0x1)
					input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);
				if (keyID == 0x2)
					input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);
				#else
				if (keyID == 0x1)
					input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);
				if (keyID == 0x2)
					input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);
				#endif
				/* 2012-10-23 JongWook-Park(blood9874@lge.com) [V3] Single Touch Bring Up [END] */ 
#endif
				/* LGE_CHANGE_E: E1 yongboem.kim@lge.com [2012-01-10] : for Rev.A Touch Key */
			}
		}

		/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-12-19] : 
		For the MIP Protocal*/

		/* 2012-11-27 JongWook-Park(blood9874@lge.com) mms-128s touch work function patch [START] */ 
		if (touchType == TOUCH_SCREEN){ 
			for(j = 0; j < MELFAS_MAX_TOUCH; j++) 
			{
				if(g_Mtouch_info[j].strength== -1)
					continue;

				/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-11-09] : 
				TD1416085584 :  After sleeping on and off while sensing a touchscreen,
				Touchscreen doesn't work*/
				if(Is_Release_Error[j]==1) {			
					input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, j);
					input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[j].posX);
					input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[j].posY);
					input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0/*g_Mtouch_info[j].strength*/ );
					input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[j].width);
	                input_report_abs(ts->input_dev, ABS_MT_PRESSURE, g_Mtouch_info[j].strength);                        
					input_mt_sync(ts->input_dev);		  
					//input_sync(ts->input_dev);/* LGE_CHANGE_S : wonsang.yoon@lge.com [2011-12-17]  blocking*/
					Is_Release_Error[j]=0;
				}		
				/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-11-09]*/
				
				if (g_Mtouch_info[j].strength > 0) {	// Press
				input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, j);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[j].posX);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[j].posY);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[j].strength);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[j].width);
	            input_report_abs(ts->input_dev, ABS_MT_PRESSURE, g_Mtouch_info[j].strength);                        
				input_mt_sync(ts->input_dev);   
				} else {	// Release
					input_mt_sync(ts->input_dev);
				}
				/* 2012-10-08 JongWook-Park(blood9874@lge.com) [V3] Melfas mms-128s TSD patch [START] */
				#if 1
				if(g_touchLogEnable)
				{
					printk(KERN_ERR "melfas_ts_work_func: Touch ID: %d, State : %d, x: %d, y: %d, z: %d w: %d\n", 
							j, (g_Mtouch_info[j].strength>0), g_Mtouch_info[j].posX, g_Mtouch_info[j].posY, g_Mtouch_info[j].strength, g_Mtouch_info[j].width);
				}
				#endif	
				/* 2012-10-08 JongWook-Park(blood9874@lge.com) [V3] Melfas mms-128s TSD patch [END] */

				if(g_Mtouch_info[j].strength == 0){
					g_Mtouch_info[j].strength = -1;
				}
			}

			#if 0
			if(g_touchLogEnable)
			{
				/* 2012-10-31 JongWook-Park(blood9874@lge.com) [V3] Melfas mms-128s Sensitivity Debug */
				intensity_extract();
			}
			#endif	

		}
		/* 2012-11-27 JongWook-Park(blood9874@lge.com) mms-128s touch work function patch [END] */ 
		
		input_sync(ts->input_dev);

	}			
	/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-11-23] 
	: For an abnormal condition of touchscreen after the phone sleeps on and off*/
	if(Is_Touch_Valid){
		/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-12-20] 
		: msleep function takes more than setting up delay time*/
		//msleep(1);
		usleep_range(1000,1000);	
		/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-12-20] 
		: msleep function takes more than setting up delay time*/
	}
	else{
		#if DEBUG_PRINT
		printk("mcs8000_work : Invalid data INT happen !!! Added more delay !!!");
		#endif
		msleep(20); 
	}
	/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-11-23] 
	: For an abnormal condition of touchscreen after the phone sleeps on and off*/  
	if (irq_flag == 0) {
		irq_flag++;
		enable_irq(ts->client->irq);
	}
	
}

static void mcs8000_Data_Clear(void) /* for touch stable */
{
	int i;

	for(i=0; i<MELFAS_MAX_TOUCH; i++)
	 {
	   if(g_Mtouch_info[i].strength != -1)
	   	{
	   	  Is_Release_Error[i]=1;
	  		g_Mtouch_info[i].strength = -1;
	   	}

	}
}


static irqreturn_t mcs8000_ts_irq_handler(int irq, void *handle)
{
	struct mcs8000_ts_device *dev = (struct mcs8000_ts_device *)handle;

#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_work_func is sending irq");
#endif		
		if (irq_flag == 1) {
		irq_flag--;	
		disable_irq_nosync(dev->num_irq);
		}
		/* schedule_delayed_work(&dev->work, 0); */
		//schedule_work(&dev->work);
		 queue_delayed_work(dev->ts_wq, &dev->work,msecs_to_jiffies(TS_POLLING_TIME)); 

	return IRQ_HANDLED;
}

#if 0
static int mcs8000_ts_off(void)
{
	struct mcs8000_ts_device *dev = NULL;
	int ret = 0;

	dev = &mcs8000_ts_dev;

/* add Touch power flag eungjin.kim@lge.com [2012-02-06]*/	
	if(power_flag==1){
			power_flag--;
			ret = dev->power(OFF);
	}	
/* add Touch power flag eungjin.kim@lge.com [2012-02-06]*/	

	if (ret < 0) {
		printk(KERN_ERR "mcs8000_ts_on power on failed\n");
		goto err_power_failed;
	}
	msleep(10);

err_power_failed:
	return ret;
}
#endif
/* LGE_CHANGE_S: E1 eungjin.kim@lge.com [2012-02-16] 
: For Touch screen non response after wakeup*/
int mcs8000_ts_on(void)
/* LGE_CHANGE_S: E1 eungjin.kim@lge.com [2012-02-16] 
: For Touch screen non response after wakeup*/
{
	struct mcs8000_ts_device *dev = NULL;
	int ret = 0;

	dev = &mcs8000_ts_dev;

/* add Touch power flag eungjin.kim@lge.com [2012-02-06]*/	
		if(power_flag==0){
			power_flag++;
			ret = dev->power(ON);
		}
/* add Touch power flag eungjin.kim@lge.com [2012-02-06]*/	

	if (ret < 0) {
		printk(KERN_ERR "mcs8000_ts_on power on failed\n");
		goto err_power_failed;
	}
	
/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-11-23] 
: For an abnormal condition of touchscreen after the phone sleeps on and off*/	
	msleep(30);
/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-11-23] 

: For an abnormal condition of touchscreen after the phone sleeps on and off*/ 
err_power_failed:
	return ret;
}

/* LGE_CHANGE_S: E1 eungjin.kim@lge.com [2012-02-16] 
: For Touch screen non response after wakeup*/
EXPORT_SYMBOL(mcs8000_ts_on);
/* LGE_CHANGE_E: E0 eungjin.kim@lge.com [2012-02-16] 
: For  For Touch screen non response after wakeup*/

void mcs8000_firmware_info(unsigned char *fw_ver, unsigned char *hw_ver,unsigned char *touch_id)
{
	unsigned char data;
	struct mcs8000_ts_device *dev = NULL;
	dev = &mcs8000_ts_dev;

	i2c_smbus_write_byte(dev->client, 0xf1);
		data = i2c_smbus_read_byte(dev->client);
		msleep(10);
		printk(KERN_INFO "HARDWARE REVISION [0x%x]\n", data);
		*hw_ver = data;
		ex_hw_ver = *hw_ver;	
		
		i2c_smbus_write_byte(dev->client, 0xf5);
		data = i2c_smbus_read_byte(dev->client);
		msleep(10);
		printk(KERN_INFO "FIRMWARE_VERSION [0x%x]\n", data);
		*fw_ver = data;
	
		ex_fw_ver=*fw_ver;
	
		data = gpio_get_value(GPIO_TOUCH_ID);
		printk(KERN_INFO "TOUCH_ID [0x%x]\n", data);
		*touch_id = data;
		ex_touch_id = *touch_id;
}
/*
static struct miscdevice mcs8000_ts_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mcs8000-touch",
	.fops = &mcs8000_ts_ioctl_fops,
};
*/

static ssize_t read_touch_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned char temp_fw_ver, temp_hw_ver, temp_touch_id;

	mcs8000_firmware_info(&temp_fw_ver, &temp_hw_ver, &temp_touch_id);
	return sprintf(buf, "M4 Touch ID:%02x,HW:%02x,FW:%02x\n",ex_touch_id,ex_hw_ver,ex_fw_ver);
}


/* static DEVICE_ATTR(touch_control, S_IRUGO|S_IWUSR,NULL,write_touch_control); */
/* static DEVICE_ATTR(touch_status, S_IRUGO,read_touch_status, NULL); */
static DEVICE_ATTR(version, S_IRUGO /*| S_IWUSR*/, read_touch_version, NULL);
/* static DEVICE_ATTR(dl_status, S_IRUGO,read_touch_dl_status, NULL); */

int mcs8000_create_file(struct input_dev *pdev)
{
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_version);
	if (ret) {
		printk(KERN_DEBUG "LG_FW : dev_attr_version create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_version);
		return ret;
	}
/*
	ret = device_create_file(&pdev->dev, &dev_attr_dl_status);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_dl_status create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_dl_status);
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_touch_status);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_touch_status create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_touch_status);
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_touch_control);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_touch_control create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_touch_control);
		return ret;
	}
*/
	return ret;
}

int mcs8000_remove_file(struct input_dev *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_version);
/*
 *	device_remove_file(&pdev->dev, &dev_attr_dl_status);
 *	device_remove_file(&pdev->dev, &dev_attr_touch_status);
 *	device_remove_file(&pdev->dev, &dev_attr_touch_control);
 */
	return 0;
}

static int mcs8000_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;
	/* int try_cnt = 0; */ /* For Touch FW Upgrade */
	/* unsigned char data; */ /* For Touch FW Upgrade */
	struct touch_platform_data *ts_pdata;
	struct mcs8000_ts_device *dev;
	unsigned char fw_ver, hw_ver;
	int fw_ret=0,vendor=0;
	
	/* unsigned char tmp_val, tmp_reg ; */
	unsigned char touch_id;
	irq_flag = 1;
	DMSG("%s: start...\n", __FUNCTION__);

	ts_pdata = client->dev.platform_data;

/* 2012-09-25 JongWook-Park(blood9874@lge.com) [V3] Bring up V3 mms-128s touch [START] */ 
	input_set_abs_params(mcs8000_ts_input, ABS_MT_POSITION_X, 0, TS_MAX_X_COORD, 0, 0);
	input_set_abs_params(mcs8000_ts_input, ABS_MT_POSITION_Y, 0, TS_MAX_Y_COORD, 0, 0);
	input_set_abs_params(mcs8000_ts_input, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
	input_set_abs_params(mcs8000_ts_input, ABS_MT_TRACKING_ID, 0, MELFAS_MAX_TOUCH-1, 0, 0);
	/* LGE_CHANGE_S mystery184.kim@lge.com fix multi-touch protocol */
	input_set_abs_params(mcs8000_ts_input, ABS_MT_PRESSURE, 0, 255, 0, 0);
	/* LGE_CHANGE_E mystery184.kim@lge.com fix multi-touch protocol */
	/* input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0); */
	/* 2012-09-25 JongWook-Park(blood9874@lge.com) [V3] Bring up V3 mms-128s touch [END] */ 

#if DEBUG_PRINT
  printk(KERN_INFO "ABS_MT_POSITION_X123 :  ABS_MT_POSITION_Y = [%d] : [%d] \n", ts_pdata->ts_x_max, ts_pdata->ts_y_max);
#endif

	dev = &mcs8000_ts_dev;

	 INIT_DELAYED_WORK(&dev->work, mcs8000_work); 
	//INIT_WORK(&dev->work, mcs8000_work);

	dev->power = ts_pdata->power;
	dev->num_irq = client->irq;
	dev->intr_gpio	= (client->irq) - NR_MSM_IRQS ;
	dev->sda_gpio = ts_pdata->sda;
	dev->scl_gpio  = ts_pdata->scl;


	dev->input_dev = mcs8000_ts_input;
	DMSG("mcs8000 dev->num_irq is %d , dev->intr_gpio is %d\n", dev->num_irq, dev->intr_gpio);

	dev->client = client;
	i2c_set_clientdata(client, dev);

	if (!(err = i2c_check_functionality(client->adapter, I2C_FUNC_I2C))) {
		printk(KERN_ERR "%s: fucntionality check failed\n", __FUNCTION__);
		return err;
	} 

	err = gpio_request(dev->intr_gpio, "touch_mcs8000");
	if (err < 0) {
		printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
		return err;
	}

	err = gpio_direction_input(dev->intr_gpio);
	if (err < 0) {
		printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
		return err;
	}

	/* TODO: You have try to change this driver's architecture using request_threaded_irq()
	 * So, I will change this to request_threaded_irq()
	 */
	err = request_threaded_irq(dev->num_irq, NULL, mcs8000_ts_irq_handler,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT, "mcs8000_ts", dev);
	
	if (err < 0) {
		printk(KERN_ERR "%s: request_irq failed\n", __FUNCTION__);
		return err;
	}
	if (irq_flag == 1) {
		irq_flag--;	
		disable_irq(dev->num_irq);
	}
	
//	mcs8000_ts_off();
//	mdelay(10);
	mcs8000_ts_on();
	mdelay(30);
	mcs8000_firmware_info(&fw_ver, &hw_ver,&touch_id);
	mdelay(10);


	//mcs8000_ext_ts = dev;
	
	
	mcs8000_create_file(mcs8000_ts_input);  
	
#if SET_DOWNLOAD_BY_GPIO
/*	buf[0] = TS_READ_VERSION_ADDR;
	ret = i2c_master_send(dev->client, buf, 1);
	if(ret < 0){
		printk(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);			
	}

	ret = i2c_master_recv(dev->client, buf, 4);
	if(ret < 0){
		printk(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]\n", ret);			
	}

*/	
	if(touch_id == 0){ //suntel
		 if(hw_ver==0x01){    	
			fw_ret = mcsdl_download_binary_data(1, 1,hw_ver,1);
			vendor=2;
		}
		if ( fw_ver !=TS_LATEST_FW_VERSION_EU_SUN ) { 
			fw_ret = mcsdl_download_binary_data(1, 1,hw_ver,1);
			vendor=2;
		}	
	}else{ //for rev C,D,1.1 Innotek
		if(hw_ver==0x00){	
			fw_ret = mcsdl_download_binary_data(1, 1,hw_ver,2);
			vendor=3;
		}
		if ( fw_ver !=TS_LATEST_FW_VERSION_EU_INO ){  
			 fw_ret =	mcsdl_download_binary_data(1, 1,hw_ver,2);
			 vendor=3;
		}	
	}	
#endif // SET_DOWNLOAD_BY_GPIO

	if (irq_flag == 0) {
		irq_flag++;
		enable_irq(dev->num_irq);
	}
	mcs8000_firmware_info(&fw_ver, &hw_ver,&touch_id);
	if(fw_ret!=0){
		mms100_ISC_download_binary_data(0,0,vendor);
	}
	
	
#ifdef CONFIG_HAS_EARLYSUSPEND
	ts_early_suspend.suspend = mcs8000_early_suspend;
	ts_early_suspend.resume = mcs8000_late_resume;
	ts_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1 ;
	register_early_suspend(&ts_early_suspend);
#endif	
	return 0;
}

static int mcs8000_ts_remove(struct i2c_client *client)
{
	struct mcs8000_ts_device *dev = i2c_get_clientdata(client);

	free_irq(dev->num_irq, dev);
	i2c_set_clientdata(client, NULL);

	return 0;
}

#ifndef CONFIG_HAS_EARLYSUSPEND
static int mcs8000_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct mcs8000_ts_device *dev = i2c_get_clientdata(client);

	
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		if (irq_flag == 1) {
		irq_flag--;	
		disable_irq(dev->num_irq);
		}
		
		DMSG("%s: irq disable\n", __FUNCTION__);
		
		if(power_flag==1){
			power_flag--;
			dev->power(OFF);
			}	

	return 0;
}

static int mcs8000_ts_resume(struct i2c_client *client)
{
	struct mcs8000_ts_device *dev = i2c_get_clientdata(client);

	
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);

		if(power_flag==0){
			power_flag++;
			dev->power(ON);
		}
			
		if (irq_flag == 0) {
			irq_flag++;
			enable_irq(dev->num_irq);
		}
		
		DMSG("%s: irq enable\n", __FUNCTION__);
	

	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mcs8000_early_suspend(struct early_suspend *h)
{
	int ret=0;
	struct mcs8000_ts_device *dev = &mcs8000_ts_dev;

		//mcs8000_Data_Clear(); 

		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		if (irq_flag == 1) {
			irq_flag--;	
			disable_irq(dev->num_irq);
			DMSG("%s: irq disable\n", __FUNCTION__);
		}

		ret = cancel_delayed_work_sync(&dev->work);
		Release_All_Fingers();
		if(power_flag==1){
			power_flag--;
			dev->power(OFF);
		}	
	
}

static void mcs8000_late_resume(struct early_suspend *h)
{
	struct mcs8000_ts_device *dev = &mcs8000_ts_dev;

	
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		
		if(power_flag==0){
			power_flag++;
			dev->power(ON);
		}
		
		if (irq_flag == 0) {
			irq_flag++;
			enable_irq(dev->num_irq);
			DMSG("%s: irq enable\n", __FUNCTION__);
		}
	
}
#endif

static const struct i2c_device_id mcs8000_ts_id[] = {
	{"touch_mcs8000", 1},
	{ }
};


static struct i2c_driver mcs8000_i2c_ts_driver = {
	.probe = mcs8000_ts_probe,
	.remove = mcs8000_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = mcs8000_ts_suspend,
	.resume  = mcs8000_ts_resume,
#endif
	.id_table = mcs8000_ts_id,
	.driver = {
		.name = "touch_mcs8000",
		.owner = THIS_MODULE,
	},
};

static int __devinit mcs8000_ts_init(void)
{
	int err = 0;
	struct mcs8000_ts_device *dev;
	dev = &mcs8000_ts_dev;

	memset(&mcs8000_ts_dev, 0, sizeof(struct mcs8000_ts_device));

	mcs8000_ts_input = input_allocate_device();
	if (mcs8000_ts_input == NULL) {
		printk(KERN_ERR "%s: input_allocate: not enough memory\n",
				__FUNCTION__);
		err = -ENOMEM;
		goto err_input_allocate;
	}

	mcs8000_ts_input->name = "touch_mcs8000";


    set_bit(EV_ABS, mcs8000_ts_input->evbit);
    set_bit(EV_KEY, mcs8000_ts_input->evbit);
    set_bit(INPUT_PROP_DIRECT, mcs8000_ts_input->propbit);
     
    mcs8000_ts_input->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);

	#if 0
    mcs8000_ts_input->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);
    mcs8000_ts_input->keybit[BIT_WORD(KEY_HOMEPAGE)] |= BIT_MASK(KEY_HOMEPAGE);
    mcs8000_ts_input->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
#if defined(CONFIG_MACH_MSM7X25A_V3_DS)    
    mcs8000_ts_input->keybit[BIT_WORD(KEY_SIM_SWITCH)] |= BIT_MASK(KEY_SIM_SWITCH);
#endif
	#else
		mcs8000_ts_input->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);
		mcs8000_ts_input->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
#if defined(CONFIG_MACH_MSM7X25A_V3_DS) || defined(CONFIG_MACH_MSM7X25A_V1)
		mcs8000_ts_input->keybit[BIT_WORD(KEY_HOMEPAGE)] |= BIT_MASK(KEY_HOMEPAGE);
		mcs8000_ts_input->keybit[BIT_WORD(KEY_SIM_SWITCH)] |= BIT_MASK(KEY_SIM_SWITCH);
#endif
	#endif	

	err = input_register_device(mcs8000_ts_input);
	if (err < 0) {
		printk(KERN_ERR "%s: Fail to register device\n", __FUNCTION__);
		goto err_input_register;
	}

	err = i2c_add_driver(&mcs8000_i2c_ts_driver);
	if (err < 0) {
		printk(KERN_ERR "%s: failed to probe i2c \n", __FUNCTION__);
		goto err_i2c_add_driver;
	}
/*
	err = misc_register(&mcs8000_ts_misc_dev);
	if (err < 0) {
		printk(KERN_ERR "%s: failed to misc register\n", __FUNCTION__);
		goto err_misc_register;
	}
*/
	dev->ts_wq = create_singlethread_workqueue("ts_wq");
	if (!dev->ts_wq) {
		printk(KERN_ERR "%s: failed to create wp\n", __FUNCTION__);
		err = -1;
	}
	return err;
/*
err_misc_register:
misc_deregister(&mcs8000_ts_misc_dev);
*/
err_i2c_add_driver:
	i2c_del_driver(&mcs8000_i2c_ts_driver);
err_input_register:
	input_unregister_device(mcs8000_ts_input);
err_input_allocate:
	input_free_device(mcs8000_ts_input);
	mcs8000_ts_input = NULL;
	return err;
}

static void __exit mcs8000_ts_exit(void)
{
	struct mcs8000_ts_device *dev;
	dev = &mcs8000_ts_dev;
/* mcs8000_remove_file(mcs8000_ts_input); */
	i2c_del_driver(&mcs8000_i2c_ts_driver);
	input_unregister_device(mcs8000_ts_input);
	input_free_device(mcs8000_ts_input);

	if (dev->ts_wq)
		destroy_workqueue(dev->ts_wq);
	printk(KERN_INFO "touchscreen driver was unloaded!\nHave a nice day!\n");
}

module_init(mcs8000_ts_init);
module_exit(mcs8000_ts_exit);

MODULE_DESCRIPTION("MELFAS MCS8000 Touchscreen Driver");
MODULE_LICENSE("GPL");

