/*
 * Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <mach/proc_comm.h>
#include "miniabb-regulator.h"

#define MV_TO_UV(mv) ((mv)*1000)
#define UV_TO_MV(uv) (((uv)+999)/1000)

/* _miniabb_vreg_switch: (vreg ID, on/off) => (return code, <null>) */
static int _miniabb_vreg_switch(int vreg_id, bool enable)
{
	unsigned _id		= (unsigned)vreg_id;
	unsigned _enable	= !!enable;
	int rc;

	rc = msm_proc_comm(PCOM_RT8966A_LDO_CONTROL, &_id, &_enable);

	if( rc < 0 )
		pr_err("### %s: ldo_control fail : vreg_id=%d, enable=%d, rc=%d ###\n", __func__, vreg_id, enable, rc);

	return rc;
}

/* _miniabb_vreg_set_level: (vreg ID, mV) => (return code, <null>) */
static int _miniabb_vreg_set_level(int vreg_id, int level_mV)
{
	unsigned _id		= (unsigned)vreg_id;
	unsigned _level		= (unsigned)level_mV;
	unsigned _level_idx = 0;
	int rc;

	/*
					0 	: 1.0V
					1 	: 1.2V
					2 	: 1.5V
					3 	: 1.8V
					4 	: 2.0V
					5 	: 2.1V
					6 	: 2.5V
					7 	: 2.6V
					8 	: 2.7V
					9 	: 2.8V
					10	: 2.9V
					11	: 3.0V
					12	: 3.1V
					13	: 3.3V
	*/
	switch( _level )
	{
		case 1000:
			_level_idx = 0;
			break;
		case 1200:
			_level_idx = 1;
			break;
		case 1500:
			_level_idx = 2;
			break;
		case 1800:
			_level_idx = 3;
			break;
		case 2000:
			_level_idx = 4;
			break;
		case 2100:
			_level_idx = 5;
			break;
		case 2500:
			_level_idx = 6;
			break;
		case 2600:
			_level_idx = 7;
			break;
		case 2700:
			_level_idx = 8;
			break;
		case 2800:
			_level_idx = 9;
			break;
		case 2900:
			_level_idx = 10;
			break;
		case 3000:
			_level_idx = 11;
			break;
		case 3100:
			_level_idx = 12;
			break;
		case 3300:
			_level_idx = 13;
			break;
		default:
			pr_err("### %s: not supported level ###\n", __func__);
			return -ENOSYS;
	}
	
	rc = msm_proc_comm(PCOM_RT8966A_LDO_SET_LEVEL, &_id, &_level_idx);

	if( rc < 0 )
		pr_err("### %s: vreg level set fail : vreg_id=%d, level=%dV, rc=%d ###\n", __func__, vreg_id, level_mV, rc);

	return rc;
}

struct miniabb_regulator_drvdata {
	struct regulator_desc	rdesc;
	int			last_voltage;
	bool			enabled;
	bool			negative;
};

static int miniabb_vreg_enable(struct regulator_dev *rdev)
{
	struct miniabb_regulator_drvdata *ddata;
	int rc;

	ddata = rdev_get_drvdata(rdev);
	rc = _miniabb_vreg_switch(rdev_get_id(rdev), VREG_SWITCH_ENABLE);

	if (rc) {
		dev_err(rdev_get_dev(rdev),
			"could not enable regulator %d (%s): %d\n",
			rdev_get_id(rdev), ddata->rdesc.name, rc);
	} else {
		dev_dbg(rdev_get_dev(rdev),
			"enabled regulator %d (%s)\n",
			rdev_get_id(rdev), ddata->rdesc.name);
		ddata->enabled = 1;
	}

	return rc;
}

static int miniabb_vreg_disable(struct regulator_dev *rdev)
{
	struct miniabb_regulator_drvdata *ddata;
	int rc;

	ddata = rdev_get_drvdata(rdev);
	rc = _miniabb_vreg_switch(rdev_get_id(rdev), VREG_SWITCH_DISABLE);

	if (rc) {
		dev_err(rdev_get_dev(rdev),
			"could not disable regulator %d (%s): %d\n",
			rdev_get_id(rdev), ddata->rdesc.name, rc);
	} else {
		dev_dbg(rdev_get_dev(rdev),
			"disabled regulator %d (%s)\n",
			rdev_get_id(rdev), ddata->rdesc.name);
		ddata->enabled = 0;
	}

	return rc;
}

static int miniabb_vreg_is_enabled(struct regulator_dev *rdev)
{
	struct miniabb_regulator_drvdata *ddata = rdev_get_drvdata(rdev);

	return ddata->enabled;
}

static int miniabb_vreg_get_voltage(struct regulator_dev *rdev)
{

	struct miniabb_regulator_drvdata *ddata = rdev_get_drvdata(rdev);

	return MV_TO_UV(ddata->last_voltage);
}

static int miniabb_vreg_set_voltage(struct regulator_dev *rdev, int min_uV, int max_uV, unsigned *sel)
{
	struct miniabb_regulator_drvdata *ddata = rdev_get_drvdata(rdev);
	int level_mV = UV_TO_MV(min_uV);
	int rc;

	rc = _miniabb_vreg_set_level(rdev_get_id(rdev),
			ddata->negative ? -level_mV : level_mV);

	if (rc) {
		dev_err(rdev_get_dev(rdev),
			"could not set voltage for regulator %d (%s) "
			"to %d mV: %d\n",
			rdev_get_id(rdev), ddata->rdesc.name, level_mV, rc);
	} else {
		dev_dbg(rdev_get_dev(rdev),
			"voltage for regulator %d (%s) set to %d mV\n",
			rdev_get_id(rdev), ddata->rdesc.name, level_mV);
		ddata->last_voltage = level_mV;
	}

	return rc;
}

static struct regulator_ops miniabb_regulator_ops = {
	.enable		= miniabb_vreg_enable,
	.disable	= miniabb_vreg_disable,
	.is_enabled	= miniabb_vreg_is_enabled,
	.get_voltage	= miniabb_vreg_get_voltage,
	.set_voltage	= miniabb_vreg_set_voltage,
};

/*
 * Create and register a struct regulator_dev based on the information in
 * a struct proccomm_regulator_info.
 * Fills in the rdev field in struct proccomm_regulator_info.
 */
static struct regulator_dev *__devinit create_miniabb_rdev(
	struct miniabb_regulator_info *info, struct device *parent)
{
	const char *name;
	struct miniabb_regulator_drvdata *d;
	struct regulator_dev *rdev;
	int rc = 0;

	if (info->id < 0) {
		dev_err(parent, "invalid regulator id %d\n", info->id);
		rc = -EINVAL;
		goto out;
	}

	name = info->init_data.constraints.name;

	if (!name) {
		dev_err(parent,
			"could not register regulator with id %d: "
			"no name specified\n", info->id);
		rc = -EINVAL;
		goto out;
	}

	d = kzalloc(sizeof(*d), GFP_KERNEL);

	if (!d) {
		dev_err(parent,
			"could not allocate struct proccomm_regulator_drvdata "
			"for regulator %d (%s)\n", info->id, name);
		rc = -ENOMEM;
		goto out;
	}

	d->rdesc.name	= name;
	d->rdesc.id	= info->id;
	d->rdesc.ops	= &miniabb_regulator_ops;
	d->rdesc.type	= REGULATOR_VOLTAGE;
	d->rdesc.owner	= THIS_MODULE;
	d->enabled	= 0;
	d->negative	= info->negative;

	rdev = regulator_register(&d->rdesc, parent, &info->init_data, d, NULL);

	if (IS_ERR(rdev)) {
		rc = PTR_ERR(rdev);
		dev_err(parent, "error registering regulator %d (%s): %d\n",
				info->id, name, rc);
		goto clean;
	}

	dev_dbg(parent, "registered regulator %d (%s)\n", info->id, name);

	return rdev;

clean:
	kfree(d);
out:
	return ERR_PTR(rc);
}

/*
 * Unregister and destroy a struct regulator_dev created by
 * create_miniabb_rdev.
 */
static void destroy_miniabb_rdev(struct regulator_dev *rdev)
{
	struct miniabb_regulator_drvdata *d;

	if (!rdev)
		return;

	d = rdev_get_drvdata(rdev);

	regulator_unregister(rdev);

	dev_dbg(rdev_get_dev(rdev)->parent,
		"unregistered regulator %d (%s)\n",
		d->rdesc.id, d->rdesc.name);

	kfree(d);
}


static int __devinit miniabb_vreg_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct miniabb_regulator_platform_data *pdata = dev->platform_data;
	struct regulator_dev **rdevs;
	int rc = 0;
	size_t i = 0;

	if (!pdata) {
		dev_err(dev, "invalid platform data\n");
		rc = -EINVAL;
		goto check_fail;
	}

	if (pdata->nregs == 0) {
		dev_err(dev, "registering an empty regulator list; "
				"this is probably not what you want\n");
		rc = -EINVAL;
		goto check_fail;
	}

	rdevs = kcalloc(pdata->nregs, sizeof(*rdevs), GFP_KERNEL);

	if (!rdevs) {
		dev_err(dev, "could not allocate storage for "
				"struct regulator_dev array\n");
		rc = -ENOMEM;
		goto check_fail;
	}

	platform_set_drvdata(pdev, rdevs);

	dev_dbg(dev, "registering %d miniabb regulators\n", pdata->nregs);

	for (i = 0; i < pdata->nregs; i++) {
		rdevs[i] = create_miniabb_rdev(&pdata->regs[i], dev);
		if (IS_ERR(rdevs[i])) {
			rc = PTR_ERR(rdevs[i]);
			goto backout;
		}
	}

	dev_dbg(dev, "%d miniabb regulators registered\n", pdata->nregs);

	return rc;

backout:
	while (--i >= 0)
		destroy_miniabb_rdev(rdevs[i]);

	kfree(rdevs);

check_fail:
	return rc;
}

static int __devexit miniabb_vreg_remove(struct platform_device *pdev)
{
	struct miniabb_regulator_platform_data *pdata;
	struct regulator_dev **rdevs;
	size_t i;

	pdata = pdev->dev.platform_data;
	rdevs = platform_get_drvdata(pdev);

	for (i = 0; i < pdata->nregs; i++)
		destroy_miniabb_rdev(rdevs[i]);

	kfree(rdevs);

	return 0;
}

static struct platform_driver miniabb_vreg_driver = {
	.probe	= miniabb_vreg_probe,
	.remove = __devexit_p(miniabb_vreg_remove),
	.driver = {
		.name	= MINIABB_REGULATOR_DEV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init miniabb_vreg_init(void)
{
	return platform_driver_register(&miniabb_vreg_driver);
}
postcore_initcall(miniabb_vreg_init);

static void __exit miniabb_vreg_exit(void)
{
	platform_driver_unregister(&miniabb_vreg_driver);
}
module_exit(miniabb_vreg_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MiniABB regulator driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:" MINIABB_REGULATOR_DEV_NAME);

