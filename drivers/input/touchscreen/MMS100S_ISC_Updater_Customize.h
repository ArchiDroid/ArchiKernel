/*
 * MMS100S ISC Updater를 customize하기 위한 헤더입니다.
 * 포팅을 위해 수정하셔야 하는 헤더입니다.
 */
#ifndef __MMS100S_ISC_Updater_CUSTOMIZE_H__
#define __MMS100S_ISC_Updater_CUSTOMIZE_H__

#include "MMS100S_ISC_Updater.h"

/*
 * TODO: 필요한 header 파일을 include해 주세요.
 * 필요한 인터페이스는 아래와 같습니다.
 * memset, malloc, free, strcmp, strstr, fopen, fclose, delay 함수, 그 외 디버그 메세지를 위한 함수 등.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/wakelock.h>
#include <linux/i2c-gpio.h>
#include CONFIG_LGE_BOARD_HEADER_FILE

#include <asm/uaccess.h>
#include <asm/gpio.h>
#include <asm/io.h>

extern const unsigned char mfs_i2c_slave_addr;

/* TODO: File name을 더 길게 수정해서 관리하는 경우 이 값을 키워 주세요. */
extern char* mfs_bin_filename;
extern char* mfs_bin_path;
extern mfs_bool_t MFS_I2C_set_slave_addr(unsigned char _slave_addr);
extern mfs_bool_t MFS_I2C_read_with_addr(unsigned char* _read_buf, unsigned char _addr, int _length);
extern mfs_bool_t MFS_I2C_write(const unsigned char* _write_buf, int _length);
extern mfs_bool_t MFS_I2C_read(unsigned char* _read_buf, int _length);
extern void MFS_debug_msg(const char* fmt, int a, int b, int c);
extern void MFS_ms_delay(int msec);
extern void MFS_reboot(void);

#endif
