
#pragma once

#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/init.h>

#include <linux/mutex.h>

#include <linux/platform_device.h>

#include <linux/slab.h>

#include <linux/input/synaptics_dsx_rmi4_i2c.h>

#include <linux/command_maker.h>

// ==========================================================================================================================

#define	INFO_BUFFER_SIZE	4096

#define	PARA_BUFFER_SIZE	1024

#define	BLOCK_DATA_BUFFER_SIZE	55

// ==========================================================================================================================

#define	INFO_BUFFER_RESET	0

#define	INFO_BUFFER_INIT	1

#define	INFO_BUFFER_ADD		2

// ==========================================================================================================================

struct	flash_information
{

	u8	*buffer;

	unsigned int	size;

	unsigned int	bootloader_mode : 1;

};

struct	file_information
{

	char		name[ 256 ];

	struct file	*image_file;

};

struct	rmi_function_descriptor
{

	u16	query_base_addr;

	u16	command_base_addr;

	u16	control_base_addr;

	u16	data_base_addr;

	u8	interrupt_source_count;

	u8	function_number;

	u8	function_version;

};

struct	rmi_function_information
{

	struct rmi_function_descriptor	descriptor;

	struct list_head	link;

};

struct	touch_command_information
{

	char	*string;

	unsigned int	command_id;

};

struct	command_string
{

	struct touch_command_information	*list;

	unsigned int	count;

};

struct	touch_tool_data
{

	struct synaptics_rmi4_data	*rmi_data;

	struct control_class_device	*file_node_class;

	struct flash_information	*flash_data;

	struct file_information		*file_data, *configuration_file, *firmware_file;

	struct command_info		command;

	struct command_string		string;

	char	*info_buffer, *parameter_buffer, *block_data_buffer;

	struct list_head		descriptor_list;

	int	command_id;

};

// ==========================================================================================================================

static inline int	rmi_read( struct synaptics_rmi4_data *rmi_data, u16 address, u8 *buffer )
{

	return	rmi_data->i2c_read( rmi_data, address, buffer, 1 );

}

static inline int	rmi_read_block( struct synaptics_rmi4_data *rmi_data, u16 address, u8 *buffer, int length )
{

	return	rmi_data->i2c_read( rmi_data, address, buffer, length );

}

static inline int	rmi_write( struct synaptics_rmi4_data *rmi_data, u16 address, u8 data )
{

	return	rmi_data->i2c_write( rmi_data, address, &data, 1 );

}

static inline int	rmi_write_block( struct synaptics_rmi4_data *rmi_data, u16 address, u8 *buffer, int length )
{

	return	rmi_data->i2c_write( rmi_data, address, buffer, length );

}
