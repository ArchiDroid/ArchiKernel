/* touch_tester.c
 *
 * Copyright (C) 2012 LGE.
 *
 * Author: yehan.ahn@lge.com
 * Modify: hyesung.shin@lge.com
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/earlysuspend.h>
#include <linux/jiffies.h>
#include <linux/sysdev.h>
#include <linux/types.h>
#include <linux/time.h>
#include <asm/atomic.h>

#include <linux/input/lge_touch_core.h>

/************************************
 *** Target dependent data start ****
 ************************************
 *      Check this before test      *
 ************************************/

#define MAX_X	1100
#define MAX_Y	1900
#define MAX_P	255	//pressure
#define MAX_W	15	//width

/* drag & flick data from TVS
 * Here are Sample data
 */
const u16 drag_x[] = {
	915,  915,  913,  911,  912,  910,  903,  897,  892,  877,
	866,  851,  832,  822,  810,  799,  783,  774,  758,  744,
	733,  720,  708,  697,  685,  669,  658,  639,  632,  618,
	608,  591,  581,  569,  553,  541,  528,  513,  505,  487,
	479,  460,  445,  433,  421,  414,  397,  385,  375,  361,
	346,  330,  321,  311,  295,  282,  273,  256,  245,  230,
	221,  201,  193,  175,  161,  150,  144,  131,  127,  123,
	119,  119,  117,  116,  117
};

const u16 drag_y[] = {
	1269, 1267, 1268, 1267, 1269, 1266, 1264, 1260, 1253, 1232,
	1220, 1205, 1192, 1173, 1168, 1161, 1151, 1131, 1116, 1102,
	1090, 1070, 1065, 1058, 1051, 1026, 1012, 999,  983,  974,
	963,  956,  948,  925,  912,  896,  881,  866,  859,  853,
	845,  822,  808,  793,  780,  763,  758,  751,  741,  718,
	701,  689,  675,  659,  654,  647,  631,  612,  598,  583,
	571,  557,  551,  543,  528,  512,  499,  489,  482,  477,
	473,  469,  468,  469,  468,  468,  462
};

const u16 flick_y[] = {1378, 1378, 1313, 1265, 1191, 1123, 1063, 984, 901};

/************************************
 **** Target dependent data end *****
 ************************************/

/* This structure should be sync up with lge_touch_core.c */
struct lge_touch_data
{
	void*			h_touch;
	atomic_t		next_work;
	atomic_t		device_init;
	u8				work_sync_err_cnt;
	u8				ic_init_err_cnt;
	volatile int	curr_pwr_state;
	struct i2c_client 			*client;
	struct input_dev 			*input_dev;
	struct hrtimer 				timer;
	struct work_struct  		work;
	struct delayed_work			work_init;
	struct delayed_work			work_touch_lock;
	struct work_struct  		work_fw_upgrade;
	struct early_suspend		early_suspend;
	struct syna_touch_platform_data 	*pdata;
	struct touch_data			ts_data;
	struct touch_fw_info		fw_info;
	struct section_info			st_info;
	struct kobject 				lge_touch_kobj;
	struct ghost_finger_ctrl	gf_ctrl;
	struct jitter_filter_info	jitter_filter;
	struct accuracy_filter_info	accuracy_filter;
};

extern struct lge_touch_data *touch_test_dev;	// form lge_touch_core.c
extern int accuracy_filter_func(struct lge_touch_data *ts);	// form lge_touch_core.c
extern int jitter_filter_func(struct lge_touch_data *ts);	// form lge_touch_core.c

#define MAX_EVENT		1000
#define FIXED_WIDTH		5
#define FIXED_PRESSURE	50
#define FIXED_ID		0

#define EVENT_IGNORE		0xFFFF

struct test_data
{
	u16 x[MAX_EVENT];
	u16 y[MAX_EVENT];
	u16 event_index;
	u16 event_count;
};

struct touch_tester_data
{
	struct input_dev	*input_dev;
	struct hrtimer 		timer;
	struct test_data	t_data;
};

#define tester_abs(x)		(x > 0 ? x : -x)

static void apply_filter(u16 data[], int size)
{
	int i;

	/* reset history data */
	memset(&touch_test_dev->jitter_filter, 0, sizeof(touch_test_dev->jitter_filter));
	memset(&touch_test_dev->accuracy_filter, 0, sizeof(touch_test_dev->accuracy_filter));

	touch_test_dev->ts_data.total_num = 1;
	touch_test_dev->ts_data.curr_data[FIXED_ID].id = FIXED_ID;
	touch_test_dev->ts_data.curr_data[FIXED_ID].width_major = FIXED_WIDTH;
	touch_test_dev->ts_data.curr_data[FIXED_ID].pressure= FIXED_PRESSURE;
	touch_test_dev->ts_data.curr_data[FIXED_ID].y_position = 0;

	for(i=0; i<size; i++){
		touch_test_dev->ts_data.curr_data[FIXED_ID].x_position = data[i];

		/* Accuracy Solution */
		if (likely(touch_test_dev->pdata->role->accuracy_filter_enable)){
			accuracy_filter_func(touch_test_dev);
		}

		/* Jitter Solution */
		if (likely(touch_test_dev->pdata->role->jitter_filter_enable)){
			if (jitter_filter_func(touch_test_dev) < 0) {
				data[i] = EVENT_IGNORE;
				continue;
			}
		}

		data[i] = touch_test_dev->ts_data.curr_data[FIXED_ID].x_position;
	}
}

static void make_drag_x(struct test_data *t_data, int filter)
{
	int i;

	memset(t_data, 0, sizeof(*t_data));
	memcpy(t_data->x, drag_x, sizeof(drag_x));

	t_data->event_count = sizeof(drag_x) / sizeof(u16);
	t_data->event_index = FIXED_ID;

	if(filter)
		apply_filter(t_data->x, t_data->event_count);

	for(i=0; i<t_data->event_count; i++){
		t_data->y[i] = touch_test_dev->pdata->caps->y_max /2;
		TOUCH_INFO_MSG("[Touch Tester] pos[%4d,%4d]\n", t_data->x[i], t_data->y[i]);
	}
}

static void make_drag_y(struct test_data *t_data, int filter)
{
	int i;

	memset(t_data, 0, sizeof(*t_data));
	memcpy(t_data->y, drag_y, sizeof(drag_y));

	t_data->event_count = sizeof(drag_y) / sizeof(u16);
	t_data->event_index = FIXED_ID;

	if(filter)
		apply_filter(t_data->y, t_data->event_count);

	for(i=0; i<t_data->event_count; i++){
		t_data->x[i] = touch_test_dev->pdata->caps->x_max /2;
		TOUCH_INFO_MSG("[Touch Tester] pos[%4d,%4d]\n", t_data->x[i], t_data->y[i]);
	}
}

static void make_flick_y(struct test_data *t_data, int filter)
{
	int i;

	memset(t_data, 0, sizeof(*t_data));
	memcpy(t_data->y, flick_y, sizeof(flick_y));

	t_data->event_count = sizeof(flick_y) / sizeof(u16);
	t_data->event_index = FIXED_ID;

	if(filter)
		apply_filter(t_data->y, t_data->event_count);

	for(i=0; i<t_data->event_count; i++){
		t_data->x[i] = touch_test_dev->pdata->caps->x_max /2;
		TOUCH_INFO_MSG("[Touch Tester] pos[%4d,%4d]\n", t_data->x[i], t_data->y[i]);
	}
}

static ssize_t make_event_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;

	ret = sprintf(buf+ret, "\n# echo [type] [filter] [start] [end] [direction] [fixing_axis] [delta] [orientation] [report_period]> make_event\n");
	ret += sprintf(buf+ret, "\t-type: 1[drag_x], 2[drag_y], 3[flick_y], 0[user_define]\n");
	ret += sprintf(buf+ret, "\t-filter: 0[disable], 1[enable]\n");
	ret += sprintf(buf+ret, "\t-start: start point\n");
	ret += sprintf(buf+ret, "\t-end: end point\n");
	ret += sprintf(buf+ret, "\t-direction: 0[start -> end], 1[end -> start]\n");
	ret += sprintf(buf+ret, "\t-fixing_axis: if [start]/[end] are in x-axis, than [fixing_axis] is y-point\n");
	ret += sprintf(buf+ret, "\t-speed: gap\n");
	ret += sprintf(buf+ret, "\t-orienatation: 0[x-axis] 1[y-axis]\n");
	ret += sprintf(buf+ret, "\t-report_period: ns\n\n");

	return ret;
}

static ssize_t make_event_store(struct device *dev, struct device_attribute *attr, const char *buffer, size_t count)
{
	struct touch_tester_data *tt = dev_get_drvdata(dev);
	int start = 0;
	int end = 0;
	int direction = 0;
	int fixing_axis = 0;
	int delta = 0;
	int orientation = 0;
	int index = 0;
	int type = 0;
	int filter = 0;
	unsigned int report_period = 0;

	sscanf(buffer, "%d %d %d %d %d %d %d %d %d",
			&type, &filter, &start, &end, &direction, &fixing_axis, &delta, &orientation, &report_period);

	switch(type){
	case 1:
		make_drag_x(&tt->t_data ,filter);
		break;
	case 2:
		make_drag_y(&tt->t_data,filter);
		break;
	case 3:
		make_flick_y(&tt->t_data,filter);
		break;
	case 0:
		if (report_period)
			touch_test_dev->pdata->role->report_period = report_period;

		if((int)(tester_abs(end - start) / delta) >= MAX_EVENT) {
			TOUCH_ERR_MSG("[Touch Tester] Event too much\n");
			break;
		}

		if (direction == 0) {
			while(start <= end){
				if(orientation){
					tt->t_data.y[index] = start;
					tt->t_data.x[index] = fixing_axis;
				}
				else{
					tt->t_data.x[index] = start;
					tt->t_data.y[index] = fixing_axis;
				}

				index++;
				start += delta;
			}
		} else {
			while(start <= end){
				if(orientation){
					tt->t_data.y[index] = end;
					tt->t_data.x[index] = fixing_axis;
				}
				else{
					tt->t_data.x[index] = end;
					tt->t_data.y[index] = fixing_axis;
				}

				index++;
				end -= delta;
			}
		}

		tt->t_data.event_count = index;
		tt->t_data.event_index = 0;
		break;
	default:
		break;
	}

	return count;
}
DEVICE_ATTR(make_event, 0660, make_event_show, make_event_store);

static ssize_t touch_tester_store(struct device *dev, struct device_attribute *attr, const char *buffer, size_t count)
{
	struct touch_tester_data *tt = dev_get_drvdata(dev);

	tt->t_data.event_index = FIXED_ID;

	hrtimer_start(&tt->timer,
				ktime_set(0, touch_test_dev->pdata->role->report_period), HRTIMER_MODE_REL);

	return count;
}
DEVICE_ATTR(touch_tester, 0660, NULL, touch_tester_store);

static enum hrtimer_restart touch_timer_handler(struct hrtimer *timer)
{
	struct touch_tester_data *tt =
			container_of(timer, struct touch_tester_data, timer);

	if(tt->t_data.event_index < tt->t_data.event_count){
		if (tt->t_data.x[tt->t_data.event_index] != EVENT_IGNORE) {
			input_report_abs(tt->input_dev, ABS_MT_POSITION_X, tt->t_data.x[tt->t_data.event_index]);
			input_report_abs(tt->input_dev, ABS_MT_POSITION_Y, tt->t_data.y[tt->t_data.event_index]);
			input_report_abs(tt->input_dev, ABS_MT_PRESSURE, FIXED_PRESSURE);
			input_report_abs(tt->input_dev, ABS_MT_WIDTH_MAJOR, FIXED_WIDTH);
			input_report_abs(tt->input_dev, ABS_MT_TRACKING_ID, 0);
			input_mt_sync(tt->input_dev);
			input_sync(tt->input_dev);
		}

		tt->t_data.event_index++;

		hrtimer_start(&tt->timer,
				ktime_set(0, touch_test_dev->pdata->role->report_period), HRTIMER_MODE_REL);
	}
	else {
		input_mt_sync(tt->input_dev);
		input_sync(tt->input_dev);
	}

	return HRTIMER_NORESTART;
}


static int touch_tester_probe(struct platform_device *pdev)
{
	struct touch_tester_data *tt;
	int ret = 0;

	tt = kzalloc(sizeof(struct touch_tester_data), GFP_KERNEL);
	if (tt == NULL) {
		TOUCH_ERR_MSG("[Touch Tester] Can not allocate memory\n");
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	tt->input_dev = input_allocate_device();
	if (tt->input_dev == NULL) {
		TOUCH_ERR_MSG("[Touch Tester] Failed to allocate input device\n");
		ret = -ENOMEM;
		goto err_input_register_device_failed;
	}

	tt->input_dev->name = "touch_tester";

	set_bit(EV_SYN, tt->input_dev->evbit);
	set_bit(EV_ABS, tt->input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, tt->input_dev->propbit);

	input_set_abs_params(tt->input_dev, ABS_MT_POSITION_X, 0, MAX_X, 0, 0);
	input_set_abs_params(tt->input_dev, ABS_MT_POSITION_Y, 0, MAX_Y, 0, 0);
	input_set_abs_params(tt->input_dev, ABS_MT_PRESSURE, 0, MAX_P, 0, 0);
	input_set_abs_params(tt->input_dev, ABS_MT_WIDTH_MAJOR, 0, MAX_W, 0, 0);
	input_set_abs_params(tt->input_dev, ABS_MT_TRACKING_ID, 0, MAX_FINGER-1, 0, 0);

	ret = input_register_device(tt->input_dev);
	if (ret < 0) {
		TOUCH_ERR_MSG("[Touch Tester] Unable to register %s input device\n", tt->input_dev->name);
		goto err_input_register_device_failed;
	}

	hrtimer_init(&tt->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	tt->timer.function = touch_timer_handler;

	if(device_create_file(&pdev->dev, &dev_attr_touch_tester)){
		TOUCH_ERR_MSG("[Touch Tester] device_create_file\n");
		goto err_sys_fs_failed;
	}

	if(device_create_file(&pdev->dev, &dev_attr_make_event)){
		TOUCH_ERR_MSG("[Touch Tester] device_create_file\n");
		goto err_sys_fs_failed;
	}

	dev_set_drvdata(&pdev->dev, tt);

	return 0;

err_sys_fs_failed:
	input_free_device(tt->input_dev);
err_input_register_device_failed:
	kfree(tt);
err_alloc_data_failed:
	return ret;
}

static int touch_tester_remove(struct platform_device *pdev)
{
	struct touch_tester_data *tt = dev_get_drvdata(&pdev->dev);

	hrtimer_cancel(&tt->timer);
	input_free_device(tt->input_dev);
	device_remove_file(&pdev->dev,  &dev_attr_touch_tester);

	kfree(tt);

	return 0;
}

static struct platform_driver touch_tester_driver = {
	.probe 	 =	 touch_tester_probe,
	.remove	 =	 touch_tester_remove,
	.driver	 =	 {
		.name = "touch_tester",
	},
};

static struct platform_device touch_tester_device = {
	.name	 =   "touch_tester",
	.id	 =   -1,
};


static int __devinit touch_tester_init(void)
{
	platform_device_register(&touch_tester_device);
	return platform_driver_register(&touch_tester_driver);
}

static void __exit touch_tester_exit(void)
{
	platform_device_unregister(&touch_tester_device);
	platform_driver_unregister(&touch_tester_driver);
}
#if 1
late_initcall(touch_tester_init);
#else
module_init(touch_tester_init);
#endif
module_exit(touch_tester_exit);

MODULE_AUTHOR("YEHAN AHN <yehan.ahn@lge.com>");
MODULE_DESCRIPTION("LGE TOUCH TESTER");
MODULE_LICENSE("GPL");

