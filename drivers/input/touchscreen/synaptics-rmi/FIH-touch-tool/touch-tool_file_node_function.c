
#include <linux/input/FIH-tool/FIH-touch-tool.h>

#include <linux/input/FIH-tool/touch-tool_functions.h>

#include <linux/delay.h>

#include <asm/uaccess.h>

// ==========================================================================================================================

void get_touch_ic_info( struct touch_tool_data*, void ( * )( int, char*, ... ) );

void info_print( int, char*, ... );

void log_print( int, char*, ... );

char* get_buffer_address( struct touch_tool_data*, enum buffer_kind*, unsigned int* );

u16 get_register_addr( struct touch_tool_data*, int, int );

char* get_register_string( int );

void creat_block_data_number_string( u8*, u8*, unsigned int );

void creat_block_data_character_string( u8*, u8*, unsigned int );

void create_return_structure( struct return_struct*, int, int, void*, int );

void dump_memory_character( struct touch_tool_data*, char*, unsigned int, unsigned int );

void dump_memory_value( struct touch_tool_data*, char*, unsigned int, unsigned int );

void create_return_string( u8*, struct return_struct* );

void get_page_description( struct touch_tool_data* );

int software_reset( struct touch_tool_data* );

int rmi_set_irq( struct synaptics_rmi4_data*, bool );

bool enter_full_power_mode( struct touch_tool_data* );

bool flash_command( struct touch_tool_data*, u8, int );

bool read_register_datas( struct touch_tool_data*, int, int, struct register_datas_info*, unsigned int );

bool read_register_data( struct touch_tool_data*, int, int, int, u8*, unsigned int );

bool write_register_data( struct touch_tool_data*, int, int, int, u8*, unsigned int );

bool special_flash_command( struct touch_tool_data*, u8, int );

bool get_start_file_data_position( struct file*, char*, long* );

bool check_image_header( struct file*, char*, unsigned );

// ==========================================================================================================================

void	command_S_get_info( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_GET_TOUCH_IC_INFO;

	get_touch_ic_info( ( struct touch_tool_data* )node_data, info_print );

}

void	S_read_function_group_value( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	function, register_id, index;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number, parameter->para3.container.number,

	};

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	u8	value;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_READ_VALUE_FROM_FUNCTION_GROUP );

	tool_data->command_id	= COMMAND_S_READ_VALUE_FROM_FUNCTION_GROUP;

	if( !read_register_data( tool_data, parameters.function, parameters.register_id, parameters.index, &value, 1 ) )
	{

		printk( "TTUCH : Read register failed\n" );

		info_print( INFO_BUFFER_ADD, "Read register failed\n" );

	}

	printk( "TTUCH : [F%02x:%s%d]=%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, value );

	info_print( INFO_BUFFER_ADD, "[F%02x:%s%d]=%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, value );

}

void	S_write_function_group_value( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	function, register_id, index, value;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number, parameter->para3.container.number, parameter->para4.container.number,

	};

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	u8	value = parameters.value;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_WRITE_VALUE_TO_FUNCTION_GROUP );

	tool_data->command_id	= COMMAND_S_WRITE_VALUE_TO_FUNCTION_GROUP;

	if( !write_register_data( tool_data, parameters.function, parameters.register_id, parameters.index, &value, 1 ) )
	{

		printk( "TTUCH : Write register failed\n" );

		info_print( INFO_BUFFER_ADD, "Write register failed\n" );

	}

	printk( "TTUCH : [F%02x:%s%d]<<0x%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, parameters.value );

	info_print( INFO_BUFFER_ADD, "[F%02x:%s%d]<<0x%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, parameters.value );

}

void	S_read_register_value( void *node_data, struct command_parameter *parameter )
{

	struct synaptics_rmi4_data	*rmi_data = ( ( struct touch_tool_data* )node_data )->rmi_data;

	int	address = parameter->para1.container.number;

	int	error;

	u8	value;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_READ_VALUE_FROM_REGISTER );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_READ_VALUE_FROM_REGISTER;

	if( ( error = rmi_read( rmi_data, address, &value ) ) < 0 )
	{

		printk( "TTUCH : RMI read failed, ERROR(%d)\n", error );

		info_print( INFO_BUFFER_ADD, "RMI read failed, ERROR(%d)\n", error );

	}
	else
	{

		printk( "TTUCH : REG%d=%x\n", address, value );

		info_print( INFO_BUFFER_ADD, "REG%d=%x\n", address, value );

	}

}

void	S_write_register_value( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	register_addr, value;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number,

	};

	struct synaptics_rmi4_data	*rmi_data = ( ( struct touch_tool_data* )node_data )->rmi_data;

	int	error;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_WRITE_VALUE_TO_REGISTER );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_WRITE_VALUE_TO_REGISTER;

	if( ( error = rmi_write( rmi_data, parameters.register_addr, parameters.value ) ) < 0 )
	{

		printk( "TTUCH : RMI write failed, ERROR(%d)\n", error );

		info_print( INFO_BUFFER_ADD, "RMI write failed, ERROR(%d)\n", error );

	}
	else
	{

		printk( "TTUCH : REG%d<<%x\n", parameters.register_addr, parameters.value );

		info_print( INFO_BUFFER_ADD, "REG%d<<%x\n", parameters.register_addr, parameters.value );

	}

}

void	S_read_function_group_block_data( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	function, register_id, start_register, count;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number, parameter->para3.container.number, parameter->para4.container.number,

	};

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	int	registry_count = parameters.count;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_READ_BLOCK_DATA_FROM_FUNCTION_GROUP );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_READ_BLOCK_DATA_FROM_FUNCTION_GROUP;

	if( registry_count >= PARA_BUFFER_SIZE )
	{

		printk( "TTUCH : Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE, registry_count );

		info_print( INFO_BUFFER_ADD, "Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE, registry_count );

		return;

	}

	if( !read_register_data( tool_data, parameters.function, parameters.register_id, parameters.start_register, values, registry_count ) )
	{

		printk( "TTUCH : Read register failed\n" );

		info_print( INFO_BUFFER_ADD, "Read register failed\n" );

	}
	else
	{

		u8	*buffer = ( ( struct touch_tool_data* )node_data )->block_data_buffer;

		int	loop, index;

		printk( "TTUCH : [F%02x:%s%d-%d], block data\n", parameters.function, get_register_string( parameters.register_id ), parameters.start_register, parameters.start_register + parameters.count - 1 );

		info_print( INFO_BUFFER_ADD, "[F%02x:%s%d-%d], block data\n", parameters.function, get_register_string( parameters.register_id ), parameters.start_register, parameters.start_register + parameters.count - 1 );

		info_print( INFO_BUFFER_ADD, "%s\n", BLOCK_DATA_NUMBER_STRING );

		for( loop = registry_count, index = 0 ; loop != 0 ; loop -= loop / MAX_BLOCK_DATA ? MAX_BLOCK_DATA : loop, index += MAX_BLOCK_DATA )
		{

			creat_block_data_number_string( buffer, values + index, loop );

			printk( "TTUCH : %04d:%s\n", parameters.start_register + index, buffer );

			info_print( INFO_BUFFER_ADD, "%04d |%s\n", parameters.start_register + index, buffer );

		}

		info_print( INFO_BUFFER_ADD, "%s\n", BLOCK_DATA_CHARACTER_STRING );

		for( loop = registry_count, index = 0 ; loop != 0 ; loop -= loop / MAX_BLOCK_DATA ? MAX_BLOCK_DATA : loop, index += MAX_BLOCK_DATA )
		{

			creat_block_data_character_string( buffer, values + index, loop );

			info_print( INFO_BUFFER_ADD, "%04d |%s\n", parameters.start_register + index, buffer );

		}

	}

}

void	S_read_register_block_data( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	start_register, count;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number,

	};

	struct synaptics_rmi4_data	*rmi_data = ( ( struct touch_tool_data* )node_data )->rmi_data;

	int	registry_count = parameters.count;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer;

	int	error;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_READ_BLOCK_DATA_FROM_REGISTER );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_READ_BLOCK_DATA_FROM_REGISTER;

	if( registry_count >= PARA_BUFFER_SIZE )
	{

		printk( "TTUCH : Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE, registry_count );

		info_print( INFO_BUFFER_ADD, "Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE, registry_count );

		return;

	}

	if( ( error = rmi_read_block( rmi_data, parameters.start_register, values, registry_count ) ) < 0 )
	{

		printk( "TTUCH : RMI block read failed, ERROR(%d)\n", error );

		info_print( INFO_BUFFER_ADD, "RMI read failed, ERROR(%d)\n", error );

	}
	else
	{

		u8	*buffer = ( ( struct touch_tool_data* )node_data )->block_data_buffer;

		int	loop, index;

		printk( "TTUCH : [REG%d-%d], block data\n", parameters.start_register, parameters.start_register + parameters.count - 1 );

		info_print( INFO_BUFFER_ADD, "[REG%d-%d], block data\n", parameters.start_register, parameters.start_register + parameters.count - 1 );

		info_print( INFO_BUFFER_ADD, "%s\n", BLOCK_DATA_NUMBER_STRING );

		for( loop = registry_count, index = 0 ; loop != 0 ; loop -= loop / MAX_BLOCK_DATA ? MAX_BLOCK_DATA : loop, index += MAX_BLOCK_DATA )
		{

			creat_block_data_number_string( buffer, values + index, loop );

			printk( "TTUCH : %04d:%s\n", parameters.start_register + index, buffer );

			info_print( INFO_BUFFER_ADD, "%04d |%s\n", parameters.start_register + index, buffer );

		}

		info_print( INFO_BUFFER_ADD, "%s\n", BLOCK_DATA_CHARACTER_STRING );

		for( loop = registry_count, index = 0 ; loop != 0 ; loop -= loop / MAX_BLOCK_DATA ? MAX_BLOCK_DATA : loop, index += MAX_BLOCK_DATA )
		{

			creat_block_data_character_string( buffer, values + index, loop );

			info_print( INFO_BUFFER_ADD, "%04d |%s\n", parameters.start_register + index, buffer );

		}

	}

}

void	N_read_function_group_value( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	function, register_id, index;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number, parameter->para3.container.number,

	};

	struct touch_tool_data	*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct	*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	int	error = ERROR_NONE;

	u8	value = 0;

	tool_data->command_id	= COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP;

	create_return_structure( return_buffer, COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, ERROR_BUSY, NULL, 0 );

	if( !read_register_data( tool_data, parameters.function, parameters.register_id, parameters.index, &value, 1 ) )
	{

		printk( "TTUCH : Read register failed\n" );

		error	= ERROR_I2C_READ;

	}
	else
		printk( "TTUCH : [F%02x:%s%d]=0x%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, value );

	create_return_structure( return_buffer, COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, error, &value, 1 );

}

void	N_write_function_group_value( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	function, register_id, index, value;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number, parameter->para3.container.number, parameter->para4.container.number,

	};

	struct touch_tool_data	*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct	*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	int	error = ERROR_NONE;

	u8	value = parameters.value;

	tool_data->command_id	= COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP;

	if( !write_register_data( tool_data, parameters.function, parameters.register_id, parameters.index, &value, 1 ) )
	{

		printk( "TTUCH : Write register failed\n" );

		error	= ERROR_I2C_WRITE;

	}
	else
		printk( "TTUCH : [F%02x:%s%d]<<0x%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, parameters.value );

	create_return_structure( return_buffer, COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, error, &value, 1 );

}

void	N_read_register_value( void *node_data, struct command_parameter *parameter )
{

	struct synaptics_rmi4_data	*rmi_data = ( ( struct touch_tool_data* )node_data )->rmi_data;

	int	address = parameter->para1.container.number;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	int	error = ERROR_NONE;

	u8	value = 0;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_READ_VALUE_FROM_REGISTER;

	if( rmi_read( rmi_data, address, &value ) < 0 )
	{

		printk( "TTUCH : RMI read failed\n" );

		error	= ERROR_I2C_READ;

	}
	else
		printk( "TTUCH : REG%d=%x\n", address, value );

	create_return_structure( return_buffer, COMMAND_N_READ_VALUE_FROM_REGISTER, error, &value, 1 );

}

void	N_write_register_value( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	register_addr, value;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number,

	};

	struct synaptics_rmi4_data	*rmi_data = ( ( struct touch_tool_data* )node_data )->rmi_data;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	int	error = ERROR_NONE;

	u8	value = parameters.value;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_WRITE_VALUE_TO_REGISTER;

	if( rmi_write( rmi_data, parameters.register_addr, parameters.value ) < 0 )
	{

		printk( "TTUCH : RMI write failed\n" );

		error	= ERROR_I2C_WRITE;

	}
	else
		printk( "TTUCH : REG%d<<%x\n", parameters.register_addr, parameters.value );

	create_return_structure( return_buffer, COMMAND_N_WRITE_VALUE_TO_REGISTER, error, &value, 1 );

}

void	N_read_function_group_block_data( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	function, register_id, start_register, count;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number, parameter->para3.container.number, parameter->para4.container.number,

	};

	struct touch_tool_data	*tool_data = ( struct touch_tool_data* )node_data;

	int	registry_count = parameters.count;

	struct return_struct	*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	u8	*values = tool_data->parameter_buffer + sizeof( struct return_struct );

	int	error = ERROR_NONE;

	tool_data->command_id	= COMMAND_N_READ_BLOCK_DATA_FROM_FUNCTION_GROUP;

	if( registry_count >= PARA_BUFFER_SIZE - sizeof( struct return_struct ) )
	{

		printk( "TTUCH : Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE - sizeof( struct return_struct ), registry_count );

		create_return_structure( return_buffer, COMMAND_N_READ_BLOCK_DATA_FROM_FUNCTION_GROUP, ERROR_BUFFER_FULL, NULL, 0 );

		return;

	}

	if( !read_register_data( tool_data, parameters.function, parameters.register_id, parameters.start_register, values, registry_count ) )
	{

		printk( "TTUCH : Read register failed\n" );

		error	= ERROR_I2C_READ;

	}

	create_return_structure( return_buffer, COMMAND_N_READ_BLOCK_DATA_FROM_FUNCTION_GROUP, error, NULL, registry_count );

}

void	N_read_register_block_data( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	start_register, count;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number,

	};

	struct synaptics_rmi4_data	*rmi_data = ( ( struct touch_tool_data* )node_data )->rmi_data;

	int	registry_count = parameters.count;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer + sizeof( struct return_struct );

	int	error = ERROR_NONE;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_READ_BLOCK_DATA_FROM_REGISTER;

	if( registry_count >= PARA_BUFFER_SIZE - sizeof( struct return_struct ) )
	{

		printk( "TTUCH : Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE - sizeof( struct return_struct ), registry_count );

		create_return_structure( return_buffer, COMMAND_N_READ_BLOCK_DATA_FROM_REGISTER, ERROR_BUFFER_FULL, NULL, 0 );

		return;

	}

	if( rmi_read_block( rmi_data, parameters.start_register, values, registry_count ) < 0 )
	{

		printk( "TTUCH : RMI read failed\n" );

		error	= ERROR_I2C_READ;

	}

	create_return_structure( return_buffer, COMMAND_N_READ_BLOCK_DATA_FROM_REGISTER, error, NULL, registry_count );

}

void	convert_information_format( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_CONVERT_NUMBER_TO_STRING;

	create_return_string( ( ( struct touch_tool_data* )node_data )->info_buffer, ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer );

}

void	dump_para_buffer_value( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_PARA_BUFFER_DUMP_VALUE;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Parameter buffer[%d]\n", PARA_BUFFER_SIZE );

	dump_memory_value( ( struct touch_tool_data* )node_data, ( ( struct touch_tool_data* )node_data )->parameter_buffer, 0, PARA_BUFFER_SIZE - 1 );

}

void	dump_para_buffer_character( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_PARA_BUFFER_DUMP_CHARACTER;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Parameter buffer[%d]\n", PARA_BUFFER_SIZE );

	dump_memory_character( ( struct touch_tool_data* )node_data, ( ( struct touch_tool_data* )node_data )->parameter_buffer, 0, PARA_BUFFER_SIZE - 1 );

}

void	enter_bootloader_mode( void *node_data, struct command_parameter *parameter )
{

	enum	check_mechanism
	{

		NO_CHECK_VALUE = 0,
		VALUE_EQUAL,
		VALUE_NO_EQUAL,
		TOTAL,

	};

	struct	register_information
	{

		int	register_access_command, function_number, register_id, register_index;

		u8	*register_offset;

	};

	struct	extented_function
	{

		unsigned int	wait_time;

		enum check_mechanism	mechanism;

		unsigned int	value;

		bool	scan_pdt;

	};

	struct	register_data
	{

		struct register_information	information;

		struct extented_function	extented;

		u8	*data_address;

		unsigned int	data_size;

		char	*string;

	};

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct flash_information	*flash_data = tool_data->flash_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	int	error = ERROR_NONE;

	u8	program_flash = F34_ENABLE_FLASH_PROGRAMMING;

	u8	bootloader_id[ 2 ], block_size, flash_data3_state, flash_program_state, interrupt_state;

	struct register_data	register_datas[] =
	{

		{ 
			{ REGISTER_READ, RMI_FUNCTION_34, REGISTER_QUERY_ID, RMI_F34_QUERY0_BOOTLOADER_ID0, NULL },
			{ 0, NO_CHECK_VALUE, 0 },
			bootloader_id, sizeof( bootloader_id ), "Get bootloader ID"
		},
		{ 
			{ REGISTER_WRITE, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA, NULL },
			{ 0, NO_CHECK_VALUE, 0 },
			bootloader_id, sizeof( bootloader_id ), "Set bootloader ID"
		},
		{
			{ REGISTER_READ, RMI_FUNCTION_34, REGISTER_QUERY_ID, RMI_F34_QUERY3_BLOCK_SIZE_7_0, NULL },
			{ 0, NO_CHECK_VALUE, 0 },
			&block_size, sizeof( block_size ), "Get block size" },
		{
			{ REGISTER_WRITE, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_FLASH_COMMAND, &block_size },
			{ 0, NO_CHECK_VALUE, 0 },
			&program_flash, sizeof( program_flash ), "Set flash command"
		},
		{
			{ REGISTER_READ, RMI_FUNCTION_01, REGISTER_DATA_ID, RMI_F01_DATA1, NULL },
			{ WAIT_1000_MS, VALUE_NO_EQUAL, 0 },
			&interrupt_state, sizeof( interrupt_state ), "Get interrupt state"
		},
		{
			{ REGISTER_READ, RMI_FUNCTION_01, REGISTER_DATA_ID, RMI_F01_DATA0, NULL },
			{ 0, NO_CHECK_VALUE, 0 },
			&flash_program_state, sizeof( flash_program_state ), "Get mode state"
		},
		{
			{ REGISTER_READ, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_FLASH_COMMAND, &block_size },
			{ 0, NO_CHECK_VALUE, 0 },
			&flash_data3_state, sizeof( flash_data3_state ), "Get flash state"
		},

	};

	int	loop;

	u16	address;

	tool_data->command_id	= COMMAND_ENTER_BOOTLOADER_MODE;

	if( !flash_data || !return_buffer || !rmi_data )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	for( loop = 0 ; loop < sizeof( register_datas ) / sizeof( *register_datas ) ; ++loop )
	{

		struct register_data		*data = register_datas + loop;

		struct register_information	*information = &data->information;

		struct extented_function	*extented = &data->extented;

		int ( *rmi_access )( struct synaptics_rmi4_data*, u16, u8*, int );

		u8	offset = information->register_offset ? *information->register_offset : 0;

		int	timeout = ( ( extented->wait_time * 1000 ) / MAX_SLEEP_TIME_US ) + 1;

		int	count = 0;

		bool	timed_out = true;

		if( extented->mechanism >= TOTAL )
		{

			printk( "TTUCH : Incorrect mechanism\n" );

			create_return_structure( return_buffer, COMMAND_ENTER_BOOTLOADER_MODE, ERROR_VALUE_INCORRECT, NULL, 0 );

			return;

		}

		if( ( address = get_register_addr( tool_data, information->function_number, information->register_id ) ) == INVALID_ADDRESS )
		{

			printk( "TTUCH : Can't get %s register address\n", get_register_string( information->register_id ) );

			create_return_structure( return_buffer, COMMAND_ENTER_BOOTLOADER_MODE, ERROR_INVALID_ADDRESS, NULL, 0 );

			break;

		}

		rmi_access	= information->register_access_command == REGISTER_READ ? rmi_read_block : rmi_write_block;

		do
		{

			if( ( error = rmi_access( rmi_data, address + information->register_index + offset, data->data_address, data->data_size ) ) < 0 )
			{

				printk( "TTUCH : RMI block %s failed, ERROR(%d)\n", information->register_access_command == REGISTER_READ ? "read" : "write", error );

				create_return_structure( return_buffer, COMMAND_ENTER_BOOTLOADER_MODE, ERROR_I2C_ACCESS, NULL, 0 );

				break;

			}
			
			count++;

			usleep_range( MIN_SLEEP_TIME_US, MAX_SLEEP_TIME_US );

			{

				bool	check_list[] =
				{

					true,
					*data->data_address == extented->value,
					*data->data_address != extented->value,

				};

				if( *( check_list + extented->mechanism ) == true )
				{

					timed_out	= false;

					break;

				}

			}

		}
		while( count < timeout );

		if( timed_out )
		{

			printk( "TTUCH : Timeout(%dms)\n", extented->wait_time );

			create_return_structure( return_buffer, COMMAND_ENTER_BOOTLOADER_MODE, ERROR_TIMEOUT, NULL, 0 );

			break;

		}

	}

	if( loop == sizeof( register_datas ) / sizeof( *register_datas ) )
	{

		printk( "TTUCH : Interrupt status(%x), Flash status(%d), Program enabled(%d), Flash program(%d)\n", interrupt_state, FLASH_STATUS( flash_data3_state ), PROGRAM_ENABLED( flash_data3_state ), FLASH_PROGRAM( flash_program_state ) );

		create_return_structure( return_buffer, COMMAND_ENTER_BOOTLOADER_MODE, ERROR_NONE, NULL, 0 );

		flash_data->bootloader_mode = ENABLE;

		get_page_description( tool_data );

	}
	else
	{

		printk( "TTUCH : %s failed\n", ( register_datas + loop )->string );

		( void )software_reset( tool_data );

	}

}

void	leave_bootloader_mode( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_data = tool_data->flash_data;

	int	error;

	tool_data->command_id	= COMMAND_LEAVE_BOOTLOADER_MODE;

	if( !flash_data || !flash_data->bootloader_mode )
	{

		printk( "TTUCH : Not found resource\n" );

		create_return_structure( return_buffer, COMMAND_LEAVE_BOOTLOADER_MODE, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	flash_data->bootloader_mode	= DISABLE;

	if( ( error = software_reset( tool_data ) ) != ERROR_NONE )
	{

		create_return_structure( return_buffer, COMMAND_LEAVE_BOOTLOADER_MODE, error, NULL, 0 );

		get_page_description( tool_data );

	}
	else
		create_return_structure( return_buffer, COMMAND_LEAVE_BOOTLOADER_MODE, ERROR_NONE, NULL, 0 );

}

void	bootloader_mode_status( void *node_data, struct command_parameter *parameter )
{

	struct	register_information
	{

		int	function_number, register_id, register_index;

		u8	*register_offset;

		u8	*data;

		char	*string;

	};

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_data = tool_data->flash_data;

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	u8	block_size, flash_program_state, flash_data3_state;

	struct register_information	register_info[] =
	{

		{ RMI_FUNCTION_34, REGISTER_QUERY_ID, RMI_F34_QUERY3_BLOCK_SIZE_7_0, NULL, &block_size, "Get block size" },
		{ RMI_FUNCTION_01, REGISTER_DATA_ID, RMI_F01_DATA0, NULL, &flash_program_state, "Get mode state" },
		{ RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_FLASH_COMMAND, &block_size, &flash_data3_state, "Get flash state" },

	};

	int	loop, error;

	u16	address;

	tool_data->command_id	= COMMAND_BOOTLOADER_MODE_STATUS;

	if( !flash_data )
	{

		printk( "TTUCH : Not found resource\n" );

		create_return_structure( return_buffer, COMMAND_BOOTLOADER_MODE_STATUS, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	for( loop = 0 ; loop < sizeof( register_info ) / sizeof( *register_info ) ; ++loop )
	{

		struct register_information	*information = register_info + loop;

		u8	offset = information->register_offset ? *information->register_offset : 0;

		if( ( address = get_register_addr( tool_data, information->function_number, information->register_id ) ) == INVALID_ADDRESS )
		{

			printk( "TTUCH : Can't get %s register address\n", get_register_string( information->register_id ) );

			create_return_structure( return_buffer, COMMAND_BOOTLOADER_MODE_STATUS, ERROR_INVALID_ADDRESS, NULL, 0 );

			break;

		}

		if( ( error = rmi_read( rmi_data, address + information->register_index + offset, information->data ) ) < 0 )
		{

			printk( "TTUCH : RMI read failed, ERROR(%d)\n", error );

			create_return_structure( return_buffer, COMMAND_BOOTLOADER_MODE_STATUS, ERROR_I2C_ACCESS, NULL, 0 );

			break;

		}

	}

	if( loop == sizeof( register_info ) / sizeof( *register_info ) )
	{

		int	mode = UI_MODE;

		printk( "TTUCH : Flash status(%d), Program enabled(%d), Flash program(%d)\n", FLASH_STATUS( flash_data3_state ), PROGRAM_ENABLED( flash_data3_state ), FLASH_PROGRAM( flash_program_state ) );

		if( FLASH_STATUS_SUCCESS( flash_data3_state ) && PROGRAM_ENABLED( flash_data3_state ) && FLASH_PROGRAM( flash_program_state ) )
			mode = BOOTLOADER_MODE;

		printk( "TTUCH : In %s mode\n", mode == UI_MODE ? "UI" : "Bootloader" );

		create_return_structure( return_buffer, COMMAND_BOOTLOADER_MODE_STATUS, ERROR_NONE, &mode, 1 );

	}
	else
		printk( "TTUCH : %s failed\n", ( register_info + loop )->string );

}

void	S_write_function_group_block_data( void *node_data, struct command_parameter *parameter )
{

	#pragma	pack( push )

	#pragma	pack( 1 )

	struct	parameters_info
	{

		int	function, register_id, start_register, parameter_count;

		int	parameter1, parameter2, parameter3, parameter4;
			
		int	parameter5, parameter6, parameter7, parameter8;

		int	parameter9, parameter10, parameter11, parameter12;

		int	parameter13, parameter14, parameter15, parameter16;

	};

	#pragma	pack( pop )

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number, parameter->para3.container.number, parameter->para4.container.number,
		parameter->para5.container.number, parameter->para6.container.number, parameter->para7.container.number, parameter->para8.container.number,
		parameter->para9.container.number, parameter->para10.container.number, parameter->para11.container.number, parameter->para12.container.number,
		parameter->para13.container.number, parameter->para14.container.number, parameter->para15.container.number, parameter->para16.container.number,
		parameter->para17.container.number, parameter->para18.container.number, parameter->para19.container.number, parameter->para20.container.number,

	};

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	int	*source = &parameters.parameter1;

	u8	*values = tool_data->parameter_buffer;

	int	loop;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP );

	tool_data->command_id	= COMMAND_S_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP;

	if( parameters.parameter_count > MAX_BLOCK_DATA )
	{

		printk( "TTUCH : Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		info_print( INFO_BUFFER_ADD, "Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		return;

	}

	for( loop = 0 ; loop < parameters.parameter_count ; ++loop )
		*( values + loop )	= *( source + loop );

	if( !write_register_data( tool_data, parameters.function, parameters.register_id, parameters.start_register, values, parameters.parameter_count ) )
	{

		printk( "TTUCH : Write register failed\n" );

		info_print( INFO_BUFFER_ADD, "Write register failed\n" );

	}
	else
	{

		u8	*buffer = ( ( struct touch_tool_data* )node_data )->block_data_buffer;

		creat_block_data_number_string( buffer, values, parameters.parameter_count );

		printk( "TTUCH : block data, [F%02x:%s%d]<<%s\n", parameters.function, get_register_string( parameters.register_id ), parameters.start_register, buffer );

		info_print( INFO_BUFFER_ADD, "block data, [F%02x:%s%d]<<%s\n", parameters.function, get_register_string( parameters.register_id ), parameters.start_register, buffer );

	}

}

void	S_write_register_block_data( void *node_data, struct command_parameter *parameter )
{

	#pragma	pack( push )

	#pragma	pack( 1 )

	struct	parameters_info
	{

		int	start_register, parameter_count;

		int	parameter1, parameter2, parameter3, parameter4;
			
		int	parameter5, parameter6, parameter7, parameter8;

		int	parameter9, parameter10, parameter11, parameter12;

		int	parameter13, parameter14, parameter15, parameter16;

	};

	#pragma	pack( pop )

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number,
		parameter->para3.container.number, parameter->para4.container.number, parameter->para5.container.number, parameter->para6.container.number,
		parameter->para7.container.number, parameter->para8.container.number, parameter->para9.container.number, parameter->para10.container.number,
		parameter->para11.container.number, parameter->para12.container.number, parameter->para13.container.number, parameter->para14.container.number,
		parameter->para15.container.number, parameter->para16.container.number, parameter->para17.container.number, parameter->para18.container.number,

	};

	struct synaptics_rmi4_data	*rmi_data = ( ( struct touch_tool_data* )node_data )->rmi_data;

	int	*source = &parameters.parameter1;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer;

	int	error, loop;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_WRITE_BLOCK_DATA_FROM_REGISTER );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_WRITE_BLOCK_DATA_FROM_REGISTER;

	if( parameters.parameter_count > MAX_BLOCK_DATA )
	{

		printk( "TTUCH : Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		info_print( INFO_BUFFER_ADD, "Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		return;

	}

	for( loop = 0 ; loop < parameters.parameter_count ; ++loop )
		*( values + loop )	= *( source + loop );

	if( ( error = rmi_write_block( rmi_data, parameters.start_register, values, parameters.parameter_count ) ) < 0 )
	{

		printk( "TTUCH : RMI block write failed, ERROR(%d)\n", error );

		info_print( INFO_BUFFER_ADD, "RMI read failed, ERROR(%d)\n", error );

	}
	else
	{

		u8	*buffer = ( ( struct touch_tool_data* )node_data )->block_data_buffer;

		creat_block_data_number_string( buffer, values, parameters.parameter_count );

		printk( "TTUCH : block data, [REG%d]<<%s\n", parameters.start_register, buffer );

		info_print( INFO_BUFFER_ADD, "block data, [REG%d]<<%s\n", parameters.start_register, buffer );

	}

}

void	N_write_function_group_block_data( void *node_data, struct command_parameter *parameter )
{

	#pragma	pack( push )

	#pragma	pack( 1 )

	struct	parameters_info
	{

		int	function, register_id, start_register, parameter_count;

		int	parameter1, parameter2, parameter3, parameter4;
			
		int	parameter5, parameter6, parameter7, parameter8;

		int	parameter9, parameter10, parameter11, parameter12;

		int	parameter13, parameter14, parameter15, parameter16;

	};

	#pragma	pack( pop )

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number, parameter->para3.container.number, parameter->para4.container.number,
		parameter->para5.container.number, parameter->para6.container.number, parameter->para7.container.number, parameter->para8.container.number,
		parameter->para9.container.number, parameter->para10.container.number, parameter->para11.container.number, parameter->para12.container.number,
		parameter->para13.container.number, parameter->para14.container.number, parameter->para15.container.number, parameter->para16.container.number,
		parameter->para17.container.number, parameter->para18.container.number, parameter->para19.container.number, parameter->para20.container.number,

	};

	struct touch_tool_data	*tool_data = ( struct touch_tool_data* )node_data;

	int	*source = &parameters.parameter1;

	struct return_struct	*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	u8	*values = tool_data->parameter_buffer + sizeof( struct return_struct );

	int	error = ERROR_NONE;

	int	loop;

	tool_data->command_id	= COMMAND_N_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP;

	if( parameters.parameter_count > MAX_BLOCK_DATA )
	{

		printk( "TTUCH : Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		create_return_structure( return_buffer, COMMAND_N_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP, ERROR_BUFFER_FULL, NULL, 0 );

		return;

	}

	for( loop = 0 ; loop < parameters.parameter_count ; ++loop )
		*( values + loop )	= *( source + loop );

	if( !write_register_data( tool_data, parameters.function, parameters.register_id, parameters.start_register, values, parameters.parameter_count ) )
	{

		printk( "TTUCH : Write register failed\n" );

		error	= ERROR_I2C_WRITE;

	}

	create_return_structure( return_buffer, COMMAND_N_WRITE_BLOCK_DATA_FROM_FUNCTION_GROUP, error, NULL, parameters.parameter_count );

}

void	N_write_register_block_data( void *node_data, struct command_parameter *parameter )
{

	#pragma	pack( push )

	#pragma	pack( 1 )

	struct	parameters_info
	{

		int	start_register, parameter_count;

		int	parameter1, parameter2, parameter3, parameter4;
			
		int	parameter5, parameter6, parameter7, parameter8;

		int	parameter9, parameter10, parameter11, parameter12;

		int	parameter13, parameter14, parameter15, parameter16;

	};

	#pragma	pack( pop )

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number,
		parameter->para3.container.number, parameter->para4.container.number, parameter->para5.container.number, parameter->para6.container.number,
		parameter->para7.container.number, parameter->para8.container.number, parameter->para9.container.number, parameter->para10.container.number,
		parameter->para11.container.number, parameter->para12.container.number, parameter->para13.container.number, parameter->para14.container.number,
		parameter->para15.container.number, parameter->para16.container.number, parameter->para17.container.number, parameter->para18.container.number,

	};

	struct synaptics_rmi4_data	*rmi_data = ( ( struct touch_tool_data* )node_data )->rmi_data;

	int	*source = &parameters.parameter1;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer + sizeof( struct return_struct );

	int	error = ERROR_NONE;

	int	loop;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_WRITE_BLOCK_DATA_FROM_REGISTER;

	if( parameters.parameter_count > MAX_BLOCK_DATA )
	{

		printk( "TTUCH : Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		create_return_structure( return_buffer, COMMAND_N_WRITE_BLOCK_DATA_FROM_REGISTER, ERROR_BUFFER_FULL, NULL, 0 );

		return;

	}

	for( loop = 0 ; loop < parameters.parameter_count ; ++loop )
		*( values + loop )	= *( source + loop );

	if( rmi_write_block( rmi_data, parameters.start_register, values, parameters.parameter_count ) < 0 )
	{

		printk( "TTUCH : RMI block write failed, ERROR(%d)\n", error );

		error	= ERROR_I2C_WRITE;

	}

	create_return_structure( return_buffer, COMMAND_N_WRITE_BLOCK_DATA_FROM_REGISTER, error, NULL, parameters.parameter_count );

}

void	scan_page_description_table( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	tool_data->command_id	= COMMAND_SCAN_PAGE_DESCRIPTION_TABLE;

	info_print( INFO_BUFFER_RESET, NULL );

	get_page_description( tool_data );

}

void	create_flash_data( void *node_data, struct command_parameter *parameter )
{

	struct	read_information
	{

		int	register_offset;

		int	*store_address;

	};

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	struct flash_information	*flash_info = tool_data->flash_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	u16	address = get_register_addr( tool_data, RMI_FUNCTION_34, REGISTER_QUERY_ID );

	int	block_size = 0, block_count = 0;

	u8	block_info[ 2 ];

	int	loop;

	struct read_information	read_list[] =
	{

		{ RMI_F34_QUERY3_BLOCK_SIZE_7_0, &block_size },
		{ RMI_F34_QUERY7_CONFIG_BLOCK_COUNT_7_0, &block_count },

	};

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_ALLOCATE_FLASH_DATA;

	if( flash_info )
	{

		printk( "TTUCH : Flash information is exist\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FLASH_DATA, ERROR_RESOURCE_EXIST, NULL, 0 );

		return;

	}

	if( address == INVALID_ADDRESS )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( REGISTER_QUERY_ID ) );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FLASH_DATA, ERROR_INVALID_ADDRESS, NULL, 0 );

		return;

	}

	for( loop = 0 ; loop < sizeof( read_list ) / sizeof( *read_list ) ; ++loop )
		if( rmi_read_block( rmi_data, address + ( read_list + loop )->register_offset, block_info, sizeof( block_info ) )  < 0 )
		{

			printk( "TTUCH : RMI block read failed\n" );

			create_return_structure( return_buffer, COMMAND_ALLOCATE_FLASH_DATA, ERROR_I2C_READ, NULL, 0 );

			return;

		}
		else
			*( read_list + loop )->store_address	= TO_INT( block_info );

	if( ( flash_info = kzalloc( sizeof( struct flash_information ), GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Get memory failed\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FLASH_DATA, ERROR_ALLOCATE_FAILED, NULL, 0 );

		return;

	}

	flash_info->size	= block_size * block_count;

	if( ( flash_info->buffer = kzalloc( flash_info->size, GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : memory of flash information failed\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FLASH_DATA, ERROR_ALLOCATE_FAILED, NULL, 0 );

		kfree( flash_info );

		return;

	}

	printk( "TTUCH : [Block size:Block count:Memory]=[%d:%d:%d]\n", block_size, block_count, flash_info->size );

	tool_data->flash_data	= flash_info;

	create_return_structure( return_buffer, COMMAND_ALLOCATE_FLASH_DATA, ERROR_NONE, &flash_info->size, 1 );

}

void	release_flash_data( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_info = tool_data->flash_data;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_RELEASE_FLASH_DATA;

	if( !flash_info )
	{

		printk( "TTUCH : Flash information is non-exist\n" );

		create_return_structure( return_buffer, COMMAND_RELEASE_FLASH_DATA, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	kfree( flash_info->buffer );

	kfree( flash_info );

	tool_data->flash_data	= NULL;

	create_return_structure( return_buffer, COMMAND_RELEASE_FLASH_DATA, ERROR_NONE, NULL, 0 );

}

void	reset_touch_ic( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	int	error;

	tool_data->command_id	= COMMAND_RESET_TOUCH_IC;

	if( ( error = software_reset( tool_data ) ) != ERROR_NONE )
		create_return_structure( return_buffer, COMMAND_RESET_TOUCH_IC, error, NULL, 0 );
	else
	{

		unsigned	length = strlen( F01_COMMAND0_RETURN_STRING );

		memcpy( ( char* )( tool_data->parameter_buffer + sizeof( struct return_struct ) ), F01_COMMAND0_RETURN_STRING, length + 1 );

		create_return_structure( return_buffer, COMMAND_RESET_TOUCH_IC, ERROR_NONE, &length, 1 );

	}

}

void	create_file_information( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data	*tool_data = ( struct touch_tool_data* )node_data;

	struct file_information	*file_info = tool_data->file_data;

	struct return_struct	*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	char			*file_name = parameter->para1.container.string;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_ALLOCATE_FILE_INFORMATION;

	if( file_info )
	{

		printk( "TTUCH : File information is exist\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FILE_INFORMATION, ERROR_RESOURCE_EXIST, NULL, 0 );

		return;

	}

	if( ( file_info = kzalloc( sizeof( struct file_information ), GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Get memory failed\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FILE_INFORMATION, ERROR_ALLOCATE_FAILED, NULL, 0 );

		return;

	}

	strncpy( file_info->name, file_name, strlen( file_name ) + 1 );

	file_info->image_file	= filp_open( file_info->name, O_WRONLY, 0 );

	if( IS_ERR( file_info->image_file ) )
	{

		printk( "TTUCH : Open file(%s) failed\n", file_info->name );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FILE_INFORMATION, ERROR_OPEN_FILE_FAILED, NULL, 0 );

		kfree( file_info );

		return;

	}

	if( !file_info->image_file->f_op || !file_info->image_file->f_op->read || !file_info->image_file->f_op->write || !file_info->image_file->f_op->llseek )
	{

		printk( "TTUCH : Not found read/write function\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FILE_INFORMATION, ERROR_NOT_FOUND_FUNCTION, NULL, 0 );

		kfree( file_info );

		return;

	}

	tool_data->file_data	= file_info;

	printk( "TTUCH : Open file(%s) success\n", file_info->name );

	create_return_structure( return_buffer, COMMAND_ALLOCATE_FILE_INFORMATION, ERROR_NONE, &file_info->name, 1 );

}

void	release_file_information( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data	*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct	*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct file_information	*file_info = tool_data->file_data;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_RELEASE_FILE_INFORMATION;

	if( !file_info )
	{

		printk( "TTUCH : File information is non-exist\n" );

		create_return_structure( return_buffer, COMMAND_RELEASE_FILE_INFORMATION, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	if( filp_close( file_info->image_file, NULL ) )
	{

		printk( "TTUCH : Close file(%s) failed\n", file_info->name );

		create_return_structure( return_buffer, COMMAND_RELEASE_FILE_INFORMATION, ERROR_CLOSE_FILE_FAILED, NULL, 0 );

	}

	kfree( file_info );

	tool_data->file_data	= NULL;

	create_return_structure( return_buffer, COMMAND_RELEASE_FILE_INFORMATION, ERROR_NONE, NULL, 0 );

}

void	memory_dump_value( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct flash_information	*flash_data = tool_data->flash_data;

	tool_data->command_id	= COMMAND_MEMORY_DUMP_VALUE;

	if( !flash_data )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Memory buffer[%d]\n", flash_data->size );

	dump_memory_value( tool_data, flash_data->buffer, 0, flash_data->size - 1 );

}

void	memory_dump_character( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct flash_information	*flash_data = tool_data->flash_data;

	tool_data->command_id	= COMMAND_MEMORY_DUMP_CHARACTER;

	if( !flash_data )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Memory buffer[%d]\n", flash_data->size );

	dump_memory_character( tool_data, flash_data->buffer, 0, flash_data->size - 1 );

}

void	memory_to_file( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_data = tool_data->flash_data;
	
	struct file_information		*file_info = tool_data->file_data;

	struct file			*image_file;

	loff_t f_ops;

	int	count;

	tool_data->command_id	= COMMAND_MEMORY_TO_FILE;

	if( !flash_data || !file_info || !return_buffer )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	if( !( image_file = file_info->image_file ) )
	{

		printk( "TTUCH : Does not open file\n" );

		create_return_structure( return_buffer, COMMAND_MEMORY_TO_FILE, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	printk( "TTUCH : Memory[0:%d] << Flash[0:%d]\n", flash_data->size - 1, flash_data->size - 1 );

	set_fs( get_ds() );

	f_ops	= image_file->f_op->llseek( image_file, 0, 0 );

	if( ( count = image_file->f_op->write( image_file, flash_data->buffer, flash_data->size, &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Write file failed\n" );

		create_return_structure( return_buffer, COMMAND_MEMORY_TO_FILE, ERROR_FILE_ACCESS_FAILED, NULL, 0 );

		return;

	}

	printk( "TTUCH : Memory[0:%d] >> File[0:%d]\n", flash_data->size - 1, count - 1 );

	create_return_structure( return_buffer, COMMAND_MEMORY_TO_FILE, count == flash_data->size ? ERROR_NONE : ERROR_VALUE_INCORRECT, NULL, 0 );

}

void	memory_to_flash( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_data = tool_data->flash_data;

	u8	reset_block_count[] = { 0, 0 };

	u8	size[ 2 ], count[ 2 ];

	u8	flash_status;

	int	loop;

	unsigned int	block_size, block_count;

	tool_data->command_id	= COMMAND_MEMORY_TO_FLASH;

	if( !flash_data || !return_buffer )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	{

		struct register_datas_info	datas_info[] =
		{

			{ RMI_F34_QUERY3_BLOCK_SIZE_7_0, size, sizeof( size ) },
			{ RMI_F34_QUERY7_CONFIG_BLOCK_COUNT_7_0, count, sizeof( count ) },

		};

		if( !read_register_datas( tool_data, RMI_FUNCTION_34, REGISTER_QUERY_ID, datas_info, sizeof( datas_info ) / sizeof( *datas_info ) ) )
		{

			printk( "TTUCH : [f34:Query], read failed\n" );

			create_return_structure( return_buffer, COMMAND_MEMORY_TO_FLASH, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

	}

	CONVERT_ARRAY_TO_INT( block_count, count ), CONVERT_ARRAY_TO_INT( block_size, size );

	printk( "TTUCH : Block[count:size]=[%d:%d], total size(%d byte)\n", block_count, block_size, block_count * block_size );

	if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_NUMBER_7_0, reset_block_count, sizeof( reset_block_count ) ) )
	{

		printk( "TTUCH : Reset block count failed\n" );

		create_return_structure( return_buffer, COMMAND_MEMORY_TO_FLASH, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	for( loop = 0 ; loop < block_count ; ++loop )
	{

		if( block_size * loop >= flash_data->size )
		{

			printk( "TTUCH : Out of flash buffer\n" );

			create_return_structure( return_buffer, COMMAND_MEMORY_TO_FLASH, ERROR_OUT_OF_BUFFER, NULL, 0 );

			return;

		}

		if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA, flash_data->buffer + block_size * loop, block_size ) )
		{

			printk( "TTUCH : write register failed\n" );

			create_return_structure( return_buffer, COMMAND_MEMORY_TO_FLASH, ERROR_I2C_READ, NULL, 0 );

			return;

		}

		if( !flash_command( tool_data, F34_WRITE_CONFIGURATION_BLOCK, WAIT_1000_MS ) )
		{

			printk( "TTUCH : Write flash command failed\n" );

			create_return_structure( return_buffer, COMMAND_MEMORY_TO_FLASH, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

		if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA + block_size, &flash_status, sizeof( flash_status ) ) )
		{

			printk( "TTUCH : Read register failed\n" );

			create_return_structure( return_buffer, COMMAND_MEMORY_TO_FLASH, ERROR_I2C_READ, NULL, 0 );

			return;

		}

		if( !FLASH_STATUS_SUCCESS( flash_status ) )
		{

			printk( "TTUCH : Flash failed\n" );

			create_return_structure( return_buffer, COMMAND_MEMORY_TO_FLASH, ERROR_FLASH_FAILED, NULL, 0 );

			return;

		}

	}

	printk( "TTUCH : Memory[0:%d] >> Flash[0:%d]\n", flash_data->size - 1, flash_data->size - 1 );

	create_return_structure( return_buffer, COMMAND_MEMORY_TO_FLASH, ERROR_NONE, NULL, 0 );

}

void	flash_to_memory( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_data = tool_data->flash_data;

	u8	reset_block_count[] = { 0, 0 };

	u8	size[ 2 ], count[ 2 ];

	int	loop;

	unsigned int	block_size, block_count;

	tool_data->command_id	= COMMAND_FLASH_TO_MEMORY;

	if( !flash_data || !return_buffer )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	{

		struct register_datas_info	datas_info[] =
		{

			{ RMI_F34_QUERY3_BLOCK_SIZE_7_0, size, sizeof( size ) },
			{ RMI_F34_QUERY7_CONFIG_BLOCK_COUNT_7_0, count, sizeof( count ) },

		};

		if( !read_register_datas( tool_data, RMI_FUNCTION_34, REGISTER_QUERY_ID, datas_info, sizeof( datas_info ) / sizeof( *datas_info ) ) )
		{

			printk( "TTUCH : [f34:Query], read failed\n" );

			create_return_structure( return_buffer, COMMAND_FLASH_TO_MEMORY, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

	}

	CONVERT_ARRAY_TO_INT( block_count, count ), CONVERT_ARRAY_TO_INT( block_size, size );

	printk( "TTUCH : Block[count:size]=[%d:%d], total size(%d byte)\n", block_count, block_size, block_count * block_size );

	if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_NUMBER_7_0, reset_block_count, sizeof( reset_block_count ) ) )
	{

		printk( "TTUCH : Reset block count failed\n" );

		create_return_structure( return_buffer, COMMAND_FLASH_TO_MEMORY, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	for( loop = 0 ; loop < block_count ; ++loop )
	{

		if( !flash_command( tool_data, F34_READ_CONFIGURATION_BLOCK, WAIT_1000_MS ) )
		{

			printk( "TTUCH : Write flash command failed\n" );

			create_return_structure( return_buffer, COMMAND_FLASH_TO_MEMORY, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

		if( block_size * loop >= flash_data->size )
		{

			printk( "TTUCH : Out of flash buffer\n" );

			create_return_structure( return_buffer, COMMAND_FLASH_TO_MEMORY, ERROR_OUT_OF_BUFFER, NULL, 0 );

			return;

		}

		if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA, flash_data->buffer + block_size * loop, block_size ) )
		{

			printk( "TTUCH : Read register failed\n" );

			create_return_structure( return_buffer, COMMAND_FLASH_TO_MEMORY, ERROR_I2C_READ, NULL, 0 );

			return;

		}

	}

	printk( "TTUCH : Memory[0:%d] << Flash[0:%d]\n", flash_data->size - 1, flash_data->size - 1 );

	create_return_structure( return_buffer, COMMAND_FLASH_TO_MEMORY, ERROR_NONE, NULL, 0 );

}

void	flash_to_file( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_data = tool_data->flash_data;
	
	struct file_information		*file_info = tool_data->file_data;

	struct file			*image_file;

	loff_t f_ops;

	int	count;

	tool_data->command_id	= COMMAND_FLASH_TO_FILE;

	if( !flash_data || !file_info || !return_buffer )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	if( !( image_file = file_info->image_file ) )
	{

		printk( "TTUCH : Does not open file\n" );

		create_return_structure( return_buffer, COMMAND_FLASH_TO_FILE, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	{

		u8	reset_block_count[] = { 0, 0 };

		u8	size[ 2 ], count[ 2 ];

		struct register_datas_info	datas_info[] =
		{

			{ RMI_F34_QUERY3_BLOCK_SIZE_7_0, size, sizeof( size ) },
			{ RMI_F34_QUERY7_CONFIG_BLOCK_COUNT_7_0, count, sizeof( count ) },

		};

		int	loop;

		unsigned int	block_size, block_count;

		if( !read_register_datas( tool_data, RMI_FUNCTION_34, REGISTER_QUERY_ID, datas_info, sizeof( datas_info ) / sizeof( *datas_info ) ) )
		{

			printk( "TTUCH : [f34:Query], read failed\n" );

			create_return_structure( return_buffer, COMMAND_FLASH_TO_FILE, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

		CONVERT_ARRAY_TO_INT( block_count, count ), CONVERT_ARRAY_TO_INT( block_size, size );

		printk( "TTUCH : Block[count:size]=[%d:%d], total size(%d byte)\n", block_count, block_size, block_count * block_size );

		if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_NUMBER_7_0, reset_block_count, sizeof( reset_block_count ) ) )
		{

			printk( "TTUCH : Reset block count failed\n" );

			create_return_structure( return_buffer, COMMAND_FLASH_TO_FILE, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

		for( loop = 0 ; loop < block_count ; ++loop )
		{

			if( !flash_command( tool_data, F34_READ_CONFIGURATION_BLOCK, WAIT_1000_MS ) )
			{

				printk( "TTUCH : Write flash command failed\n" );

				create_return_structure( return_buffer, COMMAND_FLASH_TO_FILE, ERROR_RETURN_FAILED, NULL, 0 );

				return;

			}

			if( block_size * loop >= flash_data->size )
			{

				printk( "TTUCH : Out of flash buffer\n" );

				create_return_structure( return_buffer, COMMAND_FLASH_TO_FILE, ERROR_OUT_OF_BUFFER, NULL, 0 );

				return;

			}

			if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA, flash_data->buffer + block_size * loop, block_size ) )
			{

				printk( "TTUCH : Read register failed\n" );

				create_return_structure( return_buffer, COMMAND_FLASH_TO_FILE, ERROR_I2C_READ, NULL, 0 );

				return;

			}

		}

	}

	printk( "TTUCH : Memory[0:%d] << Flash[0:%d]\n", flash_data->size - 1, flash_data->size - 1 );

	set_fs( get_ds() );

	f_ops	= image_file->f_op->llseek( image_file, 0, 0 );

	if( ( count = image_file->f_op->write( image_file, flash_data->buffer, flash_data->size, &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Write file failed\n" );

		create_return_structure( return_buffer, COMMAND_FLASH_TO_FILE, ERROR_FILE_ACCESS_FAILED, NULL, 0 );

		return;

	}

	printk( "TTUCH : Memory[0:%d] >> File[0:%d]\n", flash_data->size - 1, count - 1 );

	create_return_structure( return_buffer, COMMAND_FLASH_TO_FILE, count == flash_data->size ? ERROR_NONE : ERROR_VALUE_INCORRECT, NULL, 0 );

}

void	file_to_memory( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_data = tool_data->flash_data;
	
	struct file_information		*file_info = tool_data->file_data;

	struct file			*image_file;

	loff_t f_ops;

	int	count;

	tool_data->command_id	= COMMAND_FILE_TO_MEMORY;

	if( !flash_data || !file_info || !return_buffer )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	if( !( image_file = file_info->image_file ) )
	{

		printk( "TTUCH : Does not open file\n" );

		create_return_structure( return_buffer, COMMAND_FILE_TO_MEMORY, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	set_fs( get_ds() );

	f_ops	= image_file->f_op->llseek( image_file, 0, 0 );

	if( ( count = image_file->f_op->read( image_file, flash_data->buffer, flash_data->size, &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read file failed\n" );

		create_return_structure( return_buffer, COMMAND_FILE_TO_MEMORY, ERROR_FILE_ACCESS_FAILED, NULL, 0 );

		return;

	}

	printk( "TTUCH : Memory[0:%d] << File[0:%d]\n", flash_data->size - 1, count - 1 );

	create_return_structure( return_buffer, COMMAND_FILE_TO_MEMORY, ERROR_NONE, NULL, 0 );

}

void	file_to_flash( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_data = tool_data->flash_data;
	
	struct file_information		*file_info = tool_data->file_data;

	struct file			*image_file;

	u8	reset_block_count[] = { 0, 0 };

	u8	size[ 2 ], count[ 2 ];

	u8	flash_status;

	loff_t f_ops;

	int	loop, read_count;

	unsigned int	block_size, block_count;

	tool_data->command_id	= COMMAND_FILE_TO_FLASH;

	if( !flash_data || !file_info || !return_buffer )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	if( !( image_file = file_info->image_file ) )
	{

		printk( "TTUCH : Does not open file\n" );

		create_return_structure( return_buffer, COMMAND_FILE_TO_FLASH, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	{

		struct register_datas_info	datas_info[] =
		{

			{ RMI_F34_QUERY3_BLOCK_SIZE_7_0, size, sizeof( size ) },
			{ RMI_F34_QUERY7_CONFIG_BLOCK_COUNT_7_0, count, sizeof( count ) },

		};

		if( !read_register_datas( tool_data, RMI_FUNCTION_34, REGISTER_QUERY_ID, datas_info, sizeof( datas_info ) / sizeof( *datas_info ) ) )
		{

			printk( "TTUCH : [f34:Query], read failed\n" );

			create_return_structure( return_buffer, COMMAND_FILE_TO_FLASH, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

	}

	CONVERT_ARRAY_TO_INT( block_count, count ), CONVERT_ARRAY_TO_INT( block_size, size );

	printk( "TTUCH : Block[count:size]=[%d:%d], total size(%d byte)\n", block_count, block_size, block_count * block_size );

	if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_NUMBER_7_0, reset_block_count, sizeof( reset_block_count ) ) )
	{

		printk( "TTUCH : Reset block count failed\n" );

		create_return_structure( return_buffer, COMMAND_FILE_TO_FLASH, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	set_fs( get_ds() );

	f_ops	= image_file->f_op->llseek( image_file, 0, 0 );

	if( ( read_count = image_file->f_op->read( image_file, flash_data->buffer, flash_data->size, &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read file failed\n" );

		create_return_structure( return_buffer, COMMAND_FILE_TO_FLASH, ERROR_FILE_ACCESS_FAILED, NULL, 0 );

		return;

	}

	printk( "TTUCH : Memory[0:%d] << File[0:%d]\n", flash_data->size - 1, read_count - 1 );

	for( loop = 0 ; loop < block_count ; ++loop )
	{

		if( block_size * loop >= flash_data->size )
		{

			printk( "TTUCH : Out of flash buffer\n" );

			create_return_structure( return_buffer, COMMAND_FILE_TO_FLASH, ERROR_OUT_OF_BUFFER, NULL, 0 );

			return;

		}

		if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA, flash_data->buffer + block_size * loop, block_size ) )
		{

			printk( "TTUCH : write register failed\n" );

			create_return_structure( return_buffer, COMMAND_FILE_TO_FLASH, ERROR_I2C_READ, NULL, 0 );

			return;

		}

		if( !flash_command( tool_data, F34_WRITE_CONFIGURATION_BLOCK, WAIT_1000_MS ) )
		{

			printk( "TTUCH : Write flash command failed\n" );

			create_return_structure( return_buffer, COMMAND_FILE_TO_FLASH, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

		if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA + block_size, &flash_status, sizeof( flash_status ) ) )
		{

			printk( "TTUCH : Read register failed\n" );

			create_return_structure( return_buffer, COMMAND_FILE_TO_FLASH, ERROR_I2C_READ, NULL, 0 );

			return;

		}

		if( !FLASH_STATUS_SUCCESS( flash_status ) )
		{

			printk( "TTUCH : Flash failed\n" );

			create_return_structure( return_buffer, COMMAND_FILE_TO_FLASH, ERROR_FLASH_FAILED, NULL, 0 );

			return;

		}

	}

	printk( "TTUCH : File[0:%d] >> Flash[0:%d]\n", flash_data->size - 1, flash_data->size - 1 );

	create_return_structure( return_buffer, COMMAND_FILE_TO_FLASH, ERROR_NONE, NULL, 0 );

}

void	enable_ic_irq( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	tool_data->command_id	= COMMAND_ENABLE_IRQ;

	if( rmi_set_irq( rmi_data, true ) )
	{

		printk( "TTUCH : Set IRQ failed\n" );

		create_return_structure( return_buffer, COMMAND_ENABLE_IRQ, ERROR_IRQ_CONTROL_FAILED, NULL, 0 );

	}
	else
		create_return_structure( return_buffer, COMMAND_ENABLE_IRQ, ERROR_NONE, NULL, 0 );

}

void	disable_ic_irq( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	tool_data->command_id	= COMMAND_DISABLE_IRQ;

	if( rmi_set_irq( rmi_data, false ) )
	{

		printk( "TTUCH : Set IRQ failed\n" );

		create_return_structure( return_buffer, COMMAND_DISABLE_IRQ, ERROR_IRQ_CONTROL_FAILED, NULL, 0 );

	}
	else
		create_return_structure( return_buffer, COMMAND_DISABLE_IRQ, ERROR_NONE, NULL, 0 );

}

void	full_power_mode( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	tool_data->command_id	= COMMAND_FULL_POWER_MODE;

	create_return_structure( return_buffer, COMMAND_FULL_POWER_MODE, enter_full_power_mode( tool_data ) ? ERROR_NONE : ERROR_RETURN_FAILED, NULL, 0 );

}

void	write_memory_value( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct flash_information	*flash_data = tool_data->flash_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	unsigned int	address = parameter->para1.container.number, value = parameter->para2.container.number;

	char	*buffer;

	tool_data->command_id	= COMMAND_WRITE_VALUE_TO_MEMORY;

	if( !flash_data || !( buffer = flash_data->buffer ) || !return_buffer )
	{

		printk( "TTUCH : Invalid address\n" );

		return;

	}

	if( address >= flash_data->size )
	{

		printk( "TTUCH : Out of memory\n" );

		create_return_structure( return_buffer, COMMAND_WRITE_VALUE_TO_MEMORY, ERROR_OUT_OF_BUFFER, NULL, 0 );

		return;

	}

	*( buffer + address )	= value;

	create_return_structure( return_buffer, COMMAND_WRITE_VALUE_TO_MEMORY, ERROR_NONE, NULL, 0 );

}

void	write_memory_block_value( void *node_data, struct command_parameter *parameter )
{

	#pragma	pack( push )

	#pragma	pack( 1 )

	struct	parameters_info
	{

		int	memory_address, parameter_count;

		int	parameter1, parameter2, parameter3, parameter4;
			
		int	parameter5, parameter6, parameter7, parameter8;

		int	parameter9, parameter10, parameter11, parameter12;

		int	parameter13, parameter14, parameter15, parameter16;

	};

	#pragma	pack( pop )

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number,
		parameter->para3.container.number, parameter->para4.container.number, parameter->para5.container.number, parameter->para6.container.number,
		parameter->para7.container.number, parameter->para8.container.number, parameter->para9.container.number, parameter->para10.container.number,
		parameter->para11.container.number, parameter->para12.container.number, parameter->para13.container.number, parameter->para14.container.number,
		parameter->para15.container.number, parameter->para16.container.number, parameter->para17.container.number, parameter->para18.container.number,

	};

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct flash_information	*flash_data = tool_data->flash_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	int	*parameter_buffer;

	char	*buffer;

	int	loop;

	tool_data->command_id	= COMMAND_WRITE_BLOCK_VALUE_TO_MEMORY;

	if( !flash_data || !flash_data->buffer || !return_buffer )
	{

		printk( "TTUCH : Resource not exist\n" );

		return;

	}

	if( parameters.memory_address >= flash_data->size || parameters.memory_address + parameters.parameter_count >= flash_data->size )
	{

		printk( "TTUCH : Out of memory\n" );

		create_return_structure( return_buffer, COMMAND_WRITE_BLOCK_VALUE_TO_MEMORY, ERROR_OUT_OF_BUFFER, NULL, 0 );

		return;

	}

	parameter_buffer = &parameters.parameter1, buffer = flash_data->buffer + parameters.memory_address;

	for( loop = 0 ; loop < parameters.parameter_count ; ++loop )
		*( buffer + loop )	= *( parameter_buffer + loop );

	create_return_structure( return_buffer, COMMAND_WRITE_BLOCK_VALUE_TO_MEMORY, ERROR_NONE, NULL, 0 );

}

void	dump_flash_memory( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	address, count;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number,

	};

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct flash_information	*flash_data = tool_data->flash_data;

	tool_data->command_id	= COMMAND_MEMORY_DUMP;

	if( !flash_data || !flash_data->buffer )
	{

		printk( "TTUCH : Resource not exist\n" );

		return;

	}

	if( parameters.address >= flash_data->size || parameters.address + parameters.count - 1 >= flash_data->size )
	{

		printk( "TTUCH : Invalid address\n" );

		return;

	}

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Memory buffer[%d]\n", flash_data->size );

	dump_memory_value( tool_data, flash_data->buffer, parameters.address, parameters.address + parameters.count - 1 );

	dump_memory_character( tool_data, flash_data->buffer, parameters.address, parameters.address + parameters.count - 1 );

}

void	erase_all( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct flash_information	*flash_data = tool_data->flash_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	tool_data->command_id	= COMMAND_ERASE_ALL;

	if( parameter->para1.container.number != SECURITY_ID )
	{

		printk( "TTUCH : Incorrect value\n" );

		create_return_structure( return_buffer, COMMAND_ERASE_ALL, ERROR_VALUE_INCORRECT, NULL, 0 );

		return;

	}

	if( !flash_data || !flash_data->buffer )
	{

		printk( "TTUCH : Resource not exist\n" );

		create_return_structure( return_buffer, COMMAND_ERASE_ALL, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	create_return_structure( return_buffer, COMMAND_ERASE_ALL, special_flash_command( tool_data, F34_ERASE_ALL, WAIT_5000_MS ) ? ERROR_NONE : ERROR_RETURN_FAILED, NULL, 0 );

}

void	erase_configuration( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct flash_information	*flash_data = tool_data->flash_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	tool_data->command_id	= COMMAND_ERASE_CONFIGURATION;

	if( parameter->para1.container.number != SECURITY_ID )
	{

		printk( "TTUCH : Incorrect value\n" );

		create_return_structure( return_buffer, COMMAND_ERASE_CONFIGURATION, ERROR_VALUE_INCORRECT, NULL, 0 );

		return;

	}

	if( !flash_data || !flash_data->buffer )
	{

		printk( "TTUCH : Resource not exist\n" );

		create_return_structure( return_buffer, COMMAND_ERASE_CONFIGURATION, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	create_return_structure( return_buffer, COMMAND_ERASE_CONFIGURATION, special_flash_command( tool_data, F34_ERASE_CONFIGURATION, WAIT_1000_MS ) ? ERROR_NONE : ERROR_RETURN_FAILED, NULL, 0 );

}

void	show_configuration_id( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	u8	*configuration_id = tool_data->parameter_buffer + sizeof( struct return_struct );

	tool_data->command_id	= COMMAND_CONFIGURATION_ID;

	if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_CONTROL_ID, RMI_F34_CONTROL_CUSTOMER_CONFIGURATION_ID_7_0, configuration_id, FOUR_REGISTER ) )
	{

		printk( "TTUCH : Read register failed\n" );

		create_return_structure( return_buffer, COMMAND_CONFIGURATION_ID, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	printk( "TTUCH : Configuration ID[0:31]=[0x%02x,0x%02x,0x%02x,0x%02x]\n", *configuration_id, *( configuration_id + 1 ), *( configuration_id + 2 ), *( configuration_id + 3 ) );

	create_return_structure( return_buffer, COMMAND_CONFIGURATION_ID, ERROR_NONE, NULL, FOUR_REGISTER );

}

void	upgrade_configuration( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_data = tool_data->flash_data;
	
	struct file_information		*file_info = tool_data->configuration_file;

	struct file			*image_file;

	unsigned			index = parameter->para1.container.number;

	u8	reset_block_count[] = { 0, 0 };

	u8	size[ 2 ], count[ 2 ];

	u8	flash_status;

	loff_t f_ops;

	long	position;

	int	loop, read_count;

	unsigned int	block_size, block_count;

	tool_data->command_id	= COMMAND_UPGRADE_CONFIGURATION;

	if( !flash_data || !file_info || !return_buffer )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	if( !( image_file = file_info->image_file ) )
	{

		printk( "TTUCH : Does not open file\n" );

		create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	set_fs( get_ds() );

	if( !check_image_header( image_file, "Configuration", index ) )
	{

		printk( "TTUCH : Incorrect file header\n" );

		create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	if( !get_start_file_data_position( image_file, parameter->para1.container.string, &position ) )
	{

		printk( "TTUCH : Can't found %s\n", parameter->para1.container.string );

		create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	{

		struct register_datas_info	datas_info[] =
		{

			{ RMI_F34_QUERY3_BLOCK_SIZE_7_0, size, sizeof( size ) },
			{ RMI_F34_QUERY7_CONFIG_BLOCK_COUNT_7_0, count, sizeof( count ) },

		};

		if( !read_register_datas( tool_data, RMI_FUNCTION_34, REGISTER_QUERY_ID, datas_info, sizeof( datas_info ) / sizeof( *datas_info ) ) )
		{

			printk( "TTUCH : [f34:Query], read failed\n" );

			create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

	}

	CONVERT_ARRAY_TO_INT( block_count, count ), CONVERT_ARRAY_TO_INT( block_size, size );

	printk( "TTUCH : Configuration, block[count:size]=[%d:%d], total size(%d byte)\n", block_count, block_size, block_count * block_size );

	if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_NUMBER_7_0, reset_block_count, sizeof( reset_block_count ) ) )
	{

		printk( "TTUCH : Reset block count failed\n" );

		create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	f_ops	= image_file->f_op->llseek( image_file, position, 0 );

	if( ( read_count = image_file->f_op->read( image_file, flash_data->buffer, flash_data->size, &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read file failed\n" );

		create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_FILE_ACCESS_FAILED, NULL, 0 );

		return;

	}

	printk( "TTUCH : Memory[0:%d] << Config file[0:%d]\n", flash_data->size - 1, read_count - 1 );

	for( loop = 0 ; loop < block_count ; ++loop )
	{

		if( block_size * loop >= flash_data->size )
		{

			printk( "TTUCH : Out of flash buffer\n" );

			create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_OUT_OF_BUFFER, NULL, 0 );

			return;

		}

		if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA, flash_data->buffer + block_size * loop, block_size ) )
		{

			printk( "TTUCH : write register failed\n" );

			create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_I2C_READ, NULL, 0 );

			return;

		}

		if( !flash_command( tool_data, F34_WRITE_CONFIGURATION_BLOCK, WAIT_3000_MS ) )
		{

			printk( "TTUCH : Write flash command failed\n" );

			create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

		if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA + block_size, &flash_status, sizeof( flash_status ) ) )
		{

			printk( "TTUCH : Read register failed\n" );

			create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_I2C_READ, NULL, 0 );

			return;

		}

		if( !FLASH_STATUS_SUCCESS( flash_status ) )
		{

			printk( "TTUCH : Flash failed(%x)\n", FLASH_STATUS_SUCCESS( flash_status ) );

			create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_FLASH_FAILED, NULL, 0 );

			return;

		}

	}

	printk( "TTUCH : Memory[0:%d] >> Flash[0:%d]\n", flash_data->size - 1, flash_data->size - 1 );

	create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_NONE, NULL, 0 );

}

void	upgrade_firmware( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct flash_information	*flash_data = tool_data->flash_data;
	
	struct file_information		*file_info = tool_data->firmware_file;

	struct file			*image_file;

	unsigned	index = parameter->para1.container.number;

	u8	reset_block_count[] = { 0, 0 };

	u8	size[ 2 ], count[ 2 ];

	u8	flash_status;

	loff_t f_ops;

	long	position;

	int	loop0, loop1, read_count;

	unsigned int	block_size, block_count, firmware_size, read_file_count, write_flash_count, read_file_size;

	tool_data->command_id	= COMMAND_UPGRADE_FIRMWARE;

	if( !flash_data || !file_info || !return_buffer )
	{

		printk( "TTUCH : Not found resource\n" );

		return;

	}

	if( !( image_file = file_info->image_file ) )
	{

		printk( "TTUCH : Does not open file\n" );

		create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	set_fs( get_ds() );

	if( !check_image_header( image_file, "Firmware", index ) )
	{

		printk( "TTUCH : Incorrect file header\n" );

		create_return_structure( return_buffer, COMMAND_UPGRADE_CONFIGURATION, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	if( !get_start_file_data_position( image_file, parameter->para1.container.string, &position ) )
	{

		printk( "TTUCH : Can't found %s\n", parameter->para1.container.string );

		create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	{

		struct register_datas_info	datas_info[] =
		{

			{ RMI_F34_QUERY3_BLOCK_SIZE_7_0, size, sizeof( size ) },
			{ RMI_F34_QUERY5_FW_BLOCK_COUNT_7_0, count, sizeof( count ) },

		};

		if( !read_register_datas( tool_data, RMI_FUNCTION_34, REGISTER_QUERY_ID, datas_info, sizeof( datas_info ) / sizeof( *datas_info ) ) )
		{

			printk( "TTUCH : [f34:Query], read failed\n" );

			create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

	}

	CONVERT_ARRAY_TO_INT( block_count, count ), CONVERT_ARRAY_TO_INT( block_size, size );

	printk( "TTUCH : Firmware, block[count:size]=[%d:%d], total size(%d byte)\n", block_count, block_size, ( firmware_size = block_count * block_size ) );

	if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_NUMBER_7_0, reset_block_count, sizeof( reset_block_count ) ) )
	{

		printk( "TTUCH : Reset block count failed\n" );

		create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	f_ops	= image_file->f_op->llseek( image_file, position, 0 );

	write_flash_count = flash_data->size / block_size, read_file_count = firmware_size / flash_data->size + ( firmware_size % flash_data->size ? 1 : 0 ), read_file_size = write_flash_count * block_size;

	printk( "TTUCH : Count[Read file:Write flash]=[%d:%d]\n", read_file_count, write_flash_count );

	for( loop0 = 0 ; loop0 < read_file_count ; ++loop0 )
	{

		if( ( read_count = image_file->f_op->read( image_file, flash_data->buffer, read_file_size, &image_file->f_pos ) ) < 0 )
		{

			printk( "TTUCH : Read file failed\n" );

			create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_FILE_ACCESS_FAILED, NULL, 0 );

			return;

		}

		printk( "TTUCH : %d, Memory[%d] << FW file[%d]\n", loop0, read_file_size, read_file_size );

		for( loop1 = 0 ; loop1 < write_flash_count ; ++loop1 )
		{

			if( block_size * loop1 >= flash_data->size )
			{

				printk( "TTUCH : Out of flash buffer\n" );

				create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_OUT_OF_BUFFER, NULL, 0 );

				return;

			}

			if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA, flash_data->buffer + block_size * loop1, block_size ) )
			{

				printk( "TTUCH : write register failed\n" );

				create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_I2C_READ, NULL, 0 );

				return;

			}

			if( !flash_command( tool_data, F34_WRITE_FIRMWARE_BLOCK, WAIT_3000_MS ) )
			{

				printk( "TTUCH : Write flash command failed\n" );

				create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_RETURN_FAILED, NULL, 0 );

				return;

			}

			if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA + block_size, &flash_status, sizeof( flash_status ) ) )
			{

				printk( "TTUCH : Read register failed\n" );

				create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_I2C_READ, NULL, 0 );

				return;

			}

			if( !FLASH_STATUS_SUCCESS( flash_status ) )
			{

				printk( "TTUCH : Flash failed(%x)\n", FLASH_STATUS_SUCCESS( flash_status ) );

				create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_FLASH_FAILED, NULL, 0 );

				return;

			}

		}

		printk( "TTUCH : %d, Memory[%d] >> Flash[%d]\n", loop0, read_file_size, read_file_size );

	}

	create_return_structure( return_buffer, COMMAND_UPGRADE_FIRMWARE, ERROR_NONE, NULL, 0 );

}

void	create_configuration_file( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data	*tool_data = ( struct touch_tool_data* )node_data;

	struct file_information	*file_info = tool_data->configuration_file;

	struct return_struct	*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	char			*file_name = parameter->para1.container.string;

	struct file			*image_file;

	struct upgrade_file_header	file_header;

	int				read_count;

	tool_data->command_id	= COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION;

	if( file_info )
	{

		printk( "TTUCH : File information is exist\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION, ERROR_RESOURCE_EXIST, NULL, 0 );

		return;

	}

	if( ( file_info = kzalloc( sizeof( struct file_information ), GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Get memory failed\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION, ERROR_ALLOCATE_FAILED, NULL, 0 );

		return;

	}

	strncpy( file_info->name, file_name, strlen( file_name ) + 1 );

	file_info->image_file = image_file = filp_open( file_info->name, O_RDONLY, 0 );

	if( IS_ERR( image_file ) )
	{

		printk( "TTUCH : Open file(%s) failed\n", file_info->name );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION, ERROR_OPEN_FILE_FAILED, NULL, 0 );

		kfree( file_info );

		return;

	}

	if( !image_file->f_op || !image_file->f_op->read || !image_file->f_op->write || !image_file->f_op->llseek )
	{

		printk( "TTUCH : Not found read/write function\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION, ERROR_NOT_FOUND_FUNCTION, NULL, 0 );

		kfree( file_info );

		return;

	}

	set_fs( get_ds() );

	( void )image_file->f_op->llseek( image_file, 0, 0 );

	if( ( read_count = image_file->f_op->read( image_file, ( char* )&file_header, sizeof( struct upgrade_file_header ), &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read file header failed\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION, ERROR_FILE_ACCESS_FAILED, NULL, 0 );

		kfree( file_info );

		return;

	}

	if( read_count != sizeof( struct upgrade_file_header ) )
	{

		printk( "TTUCH : Incorrect header size\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION, ERROR_VALUE_INCORRECT, NULL, 0 );

		kfree( file_info );

		return;

	}

	if( strcmp( file_header.fih_string, "FIH" ) || strcmp( file_header.check_string, "Configuration" ) )
	{

		printk( "TTUCH : Incorrect header size\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION, ERROR_INCORRECT_INFORMATION, NULL, 0 );

		kfree( file_info );

		return;

	}

	tool_data->configuration_file	= file_info;

	printk( "TTUCH : Open configuration file(%s) success\n", file_info->name );

	create_return_structure( return_buffer, COMMAND_ALLOCATE_CONFIG_FILE_INFORMATION, ERROR_NONE, &file_info->name, 1 );

}

void	create_firmware_file( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data	*tool_data = ( struct touch_tool_data* )node_data;

	struct file_information	*file_info = tool_data->firmware_file;

	struct return_struct	*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	char			*file_name = parameter->para1.container.string;

	struct file			*image_file;

	struct upgrade_file_header	file_header;

	int	read_count;

	tool_data->command_id	= COMMAND_ALLOCATE_FW_FILE_INFORMATION;

	if( file_info )
	{

		printk( "TTUCH : File information is exist\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FW_FILE_INFORMATION, ERROR_RESOURCE_EXIST, NULL, 0 );

		return;

	}

	if( ( file_info = kzalloc( sizeof( struct file_information ), GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Get memory failed\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FW_FILE_INFORMATION, ERROR_ALLOCATE_FAILED, NULL, 0 );

		return;

	}

	strncpy( file_info->name, file_name, strlen( file_name ) + 1 );

	file_info->image_file = image_file = filp_open( file_info->name, O_RDONLY, 0 );

	if( IS_ERR( image_file ) )
	{

		printk( "TTUCH : Open file(%s) failed\n", file_info->name );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FW_FILE_INFORMATION, ERROR_OPEN_FILE_FAILED, NULL, 0 );

		kfree( file_info );

		return;

	}

	if( !image_file->f_op || !image_file->f_op->read || !image_file->f_op->write || !image_file->f_op->llseek )
	{

		printk( "TTUCH : Not found read/write function\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FW_FILE_INFORMATION, ERROR_NOT_FOUND_FUNCTION, NULL, 0 );

		kfree( file_info );

		return;

	}

	set_fs( get_ds() );

	( void )image_file->f_op->llseek( image_file, 0, 0 );

	if( ( read_count = image_file->f_op->read( image_file, ( char* )&file_header, sizeof( struct upgrade_file_header ), &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read file header failed\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FW_FILE_INFORMATION, ERROR_FILE_ACCESS_FAILED, NULL, 0 );

		kfree( file_info );

		return;

	}

	if( read_count != sizeof( struct upgrade_file_header ) )
	{

		printk( "TTUCH : Incorrect header size\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FW_FILE_INFORMATION, ERROR_VALUE_INCORRECT, NULL, 0 );

		kfree( file_info );

		return;

	}

	if( strcmp( file_header.fih_string, "FIH" ) || strcmp( file_header.check_string, "Firmware" ) )
	{

		printk( "TTUCH : Incorrect header size\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FW_FILE_INFORMATION, ERROR_INCORRECT_INFORMATION, NULL, 0 );

		kfree( file_info );

		return;

	}

	tool_data->firmware_file	= file_info;

	printk( "TTUCH : Open firmware file(%s) success\n", file_info->name );

	create_return_structure( return_buffer, COMMAND_ALLOCATE_FW_FILE_INFORMATION, ERROR_NONE, &file_info->name, 1 );

}

void	release_configuration_file( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data	*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct	*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct file_information	*file_info = tool_data->configuration_file;

	tool_data->command_id	= COMMAND_RELEASE_CONFIG_FILE_INFORMATION;

	if( !file_info )
	{

		printk( "TTUCH : File information is non-exist\n" );

		create_return_structure( return_buffer, COMMAND_RELEASE_CONFIG_FILE_INFORMATION, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	if( filp_close( file_info->image_file, NULL ) )
	{

		printk( "TTUCH : Close file(%s) failed\n", file_info->name );

		create_return_structure( return_buffer, COMMAND_RELEASE_CONFIG_FILE_INFORMATION, ERROR_CLOSE_FILE_FAILED, NULL, 0 );

	}

	kfree( file_info );

	tool_data->configuration_file	= NULL;

	create_return_structure( return_buffer, COMMAND_RELEASE_CONFIG_FILE_INFORMATION, ERROR_NONE, NULL, 0 );

}

void	release_firmware_file( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data	*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct	*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	struct file_information	*file_info = tool_data->firmware_file;

	tool_data->command_id	= COMMAND_RELEASE_FW_FILE_INFORMATION;

	if( !file_info )
	{

		printk( "TTUCH : File information is non-exist\n" );

		create_return_structure( return_buffer, COMMAND_RELEASE_FW_FILE_INFORMATION, ERROR_RESOURCE_NOT_EXIST, NULL, 0 );

		return;

	}

	if( filp_close( file_info->image_file, NULL ) )
	{

		printk( "TTUCH : Close file(%s) failed\n", file_info->name );

		create_return_structure( return_buffer, COMMAND_RELEASE_FW_FILE_INFORMATION, ERROR_CLOSE_FILE_FAILED, NULL, 0 );

	}

	kfree( file_info );

	tool_data->firmware_file	= NULL;

	create_return_structure( return_buffer, COMMAND_RELEASE_FW_FILE_INFORMATION, ERROR_NONE, NULL, 0 );

}

void	dump_image_information( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct upgrade_data_information	data_information;

	struct file			*image_file;

	struct upgrade_file_header	file_header;

	unsigned	loop, count;

	int		read_count;

	char		*image = parameter->para1.container.string;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_DUMP_IMAGE_INFORMATION );

	tool_data->command_id	= COMMAND_DUMP_IMAGE_INFORMATION;

	image_file	= filp_open( image, O_RDONLY, 0 );

	if( IS_ERR( image_file ) )
	{

		printk( "TTUCH : Open file(%s) failed\n", image );

		info_print( INFO_BUFFER_ADD, "Open file(%s) failed\n", image );

		return;

	}

	if( !image_file->f_op || !image_file->f_op->read || !image_file->f_op->write || !image_file->f_op->llseek )
	{

		printk( "TTUCH : Not found read/write function\n" );

		info_print( INFO_BUFFER_ADD, "Not found read/write function\n" );

		return;

	}

	set_fs( get_ds() );

	( void )image_file->f_op->llseek( image_file, 0, 0 );

	if( ( read_count = image_file->f_op->read( image_file, ( char* )&file_header, sizeof( struct upgrade_file_header ), &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read file header failed\n" );

		info_print( INFO_BUFFER_ADD, "Read file header failed\n" );

		return;

	}

	if( read_count != sizeof( struct upgrade_file_header ) )
	{

		printk( "TTUCH : Incorrect header size\n" );

		info_print( INFO_BUFFER_ADD, "Incorrect header size\n" );

		return;

	}

	printk( "TTUCH : Image(%s), [Mark:ID:Count]=[%s:%s:%d]\n\n", image, file_header.fih_string, file_header.check_string, file_header.count );

	info_print( INFO_BUFFER_ADD, "Image(%s), [Mark:ID:Count]=[%s:%s:%d]\n\n", image, file_header.fih_string, file_header.check_string, file_header.count );

	count	= file_header.count;

	for( loop = 0 ; loop < count ; ++loop  )
	{

		if( ( read_count = image_file->f_op->read( image_file, ( char* )&data_information, sizeof( struct upgrade_data_information ), &image_file->f_pos ) ) < 0 )
		{

			printk( "TTUCH : Read data information failed\n" );

			info_print( INFO_BUFFER_ADD, "Read data information failed\n" );

			return;

		}

		if( read_count != sizeof( struct upgrade_file_header ) )
		{

			printk( "TTUCH : Incorrect header size\n" );

			info_print( INFO_BUFFER_ADD, "Incorrect header size\n" );

			return;

		}

		printk( "TTUCH : Block%d, [ID:Convert:Start:Size]=[%s:%d:%ld:%ld]\n", loop, data_information.version_string, data_information.convert, data_information.start, data_information.size );

		info_print( INFO_BUFFER_ADD, "Block%d, [ID:Convert:Start:Size]=[%s:%d:%ld:%ld]\n", loop, data_information.version_string, data_information.convert, data_information.start, data_information.size );

	}

}

void	show_firmware_version( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	u8	*firmware_version = tool_data->parameter_buffer + sizeof( struct return_struct );

	char	acBuffer[ THREE_REGISTER ];

	unsigned	number, count;

	tool_data->command_id	= COMMAND_FIRMWARE_VERSION;

	if( !read_register_data( tool_data, RMI_FUNCTION_01, REGISTER_QUERY_ID, RMI_F01_QUERY18, acBuffer, THREE_REGISTER ) )
	{

		printk( "TTUCH : Read register failed\n" );

		create_return_structure( return_buffer, COMMAND_FIRMWARE_VERSION, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	CONVERT_ARRAY_3_TO_INT( number, acBuffer );

	count	= snprintf( firmware_version, PARA_BUFFER_SIZE - sizeof( struct return_struct ), "%d", number );

	printk( "TTUCH : Firmware version(%s)\n", firmware_version );

	create_return_structure( return_buffer, COMMAND_FIRMWARE_VERSION, ERROR_NONE, NULL, count );

}

void	dump_history_information( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct file			*image_file;

	struct history_header		file_header;

	unsigned	loop;

	int		read_count;

	char		*image = parameter->para1.container.string;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_DUMP_HISTORY_INFORMATION );

	tool_data->command_id	= COMMAND_DUMP_HISTORY_INFORMATION;

	image_file	= filp_open( image, O_RDONLY, 0 );

	if( IS_ERR( image_file ) )
	{

		printk( "TTUCH : Open file(%s) failed\n", image );

		info_print( INFO_BUFFER_ADD, "Open file(%s) failed\n", image );

		return;

	}

	if( !image_file->f_op || !image_file->f_op->read || !image_file->f_op->write || !image_file->f_op->llseek )
	{

		printk( "TTUCH : Not found read/write function\n" );

		info_print( INFO_BUFFER_ADD, "Not found read/write function\n" );

		return;

	}

	set_fs( get_ds() );

	( void )image_file->f_op->llseek( image_file, 0, 0 );

	if( ( read_count = image_file->f_op->read( image_file, ( char* )&file_header, sizeof( struct history_header ), &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read file header failed\n" );

		info_print( INFO_BUFFER_ADD, "Read file header failed\n" );

		return;

	}

	if( read_count != sizeof( struct history_header ) )
	{

		printk( "TTUCH : Incorrect header size\n" );

		info_print( INFO_BUFFER_ADD, "Incorrect header size\n" );

		return;

	}

	printk( "TTUCH : History File(%s), Check(%s)\n\n", image, file_header.check );

	info_print( INFO_BUFFER_ADD, "History File(%s), Check(%s)\n\n", image, file_header.check );

	printk( "TTUCH : Image information, Firmware image[Use index:Count]=[%d:%d]\n", file_header.firmware.use_image_index, file_header.firmware.image_count );

	info_print( INFO_BUFFER_ADD, "Image information, Firmware image[Use index:Count]=[%d:%d]\n", file_header.firmware.use_image_index, file_header.firmware.image_count );

	printk( "TTUCH : Image information, Configuration image[Use index:Count]=[%d:%d]\n", file_header.configuration.use_image_index, file_header.configuration.image_count );

	info_print( INFO_BUFFER_ADD, "Image information, Configuration image[Use index:Count]=[%d:%d]\n", file_header.configuration.use_image_index, file_header.configuration.image_count );

	printk( "TTUCH : Image history, Last firmware version[Use index:state:version]=[%d:%d:%s]\n", file_header.last_firmware_version.image_index, file_header.last_firmware_version.state, file_header.last_firmware_version.version );

	info_print( INFO_BUFFER_ADD, "Image history, Last firmware version[Use index:state:version]=[%d:%d:%s]\n", file_header.last_firmware_version.image_index, file_header.last_firmware_version.state, file_header.last_firmware_version.version );

	printk( "TTUCH : Image history, Last configuration version[Use index:state:version]=[%d:%d:%s]\n", file_header.last_configuration_version.image_index, file_header.last_configuration_version.state, file_header.last_configuration_version.version );

	info_print( INFO_BUFFER_ADD, "Image history, Last configuration version[Use index:state:version]=[%d:%d:%s]\n", file_header.last_configuration_version.image_index, file_header.last_configuration_version.state, file_header.last_configuration_version.version );

	printk( "TTUCH : [Flag:History count:Clear-ID]=[0x%x:%d:0x%x]\n", file_header.flag, file_header.image_history_count, file_header.clear_id );

	info_print( INFO_BUFFER_ADD, "[Flag:History count:Clear-ID]=[0x%x:%d:0x%x]\n", file_header.flag, file_header.image_history_count, file_header.clear_id );

	for( loop = 0 ; loop < sizeof( file_header.firmware_block_count ) / sizeof( *file_header.firmware_block_count ) ; ++loop )
	{

		unsigned short	block_count = *( file_header.firmware_block_count + loop );

		if( block_count )
		{

			printk( "TTUCH : Block count of Firmware image[Image:Count]=[%d:%d]\n", loop, block_count );

			info_print( INFO_BUFFER_ADD, "Block count of Firmware image[Image:Count]=[%d:%d]\n", loop, block_count );

		}

	}

	for( loop = 0 ; loop < sizeof( file_header.configuration_block_count ) / sizeof( *file_header.configuration_block_count ) ; ++loop )
	{

		unsigned short	block_count = *( file_header.configuration_block_count + loop );

		if( block_count )
		{

			printk( "TTUCH : Block count of Configuration image[Image:Count]=[%d:%d]\n", loop, block_count );

			info_print( INFO_BUFFER_ADD, "Block count of Configuration image[Image:Count]=[%d:%d]\n", loop, block_count );

		}

	}

}

void	show_product_id( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	u8	*product_id = tool_data->parameter_buffer + sizeof( struct return_struct );

	char	acBuffer[ TEN_REGISTER + 1 ] = { 0 };

	tool_data->command_id	= COMMAND_PRODUCT_ID;

	if( !read_register_data( tool_data, RMI_FUNCTION_01, REGISTER_QUERY_ID, RMI_F01_QUERY11, acBuffer, TEN_REGISTER ) )
	{

		printk( "TTUCH : Read register failed\n" );

		create_return_structure( return_buffer, COMMAND_PRODUCT_ID, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	printk( "TTUCH : Product ID(%s)\n", acBuffer );

	memcpy( product_id, acBuffer, sizeof( acBuffer ) );

	create_return_structure( return_buffer, COMMAND_PRODUCT_ID, ERROR_NONE, NULL, TEN_REGISTER );

}

void	dump_history_version_block( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct file			*image_file;

	struct history_header		file_header;

	struct history_body		*file_body;

	unsigned	loop;

	int		read_count;

	char		*image = parameter->para1.container.string;

	int		show_count = parameter->para2.container.number;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_DUMP_HISTORY_INFORMATION );

	tool_data->command_id	= COMMAND_DUMP_HISTORY_INFORMATION;

	if( show_count >= HISTORY_COUNT )
	{

		printk( "TTUCH : Invalid show count(%d)\n", show_count );

		info_print( INFO_BUFFER_ADD, "Invalid show count(%d)\n", show_count );

		return;

	}

	image_file	= filp_open( image, O_RDONLY, 0 );

	if( IS_ERR( image_file ) )
	{

		printk( "TTUCH : Open file(%s) failed\n", image );

		info_print( INFO_BUFFER_ADD, "Open file(%s) failed\n", image );

		return;

	}

	if( !image_file->f_op || !image_file->f_op->read || !image_file->f_op->write || !image_file->f_op->llseek )
	{

		printk( "TTUCH : Not found read/write function\n" );

		info_print( INFO_BUFFER_ADD, "Not found read/write function\n" );

		filp_close( image_file, NULL );

		return;

	}

	set_fs( get_ds() );

	( void )image_file->f_op->llseek( image_file, 0, 0 );

	if( ( read_count = image_file->f_op->read( image_file, ( char* )&file_header, sizeof( struct history_header ), &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read file header failed\n" );

		info_print( INFO_BUFFER_ADD, "Read file header failed\n" );

		filp_close( image_file, NULL );

		return;

	}

	if( read_count != sizeof( struct history_header ) )
	{

		printk( "TTUCH : Incorrect header size\n" );

		info_print( INFO_BUFFER_ADD, "Incorrect header size\n" );

		filp_close( image_file, NULL );

		return;

	}

	if( ( file_body = kzalloc( sizeof( struct history_body ), GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Allocate memory failed\n" );

		info_print( INFO_BUFFER_ADD, "Allocate memory failed\n" );

		filp_close( image_file, NULL );

		return;

	}

	if( image_file->f_op->read( image_file, ( char* )file_body->image_history_datas, sizeof( struct history_body ), &image_file->f_pos ) < 0 )
	{

		printk( "TTUCH : Read file header failed\n" );

		info_print( INFO_BUFFER_ADD, "Read file header failed\n" );

	}

	{

		unsigned	image_history_count = file_header.image_history_count;

		unsigned	start_index = image_history_count >= show_count ? image_history_count - show_count : 0;

		unsigned	dump_count = image_history_count >= show_count ? show_count : image_history_count;

		printk( "TTUCH : History File(%s), Version block[Count:Show]=[%d:%d]\n\n", image, image_history_count, show_count );

		info_print( INFO_BUFFER_ADD, "History File(%s), Version block[Count:Show]=[%d:%d]\n", image, image_history_count, show_count );

		printk( "TTUCH : [History index:Show index][Index:State:Version]\n" );

		info_print( INFO_BUFFER_ADD, "[History index:Show index][Index:State:Version]\n\n" );

		for( loop = 0 ; loop < dump_count ; ++loop )
		{

			struct image_history	*image_history_data = ( file_body->image_history_datas + ( start_index + loop ) % HISTORY_COUNT );

			printk( "TTUCH : [%d:%d][%x:%x:%s]\n", start_index + loop, loop, image_history_data->image_index, image_history_data->state, image_history_data->version );

			info_print( INFO_BUFFER_ADD, "[%d:%d][%x:%x:%s]\n", start_index + loop, loop, image_history_data->image_index, image_history_data->state, image_history_data->version );

		}

	}

	kfree( file_body );

	filp_close( image_file, NULL );

}

void	show_sensor_id( void *node_data, struct command_parameter *parameter )
{

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	u8	*sensor_id = tool_data->parameter_buffer + sizeof( struct return_struct );

	char	acBuffer[ SIX_REGISTER ];

	char	gpio_setting[] = { 0x85, 0, 0x85, 0 };

	int	timeout = ( ( WAIT_1000_MS * 1000 ) / MAX_SLEEP_TIME_US ) + 1, count = 0;

	int	block_size;

	bool	ready = false;

	tool_data->command_id	= COMMAND_SENSOR_ID;

	if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_QUERY_ID, RMI_F34_QUERY3_BLOCK_SIZE_7_0, acBuffer, 2 ) )
	{

		printk( "TTUCH : Read register failed\n" );

		create_return_structure( return_buffer, COMMAND_SENSOR_ID, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	block_size	= ( *( acBuffer + 1 ) << 8 ) + *acBuffer;

	if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA2, gpio_setting, sizeof( gpio_setting ) ) )
	{

		printk( "TTUCH : Write register failed\n" );

		create_return_structure( return_buffer, COMMAND_SENSOR_ID, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	*acBuffer	= F34_READ_SENSOR_ID;

	if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_FLASH_COMMAND + block_size, acBuffer, 1 ) )
	{

		printk( "TTUCH : Write register failed\n" );

		create_return_structure( return_buffer, COMMAND_SENSOR_ID, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	do
	{

		if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_FLASH_COMMAND, acBuffer, 1 ) )
		{

			printk( "TTUCH : Read register failed\n" );

			create_return_structure( return_buffer, COMMAND_SENSOR_ID, ERROR_RETURN_FAILED, NULL, 0 );

			return;

		}

		count++;

		usleep_range( MIN_SLEEP_TIME_US, MAX_SLEEP_TIME_US );

		if( !FLASH_STATUS( *acBuffer ) && !FLASH_PROGRAM( *acBuffer ) )
		{

			ready	= true;

			break;

		}

	}
	while( count < timeout );

	if( !ready )
	{

		printk( "TTUCH : Timeout!!! Flash status(%d), Program enabled(%d), Flash program(%d)\n", FLASH_STATUS( *acBuffer ), PROGRAM_ENABLED( *acBuffer ), FLASH_PROGRAM( *acBuffer ) );

		create_return_structure( return_buffer, COMMAND_SENSOR_ID, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA2, acBuffer, SIX_REGISTER ) )
	{

		printk( "TTUCH : Read register failed\n" );

		create_return_structure( return_buffer, COMMAND_SENSOR_ID, ERROR_RETURN_FAILED, NULL, 0 );

		return;

	}

	*sensor_id = !( !( *( acBuffer + 4 ) & 0x80 ) ), *( sensor_id + 1 ) = !( !( *( acBuffer + 4 ) & 4 ) ), *( sensor_id + 2 ) = *( acBuffer + 4 ) & 1;

	printk( "TTUCH : Sensor ID, [MSB:LSB]=[%x:%x], [ID0:ID1:ID2]=[%d:%d:%d]\n", *( acBuffer + 5 ), *( acBuffer + 4 ), *sensor_id, *( sensor_id + 1 ), *( sensor_id + 2 ) );

	create_return_structure( return_buffer, COMMAND_SENSOR_ID, ERROR_NONE, NULL, THREE_REGISTER );

}

// ==========================================================================================================================

struct load_command_data	command_list[] = { COMMAND_LIST };

struct load_command_data* get_load_command_pointer( void )
{

	return	command_list;

}

unsigned int get_load_command_counter( void )
{

	return	sizeof( command_list ) / sizeof( *command_list );

}

// ==========================================================================================================================
