/*
** =========================================================================
** File:
**     ImmVibeSPI.c
**
** Description: 
**     Device-dependent functions called by Immersion TSP API
**     to control PWM duty cycle, amp enable/disable, save IVT file, etc...
**
** Portions Copyright (c) 2008-2010 Immersion Corporation. All Rights Reserved. 
**
** This file contains Original Code and/or Modifications of Original Code 
** as defined in and that are subject to the GNU Public License v2 - 
** (the 'License'). You may not use this file except in compliance with the 
** License. You should have received a copy of the GNU General Public License 
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or contact 
** TouchSenseSales@immersion.com.
**
** The Original Code and all software distributed under the License are 
** distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESS OR IMPLIED, AND IMMERSION HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
** INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
** FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see 
** the License for the specific language governing rights and limitations 
** under the License.
** =========================================================================
*/

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com]
#include <mach/gpio.h>
#include <linux/io.h>
#include <linux/gpio.h>
//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]

#ifdef IMMVIBESPIAPI
#undef IMMVIBESPIAPI
#endif
//#define IMMVIBESPIAPI static
#define IMMVIBESPIAPI

/*
** This SPI supports only one actuator.
*/
#define NUM_ACTUATORS 1

#define PWM_DUTY_MAX    579 /* 13MHz / (579 + 1) = 22.4kHz */

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com]
#define GPIO_VIBE_PWM	96 
#define GPIO_VIBE_EN	85

#define GPIO_MOTOR_LDO_EN 116

#define GPIO_LEVEL_HIGH	1
#define GPIO_LEVEL_LOW 	0

#define CONFIG_ON 1
#define CONFIG_OFF 0

#define GPMN_M_DEFAULT			    7
#define GPMN_N_DEFAULT			    6000
#define GPMN_D_DEFAULT			    3000	/* 50% duty cycle */ 
#define GPMN_IMM_PWM_MULTIPLIER	       	    5980	/* Must be integer */

/*
** Global variables for LRA PWM M,N counters.
*/
int g_nForce_32 =0;

VibeInt32 g_nLRA_GPMN_M = GPMN_M_DEFAULT;
VibeInt32 g_nLRA_GPMN_N = GPMN_N_DEFAULT;
VibeInt32 g_nLRA_GPMN_D = GPMN_D_DEFAULT;
VibeInt32 g_nLRA_GPMN_CLK_PWM_MUL = GPMN_IMM_PWM_MULTIPLIER;

int t_nLRA_GPMN_M = GPMN_M_DEFAULT;
int t_nLRA_GPMN_N = GPMN_N_DEFAULT;
int t_nLRA_GPMN_CLK_PWM_MUL = GPMN_IMM_PWM_MULTIPLIER;


#define MSM_CLK_PHYS          0xA8600000
#define MSM_CLK_SIZE          SZ_4K
volatile void __iomem *MSM_CLK_BASE ;


#define HWIO_GP_MD_REG_ADDR                             (MSM_CLK_BASE+0x58)
#define HWIO_GP_MD_REG_OUTM(M,V)        HWIO_OUTM(GP_MD_REG,M,V)
#define HWIO_GP_NS_REG_ADDR                             (MSM_CLK_BASE+0x5C)
#define HWIO_GP_NS_REG_OUTM(M,V)        HWIO_OUTM(GP_NS_REG,M,V)

#define HWIO_GP_NS_REG_GP_ROOT_ENA_SHFT                                 11      // clk src signal enable
#define HWIO_GP_NS_REG_GP_CLK_INV_SHFT                                  10      // clk inv no
#define HWIO_GP_NS_REG_GP_CLK_BRANCH_ENA_SHFT   9               // gp_clk output enable
#define HWIO_GP_NS_REG_MNCNTR_EN_SHFT                                   8               // mn-cnt enable
#define HWIO_GP_NS_REG_MNCNTR_RST_SHFT                                  7               // rst..no
#define HWIO_GP_NS_REG_MNCNTR_MODE_SHFT                                 5               // mn-cnt SE mode
#define HWIO_GP_NS_REG_PRE_DIV_SEL_SHFT                                 3               // pre-divder 1/4
#define HWIO_GP_NS_REG_SRC_SEL_SHFT                                             0               // src=tcxo

#define HWIO_GP_MD_REG_M_VAL_SHFT                                                               0x10
#define HWIO_GP_MD_REG_M_VAL_BMSK                                                               (0xFFFF<<HWIO_GP_MD_REG_M_VAL_SHFT)
#define HWIO_GP_MD_REG_D_VAL_SHFT                                                               0x00
#define HWIO_GP_MD_REG_D_VAL_BMSK                                                               (0xFFFF<<HWIO_GP_MD_REG_D_VAL_SHFT)
#define HWIO_GP_NS_REG_N_VAL_SHFT                                                               0x10
#define HWIO_GP_NS_REG_N_VAL_BMSK                                                               (0xFFFF<<HWIO_GP_NS_REG_N_VAL_SHFT)

#define HWIO_OUTM(REG,M,V)\
        *(volatile unsigned int *)(HWIO_##REG##_ADDR)   = \
                ((*(volatile unsigned int *)(HWIO_##REG##_ADDR)) & (~(M))) | ( (V) & (M) )

#define GP_CLK_SET_Mval(mval)  \
    HWIO_GP_MD_REG_OUTM(HWIO_GP_MD_REG_M_VAL_BMSK, mval<<HWIO_GP_MD_REG_M_VAL_SHFT)

#define GP_CLK_SET_Dval(dval)  \
    HWIO_GP_MD_REG_OUTM(HWIO_GP_MD_REG_D_VAL_BMSK, ~(dval<<1)<<HWIO_GP_MD_REG_D_VAL_SHFT)

#define GP_CLK_SET_Nval(nval)  \
    HWIO_GP_NS_REG_OUTM(HWIO_GP_NS_REG_N_VAL_BMSK, ~(nval-g_nLRA_GPMN_M)<<HWIO_GP_NS_REG_N_VAL_SHFT)

#define HWIO_GP_NS_REG_CNTL_VAL_BMSK  0xfff


//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]

static bool g_bAmpEnabled = false;

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com]

static void GP_CLK_EN(bool en_gp_clk)
{
    static int gp_clk_control;

    if (en_gp_clk)
    {
        gp_clk_control = 1 << HWIO_GP_NS_REG_GP_ROOT_ENA_SHFT         // clk src signal enable
                       | 0 << HWIO_GP_NS_REG_GP_CLK_INV_SHFT          // clk inv no
                       | 1 << HWIO_GP_NS_REG_GP_CLK_BRANCH_ENA_SHFT   // gp_clk output enable
                       | 1 << HWIO_GP_NS_REG_MNCNTR_EN_SHFT           // mn-cnt enable
                       | 0 << HWIO_GP_NS_REG_MNCNTR_RST_SHFT          // rst..no
                       | 3 << HWIO_GP_NS_REG_MNCNTR_MODE_SHFT         // mn-cnt SE mode
                       | 3 << HWIO_GP_NS_REG_PRE_DIV_SEL_SHFT         // pre-divder 1/4
                       | 0 << HWIO_GP_NS_REG_SRC_SEL_SHFT      ;      // src=tcxo
    }
    else
    {
        gp_clk_control = 0;
    }
    HWIO_GP_NS_REG_OUTM(HWIO_GP_NS_REG_CNTL_VAL_BMSK, gp_clk_control);
}

static void vibe_set_pwm_freq(void)
{

    // GP_CLK control field setting : gp_clk en, clk_src=tcxo
    GP_CLK_EN(1);
    // GP_CLK N-div setting
    GP_CLK_SET_Nval(g_nLRA_GPMN_N);
    // GP_CLK M-div setting
    GP_CLK_SET_Mval(g_nLRA_GPMN_M);
    // GP_CLK Duty Setting
    GP_CLK_SET_Dval(g_nLRA_GPMN_D);

}

void ForceOut_Set(int nForce)
{

	DbgOut((KERN_DEBUG "ForceOut_Set  %d \n",nForce));

    if(nForce == 0){
        /* Set 50% duty cycle */
        GP_CLK_SET_Dval(g_nLRA_GPMN_D);
        GP_CLK_EN(0);
    }
    else{

        g_nLRA_GPMN_M = t_nLRA_GPMN_M;
        g_nLRA_GPMN_N = t_nLRA_GPMN_N;
        g_nLRA_GPMN_D = (t_nLRA_GPMN_N >> 1);
        g_nLRA_GPMN_CLK_PWM_MUL = t_nLRA_GPMN_CLK_PWM_MUL;

        vibe_set_pwm_freq();
//        g_nForce_32 = (nForce * (t_nLRA_GPMN_N - g_nLRA_GPMN_M)) >> 7;
        g_nForce_32 = ((nForce * g_nLRA_GPMN_CLK_PWM_MUL) >> 8) + g_nLRA_GPMN_D;
        GP_CLK_SET_Dval(g_nForce_32);
	}
}


int vibrator_pwm_set(int enable, int amp)
{
	if(enable){
        ForceOut_Set(amp);
    } 
	else{
        ForceOut_Set(0);
    }

    return 0;
}
EXPORT_SYMBOL(vibrator_pwm_set);

static int vibrator_power_set(int enable)
{
    
	DbgOut((KERN_DEBUG "ImmVibeSPI: vibrator_power_set %d .\n",enable));
	
	if(enable){
		gpio_set_value(GPIO_MOTOR_LDO_EN,GPIO_LEVEL_HIGH);
	}
	else{
		gpio_set_value(GPIO_MOTOR_LDO_EN,GPIO_LEVEL_LOW);
	}
	
	return 0;	
}
static int vibrator_gpio_config(int enable)
{

	int rc;

    DbgOut(("vibrator_gpio_config %d \n",enable));

	if(enable){
		gpio_tlmm_config(GPIO_CFG(GPIO_MOTOR_LDO_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(GPIO_VIBE_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	        gpio_tlmm_config(GPIO_CFG(GPIO_VIBE_PWM, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		
    	rc = gpio_request(GPIO_MOTOR_LDO_EN, "lin_motor_LDO_EN");

    	if (unlikely(rc < 0)){
        	DbgOut(("not able to get gpio %d\n",GPIO_MOTOR_LDO_EN));
			return VIBE_E_FAIL;
		}

    	rc = gpio_request(GPIO_VIBE_EN, "lin_motor_en");

    	if (unlikely(rc < 0)){
        	DbgOut(("not able to get gpio %d \n",GPIO_VIBE_EN));
			return VIBE_E_FAIL;
		}
		
	rc = gpio_request(GPIO_VIBE_PWM, "lin_motor_pwm");

    	if (unlikely(rc < 0)){
        	DbgOut(("not able to get gpio %d \n",GPIO_VIBE_EN));
			return VIBE_E_FAIL;
		}

		gpio_direction_output(GPIO_MOTOR_LDO_EN, 1);
		gpio_direction_output(GPIO_VIBE_EN, 1);
		gpio_direction_output(GPIO_VIBE_PWM, 1);
	}
	else{
		gpio_direction_output(GPIO_VIBE_PWM, 0);
		gpio_direction_output(GPIO_VIBE_EN, 0);
		gpio_direction_output(GPIO_MOTOR_LDO_EN, 0);

		gpio_free(GPIO_VIBE_PWM);
		gpio_free(GPIO_VIBE_EN);
		gpio_free(GPIO_MOTOR_LDO_EN);
		
		gpio_tlmm_config(GPIO_CFG(GPIO_VIBE_PWM, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_DISABLE /*GPIO_CFG_ENABLE */);
		gpio_tlmm_config(GPIO_CFG(GPIO_VIBE_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_DISABLE /*GPIO_CFG_ENABLE */);
		gpio_tlmm_config(GPIO_CFG(GPIO_MOTOR_LDO_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),GPIO_CFG_DISABLE /*GPIO_CFG_ENABLE */);
	}
	
	return 0;	

}

//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]

/*
** Called to disable amp (disable output force)
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_AmpDisable(VibeUInt8 nActuatorIndex)
{

	if (g_bAmpEnabled){
		DbgOut(("ImmVibeSPI_ForceOut_AmpDisable.\n"));

        g_bAmpEnabled = false;

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com]
		gpio_set_value(GPIO_VIBE_EN,GPIO_LEVEL_LOW);
	
		vibrator_pwm_set(0,0);
		vibrator_power_set(0);

//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]
    }

    return VIBE_S_SUCCESS;
}
EXPORT_SYMBOL(ImmVibeSPI_ForceOut_AmpDisable);

/*
** Called to enable amp (enable output force)
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_AmpEnable(VibeUInt8 nActuatorIndex)
{

    if (!g_bAmpEnabled){
    	DbgOut(("ImmVibeSPI_ForceOut_AmpEnable.\n"));

        g_bAmpEnabled = true;

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com]
		vibrator_power_set(1);
		vibrator_pwm_set(1,0);  

		gpio_set_value(GPIO_VIBE_EN,GPIO_LEVEL_HIGH);
    }
    else{
		DbgOut(("ImmVibeSPI_ForceOut_AmpEnabled [%d]\n",g_bAmpEnabled));
    }
//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]

    return VIBE_S_SUCCESS;
}
EXPORT_SYMBOL(ImmVibeSPI_ForceOut_AmpEnable);

/*
** Called at initialization time to set PWM freq, disable amp, etc...
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Initialize(void)
{

    DbgOut(("ImmVibeSPI_ForceOut_Initialize.\n"));

    g_bAmpEnabled = true;   /* to force ImmVibeSPI_ForceOut_AmpDisable disabling the amp */

    /* 
    ** Disable amp.
    ** If multiple actuators are supported, please make sure to call
    ** ImmVibeSPI_ForceOut_AmpDisable for each actuator (provide the actuator index as
    ** input argument).
    */
//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com]
    MSM_CLK_BASE = ioremap(MSM_CLK_PHYS,MSM_CLK_SIZE);	
	
	if(!MSM_CLK_BASE){
    	DbgOut(("ImmVibeSPI_ForceOut_Initialize failed :Unable to get CLK_BASE \n"));
		return VIBE_E_FAIL;
	}

	vibrator_gpio_config(CONFIG_ON);
//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]

	ImmVibeSPI_ForceOut_AmpDisable(0);

	return VIBE_S_SUCCESS;
}
EXPORT_SYMBOL(ImmVibeSPI_ForceOut_Initialize);

/*
** Called at termination time to set PWM freq, disable amp, etc...
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Terminate(void)
{

    DbgOut(("ImmVibeSPI_ForceOut_Terminate.\n"));

    /* 
    ** Disable amp.
    ** If multiple actuators are supported, please make sure to call
    ** ImmVibeSPI_ForceOut_AmpDisable for each actuator (provide the actuator index as
    ** input argument).
    */

	
    ImmVibeSPI_ForceOut_AmpDisable(0);

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com]

    vibrator_gpio_config(CONFIG_OFF);
    
	if(MSM_CLK_BASE){
	    iounmap(MSM_CLK_BASE);
    	MSM_CLK_BASE = 0 ;
    }	
//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]

    return VIBE_S_SUCCESS;
}
EXPORT_SYMBOL(ImmVibeSPI_ForceOut_Terminate);

/*
** Called by the real-time loop to set PWM duty cycle
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_SetSamples(VibeUInt8 nActuatorIndex, VibeUInt16 nOutputSignalBitDepth, VibeUInt16 nBufferSizeInBytes, VibeInt8* pForceOutputBuffer)
{
    VibeInt8 nForce;

    switch (nOutputSignalBitDepth)
    {
        case 8:
            /* pForceOutputBuffer is expected to contain 1 byte */
            if (nBufferSizeInBytes != 1) return VIBE_E_FAIL;

            nForce = pForceOutputBuffer[0];
            break;
        case 16:
            /* pForceOutputBuffer is expected to contain 2 byte */
            if (nBufferSizeInBytes != 2) return VIBE_E_FAIL;

            /* Map 16-bit value to 8-bit */
            nForce = ((VibeInt16*)pForceOutputBuffer)[0] >> 8;
            break;
        default:
            /* Unexpected bit depth */
            return VIBE_E_FAIL;
    }

    /* Check the Force value with Max and Min force value */

//[LGSI_SP4_BSP_BEGIN][kirankumar.vm@lge.com]
    if (nForce > 127){
    	nForce = 127;
	goto set;
    }
    if (nForce < -127){
        nForce = -127;
	goto set;
    }
    if (nForce < 0){
        nForce = 0;
	goto set;
    }

set:
    vibrator_pwm_set(1, nForce);
//[LGSI_SP4_BSP_END][kirankumar.vm@lge.com]
    return VIBE_S_SUCCESS;
}

#if 0
/*
** Called to set force output frequency parameters
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_SetFrequency(VibeUInt8 nActuatorIndex, VibeUInt16 nFrequencyParameterID, VibeUInt32 nFrequencyParameterValue)
{
    /* This function is not called for ERM device */

    return VIBE_S_SUCCESS;
}
#endif

/*
** Called to get the device name (device name must be returned as ANSI char)
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_Device_GetName(VibeUInt8 nActuatorIndex, char *szDevName, int nSize)
{

    if ((!szDevName) || (nSize < 1)) return VIBE_E_FAIL;

    DbgOut(("ImmVibeSPI_Device_GetName.\n"));

    strncpy(szDevName, "TouchSense Vibrator", nSize-1);
    szDevName[nSize - 1] = '\0';    /* make sure the string is NULL terminated */

    return VIBE_S_SUCCESS;
}
