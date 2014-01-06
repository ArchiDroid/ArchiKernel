
#include <linux/input/FIH-tool/FIH-touch-tool.h>

#include <linux/input/FIH-tool/touch-tool_functions.h>

#include <linux/delay.h>

// ==========================================================================================================================

void	no_print( int command, char *string, ... )
{
}

void	log_print( int command, char *string, ... )
{

	char	buffer[ 256 ];

	va_list	vaList;

	if( command == INFO_BUFFER_RESET )
		return;

	va_start( vaList, string );

	vsnprintf( buffer, sizeof( buffer ), string, vaList );

	va_end( vaList );

	printk( "TTUCH : %s", buffer );

}

void	info_print( int command, char *string, ... )
{

	static char		*buffer = 0;

	static unsigned int	pointer = 0;

	static size_t		size = 0;

	va_list	vaList;

	va_start( vaList, string );

	switch( command )
	{

		case	INFO_BUFFER_RESET :

			pointer = 0;

			break;

		case	INFO_BUFFER_INIT :

			buffer	= va_arg( vaList, char* ), size = va_arg( vaList, unsigned int );

			break;

		case	INFO_BUFFER_ADD :

			if( !buffer || !size )
			{

				printk( "TTUCH : info_print() needs initialization\n" );

				break;

			}

			if( pointer >= size )
			{

				printk( "TTUCH : Out of range, range[Max:Using]=[%d:%d]\n", size, pointer );

				break;

			}

			{

				int	count = vsnprintf( buffer + pointer, size - pointer, string, vaList );

				if( count < 0 )
					printk( "TTUCH : Info buffer is full\n" );
				else
					pointer += count;

			}

			break;

		default :
			break;

	}

	va_end( vaList );

}

// ==========================================================================================================================

struct rmi_function_descriptor*	get_function_descriptor( struct touch_tool_data *tool_data, int function )
{

	struct rmi_function_information	*rmi_fxx_information = NULL;

	list_for_each_entry( rmi_fxx_information, &tool_data->descriptor_list, link )
		if( rmi_fxx_information && rmi_fxx_information->descriptor.function_number == function )
			return	&rmi_fxx_information->descriptor;

	return	NULL;

}

u16	get_register_addr( struct touch_tool_data *tool_data, int function, int register_id )
{

	struct rmi_function_descriptor	*descriptor = get_function_descriptor( tool_data, function );

	u16	return_addr = INVALID_ADDRESS;

	if( !descriptor )
		printk( "TTUCH : F%02x Not found\n", function );
	else
	{

		u16	address[] =
		{

			descriptor->query_base_addr,
			descriptor->command_base_addr,
			descriptor->control_base_addr,
			descriptor->data_base_addr

		};

		if( register_id < sizeof( address ) / sizeof( *address ) )
			return_addr	= *( address + register_id );
		else
			printk( "TTUCH : Register ID(%d) Not found\n", register_id );

	}

	return	return_addr;

}

char*	get_register_string( int register_id )
{

	static char	*string[] = { "QUERY", "CMD", "CTRL", "DATA" };

	if( register_id >= sizeof( string ) / sizeof( *string ) )
		return	NULL;

	return	*( string + register_id );

}

// ==========================================================================================================================

void	creat_block_data_number_string( u8 *string, u8 *block_data, unsigned int count )
{

	unsigned int	create_count = count;

	int	string_pointer = 0, block_data_pointer = 0; 

	int	loop;

	if( create_count > MAX_BLOCK_DATA )
		create_count	= MAX_BLOCK_DATA;

	for( loop = 0 ; loop < create_count ; ++loop, ++block_data_pointer )
		string_pointer += snprintf( string + string_pointer, BLOCK_DATA_BUFFER_SIZE - string_pointer, " %02x", *( block_data + block_data_pointer ) );

}

void	creat_block_data_character_string( u8 *string, u8 *block_data, unsigned int count )
{

	unsigned int	create_count = count;

	int	string_pointer = 0, block_data_pointer = 0; 

	int	loop;

	if( create_count > MAX_BLOCK_DATA )
		create_count	= MAX_BLOCK_DATA;

	for( loop = 0 ; loop < create_count ; ++loop, ++block_data_pointer )
		string_pointer += snprintf( string + string_pointer, BLOCK_DATA_BUFFER_SIZE - string_pointer, " %c", IS_ASCII_CODE( *( block_data + block_data_pointer ) ) );

}

void	create_return_structure( struct return_struct *return_buffer, int command, int error, void *data, int count )
{

	struct	action_information
	{

		int			id;

		enum select_action	action;

	};

	struct action_information	action_list[] = { RETRUN_STRUCT_LIST };

	int	loop;

	return_buffer->error_code = error, return_buffer->command = command, strncpy( return_buffer->string, RETURN_VALUE_STRING, sizeof( return_buffer->string ) );

	if( error != ERROR_NONE )
		return;

	for( loop = 0 ; loop < sizeof( action_list ) / sizeof( *action_list ) ; ++loop )
	{

		if( ( action_list + loop )->id != command )
			continue;

		switch( ( action_list + loop )->action )
		{

			case	STORE_BLOCK_DATA :

				return_buffer->values.block_data_count	= count;

				break;

			case	STORE_SIZE :

				return_buffer->values.size	= *( unsigned int* )data;

				break;

			case	STORE_NAME :

				return_buffer->values.length	= *( unsigned* )data;

				break;

			case	STORE_VALUE :

				return_buffer->values.value	= *( u8* )data;

				break;

			default :
				break;

		}

	}

}

void	create_return_string( u8 *string, struct return_struct *return_buffer )
{

	struct	action_information
	{

		int			id;

		enum select_action	action;

	};

	struct action_information	action_list[] = { RETRUN_STRING_LIST };

	int	loop;

	info_print( INFO_BUFFER_RESET, NULL );

	if( strcmp( return_buffer->string, RETURN_VALUE_STRING ) )
	{

		info_print( INFO_BUFFER_ADD, "This is invalid format\n" );

		return;

	}
 
	info_print( INFO_BUFFER_ADD, "[Command:Error]=[%d:%d]\n", return_buffer->command, return_buffer->error_code );

	if( return_buffer->error_code != ERROR_NONE )
		return;

	for( loop = 0 ; loop < sizeof( action_list ) / sizeof( *action_list ) ; ++loop )
	{

		if( ( action_list + loop )->id != return_buffer->command )
			continue;

		switch( ( action_list + loop )->action )
		{

			case	SHOW_VALUE :

				if( return_buffer->error_code != ERROR_NONE )
					break;

				info_print( INFO_BUFFER_ADD, "Value=%x\n", return_buffer->values.value );

				break;

			case	SHOW_BLOCK_DATA :

				if( return_buffer->error_code != ERROR_NONE )
				{

					info_print( INFO_BUFFER_ADD, "Buffer is full(%d)\n", return_buffer->values.block_data_count );

					break;

				}

				{

					int	loop, loop1;

					int	row = return_buffer->values.block_data_count / 16 ? return_buffer->values.block_data_count / 16 : 1 ;

					u8	*values = ( u8* )return_buffer + sizeof( struct return_struct );

					info_print( INFO_BUFFER_ADD, "Value count(%d), vlaue=>\n", return_buffer->values.block_data_count );

					for( loop = 0 ; loop < row ; ++loop )
					{

						int	remainder = return_buffer->values.block_data_count - loop * 16;

						int	value_count = remainder >= 16 ? 16 : remainder;

						for( loop1 = 0 ; loop1 < value_count ; ++loop1 )
							info_print( INFO_BUFFER_ADD, "0x%02x ", *( values + loop * 16 + loop1 ) );

						info_print( INFO_BUFFER_ADD, "\n" );
					}

					break;

				}

				break;

			case	SHOW_SIZE :

				info_print( INFO_BUFFER_ADD, "Memory size is %d byte\n", return_buffer->values.size );

				break;

			case	SHOW_FILE_NAME :

				info_print( INFO_BUFFER_ADD, "File name(%s)\n", ( u8* )return_buffer + sizeof( struct return_struct ) );

				break;

			case	SHOW_STRING :

				info_print( INFO_BUFFER_ADD, "%s\n", ( u8* )return_buffer + sizeof( struct return_struct ) );

				break;

			case	SHOW_MODE :

				info_print( INFO_BUFFER_ADD, "In %s mode\n", return_buffer->values.value == UI_MODE ? "UI" : "Bootloader" );

				break;

			case	SHOW_CONFIG :
			{

				u8	*configuration_id = ( u8* )return_buffer + sizeof( struct return_struct );

				info_print( INFO_BUFFER_ADD, "Configuration ID(%c%c%c%c)\n", *configuration_id, *( configuration_id + 1 ), *( configuration_id + 2 ), *( configuration_id + 3 ) );

				break;

			}

			case	SHOW_FIRMWARE :
			{

				u8	*firmware_version = ( u8* )return_buffer + sizeof( struct return_struct );

				info_print( INFO_BUFFER_ADD, "Firmware version(%s)\n", firmware_version );

				break;

			}

			case	SHOW_PRODUCT :
			{

				u8	*product_id = ( u8* )return_buffer + sizeof( struct return_struct );

				info_print( INFO_BUFFER_ADD, "Product ID(%s)\n", product_id );

				break;

			}

			case	SHOW_SENSOR_ID :
			{

				u8	*sensor_id = ( u8* )return_buffer + sizeof( struct return_struct );

				info_print( INFO_BUFFER_ADD, "Sensor ID, [ID0:ID1:ID2]=[%d:%d:%d]\n", *sensor_id, *( sensor_id + 1 ), *( sensor_id + 2 ) );

				if( *( sensor_id + 1 ) && *( sensor_id + 2 ) )
					info_print( INFO_BUFFER_ADD, "Color(%s), GIS sensor\n", *sensor_id ? "white" : "black" );

				if( !*( sensor_id + 1 ) && *( sensor_id + 2 ) )
					info_print( INFO_BUFFER_ADD, "Color(%s), Mogen sensor\n", *sensor_id ? "white" : "black" );

				break;

			}

			default :
				break;

		}

	}

}

// ==========================================================================================================================

void	dump_memory_value( struct touch_tool_data *tool_data, char *buffer, unsigned int start, unsigned int end )
{

	u8		*block_data = tool_data->block_data_buffer;

	unsigned int	count = end - start + 1;

	int		loop, index;

	info_print( INFO_BUFFER_ADD, "%s\n", BUFFER_NUMBER_STRING );

	for( loop = count, index = 0 ; loop != 0 ; loop -= loop / MAX_BLOCK_DATA ? MAX_BLOCK_DATA : loop, index += MAX_BLOCK_DATA )
	{

		creat_block_data_number_string( block_data, buffer + start + index, loop );

		info_print( INFO_BUFFER_ADD, "%04d |%s\n", start + index, block_data );

	}

}

void	dump_memory_character( struct touch_tool_data *tool_data, char *buffer, unsigned int start, unsigned int end )
{

	u8		*block_data = tool_data->block_data_buffer;

	unsigned int	count = end - start + 1;

	int		loop, index;

	info_print( INFO_BUFFER_ADD, "%s\n", BUFFER_CHARACTER_STRING );

	for( loop = count, index = 0 ; loop != 0 ; loop -= loop / MAX_BLOCK_DATA ? MAX_BLOCK_DATA : loop, index += MAX_BLOCK_DATA )
	{

		creat_block_data_character_string( block_data, buffer + start + index, loop );

		info_print( INFO_BUFFER_ADD, "%04d |%s\n", start + index, block_data );

	}

}

// ==========================================================================================================================

void	get_page_description( struct touch_tool_data *tool_data )
{

	struct pdt_data
	{

		u8	query_base_addr : 8;

		u8	command_base_addr : 8;

		u8	control_base_addr : 8;

		u8	data_base_addr : 8;

		u8	interrupt_source_count : 3;

		u8	bits3and4 : 2;

		u8	function_version : 2;

		u8	bit7 : 1;

		u8	function_number : 8;

	};

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	struct rmi_function_information	*rmi_fxx_information;

	struct list_head	*pointer, *next;

	struct pdt_data		data;

	unsigned int		loop, loop1;

	if( !rmi_data )
	{

		printk( "TTUCH : The pointer of RMI data is NULL\n" );

		return;

	}

	list_for_each_safe( pointer, next, &tool_data->descriptor_list )
	{

		rmi_fxx_information	= list_entry( pointer, struct rmi_function_information, link );

		if( rmi_fxx_information )
		{

			printk( "TTUCH : f%02x, free memory\n", rmi_fxx_information->descriptor.function_number );

			list_del( pointer );

			kfree( rmi_fxx_information );

		}

	}

	for( loop = 0 ; loop < RMI4_MAX_PAGE ; ++loop )
	{

		int	page_start = RMI4_PAGE_SIZE * loop;

		int	start = page_start + RMI4_PDT_START, end = page_start + RMI4_PDT_END;

		for( loop1 = start ; loop1 >= end ; loop1 -= sizeof( struct pdt_data ) )
		{

			struct rmi_function_descriptor	*descriptor;

			if( rmi_read_block( rmi_data, loop1, ( u8* )&data, sizeof( struct pdt_data ) ) != sizeof( struct pdt_data ) )
			{

				printk( "TTUCH : RMI block read failed\n" );

				return;

			}

			if( !data.function_number || data.function_number == 0xff )
				break;

			if( get_function_descriptor( tool_data, data.function_number ) )
			{

				printk( "TTUCH : f%02x is exist\n", data.function_number );

				return;

			}

			if( ( rmi_fxx_information = kzalloc( sizeof( struct rmi_function_information ), GFP_KERNEL ) ) == NULL )
			{

				printk( "TTUCH : Memory allocation fail\n" );

				return;

			}

			descriptor	= &rmi_fxx_information->descriptor;

			descriptor->query_base_addr = data.query_base_addr + page_start, descriptor->command_base_addr = data.command_base_addr + page_start, descriptor->control_base_addr = data.control_base_addr + page_start, descriptor->data_base_addr = data.data_base_addr + page_start;

			descriptor->interrupt_source_count = data.interrupt_source_count, descriptor->function_number = data.function_number, descriptor->function_version = data.function_version;

			list_add_tail( &rmi_fxx_information->link, &tool_data->descriptor_list );

			printk( "TTUCH : f%02x, [Query:CMD:CTRL:DATA]=[0x%04x:0x%04x:0x%04x:0x%04x]\n", descriptor->function_number, descriptor->query_base_addr, descriptor->command_base_addr, descriptor->control_base_addr, descriptor->data_base_addr );

		}

	}

}

void	get_touch_ic_info( struct touch_tool_data *tool_data, void ( *show )( int, char*, ... ) )
{

	enum	value_count
	{

		GET_1_VALUE = 0,
		GET_2_VALUE,
		GET_3_VALUE,
		GET_4_VALUE,
		GET_5_VALUE,
		GET_6_VALUE,
		GET_7_VALUE,
		GET_8_VALUE,
		GET_9_VALUE,
		GET_10_VALUE,
		MAX_GET_COUNT,
	};

	struct	register_information
	{

		int	function, register_id, start_index, end_index;

		int	direction;

		char	*string;

	};

	struct register_information	register_info[] = { SHOW_ALL_OF_REGISTER };

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	struct rmi_function_information	*rmi_fxx_information;

	u8	value_buffer[ MAX_GET_COUNT ];

	int	loop, error;

	show( INFO_BUFFER_RESET, NULL );

	list_for_each_entry( rmi_fxx_information, &tool_data->descriptor_list, link )
		if( rmi_fxx_information )
		{

			struct rmi_function_descriptor	*descriptor = &rmi_fxx_information->descriptor;

			show( INFO_BUFFER_ADD, "f%02x, [Query:CMD:CTRL:DATA]=[0x%04x:0x%04x:0x%04x:0x%04x]\n", descriptor->function_number, descriptor->query_base_addr, descriptor->command_base_addr, descriptor->control_base_addr, descriptor->data_base_addr );

		}

	for( loop = 0 ; loop < sizeof( register_info ) / sizeof( *register_info ) ; ++loop )
	{

		struct register_information	*info_pointer = register_info + loop;

		int	register_count = info_pointer->end_index - info_pointer->start_index + 1;

		u16	address = get_register_addr( tool_data, info_pointer->function, info_pointer->register_id );

		if( address == INVALID_ADDRESS )
			continue;

		if( register_count == ONE_REGISTER )
			error	= rmi_read( rmi_data, address + info_pointer->start_index, value_buffer );
		else
		{

			error	= rmi_read_block( rmi_data, address + info_pointer->start_index, value_buffer, register_count );

			if( info_pointer->direction != DIRECTION_POSITIVE )
			{

				int	index;

				for( index = 0 ; index < register_count / 2 ; ++index )
				{

					u8	*start = value_buffer + index, *end = value_buffer + register_count - 1 - index;
					
					u8	temp;

					temp = *start, *start = *end, *end = temp;

				}

			}

		}

		if( error < 0 )
			printk( "TTUCH : RMI read failed, ERROR(%d)\n", error );

		switch( register_count - 1 )
		{

			case	GET_1_VALUE :

				show( INFO_BUFFER_ADD, info_pointer->string, info_pointer->function, *( value_buffer + GET_1_VALUE ) );

				break;

			case	GET_2_VALUE :

				show( INFO_BUFFER_ADD, info_pointer->string, info_pointer->function, *( value_buffer + GET_1_VALUE ), *( value_buffer + GET_2_VALUE ) );

				break;

			case	GET_3_VALUE :

				show( INFO_BUFFER_ADD, info_pointer->string, info_pointer->function, *( value_buffer + GET_1_VALUE ), *( value_buffer + GET_2_VALUE ), *( value_buffer + GET_3_VALUE ) );

				break;

			case	GET_4_VALUE :

				show( INFO_BUFFER_ADD, info_pointer->string, info_pointer->function, *( value_buffer + GET_1_VALUE ), *( value_buffer + GET_2_VALUE ), *( value_buffer + GET_3_VALUE ), *( value_buffer + GET_4_VALUE ) );

				break;

			case	GET_5_VALUE :

				show( INFO_BUFFER_ADD, info_pointer->string, info_pointer->function, *( value_buffer + GET_1_VALUE ), *( value_buffer + GET_2_VALUE ), *( value_buffer + GET_3_VALUE ), *( value_buffer + GET_4_VALUE ), *( value_buffer + GET_5_VALUE ) );

				break;

			case	GET_6_VALUE :

				show( INFO_BUFFER_ADD, info_pointer->string, info_pointer->function, *( value_buffer + GET_1_VALUE ), *( value_buffer + GET_2_VALUE ), *( value_buffer + GET_3_VALUE ), *( value_buffer + GET_4_VALUE ), *( value_buffer + GET_5_VALUE ), *( value_buffer + GET_6_VALUE ) );

				break;

			case	GET_7_VALUE :

				show( INFO_BUFFER_ADD, info_pointer->string, info_pointer->function, *( value_buffer + GET_1_VALUE ), *( value_buffer + GET_2_VALUE ), *( value_buffer + GET_3_VALUE ), *( value_buffer + GET_4_VALUE ), *( value_buffer + GET_5_VALUE ), *( value_buffer + GET_6_VALUE ), *( value_buffer + GET_7_VALUE ) );

				break;

			case	GET_8_VALUE :

				show( INFO_BUFFER_ADD, info_pointer->string, info_pointer->function, *( value_buffer + GET_1_VALUE ), *( value_buffer + GET_2_VALUE ), *( value_buffer + GET_3_VALUE ), *( value_buffer + GET_4_VALUE ), *( value_buffer + GET_5_VALUE ), *( value_buffer + GET_6_VALUE ), *( value_buffer + GET_7_VALUE ), *( value_buffer + GET_8_VALUE ) );

				break;

			case	GET_9_VALUE :

				show( INFO_BUFFER_ADD, info_pointer->string, info_pointer->function, *( value_buffer + GET_1_VALUE ), *( value_buffer + GET_2_VALUE ), *( value_buffer + GET_3_VALUE ), *( value_buffer + GET_4_VALUE ), *( value_buffer + GET_5_VALUE ), *( value_buffer + GET_6_VALUE ), *( value_buffer + GET_7_VALUE ), *( value_buffer + GET_8_VALUE ), *( value_buffer + GET_9_VALUE ) );

				break;

			case	GET_10_VALUE :

				show( INFO_BUFFER_ADD, info_pointer->string, info_pointer->function, *( value_buffer + GET_1_VALUE ), *( value_buffer + GET_2_VALUE ), *( value_buffer + GET_3_VALUE ), *( value_buffer + GET_4_VALUE ), *( value_buffer + GET_5_VALUE ), *( value_buffer + GET_6_VALUE ), *( value_buffer + GET_7_VALUE ), *( value_buffer + GET_8_VALUE ), *( value_buffer + GET_9_VALUE ), *( value_buffer + GET_10_VALUE ) );

				break;

			default :

				printk( "TTUCH : No support this count(%d)\n", register_count );

				break;

		}

	}

}

int	software_reset( struct touch_tool_data *tool_data )
{

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	u16	address = get_register_addr( tool_data, RMI_FUNCTION_01, REGISTER_COMMAND_ID );

	if( address == INVALID_ADDRESS )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( REGISTER_COMMAND_ID ) );

		return	ERROR_INVALID_ADDRESS;

	}

	if( rmi_write( rmi_data, address + RMI_F01_COMMAND0, SOFT_RESET_BIT ) < 0 )
	{

		printk( "TTUCH : RMI block write failed\n" );

		return	ERROR_I2C_WRITE;

	}

	printk( "TTUCH : [F01:CMD0:Bit0], %s\n", F01_COMMAND0_RETURN_STRING );

	return	ERROR_NONE;

}

// ==========================================================================================================================

int	rmi_set_irq( struct synaptics_rmi4_data *rmi_data, bool state )
{

	return	rmi_data->irq_enable( rmi_data, state );

}

// ==========================================================================================================================

bool	enter_full_power_mode( struct touch_tool_data *tool_data )
{

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	u16	address;

	u8	value;

	int	error;

	if( ( address = get_register_addr( tool_data, RMI_FUNCTION_01, REGISTER_CONTROL_ID ) ) == INVALID_ADDRESS )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( REGISTER_CONTROL_ID ) );

		return	false;

	}

	if( ( error = rmi_read( rmi_data, address + RMI_F01_CONTROL0, &value ) ) < 0 )
	{

		printk( "TTUCH : RMI read failed, ERROR(%d)\n", error );

		return	false;

	}

	printk( "TTUCH : [f01:CTRL0]=0x%02x\n", value );

	if( ( error = rmi_write( rmi_data, address + RMI_F01_CONTROL0, FULL_POWER_MODE( value ) ) ) < 0 )
	{

		printk( "TTUCH : RMI write failed, ERROR(%d)\n", error );

		return	false;

	}

	printk( "TTUCH : [f01:CTRL0]<<0x%02x\n", value );

	return	true;

}

// ==========================================================================================================================

bool	wait_for_idle( struct touch_tool_data *tool_data, int delay )
{

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	int	timeout = ( ( delay * 1000 ) / MAX_SLEEP_TIME_US ) + 1;

	int	count = 0;

	bool	return_state = false;

	u16	address;

	u8	interrupt_state;

	if( ( address = get_register_addr( tool_data, RMI_FUNCTION_01, REGISTER_DATA_ID ) ) == INVALID_ADDRESS )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( REGISTER_DATA_ID ) );

		return	false;

	}

	while( count < timeout )
	{
		
		usleep_range( MIN_SLEEP_TIME_US, MAX_SLEEP_TIME_US );

		if( rmi_read( rmi_data, address + RMI_F01_DATA1, &interrupt_state ) < 0 )
			printk( "TTUCH : wait_for_idle(), RMI read failed\n" );

		count++;

		if( !interrupt_state )
			continue;

		return_state	= true;

		break;

	}

	return	return_state;

}

bool	flash_command( struct touch_tool_data *tool_data, u8 command, int timeout )
{

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	u16	query_address, data_address;

	u8	size[ 2 ], block_size;

	if( ( query_address = get_register_addr( tool_data, RMI_FUNCTION_34, REGISTER_QUERY_ID ) ) == INVALID_ADDRESS || ( data_address = get_register_addr( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID ) ) == INVALID_ADDRESS )
	{

		printk( "TTUCH : Can't get register address\n" );

		return	false;

	}

	if( rmi_read_block( rmi_data, query_address + RMI_F34_QUERY3_BLOCK_SIZE_7_0, size, sizeof( size ) ) < 0 )
	{

		printk( "TTUCH : RMI read failed\n" );

		return	false;

	}

	CONVERT_ARRAY_TO_INT( block_size, size );

	if( rmi_write( rmi_data, data_address + RMI_F34_DATA_FLASH_COMMAND + block_size, command ) < 0 )
	{

		printk( "TTUCH : RMI write failed\n" );

		return	false;

	}

	return	wait_for_idle( tool_data, timeout );

}

bool	read_register_datas( struct touch_tool_data *tool_data, int function, int register_id, struct register_datas_info *datas_info, unsigned int list_size )
{

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	u16	address;

	int	loop;

	if( ( address = get_register_addr( tool_data, function, register_id ) ) == INVALID_ADDRESS )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( register_id ) );

		return	false;

	}

	for( loop = 0 ; loop < list_size ; ++loop )
	{

		struct register_datas_info	*info = datas_info + loop;

		if( rmi_read_block( rmi_data, address + info->index, info->value, info->size ) < 0 )
		{

			printk( "TTUCH : RMI block read failed\n" );

			return	false;

		}

	}

	return	true;

}

bool	read_register_data( struct touch_tool_data *tool_data, int function, int register_id, int register_index, u8 *value, unsigned int size )
{

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	u16	address;

	if( ( address = get_register_addr( tool_data, function, register_id ) ) == INVALID_ADDRESS )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( register_id ) );

		return	false;

	}

	if( rmi_read_block( rmi_data, address + register_index, value, size ) < 0 )
	{

		printk( "TTUCH : RMI block read failed\n" );

		return	false;

	}

	return	true;

}

bool	write_register_data( struct touch_tool_data *tool_data, int function, int register_id, int register_index, u8 *value, unsigned int size )
{

	struct synaptics_rmi4_data	*rmi_data = tool_data->rmi_data;

	u16	address;

	if( ( address = get_register_addr( tool_data, function, register_id ) ) == INVALID_ADDRESS )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( register_id ) );

		return	false;

	}

	if( rmi_write_block( rmi_data, address + register_index, value, size ) < 0 )
	{

		printk( "TTUCH : RMI block write failed\n" );

		return	false;

	}

	return	true;

}

bool	special_flash_command( struct touch_tool_data *tool_data, u8 command, int timeout )
{

	u8	bootloader_id[ 2 ];

	if( !read_register_data( tool_data, RMI_FUNCTION_34, REGISTER_QUERY_ID, RMI_F34_QUERY0_BOOTLOADER_ID0, bootloader_id, sizeof( bootloader_id ) ) )
		return	false;

	if( !write_register_data( tool_data, RMI_FUNCTION_34, REGISTER_DATA_ID, RMI_F34_DATA_BLOCK_DATA, bootloader_id, sizeof( bootloader_id ) ) )
		return	false;

	return	flash_command( tool_data, command, timeout );

}

// ==========================================================================================================================

bool	check_image_header( struct file *image_file, char *string, unsigned index )
{

	struct upgrade_file_header	file_header;

	int	read_count;

	( void )image_file->f_op->llseek( image_file, 0, 0 );

	if( ( read_count = image_file->f_op->read( image_file, ( char* )&file_header, sizeof( struct upgrade_file_header ), &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read file header failed\n" );

		return	false;

	}

	if( read_count != sizeof( struct upgrade_file_header ) )
	{

		printk( "TTUCH : Incorrect header size\n" );

		return	false;

	}

	if( strcmp( file_header.fih_string, "FIH" ) || strcmp( file_header.check_string, string ) )
	{

		printk( "TTUCH : Incorrect header\n" );

		return	false;

	}

	if( index >= file_header.count )
	{

		printk( "TTUCH : Incorrect index\n" );

		return	false;

	}

	return	true;

}

bool	get_start_file_data_position( struct file *image_file, unsigned index, long *position )
{

	struct upgrade_file_header	file_header;

	struct upgrade_data_information	data_information;

	int	read_count;

	( void )image_file->f_op->llseek( image_file, 0, 0 );

	if( ( read_count = image_file->f_op->read( image_file, ( char* )&file_header, sizeof( struct upgrade_file_header ), &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read file header failed\n" );

		return	false;

	}

	if( read_count != sizeof( struct upgrade_file_header ) )
	{

		printk( "TTUCH : Incorrect header size\n" );

		return	false;

	}

	( void )image_file->f_op->llseek( image_file, sizeof( struct upgrade_data_information ) * index, SEEK_CUR );

	if( ( read_count = image_file->f_op->read( image_file, ( char* )&data_information, sizeof( struct upgrade_data_information ), &image_file->f_pos ) ) < 0 )
	{

		printk( "TTUCH : Read data information failed\n" );

		return	false;

	}

	if( read_count != sizeof( struct upgrade_data_information ) )
	{

		printk( "TTUCH : Incorrect header size\n" );

		return	false;

	}

	printk( "TTUCH : [ID:Start:Size]=[%s:%ld:%ld]\n", data_information.version_string, data_information.start, data_information.size );

	*position	= data_information.start;

	return	true;

}

// ==========================================================================================================================

char*	get_buffer_address( struct touch_tool_data *tool_data, enum buffer_kind *kind, unsigned int *buffer_size )
{

	struct	buffer_information
	{

		int			ID;

		enum buffer_kind	kind;

		char			*buffer;

		unsigned int		size;

	};

	struct buffer_information	info_list[] = { BUFFER_SELECT_INFORMATION };

	char			*return_buffer = tool_data->info_buffer;

	enum buffer_kind	return_kind = INFORMATION_BUFFER;

	unsigned int		size = PAGE_SIZE;

	int			loop;

	for( loop = 0 ; loop < sizeof( info_list ) / sizeof( *info_list ) ; ++loop )
	{

		struct buffer_information	*info = info_list + loop;

		if( tool_data->command_id == info->ID )
		{

			return_kind = info->kind, size = info->size, return_buffer = info->buffer;

			break;

		}

	}

	return	*kind = return_kind, *buffer_size = size, return_buffer;

}
