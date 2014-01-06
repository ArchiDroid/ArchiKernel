
#include <linux/input/FIH-tool/FIH-touch-tool.h>

#include <linux/input/FIH-tool/touch-tool_functions.h>

#include <linux/development_tool.h>

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

		kfree( tool_data );

		return	NULL;

	}

	if( ( tool_data->parameter_buffer = kzalloc( PARA_BUFFER_SIZE, GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Get memory of parameter buffer failed\n" );

		kfree( tool_data->info_buffer );

		kfree( tool_data );

		return	NULL;

	}

	if( ( tool_data->block_data_buffer = kzalloc( BLOCK_DATA_BUFFER_SIZE, GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : Get memory of block data buffer failed\n" );

		kfree( tool_data->parameter_buffer );

		kfree( tool_data->info_buffer );

		kfree( tool_data );

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

bool	create_command_string_list( struct command_string *string_info )
{

	struct touch_command_information	touch_command[] = { COMMAND_STRING_LIST };

	struct BST_data	*sort_buffer;

	struct BS_data	*sort_temp;

	struct BST_info	sort_info;

	int	loop;

	if( ( sort_buffer = kzalloc( sizeof( struct BST_data ) * sizeof( touch_command ) / sizeof( *touch_command ), GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : no memory for sort buffer of command string\n" );

		return	false;

	}

	if( ( sort_temp = kzalloc( sizeof( struct BS_data ) * sizeof( touch_command ) / sizeof( *touch_command ), GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : no memory for sort buffer of command string\n" );

		kfree( sort_buffer );

		return	false;

	}

	if( ( string_info->list = kzalloc( sizeof( struct touch_command_information ) * sizeof( touch_command ) / sizeof( *touch_command ), GFP_KERNEL ) ) == NULL )
	{

		printk( "TTUCH : no memory for data of command string\n" );

		kfree( sort_buffer );

		kfree( sort_temp );

		return	false;

	}

	BST_init( &sort_info );

	for( loop = 0 ; loop < sizeof( touch_command ) / sizeof( *touch_command ) ; ++loop )
	{

		struct touch_command_information	*info = touch_command + loop;

		struct BST_data				*sort_data = sort_buffer + loop;

		if( !info->string )
		{

			printk( "TTUCH : The string of command ID(%d) is NULL\n", info->command_id );

			continue;

		}

		sort_data->index = info->command_id, sort_data->data = info->string;

		BST_add( &sort_info, sort_data );

	}

	string_info->count	= BST_sort( &sort_info, sort_temp, sizeof( touch_command ) / sizeof( *touch_command ) );

	kfree( sort_buffer );

	if( !string_info->count )
	{

		printk( "TTUCH : Sort command failed\n" );

		kfree( string_info->list );

		kfree( sort_temp );

		return	false;

	}

	for( loop = 0 ; loop < string_info->count ; ++loop )
	{

		struct touch_command_information	*info = string_info->list + loop;

		struct BS_data				*sort_data = sort_temp + loop;

		info->string = sort_data->data, info->command_id = sort_data->index;

	}

	kfree( sort_temp );

	return	true;

}