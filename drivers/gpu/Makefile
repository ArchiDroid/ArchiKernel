ifeq ($(CONFIG_VITHAR_HWVER_R0P0),y)
obj-y			+= drm/ vga/ stub/ ion/
else
obj-y			+= drm/ vga/ stub/ ion/
endif

obj-y           += mali400/r3p2/mali/
obj-y           += mali400/r3p2/ump/

ifeq ($(CONFIG_MALI400),y)
ifeq ($(CONFIG_MALI_VER_R3P2),y)
obj-y           += mali400/r3p2/mali/
obj-y           += mali400/r3p2/ump/
else
ifeq ($(CONFIG_MALI_VER_R3P1),y)
obj-y           += mali400/r3p1/mali/
obj-y           += mali400/r3p1/ump/
else
obj-y           += mali400/r2p4/mali/
obj-y           += mali400/r2p4/ump/
endif
endif
endif
