#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/err.h>

#include <asm/mach/mmc.h>
#include <mach/vreg.h>
#include <mach/board.h>
/*[U0][Kernel]Integrating MMC voltage regulator jyothishre.nk@lge.com [START]*/ 
#include <linux/regulator/consumer.h>
/*[END]*/
#include "board-u0.h"

#include <mach/pm.h>

#if (defined(CONFIG_MMC_MSM_SDC1_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC2_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC3_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC4_SUPPORT))

#define MAX_SDCC_CONTROLLER 4
static unsigned long vreg_sts, gpio_sts;
struct sdcc_gpio {
	struct msm_gpio *cfg_data;
	uint32_t size;
	struct msm_gpio *sleep_cfg_data;
};
/*LGE_CHANGE_S:Introducing latency to prevent Idle power collapse
*[jyothishre.nk@lge.com] 2012-03-20*/
extern struct msm_pm_platform_data msm7x27a_pm_data[MSM_PM_SLEEP_MODE_NR];

/*This is for SD card failure issue during resume time.
*The reason for resume failure is that idle power collapse.
*By introducing latency Idle power collapse is prevented*/
u32 msm7627a_power_collapse_latency(enum msm_pm_sleep_mode mode)
{
        switch (mode) {
        case MSM_PM_SLEEP_MODE_POWER_COLLAPSE:
                return msm7x27a_pm_data
                [MSM_PM_SLEEP_MODE_POWER_COLLAPSE].latency;

        case MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN:
                return msm7x27a_pm_data
                [MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].latency;

        case MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT:
                return msm7x27a_pm_data
                [MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;

        case MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT:
                return msm7x27a_pm_data
                [MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].latency;

        default:
                return 0;

        }
}
/*LGE_CHANGE_E:*/

/* LGE_CHANGE_S : SD card detect GPIO configuration
	 * 2012-02-02, jyothishre.nk@lge.com
	 * SD card GPIO configuration and detection is handled in
	 * msm7627a_sdcc_slot_status
	 */
/*static void sdcc_gpio_init(void)
{
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	int rc = 0;

	if (gpio_request(GPIO_SD_DETECT_N, "sdc1_status_irq"))
		pr_err("failed to request gpio sdc1_status_irq\n");

	rc = gpio_tlmm_config(GPIO_CFG(GPIO_SD_DETECT_N, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO %d\n",
					__func__, rc);
#endif
}*/
/* LGE_CHANGE_E : SD card detect GPIO configuration */
/**
 * Due to insufficient drive strengths for SDC GPIO lines some old versioned
 * SD/MMC cards may cause data CRC errors. Hence, set optimal values
 * for SDC slots based on timing closure and marginality. SDC1 slot
 * require higher value since it should handle bad signal quality due
 * to size of T-flash adapters.
 */
static struct msm_gpio sdc1_cfg_data[] = {
	{GPIO_CFG(51, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),
								"sdc1_dat_3"},
	{GPIO_CFG(52, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),
								"sdc1_dat_2"},
	{GPIO_CFG(53, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),
								"sdc1_dat_1"},
	{GPIO_CFG(54, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),
								"sdc1_dat_0"},
	{GPIO_CFG(55, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),
								"sdc1_cmd"},
	{GPIO_CFG(56, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA),
								"sdc1_clk"},
};

static struct msm_gpio sdc2_cfg_data[] = {
	{GPIO_CFG(62, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
								"sdc2_clk"},
	{GPIO_CFG(63, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc2_cmd"},
	{GPIO_CFG(64, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc2_dat_3"},
	{GPIO_CFG(65, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc2_dat_2"},
	{GPIO_CFG(66, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc2_dat_1"},
	{GPIO_CFG(67, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc2_dat_0"},
};

static struct msm_gpio sdc2_sleep_cfg_data[] = {
	{GPIO_CFG(62, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
								"sdc2_clk"},
	{GPIO_CFG(63, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
								"sdc2_cmd"},
	{GPIO_CFG(64, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
								"sdc2_dat_3"},
	{GPIO_CFG(65, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
								"sdc2_dat_2"},
	{GPIO_CFG(66, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
								"sdc2_dat_1"},
	{GPIO_CFG(67, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
								"sdc2_dat_0"},
};

static struct msm_gpio sdc3_cfg_data[] = {
	{GPIO_CFG(88, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
								"sdc3_clk"},
	{GPIO_CFG(89, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_cmd"},
	{GPIO_CFG(90, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_3"},
	{GPIO_CFG(91, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_2"},
	{GPIO_CFG(92, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_1"},
	{GPIO_CFG(93, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_0"},
#ifdef CONFIG_MMC_MSM_SDC3_8_BIT_SUPPORT
	{GPIO_CFG(19, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_7"},
	{GPIO_CFG(20, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_6"},
	{GPIO_CFG(21, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_5"},
	{GPIO_CFG(108, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_4"},
#endif
};

static struct msm_gpio sdc3_sleep_cfg_data[] = {
	{GPIO_CFG(88, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
								"sdc3_clk"},
	{GPIO_CFG(89, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_cmd"},
	{GPIO_CFG(90, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_3"},
	{GPIO_CFG(91, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_2"},
	{GPIO_CFG(92, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_1"},
	{GPIO_CFG(93, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_0"},
#ifdef CONFIG_MMC_MSM_SDC3_8_BIT_SUPPORT
	{GPIO_CFG(19, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_7"},
	{GPIO_CFG(20, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_6"},
	{GPIO_CFG(21, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_5"},
	{GPIO_CFG(108, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_4"},
#endif
};

static struct msm_gpio sdc4_cfg_data[] = {
	{GPIO_CFG(19, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc4_dat_3"},
	{GPIO_CFG(20, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc4_dat_2"},
	{GPIO_CFG(21, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc4_dat_1"},
	{GPIO_CFG(107, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc4_cmd"},
	{GPIO_CFG(108, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc4_dat_0"},
	{GPIO_CFG(109, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
								"sdc4_clk"},
};

static struct sdcc_gpio sdcc_cfg_data[] = {
	{
		.cfg_data = sdc1_cfg_data,
		.size = ARRAY_SIZE(sdc1_cfg_data),
	},
	{
		.cfg_data = sdc2_cfg_data,
		.size = ARRAY_SIZE(sdc2_cfg_data),
		.sleep_cfg_data = sdc2_sleep_cfg_data,
	},
	{
		.cfg_data = sdc3_cfg_data,
		.size = ARRAY_SIZE(sdc3_cfg_data),
		.sleep_cfg_data = sdc3_sleep_cfg_data,
	},
	{
		.cfg_data = sdc4_cfg_data,
		.size = ARRAY_SIZE(sdc4_cfg_data),
	},
};
/*
static int gpio_sdc1_hw_det = 85;
static void gpio_sdc1_config(void)
{
	if (machine_is_msm7627a_qrd1())
		gpio_sdc1_hw_det = 42;
}
*/

static struct regulator *sdcc_vreg_data[MAX_SDCC_CONTROLLER];

static int msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int rc = 0;
	struct sdcc_gpio *curr;

	curr = &sdcc_cfg_data[dev_id - 1];
	if (!(test_bit(dev_id, &gpio_sts)^enable))
		return rc;

	if (enable) {
		set_bit(dev_id, &gpio_sts);
		rc = msm_gpios_request_enable(curr->cfg_data, curr->size);
		if (rc)
			pr_err("%s: Failed to turn on GPIOs for slot %d\n",
					__func__,  dev_id);
	} else {
		clear_bit(dev_id, &gpio_sts);
		if (curr->sleep_cfg_data) {
			rc = msm_gpios_enable(curr->sleep_cfg_data, curr->size);
			msm_gpios_free(curr->sleep_cfg_data, curr->size);
			return rc;
		}
		msm_gpios_disable_free(curr->cfg_data, curr->size);
	}
	return rc;
}

static int msm_sdcc_setup_vreg(int dev_id, unsigned int enable)
{
	int rc = 0;
/*[U0][Kernel]Integrating MMC voltage regulator jyothishre.nk@lge.com [START]*/
	struct regulator *curr = sdcc_vreg_data[dev_id - 1];


	if (test_bit(dev_id, &vreg_sts)==enable)
		return 0;

	 if (!curr)
                return -ENODEV;

        if (IS_ERR(curr))
                return PTR_ERR(curr);

	if (dev_id==1 && enable) {
		set_bit(dev_id, &vreg_sts);

		rc = regulator_enable(curr);

		if (rc)
			pr_err("%s: could not enable regulator: %d\n", __func__, rc);
	}/*else {
		clear_bit(dev_id, &vreg_sts);
		rc = regulator_disable(curr);
		if (rc)
			pr_err("%s: could not disableregulator: %d\n", __func__, rc);
	}*/
/*[END]*/
	return rc;
}

static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;
	static int first_setup = 1;

	pdev = container_of(dv, struct platform_device, dev);

	rc = msm_sdcc_setup_gpio(pdev->id, !!vdd);
	if (rc)
		goto out;

	rc = msm_sdcc_setup_vreg(pdev->id, !!vdd);

	/* if first called related to sdcc1,
	 *  irq should be registered as wakeup source
	 * cleaneye.kim@lge.com, 2010-02-19
	 */
	if (vdd && first_setup) {
		struct mmc_platform_data *pdata = pdev->dev.platform_data;
		if (pdev->id == 1) {
			first_setup = 0;
			irq_set_irq_wake(pdata->status_irq, 1);
		}
	}
out:
	return rc;
}

#if defined(CONFIG_MMC_MSM_SDC1_SUPPORT) \
	&& defined(CONFIG_MMC_MSM_CARD_HW_DETECTION)
static unsigned int msm7627a_sdcc_slot_status(struct device *dev)
{
/* LGE_CHANGE_S : SD card detect GPIO configuration
	 * 2012-02-02, jyothishre.nk@lge.com
	 * SD card GPIO configuration and detection is handled here
	 */
	int status;

	status = gpio_tlmm_config(GPIO_CFG(GPIO_SD_DETECT_N, 0, GPIO_CFG_INPUT,
                        GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (status)
                pr_err("%s:Failed to configure tlmm for GPIO %d\n", __func__,
                                GPIO_SD_DETECT_N);
	
	status = gpio_request(GPIO_SD_DETECT_N, "sdc1_status_irq");
        if (status)
                pr_err("%s:Failed to request GPIO %d\n", __func__,
                                GPIO_SD_DETECT_N);

	status = gpio_direction_input(GPIO_SD_DETECT_N);
	if (!status)
		status = !gpio_get_value(GPIO_SD_DETECT_N);

	gpio_free(GPIO_SD_DETECT_N);

	return status;
//	return !gpio_get_value(GPIO_SD_DETECT_N);
/* LGE_CHANGE_E : SD card detect GPIO configuration */
}
#endif

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static struct mmc_platform_data sdc1_plat_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd  = msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.status      = msm7627a_sdcc_slot_status,
	.status_irq  = MSM_GPIO_TO_INT(GPIO_SD_DETECT_N),
	.irq_flags   = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
#endif
};
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
/* FIXME : u0 evb GPIO 66 WLAN SDIO PIN */
static struct mmc_platform_data sdc2_plat_data = {
	/*
	 * SDC2 supports only 1.8V, claim for 2.85V range is just
	 * for allowing buggy cards who advertise 2.8V even though
	 * they can operate at 1.8V supply.
	 */
	.ocr_mask	= MMC_VDD_28_29 | MMC_VDD_165_195,
	.translate_vdd  = msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#ifdef CONFIG_MMC_MSM_SDIO_SUPPORT
	.sdiowakeup_irq = MSM_GPIO_TO_INT(66),
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
#ifdef CONFIG_MMC_MSM_SDC2_DUMMY52_REQUIRED
	.dummy52_required = 1,
#endif
};
#endif

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
static struct mmc_platform_data sdc3_plat_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd  = msm_sdcc_setup_power,
#ifdef CONFIG_MMC_MSM_SDC3_8_BIT_SUPPORT
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
#else
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 1,
};
#endif

#if (defined(CONFIG_MMC_MSM_SDC4_SUPPORT)\
		&& !defined(CONFIG_MMC_MSM_SDC3_8_BIT_SUPPORT))
static struct mmc_platform_data sdc4_plat_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd  = msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
};
#endif
#endif
/*[U0][Kernel]Integrating MMC voltage regulator jyothishre.nk@lge.com [START]*/
static int __init mmc_regulator_init(int sdcc_no, const char *supply, int uV)
{
	int rc =0;
    BUG_ON(sdcc_no < 1 || sdcc_no > 4);
	sdcc_no--;
	sdcc_vreg_data[sdcc_no] = regulator_get(NULL, supply);	

	if (IS_ERR(sdcc_vreg_data[sdcc_no])) {
		pr_err("%s: could not get regulator %s rc=(%ld)\n",
				__func__, supply, PTR_ERR(sdcc_vreg_data[sdcc_no]));
		goto out;
	}

	rc = regulator_set_voltage(sdcc_vreg_data[sdcc_no], uV, uV);
	
	if (rc) {
                pr_err("%s: could not set voltage for \"%s\" to %d uV: %d\n",
                                __func__, supply, uV, rc);
                goto reg_free;
	}

	return rc;
reg_free:
        regulator_put(sdcc_vreg_data[sdcc_no]);
out:
        sdcc_vreg_data[sdcc_no] = NULL;
        return rc;
}
void __init msm7627a_init_mmc(void)
{
/* LGE_CHANGE_S : SD card detect GPIO configuration
	 * 2012-02-02, jyothishre.nk@lge.com
	 * added in msm7627a_sdcc_slot_status
	 */
//	sdcc_gpio_init();
/* LGE_CHANGE_E : SD card detect GPIO configuration */
	/* LGE_CHANGE_S : eMMC Power Control
	 * 2011-12-13, yoonsoo@lge.com
	 * mmc power is set wrong power domain. It affects the LCD blinking.
	 * emmc[L10] -> mmc[L13] 
	 */
	/* eMMC slot */
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
	if (mmc_regulator_init(3, "gp2", 2850000))//L12
                return;
/*LGE_CHANGE_S:Introducing latency to prevent Idle power collapse [jyothishre.nk@lge.com] 2012-03-20*/
	sdc3_plat_data.swfi_latency = msm7627a_power_collapse_latency(
                                         MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT);
/*LGE_CHANGE_E*/
	msm_add_sdcc(3, &sdc3_plat_data);
#endif /* LGE_CHANGE_E : eMMC Power Control */ 
	/* Micro-SD slot */
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
//	gpio_sdc1_config();
	if (mmc_regulator_init(1, "mmc", 2850000))
                return;
/*LGE_CHANGE_S:Introducing latency to prevent Idle power collapse [jyothishre.nk@lge.com] 2012-03-20*/
	sdc1_plat_data.swfi_latency = msm7627a_power_collapse_latency(
                                        MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT);
/*LGE_CHANGE_E*/
//	sdc1_plat_data.status_irq = MSM_GPIO_TO_INT(gpio_sdc1_hw_det);
	msm_add_sdcc(1, &sdc1_plat_data);
#endif
	/* SDIO WLAN slot */
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
         if (mmc_regulator_init(2, "smps3", 1800000))
                return;
	msm_add_sdcc(2, &sdc2_plat_data);
#endif
	/* Not Used */
#if (defined(CONFIG_MMC_MSM_SDC4_SUPPORT)\
		&& !defined(CONFIG_MMC_MSM_SDC3_8_BIT_SUPPORT))
        if (mmc_regulator_init(4, "smps3", 1800000))
                return;
	msm_add_sdcc(4, &sdc4_plat_data);
#endif
}
/*[END]*/
void __init lge_add_mmc_devices(void)
{
	msm7627a_init_mmc();
}
