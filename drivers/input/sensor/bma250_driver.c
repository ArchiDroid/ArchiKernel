/*  Date: 2012/06/04 14:07:00
 *  Revision: 2.3
 */

/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2011 Bosch Sensortec GmbH
 * All Rights Reserved
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define SENSOR_NAME 			"bma250"
#define GRAVITY_EARTH                   9806550
#define ABSMIN_2G                       (-GRAVITY_EARTH * 2)
#define ABSMAX_2G                       (GRAVITY_EARTH * 2)
#define SLOPE_THRESHOLD_VALUE 		32
#define SLOPE_DURATION_VALUE 		1
#define INTERRUPT_LATCH_MODE 		13
#define INTERRUPT_ENABLE 		1
#define INTERRUPT_DISABLE 		0
#define MAP_SLOPE_INTERRUPT 		2
#define SLOPE_X_INDEX 			5
#define SLOPE_Y_INDEX 			6
#define SLOPE_Z_INDEX 			7

#define BMA250_MIN_DELAY		1
#define BMA250_DEFAULT_DELAY	200

#define BMA250_CHIP_ID			3
#define BMA250_RANGE_SET		0
#define BMA250_BW_SET			4


/*
 *
 *      register definitions
 *
 */

#define BMA250_CHIP_ID_REG                      0x00
#define BMA250_VERSION_REG                      0x01
#define BMA250_X_AXIS_LSB_REG                   0x02
#define BMA250_X_AXIS_MSB_REG                   0x03
#define BMA250_Y_AXIS_LSB_REG                   0x04
#define BMA250_Y_AXIS_MSB_REG                   0x05
#define BMA250_Z_AXIS_LSB_REG                   0x06
#define BMA250_Z_AXIS_MSB_REG                   0x07
#define BMA250_TEMP_RD_REG                      0x08
#define BMA250_STATUS1_REG                      0x09
#define BMA250_STATUS2_REG                      0x0A
#define BMA250_STATUS_TAP_SLOPE_REG             0x0B
#define BMA250_STATUS_ORIENT_HIGH_REG           0x0C
#define BMA250_RANGE_SEL_REG                    0x0F
#define BMA250_BW_SEL_REG                       0x10
#define BMA250_MODE_CTRL_REG                    0x11
#define BMA250_LOW_NOISE_CTRL_REG               0x12
#define BMA250_DATA_CTRL_REG                    0x13
#define BMA250_RESET_REG                        0x14
#define BMA250_INT_ENABLE1_REG                  0x16
#define BMA250_INT_ENABLE2_REG                  0x17
#define BMA250_INT1_PAD_SEL_REG                 0x19
#define BMA250_INT_DATA_SEL_REG                 0x1A
#define BMA250_INT2_PAD_SEL_REG                 0x1B
#define BMA250_INT_SRC_REG                      0x1E
#define BMA250_INT_SET_REG                      0x20
#define BMA250_INT_CTRL_REG                     0x21
#define BMA250_LOW_DURN_REG                     0x22
#define BMA250_LOW_THRES_REG                    0x23
#define BMA250_LOW_HIGH_HYST_REG                0x24
#define BMA250_HIGH_DURN_REG                    0x25
#define BMA250_HIGH_THRES_REG                   0x26
#define BMA250_SLOPE_DURN_REG                   0x27
#define BMA250_SLOPE_THRES_REG                  0x28
#define BMA250_TAP_PARAM_REG                    0x2A
#define BMA250_TAP_THRES_REG                    0x2B
#define BMA250_ORIENT_PARAM_REG                 0x2C
#define BMA250_THETA_BLOCK_REG                  0x2D
#define BMA250_THETA_FLAT_REG                   0x2E
#define BMA250_FLAT_HOLD_TIME_REG               0x2F
#define BMA250_STATUS_LOW_POWER_REG             0x31
#define BMA250_SELF_TEST_REG                    0x32
#define BMA250_EEPROM_CTRL_REG                  0x33
#define BMA250_SERIAL_CTRL_REG                  0x34
#define BMA250_CTRL_UNLOCK_REG                  0x35
#define BMA250_OFFSET_CTRL_REG                  0x36
#define BMA250_OFFSET_PARAMS_REG                0x37
#define BMA250_OFFSET_FILT_X_REG                0x38
#define BMA250_OFFSET_FILT_Y_REG                0x39
#define BMA250_OFFSET_FILT_Z_REG                0x3A
#define BMA250_OFFSET_UNFILT_X_REG              0x3B
#define BMA250_OFFSET_UNFILT_Y_REG              0x3C
#define BMA250_OFFSET_UNFILT_Z_REG              0x3D
#define BMA250_SPARE_0_REG                      0x3E
#define BMA250_SPARE_1_REG                      0x3F




#define BMA250_ACC_X_LSB__POS           6
#define BMA250_ACC_X_LSB__LEN           2
#define BMA250_ACC_X_LSB__MSK           0xC0
#define BMA250_ACC_X_LSB__REG           BMA250_X_AXIS_LSB_REG

#define BMA250_ACC_X_MSB__POS           0
#define BMA250_ACC_X_MSB__LEN           8
#define BMA250_ACC_X_MSB__MSK           0xFF
#define BMA250_ACC_X_MSB__REG           BMA250_X_AXIS_MSB_REG

#define BMA250_ACC_Y_LSB__POS           6
#define BMA250_ACC_Y_LSB__LEN           2
#define BMA250_ACC_Y_LSB__MSK           0xC0
#define BMA250_ACC_Y_LSB__REG           BMA250_Y_AXIS_LSB_REG

#define BMA250_ACC_Y_MSB__POS           0
#define BMA250_ACC_Y_MSB__LEN           8
#define BMA250_ACC_Y_MSB__MSK           0xFF
#define BMA250_ACC_Y_MSB__REG           BMA250_Y_AXIS_MSB_REG

#define BMA250_ACC_Z_LSB__POS           6
#define BMA250_ACC_Z_LSB__LEN           2
#define BMA250_ACC_Z_LSB__MSK           0xC0
#define BMA250_ACC_Z_LSB__REG           BMA250_Z_AXIS_LSB_REG

#define BMA250_ACC_Z_MSB__POS           0
#define BMA250_ACC_Z_MSB__LEN           8
#define BMA250_ACC_Z_MSB__MSK           0xFF
#define BMA250_ACC_Z_MSB__REG           BMA250_Z_AXIS_MSB_REG

#define BMA250_RANGE_SEL__POS             0
#define BMA250_RANGE_SEL__LEN             4
#define BMA250_RANGE_SEL__MSK             0x0F
#define BMA250_RANGE_SEL__REG             BMA250_RANGE_SEL_REG

#define BMA250_BANDWIDTH__POS             0
#define BMA250_BANDWIDTH__LEN             5
#define BMA250_BANDWIDTH__MSK             0x1F
#define BMA250_BANDWIDTH__REG             BMA250_BW_SEL_REG

#define BMA250_EN_LOW_POWER__POS          6
#define BMA250_EN_LOW_POWER__LEN          1
#define BMA250_EN_LOW_POWER__MSK          0x40
#define BMA250_EN_LOW_POWER__REG          BMA250_MODE_CTRL_REG

#define BMA250_EN_SUSPEND__POS            7
#define BMA250_EN_SUSPEND__LEN            1
#define BMA250_EN_SUSPEND__MSK            0x80
#define BMA250_EN_SUSPEND__REG            BMA250_MODE_CTRL_REG


#define BMA250_UNLOCK_EE_WRITE_SETTING__POS     0
#define BMA250_UNLOCK_EE_WRITE_SETTING__LEN     1
#define BMA250_UNLOCK_EE_WRITE_SETTING__MSK     0x01
#define BMA250_UNLOCK_EE_WRITE_SETTING__REG     BMA250_EEPROM_CTRL_REG

#define BMA250_START_EE_WRITE_SETTING__POS      1
#define BMA250_START_EE_WRITE_SETTING__LEN      1
#define BMA250_START_EE_WRITE_SETTING__MSK      0x02
#define BMA250_START_EE_WRITE_SETTING__REG      BMA250_EEPROM_CTRL_REG

#define BMA250_EE_WRITE_SETTING_S__POS          2
#define BMA250_EE_WRITE_SETTING_S__LEN          1
#define BMA250_EE_WRITE_SETTING_S__MSK          0x04
#define BMA250_EE_WRITE_SETTING_S__REG          BMA250_EEPROM_CTRL_REG

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
#define BMA250_EN_SOFT_RESET__POS         0
#define BMA250_EN_SOFT_RESET__LEN         8
#define BMA250_EN_SOFT_RESET__MSK         0xFF
#define BMA250_EN_SOFT_RESET__REG         BMA250_RESET_REG

#define BMA250_EN_SOFT_RESET_VALUE        0xB6
#endif	


#define BMA250_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)


#define BMA250_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))


/* range and bandwidth */

#define BMA250_RANGE_2G                 3
#define BMA250_RANGE_4G                 5
#define BMA250_RANGE_8G                 8
#define BMA250_RANGE_16G                12


#define BMA250_BW_7_81HZ        0x08
#define BMA250_BW_15_63HZ       0x09
#define BMA250_BW_31_25HZ       0x0A
#define BMA250_BW_62_50HZ       0x0B
#define BMA250_BW_125HZ         0x0C
#define BMA250_BW_250HZ         0x0D
#define BMA250_BW_500HZ         0x0E
#define BMA250_BW_1000HZ        0x0F

/* mode settings */

#define BMA250_MODE_NORMAL      0
#define BMA250_MODE_LOWPOWER    1
#define BMA250_MODE_SUSPEND     2


#define BMA250_EN_SELF_TEST__POS                0
#define BMA250_EN_SELF_TEST__LEN                2
#define BMA250_EN_SELF_TEST__MSK                0x03
#define BMA250_EN_SELF_TEST__REG                BMA250_SELF_TEST_REG



#define BMA250_NEG_SELF_TEST__POS               2
#define BMA250_NEG_SELF_TEST__LEN               1
#define BMA250_NEG_SELF_TEST__MSK               0x04
#define BMA250_NEG_SELF_TEST__REG               BMA250_SELF_TEST_REG

#define BMA250_EN_FAST_COMP__POS                5
#define BMA250_EN_FAST_COMP__LEN                2
#define BMA250_EN_FAST_COMP__MSK                0x60
#define BMA250_EN_FAST_COMP__REG                BMA250_OFFSET_CTRL_REG

#define BMA250_FAST_COMP_RDY_S__POS             4
#define BMA250_FAST_COMP_RDY_S__LEN             1
#define BMA250_FAST_COMP_RDY_S__MSK             0x10
#define BMA250_FAST_COMP_RDY_S__REG             BMA250_OFFSET_CTRL_REG

#define BMA250_COMP_TARGET_OFFSET_X__POS        1
#define BMA250_COMP_TARGET_OFFSET_X__LEN        2
#define BMA250_COMP_TARGET_OFFSET_X__MSK        0x06
#define BMA250_COMP_TARGET_OFFSET_X__REG        BMA250_OFFSET_PARAMS_REG

#define BMA250_COMP_TARGET_OFFSET_Y__POS        3
#define BMA250_COMP_TARGET_OFFSET_Y__LEN        2
#define BMA250_COMP_TARGET_OFFSET_Y__MSK        0x18
#define BMA250_COMP_TARGET_OFFSET_Y__REG        BMA250_OFFSET_PARAMS_REG

#define BMA250_COMP_TARGET_OFFSET_Z__POS        5
#define BMA250_COMP_TARGET_OFFSET_Z__LEN        2
#define BMA250_COMP_TARGET_OFFSET_Z__MSK        0x60
#define BMA250_COMP_TARGET_OFFSET_Z__REG        BMA250_OFFSET_PARAMS_REG

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
#define BMA250_SHAKING_DETECT_THRESHOLD	(50)	/* threshold of shaking detection under 2G */
#endif


static const u8 bma250_valid_range[] = {
	BMA250_RANGE_2G,
	BMA250_RANGE_4G,
	BMA250_RANGE_8G,
	BMA250_RANGE_16G,
};

static const u8 bma250_valid_bw[] = {
	BMA250_BW_7_81HZ,
	BMA250_BW_15_63HZ,
	BMA250_BW_31_25HZ,
	BMA250_BW_62_50HZ,
	BMA250_BW_125HZ,
	BMA250_BW_250HZ,
	BMA250_BW_500HZ,
	BMA250_BW_1000HZ,
};

struct bma250acc{
	s16	x,
		y,
		z;
} ;

struct bma250_data {
	struct i2c_client *bma250_client;
	atomic_t delay;
	atomic_t enable;
	unsigned char mode;
	struct input_dev *input;
	struct bma250acc value;
	struct mutex enable_mutex;
	struct mutex mode_mutex;
	struct delayed_work work;
	struct work_struct irq_work;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif

	atomic_t selftest_result;
#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
	atomic_t fast_calib_x_rslt;
	atomic_t fast_calib_y_rslt;
	atomic_t fast_calib_z_rslt;
#endif	
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-02-09*/
	atomic_t fast_calib_rslt;
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-02-09*/
};
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-02-09*/
static atomic_t bma250_diag = ATOMIC_INIT(0);
static atomic_t bma250_cnt = ATOMIC_INIT(0);
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-02-09*/

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bma250_early_suspend(struct early_suspend *h);
static void bma250_late_resume(struct early_suspend *h);
#endif

static int bma250_smbus_read_byte(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_byte_data(client, reg_addr);
	if (dummy < 0)
		return -1;
	*data = dummy & 0x000000ff;

	return 0;
}

static int bma250_smbus_write_byte(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_write_byte_data(client, reg_addr, *data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int bma250_smbus_read_byte_block(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int bma250_set_mode(struct i2c_client *client, unsigned char Mode)
{
	int comres = 0;
	unsigned char data1;

	if (client == NULL) {
		comres = -1;
	} else{
		if (Mode < 3) {
			comres = bma250_smbus_read_byte(client,
					BMA250_EN_LOW_POWER__REG, &data1);
			switch (Mode) {
			case BMA250_MODE_NORMAL:
				data1  = BMA250_SET_BITSLICE(data1,
						BMA250_EN_LOW_POWER, 0);
				data1  = BMA250_SET_BITSLICE(data1,
						BMA250_EN_SUSPEND, 0);
				break;
			case BMA250_MODE_LOWPOWER:
				data1  = BMA250_SET_BITSLICE(data1,
						BMA250_EN_LOW_POWER, 1);
				data1  = BMA250_SET_BITSLICE(data1,
						BMA250_EN_SUSPEND, 0);
				break;
			case BMA250_MODE_SUSPEND:
				data1  = BMA250_SET_BITSLICE(data1,
						BMA250_EN_LOW_POWER, 0);
				data1  = BMA250_SET_BITSLICE(data1,
						BMA250_EN_SUSPEND, 1);
				break;
			default:
				break;
			}

			comres += bma250_smbus_write_byte(client,
					BMA250_EN_LOW_POWER__REG, &data1);
		} else{
			comres = -1;
		}
	}

	return comres;
}

static int bma250_get_mode(struct i2c_client *client, unsigned char *Mode)
{
	int comres = 0;

	if (client == NULL) {
		comres = -1;
	} else{
		comres = bma250_smbus_read_byte(client,
				BMA250_EN_LOW_POWER__REG, Mode);
		*Mode  = (*Mode) >> 6;
	}

	return comres;
}

static int bma250_set_range(struct i2c_client *client, unsigned char Range)
{
	int comres = 0;
	unsigned char data1;
	int i;

	if (client == NULL) {
		comres = -1;
	} else{
		for (i = 0; i < ARRAY_SIZE(bma250_valid_range); i++) {
			if (bma250_valid_range[i] == Range)
				break;
		}

		if (ARRAY_SIZE(bma250_valid_range) > i) {
			comres = bma250_smbus_read_byte(client,
					BMA250_RANGE_SEL_REG, &data1);

			data1  = BMA250_SET_BITSLICE(data1,
					BMA250_RANGE_SEL, Range);

			comres += bma250_smbus_write_byte(client,
					BMA250_RANGE_SEL_REG, &data1);
		} else {
			comres = -EINVAL;
		}
	}

	return comres;
}

static int bma250_get_range(struct i2c_client *client, unsigned char *Range)
{
	int comres = 0;
	unsigned char data;

	if (client == NULL) {
		comres = -1;
	} else{
		comres = bma250_smbus_read_byte(client, BMA250_RANGE_SEL__REG,
				&data);
		data = BMA250_GET_BITSLICE(data, BMA250_RANGE_SEL);
		*Range = data;
	}

	return comres;
}


static int bma250_set_bandwidth(struct i2c_client *client, unsigned char BW)
{
	int comres = 0;
	unsigned char data;
	int i = 0;

	if (client == NULL) {
		comres = -1;
	} else {

		for (i = 0; i < ARRAY_SIZE(bma250_valid_bw); i++) {
			if (bma250_valid_bw[i] == BW)
				break;
		}

		if (ARRAY_SIZE(bma250_valid_bw) > i) {
			comres = bma250_smbus_read_byte(client,
					BMA250_BANDWIDTH__REG, &data);
			data = BMA250_SET_BITSLICE(data, BMA250_BANDWIDTH, BW);
			comres += bma250_smbus_write_byte(client,
					BMA250_BANDWIDTH__REG, &data);
		} else {
			comres = -EINVAL;
		}
	}

	return comres;
}

static int bma250_get_bandwidth(struct i2c_client *client, unsigned char *BW)
{
	int comres = 0;
	unsigned char data;

	if (client == NULL) {
		comres = -1;
	} else{
		comres = bma250_smbus_read_byte(client, BMA250_BANDWIDTH__REG,
				&data);
		data = BMA250_GET_BITSLICE(data, BMA250_BANDWIDTH);
		if (data < BMA250_BW_7_81HZ)
			*BW = BMA250_BW_7_81HZ;
		else if (data > BMA250_BW_1000HZ)
			*BW = BMA250_BW_1000HZ;
		else
			*BW = data;
	}

	return comres;
}

static int bma250_read_accel_xyz(struct i2c_client *client,
		struct bma250acc *acc)
{
	int comres;
	unsigned char data[6];
	if (client == NULL) {
		comres = -1;
	} else{
		comres = bma250_smbus_read_byte_block(client,
				BMA250_ACC_X_LSB__REG, data, 6);

		acc->x = BMA250_GET_BITSLICE(data[0], BMA250_ACC_X_LSB)
			|(BMA250_GET_BITSLICE(data[1], BMA250_ACC_X_MSB)
					<<BMA250_ACC_X_LSB__LEN);
		acc->x = acc->x << (sizeof(short)*8-(BMA250_ACC_X_LSB__LEN
					+ BMA250_ACC_X_MSB__LEN));
		acc->x = acc->x >> (sizeof(short)*8-(BMA250_ACC_X_LSB__LEN
					+ BMA250_ACC_X_MSB__LEN));
		acc->y = BMA250_GET_BITSLICE(data[2], BMA250_ACC_Y_LSB)
			| (BMA250_GET_BITSLICE(data[3], BMA250_ACC_Y_MSB)
					<<BMA250_ACC_Y_LSB__LEN);
		acc->y = acc->y << (sizeof(short)*8-(BMA250_ACC_Y_LSB__LEN
					+ BMA250_ACC_Y_MSB__LEN));
		acc->y = acc->y >> (sizeof(short)*8-(BMA250_ACC_Y_LSB__LEN
					+ BMA250_ACC_Y_MSB__LEN));

		acc->z = BMA250_GET_BITSLICE(data[4], BMA250_ACC_Z_LSB)
			| (BMA250_GET_BITSLICE(data[5], BMA250_ACC_Z_MSB)
					<<BMA250_ACC_Z_LSB__LEN);
		acc->z = acc->z << (sizeof(short)*8-(BMA250_ACC_Z_LSB__LEN
					+ BMA250_ACC_Z_MSB__LEN));
		acc->z = acc->z >> (sizeof(short)*8-(BMA250_ACC_Z_LSB__LEN
					+ BMA250_ACC_Z_MSB__LEN));
	}

	return comres;
}

static int bma250_set_ee_w(struct i2c_client *client, unsigned char eew)
{
	int comres = 0;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
			BMA250_UNLOCK_EE_WRITE_SETTING__REG, &data);
	data = BMA250_SET_BITSLICE(data, BMA250_UNLOCK_EE_WRITE_SETTING, eew);
	comres = bma250_smbus_write_byte(client,
			BMA250_UNLOCK_EE_WRITE_SETTING__REG, &data);
	return comres;
}


static int bma250_set_ee_prog_trig(struct i2c_client *client)
{
	int comres = 0;
	unsigned char data;
	unsigned char eeprog;
	eeprog = 0x01;

	comres = bma250_smbus_read_byte(client,
			BMA250_START_EE_WRITE_SETTING__REG, &data);
	data = BMA250_SET_BITSLICE(data,
				BMA250_START_EE_WRITE_SETTING, eeprog);
	comres = bma250_smbus_write_byte(client,
			BMA250_START_EE_WRITE_SETTING__REG, &data);
	return comres;
}

static int bma250_get_eeprom_writing_status(struct i2c_client *client,
							unsigned char *eewrite)
{
	int comres = 0;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
				BMA250_EEPROM_CTRL_REG, &data);
	data = BMA250_GET_BITSLICE(data, BMA250_EE_WRITE_SETTING_S);
	*eewrite = data;

	return comres;
}


static void bma250_work_func(struct work_struct *work)
{
	struct bma250_data *bma250 = container_of((struct delayed_work *)work,
			struct bma250_data, work);
	static struct bma250acc acc;
	unsigned long delay = msecs_to_jiffies(atomic_read(&bma250->delay));

	bma250_read_accel_xyz(bma250->bma250_client, &acc);
	input_report_abs(bma250->input, ABS_X, acc.x);
	input_report_abs(bma250->input, ABS_Y, acc.y);
	input_report_abs(bma250->input, ABS_Z, acc.z);
	input_sync(bma250->input);
	bma250->value = acc;
	schedule_delayed_work(&bma250->work, delay);
}

static ssize_t bma250_range_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_get_range(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma250_range_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma250_set_range(bma250->bma250_client, (unsigned char) data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma250_bandwidth_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_get_bandwidth(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma250_bandwidth_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma250_set_bandwidth(bma250->bma250_client,
				(unsigned char) data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma250_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_get_mode(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma250_mode_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma250_set_mode(bma250->bma250_client, (unsigned char) data) < 0)
		return -EINVAL;

	return count;
}


static ssize_t bma250_value_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bma250_data *bma250 = input_get_drvdata(input);
	int err;

	err = sprintf(buf, "%d %d %d\n", bma250->value.x, bma250->value.y,
			bma250->value.z);

	return err;
}

static ssize_t bma250_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma250->delay));

}

static ssize_t bma250_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (data <= 0)
		return -EINVAL;

	if (data < BMA250_MIN_DELAY)
		data = BMA250_MIN_DELAY;

	atomic_set(&bma250->delay, (unsigned int) data);

	return count;
}


static ssize_t bma250_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma250->enable));

}

static void bma250_set_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);
	int pre_enable = atomic_read(&bma250->enable);

	mutex_lock(&bma250->enable_mutex);
	if (enable) {
		if (pre_enable == 0) {
			schedule_delayed_work(&bma250->work,
					msecs_to_jiffies(
						atomic_read(&bma250->delay)));
			atomic_set(&bma250->enable, 1);
		}

	} else {
		if (pre_enable == 1) {
			cancel_delayed_work_sync(&bma250->work);
			atomic_set(&bma250->enable, 0);
		}
	}
	mutex_unlock(&bma250->enable_mutex);

}

static ssize_t bma250_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if ((data == 0) || (data == 1))
		bma250_set_enable(dev, data);

	return count;
}

static ssize_t bma250_update_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	bma250_read_accel_xyz(bma250->bma250_client, &bma250->value);
	return count;
}

static int bma250_set_selftest_st(struct i2c_client *client,
		unsigned char selftest)
{
	int comres = 0;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
			BMA250_EN_SELF_TEST__REG, &data);
	data = BMA250_SET_BITSLICE(data,
			BMA250_EN_SELF_TEST, selftest);
	comres = bma250_smbus_write_byte(client,
			BMA250_EN_SELF_TEST__REG, &data);

	return comres;
}

static int bma250_set_selftest_stn(struct i2c_client *client, unsigned char stn)
{
	int comres = 0;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
			BMA250_NEG_SELF_TEST__REG, &data);
	data = BMA250_SET_BITSLICE(data, BMA250_NEG_SELF_TEST, stn);
	comres = bma250_smbus_write_byte(client,
			BMA250_NEG_SELF_TEST__REG, &data);

	return comres;
}

static int bma250_read_accel_x(struct i2c_client *client, short *a_x)
{
	int comres;
	unsigned char data[2];
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-02-09*/
	if (atomic_read(&bma250_diag) == 1)
		 atomic_inc(&bma250_cnt);
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-02-09*/
	comres = bma250_smbus_read_byte_block(client,
			BMA250_ACC_X_LSB__REG, data, 2);
	*a_x = BMA250_GET_BITSLICE(data[0], BMA250_ACC_X_LSB)
		| (BMA250_GET_BITSLICE(data[1], BMA250_ACC_X_MSB)
				<<BMA250_ACC_X_LSB__LEN);
	*a_x = *a_x << (sizeof(short)*8
			-(BMA250_ACC_X_LSB__LEN+BMA250_ACC_X_MSB__LEN));
	*a_x = *a_x >> (sizeof(short)*8
			-(BMA250_ACC_X_LSB__LEN+BMA250_ACC_X_MSB__LEN));

	return comres;
}
static int bma250_read_accel_y(struct i2c_client *client, short *a_y)
{
	int comres;
	unsigned char data[2];
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-02-09*/
	if (atomic_read(&bma250_diag) == 1)
		 atomic_inc(&bma250_cnt);
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-02-09*/
	comres = bma250_smbus_read_byte_block(client,
			BMA250_ACC_Y_LSB__REG, data, 2);
	*a_y = BMA250_GET_BITSLICE(data[0], BMA250_ACC_Y_LSB)
		| (BMA250_GET_BITSLICE(data[1], BMA250_ACC_Y_MSB)
				<<BMA250_ACC_Y_LSB__LEN);
	*a_y = *a_y << (sizeof(short)*8
			-(BMA250_ACC_Y_LSB__LEN+BMA250_ACC_Y_MSB__LEN));
	*a_y = *a_y >> (sizeof(short)*8
			-(BMA250_ACC_Y_LSB__LEN+BMA250_ACC_Y_MSB__LEN));

	return comres;
}

static int bma250_read_accel_z(struct i2c_client *client, short *a_z)
{
	int comres;
	unsigned char data[2];
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-02-09*/
	if (atomic_read(&bma250_diag) == 1)
		 atomic_inc(&bma250_cnt);
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-02-09*/
	comres = bma250_smbus_read_byte_block(client,
			BMA250_ACC_Z_LSB__REG, data, 2);
	*a_z = BMA250_GET_BITSLICE(data[0], BMA250_ACC_Z_LSB)
		| BMA250_GET_BITSLICE(data[1], BMA250_ACC_Z_MSB)
		<<BMA250_ACC_Z_LSB__LEN;
	*a_z = *a_z << (sizeof(short)*8
			-(BMA250_ACC_Z_LSB__LEN+BMA250_ACC_Z_MSB__LEN));
	*a_z = *a_z >> (sizeof(short)*8
			-(BMA250_ACC_Z_LSB__LEN+BMA250_ACC_Z_MSB__LEN));

	return comres;
}

static ssize_t bma250_selftest_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma250->selftest_result));
}

static ssize_t bma250_selftest_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	unsigned char clear_value = 0;
	int error;
	short value1 = 0;
	short value2 = 0;
	short diff = 0;
	unsigned long result = 0;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (data != 1)
		return -EINVAL;
	/* set to 2 G range */
	if (bma250_set_range(bma250->bma250_client, BMA250_RANGE_2G) < 0)
		return -EINVAL;

	bma250_smbus_write_byte(bma250->bma250_client, 0x32, &clear_value);
	/* 1 for x-axis*/
	bma250_set_selftest_st(bma250->bma250_client, 1);
	/* positive direction*/
	bma250_set_selftest_stn(bma250->bma250_client, 0);
	mdelay(10);
	bma250_read_accel_x(bma250->bma250_client, &value1);
	/* negative direction*/
	bma250_set_selftest_stn(bma250->bma250_client, 1);
	mdelay(10);
	bma250_read_accel_x(bma250->bma250_client, &value2);
	diff = value1-value2;

	printk(KERN_INFO "diff x is %d, value1 is %d, value2 is %d\n",
			diff, value1, value2);

	if (abs(diff) < 204)
		result |= 1;

	/* 2 for y-axis*/
	bma250_set_selftest_st(bma250->bma250_client, 2);
	/* positive direction*/
	bma250_set_selftest_stn(bma250->bma250_client, 0);
	mdelay(10);
	bma250_read_accel_y(bma250->bma250_client, &value1);
	/* negative direction*/
	bma250_set_selftest_stn(bma250->bma250_client, 1);
	mdelay(10);
	bma250_read_accel_y(bma250->bma250_client, &value2);
	diff = value1-value2;
	printk(KERN_INFO "diff y is %d, value1 is %d, value2 is %d\n",
			diff, value1, value2);
	if (abs(diff) < 204)
		result |= 2;

	/* 3 for z-axis*/
	bma250_set_selftest_st(bma250->bma250_client, 3);
	/* positive direction*/
	bma250_set_selftest_stn(bma250->bma250_client, 0);
	mdelay(10);
	bma250_read_accel_z(bma250->bma250_client, &value1);
	/* negative direction*/
	bma250_set_selftest_stn(bma250->bma250_client, 1);
	mdelay(10);
	bma250_read_accel_z(bma250->bma250_client, &value2);
	diff = value1 - value2;

	printk(KERN_INFO "diff z is %d, value1 is %d, value2 is %d\n",
			diff, value1, value2);
	if (abs(diff) < 102)
		result |= 4;

	atomic_set(&bma250->selftest_result, (unsigned int) result);

	printk(KERN_INFO "self test finished\n");


	return count;
}

static int bma250_set_offset_target_x(struct i2c_client *client,
		unsigned char offsettarget)
{
	int comres = 0;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
			BMA250_COMP_TARGET_OFFSET_X__REG, &data);
	data = BMA250_SET_BITSLICE(data,
			BMA250_COMP_TARGET_OFFSET_X, offsettarget);
	comres = bma250_smbus_write_byte(client,
			BMA250_COMP_TARGET_OFFSET_X__REG, &data);

	return comres;
}

#ifndef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
static int bma250_get_offset_target_x(struct i2c_client *client,
		unsigned char *offsettarget)
{
	int comres = 0 ;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
			BMA250_OFFSET_PARAMS_REG, &data);
	data = BMA250_GET_BITSLICE(data, BMA250_COMP_TARGET_OFFSET_X);
	*offsettarget = data;

	return comres;
}
#endif

static int bma250_set_offset_target_y(struct i2c_client *client,
		unsigned char offsettarget)
{
	int comres = 0;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
			BMA250_COMP_TARGET_OFFSET_Y__REG, &data);
	data = BMA250_SET_BITSLICE(data,
			BMA250_COMP_TARGET_OFFSET_Y, offsettarget);
	comres = bma250_smbus_write_byte(client,
			BMA250_COMP_TARGET_OFFSET_Y__REG, &data);

	return comres;
}

#ifndef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/


static int bma250_get_offset_target_y(struct i2c_client *client,
		unsigned char *offsettarget)
{
	int comres = 0 ;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
			BMA250_OFFSET_PARAMS_REG, &data);
	data = BMA250_GET_BITSLICE(data, BMA250_COMP_TARGET_OFFSET_Y);
	*offsettarget = data;

	return comres;
}
#endif

static int bma250_set_offset_target_z(struct i2c_client *client,
		unsigned char offsettarget)
{
	int comres = 0;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
			BMA250_COMP_TARGET_OFFSET_Z__REG, &data);
	data = BMA250_SET_BITSLICE(data,
			BMA250_COMP_TARGET_OFFSET_Z, offsettarget);
	comres = bma250_smbus_write_byte(client,
			BMA250_COMP_TARGET_OFFSET_Z__REG, &data);

	return comres;
}

#ifndef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/

static int bma250_get_offset_target_z(struct i2c_client *client,
		unsigned char *offsettarget)
{
	int comres = 0 ;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
			BMA250_OFFSET_PARAMS_REG, &data);
	data = BMA250_GET_BITSLICE(data, BMA250_COMP_TARGET_OFFSET_Z);
	*offsettarget = data;

	return comres;
}
#endif

static int bma250_set_offset_filt_x(struct i2c_client *client, unsigned char
		offsetfilt)
{
	int comres = 0;
	unsigned char data;

	data =  offsetfilt;
	comres = bma250_smbus_write_byte(client, BMA250_OFFSET_FILT_X_REG,
						&data);

	return comres;
}


static int bma250_get_offset_filt_x(struct i2c_client *client, unsigned char
						*offsetfilt)
{
	int comres = 0;
	unsigned char data;

	comres = bma250_smbus_read_byte(client, BMA250_OFFSET_FILT_X_REG,
						&data);
	*offsetfilt = data;

	return comres;
}

static int bma250_set_offset_filt_y(struct i2c_client *client, unsigned char
						offsetfilt)
{
	int comres = 0;
	unsigned char data;

	data =  offsetfilt;
	comres = bma250_smbus_write_byte(client, BMA250_OFFSET_FILT_Y_REG,
						&data);

	return comres;
}

static int bma250_get_offset_filt_y(struct i2c_client *client, unsigned char
						*offsetfilt)
{
	int comres = 0;
	unsigned char data;

	comres = bma250_smbus_read_byte(client, BMA250_OFFSET_FILT_Y_REG,
						&data);
	*offsetfilt = data;

	return comres;
}

static int bma250_set_offset_filt_z(struct i2c_client *client, unsigned char
						offsetfilt)
{
	int comres = 0;
	unsigned char data;

	data =  offsetfilt;
	comres = bma250_smbus_write_byte(client, BMA250_OFFSET_FILT_Z_REG,
						&data);

	return comres;
}

static int bma250_get_offset_filt_z(struct i2c_client *client, unsigned char
						*offsetfilt)
{
	int comres = 0;
	unsigned char data;

	comres = bma250_smbus_read_byte(client, BMA250_OFFSET_FILT_Z_REG,
						&data);
	*offsetfilt = data;

	return comres;
}


static int bma250_get_cal_ready(struct i2c_client *client,
		unsigned char *calrdy)
{
	int comres = 0 ;
	unsigned char data;

	comres = bma250_smbus_read_byte(client,
			BMA250_OFFSET_CTRL_REG, &data);
	data = BMA250_GET_BITSLICE(data, BMA250_FAST_COMP_RDY_S);
	*calrdy = data;

	return comres;
}

static int bma250_set_cal_trigger(struct i2c_client *client,
		unsigned char caltrigger)
{
	int comres = 0;
	unsigned char data;

/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-02-09 sensor SURV shake detection*/
	struct bma250_data *bma250 = i2c_get_clientdata(client);
	if(atomic_read(&bma250->fast_calib_rslt) != 0)
	{
		atomic_set(&bma250->fast_calib_rslt, 0);
		printk(KERN_INFO "[set] bma250->fast_calib_rslt:%d\n",atomic_read(&bma250->fast_calib_rslt));
	}
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-02-09 sensor SURV shake detection*/

	comres = bma250_smbus_read_byte(client,
			BMA250_EN_FAST_COMP__REG, &data);
	data = BMA250_SET_BITSLICE(data,
			BMA250_EN_FAST_COMP, caltrigger);
	comres = bma250_smbus_write_byte(client,
			BMA250_EN_FAST_COMP__REG, &data);

	return comres;
}

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
static int bma250_soft_reset(struct i2c_client *client)
{
	int comres = 0;
	unsigned char data = BMA250_EN_SOFT_RESET_VALUE ;

	comres = bma250_smbus_write_byte(client, BMA250_EN_SOFT_RESET__REG,
					&data);

	return comres;
}


static ssize_t bma250_softreset_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_soft_reset(bma250->bma250_client) < 0)
		return -EINVAL;

	return count;
}
#endif


static ssize_t bma250_fast_calibration_x_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
		return sprintf(buf, "%d\n", atomic_read(&bma250->fast_calib_x_rslt));	
#if 0
	if (bma250_get_offset_target_x(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
#endif
#endif
}

static ssize_t bma250_fast_calibration_x_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	signed char tmp;
	unsigned int timeout = 0;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);
#ifdef CONFIG_MACH_LGE
	/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
	* sensor SURV shake detection" 
	*/
	struct bma250acc acc_cal;
	struct bma250acc acc_cal_pre; 
#endif
	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
	bma250_read_accel_xyz(bma250->bma250_client, &acc_cal_pre);
	mdelay(50);
#endif
	if (bma250_set_offset_target_x(bma250->bma250_client,
				(unsigned char)data) < 0)
		return -EINVAL;

	if (bma250_set_cal_trigger(bma250->bma250_client, 1) < 0)
		return -EINVAL;
#ifdef CONFIG_MACH_LGE
	/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
	* sensor SURV shake detection" 
	*/
	atomic_set(&bma250->fast_calib_x_rslt, 0);
#endif
	do {
		mdelay(2);
		bma250_get_cal_ready(bma250->bma250_client, &tmp);
#ifdef CONFIG_MACH_LGE
		/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
		* sensor SURV shake detection" 
		*/

		bma250_read_accel_xyz(bma250->bma250_client, &acc_cal);

		if( (tmp == 0)	&&
			((abs(acc_cal.x - acc_cal_pre.x) > BMA250_SHAKING_DETECT_THRESHOLD)
				|| (abs((acc_cal.y - acc_cal_pre.y)) > BMA250_SHAKING_DETECT_THRESHOLD)
				|| (abs((acc_cal.z - acc_cal_pre.z)) > BMA250_SHAKING_DETECT_THRESHOLD))
		  )
		{	

			bma250_soft_reset(bma250->bma250_client);
			printk(KERN_INFO "===============moved x===============\n");
			printk(KERN_INFO "(%d, %d, %d) (%d, %d, %d)\n", acc_cal_pre.x,  acc_cal_pre.y,  acc_cal_pre.z, acc_cal.x,acc_cal.y,acc_cal.z );
			return -EINVAL;
		}
		else
		{
			acc_cal_pre.x = acc_cal.x;
			acc_cal_pre.y = acc_cal.y;
			acc_cal_pre.z = acc_cal.z;
		}

#endif
		printk(KERN_INFO "[x] wait 2ms and got cal ready flag is %d\n",
				tmp);
		timeout++;
		if (timeout == 1000) {
			printk(KERN_INFO "get fast calibration ready error\n");
			return -EINVAL;
		};

	} while (tmp == 0);

	atomic_set(&bma250->fast_calib_x_rslt, 1);

	printk(KERN_INFO "x axis fast calibration finished\n");
	return count;
}

static ssize_t bma250_fast_calibration_y_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{


	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);
#ifdef CONFIG_MACH_LGE
	/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
	* sensor SURV shake detection" 
	*/

	return sprintf(buf, "%d\n", atomic_read(&bma250->fast_calib_y_rslt));	

#if 0
	if (bma250_get_offset_target_y(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
#endif
#endif

}

static ssize_t bma250_fast_calibration_y_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	signed char tmp;
	unsigned int timeout = 0;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);
#ifdef CONFIG_MACH_LGE
	/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
	* sensor SURV shake detection" 
	*/
	struct bma250acc acc_cal;
	struct bma250acc acc_cal_pre; 
#endif

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
#ifdef CONFIG_MACH_LGE
	/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
	* sensor SURV shake detection" 
	*/

	bma250_read_accel_xyz(bma250->bma250_client, &acc_cal_pre);
	mdelay(50);
#endif

	if (bma250_set_offset_target_y(bma250->bma250_client,
				(unsigned char)data) < 0)
		return -EINVAL;

	if (bma250_set_cal_trigger(bma250->bma250_client, 2) < 0)
		return -EINVAL;

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
	atomic_set(&bma250->fast_calib_y_rslt, 0);
#endif


	do {
		mdelay(2);
		bma250_get_cal_ready(bma250->bma250_client, &tmp);
#ifdef CONFIG_MACH_LGE
		/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
		* sensor SURV shake detection" 
		*/

		bma250_read_accel_xyz(bma250->bma250_client, &acc_cal);		
		if( (tmp == 0)	&&
			((abs(acc_cal.x - acc_cal_pre.x) > BMA250_SHAKING_DETECT_THRESHOLD)
				|| (abs((acc_cal.y - acc_cal_pre.y)) > BMA250_SHAKING_DETECT_THRESHOLD)
				|| (abs((acc_cal.z - acc_cal_pre.z)) > BMA250_SHAKING_DETECT_THRESHOLD))
		  )
		{
			
			bma250_soft_reset(bma250->bma250_client);
			printk(KERN_INFO "===============moved y===============\n");
			printk(KERN_INFO "(%d, %d, %d) (%d, %d, %d)\n", acc_cal_pre.x,  acc_cal_pre.y,  acc_cal_pre.z, acc_cal.x,acc_cal.y,acc_cal.z );
			return -EINVAL;

		}
		else
		{
			acc_cal_pre.x = acc_cal.x;
			acc_cal_pre.y = acc_cal.y;
			acc_cal_pre.z = acc_cal.z;
		}

#endif

		printk(KERN_INFO "[y] wait 2ms and got cal ready flag is %d\n",
				tmp);
		timeout++;
		if (timeout == 1000) {
			printk(KERN_INFO "get fast calibration ready error\n");
			return -EINVAL;
		};

	} while (tmp == 0);

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
	atomic_set(&bma250->fast_calib_y_rslt, 1);
#endif

	printk(KERN_INFO "y axis fast calibration finished\n");
	return count;
}

static ssize_t bma250_fast_calibration_z_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{


	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
	return sprintf(buf, "%d\n", atomic_read(&bma250->fast_calib_z_rslt));	
#if 0
	if (bma250_get_offset_target_z(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
#endif
#endif
}

static ssize_t bma250_fast_calibration_z_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	signed char tmp;
	unsigned int timeout = 0;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);
#ifdef CONFIG_MACH_LGE
	/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
	* sensor SURV shake detection" 
	*/

	struct bma250acc acc_cal;
	struct bma250acc acc_cal_pre; 
#endif

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
	bma250_read_accel_xyz(bma250->bma250_client, &acc_cal_pre);
	mdelay(50);
#endif
	if (bma250_set_offset_target_z(bma250->bma250_client,
				(unsigned char)data) < 0)
		return -EINVAL;

	if (bma250_set_cal_trigger(bma250->bma250_client, 3) < 0)
		return -EINVAL;

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
		atomic_set(&bma250->fast_calib_z_rslt, 0);
#endif	

	do {
		mdelay(2);
		bma250_get_cal_ready(bma250->bma250_client, &tmp);
#ifdef CONFIG_MACH_LGE
		/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
		* sensor SURV shake detection" 
		*/
		bma250_read_accel_xyz(bma250->bma250_client, &acc_cal);

		if( (tmp == 0)	&&
			((abs(acc_cal.x - acc_cal_pre.x) > BMA250_SHAKING_DETECT_THRESHOLD)
				|| (abs((acc_cal.y - acc_cal_pre.y)) > BMA250_SHAKING_DETECT_THRESHOLD)
				|| (abs((acc_cal.z - acc_cal_pre.z)) > BMA250_SHAKING_DETECT_THRESHOLD))
		  )
		{
			bma250_soft_reset(bma250->bma250_client);
			printk(KERN_INFO "===============moved z===============\n");
			printk(KERN_INFO "(%d, %d, %d) (%d, %d, %d)\n", acc_cal_pre.x,  acc_cal_pre.y,  acc_cal_pre.z, acc_cal.x,acc_cal.y,acc_cal.z );
			return -EINVAL;
		}
		else
		{
			acc_cal_pre.x = acc_cal.x;
			acc_cal_pre.y = acc_cal.y;
			acc_cal_pre.z = acc_cal.z;
		}

#endif

		printk(KERN_INFO "[z] wait 2ms and got cal ready flag is %d\n",
				tmp);
		timeout++;
		if (timeout == 1000) {
			printk(KERN_INFO "get fast calibration ready error\n");
			return -EINVAL;
		}

	} while (tmp == 0);

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
	atomic_set(&bma250->fast_calib_z_rslt, 1);
#endif
	printk(KERN_INFO "z axis fast calibration finished\n");
	return count;
}

static ssize_t bma250_eeprom_writing_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	signed char tmp;
	int timeout = 0;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;



	if (data != 1)
		return -EINVAL;

	/* unlock eeprom */
	if (bma250_set_ee_w(bma250->bma250_client, 1) < 0)
		return -EINVAL;

	printk(KERN_INFO "unlock eeprom successful\n");

	if (bma250_set_ee_prog_trig(bma250->bma250_client) < 0)
		return -EINVAL;
	printk(KERN_INFO "start update eeprom\n");

	do {
		mdelay(2);
		bma250_get_eeprom_writing_status(bma250->bma250_client, &tmp);

		printk(KERN_INFO "wait 2ms eeprom write status is %d\n", tmp);
		timeout++;
		if (timeout == 1000) {
			printk(KERN_INFO "get eeprom writing status error\n");
			return -EINVAL;
		};

	} while (tmp == 0);

	printk(KERN_INFO "eeprom writing is finished\n");

	/* unlock eeprom */
	if (bma250_set_ee_w(bma250->bma250_client, 0) < 0)
		return -EINVAL;
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-02-09 sensor SURV shake detection*/
	atomic_set(&bma250->fast_calib_rslt, 1);
	printk(KERN_INFO "[set] bma250->fast_calib_rslt:%d\n", atomic_read(&bma250->fast_calib_rslt));
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-02-09 sensor SURV shake detection*/
	printk(KERN_INFO "lock eeprom successful\n");
	return count;
}


static ssize_t bma250_eeprom_writing_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_get_eeprom_writing_status(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}


static ssize_t bma250_offset_filt_x_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_get_offset_filt_x(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma250_offset_filt_x_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma250_set_offset_filt_x(bma250->bma250_client, (unsigned
					char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma250_offset_filt_y_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_get_offset_filt_y(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma250_offset_filt_y_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma250_set_offset_filt_y(bma250->bma250_client, (unsigned
					char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma250_offset_filt_z_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_get_offset_filt_z(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma250_offset_filt_z_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma250_set_offset_filt_z(bma250->bma250_client, (unsigned
					char)data) < 0)
		return -EINVAL;

	return count;
}
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-02-09*/

static ssize_t bma250_x_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	short data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);
	if (bma250_read_accel_x(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");
	
	return sprintf(buf, "%d\n", data);

}
static ssize_t bma250_y_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	short data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);
	if (bma250_read_accel_y(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}
static ssize_t bma250_z_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	short data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);
	if (bma250_read_accel_z(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}
static ssize_t bma250_cal_result_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma250->fast_calib_rslt));
}

static ssize_t bma250_diag_cnt_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", atomic_read(&bma250_cnt));
}

static ssize_t bma250_diag_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", atomic_read(&bma250_diag));
}
static ssize_t bma250_diag_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int value;

	sscanf(buf, "%d", &value);
	if(value == 1){
		atomic_set(&bma250_diag,1);
	}else{
		atomic_set(&bma250_diag,0);
	}
	return count;
}
static DEVICE_ATTR(x, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_x_show, NULL);
static DEVICE_ATTR(y, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_y_show, NULL);
static DEVICE_ATTR(z, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_z_show, NULL);
static DEVICE_ATTR(cal_result,S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_cal_result_show, NULL);
static DEVICE_ATTR(cnt, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_diag_cnt_show, NULL);
static DEVICE_ATTR(diag, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_diag_show, bma250_diag_store);
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-02-09*/

static DEVICE_ATTR(range, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_range_show, bma250_range_store);
static DEVICE_ATTR(bandwidth, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_bandwidth_show, bma250_bandwidth_store);
static DEVICE_ATTR(mode, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_mode_show, bma250_mode_store);
static DEVICE_ATTR(value, S_IRUGO,
		bma250_value_show, NULL);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_delay_show, bma250_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_enable_show, bma250_enable_store);
static DEVICE_ATTR(update, S_IRUGO|S_IWUSR|S_IWGRP,
		NULL, bma250_update_store);
static DEVICE_ATTR(selftest, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_selftest_show, bma250_selftest_store);
static DEVICE_ATTR(fast_calibration_x, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_fast_calibration_x_show,
		bma250_fast_calibration_x_store);
static DEVICE_ATTR(fast_calibration_y, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_fast_calibration_y_show,
		bma250_fast_calibration_y_store);
static DEVICE_ATTR(fast_calibration_z, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_fast_calibration_z_show,
		bma250_fast_calibration_z_store);

static DEVICE_ATTR(eeprom_writing, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_eeprom_writing_show, bma250_eeprom_writing_store);

static DEVICE_ATTR(offset_filt_x, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_offset_filt_x_show,
		bma250_offset_filt_x_store);

static DEVICE_ATTR(offset_filt_y, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_offset_filt_y_show,
		bma250_offset_filt_y_store);

static DEVICE_ATTR(offset_filt_z, S_IRUGO|S_IWUSR|S_IWGRP,
		bma250_offset_filt_z_show,
		bma250_offset_filt_z_store);

#ifdef CONFIG_MACH_LGE
/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
* sensor SURV shake detection" 
*/
static DEVICE_ATTR(softreset, S_IWUSR|S_IWGRP,
		NULL, bma250_softreset_store);
#endif


static struct attribute *bma250_attributes[] = {
/* LGE_CHANGE_S [jiyeon.park@lge.com] 2012-02-09*/
	&dev_attr_x.attr,
	&dev_attr_y.attr,
	&dev_attr_z.attr,
	&dev_attr_diag.attr,
	&dev_attr_cnt.attr,
	&dev_attr_cal_result.attr,
/* LGE_CHANGE_E [jiyeon.park@lge.com] 2012-02-09*/
	&dev_attr_range.attr,
	&dev_attr_bandwidth.attr,
	&dev_attr_mode.attr,
	&dev_attr_value.attr,
	&dev_attr_delay.attr,
	&dev_attr_enable.attr,
	&dev_attr_update.attr,
	&dev_attr_selftest.attr,
	&dev_attr_fast_calibration_x.attr,
	&dev_attr_fast_calibration_y.attr,
	&dev_attr_fast_calibration_z.attr,
	&dev_attr_eeprom_writing.attr,
	&dev_attr_offset_filt_x.attr,
	&dev_attr_offset_filt_y.attr,
	&dev_attr_offset_filt_z.attr,
#ifdef CONFIG_MACH_LGE
	/*LGE_CHANGE : 2012-09-24 Sanghun,Lee(eee3114.lee@lge.com)
	* sensor SURV shake detection" 
	*/
	&dev_attr_softreset.attr,	
#endif	
	NULL
};

static struct attribute_group bma250_attribute_group = {
	.attrs = bma250_attributes
};

static int bma250_input_init(struct bma250_data *bma250)
{
	struct input_dev *dev;
	int err;

	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;
	dev->name = SENSOR_NAME;
	dev->id.bustype = BUS_I2C;

	input_set_capability(dev, EV_ABS, ABS_MISC);
	input_set_abs_params(dev, ABS_X, ABSMIN_2G, ABSMAX_2G, 0, 0);
	input_set_abs_params(dev, ABS_Y, ABSMIN_2G, ABSMAX_2G, 0, 0);
	input_set_abs_params(dev, ABS_Z, ABSMIN_2G, ABSMAX_2G, 0, 0);
	input_set_drvdata(dev, bma250);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		return err;
	}
	bma250->input = dev;

	return 0;
}

static void bma250_input_delete(struct bma250_data *bma250)
{
	struct input_dev *dev = bma250->input;

	input_unregister_device(dev);
	input_free_device(dev);
}

static int bma250_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int err = 0;
	int tempvalue;
	struct bma250_data *data;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_INFO "i2c_check_functionality error\n");
		goto exit;
	}
	data = kzalloc(sizeof(struct bma250_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	/* read chip id */
	tempvalue = 0;
	tempvalue = i2c_smbus_read_word_data(client, BMA250_CHIP_ID_REG);

	if (tempvalue < 0) {
		printk(KERN_ERR "Bosch Sensortec Device not found, \
				i2c error %#x \n", tempvalue);
		err = tempvalue;
		goto kfree_exit;
	}

	if ((tempvalue & 0xFF) == BMA250_CHIP_ID) {
		printk(KERN_INFO "Bosch Sensortec Device detected!\n" \
				"BMA250 registered I2C driver!\n");
	} else {
		printk(KERN_ERR "Bosch Sensortec Device not found, \
				chip ID mismatch");
		err = -ENODEV;
		goto kfree_exit;
	}
	i2c_set_clientdata(client, data);
	data->bma250_client = client;
	mutex_init(&data->mode_mutex);
	mutex_init(&data->enable_mutex);
	bma250_set_bandwidth(client, BMA250_BW_SET);
	bma250_set_range(client, BMA250_RANGE_SET);

	INIT_DELAYED_WORK(&data->work, bma250_work_func);
	atomic_set(&data->delay, BMA250_DEFAULT_DELAY);
	atomic_set(&data->enable, 0);
	err = bma250_input_init(data);
	if (err < 0)
		goto kfree_exit;

	err = sysfs_create_group(&data->input->dev.kobj,
			&bma250_attribute_group);
	if (err < 0)
		goto error_sysfs;

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = bma250_early_suspend;
	data->early_suspend.resume = bma250_late_resume;
	register_early_suspend(&data->early_suspend);
#endif
	err = bma250_set_mode(client, BMA250_MODE_SUSPEND);
	if (err)
		goto error_sysfs;

	return 0;

error_sysfs:
	bma250_input_delete(data);

kfree_exit:
	kfree(data);
exit:
	return err;
}


#ifdef CONFIG_HAS_EARLYSUSPEND
static void bma250_early_suspend(struct early_suspend *h)
{
	struct bma250_data *data =
		container_of(h, struct bma250_data, early_suspend);

	mutex_lock(&data->enable_mutex);
	if (atomic_read(&data->enable) == 1) {
		bma250_set_mode(data->bma250_client, BMA250_MODE_SUSPEND);
		cancel_delayed_work_sync(&data->work);
	}
	mutex_unlock(&data->enable_mutex);
}


static void bma250_late_resume(struct early_suspend *h)
{
	struct bma250_data *data =
		container_of(h, struct bma250_data, early_suspend);

	mutex_lock(&data->enable_mutex);
	if (atomic_read(&data->enable) == 1) {
		bma250_set_mode(data->bma250_client, BMA250_MODE_NORMAL);
		schedule_delayed_work(&data->work,
				msecs_to_jiffies(atomic_read(&data->delay)));
	}
	mutex_unlock(&data->enable_mutex);
}
#endif

static int bma250_remove(struct i2c_client *client)
{
	struct bma250_data *data = i2c_get_clientdata(client);

	bma250_set_enable(&client->dev, 0);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	sysfs_remove_group(&data->input->dev.kobj, &bma250_attribute_group);
	bma250_input_delete(data);
	kfree(data);
	return 0;
}


static const struct i2c_device_id bma250_id[] = {
	{ SENSOR_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, bma250_id);

static struct i2c_driver bma250_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= SENSOR_NAME,
	},
	.id_table	= bma250_id,
	.probe		= bma250_probe,
	.remove		= bma250_remove,

};

static int __init BMA250_init(void)
{
	return i2c_add_driver(&bma250_driver);
}

static void __exit BMA250_exit(void)
{
	i2c_del_driver(&bma250_driver);
}

MODULE_AUTHOR("Albert Zhang <xu.zhang@bosch-sensortec.com>");
MODULE_DESCRIPTION("BMA250 driver");
MODULE_LICENSE("GPL");

module_init(BMA250_init);
module_exit(BMA250_exit);

