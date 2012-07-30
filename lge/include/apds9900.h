/* lge/include/apds9900.h
 *
 * Copyright (C) 2011 LGE, Inc.
 *
 * Author: platform.team@lge.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#ifdef CONFIG_LGE_SENSOR_PROXIMITY
struct apds9900_platform_data {
	int irq_num;
	int (*power)(unsigned char onoff);
	unsigned int  prox_int_low_threshold;
	unsigned int  prox_int_high_threshold;
	unsigned int  als_threshold_hsyteresis;
	unsigned int  ppcount;
	unsigned int  B;
	unsigned int  C;
	unsigned int  D;
	unsigned int  alsit;
	unsigned int  ga_value;
	unsigned int  df_value;	
};
#endif
