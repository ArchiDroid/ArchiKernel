
#include <linux/input/FIH-tool/FIH-tool.h>

#include <linux/input/FIH-tool/tool_functions.h>

// ==========================================================================================================================

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

struct rmi_function_container*	get_function_container( struct rmi_device *rmi_dev, int function )
{

	struct rmi_driver_data		*driver_data = rmi_get_driverdata( rmi_dev );

	struct rmi_function_container	*container = NULL;

	if( function == RMI_FUNCTION_01 )
	{

		if( !driver_data )
		{

			printk( "TTUCH : The pointer of driver data is NULL\n" );

			return	container;

		}

		if( !( container = driver_data->f01_container ) )
		{

			printk( "TTUCH : The pointer of F%02x is NULL\n", function );

			return	container;

		}

		if( function != container->fd.function_number )
			container	= NULL;

	}
	else
		list_for_each_entry( container, &driver_data->rmi_functions.list, list )
		{

			if( !container )
			{

				printk( "TTUCH : The pointer of F%02x is NULL\n", function );

				return	container;

			}

			if( function == container->fd.function_number )
				break;

		}

	return	container;

}

u16	get_register_addr( struct rmi_device *rmi_dev, int function, int register_id )
{

	struct rmi_function_container	*container = get_function_container( rmi_dev, function );

	u16	return_addr = -1;

	if( !container )
		printk( "TTUCH : F%02x Not found\n", function );
	else
	{

		struct rmi_function_descriptor	*all_addr = &container->fd;

		u16	address[] =
		{

			all_addr->query_base_addr,
			all_addr->command_base_addr,
			all_addr->control_base_addr,
			all_addr->data_base_addr

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

	return_buffer->error_code = error, return_buffer->command = command, strcpy( return_buffer->string, RETURN_VALUE_STRING );

	if( error != ERROR_NONE )
		return;

	for( loop = 0 ; loop < sizeof( action_list ) / sizeof( *action_list ) ; ++loop )
	{

		if( ( action_list + loop )->id != command )
			continue;

		switch( ( action_list + loop )->action )
		{

			case	ACTION_1 :

				return_buffer->values.block_data.value	= ( u8* )return_buffer + sizeof( struct return_struct );

				return_buffer->values.block_data.count	= count;

				break;

			case	ACTION_2 :

				return_buffer->values.size	= *( unsigned int* )data;

				break;

			case	ACTION_3 :

				return_buffer->values.name	= ( char* )data;

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

			case	ACTION_1 :

				if( return_buffer->error_code != ERROR_NONE )
					break;

				info_print( INFO_BUFFER_ADD, "Value=%x\n", return_buffer->values.value );

				break;

			case	ACTION_2 :

				if( return_buffer->error_code != ERROR_NONE )
				{

					info_print( INFO_BUFFER_ADD, "Buffer is full(%d)\n", return_buffer->values.block_data.count );

					break;

				}

				{

					int	loop, loop1;

					int	row = return_buffer->values.block_data.count / 16 ? return_buffer->values.block_data.count / 16 : 1 ;

					u8	*values = return_buffer->values.block_data.value;

					info_print( INFO_BUFFER_ADD, "Value count(%d), vlaue=>\n", return_buffer->values.block_data.count );

					for( loop = 0 ; loop < row ; ++loop )
					{

						int	remainder = return_buffer->values.block_data.count - loop * 16;

						int	value_count = remainder >= 16 ? 16 : remainder;

						for( loop1 = 0 ; loop1 < value_count ; ++loop1 )
							info_print( INFO_BUFFER_ADD, "%02x ", *( values + loop * 16 + loop1 ) );

						info_print( INFO_BUFFER_ADD, "\n" );
					}

					break;

				}

				break;

			case	ACTION_3 :

				info_print( INFO_BUFFER_ADD, "Memory size is %d byte\n", return_buffer->values.size );

				break;

			case	ACTION_4 :

				info_print( INFO_BUFFER_ADD, "File name(%s)\n", return_buffer->values.name );

				break;

			case	ACTION_5 :

				info_print( INFO_BUFFER_ADD, "%s\n", return_buffer->values.name );

				break;

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

		creat_block_data_number_string( block_data, buffer + index, loop );

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

		creat_block_data_character_string( block_data, buffer + index, loop );

		info_print( INFO_BUFFER_ADD, "%04d |%s\n", start + index, block_data );

	}

}

// ==========================================================================================================================

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

	struct rmi_device		*rmi_dev = ( ( struct touch_tool_data* )tool_data )->rmi_dev;

	struct rmi_function_container	*container = get_function_container( rmi_dev, RMI_FUNCTION_01 );

	struct rmi_driver_data		*driver_data = rmi_get_driverdata( rmi_dev );

	struct rmi_function_descriptor	*descriptor;

	u8	value_buffer[ MAX_GET_COUNT ];

	int	loop, error;

	show( INFO_BUFFER_RESET, NULL );

	if( container )
	{

		descriptor	= &container->fd;

		show( INFO_BUFFER_ADD, "f01, [Query:CMD:CTRL:DATA]=[0x%04x:0x%04x:0x%04x:0x%04x]\n", descriptor->query_base_addr, descriptor->command_base_addr, descriptor->control_base_addr, descriptor->data_base_addr );

	}

	list_for_each_entry( container, &driver_data->rmi_functions.list, list )
		if( container )
		{

			descriptor	= &container->fd;

			show( INFO_BUFFER_ADD, "f%02x, [Query:CMD:CTRL:DATA]=[0x%04x:0x%04x:0x%04x:0x%04x]\n", descriptor->function_number, descriptor->query_base_addr, descriptor->command_base_addr, descriptor->control_base_addr, descriptor->data_base_addr );

		}

	for( loop = 0 ; loop < sizeof( register_info ) / sizeof( *register_info ) ; ++loop )
	{

		struct register_information	*info_pointer = register_info + loop;

		int	register_count = info_pointer->end_index - info_pointer->start_index + 1;

		u16	address = get_register_addr( rmi_dev, info_pointer->function, info_pointer->register_id );

		if( address == -1 )
			continue;

		if( register_count == ONE_REGISTER )
			error	= rmi_read( rmi_dev, address + info_pointer->start_index, value_buffer );
		else
		{

			error	= rmi_read_block( rmi_dev, address + info_pointer->start_index, value_buffer, register_count );

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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	error;

	u16	address = get_register_addr( rmi_dev, parameters.function, parameters.register_id );

	u8	value;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_READ_VALUE_FROM_FUNCTION_GROUP );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_READ_VALUE_FROM_FUNCTION_GROUP;

	if( address == -1 )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( parameters.register_id ) );

		info_print( INFO_BUFFER_ADD, "Can't get address of f%02x %s register\n", parameters.function, get_register_string( parameters.register_id ) );

		return;

	}

	if( ( error = rmi_read( rmi_dev, address + parameters.index, &value ) ) < 0 )
	{

		printk( "TTUCH : RMI read failed, ERROR(%d)\n", error );

		info_print( INFO_BUFFER_ADD, "RMI read failed, ERROR(%d)\n", error );

	}
	else
	{

		printk( "TTUCH : [F%02x:%s%d]=%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, value );

		info_print( INFO_BUFFER_ADD, "[F%02x:%s%d]=%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, value );

	}

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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	error;

	u16	address = get_register_addr( rmi_dev, parameters.function, parameters.register_id );

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_WRITE_VALUE_TO_FUNCTION_GROUP );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_WRITE_VALUE_TO_FUNCTION_GROUP;

	if( address == -1 )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( parameters.register_id ) );

		info_print( INFO_BUFFER_ADD, "Can't get address of f%02x %s register\n", parameters.function, get_register_string( parameters.register_id ) );

		return;

	}

	if( ( error = rmi_write( rmi_dev, address + parameters.index, parameters.value ) ) < 0 )
	{

		printk( "TTUCH : RMI write failed, ERROR(%d)\n", error );

		info_print( INFO_BUFFER_ADD, "RMI write failed, ERROR(%d)\n", error );

	}
	else
	{

		printk( "TTUCH : [F%02x:%s%d]<<%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, parameters.value );

		info_print( INFO_BUFFER_ADD, "[F%02x:%s%d]<<%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, parameters.value );

	}

}

void	S_read_register_value( void *node_data, struct command_parameter *parameter )
{

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	address = parameter->para1.container.number;

	int	error;

	u8	value;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_READ_VALUE_FROM_REGISTER );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_READ_VALUE_FROM_REGISTER;

	if( ( error = rmi_read( rmi_dev, address, &value ) ) < 0 )
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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	error;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_WRITE_VALUE_TO_REGISTER );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_WRITE_VALUE_TO_REGISTER;

	if( ( error = rmi_write( rmi_dev, parameters.register_addr, parameters.value ) ) < 0 )
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

		int	function, register_id, start_register, end_register;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number, parameter->para3.container.number, parameter->para4.container.number,

	};

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	registry_count = parameters.end_register - parameters.start_register + 1;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer;

	u16	address = get_register_addr( rmi_dev, parameters.function, parameters.register_id );

	int	error;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_READ_BLOC_DATA_FROM_FUNCTION_GROUP );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_READ_BLOC_DATA_FROM_FUNCTION_GROUP;

	if( registry_count >= PARA_BUFFER_SIZE )
	{

		printk( "TTUCH : Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE, registry_count );

		info_print( INFO_BUFFER_ADD, "Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE, registry_count );

		return;

	}

	if( address == -1 )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( parameters.register_id ) );

		info_print( INFO_BUFFER_ADD, "Can't get address of f%02x %s register\n", parameters.function, get_register_string( parameters.register_id ) );

		return;

	}

	if( ( error = rmi_read_block( rmi_dev, address + parameters.start_register, values, registry_count ) ) < 0 )
	{

		printk( "TTUCH : RMI block read failed, ERROR(%d)\n", error );

		info_print( INFO_BUFFER_ADD, "RMI read failed, ERROR(%d)\n", error );

	}
	else
	{

		u8	*buffer = ( ( struct touch_tool_data* )node_data )->block_data_buffer;

		int	loop, index;

		printk( "TTUCH : [F%02x:%s%d-%d], block data\n", parameters.function, get_register_string( parameters.register_id ), parameters.start_register, parameters.end_register );

		info_print( INFO_BUFFER_ADD, "[F%02x:%s%d-%d], block data\n", parameters.function, get_register_string( parameters.register_id ), parameters.start_register, parameters.end_register );

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

		int	start_register, end_register;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number,

	};

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	registry_count = parameters.end_register - parameters.start_register + 1;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer;

	int	error;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_READ_BLOC_DATA_FROM_REGISTER );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_READ_BLOC_DATA_FROM_REGISTER;

	if( registry_count >= PARA_BUFFER_SIZE )
	{

		printk( "TTUCH : Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE, registry_count );

		info_print( INFO_BUFFER_ADD, "Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE, registry_count );

		return;

	}

	if( ( error = rmi_read_block( rmi_dev, parameters.start_register, values, registry_count ) ) < 0 )
	{

		printk( "TTUCH : RMI block read failed, ERROR(%d)\n", error );

		info_print( INFO_BUFFER_ADD, "RMI read failed, ERROR(%d)\n", error );

	}
	else
	{

		u8	*buffer = ( ( struct touch_tool_data* )node_data )->block_data_buffer;

		int	loop, index;

		printk( "TTUCH : [REG%d-%d], block data\n", parameters.start_register, parameters.end_register );

		info_print( INFO_BUFFER_ADD, "[REG%d-%d], block data\n", parameters.start_register, parameters.end_register );

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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	int	error = ERROR_NONE;

	u16	address = get_register_addr( rmi_dev, parameters.function, parameters.register_id );

	u8	value = 0;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP;

	if( address == -1 )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( parameters.register_id ) );

		create_return_structure( return_buffer, COMMAND_N_READ_VALUE_FROM_FUNCTION_GROUP, ERROR_INVALID_ADDRESS, NULL, 0 );

		return;

	}

	if( rmi_read( rmi_dev, address + parameters.index, &value ) < 0 )
	{

		printk( "TTUCH : RMI read failed\n" );

		error	= ERROR_I2C_READ;

	}
	else
		printk( "TTUCH : [F%02x:%s%d]=%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, value );

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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	int	error = ERROR_NONE;

	u16	address = get_register_addr( rmi_dev, parameters.function, parameters.register_id );

	u8	value = parameters.value;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP;

	if( address == -1 )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( parameters.register_id ) );

		create_return_structure( return_buffer, COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, ERROR_INVALID_ADDRESS, NULL, 0 );

		return;

	}

	if( rmi_write( rmi_dev, address + parameters.index, parameters.value ) < 0 )
	{

		printk( "TTUCH : RMI write failed\n" );

		error	= ERROR_I2C_WRITE;

	}
	else
		printk( "TTUCH : [F%02x:%s%d]<<%x\n", parameters.function, get_register_string( parameters.register_id ), parameters.index, parameters.value );

	create_return_structure( return_buffer, COMMAND_N_WRITE_VALUE_TO_FUNCTION_GROUP, error, &value, 1 );

}

void	N_read_register_value( void *node_data, struct command_parameter *parameter )
{

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	address = parameter->para1.container.number;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	int	error = ERROR_NONE;

	u8	value = 0;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_READ_VALUE_FROM_REGISTER;

	if( rmi_read( rmi_dev, address, &value ) < 0 )
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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	int	error = ERROR_NONE;

	u8	value = parameters.value;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_WRITE_VALUE_TO_REGISTER;

	if( rmi_write( rmi_dev, parameters.register_addr, parameters.value ) < 0 )
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

		int	function, register_id, start_register, end_register;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number, parameter->para3.container.number, parameter->para4.container.number,

	};

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	registry_count = parameters.end_register - parameters.start_register + 1;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer + sizeof( struct return_struct );

	int	error = ERROR_NONE;

	u16	address = get_register_addr( rmi_dev, parameters.function, parameters.register_id );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_READ_BLOC_DATA_FROM_FUNCTION_GROUP;

	if( registry_count >= PARA_BUFFER_SIZE - sizeof( struct return_struct ) )
	{

		printk( "TTUCH : Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE - sizeof( struct return_struct ), registry_count );

		create_return_structure( return_buffer, COMMAND_N_READ_BLOC_DATA_FROM_FUNCTION_GROUP, ERROR_BUFFER_FULL, NULL, 0 );

		return;

	}

	if( address == -1 )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( parameters.register_id ) );

		create_return_structure( return_buffer, COMMAND_N_READ_BLOC_DATA_FROM_FUNCTION_GROUP, ERROR_INVALID_ADDRESS, NULL, 0 );

		return;

	}

	if( rmi_read_block( rmi_dev, address + parameters.start_register, values, registry_count ) < 0 )
	{

		printk( "TTUCH : RMI block read failed\n" );

		error	= ERROR_I2C_READ;

	}

	create_return_structure( return_buffer, COMMAND_N_READ_BLOC_DATA_FROM_FUNCTION_GROUP, error, NULL, registry_count );

}

void	N_read_register_block_data( void *node_data, struct command_parameter *parameter )
{

	struct	parameters_info
	{

		int	start_register, end_register;

	};

	struct parameters_info	parameters =
	{

		parameter->para1.container.number, parameter->para2.container.number,

	};

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	registry_count = parameters.end_register - parameters.start_register + 1;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer + sizeof( struct return_struct );

	int	error = ERROR_NONE;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_READ_BLOC_DATA_FROM_REGISTER;

	if( registry_count >= PARA_BUFFER_SIZE - sizeof( struct return_struct ) )
	{

		printk( "TTUCH : Out of buffer, buffer[Max:Using]=[%d:%d]\n", PARA_BUFFER_SIZE - sizeof( struct return_struct ), registry_count );

		create_return_structure( return_buffer, COMMAND_N_READ_BLOC_DATA_FROM_REGISTER, ERROR_BUFFER_FULL, NULL, 0 );

		return;

	}

	if( rmi_read_block( rmi_dev, parameters.start_register, values, registry_count ) < 0 )
	{

		printk( "TTUCH : RMI block read failed\n" );

		error	= ERROR_I2C_READ;

	}

	create_return_structure( return_buffer, COMMAND_N_READ_BLOC_DATA_FROM_REGISTER, error, NULL, registry_count );

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

	dump_memory_value( ( struct touch_tool_data* )node_data, ( ( struct touch_tool_data* )node_data )->parameter_buffer, 0, PARA_BUFFER_SIZE );

}

void	dump_para_buffer_character( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_PARA_BUFFER_DUMP_CHARACTER;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Parameter buffer[%d]\n", PARA_BUFFER_SIZE );

	dump_memory_character( ( struct touch_tool_data* )node_data, ( ( struct touch_tool_data* )node_data )->parameter_buffer, 0, PARA_BUFFER_SIZE );

}

void	enter_bootloader_mode( void *node_data, struct command_parameter *parameter )
{

	struct flash_information	*flash_data = ( ( struct touch_tool_data* )node_data )->flash_data;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_ENTER_BOOTLOADER_MODE;

	info_print( INFO_BUFFER_RESET, NULL );

	if( !flash_data )
	{

		printk( "TTUCH : Not found flash information\n" );

		info_print( INFO_BUFFER_ADD, "Not found flash information\n" );

		return;

	}

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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	*source = &parameters.parameter1;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer;

	u16	address = get_register_addr( rmi_dev, parameters.function, parameters.register_id );

	int	error, loop;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP;

	if( parameters.parameter_count > MAX_BLOCK_DATA )
	{

		printk( "TTUCH : Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		info_print( INFO_BUFFER_ADD, "Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		return;

	}

	if( address == -1 )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( parameters.register_id ) );

		info_print( INFO_BUFFER_ADD, "Can't get address of f%02x %s register\n", parameters.function, get_register_string( parameters.register_id ) );

		return;

	}

	for( loop = 0 ; loop < parameters.parameter_count ; ++loop )
		*( values + loop )	= *( source + loop );

	if( ( error = rmi_write_block( rmi_dev, address + parameters.start_register, values, parameters.parameter_count ) ) < 0 )
	{

		printk( "TTUCH : RMI block read failed, ERROR(%d)\n", error );

		info_print( INFO_BUFFER_ADD, "RMI read failed, ERROR(%d)\n", error );

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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	*source = &parameters.parameter1;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer;

	int	error, loop;

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "Command%02d : ", COMMAND_S_WRITE_BLOC_DATA_FROM_REGISTER );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_S_WRITE_BLOC_DATA_FROM_REGISTER;

	if( parameters.parameter_count > MAX_BLOCK_DATA )
	{

		printk( "TTUCH : Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		info_print( INFO_BUFFER_ADD, "Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		return;

	}

	for( loop = 0 ; loop < parameters.parameter_count ; ++loop )
		*( values + loop )	= *( source + loop );

	if( ( error = rmi_write_block( rmi_dev, parameters.start_register, values, parameters.parameter_count ) ) < 0 )
	{

		printk( "TTUCH : RMI block read failed, ERROR(%d)\n", error );

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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	*source = &parameters.parameter1;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer + sizeof( struct return_struct );

	u16	address = get_register_addr( rmi_dev, parameters.function, parameters.register_id );

	int	error = ERROR_NONE;

	int	loop;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP;

	if( parameters.parameter_count > MAX_BLOCK_DATA )
	{

		printk( "TTUCH : Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		create_return_structure( return_buffer, COMMAND_N_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP, ERROR_BUFFER_FULL, NULL, 0 );

		return;

	}

	if( address == -1 )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( parameters.register_id ) );

		create_return_structure( return_buffer, COMMAND_N_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP, ERROR_INVALID_ADDRESS, NULL, 0 );

		return;

	}

	for( loop = 0 ; loop < parameters.parameter_count ; ++loop )
		*( values + loop )	= *( source + loop );

	if( rmi_write_block( rmi_dev, address + parameters.start_register, values, parameters.parameter_count ) < 0 )
	{

		printk( "TTUCH : RMI block read failed\n" );

		error	= ERROR_I2C_WRITE;

	}

	create_return_structure( return_buffer, COMMAND_N_WRITE_BLOC_DATA_FROM_FUNCTION_GROUP, error, NULL, parameters.parameter_count );

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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	int	*source = &parameters.parameter1;

	struct return_struct	*return_buffer = ( struct return_struct* )( ( struct touch_tool_data* )node_data )->parameter_buffer;

	u8	*values = ( ( struct touch_tool_data* )node_data )->parameter_buffer + sizeof( struct return_struct );

	int	error = ERROR_NONE;

	int	loop;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_N_WRITE_BLOC_DATA_FROM_REGISTER;

	if( parameters.parameter_count > MAX_BLOCK_DATA )
	{

		printk( "TTUCH : Out of block buffer, block buffer[Max:Using]=[%d:%d]\n", MAX_BLOCK_DATA, parameters.parameter_count );

		create_return_structure( return_buffer, COMMAND_N_WRITE_BLOC_DATA_FROM_REGISTER, ERROR_BUFFER_FULL, NULL, 0 );

		return;

	}

	for( loop = 0 ; loop < parameters.parameter_count ; ++loop )
		*( values + loop )	= *( source + loop );

	if( rmi_write_block( rmi_dev, parameters.start_register, values, parameters.parameter_count ) < 0 )
	{

		printk( "TTUCH : RMI block read failed, ERROR(%d)\n", error );

		error	= ERROR_I2C_WRITE;

	}

	create_return_structure( return_buffer, COMMAND_N_WRITE_BLOC_DATA_FROM_REGISTER, error, NULL, parameters.parameter_count );

}

void	scan_page_description_table( void *node_data, struct command_parameter *parameter )
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

	struct rmi_device	*rmi_dev = ( ( struct touch_tool_data* )node_data )->rmi_dev;

	struct pdt_data		data;

	unsigned int		loop, loop1;

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_SCAN_PAGE_DESCRIPTION_TABLE;

	info_print( INFO_BUFFER_RESET, NULL );

	for( loop = 0 ; loop < RMI4_MAX_PAGE ; ++loop )
	{

		int	page_start = RMI4_PAGE_SIZE * loop;

		int	start = page_start + RMI4_PDT_START, end = page_start + RMI4_PDT_END;

		for( loop1 = start ; loop1 >= end ; loop1 -= sizeof( struct pdt_data ) )
		{

			if( rmi_read_block( rmi_dev, loop1, ( u8* )&data, sizeof( struct pdt_data ) ) != sizeof( struct pdt_data ) )
			{

				printk( "TTUCH : RMI block read failed\n" );

				return;

			}

			if( !data.function_number )
				break;

			printk( "TTUCH : Found f%02x\n", data.function_number );

			info_print( INFO_BUFFER_ADD, "f%02x : Version(%d), interrupt count(%d)\n", data.function_number, data.function_version, data.interrupt_source_count );

			info_print( INFO_BUFFER_ADD, "Query address   : 0x%04x\n", page_start + data.query_base_addr );

			info_print( INFO_BUFFER_ADD, "Command address : 0x%04x\n", page_start + data.command_base_addr );

			info_print( INFO_BUFFER_ADD, "Control address : 0x%04x\n", page_start + data.control_base_addr );

			info_print( INFO_BUFFER_ADD, "Data address    : 0x%04x\n\n", page_start + data.data_base_addr );

		}

	}

}

void	create_flash_data( void *node_data, struct command_parameter *parameter )
{

	struct	read_information
	{

		int	register_offset;

		int	*store_address;

	};

	struct touch_tool_data		*tool_data = ( struct touch_tool_data* )node_data;

	struct rmi_device		*rmi_dev = tool_data->rmi_dev;

	struct flash_information	*flash_info = tool_data->flash_data;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	u16	address = get_register_addr( rmi_dev, RMI_FUNCTION_34, REGISTER_QUERY_ID );

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

	if( address == -1 )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( REGISTER_QUERY_ID ) );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FLASH_DATA, ERROR_INVALID_ADDRESS, NULL, 0 );

		return;

	}

	for( loop = 0 ; loop < sizeof( read_list ) / sizeof( *read_list ) ; ++loop )
		if( rmi_read_block( rmi_dev, address + ( read_list + loop )->register_offset, block_info, sizeof( block_info ) )  < 0 )
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

	if( flash_info->size < MEMORY_BUFFER_SIZE )
		flash_info->size	= MEMORY_BUFFER_SIZE;

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

		create_return_structure( return_buffer, COMMAND_RELEASE_FLASH_DATA, ERROR_RESOURCE_NON_EXIST, NULL, 0 );

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

	struct rmi_device		*rmi_dev = tool_data->rmi_dev;

	struct return_struct		*return_buffer = ( struct return_struct* )tool_data->parameter_buffer;

	u16	address = get_register_addr( rmi_dev, RMI_FUNCTION_01, REGISTER_COMMAND_ID );

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_RESET_TOUCH_IC;

	if( address == -1 )
	{

		printk( "TTUCH : Can't get %s register address\n", get_register_string( REGISTER_COMMAND_ID ) );

		create_return_structure( return_buffer, COMMAND_RESET_TOUCH_IC, ERROR_INVALID_ADDRESS, NULL, 0 );

		return;

	}

	if( rmi_write( rmi_dev, address + RMI_F01_COMMAND0, SOFT_RESET_BIT ) < 0 )
	{

		printk( "TTUCH : RMI block write failed\n" );

		create_return_structure( return_buffer, COMMAND_RESET_TOUCH_IC, ERROR_I2C_WRITE, NULL, 0 );

		return;

	}

	printk( "TTUCH : [F01:CMD0:Bit0], %s\n", F01_COMMAND0_RETURN_STRING );

	create_return_structure( return_buffer, COMMAND_RESET_TOUCH_IC, ERROR_NONE, F01_COMMAND0_RETURN_STRING, 1 );

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

	strcpy( file_info->name, file_name );

	file_info->image_file	= filp_open( file_info->name, O_WRONLY | O_CREAT, 0 );

	if( IS_ERR( file_info->image_file ) )
	{

		printk( "TTUCH : Open file(%s) failed\n", file_info->name );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FILE_INFORMATION, ERROR_OPEN_FILE_FAILED, NULL, 0 );

		kfree( file_info );

		return;

	}

	if( !file_info->image_file->f_op || !file_info->image_file->f_op->read || !file_info->image_file->f_op->write )
	{

		printk( "TTUCH : Not found read/write function\n" );

		create_return_structure( return_buffer, COMMAND_ALLOCATE_FILE_INFORMATION, ERROR_NOT_FOUND_FUNCTION, NULL, 0 );

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

		create_return_structure( return_buffer, COMMAND_RELEASE_FILE_INFORMATION, ERROR_RESOURCE_NON_EXIST, NULL, 0 );

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

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_MEMORY_DUMP_VALUE;

}

void	memory_dump_character( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_MEMORY_DUMP_CHARACTER;

}

void	memory_to_file( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_MEMORY_TO_FILE;

}

void	memory_to_flash( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_MEMORY_TO_FLASH;

}

void	flash_to_memory( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_FLASH_TO_MEMORY;

}

void	configuration_to_memory( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_CONFIGURATION_TO_MEMORY;

}

void	configuration_to_flash( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_CONFIGURATION_TO_FLASH;

}

void	file_to_memory( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_FILE_TO_MEMORY;

}

void	file_to_flash( void *node_data, struct command_parameter *parameter )
{

	( ( struct touch_tool_data* )node_data )->command_id	= COMMAND_FILE_TO_FLASH;

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

struct touch_tool_data*	allocate_memory( void )
{

	struct touch_tool_data	*tool_data;

	if( ( tool_data = kzalloc( sizeof( struct touch_tool_data ), GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Get memory of tool data failed\n" );

		return	NULL;

	}

	if( ( tool_data->info_buffer = kzalloc( INFO_BUFFER_SIZE, GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Get memory of infomation buffer failed\n" );

		return	NULL;

	}

	if( ( tool_data->parameter_buffer = kzalloc( PARA_BUFFER_SIZE, GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Get memory of parameter buffer failed\n" );

		return	NULL;

	}

	if( ( tool_data->block_data_buffer = kzalloc( BLOCK_DATA_BUFFER_SIZE, GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Get memory of block data buffer failed\n" );

		return	NULL;

	}

	return	tool_data;

}

void	release_memory( struct touch_tool_data *tool_data )
{

	kfree( tool_data->block_data_buffer );

	kfree( tool_data->parameter_buffer );

	kfree( tool_data->info_buffer );

	kfree( tool_data );

}

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

// ==========================================================================================================================

int	get_rmi_memory( struct device *dev, void *data )
{

	struct rmi_device	*device_data, **return_pointer;

	struct rmi_device_platform_data *platform_data;

	device_data	= to_rmi_device( dev );

	return_pointer	= ( struct rmi_device** )data;
	
	platform_data	= to_rmi_platform_data( device_data );

	if( !strcmp( platform_data->driver_name, "rmi_generic" ) )
	{

		*return_pointer	= device_data;

		printk( "TTUCH : Found device of rmi_generic and store device data\n" );

	}

	return	0;

}
