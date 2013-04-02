#
# Makefile for the sensor drivers.
#

# Each configuration option enables a list of files.

obj-$(CONFIG_SENSOR_GP2AP)      += gp2ap002.o
#LGE_CHANGE : 2012-10-24 Sanghun,Lee(eee3114.@lge.com) sensor change from bmc150 to bmc050
obj-$(CONFIG_SENSORS_BMA2X2)	+= bma2x2_driver.o
#LGE_CHANGE : 2012-10-24 Sanghun,Lee(eee3114.@lge.com) sensor change from bmc150 to bmc050
obj-$(CONFIG_SENSORS_BMA250)    += bma250_driver.o
obj-$(CONFIG_SENSORS_BMM050)   += bmm050_driver.o bmm050.o
obj-$(CONFIG_SENSOR_APDS9190)   += apds9190.o
#[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com] Avago Proximity Changes
obj-$(CONFIG_SENSOR_APDS9130)   += apds9130.o
#[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]

#LGE_CHANGE : 2013-02-04 jinseok.choi@lge.com V1 uses Rohm RPR0400 for Proximity Sensor
obj-$(CONFIG_SENSOR_RPR0400)   += rpr0400_driver.o

ifeq ($(CONFIG_SENSORS_BMM050),y)
EXTRA_CFLAGS += -DBMM_USE_BASIC_I2C_FUNC
endif
