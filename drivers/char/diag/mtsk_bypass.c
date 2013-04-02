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
#define _BUF_SIZE_MTSK_16K 16384
#define _BUF_SIZE_MTSK (_BUF_SIZE_MTSK_16K + 1024)

inline static int mtsk_tty_msg_type (char cmd) {
	if(IS_MSG_CMD(cmd)) // MSG
		return TRUE;

	if(IS_MASK_CMD(cmd)) // MSG CFG
			return TRUE;

	return FALSE;
}

void mtsk_tty_busy_unlock(char *buf, int proc_num) {
	if (buf == (void *)driver->buf_in_1 && driver->ch) {
		driver->in_busy_1 = 0;
		queue_work(driver->diag_wq, &(driver->diag_read_smd_work));
	}
	else if (buf == (void *)driver->buf_in_2 && driver->ch) {
		driver->in_busy_2 =0;
		queue_work(driver->diag_wq, &(driver->diag_read_smd_work));
	}
	else if (buf == (void *)driver->buf_in_qdsp_1 && driver->chqdsp) {
		driver->in_busy_qdsp_1 =0;
		queue_work(driver->diag_wq, &(driver->diag_read_smd_qdsp_work));
	}
	else if (buf == (void *)driver->buf_in_qdsp_2 && driver->chqdsp) {
		driver->in_busy_qdsp_2 =0;
		queue_work(driver->diag_wq, &(driver->diag_read_smd_qdsp_work));
	}
	else if (buf == (void *)driver->buf_in_wcnss_1 && driver->ch_wcnss) {
		driver->in_busy_wcnss_1 = 0;
		queue_work(driver->diag_wq, &(driver->diag_read_smd_wcnss_work));
	}
	else if (buf == (void *)driver->buf_in_wcnss_2 && driver->ch_wcnss) {
		driver->in_busy_wcnss_2 = 0;
		queue_work(driver->diag_wq, &(driver->diag_read_smd_wcnss_work));
	}
#ifdef CONFIG_MTS_USE_HSIC
	else if (proc_num == HSIC_DATA && driver->hsic_device_enabled&& driver->hsic_ch) {
		diagmem_free(driver, (unsigned char *)buf, POOL_TYPE_HSIC);
		queue_work(driver->diag_bridge_wq, &driver->diag_read_hsic_work);
	}
#endif
}

inline int mtsk_tty_msg_prev (char* b, int len, int prev_item, int* buf_pos) {
	int loop = 0;

	for (loop = 0; loop < len; loop++) {
		if(b[loop] == HDLC_END)
			break;
	}

	if(prev_item == MTS_BYPASS_TYPE_CMD)
		*buf_pos = loop + 1;
	else
		*buf_pos = 0;

	return loop + 1;
}

inline int mtsk_tty_msg_main (char* buf, int len, char* b, int buf_pos, int loop_start, int* remind) {
	int loop = 0;
	int pos = 0;

	for (loop = loop_start  ; loop < len; loop++) {
		buf[buf_pos + pos] = b[loop];

		if (buf[buf_pos + pos] == HDLC_END) {
			if(mtsk_tty_msg_type(buf[buf_pos]) == FALSE) /*DIAG CMD */
				buf_pos = buf_pos + pos + 1;
			pos = 0;
		}
		else
			pos++;
	}

	*remind = pos;
	return buf_pos;
}

int mtsk_tty_msg (char* buf, int len, int proc_num) {
	static int prev_item = MTS_BYPASS_TYPE_NONE;
	static char* b = NULL;
	int count = 0;
	int loop_start = 0;
	int buf_pos = 0;
	int remind = 0;

	if(len > _BUF_SIZE_MTSK_16K)
		return len;

	if(b == NULL) {
		b = kzalloc(_BUF_SIZE_MTSK, GFP_KERNEL);

		if(b == NULL)
			goto _mem_fail;
	}

	switch (proc_num) {
#ifdef CONFIG_MTS_USE_HSIC
		case HSIC_DATA:
			if(mts_state ==  MTS_TTY_56K_DATA)
				goto _bypass;
#else
		case MODEM_DATA:
#endif
			break;

		default :
			goto _bypass;
			break;
	}

	memcpy (b, buf , len);

	if (prev_item != MTS_BYPASS_TYPE_NONE)
		loop_start = mtsk_tty_msg_prev(b, len, prev_item, &buf_pos);

	count =  mtsk_tty_msg_main (buf, len, b, buf_pos, loop_start, &remind);
	prev_item = MTS_BYPASS_TYPE_NONE;

	if(remind > 0) {
		if(mtsk_tty_msg_type(buf[count]) == FALSE) {
			count = count + remind;
			prev_item = MTS_BYPASS_TYPE_CMD;
		}
		else
			prev_item = MTS_BYPASS_TYPE_MSG;
	}

	mtsk_tty_push(b ,len);

	if(count == 0)
		mtsk_tty_busy_unlock(buf, proc_num);

	return count;

	_bypass:
	mtsk_tty_push(buf,len);
	mtsk_tty_busy_unlock(buf, proc_num);
	return 0;

	_mem_fail:
	if(b != NULL)
		kfree(b);

	return len;
}

int mtsk_tty_process (char * buf, int* plen, int proc_num) {
	int *plen_local = plen;

	if (init_mtsk_tty == FALSE || !IS_MTS_ACTIVE(mts_state))
		return TRUE;

#ifdef CONFIG_MTS_USE_HSIC
	if(proc_num == HSIC_DATA && driver->hsic_device_enabled)
		plen_local = &(driver->write_len_mdm);
	else
		return TRUE;
#endif

	if(plen_local) {
		if(buf == NULL || *plen_local > _BUF_SIZE_MTSK_16K)
			return TRUE;

#if defined (CONFIG_MACH_MSM7X25A_V3) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
		mutex_lock(&driver->diagchar_mutex);
#endif
		*plen_local = mtsk_tty_msg(buf , *plen_local, proc_num);
#if defined (CONFIG_MACH_MSM7X25A_V3) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
		mutex_unlock(&driver->diagchar_mutex);
#endif
	}

	return *plen_local;
}
#endif
