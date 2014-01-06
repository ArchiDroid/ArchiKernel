/********************************************************************
@
@
@
@
@
@
@
@
@ Foxconn, Inc
@
@ 20120901, OwenHuang, Initial Version
********************************************************************/
#include <linux/mfd/pm8xxx/gpio.h>
#include <linux/mfd/pm8xxx/mpp.h>
#include <linux/mfd/pm8xxx/core.h>
#include <linux/mfd/pm8xxx/pwm.h>
#include <linux/fih_leds.h>

/*LED action type*/
enum {
LED_ACTION_ON = 0,
LED_ACTION_OFF,
LED_ACTION_BLINK_ON,
LED_ACTION_BLINK_OFF,
LED_ACTION_FADE_ON,
LED_ACTION_FADE_OFF,
LED_ACTION_MAX,
};

/*Hardware PIN type*/
enum {
TYPE_PMIC_GPIO = 0,
TYPE_PMIC_MPP,
TYPE_GPIO,
TYPE_PMIC_LPG,
TYPE_MAX,
};

//PERI-BJ-SetCurrent-00+{
/*Dedicated RGB PIN current*/
enum{
RGB_0mA = 0,
RGB_4mA,
RGB_8mA,
RGB_12mA,
};
//PERI-BJ-SetCurrent-00+}

//MTD-SW3-PERIPHERAL-OH-LED_Porting-01+{
//For compile error, there is no define these two structure type
struct pm8xxx_pwm_chip{
	struct pwm_device		*pwm_dev;
	u8				pwm_channels;
	u8				pwm_total_pre_divs;
	u8				bank_mask;
	struct mutex			pwm_mutex;
	struct device			*dev;
	bool				is_lpg_supported;
};

#define PM8XXX_LPG_CTL_REGS		7
struct pwm_device {
	int			pwm_id;		/* = bank/channel id */
	int			in_use;
	const char		*label;
	struct pm8xxx_pwm_period	period;
	int			pwm_value;
	int			pwm_period;
	int			pwm_duty;
	u8			pwm_lpg_ctl[PM8XXX_LPG_CTL_REGS];
	u8			pwm_ctl1;
	u8			pwm_ctl2;
	int			irq;
	struct pm8xxx_pwm_chip	*chip;
	int			bypass_lut;
	int			dtest_mode_supported;
};
//MTD-SW3-PERIPHERAL-OH-LED_Porting-01+}

/*For leds driver use, it could control PMIC GPIO/PMIC GPIO/MPP/Pure LPG function*/
int common_control_leds(struct pwm_device *pwm_dev, struct led_pmic_data *pmic_data);
int control_fade_nonsync_leds(struct pwm_device *pwm_dev_r, struct led_pmic_data *pmic_data_r, struct pwm_device *pwm_dev_g, struct led_pmic_data *pmic_data_g, struct pwm_device *pwm_dev_b, struct led_pmic_data *pmic_data_b);//MTD-SW3-PERIPHERAL-BJ-LED_FADE_IN_OUT_DIFF-00+
void set_default_pwm_register(struct pwm_device *pwm_dev);//PERI-BJ-Set_PWM_register_default-00+

