#
# Makefile for the kernel mobicore drivers
#
##################################
#mobicore_security_team_add
##################################
GUD_ROOT_FOLDER := $(srctree)/drivers/gud20/
##################################
# add our modules to kernel.
##################################
#mobicore_security_team_add
##################################
obj-$(CONFIG_MOBICORE_API20) += mcKernelApi.o
##################################
obj-$(CONFIG_MOBICORE_DRIVER) += mcDrvModule.o
mcDrvModule-objs := MobiCoreDriver/logging.o \
		MobiCoreDriver/ops.o \
		MobiCoreDriver/mem.o \
		MobiCoreDriver/api.o \
		MobiCoreDriver/pm.o \
		MobiCoreDriver/main.o

mcKernelApi-objs := MobiCoreKernelApi/main.o \
		MobiCoreKernelApi/clientlib.o \
		MobiCoreKernelApi/device.o \
		MobiCoreKernelApi/session.o \
		MobiCoreKernelApi/connection.o

# Release mode by default
ccflags-y := -DNDEBUG -include $(GUD_ROOT_FOLDER)/build_tag.h
ccflags-y += -Wno-declaration-after-statement

ccflags-$(CONFIG_MOBICORE_DEBUG) += -DDEBUG
ccflags-$(CONFIG_MOBICORE_VERBOSE) += -DDEBUG_VERBOSE

# Choose one platform from the folder
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver/platforms/EXYNOS_4X12_STD
# MobiCore Driver includes
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver/public
# MobiCore KernelApi required incldes
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreKernelApi/include \
		-I$(GUD_ROOT_FOLDER)/MobiCoreKernelApi/public
