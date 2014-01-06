/*
 *  stk_i2c_ps31xx_int.c - Linux kernel modules for proximity/ambient light sensor
 *  (Intrrupt Mode)
 *
 *  Copyright (C) 2011 Patrick Chang / SenseTek <patrick_chang@sitronix.com.tw>
 *  Copyright(C) 2011-2012 Foxconn International Holdings, Ltd. All rights reserved
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/stk_i2c_ps31xx.h>

#include "stk_defines.h"
#include "stk_lk_defs.h"
#ifndef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
#include "stk_ps31xx_lux_threshold_table.h"
#endif // CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD

#define ADDITIONAL_GPIO_CFG 1

/* // Additional GPIO CFG Header */
#if ADDITIONAL_GPIO_CFG
#include <linux/gpio.h>
//#define EINT_GPIO 122
// Use irq_to_gpio() if it is possible
// #include <plat/gpio.h>
// #define EINT_GPIO (irq_to_gpio(client->irq))

#endif

#define STKALS_DRV_NAME	"stk_als"
#define STKPS_DRV_NAME "stk_ps"
#define DEVICE_NAME		"stk-oss"
#define DRIVER_VERSION  STK_DRIVER_VER
#define LightSensorDevName "stk_als"
#define ProximitySensorDevName "stk_ps"

#define DEFAULT_ALS_DELAY 20
#define ALS_ODR_DELAY (1000/10)
#define PS_ODR_DELAY (1000/50)

static struct stkps31xx_data* pStkPsData = NULL;
static struct platform_device *stk_oss_dev = NULL; /* Device structure */

#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
static int32_t als_transmittance = CONFIG_STK_ALS_TRANSMITTANCE;
#endif //CONFIG_STK_ALS_TRANSMITTANCE_TUNING

#ifdef CONFIG_STK_PS_ENGINEER_TUNING
static uint8_t ps_code_low_thd;
static uint8_t ps_code_high_thd;
#endif

inline uint32_t alscode2lux(uint32_t alscode)
{
   alscode += ((alscode<<7)+(alscode<<3)+(alscode>>1));     // 137.5
      //x1       //   x128         x8            x0.5
    alscode<<=3; // x 8 (software extend to 19 bits)
    // Gain & IT setting ==> x8
    // ==> i.e. code x 8800
    // Org : 1 code = 0.88 Lux
    // 8800 code = 0.88 lux --> this means it must be * 1/10000

#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
    alscode/=als_transmittance;
#else
    alscode/=CONFIG_STK_ALS_TRANSMITTANCE;
#endif //CONFIG_STK_ALS_TRANSMITTANCE_TUNING
    return alscode;
}

inline uint32_t lux2alscode(uint32_t lux)
{

#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
    lux*=als_transmittance;
#else
    lux*=CONFIG_STK_ALS_TRANSMITTANCE;
#endif //CONFIG_STK_ALS_TRANSMITTANCE_TUNING
    lux/=1100;
    if (unlikely(lux>=(1<<16)))
        lux = (1<<16) -1;
    return lux;

}

#ifndef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
static void init_code_threshold_table(void)
{
    uint32_t i,j;
    uint32_t alscode;

    code_threshold_table[0] = 0;
    PSENSOR_DEBUG(LEVEL0,"alscode[0]=%d\n",code_threshold_table[0]);
    for (i=1,j=0;i<LUX_THD_TABLE_SIZE;i++,j++)
    {
        alscode = lux2alscode(lux_threshold_table[j]);
        PSENSOR_DEBUG(LEVEL0,"alscode[%d]=%d\n",i,alscode);
        code_threshold_table[i] = (uint16_t)(alscode);
    }
    code_threshold_table[i] = 0xffff;
    PSENSOR_DEBUG(LEVEL0,"alscode[%d]=%d\n",i,alscode);
}

static uint32_t get_lux_interval_index(uint16_t alscode)
{
    uint32_t i;
    for (i=1;i<=LUX_THD_TABLE_SIZE;i++)
    {
        if ((alscode>=code_threshold_table[i-1])&&(alscode<code_threshold_table[i]))
        {
            return i;
        }
    }
    return LUX_THD_TABLE_SIZE;
}
#else
inline void set_als_new_thd_by_reading(uint16_t alscode)
{
    int32_t high_thd,low_thd;
    high_thd = alscode + lux2alscode(CONFIG_STK_ALS_CHANGE_THRESHOLD);
    low_thd = alscode - lux2alscode(CONFIG_STK_ALS_CHANGE_THRESHOLD);
    if (high_thd >= (1<<16))
        high_thd = (1<<16) -1;
    if (low_thd <0)
        low_thd = 0;
    set_als_thd_h(stk3171->client,(uint16_t)high_thd);
    set_als_thd_l(stk3171->client,(uint16_t)low_thd);
}

#endif // CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD

static int32_t software_reset(struct i2c_client *client)
{
    // software reset and check stk 83xx is valid
    //struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    int32_t r;
    uint8_t w_reg;
    uint8_t org_reg;
    r = i2c_smbus_read_byte_data(client,STK_PS_STATUS_REG);
    if (r<0)
    {
        PSENSOR_DEBUG(LEVEL0,"read i2c error\n");
        return r;
    }
    if ((r&STK_PS_STATUS_ID_MASK)!=STK_PS31xx_ID)
    {
        PSENSOR_DEBUG(LEVEL0,"invalid ID number");
        return -EINVAL;
    }
    r = i2c_smbus_read_byte_data(client,STK_PS_GC_REG);
    if (r<0)
    {
        PSENSOR_DEBUG(LEVEL0,"read i2c error\n");
        return r;
    }
    org_reg = (uint8_t)(r&0xf0);
    w_reg = ~((uint8_t)(r&0xff));
    r = i2c_smbus_write_byte_data(client,STK_PS_GC_REG,w_reg);
    if (r<0)
    {
        PSENSOR_DEBUG(LEVEL0,"write i2c error\n");
        return r;
    }
    r = i2c_smbus_read_byte_data(client,STK_PS_GC_REG);
    if (w_reg!=(uint8_t)(r&0xff))
    {
        PSENSOR_DEBUG(LEVEL0,"read-back value is not  the same\n");
        return -1;
    }
    r = i2c_smbus_write_byte_data(client,STK_PS_SOFTWARE_RESET_REG,0);
    msleep(5);
    if (r<0)
    {
        PSENSOR_DEBUG(LEVEL0,"read error after reset\n");
        return r;
    }
    return 0;
}

static int32_t enable_ps_int(struct i2c_client *client,uint8_t enable)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    stk3171->ps_cmd_reg &= (~STK_PS_CMD_INT_MASK);
    stk3171->ps_cmd_reg |= STK_PS_CMD_INT(enable);
    return i2c_smbus_write_byte_data(stk3171->client,STK_PS_CMD_REG,stk3171->ps_cmd_reg);
}
static int32_t enable_als_int(struct i2c_client *client,uint8_t enable)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    stk3171->als_cmd_reg &= (~STK_ALS_CMD_INT_MASK);
    stk3171->als_cmd_reg |= STK_ALS_CMD_INT(enable);
    return i2c_smbus_write_byte_data(stk3171->client,STK_ALS_CMD_REG,stk3171->als_cmd_reg);
}


static int32_t set_als_it(struct i2c_client *client,uint8_t it)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    stk3171->als_cmd_reg &= (~STK_ALS_CMD_IT_MASK);
    stk3171->als_cmd_reg |= (STK_ALS_CMD_IT_MASK & STK_ALS_CMD_IT(it));
    return i2c_smbus_write_byte_data(stk3171->client,STK_ALS_CMD_REG,stk3171->als_cmd_reg);

}
static int32_t set_als_gain(struct i2c_client *client,uint8_t gain)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    if(gain >= 2)
    {
        PSENSOR_DEBUG(LEVEL0,"als_gain >= 2\n");
    }
    stk3171->als_cmd_reg &= (~STK_ALS_CMD_GAIN_MASK);
    stk3171->als_cmd_reg |= (STK_ALS_CMD_GAIN_MASK & STK_ALS_CMD_GAIN(gain));
    return i2c_smbus_write_byte_data(stk3171->client,STK_ALS_CMD_REG,stk3171->als_cmd_reg);
}
static int32_t set_ps_it(struct i2c_client *client,uint8_t it)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    stk3171->ps_cmd_reg &= (~STK_PS_CMD_IT_MASK);
    stk3171->ps_cmd_reg |= (STK_PS_CMD_IT_MASK & STK_PS_CMD_IT(it));
    return i2c_smbus_write_byte_data(stk3171->client,STK_PS_CMD_REG,stk3171->ps_cmd_reg);
}
static int32_t set_ps_slp(struct i2c_client *client,uint8_t slp)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    stk3171->ps_cmd_reg &= (~STK_PS_CMD_SLP_MASK);
    stk3171->ps_cmd_reg |= (STK_PS_CMD_SLP_MASK & STK_PS_CMD_SLP(slp));
    return i2c_smbus_write_byte_data(stk3171->client,STK_PS_CMD_REG,stk3171->ps_cmd_reg);

}
static int32_t set_ps_led_driving_current(struct i2c_client *client,uint8_t irdr)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    stk3171->ps_cmd_reg &= (~STK_PS_CMD_DR_MASK);
    stk3171->ps_cmd_reg |= (STK_PS_CMD_DR_MASK & STK_PS_CMD_DR(irdr));
    return i2c_smbus_write_byte_data(stk3171->client,STK_PS_CMD_REG,stk3171->ps_cmd_reg);
}
static int32_t set_ps_gc(struct i2c_client *client,uint8_t gc)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    int32_t retval;

    retval = i2c_smbus_read_byte_data(stk3171->client,STK_PS_GC_REG);
    if (retval<0)
        return retval;
    stk3171->ps_gc_reg = (uint8_t)retval;
    stk3171->ps_gc_reg &= (~STK_PS_GC_GAIN_MASK);
    stk3171->ps_gc_reg |= (STK_PS_GC_GAIN(gc)&STK_PS_GC_GAIN_MASK);

    return i2c_smbus_write_byte_data(stk3171->client,STK_PS_GC_REG,stk3171->ps_gc_reg);
}


int32_t set_ps_thd_l(struct i2c_client *client,uint8_t thd_l)
{
#ifdef CONFIG_STK_PS_ENGINEER_TUNING
    ps_code_low_thd = thd_l;
#endif
  return i2c_smbus_write_byte_data(client,STK_PS_THD_L_REG,thd_l);
}

static int32_t set_ps_thd_h(struct i2c_client *client,uint8_t thd_h)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    stk3171->ps_thd_high = thd_h;
    return i2c_smbus_write_byte_data(client,STK_PS_THD_H_REG,thd_h);
}

static int32_t set_als_thd_l(struct i2c_client *client,uint16_t thd_l)
{
    uint8_t temp;
    uint8_t* pSrc = (uint8_t*)&thd_l;
    temp = *pSrc;
    *pSrc = *(pSrc+1);
    *(pSrc+1) = temp;
    return i2c_smbus_write_word_data(client,STK_ALS_THD_L1_REG,thd_l);
}
static int32_t set_als_thd_h(struct i2c_client *client,uint16_t thd_h)
{
    uint8_t temp;
    uint8_t* pSrc = (uint8_t*)&thd_h;
    temp = *pSrc;
    *pSrc = *(pSrc+1);
    *(pSrc+1) = temp;
    return i2c_smbus_write_word_data(client,STK_ALS_THD_H1_REG,thd_h);
}

/*a:client data point, b: high threshold value, c:low and high threshold range, d:sleep time*/
#define STK_PS_SET_THRESHOLD(a,b,c,d)    \
    do { \
        set_ps_thd_h(a,b); \
        set_ps_thd_l(a,(b-c)); \
        msleep(d); \
    } while(0)

/*a:client data point, b: high threshold value, c:low and high threshold range*/
#define STK_PS_THRESHOLD_DATA(a,b,c)    \
    do{ \
    a->ps_thd_high= b;  \
    a->ps_thd_low= b-c; \
    }while(0)

inline int32_t get_status_reg(struct i2c_client *client)
{
    return i2c_smbus_read_byte_data(client,STK_PS_STATUS_REG);
}

static int32_t reset_int_flag(struct i2c_client *client,uint8_t org_status,uint8_t disable_flag)
{
	uint8_t val;

	org_status &= (STK_PS_STATUS_PS_INT_FLAG_MASK | STK_PS_STATUS_ALS_INT_FLAG_MASK);
    val = (uint8_t)(org_status&(~disable_flag));
    return i2c_smbus_write_byte_data(client,STK_PS_STATUS_REG,val);
}

inline int32_t get_als_reading(struct i2c_client *client)
{

    int32_t word_data;
    int32_t lsb,msb;
    msb = i2c_smbus_read_byte_data(client,STK_ALS_DT1_REG);
    lsb = i2c_smbus_read_byte_data(client,STK_ALS_DT2_REG);
    word_data = (msb<<8) | lsb;
    return word_data;
}

inline int32_t get_ps_reading(struct i2c_client *client)
{
    return i2c_smbus_read_byte_data(client,STK_PS_READING_REG);
}


inline void als_report_event(struct i2c_client *client,int32_t report_value)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    stk3171->als_lux_last = report_value;
    input_report_abs(stk3171->als_input, ABS_MISC, report_value);
    input_sync(stk3171->als_input);
    PSENSOR_DEBUG(LEVEL0,"als input event %d lux\n",report_value);
}

inline void ps_report_event(struct i2c_client *client,int32_t report_value)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    input_report_abs(stk3171->ps_input, ABS_DISTANCE, report_value);
    input_sync(stk3171->ps_input);
    wake_lock_timeout(&stk3171->wakelock, 2*HZ);
    PSENSOR_DEBUG(LEVEL0,"ps input event %d cm\n",report_value);
}


static int32_t enable_ps(struct i2c_client *client,uint8_t enable)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    int32_t ret;
    uint8_t status = enable,shutdown = 0x01;
    stk3171->ps_cmd_reg &= (~STK_PS_CMD_SD_MASK);
    stk3171->ps_cmd_reg |= STK_PS_CMD_SD(enable?0:1);
    // Dummy Report
    /* ps_report_event(pStkPsData->ps_input,-1); */

    ret = i2c_smbus_write_byte_data(client,STK_PS_CMD_REG,stk3171->ps_cmd_reg);
    if(status)
    {
        if(!stk3171->ps_enable)
        {
            enable_irq(stk3171->irq);
            irq_set_irq_wake(stk3171->irq, 1);
            stk3171->ps_enable = true;
            PSENSOR_DEBUG(LEVEL0,"Enable interrupt \n");
        }
    }
    else
    {
        if(stk3171->ps_enable)
        {
            stk3171->ps_enable = false;
            irq_set_irq_wake(stk3171->irq, 0);
            disable_irq(stk3171->irq);
            PSENSOR_DEBUG(LEVEL0,"Disable interrupt\n");
            /*when 0x01 and 0x09 = 0x01 device will enter shutdown mode*/
            ret = i2c_smbus_write_byte_data(client,STK_PS_CMD_REG,shutdown);
        }
    }
    return ret;

}
static int32_t enable_als(struct i2c_client *client,uint8_t enable)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    int32_t ret,reading;
    uint8_t shutdown=0x01;
    if (enable)
        enable_als_int(stk3171->client,0);
    stk3171->als_cmd_reg &= (~STK_ALS_CMD_SD_MASK);
    stk3171->als_cmd_reg |= STK_ALS_CMD_SD(enable?0:1);
    ret = i2c_smbus_write_byte_data(client,STK_ALS_CMD_REG,stk3171->als_cmd_reg);

    if (enable)
    {
        /*wait for Interrupt*/
		    msleep(1000);
        reading = get_als_reading(client);
        als_report_event(stk3171->client,alscode2lux(reading));
#ifndef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
        nLuxIndex = get_lux_interval_index((uint16_t)reading);
        set_als_thd_h(client,code_threshold_table[nLuxIndex]);
        set_als_thd_l(client,code_threshold_table[nLuxIndex-1]);
#else
        set_als_new_thd_by_reading(reading);
#endif
        enable_als_int(stk3171->client,1);
    }
    else
    {
        /*when 0x01 and 0x09 = 0x01 device will enter shutdown mode*/
        ret = i2c_smbus_write_byte_data(client,STK_ALS_CMD_REG,shutdown);
    }
    return ret;
}

static int32_t init_all_setting(struct i2c_client *client)
{
    struct stkps31xx_data *stk3171 = i2c_get_clientdata(client);
    if (software_reset(client)<0)
    {
        PSENSOR_DEBUG(LEVEL0,"error --> device not found\n");
        return 0;
    }
    set_ps_slp(client,STK_PS_SLEEP_TIME);
    set_ps_gc(client,CONFIG_STK_PS_GAIN_SETTING);
    set_ps_it(client,STK_PS_INTEGRAL_TIME);
    set_ps_led_driving_current(client,STK_PS_IRLED_DRIVING_CURRENT);
    /*x2*/
    set_als_gain(client,0x01);
    /*x4*/
    set_als_it(client,0x02);
    /*Near interrupt threshold*/
    set_ps_thd_h(client,stk3171->ps_thd_high);
    /*away interrupt threshold*/
    set_ps_thd_l(client,stk3171->ps_thd_low);
    enable_ps_int(client,1);
    enable_als_int(client,1);
    /*PS and LS disable will enter shutdown mode*/
    enable_ps(client,0);
    enable_als(client,0);
    return 1;
}

#ifdef CONFIG_STK_SYSFS_DBG
// For Debug
static ssize_t help_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
     return sprintf(buf, "Usage : cat xxxx\nor echo val > xxxx\
     \nWhere xxxx = ps_code : RO (0~255)\nals_code : RO (0~65535)\nlux : RW (0~by your setting)\ndistance : RW (0 or 1)\
     \nals_enable : RW (0~1)\nps_enable : RW(0~1)\nals_transmittance : RW (1~10000)\
     \nps_sleep_time : RW (0~3)\nps_led_driving_current : RW(0~1)\nps_integral_time(0~3)\nps_gain_setting : RW(0~3)\n");

}

static ssize_t driver_version_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf,"%s\n",STK_DRIVER_VER);
}

static ssize_t als_code_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t reading;
    struct stkps31xx_data *stk3171 = dev_get_drvdata(dev);
    mutex_lock(&stk3171->mutex);
    reading = get_als_reading(stk3171->client);
    mutex_unlock(&stk3171->mutex);
    return sprintf(buf, "%d\n", reading);
}


static ssize_t ps_code_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t reading;
    struct stkps31xx_data *stk3171 = dev_get_drvdata(dev);

    mutex_lock(&stk3171->mutex);
    reading = get_ps_reading(stk3171->client);
    mutex_unlock(&stk3171->mutex);
    return sprintf(buf, "%d\n", reading);
}
#endif //CONFIG_STK_SYSFS_DBG

static ssize_t lux_range_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n", alscode2lux((1<<16) -1));//full code

}

static ssize_t dist_mode_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf,"%d\n",(int32_t)STK_PS_DISTANCE_MODE);
}
static ssize_t dist_res_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "1\n"); // means 1 cm in Android
}
static ssize_t lux_res_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "1\n"); // means 1 lux in Android
}
static ssize_t distance_range_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n",1);
}

static ssize_t ps_enable_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t enable;
    mutex_lock(&stk3171->mutex);
    enable = (stk3171->ps_cmd_reg & STK_PS_CMD_SD_MASK)?0:1;
    mutex_unlock(&stk3171->mutex);
    return sprintf(buf, "%d\n", enable);
}

static ssize_t ps_enable_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int reading=0;
    uint32_t value = simple_strtoul(buf, NULL, 10);
    PSENSOR_DEBUG(LEVEL0,"Enable PS : %d\n",value);
    mutex_lock(&stk3171->mutex);

    enable_ps(stk3171->client,value);
    reading = get_ps_reading(stk3171->client);
    PSENSOR_DEBUG(LEVEL0,"%s : ps code = %d\n",__func__,reading);

    if (reading >= stk3171->ps_thd_high)
    {
        ps_report_event(stk3171->client,0);
    }
    else
    {
        ps_report_event(stk3171->client,1);
    }
    mutex_unlock(&stk3171->mutex);
    return len;
}

static ssize_t als_enable_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t enable;
    //if(stk3171 == )
    mutex_lock(&stk3171->mutex);
    enable = (stk3171->als_cmd_reg & STK_ALS_CMD_SD_MASK)?0:1;
    mutex_unlock(&stk3171->mutex);
    return sprintf(buf, "%d\n", enable);
}

static ssize_t als_enable_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    uint32_t value = simple_strtoul(buf, NULL, 10);
    PSENSOR_DEBUG(LEVEL0,"Enable ALS : %d\n",value);
    mutex_lock(&stk3171->mutex);
    enable_als(stk3171->client,value);
    mutex_unlock(&stk3171->mutex);
    return len;
}

static ssize_t lux_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t als_reading;
    mutex_lock(&stk3171->mutex);
    als_reading = get_als_reading(stk3171->client);
    mutex_unlock(&stk3171->mutex);
    return sprintf(buf, "%d lux\n", alscode2lux(als_reading));
}

static ssize_t lux_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 16);
    mutex_lock(&stk3171->mutex);
    als_report_event(stk3171->client,value);
    mutex_unlock(&stk3171->mutex);
    return len;
}

static ssize_t distance_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t dist=1;
    mutex_lock(&stk3171->mutex);

    if (get_ps_reading(stk3171->client)>=stk3171->ps_thd_high)
    {
        ps_report_event(stk3171->client,0);
        dist=0;
    }
    else
    {
        ps_report_event(stk3171->client,1);
        dist=1;
    }
    mutex_unlock(&stk3171->mutex);
    return sprintf(buf, "%d\n", dist);
}

static ssize_t distance_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 16);
    mutex_lock(&stk3171->mutex);
    ps_report_event(stk3171->client,value);
    mutex_unlock(&stk3171->mutex);
    return len;
}

static ssize_t ps_sensitivity_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    return sprintf(buf, "H(%d),L(%d)\n", stk3171->ps_thd_high,stk3171->ps_thd_low);
}

static ssize_t ps_sensitivity_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int i=0,new_value,value = simple_strtoul(buf, NULL, 10);
    int32_t reading=0,penable=0;

    mutex_lock(&stk3171->mutex);

    PSENSOR_DEBUG(LEVEL0,"start value=%d\n",value);
    /*When value < 255, directly setup near interrupt threshold*/
    if(value <= 255)
    {
        new_value = value;
        PSENSOR_DEBUG(LEVEL0,"new =%d\n", new_value);
        stk3171->ps_thd_high = new_value;
        stk3171->ps_thd_low = new_value - 1;
        set_ps_thd_h(stk3171->client,stk3171->ps_thd_high);
        set_ps_thd_l(stk3171->client,stk3171->ps_thd_low);
    }
    /*When Value more then 256 setup auto-test cycle time(ms).
     *The actual sleep time is value - 100*/
    else if(value > 256 && value < 1000)
    {
        /* Avoid Sensor IC is working in interrupt that affect auto-sensitivity test
         * ewault. First disable proximity sensor all setting after sleep a periid of time
         * then enable proximity sensor */
        penable = stk3171->ps_enable;
        enable_ps(stk3171->client,0);
        STK_PS_SET_THRESHOLD(stk3171->client,255,1,value);
        enable_ps(stk3171->client,1);
        /*There are 17 level for threshold. Each level have 15 uints(the sensor uints isn't linear)*/
        for (i=17; i>=1; i--)
        {
            new_value = i*15;
            STK_PS_SET_THRESHOLD(stk3171->client,new_value,1,value-100);
            PSENSOR_DEBUG(LEVEL1,"new =%d\n", new_value);
            reading = gpio_get_value(EINT_GPIO);
            /*As scan interrupt is low or threshold is only 15. store latest data*/
            if(!reading||new_value==15)
            {
                PSENSOR_DEBUG(LEVEL1,"reading refind =%d\n", new_value);
                STK_PS_THRESHOLD_DATA(stk3171,new_value,1);
                /*Threshold less than 15, the sensitivity is nonlinear.
                 *So the calibration data */
                if(new_value==15)
                {
                    PSENSOR_DEBUG(LEVEL1,"new_value =%d\n", new_value);
                    for (i=15; i>=2; i--)
                    {
                        new_value = i*1;
                        STK_PS_SET_THRESHOLD(stk3171->client,new_value,1,value-100);
                        PSENSOR_DEBUG(LEVEL1,"new_value refind =%d\n", new_value);
                        reading = gpio_get_value(EINT_GPIO);
                        if(!reading||new_value==2)
                        {
                            STK_PS_THRESHOLD_DATA(stk3171,new_value,1);
                            break;
                        }
                    }
                }
                break;
            }
            //set_ps_thd_h(stk3171->client,stk3171->ps_thd_high);
            //set_ps_thd_l(stk3171->client,stk3171->ps_thd_low);
        } 
    }
    /*If originally status is in disable, restitute state*/
    if(!penable)
    {
        enable_ps(stk3171->client,penable);
    }
    mutex_unlock(&stk3171->mutex);
    return len;
}

#ifdef CONFIG_STK_SYSFS_DBG

#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
#pragma message("Enable STK ALS Transmittance Tuning w/ SYSFS")
static ssize_t als_transmittance_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t transmittance;
    mutex_lock(&stk3171->mutex);
    transmittance = als_transmittance;
    mutex_unlock(&stk3171->mutex);
    return sprintf(buf, "%d\n", transmittance);
}

static ssize_t als_transmittance_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 10);
    mutex_lock(&stk3171->mutex);
    als_transmittance = value;
    mutex_unlock(&stk3171->mutex);
    return len;
}

#endif // CONFIG_STK_ALS_TRANSMITTANCE_TUNING



#ifdef CONFIG_STK_PS_ENGINEER_TUNING

#pragma message("Enable STK PS Engineering Tuning w/ SYSFS")

static ssize_t ps_code_thd_l_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t ps_thd_l_reg;
    mutex_lock(&stk3171->mutex);
    ps_thd_l_reg = i2c_smbus_read_byte_data(stk3171->client,STK_PS_THD_L_REG);
    mutex_unlock(&stk3171->mutex);
    return sprintf(buf, "%d\n", ps_thd_l_reg);
}


static ssize_t ps_code_thd_l_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 10);
    mutex_lock(&stk3171->mutex);
    set_ps_thd_l(stk3171->client,value);
    mutex_unlock(&stk3171->mutex);
    return len;
}

static ssize_t ps_code_thd_h_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t ps_thd_h_reg;
    mutex_lock(&stk3171->mutex);
    ps_thd_h_reg = i2c_smbus_read_byte_data(stk3171->client,STK_PS_THD_H_REG);
    mutex_unlock(&stk3171->mutex);
    return sprintf(buf, "%d\n", ps_thd_h_reg);
}


static ssize_t ps_code_thd_h_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 10);
    mutex_lock(&stk3171->mutex);
    if((value >= 20) && (value<=250))
    {
        stk3171->ps_thd_high = value;
        stk3171->ps_thd_low = value-10;
    }
    else
    {
        PSENSOR_DEBUG(LEVEL0,"fail data=%d", (int)value);
        mutex_unlock(&stk3171->mutex);
        return len;
    }
    set_ps_thd_h(stk3171->client,value);
    mutex_unlock(&stk3171->mutex);
    return len;
}

static ssize_t ps_sleep_time_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t value;
    mutex_lock(&stk3171->mutex);
    value = stk3171->ps_cmd_reg;
    mutex_unlock(&stk3171->mutex);
    value&=STK_PS_CMD_SLP_MASK;
    value>>=STK_PS_CMD_SLP_SHIFT;
    return sprintf(buf, "0x%x\n", value);
}


static ssize_t ps_sleep_time_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 10);
    mutex_lock(&stk3171->mutex);
    set_ps_slp(stk3171->client,value);
    mutex_unlock(&stk3171->mutex);
    return len;
}


static ssize_t ps_led_driving_current_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
     struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t value;
    mutex_lock(&stk3171->mutex);
    value = stk3171->ps_cmd_reg;
    mutex_unlock(&stk3171->mutex);
    value&=STK_PS_CMD_DR_MASK;
    value>>=STK_PS_CMD_DR_SHIFT;
    return sprintf(buf, "0x%x\n", value);
}


static ssize_t ps_led_driving_current_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 10);
    mutex_lock(&stk3171->mutex);
    set_ps_led_driving_current(stk3171->client,value);
    mutex_unlock(&stk3171->mutex);
    return len;
}
static ssize_t ps_integral_time_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t value;
    mutex_lock(&stk3171->mutex);
    value = stk3171->ps_cmd_reg;
    mutex_unlock(&stk3171->mutex);
    value&=STK_PS_CMD_IT_MASK;
    value>>=STK_PS_CMD_IT_SHIFT;
    return sprintf(buf, "0x%x\n", value);
}


static ssize_t ps_integral_time_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 10);
    mutex_lock(&stk3171->mutex);
    set_ps_it(stk3171->client,(uint8_t)value);
    mutex_unlock(&stk3171->mutex);
    return len;
}
static ssize_t ps_gain_setting_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t gc_reg;
    mutex_lock(&stk3171->mutex);
    gc_reg = stk3171->ps_gc_reg;
    mutex_unlock(&stk3171->mutex);
    return sprintf(buf, "0x%x\n", gc_reg);
}


static ssize_t ps_gain_setting_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 10);
    mutex_lock(&stk3171->mutex);
    set_ps_gc(stk3171->client,(uint8_t)value);
    mutex_unlock(&stk3171->mutex);
    return len;
}

static ssize_t als_lux_thd_l_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t als_thd_l0_reg,als_thd_l1_reg;
    mutex_lock(&stk3171->mutex);
    als_thd_l0_reg = i2c_smbus_read_byte_data(stk3171->client,STK_ALS_THD_L0_REG);
    als_thd_l1_reg = i2c_smbus_read_byte_data(stk3171->client,STK_ALS_THD_L1_REG);
    mutex_unlock(&stk3171->mutex);
    als_thd_l0_reg|=(als_thd_l1_reg<<8);

    return sprintf(buf, "%d\n", alscode2lux(als_thd_l0_reg));
}


static ssize_t als_lux_thd_l_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 20);
    value = lux2alscode(value);
    mutex_lock(&stk3171->mutex);
    set_als_thd_l(stk3171->client,value);
    mutex_unlock(&stk3171->mutex);
    return len;
}

static ssize_t als_lux_thd_h_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t als_thd_h0_reg,als_thd_h1_reg;
    mutex_lock(&stk3171->mutex);
    als_thd_h0_reg = i2c_smbus_read_byte_data(stk3171->client,STK_ALS_THD_H0_REG);
    als_thd_h1_reg = i2c_smbus_read_byte_data(stk3171->client,STK_ALS_THD_H1_REG);
    mutex_unlock(&stk3171->mutex);
    als_thd_h0_reg|=(als_thd_h1_reg<<8);

    return sprintf(buf, "%d\n", alscode2lux(als_thd_h0_reg));
}


static ssize_t als_lux_thd_h_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf, size_t len)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    unsigned long value = simple_strtoul(buf, NULL, 20);
    value = lux2alscode(value);
    mutex_lock(&stk3171->mutex);
    set_als_thd_h(stk3171->client,value);
    mutex_unlock(&stk3171->mutex);
    return len;
}

static ssize_t all_reg_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    struct stkps31xx_data *stk3171 = pStkPsData;
    int32_t ps_reg[12];
    uint8_t cnt;
    mutex_lock(&stk3171->mutex);
    for(cnt=0;cnt<12;cnt++)
    {
        ps_reg[cnt] = i2c_smbus_read_byte_data(stk3171->client, (cnt+1));
        if(ps_reg[cnt] < 0)
        {
            mutex_unlock(&stk3171->mutex);
            PSENSOR_DEBUG(LEVEL0,"all_reg_show:i2c_smbus_read_byte_data fail, ret=%d", ps_reg[cnt]);
            return -1;
        }
        else
        {
            PSENSOR_DEBUG(LEVEL0,"reg[%2X]=0x%2X\n", cnt+1, ps_reg[cnt]);
        }

    }
    mutex_unlock(&stk3171->mutex);

    return sprintf(buf, "%2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X\n", ps_reg[0], ps_reg[1], ps_reg[2], ps_reg[3], ps_reg[4], ps_reg[5], ps_reg[6], ps_reg[7], ps_reg[8], ps_reg[9], ps_reg[10], ps_reg[11]);
}

static struct kobj_attribute ps_sleep_time_attribute = (struct kobj_attribute)__ATTR_RW(ps_sleep_time);
static struct kobj_attribute ps_led_driving_current_attribute = (struct kobj_attribute)__ATTR_RW(ps_led_driving_current);
static struct kobj_attribute ps_integral_time_attribute = (struct kobj_attribute)__ATTR_RW(ps_integral_time);
static struct kobj_attribute ps_gain_setting_attribute = (struct kobj_attribute)__ATTR_RW(ps_gain_setting);
static struct kobj_attribute als_lux_thd_l_attribute = (struct kobj_attribute)__ATTR_RW(als_lux_thd_l);
static struct kobj_attribute als_lux_thd_h_attribute = (struct kobj_attribute)__ATTR_RW(als_lux_thd_h);
static struct kobj_attribute all_reg_attribute = (struct kobj_attribute)__ATTR_RO(all_reg);
static struct kobj_attribute ps_code_thd_l_attribute = (struct kobj_attribute)__ATTR_RW(ps_code_thd_l);
static struct kobj_attribute ps_code_thd_h_attribute = (struct kobj_attribute)__ATTR_RW(ps_code_thd_h);
#endif //CONFIG_STK_PS_ENGINEER_TUNING

#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
static struct kobj_attribute als_transmittance_attribute = (struct kobj_attribute)__ATTR_RW(als_transmittance);
#endif /*CONFIG_STK_ALS_TRANSMITTANCE_TUNING*/

#endif //CONFIG_STK_SYSFS_DBG


#ifdef CONFIG_STK_SYSFS_DBG
/* Only for debug */
static struct kobj_attribute help_attribute = (struct kobj_attribute)__ATTR_RO(help);
static struct kobj_attribute driver_version_attribute = (struct kobj_attribute)__ATTR_RO(driver_version);
static struct kobj_attribute als_code_attribute = (struct kobj_attribute)__ATTR_RO(als_code);
static struct kobj_attribute ps_code_attribute = (struct kobj_attribute)__ATTR_RO(ps_code);
#endif //CONFIG_STK_SYSFS_DBG

static struct kobj_attribute lux_range_attribute = (struct kobj_attribute)__ATTR_RO(lux_range);
static struct kobj_attribute lux_attribute = (struct kobj_attribute)__ATTR_RW(lux);
static struct kobj_attribute distance_attribute = (struct kobj_attribute)__ATTR_RW(distance);
static struct kobj_attribute ps_enable_attribute = (struct kobj_attribute)__ATTR_RW(ps_enable);
static struct kobj_attribute als_enable_attribute = (struct kobj_attribute)__ATTR_RW(als_enable);
static struct kobj_attribute ps_dist_mode_attribute = (struct kobj_attribute)__ATTR_RO(dist_mode);
static struct kobj_attribute ps_dist_res_attribute = (struct kobj_attribute)__ATTR_RO(dist_res);
static struct kobj_attribute lux_res_attribute = (struct kobj_attribute)__ATTR_RO(lux_res);
static struct kobj_attribute ps_distance_range_attribute = (struct kobj_attribute)__ATTR_RO(distance_range);
static struct kobj_attribute ps_sensitivity_attribute = (struct kobj_attribute)__ATTR_RW(ps_sensitivity);


static struct attribute* sensetek_proximity_sensors_attrs [] =
{
    &distance_attribute.attr,
    &ps_enable_attribute.attr,
    &ps_dist_mode_attribute.attr,
    &ps_dist_res_attribute.attr,
    &ps_distance_range_attribute.attr,
    &lux_range_attribute.attr,
    &lux_attribute.attr,
    &als_enable_attribute.attr,
    &lux_res_attribute.attr,
    &ps_sensitivity_attribute.attr,
    NULL,
};


static struct attribute_group stk3171_proximity_attribute_group = {
    .attrs = sensetek_proximity_sensors_attrs
};


#ifdef CONFIG_STK_SYSFS_DBG

static struct attribute* sensetek_optical_sensors_dbg_attrs [] =
{
    &help_attribute.attr,
    &driver_version_attribute.attr,
    &lux_range_attribute.attr,
    &distance_attribute.attr,
    &ps_code_attribute.attr,
    &als_code_attribute.attr,
    &lux_attribute.attr,
    &ps_enable_attribute.attr,
    &als_enable_attribute.attr,
    &ps_dist_mode_attribute.attr,
    &ps_dist_res_attribute.attr,
    &lux_res_attribute.attr,
    &ps_distance_range_attribute.attr,
#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
    &als_transmittance_attribute.attr,
#endif
#ifdef CONFIG_STK_PS_ENGINEER_TUNING
    &ps_sleep_time_attribute.attr,
    &ps_led_driving_current_attribute.attr,
    &ps_integral_time_attribute.attr,
    &ps_gain_setting_attribute.attr,
    &ps_code_thd_l_attribute.attr,
    &ps_code_thd_h_attribute.attr,
    &als_lux_thd_l_attribute.attr,
    &als_lux_thd_h_attribute.attr,
    &all_reg_attribute.attr,

#endif /*CONFIG_STK_PS_ENGINEER_TUNING*/
    NULL,
};
// those attributes are only for engineer test/debug
static struct attribute_group sensetek_optics_sensors_attrs_group =
{
    .name = "DBG",
    .attrs = sensetek_optical_sensors_dbg_attrs,
};
#endif /*CONFIG_STK_SYSFS_DBG*/

static int stk_file_attr_register(void)
{
    int ret;
    ret = sysfs_create_group(&stk_oss_dev->dev.kobj,&stk3171_proximity_attribute_group);
    if (ret)
    {
        return -ENOMEM;
    }

#ifdef CONFIG_STK_SYSFS_DBG
    ret = sysfs_create_group(&stk_oss_dev->dev.kobj, &sensetek_optics_sensors_attrs_group);
    if (ret)
    {
        return -ENOMEM;
    }
#endif //CONFIG_STK_SYSFS_DBG

    return 0;
}

static void stk_oss_work(struct work_struct *work)
{
    struct stkps31xx_data *stk3171 = container_of((struct delayed_work *)work, struct stkps31xx_data, work);
    int32_t ret,reading;
    uint8_t disable_flag = 0;
    mutex_lock(&stk3171->mutex);
    /*Get interrupt status from Register*/
    ret = get_status_reg(stk3171->client);

    if(ret < 0)
    {
        mutex_unlock(&stk3171->mutex);
        PSENSOR_DEBUG(LEVEL0,"stk_oss_work:get_status_reg fail, ret=%d", ret);
        msleep(30);
        enable_irq(stk3171->irq);
        return;
    }

    /*Ambient light sensor trigger interrupt */
    if (ret&STK_PS_STATUS_ALS_INT_FLAG_MASK)
    {
        disable_flag = STK_PS_STATUS_ALS_INT_FLAG_MASK;
        reading = get_als_reading(stk3171->client);
#ifndef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
        nLuxIndex = get_lux_interval_index(reading);
        set_als_thd_h(stk3171->client,code_threshold_table[nLuxIndex]);
        set_als_thd_l(stk3171->client,code_threshold_table[nLuxIndex-1]);
#else
        set_als_new_thd_by_reading(reading);
#endif //CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
        als_report_event(stk3171->client,alscode2lux(reading));

    }

    /*Proximity sensor trigger interrupt */
    if (ret&STK_PS_STATUS_PS_INT_FLAG_MASK)
    {
        reading = get_ps_reading(stk3171->client);
        PSENSOR_DEBUG(LEVEL0,"%s : ps code = %d\n",__func__,reading);

        /*Close the sensor*/
        if (reading >= stk3171->ps_thd_high)

        {
            disable_flag |= STK_PS_STATUS_PS_INT_FLAG_MASK;
            ps_report_event(stk3171->client,0);
        }

        /*away the sensor*/
        else  if (reading < stk3171->ps_thd_high)

        {
            disable_flag |= STK_PS_STATUS_PS_INT_FLAG_MASK;
            ps_report_event(stk3171->client,1);
        }
        else
            msleep(10);
    }

    ret = reset_int_flag(stk3171->client,ret,disable_flag);

    if(ret < 0)
    {
        mutex_unlock(&stk3171->mutex);
        PSENSOR_DEBUG(LEVEL0,"stk_oss_work:reset_int_flag fail, ret=%d", ret);
        msleep(30);
        enable_irq(stk3171->irq);
        return;
    }

    msleep(1);
    enable_irq(stk3171->irq);

    mutex_unlock(&stk3171->mutex);
}

static irqreturn_t stk_oss_irq_handler(int irq, void *data)
{
    struct stkps31xx_data *pData = data;
    /*Disable irq*/
    disable_irq_nosync(irq);
    /*schedule work queue*/
    queue_delayed_work(pData->workqueue,&pData->work,0);
    return IRQ_HANDLED;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void stk3171_early_suspend(struct early_suspend * h)
{
    PSENSOR_DEBUG(LEVEL0,"%s:Done.\n",__FUNCTION__);
    return;
}

static void stk3171_late_resume(struct early_suspend *h)
{
    PSENSOR_DEBUG(LEVEL0,"%s:Done\n",__FUNCTION__);
    return;
}
#endif

static int stk_ps_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
    int err;
    struct stkps31xx_data *ps_data;
    PSENSOR_DEBUG(LEVEL0,"stk_ps_probe start\n");
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
    {
        PSENSOR_DEBUG(LEVEL0,"STKPS -- No Support for I2C_FUNC_SMBUS_BYTE_DATA\n");
        return -ENODEV;
    }
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WORD_DATA))
    {
        PSENSOR_DEBUG(LEVEL0,"STKPS -- No Support for I2C_FUNC_SMBUS_WORD_DATA\n");
        return -ENODEV;
    }


    ps_data = kzalloc(sizeof(struct stkps31xx_data),GFP_KERNEL);

    ps_data->platform_data = client->dev.platform_data;
    if (!ps_data->platform_data)
    {
        err = -ENODEV;
        PSENSOR_DEBUG(LEVEL0, "Check platform data failed.");
        goto ERR_CHECK_PLATFORM_DATA;
    }
    if(!ps_data->platform_data->gpio_init)
    {
        /*The platform data didn't register gpio function unregister driver*/
        err = -EIO;
        PSENSOR_DEBUG(LEVEL0, "Check platform data can't find GPIO function");
        goto ERR_CHECK_PLATFORM_DATA;
    }
    else
    {
        /* Init GPIO */
        err = ps_data->platform_data->gpio_init();
    }
    if (err < 0)
        goto ERR_CHECK_PLATFORM_DATA;

    i2c_set_clientdata(client,ps_data);
    ps_data->client = client;
    ps_data->irq = client->irq;
    mutex_init(&ps_data->mutex);
    ps_data->ps_enable = 0;
    ps_data->als_enable = 0;
    /*near interrupt threshold: When close sensor value is large and trigger interrupt*/
    ps_data->ps_thd_high = CONFIG_STK_PS_CODE_HIGH_THRESHOLD;
    /*away interrupt threshold: When away sensor value is small and trigger interrupt*/
    ps_data->ps_thd_low = CONFIG_STK_PS_CODE_LOW_THRESHOLD;
    /*Allocate ambient light sensor input event*/
    ps_data->als_input = input_allocate_device();
    if (ps_data->als_input==NULL)
    {
        err= -ENOMEM;
        PSENSOR_DEBUG(LEVEL0,"%s: could not allocate PS input device\n", __func__);
        goto RELEASE_ALS;
    }
    /*Allocate proximity sensor input event*/
    ps_data->ps_input = input_allocate_device();
    if (ps_data->ps_input==NULL)
    {
        err= -ENOMEM;
        PSENSOR_DEBUG(LEVEL0,"%s: could not allocate ALS input device\n", __func__);
        goto RELEASE_PS;
    }
    ps_data->als_input->name = ALS_NAME;
    ps_data->ps_input->name = PS_NAME;
    set_bit(EV_ABS, ps_data->als_input->evbit);
    set_bit(EV_ABS, ps_data->ps_input->evbit);
    input_set_abs_params(ps_data->als_input, ABS_MISC, 0, alscode2lux((1<<16)-1), 0, 0);
    input_set_abs_params(ps_data->ps_input, ABS_DISTANCE, 0,1, 0, 0);
    /*Register ambient light sensor input event*/
    err = input_register_device(ps_data->als_input);
    if (err<0)
    {
        PSENSOR_DEBUG(LEVEL0,"can not register als input device\n");
        goto FAIL_REGISTER_PS;
    }
    PSENSOR_DEBUG(LEVEL0,"register als input device OK\n");

    /*Register proximity sensor input event*/
    err = input_register_device(ps_data->ps_input);
    if (err<0)
    {
        PSENSOR_DEBUG(LEVEL0,"can not register ps input device\n");
        goto FAIL_REGISTER_ALS;
    }

    /*Assign allocate data point to global variable*/
    pStkPsData = ps_data;
    ps_data->ps_delay = PS_ODR_DELAY;
    ps_data->als_delay = ALS_ODR_DELAY;
    PSENSOR_DEBUG(LEVEL0,"irq # = %d\n",client->irq);

    /*create a new queue*/
    ps_data->workqueue = create_workqueue("stk_oss_wq");
    /*create a new delay queue work*/
    INIT_DELAYED_WORK(&ps_data->work, stk_oss_work);
    /*Disable Proximity sensor and light sensor*/
    enable_als(ps_data->client,0);
    enable_ps(ps_data->client,0);

    /*Register interrupt handle*/
    err = request_irq(ps_data->irq, stk_oss_irq_handler, IRQF_TRIGGER_LOW, DEVICE_NAME, ps_data);
    if (err < 0) {
        PSENSOR_DEBUG(LEVEL0,"%s: request_irq(%d) failed for (%d)\n",
            __func__, client->irq, err);
        goto FAIL_RETURN;
    }

    /*Dsiable interrupt*/
    disable_irq(ps_data->irq);

    /*Init wake lock for report proximity sensor data*/
    wake_lock_init(&ps_data->wakelock,WAKE_LOCK_SUSPEND,"stk_ps_wakelock");
#ifdef CONFIG_HAS_EARLYSUSPEND
    ps_data->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 10;
    ps_data->early_suspend.suspend = stk3171_early_suspend;
    ps_data->early_suspend.resume = stk3171_late_resume;
    register_early_suspend(&ps_data->early_suspend);
#endif

    pStkPsData->irq = client->irq;

#ifndef CONFIG_STK_PS_ALS_USE_CHANGE_THRESHOLD
    init_code_threshold_table();
#endif
    /*Init sensor register setting*/
    if (!init_all_setting(ps_data->client))
    {
        err= -EINVAL;
        goto  FAIL_RETURN;
    }
    PSENSOR_DEBUG(LEVEL0,"stk_ps_probe end\n");
    return 0;

FAIL_RETURN:
FAIL_REGISTER_PS:
    input_unregister_device(ps_data->ps_input);
FAIL_REGISTER_ALS:
    input_unregister_device(ps_data->als_input);
RELEASE_PS:
    input_free_device(ps_data->ps_input);
RELEASE_ALS:
    input_free_device(ps_data->als_input);
    mutex_destroy(&ps_data->mutex);
ERR_CHECK_PLATFORM_DATA:
    kfree(ps_data);
    PSENSOR_DEBUG(LEVEL0,"STK 3171 : Failed\n");
    return err;
}


static int stk_ps_remove(struct i2c_client *client)
{
    struct stkps31xx_data *data=i2c_get_clientdata(client);
    mutex_destroy(&data->mutex);
    wake_lock_destroy(&data->wakelock);
    if (data)
    {
        disable_irq(data->irq);
#if ADDITIONAL_GPIO_CFG // Additional GPIO CFG
        gpio_free(EINT_GPIO);
#endif // Additional GPIO CFG
#ifdef CONFIG_HAS_EARLYSUSPEND
        unregister_early_suspend(&data->early_suspend);
#endif
        if (data->workqueue)
            destroy_workqueue(data->workqueue);
        input_unregister_device(data->als_input);
        input_unregister_device(data->ps_input);
        input_free_device(data->als_input);
        input_free_device(data->ps_input);
        kfree(data);
        pStkPsData = 0;
    }
    return 0;
}

static const struct i2c_device_id stk_ps_id[] =
{
    { "stk_ps", 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, stk_ps_id);

static struct i2c_driver stk_ps_driver =
{
    .driver = {
        .name = STKPS_DRV_NAME,
    },
    .probe = stk_ps_probe,
    .remove = stk_ps_remove,
    .id_table = stk_ps_id,
};


static int __init stk_i2c_ps31xx_init(void)
{
    int ret;
    PSENSOR_DEBUG(LEVEL0,"stk_i2c_ps31xx_init start\n");
    ret = i2c_add_driver(&stk_ps_driver);
    if (ret)
        return ret;

    stk_oss_dev = platform_device_alloc(DEVICE_NAME,-1);
    if (!stk_oss_dev)
    {
        i2c_del_driver(&stk_ps_driver);
        return -ENOMEM;
    }
    if (platform_device_add(stk_oss_dev))
    {
        i2c_del_driver(&stk_ps_driver);
        return -ENOMEM;
    }

    if(stk_file_attr_register() != 0)
    {
        PSENSOR_DEBUG(LEVEL0,"can not create ps file node\n");
        return -ENOMEM;
    }
    PSENSOR_DEBUG(LEVEL0,"STK PS Module initialized.\n");
    return 0;
}

static void __exit stk_i2c_ps31xx_exit(void)
{
    if (stk_oss_dev);
    {
#ifdef CONFIG_STK_SYSFS_DBG
        sysfs_remove_group(&(stk_oss_dev->dev.kobj), &sensetek_optics_sensors_attrs_group);
#endif
        sysfs_remove_group(&(stk_oss_dev->dev.kobj),&stk3171_proximity_attribute_group);
    }
    platform_device_put(stk_oss_dev);
    i2c_del_driver(&stk_ps_driver);
    platform_device_unregister(stk_oss_dev);
}

MODULE_AUTHOR("Patrick Chang <patrick_chang@sitronix.com>");
MODULE_DESCRIPTION("SenseTek Proximity Sensor driver");
MODULE_LICENSE("GPL");
module_init(stk_i2c_ps31xx_init);
module_exit(stk_i2c_ps31xx_exit);
