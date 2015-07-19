#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/input.h>

#include <mach/board.h>

#include "../../devices.h"
#include "../../timer.h"
#include "../../board-msm7x27a-regulator.h"
#include "../../devices-msm7x2xa.h"
#include "../../pm.h"

#include <mach/rpc_server_handset.h>
#include <mach/socinfo.h>

#include CONFIG_LGE_BOARD_HEADER_FILE
#include "../../pm-boot.h"
#include "../../board-msm7627a.h"

#include <sound/fsa8008.h>

#include <mach/pmic.h>

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
	DEC_INFO("AUDPLAY1TASK", 14, 1, 11), /* AudPlay1BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY2TASK", 15, 2, 11), /* AudPlay2BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY3TASK", 16, 3, 11), /* AudPlay3BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY4TASK", 17, 4, 1),  /* AudPlay4BitStreamCtrlQueue */
};

static struct msm_adspdec_database msm_device_adspdec_database = {
	.num_dec = ARRAY_SIZE(dec_info_list),
	.num_concurrency_support = (ARRAY_SIZE(dec_concurrency_table) / ARRAY_SIZE(dec_info_list)),
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
	CAD(BT_SCO_MIC_NREC, 8, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(BT_SCO_MIC, 9, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(BT_SCO_SPKR, 10, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(BT_A2DP_SPKR, 11, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(TTY_HEADSET_MIC, 12, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY)),
	CAD(TTY_HEADSET_SPKR, 13, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY)),
	CAD(LB_HANDSET_MIC, 14, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(LB_HANDSET_SPKR, 15, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(LB_HEADSET_MIC, 16, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(LB_HEADSET_SPKR, 17, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(LB_SPKRPHONE_MIC, 20, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
	CAD(LB_SPKRPHONE_SPKR, 21, (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE)),
	CAD(HEADSET_STEREO_PLUS_SPKR_MONO_RX, 19, (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE)),
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
	CAD(HEADSET_STEREO_LB_PLUS_HEADSET_SPKR_STEREO_RX, 51, (SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(SPEAKER_MONO_LB_PLUS_SPEAKER_MONO_RX, 54, (SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(LP_FM_HEADSET_SPKR_STEREO_PLUS_HEADSET_SPKR_STEREO_RX, 57, (SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(FM_DIGITAL_HEADSET_SPKR_STEREO, 65, (SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(FM_DIGITAL_SPEAKER_PHONE_MONO, 67, (SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
	CAD(FM_DIGITAL_SPEAKER_PHONE_MIC, 68, (SNDDEV_CAP_FM | SNDDEV_CAP_TX)),
	CAD(FM_DIGITAL_BT_A2DP_SPKR, 69, (SNDDEV_CAP_FM | SNDDEV_CAP_RX)),
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

#include <linux/input.h>

void fsa8008_set_headset_mic_bias(int enable) {
   printk(KERN_INFO "[FSA8008] Set MIC BIAS %d .\n", enable);
   pmic_hsed_enable(PM_HSED_CONTROLLER_0,(enable ? PM_HSED_ENABLE_ALWAYS : PM_HSED_ENABLE_OFF )); 
}

static struct fsa8008_platform_data fsa8008_platform_data = {
    .switch_name = "h2w",

    .keypad_name = "ffa-keypad",

    .key_code =  KEY_MEDIA,

    .gpio_detect = GPIO_EAR_SENSE_N,
    .gpio_mic_en = GPIO_EAR_MIC_EN,
    .gpio_jpole  = GPIO_EARPOL_DETECT,
    .gpio_key    = GPIO_EAR_KEY_INT,

    .set_headset_mic_bias = fsa8008_set_headset_mic_bias,

    .latency_for_detection = 10,
};

static struct platform_device lge_hsd_device = {
   .name = "fsa8008",
   .id   = -1,
   .dev = {
      .platform_data = &fsa8008_platform_data,
   },   
};

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

static struct platform_device *u0_sound_devices[] __initdata = {
	&msm_device_snd,
	&msm_device_cad,
	&msm_device_adspdec,
	&lge_hsd_device,
	&asoc_msm_pcm,
	&asoc_msm_dai0,
	&asoc_msm_dai1,
};

void __init lge_add_sound_devices(void)
{
	platform_add_devices(u0_sound_devices, ARRAY_SIZE(u0_sound_devices));
}

