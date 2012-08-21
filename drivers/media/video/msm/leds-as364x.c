/*
 * as364x.c -  Led flash driver
 *
 * Version:
 * 2011-01-14: v0.7 : first working version
 * 2011-01-18: v1.0 : adapt also to AS3647
 *
 * Copyright (C) 2010 Ulrich Herrmann <ulrich.herrmann@austriamicrosystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>		//samjinjang
//#include <linux/leds-as364x.h>
//#include <leds-as364x.h>

#ifdef CONFIG_AS3647
#define AS364X_CURR_STEP 6274U /* uA */
#define AS364X_TXMASK_STEP 100400 /* uA */
#define AS364X_NUM_REGS 11
#define AS364X_MAX_PEAK_CURRENT 1600
#endif

#define AS364X_MAX_ASSIST_CURRENT    \
	DIV_ROUND_UP((AS364X_CURR_STEP * 0xff * 0x7f / 0xff), 1000)
#define AS364X_MAX_INDICATOR_CURRENT \
	DIV_ROUND_UP((AS364X_CURR_STEP * 0xff * 0x3f / 0xff), 1000)

#define AS364X_REG_ChipID 0
#define AS364X_REG_Current_Set_LED1 1
#define AS364X_REG_TXMask 3
#define AS364X_REG_Low_Voltage 4
#define AS364X_REG_Flash_Timer 5
#define AS364X_REG_Control 6
#define AS364X_REG_Strobe_Signalling 7
#define AS364X_REG_Fault 8
#define AS364X_REG_PWM_and_Indicator 9
#define AS364X_REG_min_LED_Current 0xE
#define AS364X_REG_act_LED_Current 0xF

#define AS364X_WRITE_REG(a, b) i2c_smbus_write_byte_data(data->client, a, b)
#define AS364X_READ_REG(a) i2c_smbus_read_byte_data(data->client, a)

#define AS364X_LOCK()   mutex_lock(&(data)->update_lock)
#define AS364X_UNLOCK() mutex_unlock(&(data)->update_lock)


struct as364x_platform_data {
	bool use_tx_mask;
	u16 I_limit_mA; /* 2000, 2500, 3000, 3500 mA for the coil*/
	u16 txmasked_current_mA; /* 57,113,...847, roughly 56.47 mA steps */
	u16 vin_low_v_run_mV; /* 0=off, 3000, 3070, 3140, 3220, 3300, 3338,
				 3470 mV battery limit for dynamic flash
				 reduction */
	u16 vin_low_v_mV; /* 0=off, 3000, 3070, 3140, 3220, 3300, 3338, 3470 mV
			     battery limit for flash denial */
	u8 strobe_type; /* 0=edge, 1=level */
	bool freq_switch_on;
	bool led_off_when_vin_low; /* if 0 txmask current is used */
	/* LED configuration, two identical leds must be connected. */
	u16 max_peak_current_mA; /* This leds maximum peak current in mA */
	u16 max_peak_duration_ms; /* the maximum duration max_peak_current_mA
				     can be applied */
	u16 max_sustained_current_mA; /* This leds maximum sustained current
					 in mA */
	u16 min_current_mA; /* This leds minimum current in mA, desired
			       values smaller than this will be realised
			       using PWM. */
};


enum {
	AS3647_LED_OFF,
	AS3647_LED_LOW,
	AS3647_LED_HIGH,
	AS3647_LED_MAX
};

static struct as364x_data *as3647_data;

struct as364x_reg {
	const char *name;
	u8 id;
	u8 value;
};

struct as364x_data {
	struct as364x_platform_data *pdata;
	struct i2c_client *client;
	struct mutex update_lock;
	struct as364x_reg regs[AS364X_NUM_REGS];
	bool flash_mode;
	u8 flash_curr;
	u8 flash_time;
	u8 normal_curr;
	u8 normal_ctrl;
	u8 led_mask;
	u8 strobe_reg;
};

#define AS364X_ATTR(_name)  \
	__ATTR(_name, 0644, as364x_##_name##_show, as364x_##_name##_store)

#define AS364X_DEV_ATTR(_name)  \
	struct device_attribute as364x_##_name = AS364X_ATTR(_name);

static void as364x_set_leds(struct as364x_data *data, u8 ledMask,
		u8 ctrl, u8 curr);


#define AS364X_REG(NAME, VAL) \
	{.name = __stringify(NAME), .id = AS364X_REG_##NAME, .value = (VAL)}

static const struct as364x_data as364x_default_data = {
	.client = NULL,
	.regs = {

		AS364X_REG(ChipID, 0xB1),
		AS364X_REG(Current_Set_LED1, 0x9C),
		AS364X_REG(TXMask, 0x68),
		AS364X_REG(Low_Voltage, 0x2C),
/*LGE_CHANGE_S : as3647 flash reg setting
  2011-12-20, suk.kitak@lge.com, 
  adjust light intensity and timing   */
		AS364X_REG(Flash_Timer, 0xC0),	//0x23->0xC0->0x8F?
		AS364X_REG(Control, 0x00),
		AS364X_REG(Strobe_Signalling, 0x00),	//0xc0->0x00
/* LGE_CHANGE_E : as3647 flash reg setting */
		AS364X_REG(Fault, 0x00),
		AS364X_REG(PWM_and_Indicator, 0x00),
		AS364X_REG(min_LED_Current, 0x00),
		AS364X_REG(act_LED_Current, 0x00),
	},
};

#if 0 // CONFIG_PM
static int as364x_suspend(struct i2c_client *client, pm_message_t msg)
{
	dev_info(&client->dev, "Suspending AS364X\n");

	return 0;
}

static int as364x_resume(struct i2c_client *client)
{
	dev_info(&client->dev, "Resuming AS364X\n");

	return 0;
}

static void as364x_shutdown(struct i2c_client *client)
{
	struct as364x_data *data = i2c_get_clientdata(client);

	dev_info(&client->dev, "Shutting down AS364X\n");

	AS364X_LOCK();
	as364x_set_leds(data, 3, 0, 0);
	AS364X_UNLOCK();
}
#endif

static int device_add_attributes(struct device *dev,
				 struct device_attribute *attrs)
{
	int error = 0;
	int i;

	if (attrs) {
		for (i = 0; attr_name(attrs[i]); i++) {
			error = device_create_file(dev, &attrs[i]);
			if (error)
				break;
		}
		if (error)
			while (--i >= 0)
				device_remove_file(dev, &attrs[i]);
	}
	return error;
}

static void device_remove_attributes(struct device *dev,
				     struct device_attribute *attrs)
{
	int i;

	if (attrs)
		for (i = 0; attr_name(attrs[i]); i++)
			device_remove_file(dev, &attrs[i]);
}

#define MSNPRINTF(...) do { 	act = snprintf(buf, rem, __VA_ARGS__);	\
				if (act > rem)				\
					goto exit;			\
				buf += act;				\
				rem -= act;				\
			} while (0)


static void as364x_set_leds(struct as364x_data *data,
			u8 ledMask, u8 ctrl, u8 curr)
{

	if (ledMask & 1)
		AS364X_WRITE_REG(AS364X_REG_Current_Set_LED1, curr);
	else
		AS364X_WRITE_REG(AS364X_REG_Current_Set_LED1, 0);

	if (ledMask == 0 || curr == 0)
		AS364X_WRITE_REG(AS364X_REG_Control, ctrl & ~0x08);
	else
		AS364X_WRITE_REG(AS364X_REG_Control, ctrl);

}


int as3647_flash_set_led_state(int state)
{
	int rc = 0;

	switch (state) {

	case AS3647_LED_OFF:
		as364x_set_leds(as3647_data, 0,0,0);
		break;
	case AS3647_LED_LOW:
        as364x_set_leds(as3647_data, 1,0x0d,0x3F); // 0x9C->0x78->3F( 20120202 from U0 )
		break;
	case AS3647_LED_HIGH:
	    as364x_set_leds(as3647_data, 1,0x0b,0x80);	// 0x9C->0x78->3F( 20120202 from U0 )
		break;

	default:
		rc = -EFAULT;
		break;
	}

    return rc;
    
}

EXPORT_SYMBOL(as3647_flash_set_led_state);


static void as364x_set_txmask(struct as364x_data *data)
{
	u8 tm;
	u32 limit = 0, txmask;

	tm = (data->pdata->use_tx_mask ? 1 : 0);

	if (data->pdata->I_limit_mA > 2000)
		limit = (data->pdata->I_limit_mA - 2000) / 500;

	if (limit > 3)
		limit = 3;
	tm |= limit<<2;

	txmask = data->pdata->txmasked_current_mA * 1000;

	txmask /= AS364X_TXMASK_STEP;

	if (txmask > 0xf)
		txmask = 0xf;

	tm |= txmask<<4;

	AS364X_WRITE_REG(AS364X_REG_TXMask, tm);
}


static ssize_t as364x_debug_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	size_t ps = PAGE_SIZE, cw = 0;
	u8 cr;
	int i = 0;
	struct as364x_reg *reg;

	while (cw < ps && i < ARRAY_SIZE(data->regs)) {
		ps -= cw;
		buf += cw;
		cw = 0;
		reg = data->regs + i;
		if (reg->name) {
			cr = i2c_smbus_read_byte_data(data->client, reg->id);
			cw = snprintf(buf, ps, "%34s %02x: %#04x\n",
					reg->name, reg->id, cr);
		}
		i++;
	}
	ps -= cw;
	buf += cw;
	return PAGE_SIZE - ps;
}

static ssize_t as364x_debug_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	int i;
	u8 reg, val;

	i = sscanf(buf, "0x%hhx=0x%hhx", &reg, &val);
	if (i != 2)
		return -EINVAL;
	AS364X_LOCK();
	AS364X_WRITE_REG(reg, val);
	AS364X_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as364x_brightness_uA_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	size_t ps = PAGE_SIZE;
	u32 val;


	val = data->normal_curr * AS364X_CURR_STEP;
	if ((data->normal_ctrl & 0x3) == 0x01)
		val /= 16;
	return snprintf(buf, ps, "%d\n", val);
}

static ssize_t as364x_brightness_uA_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	int i;
	u32 val, curr, ctrl;
    
	i = sscanf(buf, "%d", &val);
	if (i != 1)
		return -EINVAL;

	if (val > (data->pdata->max_sustained_current_mA * 1000))
		val = data->pdata->max_sustained_current_mA * 1000;

	if (val == 0) {
		curr = 0;
		ctrl = 0;
	} else if (val > (data->pdata->min_current_mA * 1000)) {
		/* compute curr and write it */
		curr = DIV_ROUND_CLOSEST(val, AS364X_CURR_STEP);
		/* turn on assist mode */
		ctrl = 0xa;
	} else {
		/* turn on indicator mode */
		ctrl = 0x9;
		/* compute curr and write it assumes inct_pwm = 16 always*/
		if (16*val < data->pdata->min_current_mA * 1000) {
			curr = DIV_ROUND_CLOSEST(
					data->pdata->min_current_mA * 1000,
					AS364X_CURR_STEP);
		} else
			curr = DIV_ROUND_CLOSEST(16*val, AS364X_CURR_STEP);
	}
	AS364X_LOCK();
	data->normal_curr = curr;
	data->normal_ctrl = ctrl;

	if (!data->flash_mode)
		as364x_set_leds(data, data->led_mask, ctrl, curr);
	AS364X_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as364x_flash_time_ms_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	size_t ps = PAGE_SIZE;
	u32 ft = data->flash_time;

	if (ft < 0x80)
		ft = ft * 2 + 2; /* 2ms steps */
	else
		ft = 256 + (ft - 0x7f) * 8; /* above 7f 8ms steps */

	return snprintf(buf, ps, "%d\n", ft);
}

static ssize_t as364x_flash_time_ms_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	int i;
	u32 val;
	u8 reg;

	i = sscanf(buf, "%d", &val);
	if (i != 1)
		return -EINVAL;
	if (val == 0)
		return -EINVAL;

	if (val < 256)
		reg = DIV_ROUND_CLOSEST(val, 2) - 1;
	else
		reg = 0x7f + DIV_ROUND_CLOSEST(val-257, 8);

	AS364X_LOCK();
	data->flash_time = reg;

	AS364X_WRITE_REG(AS364X_REG_Flash_Timer, reg);
	AS364X_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as364x_flash_trigger_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return -EINVAL;
}

static ssize_t as364x_flash_trigger_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	int i;
	u32 val;

	i = sscanf(buf, "%d", &val);
	if (i != 1)
		return -EINVAL;
	if (val != 1)
		return -EINVAL;

	AS364X_WRITE_REG(AS364X_REG_Strobe_Signalling,
			data->strobe_reg & ~0x80);

	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as364x_flash_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	size_t ps = PAGE_SIZE;
	return snprintf(buf, ps, "%d\n", data->flash_mode);
}

static ssize_t as364x_flash_mode_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	int i;
	u32 val;

	i = sscanf(buf, "%d", &val);
	if (i != 1)
		return -EINVAL;
	if ((val != 0) && (val != 1))
		return -EINVAL;

	if (val == data->flash_mode)
		goto exit;

	AS364X_LOCK();
	data->flash_mode = val;
	AS364X_WRITE_REG(AS364X_REG_Strobe_Signalling, data->strobe_reg|0x80);
	if (val) {
		as364x_set_txmask(data);
		/* reset fault reg */
		i2c_smbus_read_byte_data(data->client, AS364X_REG_Fault);
		AS364X_WRITE_REG(AS364X_REG_Control, 0x00);
		as364x_set_leds(data, 3, 0x0b, data->flash_curr);
	} else {
		as364x_set_leds(data, data->led_mask,
				data->normal_ctrl, data->normal_curr);
	}
	AS364X_UNLOCK();


exit:
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as364x_flash_brightness_mA_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	u32 val;

	val = DIV_ROUND_CLOSEST(data->flash_curr *
				AS364X_CURR_STEP, 1000);

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t as364x_flash_brightness_mA_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	u32 val, fb;
	int i;

	i = sscanf(buf, "%u", &val);
	if (i != 1)
		return -EINVAL;

	AS364X_LOCK();
	if (val > data->pdata->max_peak_current_mA)
		val = data->pdata->max_peak_current_mA;

	fb = (val * 1000) / AS364X_CURR_STEP;
	data->flash_curr = fb;
	AS364X_UNLOCK();

	return strnlen(buf, PAGE_SIZE);
}


static ssize_t as364x_flash_last_min_current_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as364x_data *data = dev_get_drvdata(dev);
	u32 min_curr;
	u8 fault;

	if (!data->flash_mode) {
		snprintf(buf, PAGE_SIZE, "0 (not in flash_mode)\n");
		goto exit;
	}

	AS364X_LOCK();
	min_curr = i2c_smbus_read_byte_data(data->client,
			AS364X_REG_min_LED_Current);

	fault = i2c_smbus_read_byte_data(data->client,
			AS364X_REG_Fault);

	if (min_curr == 0 && !(fault&1))
		min_curr = data->flash_curr;
	min_curr *= AS364X_CURR_STEP;

	AS364X_UNLOCK();

	min_curr = DIV_ROUND_CLOSEST(min_curr, 1000);

	snprintf(buf, PAGE_SIZE, "%d\n", min_curr);
exit:
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as364x_flash_last_min_current_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return -EINVAL;
}
static struct device_attribute as364x_attributes[] =
{
	AS364X_ATTR(debug),
	AS364X_ATTR(brightness_uA),
	AS364X_ATTR(flash_time_ms),
	AS364X_ATTR(flash_trigger),
	AS364X_ATTR(flash_brightness_mA),
	AS364X_ATTR(flash_last_min_current),
	AS364X_ATTR(flash_mode),
	__ATTR_NULL
};

static const u16 v_in_low[] = {3070, 3140, 3220, 3300, 3338, 3470};
static u8 as364x_get_vin_index(u16 mV)
{
	s8 vin;
	if (mV == 0)
		return 0;
	for (vin = ARRAY_SIZE(v_in_low) - 1; vin >= 0; vin--) {
		if (mV >= v_in_low[vin])
			break;
	}
	vin += 2;

	return vin;
}

static int as364x_configure(struct i2c_client *client,
		struct as364x_data *data, struct as364x_platform_data *pdata)
{
	int err = 0;
	u8 lv, vin;

//	AS364X_WRITE_REG(1, 1);
//	AS364X_WRITE_REG(2, 2);
/*LGE_CHANGE_S : as3647 flash reg setting
  2011-12-20, suk.kitak@lge.com, 
  adjust light intensity and timing   */
//	AS364X_WRITE_REG(5, 0xC0); 
/* LGE_CHANGE_E : as3647 flash reg setting */	
#ifdef CONFIG_AS3647
	if (i2c_smbus_read_byte_data(client, 2) != 0) {
		dev_err(&client->dev, "Not AS3647, maybe AS3648, exiting\n");
		return -ENXIO;
	}
#endif

	data->pdata = pdata;

	as364x_set_txmask(data);

	vin = as364x_get_vin_index(pdata->vin_low_v_run_mV);
	lv = vin<<0;

	vin = as364x_get_vin_index(pdata->vin_low_v_mV);
	lv |= vin<<3;

	if (pdata->led_off_when_vin_low)
		lv |= 0x40;
	AS364X_WRITE_REG(AS364X_REG_Low_Voltage, lv);

	AS364X_WRITE_REG(AS364X_REG_PWM_and_Indicator,
			pdata->freq_switch_on ? 0x04 : 0);

/*LGE_CHANGE_S : as3647 flash reg setting
  2011-12-20, suk.kitak@lge.com, 
  adjust light intensity and timing   */
	data->strobe_reg = 0x00;	//data->strobe_reg = pdata->strobe_type ? 0xc0 : 0x80;
/* LGE_CHANGE_E : as3647 flash reg setting */
	
	AS364X_WRITE_REG(AS364X_REG_Strobe_Signalling, data->strobe_reg);

	if (data->pdata->max_peak_current_mA > AS364X_MAX_PEAK_CURRENT) {
		dev_warn(&client->dev,
				"max_peak_current_mA of %d higher than"
				" possible, reducing to %d\n",
				data->pdata->max_peak_current_mA,
				AS364X_MAX_PEAK_CURRENT);
		data->pdata->max_peak_current_mA = AS364X_MAX_PEAK_CURRENT;
	}
	if (data->pdata->max_sustained_current_mA > AS364X_MAX_ASSIST_CURRENT) {
		dev_warn(&client->dev,
				"max_sustained_current_mA of %d higher than"
				" possible, reducing to %d\n",
				data->pdata->max_sustained_current_mA,
				AS364X_MAX_ASSIST_CURRENT);
		data->pdata->max_sustained_current_mA =
			AS364X_MAX_ASSIST_CURRENT / 1000;
	}
	if ((1000*data->pdata->min_current_mA) < AS364X_CURR_STEP) {
		data->pdata->min_current_mA = AS364X_CURR_STEP / 1000;
		dev_warn(&client->dev,
				"min_current_mA lower than possible, icreasing"
				" to %d\n",
				data->pdata->min_current_mA);
	}
	if (data->pdata->min_current_mA > AS364X_MAX_INDICATOR_CURRENT) {
		dev_warn(&client->dev,
				"min_current_mA of %d higher than possible,"
				" reducing to %d",
				data->pdata->min_current_mA,
				AS364X_MAX_INDICATOR_CURRENT);
		data->pdata->min_current_mA =
			AS364X_MAX_INDICATOR_CURRENT / 1000;
	}

	data->led_mask = 3;
	data->normal_ctrl = 0;
	data->normal_curr = 0;

	as364x_set_leds(data, data->led_mask,
			data->normal_ctrl, data->normal_curr);

	err = device_add_attributes(&client->dev, as364x_attributes);

	if (err)
		goto exit;

	return 0;

exit:
	device_remove_attributes(&client->dev, as364x_attributes);
	return err;
}


static int as364x_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	//struct as364x_data *data;
	struct as364x_platform_data *as364x_pdata = client->dev.platform_data;
	int id1, i;
	int err = 0;

	if (!as364x_pdata)
		return -EIO;

	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	as3647_data = kzalloc(sizeof(struct as364x_data), GFP_USER);
	if (!as3647_data)
		return -ENOMEM;

	/* initialize with meaningful data (register names, etc.) */
	memcpy(as3647_data, &as364x_default_data, sizeof(struct as364x_data));

	dev_set_drvdata(&client->dev, as3647_data);

	as3647_data->client = client;
	mutex_init(&as3647_data->update_lock);


	id1 = i2c_smbus_read_byte_data(client, AS364X_REG_ChipID);

	if (id1 < 0) {
		err = id1;
		goto exit;
	}
    
	if ((id1 & 0xf8) != 0xb0) {
		err = -ENXIO;
		dev_err(&client->dev, "wrong chip detected, ids %x", id1);
		goto exit;
	}
    
	dev_info(&client->dev, "AS364X driver v1.0: detected AS364X "
			"compatible chip with id %x\n", id1);
	as3647_data->client = client;

	for (i = 0; i < ARRAY_SIZE(as3647_data->regs); i++) {
		if (as3647_data->regs[i].name)
			i2c_smbus_write_byte_data(client,
					as3647_data->regs[i].id, as3647_data->regs[i].value);
	}

	for (i = 0; i < ARRAY_SIZE(as3647_data->regs); i++) {
		if (!as3647_data->regs[i].name)
			continue;
		as3647_data->regs[i].value = i2c_smbus_read_byte_data(client, i);
	}

	i2c_set_clientdata(client, as3647_data);

	err = as364x_configure(client, as3647_data, as364x_pdata);
exit:
	if (err) {
		dev_err(&client->dev, "could not configure %x", err);
		i2c_set_clientdata(client, NULL);
	}

	return err;
}

static int as364x_remove(struct i2c_client *client)
{
	struct as364x_data *data = i2c_get_clientdata(client);
	kfree(data);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id as364x_id[] = {
	{ "as364x", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, as364x_id);

static struct i2c_driver as364x_driver = {
	.driver = {
#ifdef CONFIG_AS3647
		.name   = "as3647",
#endif
	},
	.probe  = as364x_probe,
	.remove = as364x_remove,
#if 0 // CONFIG_PM
	.shutdown = as364x_shutdown,
	.suspend  = as364x_suspend,
	.resume   = as364x_resume,
#endif
	.id_table = as364x_id,
};

static int __init as364x_init(void)
{
	return i2c_add_driver(&as364x_driver);
}

static void __exit as364x_exit(void)
{
	i2c_del_driver(&as364x_driver);
}


MODULE_AUTHOR("Ulrich Herrmann <ulrich.herrmann@austriamicrosystems.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AS364X LED flash light");

module_init(as364x_init);
module_exit(as364x_exit);
