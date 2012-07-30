#
# Makefile for the sensor drivers.
#

# Each configuration option enables a list of files.

obj-$(CONFIG_SENSOR_GP2AP)	+= gp2ap002.o
obj-$(CONFIG_SENSOR_AK8975)	+= akm8975.o
obj-$(CONFIG_SENSOR_BMA222)	+= bma222_driver.o bma222.o 
obj-$(CONFIG_SENSOR_K3DH)       += k3dh.o
obj-$(CONFIG_SENSOR_AMI304)     += ami304.o
obj-$(CONFIG_SENSOR_AMI306)     += ami306.o
obj-$(CONFIG_SENSORS_BH1721)	+= bh1721fvc.o
obj-$(CONFIG_SENSOR_HSCDTD004A)	+= alps_IF_Driver/
obj-$(CONFIG_SENSORS_BMA250)    += bma250_driver.o
obj-$(CONFIG_SENSORS_BMM050)   += bmm050_driver.o bmm050.o

ifeq ($(CONFIG_SENSOR_BMA222_USE_SMBUS),y)
EXTRA_CFLAGS += -DBMA222_SMBUS
endif
ifeq ($(CONFIG_SENSOR_BMA222_WITH_DEBUG),y)
EXTRA_CFLAGS += -DBMA222_DEBUG
endif
ifeq ($(CONFIG_SENSOR_BMA222_HAS_EARLYSUSPEND),y)
EXTRA_CFLAGS += -DBMA222_HAS_EARLYSUSPEND
endif
