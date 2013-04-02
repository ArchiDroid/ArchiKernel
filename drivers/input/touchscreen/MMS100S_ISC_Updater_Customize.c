/*
 * MMS100S ISC Updater를 customize하기 위한 소스입니다.
 * 포팅을 위해 수정하셔야 하는 소스입니다.
 */
#include "MMS100S_ISC_Updater_Customize.h"

#define ON	1
#define OFF	0

/*
 *	MMS100S_ISC_Updater_V03.c의 MMS100S_FIRMWARE.c를 펌웨어 파일명으로 변경해 주세요.
 * */

/*
 *  TODO: 통신에 사용하는 slave address를 기록해 주세요.
 * !! 주의를 부탁드립니다 !!
 * Config 실패 시 사용되는 default slave address는 0x48 입니다.
 * 해당 slave address를 사용하는 I2C slave device가 없는지 확인해 주시고,
 * 만일 있다면 MMM100S를 다운로드 할 때는 해당 slave device를 disable 시켜 주십시오.
 */
const unsigned char mfs_i2c_slave_addr = 0x48;

/*
 * TODO: .bin file의 경로를 설정해 주세요.
 * 마지막 경로 표시문자(e.g. slash)를 함께 입력해 주세요.
 */
char* mfs_bin_path;// = "./";


/*
 * TODO: .bin을 제외한 각section의 filename을 설정해 주세요.
 */

char* mfs_bin_filename;

/*현재 setting된 slave address를 알고 싶을 때 참조하세요.*/
unsigned char mfs_slave_addr;

#if defined (CONFIG_MACH_LGE)
#define SUPPORT_TOUCH_KEY 1
#else
#define SUPPORT_TOUCH_KEY 0
#endif

#if SUPPORT_TOUCH_KEY
#define LG_FW_HARDKEY_BLOCK
#endif

struct mms100s_ts_device {
	struct i2c_client *client;
	struct input_dev *input_dev;
	/* struct delayed_work work; */
	struct work_struct  work;
#ifdef LG_FW_HARDKEY_BLOCK
	struct hrtimer touch_timer;
	bool hardkey_block;
#endif
	int num_irq;
	int intr_gpio;
	int scl_gpio;
	int sda_gpio;
	bool pendown;
	int (*power)(unsigned char onoff);
	struct workqueue_struct *ts_wq;

	/*20110607 seven.kim@lge.com for touch frimware download [START] */
	struct wake_lock wakelock;
	int irq_sync;
	int fw_version;
	int hw_version;
	int status;
	int tsp_type;
	/*20110607 seven.kim@lge.com for touch frimware download [END] */
};
extern struct mms100s_ts_device mcs8000_ts_dev;

mfs_bool_t MFS_I2C_set_slave_addr(unsigned char _slave_addr)
{
	mfs_slave_addr = _slave_addr << 1; /*수정하지 마십시오.*/

	/* TODO: I2C slave address를 셋팅해 주세요. */
	return MFS_TRUE;
}

mfs_bool_t MFS_I2C_read_with_addr(unsigned char* _read_buf,
		unsigned char _addr, int _length)
{
	/* TODO: I2C로 1 byte address를 쓴 후 _length 갯수만큼 읽어 _read_buf에 채워 주세요. */

	unsigned char ucTXBuf[1] = {0};
	int iRet = 0;
	struct mms100s_ts_device *dev = NULL;

	dev = &mcs8000_ts_dev;
	ucTXBuf[0] = _addr;

	iRet = i2c_master_send(dev->client, ucTXBuf, 1);
	if(iRet < 0)
	{
		printk(KERN_ERR "MFS_I2C_read_with_addr: i2c failed\n");
		return MFS_FALSE;	
	}

	iRet = i2c_master_recv(dev->client, _read_buf, _length);
	if(iRet < 0)
	{
		printk(KERN_ERR "MFS_I2C_read_with_addr: i2c failed\n");
		return MFS_FALSE;	
	}
	return MFS_TRUE;
}

mfs_bool_t MFS_I2C_write(const unsigned char* _write_buf, int _length)
{
	/*
	 * TODO: I2C로 _write_buf의 내용을 _length 갯수만큼 써 주세요.
	 * address를 명시해야 하는 인터페이스의 경우, _write_buf[0]이 address가 되고
	 * _write_buf+1부터 _length-1개를 써 주시면 됩니다.
	 */
	int iRet = 0;
	struct mms100s_ts_device *dev = NULL;

	dev = &mcs8000_ts_dev;

	iRet = i2c_master_send(dev->client, _write_buf, _length);
	if(iRet < 0)
	{
		printk(KERN_ERR "MFS_I2C_write: i2c failed\n");
		return MFS_FALSE;	
	}

	return MFS_TRUE;
}

mfs_bool_t MFS_I2C_read(unsigned char* _read_buf, int _length)
{
	/* TODO: I2C로 _length 갯수만큼 읽어 _read_buf에 채워 주세요. */
	int iRet = 0;
	struct mms100s_ts_device *dev = NULL;

	dev = &mcs8000_ts_dev;

	iRet = i2c_master_recv(dev->client, _read_buf, _length);
	if(iRet < 0)
	{
		printk(KERN_ERR "MFS_I2C_read: i2c failed\n");
		return MFS_FALSE;	
	}

	return MFS_TRUE;
}

void MFS_debug_msg(const char* fmt, int a, int b, int c)
{
#if 0
	log_printf(0, fmt, a, b, c);
#endif
}
void MFS_ms_delay(int msec)
{
	msleep(msec);
}

void MFS_reboot(void)
{

	struct mms100s_ts_device *dev = NULL;
	dev = &mcs8000_ts_dev;

	printk("<MELFAS> TOUCH IC REBOOT!!!\n");
	MFS_debug_msg("<MELFAS> TOUCH IC REBOOT!!!\n", 0, 0, 0);

	dev->power(OFF);
	msleep(100);

	dev->power(ON);
	msleep(100);
}
