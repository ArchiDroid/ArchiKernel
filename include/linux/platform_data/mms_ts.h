/*
 * mms_ts.h - Platform data for Melfas MMS-series touch driver
 *
 * Copyright (C) 2013 Melfas Inc.
 * Author: DVK team <dvk@melfas.com>
 *
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#ifndef _LINUX_MMS_TOUCH_H
#define _LINUX_MMS_TOUCH_H

struct mms_ts_platform_data {
	int	max_x;
	int	max_y;

	//int	gpio_sda;
	//int	gpio_scl;
	int	gpio_resetb;
	int (*power)(struct device *dev, int on);
	//int	gpio_vdd_en;
};

#endif /* _LINUX_MMS_TOUCH_H */
