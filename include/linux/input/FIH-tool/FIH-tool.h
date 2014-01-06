
#pragma once

#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/init.h>

#include <linux/mutex.h>

#include <linux/platform_device.h>

#include <linux/slab.h>

#include <linux/input/rmi.h>

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

};

struct	file_information
{

	char		name[ 256 ];

	struct file	*image_file;

};

struct	touch_tool_data
{

	struct rmi_device		*rmi_dev;

	struct control_class_device	*file_node_class;

	struct flash_information	*flash_data;

	struct file_information		*file_data;

	struct command_info		command;

	char	*info_buffer, *parameter_buffer, *block_data_buffer;

	int	command_id;

};
