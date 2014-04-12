/* 
 * Copyright(C) 2013 Foxconn International Holdings, Ltd. All rights reserved.
 */

#include "msm_sensor.h"
#include "fih_read_otp.h"

#define OTP_INFO_MAX 10

/* REG define */
#define OTP_SWITCH_REG   0x304C
#define OTP_POLL_REG     0x304A

#define OTP_PID_REG      0x3800 /* Product ID */
#define OTP_SID_1_REG    0x3802 /* Serial No */
#define OTP_SID_2_REG    0x3804 /* Serial No */
#define OTP_AWB_RG_REG   0x3806 /* AWB-RG */
#define OTP_AWB_BG_REG   0x3808 /* AWB-BG */
#define OTP_AWB_GRGB_REG 0x380A /* AWB-GrGb */
#define OTP_AF_INFINITY_REG 0x380C /* Infinity (DAC code) */
#define OTP_AF_100CM_REG    0x380E /* 100cm (DAC code) */
#define OTP_AF_10CM_REG     0x3810 /* 10cm (DAC code) */
#define OTP_SHADING_INDEX_REG 0x3812 

#define TYPE_MAX 3
enum otp_type_index {
	TYPE_30 = 30,
	TYPE_31,
	TYPE_32,
};

static struct msm_camera_i2c_reg_conf otp_init_settings_t32[] = {
	{0x301A, 0x0610},/* Disable streaming */
    {0x3134, 0xCD95},/* Timing parameters for otp read */
    {0x304C, 0x3200},/* Choose to only read record type 0x30 */
    {0x304A, 0x0200},/* Only read single record at a time */
    {0x304A, 0x0210},/* Auto read start */  
};

static struct msm_camera_i2c_reg_conf otp_init_settings_t31[] = {
    {0x304C, 0x3100},/* Choose to only read record type 0x30 */
    {0x304A, 0x0200},/* Only read single record at a time */
    {0x304A, 0x0210},/* Auto read start */  
};

static struct msm_camera_i2c_reg_conf otp_init_settings_t30[] = {
    {0x304C, 0x3000},/* Choose to only read record type 0x30 */
    {0x304A, 0x0200},/* Only read single record at a time */
    {0x304A, 0x0210},/* Auto read start */    
};

int32_t g_IsOtpInitDone_ar0543     = 0;
uint16_t otp_data_ar0543[OTP_INFO_MAX];

int32_t fih_check_otp_status_ar0543(struct msm_sensor_ctrl_t *s_ctrl, int otp_type)
{
    int32_t  rc = 0;
    uint16_t i  = 0;
    uint16_t read_val = 0;

    switch (otp_type) {
        case TYPE_30:
            rc = msm_camera_i2c_write_tbl(
                    s_ctrl->sensor_i2c_client,
                    otp_init_settings_t30,
                    ARRAY_SIZE(otp_init_settings_t30),
                    MSM_CAMERA_I2C_WORD_DATA);
            break;
        case TYPE_31:
            rc = msm_camera_i2c_write_tbl(
                    s_ctrl->sensor_i2c_client,
                    otp_init_settings_t31,
                    ARRAY_SIZE(otp_init_settings_t31),
                    MSM_CAMERA_I2C_WORD_DATA);
            break;
        case TYPE_32:
            rc = msm_camera_i2c_write_tbl(
                    s_ctrl->sensor_i2c_client,
                    otp_init_settings_t32,
                    ARRAY_SIZE(otp_init_settings_t32),
                    MSM_CAMERA_I2C_WORD_DATA);
            break; 
        default:
            pr_err("fih_check_otp_status_ar0543: ERR: Invalid otp_type = %d !\n", otp_type);
            rc = -EFAULT;
            break;
    }
    if (rc < 0)
    {
        pr_err("fih_check_otp_status_ar0543: Write REG table failed, otp_type = %d !\n", otp_type);
        return rc;
    }

    /* Polling for OTPM load */
    do {
        rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,OTP_POLL_REG,&read_val,
             MSM_CAMERA_I2C_WORD_DATA);
        if (rc < 0)
        {
            pr_err("fih_check_otp_status_ar0543: Read OTP_POLL_REG for polling failed !\n");
            return rc;
        }
        
        if (0x60 == (read_val & 0x60))
        {
            break;
        }
        msleep(10);
        i++;
    }while(i < 10);
    
    if (i >= 10)
    {
        pr_err("fih_check_otp_status_ar0543: Polling for OTPM load timeout !\n");
        rc = -EFAULT;
        return rc;
    }
    pr_err("fih_check_otp_status_ar0543: Check otp_type = %d success, rc = %d.\n", otp_type, rc);
    return rc;
}



int32_t fih_parse_otp_info_ar0543(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t  rc       = 0;
    int32_t  i        = 0;
    uint16_t read_val = 0;

    for (i = 0; i < TYPE_MAX; i++) 
    {
        if (fih_check_otp_status_ar0543(s_ctrl, TYPE_32 - i) >= 0)
        {
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,OTP_PID_REG,&read_val,
                 MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0)
            {
                pr_err("fih_parse_otp_info_ar0543: Read OTP_REG_PID failed, otp_type = %d !\n", (TYPE_32 - i));
                return rc;
            }

            if (read_val != 0)
            {
                printk("fih_parse_otp_info_ar0543: Start parse OTP data, otp_type = %d !\n", (TYPE_32 - i));
                goto PARSE_DATA;
            }
            else
                printk("fih_parse_otp_info_ar0543: TYPE_%d has no valid otp data.\n", (TYPE_32 - i));
        }
    }
    
    if (read_val == 0)
    {
        pr_err("fih_parse_otp_info_ar0543: ERR: All type have no valid otp data !\n");
        rc = -EFAULT;
        return rc;
    }

PARSE_DATA:

    for (i = 0; i < OTP_INFO_MAX; i++) 
    {
      rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (OTP_PID_REG + i*2), &read_val, MSM_CAMERA_I2C_WORD_DATA);
      if (rc < 0)
      {
          pr_err("fih_parse_otp_info_ar0543: Read REG_0x%x failed !\n", (OTP_PID_REG + i*2));
          return rc;
      }
      otp_data_ar0543[i] = read_val;
      printk("fih_parse_otp_info_ar0543: otp_data_ar0543[%d] = REG_0x%x = 0x%x. \n", i, (OTP_PID_REG + i*2), otp_data_ar0543[i]);
    }

    printk("fih_parse_otp_info_ar0543: Success.\n");
    return rc;
}

int32_t fih_init_otp_ar0543(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;
    int32_t i  = 0;
    //g_IsOtpInitDone_ar0543 = 0; /* MM-UW-Improve camera open performance-00+{ */

    /* Init OTP data array. */
    for (i = 0; i < OTP_INFO_MAX; i++)
        otp_data_ar0543[i] = 0;

    rc = fih_parse_otp_info_ar0543(s_ctrl);
    if (rc < 0)
    {
        pr_err("fih_init_otp_ar0543: fih_parse_otp_info() failed !\n");
        return rc;
    }

    g_IsOtpInitDone_ar0543 = 1;
    printk("fih_init_otp_ar0543: Init success. \n");
    return rc;
}


int32_t fih_get_otp_data_ar0543(int32_t id, int32_t* val)
{
    int32_t rc = 0;
    int32_t otp_val = 0;

    if (g_IsOtpInitDone_ar0543 == 0)
    {
        pr_err("fih_get_otp_data_ar0543: ERR: Please init OTP before get OTP data !\n");
    }

    switch (id) {
    case OTP_MI_PID:
        otp_val = otp_data_ar0543[0];
        break;
    case OTP_MI_SERIAL_ID:
        otp_val = otp_val | (otp_data_ar0543[1] << 8);
        otp_val = otp_val |  otp_data_ar0543[2];
        break;
    case OTP_AWB_RG:
        otp_val = otp_data_ar0543[3];
        break;
    case OTP_AWB_BG:
        otp_val = otp_data_ar0543[4];
        break;
    case OTP_AWB_GRGB:
        otp_val = otp_data_ar0543[5];
        break;
    case OTP_AF_INFINITY_DAC:
        otp_val = otp_data_ar0543[6];
        break;
    case OTP_AF_100CM_DAC:
        otp_val = otp_data_ar0543[7];
        break;
    case OTP_AF_10CM_DAC:
        otp_val = otp_data_ar0543[8];
        break;
    /* MM-MC-ImplementRegSwitchMechanismForShading-01+{ */
    case OTP_SHADING_INDEX:
        otp_val = otp_data_ar0543[9];
        break;
    /* MM-MC-ImplementRegSwitchMechanismForShading-01+} */
    default:
        pr_err("fih_get_otp_data_ar0543: ERR: Invalid ID !\n");
        rc = -EFAULT;
        break;
    }

    if (rc >= 0)
    {
        *val = otp_val;
        printk("fih_get_otp_data_ar0543: ID = %d, otp_val = 0x%x. \n", id, otp_val);
    }
    
    return rc;
}
