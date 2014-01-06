
#pragma once

// ==========================================================================================================================

#define	INDEX( VALUE )				VALUE

#define	IS_ASCII_CODE( CHARACTER )		( CHARACTER <= 127 ? ( CHARACTER <= 31 ? '.' : CHARACTER ) : '?' )

#define	TO_INT( ARRAY )				( *( ARRAY + 1 ) * 0x100 + *ARRAY )

#define	FLASH_STATUS( REG_VALUE )		( ( REG_VALUE & 0x70 ) >> 4 )

#define	FLASH_STATUS_SUCCESS( REG_VALUE )	( !FLASH_STATUS( REG_VALUE ) )

#define	PROGRAM_ENABLED( REG_VALUE )		( ( REG_VALUE & 0x80 ) >> 7 )

#define	FLASH_PROGRAM( REG_VALUE )		( ( REG_VALUE & 0x40 ) >> 6 )

#define	FULL_POWER_MODE( REG_VALUE )		( REG_VALUE = ( REG_VALUE & 0xf8 ) | 0x4 )

#define	CONVERT_ARRAY_TO_INT( VALUE, ARRAY )	( VALUE = *( ARRAY + 1 ) * 0x100 + *ARRAY )

#define	CONVERT_ARRAY_3_TO_INT( VALUE, ARRAY )	( VALUE = *( ARRAY + 2 ) * 0x10000 + *( ARRAY + 1 ) * 0x100 + *ARRAY )

// ==========================================================================================================================

#define	HISTORY_COUNT			400

// ==========================================================================================================================

#define	ONE_REGISTER			1

#define	TWO_REGISTER			2

#define	THREE_REGISTER			3

#define	FOUR_REGISTER			4

#define	SIX_REGISTER			6

#define	TEN_REGISTER			10

#define	MAX_BLOCK_DATA			16

#define	COMMAND_GROUP_RANGE		20

// ==========================================================================================================================

#define	RETURN_VALUE_STRING		"a9115055"

#define	SECURITY_ID			0xa9115055

// ==========================================================================================================================

#define	INVALID_ADDRESS			( u16 )-1

// ==========================================================================================================================

#define	UI_MODE				0

#define	BOOTLOADER_MODE			1

// ==========================================================================================================================

#define	DISABLE				0

#define	ENABLE				1

// ==========================================================================================================================

#define	WAIT_1000_MS			1000

#define	WAIT_3000_MS			3000

#define	WAIT_5000_MS			5000

#define	MIN_SLEEP_TIME_US		100

#define	MAX_SLEEP_TIME_US		200

// ==========================================================================================================================

#define	BLOCK_DATA_NUMBER_STRING	"REG  | 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15\n-----+------------------------------------------------"

#define	BLOCK_DATA_CHARACTER_STRING	"\nREG  | 0 1 2 3 4 5 6 7 8 9 a b c d e f\n-----+---------------------------------"

#define	BUFFER_NUMBER_STRING		"MEM  | 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15\n-----+------------------------------------------------"

#define	BUFFER_CHARACTER_STRING		"\nMEM  | 0 1 2 3 4 5 6 7 8 9 a b c d e f\n-----+---------------------------------"

// ==========================================================================================================================

#define	F01_COMMAND0_RETURN_STRING	"software reset"

// ==========================================================================================================================

#define	INFO_BUFFER_RESET		0

#define	INFO_BUFFER_INIT		1

#define	INFO_BUFFER_ADD			2

// ==========================================================================================================================

#define	RMI4_MAX_PAGE				0xff

#define	RMI4_PAGE_SIZE				0x100

#define	RMI4_PDT_START				0xe9

#define	RMI4_PDT_END				0x5

// ==========================================================================================================================

#define	REGISTER_READ				0

#define	REGISTER_WRITE				1

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

#define	RMI_F01_DATA0				0

#define	RMI_F01_DATA1				1

#define	RMI_F01_CONTROL0			0

#define	RMI_F01_QUERY11				11

#define	RMI_F01_QUERY18				18

// ==========================================================================================================================

#define	RMI_F34_DATA_FLASH_COMMAND			2

#define	RMI_F34_DATA2					2

#define	RMI_F34_QUERY0_BOOTLOADER_ID0			0

#define	RMI_F34_QUERY1_BOOTLOADER_ID1			1

#define	RMI_F34_QUERY2					2

#define	RMI_F34_QUERY3_BLOCK_SIZE_7_0			3

#define	RMI_F34_QUERY4_BLOCK_SIZE_15_8			4

#define	RMI_F34_QUERY5_FW_BLOCK_COUNT_7_0		5

#define	RMI_F34_QUERY6_FW_BLOCK_COUNT_15_8		6

#define	RMI_F34_QUERY7_CONFIG_BLOCK_COUNT_7_0		7

#define	RMI_F34_QUERY8_CONFIG_BLOCK_COUNT_15_8		8

#define	RMI_F34_DATA_BLOCK_NUMBER_7_0			0

#define	RMI_F34_DATA_BLOCK_NUMBER_15_8			1

#define	RMI_F34_DATA_BLOCK_DATA				2

#define	RMI_F34_CONTROL_CUSTOMER_CONFIGURATION_ID_7_0	0

#define	RMI_F34_CONTROL_CUSTOMER_CONFIGURATION_ID_15_8	1

#define	RMI_F34_CONTROL_CUSTOMER_CONFIGURATION_ID_23_16	2

#define	RMI_F34_CONTROL_CUSTOMER_CONFIGURATION_ID_31_24	3

// ==========================================================================================================================

#define	F34_WRITE_FIRMWARE_BLOCK		0x2

#define	F34_ERASE_ALL				0x3

#define	F34_READ_CONFIGURATION_BLOCK		0x5

#define	F34_WRITE_CONFIGURATION_BLOCK		0x6

#define	F34_ERASE_CONFIGURATION			0x7

#define	F34_READ_SENSOR_ID			0x8

#define	F34_ENABLE_FLASH_PROGRAMMING		0xf

// ==========================================================================================================================

#define	SOFT_RESET_BIT				0x1

// ==========================================================================================================================

#define	DIRECTION_NONE				0

#define	DIRECTION_POSITIVE			1

#define	DIRECTION_NEGATIVE			2

// ==========================================================================================================================

#define	SHOW_ALL_OF_REGISTER \
{ RMI_FUNCTION_01, REGISTER_QUERY_ID, INDEX( 0 ), INDEX( 0 ), DIRECTION_NONE, "f%02x, Manufacturer ID(%d)\n" }, \
{ RMI_FUNCTION_01, REGISTER_QUERY_ID, INDEX( 3 ), INDEX( 3 ), DIRECTION_NONE, "f%02x, Product Specific Firmware Revision(%d)\n" }, \
{ RMI_FUNCTION_01, REGISTER_QUERY_ID, INDEX( 22 ), INDEX( 22 ), DIRECTION_NONE, "f%02x, Sensor ID(%d)\n" }, \
{ RMI_FUNCTION_01, REGISTER_QUERY_ID, INDEX( 4 ), INDEX( 10 ), DIRECTION_POSITIVE, "f%02x, Serial Number[1:7]=[%02x,%02x,%02x,%02x,%02x,%02x,%02x]\n" }, \
{ RMI_FUNCTION_01, REGISTER_QUERY_ID, INDEX( 11 ), INDEX( 20 ), DIRECTION_POSITIVE, "f%02x, Product ID[1:10]=[%x,%x,%x,%x,%x,%x,%x,%x,%x,%x]\n" }, \
{ RMI_FUNCTION_34, REGISTER_CONTROL_ID, INDEX( 0 ), INDEX( 3 ), DIRECTION_POSITIVE, "f%02x, Configuration ID[3:0]=[%x,%x,%x,%x]\n" }, \
{ RMI_FUNCTION_34, REGISTER_QUERY_ID, INDEX( 0 ), INDEX( 1 ), DIRECTION_NEGATIVE, "f%02x, Bootloader ID[1:0]=[%x,%x]\n" }, \
{ RMI_FUNCTION_34, REGISTER_QUERY_ID, INDEX( 7 ), INDEX( 8 ), DIRECTION_NEGATIVE, "f%02x, Configuration Block Count(%d%d)\n" }, \
{ RMI_FUNCTION_34, REGISTER_QUERY_ID, INDEX( 5 ), INDEX( 6 ), DIRECTION_NEGATIVE, "f%02x, Firmware Block Count(%d%d)\n" }, \
{ RMI_FUNCTION_34, REGISTER_QUERY_ID, INDEX( 3 ), INDEX( 4 ), DIRECTION_NEGATIVE, "f%02x, The block size is %d%d byte\n" }, \
{ RMI_FUNCTION_34, REGISTER_CONTROL_ID, INDEX( 0 ), INDEX( 3 ), DIRECTION_POSITIVE, "f%02x, Configuration ID[3:0]=\"%c%c%c%c\"\n" }, \

// ==========================================================================================================================

#define	COMMAND_STRING_LIST \
{ "%02d : Get Touch IC information, [NULL]\n", COMMAND_S_GET_TOUCH_IC_INFO }, \
{ "%02d : Read from RMI4 register of function group, [FUNC,REG,INDX]=[H,D,D]\n", COMMAND_S_READ_VALUE_FROM_FUNCTION_GROUP }, \
{ "%02d : Write to RMI4 register of function group, [FUNC,REG,INDX,VAL]=[H,D,D,H]\n", COMMAND_S_WRITE_VALUE_TO_FUNCTION_GROUP }, \
{ "%02d : Read from RMI4 register, [ADDR]=[H]\n", COMMAND_S_READ_VALUE_FROM_REGISTER }, \
{ "%02d : Write to RMI4 register, [ADDR,VAL]=[H,H]\n", COMMAND_S_WRITE_VALUE_TO_REGISTER }, \
{ "%02d : Read from block data of function group, [FUNC,REG,S_REG,COUT]=[H,D,D,D]\n", COMMAND_S_READ_BLOCK_DATA_FROM_FUNCTION_GROUP }, \
{ "%02d : Write to block data of function group, [FUNC,REG,S_REG,COUT,VAL0-VAL15]=[H,D,D,D,H-H]\n", COMMAND_S_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP }, \
{ "%02d : Read from block data of register, [S_REG,COUT]=[H,D]\n", COMMAND_S_READ_BLOCK_DATA_FROM_REGISTER }, \
{ "%02d : Write to block data of register, [S_REG,COUT,VAL0-VAL15]=[H,D,H-H]\n", COMMAND_S_WRITE_BLOCK_DATA_FROM_REGISTER }, \
{ "%02d : Read from RMI4 register of function group for Linux AP, [FUNC,REG,INDX]=[H,D,D]\n", COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP }, \
{ "%02d : Write to RMI4 register of function group for Linux AP, [FUNC,REG,INDX,VAL]=[H,D,D,H]\n", COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP }, \
{ "%02d : Read from RMI4 register for Linux AP, [ADDR]=[H]\n", COMMAND_N_READ_VALUE_FROM_REGISTER }, \
{ "%02d : Write to RMI4 register for Linux AP, [ADDR,VAL]=[H,H]\n", COMMAND_N_WRITE_VALUE_TO_REGISTER }, \
{ "%02d : Read from block data of function group for Linux AP, [FUNC,REG,S_REG,COUT]=[H,D,D,D]\n", COMMAND_N_READ_BLOCK_DATA_FROM_FUNCTION_GROUP }, \
{ "%02d : Write to block data of function group for Linux AP, [FUNC,REG,S_REG,COUT,VAL0-VAL15]=[H,D,D,D,H-H]\n", COMMAND_N_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP }, \
{ "%02d : Read from block data of register for Linux AP, [S_REG,COUT]=[H,D]\n", COMMAND_N_READ_BLOCK_DATA_FROM_REGISTER }, \
{ "%02d : Write to block data of register for Linux AP, [S_REG,COUT,VAL0-VAL15]=[H,D,H-H]\n", COMMAND_N_WRITE_BLOCK_DATA_FROM_REGISTER }, \
{ "%02d : Enter bootloader mode, [NULL]\n", COMMAND_ENTER_BOOTLOADER_MODE }, \
{ "%02d : Leave bootloader mode, [NULL]\n", COMMAND_LEAVE_BOOTLOADER_MODE }, \
{ "%02d : Check bootloader mode status, [NULL]\n", COMMAND_BOOTLOADER_MODE_STATUS }, \
{ "%02d : Memory >> File, Write data from memory to file, [NULL]\n", COMMAND_MEMORY_TO_FILE }, \
{ "%02d : Memory >> Flash, Write data from memory to flash, [NULL]\n", COMMAND_MEMORY_TO_FLASH }, \
{ "%02d : Flash >> Memory, Read data from flash to memory, [NULL]\n", COMMAND_FLASH_TO_MEMORY }, \
{ "%02d : Flash >> File, Read data from flash to file, [NULL]\n", COMMAND_FLASH_TO_FILE }, \
{ "%02d : File >> Memory, Read data from file to memory, [NULL]\n", COMMAND_FILE_TO_MEMORY }, \
{ "%02d : File >> Flash, Write data from file to flash, [NULL]\n", COMMAND_FILE_TO_FLASH }, \
{ "%02d : Erase all, [ID]=[H]\n", COMMAND_ERASE_ALL }, \
{ "%02d : Erase configuration, [ID]=[H]\n", COMMAND_ERASE_CONFIGURATION }, \
{ "%02d : Convert information from number to string, [NULL]\n", COMMAND_CONVERT_NUMBER_TO_STRING }, \
{ "%02d : Dump memory of parameter buffer( show value ), [NULL]\n", COMMAND_PARA_BUFFER_DUMP_VALUE }, \
{ "%02d : Dump memory of parameter buffer( show character ), [NULL]\n", COMMAND_PARA_BUFFER_DUMP_CHARACTER }, \
{ "%02d : Dump memory of flash information( show value ), [NULL]\n", COMMAND_MEMORY_DUMP_VALUE }, \
{ "%02d : Dump memory of flash information( show character ), [NULL]\n", COMMAND_MEMORY_DUMP_CHARACTER }, \
{ "%02d : Scan page description table, [NULL]\n", COMMAND_SCAN_PAGE_DESCRIPTION_TABLE }, \
{ "%02d : Write value to memory, [ADDR,VAL]=[D,H]\n", COMMAND_WRITE_VALUE_TO_MEMORY }, \
{ "%02d : Write block value to memory, [ADDR,COUT,VAL0-VAL15]=[D,D,H-H]\n", COMMAND_WRITE_BLOCK_VALUE_TO_MEMORY }, \
{ "%02d : Dump memory of flash information, [ADDR,COUT]=[D,D]\n", COMMAND_MEMORY_DUMP }, \
{ "%02d : Reset touch IC, [NULL]\n", COMMAND_RESET_TOUCH_IC }, \
{ "%02d : Create flash data, [NULL]\n", COMMAND_ALLOCATE_FLASH_DATA }, \
{ "%02d : Release flash data, [NULL]\n", COMMAND_RELEASE_FLASH_DATA }, \
{ "%02d : Create file information, [NAME]=[S]\n", COMMAND_ALLOCATE_FILE_INFORMATION }, \
{ "%02d : Release file information, [NULL]\n", COMMAND_RELEASE_FILE_INFORMATION }, \
{ "%02d : Enable IRQ, [NULL]\n", COMMAND_ENABLE_IRQ }, \
{ "%02d : Disable IRQ, [NULL]\n", COMMAND_DISABLE_IRQ }, \
{ "%02d : Set sleep mode to FULL-POWER mode, [NULL]\n", COMMAND_FULL_POWER_MODE }, \
{ "%02d : Show configuration ID, [NULL]\n", COMMAND_CONFIGURATION_ID }, \
{ "%02d : Upgrade configuration, [INDX]=[D]\n", COMMAND_UPGRADE_CONFIGURATION }, \
{ "%02d : Upgrade firmware, [INDX]=[D]\n", COMMAND_UPGRADE_FIRMWARE }, \
{ "%02d : Allocate file information of configuration, [NAME]=[S]\n", COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION }, \
{ "%02d : Allocate file information of firmware, [NAME]=[S]\n", COMMAND_ALLOCATE_FW_FILE_INFORMATION }, \
{ "%02d : Release file information of configuration, [NULL]\n", COMMAND_RELEASE_CONFIG_FILE_INFORMATION }, \
{ "%02d : Release file information of firmware, [NULL]\n", COMMAND_RELEASE_FW_FILE_INFORMATION }, \
{ "%02d : Dump image information, [NAME]=[S]\n", COMMAND_DUMP_IMAGE_INFORMATION }, \
{ "%02d : Show firmware version, [NULL]\n", COMMAND_FIRMWARE_VERSION }, \
{ "%02d : Dump history information, [NAME]=[S]\n", COMMAND_DUMP_HISTORY_INFORMATION }, \
{ "%02d : Show product ID, [NULL]\n", COMMAND_PRODUCT_ID }, \
{ "%02d : Dump history version block, [NAME,COUT]=[S,D]\n", COMMAND_DUMP_HISTORY_VERSION_BLOCK }, \
{ "%02d : Show sensor ID, [NULL]\n", COMMAND_SENSOR_ID }, \

// ==========================================================================================================================

#define	COMMAND_LIST \
{ COMMAND_S_GET_TOUCH_IC_INFO, command_S_get_info, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_S_READ_VALUE_FROM_FUNCTION_GROUP, S_read_function_group_value, "%x %d %d", COMMAND_3_PARAMENTER }, \
{ COMMAND_S_WRITE_VALUE_TO_FUNCTION_GROUP, S_write_function_group_value, "%x %d %d %x", COMMAND_4_PARAMENTER }, \
{ COMMAND_S_READ_VALUE_FROM_REGISTER, S_read_register_value, "%x", COMMAND_1_PARAMENTER }, \
{ COMMAND_S_WRITE_VALUE_TO_REGISTER, S_write_register_value, "%x %x", COMMAND_2_PARAMENTER }, \
{ COMMAND_S_READ_BLOCK_DATA_FROM_FUNCTION_GROUP, S_read_function_group_block_data, "%x %d %d %d", COMMAND_4_PARAMENTER }, \
{ COMMAND_S_READ_BLOCK_DATA_FROM_REGISTER, S_read_register_block_data, "%x %d", COMMAND_2_PARAMENTER }, \
{ COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, N_read_function_group_value, "%x %d %d", COMMAND_3_PARAMENTER }, \
{ COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, N_write_function_group_value, "%x %d %d %x", COMMAND_4_PARAMENTER }, \
{ COMMAND_N_READ_VALUE_FROM_REGISTER, N_read_register_value, "%x", COMMAND_1_PARAMENTER }, \
{ COMMAND_N_WRITE_VALUE_TO_REGISTER, N_write_register_value, "%x %x", COMMAND_2_PARAMENTER }, \
{ COMMAND_N_READ_BLOCK_DATA_FROM_FUNCTION_GROUP, N_read_function_group_block_data, "%x %d %d %d", COMMAND_4_PARAMENTER }, \
{ COMMAND_N_READ_BLOCK_DATA_FROM_REGISTER, N_read_register_block_data, "%x %d", COMMAND_2_PARAMENTER }, \
{ COMMAND_CONVERT_NUMBER_TO_STRING, convert_information_format, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_PARA_BUFFER_DUMP_VALUE, dump_para_buffer_value, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_PARA_BUFFER_DUMP_CHARACTER, dump_para_buffer_character, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_ENTER_BOOTLOADER_MODE, enter_bootloader_mode, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_S_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP, S_write_function_group_block_data, "%x %d %d %d %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", COMMAND_N_PARAMENTER }, \
{ COMMAND_S_WRITE_BLOCK_DATA_FROM_REGISTER, S_write_register_block_data, "%x %d %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", COMMAND_N_PARAMENTER }, \
{ COMMAND_N_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP, N_write_function_group_block_data, "%x %d %d %d %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", COMMAND_N_PARAMENTER }, \
{ COMMAND_N_WRITE_BLOCK_DATA_FROM_REGISTER, N_write_register_block_data, "%x %d %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", COMMAND_N_PARAMENTER }, \
{ COMMAND_SCAN_PAGE_DESCRIPTION_TABLE, scan_page_description_table, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_ALLOCATE_FLASH_DATA, create_flash_data, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_RELEASE_FLASH_DATA, release_flash_data, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_RESET_TOUCH_IC, reset_touch_ic, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_ALLOCATE_FILE_INFORMATION, create_file_information, "%s", COMMAND_1_PARAMENTER }, \
{ COMMAND_RELEASE_FILE_INFORMATION, release_file_information, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_MEMORY_DUMP_VALUE, memory_dump_value, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_MEMORY_DUMP_CHARACTER, memory_dump_character, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_MEMORY_TO_FILE, memory_to_file, "%d %d", COMMAND_0_PARAMENTER }, \
{ COMMAND_MEMORY_TO_FLASH, memory_to_flash, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_FLASH_TO_MEMORY, flash_to_memory, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_FILE_TO_MEMORY, file_to_memory, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_FILE_TO_FLASH, file_to_flash, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_LEAVE_BOOTLOADER_MODE, leave_bootloader_mode, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_BOOTLOADER_MODE_STATUS, bootloader_mode_status, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_ENABLE_IRQ, enable_ic_irq, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_DISABLE_IRQ, disable_ic_irq, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_FULL_POWER_MODE, full_power_mode, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_WRITE_VALUE_TO_MEMORY, write_memory_value, "%d %x", COMMAND_2_PARAMENTER }, \
{ COMMAND_WRITE_BLOCK_VALUE_TO_MEMORY, write_memory_block_value, "%d %d %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", COMMAND_N_PARAMENTER }, \
{ COMMAND_MEMORY_DUMP, dump_flash_memory, "%d %d", COMMAND_2_PARAMENTER }, \
{ COMMAND_ERASE_ALL, erase_all, "%x", COMMAND_1_PARAMENTER }, \
{ COMMAND_ERASE_CONFIGURATION, erase_configuration, "%x", COMMAND_1_PARAMENTER }, \
{ COMMAND_CONFIGURATION_ID, show_configuration_id, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_FLASH_TO_FILE, flash_to_file, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_UPGRADE_CONFIGURATION, upgrade_configuration, "%d", COMMAND_1_PARAMENTER }, \
{ COMMAND_UPGRADE_FIRMWARE, upgrade_firmware, "%d", COMMAND_1_PARAMENTER }, \
{ COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION, create_configuration_file, "%s", COMMAND_1_PARAMENTER }, \
{ COMMAND_ALLOCATE_FW_FILE_INFORMATION, create_firmware_file, "%s", COMMAND_1_PARAMENTER }, \
{ COMMAND_RELEASE_CONFIG_FILE_INFORMATION, release_configuration_file, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_RELEASE_FW_FILE_INFORMATION, release_firmware_file, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_DUMP_IMAGE_INFORMATION, dump_image_information, "%s", COMMAND_1_PARAMENTER }, \
{ COMMAND_FIRMWARE_VERSION, show_firmware_version, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_DUMP_HISTORY_INFORMATION, dump_history_information, "%s", COMMAND_1_PARAMENTER }, \
{ COMMAND_PRODUCT_ID, show_product_id, NULL, COMMAND_0_PARAMENTER }, \
{ COMMAND_DUMP_HISTORY_VERSION_BLOCK, dump_history_version_block, "%s %d", COMMAND_2_PARAMENTER }, \
{ COMMAND_SENSOR_ID, show_sensor_id, NULL, COMMAND_0_PARAMENTER }, \

// ==========================================================================================================================

#define BUFFER_SELECT_INFORMATION \
{ COMMAND_N_READ_BLOCK_DATA_FROM_FUNCTION_GROUP, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data_count }, \
{ COMMAND_N_READ_BLOCK_DATA_FROM_REGISTER, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data_count }, \
{ COMMAND_N_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data_count }, \
{ COMMAND_N_WRITE_BLOCK_DATA_FROM_REGISTER, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data_count }, \
{ COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_N_READ_VALUE_FROM_REGISTER, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_N_WRITE_VALUE_TO_REGISTER, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_RESET_TOUCH_IC, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ALLOCATE_FLASH_DATA, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_RELEASE_FLASH_DATA, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ENTER_BOOTLOADER_MODE, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_LEAVE_BOOTLOADER_MODE, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_MEMORY_TO_FILE, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_MEMORY_TO_FLASH, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_FLASH_TO_MEMORY, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_FILE_TO_MEMORY, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_FILE_TO_FLASH, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ALLOCATE_FILE_INFORMATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_RELEASE_FILE_INFORMATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_BOOTLOADER_MODE_STATUS, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ENABLE_IRQ, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_DISABLE_IRQ, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_FULL_POWER_MODE, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_WRITE_VALUE_TO_MEMORY, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_WRITE_BLOCK_VALUE_TO_MEMORY, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ERASE_ALL, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ERASE_CONFIGURATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_CONFIGURATION_ID, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data_count }, \
{ COMMAND_FLASH_TO_FILE, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_UPGRADE_CONFIGURATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_UPGRADE_FIRMWARE, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_ALLOCATE_FW_FILE_INFORMATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_RELEASE_CONFIG_FILE_INFORMATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_RELEASE_FW_FILE_INFORMATION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) }, \
{ COMMAND_FIRMWARE_VERSION, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data_count }, \
{ COMMAND_PRODUCT_ID, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data_count }, \
{ COMMAND_SENSOR_ID, PARAMETER_BUFFER, tool_data->parameter_buffer, sizeof( struct return_struct ) + ( ( struct return_struct* )tool_data->parameter_buffer )->values.block_data_count }, \

// ==========================================================================================================================

#define	RETRUN_STRUCT_LIST \
{ COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, STORE_VALUE }, \
{ COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, STORE_VALUE }, \
{ COMMAND_N_READ_VALUE_FROM_REGISTER, STORE_VALUE }, \
{ COMMAND_N_WRITE_VALUE_TO_REGISTER, STORE_VALUE }, \
{ COMMAND_BOOTLOADER_MODE_STATUS, STORE_VALUE },\
{ COMMAND_N_READ_BLOCK_DATA_FROM_FUNCTION_GROUP, STORE_BLOCK_DATA }, \
{ COMMAND_N_READ_BLOCK_DATA_FROM_REGISTER, STORE_BLOCK_DATA }, \
{ COMMAND_N_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP, STORE_BLOCK_DATA }, \
{ COMMAND_N_WRITE_BLOCK_DATA_FROM_REGISTER, STORE_BLOCK_DATA }, \
{ COMMAND_CONFIGURATION_ID, STORE_BLOCK_DATA },\
{ COMMAND_ALLOCATE_FILE_INFORMATION, STORE_NAME }, \
{ COMMAND_RESET_TOUCH_IC, STORE_NAME },\
{ COMMAND_ALLOCATE_FLASH_DATA, STORE_SIZE }, \
{ COMMAND_FIRMWARE_VERSION, STORE_BLOCK_DATA },\
{ COMMAND_PRODUCT_ID, STORE_BLOCK_DATA },\
{ COMMAND_SENSOR_ID, STORE_BLOCK_DATA },\

// ==========================================================================================================================

#define	RETRUN_STRING_LIST \
{ COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, SHOW_VALUE }, \
{ COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, SHOW_VALUE }, \
{ COMMAND_N_READ_VALUE_FROM_REGISTER, SHOW_VALUE }, \
{ COMMAND_N_WRITE_VALUE_TO_REGISTER, SHOW_VALUE }, \
{ COMMAND_N_READ_BLOCK_DATA_FROM_FUNCTION_GROUP, SHOW_BLOCK_DATA }, \
{ COMMAND_N_READ_BLOCK_DATA_FROM_REGISTER, SHOW_BLOCK_DATA }, \
{ COMMAND_N_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP, SHOW_COUNT }, \
{ COMMAND_N_WRITE_BLOCK_DATA_FROM_REGISTER, SHOW_COUNT }, \
{ COMMAND_ALLOCATE_FLASH_DATA, SHOW_SIZE }, \
{ COMMAND_ALLOCATE_FILE_INFORMATION, SHOW_STRING }, \
{ COMMAND_RESET_TOUCH_IC, SHOW_STRING },\
{ COMMAND_BOOTLOADER_MODE_STATUS, SHOW_MODE },\
{ COMMAND_CONFIGURATION_ID, SHOW_CONFIG },\
{ COMMAND_FIRMWARE_VERSION, SHOW_FIRMWARE },\
{ COMMAND_PRODUCT_ID, SHOW_PRODUCT },\
{ COMMAND_SENSOR_ID, SHOW_SENSOR_ID },\

// ==========================================================================================================================

#pragma	pack( push )

#pragma	pack( 1 )

union	return_value
{

	unsigned int	block_data_count;

	unsigned int	size, length;

	u8		value;

};

struct	return_struct
{

	unsigned int		error_code;

	char			string[ 9 ];

	int			command;

	union return_value	values;

};

struct	image_information
{

	char	use_image_index;

	char	image_count;

};

struct	image_history
{

	char	image_index;

	char	state;

	char	version[ 8 ];

};

struct	history_header
{

	char				mark[ 3 ];
			
	char				check[ 8 ];

	struct image_information	firmware, configuration;

	struct image_history		last_firmware_version, last_configuration_version;

	unsigned			flag;

	unsigned			image_history_count;

	unsigned			clear_id;

	char				reserve[ 17 ];

	unsigned short			firmware_block_count[ 8 ], configuration_block_count[ 8 ];

};

struct	history_body
{

	struct image_history		image_history_datas[ HISTORY_COUNT ];

};

#pragma	pack( pop )

enum	buffer_kind
{

	INFORMATION_BUFFER = 0,
	PARAMETER_BUFFER,

};

enum	select_action
{

	// Store action
	STORE_BLOCK_DATA,
	STORE_VALUE,
	STORE_SIZE,
	STORE_NAME,
	STORE_FILE_NAME,

	// Show action
	SHOW_CONFIG,
	SHOW_MODE,
	SHOW_STRING,
	SHOW_FILE_NAME,
	SHOW_SIZE,
	SHOW_BLOCK_DATA,
	SHOW_VALUE,
	SHOW_FIRMWARE,
	SHOW_PRODUCT,
	SHOW_SENSOR_ID,
	SHOW_COUNT,

};

enum	touch_command
{

	// 0 ~ 19
	COMMAND_S_GET_TOUCH_IC_INFO			= 0,
	COMMAND_S_READ_VALUE_FROM_FUNCTION_GROUP	= 1,
	COMMAND_S_READ_VALUE_FROM_REGISTER		= 2,
	COMMAND_S_READ_BLOCK_DATA_FROM_FUNCTION_GROUP	= 3,
	COMMAND_S_READ_BLOCK_DATA_FROM_REGISTER		= 4,
	COMMAND_S_WRITE_VALUE_TO_FUNCTION_GROUP		= 5,
	COMMAND_S_WRITE_VALUE_TO_REGISTER		= 6,
	COMMAND_S_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP	= 7,
	COMMAND_S_WRITE_BLOCK_DATA_FROM_REGISTER	= 8,

	// 20 ~ 39
	COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP	= 21,
	COMMAND_N_READ_VALUE_FROM_REGISTER		= 22,
	COMMAND_N_READ_BLOCK_DATA_FROM_FUNCTION_GROUP	= 23,
	COMMAND_N_READ_BLOCK_DATA_FROM_REGISTER		= 24,
	COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP		= 25,
	COMMAND_N_WRITE_VALUE_TO_REGISTER		= 26,
	COMMAND_N_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP	= 27,
	COMMAND_N_WRITE_BLOCK_DATA_FROM_REGISTER	= 28,

	// 40 ~ 59
	COMMAND_BOOTLOADER_MODE_STATUS			= 40,
	COMMAND_ENTER_BOOTLOADER_MODE			= 41,
	COMMAND_LEAVE_BOOTLOADER_MODE			= 42,
	COMMAND_ERASE_ALL				= 43,
	COMMAND_MEMORY_TO_FILE				= 44,
	COMMAND_MEMORY_TO_FLASH				= 45,
	COMMAND_FLASH_TO_MEMORY				= 46,
	COMMAND_FLASH_TO_FILE				= 47,
	COMMAND_FILE_TO_MEMORY				= 50,
	COMMAND_FILE_TO_FLASH				= 51,
	COMMAND_ERASE_CONFIGURATION			= 52,
	COMMAND_UPGRADE_CONFIGURATION			= 53,
	COMMAND_UPGRADE_FIRMWARE			= 54,

	// 60 ~ 79
	COMMAND_CONVERT_NUMBER_TO_STRING		= 60,
	COMMAND_SCAN_PAGE_DESCRIPTION_TABLE		= 61,
	COMMAND_WRITE_VALUE_TO_MEMORY			= 62,
	COMMAND_WRITE_BLOCK_VALUE_TO_MEMORY		= 63,
	COMMAND_MEMORY_DUMP				= 64,
	COMMAND_PARA_BUFFER_DUMP_VALUE			= 65,
	COMMAND_MEMORY_DUMP_VALUE			= 66,
	COMMAND_PARA_BUFFER_DUMP_CHARACTER		= 67,
	COMMAND_MEMORY_DUMP_CHARACTER			= 68,

	// 80 ~ 99
	COMMAND_RESET_TOUCH_IC				= 80,
	COMMAND_SENSOR_ID				= 81,
	COMMAND_PRODUCT_ID				= 82,
	COMMAND_FIRMWARE_VERSION			= 83,
	COMMAND_CONFIGURATION_ID			= 84,
	COMMAND_FULL_POWER_MODE				= 85,
	COMMAND_ENABLE_IRQ				= 86,
	COMMAND_DISABLE_IRQ				= 87,
	COMMAND_ALLOCATE_FLASH_DATA			= 88,
	COMMAND_ALLOCATE_FILE_INFORMATION		= 89,
	COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION	= 90,
	COMMAND_ALLOCATE_FW_FILE_INFORMATION		= 91,
	COMMAND_RELEASE_FLASH_DATA			= 92,
	COMMAND_RELEASE_FILE_INFORMATION		= 93,
	COMMAND_RELEASE_CONFIG_FILE_INFORMATION		= 94,
	COMMAND_RELEASE_FW_FILE_INFORMATION		= 95,
	COMMAND_DUMP_IMAGE_INFORMATION			= 96,
	COMMAND_DUMP_HISTORY_INFORMATION		= 97,
	COMMAND_DUMP_HISTORY_VERSION_BLOCK		= 98,

};

enum	return_error_define
{

	ERROR_BUSY			= 1,
	ERROR_NONE			= 0,
	ERROR_INVALID_ADDRESS		= -1,
	ERROR_I2C_READ			= -2,
	ERROR_I2C_WRITE			= -3,
	ERROR_I2C_ACCESS		= -4,
	ERROR_BUFFER_FULL		= -5,
	ERROR_RESOURCE_EXIST		= -6,
	ERROR_RESOURCE_NOT_EXIST	= -7,
	ERROR_ALLOCATE_FAILED		= -8,
	ERROR_OPEN_FILE_FAILED		= -9,
	ERROR_NOT_FOUND_FUNCTION	= -10,
	ERROR_CLOSE_FILE_FAILED		= -11,
	ERROR_VALUE_INCORRECT		= -12,
	ERROR_IRQ_CONTROL_FAILED	= -13,
	ERROR_TIMEOUT			= -14,
	ERROR_RETURN_FAILED		= -15,
	ERROR_OUT_OF_BUFFER		= -16,
	ERROR_FLASH_FAILED		= -17,
	ERROR_FILE_ACCESS_FAILED	= -18,
	ERROR_INCORRECT_INFORMATION	= -19,

};

struct	register_datas_info
{

	int	index;

	u8	*value;

	unsigned int	size;

};

struct	upgrade_data_information
{

	char		version_string[ 16 ];

	unsigned	convert;

	long		start, size;

};

struct	upgrade_file_header
{

	char	fih_string[ 8 ];

	char	check_string[ 16 ];

	unsigned int	count;

};

// ==========================================================================================================================
