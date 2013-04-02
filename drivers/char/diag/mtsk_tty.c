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
#include "mtsk_tty.h"
#ifdef CONFIG_LGE_MTS
#define HDLC_END 0x7E

struct mts_tty *mtsk_tty = NULL;

int init_mtsk_tty = FALSE;
int mts_state  = MTS_TTY_NONE;
#if defined (CONFIG_MACH_MSM7X25A_V3) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
extern short mtsk_factory_mode;
#endif

short mts_tty_cable_info (void) {
	unsigned int smem_size = 0;
	unsigned int * cable_info_local ;

	cable_info_local = smem_get_entry(SMEM_ID_VENDOR1, &smem_size);

	if (smem_size == 0 || !cable_info_local) {
		printk("%s : smem_get_entry EFAULT\n", __func__);
		return 0;
	}

	printk("%s : detect %d\n", __func__, *cable_info_local);

	return  (short)*cable_info_local;
}

void mtsk_tty_push (char *buf , int  left) {
	int num_push = 0;
	int total_push = 0;

	struct mts_tty *mtsk_tty_drv = mtsk_tty;

	if(mtsk_tty_drv == NULL)  {
		return;
	}

#ifndef CONFIG_MTS_USE_HSIC
	do {
#endif
		num_push = tty_insert_flip_string(mtsk_tty_drv->tty_struct, buf + total_push, left);
		total_push += num_push;
		left -= num_push;
		tty_flip_buffer_push(mtsk_tty_drv->tty_struct);
#ifndef CONFIG_MTS_USE_HSIC
	} while (left != 0);
#endif
	return;
}

int mtsk_tty_modem_request(const unsigned char *buf, int count) {
	if(!IS_MTS_ACTIVE(mts_state)) {
		return 0;
	}

	if(!IS_MASK_CMD(buf[6])) /*MSG CONFIG*/
		return count;

#ifdef CONFIG_MTS_USE_HSIC
	driver->usb_read_mdm_ptr->actual = count - 6;

	if(driver->usb_read_mdm_ptr->actual >= 2048)
		return count;

	memcpy (driver->usb_buf_mdm_out, (char *)buf+6, driver->usb_read_mdm_ptr->actual);
	driver->usb_read_mdm_ptr->buf = driver->usb_buf_mdm_out;
	driver->usb_read_mdm_ptr->length = USB_MAX_OUT_BUF;
	mtsk_tty_send_mask(driver->usb_read_mdm_ptr);

	queue_work(driver->diag_bridge_wq, &driver->diag_read_mdm_work);
#else
	diag_process_hdlc((void *)buf + 6, count - 6);
#endif
	return count;
}

static int mtsk_tty_open(struct tty_struct *tty, struct file *file) {
	struct mts_tty *mtsk_tty_drv = NULL;

	if (!tty)
		return -ENODEV;

	mtsk_tty_drv = mtsk_tty;

	if (!mtsk_tty_drv)
		return -ENODEV;

	tty->driver_data = mtsk_tty_drv;
	mtsk_tty_drv->tty_struct = tty;

	switch (mts_state)  {
		case MTS_TTY_OPEN:
		case MTS_TTY_HSIC_DATA:
		case MTS_TTY_56K_DATA:
		case MTS_TTY_FACTORY_MODE:
			return -EBUSY;

		default :
			break;
	}

	set_bit(TTY_NO_WRITE_SPLIT, &mtsk_tty_drv->tty_struct->flags);

	mts_state = MTS_TTY_OPEN;

	printk(KERN_INFO "mtsk_tty_open TTY device open %d,%d\n", 0, 0);

	return 0;
}

static void mtsk_tty_close(struct tty_struct *tty, struct file *file) {
	struct mts_tty *mtsk_tty_drv = NULL;

	if (!tty) {
		printk(KERN_INFO "mtsk_tty_close FAIL. tty is Null %d,%d\n", 0, 0);
		return;
	}

	mtsk_tty_drv = tty->driver_data;

	switch (mts_state) {
		case MTS_TTY_OPEN:
		case MTS_TTY_HSIC_DATA:
		case MTS_TTY_56K_DATA:
		case MTS_TTY_FACTORY_MODE:
			printk(KERN_INFO "mtsk_tty_close FAIL. why MTS_TTY_OPEN is not opend  %d,%d \n", mts_state, 0);
			break;

		default :
			return;
	}

	mts_state = MTS_TTY_CLOSED;
	//mtsk_tty_drv->tty_struct = NULL;
	printk(KERN_INFO "mtsk_tty_close SUCCESS. %d,%d \n", mts_state, 0);

	return;
}

static int mtsk_tty_ioctl(struct tty_struct *tty, unsigned int cmd, unsigned long arg) {
	struct mts_tty *mtsk_tty_drv = NULL;
	int ret = 0;
	short info = 0;

	mtsk_tty_drv = mtsk_tty;
	tty->driver_data = mtsk_tty_drv;
	mtsk_tty_drv->tty_struct = tty;

	if (_IOC_TYPE(cmd) != MTSK_TTY_IOCTL_MAGIC) {
		printk(KERN_INFO "mtsk_tty_ioctl %d - %d\n", cmd, 0);
		return -EINVAL;
	}

	switch (cmd) {
		case MTSK_TTY_START:
			printk(KERN_INFO "%s : MTSK_TTY_START\n", __func__);
			break;
		case MTSK_TTY_STOP:
			printk(KERN_INFO "%s : MTSK_TTY_STOP\n", __func__);
			break;
		case MTSK_TTY_CHK_REG:
			info = mts_tty_cable_info();
			if(info == 0)
				return -EFAULT;

			if (copy_to_user((void *)arg, (const void *)&info, sizeof(unsigned int)) == 0) {
				printk(KERN_INFO "%s MTSK_TTY_CHK_REG: complete sending pif cable info %d\n",__func__, info);
			} else {
				printk(KERN_INFO "%s MTSK_TTY_CHK_REG: fail to sending pif cable info\n", __func__);
			}
			break;
#ifdef CONFIG_MTS_USE_HSIC
		case MTSK_TTY_WRITE_ENABLE:
			info = (short)(!(driver->in_busy_hsic_write) && driver->hsic_ch && driver->hsic_device_enabled && mts_state != MTS_TTY_SEND_MASK);
			if (copy_to_user((void *)arg, (const void *)&info , sizeof(info)) != 0) {
				printk(KERN_INFO "%s MTSK_TTY_WRITE_ENABLE fail to sending hsic ch info\n", __func__);
			}
			break;
		case MTSK_TTY_CHK_HSIC:
			info = (short)driver->hsic_device_enabled;

			if (copy_to_user((void *)arg, (const void *)&info , sizeof(info)) != 0) {
				printk(KERN_INFO "%s MTSK_TTY_CHK_HSIC fail to sending hsic_device_enabled \n", __func__);
			}
			break;
		case MTSK_TTY_SET_56K :
			info =	mts_tty_cable_info();

			if(info == LT_CABLE_56K) {
				mts_state = MTS_TTY_56K_DATA;
				diagfwd_disconnect_bridge(1);
				diagfwd_connect_bridge(0);
			}

			if (copy_to_user((void *)arg, (const void *)&info, sizeof(info)) != 0) {
				printk(KERN_INFO "%s MTSK_TTY_STATE_DATA fail to sending hsic ch info\n", __func__);
			}
			break;
		case MTSK_TTY_SET_130K :
			info =	mts_tty_cable_info();

			if(info == LT_CABLE_130K) {
				mts_state = MTS_TTY_HSIC_DATA;
			}

			if (copy_to_user((void *)arg, (const void *)&info, sizeof(info)) != 0) {
				printk(KERN_INFO "%s MTSK_TTY_STATE_DATA fail to sending hsic ch info\n", __func__);
			}
			break;
#endif
#if defined (CONFIG_MACH_MSM7X25A_V3) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
		case MTSK_TTY_FACTORY_MODE:
			info = mtsk_factory_mode;
			if(info)
				mts_state = MTS_TTY_FACTORY_MODE;
			if (copy_to_user((void *)arg, (const void *)&info, sizeof(info)) != 0) {
				printk(KERN_INFO "%s MTSK_TTY_STATE_DATA fail to sending hsic ch info\n", __func__);
			}
			break;
#endif
		default:
			ret = -EINVAL;
			break;
	}

	return ret;
}

static void mtsk_tty_unthrottle(struct tty_struct *tty) {
	return;
}

static int mtsk_tty_write_room(struct tty_struct *tty) {
	return DIAG_MTS_TX_SIZE;
}

static int mtsk_tty_write(struct tty_struct *tty, const unsigned char *buf, int count) {
	int result;
	struct mts_tty *mtsk_tty_drv = NULL;

	mtsk_tty_drv = mtsk_tty;
	tty->driver_data = mtsk_tty_drv;
	mtsk_tty_drv->tty_struct = tty;

	/* check the packet size */
	if (count > DIAG_MTS_RX_MAX_PACKET_SIZE) {
		printk(KERN_INFO "mtsk_tty_write packet size  %d,%d\n", count, DIAG_MTS_RX_MAX_PACKET_SIZE);
		return -EPERM;
	}

	result = mtsk_tty_modem_request(buf, count);
	return result;
}

static const struct tty_operations mtsk_tty_ops = {
	.open = mtsk_tty_open,
	.close = mtsk_tty_close,
	.write = mtsk_tty_write,
	.write_room = mtsk_tty_write_room,
	.unthrottle = mtsk_tty_unthrottle,
	.ioctl = mtsk_tty_ioctl,
};

static int __init mtsk_tty_init(void) {
	int ret = 0;
	struct device *tty_dev =  NULL;
	struct mts_tty *mtsk_tty_drv = NULL;

	pr_info(MTS_TTY_MODULE_NAME ": %s\n", __func__);
	mtsk_tty_drv = kzalloc(sizeof(struct mts_tty), GFP_KERNEL);

	if (mtsk_tty_drv == NULL) {
		printk(KERN_INFO "lge_diag_mts_init FAIL %d - %d\n", 0, 0);
		return 0;
	}

	mtsk_tty = mtsk_tty_drv;
	mtsk_tty_drv->tty_drv = alloc_tty_driver(MAX_DIAG_MTS_DRV);

	if (!mtsk_tty_drv->tty_drv) {
		printk(KERN_INFO "lge_diag_mts_init init FAIL %d - %d\n", 1, 0);
		kfree(mtsk_tty_drv);
		return 0;
	}

	mtsk_tty_drv->tty_drv->name = "mtsk_tty";
	mtsk_tty_drv->tty_drv->owner = THIS_MODULE;
	mtsk_tty_drv->tty_drv->driver_name = "mtsk_tty";

	/* uses dynamically assigned dev_t values */
	mtsk_tty_drv->tty_drv->type = TTY_DRIVER_TYPE_SERIAL;
	mtsk_tty_drv->tty_drv->subtype = SERIAL_TYPE_NORMAL;
	mtsk_tty_drv->tty_drv->flags = TTY_DRIVER_REAL_RAW     | TTY_DRIVER_DYNAMIC_DEV | TTY_DRIVER_RESET_TERMIOS;

	/* initializing the mts driver */
	mtsk_tty_drv->tty_drv->init_termios = tty_std_termios;
	mtsk_tty_drv->tty_drv->init_termios.c_iflag = IGNBRK | IGNPAR;
	mtsk_tty_drv->tty_drv->init_termios.c_oflag = 0;
	mtsk_tty_drv->tty_drv->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	mtsk_tty_drv->tty_drv->init_termios.c_lflag = 0;
	tty_set_operations(mtsk_tty_drv->tty_drv, &mtsk_tty_ops);
	ret = tty_register_driver(mtsk_tty_drv->tty_drv);

	if (ret) {
		put_tty_driver(mtsk_tty_drv->tty_drv);
		mtsk_tty_drv->tty_drv = NULL;
		kfree(mtsk_tty_drv);
		return 0;
	}

	tty_dev = tty_register_device(mtsk_tty_drv->tty_drv, 0, NULL);

	if (IS_ERR(tty_dev)) {
		tty_unregister_driver(mtsk_tty_drv->tty_drv);
		put_tty_driver(mtsk_tty_drv->tty_drv);
		kfree(mtsk_tty_drv);
		return 0;
	}
	mts_state = MTS_TTY_REGISTERED;


	printk(KERN_INFO "mtsk_tty_init  SUCESS MTS_TTY_REGISTERED \n");
	init_mtsk_tty = TRUE;

	return 0;
}

static void __exit mtsk_tty_exit(void) {
	int ret = 0;
	struct mts_tty *mtsk_tty_drv = NULL;

	init_mtsk_tty = FALSE;

	mtsk_tty_drv = mtsk_tty;

	if (!mtsk_tty_drv) {
		pr_err(MTS_TTY_MODULE_NAME ": %s:"
				"NULL mtsk_tty_drv", __func__);
		return;
	}

	mdelay(20);

	tty_unregister_device(mtsk_tty_drv->tty_drv, 0);
	ret = tty_unregister_driver(mtsk_tty_drv->tty_drv);
	put_tty_driver(mtsk_tty_drv->tty_drv);
	mts_state = MTS_TTY_NONE;
	mtsk_tty_drv->tty_drv = NULL;

	kfree(mtsk_tty_drv);
	mtsk_tty = NULL;
	printk(KERN_INFO "mtsk_tty_exit  SUCESS %d - %d\n", 0, 0);
	return;
}

module_init(mtsk_tty_init);
module_exit(mtsk_tty_exit);

MODULE_DESCRIPTION("LGE MTS TTY");
MODULE_LICENSE("GPL");
MODULE_AUTHOR(" lg-msp TEAM <lg-msp@lge.com>");
#endif
