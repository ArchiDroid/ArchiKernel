/* drivers/video/backlight/bu61800_bl.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include CONFIG_LGE_BOARD_HEADER_FILE

#define MODULE_NAME  "bu61800bl"
#define CONFIG_BACKLIGHT_LEDS_CLASS
#define USE_BUTTON_BACKLIGHT

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#include <linux/leds.h>
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-20] : Factory reset white screen */
//#include <linux/notifier.h> 
#include <linux/reboot.h>
/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-20] : Factory reset white screen */


/********************************************
 * Definition
 ********************************************/
#define LCD_LED_MAX 21 /* 20.32mA */
#define LCD_LED_MIN 4  /* 3.60mA */

#if 1//defined(CONFIG_MACH_MSM7X25A_E1BR)
#define KEY_BL_ON   0x3C /* y0011.1100 : Key LED 8,7,6,5 -On */
#define KEY_BL_OFF  0xC3 /* y1100.0011 : Key LED 8,7,6,5 -Off */

#define LCD_BL_ON   0x01 /* y0000.0001 : LCD Main Group LED -On */ 
#define LCD_BL_OFF  0xFE /* y1111.1110 : LCD Main Group LED -Off */
#else
#define LCD_BL_ON   1 
#define LCD_BL_OFF  0
#endif

/* BU61800 Each LDO Voltage Value */
#define LDO_1_VOLTAGE 0x0a  /* 2.8v */
#define LDO_2_VOLTAGE 0xa0	/* 2.8v */
#define LDO_3_VOLTAGE 0x00  /* 1.2v */	
#define LDO_4_VOLTAGE 0x40	/* 1.8v */

#define DEFAULT_BRIGHTNESS 13
#define BU61800_LDO_NUM 4

#define BU61800BL_REG_MAINLED     0x02  /* Register address to control Key & LCD Backlight On/Off */
#define BU61800BL_REG_CURRENT	  0x03  /* Register address to control Backlight Level */

#define BU61800BL_REG_KEY_BL_CUR_LED5  0x05  /* Register address for KEY BL LED5 Current */
#define BU61800BL_REG_KEY_BL_CUR_LED6  0x06  /* Register address for KEY BL LED6 Current */
#define BU61800BL_REG_KEY_BL_CUR_LED7  0x07  /* Register address for KEY BL LED7 Current */
#define BU61800BL_REG_KEY_BL_CUR_LED8  0x08  /* Register address for KEY BL LED8 Current */

#define BU61800BL_REG_LDO12	      0x14	/* Register address for LDO 1,2 voltage setting */
#define BU61800BL_REG_LDO34  	  0x15	/* Register address for LDO 3,4 voltage setting */
#define BU61800BL_REG_LDOEN       0x13  /* Register address for LDO Enable */

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#define LEDS_BACKLIGHT_NAME "lcd-backlight"
#ifdef USE_BUTTON_BACKLIGHT
#define BUTTON_LEDS_BACKLIGHT_NAME "button-backlight"
#endif
#endif

enum {
	ALC_MODE,
	NORMAL_MODE,
} BU61800BL_MODE;

enum {
	UNINIT_STATE=-1,
	POWERON_STATE,
	NORMAL_STATE,
	SLEEP_STATE,
	POWEROFF_STATE
} BU61800BL_STATE;

#define dprintk(fmt, args...) \
	do { \
		if (debug) \
			printk(KERN_INFO "%s:%s: " fmt, MODULE_NAME, __func__, ## args); \
	} while(0);

#define eprintk(fmt, args...)   printk(KERN_ERR "%s:%s: " fmt, MODULE_NAME, __func__, ## args)

struct ldo_vout_struct {
	unsigned char reg;
	unsigned vol;
};

struct bu61800_ctrl_tbl {
	unsigned char reg;
	unsigned char val;
};

struct bu61800_reg_addrs {
	unsigned char bl_m;        /* Register address to control Key & LCD Backlight On/Off */
	unsigned char bl_current;  /* Register address to control LCD Backlight Level */
	unsigned char bl_current_led5;
	unsigned char bl_current_led6;	
	unsigned char bl_current_led7;
	unsigned char bl_current_led8;	
	unsigned char ldo_12;	
	unsigned char ldo_34;	
	unsigned char ldo_en;
};

struct bu61800_cmds {
	struct bu61800_ctrl_tbl *normal;
	struct bu61800_ctrl_tbl *sleep;
};

struct bu61800_driver_data {
	struct i2c_client *client;
	struct backlight_device *bd;
	struct led_classdev *led;
	int gpio;
	int intensity;
	int max_intensity;
	int mode;
	int state;
	int ldo_ref[BU61800_LDO_NUM];
	unsigned char reg_ldo_enable;
	unsigned char reg_ldo_vout[2];
	int version;
	struct bu61800_cmds cmds;
	struct bu61800_reg_addrs reg_addrs;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

/********************************************
 * Global variable
 ********************************************/
static struct bu61800_driver_data *bu61800_ref;
static unsigned int debug = 0;
module_param(debug, uint, 0644);

#if 1//defined(CONFIG_MACH_MSM7X25A_E1BR)
	unsigned char led_bl_onoff_state;
#endif


/* Set to initial mode */
static struct bu61800_ctrl_tbl bu61800bl_inital_tbl[] = {
	//{ 0x00, 0x01 },  // RESET(All register Initializing)
	{ 0x01, 0x02 },  // WLED5= MainLED, WLED 5,6,7,8=off,MainLED is normal mode, WPWMIN=OFF
	{ 0x03, 0x63 },  // (initial) LED current =20mA at normal mode
#if 1//defined(CONFIG_MACH_MSM7X25A_E1BR)	
	{ 0x05, 0x18 },  // (initial) LED5 current =15mA at normal mode
	{ 0x06, 0x18 },  // (initial) LED6 current =15mA at normal mode
	{ 0x07, 0x18 },  // (initial) LED7 current =15mA at normal mode
	{ 0x08, 0x18 },  // (initial) LED8 current =15mA at normal mode	
#endif
	{ 0x09, 0x00 },  // TLH=THL=minmum setting(0.284ms)	
	{ 0x14, 0xaa },  // LDO 1CH=2.8V_2CH =2.8V
	{ 0x15, 0x40 },   // LDO 3CH=1.2V_4CH=1.8V	
	{ 0xFF, 0xFE }  //end of table	
};

/* Set to sleep mode */
static struct bu61800_ctrl_tbl bu61800bl_sleep_tbl[] = {
	{ 0x02, 0x00 },  // All led off
	{ 0xFF, 0xFE }   //end of table	
};

/********************************************
 * Functions
 ********************************************/
static int bu61800_setup_version(struct bu61800_driver_data *drvdata)
{
	if(!drvdata)
		return -ENODEV;

		drvdata->cmds.normal = bu61800bl_inital_tbl;
		drvdata->cmds.sleep = bu61800bl_sleep_tbl;
		drvdata->reg_addrs.bl_m = BU61800BL_REG_MAINLED;      /* Register address to control Key & LCD Backlight On/Off */
		drvdata->reg_addrs.bl_current= BU61800BL_REG_CURRENT; /* Register address to control Backlight Level */		
		drvdata->reg_addrs.bl_current_led5= BU61800BL_REG_KEY_BL_CUR_LED5;
		drvdata->reg_addrs.bl_current_led6= BU61800BL_REG_KEY_BL_CUR_LED6;	
		drvdata->reg_addrs.bl_current_led7= BU61800BL_REG_KEY_BL_CUR_LED7;
		drvdata->reg_addrs.bl_current_led8= BU61800BL_REG_KEY_BL_CUR_LED8;
		drvdata->reg_addrs.ldo_12 = BU61800BL_REG_LDO12;
		drvdata->reg_addrs.ldo_34 = BU61800BL_REG_LDO34;		
		drvdata->reg_addrs.ldo_en = BU61800BL_REG_LDOEN;

	return 0;
}

static int bu61800_read(struct i2c_client *client, u8 reg, u8 *pval)
{
	int ret;
	int status = 0;

	if (client == NULL) { 	/* No global client pointer? */
		eprintk("client is null\n");
		return -1;
	}

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		status = -EIO;
		eprintk("fail to read(reg=0x%x,val=0x%x)\n", reg,*pval);
	}

	*pval = ret;
	return status;
}

static int bu61800_write(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	int status = 0;

	if (client == NULL) {	/* No global client pointer? */
		eprintk("client is null\n");
		return -1;
	}

	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret != 0) {
		status = -EIO;
		eprintk("fail to write(reg=0x%x,val=0x%x)\n", reg, val);
	}

	return status;
}

static int bu61800_set_ldos(struct i2c_client *i2c_dev, unsigned num, int enable)
{
	struct bu61800_driver_data *drvdata = i2c_get_clientdata(i2c_dev);

	if (drvdata) {
		if (enable) drvdata->reg_ldo_enable |= 1 << (num-1);
		else drvdata->reg_ldo_enable &= ~(1 << (num-1));

		dprintk("enable ldos, reg:0x13 value:0x%x\n", drvdata->reg_ldo_enable);

#if 0 /* LDO Votage is already setted in initial code. if need to set again,user this code */ 
		bu61800_write(i2c_dev, drvdata->reg_addrs.ldo_12, LDO_1_VOLTAGE|LDO_2_VOLTAGE);
		bu61800_write(i2c_dev, drvdata->reg_addrs.ldo_34, LDO_3_VOLTAGE|LDO_4_VOLTAGE);
#endif		
		return bu61800_write(i2c_dev, drvdata->reg_addrs.ldo_en, drvdata->reg_ldo_enable);
	}
	return -EIO;
}

int bu61800_ldo_enable(struct device *dev, unsigned num, unsigned enable)
{
	struct i2c_client *client;
	struct bu61800_driver_data *drvdata;
	int err = 0;

	if(bu61800_ref == NULL)
		return -ENODEV;
	
	drvdata = bu61800_ref;
	client = bu61800_ref->client;
	
	dprintk("ldo_no[%d], on/off[%d]\n",num, enable);

	if (num > 0 && num <= BU61800_LDO_NUM) {
		if(client) {
			if (enable) {
				if (drvdata->ldo_ref[num-1]++ == 0) {
					dprintk("ref count = 0, call bu61800_set_ldos\n");
					err = bu61800_set_ldos(client, num, enable);
				}
			}
			else {
				if (--drvdata->ldo_ref[num-1] == 0) {
					dprintk("ref count = 0, call bu61800_set_ldos\n");
					err = bu61800_set_ldos(client, num, enable);
				}
			}
			return err;
		}
	}
	return -ENODEV;
}
EXPORT_SYMBOL(bu61800_ldo_enable);

static int bu61800_set_table(struct bu61800_driver_data *drvdata, struct bu61800_ctrl_tbl *ptbl)
{
	unsigned int i = 0;
	unsigned long delay = 0;

	if (ptbl == NULL) {
		eprintk("input ptr is null\n");
		return -EIO;
	}

	for( ;;) 
	{
		if (ptbl->reg == 0xFF) {
			if (ptbl->val != 0xFE) {
				delay = (unsigned long)ptbl->val;
				udelay(delay);
			}
			else
			break;
		}
		else {
		   if (bu61800_write(drvdata->client, ptbl->reg, ptbl->val) != 0)
			 dprintk("i2c failed addr:%d, value:%d\n", ptbl->reg, ptbl->val);
			}
		ptbl++;
		i++;
	}
	return 0;
}

#if 0 //not user in bu61800
static void bu61800_hw_reset(struct bu61800_driver_data *drvdata)
{
	if (drvdata->client && gpio_is_valid(drvdata->gpio)) {
		gpio_tlmm_config(GPIO_CFG(drvdata->gpio, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		/* EN set to LOW(shutdown) -> HIGH(enable) */
		gpio_set_value(drvdata->gpio, 0);
		udelay(5);
		gpio_set_value(drvdata->gpio, 1);
		udelay(5);
	}
}
#endif
static void bu61800_go_opmode(struct bu61800_driver_data *drvdata)
{
	//dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");
	switch (drvdata->mode) {
		case NORMAL_MODE:
			bu61800_set_table(drvdata, drvdata->cmds.normal);
			drvdata->state = NORMAL_STATE;
			break;
		case ALC_MODE:	//not use
			break;
		default:
			eprintk("Invalid Mode\n");
			break;
	}
}

static void bu61800_device_init(struct bu61800_driver_data *drvdata)
{
	bu61800_go_opmode(drvdata);
}

#ifdef CONFIG_PM
#if 0
static void bu61800_poweron(struct bu61800_driver_data *drvdata)
{
	//unsigned int bu61800_intensity;
	if (!drvdata || drvdata->state != POWEROFF_STATE)
		return;

	dprintk("POWER ON \n");

	bu61800_device_init(drvdata);
	//bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
}
#endif
#endif /* CONFIG_PM */

/* This function provide sleep enter routine for power management. */
#ifdef CONFIG_PM
static void bu61800_sleep(struct bu61800_driver_data *drvdata)
{
	if (!drvdata || drvdata->state == SLEEP_STATE)
		return;

	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");

	switch (drvdata->mode) {
		case NORMAL_MODE:
			drvdata->state = SLEEP_STATE;
			bu61800_set_table(drvdata, drvdata->cmds.sleep); //All led off
			break;

		case ALC_MODE: //not use
			break;
		default:
			eprintk("Invalid Mode\n");
			break;
	}
}

static void bu61800_wakeup(struct bu61800_driver_data *drvdata)
{
	if (!drvdata || drvdata->state == NORMAL_STATE)
		return;

	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");

	if (drvdata->state == POWEROFF_STATE) {
		//bu61800_poweron(drvdata);
	} else if (drvdata->state == SLEEP_STATE) {
		if (drvdata->mode == NORMAL_MODE) 
			  {
				//bu61800_set_table(drvdata, drvdata->cmds.normal);
				//bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
			    drvdata->state = NORMAL_STATE;
		} else if (drvdata->mode == ALC_MODE) {
          //nothing
		}
	}
}
#endif /* CONFIG_PM */


/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
#if 1
extern int display_on; 
int Is_Backlight_Set = 0;
#endif
/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */

#if 1//defined(CONFIG_MACH_MSM7X25A_E1BR)

static int bu61800_send_intensity(struct bu61800_driver_data *drvdata, int level)
{
	int current_value=0;
	unsigned char val=led_bl_onoff_state;

	if (level > drvdata->max_intensity)
		level = drvdata->max_intensity;

	if (level != 0 && level < LCD_LED_MIN)
		level = LCD_LED_MIN;

	if( level>3 && level < 22)
		{
		  current_value = 0x63*level/LCD_LED_MAX;
		  dprintk("Setting level= %d,current_value is 0x%x\n",level, current_value);
		}
	else
		{
	      current_value = 0x63;			
		  dprintk("Invalid setting level = %d\n", level);
		}
	/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
#if 1
	if((display_on==0) && (level!=0)) {
		   drvdata->intensity = level;
		led_bl_onoff_state |= 0x01;
	      return 0;
	   	}
#endif		
	/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */

	if ((drvdata->intensity != level)&& (level != 0))
	{
		val = val | LCD_BL_ON;
		dprintk("BACK LIGHT SETTING....\n");
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current, current_value);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, val);
		Is_Backlight_Set = 1; /* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
	}
	else if(level == 0)
	{
		val = val & LCD_BL_OFF;
		dprintk("BACK LIGHT OFF\n");		
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current, current_value);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, val);
		Is_Backlight_Set = 0; /* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
	}
	else
	{
	    Is_Backlight_Set = 1; /* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
	}

	led_bl_onoff_state = val;
	drvdata->intensity = level;

	return 0;
}
#else //CONFIG_MACH_MSM7X25A_E1BR

static int bu61800_send_intensity(struct bu61800_driver_data *drvdata, int level)
{
	int current_value=0;

	if (level > drvdata->max_intensity)
		level = drvdata->max_intensity;

	if (level != 0 && level < LCD_LED_MIN)
		level = LCD_LED_MIN;

	if( level>3 && level < 22)
		{
		  current_value = 0x63*level/LCD_LED_MAX;
		  dprintk("Setting level= %d,current_value is 0x%x\n",level, current_value);
		}
	else
		{
	      current_value = 0x63;			
		  dprintk("Invalid setting level = %d\n", level);
		}
	/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */
#if 1
	   if((display_on==0) && (level!=0)) 
	   	{
		   drvdata->intensity = level;
	      return 0;
	   	}
#endif		
	/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */


	/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-05] : bl dimming error */
	if ((drvdata->intensity != level)&& (level != 0))
	{
		dprintk("BACK LIGHT SETTING....\n");
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current, current_value);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, LCD_BL_ON);
		Is_Backlight_Set = 1; /* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */

	}
	else if(level == 0)
	{
		dprintk("BACK LIGHT OFF\n");		
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current, current_value);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, LCD_BL_OFF);
		Is_Backlight_Set = 0; /* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */

	}
	else
	{
	      Is_Backlight_Set = 1; /* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */

	}
	/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-05] : bl dimming error */

	drvdata->intensity = level;	
	return 0;
}
#endif //CONFIG_MACH_MSM7X25A_E1BR


#ifdef USE_BUTTON_BACKLIGHT
int bu61800_send_intensity_button(struct bu61800_driver_data *drvdata, int level)
{
	u8 val = led_bl_onoff_state;

	if(level)
	{
		val |= KEY_BL_ON;
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current_led5, 0x18);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current_led6, 0x18);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current_led7, 0x18);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current_led8, 0x18);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, val);
	}
	else
	{
		val &= KEY_BL_OFF;
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, val);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current_led5, 0);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current_led6, 0);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current_led7, 0);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current_led8, 0);
	}

    led_bl_onoff_state = val;

	return 0;
}
#endif

static int bu61800_get_intensity(struct bu61800_driver_data *drvdata)
{
	return drvdata->intensity;
}


#if 1//defined(CONFIG_MACH_MSM7X25A_E1BR)

int bu61800_force_set(void)
{
	struct bu61800_driver_data *drvdata = bu61800_ref;
	int brightness;
	int current_value;
	unsigned char val=led_bl_onoff_state;

	brightness = bu61800_get_intensity(drvdata);
	current_value = 0x63*brightness/LCD_LED_MAX;

	dprintk("[bu61800_force_set] brightness= %d,current_value is 0x%x\n",brightness, current_value);

	if (brightness != 0)
	{
		val = val | LCD_BL_ON;
		dprintk("BACK LIGHT AFTER SETTING....\n");
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current, current_value);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, val);
		Is_Backlight_Set = 1; 
		
	}
	else
	{
		val = val & LCD_BL_OFF;
		dprintk("BACK LIGHT  AFTER OFF\n");		
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current, current_value);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, val);
		Is_Backlight_Set = 0; 
	}

	led_bl_onoff_state = val;

	return 0;
}
EXPORT_SYMBOL(bu61800_force_set);

#else

/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */

int bu61800_force_set(void)
{
	struct bu61800_driver_data *drvdata = bu61800_ref;
	int brightness;
	int current_value;

	brightness = bu61800_get_intensity(drvdata);
	current_value = 0x63*brightness/LCD_LED_MAX;

	dprintk("[bu61800_force_set] brightness= %d,current_value is 0x%x\n",brightness, current_value);

	if (brightness != 0)
	{
		dprintk("BACK LIGHT AFTER SETTING....\n");
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current, current_value);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, LCD_BL_ON);
		Is_Backlight_Set = 1; 
		
	}
	else
	{
		dprintk("BACK LIGHT  AFTER OFF\n");		
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_current, current_value);
		bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, LCD_BL_OFF);
		Is_Backlight_Set = 0; 
	}

	return 0;
	
}

EXPORT_SYMBOL(bu61800_force_set);
#endif
/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-22] : BL control error fix */


#ifdef CONFIG_PM
#ifdef CONFIG_HAS_EARLYSUSPEND
static void bu61800_early_suspend(struct early_suspend * h)
{
	struct bu61800_driver_data *drvdata = container_of(h, struct bu61800_driver_data,
						    early_suspend);

	dprintk("start\n");
	bu61800_sleep(drvdata);

	return;
}

static void bu61800_late_resume(struct early_suspend * h)
{
	struct bu61800_driver_data *drvdata = container_of(h, struct bu61800_driver_data,
						    early_suspend);

	dprintk("start\n");
	bu61800_wakeup(drvdata);

	return;
}
#else
static int bu61800_suspend(struct i2c_client *i2c_dev, pm_message_t state)
{
	struct bu61800_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	bu61800_sleep(drvdata);
	return 0;
}

static int bu61800_resume(struct i2c_client *i2c_dev)
{
	struct bu61800_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	bu61800_wakeup(drvdata);
	return 0;
}
#endif	/* CONFIG_HAS_EARLYSUSPEND */
#else
#define bu61800_suspend NULL
#define bu61800_resume	NULL
#endif	/* CONFIG_PM */

void bu61800_switch_mode(struct device *dev, int next_mode)
{
	struct bu61800_driver_data *drvdata = dev_get_drvdata(dev);

	if (!drvdata || drvdata->mode == next_mode)
		return;

	if (next_mode == ALC_MODE) {
      //not use
	}
	else if (next_mode == NORMAL_MODE) {
		//bu61800_set_table(drvdata, drvdata->cmds.alc);
		//bu61800_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
	} else {
		printk(KERN_ERR "%s: invalid mode(%d)!!!\n", __func__, next_mode);
		return;
	}

	drvdata->mode = next_mode;
	return;
}

ssize_t bu61800_show_alc(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct bu61800_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int r;

	if (!drvdata) return 0;

	r = snprintf(buf, PAGE_SIZE, "%s\n", (drvdata->mode == ALC_MODE) ? "1":"0");

	return r;
}

ssize_t bu61800_store_alc(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int alc;
	int next_mode;

	if (!count)
		return -EINVAL;

	sscanf(buf, "%d", &alc);

	if (alc)
		next_mode = ALC_MODE;
	else
		next_mode = NORMAL_MODE;

	bu61800_switch_mode(dev->parent, next_mode);

	return count;
}

ssize_t bu61800_show_reg(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct bu61800_driver_data *drvdata = dev_get_drvdata(dev);
	int len = 0;
	unsigned char val;

	len += snprintf(buf, PAGE_SIZE, "\nBU61800 Registers is following..\n");
	bu61800_read(drvdata->client, 0x00, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[CH_EN(0x00)] = 0x%x\n", val);
	bu61800_read(drvdata->client, 0x01, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[BLM(0x01)] = 0x%x\n", val);
	bu61800_read(drvdata->client, 0x0E, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[ALS(0x0E)] = 0x%x\n", val);
	bu61800_read(drvdata->client, 0x0F, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[SBIAS(0x0F)] = 0x%x\n", val);
	bu61800_read(drvdata->client, 0x10, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[ALS_GAIN(0x10)] = 0x%x\n", val);
	bu61800_read(drvdata->client, 0x11, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[AMBIENT_LEVEL(0x11)] = 0x%x\n", val);

	return len;
}

ssize_t bu61800_show_drvstat(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct bu61800_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int len = 0;

	len += snprintf(buf,  PAGE_SIZE,     "\nBu61800 Backlight Driver Status is following..\n");
	len += snprintf(buf + len, PAGE_SIZE - len, "mode                   = %3d\n", drvdata->mode);
	len += snprintf(buf + len, PAGE_SIZE - len, "state                  = %3d\n", drvdata->state);
	len += snprintf(buf + len, PAGE_SIZE - len, "current intensity      = %3d\n", drvdata->intensity);

	return len;
}

DEVICE_ATTR(alc, 0664, bu61800_show_alc, bu61800_store_alc);
DEVICE_ATTR(reg, 0444, bu61800_show_reg, NULL);
DEVICE_ATTR(drvstat, 0444, bu61800_show_drvstat, NULL);

static int bu61800_set_brightness(struct backlight_device *bd)
{
	struct bu61800_driver_data *drvdata = dev_get_drvdata(bd->dev.parent);
	return bu61800_send_intensity(drvdata, bd->props.brightness);
}

static int bu61800_get_brightness(struct backlight_device *bd)
{
	struct bu61800_driver_data *drvdata = dev_get_drvdata(bd->dev.parent);
	return bu61800_get_intensity(drvdata);
}

static struct backlight_ops bu61800_ops = {
	.get_brightness = bu61800_get_brightness,
	.update_status  = bu61800_set_brightness,
};


#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
static void leds_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct bu61800_driver_data *drvdata = dev_get_drvdata(led_cdev->dev->parent);
	int brightness;
	int next;

	if (!drvdata) {
		eprintk("Error getting drvier data\n");
		return;
	}

	brightness = bu61800_get_intensity(drvdata);

	dprintk("input brightness org =%d, value = %d\n", brightness,value);

	next = value * drvdata->max_intensity / LED_FULL;
	/* If value is not 0, should not backlight off by bongkyu.kim */
	if (value !=0 && next == 0)
		next = 1;

	if (brightness != next) {
		dprintk("brightness[current=%d, next=%d]\n", brightness, next);
		bu61800_send_intensity(drvdata, next);
	}
}

static struct led_classdev bu61800_led_dev = {
	.name = LEDS_BACKLIGHT_NAME,
	.brightness_set = leds_brightness_set,
};

/* dajin.kim@lge.com */
#ifdef USE_BUTTON_BACKLIGHT
static void button_leds_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct bu61800_driver_data *drvdata = dev_get_drvdata(led_cdev->dev->parent);

	if (!drvdata) {
		eprintk("Error getting drvier data\n");
		return;
	}
	dprintk("input brightness value = %d\n", value);
	bu61800_send_intensity_button(drvdata, value);
}

static struct led_classdev bu61800_keyled_dev = {
	.name			= BUTTON_LEDS_BACKLIGHT_NAME,
	.brightness_set = button_leds_brightness_set,
	.brightness		= LED_OFF,
};
#endif
/* dajin.kim@lge.com */
#endif

static int bu61800_probe(struct i2c_client *i2c_dev, const struct i2c_device_id *i2c_dev_id)
{
	struct lge_backlight_platform_data *pdata;
	struct bu61800_driver_data *drvdata;
	struct backlight_device *bd;
	int err;

	dprintk("start, client addr=0x%x\n", i2c_dev->addr);

	pdata = i2c_dev->dev.platform_data;
	if(!pdata)
		return -EINVAL;

	drvdata = kzalloc(sizeof(struct bu61800_driver_data), GFP_KERNEL);
	if (!drvdata) {
		dev_err(&i2c_dev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	if (pdata && pdata->platform_init)
		pdata->platform_init();

	drvdata->client = i2c_dev;
	drvdata->gpio = pdata->gpio;
	drvdata->max_intensity = LCD_LED_MAX;
	if (pdata->max_current > 0)
		drvdata->max_intensity = pdata->max_current;
	drvdata->intensity = LCD_LED_MIN;
	drvdata->mode = NORMAL_MODE;
	drvdata->state = UNINIT_STATE;
	drvdata->version = pdata->version;

	if(bu61800_setup_version(drvdata) != 0) {
		eprintk("Error while requesting gpio %d\n", drvdata->gpio);
		kfree(drvdata);
		return -ENODEV;
	}

#if 0 //not use enable pin in bu61800	
	if (drvdata->gpio && gpio_request(drvdata->gpio, "bu61800_en") != 0) {
		eprintk("Error while requesting gpio %d\n", drvdata->gpio);
		kfree(drvdata);
		return -ENODEV;
	}
#endif

	bd = backlight_device_register("bu61800-bl", &i2c_dev->dev, NULL, &bu61800_ops, NULL);
	if (bd == NULL) {
		eprintk("entering bu61800 probe function error \n");
		//if (gpio_is_valid(drvdata->gpio))
		//	gpio_free(drvdata->gpio);
		kfree(drvdata);
		return -1;
	}
	bd->props.power = FB_BLANK_UNBLANK;
	bd->props.brightness = drvdata->intensity;
	bd->props.max_brightness = drvdata->max_intensity;
	drvdata->bd = bd;

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
	if (led_classdev_register(&i2c_dev->dev, &bu61800_led_dev) == 0) {
		eprintk("Registering led class dev successfully.\n");
		drvdata->led = &bu61800_led_dev;
		err = device_create_file(drvdata->led->dev, &dev_attr_alc);
		err = device_create_file(drvdata->led->dev, &dev_attr_reg);
		err = device_create_file(drvdata->led->dev, &dev_attr_drvstat);
	}
#ifdef USE_BUTTON_BACKLIGHT
	/* dajin.kim@lge.com */
	if (led_classdev_register(&i2c_dev->dev, &bu61800_keyled_dev) == 0) {
		eprintk("Registering led class dev successfully.\n");
		drvdata->led = &bu61800_keyled_dev;
	}
	/* dajin.kim@lge.com */
#endif
#endif

	i2c_set_clientdata(i2c_dev, drvdata);
	i2c_set_adapdata(i2c_dev->adapter, i2c_dev);

	bu61800_device_init(drvdata);
	bu61800_send_intensity(drvdata, DEFAULT_BRIGHTNESS);

#ifdef CONFIG_HAS_EARLYSUSPEND
	drvdata->early_suspend.suspend = bu61800_early_suspend;
	drvdata->early_suspend.resume = bu61800_late_resume;
	drvdata->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 40;
	register_early_suspend(&drvdata->early_suspend);
#endif

	bu61800_ref = drvdata;

	eprintk("done\n");
	return 0;
}

static int __devexit bu61800_remove(struct i2c_client *i2c_dev)
{
	struct bu61800_driver_data *drvdata = i2c_get_clientdata(i2c_dev);

	bu61800_send_intensity(drvdata, 0);

	backlight_device_unregister(drvdata->bd);
	led_classdev_unregister(drvdata->led);
	i2c_set_clientdata(i2c_dev, NULL);
	//if (gpio_is_valid(drvdata->gpio))
	//	gpio_free(drvdata->gpio);
	kfree(drvdata);

	return 0;
}

static struct i2c_device_id bu61800_idtable[] = {
	{ MODULE_NAME, 0 },
};

MODULE_DEVICE_TABLE(i2c, bu61800_idtable);

static struct i2c_driver bu61800_driver = {
	.probe 		= bu61800_probe,
	.remove 	= bu61800_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend 	= bu61800_suspend,
	.resume 	= bu61800_resume,
#endif
	.id_table 	= bu61800_idtable,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
	},
};


/* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-20] : Factory reset white screen */
static int bu61800_send_off(struct notifier_block *this,
				unsigned long event, void *cmd)
{
	
	if ((event == SYS_RESTART) || (event == SYS_POWER_OFF))
	    	bu61800_send_intensity(bu61800_ref, 0);
	
	return NOTIFY_DONE;
}

struct notifier_block lge_chg_reboot_nb = {
	.notifier_call = bu61800_send_off, 
};

extern int register_reboot_notifier(struct notifier_block *nb);
/* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-20] : Factory reset white screen */



static int __init bu61800_init(void)
{
	printk("BU61800 init start\n");

 /* LGE_CHANGE_S: E0 jiwon.seo@lge.com [2011-11-20] : Factory reset white screen */
       register_reboot_notifier(&lge_chg_reboot_nb);
 /* LGE_CHANGE_E: E0 jiwon.seo@lge.com [2011-11-20] : Factory reset white screen */
 
	return i2c_add_driver(&bu61800_driver);
}

static void __exit bu61800_exit(void)
{
	i2c_del_driver(&bu61800_driver);
}

module_init(bu61800_init);
module_exit(bu61800_exit);

MODULE_DESCRIPTION("Backlight driver for ROHM BU61800");
MODULE_AUTHOR("Jiwon Seo <jiwon.seo@lge.com>");
MODULE_LICENSE("GPL");
