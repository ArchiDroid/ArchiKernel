
#pragma once

#include <linux/platform_device.h>

#ifdef	CONFIG_FIH_TOUCH_TOOLS
	#define	REGISTER_TOUCH_TOOL_DRIVER( DATA ) \
	{ \
		static struct platform_device FIH_touch_tool; \
		FIH_touch_tool.name = "fih_touch_tools", FIH_touch_tool.id = -1; \
		FIH_touch_tool.dev.platform_data = DATA; \
		platform_device_register( &FIH_touch_tool ); \
		printk( "TTUCH : Register touch tool\n" ); \
	}
#else
	#define	REGISTER_TOUCH_TOOL_DRIVER( DATA )
#endif
