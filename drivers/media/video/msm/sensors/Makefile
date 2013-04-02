GCC_VERSION      := $(shell $(CONFIG_SHELL) $(PWD)/scripts/gcc-version.sh $(CROSS_COMPILE)gcc)
#LGE_CHANGE_S : 2012-12-28 sungmin.cho@lge.com vt camera flicker
ifeq ($(findstring cis, $(TARGET_PRODUCT)),cis)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
else ifeq ($(findstring cn, $(TARGET_PRODUCT)),cn)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
else ifeq ($(findstring ame, $(TARGET_PRODUCT)),ame)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
else ifeq ($(findstring hk, $(TARGET_PRODUCT)),hk)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
else ifeq ($(findstring eu, $(TARGET_PRODUCT)),eu)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
else ifeq ($(findstring esa, $(TARGET_PRODUCT)),esa)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
else ifeq ($(findstring in, $(TARGET_PRODUCT)),in)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
else ifeq ($(findstring au, $(TARGET_PRODUCT)),au)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
else ifeq ($(findstring il, $(TARGET_PRODUCT)),il)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
else ifeq ($(findstring nz, $(TARGET_PRODUCT)),nz)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
else ifeq ($(findstring nz, $(TARGET_PRODUCT)),th)
  KBUILD_CFLAGS += -DLGE_CAMERA_ANTIBAND_50HZ
endif
#LGE_CHANGE_E : 2012-12-28 sungmin.cho@lge.com vt camera flicker
EXTRA_CFLAGS += -Idrivers/media/video/msm
EXTRA_CFLAGS += -Idrivers/media/video/msm/io
EXTRA_CFLAGS += -Idrivers/media/video/msm/eeprom
EXTRA_CFLAGS += -Idrivers/media/video/msm/csi
EXTRA_CFLAGS += -Idrivers/media/video/msm/actuators
obj-$(CONFIG_MSM_CAMERA_SENSOR) += msm_sensor.o
obj-$(CONFIG_OV5647) += ov5647_v4l2.o
obj-$(CONFIG_OV8825) += ov8825_v4l2.o
obj-$(CONFIG_IMX074) += imx074_v4l2.o
obj-$(CONFIG_S5K3L1YX) += s5k3l1yx.o
obj-$(CONFIG_IMX091) += imx091.o
obj-$(CONFIG_OV2720) += ov2720.o
obj-$(CONFIG_MT9M114) += mt9m114_v4l2.o
obj-$(CONFIG_S5K4E1) += s5k4e1_v4l2.o
obj-$(CONFIG_MT9E013) += mt9e013_v4l2.o
obj-$(CONFIG_WEBCAM_OV9726) += ov9726_v4l2.o
obj-$(CONFIG_OV7692) += ov7692_v4l2.o
obj-$(CONFIG_VX6953) += vx6953.o
# LGE_CHANGE_S : 2012-09-14 sungmin.cho@lge.com camera bring up
obj-$(CONFIG_T8EV4) += t8ev4_v4l2.o
obj-$(CONFIG_IMX111) += imx111.o
obj-$(CONFIG_HI707) += hi707_v4l2.o
# LGE_CHANGE_E : 2012-09-14 sungmin.cho@lge.com camera bring up
# LGE_CHANGE_S: V1 T4K28 Camera Bringup - aidan.cho@lge.com 2013/03/08
obj-$(CONFIG_T4K28) += t4k28_v4l2.o
# LGE_CHANGE_E: V1 T4K28 Camera Bringup - aidan.cho@lge.com 2013/03/08
# LGE_CHANGE_S: V3 HI351 Camera Bringup - hong.junki@lge.com 2012/09/25
obj-$(CONFIG_HI351) += hi351_v4l2.o
# LGE_CHANGE_E: V3 HI351 Camera Bringup - hong.junki@lge.com 2012/09/25
# LGE_CHANGE_S: 2012/11/2 mjoh@lge.com U0 Camera Bring Up
obj-$(CONFIG_HI542) += hi542_v4l2.o
obj-$(CONFIG_MT9E013_LGIT) += mt9e013_lgit_v4l2.o
obj-$(CONFIG_MT9V113) += mt9v113_v4l2.o
# LGE_CHANGE_E: 2012/11/2 mjoh@lge.com U0 Camera Bring Up
