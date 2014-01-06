
#include <linux/input/FIH-tool/FIH-tool.h>

#include <linux/input/FIH-tool/tool_functions.h>

// ==========================================================================================================================

void info_print( int command, char *string, ... );

char* get_buffer_address( struct touch_tool_data*, enum buffer_kind*, unsigned int* );

// ==========================================================================================================================

ssize_t	tool_command( void *node_data, struct device_attribute *attribute, char *buffer )
{

	char	*command_string =
	{

		"\nFUNC=Function\n"
		"REG=Register, [QUERY:CMD:CTRL:DATA]=[0:1:2:3]\n"
		"INDX=Index, QUERY0=>Index 0, CTRL55=>Index 55\n"
		"ADDR=Register address\n"
		"S_REG=Start register address\n"
		"E_REG=End register address\n"
		"S_ADD=Start memory address\n"
		"E_ADD=End memory address\n"
		"COUT=Parameter count\n"
		"VAL=Value\n"
		"\n"
		"H=>Hex, D=>Dec, S=>String\n\n"

	};

	info_print( INFO_BUFFER_RESET, NULL );

	info_print( INFO_BUFFER_ADD, "%s", command_string );

	SHOW_INFORMATION();

	return	snprintf( buffer, PAGE_SIZE, ( ( struct touch_tool_data* )node_data )->info_buffer );

}

// ==========================================================================================================================

ssize_t	tool_control( void *node_data, struct device_attribute *attribute, const char *buffer, size_t size )
{

	struct touch_tool_data	*tool_data = node_data;
	
	char	*parameter_buffer = tool_data->parameter_buffer;

	struct	command_info	*command = &tool_data->command;

	unsigned int	pointer = 0, para_buffer_pointer = 0;

	int	rem_size = size;

	while( pointer < rem_size )
	{

		struct BS_data	*search_data;

		struct command_data	*valid_command;

		unsigned int	char_count;

		int	user_command;

		info_print( INFO_BUFFER_RESET, NULL );

		if( !get_para_from_buffer( buffer + pointer, "%d", &user_command ) )
		{

			printk( "TTUCH : Can't get command from buffer\n" );

			info_print( INFO_BUFFER_ADD, "Can't get command from buffer\n" );

			break;

		}

		if( !( search_data = binary_search( command->commmand_buffer, command->count, user_command ) ) )
		{

			printk( "TTUCH : Can't support command(%d)\n", user_command );

			info_print( INFO_BUFFER_ADD, "Can't support command(%d)\n", user_command );

			break;

		}

		valid_command	= ( struct command_data* )search_data->data;

		if( !( char_count = get_para_char_count( buffer + pointer, buffer + size, ONE_PARAMETER ) ) )
		{

			printk( "TTUCH : Can't get charactor count.\n" );

			info_print( INFO_BUFFER_ADD, "Can't get charactor count.\n" );

			break;

		}

		pointer += char_count;

		printk( "TTUCH : [CMD:PARA]=[%d:%d]\n", user_command, valid_command->parameter_count );

		{

			struct command_parameter	parameter;

			struct parameter_info	*parameter_pointer[] =
			{

				&parameter.para1, &parameter.para2, &parameter.para3, &parameter.para4, &parameter.para5,
				&parameter.para6, &parameter.para7, &parameter.para8, &parameter.para9, &parameter.para10,
				&parameter.para11, &parameter.para12, &parameter.para13, &parameter.para14, &parameter.para15,
				&parameter.para16, &parameter.para17, &parameter.para18, &parameter.para19, &parameter.para20

			};

			char	para_string[ THREE_CHARATOR ] = { 0 };

			unsigned int	para_count = 0, parameter_string_lenght = 0;

			int		command_parameter_count = valid_command->parameter_count;

			unsigned int	loop;

			if( command_parameter_count == COMMAND_N_PARAMENTER )
				command_parameter_count	= sizeof( parameter_pointer ) / sizeof( *parameter_pointer );

			if( command_parameter_count > sizeof( parameter_pointer ) / sizeof( *parameter_pointer ) )
			{

				printk( "TTUCH : Out of parameter array\n" );

				info_print( INFO_BUFFER_ADD, "Out of parameter array\n" );

				break;

			}

			if( command_parameter_count && !valid_command->parameter_string )
			{

				printk( "TTUCH : Parameter count(%d), but no parameter string\n", command_parameter_count );

				info_print( INFO_BUFFER_ADD, "Parameter count(%d), but no parameter string\n", command_parameter_count );

				break;

			}

			if( valid_command->parameter_string )
				parameter_string_lenght	= strlen( valid_command->parameter_string ) + 1;

			for( loop = 0 ; loop < command_parameter_count ; ++loop )
			{

				struct parameter_info	*para_info = *( parameter_pointer + loop );

				unsigned int	return_value;

				memcpy( para_string, valid_command->parameter_string + para_count, TWO_CHARATOR );

				para_count += get_para_char_count( valid_command->parameter_string + para_count, valid_command->parameter_string + parameter_string_lenght, ONE_PARAMETER );

				char_count	= get_para_char_count( buffer + pointer, buffer + size, ONE_PARAMETER );

				if( !strcmp( para_string, "%s" ) )
				{

					if( para_buffer_pointer >= PARA_BUFFER_SIZE || para_buffer_pointer + char_count >= PARA_BUFFER_SIZE )
					{

						printk( "TTUCH : Parameter buffer is full\n" );

						info_print( INFO_BUFFER_ADD, "Parameter buffer is full\n" );

						pointer	= rem_size;

						break;

					}

					para_info->kind	= PARAMETER_KIND_STRING;

					return_value	= get_para_from_buffer( buffer + pointer, para_string, parameter_buffer + para_buffer_pointer );

					para_info->container.string = parameter_buffer + para_buffer_pointer;

					para_buffer_pointer += char_count; 

				}
				else
				{

					para_info->kind	= PARAMETER_KIND_NUMBER;

					return_value	= get_para_from_buffer( buffer + pointer, para_string, &para_info->container.number );

				}

				if( !return_value )
				{

					if( valid_command->parameter_count == COMMAND_N_PARAMENTER )
					{

						printk( "TTUCH : Parameter count(%d)\n", loop );

						info_print( INFO_BUFFER_ADD, "Parameter count(%d)\n", loop );

						break;

					}

					printk( "TTUCH : Get parameter failed\n" );

					info_print( INFO_BUFFER_ADD, "Get parameter failed\n" );

					pointer	= rem_size;

					break;

				}

				if( !char_count )
				{

					printk( "TTUCH : Can't get charactor count..\n" );

					info_print( INFO_BUFFER_ADD, "Can't get charactor count..\n" );

					pointer	= rem_size;

					break;

				}

				pointer += char_count;

			}

			if( valid_command->parameter_count == COMMAND_N_PARAMENTER || loop == command_parameter_count )
				valid_command->function( node_data, &parameter );

		}

	}

	return size;

}

// ==========================================================================================================================

ssize_t	tool_info( void *node_data, struct device_attribute *attribute, char *buffer )
{

	enum buffer_kind	kind = INFORMATION_BUFFER;

	unsigned int		count = 0;

	char			*source = get_buffer_address( ( struct touch_tool_data* )node_data, &kind, &count );

	if( kind == INFORMATION_BUFFER )
		return	snprintf( buffer, PAGE_SIZE, source );
	else
	{

		if( count > PAGE_SIZE )
			return	0;

		memcpy( buffer, source, count );

		return	count;

	}

}
