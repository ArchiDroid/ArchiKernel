#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/input.h>

#include <mach/board.h>
#include <mach/board_lge.h>

#include "board-u0.h"

#include <mach/pmic.h>

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
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
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
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
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

/* ear sense driver */
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

/* LGE_CHANGE_S : 15/03/2012 yoonsoo.kim@lge.com*/
/*Start :: To Block Hook Events for 3 Pole */
enum {
	NO_HEADSET = 0,
	HEADSET_WITH_OUT_MIC = 1,
	HEADSET_WITH_MIC = 2
};
static int i_headset_type;
/*END :: To Block Hook Events for 3 Pole */
/* LGE_CHANGE_E : 15/03/2012 yoonsoo.kim@lge.com*/

static int u0_gpio_earsense_work_func(int *value)
{
	int state;
	int gpio_value;

	if (lge_bd_rev == LGE_REV_B)
		gpio_value = gpio_get_value(GPIO_EAR_SENSE);
	else
	gpio_value = !gpio_get_value(GPIO_EAR_SENSE);
	printk(KERN_INFO "%s: ear sense detected : %s[rev:%d]\n", __func__,
		gpio_value ? "injected" : "ejected", lge_bd_rev);

	if (gpio_value == EAR_EJECT) {
		state = EAR_STATE_EJECT;
/* LGE_CHANGE_S : 15/03/2012 yoonsoo.kim@lge.com*/
/*Start :: To Block Hook Events for 3 Pole */
/* Update Headset state to Headset Removed*/
		i_headset_type = NO_HEADSET;
/*END :: To Block Hook Events for 3 Pole */
/* LGE_CHANGE_E : 15/03/2012 yoonsoo.kim@lge.com*/
		*value = 0;
		gpio_set_value(GPIO_MIC_MODE, 0);
		snd_fm_vol_mute();
	} else {
		state = EAR_STATE_INJECT;

/* LGE_CHANGE_S :  2011-12-12, gt.kim@lge.com, Desc: Earjack 3pole noise */
        gpio_set_value(GPIO_MIC_MODE, 1);   // move code..
/* LGE_CHANGE_E :Earjack 3pole noise*/

/* LGE_CHANGE_S :	yangwook.lim@lge.com 2012.02.02, Desc: Earjack 4pole hook key issue when it is clicked twice.  */
		msleep(100);
/* LGE_CHANGE_S :	yangwook.lim@lge.com 2012.02.02, Desc: Earjack 4pole hook key issue when it is clicked twice.  */
		if (lge_bd_rev == LGE_REV_B)	// always 4pole headset detection - should be modified
		{
			*value = SW_MICROPHONE_INSERT;
		pmic_hsed_enable(PM_HSED_CONTROLLER_0, PM_HSED_ENABLE_ALWAYS);
		} else
		{
		gpio_value = !gpio_get_value(GPIO_BUTTON_DETECT);
		if (gpio_value) {
			printk(KERN_INFO "headphone was inserted!\n");
			*value = SW_HEADPHONE_INSERT;
/* LGE_CHANGE_S : 15/03/2012 yoonsoo.kim@lge.com*/
/*Start :: To Block Hook Events for 3 Pole */
/* Update Headset state to 3 pole Headset. ( Only receiver No Mic/Hook keys )*/
			i_headset_type = HEADSET_WITH_OUT_MIC;
/*END :: To Block Hook Events for 3 Pole */
/* LGE_CHANGE_E : 15/03/2012 yoonsoo.kim@lge.com*/
		} else {
			printk(KERN_INFO "micorphone was inserted!\n");
			*value = SW_MICROPHONE_INSERT;
/* LGE_CHANGE_S : 15/03/2012 yoonsoo.kim@lge.com*/
/*Start :: To Block Hook Events for 3 Pole */
/* Update Headset state to 4 pole Headset. ( Has receiver + hook key + Mic )*/			
			i_headset_type = HEADSET_WITH_MIC;
/*END :: To Block Hook Events for 3 Pole */
/* LGE_CHANGE_E : 15/03/2012 yoonsoo.kim@lge.com*/

//			gpio_set_value(GPIO_MIC_MODE, 1);
		}
	}
	}

	return state;
}

static char *u0_gpio_earsense_print_name(int state)
{
	if (state)
		return "Headset";
	else
		return "No Device";
}

static char *u0_gpio_earsense_print_state(int state)
{
	if (state == SW_HEADPHONE_INSERT)
		return ear_state_string[2];
	else if (state == SW_MICROPHONE_INSERT)
		return ear_state_string[1];
	else
		return ear_state_string[0];
}

static int u0_gpio_earsense_sysfs_store(const char *buf, size_t size)
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

static unsigned u0_earsense_gpios[] = {
	GPIO_EAR_SENSE,
};

/* especially to address gpio key */
static unsigned u0_hook_key_gpios[] = {
	GPIO_BUTTON_DETECT,
};

static int u0_gpio_hook_key_work_func(int *value)
{
	int gpio_value;
/* To Block Spurious Hook Key Events*/
/*Try to read gpio status 10 times with 1 msec delay.*/
	int i_val1,i_val2 = 0;
	int i_retries = 20;
	
	*value = KEY_MEDIA;
	gpio_value = !gpio_get_value(GPIO_BUTTON_DETECT);

/* To Block Spurious Hook Key Events*/
/*Check the gpio status after 10msec. If the value is different then fake interrupt*/
	i_val1 = gpio_value;
	do{
		i_val2 = !gpio_get_value(GPIO_BUTTON_DETECT);
		mdelay(1); /*Do we need to increase this ?? */
	}while( 0 > i_retries-- );

/* Value different means Fake interrupt. Ignore it*/
	if( i_val1 ^ i_val2)
	{ /* If the button value is not same after 10mSec Hope it's a fake interrupt*/
	  /* No need to send Hook Key event */
		printk("spurious Hook Key Event \n");
		*value = 0;
		return gpio_value;
	}
	
	printk(KERN_INFO "%s: hook key detected : %s\n", __func__,
		gpio_value ? "pressed" : "released");

/* LGE_CHANGE_S :	yangwook.lim@lge.com 2012.02.02, Desc: Earjack 4pole hook key issue when it is clicked twice.  */
//	msleep(50);
/* LGE_CHANGE_S :	yangwook.lim@lge.com 2012.02.02, Desc: Earjack 4pole hook key issue when it is clicked twice.  */

/* LGE_CHANGE_S : 15/03/2012 yoonsoo.kim@lge.com*/
/*Start :: To Block Hook Events for 3 Pole */
/* If Headset type not yet detected or 3 pole headset ( Only recevier , No hook key & Mic )
     ignore hook key event. Only allow hook key events for 4 pole device. ( which actually has a hook key)*/	
	if (  (gpio_get_value(GPIO_EAR_SENSE))
		 ||( NO_HEADSET           == i_headset_type )		
		 ||( HEADSET_WITH_OUT_MIC == i_headset_type )
		)
/*END :: To Block Hook Events for 3 Pole */
/* LGE_CHANGE_E : 15/03/2012 yoonsoo.kim@lge.com*/	
	{
		printk(KERN_INFO "Ignore hook key event\n");
		*value = 0;
/* LGE_CHANGE_S :	yangwook.lim@lge.com 2012.02.02, Desc: Earjack 4pole hook key issue when it is clicked twice.  */
//		msleep(300);
/* LGE_CHANGE_S :	yangwook.lim@lge.com 2012.02.02, Desc: Earjack 4pole hook key issue when it is clicked twice.  */
	}

/* Headset removed but we already sent one press event.*/
/* Send release event also */
	if (   (gpio_get_value(GPIO_EAR_SENSE)) /*Indicates Headset removed*/
		&& ( (HEADSET_WITH_MIC == i_headset_type) )
		&& ( 0 == gpio_value ) /* Indicates Button Released */
	   )
	{
		/* When headset is 4 pole & it comes to this point means previously we sent a hook key press event & not released yet*/
		/* Anyway headset is removed, so what we are waiting for. Release it Now. Otherwise frameworks behaving strangely */
		printk("EarJack Removed.Sending Hook Key release \n");
		*value = KEY_MEDIA;		
	}

	
	return gpio_value;
}

static struct lge_gpio_switch_platform_data u0_earsense_data = {
	/* LGE_UPDATE_S	myunghee.kim 2011.12.09 			*/
	/* WiredAccessoryObserver.java check h2w device not h2w_headset */
	/* modify here because of OS Upgrade				*/
	/* original code : .name = "h2w_headset", 			*/
	.name = "h2w",
	/* LGE_UPDATE_E							*/
	.gpios = u0_earsense_gpios,
	.num_gpios = ARRAY_SIZE(u0_earsense_gpios),
	.irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wakeup_flag = 1,
	.work_func = u0_gpio_earsense_work_func,
	.print_name = u0_gpio_earsense_print_name,
	.print_state = u0_gpio_earsense_print_state,
	.sysfs_store = u0_gpio_earsense_sysfs_store,

	/* especially to address gpio key */
	.key_gpios = u0_hook_key_gpios,
	.num_key_gpios = ARRAY_SIZE(u0_hook_key_gpios),
	.key_work_func = u0_gpio_hook_key_work_func,
};

static struct platform_device u0_earsense_device = {
	.name	= "lge-switch-gpio",
	.id		= -1,
	.dev	= {
		.platform_data = &u0_earsense_data,
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

/* input platform device */
static struct platform_device *u0_sound_devices[] __initdata = {
	&msm_device_snd,
	&msm_device_adspdec,
	&u0_earsense_device,
	&asoc_msm_pcm,
	&asoc_msm_dai0,
	&asoc_msm_dai1,
};

/* common function */
void __init lge_add_sound_devices(void)
{
	int rc;

	rc = gpio_request(GPIO_MIC_MODE, "mic_en");
	if (rc) {
		printk(KERN_ERR "%d gpio request is failed\n", GPIO_MIC_MODE);
	} else {
		rc = gpio_tlmm_config(GPIO_CFG(GPIO_MIC_MODE, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		if (rc)
			printk(KERN_ERR "%d gpio tlmm config is failed\n", GPIO_MIC_MODE);
	}

	platform_add_devices(u0_sound_devices, ARRAY_SIZE(u0_sound_devices));
}

