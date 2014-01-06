/*============================================================================

FILE:      FIH_hwid.c

DESCRIPTION:
    This file support API for read FIH board ID.

PUBLIC CLASSES:
  hw_platform

===============================================================================
    Hardware ID Structure
    Below is a 32 bit data, it have determined to 3 part. We will definetion
    enum type every part.
    Erery part is a bit ID number.
    bit number: 23  16  15   8  7   0
                [0x00]  [0x00]  [0x00]
    definetion of part:
        bit23 ~ bit16   --> Band ID: this number is store RF band
                                     number(WCDMA1245, WCDMA1248, WCDMA125 etc...).

        bit15 ~ bit8    --> Phase ID: This number is store
                                      this project phase number(EVB, PR1, PR2 etc...).

        bit7  ~ bit0    --> Project ID: this number defintion for project.
=============================================================================*/


#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/hwmon.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/hwmon-sysfs.h>
#include <linux/mfd/pm8xxx/mpp.h>
#include <linux/platform_device.h>
#include <linux/mfd/pm8xxx/core.h>
#include <linux/regulator/consumer.h>
#include <linux/mfd/pm8xxx/gpio.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#include <linux/fih_hw_info.h>
#include <linux/fih_sw_info.h>
#include <mach/msm_smd.h>

#include <linux/gpio.h>


/* Extend HWID ADC detect range */
/*-----------------------
MPP Index    Min     Max
=========================
0            518     546
1            691     726
2            869     911
3            1054    1103
4            1237    1292
5            1446    1505
6            1636    1699
7            1864    1929
8            2072    2138
9            2317   2384
10           2539    2604
11           2779    2840
-------------------------*/
int hwid_decide_hw_version(unsigned int hwid_adc_mv)
{
    struct adc_convert_table {
        int id;
        char found_it;
    };

#define CHECK_ADC( LOW, HIGH )     ( hwid_adc_mv >= LOW && hwid_adc_mv <= HIGH )

    struct adc_convert_table   adc_table[] = {
        { 0, CHECK_ADC( 518 - 100, 618 ) }, //if ((hwid_adc_mv >= 518-100) && (hwid_adc_mv <= 618)) return 0;
        { 1, CHECK_ADC( 619, 797 ) },
        { 2, CHECK_ADC( 798, 982 ) },
        { 3, CHECK_ADC( 983, 1170 ) },
        { 4, CHECK_ADC( 1171, 1372 ) },
        { 5, CHECK_ADC( 1373, 1570 ) },
        { 6, CHECK_ADC( 1571, 1781 ) },
        { 7, CHECK_ADC( 1782, 2000 ) },
        { 8, CHECK_ADC( 2001, 2227 ) },
        { 9, CHECK_ADC( 2228, 2461 ) },
        { 10, CHECK_ADC( 2462, 2691 ) },
        { 11, CHECK_ADC( 2692, 2480 + 100 ) },
        { 0xffff, 1 },
    };

#undef CHECK_ADC

    int loop = 0;
    int hw_id = 0;

    for( loop = 0 ; loop < sizeof( adc_table ) / sizeof( *adc_table ) ; ++loop )
        if( ( adc_table + loop )->found_it ) {
            hw_id = ( adc_table + loop )->id;
            break;
        }
    return hw_id;
}


const fih_hwid_entry HWID_TABLE[] = {
    /* Nicki(S3A) */
    { 1, 1, 1, PROJECT_S3A, PHASE_EVM2, BAND_1245, SIM_MAX},
    { 1, 1, 2, PROJECT_S3A, PHASE_PD, BAND_18, SIM_MAX},
    { 1, 1, 3, PROJECT_S3A, PHASE_DP, BAND_18, SIM_MAX},
    { 1, 1, 4, PROJECT_S3A, PHASE_SP, BAND_18, SIM_MAX},
    { 1, 1, 5, PROJECT_S3A, PHASE_PreAP, BAND_18, SIM_MAX},
    { 1, 1, 6, PROJECT_S3A, PHASE_AP, BAND_18, SIM_MAX},
    { 1, 1, 7, PROJECT_S3A, PHASE_TP, BAND_18, SIM_MAX},
    { 1, 1, 8, PROJECT_S3A, PHASE_PQ, BAND_18, SIM_MAX},
    { 1, 1, 9, PROJECT_S3A, PHASE_TP2_MP, BAND_18, SIM_MAX},

    { 1, 2, 1, PROJECT_S3A, PHASE_EVM, BAND_1245, SIM_MAX},
    { 1, 2, 2, PROJECT_S3A, PHASE_PD, BAND_1245, SIM_MAX},
    { 1, 2, 3, PROJECT_S3A, PHASE_DP, BAND_1245, SIM_MAX},
    { 1, 2, 4, PROJECT_S3A, PHASE_SP, BAND_1245, SIM_MAX},
    { 1, 2, 5, PROJECT_S3A, PHASE_PreAP, BAND_1245, SIM_MAX},
    { 1, 2, 6, PROJECT_S3A, PHASE_AP, BAND_1245, SIM_MAX},
    { 1, 2, 7, PROJECT_S3A, PHASE_TP, BAND_1245, SIM_MAX},
    { 1, 2, 8, PROJECT_S3A, PHASE_PQ, BAND_1245, SIM_MAX},
    { 1, 2, 9, PROJECT_S3A, PHASE_TP2_MP, BAND_1245, SIM_MAX},

    { 1, 3, 2, PROJECT_S3A, PHASE_PD, BAND_18_INDIA, SIM_MAX},
    { 1, 3, 3, PROJECT_S3A, PHASE_DP, BAND_18_INDIA, SIM_MAX},
    { 1, 3, 4, PROJECT_S3A, PHASE_SP, BAND_18_INDIA, SIM_MAX},
    { 1, 3, 5, PROJECT_S3A, PHASE_PreAP, BAND_18_INDIA, SIM_MAX},
    { 1, 3, 6, PROJECT_S3A, PHASE_AP, BAND_18_INDIA, SIM_MAX},
    { 1, 3, 7, PROJECT_S3A, PHASE_TP, BAND_18_INDIA, SIM_MAX},
    { 1, 3, 8, PROJECT_S3A, PHASE_PQ, BAND_18_INDIA, SIM_MAX},
    { 1, 3, 9, PROJECT_S3A, PHASE_TP2_MP, BAND_18_INDIA, SIM_MAX},
};


/*=========================================================================

FUNCTION fih_lookup_hwid_table

DESCRIPTION
   Lookup HWID table to transfer mpp_voltage_index as virtual product id,
   virtual phase id, and virtual band id.

===========================================================================*/
unsigned long fih_hw_id_ver = 0;

fih_hw_info_type fih_lookup_hwid_table(int HWID1, int HWID2, int HWID3)
{
    int index = 0;

    u32 GPIO_SIM_ID = 98;


    fih_hw_info_type result = {PROJECT_MAX, PHASE_MAX, BAND_MAX, SIM_MAX};

    /* Using build flag separate project id. */
#if 1 //#ifdef FIH_PROJECT_S3A
    result.virtual_project_id = PROJECT_S3A;
    fih_hw_id_ver&= ~(0xff << PROJECT_ID_SHIFT_MASK);
    fih_hw_id_ver|= PROJECT_S3A << PROJECT_ID_SHIFT_MASK;
#endif

    /* Search HWID_TABLE to map 11 physical voltage range indexes to
       virtaul Project enum, HW version enum, and RF band enum  */
    for ( index=0; index < sizeof( (HWID_TABLE) ) / sizeof( (HWID_TABLE[0]) ); index++ ) {
        /* Map an enum value to a real HW name
          ** Result contains the following information.
          ** ---------------------------------------------------------------------------------
          ** Check(1 bit) Reserved (5 bit) Sim type (2bit)| Band_ID (8 bit) | Phase_ID (8 bit) | Pro_ID (8 bit)
          ** ---------------------------------------------------------------------------------
          */
        if ((HWID_TABLE[index].HWID1 == HWID1)
            &&(HWID_TABLE[index].HWID2 == HWID2)
            &&(HWID_TABLE[index].HWID3 == HWID3)) {
            result.virtual_phase_id = HWID_TABLE[index].phase_id;
            fih_hw_id_ver &= ~(0xff << PHASE_ID_SHIFT_MASK);
            fih_hw_id_ver |= HWID_TABLE[index].phase_id << PHASE_ID_SHIFT_MASK;

            result.virtual_band_id = HWID_TABLE[index].band_id;
            fih_hw_id_ver &= ~(0xff << BAND_ID_SHIFT_MASK);
            fih_hw_id_ver |= HWID_TABLE[index].band_id << BAND_ID_SHIFT_MASK;

            if (HWID_TABLE[index].sim_type == SIM_MAX) {
                if ((gpio_get_value(GPIO_SIM_ID) == 0)) {
                    result.virtual_sim_type = SINGLE_SIM;
                    printk(KERN_ERR "FIH kernel [HWID] - GPIO %d pull low. [%d]:Single SIM.\n", GPIO_SIM_ID,gpio_get_value(GPIO_SIM_ID));
                } else if (gpio_get_value(GPIO_SIM_ID) == 1) {
                    result.virtual_sim_type = DUAL_SIM;
                    printk(KERN_ERR "FIH kernel [HWID] - GPIO %d pull high. [%d]:Dual SIM.\n", GPIO_SIM_ID,gpio_get_value(GPIO_SIM_ID));
                }
            }

            fih_hw_id_ver&= ~(0x3 << SIM_ID_SHIFT_MASK);
            fih_hw_id_ver |= result.virtual_sim_type<<SIM_ID_SHIFT_MASK;
            printk(KERN_ERR "FIH kernel [HWID] - HWID_TABLE[index].sim_type = %d \n",HWID_TABLE[index].sim_type);
            printk(KERN_ERR "FIH kernel [HWID] - result.virtual_sim_type = %d \n",result.virtual_sim_type);


            return result;
        }
    }


    /* Default HWID : PHASE_PD, BAND_18, SINGLE_SIM */

    result.virtual_phase_id = PHASE_PD;
    fih_hw_id_ver &= ~(0xff << PHASE_ID_SHIFT_MASK);
    fih_hw_id_ver |= PHASE_PD << PHASE_ID_SHIFT_MASK;

    result.virtual_band_id = BAND_18;
    fih_hw_id_ver &= ~(0xff << BAND_ID_SHIFT_MASK);
    fih_hw_id_ver |= BAND_18 << BAND_ID_SHIFT_MASK;

    result.virtual_sim_type = SINGLE_SIM;
    fih_hw_id_ver &= ~(0x3 << SIM_ID_SHIFT_MASK);
    fih_hw_id_ver |= SINGLE_SIM<<SIM_ID_SHIFT_MASK;
    //this bit stand for default HWID if be set 1
    fih_hw_id_ver |=0x80000000;

#ifdef FIH_PROJECT_S3A
    /* Default HWID : PHASE_PD, BAND_18, SINGLE_SIM */

    result.virtual_phase_id = PHASE_PD;
    fih_hw_id_ver &= ~(0xff << PHASE_ID_SHIFT_MASK);
    fih_hw_id_ver |= PHASE_PD << PHASE_ID_SHIFT_MASK;

    result.virtual_band_id = BAND_18;
    fih_hw_id_ver &= ~(0xff << BAND_ID_SHIFT_MASK);
    fih_hw_id_ver |= BAND_18 << BAND_ID_SHIFT_MASK;

    result.virtual_sim_type = SINGLE_SIM;
    fih_hw_id_ver &= ~(0x3 << SIM_ID_SHIFT_MASK);
    fih_hw_id_ver |= SINGLE_SIM<<SIM_ID_SHIFT_MASK;
    //this bit stand for default HWID if be set 1
    fih_hw_id_ver |=0x80000000;
#endif

    return result;
}


//Macros assume PMIC GPIOs and MPPs start at 1
#define PM8038_GPIO_BASE		152
#define PM8038_GPIO_PM_TO_SYS(pm_gpio)	(pm_gpio - 1 + PM8038_GPIO_BASE)
#define PM8917_NR_GPIOS		38
#define PM8038_MPP_BASE			(PM8038_GPIO_BASE + PM8917_NR_GPIOS)
#define PM8038_MPP_PM_TO_SYS(pm_gpio)	(pm_gpio - 1 + PM8038_MPP_BASE)


void hwid_pull_pm_gpio(int PL1, int PL2, int PL3)
{
    int rc;

    static struct pm_gpio HWID_GPIO_high = {
        .direction		  = PM_GPIO_DIR_IN,
        .output_buffer	  = PM_GPIO_OUT_BUF_OPEN_DRAIN,
        .output_value	  = 1,
        .pull			  = PM_GPIO_PULL_NO,
        .vin_sel		  = PM_GPIO_VIN_VPH,
        .out_strength	  = PM_GPIO_STRENGTH_HIGH,
        .function		  = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol	  = 0,
        .disable_pin	  = 0,
    };

    static struct pm_gpio HWID_GPIO_low = {
        .direction		  = PM_GPIO_DIR_OUT,
        .output_buffer	  = PM_GPIO_OUT_BUF_OPEN_DRAIN,
        .output_value	  = 0,
        .pull			  = PM_GPIO_PULL_NO,
        .vin_sel		  = PM_GPIO_VIN_VPH,
        .out_strength	  = PM_GPIO_STRENGTH_HIGH,
        .function		  = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol	  = 0,
        .disable_pin	  = 0,
    };


    if (PL1 == 1) {
        rc = pm8xxx_gpio_config(PM8038_GPIO_PM_TO_SYS(10),&HWID_GPIO_high);
    } else {
        rc = pm8xxx_gpio_config(PM8038_GPIO_PM_TO_SYS(10),&HWID_GPIO_low);
    }
    if (rc) {
        printk(KERN_ERR "FIH kernel [HWID] - PM GPIO 10 setting %d fail, rc=%d\n", PL1, rc);
    }

    if (PL2 == 1) {
        rc = pm8xxx_gpio_config(PM8038_GPIO_PM_TO_SYS(11),&HWID_GPIO_high);
    } else {
        rc = pm8xxx_gpio_config(PM8038_GPIO_PM_TO_SYS(11),&HWID_GPIO_low);
    }
    if (rc) {
        printk(KERN_ERR "FIH kernel [HWID] - PM GPIO 11 setting %d fail, rc=%d\n", PL2, rc);
    }

    if (PL3 == 1) {
        rc = pm8xxx_gpio_config(PM8038_GPIO_PM_TO_SYS(12),&HWID_GPIO_high);
    } else {
        rc = pm8xxx_gpio_config(PM8038_GPIO_PM_TO_SYS(12),&HWID_GPIO_low);
    }
    if (rc) {
        printk(KERN_ERR "FIH kernel [HWID] - PM GPIO 12 setting %d fail, rc=%d\n", PL3, rc);
    }
    msleep(1);
}

#define HW_REVISION_NUMBER_ADDR	0x00802054
uint32_t HW_REVISION_NUMBER = 0;

int fih_hwid_read(void)
{
    struct product_id_info {
        fih_product_id_type product_id_num;
        char *name;
    };

    struct product_id_info product_id_info_name[] = {
        { PROJECT_S3A, "S3A" },
        { PROJECT_MAX, "MAX" },
    };


    struct phase_id_info {
        fih_product_phase_type phase_id_num;
        char *name;
    };

    struct phase_id_info phase_id_info_name[] = {
        { PHASE_EVM, "EVM" },
        { PHASE_EVM2, "EVM2" },
        { PHASE_PD, "PD" },
        { PHASE_DP, "DP" },
        { PHASE_SP, "SP" },
        { PHASE_PreAP, "PreAP" },
        { PHASE_AP, "AP" },
        { PHASE_TP, "TP" },
        { PHASE_PQ, "PQ" },
        { PHASE_TP2_MP, "TP2_MP" },
        { PHASE_ANY, "ANY" },
        { PHASE_MAX, "MAX" },
    };


    struct band_id_info {
        fih_band_id_type band_id_num;
        char *name;
    };

    struct band_id_info band_id_info_name[] = {
        { BAND_18, "BAND_18" },
        { BAND_1245, "BAND_1245" },
        { BAND_18_INDIA, "BAND_18_INDIA" },
        { BAND_MAX, "BAND_MAX" },
    };


    struct sim_type_info {
        fih_sim_type sim_type_num;
        char *name;
    };

    struct sim_type_info sim_type_info_name[] = {
        { SINGLE_SIM, "SINGLE_SIM" },
        { DUAL_SIM, "DUAL_SIM" },
        { SIM_MAX, "SIM_MAX" },
    };


    int loop;
    int rc = 0;
    int HWID_1_uv = 0, HWID_2_uv = 0, HWID_3_uv = 0; //in micro-volts.
    int HWID_1_mv = 0, HWID_2_mv = 0, HWID_3_mv = 0; //in milli-volts
    int HWID_1_mv_range = 0, HWID_2_mv_range = 0, HWID_3_mv_range = 0;
    static struct smem_oem_info oem_info = {0};
    static unsigned int fih_product_id = 0;
    static unsigned int fih_product_phase = 0;
    static unsigned int fih_band_id = 0;
    static unsigned int fih_sim_type_value = 0;

    struct smem_oem_info *fih_smem_info = smem_alloc2(SMEM_ID_VENDOR0, sizeof(oem_info));

    struct pm8xxx_adc_chan_result adc_result;
    struct pm8xxx_mpp_config_data hwid_mpp_init = {
        .type       = PM8XXX_MPP_TYPE_A_INPUT,
        .level      = PM8XXX_MPP_AIN_AMUX_CH7,
        .control    = PM8XXX_MPP_DOUT_CTRL_LOW,
    };

    static struct pm_gpio PM8038_GPIO_DISABLE = {
        .direction        = PM_GPIO_DIR_IN,
        .output_buffer    = 0,
        .output_value     = 0,
        .pull             = 0,
        .vin_sel          = PM8038_GPIO_VIN_L11,
        .out_strength     = 0,
        .function         = 0,
        .inv_int_pol      = 0,
        .disable_pin      = 1,
    };

    char *cp;
    uint32_t *hp;
    printk(KERN_DEBUG "FIH kernel [HWID] - fih_hwid_probe++\n");


    rc = pm8xxx_mpp_config(PM8038_MPP_PM_TO_SYS(5), &hwid_mpp_init);
    if (rc) {
        printk(KERN_DEBUG "FIH kernel [HWID] - pm8xxx_mpp_config: rc=%d\n", rc);
        return 0;
    }


    hwid_pull_pm_gpio(0,1,1); //HW ID: Pull PM_GPIO_10, 11, 12 as (L, H, H), Set TRUE/FALSE to output HIGH/LOW
    rc = pm8xxx_adc_read(CHANNEL_MPP_2, &adc_result); //get mV from MPP5
    if (rc != 0)
        pr_err("%s: pm8xxx_adc_read fail: rc=%d\n", __func__, rc);

    HWID_1_uv = adc_result.physical;
    HWID_1_mv = HWID_1_uv/1000;
    HWID_1_mv_range = hwid_decide_hw_version(HWID_1_mv);


    hwid_pull_pm_gpio(1,0,1); //Band ID: Pull PM_GPIO_10, 11, 12 as (H, L, H), Set TRUE/FALSE to output HIGH/LOW
    rc = pm8xxx_adc_read(CHANNEL_MPP_2, &adc_result); //get mV from MPP5
    if (rc != 0)
        pr_err("%s: pm8xxx_adc_read fail: rc=%d\n", __func__, rc);

    HWID_2_uv = adc_result.physical;
    HWID_2_mv = HWID_2_uv/1000;
    HWID_2_mv_range = hwid_decide_hw_version(HWID_2_mv);


    hwid_pull_pm_gpio(1,1,0); //Phase ID: Pull PM_GPIO_10, 11, 12 as (H, H, L), Set TRUE/FALSE to output HIGH/LOW
    rc = pm8xxx_adc_read(CHANNEL_MPP_2, &adc_result); //get mV from MPP5
    if (rc != 0)
        pr_err("%s: pm8xxx_adc_read fail: rc=%d\n", __func__, rc);


    HWID_3_uv = adc_result.physical;
    HWID_3_mv = HWID_3_uv/1000;
    HWID_3_mv_range = hwid_decide_hw_version(HWID_3_mv);


    printk(KERN_DEBUG "FIH kernel [HWID] - HWID ADC = %d, %d, %d\n", HWID_1_mv, HWID_2_mv, HWID_3_mv);
    printk(KERN_DEBUG "FIH kernel [HWID] - HWID = %d %d %d\n", HWID_1_mv_range, HWID_2_mv_range, HWID_3_mv_range);

    fih_lookup_hwid_table(HWID_1_mv_range, HWID_2_mv_range, HWID_3_mv_range);


    printk(KERN_DEBUG "FIH kernel [HWID] - fih_hw_id_ver = 0x%lx\n",fih_hw_id_ver);

    //get product id
    fih_product_id = fih_hw_id_ver&0xff;
    printk(KERN_DEBUG "FIH kernel [HWID] - fih_product_id = %d \n",fih_product_id);

    //get product phase
    fih_product_phase = (fih_hw_id_ver>>PHASE_ID_SHIFT_MASK)&0xff;
    printk(KERN_DEBUG "FIH kernel [HWID] - fih_product_phase = %d \n",fih_product_phase);

    //get band id
    fih_band_id = (fih_hw_id_ver>>BAND_ID_SHIFT_MASK)&0xff;
    printk(KERN_DEBUG "FIH kernel [HWID] - fih_band_id = %d \n",fih_band_id);

    fih_sim_type_value= (fih_hw_id_ver>>SIM_ID_SHIFT_MASK)&0x3;
    printk(KERN_DEBUG "FIH kernel [HWID] - fih_sim_type_value = %d \n",fih_sim_type_value);

    fih_smem_info->hw_id = fih_hw_id_ver;


    printk( "\n\n====================FIH HWID====================[\n" );
    printk("FIH kernel [HWID] - 0x%lx <-- fih_hw_id_ver\n",fih_hw_id_ver);
    for( loop = 0 ; loop < sizeof( product_id_info_name ) / sizeof( *product_id_info_name ) ; ++loop ) {
        if( ( product_id_info_name + loop )->product_id_num == fih_product_id ) {
            printk( "FIH kernel [HWID] - %s <-- fih_product_id name\n", ( product_id_info_name + loop )->name );
            break;
        }
    }

    for( loop = 0 ; loop < sizeof( phase_id_info_name ) / sizeof( *phase_id_info_name ) ; ++loop ) {
        if( ( phase_id_info_name + loop )->phase_id_num == fih_product_phase ) {
            printk( "FIH kernel [HWID] - %s <-- fih_product_phase name\n", ( phase_id_info_name + loop )->name );
            break;
        }
    }

    for( loop = 0 ; loop < sizeof( band_id_info_name ) / sizeof( *band_id_info_name ) ; ++loop ) {
        if( ( band_id_info_name + loop )->band_id_num == fih_band_id ) {
            printk( "FIH kernel [HWID] - %s <-- fih_band_id name\n", ( band_id_info_name + loop )->name );
            break;
        }
    }

    for( loop = 0 ; loop < sizeof( sim_type_info_name ) / sizeof( *sim_type_info_name ) ; ++loop ) {
        if( ( sim_type_info_name + loop )->sim_type_num == fih_sim_type_value ) {
            printk( "FIH kernel [HWID] - %s <-- fih_sim_type_value name\n", ( sim_type_info_name + loop )->name );
            break;
        }
    }

    cp = (char *)ioremap(HW_REVISION_NUMBER_ADDR, 0x16);
    if (cp == NULL)
        printk("FIH kernel [HWID] - NULL <-- HW_REVISION_NUMBER read fail.\n");
    else {
        hp = (uint32_t *)cp;	/* HW_REVISION_NUMBER */
        HW_REVISION_NUMBER = *hp;
        printk("FIH kernel [HWID] - 0x%x <-- HW_REVISION_NUMBER\n",HW_REVISION_NUMBER);
    }

    printk( "====================FIH HWID====================]\n\n" );


    fih_get_oem_info();
    fih_info_init();


    // Disable the hwid pin setting[
    rc = pm8xxx_gpio_config(PM8038_GPIO_PM_TO_SYS(10),
                            &PM8038_GPIO_DISABLE);
    if (rc != 0)
        pr_err("%s: pm8xxx_gpio_config: rc=%d\n", __func__, rc);

    rc = pm8xxx_gpio_config(PM8038_GPIO_PM_TO_SYS(11),
                            &PM8038_GPIO_DISABLE);
    if (rc != 0)
        pr_err("%s: pm8xxx_gpio_config: rc=%d\n", __func__, rc);

    rc = pm8xxx_gpio_config(PM8038_GPIO_PM_TO_SYS(12),
                            &PM8038_GPIO_DISABLE);
    if (rc != 0)
        pr_err("%s: pm8xxx_gpio_config: rc=%d\n", __func__, rc);
    // Disable the hwid pin setting]



    printk(KERN_DEBUG "FIH kernel [HWID] - fih_hwid_probe--\n");

    return 0;
}

