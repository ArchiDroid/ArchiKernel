/*
 * Copyright (C) 2010 NXP Semiconductors
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/nfc/nfc_nxp_pn544pn65n.h>

#define MAX_BUFFER_SIZE	512
#define PN544_RESET_CMD 	0
#define PN544_DOWNLOAD_CMD	1

/* seokmin added for debugging */
#define PN544_INTERRUPT_CMD	2
#define PN544_READ_POLLING_CMD 3

#if 0
#define PN544_PROTOCOL_DATAVIEW_ENABLED
#define PN544_PROTOCOL_ANALYZE_ENABLED
#endif
#define READ_IRQ_MODIFY//DY_TEST
#ifdef READ_IRQ_MODIFY
bool do_reading = false;//DY_TEST
static bool cancle_read = false;//DY_TEST
#endif

struct pn544_dev	{
	wait_queue_head_t	read_wq;
	struct mutex		read_mutex;
	struct i2c_client	*client;
	struct miscdevice	pn544_device;
	unsigned int 		ven_gpio;
	unsigned int 		firm_gpio;
	unsigned int		irq_gpio;
	bool			irq_enabled;
	spinlock_t		irq_enabled_lock;
};

static int	stReadIntFlag;
static struct i2c_client *pn544_client;
void pn544_factory_standby_set(void)
{
    int ret;
    struct pn544_dev *pn544_dev;
    struct nxp_pn544pn65n_platform_data *platform_data;
    uint8_t EEDATA_WRITE[9] = {0x08, 0x00, 0x06, 0x00, 0x9E, 0xAA, 0x00, 0x01, 0x01};
           
    platform_data = pn544_client->dev.platform_data;
           
    pn544_dev = i2c_get_clientdata(pn544_client);
    // 1. Go To Dnld mode 2
    pr_info("%s Go To Dnld mode 2\n", __func__);
    pr_info("%s platform_data->irq_gpio[%d]\n", __func__, platform_data->irq_gpio);
    pr_info("%s platform_data->ven_gpio[%d]\n", __func__, platform_data->ven_gpio);
    pr_info("%s platform_data->firm_gpio[%d]\n", __func__, platform_data->firm_gpio);

    pr_info("%s pn544_dev->irq_gpio[%d]\n", __func__, pn544_dev->irq_gpio);
    pr_info("%s pn544_dev->ven_gpio[%d]\n", __func__, pn544_dev->ven_gpio);
    pr_info("%s pn544_dev->firm_gpio[%d]\n", __func__, pn544_dev->firm_gpio);
            
    gpio_tlmm_config(GPIO_CFG(platform_data->irq_gpio, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    msleep(10);
    gpio_set_value(platform_data->ven_gpio, 1);
    gpio_set_value(platform_data->firm_gpio, 1);
    msleep(10);
    gpio_set_value(platform_data->ven_gpio, 0);
    msleep(10);
    gpio_set_value(platform_data->ven_gpio, 1);
    msleep(10);

    // 2. I2c write
    pr_info("%s Go To I2c write\n", __func__);
    ret = 0;
    ret = i2c_master_send(pn544_client, EEDATA_WRITE, 9);
    if (ret != 9) {
         pr_err(PN544_DRV_NAME ":%s : i2c_master_send returned %d\n", __func__, ret);
    }
    msleep(10);

    // 3. HW reset 1,0,1
    pr_info("%s Go To HW reset\n", __func__);
           
    //--> # reset 1
    gpio_tlmm_config(GPIO_CFG(platform_data->irq_gpio, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    msleep(10);
    gpio_set_value(platform_data->firm_gpio, 0);
    gpio_set_value(platform_data->ven_gpio, 1);
    msleep(10);
                     
    //--> # reset 0   
    gpio_set_value(platform_data->firm_gpio, 0);
    gpio_set_value(platform_data->ven_gpio, 0);
    msleep(10);

    gpio_tlmm_config(GPIO_CFG(platform_data->irq_gpio, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    msleep(10);

    //--> # reset 1                              
    gpio_tlmm_config(GPIO_CFG(platform_data->irq_gpio, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    msleep(10);

    gpio_set_value(platform_data->firm_gpio, 0);
    gpio_set_value(platform_data->ven_gpio, 1);
    msleep(10);

           
    // 4. power off
    pr_debug(PN544_DRV_NAME ":%s power off\n", __func__);
           gpio_set_value(pn544_dev->firm_gpio, 0);
    gpio_set_value(pn544_dev->ven_gpio, 0);
    msleep(10);

    /* for HDK_8x60 Board */
    gpio_tlmm_config(GPIO_CFG(pn544_dev->irq_gpio, 0, GPIO_CFG_INPUT,
                                          GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    msleep(10);
}

static void pn544_disable_irq(struct pn544_dev *pn544_dev)
{
	unsigned long flags;

	spin_lock_irqsave(&pn544_dev->irq_enabled_lock, flags);
	if (pn544_dev->irq_enabled) {
		disable_irq_nosync(pn544_dev->client->irq);
		pn544_dev->irq_enabled = false;
	}
	spin_unlock_irqrestore(&pn544_dev->irq_enabled_lock, flags);
}

static irqreturn_t pn544_dev_irq_handler(int irq, void *dev_id)
{
	struct pn544_dev *pn544_dev = dev_id;

	pr_debug(PN544_DRV_NAME ":pn544_dev_irq_handler : %d\n", irq);

	pn544_disable_irq(pn544_dev);
#ifdef READ_IRQ_MODIFY
	do_reading=1;//DY_TEST
#endif

	/* Wake up waiting readers */
	wake_up(&pn544_dev->read_wq);

	return IRQ_HANDLED;
}

static ssize_t pn544_dev_read(struct file *filp, char __user *buf,
		size_t count, loff_t *offset)
{
	struct pn544_dev *pn544_dev = filp->private_data;
	char tmp[MAX_BUFFER_SIZE];
	int ret;
	int irq_gpio_val = 0;

#ifdef PN544_PROTOCOL_DATAVIEW_ENABLED
	int i, pos;
	char msg[32];
#ifdef PN544_PROTOCOL_ANALYZE_ENABLED
	char llc_len = 0;
	char llc_head = 0;
	char hcp_head = 0;
	char hcpm_head = 0;
#endif
#endif

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	pr_debug(PN544_DRV_NAME ":%s : reading %zu bytes.\n", __func__, count);

	mutex_lock(&pn544_dev->read_mutex);

	if (!stReadIntFlag) {
		irq_gpio_val = gpio_get_value(pn544_dev->irq_gpio);
		pr_debug(PN544_DRV_NAME ":IRQ GPIO = %d\n", irq_gpio_val);
		if (irq_gpio_val == 0) {
			if (filp->f_flags & O_NONBLOCK) {
				pr_err(PN544_DRV_NAME ":f_falg has O_NONBLOCK. EAGAIN!\n");
				ret = -EAGAIN;
				goto fail;
			}

			pn544_dev->irq_enabled = true;
#ifdef READ_IRQ_MODIFY
		do_reading=0;//DY_TEST
#endif
			enable_irq(pn544_dev->client->irq);
#ifdef READ_IRQ_MODIFY
		ret = wait_event_interruptible(pn544_dev->read_wq, do_reading);
#else
		ret = wait_event_interruptible(pn544_dev->read_wq,
				gpio_get_value(pn544_dev->irq_gpio));
#endif
			pn544_disable_irq(pn544_dev);
			pr_debug(PN544_DRV_NAME ":wait_event_interruptible : %d\n", ret);
#ifdef READ_IRQ_MODIFY
        //DY_TEST
        if(cancle_read == true)
        {
            cancle_read = false;
            ret = -1;
            goto fail;
        }
#endif

			if (ret)
				goto fail;
		}
	}

	/* Read data */
	memset(tmp, 0x00, MAX_BUFFER_SIZE);
	ret = i2c_master_recv(pn544_dev->client, tmp, count);
	mutex_unlock(&pn544_dev->read_mutex);

	if (ret < 0) {
		pr_err(PN544_DRV_NAME ":%s: i2c_master_recv returned %d\n", __func__, ret);
		return ret;
	}
	if (ret > count) {
		pr_err(PN544_DRV_NAME ":%s: received too many bytes from i2c (%d)\n",
			__func__, ret);
		return -EIO;
	}
	if (copy_to_user(buf, tmp, ret)) {
		pr_warning("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}

#ifdef PN544_PROTOCOL_DATAVIEW_ENABLED
	pr_debug(PN544_DRV_NAME ":%s : Received %d bytes successfully!\n", __func__, ret);
	pos = 0;
	memset(msg, 0x00, 32);
	for (i = 0; i < ret; i++) {
		if ((i%8 == 0) && (i > 0)) {
			pr_debug(PN544_DRV_NAME ":[R]%04d : %s\n", i-8, msg);
			pos = 0;
			memset(msg, 0x00, 32);
		}
		sprintf(&msg[pos], "%02X ", tmp[i]);
		pos += 3;
	}
	if (msg[0] != 0) {
		pr_debug(PN544_DRV_NAME ":[R]%04d : %s\n", ((i-1)/8)*8, msg);
	}
#endif
#ifdef PN544_PROTOCOL_ANALYZE_ENABLED
	pr_debug(PN544_DRV_NAME ":=============== RECEIVED PACKET ANALYSIS ===============\n");
	if (ret == 1) {
		llc_len = tmp[0];
		llc_head = 0;
		hcp_head = 0;
		hcpm_head = 0;
	} else if (ret == 3) {
		llc_len = 0;
		llc_head = tmp[0];
		hcp_head = 0;
		hcpm_head = 0;

	} else if (ret > 3) {
		llc_len = 0;
		llc_head = tmp[0];
		hcp_head = tmp[1];
		hcpm_head = tmp[2];
	}

	/* LLC Frame : Length */
	if ((llc_len > 0) && (llc_len < 0x80)) {
		pr_debug(PN544_DRV_NAME ":LLC Length : %d\n", llc_len);
	}

	/* LLC Frame : Header */
	if (llc_head != 0) {
		if (llc_head < 0xC0) {
			pr_debug(PN544_DRV_NAME ":LLC Header : I-Frame (NS = %d, NR = %d)\n",
					(llc_head&0x38)>>3, (llc_head&0x07));
		} else if (llc_head < 0x0E) {
			pr_debug(PN544_DRV_NAME ":LLC Header : S-Frame (Type = %d, NR = %d)\n",
					(llc_head&0x18)>>3, (llc_head&0x07));
		} else {
			pr_debug(PN544_DRV_NAME ":LLC Header : U-Frame (Modifier = %d)\n",
					(llc_head&0x1F));
		}
	}

	if (llc_head < 0x0E) {
		/* HCP : Header */
		if (hcp_head != 0) {
			pr_debug(PN544_DRV_NAME ":HCP Header : CB = %d, pID = %d\n",
					(hcp_head&0x80)>>7, (hcp_head&0x7F));
		}
		/* HCP : Message Header */
		if (hcpm_head != 0) {
			pr_debug(PN544_DRV_NAME ":HCPM Header : Type = %d, Ins = %d\n",
					(hcpm_head&0xC0)>>6, (hcpm_head&0x3F));
		}
	}
	pr_debug(PN544_DRV_NAME ":=============== RECEIVED PACKET ANALYSIS ===============\n");
#endif
	return ret;

fail:
	mutex_unlock(&pn544_dev->read_mutex);
	return ret;
}

static ssize_t pn544_dev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *offset)
{
	struct pn544_dev  *pn544_dev;
	char tmp[MAX_BUFFER_SIZE];
	int ret;

#ifdef PN544_PROTOCOL_DATAVIEW_ENABLED
	int i, pos;
	char msg[32];
#ifdef PN544_PROTOCOL_ANALYZE_ENABLED
	char llc_len = 0;
	char llc_head = 0;
	char hcp_head = 0;
	char hcpm_head = 0;
#endif
#endif

	pn544_dev = filp->private_data;

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	memset(tmp, 0x00, MAX_BUFFER_SIZE);
	if (copy_from_user(tmp, buf, count)) {
		pr_err(PN544_DRV_NAME ":%s : failed to copy from user space\n", __func__);
		return -EFAULT;
	}

	/* Write data */
	pr_debug(PN544_DRV_NAME ":write: pn544_write len=:%d\n", count);

	ret = i2c_master_send(pn544_dev->client, tmp, count);
	if (ret != count) {
		pr_err(PN544_DRV_NAME ":%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}

#ifdef PN544_PROTOCOL_DATAVIEW_ENABLED
	else {
		pr_debug(PN544_DRV_NAME ":%s : Send %d bytes successfully!\n", __func__, ret);
		pos = 0;
		memset(msg, 0x00, 32);
		for (i = 0; i < ret; i++) {
			if ((i%8 == 0) && (i > 0)) {
				pr_debug(PN544_DRV_NAME ":[S]%04d : %s\n", i-8, msg);
				pos = 0;
				memset(msg, 0x00, 32);
			}
			sprintf(&msg[pos], "%02X ", tmp[i]);
			pos += 3;
		}
		if (msg[0] != 0) {
			pr_debug(PN544_DRV_NAME ":[S]%04d : %s\n", ((i-1)/8)*8, msg);
		}
	}
#endif
#ifdef PN544_PROTOCOL_ANALYZE_ENABLED
	pr_debug(PN544_DRV_NAME ":=============== SEND PACKET ANALYSIS ===============\n");
	if (ret == 4) {
		llc_len = tmp[0];
		llc_head = tmp[1];
		hcp_head = 0;
		hcpm_head = 0;
	} else if (ret > 4) {
		llc_len = tmp[0];
		llc_head = tmp[1];
		hcp_head = tmp[2];
		hcpm_head = tmp[3];
	}

	/* LLC Frame : Length */
	if ((llc_len > 0) && (llc_len < 0x80)) {
		pr_debug(PN544_DRV_NAME ":LLC Length : %d\n", llc_len);
	}

	/* LLC Frame : Header */
	if (llc_head != 0) {
		if (llc_head < 0xC0) {
			pr_debug(PN544_DRV_NAME ":LLC Header : I-Frame (NS = %d, NR = %d)\n",
					(llc_head&0x38)>>3, (llc_head&0x07));
		} else if (llc_head < 0x0E) {
			pr_debug(PN544_DRV_NAME ":LLC Header : S-Frame (Type = %d, NR = %d)\n",
					(llc_head&0x18)>>3, (llc_head&0x07));
		} else {
			pr_debug(PN544_DRV_NAME ":LLC Header : U-Frame (Modifier = %d)\n",
					(llc_head&0x1F));
		}
	}

	if (llc_head < 0x0E) {
		/* HCP : Header */
		if (hcp_head != 0) {
			pr_debug(PN544_DRV_NAME ":HCP Header : CB = %d, pID = %d\n",
					(hcp_head&0x80)>>7, (hcp_head&0x7F));
		}
		/* HCP : Message Header */
		if (hcpm_head != 0) {
			pr_debug(PN544_DRV_NAME ":HCPM Header : Type = %d, Ins = %d\n",
					(hcpm_head&0xC0)>>6, (hcpm_head&0x3F));
		}
	}
	pr_debug(PN544_DRV_NAME ":=============== SEND PACKET ANALYSIS ===============\n");
#endif

	return ret;
}

static int pn544_dev_open(struct inode *inode, struct file *filp)
{
	filp->private_data = i2c_get_clientdata(pn544_client);
	pr_debug(PN544_DRV_NAME ":%s : %d,%d\n", __func__, imajor(inode), iminor(inode));

	return 0;
}

static long pn544_dev_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pn544_dev *pn544_dev = filp->private_data;

	pr_debug(PN544_DRV_NAME ":%s command %u\n", __func__, cmd);
	switch (cmd) {
	case PN544_SET_PWR:
		if (arg == 2) {
			/*
			power on with firmware download (requires hw reset)
			*/
			pr_debug(PN544_DRV_NAME ":%s power on with firmware\n", __func__);

			/* for HDK_8x60 Board */
			gpio_tlmm_config(GPIO_CFG(pn544_dev->irq_gpio, 0, GPIO_CFG_INPUT,
						GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			msleep(10);

			gpio_set_value(pn544_dev->ven_gpio, 1);
			gpio_set_value(pn544_dev->firm_gpio, 1);
			msleep(10);
			gpio_set_value(pn544_dev->ven_gpio, 0);
			msleep(10);
			gpio_set_value(pn544_dev->ven_gpio, 1);
			msleep(10);
		} else if (arg == 1) {
			/* power on */
			pr_debug(PN544_DRV_NAME ":%s power on\n", __func__);

			/* for HDK_8x60 Board */
			gpio_tlmm_config(GPIO_CFG(pn544_dev->irq_gpio, 0, GPIO_CFG_INPUT,
						GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			msleep(10);

			gpio_set_value(pn544_dev->firm_gpio, 0);
			gpio_set_value(pn544_dev->ven_gpio, 1);
			msleep(10);
		} else  if (arg == 0) {
			/* power off */
			pr_debug(PN544_DRV_NAME ":%s power off\n", __func__);
			gpio_set_value(pn544_dev->firm_gpio, 0);
			gpio_set_value(pn544_dev->ven_gpio, 0);
			msleep(10);

			/* for HDK_8x60 Board */
			gpio_tlmm_config(GPIO_CFG(pn544_dev->irq_gpio, 0, GPIO_CFG_INPUT,
						GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			msleep(10);
#ifdef READ_IRQ_MODIFY
		} else if (arg == 3) {//DY_TEST
			pr_info("%s Read Cancle\n", __func__);
            cancle_read = true;
            do_reading = 1;
        	wake_up(&pn544_dev->read_wq);
#endif
		} else {
			pr_err(PN544_DRV_NAME ":%s bad arg %ld\n", __func__, arg);
			return -EINVAL;
		}
		break;
	case PN544_INTERRUPT_CMD:
		{
			/*
			pn544_disable_irq = level;
			*/
			pr_debug(PN544_DRV_NAME ":ioctl: pn544_interrupt enable level:%ld\n", arg);
			break;
		}
	case PN544_READ_POLLING_CMD:
		{
			stReadIntFlag = arg;
			pr_debug(PN544_DRV_NAME ":ioctl: pn544_polling flag set:%ld\n", arg);
			break;
		}
	default:
		pr_err(PN544_DRV_NAME ":%s bad ioctl %d\n", __func__, cmd);
		return -EINVAL;
	}

	/* Debug Routine for GPIOs */
#if 0
	ven = gpio_get_value(pn544_dev->ven_gpio);
	firm = gpio_get_value(pn544_dev->firm_gpio);
	irq = gpio_get_value(pn544_dev->irq_gpio);
	sda = gpio_get_value(32);
	pr_debug(PN544_DRV_NAME ":GPIO Status : VEN=%d, FIRM=%d, IRQ=%d, SDA=%d\n",
			ven, firm, irq, sda);
#endif
	return 0;
}

static const struct file_operations pn544_dev_fops = {
	.owner	= THIS_MODULE,
	.llseek	= no_llseek,
	.read	= pn544_dev_read,
	.write	= pn544_dev_write,
	.open	= pn544_dev_open,
	.unlocked_ioctl = pn544_dev_unlocked_ioctl,
};

static int pn544_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret;
	struct nxp_pn544pn65n_platform_data *platform_data;
	struct pn544_dev *pn544_dev = NULL;

	pr_debug(PN544_DRV_NAME ": pn544_probe() start\n");

	pn544_client = client;
	platform_data = client->dev.platform_data;

	if (platform_data == NULL) {
		pr_err(PN544_DRV_NAME ":%s : nfc probe fail\n", __func__);
		return  -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err(PN544_DRV_NAME ":%s : need I2C_FUNC_I2C\n", __func__);
		return  -ENODEV;
	}

	ret = gpio_request(platform_data->irq_gpio, "nfc_int");
	if (ret) {
		pr_err(PN544_DRV_NAME ":pn544_probe() : nfc_int request failed!\n");
		return  -ENODEV;
	}
	ret = gpio_request(platform_data->ven_gpio, "nfc_ven");
	if (ret) {
		pr_err(PN544_DRV_NAME ":pn544_probe() : nfc_ven request failed!\n");
		goto err_ven;
	}
	ret = gpio_request(platform_data->firm_gpio, "nfc_firm");
	if (ret) {
		pr_err(PN544_DRV_NAME ":pn544_probe() : nfc_firm request failed!\n");
		goto err_firm;
	}

	pn544_dev = kzalloc(sizeof(*pn544_dev), GFP_KERNEL);
	if (pn544_dev == NULL) {
		dev_err(&client->dev,
				"failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}

	pn544_dev->irq_gpio = platform_data->irq_gpio;
	pn544_dev->ven_gpio  = platform_data->ven_gpio;
	pn544_dev->firm_gpio  = platform_data->firm_gpio;
	pn544_dev->client   = client;
	pr_debug(PN544_DRV_NAME ":IRQ : %d\nVEN : %d\nFIRM : %d\n",
			pn544_dev->irq_gpio, pn544_dev->ven_gpio, pn544_dev->firm_gpio);

	/* init mutex and queues */
	init_waitqueue_head(&pn544_dev->read_wq);
	mutex_init(&pn544_dev->read_mutex);
	spin_lock_init(&pn544_dev->irq_enabled_lock);

	pn544_dev->pn544_device.minor = MISC_DYNAMIC_MINOR;
	pn544_dev->pn544_device.name = PN544_DRV_NAME;
	pn544_dev->pn544_device.fops = &pn544_dev_fops;

	ret = misc_register(&pn544_dev->pn544_device);
	if (ret) {
		pr_err(PN544_DRV_NAME ":%s : misc_register failed\n", __FILE__);
		goto err_misc_register;
	}

	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	pr_debug(PN544_DRV_NAME ":%s : requesting IRQ %d\n", __func__, client->irq);
	pn544_dev->irq_enabled = true;
	ret = request_irq(client->irq, pn544_dev_irq_handler,
			  IRQF_TRIGGER_HIGH, client->name, pn544_dev);
	if (ret) {
		dev_err(&client->dev, "request_irq failed\n");
		goto err_request_irq_failed;
	}

	pn544_disable_irq(pn544_dev);
	i2c_set_clientdata(client, pn544_dev);
	pr_info(PN544_DRV_NAME ": pn544_probe() end\n");
	#if defined(CONFIG_LGE_NFC_PRESTANBY)
	pn544_factory_standby_set();
	#endif
	return 0;

err_request_irq_failed:
	misc_deregister(&pn544_dev->pn544_device);

err_misc_register:
	mutex_destroy(&pn544_dev->read_mutex);
	kfree(pn544_dev);

err_exit:
	gpio_free(pn544_dev->irq_gpio);

err_firm:
	gpio_free(pn544_dev->ven_gpio);

err_ven:
	gpio_free(pn544_dev->firm_gpio);

	pr_err(PN544_DRV_NAME ": pn544_probe() end with error!\n");

	return ret;
}

static int pn544_remove(struct i2c_client *client)
{
	struct pn544_dev *pn544_dev;
	
	printk("pn544_remove Start \n");

	pn544_dev = i2c_get_clientdata(client);
	free_irq(client->irq, pn544_dev);
	misc_deregister(&pn544_dev->pn544_device);
	mutex_destroy(&pn544_dev->read_mutex);
	gpio_free(pn544_dev->irq_gpio);
	gpio_free(pn544_dev->ven_gpio);
	gpio_free(pn544_dev->firm_gpio);
	kfree(pn544_dev);

	printk("pn544_remove End \n");

	return 0;
}

static const struct i2c_device_id pn544_id[] = {
	{ PN544_DRV_NAME, 0 },
	{ }
};

static struct i2c_driver pn544_driver = {
	.id_table	= pn544_id,
	.probe		= pn544_probe,
	.remove		= pn544_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= PN544_DRV_NAME,
	},
};

/*
 * module load/unload record keeping
 */

static int __init pn544_dev_init(void)
{
	pr_info(PN544_DRV_NAME ":Loading pn544 driver\n");
	return i2c_add_driver(&pn544_driver);
}
module_init(pn544_dev_init);

static void __exit pn544_dev_exit(void)
{
	pr_info(PN544_DRV_NAME ":Unloading pn544 driver\n");
	i2c_del_driver(&pn544_driver);
}
module_exit(pn544_dev_exit);

MODULE_AUTHOR("Sylvain Fonteneau");
MODULE_DESCRIPTION("NFC PN544 driver");
MODULE_LICENSE("GPL");
