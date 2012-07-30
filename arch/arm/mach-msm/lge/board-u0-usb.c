#include <linux/init.h>

#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/msm_hsusb.h>
#include <mach/rpc_hsusb.h>
#include <mach/usbdiag.h>

#include <linux/usb/android.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <mach/vreg.h>

/*LGE_CHANGE_S : seven.kim@lge.com to use regulator*/
#include <linux/regulator/consumer.h>
/*LGE_CHANGE_E : seven.kim@lge.com to use regulator*/

#include "devices.h"
#include "devices-msm7x2xa.h"
#include <mach/pm.h>

/*LGE_CHANGE_S : seven.kim@lge.com kernel3.0 porting*/
#include "pm-boot.h"
/*LGE_CHANGE_E : seven.kim@lge.com kernel3.0 porting*/
#include <mach/board_lge.h>

#include "board-u0.h"

#ifdef CONFIG_USB_EHCI_MSM_72K
static void msm_hsusb_vbus_power(unsigned phy_info, int on)
{
	int rc = 0;
	unsigned gpio;

	gpio = GPIO_HOST_VBUS_EN;

	rc = gpio_request(gpio, "i2c_host_vbus_en");
	if (rc < 0) {
		pr_err("failed to request %d GPIO\n", gpio);
		return;
	}
	gpio_direction_output(gpio, !!on);
	gpio_set_value_cansleep(gpio, !!on);
	gpio_free(gpio);
}

static struct msm_usb_host_platform_data msm_usb_host_pdata = {
	.phy_info       = (USB_PHY_INTEGRATED | USB_PHY_MODEL_45NM),
};

static void __init msm7x2x_init_host(void)
{
	msm_add_host(0, &msm_usb_host_pdata);
}
#endif

#ifdef CONFIG_USB_MSM_OTG_72K
static int hsusb_rpc_connect(int connect)
{
	if (connect)
		return msm_hsusb_rpc_connect();
	else
		return msm_hsusb_rpc_close();
}

/*LGE_CHANGE_S : seven.kim@lge.com changed the USB LDO contorl based on board-msm7227a.c*/
static struct regulator *reg_hsusb;
static int msm_hsusb_ldo_init(int init)
{
	int rc = 0;

	if (init) {
		reg_hsusb = regulator_get(NULL, "usb");
		if (IS_ERR(reg_hsusb)) {
			rc = PTR_ERR(reg_hsusb);
			pr_err("%s: could not get regulator: %d\n",
					__func__, rc);
			goto out;
		}

		rc = regulator_set_voltage(reg_hsusb, 3300000, 3300000);
		if (rc) {
			pr_err("%s: could not set voltage: %d\n",
					__func__, rc);
			goto reg_free;
		}

		return 0;
	}
	/* else fall through */
reg_free:
	regulator_put(reg_hsusb);
out:
	reg_hsusb = NULL;
	return rc;
}

static int msm_hsusb_ldo_enable(int enable)
{
	static int ldo_status;

	if (IS_ERR_OR_NULL(reg_hsusb))
		return reg_hsusb ? PTR_ERR(reg_hsusb) : -ENODEV;

	if (ldo_status == enable)
		return 0;

	ldo_status = enable;

	return enable ?
		regulator_enable(reg_hsusb) :
		regulator_disable(reg_hsusb);
}
/*LGE_CHANGE_E : seven.kim@lge.com changed the USB LDO contorl based on board-msm7227a.c*/

#ifndef CONFIG_USB_EHCI_MSM_72K
static int msm_hsusb_pmic_notif_init(void (*callback)(int online), int init)
{
	int ret = 0;

	if (init)
		ret = msm_pm_app_rpc_init(callback);
	else
		msm_pm_app_rpc_deinit(callback);

	return ret;
}
#endif

static struct msm_otg_platform_data msm_otg_pdata = { 
#ifndef CONFIG_USB_EHCI_MSM_72K
	.pmic_vbus_notif_init	 = msm_hsusb_pmic_notif_init,
#else
	.vbus_power		 = msm_hsusb_vbus_power,
#endif
	.rpc_connect		 = hsusb_rpc_connect,
	.pemp_level		 = PRE_EMPHASIS_WITH_20_PERCENT,
	.cdr_autoreset		 = CDR_AUTO_RESET_DISABLE,
	.drv_ampl		 = HS_DRV_AMPLITUDE_DEFAULT,
	.se1_gating		 = SE1_GATING_DISABLE,
	.ldo_init		 = msm_hsusb_ldo_init,
	.ldo_enable		 = msm_hsusb_ldo_enable,
	.chg_init		 = hsusb_chg_init,
	.chg_connected		 = hsusb_chg_connected,
	.chg_vbus_draw		 = hsusb_chg_vbus_draw,
};
#endif
/*LGE_CHANGE_S: jyothishre.nk@lge.com 2012-03-20
 *This structure is also used in board-u0-mmc.c*/
struct msm_pm_platform_data msm7x27a_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 16000,
					.residency = 20000,
	},
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 12000,
					.residency = 20000,
	},
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 0,
					.suspend_enabled = 1,
					.latency = 2000,
					.residency = 0,
	},
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 2,
					.residency = 0,
	},
};
EXPORT_SYMBOL(msm7x27a_pm_data);
/*LGE_CHANGE_E:This symbol exported to use in board-u0-mmc.c file*/

static struct android_usb_platform_data android_usb_pdata = {
	.update_pid_and_serial_num = usb_diag_update_pid_and_serial_num,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data = &android_usb_pdata,
	},
};

static struct platform_device *u0_usb_devices[] __initdata = {
	&msm_device_otg,
	&msm_device_gadget_peripheral,
	&android_usb_device,
};

static struct msm_hsusb_gadget_platform_data msm_gadget_pdata = {
/*LGE_CHANGE_SE : hyunjin2.lim@lge.com 1 ->0 */
	.is_phy_status_timer_on = 0,
};

/*LGE_CHANGE_S : seven.kim@lge.com kernel3.0 porting
 * qct ES4 structure data type was changed in ES4
 */
 static struct msm_pm_boot_platform_data msm_pm_boot_pdata __initdata = {
	.mode = MSM_PM_BOOT_CONFIG_RESET_VECTOR_PHYS,
	.p_addr = 0,
};
/*LGE_CHANGE_E : seven.kim@lge.com kernel3.0 porting*/

void __init lge_add_usb_devices(void)
{
	
#ifdef CONFIG_USB_MSM_OTG_72K
	msm_otg_pdata.swfi_latency =
		msm7x27a_pm_data
		[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;
	msm_device_otg.dev.platform_data = &msm_otg_pdata;
#endif

	msm_device_gadget_peripheral.dev.platform_data =
		&msm_gadget_pdata;

	platform_add_devices(u0_usb_devices, ARRAY_SIZE(u0_usb_devices));
#ifdef CONFIG_USB_EHCI_MSM_72K
	msm7x2x_init_host();
#endif
	msm_pm_set_platform_data(msm7x27a_pm_data,
			ARRAY_SIZE(msm7x27a_pm_data));
	/*LGE_CHANGE_S : seven.kim@lge.com kernel3.0 porting 
	 * newly added pm initialize function in kernel3.0.8
	 * BUG_FIX : If we don't add msm_pm_boot_init() , then system was reset when booting with only battery
	 */
	BUG_ON(msm_pm_boot_init(&msm_pm_boot_pdata));
	/*LGE_CHANGE_E : seven.kim@lge.com kernel3.0 porting*/
}

