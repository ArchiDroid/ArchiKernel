#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/android_pmem.h>

#include <mach/board.h>


/*LGE_CHANGE_S : seven.kim@lge.com kernel3.4 for v3/v5*/
#if defined (CONFIG_MACH_LGE)
#include "../../devices.h"
#include "../../timer.h"
#include "../../board-msm7x27a-regulator.h"
#include "../../devices-msm7x2xa.h"
#include "../../pm.h"
#else /*qct original*/
#include "devices.h"
#include "timer.h"
#include "board-msm7x27a-regulator.h"
#include "devices-msm7x2xa.h"
#include "pm.h"
#endif /*CONFIG_MACH_LGE*/
/*LGE_CHANGE_E : seven.kim@lge.com kernel3.4 for v3/v5*/
#include <mach/rpc_server_handset.h>
#include <mach/socinfo.h>
/*LGE_CHANGE_S : seven.kim@lge.com kernel3.4 for v3/v5*/
#if defined (CONFIG_MACH_LGE)
#include CONFIG_LGE_BOARD_HEADER_FILE
#include "../../pm-boot.h"
#include "../../board-msm7627a.h"
#else
#include "pm-boot.h"
#include "board-msm7627a.h"
#endif /*CONFIG_MACH_LGE*/
/*LGE_CHANGE_E: seven.kim@lge.com kernel3.4 for v3/v5*/
//#include "../board-u0.h"

#include <mach/pmic.h>

#define NEW_ENDPOINTS

#ifdef NEW_ENDPOINTS

#define SND(desc, num) { .name = #desc, .id = num }
static struct snd_endpoint snd_endpoints_list[] = {
	SND(HANDSET, 0),
	SND(MONO_HEADSET, 2),
	SND(HEADSET, 3),
	SND(SPEAKER, 6),
	SND(TTY_HEADSET, 8),
	SND(TTY_VCO, 9),
	SND(TTY_HCO, 10),
	SND(BT, 12),
	SND(IN_S_SADC_OUT_HANDSET, 16),
	SND(IN_S_SADC_OUT_SPEAKER_PHONE, 25),
	SND(FM_DIGITAL_STEREO_HEADSET, 26),
	SND(FM_DIGITAL_SPEAKER_PHONE, 27),
	SND(FM_DIGITAL_BT_A2DP_HEADSET, 28),
	SND(STEREO_HEADSET_AND_SPEAKER, 31),
	SND(CURRENT, 0x7FFFFFFE),
	SND(FM_ANALOG_STEREO_HEADSET, 35),
	SND(FM_ANALOG_STEREO_HEADSET_CODEC, 36),
};
#undef SND

#else
#define SND(desc, num) { .name = #desc, .id = num }
static struct snd_endpoint snd_endpoints_list[] = {
	SND(HANDSET_LOOPBACK, 5),
	SND(HANDSET, 6),
	SND(HEADSET_LOOPBACK, 1),
	SND(HEADSET, 2),
	SND(HEADSET_STEREO, 3),
	SND(SPEAKER, 0),
	SND(SPEAKER_IN_CALL, 7),
	SND(SPEAKER_RING, 8),
	SND(HEADSET_AND_SPEAKER, 8),
	SND(FM_HEADSET, 10),
	SND(FM_SPEAKER, 11),
	SND(BT, 13),
	SND(TTY_HEADSET, 15),
	SND(TTY_VCO, 16),
	SND(TTY_HCO, 17),
	SND(TTY_HCO_SPEAKER, 18),
	SND(HANDSET_VR, 20),
	SND(HEADSET_VR, 21),
	SND(BT_VR, 23),
	SND(HEADSET_WITHOUT_MIC, 4),
	SND(HEADSET_WITHOUT_MIC_VR, 22),
	SND(SPEAKER_LOOPBACK, 29),
/* LGE_CHANGE_S : Add Camcoder MIC2 Path
 * 2011-11-10, gt.kim@lge.com,
 * Description:  Add Camcoder MIC2 Path rpc Num..
 * Repository : android\kernel\arch\arm\mach-msm\lge
*/
#if 1
//Add Audio Path Device  gt.kim@lge.com
	SND(MIC2_FOR_CAMCODER, 30),
	SND(CURRENT, 32),
#else
	SND(CURRENT, 31),
#endif
/* LGE_CHANGE_E : Add Camcoder MIC2 Path */
};
#undef SND
#endif

static struct msm_snd_endpoints msm_device_snd_endpoints = {
	.endpoints = snd_endpoints_list,
	.num = sizeof(snd_endpoints_list) / sizeof(struct snd_endpoint)
};

static struct platform_device msm_device_snd = {
	.name = "msm_snd",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_snd_endpoints
	},
};

#define DEC0_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP)| \
	(1<<MSM_ADSP_CODEC_AC3))
#define DEC1_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC2_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC3_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC4_FORMAT (1<<MSM_ADSP_CODEC_MIDI)

static unsigned int dec_concurrency_table[] = {
	/* Audio LP */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DMA)), 0,
	0, 0, 0,

	/* Concurrency 1 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	 /* Concurrency 2 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 3 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 4 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 5 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 6 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|
			(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	0, 0, 0, 0,

	/* Concurrency 7 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),
};

#define DEC_INFO(name, queueid, decid, nr_codec) { .module_name = name, \
	.module_queueid = queueid, .module_decid = decid, \
	.nr_codec_support = nr_codec}

static struct msm_adspdec_info dec_info_list[] = {
	DEC_INFO("AUDPLAY0TASK", 13, 0, 11), /* AudPlay0BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY1TASK", 14, 1, 11),  /* AudPlay1BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY2TASK", 15, 2, 11),  /* AudPlay2BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY3TASK", 16, 3, 11),  /* AudPlay3BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY4TASK", 17, 4, 1),  /* AudPlay4BitStreamCtrlQueue */
};

static struct msm_adspdec_database msm_device_adspdec_database = {
	.num_dec = ARRAY_SIZE(dec_info_list),
	.num_concurrency_support = (ARRAY_SIZE(dec_concurrency_table) / \
					ARRAY_SIZE(dec_info_list)),
	.dec_concurrency_table = dec_concurrency_table,
	.dec_info_list = dec_info_list,
};

static struct platform_device msm_device_adspdec = {
	.name = "msm_adspdec",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_adspdec_database
	},
};

/* add original code */
#define SNDDEV_CAP_NONE 0x0
#define SNDDEV_CAP_RX 0x1 /* RX direction */
#define SNDDEV_CAP_TX 0x2 /* TX direction */
#define SNDDEV_CAP_VOICE 0x4 /* Support voice call */
#define SNDDEV_CAP_FM 0x10 /* Support FM radio */
#define SNDDEV_CAP_TTY 0x20 /* Support TTY */
#define CAD(desc, num, cap) { .name = #desc, .id = num, .capability = cap, }
static struct cad_endpoint cad_endpoints_list[] = {
	CAD(NONE, 0, SNDDEV_CAP_NONE),
	CAD(HANDSET_SPKR, 1, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(HANDSET_MIC, 2, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(HEADSET_MIC, 3, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(HEADSET_SPKR_MONO, 4, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(HEADSET_SPKR_STEREO, 5, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(SPEAKER_PHONE_MIC, 6, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(SPEAKER_PHONE_MONO, 7, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
/* 2013-01-14 Hoseong Kang(hoseong.kang@lge.com) add BT NREC tx device [START]*/ 
//	CAD(SPEAKER_PHONE_STEREO, 8, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(BT_SCO_MIC_NREC, 8, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
/* 2013-01-14 Hoseong Kang(hoseong.kang@lge.com) add BT NREC tx device [END]*/
	CAD(BT_SCO_MIC, 9, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(BT_SCO_SPKR, 10, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(BT_A2DP_SPKR, 11, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(TTY_HEADSET_MIC, 12, (SNDDEV_CAP_TX | \
			SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY)),
	CAD(TTY_HEADSET_SPKR, 13, (SNDDEV_CAP_RX | \
			SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY)),
	CAD(LB_HANDSET_MIC, 14, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(LB_HANDSET_SPKR, 15, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(LB_HEADSET_MIC, 16, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(LB_HEADSET_SPKR, 17, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(LB_SPKRPHONE_MIC, 20, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(LB_SPKRPHONE_SPKR, 21, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(HEADSET_STEREO_PLUS_SPKR_MONO_RX, 19, (SNDDEV_CAP_TX | \
				SNDDEV_CAP_VOICE)),
	CAD(SPEAKER_PHONE_SPKR_MEDIA, 22, (SNDDEV_CAP_RX)),
	CAD(SPEAKER_PHONE_MIC_MEDIA, 23, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(LP_FM_HEADSET_SPKR_STEREO_RX, 25, (SNDDEV_CAP_TX | SNDDEV_CAP_FM)),
	CAD(I2S_RX, 32, (SNDDEV_CAP_RX)),
	CAD(HANDSET_VR_MIC, 27, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(HEADSET_VR_MIC, 29, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(BT_VR_MIC, 55, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(SPEAKER_PHONE_MIC_ENDFIRE, 45, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(HANDSET_MIC_ENDFIRE, 46, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(I2S_TX, 48, (SNDDEV_CAP_TX)),
	CAD(HEADSET_STEREO_LB_PLUS_HEADSET_SPKR_STEREO_RX, 51, \
			(SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(SPEAKER_MONO_LB_PLUS_SPEAKER_MONO_RX, 54, \
			(SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(LP_FM_HEADSET_SPKR_STEREO_PLUS_HEADSET_SPKR_STEREO_RX, 57, \
			(SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(FM_DIGITAL_HEADSET_SPKR_STEREO, 65, \
			(SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(FM_DIGITAL_SPEAKER_PHONE_MONO, 67, \
			(SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(FM_DIGITAL_SPEAKER_PHONE_MIC, 68, \
			(SNDDEV_CAP_FM | SNDDEV_CAP_TX)),
	CAD(FM_DIGITAL_BT_A2DP_SPKR, 69, \
			(SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(MAX, 80, SNDDEV_CAP_NONE),
};
#undef CAD

static struct msm_cad_endpoints msm_device_cad_endpoints = {
	.endpoints = cad_endpoints_list,
	.num = sizeof(cad_endpoints_list) / sizeof(struct cad_endpoint)
};

struct platform_device msm_device_cad = {
	.name = "msm_cad",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_cad_endpoints
	},
};
/* add orignal code */
/* ear sense driver */
#if 1 //HEADSET CHANGE
static struct gpio_h2w_platform_data m4_h2w_data = {
	.gpio_detect = GPIO_EAR_SENSE,
	.gpio_button_detect = GPIO_BUTTON_DETECT,
};

static struct platform_device m4_h2w_device = {
	.name = "lge-switch-gpio",
	.id = -1,
	.dev = {
		.platform_data = &m4_h2w_data,
	},
};


#else
static char *ear_state_string[] = {
	"0",
	"1",
	"2",
};

enum {
	EAR_STATE_EJECT = 0,
	EAR_STATE_INJECT = 1,
};

enum {
	EAR_EJECT = 0,
	EAR_INJECT = 1,
};

#if 0 //tamedwolph unnecessary
int headset_state;
#endif

static int m4eu_gpio_earsense_work_func(int *value)
{
	int state;
	int gpio_value;

	msleep(100);

	gpio_value = !gpio_get_value(GPIO_EAR_SENSE);
	printk(KERN_INFO "%s: ear sense detected : %s\n", __func__,
		gpio_value ? "injected" : "ejected");

	if (gpio_value == EAR_EJECT) {
		state = EAR_STATE_EJECT;
		*value = 0;
		gpio_set_value(GPIO_MIC_MODE, 0);
		snd_fm_vol_mute();
	} else {
		state = EAR_STATE_INJECT;
		msleep(50);
		gpio_value = !gpio_get_value(GPIO_BUTTON_DETECT);
		if (gpio_value) {
			printk(KERN_INFO "headphone was inserted!\n");
			*value = SW_HEADPHONE_INSERT;
			gpio_set_value(GPIO_MIC_MODE,1);
		} else {
			printk(KERN_INFO "micorphone was inserted!\n");
			*value = SW_MICROPHONE_INSERT;
			gpio_set_value(GPIO_MIC_MODE, 1);
		}
	}
#if 0 //tamedwolph unnecessary
	headset_state = *value;
#endif

	return state;
}

static char *m4eu_gpio_earsense_print_name(int state)
{
	if (state)
		return "Headset";
	else
		return "No Device";
}

static char *m4eu_gpio_earsense_print_state(int state)
{
	if (state == SW_HEADPHONE_INSERT)
		return ear_state_string[2];
	else if (state == SW_MICROPHONE_INSERT)
		return ear_state_string[1];
	else
		return ear_state_string[0];
}

static int m4eu_gpio_earsense_sysfs_store(const char *buf, size_t size)
{
	int state;

	if (!strncmp(buf, "eject", size - 1))
		state = EAR_STATE_EJECT;
	else if (!strncmp(buf, "inject", size - 1))
		state = EAR_STATE_INJECT;
	else
		return -EINVAL;

	return state;
}

static unsigned m4eu_earsense_gpios[] = {
	GPIO_EAR_SENSE,
};

/* especially to address gpio key */
static unsigned m4eu_hook_key_gpios[] = {
	GPIO_BUTTON_DETECT,
};

static int m4eu_gpio_hook_key_work_func(int *value)
{
	int gpio_value;

	*value = KEY_MEDIA;
	gpio_value = !gpio_get_value(GPIO_BUTTON_DETECT);
	printk(KERN_INFO "%s: hook key detected : %s\n", __func__,
		gpio_value ? "pressed" : "released");

	msleep(150);
	if (gpio_get_value(GPIO_EAR_SENSE)) {
		printk(KERN_INFO "Ignore hook key event\n");
		*value = 0;
	}

	return gpio_value;
}

static struct lge_gpio_switch_platform_data m4eu_earsense_data = {
	/* LGE_UPDATE_S	myunghee.kim 2011.12.09 			*/
	/* WiredAccessoryObserver.java check h2w device not h2w_headset */
	/* modify here because of OS Upgrade				*/
	/* original code : .name = "h2w_headset", 			*/
	.name = "h2w",
	/* LGE_UPDATE_E							*/
	.gpios = m4eu_earsense_gpios,
	.num_gpios = ARRAY_SIZE(m4eu_earsense_gpios),
	.irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wakeup_flag = 1,
	.work_func = m4eu_gpio_earsense_work_func,
	.print_name = m4eu_gpio_earsense_print_name,
	.print_state = m4eu_gpio_earsense_print_state,
	.sysfs_store = m4eu_gpio_earsense_sysfs_store,

	/* especially to address gpio key */
	.key_gpios = m4eu_hook_key_gpios,
	.num_key_gpios = ARRAY_SIZE(m4eu_hook_key_gpios),
	.key_work_func = m4eu_gpio_hook_key_work_func,
};

static struct platform_device m4eu_earsense_device = {
	.name	= "lge-switch-gpio",
	.id		= -1,
	.dev	= {
		.platform_data = &m4eu_earsense_data,
	},
};
#endif
struct platform_device asoc_msm_pcm = {
	.name   = "msm-dsp-audio",
	.id     = 0,
};

struct platform_device asoc_msm_dai0 = {
	.name   = "msm-codec-dai",
	.id     = 0,
};

struct platform_device asoc_msm_dai1 = {
	.name   = "msm-cpu-dai",
	.id     = 0,
};

/* input platform device */
static struct platform_device *m4eu_sound_devices[] __initdata = {
	&msm_device_snd,
	&msm_device_cad,
	&msm_device_adspdec,
	&m4_h2w_device,
	&asoc_msm_pcm,
	&asoc_msm_dai0,
	&asoc_msm_dai1,
};

/* common function */
void __init lge_add_sound_devices(void)
{
#if 0 //tamedwolph unnecessary
	int rc;
	headset_state = 0;
	rc = gpio_request(GPIO_MIC_MODE, "mic_en");
	if (rc) {
		printk(KERN_ERR "%d gpio request is failed\n", GPIO_MIC_MODE);
	} else {
		rc = gpio_tlmm_config(GPIO_CFG(GPIO_MIC_MODE, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

		gpio_set_value(GPIO_MIC_MODE, 1);

//		if (rc)
			printk(KERN_ERR "%d gpio tlmm config is failed\n", GPIO_MIC_MODE);
	}
#endif

	platform_add_devices(m4eu_sound_devices, ARRAY_SIZE(m4eu_sound_devices));
}
