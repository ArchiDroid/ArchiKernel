/*
 * bh1721fvc.c
 * ROHM Ambient Light Sensor Driver
 *
 * Copyright (C) 2011 LG Electronics Inc
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <mach/board_lge.h>
#include <mach/gpio.h>

#define BH1721_I2C_DEV_ADDR	0x23
#define BH1721_PWD_OFF		0x00
#define BH1721_PWD_ON		0x01
#define BH1721_AUTO_MODE_1	0x10
#define BH1721_AUTO_MODE_2	0x20

#define H_MODE			0x12
#define L_MODE			0x13

/* power on settling time in ms */
#define BH1721_PON_DELAY	2

struct bh1721_data {
	struct i2c_client *client;
	int power_state;
	/* lock for sysfs operations */
	struct mutex lock;
	int dvi_gpio;
	int (*dvi_reset)(unsigned char);
	int(*power_on)(unsigned char);
};

struct light_ambient_platform_data *pdata = NULL;

static int bh1721_write(struct bh1721_data *ddata, u8 val, char *msg)
{
	int ret = i2c_smbus_write_byte(ddata->client, val);
	if (ret < 0)
		dev_err(&ddata->client->dev,
			"i2c_smbus_write_byte failed error %d\
			Register (%s)\n", ret, msg);
	return ret;
}

static int bh1721_read(struct bh1721_data *ddata, char *msg)
{
	int ret = i2c_smbus_read_byte(ddata->client);
	if (ret < 0)
		dev_err(&ddata->client->dev,
			"i2c_smbus_read_byte failed error %d\
			 Register (%s)\n", ret, msg);
	return ret;
}



static int bh1721_auto_resolution_mode(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct bh1721_data *ddata = platform_get_drvdata(pdev);
	unsigned char lsb, msb;
	int ret = 0, value = 0;

	ret = bh1721_write(ddata, BH1721_AUTO_MODE_1, "CONTROL");
	ret = bh1721_write(ddata, BH1721_AUTO_MODE_2, "CONTROL");
		
	/* As per bh1721 spec 16 - 180 ms */
	usleep_range(180000, 180000);  
		
	msb = bh1721_read(ddata, "DHIGH");
	if (msb < 0)
		return msb;

	lsb = bh1721_read(ddata, "DLOW");
	if (lsb < 0)
		return lsb;
		
	value = msb;
	value = ((value << 8) | lsb);

	return value;
}

static ssize_t bh1721_show_lux(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct bh1721_data *ddata = platform_get_drvdata(pdev);
	int value = 0;
	if(BH1721_PWD_ON == ddata->power_state) {
		mutex_lock(&ddata->lock);
		
		bh1721_write(ddata, BH1721_PWD_OFF, "CONTROL");
		bh1721_write(ddata, BH1721_PWD_ON, "CONTROL");
		
		value = bh1721_auto_resolution_mode(dev);
		mutex_unlock(&ddata->lock);
	}
	else
		value = 0;

	return sprintf(buf, "%d\n", value);
}

static ssize_t bh1721_show_power_state(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct bh1721_data *ddata = platform_get_drvdata(pdev);

	return sprintf(buf, "%d\n", ddata->power_state);
}

static ssize_t bh1721_store_power_state(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct bh1721_data *ddata = platform_get_drvdata(pdev);
	unsigned long val = 0;
	int error;

	error = strict_strtoul(buf, 0, &val);
	if (error)
		return error;

	if (val < BH1721_PWD_OFF || val > BH1721_PWD_ON)
		return -EINVAL;

	if ((BH1721_PWD_ON == ddata->power_state) && (BH1721_PWD_ON == val))
		return count;

	if(BH1721_PWD_ON == val) {
		ddata->power_on(1);
		usleep(2);
		ddata->dvi_reset(1);
	}
	else {
		ddata->power_on(0);
		ddata->dvi_reset(0);
	}

	ddata->power_state = val;

	return count;
}

static DEVICE_ATTR(lux, S_IRUGO, bh1721_show_lux, NULL);

static DEVICE_ATTR(power_state, S_IWUSR | S_IRUGO,
		bh1721_show_power_state, bh1721_store_power_state);

static struct attribute *bh1721_attributes[] = {
	&dev_attr_power_state.attr,
	&dev_attr_lux.attr,
	NULL,
};

static const struct attribute_group bh1721_attr_group = {
	.attrs = bh1721_attributes,
};

static int __devinit bh1721_probe(struct i2c_client *client,
						const struct i2c_device_id *id)
{
	int ret;
	struct bh1721_data *ddata = NULL;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		ret = -EIO;
		goto err_op_failed;
	}

	ddata = kzalloc(sizeof(struct bh1721_data), GFP_KERNEL);
	if (ddata == NULL) {
		ret = -ENOMEM;
		goto err_op_failed;
	}

	pdata = client->dev.platform_data;
	
	ddata->power_state = pdata->power_state;
	ddata->power_on = pdata->power_on;
	ddata->dvi_gpio = pdata->dvi_gpio;
	ddata->dvi_reset = pdata->dvi_reset_ctrl;
	ddata->client = client;
	i2c_set_clientdata(client, ddata);

	mutex_init(&ddata->lock);

	ret = gpio_tlmm_config(GPIO_CFG(ddata->dvi_gpio, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (ret) {
		pr_err("%s: gpio_tlmm_config for %d failed\n",
			__func__, ddata->dvi_gpio);
		goto err_op_failed;
	}
	
	ddata->dvi_reset(0);
	
	if (ddata->power_on) {
		ret = ddata->power_on(1);
		if (ret) {
			dev_err(&client->dev, "power on failed");
			goto err_op_failed;
		}
	}
	
	usleep(2);
	ddata->dvi_reset(1);
	ddata->power_state = BH1721_PWD_ON;

	ret = sysfs_create_group(&client->dev.kobj, &bh1721_attr_group);
	if (ret){
			goto err_op_failed;
	}
	
	return 0;

err_op_failed:
	kfree(ddata);
	return ret;
}

static int __devexit bh1721_remove(struct i2c_client *client)
{
	struct bh1721_data *ddata;

	ddata = i2c_get_clientdata(client);
	sysfs_remove_group(&client->dev.kobj, &bh1721_attr_group);
	kfree(ddata);

	return 0;
}

#ifdef CONFIG_PM
static int bh1721_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct bh1721_data *ddata = platform_get_drvdata(pdev);
	int ret;
	
	if(BH1721_PWD_OFF == ddata->power_state )
		return 0;

	ddata->power_state = BH1721_PWD_OFF;
	
	ddata->dvi_reset(0);
	
	ret = ddata->power_on(0);
	if (ret) {
		return ret;
	}

	return 0;
}

static int bh1721_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct bh1721_data *ddata = platform_get_drvdata(pdev);
	int ret;
	printk(KERN_ERR "bh1721_resume called\n");
		
	if(BH1721_PWD_ON == ddata->power_state )
		return 0;
	
	ret = ddata->power_on(1);
	if (ret) {
		return ret;
	}
	usleep(2);
	ddata->dvi_reset(1);

	return 0;
}
#else
#define bh1721_suspend NULL
#define bh1721_resume NULL
#endif /* CONFIG_PM */


#ifdef CONFIG_PM
static const struct dev_pm_ops ambient_pm_ops = {
	.suspend	= bh1721_suspend,
	.resume		= bh1721_resume,
};
#endif

static const struct i2c_device_id bh1721_id[] = {
	{ "ambient_bh1721", 0 },
	{ },
};

static struct i2c_driver bh1721_driver = {
	.probe		= bh1721_probe,
	.remove		= bh1721_remove,
	.id_table	= bh1721_id,
	.driver = {
		.name = "ambient_bh1721",
		#ifdef CONFIG_PM
		.pm = &ambient_pm_ops,
		#endif
		},
};

static int __init bh1721_init(void)
{
	return i2c_add_driver(&bh1721_driver);
}

static void __exit bh1721_exit(void)
{
	i2c_del_driver(&bh1721_driver);
}

module_init(bh1721_init)
module_exit(bh1721_exit)

MODULE_DESCRIPTION("BH1721FVC Ambient Light Sensor Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LG Electronics Inc");
