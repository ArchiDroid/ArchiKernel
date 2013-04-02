/*
 * Touchscreen driver for Melfas MMS-100s series 
 *
 * Copyright (C) 2013 Melfas Inc.
 * Author: DVK team <dvk@melfas.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
//#define DEBUG /* if DEBUG is activated dev_dbg will be printed */
#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/earlysuspend.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/platform_data/mms_ts.h>
#include <linux/completion.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>

/* Flag to enable touch key */
#define MMS_HAS_TOUCH_KEY	1
/* if not defined MT_PROTOCOL_TYPE_A, then work MT_PROTOCOL_TYPE_B */
#define MT_PROTOCOL_TYPE_A
#undef MT_PROTOCOL_TYPE_A //Default is MT_PROTOCOL_TYPE_B

/*
 * Set this 0 when don't need to upgrade the firmware
 */
#define MMS_TS_FW_AUTO_UPDATE	1
//#define MMS_TS_FW_VERIFIY

#define GPIO_TOUCH_ID					121

/*
 * ISC_XFER_LEN	- ISC unit transfer length.
 * Give number of 2 power n, where  n is between 2 and 10 
 * i.e. 4, 8, 16 ,,, 1024 
 */
#define ISC_XFER_LEN		1024

#define MMS_FLASH_PAGE_SZ	1024
#define ISC_BLOCK_NUM		(MMS_FLASH_PAGE_SZ / ISC_XFER_LEN)

#define FLASH_VERBOSE_DEBUG	1
#define MAX_SECTION_NUM		3

#define MAX_FINGER_NUM		5
#define FINGER_EVENT_SZ		6
#define MAX_WIDTH		30
#define MAX_PRESSURE		255
#define MAX_LOG_LENGTH		128

/* Registers */
#define MMS_MODE_CONTROL	0x01
#define MMS_TX_NUM		0x0B
#define MMS_RX_NUM		0x0C
#define MMS_EVENT_PKT_SZ	0x0F
#define MMS_INPUT_EVENT		0x10
#define MMS_UNIVERSAL_CMD	0xA0
#define MMS_UNIVERSAL_RESULT	0xAF
#define MMS_CMD_ENTER_ISC	0x5F
#define MMS_FW_VERSION		0xE1

/* Universal commands */
#define MMS_CMD_SET_LOG_MODE	0x20

/* Event types */
#define MMS_LOG_EVENT		0xD
#define MMS_NOTIFY_EVENT	0xE
#define MMS_ERROR_EVENT		0xF
#define MMS_TOUCH_KEY_EVENT	0x40

/* Firmware file name */
#define FW_NAME_TOVIS			"mms_ts.fw"
//#define FW_NAME_TOVIS			"mms_ts_tovis.fw"
#define FW_NAME_XXX			"mms_ts_xxx.fw"

enum {
	GET_RX_NUM	= 1,
	GET_TX_NUM,
	GET_EVENT_DATA,
};

enum {
	LOG_TYPE_U08	= 2,
	LOG_TYPE_S08,
	LOG_TYPE_U16,
	LOG_TYPE_S16,
	LOG_TYPE_U32	= 8,
	LOG_TYPE_S32,
};

enum {
	ISC_ADDR		= 0xD5,

	ISC_CMD_READ_STATUS	= 0xD9,	
	ISC_CMD_READ		= 0x4000,
	ISC_CMD_EXIT		= 0x8200,
	ISC_CMD_PAGE_ERASE	= 0xC000,
	
	ISC_PAGE_ERASE_DONE	= 0x10000,
	ISC_PAGE_ERASE_ENTER	= 0x20000,
};

struct mms_ts_info {
	struct i2c_client 		*client;
	struct input_dev 		*input_dev;
	char 				phys[32];

	u8				tx_num;
	u8				rx_num;

	int 				irq;

	struct mms_ts_platform_data 	*pdata;

	char 				*fw_name;
	u8 ver[MAX_SECTION_NUM];
	int  		fw_update_done;
	int  		fw_read_fail;
	struct early_suspend		early_suspend;

	struct mutex 			lock;
	bool				enabled;

	struct cdev			cdev;
	dev_t				mms_dev;
	struct class			*class;

	struct mms_log_data {
		u8			*data;
		int			cmd;
	} log;
#if defined(MT_PROTOCOL_TYPE_A)
	struct mms_touch_data {
		int x;
		int y;
		int touch_major;
		int pressure;
	} mt_data[MAX_FINGER_NUM];
#else

#endif
};

struct mms_bin_hdr {
	char	tag[8];

	u16	core_version;
	u16	section_num;
	u16	contains_full_binary;
	u16	reserved0;

	u32	binary_offset;
	u32	binary_length;

	u32	extention_offset;	
	u32	reserved1;
	
} __attribute__ ((packed));

struct mms_fw_img {
	u16	type;
	u16	version;

	u16	start_page;
	u16	end_page;

	u32	offset;
	u32	length;

} __attribute__ ((packed));

struct isc_packet {
	u8	cmd;
	u32	addr;
	u8	data[0];
} __attribute__ ((packed));

static void mms_report_input_data(struct mms_ts_info *info, u8 sz, u8 *buf);
static void mms_ts_early_suspend(struct early_suspend *h);
static void mms_ts_late_resume(struct early_suspend *h);

static int mms100s_touch_id(void)
{
	return gpio_get_value(GPIO_TOUCH_ID);
}


static ssize_t read_touch_fw_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	struct i2c_client *client = to_i2c_client(dev);
	struct mms_ts_info *info = i2c_get_clientdata(client);
		
	printk(KERN_INFO "Firmware bootloader version : %02x \n", info->ver[0]);
	printk(KERN_INFO "Firmware core version : %02x \n", info->ver[1]);
	printk(KERN_INFO "Firmware config version  : %02x \n", info->ver[2]);

	ret = sprintf(buf, "%02x \n", info->ver[2]);

	return ret;
}

static DEVICE_ATTR(version, S_IRUGO /*| S_IWUSR*/, read_touch_fw_version, NULL);

static int mms100s_create_file(struct input_dev *pdev)
{
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_version);
	if(ret)
	{
		printk(KERN_DEBUG "LG_FW : dev_attr_version create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_version);
		return ret;
	}

	return ret;
}

static int mms_ts_power_on(struct mms_ts_info *info, int on)
{
	int rc = 0;

	if(info->pdata->power)
		rc = info->pdata->power(&info->client->dev, on);

	return rc;
}

static int mms_ts_reseting(struct mms_ts_info *info)
{
	if(!info->pdata->gpio_resetb)
		return 0;
	if(gpio_get_value(info->pdata->gpio_resetb))
	{
		return 1;
	}

	return 0;
}

static void mms_ts_enable(struct mms_ts_info *info)
{
	dev_dbg(&info->client->dev, "%s\n", __func__);

	if (info->enabled)
		return;
	
	mutex_lock(&info->lock);

	/*
	gpio_direction_output(info->pdata->gpio_resetb, 0);
	udelay(5);
	gpio_direction_input(info->pdata->gpio_resetb);
	*/

	/* use vdd control instead */
	//gpio_direction_output(info->pdata->gpio_vdd_en, 1);
	mms_ts_power_on(info, 1);
	msleep(50);
	
	info->enabled = true;
	if(info->fw_update_done || info->fw_read_fail) 
		enable_irq(info->irq);

	mutex_unlock(&info->lock);

}

static void mms_ts_disable(struct mms_ts_info *info)
{
	if (!info->enabled)
		return;
	dev_dbg(&info->client->dev, "%s\n", __func__);

	mutex_lock(&info->lock);

	disable_irq(info->irq);

	/*
	i2c_smbus_write_byte_data(info->client, MMS_MODE_CONTROL, 0);
	usleep_range(10000, 12000);
	*/

	mms_ts_power_on(info, 0);
	msleep(50);

	info->enabled = false;

	mutex_unlock(&info->lock);
}

static void mms_reboot(struct mms_ts_info *info)
{
	struct i2c_adapter *adapter = to_i2c_adapter(info->client->dev.parent);

	i2c_lock_adapter(adapter);
	dev_dbg(&info->client->dev, "%s\n", __func__);

	mms_ts_power_on(info, 0);
	msleep(150);

	mms_ts_power_on(info, 1);
	msleep(50);

	i2c_unlock_adapter(adapter);
}

static void mms_clear_input_data(struct mms_ts_info *info)
{
	int i;
	dev_dbg(&info->client->dev, "%s\n", __func__);
	
	for (i = 0; i < MAX_FINGER_NUM; i++) {
#if defined(MT_PROTOCOL_TYPE_A)
		input_report_abs(info->input_dev, ABS_MT_PRESSURE, 0);
		input_mt_sync(info->input_dev);
#else
		input_mt_slot(info->input_dev, i);
		input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, false);
#endif 
	}

	input_sync(info->input_dev);
#if defined(MT_PROTOCOL_TYPE_A)
	memset(&info->mt_data, 0, sizeof(info->mt_data));
#else

#endif

	return;
}

static int mms_fs_open(struct inode *node, struct file *fp)
{
	struct mms_ts_info *info;
	struct i2c_client *client;
	struct i2c_msg msg;
	u8 buf[3] = {
		MMS_UNIVERSAL_CMD,
		MMS_CMD_SET_LOG_MODE,
		true,
	};
	
	info = container_of(node->i_cdev, struct mms_ts_info, cdev);
	client = info->client;

	dev_dbg(&info->client->dev, "%s\n", __func__);

	disable_irq(info->irq);
	fp->private_data = info;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = sizeof(buf);

	i2c_transfer(client->adapter, &msg, 1);

	info->log.data = kzalloc(MAX_LOG_LENGTH * 20 + 5, GFP_KERNEL);

	mms_clear_input_data(info);

	return 0;
}

static int mms_fs_release(struct inode *node, struct file *fp)
{
	struct mms_ts_info *info = fp->private_data;

	dev_dbg(&info->client->dev, "%s\n", __func__);

	mms_clear_input_data(info);
	mms_reboot(info);

	kfree(info->log.data);
	enable_irq(info->irq);

	return 0;
}

static void mms_event_handler(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int sz;
	int ret;
	int row_num;
	u8 reg = MMS_INPUT_EVENT;
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.buf = &reg,
			.len = 1,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.buf = info->log.data,
		},

	};
	struct mms_log_pkt {
		u8	marker;
		u8	log_info;
		u8	code;
		u8	element_sz;
		u8	row_sz;
	} __attribute__ ((packed)) *pkt = (struct mms_log_pkt *)info->log.data;

	dev_dbg(&info->client->dev, "%s\n", __func__);
	
	memset(pkt, 0, sizeof(*pkt));

	if (mms_ts_reseting(info))
		return;
	
	
	sz = i2c_smbus_read_byte_data(client, MMS_EVENT_PKT_SZ);
	if(sz < 0)
	{
		dev_err(&client->dev, "failed to read %d bytes of data\n", sz);
		mms_clear_input_data(info);
		mms_reboot(info);
		return;
	}
	
	msg[1].len = sz;

	ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
	if (ret != ARRAY_SIZE(msg)) {
		dev_err(&client->dev,
			"failed to read %d bytes of data\n",
			sz);
		return;
	}

	if ((pkt->marker & 0xf) == MMS_LOG_EVENT) {
		if ((pkt->log_info & 0x7) == 0x1) {
			pkt->element_sz = 0;
			pkt->row_sz = 0;

			return;
		}

		switch (pkt->log_info >> 4) {
		case LOG_TYPE_U08:
		case LOG_TYPE_S08:
			msg[1].len = pkt->element_sz;
			break;
		case LOG_TYPE_U16:
		case LOG_TYPE_S16:
			msg[1].len = pkt->element_sz * 2;
			break;
		case LOG_TYPE_U32:
		case LOG_TYPE_S32:
			msg[1].len = pkt->element_sz * 4;
			break;
		default:
			dev_err(&client->dev, "invalied log type\n");
			return;
		}

		msg[1].buf = info->log.data + sizeof(struct mms_log_pkt);
		reg = MMS_UNIVERSAL_RESULT;
		row_num = pkt->row_sz ? pkt->row_sz : 1;

		while (row_num--) {
			while (mms_ts_reseting(info))
				;
			ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
			msg[1].buf += msg[1].len;
		};
	} else {
		mms_report_input_data(info, sz, info->log.data);
		memset(pkt, 0, sizeof(*pkt));
	}

	return;
}

static void mms_report_input_data(struct mms_ts_info *info, u8 sz, u8 *buf)
{
	int i;
	struct i2c_client *client = info->client;
	int id;
	int x;
	int y;
	int touch_major;
	int pressure;
	int key_code;
	int key_state;
	u8 *tmp;

	if (buf[0] == MMS_NOTIFY_EVENT) {
		dev_info(&client->dev, "TSP mode changed (%d)\n", buf[1]);
		goto out;
	} else if (buf[0] == MMS_ERROR_EVENT) {
		dev_info(&client->dev, "Error detected, restarting TSP\n");
		mms_clear_input_data(info);
		mms_reboot(info);
		goto out;
	}

	for (i = 0; i < sz; i += FINGER_EVENT_SZ) {
		tmp = buf + i;

		if (tmp[0] & MMS_TOUCH_KEY_EVENT) {

			switch (tmp[0] & 0xf) {
			case 1:
				key_code = KEY_BACK;
				break;
			case 2:
				key_code = KEY_HOMEPAGE;
				break;
			case 3:
				key_code = KEY_MENU;
				break;
			case 4:
				key_code = KEY_SIM_SWITCH;
				break;
			default:
				dev_err(&client->dev, "unknown key type : %d, tmp[0] : %d, sz : %d, i :%d \n", tmp[0] & 0xf, tmp[0], sz, i);
				goto out;
				break;
			}

			key_state = (tmp[0] & 0x80) ? 1 : 0;
			input_report_key(info->input_dev, key_code, key_state);

		} else {
			id = (tmp[0] & 0xf) -1;
			x = tmp[2] | ((tmp[1] & 0xf) << 8);
			y = tmp[3] | (((tmp[1] >> 4 ) & 0xf) << 8);
			touch_major = tmp[4];
#if defined(MT_PROTOCOL_TYPE_A)
			pressure = (tmp[0] & 0x80) ? tmp[5] : 0;

			info->mt_data[id].x = x;
			info->mt_data[id].y = y;
			info->mt_data[id].touch_major = touch_major;
			info->mt_data[id].pressure = pressure;

			if (pressure == 0) {
				input_report_abs(info->input_dev, ABS_MT_TRACKING_ID, i);
				input_report_abs(info->input_dev, ABS_MT_POSITION_X, info->mt_data[i].x);
				input_report_abs(info->input_dev, ABS_MT_POSITION_Y, info->mt_data[i].y);
				input_report_abs(info->input_dev, ABS_MT_TOUCH_MAJOR,  info->mt_data[i].pressure);
				input_report_abs(info->input_dev, ABS_MT_WIDTH_MAJOR, info->mt_data[i].touch_major);
				input_report_abs(info->input_dev, ABS_MT_PRESSURE, info->mt_data[i].pressure);
				input_mt_sync(info->input_dev);
				printk(KERN_INFO "id:%d, x:%d, y:%d, tourch_major:%d, pressure:%d\n", id, x,y,touch_major,pressure);

				}
			}
	}

		
	
			for (i = 0; i < MAX_FINGER_NUM; i++) {
			if(info->mt_data[i].pressure  == 0) {
				//input_mt_sync(info->input_dev);
				continue;
			}
			input_report_abs(info->input_dev, ABS_MT_TRACKING_ID, i);
			input_report_abs(info->input_dev, ABS_MT_POSITION_X, info->mt_data[i].x);
			input_report_abs(info->input_dev, ABS_MT_POSITION_Y, info->mt_data[i].y);
			input_report_abs(info->input_dev, ABS_MT_TOUCH_MAJOR,  info->mt_data[i].pressure);
			input_report_abs(info->input_dev, ABS_MT_WIDTH_MAJOR, info->mt_data[i].touch_major);
			input_report_abs(info->input_dev, ABS_MT_PRESSURE, info->mt_data[i].pressure);
			input_mt_sync(info->input_dev);
			
			dev_dbg(&info->client->dev, "input report - id:%d, x:%d, y:%d, tourch_major:%d, pressure:%d\n",  i, info->mt_data[i].x,info->mt_data[i].y,info->mt_data[i].touch_major,info->mt_data[i].pressure);
			
		}


#else
			pressure = tmp[5];
			if (pressure == 0)
				dev_info(&client->dev, "id:%d, x:%d, y:%d, tourch_major:%d, pressure:%d\n", id, x,y,touch_major,pressure);
//				dev_dbg(&client->dev, "tmp[0]:%d, tmp[1]:%d, tmp[2]:%d, tmp[3]:%d\n", tmp[0], tmp[1], tmp[2], tmp[3]);
//				dev_dbg(&client->dev, "sz : %d, i :%d \n",  sz, i);
			input_mt_slot(info->input_dev, id);
			
			if (!(tmp[0] & 0x80)) {
				input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, false);
				continue;
			}

			input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, true);
			input_report_abs(info->input_dev, ABS_MT_POSITION_X, x);
			input_report_abs(info->input_dev, ABS_MT_POSITION_Y, y);
			input_report_abs(info->input_dev, ABS_MT_TOUCH_MAJOR, touch_major);
			input_report_abs(info->input_dev, ABS_MT_PRESSURE, pressure);
		}
	}

#endif

	input_sync(info->input_dev);
out:
	return;

}

static ssize_t mms_fs_read(struct file *fp, char *rbuf, size_t cnt, loff_t *fpos)
{
	struct mms_ts_info *info = fp->private_data;
	struct i2c_client *client = info->client;
	int ret = 0;

	dev_dbg(&info->client->dev, "%s\n", __func__);

	switch (info->log.cmd) {
	case GET_RX_NUM:
		ret = copy_to_user(rbuf, &info->rx_num, 1);
		break;
	case GET_TX_NUM:
		ret = copy_to_user(rbuf, &info->tx_num, 1);
		break;
	case GET_EVENT_DATA:
		mms_event_handler(info);
		/* copy data without log marker */
		ret = copy_to_user(rbuf, info->log.data + 1, cnt);
		break;
	default:
		dev_err(&client->dev, "unknown command\n");
		ret = -EFAULT;
		break;
	}

	return ret;
}

static ssize_t mms_fs_write(struct file *fp, const char *wbuf, size_t cnt, loff_t *fpos)
{
	struct mms_ts_info *info = fp->private_data;
	struct i2c_client *client = info->client;
	u8 *buf;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = cnt,
	};
	int ret = 0;
	dev_dbg(&info->client->dev, "%s\n", __func__);
	
	mutex_lock(& info->lock);

	if (!info->enabled)
		goto tsp_disabled;
	
	msg.buf = buf = kzalloc(cnt + 1, GFP_KERNEL);

	if ((buf == NULL) || copy_from_user(buf, wbuf, cnt)) {
		dev_err(&client->dev, "failed to read data from user\n");
		ret = -EIO;
		goto out;
	}

	if (cnt == 1) {
		info->log.cmd = *buf;
	} else {
		if (i2c_transfer(client->adapter, &msg, 1) != 1) {
			dev_err(&client->dev, "failed to transfer data\n");
			ret = -EIO;
			goto out;
		}
	}

	ret = 0;

out:
	kfree(buf);
tsp_disabled:
	mutex_unlock(&info->lock);

	return ret;
}

static int mms_isc_read_status(struct mms_ts_info *info, u32 val)
{
	struct i2c_client *client = info->client;
	u8 cmd = ISC_CMD_READ_STATUS;
	u32 result = 0;
	int cnt = 100;
	int ret = 0, r = 0;

	dev_dbg(&info->client->dev, "%s\n", __func__);
	
	do {
		r = i2c_smbus_read_i2c_block_data(client, cmd, 4, (u8 *)&result);
		if(r < 0)
		{
			dev_err(&client->dev,"i2c read faild : %d\n", r);
			mms_clear_input_data(info);
			mms_reboot(info);
			ret = r;
			break;
		}
		if (result == val)
			break;
		msleep(1);
	} while (--cnt);

	if (!cnt) {
		dev_err(&client->dev,
			"status read fail. cnt : %d, val : 0x%x != 0x%x\n",
			cnt, result, val);
	}

	return ret;
}

static int mms_isc_transfer_cmd(struct mms_ts_info *info, int cmd)
{
	struct i2c_client *client = info->client;
	struct isc_packet pkt = { ISC_ADDR, cmd };
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = sizeof(struct isc_packet),
		.buf = (u8 *)&pkt,
	};

	return (i2c_transfer(client->adapter, &msg, 1) != 1);
}

static int mms_isc_erase_page(struct mms_ts_info *info, int page)
{
	return mms_isc_transfer_cmd(info, ISC_CMD_PAGE_ERASE | page) ||
		mms_isc_read_status(info, ISC_PAGE_ERASE_DONE | ISC_PAGE_ERASE_ENTER | page);
}

static int mms_isc_enter(struct mms_ts_info *info)
{
	return i2c_smbus_write_byte_data(info->client, MMS_CMD_ENTER_ISC, true);
}

static int mms_isc_exit(struct mms_ts_info *info)
{
	return mms_isc_transfer_cmd(info, ISC_CMD_EXIT);
}

static int mms_flash_section(struct mms_ts_info *info, struct mms_fw_img *img, const u8 *data)
{
	struct i2c_client *client = info->client;
	struct isc_packet *isc_packet;
	int ret;
	int page, i;
	struct i2c_msg msg[2] = {
		{
			.addr = client->addr,
			.flags = 0,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = ISC_XFER_LEN,
		},
	};
	int ptr = img->offset;
	
	dev_dbg(&info->client->dev, "%s\n", __func__);
	
	isc_packet = kzalloc(sizeof(*isc_packet) + ISC_XFER_LEN, GFP_KERNEL);
	isc_packet->cmd = ISC_ADDR;

	msg[0].buf = (u8 *)isc_packet;
	msg[1].buf = kzalloc(ISC_XFER_LEN, GFP_KERNEL);

	for (page = img->start_page; page <= img->end_page; page++) {
		mms_isc_erase_page(info, page);

		for (i = 0; i < ISC_BLOCK_NUM; i++, ptr += ISC_XFER_LEN) {
			/* flash firmware */
			u32 tmp = page * 256 + i * (ISC_XFER_LEN / 4);
			put_unaligned_le32(tmp, &isc_packet->addr);
			msg[0].len = sizeof(struct isc_packet) + ISC_XFER_LEN;

			memcpy(isc_packet->data, data + ptr, ISC_XFER_LEN);
			if (i2c_transfer(client->adapter, msg, 1) != 1)
				goto i2c_err;

			/* verify firmware */
			tmp |= ISC_CMD_READ;
			put_unaligned_le32(tmp, &isc_packet->addr);
			msg[0].len = sizeof(struct isc_packet);

			if (i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg)) != ARRAY_SIZE(msg))
				goto i2c_err;
#ifdef MMS_TS_FW_VERIFIY
			if (memcmp(isc_packet->data, msg[1].buf, ISC_XFER_LEN)) {
#if FLASH_VERBOSE_DEBUG
				print_hex_dump(KERN_ERR, "mms fw wr : ",
						DUMP_PREFIX_OFFSET, 16, 1,
						isc_packet->data, ISC_XFER_LEN, false);

				print_hex_dump(KERN_ERR, "mms fw rd : ",
						DUMP_PREFIX_OFFSET, 16, 1,
						msg[1].buf, ISC_XFER_LEN, false);
#endif
				dev_err(&client->dev, "flash verify failed\n");
				ret = -1;
				goto out;
			}
#endif
		}
	}

	dev_info(&client->dev, "section [%d] update succeeded\n", img->type);

	ret = 0;
	goto out;

i2c_err:
	dev_err(&client->dev, "i2c failed @ %s\n", __func__);
	ret = -1;

out:
	kfree(isc_packet);
	kfree(msg[1].buf);

	return ret;
}

static int get_fw_version(struct i2c_client *client, u8 *buf)
{
	u8 cmd = MMS_FW_VERSION;
	struct i2c_msg msg[2] = {
		{
			.addr = client->addr,
			.flags = 0,
			.buf = &cmd,
			.len = 1,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.buf = buf,
			.len = MAX_SECTION_NUM,
		},
	};

	return (i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg)) != ARRAY_SIZE(msg));
}

static int mms_flash_fw(const struct firmware *fw, struct mms_ts_info *info)
{
	int ret;
	int i;
	struct mms_bin_hdr *fw_hdr;
	struct mms_fw_img **img = NULL;
	struct i2c_client *client = info->client;
	u8 ver[MAX_SECTION_NUM];
	u8 target[MAX_SECTION_NUM];
	int offset = sizeof(struct mms_bin_hdr);
	int retires = 3;
	bool update_flag = false;

	dev_dbg(&info->client->dev, "%s\n", __func__);

	fw_hdr = (struct mms_bin_hdr *)fw->data;

	img = kzalloc(sizeof(*img) * fw_hdr->section_num, GFP_KERNEL);

	while (retires--) {
		if (!get_fw_version(client, ver))
			break;
		else
			mms_reboot(info);
	}

	if (retires < 0) {
		dev_warn(&client->dev, "failed to obtain ver. info\n");
		memset(ver, 0xff, sizeof(ver));
	} else {
		print_hex_dump(KERN_INFO, "mms_ts fw ver : ", DUMP_PREFIX_NONE, 16, 1,
				ver, MAX_SECTION_NUM, false);
		mms_isc_enter(info);
	}

	memcpy(info->ver, ver, sizeof(ver));

	for (i = 0; i < fw_hdr->section_num; i++, offset += sizeof(struct mms_fw_img)) {
		img[i] = (struct mms_fw_img *)(fw->data + offset);
		target[i] = img[i]->version;

		if (ver[img[i]->type] != target[i]) {
			update_flag = true;
			dev_info(&client->dev,
				"section [%d] is need to be updated. ver : 0x%x, bin : 0x%x\n",
				img[i]->type, ver[img[i]->type], target[i]);

			if ((ret = mms_flash_section(info, img[i], fw->data + fw_hdr->binary_offset))) {
				mms_reboot(info);
				goto out;
			}

		} else {
			dev_info(&client->dev, "section [%d] is up to date\n", i);
		}
	}

	if (update_flag)
		mms_isc_exit(info);

	msleep(5);
	mms_reboot(info);

	if (update_flag) {
			if (get_fw_version(client, ver)) {
				dev_err(&client->dev, "failed to obtain version after flash\n");
				ret = -1;
				goto out;
			} else {
				//Touch Firmware Version update after flashing
				dev_info(&client->dev, "old firmware version %02x\n", info->ver[2]);
				memcpy(info->ver, ver, sizeof(ver));
				dev_info(&client->dev, "new firmware version %02x\n", info->ver[2]);
				
				for (i = 0; i < fw_hdr->section_num; i++) {
					if (ver[img[i]->type] != target[i]) {
						dev_info(&client->dev,
							"version mismatch after flash. [%d] 0x%x != 0x%x\n",
							i, ver[img[i]->type], target[i]);

						ret = -1;
						goto out;
					}
				}
			}
	}

	ret = 0;

out:
	if (img)
		kfree(img);

	return ret;
}


static void mms_flash_fw_controller(const struct firmware *fw, void *context)
{
	int retires = 3;
	int ret;
	u8 ver[MAX_SECTION_NUM];
	struct mms_ts_info *info = context;
	struct i2c_client *client = info->client;

	if(!fw) {	/*In cse of there is no firmware or reading fail firmware.*/
		dev_info(&client->dev, "failed to read firmware from filesystem.\n");

		if (!get_fw_version(client, ver)) /* Update current firmware version in the structure */
			memcpy(info->ver, ver, sizeof(ver));
		else
			dev_info(&client->dev, "failed to read firmware in the IC.\n");

		info->fw_read_fail = 1;
		
		enable_irq(client->irq);
		return;
	}

	//disable_irq(client->irq);

	dev_info(&info->client->dev, "touch firmware is updating.\n");

	do {
		ret = mms_flash_fw(fw, info);
	} while (ret && --retires);

	if (!retires)
		dev_err(&client->dev, "failed to update firmware.\n");

	release_firmware(fw);

	info->tx_num = i2c_smbus_read_byte_data(client, MMS_TX_NUM);
	info->rx_num = i2c_smbus_read_byte_data(client, MMS_RX_NUM);

	info->fw_update_done = 1;
	enable_irq(client->irq);

	if(info->tx_num < 0 || info->rx_num < 0)
	{
		dev_err(&client->dev,"i2c read faild  info->tx_num : %d, info->rx_num : %d\n", info->tx_num, info->rx_num);
		mms_clear_input_data(info);
		mms_reboot(info);
		return;
	}

	dev_info(&info->client->dev, "touch firmware is updated.\n");	
}


static irqreturn_t mms_ts_interrupt(int irq, void *dev_id)
{
	struct mms_ts_info *info = dev_id;
	struct i2c_client *client = info->client;
	u8 buf[MAX_FINGER_NUM * FINGER_EVENT_SZ] = { 0, };
	int ret;
	int sz;
	u8 reg = MMS_INPUT_EVENT;
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.buf = &reg,
			.len = 1,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.buf = buf,
		},
	};
	
	dev_dbg(&info->client->dev, "%s\n", __func__);
	
	sz = i2c_smbus_read_byte_data(client, MMS_EVENT_PKT_SZ);
	// Protection code for ESD
	//mms_ts 2-0048: unknown key type : 5, tmp[0] : 117, sz : 36, i :18  
	//buffer overflow : u8 buf[MAX_FINGER_NUM * FINGER_EVENT_SZ] = { 0, };
         if(sz > (FINGER_EVENT_SZ * MAX_FINGER_NUM ) || sz < 0)
         {
                       mms_clear_input_data(info);
                       mms_reboot(info);
                       dev_info(&client->dev, "MMS_EVENT_PKT_SZ invalid : %d\n", sz);
                       return IRQ_HANDLED;
         }

	msg[1].len = sz;

	ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));

	if (ret != ARRAY_SIZE(msg)) {
		dev_err(&client->dev,
			"failed to read %d bytes of touch data (%d)\n",
			sz, ret);
	} else {
		mms_report_input_data(info, sz, buf);
	}

	return IRQ_HANDLED;
}

static int mms_ts_config(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int ret;
//	const char *fw_name = FW_NAME_TOVIS;		//default is tovis
	int ts_vendor;

	dev_dbg(&info->client->dev, "%s\n", __func__);

	mms_reboot(info);

	if(!MMS_TS_FW_AUTO_UPDATE) {
		info->fw_update_done = 1;
		enable_irq(client->irq);
		return 0;
	}

	ts_vendor = mms100s_touch_id();
	dev_info(&client->dev, "touch_id pin status : %d\n", ts_vendor);
/* When Touch dualization */
	if(1/*ts_vendor == 0*/)
	{
		info->fw_name = kstrdup(FW_NAME_TOVIS, GFP_KERNEL);
		dev_info(&client->dev, "ts_vendor : tovis\n");
	}
	else
	{
		info->fw_name = kstrdup(FW_NAME_XXX, GFP_KERNEL);
		dev_info(&client->dev, "ts_vendor : xxx\n");
	}

	ret = request_firmware_nowait(THIS_MODULE, true, info->fw_name,
					&client->dev, GFP_KERNEL,
					info, mms_flash_fw_controller);

	if (ret) {
		dev_err(&client->dev, "failed to schedule firmware update\n");
	}

	return 0;
}

static int mms_ts_input_open(struct input_dev *dev)
{
	struct mms_ts_info *info = input_get_drvdata(dev);
	struct i2c_client *client = info->client;
	dev_dbg(&info->client->dev, "%s\n", __func__);
//	ret = wait_for_completion_interruptible_timeout(&info->init_done,
//				msecs_to_jiffies(90 * MSEC_PER_SEC));

	mms_ts_enable(info);

	info->tx_num = i2c_smbus_read_byte_data(client, MMS_TX_NUM);
	info->rx_num = i2c_smbus_read_byte_data(client, MMS_RX_NUM);

	if(info->tx_num < 0 || info->rx_num < 0)
	{
		dev_err(&client->dev,"i2c read faild  info->tx_num : %d, info->rx_num : %d\n", info->tx_num, info->rx_num);
		mms_clear_input_data(info);
		mms_reboot(info);		
	}
	
	return 0;
}

static void mms_ts_input_close(struct input_dev *dev)
{
	struct mms_ts_info *info = input_get_drvdata(dev);
	dev_dbg(&info->client->dev, "%s\n", __func__);
	
	mms_ts_disable(info);
}

static struct file_operations mms_fops = {
	.owner		= THIS_MODULE,
	.open		= mms_fs_open,
	.release	= mms_fs_release,
	.read		= mms_fs_read,
	.write		= mms_fs_write,
};

static int __devinit mms_ts_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct mms_ts_info *info;
	struct input_dev *input_dev;
	int ret = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	input_dev = input_allocate_device();

	if (!info || !input_dev) {
		dev_err(&client->dev, "Failed to allocated memory\n");
		return -ENOMEM;
	}

	info->client = client;
	info->input_dev = input_dev;
	info->pdata = client->dev.platform_data;
	info->fw_update_done = 0;
	info->irq = client->irq;

	dev_dbg(&info->client->dev, "%s\n", __func__);
	mutex_init(&info->lock);
#if defined(MT_PROTOCOL_TYPE_A)
#else
	input_mt_init_slots(input_dev, MAX_FINGER_NUM);
#endif

	snprintf(info->phys, sizeof(info->phys),
		"%s/input0", dev_name(&client->dev));

	input_dev->name = "mms100s_ts";
	input_dev->phys = info->phys;
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
	input_dev->open = mms_ts_input_open;
	input_dev->close = mms_ts_input_close;

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
#if defined(MT_PROTOCOL_TYPE_A)
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, MAX_FINGER_NUM, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, MAX_PRESSURE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, 240, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, 320, 0, 0);
#else
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, MAX_WIDTH, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, MAX_PRESSURE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, info->pdata->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, info->pdata->max_y, 0, 0);

#endif
	
	input_set_drvdata(input_dev, info);

#if MMS_HAS_TOUCH_KEY
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(KEY_MENU, input_dev->keybit);
	__set_bit(KEY_BACK, input_dev->keybit);
	__set_bit(KEY_HOMEPAGE, input_dev->keybit);
	__set_bit(KEY_SIM_SWITCH, input_dev->keybit);
#endif

	ret = input_register_device(input_dev);
	if (ret) {
		dev_err(&client->dev, "failed to register input dev\n");
		return -EIO;
	}

	i2c_set_clientdata(client, info);

#ifdef CONFIG_HAS_EARLYSUSPEND
	info->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	info->early_suspend.suspend = mms_ts_early_suspend;
	info->early_suspend.resume = mms_ts_late_resume;
	register_early_suspend(&info->early_suspend);
#endif

	if (alloc_chrdev_region(&info->mms_dev, 0, 1, "mms_ts")) {
		dev_err(&client->dev, "failed to allocate device region\n");
		return -ENOMEM;
	}

	cdev_init(&info->cdev, &mms_fops);
	info->cdev.owner = THIS_MODULE;

	if (cdev_add(&info->cdev, info->mms_dev, 1)) {
		dev_err(&client->dev, "failed to add ch dev\n");
		return -EIO;
	}

	info->class = class_create(THIS_MODULE, "mms_ts");
	device_create(info->class, NULL, info->mms_dev, NULL, "mms_ts");

	dev_dbg(&info->client->dev, "before request_threaded_irq\n");
	ret = request_threaded_irq(client->irq, NULL, mms_ts_interrupt,
		IRQF_TRIGGER_LOW | IRQF_ONESHOT,
		"mms_ts", info);	
	dev_dbg(&info->client->dev, "after request_threaded_irq\n");
	disable_irq(client->irq);
		
	mms_ts_config(info);

	mms100s_create_file(input_dev);

	dev_notice(&client->dev, "mms dev initialized\n");

	return 0;
}

static int __devexit mms_ts_remove(struct i2c_client *client)
{
	struct mms_ts_info *info = i2c_get_clientdata(client);
	
	dev_dbg(&info->client->dev, "%s\n", __func__);
	
	if (info->irq >= 0)
		free_irq(info->irq, info);

	input_unregister_device(info->input_dev);
	unregister_early_suspend(&info->early_suspend);
	device_destroy(info->class, info->mms_dev);
	class_destroy(info->class);
	
	kfree(info->fw_name);
	kfree(info);

	return 0;
}

#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
static int mms_ts_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mms_ts_info *info = i2c_get_clientdata(client);

	dev_dbg(&info->client->dev, "%s\n", __func__);
	
	mutex_lock(&info->input_dev->mutex);

	if (info->input_dev->users) {
		mms_ts_disable(info);
		mms_clear_input_data(info);
	}

	mutex_unlock(&info->input_dev->mutex);
	return 0;

}

static int mms_ts_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mms_ts_info *info = i2c_get_clientdata(client);

	dev_dbg(&info->client->dev, "%s\n", __func__);
	mutex_lock(&info->input_dev->mutex);

	if (info->input_dev->users)
		mms_ts_enable(info);

	mutex_unlock(&info->input_dev->mutex);

	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mms_ts_early_suspend(struct early_suspend *h)
{
	struct mms_ts_info *info;
	info = container_of(h, struct mms_ts_info, early_suspend);
	mms_ts_suspend(&info->client->dev);
}

static void mms_ts_late_resume(struct early_suspend *h)
{
	struct mms_ts_info *info;
	info = container_of(h, struct mms_ts_info, early_suspend);
	mms_ts_resume(&info->client->dev);
}
#endif

#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
static const struct dev_pm_ops mms_ts_pm_ops = {
	.suspend	= mms_ts_suspend,
	.resume		= mms_ts_resume,
}
#endif

static const struct i2c_device_id mms_ts_id[] = {
	{"mms_ts", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, mms_ts_id);


static struct i2c_driver mms_ts_driver = {
	.probe		= mms_ts_probe,
	.remove		= __devexit_p(mms_ts_remove),
	.driver		= {
				.name	= "mms_ts",
#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
				.pm	= &mms_ts_pm_ops,
#endif
	},
	.id_table	= mms_ts_id,
};

static int __init mms_ts_init(void)
{
	return i2c_add_driver(&mms_ts_driver);
}

static void __exit mms_ts_exit(void)
{
	return i2c_del_driver(&mms_ts_driver);
}

module_init(mms_ts_init);
module_exit(mms_ts_exit);

MODULE_VERSION("1.2");
MODULE_DESCRIPTION("Touchscreen driver for MELFAS MMS-100s series");
MODULE_LICENSE("GPL");

