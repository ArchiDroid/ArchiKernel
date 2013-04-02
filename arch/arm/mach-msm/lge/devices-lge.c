#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/android_pmem.h>
#include <linux/gpio.h>
//LGE_CHANGE_S jongjin7.park@lge.com for frst 2012-11-22 
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_V1)
#include <linux/export.h>
#endif
//LGE_CHANGE_E jongjin7.park
#include <asm/mach-types.h>

#ifdef CONFIG_ANDROID_RAM_CONSOLE
#include <asm/setup.h>
#endif

#ifdef CONFIG_LGE_POWER_ON_STATUS_PATCH
#include <mach/msm_smsm.h>
#include <asm/processor.h>
#endif

#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include CONFIG_LGE_BOARD_HEADER_FILE
#include <mach/lge/lge_pm.h>

/* setting whether uart console is enabled or disabled */
static int uart_console_mode = 0;
static int reboot_mode = 0;

int __init lge_get_uart_mode(void)
{
	return uart_console_mode;
}

static int __init lge_uart_mode(char *uart_mode)
{
	if (!strncmp("enable", uart_mode, 5)) {
		printk(KERN_INFO "UART CONSOLE : enable\n");
		uart_console_mode = 1;
	} else
		printk(KERN_INFO "UART CONSOLE : disable\n");

	return 1;
}

__setup("uart_console=", lge_uart_mode);

#ifdef CONFIG_LGE_HW_REVISION

/* setting board revision information */
lge_pcb_rev  lge_bd_rev = 0;

lge_pcb_rev  lge_get_board_revno(void)
{
	return lge_bd_rev;
}

static int __init board_revno_setup(char *rev_info)
{
	char *rev_str[] = { "evb", "rev_a", "rev_b", "rev_c", "rev_d", "rev_e",
		"rev_f", "rev_g", "rev_10", "rev_11", "rev_12" ,"reserved"};
	int i;

	lge_bd_rev = LGE_REV_TOT_NUM;

	for (i = 0; i < LGE_REV_TOT_NUM; i++)
		if (!strncmp(rev_info, rev_str[i], 6)) {
			lge_bd_rev = i;
			break;
		}

	printk(KERN_INFO"BOARD: LGE %s\n", rev_str[lge_bd_rev]);
	return 1;
}

__setup("lge.rev=", board_revno_setup);

#endif

/* setting whether factory reset is */
static char frst_mode[6];

static int __init lge_frst_mode(char *cmdline)
{
	strncpy(frst_mode, cmdline, 5);
	frst_mode[5]='\0';
	printk(KERN_INFO "FRST MODE : %s",frst_mode);

	return 1;
}

__setup("lge.frst=", lge_frst_mode);

//LGE_CHANGE_S jongjin7.park@lge.com for frst 2012-11-22 
#if defined(CONFIG_MACH_MSM7X27A_U0) || defined(CONFIG_MACH_MSM7X25A_V1)
char* get_frst_mode(void)
{
	return frst_mode;
}
EXPORT_SYMBOL(get_frst_mode);
#endif


/* LGE_CHANGE_S,narasimha.chikka@lge.com,Add BATT_ID Check */
#ifdef CONFIG_LGE_PM_BATT_ID_DETECTION

static char battery_id[10];

static int __init lge_battery_id(char *battery_id_cmd)
{
	strncpy(battery_id, battery_id_cmd, 9);
	battery_id[9]='\0';
	printk(KERN_INFO "BATT_ID : %s",battery_id);

	return 1;
}

__setup("lge.batt_id_info=", lge_battery_id);

char	 *lge_get_battery_id(void)
{
	return battery_id;
}

//EXPORT_SYMBOL(lge_get_battery_id);

#endif

/* LGE_CHANGE_S,narasimha.chikka@lge.com,Add BATT_ID Check */

//LGE_CHANGE_E jongjin7.park
/* LGE_CHANGE_S: murali.ramaiah@lge.com [2011-09-22]
	Read power on status from modem, and update boot reason.
	Ref:- Documentation\arm\msm\boot.txt
*/
#ifdef CONFIG_LGE_POWER_ON_STATUS_PATCH
void __init lge_board_pwr_on_status(void)
{
	unsigned smem_size;
	boot_reason = *(unsigned int *)
		(smem_get_entry(SMEM_POWER_ON_STATUS_INFO, &smem_size));
	/* filter pwr on status byte */
	boot_reason &= 0xFF;
	printk(KERN_NOTICE "Boot Reason = 0x%02x\n", boot_reason);
}
#endif /* CONFIG_LGE_POWER_ON_STATUS_PATCH */

/* LGE_CHANGE_S, [20121110][youngbae.choi@lge.com] */
static int __init panicmode_setup(char *arg)
{	
	if ((!strcmp(arg, "soff_pon")) || (!strcmp(arg, "son_pon")))
			set_kernel_panicmode(2);

#ifdef CONFIG_LGE_SILENCE_RESET
	if (!strcmp(arg, "son_pon"))
			set_kernel_panicmode(0);
	
	if ((!strcmp(arg, "son_pon")) || (!strcmp(arg, "son_poff"))){
			set_kernel_silencemode(1);
			lge_silence_reset_f(1);
	}
	else{
			set_kernel_silencemode(0);
			lge_silence_reset_f(0);
	}

	printk(KERN_INFO " panicmode_setup , panicmode = %d, silencemode = %d \n", get_kernel_panicmode(), get_kernel_silencemode());
#endif

	return 1;
}
__setup("lge.panicmode=", panicmode_setup);

static int atoi(const char *name)
{
	int val = 0;

	for (;; name++) {
		switch (*name) {
		case '0' ... '9':
			val = 10*val+(*name-'0');
			break;
		default:
			return val;
		}
	}
}

static int __init rebootmode_setup(char *arg)
{	
	reboot_mode = atoi(arg);
	printk(KERN_INFO " rebootmode_setup , reboot_mode = %x \n", reboot_mode);

	return 1;
}
__setup("lge.reboot=", rebootmode_setup);

//LGE_CHANGE_S FTM boot mode
#if (defined (CONFIG_MACH_MSM7X25A_V3) && !defined (CONFIG_MACH_MSM7X25A_M4)) || defined (CONFIG_MACH_MSM8X25_V7) || defined(CONFIG_MACH_MSM7X25A_V1)
static enum lge_fboot_mode_type lge_fboot_mode = second_boot;
int __init lge_fboot_mode_init(char *s)
{
	if (!strcmp(s, "FirstBoot"))
		lge_fboot_mode = first_boot;
	else if (!strcmp(s, "SecondBoot"))
		lge_fboot_mode = second_boot;
	else
		lge_fboot_mode = second_boot;

	printk(KERN_INFO "ANDROID FBOOT MODE : %d \n", lge_fboot_mode);

	return 1;
}
__setup("androidboot.mode=", lge_fboot_mode_init);

enum lge_fboot_mode_type lge_get_fboot_mode(void)
{
	return lge_fboot_mode;
}
#endif
//LGE_CHANGE_E FTM boot mode

int get_reboot_mode(void)
{
	return reboot_mode;
}
/* LGE_CHANGE_E, [20121110][youngbae.choi@lge.com] */

#ifdef CONFIG_LGE_BOOT_MODE
static struct platform_device lge_boot_mode_device = {
	.name = LGE_BOOT_MODE_DEVICE,
	.id = -1,
	.dev = {
		.platform_data = NULL,
	},
};

void __init lge_add_boot_mode_devices(void)
{
	platform_device_register(&lge_boot_mode_device);
}
#endif

#if defined(CONFIG_ANDROID_RAM_CONSOLE) && defined(CONFIG_LGE_HANDLE_PANIC)
static struct resource crash_log_resource[] = {
	{
		.name = "crash_log",
		.flags = IORESOURCE_MEM,
	}
};

static struct platform_device panic_handler_device = {
	.name = "panic-handler",
	.num_resources = ARRAY_SIZE(crash_log_resource),
	.resource = crash_log_resource,
	.dev    = {
		.platform_data = NULL,
	}
};

void __init lge_add_panic_handler_devices(void)
{
	struct resource *res = crash_log_resource;
	struct membank *bank = &meminfo.bank[0];

	res->start = bank->start + bank->size + LGE_RAM_CONSOLE_SIZE;
	res->end = res->start + LGE_CRASH_LOG_SIZE - 1;

	printk(KERN_INFO "CRASH LOG START ADDR : 0x%x\n", res->start);
	printk(KERN_INFO "CRASH LOG END ADDR   : 0x%x\n", res->end);

	platform_device_register(&panic_handler_device);
}
#endif /*CONFIG_ANDROID_RAM_CONSOLE && CONFIG_LGE_HANDLE_PANIC*/

// LGE_CHANGE_S, narasimha.chikka@lge.com,Add pm device
static struct platform_device lge_pm_device = {
	.name = LGE_PM_DEVICE,
	.id      = -1,
	.dev   = {
		.platform_data = NULL,
	}
};

void __init lge_add_pm_devices(void)
{
	platform_device_register(&lge_pm_device);
}
// LGE_CHANGE_E, narasimha.chikka@lge.com,Add pm device

/* lge gpio i2c device */
#define MAX_GPIO_I2C_DEV_NUM		10
#define LOWEST_GPIO_I2C_BUS_NUM		2

static gpio_i2c_init_func_t *i2c_init_func[MAX_GPIO_I2C_DEV_NUM] __initdata;
static int i2c_dev_num __initdata;

void __init lge_add_gpio_i2c_device(gpio_i2c_init_func_t *init_func)
{
	i2c_init_func[i2c_dev_num] = init_func;
	i2c_dev_num++;
}

void __init lge_add_gpio_i2c_devices(void)
{
	int index;
	gpio_i2c_init_func_t *init_func_ptr;

	for (index = 0; index < i2c_dev_num; index++) {
		init_func_ptr = i2c_init_func[index];
		(*init_func_ptr)(LOWEST_GPIO_I2C_BUS_NUM + index);
	}
}

int __init lge_init_gpio_i2c_pin(struct i2c_gpio_platform_data *i2c_adap_pdata,
		struct gpio_i2c_pin gpio_i2c_pin,
		struct i2c_board_info *i2c_board_info_data)
{
	i2c_adap_pdata->sda_pin = gpio_i2c_pin.sda_pin;
	i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;

	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_CFG_OUTPUT,
		GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_CFG_OUTPUT,
		GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(gpio_i2c_pin.sda_pin, 1);
	gpio_set_value(gpio_i2c_pin.scl_pin, 1);

	if (gpio_i2c_pin.reset_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.reset_pin, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.reset_pin, 1);
	}

	if (gpio_i2c_pin.irq_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.irq_pin, 0,
			GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
		i2c_board_info_data->irq =
			MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
	}

	return 0;
}

int lge_init_gpio_i2c_pin_pullup(struct i2c_gpio_platform_data *i2c_adap_pdata,
		struct gpio_i2c_pin gpio_i2c_pin,
		struct i2c_board_info *i2c_board_info_data)
{
	i2c_adap_pdata->sda_pin = gpio_i2c_pin.sda_pin;
	i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;

	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_CFG_OUTPUT,
		GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_CFG_OUTPUT,
		GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(gpio_i2c_pin.sda_pin, 1);
	gpio_set_value(gpio_i2c_pin.scl_pin, 1);

	if (gpio_i2c_pin.reset_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.reset_pin, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.reset_pin, 1);
	}

	if (gpio_i2c_pin.irq_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.irq_pin, 0,
			GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
		i2c_board_info_data->irq =
			MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
	}

	return 0;
}

