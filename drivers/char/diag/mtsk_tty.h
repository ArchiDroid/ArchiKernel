#ifndef MTSK_TTY_H_
#define MTSK_TTY_H_
/*
 * DIAG MTS for LGE MTS Kernel Driver
 *
 *  lg-msp TEAM <lg-msp@lge.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include <linux/list.h>
#include "stddef.h"
#include "diagchar_hdlc.h"
#include "diagmem.h"
#include "diagchar.h"
#include "diagfwd.h"
#include <linux/diagchar.h>
#ifdef CONFIG_DIAG_OVER_USB
#include <mach/usbdiag.h>
#endif
#ifdef CONFIG_DIAG_BRIDGE_CODE
#include "diagfwd_hsic.h"
#endif
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <asm/current.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef CONFIG_DIAG_BRIDGE_CODE
	#ifndef CONFIG_MACH_MSM8960_FX1
	#define CONFIG_MTS_USE_HSIC
	#endif
#endif

#define MTS_TTY_MODULE_NAME		"MTS_TTY"
#define DIAG_MTS_RX_MAX_PACKET_SIZE        9000     /* max size = 9000B */
#define DIAG_MTS_TX_SIZE        8192
#define MAX_DIAG_MTS_DRV        1
#define DIAG_MSG_F          31
#define DIAG_EVENT_REPORT_F       96 /*96*/
#define DIAG_EXT_MSG_F             0x79/* 121*/
#define DIAG_QSR_EXT_MSG_TERSE_F   0x92 /*146*/
#define DIAG_LOG_CONFIG_F     0x73 /*115*/
#define DIAG_EXT_MSG_CONFIG_F     0x7D /*125*/
#define ASYNC_HDLC_ESC       0x7d

#define MTSK_TTY_IOCTL_MAGIC        'S'
#define MTSK_TTY_OPEN               _IOWR(MTSK_TTY_IOCTL_MAGIC, 0x01, short)
#define MTSK_TTY_CLOSE              _IOWR(MTSK_TTY_IOCTL_MAGIC, 0x02, short)
#define MTSK_TTY_START              _IOWR(MTSK_TTY_IOCTL_MAGIC, 0x03, short)
#define MTSK_TTY_STOP               _IOWR(MTSK_TTY_IOCTL_MAGIC, 0x04, short)
#define MTSK_TTY_CHK_REG            _IOWR(MTSK_TTY_IOCTL_MAGIC, 0x06, short)
#define MTSK_TTY_WRITE_ENABLE       _IOWR(MTSK_TTY_IOCTL_MAGIC, 0x07, short)
#define MTSK_TTY_CHK_HSIC           _IOWR(MTSK_TTY_IOCTL_MAGIC, 0x08, short)
#define MTSK_TTY_SET_56K            _IOWR(MTSK_TTY_IOCTL_MAGIC, 0x09, short)
#define MTSK_TTY_SET_130K           _IOWR(MTSK_TTY_IOCTL_MAGIC, 0x0A, short)
#define MTSK_TTY_FACTORY_MODE       _IOWR(MTSK_TTY_IOCTL_MAGIC, 0x0B, short)

/* PIF cable type */
#define LT_CABLE_56K	6
#define LT_CABLE_130K	7
#define LT_CABLE_910K	11

/* Use SMEM_ID_VENDOR1 for LGE pif detect information. */
#define SMEM_ID_VENDOR1	135

/* TTY driver status */
enum {
	MTS_TTY_NONE            = 0,
	MTS_TTY_INITIAL         = 1,
	MTS_TTY_REGISTERED      = 2,
	MTS_TTY_OPEN            = 3,
	MTS_TTY_HSIC_DATA       = 4,
	MTS_TTY_56K_DATA        = 5,
	MTS_TTY_FACTORY_MODE	= 6,
	MTS_TTY_SEND_MASK	    = 7,
	MTS_TTY_CLOSED
};

struct mts_tty {
	struct tty_driver *tty_drv;
	struct tty_struct *tty_struct ;
};

enum {
	MTS_BYPASS_TYPE_NONE 	= 0,
	MTS_BYPASS_TYPE_MSG 	= 1,
	MTS_BYPASS_TYPE_CMD 	= 2,
	MTS_BYPASS_TYPE_MASK 	= 3,
	MTS_BYPASS_TYPE_MAX 	= 4
};

typedef enum {
	MTSK_TTY_SET_MASK_NONE        = 0,
	MTSK_TTY_SET_MASK_BT_ON       = 1,
	MTSK_TTY_SET_MASK_WIFI_ON     = 2,
	MTSK_TTY_SET_MASK_RF_CDMA_ON  = 3,
	MTSK_TTY_SET_MASK_RF_WCDMA_ON = 4,
	MTSK_TTY_SET_MASK_RF_LTE_ON   = 5,
	MTSK_TTY_SET_MASK_MAX         = 6
} _type_mtsk_tty_mask_table;

extern struct mts_tty *mtsk_tty;
extern int init_mtsk_tty;
extern int mts_state;

int mtsk_tty_modem_request(const unsigned char *buf, int count);
void mtsk_tty_busy_unlock(char *, int);
void mtsk_tty_push (char *buf , int  left);
int mtsk_tty_process (char *, int*, int);

#ifdef CONFIG_MTS_USE_HSIC
/* APQ8064 */
int mtsk_tty_send_mask(struct diag_request *diag_read_ptr);
#define IS_MTS_ACTIVE(x) (x  == MTS_TTY_HSIC_DATA || x  == MTS_TTY_56K_DATA)
#elif defined (CONFIG_MACH_MSM7X25A_V3) || defined (CONFIG_MACH_MSM8X25_V7)
/* V3/V7 */
#define IS_MTS_ACTIVE(x) (x == MTS_TTY_FACTORY_MODE)
#else
/* MSM8960 / MSM8930 */
#define IS_MTS_ACTIVE(x) (x == MTS_TTY_OPEN)
#endif

#define IS_MSG_CMD(c) (c == DIAG_EXT_MSG_F || c == DIAG_QSR_EXT_MSG_TERSE_F)
#define IS_MASK_CMD(c) (c == DIAG_EXT_MSG_CONFIG_F)
#endif /* MTSK_TTY_H_ */
