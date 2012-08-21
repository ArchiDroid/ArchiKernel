#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/slab.h>

#define MAX17040_VCELL_MSB	0x02
#define MAX17040_VCELL_LSB	0x03
#define MAX17040_SOC_MSB	0x04
#define MAX17040_SOC_LSB	0x05
#define MAX17040_MODE_MSB	0x06
#define MAX17040_MODE_LSB	0x07
#define MAX17040_VER_MSB	0x08
#define MAX17040_VER_LSB	0x09
#define MAX17040_RCOMP_MSB	0x0C
#define MAX17040_RCOMP_LSB	0x0D
#define MAX17040_CMD_MSB	0xFE
#define MAX17040_CMD_LSB	0xFF

#define MAX17040_DELAY		1000
#define MAX17040_BATTERY_FULL	95

struct maxim17040_chip {
	struct i2c_client		*client;
	struct delayed_work		work;
	struct power_supply		battery;
	struct maxim17040_platform_data	*pdata;

	/* State Of Connect */
	int online;
	/* battery voltage */
	int vcell;
	/* battery capacity */
	int soc;
	/* State Of Charge */
	int status;
};

static int maxim17040_write_reg(struct i2c_client *client, int reg, u8 value)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, value);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static int maxim_i2c_read(struct i2c_client *client, void *buf,
	u8 addr, size_t size)
{
	int ret;
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.len = sizeof(addr),
			.buf = &addr,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = size,
			.buf = buf,
		},
	};

	ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));

	return ret == ARRAY_SIZE(msg) ? size : -EIO;
}
 

static int maxim17040_read_reg(struct i2c_client *client, int reg)
{
	int ret;
	unsigned char data;

	ret = i2c_smbus_read_byte_data(client, reg);

	if (ret < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);
		return ret;
	}

	ret = maxim_i2c_read(client, &data, reg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);
		return ret;
	}

	return data;
}

static void maxim17040_reset(struct i2c_client *client)
{
	maxim17040_write_reg(client, MAX17040_CMD_MSB, 0x54);
	maxim17040_write_reg(client, MAX17040_CMD_LSB, 0x00);
}

static void maxim17040_get_vcell(struct i2c_client *client)
{
	struct maxim17040_chip *chip = i2c_get_clientdata(client);
	u8 msb;
	u8 lsb;

	msb = maxim17040_read_reg(client, MAX17040_VCELL_MSB);
	lsb = maxim17040_read_reg(client, MAX17040_VCELL_LSB);

	chip->vcell = (msb << 4) + (lsb >> 4);

	printk("MAX17040 Fuel-Gauge vcell = %d\n", chip->vcell);
}

static void maxim17040_get_soc(struct i2c_client *client)
{
	struct maxim17040_chip *chip = i2c_get_clientdata(client);
	u8 msb;
	u8 lsb;

	msb = maxim17040_read_reg(client, MAX17040_SOC_MSB);
	lsb = maxim17040_read_reg(client, MAX17040_SOC_LSB);

	chip->soc = msb;

	printk("MAX17040 Fuel-Gauge Soc = %d\n", chip->soc);
}

static void maxim17040_get_version(struct i2c_client *client)
{
	u8 msb;
	u8 lsb;

	msb = maxim17040_read_reg(client, MAX17040_VER_MSB);
	lsb = maxim17040_read_reg(client, MAX17040_VER_LSB);

	printk("MAX17040 Fuel-Gauge Ver %d%d\n", msb, lsb);
}

static void maxim17040_work(struct work_struct *work)
{
	struct maxim17040_chip *chip;

	chip = container_of(work, struct maxim17040_chip, work.work);

	maxim17040_get_vcell(chip->client);
	maxim17040_get_soc(chip->client);

	schedule_delayed_work(&chip->work, MAX17040_DELAY);
}

static int __devinit maxim17040_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct maxim17040_chip *chip;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;
	chip->pdata = client->dev.platform_data;

	i2c_set_clientdata(client, chip);

	maxim17040_reset(client);
	maxim17040_get_version(client);

	INIT_DELAYED_WORK_DEFERRABLE(&chip->work, maxim17040_work);
	schedule_delayed_work(&chip->work, MAX17040_DELAY);

	return 0;
}

static int __devexit maxim17040_remove(struct i2c_client *client)
{
	struct maxim17040_chip *chip = i2c_get_clientdata(client);

	power_supply_unregister(&chip->battery);
	cancel_delayed_work(&chip->work);
	kfree(chip);
	return 0;
}

#ifdef CONFIG_PM

static int maxim17040_suspend(struct i2c_client *client,
		pm_message_t state)
{
	struct maxim17040_chip *chip = i2c_get_clientdata(client);

	cancel_delayed_work(&chip->work);
	return 0;
}

static int maxim17040_resume(struct i2c_client *client)
{
	struct maxim17040_chip *chip = i2c_get_clientdata(client);

	schedule_delayed_work(&chip->work, MAX17040_DELAY);
	return 0;
}

#else

#define maxim17040_suspend NULL
#define maxim17040_resume NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id maxim17040_id[] = {
	{ "maxim17040_hdk", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, maxim17040_id);

static struct i2c_driver maxim17040_i2c_driver = {
	.driver	= {
		.name	= "maxim17040_hdk",
	},
	.probe		= maxim17040_probe,
	.remove		= __devexit_p(maxim17040_remove),
	.suspend	= maxim17040_suspend,
	.resume		= maxim17040_resume,
	.id_table	= maxim17040_id,
};

static int __init maxim17040_init(void)
{
	return i2c_add_driver(&maxim17040_i2c_driver);
}
module_init(maxim17040_init);

static void __exit maxim17040_exit(void)
{
	i2c_del_driver(&maxim17040_i2c_driver);
}
module_exit(maxim17040_exit);

