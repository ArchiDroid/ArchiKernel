/*
 * MMS100S ISC Updater의 알고리듬이 구현된 부분입니다.
 * !!주의!!
 * 내용을 수정하시면 안됩니다.
 */
#include "MMS100S_ISC_Updater_Customize.h"
#define MFS_HEADER_		5
#define MFS_DATA_		20480

#define DATA_SIZE 1024
#define CLENGTH 4
#define PACKET_			(MFS_HEADER_ + MFS_DATA_)

/*
 * State Registers
 */

/*
 * Config Update Commands
 */
#define ISC_CMD_ENTER_ISC						0x5F
#define ISC_CMD_ENTER_ISC_PARA1					0x01
#define ISC_CMD_ISC_ADDR						0xD5
#define ISC_CMD_ISC_STATUS_ADDR					0xD9

/*
 * ISC Status Value
 */
#define ISC_STATUS_RET_MASS_ERASE_DONE			0X0C
#define ISC_STATUS_RET_MASS_ERASE_MODE			0X08

//typedef int

#define MFS_CHAR_2_BCD(num)	\
	(((num/10)<<4) + (num%10))
#define MFS_MAX(x, y)		( ((x) > (y))? (x) : (y) )

#define MFS_DEFAULT_SLAVE_ADDR	0x48


// static eMFSRet_t exit_config_update_mode(void);
static int firmware_write(const unsigned char *_pBinary_Data);
static int firmware_verify(const unsigned char *_pBinary_Data);
static int mass_erase(void);

#if 0
static unsigned char *buf;
static void open_bin(void);
static eMFSRet_t close_bin(void);
static FILE* fh;
void MFS_open_bins(void)
{
	buf = (unsigned char*) malloc(PACKET_);
	open_bin();
}

mfs_bool_t MFS_close_bins(void)
{
	free(buf);
	return close_bin();
}
#endif

eMFSRet_t MFS_ISC_update(void)
{
	eMFSRet_t ret;

	/*Config 모드 진입 전에는 설정된 slave address를 사용.*/
	MFS_I2C_set_slave_addr(mfs_i2c_slave_addr);

	// Reboot 해야 합니다!!!
	#if 1 // blood
	MFS_reboot();
	#endif

	if ((ret = mass_erase()) && ret != MRET_SUCCESS)
	{
		/* Slave Address가 Default Address(0x48)와 다른  경우에만 필요합니다.*/
		/* Slave Address가 0x48일 경우에는 goto MCSDL_DOWNLOAD_FINISH;만 있으면 됩니다. */

		/* Slave Address != Default Address(0x48) 일 경우 Code start!!! */
		if (mfs_i2c_slave_addr != MFS_DEFAULT_SLAVE_ADDR)
		{
			MFS_debug_msg("<MELFAS> Slave Address is not matched!!!\n", 0, 0,
					0);
			MFS_debug_msg(
					"<MELFAS> Retry Mass Erase with the Default Address(0x%2X)!!!\n\n",
					MFS_DEFAULT_SLAVE_ADDR, 0, 0);

			#if 1 // blood
			printk("<MELFAS> Slave Address is not matched!!!\n");
			printk("<MELFAS> Retry Mass Erase with the Default Address(0x%2X)!!!\n\n",
		            MFS_DEFAULT_SLAVE_ADDR);
			#endif

			MFS_I2C_set_slave_addr(MFS_DEFAULT_SLAVE_ADDR);
			if ((ret = mass_erase()) && ret != MRET_SUCCESS)
				goto MCSDL_DOWNLOAD_FINISH;
		}
		else
			/* Slave Address != Default Address(0x48) 일 경우 Code end!!! */
			goto MCSDL_DOWNLOAD_FINISH;
	}

	MFS_reboot();
	MFS_I2C_set_slave_addr(MFS_DEFAULT_SLAVE_ADDR);


	/* 2012-10-23 JongWook-Park(blood9874@lge.com) [V3] Single Touch Bring Up [START] */
#if defined(CONFIG_MACH_MSM7X25A_V3_DS)
	if ((ret = firmware_write(MELFAS_binary)) && ret != MRET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;

	if ((ret = firmware_verify(MELFAS_binary)) && ret != MRET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;
#else
	if ((ret = firmware_write(MELFAS_SINGLE_binary)) && ret != MRET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;

	if ((ret = firmware_verify(MELFAS_SINGLE_binary)) && ret != MRET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;
#endif
	/* 2012-10-23 JongWook-Park(blood9874@lge.com) [V3] Single Touch Bring Up [END] */

	MFS_debug_msg("<MELFAS> FIRMWARE_UPDATE_FINISHED!!!\n\n", 0, 0, 0);

	#if 1 // blood
	printk("<MELFAS> FIRMWARE_UPDATE_FINISHED!!!\n\n");
	#endif

	MCSDL_DOWNLOAD_FINISH:

	MFS_I2C_set_slave_addr(mfs_i2c_slave_addr);

	#if 1 // blood
	MFS_reboot();
	#endif

	// Reboot 해야 합니다!!!
	return ret;
}

#if 1 // blood
//static 	unsigned char write_buffer[MFS_HEADER_ + DATA_SIZE];
static 	unsigned char working_buffer[MFS_HEADER_ + DATA_SIZE];
#endif

int firmware_write(const unsigned char *_pBinary_Data)
{
	int i, k = 0;
	#if 0 // blood
	unsigned char write_buffer[MFS_HEADER_ + DATA_SIZE];
	#endif
	unsigned short int start_addr = 0;

	MFS_debug_msg("<MELFAS> FIRMARE WRITING...\n", 0, 0, 0);
	MFS_debug_msg("<MELFAS> ", 0, 0, 0);

	#if 1 // blood
	printk("<MELFAS> FIRMARE WRITING...\n");
	printk("<MELFAS> FIRMARE WRITING...\n");
	#endif


	while (start_addr * CLENGTH < MFS_DATA_)
	{
		#if 0
		write_buffer[0] = ISC_CMD_ISC_ADDR;
		write_buffer[1] = (unsigned char) ((start_addr) & 0X00FF);
		write_buffer[2] = (unsigned char) ((start_addr >> 8) & 0X00FF);
		write_buffer[3] = 0X00;
		write_buffer[4] = 0X00;

		for (i = 0; i < DATA_SIZE; i++)
			write_buffer[MFS_HEADER_ + i] = _pBinary_Data[i
					+ start_addr * CLENGTH];
		#else
		working_buffer[0] = ISC_CMD_ISC_ADDR;
		working_buffer[1] = (unsigned char) ((start_addr) & 0X00FF);
		working_buffer[2] = (unsigned char) ((start_addr >> 8) & 0X00FF);
		working_buffer[3] = 0X00;
		working_buffer[4] = 0X00;

		for (i = 0; i < DATA_SIZE; i++)
			working_buffer[MFS_HEADER_ + i] = _pBinary_Data[i
					+ start_addr * CLENGTH];

		#endif
		//MFS_ms_delay(5);
		MFS_ms_delay(10);

		if (!MFS_I2C_write(working_buffer, MFS_HEADER_ + DATA_SIZE))
		{
			#if 1 // blood
			printk("<MELFAS> firmware_write fail (1)!! \n\n");
			#endif
			
			return MRET_I2C_ERROR;

		}

		MFS_debug_msg("%dKB ", k, 0, 0);

		#if 1 // blood
		printk("%dKB ", k);
		#endif


		//MFS_ms_delay(5);

		k++;
		start_addr = DATA_SIZE * k / CLENGTH;
	}
	MFS_debug_msg("\n", 0, 0, 0);

	#if 1 // blood
	printk("firmware_write ... SUCCESS !!\n");
	#endif

	return MRET_SUCCESS;
}

int firmware_verify(const unsigned char *_pBinary_Data)
{
	int i, k = 0;
	#if 0
	unsigned char write_buffer[MFS_HEADER_], read_buffer[DATA_SIZE];
	#else // blood
	unsigned char write_buffer[MFS_HEADER_];
	#endif
	unsigned short int start_addr = 0;

	MFS_debug_msg("<MELFAS> FIRMARE VERIFY...\n", 0, 0, 0);
	MFS_debug_msg("<MELFAS> ", 0, 0, 0);

	#if 1 // blood
	printk("<MELFAS> FIRMARE VERIFY...\n");
	printk("<MELFAS> FIRMARE VERIFY...\n");
	#endif

	while (start_addr * CLENGTH < MFS_DATA_)
	{
		write_buffer[0] = ISC_CMD_ISC_ADDR;
		write_buffer[1] = (unsigned char) ((start_addr) & 0X00FF);
		write_buffer[2] = 0x40 + (unsigned char) ((start_addr >> 8) & 0X00FF);
		write_buffer[3] = 0X00;
		write_buffer[4] = 0X00;

		#if 1 // blood
		MFS_ms_delay(10);
		#endif

		if (!MFS_I2C_write(write_buffer, MFS_HEADER_))
		{
			#if 1 // blood
			printk("<MELFAS> firmware_verify fail (1)!! \n\n");
			#endif
			return MRET_I2C_ERROR;
		}

		//MFS_ms_delay(5);
		MFS_ms_delay(10);

		#if 0
		if (!MFS_I2C_read(read_buffer, DATA_SIZE))
		{
			#if 1 // blood
			printk("<MELFAS> firmware_verify fail (2)!! \n\n");
			#endif
			return MRET_I2C_ERROR;
		}
		for (i = 0; i < DATA_SIZE; i++)
			if (read_buffer[i] != _pBinary_Data[i + start_addr * CLENGTH])
			{
				MFS_debug_msg("<MELFAS> VERIFY Failed\n", i, 0, 0);
				MFS_debug_msg(
						"<MELFAS> original : 0x%2x, buffer : 0x%2x, addr : %d \n",
						_pBinary_Data[i + start_addr * CLENGTH], read_buffer[i],
						i);
				#if 1 // blood
				printk("<MELFAS> VERIFY Failed\n");
				printk("<MELFAS> original : 0x%2x, buffer : 0x%2x, addr : %d \n",
						_pBinary_Data[i + start_addr * CLENGTH], read_buffer[i],
						i);				
				#endif
				
				return MRET_FIRMWARE_VERIFY_ERROR;
			}
		#else // blood
		if (!MFS_I2C_read(working_buffer, DATA_SIZE))
		{
			#if 1 // blood
			printk("<MELFAS> firmware_verify fail (2)!! \n\n");
			#endif
			return MRET_I2C_ERROR;
		}
		for (i = 0; i < DATA_SIZE; i++)
			if (working_buffer[i] != _pBinary_Data[i + start_addr * CLENGTH])
			{
				MFS_debug_msg("<MELFAS> VERIFY Failed\n", i, 0, 0);
				MFS_debug_msg(
						"<MELFAS> original : 0x%2x, buffer : 0x%2x, addr : %d \n",
						_pBinary_Data[i + start_addr * CLENGTH], working_buffer[i],
						i);
				#if 1 // blood
				printk("<MELFAS> VERIFY Failed\n");
				printk("<MELFAS> original : 0x%2x, buffer : 0x%2x, addr : %d \n",
						_pBinary_Data[i + start_addr * CLENGTH], working_buffer[i],
						i);				
				#endif
				
				return MRET_FIRMWARE_VERIFY_ERROR;
			}
		#endif

		MFS_debug_msg("%dKB ", k, 0, 0);
		#if 1 // blood
		printk("%dKB ", k);
		#endif

		k++;
		start_addr = DATA_SIZE * k / CLENGTH;

	}
	MFS_debug_msg("\n", 0, 0, 0);
	#if 1 // blood
	printk("firmware_verify.... SUCCESS!!!\n");
	#endif

	return MRET_SUCCESS;
}

int mass_erase(void)
{
	int i = 0;
	const unsigned char mass_erase_cmd[MFS_HEADER_] =
	{ ISC_CMD_ISC_ADDR, 0, 0xC1, 0, 0 };
	unsigned char read_buffer[4] =
	{ 0, };

	MFS_debug_msg("<MELFAS> mass erase start\n\n", 0, 0, 0);

	#if 1 // blood
	printk("<MELFAS> mass erase start\n\n");
	#endif

	//MFS_ms_delay(5);
	MFS_ms_delay(10);

	if (!MFS_I2C_write(mass_erase_cmd, MFS_HEADER_))
	{
		printk("<MELFAS> mass erase fail (1)!! \n\n");

		return MRET_I2C_ERROR;
	}

	//MFS_ms_delay(5);
	MFS_ms_delay(10);

	while (read_buffer[2] != ISC_STATUS_RET_MASS_ERASE_DONE)
	{
		if (!MFS_I2C_read_with_addr(read_buffer, ISC_CMD_ISC_STATUS_ADDR, 4))
		{
			printk("<MELFAS> mass erase fail (2)!! \n\n");

			return MRET_I2C_ERROR;
		}
		
		if (read_buffer[2] == ISC_STATUS_RET_MASS_ERASE_DONE)
		{
			MFS_debug_msg("<MELFAS> Firmware Mass Erase done.\n", 0, 0, 0);
			#if 1 // blood
			printk("<MELFAS> Firmware Mass Erase done.\n");
			#endif

			return MRET_SUCCESS;
		}
		else if (read_buffer[2] == ISC_STATUS_RET_MASS_ERASE_MODE)
		{
			MFS_debug_msg("<MELFAS> Firmware Mass Erase enter success!!!\n", 0,	0, 0);
			#if 1 // blood
			printk("<MELFAS> Firmware Mass Erase enter success!!!\n");
			#endif
		}
		//MFS_ms_delay(1);
		MFS_ms_delay(5);

		if (i > 20)
			return MRET_MASS_ERASE_ERROR;
		i++;
	}
	return MRET_SUCCESS;
}

#if 0
static void open_bin(void)
{
	char* s;
	int maxlen = 0;
	if (maxlen < strlen(mfs_bin_filename))
		maxlen = strlen(mfs_bin_filename);

	s = (char*) malloc(maxlen + strlen(mfs_bin_path) + 4);

	s[0] = '\0';
	strcat(s, mfs_bin_path);
	strcat(s, mfs_bin_filename);
	fh = fopen(s, "rb");

	free(s);
}

eMFSRet_t close_bin(void)
{
	if (fh != NULL && fclose(fh))
		return MRET_FILE_CLOSE_ERROR;
	return MRET_SUCCESS;
}
#endif
