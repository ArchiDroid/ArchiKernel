
#pragma once

#include <linux/input/rmi_driver.h>

#include <linux/input/rmi.h>

// ==========================================================================================================================

#define	INDEX( VALUE )			VALUE

#define	IS_ASCII_CODE( CHARACTER )	( CHARACTER <= 127 ? ( CHARACTER <= 31 ? '.' : CHARACTER ) : '?' )

#define	TO_INT( ARRAY )			( *( ARRAY + 1 ) * 0x100 + *ARRAY )

#define	CASE( NAME )			case NAME :

// ==========================================================================================================================

#define	ONE_REGISTER			1

#define	MAX_BLOCK_DATA			16

#define	MEMORY_BUFFER_SIZE		1024

// ==========================================================================================================================

#define	RETURN_VALUE_STRING		"a9115055"

// ==========================================================================================================================

#define	BLOCK_DATA_NUMBER_STRING	"REG  | 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15\n-----+------------------------------------------------"

#define	BLOCK_DATA_CHARACTER_STRING	"\nREG  | 0 1 2 3 4 5 6 7 8 9 a b c d e f\n-----+---------------------------------"

#define	BUFFER_NUMBER_STRING		"MEM  | 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15\n-----+------------------------------------------------"

#define	BUFFER_CHARACTER_STRING		"\nMEM  | 0 1 2 3 4 5 6 7 8 9 a b c d e f\n-----+---------------------------------"

// ==========================================================================================================================

#define	F01_COMMAND0_RETURN_STRING	"Software reset"

// ==========================================================================================================================

#define	INFO_BUFFER_RESET		0

#define	INFO_BUFFER_INIT		1

#define	INFO_BUFFER_ADD			2

// ==========================================================================================================================

// 0 ~ 19
#define	COMMAND_S_GET_TOUCH_IC_INFO			0

#define	COMMAND_S_READ_VALUE_FROM_FUNCTION_GROUP	1

#define	COMMAND_S_WRITE_VALUE_TO_FUNCTION_GROUP		2

#define	COMMAND_S_READ_VALUE_FROM_REGISTER		3

#define	COMMAND_S_WRITE_VALUE_TO_REGISTER		4

#define	COMMAND_S_READ_BLOC_DATA_FROM_FUNCTION_GROUP	5

#define	COMMAND_S_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP	6

#define	COMMAND_S_READ_BLOC_DATA_FROM_REGISTER		7

#define	COMMAND_S_WRITE_BLOC_DATA_FROM_REGISTER		8

// 20 ~ 39
#define	COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP	21

#define	COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP		22

#define	COMMAND_N_READ_VALUE_FROM_REGISTER		23

#define	COMMAND_N_WRITE_VALUE_TO_REGISTER		24

#define	COMMAND_N_READ_BLOC_DATA_FROM_FUNCTION_GROUP	25

#define	COMMAND_N_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP	26

#define	COMMAND_N_READ_BLOC_DATA_FROM_REGISTER		27

#define	COMMAND_N_WRITE_BLOC_DATA_FROM_REGISTER		28

// 40 ~ 59
#define	COMMAND_ENTER_BOOTLOADER_MODE			40

#define	COMMAND_MEMORY_TO_FILE				41

#define	COMMAND_MEMORY_TO_FLASH				42

#define	COMMAND_FLASH_TO_MEMORY				43

#define	COMMAND_CONFIGURATION_TO_MEMORY			44

#define	COMMAND_CONFIGURATION_TO_FLASH			45

#define	COMMAND_FILE_TO_MEMORY				46

#define	COMMAND_FILE_TO_FLASH				47

// 60 ~ 79
#define	COMMAND_CONVERT_NUMBER_TO_STRING		60

#define	COMMAND_PARA_BUFFER_DUMP_VALUE			61

#define	COMMAND_PARA_BUFFER_DUMP_CHARACTER		62

#define	COMMAND_MEMORY_DUMP_VALUE			63

#define	COMMAND_MEMORY_DUMP_CHARACTER			64

#define	COMMAND_SCAN_PAGE_DESCRIPTION_TABLE		65

// 80 ~ 99
#define	COMMAND_RESET_TOUCH_IC				80

#define	COMMAND_ALLOCATE_FLASH_DATA			81

#define	COMMAND_RELEASE_FLASH_DATA			82

#define	COMMAND_ALLOCATE_FILE_INFORMATION		83

#define	COMMAND_RELEASE_FILE_INFORMATION		84

// ==========================================================================================================================

#define	RMI4_MAX_PAGE				0xff

#define	RMI4_PAGE_SIZE				0x100

#define	RMI4_PDT_START				0xe9

#define	RMI4_PDT_END				0x5

// ==========================================================================================================================

#define	RMI_FUNCTION_01				0x1

#define	RMI_FUNCTION_34				0x34

// ==========================================================================================================================

#define	REGISTER_QUERY_ID			0

#define	REGISTER_COMMAND_ID			1

#define	REGISTER_CONTROL_ID			2

#define	REGISTER_DATA_ID			3

// ==========================================================================================================================

#define	RMI_F01_COMMAND0			0

// ==========================================================================================================================

#define	RMI_F34_QUERY0_BOOTLOADER_ID0		0

#define	RMI_F34_QUERY1_BOOTLOADER_ID1		1

#define	RMI_F34_QUERY2				2

#define	RMI_F34_QUERY3_BLOCK_SIZE_7_0		3

#define	RMI_F34_QUERY4_BLOCK_SIZE_15_8		4

#define	RMI_F34_QUERY5_FW_BLOCK_COUNT_7_0	5

#define	RMI_F34_QUERY6_FW_BLOCK_COUNT_15_8	6

#define	RMI_F34_QUERY7_CONFIG_BLOCK_COUNT_7_0	7

#define	RMI_F34_QUERY8_CONFIG_BLOCK_COUNT_15_8	8

// ==========================================================================================================================

#define	SOFT_RESET_BIT				0x1

// ==========================================================================================================================

#define	DIRECTION_NONE				0

#define	DIRECTION_POSITIVE			1

#define	DIRECTION_NEGATIVE			2

// ==========================================================================================================================

#define	ERROR_NONE				0

#define	ERROR_INVALID_ADDRESS			-1

#define	ERROR_I2C_READ				-2

#define	ERROR_I2C_WRITE				-3

#define	ERROR_BUFFER_FULL			-4

#define	ERROR_RESOURCE_EXIST			-5

#define	ERROR_RESOURCE_NON_EXIST		-6

#define	ERROR_ALLOCATE_FAILED			-7

#define	ERROR_OPEN_FILE_FAILED			-8

#define	ERROR_NOT_FOUND_FUNCTION		-9

#define	ERROR_CLOSE_FILE_FAILED			-10

// ==========================================================================================================================

#define	SHOW_ALL_OF_REGISTER \
{ RMI_FUNCTION_01, REGISTER_QUERY_ID, INDEX( 0 ), INDEX( 0 ), DIRECTION_NONE, "f%02x, Manufacturer ID(%d)\n" }, \
{ RMI_FUNCTION_01, REGISTER_QUERY_ID, INDEX( 3 ), INDEX( 3 ), DIRECTION_NONE, "f%02x, Product Specific Firmware Revision(%d)\n" }, \
{ RMI_FUNCTION_01, REGISTER_QUERY_ID, INDEX( 22 ), INDEX( 22 ), DIRECTION_NONE, "f%02x, Sensor ID(%d)\n" }, \
{ RMI_FUNCTION_01, REGISTER_QUERY_ID, INDEX( 4 ), INDEX( 10 ), DIRECTION_POSITIVE, "f%02x, Serial Number[1:7]=[%02x,%02x,%02x,%02x,%02x,%02x,%02x]\n" }, \
{ RMI_FUNCTION_01, REGISTER_QUERY_ID, INDEX( 11 ), INDEX( 20 ), DIRECTION_POSITIVE, "f%02x, Product ID[1:10]=[%x,%x,%x,%x,%x,%x,%x,%x,%x,%x]\n" }, \
{ RMI_FUNCTION_34, REGISTER_CONTROL_ID, INDEX( 0 ), INDEX( 3 ), DIRECTION_NEGATIVE, "f%02x, Configuration ID[3:0]=[%x,%x,%x,%x]\n" }, \
{ RMI_FUNCTION_34, REGISTER_CONTROL_ID, INDEX( 0 ), INDEX( 3 ), DIRECTION_NEGATIVE, "f%02x, Configuration ID[3:0]=\"%c%c%c%c\"\n" }, \
{ RMI_FUNCTION_34, REGISTER_QUERY_ID, INDEX( 0 ), INDEX( 1 ), DIRECTION_NEGATIVE, "f%02x, Bootloader ID[1:0]=[%x,%x]\n" }, \
{ RMI_FUNCTION_34, REGISTER_QUERY_ID, INDEX( 7 ), INDEX( 8 ), DIRECTION_NEGATIVE, "f%02x, Configuration Block Count(%d%d)\n" }, \
{ RMI_FUNCTION_34, REGISTER_QUERY_ID, INDEX( 5 ), INDEX( 6 ), DIRECTION_NEGATIVE, "f%02x, Firmware Block Count(%d%d)\n" }, \
{ RMI_FUNCTION_34, REGISTER_QUERY_ID, INDEX( 3 ), INDEX( 4 ), DIRECTION_NEGATIVE, "f%02x, The block size is %d%d byte\n" }, \

// ==========================================================================================================================

#define	SHOW_INFORMATION() \
info_print( INFO_BUFFER_ADD, "CMD[0:19]\n", NULL ); \
info_print( INFO_BUFFER_ADD, "%02d : Get Touch IC information, [NULL]\n", COMMAND_S_GET_TOUCH_IC_INFO ); \
info_print( INFO_BUFFER_ADD, "%02d : Read from RMI4 register of function group, [FUNC,REG,INDX]=[H,D,D]\n", COMMAND_S_READ_VALUE_FROM_FUNCTION_GROUP ); \
info_print( INFO_BUFFER_ADD, "%02d : Write to RMI4 register of function group, [FUNC,REG,INDX,VAL]=[H,D,D,H]\n", COMMAND_S_WRITE_VALUE_TO_FUNCTION_GROUP ); \
info_print( INFO_BUFFER_ADD, "%02d : Read from RMI4 register, [ADDR]=[H]\n", COMMAND_S_READ_VALUE_FROM_REGISTER ); \
info_print( INFO_BUFFER_ADD, "%02d : Write to RMI4 register, [ADDR,VAL]=[H,H]\n", COMMAND_S_WRITE_VALUE_TO_REGISTER ); \
info_print( INFO_BUFFER_ADD, "%02d : Read from block data of function group, [FUNC,REG,S_REG,E_REG]=[H,D,D,D]\n", COMMAND_S_READ_BLOC_DATA_FROM_FUNCTION_GROUP ); \
info_print( INFO_BUFFER_ADD, "%02d : Write to block data of function group, [FUNC,REG,S_REG,COUT,VAL0-VAL15]=[H,D,D,D,H-H]\n", COMMAND_S_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP ); \
info_print( INFO_BUFFER_ADD, "%02d : Read from block data of register, [S_REG,E_REG]=[H,H]\n", COMMAND_S_READ_BLOC_DATA_FROM_REGISTER ); \
info_print( INFO_BUFFER_ADD, "%02d : Write to block data of register, [S_REG,COUT,VAL0-VAL15]=[H,D,H-H]\n", COMMAND_S_WRITE_BLOC_DATA_FROM_REGISTER ); \
info_print( INFO_BUFFER_ADD, "\nCMD[20:39]\n", NULL ); \
info_print( INFO_BUFFER_ADD, "%02d : Read from RMI4 register of function group for Linux AP, [FUNC,REG,INDX]=[H,D,D]\n", COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP ); \
info_print( INFO_BUFFER_ADD, "%02d : Write to RMI4 register of function group for Linux AP, [FUNC,REG,INDX,VAL]=[H,D,D,H]\n", COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP ); \
info_print( INFO_BUFFER_ADD, "%02d : Read from RMI4 register for Linux AP, [ADDR]=[H]\n", COMMAND_N_READ_VALUE_FROM_REGISTER ); \
info_print( INFO_BUFFER_ADD, "%02d : Write to RMI4 register for Linux AP, [ADDR,VAL]=[H,H]\n", COMMAND_N_WRITE_VALUE_TO_REGISTER ); \
info_print( INFO_BUFFER_ADD, "%02d : Read from block data of function group for Linux AP, [FUNC,REG,S_REG,E_REG]=[H,D,D,D]\n", COMMAND_N_READ_BLOC_DATA_FROM_FUNCTION_GROUP ); \
info_print( INFO_BUFFER_ADD, "%02d : Write to block data of function group for Linux AP, [FUNC,REG,S_REG,COUT,VAL0-VAL15]=[H,D,D,D,H-H]\n", COMMAND_N_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP ); \
info_print( INFO_BUFFER_ADD, "%02d : Read from block data of register for Linux AP, [S_REG,E_REG]=[H,H]\n", COMMAND_N_READ_BLOC_DATA_FROM_REGISTER ); \
info_print( INFO_BUFFER_ADD, "%02d : Write to block data of register for Linux AP, [S_REG,COUT,VAL0-VAL15]=[H,D,H-H]\n", COMMAND_N_WRITE_BLOC_DATA_FROM_REGISTER ); \
info_print( INFO_BUFFER_ADD, "\nCMD[40:59]\n", NULL ); \
info_print( INFO_BUFFER_ADD, "%02d : Enter bootloader mode, [NULL]\n", COMMAND_ENTER_BOOTLOADER_MODE ); \
info_print( INFO_BUFFER_ADD, "%02d : Write data from memory to file, [NULL]\n", COMMAND_MEMORY_TO_FILE ); \
info_print( INFO_BUFFER_ADD, "%02d : Write data from memory to flash, [NULL]\n", COMMAND_MEMORY_TO_FLASH ); \
info_print( INFO_BUFFER_ADD, "%02d : Read data from flash to memory, [NULL]\n", COMMAND_FLASH_TO_MEMORY ); \
info_print( INFO_BUFFER_ADD, "%02d : Read data from configuration to memory, [NULL]\n", COMMAND_CONFIGURATION_TO_MEMORY ); \
info_print( INFO_BUFFER_ADD, "%02d : Read data from configuration to flash, [NULL]\n", COMMAND_CONFIGURATION_TO_FLASH ); \
info_print( INFO_BUFFER_ADD, "%02d : Read data from file to memory, [NULL]\n", COMMAND_FILE_TO_MEMORY ); \
info_print( INFO_BUFFER_ADD, "%02d : write data from file to flash, [NULL]\n", COMMAND_FILE_TO_FLASH ); \
info_print( INFO_BUFFER_ADD, "\nCMD[60:79]\n", NULL ); \
info_print( INFO_BUFFER_ADD, "%02d : Convert information from number to string, [NULL]\n", COMMAND_CONVERT_NUMBER_TO_STRING ); \
info_print( INFO_BUFFER_ADD, "%02d : Dump memory of parameter buffer( show value ), [NULL]\n", COMMAND_PARA_BUFFER_DUMP_VALUE ); \
info_print( INFO_BUFFER_ADD, "%02d : Dump memory of parameter buffer( show character ), [NULL]\n", COMMAND_PARA_BUFFER_DUMP_CHARACTER ); \
info_print( INFO_BUFFER_ADD, "%02d : Dump memory of flash information( show value ), [NULL]\n", COMMAND_MEMORY_DUMP_VALUE ); \
info_print( INFO_BUFFER_ADD, "%02d : Dump memory of flash information( show character ), [NULL]\n", COMMAND_MEMORY_DUMP_CHARACTER ); \
info_print( INFO_BUFFER_ADD, "%02d : Scan page description table, [NULL]\n", COMMAND_SCAN_PAGE_DESCRIPTION_TABLE ); \
info_print( INFO_BUFFER_ADD, "\nCMD[80:99]\n", NULL ); \
info_print( INFO_BUFFER_ADD, "%02d : Reset touch IC, [NULL]\n", COMMAND_RESET_TOUCH_IC ); \
info_print( INFO_BUFFER_ADD, "%02d : Create flash data, [NULL]\n", COMMAND_ALLOCATE_FLASH_DATA ); \
info_print( INFO_BUFFER_ADD, "%02d : Release flash data, [NULL]\n", COMMAND_RELEASE_FLASH_DATA ); \
info_print( INFO_BUFFER_ADD, "%02d : Create file information, [NAME]=[S]\n", COMMAND_ALLOCATE_FILE_INFORMATION ); \
info_print( INFO_BUFFER_ADD, "%02d : Release file information, [NULL]\n", COMMAND_RELEASE_FILE_INFORMATION ); \
info_print( INFO_BUFFER_ADD, "\n", NULL ); \

// ==========================================================================================================================

#define	COMMAND_LIST \
{ COMMAND_S_GET_TOUCH_IC_INFO, command_S_get_info, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_S_READ_VALUE_FROM_FUNCTION_GROUP, S_read_function_group_value, "%x %d %d", COMMAND_3_PARAMENTER }, \
{ COMMAND_S_WRITE_VALUE_TO_FUNCTION_GROUP, S_write_function_group_value, "%x %d %d %x", COMMAND_4_PARAMENTER }, \
{ COMMAND_S_READ_VALUE_FROM_REGISTER, S_read_register_value, "%x", COMMAND_1_PARAMENTER }, \
{ COMMAND_S_WRITE_VALUE_TO_REGISTER, S_write_register_value, "%x %x", COMMAND_2_PARAMENTER }, \
{ COMMAND_S_READ_BLOC_DATA_FROM_FUNCTION_GROUP, S_read_function_group_block_data, "%x %d %d %d", COMMAND_4_PARAMENTER }, \
{ COMMAND_S_READ_BLOC_DATA_FROM_REGISTER, S_read_register_block_data, "%x %x", COMMAND_2_PARAMENTER }, \
{ COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, N_read_function_group_value, "%x %d %d", COMMAND_3_PARAMENTER }, \
{ COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, N_write_function_group_value, "%x %d %d %x", COMMAND_4_PARAMENTER }, \
{ COMMAND_N_READ_VALUE_FROM_REGISTER, N_read_register_value, "%x", COMMAND_1_PARAMENTER }, \
{ COMMAND_N_WRITE_VALUE_TO_REGISTER, N_write_register_value, "%x %x", COMMAND_2_PARAMENTER }, \
{ COMMAND_N_READ_BLOC_DATA_FROM_FUNCTION_GROUP, N_read_function_group_block_data, "%x %d %d %d", COMMAND_4_PARAMENTER }, \
{ COMMAND_N_READ_BLOC_DATA_FROM_REGISTER, N_read_register_block_data, "%x %x", COMMAND_2_PARAMENTER }, \
{ COMMAND_CONVERT_NUMBER_TO_STRING, convert_information_format, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_PARA_BUFFER_DUMP_VALUE, dump_para_buffer_value, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_PARA_BUFFER_DUMP_CHARACTER, dump_para_buffer_character, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_ENTER_BOOTLOADER_MODE, enter_bootloader_mode, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_S_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP, S_write_function_group_block_data, "%x %d %d %d %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", COMMAND_N_PARAMENTER }, \
{ COMMAND_S_WRITE_BLOC_DATA_FROM_REGISTER, S_write_register_block_data, "%x %d %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", COMMAND_N_PARAMENTER }, \
{ COMMAND_N_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP, N_write_function_group_block_data, "%x %d %d %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", COMMAND_N_PARAMENTER }, \
{ COMMAND_N_WRITE_BLOC_DATA_FROM_REGISTER, N_write_register_block_data, "%x %d %x %x %x %x %x %x %d %x %x %x %x %x %x %x %x %x %x", COMMAND_N_PARAMENTER }, \
{ COMMAND_SCAN_PAGE_DESCRIPTION_TABLE, scan_page_description_table, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_ALLOCATE_FLASH_DATA, create_flash_data, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_RELEASE_FLASH_DATA, release_flash_data, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_RESET_TOUCH_IC, reset_touch_ic, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_ALLOCATE_FILE_INFORMATION, create_file_information, "%s", COMMAND_1_PARAMENTER }, \
{ COMMAND_RELEASE_FILE_INFORMATION, release_file_information, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_MEMORY_DUMP_VALUE, memory_dump_value, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_MEMORY_DUMP_CHARACTER, memory_dump_character, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_MEMORY_TO_FILE, memory_to_file, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_MEMORY_TO_FLASH, memory_to_flash, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_FLASH_TO_MEMORY, flash_to_memory, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_CONFIGURATION_TO_MEMORY, configuration_to_memory, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_CONFIGURATION_TO_FLASH, configuration_to_flash, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_FILE_TO_MEMORY, file_to_memory, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_FILE_TO_FLASH, file_to_flash, NULL, COMMAND_0_PARAMENTER }, \

// ==========================================================================================================================

#define BUFFER_SELECT_INFORMATION \
{ COMMAND_N_READ_BLOC_DATA_FROM_FUNCTION_GROUP, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data.count }, \
{ COMMAND_N_READ_BLOC_DATA_FROM_REGISTER, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data.count }, \
{ COMMAND_N_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data.count }, \
{ COMMAND_N_WRITE_BLOC_DATA_FROM_REGISTER, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data.count }, \
{ COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_N_READ_VALUE_FROM_REGISTER, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_N_WRITE_VALUE_TO_REGISTER, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_RESET_TOUCH_IC, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ALLOCATE_FLASH_DATA, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_RELEASE_FLASH_DATA, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ALLOCATE_FILE_INFORMATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_RELEASE_FILE_INFORMATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ENTER_BOOTLOADER_MODE, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_MEMORY_TO_FILE, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_MEMORY_TO_FLASH, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_FLASH_TO_MEMORY, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_CONFIGURATION_TO_MEMORY, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_CONFIGURATION_TO_FLASH, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_FILE_TO_MEMORY, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_FILE_TO_FLASH, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_RESET_TOUCH_IC, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ALLOCATE_FILE_INFORMATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_RELEASE_FILE_INFORMATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \

// ==========================================================================================================================

#define	RETRUN_STRUCT_LIST \
{ COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, ACTION_1 }, \
{ COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, ACTION_1 }, \
{ COMMAND_N_READ_VALUE_FROM_REGISTER, ACTION_1 }, \
{ COMMAND_N_WRITE_VALUE_TO_REGISTER, ACTION_1 }, \
{ COMMAND_ALLOCATE_FLASH_DATA, ACTION_2 }, \
{ COMMAND_ALLOCATE_FILE_INFORMATION, ACTION_3 }, \
{ COMMAND_RESET_TOUCH_IC, ACTION_3 },\

// ==========================================================================================================================

#define	RETRUN_STRING_LIST \
{ COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, ACTION_1 }, \
{ COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, ACTION_1 }, \
{ COMMAND_N_READ_VALUE_FROM_REGISTER, ACTION_1 }, \
{ COMMAND_N_WRITE_VALUE_TO_REGISTER, ACTION_1 }, \
{ COMMAND_N_READ_BLOC_DATA_FROM_FUNCTION_GROUP, ACTION_2 }, \
{ COMMAND_N_READ_BLOC_DATA_FROM_REGISTER, ACTION_2 }, \
{ COMMAND_N_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP, ACTION_2 }, \
{ COMMAND_N_WRITE_BLOC_DATA_FROM_REGISTER, ACTION_2 }, \
{ COMMAND_ALLOCATE_FLASH_DATA, ACTION_3 }, \
{ COMMAND_ALLOCATE_FILE_INFORMATION, ACTION_4 }, \
{ COMMAND_RESET_TOUCH_IC, ACTION_5 },\

// ==========================================================================================================================

#pragma	pack( push )

#pragma	pack( 1 )

struct	block_info
{

	int	count;

	u8	*value;

};

union	return_value
{

	struct block_info	block_data;

	unsigned int		size;

	char			*name;

	u8			value;

};

struct	return_struct
{

	unsigned int		error_code;

	char			string[ 9 ];

	int			command;

	union return_value	values;

};

#pragma	pack( pop )

enum	buffer_kind
{

	INFORMATION_BUFFER = 0,
	PARAMETER_BUFFER,

};

enum	select_action
{

	ACTION_1 = 0,
	ACTION_2,
	ACTION_3,
	ACTION_4,
	ACTION_5,
	ACTION_6,
	ACTION_7,
	ACTION_8,
	ACTION_9,
	ACTION_10,

};

// ==========================================================================================================================
