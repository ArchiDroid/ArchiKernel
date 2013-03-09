/*
 * Regulator driver for RICHTEK RT8053 PMIC chip
 *
 * Based on lp3972.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/bug.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/rt8053.h>
#include <linux/slab.h>
#include <linux/gpio.h>

struct rt8053 {
	struct device *dev;
	struct mutex io_lock;
	struct i2c_client *i2c;
	int num_regulators;
	struct regulator_dev **rdev;
	int enable_gpio;
	int refcnt;
};

/* RT8053 Control Registers */
#define RT8053_GENERAL_REG  0x00
#define RT8053_LDO1_REG     0x01
#define RT8053_LDO2_REG     0x02
#define RT8053_LDO3_REG     0x03
#define RT8053_LDO4_REG     0x04
#define RT8053_LDO5_REG     0x05
#define RT8053_BUCK1_REG    0x06
#define RT8053_BUCK2_REG    0x07
#define RT8053_ENABLE_REG   0x08
#define RT8053_PULLDOWN_REG 0x09
#define RT8053_STATUS_REG   0x0A
#define RT8053_INT_REG      0x0B
#define RT8053_INTMASK_REG  0x0C

/* LDO output enable mask */
#define RT8053_ENABLE_LDO1  BIT(0)
#define RT8053_ENABLE_LDO2  BIT(1)
#define RT8053_ENABLE_LDO3  BIT(2)
#define RT8053_ENABLE_LDO4  BIT(3)
#define RT8053_ENABLE_LDO5  BIT(4)
#define RT8053_ENABLE_BUCK1 BIT(5)

static const int ldo1235_voltage_map[] = {
	1200, 1250, 1300, 1350, 1400, 1450, 1500, 1550,
	1600, 1650, 1700, 1750, 1800, 1850, 1900, 2000,
	2100, 2200, 2300, 2400, 2500, 2600, 2650, 2700,
	2750, 2800, 2850, 2900, 2950, 3000, 3100, 3300,
};

static const int ldo4_voltage_map[] = {
	 800,  850,  900, 1000, 1100, 1200, 1250, 1300,
	1350, 1400, 1450, 1500, 1550, 1600, 1650, 1700,
	1750, 1800, 1850, 1900, 2000, 2100, 2200, 2300,
	2400, 2500, 2600, 2650, 2700, 2750, 2800, 2850,
};

static const int buck1_voltage_map[] = {
	   0,  800,  850,  900,  950, 1000, 1050, 1100,
	1150, 1200, 1250, 1300, 1350, 1400, 1450, 1500,
	1550, 1600, 1650, 1700, 1750, 1800, 1850, 1900,
	1950, 2000, 2050, 2100, 2150, 2200, 2250, 2300,
};

static const int *ldo_voltage_map[] = {
	ldo1235_voltage_map,
	ldo1235_voltage_map,
	ldo1235_voltage_map,
	ldo4_voltage_map,
	ldo1235_voltage_map,
};

static const int *buck_voltage_map[] = {
	buck1_voltage_map,
};

static const int ldo_output_enable_mask[] = {
	RT8053_ENABLE_LDO1,
	RT8053_ENABLE_LDO2,
	RT8053_ENABLE_LDO3,
	RT8053_ENABLE_LDO4,
	RT8053_ENABLE_LDO5,
};

static const int ldo_output_enable_addr[] = {
	RT8053_ENABLE_REG,
	RT8053_ENABLE_REG,
	RT8053_ENABLE_REG,
	RT8053_ENABLE_REG,
	RT8053_ENABLE_REG,
};

static const int ldo_vol_ctl_addr[] = {
	RT8053_LDO1_REG,
	RT8053_LDO2_REG,
	RT8053_LDO3_REG,
	RT8053_LDO4_REG,
	RT8053_LDO5_REG,
};

static const int buck_vol_enable_mask[] = {
	RT8053_ENABLE_BUCK1,
};

static const int buck_vol_enable_addr[] = {
	RT8053_ENABLE_REG,
};

static const int buck_vol_ctl_addr[] = {
	RT8053_BUCK1_REG,
	RT8053_BUCK2_REG,
};

#define RT8053_LDO_VOL_VALUE_MAP(x)      (ldo_voltage_map[x])
#define RT8053_LDO_OUTPUT_ENABLE_MASK(x) (ldo_output_enable_mask[x])
#define RT8053_LDO_OUTPUT_ENABLE_REG(x)  (ldo_output_enable_addr[x])
#define RT8053_LDO_VOL_CONTR_REG(x)      (ldo_vol_ctl_addr[x])

#define RT8053_LDO_VOL_MASK       (0x1F)
#define RT8053_LDO_VOL_MIN_IDX(x) (0x00)
#define RT8053_LDO_VOL_MAX_IDX(x) (0x1F)

#define RT8053_BUCK_VOL_VALUE_MAP(x)   (buck_voltage_map[x])
#define RT8053_BUCK_VOL_ENABLE_MASK(x) (buck_vol_enable_mask[x])
#define RT8053_BUCK_VOL_ENABLE_REG(x)  (buck_vol_enable_addr[x])
#define RT8053_BUCK_VOL_CONTR_REG(x)   (buck_vol_ctl_addr[x*2])

#define RT8053_BUCK_VOL_MASK       (0x1F)
#define RT8053_BUCK_VOL_MIN_IDX(x) (0x00)
#define RT8053_BUCK_VOL_MAX_IDX(x) (0x1F)

static int rt8053_i2c_read(struct i2c_client *i2c, char reg, int count,
	u16 *dest)
{
	int ret;

	if (count != 1)
		return -EIO;
	ret = i2c_smbus_read_byte_data(i2c, reg);
	if (ret < 0)
		return ret;

	*dest = ret;
	return 0;
}

static int rt8053_i2c_write(struct i2c_client *i2c, char reg, int count,
	const u16 *src)
{
	if (count != 1)
		return -EIO;
	return i2c_smbus_write_byte_data(i2c, reg, *src);
}

static u8 rt8053_reg_read(struct rt8053 *rt8053, u8 reg)
{
	u16 val = 0;

	mutex_lock(&rt8053->io_lock);

	rt8053_i2c_read(rt8053->i2c, reg, 1, &val);

	dev_dbg(rt8053->dev, "reg read 0x%02x -> 0x%02x\n", (int)reg,
		(unsigned)val & 0xff);

	mutex_unlock(&rt8053->io_lock);

	return val & 0xff;
}

static u8 rt8053_reg_write(struct rt8053 *rt8053, u8 reg, u16 val)
{
	int ret;

	mutex_lock(&rt8053->io_lock);
	ret = rt8053_i2c_write(rt8053->i2c, reg, 1, &val);

	dev_dbg(rt8053->dev, "reg write 0x%02x -> 0x%02x\n", (int)reg,
		(unsigned)val & 0xff);

	mutex_unlock(&rt8053->io_lock);

	return ret;
}

static int rt8053_set_bits(struct rt8053 *rt8053, u8 reg, u16 mask, u16 val)
{
	u16 tmp;
	int ret;

	mutex_lock(&rt8053->io_lock);

	ret = rt8053_i2c_read(rt8053->i2c, reg, 1, &tmp);
	tmp = (tmp & ~mask) | val;
	if (ret == 0) {
		ret = rt8053_i2c_write(rt8053->i2c, reg, 1, &tmp);
		dev_dbg(rt8053->dev, "reg write 0x%02x -> 0x%02x\n", (int)reg,
			(unsigned)val & 0xff);
	}
	mutex_unlock(&rt8053->io_lock);

	return ret;
}

static int rt8053_ldo_list_voltage(struct regulator_dev *dev, unsigned index)
{
	int ldo = rdev_get_id(dev) - RT8053_LDO1;

	return 1000 * RT8053_LDO_VOL_VALUE_MAP(ldo)[index];
}

static int rt8053_ldo_is_enabled(struct regulator_dev *dev)
{
	struct rt8053 *rt8053 = rdev_get_drvdata(dev);
	int ldo = rdev_get_id(dev) - RT8053_LDO1;
	u16 mask = RT8053_LDO_OUTPUT_ENABLE_MASK(ldo);
	u16 val;

	val = rt8053_reg_read(rt8053, RT8053_LDO_OUTPUT_ENABLE_REG(ldo));
	return !!(val & mask);
}

static int rt8053_ldo_enable(struct regulator_dev *dev)
{
	struct rt8053 *rt8053 = rdev_get_drvdata(dev);
	int ldo = rdev_get_id(dev) - RT8053_LDO1;
	u16 mask = RT8053_LDO_OUTPUT_ENABLE_MASK(ldo);

	if (rt8053->refcnt == 0)
		gpio_set_value(rt8053->enable_gpio, 1);

	rt8053->refcnt++;

	return rt8053_set_bits(rt8053, RT8053_LDO_OUTPUT_ENABLE_REG(ldo),
				mask, mask);
}

static int rt8053_ldo_disable(struct regulator_dev *dev)
{
	struct rt8053 *rt8053 = rdev_get_drvdata(dev);
	int ldo = rdev_get_id(dev) - RT8053_LDO1;
	u16 mask = RT8053_LDO_OUTPUT_ENABLE_MASK(ldo);

	rt8053->refcnt--;

	if (rt8053->refcnt == 0)
		gpio_set_value(rt8053->enable_gpio, 0);

	return rt8053_set_bits(rt8053, RT8053_LDO_OUTPUT_ENABLE_REG(ldo),
				mask, 0);
}

static int rt8053_ldo_get_voltage(struct regulator_dev *dev)
{
	struct rt8053 *rt8053 = rdev_get_drvdata(dev);
	int ldo = rdev_get_id(dev) - RT8053_LDO1;
	u16 mask = RT8053_LDO_VOL_MASK;
	u16 val, reg;

	reg = rt8053_reg_read(rt8053, RT8053_LDO_VOL_CONTR_REG(ldo));
	val = reg & mask;

	return 1000 * RT8053_LDO_VOL_VALUE_MAP(ldo)[val];
}

static int rt8053_ldo_set_voltage(struct regulator_dev *dev,
				  int min_uV, int max_uV,
				  unsigned int *selector)
{
	struct rt8053 *rt8053 = rdev_get_drvdata(dev);
	int ldo = rdev_get_id(dev) - RT8053_LDO1;
	int min_vol = min_uV / 1000, max_vol = max_uV / 1000;
	const int *vol_map = RT8053_LDO_VOL_VALUE_MAP(ldo);
	u16 val;
	int ret;

	if (min_vol < vol_map[RT8053_LDO_VOL_MIN_IDX(ldo)] ||
	    min_vol > vol_map[RT8053_LDO_VOL_MAX_IDX(ldo)])
		return -EINVAL;

	for (val = RT8053_LDO_VOL_MIN_IDX(ldo);
		val <= RT8053_LDO_VOL_MAX_IDX(ldo); val++)
		if (vol_map[val] >= min_vol)
			break;

	if (val > RT8053_LDO_VOL_MAX_IDX(ldo) ||
		(max_vol != 0 && vol_map[val] > max_vol))
		return -EINVAL;

	*selector = val;

	ret = rt8053_reg_write(rt8053, RT8053_LDO_VOL_CONTR_REG(ldo),
		val & RT8053_LDO_VOL_MASK);

	return ret;
}

static struct regulator_ops rt8053_ldo_ops = {
	.list_voltage = rt8053_ldo_list_voltage,
	.is_enabled = rt8053_ldo_is_enabled,
	.enable = rt8053_ldo_enable,
	.disable = rt8053_ldo_disable,
	.get_voltage = rt8053_ldo_get_voltage,
	.set_voltage = rt8053_ldo_set_voltage,
};

static int rt8053_dcdc_list_voltage(struct regulator_dev *dev, unsigned index)
{
	int buck = rdev_get_id(dev) - RT8053_DCDC1;
	return 1000 * buck_voltage_map[buck][index];
}

static int rt8053_dcdc_is_enabled(struct regulator_dev *dev)
{
	struct rt8053 *rt8053 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RT8053_DCDC1;
	u16 mask = RT8053_BUCK_VOL_ENABLE_MASK(buck);
	u16 val;

	val = rt8053_reg_read(rt8053, RT8053_BUCK_VOL_ENABLE_REG(buck));
	return !!(val & mask);
}

static int rt8053_dcdc_enable(struct regulator_dev *dev)
{
	struct rt8053 *rt8053 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RT8053_DCDC1;
	u16 mask = RT8053_BUCK_VOL_ENABLE_MASK(buck);
	u16 val;

	val = rt8053_set_bits(rt8053, RT8053_BUCK_VOL_ENABLE_REG(buck),
				mask, mask);

	if (rt8053->refcnt == 0)
		gpio_set_value(rt8053->enable_gpio, 1);

	rt8053->refcnt++;
	return val;
}

static int rt8053_dcdc_disable(struct regulator_dev *dev)
{
	struct rt8053 *rt8053 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RT8053_DCDC1;
	u16 mask = RT8053_BUCK_VOL_ENABLE_MASK(buck);
	u16 val;

	val = rt8053_set_bits(rt8053, RT8053_BUCK_VOL_ENABLE_REG(buck),
				mask, 0);

	rt8053->refcnt--;

	if (rt8053->refcnt == 0)
		gpio_set_value(rt8053->enable_gpio, 0);

	return val;
}

static int rt8053_dcdc_get_voltage(struct regulator_dev *dev)
{
	struct rt8053 *rt8053 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RT8053_DCDC1;
	u16 reg;
	int val;

	reg = rt8053_reg_read(rt8053, RT8053_BUCK_VOL_CONTR_REG(buck));
	reg &= RT8053_BUCK_VOL_MASK;
	if (reg <= RT8053_BUCK_VOL_MAX_IDX(buck))
		val = 1000 * buck_voltage_map[buck][reg];
	else {
		val = 0;
		dev_warn(&dev->dev, "chip reported incorrect voltage value."
				    " reg = %d\n", reg);
	}

	return val;
}

static int rt8053_dcdc_set_voltage(struct regulator_dev *dev,
				   int min_uV, int max_uV,
				   unsigned int *selector)
{
	struct rt8053 *rt8053 = rdev_get_drvdata(dev);
	int buck = rdev_get_id(dev) - RT8053_DCDC1;
	int min_vol = min_uV / 1000, max_vol = max_uV / 1000;
	const int *vol_map = buck_voltage_map[buck];
	u16 val;
	int ret;

	if (min_vol < vol_map[RT8053_BUCK_VOL_MIN_IDX(buck)] ||
	    min_vol > vol_map[RT8053_BUCK_VOL_MAX_IDX(buck)])
		return -EINVAL;

	for (val = RT8053_BUCK_VOL_MIN_IDX(buck);
		val <= RT8053_BUCK_VOL_MAX_IDX(buck); val++)
		if (vol_map[val] >= min_vol)
			break;

	if (val > RT8053_BUCK_VOL_MAX_IDX(buck) ||
	    (max_vol != 0 && vol_map[val] > max_vol))
		return -EINVAL;

	*selector = val;

	ret = rt8053_reg_write(rt8053, RT8053_BUCK_VOL_CONTR_REG(buck),
		val & RT8053_BUCK_VOL_MASK);
	if (ret)
		return ret;

	/* BUCK1 & BUCK2 same register setting */
	ret = rt8053_reg_write(rt8053, RT8053_BUCK_VOL_CONTR_REG(buck+1),
		val & RT8053_BUCK_VOL_MASK);

	return ret;
}

static struct regulator_ops rt8053_dcdc_ops = {
	.list_voltage = rt8053_dcdc_list_voltage,
	.is_enabled = rt8053_dcdc_is_enabled,
	.enable = rt8053_dcdc_enable,
	.disable = rt8053_dcdc_disable,
	.get_voltage = rt8053_dcdc_get_voltage,
	.set_voltage = rt8053_dcdc_set_voltage,
};

static struct regulator_desc regulators[] = {
	{
		.name = "RT8053_LDO1",
		.id = RT8053_LDO1,
		.ops = &rt8053_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo1235_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RT8053_LDO2",
		.id = RT8053_LDO2,
		.ops = &rt8053_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo1235_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RT8053_LDO3",
		.id = RT8053_LDO3,
		.ops = &rt8053_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo1235_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RT8053_LDO4",
		.id = RT8053_LDO4,
		.ops = &rt8053_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo4_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RT8053_LDO5",
		.id = RT8053_LDO5,
		.ops = &rt8053_ldo_ops,
		.n_voltages = ARRAY_SIZE(ldo1235_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "RT8053_DCDC1",
		.id = RT8053_DCDC1,
		.ops = &rt8053_dcdc_ops,
		.n_voltages = ARRAY_SIZE(buck1_voltage_map),
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
};

static int __devinit setup_regulators(struct rt8053 *rt8053,
	struct rt8053_platform_data *pdata)
{
	int i, err;

	rt8053->num_regulators = pdata->num_regulators;
	rt8053->rdev = kcalloc(pdata->num_regulators,
				sizeof(struct regulator_dev *), GFP_KERNEL);
	if (!rt8053->rdev) {
		err = -ENOMEM;
		goto err_nomem;
	}

	/* Instantiate the regulators */
	for (i = 0; i < RT8053_NUM_REGULATORS; i++) {
		struct rt8053_regulator_subdev *reg = &pdata->regulators[i];
		rt8053->rdev[i] = regulator_register(&regulators[reg->id],
					rt8053->dev, &reg->initdata, rt8053);

		if (IS_ERR(rt8053->rdev[i])) {
			err = PTR_ERR(rt8053->rdev[i]);
			dev_err(rt8053->dev, "regulator init failed: %d\n",
				err);
			goto error;
		}
	}

	return 0;
error:
	while (--i >= 0)
		regulator_unregister(rt8053->rdev[i]);
	kfree(rt8053->rdev);
	rt8053->rdev = NULL;
err_nomem:
	return err;
}

static int __devinit rt8053_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct rt8053 *rt8053;
	struct rt8053_platform_data *pdata = i2c->dev.platform_data;
	int ret;
	u16 val;

	if (!pdata) {
		dev_dbg(&i2c->dev, "No platform init data supplied\n");
		return -ENODEV;
	}

	rt8053 = kzalloc(sizeof(struct rt8053), GFP_KERNEL);
	if (!rt8053)
		return -ENOMEM;

	rt8053->i2c = i2c;
	rt8053->dev = &i2c->dev;
	rt8053->enable_gpio = pdata->enable_gpio;
	rt8053->refcnt = 0;

	mutex_init(&rt8053->io_lock);

	/* Detect RT8053 : just read testing */
	ret = rt8053_i2c_read(i2c, RT8053_GENERAL_REG, 1, &val);
	if (ret < 0) {
		dev_err(&i2c->dev, "failed to detect device. ret = %d\n", ret);
		goto err_detect;
	}

	/* Because RT8053_ENABLE_REG reset value is not 0,
	 * enable/disalbe control failed, so clear all enable bits */
	rt8053_reg_write(rt8053, RT8053_ENABLE_REG, 0x80);

	ret = gpio_request(rt8053->enable_gpio, "rt8053_enable");
	if (ret < 0) {
		dev_err(&i2c->dev, "failed to gpio_request: %d\n", ret);
		goto err_detect;
	}

	ret = gpio_direction_output(rt8053->enable_gpio, 0);
	if (ret < 0) {
		dev_err(&i2c->dev, "failed to gpio_direction_output: %d\n",
			ret);
		goto err_detect;
	}

	ret = setup_regulators(rt8053, pdata);
	if (ret < 0)
		goto err_detect;

	i2c_set_clientdata(i2c, rt8053);
	return 0;

err_detect:
	kfree(rt8053);
	return ret;
}

static int __devexit rt8053_i2c_remove(struct i2c_client *i2c)
{
	struct rt8053 *rt8053 = i2c_get_clientdata(i2c);
	int i;

	for (i = 0; i < rt8053->num_regulators; i++)
		regulator_unregister(rt8053->rdev[i]);
	kfree(rt8053->rdev);
	kfree(rt8053);

	return 0;
}

static const struct i2c_device_id rt8053_i2c_id[] = {
	{ "rt8053", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, rt8053_i2c_id);

static struct i2c_driver rt8053_i2c_driver = {
	.driver = {
		.name = "rt8053",
		.owner = THIS_MODULE,
	},
	.probe    = rt8053_i2c_probe,
	.remove   = __devexit_p(rt8053_i2c_remove),
	.id_table = rt8053_i2c_id,
};

static int __init rt8053_module_init(void)
{
	return i2c_add_driver(&rt8053_i2c_driver);
}
module_init(rt8053_module_init);

static void __exit rt8053_module_exit(void)
{
	i2c_del_driver(&rt8053_i2c_driver);
}
module_exit(rt8053_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bongkyu Kim <bongkyu.kim@lge.com>");
MODULE_DESCRIPTION("RT8053 PMIC driver");
