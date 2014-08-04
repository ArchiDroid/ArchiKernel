/*
 * gpu_clock_control.c -- a clock control interface for the sgs2/3
 *
 *  Copyright (C) 2011 Michael Wodkins
 *  twitter - @xdanetarchy
 *  XDA-developers - netarchy
 *  modified by gokhanmoral
 *
 *  Modified by Andrei F. for Galaxy S3 / Perseus kernel (June 2012)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of the GNU General Public License as published by the
 *  Free Software Foundation;
 *
 */

#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#include "gpu_clock_control.h"

#define GPU_MAX_CLOCK 800
#define GPU_MIN_CLOCK 10
#define MALI_STEPS 5

typedef struct mali_dvfs_tableTag{
    unsigned int clock;
    unsigned int freq;
    unsigned int vol;
    unsigned int downthreshold;
    unsigned int upthreshold;
}mali_dvfs_table;

extern mali_dvfs_table mali_dvfs[MALI_STEPS];

static ssize_t gpu_clock_show(struct device *dev, struct device_attribute *attr, char *buf) {
        return sprintf(buf, "Step0: %d\nStep1: %d\nStep2: %d\nStep3: %d\nStep4: %d\n"
                                                "Threshold0-1/up-down: %d%% %d%%\n"
                                                "Threshold1-2/up-down: %d%% %d%%\n"
                                                "Threshold2-3/up-down: %d%% %d%%\n"
                                                "Threshold3-4/up-down: %d%% %d%%\n",
                mali_dvfs[0].clock,
                mali_dvfs[1].clock,
                mali_dvfs[2].clock,
                mali_dvfs[3].clock,
                mali_dvfs[4].clock,
                mali_dvfs[0].upthreshold,
                mali_dvfs[1].downthreshold,
                mali_dvfs[1].upthreshold,
                mali_dvfs[2].downthreshold,
                mali_dvfs[2].upthreshold,
                mali_dvfs[3].downthreshold,
                mali_dvfs[3].upthreshold,
                mali_dvfs[4].downthreshold
                );
}

unsigned int g[(MALI_STEPS-1)*2];

static ssize_t gpu_clock_store(struct device *dev, struct device_attribute *attr,
                               const char *buf, size_t count) {
        unsigned int ret = -EINVAL;
        int i = 0;

        if ( (ret=sscanf(buf, "%d%% %d%% %d%% %d%% %d%% %d%% %d%% %d%%",
                         &g[0], &g[1], &g[2], &g[3], &g[4], &g[5], &g[6], &g[7]))
              == (MALI_STEPS-1)*2 ) i=1;

        if(i) {
                if(g[1]<0 || g[0]>100 || g[3]<0 || g[2]>100 ||
			 g[5]<0 || g[4]>100 || g[7]<0 || g[6]>100) 
                        return -EINVAL;

                mali_dvfs[0].upthreshold = (int)(g[0]);
                mali_dvfs[1].downthreshold = (int)(g[1]);
                mali_dvfs[1].upthreshold = (int)(g[2]);
                mali_dvfs[2].downthreshold = (int)(g[3]);
                mali_dvfs[2].upthreshold = (int)(g[4]);
                mali_dvfs[3].downthreshold = (int)(g[5]);
                mali_dvfs[3].upthreshold = (int)(g[6]);
                mali_dvfs[4].downthreshold = (int)(g[7]);
        } else {
                if ( (ret=sscanf(buf, "%d %d %d %d %d", &g[0], &g[1], &g[2], &g[3], &g[4])) != MALI_STEPS)
                        return -EINVAL;

                /* safety floor and ceiling - netarchy */
                for( i = 0; i < MALI_STEPS; i++ ) {
                        if (g[i] < GPU_MIN_CLOCK) {
                                g[i] = GPU_MIN_CLOCK;
                        }
                        else if (g[i] > GPU_MAX_CLOCK) {
                                g[i] = GPU_MAX_CLOCK;
                        }

                        if(ret==MALI_STEPS)
                                mali_dvfs[i].clock=g[i];
                }
        }

        return count;
}


static DEVICE_ATTR(gpu_control, S_IRUGO | S_IWUGO, gpu_clock_show, gpu_clock_store);

static struct attribute *gpu_clock_control_attributes[] = {
        &dev_attr_gpu_control.attr,
        NULL
};

static struct attribute_group gpu_clock_control_group = {
        .attrs = gpu_clock_control_attributes,
};

static struct miscdevice gpu_clock_control_device = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "gpu_clock_control",
};

void gpu_clock_control_start()
{
        printk("Initializing gpu clock control interface\n");

        misc_register(&gpu_clock_control_device);
        if (sysfs_create_group(&gpu_clock_control_device.this_device->kobj,
                                &gpu_clock_control_group) < 0) {
                printk("%s sysfs_create_group failed\n", __FUNCTION__);
                pr_err("Unable to create group for %s\n", gpu_clock_control_device.name);
        }
}
