obj-$(CONFIG_DIAG_CHAR) := diagchar.o
obj-$(CONFIG_DIAG_SDIO_PIPE) += diagfwd_sdio.o
obj-$(CONFIG_DIAG_BRIDGE_CODE) += diagfwd_hsic.o
obj-$(CONFIG_DIAG_BRIDGE_CODE) += diagfwd_smux.o
diagchar-objs := diagchar_core.o diagchar_hdlc.o diagfwd.o diagmem.o diagfwd_cntl.o diag_dci.o
obj-$(CONFIG_LGE_MTS) += mtsk_tty.o mtsk_bypass.o
obj-$(CONFIG_LGE_MTS_CHAR) += mtschar.o
# LGE TestMode interface porting, myunghwan.kim@lge.com [START]
ifeq ($(CONFIG_LGE_DIAGTEST), y)
	diagchar-objs += lg_diag_kernel_service.o
endif
# LGE TestMode interface porting, myunghwan.kim@lge.com [END]
