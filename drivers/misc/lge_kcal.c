/*
 * arch/arm/mach-msm/lge/lge_emmc_direct_access.c
 *
 * Copyright (C) 2010 LGE, Inc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <asm/div64.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

/* LGE_CHANGE_S: E0 kevinzone.han@lge.com [2012-01-04] 
: For the calibration of LCD Color temperature */
//#ifdef CONFIG_FB_MSM_MDP_LUT_ENABLE
/* LGE_CHANGE_S jungrock.oh@lge.com 2013-01-05 change u0 featuring*/
#ifdef CONFIG_LGE_FB_MSM_MDP_LUT_ENABLE
/* LGE_CHANGE_E jungrock.oh@lge.com 2013-01-05 change u0 featuring */
#define LCD_K_CAL_SIZE 6
static unsigned char lcd_buf[LCD_K_CAL_SIZE]={255,};

int misc_write_block(const char* buffer, int size);
int misc_read_block(char* buffer, int size);
int read_lcd_k_cal( char *buf);

static int write_lcd_k_cal(const char *val, struct kernel_param *kp)
{
	char myBuf[10];
	int i = 0;
	int iRed 		= 0;
	int iGreen = 0;
	int iBlue 	= 0;

	char iChkSum1 = 'c';
	char iChkSum2 = 'a';
	char iChkSum3 = 'l';

	sscanf(val, "%d,%d,%d", &iRed, &iGreen, &iBlue);

	printk(KERN_ERR "================mdp_write_kcal_reg function starts ~~~~~~!\n");
	printk(KERN_ERR "================Param 1 : %d, Param 2 : %d, Param 3: %d ~~~~~~!\n", iRed, iGreen, iBlue);
	//printk(KERN_ERR "================CheckSum 1 : %d, CheckSum 2 : %d, CheckSum 3: %d ~~~~~~!\n", iChkSum1, iChkSum2, iChkSum3);

	lcd_buf[0] = (char)iRed;
	lcd_buf[1] = (char)iGreen;
	lcd_buf[2] = (char)iBlue;
	lcd_buf[3] = iChkSum1;
	lcd_buf[4] = iChkSum2;
	lcd_buf[5] = iChkSum3;

	i = misc_write_block(lcd_buf, LCD_K_CAL_SIZE);
	if (i == LCD_K_CAL_SIZE)
		printk("<6>" "write %d block\n", i);
	else
		printk("<6>" "write fail\n");//*/

	memset(myBuf, 0, 10);

	read_lcd_k_cal(myBuf);
	printk(KERN_ERR "================Param 1 : %d, Param 2 : %d, Param 3: %d ~~~~~~!\n", myBuf[0], myBuf[1], myBuf[2]);
	printk(KERN_ERR "================CheckSum 1 : %d, CheckSum 2 : %d, CheckSum 3: %d ~~~~~~!\n", myBuf[3], myBuf[4], myBuf[5]);

	return 0;
}


int read_lcd_k_cal( char *buf)
{
	int size = misc_read_block(buf, LCD_K_CAL_SIZE);
	buf[LCD_K_CAL_SIZE-1] = '\0';
	return size;
}
EXPORT_SYMBOL(read_lcd_k_cal);
module_param_call(lcd_k_cal, write_lcd_k_cal, read_lcd_k_cal, NULL,S_IWUSR|S_IRUSR|S_IRGRP|S_IWGRP);  

int misc_write_block(const char* buffer, int size)
{
	struct file *fp_misc = NULL;
	mm_segment_t old_fs;
	unsigned int write_bytes = 0;

	// exception handling
	if((buffer == NULL) || size <= 0)
	{
		printk(KERN_ERR "%s, NULL buffer or NULL size : %d\n", __func__, size);
		return 0;
	}

	old_fs=get_fs();
	set_fs(get_ds());

	// try to open
	fp_misc = filp_open("/dev/block/mmcblk0p8", O_WRONLY | O_SYNC, 0);
	if(IS_ERR(fp_misc))
	{
		printk(KERN_ERR "%s, Can not access MISC\n", __func__);
		goto write_fail;
	}

	fp_misc->f_pos = (loff_t) (512 * 44);
	write_bytes = fp_misc->f_op->write(fp_misc, buffer, size, &fp_misc->f_pos);

	if(write_bytes <= 0)
	{
		printk(KERN_ERR "%s, Can not write (MISC) \n", __func__);
		goto write_fail;
	}

write_fail:
	if (!IS_ERR(fp_misc))
		filp_close(fp_misc, NULL);
	set_fs(old_fs);
	
	return write_bytes;
}
EXPORT_SYMBOL(misc_write_block);

int misc_read_block(char* buffer, int size)
{
	struct file *fp_misc = NULL;
	mm_segment_t old_fs;
	unsigned int read_bytes = 0;

	// exception handling
	if((buffer == NULL) || size <= 0)
	{
		printk(KERN_ERR "%s, NULL buffer or NULL size : %d\n", __func__, size);
		return 0;
	}

	old_fs=get_fs();
	set_fs(get_ds());

	// try to open
	fp_misc = filp_open("/dev/block/mmcblk0p8", O_RDONLY | O_SYNC, 0);
	if(IS_ERR(fp_misc))
	{
		printk(KERN_ERR "%s, Can not access MISC (%ld)\n", __func__, PTR_ERR(fp_misc));
		goto read_fail;
	}

	fp_misc->f_pos = (loff_t) (512*44);
	read_bytes = fp_misc->f_op->read(fp_misc, buffer, size, &fp_misc->f_pos);

	if(read_bytes <= 0)
	{
		printk(KERN_ERR "%s, Can not read (MISC) \n", __func__);
		goto read_fail;
	}

read_fail:
	if (!IS_ERR(fp_misc))
		filp_close(fp_misc, NULL);
	set_fs(old_fs);
	
	return read_bytes;
}
EXPORT_SYMBOL(misc_read_block);
/* LGE_CHANGE_E: E0 kevinzone.han@lge.com [2012-01-07] 
: For the calibration of LCD Color temperature */

/* BEGIN: 0013860 jihoon.lee@lge.com 20110111 */
/* ADD 0013860: [FACTORY RESET] ERI file save */


static int __init lge_kcal_init(void)
{
	printk(KERN_INFO"%s: finished\n", __func__);

	return 0;
}

static void __exit lge_kcal_exit(void)
{
	return;
}

module_init(lge_kcal_init);
module_exit(lge_kcal_exit);

MODULE_DESCRIPTION("LGE kcal apis");
MODULE_AUTHOR("SeHyun Kim <sehyuny.kim@lge.com>");
MODULE_LICENSE("GPL");
#endif //CONFIG_FB_MSM_MDP_LUT_ENABLE

