/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2011 Bosch Sensortec GmbH All Rights Reserved
 */

/*  Disclaimer
 *
 * Common: Bosch Sensortec products are developed for the consumer goods
 * industry. They may only be used within the parameters of the respective valid
 * product data sheet.  Bosch Sensortec products are provided with the express
 * understanding that there is no warranty of fitness for a particular purpose.
 * They are not fit for use in life-sustaining, safety or security sensitive
 * systems or any system or device that may lead to bodily harm or property
 * damage if the system or device malfunctions. In addition, Bosch Sensortec
 * products are not fit for use in products which interact with motor vehicle
 * systems.  The resale and/or use of products are at the purchaser's own risk
 * and his own responsibility. The examination of fitness for the intended use
 * is the sole responsibility of the Purchaser.
 *
 * The purchaser shall indemnify Bosch Sensortec from all third party claims,
 * including any claims for incidental, or consequential damages, arising from
 * any product use not covered by the parameters of the respective valid product
 * data sheet or not approved by Bosch Sensortec and reimburse Bosch Sensortec
 * for all costs in connection with such claims.
 *
 * The purchaser must monitor the market for the purchased products,
 * particularly with regard to product safety and inform Bosch Sensortec without
 * delay of all security relevant incidents.
 *
 * Engineering Samples are marked with an asterisk (*) or (e). Samples may vary
 * from the valid technical specifications of the product series. They are
 * therefore not intended or fit for resale to third parties or for use in end
 * products. Their sole purpose is internal client testing. The testing of an
 * engineering sample may in no way replace the testing of a product series.
 * Bosch Sensortec assumes no liability for the use of engineering samples. By
 * accepting the engineering samples, the Purchaser agrees to indemnify Bosch
 * Sensortec from all claims arising from the use of engineering samples.
 *
 * Special: This software module (hereinafter called "Software") and any
 * information on application-sheets (hereinafter called "Information") is
 * provided free of charge for the sole purpose to support your application
 * work. The Software and Information is subject to the following terms and
 * conditions:
 *
 * The Software is specifically designed for the exclusive use for Bosch
 * Sensortec products by personnel who have special experience and training. Do
 * not use this Software if you do not have the proper experience or training.
 *
 * This Software package is provided `` as is `` and without any expressed or
 * implied warranties, including without limitation, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 * Bosch Sensortec and their representatives and agents deny any liability for
 * the functional impairment of this Software in terms of fitness, performance
 * and safety. Bosch Sensortec and their representatives and agents shall not be
 * liable for any direct or indirect damages or injury, except as otherwise
 * stipulated in mandatory applicable law.
 *
 * The Information provided is believed to be accurate and reliable. Bosch
 * Sensortec assumes no responsibility for the consequences of use of such
 * Information nor for any infringement of patents or other rights of third
 * parties which may result from its use. No license is granted by implication
 * or otherwise under any patent or patent rights of Bosch. Specifications
 * mentioned in the Information are subject to change without notice.
 *
 * It is not allowed to deliver the source code of the Software to any third
 * party without permission of Bosch Sensortec.
 */

#ifndef __BMC050_H__
#define __BMC050_H__

#define BMC050_U16 unsigned short
#define BMC050_S16 signed short
#define BMC050_S32 signed int


#define BMC050_BUS_WR_RETURN_TYPE char
#define BMC050_BUS_WR_PARAM_TYPES\
	unsigned char, unsigned char, unsigned char *, unsigned char
#define BMC050_BUS_WR_PARAM_ORDER\
	(device_addr, register_addr, register_data, wr_len)
#define BMC050_BUS_WRITE_FUNC(\
		device_addr, register_addr, register_data, wr_len)\
	bus_write(device_addr, register_addr, register_data, wr_len)

#define BMC050_BUS_RD_RETURN_TYPE char

#define BMC050_BUS_RD_PARAM_TYPES\
	unsigned char, unsigned char, unsigned char *, unsigned char

#define BMC050_BUS_RD_PARAM_ORDER (device_addr, register_addr, register_data)

#define BMC050_BUS_READ_FUNC(device_addr, register_addr, register_data, rd_len)\
	bus_read(device_addr, register_addr, register_data, rd_len)


#define BMC050_DELAY_RETURN_TYPE void

#define BMC050_DELAY_PARAM_TYPES unsigned int

#define BMC050_DELAY_FUNC(delay_in_msec)\
	delay_func(delay_in_msec)

#define BMC050_DELAY_POWEROFF_SUSPEND      1
#define BMC050_DELAY_SUSPEND_SLEEP         2
#define BMC050_DELAY_SLEEP_ACTIVE          1
#define BMC050_DELAY_ACTIVE_SLEEP          1
#define BMC050_DELAY_SLEEP_SUSPEND         1
#define BMC050_DELAY_ACTIVE_SUSPEND        1
#define BMC050_DELAY_SLEEP_POWEROFF        1
#define BMC050_DELAY_ACTIVE_POWEROFF       1
#define BMC050_DELAY_SETTLING_TIME         2


#define BMC050_RETURN_FUNCTION_TYPE        char
#define BMC050_I2C_ADDRESS                 0x10

/*General Info datas*/
#define BMC050_SOFT_RESET7_ON              1
#define BMC050_SOFT_RESET1_ON              1
#define BMC050_SOFT_RESET7_OFF             0
#define BMC050_SOFT_RESET1_OFF             0
#define BMC050_DELAY_SOFTRESET             1

/* Fixed Data Registers */
#define BMC050_CHIP_ID                     0x40
#define BMC050_REVISION_ID                 0x41

/* Data Registers */
#define BMC050_DATAX_LSB                   0x42
#define BMC050_DATAX_MSB                   0x43
#define BMC050_DATAY_LSB                   0x44
#define BMC050_DATAY_MSB                   0x45
#define BMC050_DATAZ_LSB                   0x46
#define BMC050_DATAZ_MSB                   0x47
#define BMC050_R_LSB                       0x48
#define BMC050_R_MSB                       0x49

/* Status Registers */
#define BMC050_INT_STAT                    0x4A

/* Control Registers */
#define BMC050_POWER_CNTL                  0x4B
#define BMC050_CONTROL                     0x4C
#define BMC050_INT_CNTL                    0x4D
#define BMC050_SENS_CNTL                   0x4E
#define BMC050_LOW_THRES                   0x4F
#define BMC050_HIGH_THRES                  0x50
#define BMC050_NO_REPETITIONS_XY           0x51
#define BMC050_NO_REPETITIONS_Z            0x52

/* Trim Extended Registers */
#define BMC050_DIG_X1                      0x5D
#define BMC050_DIG_Y1                      0x5E
#define BMC050_DIG_Z4_LSB                  0x62
#define BMC050_DIG_Z4_MSB                  0x63
#define BMC050_DIG_X2                      0x64
#define BMC050_DIG_Y2                      0x65
#define BMC050_DIG_Z2_LSB                  0x68
#define BMC050_DIG_Z2_MSB                  0x69
#define BMC050_DIG_Z1_LSB                  0x6A
#define BMC050_DIG_Z1_MSB                  0x6B
#define BMC050_DIG_XYZ1_LSB                0x6C
#define BMC050_DIG_XYZ1_MSB                0x6D
#define BMC050_DIG_Z3_LSB                  0x6E
#define BMC050_DIG_Z3_MSB                  0x6F
#define BMC050_DIG_XY2                     0x70
#define BMC050_DIG_XY1                     0x71


/* Data X LSB Regsiter */
#define BMC050_DATAX_LSB_VALUEX__POS        3
#define BMC050_DATAX_LSB_VALUEX__LEN        5
#define BMC050_DATAX_LSB_VALUEX__MSK        0xF8
#define BMC050_DATAX_LSB_VALUEX__REG        BMC050_DATAX_LSB

#define BMC050_DATAX_LSB_TESTX__POS         0
#define BMC050_DATAX_LSB_TESTX__LEN         1
#define BMC050_DATAX_LSB_TESTX__MSK         0x01
#define BMC050_DATAX_LSB_TESTX__REG         BMC050_DATAX_LSB

/* Data Y LSB Regsiter */
#define BMC050_DATAY_LSB_VALUEY__POS        3
#define BMC050_DATAY_LSB_VALUEY__LEN        5
#define BMC050_DATAY_LSB_VALUEY__MSK        0xF8
#define BMC050_DATAY_LSB_VALUEY__REG        BMC050_DATAY_LSB

#define BMC050_DATAY_LSB_TESTY__POS         0
#define BMC050_DATAY_LSB_TESTY__LEN         1
#define BMC050_DATAY_LSB_TESTY__MSK         0x01
#define BMC050_DATAY_LSB_TESTY__REG         BMC050_DATAY_LSB

/* Data Z LSB Regsiter */
#define BMC050_DATAZ_LSB_VALUEZ__POS        1
#define BMC050_DATAZ_LSB_VALUEZ__LEN        7
#define BMC050_DATAZ_LSB_VALUEZ__MSK        0xFE
#define BMC050_DATAZ_LSB_VALUEZ__REG        BMC050_DATAZ_LSB

#define BMC050_DATAZ_LSB_TESTZ__POS         0
#define BMC050_DATAZ_LSB_TESTZ__LEN         1
#define BMC050_DATAZ_LSB_TESTZ__MSK         0x01
#define BMC050_DATAZ_LSB_TESTZ__REG         BMC050_DATAZ_LSB

/* Hall Resistance LSB Regsiter */
#define BMC050_R_LSB_VALUE__POS             2
#define BMC050_R_LSB_VALUE__LEN             6
#define BMC050_R_LSB_VALUE__MSK             0xFC
#define BMC050_R_LSB_VALUE__REG             BMC050_R_LSB

#define BMC050_DATA_RDYSTAT__POS            0
#define BMC050_DATA_RDYSTAT__LEN            1
#define BMC050_DATA_RDYSTAT__MSK            0x01
#define BMC050_DATA_RDYSTAT__REG            BMC050_R_LSB

/* Interupt Status Register */
#define BMC050_INT_STAT_DOR__POS            7
#define BMC050_INT_STAT_DOR__LEN            1
#define BMC050_INT_STAT_DOR__MSK            0x80
#define BMC050_INT_STAT_DOR__REG            BMC050_INT_STAT

#define BMC050_INT_STAT_OVRFLOW__POS        6
#define BMC050_INT_STAT_OVRFLOW__LEN        1
#define BMC050_INT_STAT_OVRFLOW__MSK        0x40
#define BMC050_INT_STAT_OVRFLOW__REG        BMC050_INT_STAT

#define BMC050_INT_STAT_HIGH_THZ__POS       5
#define BMC050_INT_STAT_HIGH_THZ__LEN       1
#define BMC050_INT_STAT_HIGH_THZ__MSK       0x20
#define BMC050_INT_STAT_HIGH_THZ__REG       BMC050_INT_STAT

#define BMC050_INT_STAT_HIGH_THY__POS       4
#define BMC050_INT_STAT_HIGH_THY__LEN       1
#define BMC050_INT_STAT_HIGH_THY__MSK       0x10
#define BMC050_INT_STAT_HIGH_THY__REG       BMC050_INT_STAT

#define BMC050_INT_STAT_HIGH_THX__POS       3
#define BMC050_INT_STAT_HIGH_THX__LEN       1
#define BMC050_INT_STAT_HIGH_THX__MSK       0x08
#define BMC050_INT_STAT_HIGH_THX__REG       BMC050_INT_STAT

#define BMC050_INT_STAT_LOW_THZ__POS        2
#define BMC050_INT_STAT_LOW_THZ__LEN        1
#define BMC050_INT_STAT_LOW_THZ__MSK        0x04
#define BMC050_INT_STAT_LOW_THZ__REG        BMC050_INT_STAT

#define BMC050_INT_STAT_LOW_THY__POS        1
#define BMC050_INT_STAT_LOW_THY__LEN        1
#define BMC050_INT_STAT_LOW_THY__MSK        0x02
#define BMC050_INT_STAT_LOW_THY__REG        BMC050_INT_STAT

#define BMC050_INT_STAT_LOW_THX__POS        0
#define BMC050_INT_STAT_LOW_THX__LEN        1
#define BMC050_INT_STAT_LOW_THX__MSK        0x01
#define BMC050_INT_STAT_LOW_THX__REG        BMC050_INT_STAT

/* Power Control Register */
#define BMC050_POWER_CNTL_SRST7__POS       7
#define BMC050_POWER_CNTL_SRST7__LEN       1
#define BMC050_POWER_CNTL_SRST7__MSK       0x80
#define BMC050_POWER_CNTL_SRST7__REG       BMC050_POWER_CNTL

#define BMC050_POWER_CNTL_SPI3_EN__POS     2
#define BMC050_POWER_CNTL_SPI3_EN__LEN     1
#define BMC050_POWER_CNTL_SPI3_EN__MSK     0x04
#define BMC050_POWER_CNTL_SPI3_EN__REG     BMC050_POWER_CNTL

#define BMC050_POWER_CNTL_SRST1__POS       1
#define BMC050_POWER_CNTL_SRST1__LEN       1
#define BMC050_POWER_CNTL_SRST1__MSK       0x02
#define BMC050_POWER_CNTL_SRST1__REG       BMC050_POWER_CNTL

#define BMC050_POWER_CNTL_PCB__POS         0
#define BMC050_POWER_CNTL_PCB__LEN         1
#define BMC050_POWER_CNTL_PCB__MSK         0x01
#define BMC050_POWER_CNTL_PCB__REG         BMC050_POWER_CNTL

/* Control Register */
#define BMC050_CNTL_ADV_ST__POS            6
#define BMC050_CNTL_ADV_ST__LEN            2
#define BMC050_CNTL_ADV_ST__MSK            0xC0
#define BMC050_CNTL_ADV_ST__REG            BMC050_CONTROL

#define BMC050_CNTL_DR__POS                3
#define BMC050_CNTL_DR__LEN                3
#define BMC050_CNTL_DR__MSK                0x38
#define BMC050_CNTL_DR__REG                BMC050_CONTROL

#define BMC050_CNTL_OPMODE__POS            1
#define BMC050_CNTL_OPMODE__LEN            2
#define BMC050_CNTL_OPMODE__MSK            0x06
#define BMC050_CNTL_OPMODE__REG            BMC050_CONTROL

#define BMC050_CNTL_S_TEST__POS            0
#define BMC050_CNTL_S_TEST__LEN            1
#define BMC050_CNTL_S_TEST__MSK            0x01
#define BMC050_CNTL_S_TEST__REG            BMC050_CONTROL

/* Interupt Control Register */
#define BMC050_INT_CNTL_DOR_EN__POS            7
#define BMC050_INT_CNTL_DOR_EN__LEN            1
#define BMC050_INT_CNTL_DOR_EN__MSK            0x80
#define BMC050_INT_CNTL_DOR_EN__REG            BMC050_INT_CNTL

#define BMC050_INT_CNTL_OVRFLOW_EN__POS        6
#define BMC050_INT_CNTL_OVRFLOW_EN__LEN        1
#define BMC050_INT_CNTL_OVRFLOW_EN__MSK        0x40
#define BMC050_INT_CNTL_OVRFLOW_EN__REG        BMC050_INT_CNTL

#define BMC050_INT_CNTL_HIGH_THZ_EN__POS       5
#define BMC050_INT_CNTL_HIGH_THZ_EN__LEN       1
#define BMC050_INT_CNTL_HIGH_THZ_EN__MSK       0x20
#define BMC050_INT_CNTL_HIGH_THZ_EN__REG       BMC050_INT_CNTL

#define BMC050_INT_CNTL_HIGH_THY_EN__POS       4
#define BMC050_INT_CNTL_HIGH_THY_EN__LEN       1
#define BMC050_INT_CNTL_HIGH_THY_EN__MSK       0x10
#define BMC050_INT_CNTL_HIGH_THY_EN__REG       BMC050_INT_CNTL

#define BMC050_INT_CNTL_HIGH_THX_EN__POS       3
#define BMC050_INT_CNTL_HIGH_THX_EN__LEN       1
#define BMC050_INT_CNTL_HIGH_THX_EN__MSK       0x08
#define BMC050_INT_CNTL_HIGH_THX_EN__REG       BMC050_INT_CNTL

#define BMC050_INT_CNTL_LOW_THZ_EN__POS        2
#define BMC050_INT_CNTL_LOW_THZ_EN__LEN        1
#define BMC050_INT_CNTL_LOW_THZ_EN__MSK        0x04
#define BMC050_INT_CNTL_LOW_THZ_EN__REG        BMC050_INT_CNTL

#define BMC050_INT_CNTL_LOW_THY_EN__POS        1
#define BMC050_INT_CNTL_LOW_THY_EN__LEN        1
#define BMC050_INT_CNTL_LOW_THY_EN__MSK        0x02
#define BMC050_INT_CNTL_LOW_THY_EN__REG        BMC050_INT_CNTL

#define BMC050_INT_CNTL_LOW_THX_EN__POS        0
#define BMC050_INT_CNTL_LOW_THX_EN__LEN        1
#define BMC050_INT_CNTL_LOW_THX_EN__MSK        0x01
#define BMC050_INT_CNTL_LOW_THX_EN__REG        BMC050_INT_CNTL

/* Sensor Control Register */
#define BMC050_SENS_CNTL_DRDY_EN__POS          7
#define BMC050_SENS_CNTL_DRDY_EN__LEN          1
#define BMC050_SENS_CNTL_DRDY_EN__MSK          0x80
#define BMC050_SENS_CNTL_DRDY_EN__REG          BMC050_SENS_CNTL

#define BMC050_SENS_CNTL_IE__POS               6
#define BMC050_SENS_CNTL_IE__LEN               1
#define BMC050_SENS_CNTL_IE__MSK               0x40
#define BMC050_SENS_CNTL_IE__REG               BMC050_SENS_CNTL

#define BMC050_SENS_CNTL_CHANNELZ__POS         5
#define BMC050_SENS_CNTL_CHANNELZ__LEN         1
#define BMC050_SENS_CNTL_CHANNELZ__MSK         0x20
#define BMC050_SENS_CNTL_CHANNELZ__REG         BMC050_SENS_CNTL

#define BMC050_SENS_CNTL_CHANNELY__POS         4
#define BMC050_SENS_CNTL_CHANNELY__LEN         1
#define BMC050_SENS_CNTL_CHANNELY__MSK         0x10
#define BMC050_SENS_CNTL_CHANNELY__REG         BMC050_SENS_CNTL

#define BMC050_SENS_CNTL_CHANNELX__POS         3
#define BMC050_SENS_CNTL_CHANNELX__LEN         1
#define BMC050_SENS_CNTL_CHANNELX__MSK         0x08
#define BMC050_SENS_CNTL_CHANNELX__REG         BMC050_SENS_CNTL

#define BMC050_SENS_CNTL_DR_POLARITY__POS      2
#define BMC050_SENS_CNTL_DR_POLARITY__LEN      1
#define BMC050_SENS_CNTL_DR_POLARITY__MSK      0x04
#define BMC050_SENS_CNTL_DR_POLARITY__REG      BMC050_SENS_CNTL

#define BMC050_SENS_CNTL_INTERRUPT_LATCH__POS            1
#define BMC050_SENS_CNTL_INTERRUPT_LATCH__LEN            1
#define BMC050_SENS_CNTL_INTERRUPT_LATCH__MSK            0x02
#define BMC050_SENS_CNTL_INTERRUPT_LATCH__REG            BMC050_SENS_CNTL

#define BMC050_SENS_CNTL_INTERRUPT_POLARITY__POS         0
#define BMC050_SENS_CNTL_INTERRUPT_POLARITY__LEN         1
#define BMC050_SENS_CNTL_INTERRUPT_POLARITY__MSK         0x01
#define BMC050_SENS_CNTL_INTERRUPT_POLARITY__REG         BMC050_SENS_CNTL

/* Register 6D */
#define BMC050_DIG_XYZ1_MSB__POS         0
#define BMC050_DIG_XYZ1_MSB__LEN         7
#define BMC050_DIG_XYZ1_MSB__MSK         0x7F
#define BMC050_DIG_XYZ1_MSB__REG         BMC050_DIG_XYZ1_MSB


#define BMC050_X_AXIS               0
#define BMC050_Y_AXIS               1
#define BMC050_Z_AXIS               2
#define BMC050_RESISTANCE           3
#define BMC050_X                    1
#define BMC050_Y                    2
#define BMC050_Z                    4
#define BMC050_XYZ                  7

/* Constants */
#define BMC050_NULL                             0
#define BMC050_INTPIN_DISABLE                   1
#define BMC050_INTPIN_ENABLE                    0
#define BMC050_DISABLE                          0
#define BMC050_ENABLE                           1
#define BMC050_CHANNEL_DISABLE                  1
#define BMC050_CHANNEL_ENABLE                   0
#define BMC050_INTPIN_LATCH_ENABLE              1
#define BMC050_INTPIN_LATCH_DISABLE             0
#define BMC050_OFF                              0
#define BMC050_ON                               1

#define BMC050_NORMAL_MODE                      0x00
#define BMC050_FORCED_MODE                      0x01
#define BMC050_SUSPEND_MODE                     0x02
#define BMC050_SLEEP_MODE                       0x03

#define BMC050_ADVANCED_SELFTEST_OFF            0
#define BMC050_ADVANCED_SELFTEST_NEGATIVE       2
#define BMC050_ADVANCED_SELFTEST_POSITIVE       3

#define BMC050_NEGATIVE_SATURATION_Z            -32767
#define BMC050_POSITIVE_SATURATION_Z            32767

#define BMC050_SPI_RD_MASK                      0x80
#define BMC050_READ_SET                         0x01

#define E_BMC050_NULL_PTR                       ((char)-127)
#define E_BMC050_COMM_RES                       ((char)-1)
#define E_BMC050_OUT_OF_RANGE                   ((char)-2)
#define E_BMC050_UNDEFINED_MODE                 0

#define BMC050_WR_FUNC_PTR\
	char (*bus_write)(unsigned char, unsigned char,\
			unsigned char *, unsigned char)

#define BMC050_RD_FUNC_PTR\
	char (*bus_read)(unsigned char, unsigned char,\
			unsigned char *, unsigned char)
#define BMC050_MDELAY_DATA_TYPE unsigned int

/*Shifting Constants*/
#define SHIFT_RIGHT_1_POSITION                  1
#define SHIFT_RIGHT_2_POSITION                  2
#define SHIFT_RIGHT_3_POSITION                  3
#define SHIFT_RIGHT_4_POSITION                  4
#define SHIFT_RIGHT_5_POSITION                  5
#define SHIFT_RIGHT_6_POSITION                  6
#define SHIFT_RIGHT_7_POSITION                  7
#define SHIFT_RIGHT_8_POSITION                  8

#define SHIFT_LEFT_1_POSITION                   1
#define SHIFT_LEFT_2_POSITION                   2
#define SHIFT_LEFT_3_POSITION                   3
#define SHIFT_LEFT_4_POSITION                   4
#define SHIFT_LEFT_5_POSITION                   5
#define SHIFT_LEFT_6_POSITION                   6
#define SHIFT_LEFT_7_POSITION                   7
#define SHIFT_LEFT_8_POSITION                   8

/* Conversion factors*/
#define BMC050_CONVFACTOR_LSB_UT                6

/* get bit slice  */
#define BMC050_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)

/* Set bit slice */
#define BMC050_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))

/* compensated output value returned if sensor had overflow */
#define BMC050_OVERFLOW_OUTPUT       -32768
/* Flipcore overflow ADC value */
#define BMC050_FLIP_OVERFLOW_ADCVAL  -4096
/* Hall overflow 1 ADC value */
#define BMC050_HALL_OVERFLOW_ADCVAL  -16384


#define BMC050_PRESETMODE_LOWPOWER                  1
#define BMC050_PRESETMODE_REGULAR                   2
#define BMC050_PRESETMODE_HIGHACCURACY              3

/* PRESET MODES - DATA RATES */
#define BMC050_LOWPOWER_DR                       BMC050_DR_10HZ
#define BMC050_REGULAR_DR                        BMC050_DR_10HZ
#define BMC050_HIGHACCURACY_DR                   BMC050_DR_20HZ

/* PRESET MODES - REPETITIONS-XY RATES */
#define BMC050_LOWPOWER_REPXY                     2
#define BMC050_REGULAR_REPXY                      5
#define BMC050_HIGHACCURACY_REPXY                40

/* PRESET MODES - REPETITIONS-Z RATES */
#define BMC050_LOWPOWER_REPZ                      4
#define BMC050_REGULAR_REPZ                      13
#define BMC050_HIGHACCURACY_REPZ                 89

/* Data Rates */

#define BMC050_DR_10HZ                     0
#define BMC050_DR_02HZ                     1
#define BMC050_DR_06HZ                     2
#define BMC050_DR_08HZ                     3
#define BMC050_DR_15HZ                     4
#define BMC050_DR_20HZ                     5
#define BMC050_DR_25HZ                     6
#define BMC050_DR_30HZ                     7

/*user defined Structures*/
struct bmc050_mdata {
	BMC050_S16 datax;
	BMC050_S16 datay;
	BMC050_S16 dataz;
	BMC050_U16 resistance;
};

struct bmc050_offset {
	BMC050_S16 datax;
	BMC050_S16 datay;
	BMC050_S16 dataz;
};

struct bmc050 {
	unsigned char company_id;
	unsigned char revision_info;
	unsigned char dev_addr;

	BMC050_WR_FUNC_PTR;
	BMC050_RD_FUNC_PTR;
	void(*delay_msec)(BMC050_MDELAY_DATA_TYPE);

	signed char dig_x1;
	signed char dig_y1;

	signed char dig_x2;
	signed char dig_y2;

	BMC050_U16 dig_z1;
	BMC050_S16 dig_z2;
	BMC050_S16 dig_z3;
	BMC050_S16 dig_z4;

	unsigned char dig_xy1;
	signed char dig_xy2;

	BMC050_U16 dig_xyz1;
};


BMC050_RETURN_FUNCTION_TYPE bmc050_init(struct bmc050 *p_bmc050);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_flipdataX(
		BMC050_S16 *mdata_x);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_flipdataY(
		BMC050_S16 *mdata_y);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_halldataZ(
		BMC050_S16 *mdata_z);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_raw_xyz(
		struct bmc050_mdata *mdata);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_raw_xyzr(
		struct bmc050_mdata *mdata);
BMC050_RETURN_FUNCTION_TYPE bmc050_read_mdataXYZ(
		struct bmc050_mdata *mdata);
BMC050_RETURN_FUNCTION_TYPE bmc050_read_mdataX(
		BMC050_S16 *mdata_x);
BMC050_RETURN_FUNCTION_TYPE bmc050_read_mdataY(
		BMC050_S16 *mdata_y);
BMC050_RETURN_FUNCTION_TYPE bmc050_read_mdataZ(
		BMC050_S16 *mdata_z);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_mdataResistance(
		BMC050_U16  *mdata_resistance);
BMC050_RETURN_FUNCTION_TYPE bmc050_read_register(
		unsigned char addr, unsigned char *data, unsigned char len);
BMC050_RETURN_FUNCTION_TYPE bmc050_write_register(
		unsigned char addr, unsigned char *data, unsigned char len);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_self_test_X(
		unsigned char *self_testx);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_self_test_Y(
		unsigned char *self_testy);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_self_test_Z(
		unsigned char *self_testz);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_self_test_XYZ(
		unsigned char *self_testxyz);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_data_rdy_status(
		unsigned char *rdy_status);
BMC050_S16 bmc050_compensate_X(
		BMC050_S16 mdata_x, BMC050_U16 data_R);
BMC050_S16 bmc050_compensate_Y(
		BMC050_S16 mdata_y, BMC050_U16 data_R);
BMC050_S16 bmc050_compensate_Z(
		BMC050_S16 mdata_z,  BMC050_U16 data_R);
BMC050_RETURN_FUNCTION_TYPE bmc050_init_trim_registers(void);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_status_reg(
		unsigned char *status_data);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_power_control_reg(
		unsigned char *pwr_cntl_data);
BMC050_RETURN_FUNCTION_TYPE bmc050_soft_reset(void);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_spi3(
		unsigned char value);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_powermode(
		unsigned char *mode);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_powermode(
		unsigned char mode);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_adv_selftest(
		unsigned char adv_selftest);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_adv_selftest(
		unsigned char *adv_selftest);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_datarate(
		unsigned char data_rate);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_datarate(
		unsigned char *data_rate);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_functional_state(
		unsigned char functional_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_functional_state(
		unsigned char *functional_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_selftest(
		unsigned char selftest);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_selftest(
		unsigned char *selftest);
BMC050_RETURN_FUNCTION_TYPE bmc050_perform_advanced_selftest(
		BMC050_S16 *diff_z);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_data_overrun_function(
		unsigned char data_overrun_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_data_overrun_function(
		unsigned char *data_overrun_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_data_overflow_function(
		unsigned char data_overflow);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_data_overflow_function(
		unsigned char *data_overflow);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_data_highthreshold_Z_function(
		unsigned char data_highthreshold_z_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_data_highthreshold_Z_function(
		unsigned char *data_highthreshold_z_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_data_highthreshold_Y_function(
		unsigned char data_highthreshold_y_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_data_highthreshold_Y_function(
		unsigned char *data_highthreshold_y_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_data_highthreshold_X_function(
		unsigned char data_highthreshold_x_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_data_highthreshold_X_function(
		unsigned char *data_highthreshold_x_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_data_lowthreshold_Z_function(
		unsigned char data_lowthreshold_z_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_data_lowthreshold_Z_function(
		unsigned char *data_lowthreshold_z_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_data_lowthreshold_Y_function(
		unsigned char data_lowthreshold_y_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_data_lowthreshold_Y_function(
		unsigned char *data_lowthreshold_y_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_data_lowthreshold_X_function(
		unsigned char data_lowthreshold_x_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_data_lowthreshold_X_function(
		unsigned char *data_lowthreshold_x_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_data_ready_function(
		unsigned char *data_ready_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_data_ready_function(
		unsigned char data_ready_function_state);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_interrupt_func(
		unsigned char *int_func);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_interrupt_func(
		unsigned char int_func);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_control_measurement_z(
		unsigned char *enable_disable);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_control_measurement_z(
		unsigned char enable_disable);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_control_measurement_y(
		unsigned char enable_disable);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_control_measurement_y(
		unsigned char *enable_disable);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_control_measurement_x(
		unsigned char enable_disable);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_control_measurement_x(
		unsigned char *enable_disable);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_DR_polarity(
		unsigned char dr_polarity_select);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_DR_polarity(
		unsigned char *dr_polarity_select);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_interrupt_latch(
		unsigned char interrupt_latch_select);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_interrupt_latch(
		unsigned char *interrupt_latch_select);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_intpin_polarity(
		unsigned char int_polarity_select);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_intpin_polarity(
		unsigned char *int_polarity_select);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_low_threshold(
		BMC050_S16 *low_threshold);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_low_threshold(
		BMC050_S16 low_threshold);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_high_threshold(
		BMC050_S16 *high_threshold);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_high_threshold(
		BMC050_S16 high_threshold);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_repetitions_XY(
		unsigned char *no_repetitions_xy);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_repetitions_XY(
		unsigned char no_repetitions_xy);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_repetitions_Z(
		unsigned char *no_repetitions_z);
BMC050_RETURN_FUNCTION_TYPE bmc050_set_repetitions_Z(
		unsigned char no_repetitions_z);
BMC050_RETURN_FUNCTION_TYPE bmc050_get_presetmode(unsigned char *mode);

BMC050_RETURN_FUNCTION_TYPE bmc050_set_presetmode(unsigned char mode);
#endif
