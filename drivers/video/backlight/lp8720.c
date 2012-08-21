
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/gpio.h>
#include "lp8720.h"

static unsigned char lp8720_output_status = 0x00; //default on 0x3F
struct i2c_client *lp8720_client=NULL;

void lp8720_write_reg(u8 reg, u8 data)
{
	int err;

	struct i2c_msg	msg;
    u8 buf[2];

	msg.addr = (u16)lp8720_client->addr;
	msg.flags =0;
	msg.len =2;

	buf[0]=reg;
	buf[1]=data;

	msg.buf = &buf[0];

	printk("lp8720_write_reg Address: 0x%x Data: %d\n", reg,data);
	
	if ((err = i2c_transfer(lp8720_client->adapter, &msg, 1)) < 0) {
		dev_err(&lp8720_client->dev, "i2c write error\n");
		printk("lp8720_write_reg error!!!!\n");
	}

	return;
}

void subpm_set_output(subpm_output_enum outnum, int onoff)
{
    if(outnum > 5){
        dev_err(&lp8720_client->dev, "outnum error\n");
		return;
    }

    if(onoff == 0)
	    lp8720_output_status &= ~(1<<outnum);
    else
		lp8720_output_status |= (1<<outnum);
}

void subpm_output_enable(void)
{
    if(lp8720_client == NULL)
		return;

	lp8720_write_reg(LP8720_OUTPUT_ENABLE, lp8720_output_status);
}

void subpm_gpio_output(int onoff)
{
	struct lp8720_platform_data *pdata;
	pdata = lp8720_client->dev.platform_data;

	if(onoff == 1)
		gpio_direction_output(pdata->en_gpio_num, 1);
	else {
		if (lp8720_output_status == 0)
			gpio_direction_output(pdata->en_gpio_num, 0);
	}
}

EXPORT_SYMBOL(subpm_set_output);
EXPORT_SYMBOL(subpm_output_enable);
EXPORT_SYMBOL(lp8720_write_reg);
EXPORT_SYMBOL(subpm_gpio_output);

static int lp8720_enable(unsigned gpio, int onoff)
{
	int ret = 0;
	printk(KERN_ERR " lp8720_enable 1 ");
	ret = gpio_request(gpio, "CAM_SUBPM_EN");
	if (ret) {
		pr_err("Requesting CAM_SUBPM_EN GPIO failed!\n");
		goto err;
	}
	
	gpio_tlmm_config( GPIO_CFG(gpio, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	ret = gpio_direction_output(gpio, onoff);
	if (ret) {
		pr_err("Setting CAM_SUBPM_EN GPIO direction failed!\n");
		goto err2;
	}

	printk(KERN_ERR " lp8720_enable 2 ");
err2:
	// gpio_free(gpio);
err:
	return 0;
}

static void lp8720_init(struct i2c_client *client)
{
      int ret;
      struct lp8720_platform_data *pdata;
	pdata = client->dev.platform_data;

	printk(KERN_ERR " lp8720_init 1 ");

	// ret = gpio_request(pdata->en_gpio_num, "lp8720");
	
	// gpio_direction_output(pdata->en_gpio_num, 0);
	ret = lp8720_enable(pdata->en_gpio_num, 1);

	mdelay(5);
	printk(KERN_ERR " lp8720_init 2 ");

	/*
	lp8720_write_reg(LP8720_LDO1_SETTING, LP8720_STARTUP_DELAY_3TS | 0x19); //2.8v - CAM_IOVDD_2.8V
	lp8720_write_reg(LP8720_LDO2_SETTING, LP8720_STARTUP_DELAY_3TS | 0x19); //2.8v - CAM_AVDD_2.8V
	lp8720_write_reg(LP8720_LDO3_SETTING, LP8720_STARTUP_DELAY_3TS | 0x0C); //1.8v - CAM_DVDD_1.8V
	lp8720_write_reg(LP8720_LDO4_SETTING, LP8720_STARTUP_DELAY_3TS | 0x11); //1.8v - VCAM_DVDD_1.8V
	lp8720_write_reg(LP8720_LDO5_SETTING, LP8720_STARTUP_DELAY_3TS | 0x1F); //3.3v - VREG_MOTOR_3.3V
	lp8720_write_reg(LP8720_OUTPUT_ENABLE, lp8720_output_status);
	*/
	
	lp8720_write_reg(LP8720_LDO1_SETTING, LP8720_STARTUP_DELAY_3TS | 0x0C); //2.8v - CAM_IOVDD_1.8V
	lp8720_write_reg(LP8720_LDO2_SETTING, LP8720_STARTUP_DELAY_3TS | 0x19); //2.8v - CAM_AVDD_2.8V
	lp8720_write_reg(LP8720_LDO3_SETTING, LP8720_STARTUP_DELAY_3TS | 0x0C); //1.8v - CAM_DVDD_1.8V
//	lp8720_write_reg(LP8720_LDO4_SETTING, LP8720_STARTUP_DELAY_3TS | 0x11); //1.8v - VCAM_DVDD_1.8V
	lp8720_write_reg(LP8720_LDO5_SETTING, LP8720_STARTUP_DELAY_3TS | 0x19); //3.3v - VREG_MOTOR_2.8V
	lp8720_write_reg(LP8720_OUTPUT_ENABLE, lp8720_output_status);
	
	mdelay(5);

	printk(KERN_ERR " lp8720_init 3 ");
// test
/*
	gpio_direction_output(pdata->en_gpio_num, 1);
	subpm_set_output(LDO1,1);
	subpm_set_output(LDO2,1);
	subpm_set_output(LDO3,1);
	subpm_set_output(LDO4,1);
	subpm_set_output(LDO5,1);
	subpm_output_enable();
*/
	return;
}

static int __devinit lp8720_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk(KERN_ERR " lp8720_probe 1 ");

	if (i2c_get_clientdata(client))
		return -EBUSY;

	printk(KERN_ERR " lp8720_probe 2 ");
	lp8720_client = client;
    lp8720_init(client);

    return 0;
}

static int lp8720_remove(struct i2c_client *client)
{
    struct lp8720_platform_data *pdata;
	pdata = client->dev.platform_data;

	printk(KERN_ERR "lp8720_remove");
	
       gpio_direction_output(pdata->en_gpio_num, 0);
	   
	return 0;
}

static const struct i2c_device_id lp8720_ids[] = {
	{ LP8720_I2C_NAME, 0 },	/*lp8720*/
	{ /* end of list */ },
};

static struct i2c_driver subpm_lp8720_driver = {
	.probe = lp8720_probe,
	.remove = lp8720_remove,
	.id_table	= lp8720_ids,
	.driver = {
		.name = LP8720_I2C_NAME,
		.owner = THIS_MODULE,
    },
};

void lp8720_reinit(void)
{
	printk(KERN_ERR "lp8720_remove");
	 lp8720_init(lp8720_client);

}


static int __init subpm_lp8720_init(void)
{
	printk(KERN_ERR " subpm_lp8720_init ");
	return i2c_add_driver(&subpm_lp8720_driver);
}

static void __exit subpm_lp8720_exit(void)
{
	printk(KERN_ERR "lp8720_remove");
	i2c_del_driver(&subpm_lp8720_driver);
}

module_init(subpm_lp8720_init);
module_exit(subpm_lp8720_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("LP8720 sub pmic Driver");
MODULE_LICENSE("GPL");
