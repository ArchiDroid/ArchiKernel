/*
 * MELFAS mcs8000 touchscreen driver
 *
 * Copyright (C) 2011 LGE, Inc.
 * Copyright (C) 2015 TeamVee.
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

#include <linux/wakelock.h>
#include "mms128s_ISC_download.h"
#include "mms128s_ioctl.h"

#include "MMS100S_ISC_Updater.h"

#include <linux/i2c-gpio.h>
#include CONFIG_LGE_BOARD_HEADER_FILE

#include <mach/vreg.h>

struct vreg {
	const char *name;
	unsigned id;
	int status;
	unsigned refcnt;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>

extern void GetManual(void* wParam, void* lParam);
extern void SetManual(void);
extern void ResetManual(void);

extern int mms100_ISP_download_binary_data(int dl_mode);
extern void mms100_download(void);

static struct early_suspend ts_early_suspend;
static void mcs8000_early_suspend(struct early_suspend *h);
static void mcs8000_late_resume(struct early_suspend *h);
#endif

static int mcs8000_ts_off(void);
int mcs8000_ts_on(void);
static void mcs8000_Data_Clear(void);
static void ResetTS(void);

#ifdef CONFIG_MACH_LGE
#define SUPPORT_TOUCH_KEY 1
#else
#define SUPPORT_TOUCH_KEY 0
#endif

#if SUPPORT_TOUCH_KEY
#define LG_FW_HARDKEY_BLOCK
#define TOUCH_SEARCH    247
#define TOUCH_BACK      248
#endif

#if defined (CONFIG_MACH_MSM7X27_JUMP)
#define TS_POLLING_TIME 2 /* msec */
#else
#define TS_POLLING_TIME 0 /* msec */
#endif

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
#define MCS8000_TS_MAX_HW_VERSION				0x40
#define MCS8000_TS_MAX_FW_VERSION				0x20

/* melfas data */
#define TS_MAX_Z_TOUCH	255
#define TS_MAX_W_TOUCH	30
#define MTSI_VERSION	0x07	/* 0x05 */
#define TS_MAX_X_COORD	240
#define TS_MAX_Y_COORD	320
#define FW_VERSION	0x00

#define TS_READ_START_ADDR	0x0F
#define TS_READ_START_ADDR2	0x10

#define TS_READ_HW_VERSION_ADDR	0xF1	/* HW Revision Info Address */

#define TS_HW_VERSION_ADDR	0xC2	/* FW Version Info Address  mms-128s*/
#define TS_READ_VERSION_ADDR	0xE3	/* FW Version Info Address  mms-128s*/
#define TS_PRODUCT_VERSION_ADDR	0xF6	/* Product Version Info Address  mms-128s*/

#define MELFAS_HW_VERSION	0x02	/* HW Version mms-128s*/
#define MELFAS_FW_VERSION	0x19 	/* FW Version mms-128s*/

#define MELFAS_HW_VERSION_SINGLE	0x01	/* HW Version mms-128s*/
#define MELFAS_FW_VERSION_SINGLE	0x16	/* FW Version mms-128s*/

#define TS_READ_HW_COMPATIBILITY_ADDR	0xF2	/* HW COMPATIBILITY Info Address */

#define TS_READ_REGS_LEN	66
#define MELFAS_MAX_TOUCH	0x05

#define I2C_RETRY_CNT	10
#define PRESS_KEY	1
#define RELEASE_KEY	0
#define DEBUG_PRINT	0



#define	SET_DOWNLOAD_BY_GPIO	1
#define TS_MODULE_A	0
#define TS_MODULE_B	16
#define TS_MODULE_C	17

#define TS_COMPATIBILITY_0	0
#define TS_COMPATIBILITY_A	1
#define TS_COMPATIBILITY_B	2

#define TS_LATEST_FW_VERSION_HW_00	5
#define TS_LATEST_FW_VERSION_HW_10	6
#define TS_LATEST_FW_VERSION_HW_11	7

#define GPIO_TS_ID	121

#define GPIO_TS_INT	39

#define KEY_SIM_SWITCH	228

int power_flag=0;


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

	struct work_struct  work;
#ifdef LG_FW_HARDKEY_BLOCK
	struct hrtimer touch_timer;
	bool hardkey_block;
#endif
	int num_irq;
	int intr_gpio;
	int scl_gpio;
	int sda_gpio;
	bool pendown;
	int (*power)(unsigned char onoff);
	struct workqueue_struct *ts_wq;

	struct wake_lock wakelock;
	int irq_sync;
	int fw_version;
	int hw_version;
	int status;
	int tsp_type;
};

static struct input_dev *mcs8000_ts_input = NULL;
struct mcs8000_ts_device mcs8000_ts_dev;
static int is_downloading = 0;
static int is_touch_suspend = 0;
int fw_rev = 0;
int Is_Release_Error[MELFAS_MAX_TOUCH]={0};

static unsigned char ucSensedInfo = 0;
static int iLevel = 0;

#define READ_NUM 8

enum {
	MCS8000_DM_TRACE_NO   = 1U << 0,
	MCS8000_DM_TRACE_YES  = 1U << 1,
	MCS8000_DM_TRACE_FUNC = 1U << 2,
	MCS8000_DM_TRACE_REAL = 1U << 3, 
};

enum {
	MCS8000_DEV_NORMAL,
	MCS8000_DEV_SUSPEND,
	MCS8000_DEV_DOWNLOAD
};
static unsigned char g_touchLogEnable = 0;
static int irq_flag = 0;

void mcs8000_firmware_info(unsigned char* fw_ver, unsigned char* hw_ver, unsigned char *comp_ver);
	
static int misc_opened = 0;
static unsigned int mcs8000_debug_mask = MCS8000_DM_TRACE_NO;

static struct mcs8000_ts_device *mcs8000_ext_ts = (void *)NULL; 

static __inline int mcs8000_ioctl_down_i2c_write(struct file *file, unsigned char addr,unsigned char val)
{
	struct mcs8000_ts_device *ts = file->private_data;
	int err = 0;
	struct i2c_msg msg;

	if (MCS8000_DM_TRACE_FUNC & mcs8000_debug_mask)
		DMSG("\n");

	if (ts == (void *)NULL) {
		printk(KERN_ERR "mcs8000 ts data is null\n");
		return -1;
	}

	if (ts->client == NULL) {
		printk(KERN_ERR "mcs8000_ts_ioctl_down_i2c_write: client is null\n");
		return -1;
	}
	msg.addr = addr;
	msg.flags = 0;
	msg.len = 1;
	msg.buf = &val;

	if ((err = i2c_transfer(ts->client->adapter, &msg, 1)) < 0) {
		printk(KERN_ERR "mcs8000_ts_ioctl_down_i2c_write: transfer failed[%d]\n", err);
	}

	return err;
}

static __inline int mcs8000_ioctl_down_i2c_read(struct file *file, unsigned char addr, unsigned char *ret)
{
	struct mcs8000_ts_device *ts = file->private_data;
	int err = 0;
	struct i2c_msg msg;

	if (MCS8000_DM_TRACE_FUNC & mcs8000_debug_mask)
		DMSG("\n");

	if (ts == (void *)NULL) {
		printk(KERN_ERR "mcs8000_ts_ioctl_down_i2c_read: client is null\n");
		return -1;
	}

	if (ts->client == NULL) {
		printk(KERN_ERR "mcs8000_ts_ioctl_down_i2c_read: transfer failed[%d]\n", err);
		return -1;
	}
	msg.addr = addr;
	msg.flags = 1;
	msg.len = 1;
	msg.buf = ret;

	if ((err = i2c_transfer(ts->client->adapter, &msg, 1)) < 0) {
		printk(KERN_ERR "mcs8000_ts_ioctl_down_i2c_read: transfer failed[%d]\n", err);
	}

	return err;
}

int mcs8000_ts_ioctl_down(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct mcs8000_ts_device *ts = file->private_data;
	int err = 0;
	struct mcs8000_ts_down_ioctl_i2c_type client_data;

	if (MCS8000_DM_TRACE_FUNC & mcs8000_debug_mask)
		DMSG("\n");

	if (_IOC_NR(cmd) >= MCS8000_TS_DOWN_IOCTL_MAXNR)
		return -EINVAL;

	switch (cmd) {
		case MCS8000_TS_DOWN_IOCTL_VDD_HIGH:
			err = ts->power(1);
			if (err < 0)
				printk(KERN_INFO "mcs8000_ts_ioctl_down: Power up failed\n");
			break;
		case MCS8000_TS_DOWN_IOCTL_VDD_LOW:
			err = ts->power(0);		
			if (err < 0)
				printk(KERN_INFO "mcs8000_ts_ioctl_down: Power down failed\n");
			break;
		case MCS8000_TS_DOWN_IOCTL_INTR_HIGH:
			gpio_set_value(ts->intr_gpio, 1);
			break;
		case MCS8000_TS_DOWN_IOCTL_INTR_LOW:
			gpio_set_value(ts->intr_gpio, 0);
			break;
		case MCS8000_TS_DOWN_IOCTL_INTR_OUT_HIGH:
			gpio_direction_output(ts->intr_gpio, 1);
			break;
		case MCS8000_TS_DOWN_IOCTL_INTR_OUT_LOW:
			gpio_direction_output(ts->intr_gpio, 0);
			break;
		case MCS8000_TS_DOWN_IOCTL_INTR_IN:
			gpio_direction_input(ts->intr_gpio);
			break;
		case MCS8000_TS_DOWN_IOCTL_SCL_HIGH:
			gpio_set_value(ts->scl_gpio, 1);
			break;
		case MCS8000_TS_DOWN_IOCTL_SCL_LOW:
			gpio_set_value(ts->scl_gpio, 0);
			break;
		case MCS8000_TS_DOWN_IOCTL_SDA_HIGH:
			gpio_set_value(ts->sda_gpio, 1);
			break;
		case MCS8000_TS_DOWN_IOCTL_SDA_LOW:
			gpio_set_value(ts->sda_gpio, 0);
			break;
		case MCS8000_TS_DOWN_IOCTL_SCL_OUT_HIGH:
			gpio_direction_output(ts->scl_gpio, 1);
			break;
		case MCS8000_TS_DOWN_IOCTL_SCL_OUT_LOW:
			gpio_direction_output(ts->scl_gpio, 0);
			break;
		case MCS8000_TS_DOWN_IOCTL_SDA_OUT_HIGH:
			gpio_direction_output(ts->sda_gpio, 1);
			break;
		case MCS8000_TS_DOWN_IOCTL_SDA_OUT_LOW:
			gpio_direction_output(ts->sda_gpio, 0);
			break;
		case MCS8000_TS_DOWN_IOCTL_SCL_IN:
			gpio_direction_input(ts->scl_gpio);
			break;
		case MCS8000_TS_DOWN_IOCTL_SDA_IN:
			gpio_direction_input(ts->sda_gpio);
			break;
		case MCS8000_TS_DOWN_IOCTL_SCL_READ:
			return gpio_get_value(ts->scl_gpio);
			break;
		case MCS8000_TS_DOWN_IOCTL_SDA_READ:
			return gpio_get_value(ts->sda_gpio);
			break;
		case MCS8000_TS_DOWN_IOCTL_I2C_ENABLE:
			break;
		case MCS8000_TS_DOWN_IOCTL_I2C_DISABLE:
			break;

		case MCS8000_TS_DOWN_IOCTL_I2C_READ:
			if (copy_from_user(&client_data, (struct mcs8000_ts_down_ioctl_i2c_type *)arg,
						sizeof(struct mcs8000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "mcs8000_ts_ioctl_down: copyfromuser-read error\n");
				return -EFAULT;
			}

			if (0 > mcs8000_ioctl_down_i2c_read(file, (unsigned char)client_data.addr,
						(unsigned char *)&client_data.data)) {
				err = -EIO;
			}

			if (copy_to_user((void *)arg, (const void *)&client_data,
						sizeof(struct mcs8000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "mcs8000_ts_ioctl_down: copytouser-read error\n");
				err = -EFAULT;
			}
			break;
		case MCS8000_TS_DOWN_IOCTL_I2C_WRITE:
			if (copy_from_user(&client_data, (struct mcs8000_ts_down_ioctl_i2c_type *)arg,
						sizeof(struct mcs8000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "mcs8000_ts_ioctl_down: copyfromuser-write error\n");
				return -EFAULT;
			}

			if (0 > mcs8000_ioctl_down_i2c_write(file, (unsigned char)client_data.addr,
						(unsigned char)client_data.data)) {
				err = -EIO;
			}
			break;
		case MCS8000_TS_DOWN_IOCTL_SELECT_TS_TYPE:
			break;
		default:
			err = -EINVAL;
			break;
	}

	if (err < 0)
		printk(KERN_ERR "\n==== Touch DONW IOCTL Fail....%d\n",_IOC_NR(cmd));

	return err;
}

int mcs8000_ts_ioctl_delay (unsigned int cmd)
{
	int err = 0;
	int delay=0;

	switch (cmd) {
		case MCS8000_TS_DOWN_IOCTL_DEALY_1US:	delay=0;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_2US:	delay=1;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_3US:	delay=1;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_5US:	delay=3;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_7US:	delay=5;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_10US:	delay=7;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_15US:	delay=13;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_20US:	delay=18;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_40US:	delay=37;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_70US:	delay=67;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_100US:	delay=97;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_150US:	delay=150;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_300US:	delay=300;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_500US:	delay=500;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_800US:	delay=800;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_1MS:	delay=1000;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_5MS:	delay=5000;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_10MS:	delay=10000;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_25MS:	delay=25000;	break; 
		case MCS8000_TS_DOWN_IOCTL_DEALY_30MS:	delay=30000;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_40MS:	delay=40000;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_45MS:	delay=45000;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_60MS:	delay=60000;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_80MS:	delay=80000;	break;
		case MCS8000_TS_DOWN_IOCTL_DEALY_100MS:	delay=100000;	break;
		default: err = -EINVAL; break;
	}
		udelay(delay);
	return err;
}

int Ask_INT_Status(void)
{
	int value = 0;
	value = gpio_get_value(GPIO_TS_INT);

    return value;
}

int AskTSisConnected(void)
{
	gpio_tlmm_config(GPIO_CFG(GPIO_TS_ID, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	iLevel = gpio_get_value(GPIO_TS_ID);
	if (!iLevel)
		return TRUE;
	else
		return FALSE;
}

static int firmware_update(struct mcs8000_ts_device *ts)
{
    int ret = 0;
	unsigned char fw_ver = 0, hw_ver = 0, comp_ver = 0;

	int iValue = 0;

	ret = AskTSisConnected();
	if (ret == FALSE) 
	{
		printk(KERN_ERR "Check H/W !!!... firmware_update Failed!\n");
		return ret;
	}

	GetManual(&iValue, 0);
	if (iValue == MANUAL_DOWNLOAD_ENABLE)
	{
		printk(KERN_INFO ".......MANUAL_DOWNLOAD_ENABLE : MFS_ISC_update \n");
        ret = MFS_ISC_update();
	}
	else
	{
		mcs8000_firmware_info(&fw_ver, &hw_ver, &comp_ver);
		printk(KERN_INFO "Firmware ver : [%d],HW ver : [%d] \n", fw_ver, hw_ver);

		if (((fw_ver < MELFAS_FW_VERSION)&&(comp_ver == 2))||(fw_ver == 0x0)||(fw_ver == 0xFF)||(comp_ver == 0))
		{
			printk(KERN_INFO ".......MFS_ISC_update......... \n");
			ret = MFS_ISC_update();
		}
		else
		{
			printk(KERN_INFO ".......MFS_ISC_update SKIP!! ....... \n");
		}

	}
    return ret;
}

void intensity_extract(void)
{
   int r;
   unsigned char master_write_buf[4] = {0,};
   unsigned char master_read_buf_array[28] = {0,};
   struct mcs8000_ts_device *dev = NULL;
   dev = &mcs8000_ts_dev;

   printk(KERN_DEBUG "\n===START===\n");
   for (r = 0; r < 11; r++)  /* Rx channel count */
   {
     master_write_buf[0] = 0xA0;
     master_write_buf[1] = 0x4D;
     master_write_buf[2] = 0;
     master_write_buf[3] = r;
     if (!i2c_master_send(dev->client,master_write_buf, 4))
     {
        goto ERROR_HANDLE;
     }

     master_write_buf[0] = 0xAE;
     if (!i2c_master_send(dev->client,master_write_buf, 1))
     {
        goto ERROR_HANDLE;
     }
     if (!i2c_master_recv(dev->client,master_read_buf_array, 1))
     {
        goto ERROR_HANDLE;
     }

     master_write_buf[0] = 0xAF;
     if (!i2c_master_send(dev->client,master_write_buf, 1))
     {
        goto ERROR_HANDLE;
     }
     if (!i2c_master_recv(dev->client,master_read_buf_array, master_read_buf_array[0]))
     {
        goto ERROR_HANDLE;
     }

	 /* Tx channel count */ 
     {
        printk(KERN_DEBUG "%3d,    %3d,    %3d,    %3d,    %3d,    %3d,    %3d,    %3d,    %3d,    %3d,    %3d,    %3d,    %3d,    %3d",
			 ((signed short*) master_read_buf_array)[0],((signed short*) master_read_buf_array)[1],((signed short*) master_read_buf_array)[2],
			 ((signed short*) master_read_buf_array)[3],((signed short*) master_read_buf_array)[4],((signed short*) master_read_buf_array)[5],
			 ((signed short*) master_read_buf_array)[6],((signed short*) master_read_buf_array)[7],((signed short*) master_read_buf_array)[8],
			 ((signed short*) master_read_buf_array)[9],((signed short*) master_read_buf_array)[10],((signed short*) master_read_buf_array)[11],
			 ((signed short*) master_read_buf_array)[12],((signed short*) master_read_buf_array)[13]);
     }
     printk(KERN_DEBUG "\n");
   }
   printk(KERN_DEBUG "\n===END===\n");
   return;
   
   ERROR_HANDLE: printk(KERN_DEBUG "ERROR!!! intensity_extract: i2c error \n");
   return;
}

static long mcs8000_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct mcs8000_ts_device *dev = &mcs8000_ts_dev;
  long lRet;
	int err = 0;
	unsigned char fw_ver = 0, hw_ver = 0, comp_ver = 0;;

	if (MCS8000_DM_TRACE_FUNC & mcs8000_debug_mask)
		DMSG("\n");

	switch (_IOC_TYPE(cmd)) {
		case MCS8000_TS_DOWN_IOCTL_MAGIC:
			err = mcs8000_ts_ioctl_down(file, cmd, arg);
			break;
		case MCS8000_TS_IOCTL_MAGIC :
			switch(cmd) {
				case MCS8000_TS_IOCTL_FW_VER:
				{
					if(is_touch_suspend)
					{
						if(power_flag==0){
							power_flag++;
							dev->power(ON);
							msleep(100);
						}
					}

					mcs8000_firmware_info(&fw_ver, &hw_ver, &comp_ver);
					printk(KERN_INFO "Firmware ver : [%d],HW ver : [%d], Product ver : [%d] \n", fw_ver, hw_ver, comp_ver);
	                
					if(is_touch_suspend)
					{
						if(power_flag==1){
							power_flag--;
							dev->power(OFF);
							msleep(100);
						}	
					}
					break;
				}
				case MCS8000_TS_IOCTL_MAIN_ON: 
				case MCS8000_TS_IOCTL_MAIN_OFF:
					break;

				case MCS8000_TS_IOCTL_DEBUG:
				{
				if (g_touchLogEnable)
					g_touchLogEnable = 0;
				else
					g_touchLogEnable = 1;
                    
					err = g_touchLogEnable;
					printk(KERN_INFO "<MELFAS> : Touch Log : %s \n", g_touchLogEnable ? "ENABLE" : "DISABLE");
				break;
				}

			  	case MCS8000_TS_IOCTL_KERNEL_DOWN:
				{
					printk(KERN_INFO "Normal Touch Firmware Down Load\n");

					if(is_touch_suspend)
					{
						if(power_flag==0){
							power_flag++;
							dev->power(ON);
							msleep(100);
						}
					}

					if(irq_flag == 0)
					{
						disable_irq(dev->num_irq);
						irq_flag = 1;
					}

					firmware_update(dev);

					dev->power(OFF);
					msleep(20);
					dev->power(ON);

					if (irq_flag == 1)
					{
						printk("[MELFAS] enable_irq\n");
						enable_irq(dev->num_irq);
						irq_flag = 0;
					}
				break;
			}
		case MCS8000_TS_IOCTL_KERNEL_DOWN_MANUAL:
		{
			printk(KERN_INFO "Manual Touch Firmware Down Load\n");
			SetManual();

			if(is_touch_suspend)
			{
				if(power_flag==0)
				{
					power_flag++;
					dev->power(ON);
					msleep(100);
				}

			}

			if(irq_flag == 0)
			{
				disable_irq(dev->num_irq);
					irq_flag = 1;
			}
			firmware_update(dev);
	                
	                dev->power(OFF);
	                msleep(20);
	                dev->power(ON);

	                if (irq_flag == 1) {
	                    printk("[MELFAS] enable_irq\n");
	                    enable_irq(dev->num_irq);
	                    irq_flag = 0;
	                }

            		ResetManual();
               		break;
                }
	}
	break;
            
	case MCS8000_TS_DOWN_IOCTL_DELAY :
		mcs8000_ts_ioctl_delay(cmd);
	break;
			
	default:
		printk(KERN_ERR "mcs8000_ts_ioctl: unknown ioctl\n");
		err = -EINVAL;
		break;
	}
	lRet = (long)err;
 
	return lRet;
}

static int mcs8000_open(struct inode *inode, struct file *file) 
{
	struct mcs8000_ts_device *ts = mcs8000_ext_ts;

	if (MCS8000_DM_TRACE_FUNC & mcs8000_debug_mask)
		DMSG("\n");

	if (ts == (void *)NULL)
		return -EIO;

	if (misc_opened)
		return -EBUSY;

	if (ts->status == MCS8000_DEV_NORMAL) {
		disable_irq(ts->num_irq);
		ts->irq_sync--;
			DMSG("touch download start: irq disabled by ioctl\n");
	}

	misc_opened = 1;

	file->private_data = ts;

	wake_lock(&ts->wakelock);

	ts->status = MCS8000_DEV_DOWNLOAD;
	is_downloading = 1;

	return 0;
}

static int mcs8000_release(struct inode *inode, struct file *file) 
{
	struct mcs8000_ts_device *ts = file->private_data;

	if (MCS8000_DM_TRACE_FUNC & mcs8000_debug_mask)
		DMSG("\n");

	if (ts == (void *)NULL)
		return -EIO;	

	if (ts->status == MCS8000_DEV_SUSPEND) {

		if(power_flag==1){
			power_flag--;
			ts->power(OFF);
			}

		if (MCS8000_DM_TRACE_YES & mcs8000_debug_mask)
			DMSG("touch download done: power off by ioctl\n");
	} 
	else {
		enable_irq(ts->num_irq);
		ts->irq_sync++;
			DMSG("touch download done: irq enabled by ioctl\n");

		ts->status = MCS8000_DEV_NORMAL;
	}

	misc_opened = 0;

	wake_unlock(&ts->wakelock);
	is_downloading = 0;

	return 0;
}

static struct file_operations mcs8000_ts_ioctl_fops = {
	.owner 						= THIS_MODULE, 
	.unlocked_ioctl 	= mcs8000_ioctl, 
	.open 						= mcs8000_open, 
	.release					= mcs8000_release,
};

static struct miscdevice mcs8000_ts_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "tsmms128",
	.fops = &mcs8000_ts_ioctl_fops,
};

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

#ifdef LG_FW_HARDKEY_BLOCK
static enum hrtimer_restart timed_touch_timer_func(struct hrtimer *timer)
{
	mcs8000_ts_dev.hardkey_block = 0;
	return HRTIMER_NORESTART;
}
#endif
static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];

static void ResetTS(void)
{
	struct mcs8000_ts_device *dev;
	dev = &mcs8000_ts_dev;

	mcs8000_Data_Clear();

	if(power_flag==1){
			power_flag--;
			dev->power(OFF);
	}	

	mdelay(20);

	mcs8000_ts_on();	

	printk(KERN_DEBUG "Reset TS For ESD\n");
}

int CheckTSForESD(unsigned char ucData)
{
	unsigned char ucStatus;
	ucStatus = ucData&0x0f;

	if (ucStatus == 0x0f)
	{
		ResetTS();
		return TRUE;
	} else {
		return FALSE;
	}
}

static void mcs8000_work(struct work_struct *work)
{
	int read_num, FingerID;
	int touchType = 0, touchState = 0;
	struct mcs8000_ts_device *ts = container_of(work, struct mcs8000_ts_device, work);
	int ret	= 0;
	int i = 0, j = 0;
	uint8_t buf[TS_READ_REGS_LEN];
	int keyID = 0;
	int iTouchedCnt;
	
	int Is_Touch_Valid = 0;
	
	#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_work_func\n");
	if (ts == NULL)
		printk(KERN_ERR "melfas_ts_work_func : TS NULL\n");
	#endif

	if(is_touch_suspend == 1) 
	{	
		#if DEBUG_PRINT
		printk(KERN_ERR "mcs8000_work : is_touch_suspend == 1, EXIT!! \n");
		#endif
		/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-11-23] 
		: For an abnormal condition of touchscreen after the phone sleeps on and off*/				
		msleep(20); 
		enable_irq(ts->client->irq);
		/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-11-23] 
		: For an abnormal condition of touchscreen after the phone sleeps on and off*/ 		  
		return;
	}

	for(i=0; i< I2C_RETRY_CNT; i++)
	{
		buf[0] = TS_READ_START_ADDR; /* 0x0F */

		ret = i2c_master_send(ts->client, buf, 1);
		ret = i2c_master_recv(ts->client, buf, 1);

	    if (ret >= 0)
	    {
	        break; /* i2c success!! */
	    }
	}
	if(ret < 0 )
	{
		#if DEBUG_PRINT
		printk(KERN_ERR "mcs8000_work: i2c failed\n");
		#endif 
		ResetTS(); /* 2012-11-06 JongWook-Park(blood9874@lge.com) [V3] i2c re-try fail, re-boot */
		enable_irq(ts->client->irq);
		return ;
	}
	else
	{
		read_num = buf[0];
		#if DEBUG_PRINT
		printk("mcs8000_work : read_num : [%d] \n", read_num);
		#endif
	}	

	/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2011-10-17] : 
	TD1416085584 :  After sleeping on and off while sensing a touchscreen,
	Touchscreen doesn't work*/

	/* 2012-11-27 JongWook-Park(blood9874@lge.com) mms-128s touch work function patch */ 
	iTouchedCnt = 6*5;
	if(read_num > iTouchedCnt)
	{	
		#if DEBUG_PRINT
		printk("mcs8000_work : read_num > iTouchedCnt EXIT !! \n");
		#endif
		enable_irq(ts->client->irq);
		return ;
	}
	/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2011-11-09]*/

	if(read_num > 0)
	{
	    Is_Touch_Valid = 1;

        for (i = 0; i < I2C_RETRY_CNT; i++)
        {
			buf[0] = TS_READ_START_ADDR2; /* 0x10 */

			ret = i2c_master_send(ts->client, buf, 1);
			ret = i2c_master_recv(ts->client, buf, read_num);

			if (ret >= 0)
			{
				break; /* i2c success!! */
			}
        }
		if(ret < 0)
		{
			#if DEBUG_PRINT
			printk(KERN_ERR "mcs8000_work: i2c failed\n");
			#endif	
			ResetTS(); /* 2012-11-06 JongWook-Park(blood9874@lge.com) [V3] i2c re-try fail, re-boot */
			enable_irq(ts->client->irq);
			return ;	

		}

		ucSensedInfo  = buf[0];
		if (CheckTSForESD(ucSensedInfo))
		{
			#if DEBUG_PRINT
			printk("mcs8000_work : ESD EXIT !! \n");
			#endif
			enable_irq(ts->client->irq);
			return;
		}

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
			else if (touchType == TOUCH_KEY)
			{

				keyID = (buf[i] & 0x0F);
				touchState = (buf[i] & 0x80);

				printk(KERN_INFO "keyID : [%d]\n", keyID);

				switch(keyID)
				{
					case 0x1:
						input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);
						break;
#ifdef CONFIG_MACH_MSM7X25A_V3_DS
					case 0x2:
						input_report_key(ts->input_dev, KEY_HOMEPAGE, touchState ? PRESS_KEY : RELEASE_KEY);
						break;
#else
					case 0x2:
						input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);
						break;
#endif
					case 0x3:
						input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);
						break;
					case 0x4:
						input_report_key(ts->input_dev, KEY_SIM_SWITCH, touchState ? PRESS_KEY : RELEASE_KEY);
						break;
					default:
						break;					
				}
			}
		}

		if (touchType == TOUCH_SCREEN){ 
			for(j = 0; j < MELFAS_MAX_TOUCH; j++) 
			{
				if(g_Mtouch_info[j].strength== -1)
					continue;

				if(Is_Release_Error[j]==1) {			
					input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, j);
					input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[j].posX);
					input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[j].posY);
					input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0/*g_Mtouch_info[j].strength*/ );
					input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[j].width);
	                input_report_abs(ts->input_dev, ABS_MT_PRESSURE, g_Mtouch_info[j].strength);                        
					input_mt_sync(ts->input_dev);		  
					Is_Release_Error[j]=0;
				}		

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

				if(g_touchLogEnable)
				{
					printk(KERN_ERR "melfas_ts_work_func: Touch ID: %d, State : %d, x: %d, y: %d, z: %d w: %d\n", 
							j, (g_Mtouch_info[j].strength>0), g_Mtouch_info[j].posX, g_Mtouch_info[j].posY, g_Mtouch_info[j].strength, g_Mtouch_info[j].width);
				}

				if(g_Mtouch_info[j].strength == 0){
					g_Mtouch_info[j].strength = -1;
				}
			}

			if(g_touchLogEnable)
			{
				intensity_extract();
			}

		}
		input_sync(ts->input_dev);

	}
	if(Is_Touch_Valid){
		usleep_range(1000,1000);
	}
	else{
		#if DEBUG_PRINT
		printk("mcs8000_work : Invalid data INT happen !!! Added more delay !!!");
		#endif
		msleep(20); 
	}

	enable_irq(ts->client->irq);
}

static void mcs8000_Data_Clear(void)
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

		disable_irq_nosync(dev->num_irq);

		schedule_work(&dev->work);
#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_work_func is sending irq");
#endif
	return IRQ_HANDLED;
}

static int mcs8000_ts_off(void)
{
	struct mcs8000_ts_device *dev = NULL;
	int ret = 0;

	dev = &mcs8000_ts_dev;

	if(power_flag==1){
			power_flag--;
			ret = dev->power(OFF);
	}

	if (ret < 0) {
		printk(KERN_ERR "mcs8000_ts_on power on failed\n");
		goto err_power_failed;
	}
	msleep(10);

err_power_failed:
	return ret;
}

int mcs8000_ts_on(void)
{
	struct mcs8000_ts_device *dev = NULL;
	int ret = 0;

	dev = &mcs8000_ts_dev;

	if(power_flag==0){
		power_flag++;
		ret = dev->power(ON);
	}

	if (ret < 0) {
		printk(KERN_ERR "mcs8000_ts_on power on failed\n");
		goto err_power_failed;
	}

	msleep(30);

err_power_failed:
	return ret;
}

EXPORT_SYMBOL(mcs8000_ts_on);

void mcs8000_firmware_info(unsigned char *fw_ver, unsigned char *hw_ver, unsigned char *comp_ver)
{
	unsigned char ucTXBuf[1] = {0};
	unsigned char ucRXBuf[10] = {0,};
	int iRet = 0;
	struct mcs8000_ts_device *dev = NULL;
	dev = &mcs8000_ts_dev;

	ucTXBuf[0] = TS_HW_VERSION_ADDR;

	iRet = i2c_master_send(dev->client, ucTXBuf, 1);
	if(iRet < 0)
	{
#if DEBUG_PRINT
		printk(KERN_ERR "mcs8000_firmware_info: i2c failed\n");
		return ;	
#endif 
	}

	iRet = i2c_master_recv(dev->client, ucRXBuf, 1);
	if(iRet < 0)
	{
#if DEBUG_PRINT
		printk(KERN_ERR "mcs8000_firmware_info: i2c failed\n");
		return ;	
#endif 
	}
	*hw_ver = ucRXBuf[0];

	ucTXBuf[0] = TS_READ_VERSION_ADDR;

	iRet = i2c_master_send(dev->client, ucTXBuf, 1);
	if(iRet < 0)
	{
#if DEBUG_PRINT
		printk(KERN_ERR "mcs8000_firmware_info: i2c failed\n");
		return ;	
#endif 
	}

	iRet = i2c_master_recv(dev->client, ucRXBuf, 1);
	if(iRet < 0)
	{
#if DEBUG_PRINT
		printk(KERN_ERR "mcs8000_firmware_info: i2c failed\n");
		return ;	
#endif 
	}
	*fw_ver = ucRXBuf[0];

	ucTXBuf[0] = TS_PRODUCT_VERSION_ADDR;

	iRet = i2c_master_send(dev->client, ucTXBuf, 1);
	if(iRet < 0)
	{
#if DEBUG_PRINT
		printk(KERN_ERR "mcs8000_firmware_info: i2c failed\n");
		return ;	
#endif 
	}

	iRet = i2c_master_recv(dev->client, ucRXBuf, 10);
	if(iRet < 0)
	{
#if DEBUG_PRINT
		printk(KERN_ERR "mcs8000_firmware_info: i2c failed\n");
		return ;	
#endif 
	}

	if(strncmp(ucRXBuf, "V3_S", 4) == 0)
		*comp_ver = 1; /* V3 Single */
	else if((strncmp(ucRXBuf, "V3_D", 4) == 0)||(strncmp(ucRXBuf, "V3", 2) == 0))
		*comp_ver = 2; /* V3 Dual */
	else
		*comp_ver = 0; /* V3 Unknown */

	printk("Touch PRODUCT = %s, comp_ver = %d\n",ucRXBuf,*comp_ver);
}

static ssize_t read_touch_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	int iRet = 0;
	unsigned char hw_ver, fw_ver, comp_ver;

	if(is_touch_suspend)
	{
		mcs8000_ts_on();
	}

	printk(KERN_DEBUG "TOUCHSCREEN FW VERSION Starts \n");

	mcs8000_firmware_info(&fw_ver, &hw_ver, &comp_ver);

	iRet = sprintf(buf, "%02x \n", fw_ver);
	printk(KERN_DEBUG "TOUCHSCREEN FW VERSION : %d \n", fw_ver);

	if(is_touch_suspend)
	{
		mcs8000_ts_off();
	}
	return iRet;
}

static DEVICE_ATTR(version, S_IRUGO, read_touch_version, NULL);

int mcs8000_create_file(struct input_dev *pdev)
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

int mcs8000_remove_file(struct input_dev *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_version);
	return 0;
}

static int mcs8000_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;

	struct touch_platform_data *ts_pdata;
	struct mcs8000_ts_device *dev;

	DMSG("%s: start...\n", __FUNCTION__);

	ts_pdata = client->dev.platform_data;

	input_set_abs_params(mcs8000_ts_input, ABS_MT_POSITION_X, 0, TS_MAX_X_COORD, 0, 0);
	input_set_abs_params(mcs8000_ts_input, ABS_MT_POSITION_Y, 0, TS_MAX_Y_COORD, 0, 0);
	input_set_abs_params(mcs8000_ts_input, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
	input_set_abs_params(mcs8000_ts_input, ABS_MT_TRACKING_ID, 0, MELFAS_MAX_TOUCH-1, 0, 0);
	input_set_abs_params(mcs8000_ts_input, ABS_MT_PRESSURE, 0, 255, 0, 0);
#if DEBUG_PRINT
	printk(KERN_INFO "ABS_MT_POSITION_X :  ABS_MT_POSITION_Y = [%d] : [%d] \n", ts_pdata->ts_x_max, ts_pdata->ts_y_max);
#endif

	dev = &mcs8000_ts_dev;

	INIT_WORK(&dev->work, mcs8000_work);

	dev->power = ts_pdata->power;
	dev->num_irq = client->irq;
	dev->intr_gpio	= (client->irq) - NR_MSM_IRQS ;
	dev->sda_gpio = ts_pdata->sda;
	dev->scl_gpio  = ts_pdata->scl;

#ifdef LG_FW_HARDKEY_BLOCK
	hrtimer_init(&dev->touch_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dev->touch_timer.function = timed_touch_timer_func;
#endif
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

	disable_irq(dev->num_irq);
	mcs8000_ts_off();
	mdelay(10);
	mcs8000_ts_on();

	mcsdl_delay(MCSDL_DELAY_100MS);
	mcsdl_delay(MCSDL_DELAY_100MS);

	err = firmware_update(dev);

	if (err < 0)
	{
		printk(KERN_ERR "[mms-128s]:firmware update fail\n");
	}

	err = misc_register(&mcs8000_ts_misc_dev);
	if (err < 0) {
		printk(KERN_ERR "mcs8000_probe_ts: misc register failed\n");
		return err;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts_early_suspend.suspend = mcs8000_early_suspend;
	ts_early_suspend.resume = mcs8000_late_resume;
	ts_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1 ;
	register_early_suspend(&ts_early_suspend);
#endif

	mcs8000_ext_ts = dev;
	wake_lock_init(&dev->wakelock, WAKE_LOCK_SUSPEND, "mcs8000");

	mcs8000_create_file(mcs8000_ts_input);  

	enable_irq(dev->num_irq);
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

	if (is_downloading == 0) {
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		disable_irq(dev->num_irq);
		DMSG("%s: irq disable\n", __FUNCTION__);

		if(power_flag==1){
			power_flag--;
			dev->power(OFF);
			}
	}
	is_touch_suspend = 1;

	return 0;
}

static int mcs8000_ts_resume(struct i2c_client *client)
{
	struct mcs8000_ts_device *dev = i2c_get_clientdata(client);

	if (is_downloading == 0) {
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);

		if(power_flag==0){
			power_flag++;
			dev->power(ON);
		}

		enable_irq(dev->num_irq);
		DMSG("%s: irq enable\n", __FUNCTION__);
	}
	is_touch_suspend = 0;

	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mcs8000_early_suspend(struct early_suspend *h)
{
	struct mcs8000_ts_device *dev = &mcs8000_ts_dev;

	if (is_downloading == 0) {

		mcs8000_Data_Clear(); 


		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		disable_irq(dev->num_irq);
		DMSG("%s: irq disable\n", __FUNCTION__);

		if(power_flag==1){
			power_flag--;
			dev->power(OFF);
			}

			}
	is_touch_suspend = 1;
}

static void mcs8000_late_resume(struct early_suspend *h)
{
	struct mcs8000_ts_device *dev = &mcs8000_ts_dev;

	if (is_downloading == 0) {
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
	
		if(power_flag==0){
			power_flag++;
			dev->power(ON);
		}

		enable_irq(dev->num_irq);
		DMSG("%s: irq enable\n", __FUNCTION__);
	}
	is_touch_suspend = 0;
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

	mcs8000_ts_input->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);
	mcs8000_ts_input->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
#ifdef CONFIG_MACH_MSM7X25A_V3_DS
	mcs8000_ts_input->keybit[BIT_WORD(KEY_HOMEPAGE)] |= BIT_MASK(KEY_HOMEPAGE);
#endif
	mcs8000_ts_input->keybit[BIT_WORD(KEY_SIM_SWITCH)] |= BIT_MASK(KEY_SIM_SWITCH);

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

	dev->ts_wq = create_singlethread_workqueue("ts_wq");
	if (!dev->ts_wq) {
		printk(KERN_ERR "%s: failed to create wp\n", __FUNCTION__);
		err = -1;
	}
	return err;

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
