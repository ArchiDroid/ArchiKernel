#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/fb.h>
#include <linux/earlysuspend.h>
#include <linux/leds.h>
#include <linux/i2c.h>
#include <asm/gpio.h>

#include CONFIG_LGE_BOARD_HEADER_FILE

#define LM3528_I2C_NAME		"lm3528bl"
#define LM3528_I2C_ADDR		0x36

struct lm3528_private_data {
	unsigned char	reg_gp;
	unsigned char	reg_bmain;
	unsigned char	reg_bsub;
	unsigned char	reg_hpg;
	unsigned char	reg_gpio;
	unsigned char	reg_pgen0;
	unsigned char	reg_pgen1;
	unsigned char	reg_pgen2;
	unsigned char	reg_pgen3;

	struct i2c_client*	client;
	struct mutex	update_lock;
};

struct lm3528_platform_data {
	int		gpio_hwen;
	struct lm3528_private_data	private;
};

#define	LM3528_REG_GP		0x10
#define	LM3528_REG_BMAIN	0xa0
#define	LM3528_REG_BSUB		0xb0
#define	LM3528_REG_HPG		0x80
#define	LM3528_REG_GPIO		0x81
#define	LM3528_REG_PGEN0	0x90
#define	LM3528_REG_PGEN1	0x91
#define	LM3528_REG_PGEN2	0x92
#define	LM3528_REG_PGEN3	0x93

#define	LM3528_BMASK		0x7f	// Brightness Mask

#define LCD_CP_EN				124
#define MAX_BRIGHTNESS		0xFF	// MAX current, about ??? cd/m2
#define DEFAULT_BRIGHTNESS	0x7F	// about 220 cd/m2 

struct lm3528_device {
	struct i2c_client *client;
	struct backlight_device *bl_dev;
	struct led_classdev *led;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

static struct lm3528_device *main_lm3528_dev = NULL;
static struct i2c_client 	*lm3528_i2c_client;

unsigned int cur_main_lcd_level = DEFAULT_BRIGHTNESS;

#ifdef CONFIG_HAS_EARLYSUSPEND
static int early_bl_timer = 1;	//when screen off: 0, screen on: 1
static int early_bl_value = 0;
#endif

/* SYSFS for brightness control
 */

u8 backlight_keyled_flag=1;

static int	lm3528_read_byte(struct lm3528_private_data* pdata, int reg)
{
	int		ret;

	mutex_lock(&pdata->update_lock);
	ret	=	i2c_smbus_read_byte_data(pdata->client, reg);
	mutex_unlock(&pdata->update_lock);

	return	ret;
}

static int	lm3528_write_byte(struct lm3528_private_data* pdata, int reg, int value)
{
	int		ret;

	mutex_lock(&pdata->update_lock);
	ret	=	i2c_smbus_write_byte_data(pdata->client, reg, value);
	mutex_unlock(&pdata->update_lock);

	return	ret;
}

static void	lm3528_store(struct lm3528_private_data* pdata)
{
	lm3528_write_byte(pdata, LM3528_REG_GP, pdata->reg_gp);
	lm3528_write_byte(pdata, LM3528_REG_BMAIN, pdata->reg_bmain);
	lm3528_write_byte(pdata, LM3528_REG_BSUB, pdata->reg_bsub);
	lm3528_write_byte(pdata, LM3528_REG_HPG, pdata->reg_hpg);
	lm3528_write_byte(pdata, LM3528_REG_GPIO, pdata->reg_gpio);
	lm3528_write_byte(pdata, LM3528_REG_PGEN0, pdata->reg_pgen0);
	lm3528_write_byte(pdata, LM3528_REG_PGEN1, pdata->reg_pgen1);
	lm3528_write_byte(pdata, LM3528_REG_PGEN2, pdata->reg_pgen2);
	lm3528_write_byte(pdata, LM3528_REG_PGEN3, pdata->reg_pgen3);
}

static void	lm3528_load(struct lm3528_private_data* pdata)
{
	pdata->reg_gp		=	lm3528_read_byte(pdata, LM3528_REG_GP);
	pdata->reg_bmain	=	lm3528_read_byte(pdata, LM3528_REG_BMAIN);
	pdata->reg_bsub	=	lm3528_read_byte(pdata, LM3528_REG_BSUB);
	pdata->reg_hpg		=	lm3528_read_byte(pdata, LM3528_REG_HPG);
	pdata->reg_gpio		=	lm3528_read_byte(pdata, LM3528_REG_GPIO);
	pdata->reg_pgen0	=	lm3528_read_byte(pdata, LM3528_REG_PGEN0);
	pdata->reg_pgen1	=	lm3528_read_byte(pdata, LM3528_REG_PGEN1);
	pdata->reg_pgen2	=	lm3528_read_byte(pdata, LM3528_REG_PGEN2);
	pdata->reg_pgen3	=	lm3528_read_byte(pdata, LM3528_REG_PGEN3);
}


int	lm3528_set_hwen(struct lm3528_private_data* pdata, int gpio, int status)
{
	if (status == 0) {
		lm3528_load(pdata);
		gpio_set_value(gpio, 0);
		return	0;
	}

	gpio_set_value(gpio, 1);
	lm3528_store(pdata);

	return 1;
}

int	lm3528_get_hwen(struct lm3528_private_data* pdata, int gpio)
{
	return	gpio_get_value(gpio);
}

int	lm3528_set_bmain(struct lm3528_private_data* pdata, int val)
{
	if ((val < 0) || (val > 127))
		return	-EINVAL;

	printk("[%s] lm3528_set_bmain =%d",__func__,val);
	
	return	lm3528_write_byte(pdata, LM3528_REG_BMAIN, val);
}

int	lm3528_get_bmain(struct lm3528_private_data* pdata)
{
	int		val;

	mutex_lock(&pdata->update_lock);
	val	=	lm3528_read_byte(pdata, LM3528_REG_BMAIN);
	mutex_unlock(&pdata->update_lock);

	if (val < 0)
		return	val;

	return	(val & LM3528_BMASK);
}

void lm3528_init(struct lm3528_private_data* pdata, struct i2c_client* client)
{
	mutex_init(&pdata->update_lock);
	pdata->client	=	client;

	lm3528_load(pdata);
}


static void lm3528_set_main_current_level(struct i2c_client *client, int level);
static ssize_t lcd_backlight_store_on_off(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int on_off;
	int ret;
	struct lm3528_platform_data*	pdata;
	struct i2c_client *client = to_i2c_client(dev);

	if (!count)
		return -EINVAL;

	printk("%s\n", __func__);

	on_off = simple_strtoul(buf, NULL, 10);
	sscanf(buf, "%d", &ret);
	pdata = client->dev.platform_data;

	printk(KERN_ERR "%d\n",on_off);

	if(ret == 0){
		lm3528_set_bmain(&pdata->private, 0);
	}else {
	    lm3528_set_bmain(&pdata->private, DEFAULT_BRIGHTNESS);
	}

	return count;

}

DEVICE_ATTR(backlight_on_off, 0666, NULL, lcd_backlight_store_on_off);

/* Driver
 */

static void lm3528_set_main_current_level(struct i2c_client *client, int level)
{
	struct lm3528_device *dev;
	struct lm3528_platform_data*	pdata;
	//unsigned char muic_mode; 

	dev = (struct lm3528_device *)i2c_get_clientdata(client);
	cur_main_lcd_level = level;
	dev->bl_dev->props.brightness = cur_main_lcd_level;

	#if 0
	muic_mode = get_muic_mode();
	if((muic_mode == 9 || muic_mode == 7) && (check_battery_present() == 0))
	{
		printk(KERN_INFO"CP_USB/UART & No-Battery -> Backlight level = 0x28(40)\n");
		level = 0x28; //2.892mA
	}
	#endif

	pdata = client->dev.platform_data;
	lm3528_set_bmain(&pdata->private, level);

	if(level==0)	backlight_keyled_flag=0;
	else			backlight_keyled_flag=1;
}

static void leds_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct lm3528_platform_data*	pdata;
	u8 level;
	pdata = lm3528_i2c_client->dev.platform_data;

	// value : 30 ~ 255
	// lm3528 brightness : 0 ~ 127
	//2011-03-11, Max:UI 255(driver 127) / Min:UI 30(driver 70)
	if(value >= 30)
		level = ((value - 30) * 57) / 225 + 70;
	else
		level = value*70/30;

	//printk("[lm3528]leds_brightness_set : %d, %d\n, ", value, level);
	if(level > MAX_BRIGHTNESS)
		level = MAX_BRIGHTNESS;
	else if (level < 0)
		level = 0;

	if(early_bl_timer == 0) {
		early_bl_value = level;
		return;
	}

	//lm3528_set_bmain(&pdata->private, value);
	lm3528_set_main_current_level(lm3528_i2c_client, level);
	cur_main_lcd_level = level;

	return;
}

static struct led_classdev lcd_backlight = {
	.name = "lcd-backlight",
	.brightness = MAX_BRIGHTNESS,
	.brightness_set = leds_brightness_set,
};

static int lm3528bl_set_brightness(struct backlight_device *bd)
{
	struct lm3528_platform_data*	pdata;

	printk("[lm3528]%s:%d\n",__func__,bd->props.brightness);

	pdata = lm3528_i2c_client->dev.platform_data;
	//lm3528_set_bmain(&pdata->private, bd->props.brightness);
	lm3528_set_main_current_level(lm3528_i2c_client, bd->props.brightness);
	cur_main_lcd_level = bd->props.brightness;

	return 0;
}

static int lm3528bl_get_brightness(struct backlight_device *bd)
{
	struct lm3528_platform_data*	pdata;
	unsigned char val=0;

	pdata = lm3528_i2c_client->dev.platform_data;
	val = lm3528_get_bmain(&pdata->private);

	printk("[lm3528]%s:%d\n",__func__,val);

	return (int)val;
}

static struct backlight_ops lm3528bl_ops = {
	.update_status	= lm3528bl_set_brightness,
	.get_brightness	= lm3528bl_get_brightness,
};

static int lm3528_suspend(struct i2c_client *client, pm_message_t state)
{
	struct lm3528_platform_data*	pdata;
	printk("[lm3528]%s\n",__func__);

	pdata = client->dev.platform_data;
	client->dev.power.power_state = state;
	//lm3528_set_hwen(client->dev.platform_data, LCD_CP_EN, 0);
	lm3528_set_bmain(&pdata->private, 0);

	return 0;
}

static int lm3528_resume(struct i2c_client *client)
{

	struct lm3528_platform_data*	pdata;
	printk("[lm3528]%s\n",__func__);

	pdata = client->dev.platform_data;
	client->dev.power.power_state = PMSG_ON;
	//lm3528_set_hwen(client->dev.platform_data, LCD_CP_EN, 1);
	lm3528_set_bmain(&pdata->private, cur_main_lcd_level);
//	doing_wakeup = 0;
	return 0;
}

int lcd_backlight_status = 1;
int lcd_status_backup = 1;
int lcd_cp_en_status = 1;

void lm3258_power_switch(int val)
{
	if(lcd_cp_en_status == val)
		return;

	if(val==0)
	{
		gpio_direction_output(124, 0);		// LCD_CP_EN
		lcd_cp_en_status = 0;
		printk("[lcd & touch power] OFF!\n");
	}
	else
	{
		gpio_direction_output(124, 1);		// LCD_CP_EN
		lcd_cp_en_status = 1;
		printk("[lcd & touch power] ON!\n");
		mdelay(50);
	}
	return ;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static int lm3528bl_suspend(struct i2c_client *client)
{
	struct lm3528_platform_data*	pdata;
	pdata = client->dev.platform_data;

	printk("[lm3528]%s\n",__func__);


	lm3528_set_main_current_level(client, 0);

	early_bl_timer = 0;
	
	if(lcd_status_backup == 0)
	{
		lm3258_power_switch(0);
	}
	lcd_backlight_status = 0;


	return 0;
}

static int lm3528bl_resume(struct i2c_client *client)
{
	struct lm3528_platform_data*	pdata;
	pdata = client->dev.platform_data;

	mdelay(50);//NATTING_TEST : Justin LCD resume time °³¼±...

	printk("[lm3528]%s\n",__func__);
	
	lm3258_power_switch(1);
	lm3528_set_hwen(&pdata->private, LCD_CP_EN, 1);

	lcd_backlight_status = 1;

	early_bl_timer = 1;

	lm3528_set_main_current_level(client, early_bl_value);

	return 0;
}

static void lm3528_early_suspend(struct early_suspend *h)
{
	struct lm3528_device *dev;
	dev = container_of(h, struct lm3528_device, early_suspend);
	lm3528bl_suspend(dev->client);
}

static void lm3528_late_resume(struct early_suspend *h)
{
	struct lm3528_device *dev;
	dev = container_of(h, struct lm3528_device, early_suspend);
	lm3528bl_resume(dev->client);
}
#endif//CONFIG_HAS_EARLYSUSPEND

static int __init lm3528bl_probe(struct i2c_client* client,
							const struct i2c_device_id* id)
{
	struct backlight_device *bl_dev;
	struct lm3528_platform_data*	pdata;
	struct lm3528_device	*dev;

	int	error;

	lm3528_i2c_client = client;

	dev = kzalloc(sizeof(struct lm3528_device), GFP_KERNEL);

	if (dev == NULL) {
		dev_err(&client->dev, "fail alloc for lm3528_device\n");
		return 0;
	}

	main_lm3528_dev = dev;

	printk("%s :  start\n",__func__);

	bl_dev = backlight_device_register(LM3528_I2C_NAME, &client->dev, NULL, &lm3528bl_ops,NULL);
	bl_dev->props.max_brightness = MAX_BRIGHTNESS;
	bl_dev->props.brightness = DEFAULT_BRIGHTNESS;
	bl_dev->props.power = FB_BLANK_UNBLANK;

	dev->bl_dev = bl_dev;
	dev->client = client;
	i2c_set_clientdata(client, dev);

	pdata	=	client->dev.platform_data;
	//gpio_request(pdata->gpio_hwen, "backlight_enable");
	//gpio_direction_output(pdata->gpio_hwen, 1);	// OUTPUT
	lm3528_init(&pdata->private, client);

	led_classdev_register(&client->dev, &lcd_backlight);	//UI control

	lm3528_set_bmain(&pdata->private, DEFAULT_BRIGHTNESS);

	//device_create_file(&client->dev, &dev_attr_enable);
	//device_create_file(&client->dev, &dev_attr_brightness);

	error = device_create_file(&client->dev, &dev_attr_backlight_on_off);

#ifdef CONFIG_HAS_EARLYSUSPEND
	dev->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	dev->early_suspend.suspend = lm3528_early_suspend;
	dev->early_suspend.resume = lm3528_late_resume;
	register_early_suspend(&dev->early_suspend);
#endif

	printk("%s :   end\n",__func__);

	return	0;
}

static int	lm3528bl_remove(struct i2c_client* client)
{
	return	0;
}

static struct i2c_device_id lm3528bl_ids[] = {
	{ LM3528_I2C_NAME, 0 },
};


MODULE_DEVICE_TABLE(i2c, lm3528bl_ids);

static struct i2c_driver lm3528_driver __refdata = {
	.probe 		= lm3528bl_probe,
	.remove 	= lm3528bl_remove,
#ifdef CONFIG_HAS_EARLYSUSPEND
	.suspend 	= lm3528_suspend,
	.resume 	= lm3528_resume,
#endif
	.id_table 	= lm3528bl_ids,
	.driver = {
		.name = LM3528_I2C_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init lm3528bl_init(void)
{
	printk("lm3528_init init start\n");
	return i2c_add_driver(&lm3528_driver);
}

static void __exit lm3528bl_exit(void)
{
	i2c_del_driver(&lm3528_driver);
}

module_init(lm3528bl_init);
module_exit(lm3528bl_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("Backlight driver (LM3528)");
MODULE_LICENSE("GPL");
