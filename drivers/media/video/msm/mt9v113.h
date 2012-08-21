/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#ifndef MT9V113_H
#define MT9V113_H

#include <linux/types.h>
#include <mach/camera.h>

enum mt9v113_wb_type{
	CAMERA_WB_MIN_MINUS_1,
	CAMERA_WB_AUTO = 1,  /* This list must match aeecamera.h */
	CAMERA_WB_CUSTOM,
	CAMERA_WB_INCANDESCENT,
	CAMERA_WB_FLUORESCENT,
	CAMERA_WB_DAYLIGHT,
	CAMERA_WB_CLOUDY_DAYLIGHT,
	CAMERA_WB_TWILIGHT,
	CAMERA_WB_SHADE,
	CAMERA_WB_MAX_PLUS_1
};

/* Enum Type for different ISO Mode supported */
enum mt9v113_iso_value {
	CAMERA_ISO_AUTO = 0,
	CAMERA_ISO_DEBLUR,
	CAMERA_ISO_100,
	CAMERA_ISO_200,
	CAMERA_ISO_400,
	CAMERA_ISO_MAX
};

enum mt9v113_antibanding_type {
	CAMERA_ANTIBANDING_60HZ,
	CAMERA_ANTIBANDING_50HZ,
	CAMERA_MAX_ANTIBANDING,
};

extern struct mt9v113_reg mt9v113_regs;

enum mt9v113_width {
	BYTE_LEN,
	WORD_LEN,
};

struct mt9v113_i2c_reg_conf {
	unsigned short waddr;
	unsigned short wdata;
};

struct mt9v113_reg {

	struct mt9v113_i2c_reg_conf const *init_tbl;
	uint16_t inittbl_size;

	struct mt9v113_i2c_reg_conf const *init_tbl1;
	uint16_t inittbl1_size;
	
	struct mt9v113_i2c_reg_conf const *init_tbl2;
	uint16_t inittbl2_size;
	
	// Effect
	struct mt9v113_i2c_reg_conf const *effect_default_tbl;
	uint16_t effect_default_tbl_size;
	struct mt9v113_i2c_reg_conf const *effect_mono_tbl;
	uint16_t effect_mono_tbl_size;
	struct mt9v113_i2c_reg_conf const *effect_sepia_tbl;
	uint16_t effect_sepia_tbl_size;
	struct mt9v113_i2c_reg_conf const *effect_aqua_tbl;
	uint16_t effect_aqua_tbl_size;
	struct mt9v113_i2c_reg_conf const *effect_negative_tbl;
	uint16_t effect_negative_tbl_size;
	struct mt9v113_i2c_reg_conf const *effect_solarization_tbl;
	uint16_t effect_solarization_tbl_size;
	// White balance
	struct mt9v113_i2c_reg_conf const *wb_default_tbl;
	uint16_t wb_default_tbl_size;
	struct mt9v113_i2c_reg_conf const *wb_sunny_tbl;
	uint16_t wb_sunny_tbl_size;
	struct mt9v113_i2c_reg_conf const *wb_cloudy_tbl;
	uint16_t wb_cloudy_tbl_size;
	struct mt9v113_i2c_reg_conf const *wb_fluorescent_tbl;
	uint16_t wb_fluorescent_tbl_size;
	struct mt9v113_i2c_reg_conf const *wb_incandescent_tbl;
	uint16_t wb_incandescent_tbl_size;
	// ISO
	struct mt9v113_i2c_reg_conf const *iso_default_tbl;
	uint16_t iso_default_tbl_size;
	struct mt9v113_i2c_reg_conf const *iso_160_tbl;
	uint16_t iso_160_tbl_size;
	struct mt9v113_i2c_reg_conf const *iso_200_tbl;
	uint16_t iso_200_tbl_size;
	struct mt9v113_i2c_reg_conf const *iso_400_tbl;
	uint16_t iso_400_tbl_size;
	// FPS
	struct mt9v113_i2c_reg_conf const *fps_fixed_15_tbl;
	uint16_t fps_fixed_15_tbl_size;
	struct mt9v113_i2c_reg_conf const *fps_fixed_30_tbl;
	uint16_t fps_fixed_30_tbl_size;
	struct mt9v113_i2c_reg_conf const *fps_auto_1030_tbl;
	uint16_t fps_auto_1030_tbl_size;
	struct mt9v113_i2c_reg_conf const *fps_auto_730_tbl;
	uint16_t fps_auto_730_tbl_size;
	
	// Change-config
	struct mt9v113_i2c_reg_conf const *change_config_tbl;
	uint16_t change_config_tbl_size;
	
};


#endif /* MT9V113_H */
