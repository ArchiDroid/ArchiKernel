/*
 * drivers/i2c/chips/SenseTek/stk_i2c_ps31xx.h
 *
 * $Id: stk_i2c_ps31xx.h,v 1.0 2011/02/26 11:12:08 jsgood Exp $
 *
 * Copyright (C) 2011 Patrick Chang <patrick_chang@sitronix.com.tw>
 * Copyright(C) 2011-2012 Foxconn International Holdings, Ltd. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	SenseTek/Sitronix Proximity & Ambient Light Sensor Driver
 *	based on stk_i2c_ps31xx.c
 */
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/errno.h>
#include <linux/wakelock.h>
#include <linux/interrupt.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#ifndef __STK_I2C_PS31XX
#define __STK_I2C_PS31XX

#define STK_ALS_CMD_REG 0x01
#define STK_ALS_DT1_REG 0x02
#define STK_ALS_DT2_REG 0x03

#define STK_ALS_THD_H1_REG 0x04
#define STK_ALS_THD_H0_REG 0x05
#define STK_ALS_THD_L1_REG 0x06
#define STK_ALS_THD_L0_REG 0x07

#define STK_PS_STATUS_REG 0x08
#define STK_PS_CMD_REG 0x09
#define STK_PS_READING_REG 0x0A
#define STK_PS_THD_H_REG 0x0B
#define STK_PS_THD_L_REG 0x0C

#define STK_PS_SOFTWARE_RESET_REG 0x80
#define STK_PS_GC_REG 0x82

/* Define ALS CMD */

#define STK_ALS_CMD_GAIN_SHIFT  6
#define STK_ALS_CMD_IT_SHIFT    2
#define STK_ALS_CMD_SD_SHIFT    0
#define STK_ALS_CMD_INT_SHIFT   1

#define STK_ALS_CMD_GAIN(x) ((x)<<STK_ALS_CMD_GAIN_SHIFT)
#define STK_ALS_CMD_IT(x) ((x)<<STK_ALS_CMD_IT_SHIFT)
#define STK_ALS_CMD_INT(x) ((x)<<STK_ALS_CMD_INT_SHIFT)
#define STK_ALS_CMD_SD(x) ((x)<<STK_ALS_CMD_SD_SHIFT)

#define STK_ALS_CMD_GAIN_MASK 0x40
#define STK_ALS_CMD_IT_MASK 0x0C
#define STK_ALS_CMD_INT_MASK 0x02
#define STK_ALS_CMD_SD_MASK 0x1

/* Define PS Status */
#define STK_PS_STATUS_ID_SHIFT  6

#define STK_PS_STATUS_ID(x) ((x)<<STK_PS_STATUS_ID_SHIFT)

#define STK_PS_STATUS_ID_MASK  0xC0
#define STK_PS_STATUS_PS_INT_FLAG_MASK 0x20
#define STK_PS_STATUS_ALS_INT_FLAG_MASK 0x10

#define STK_PS31xx_ID 0x00


/* Define PS CMD */

#define STK_PS_CMD_INTCTRL_SHIFT 7
#define STK_PS_CMD_SLP_SHIFT    5
#define STK_PS_CMD_DR_SHIFT     4
#define STK_PS_CMD_IT_SHIFT     2
#define STK_PS_CMD_INT_SHIFT    1
#define STK_PS_CMD_SD_SHIFT     0

#define STK_PS_CMD_INTCTRL(x) ((x)<<STK_PS_CMD_INTCTRL_SHIFT)
#define STK_PS_CMD_SLP(x) ((x)<<STK_PS_CMD_SLP_SHIFT)
#define STK_PS_CMD_DR(x) ((x)<<STK_PS_CMD_DR_SHIFT)
#define STK_PS_CMD_IT(x) ((x)<<STK_PS_CMD_IT_SHIFT)
#define STK_PS_CMD_INT(x) ((x)<<STK_PS_CMD_INT_SHIFT)
#define STK_PS_CMD_SD(x) ((x)<<STK_PS_CMD_SD_SHIFT)

#define STK_PS_CMD_INTCTRL_MASK 0x80
#define STK_PS_CMD_SLP_MASK 0x60
#define STK_PS_CMD_DR_MASK 0x10
#define STK_PS_CMD_IT_MASK 0x0C
#define STK_PS_CMD_INT_MASK 0x2
#define STK_PS_CMD_SD_MASK 0x1

/* Define PS GC */
#define STK_PS_GC_GAIN(x) ((x)<<0)
#define STK_PS_GC_GAIN_MASK 0x0f

#define STK_IRQF_MODE IRQF_TRIGGER_LOW

#define EINT_GPIO 55

struct stk3171_platform_data {
    int (*gpio_init)(void);
    int sensitivity;
};

struct stkps31xx_data {
    struct i2c_client *client;
    
    uint8_t ps_reading;
    uint16_t als_reading;
    
    uint8_t ps_gc_reg;
    uint8_t ps_cmd_reg;
    uint8_t als_cmd_reg;
    
    struct input_dev *als_input;
    struct input_dev *ps_input;
    //int32_t ps_distance_last;
    //int32_t ps_code_last;
    int32_t als_lux_last;
    int32_t ps_enable;
    int32_t als_enable;
    int32_t ps_thd_high;
    int32_t ps_thd_low;
    int32_t ps_thd_range;

/* ALS/PS Delay would be USELESS for Interrupt Mode, but provide an unified interface for Android HAL*/
    uint32_t ps_delay;
    uint32_t als_delay;
    struct stk3171_platform_data *platform_data;
    struct delayed_work work;
    int32_t irq;
    struct mutex mutex;
    struct workqueue_struct *workqueue;
    struct wake_lock wakelock;
    #ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend;
    #endif

};

#if defined(CONFIG_STK_PS_SLP00)
#define STK_PS_SLEEP_TIME 0x00
#elif defined(CONFIG_STK_PS_SLP01)
#define STK_PS_SLEEP_TIME 0x01
#elif defined(CONFIG_STK_PS_SLP10)
#define STK_PS_SLEEP_TIME 0x02
#elif defined(CONFIG_STK_PS_SLP11)
#define STK_PS_SLEEP_TIME 0x03
#else
#error
#endif

#if defined(CONFIG_STK_PS_IT00)
#define STK_PS_INTEGRAL_TIME 0x00
#elif defined(CONFIG_STK_PS_IT01)
#define STK_PS_INTEGRAL_TIME 0x01
#elif defined(CONFIG_STK_PS_IT10)
#define STK_PS_INTEGRAL_TIME 0x02
#elif defined(CONFIG_STK_PS_IT11)
#define STK_PS_INTEGRAL_TIME 0x03
#else
#error
#endif

#if defined(CONFIG_STK_PS_IRDR0)
#define STK_PS_IRLED_DRIVING_CURRENT 0
#elif defined(CONFIG_STK_PS_IRDR1)
#define STK_PS_IRLED_DRIVING_CURRENT 1
#else
#error
#endif

#define ALS_MIN_DELAY   100
#define PS_MIN_DELAY    10


#ifdef CONFIG_STK_PS_DISTANCE_MODE
#define STK_PS_DISTANCE_MODE 1
// distance mode
#else
// 0 and 1 (switch) mode
#define STK_PS_DISTANCE_MODE 0
#endif //CONFIG_STK_PS_DISTANCE_MODE

#define LEVEL0 0
#define LEVEL1 1
#define PSENSOR_DBG
static int psensor_debug_level = 0;
#ifdef PSENSOR_DBG
#define PSENSOR_DEBUG(level, fmt, ...) \
    do { \
        if (level <= psensor_debug_level) \
            printk("[PSENSOR] %s(%d): " fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__); \
    } while(0)
#else
#define PSENSOR_DEBUG(level, fmt, ...)
#endif

#endif // __STK_I2C_PS31XX
