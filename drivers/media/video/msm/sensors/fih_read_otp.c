/* 
 * Copyright(C) 2013 Foxconn International Holdings, Ltd. All rights reserved.
 */

#include "msm_sensor.h"
#include "fih_read_otp.h"

#define OTP_INFO_MAX 34/* MM-MC-ImplementRegSwitchMechanismForShading-00* */
#define OTP_BANK_MAX 5
//REG define
#define OTP_BANK_REG_B1B2 0x310F
#define OTP_BANK_REG_B3B4 0x310E
#define OTP_BANK_REG_B5   0x310D
#define OTP_REG_SWITCH    0x310C

static struct msm_camera_i2c_reg_conf otp_init_settings[] = {
	{0x30F9, 0x0E},
    {0x30FA, 0x0A},
    {0x30FB, 0x71},
    {0x30FB, 0x70},
};

int32_t g_IsOtpInitDone     = 0;
int32_t g_IsFihMainCamProbe = 0;//Use to reduce main camera probe times.
uint8_t otp_data[OTP_INFO_MAX];

int32_t fih_parse_otp_info(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t  rc       = 0;
    int32_t  i        = 0;
    uint8_t  bank_val = 0;
    uint16_t val_b2b1 = 0;
    uint16_t val_b4b3 = 0;
    uint16_t val_bxb5 = 0;

    for (i = 0; i < OTP_INFO_MAX; i++)
    {
        //Select address 0~Max for switch contents
    	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, OTP_REG_SWITCH, i, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
            pr_err("fih_parse_otp_info: Write switch reg failed, content = %d !\n", i);
            goto ERR;
        }

        //Read bank1 and bank2 value. (One bank size = 4bits)
        rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, OTP_BANK_REG_B1B2, &val_b2b1, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
            pr_err("fih_parse_otp_info: Read bank1 and bank2 value failed !\n");
            goto ERR;
        }

        //Read bank3 and bank4 value. (One bank size = 4bits)
        rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, OTP_BANK_REG_B3B4, &val_b4b3, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
            pr_err("fih_parse_otp_info: Read bank3 and bank4 value failed !\n");
            goto ERR;
        } 

        //Read bank5 value. (One bank size = 4bits)
        rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, OTP_BANK_REG_B5, &val_bxb5, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
            pr_err("fih_parse_otp_info: Read bank5 value failed !\n");
            goto ERR;
        }

        if (val_bxb5 != 0)
        {
            //Get and parse bank5 value
            bank_val = (uint8_t)(val_bxb5 & 0x0F);
            otp_data[i] = bank_val;
            printk("fih_parse_otp_info: otp_data[%d] = bank5 = 0x%x. \n", i, otp_data[i]);
            continue;
        }

        if (val_b4b3 != 0)
        {
            //Get and parse bank4 value
            bank_val = (uint8_t)(val_b4b3 >> 4);
            if (bank_val != 0)
            {
                otp_data[i] = bank_val;
                printk("fih_parse_otp_info: otp_data[%d] = bank4 = 0x%x. \n", i, otp_data[i]);
                continue;
            }
        
            //Get and parse bank3 value
            bank_val = (uint8_t)(val_b4b3 & 0x0F);
            if (bank_val != 0)
            {
                otp_data[i] = bank_val;
                printk("fih_parse_otp_info: otp_data[%d] = bank3 = 0x%x. \n", i, otp_data[i]);
                continue;
            }
        }

        if (val_b2b1 != 0)
        {
            //Get and parse bank2 value
            bank_val = (uint8_t)(val_b2b1 >> 4);
            if (bank_val != 0)
            {
                otp_data[i] = bank_val;
                printk("fih_parse_otp_info: otp_data[%d] = bank2 = 0x%x. \n", i, otp_data[i]);
                continue;
            }

            //Get and parse bank1 value
            bank_val = (uint8_t)(val_b2b1 & 0x0F);
            if (bank_val != 0)
            {
                otp_data[i] = bank_val;
                printk("fih_parse_otp_info: otp_data[%d] = bank1 = 0x%x. \n", i, otp_data[i]);
                continue;
            }
        }
    }

    printk("fih_parse_otp_info: Success. \n");
ERR:
    return rc;
}

int32_t fih_init_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;
    int32_t i  = 0;
    //g_IsOtpInitDone = 0; /* MM-UW-Improve camera open performance-00+{ */

    rc = msm_camera_i2c_write_tbl(
            s_ctrl->sensor_i2c_client,
            otp_init_settings,
            ARRAY_SIZE(otp_init_settings),
            MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
    {
        pr_err("fih_init_otp: Write INIT table failed !\n");
        return rc;
    }
    usleep(3200);//Waiting time : 3.2ms

    //Init OTP data array.
    for (i = 0; i < OTP_INFO_MAX; i++)
        otp_data[i] = 0;

    rc = fih_parse_otp_info(s_ctrl);
    if (rc < 0)
    {
        pr_err("fih_init_otp: fih_parse_otp_info() failed !\n");
        return rc;
    }

    g_IsOtpInitDone = 1;
    printk("fih_init_otp: Init success. \n");
    return rc;
}


int32_t fih_get_otp_data(int32_t id, int32_t* val)
{
    int32_t rc = 0;
    int32_t otp_val = 0;

    if (g_IsOtpInitDone == 0)
    {
        pr_err("fih_get_otp_data: ERR: Please init OTP before get OTP data !\n");
    }

    switch (id) {
    case OTP_MI_PID:
        otp_val = otp_data[0];
        break;
    case OTP_MI_SERIAL_ID:
        otp_val = otp_val |  otp_data[8];
        otp_val = otp_val | (otp_data[7] << 4);
        otp_val = otp_val | (otp_data[6] << 8);
        otp_val = otp_val | (otp_data[5] << 12);
        otp_val = otp_val | (otp_data[4] << 16);
        otp_val = otp_val | (otp_data[3] << 20); 
        otp_val = otp_val | (otp_data[2] << 24);
        otp_val = otp_val | (otp_data[1] << 28);
        break;
    case OTP_AWB_RG:
        otp_val = otp_val |  otp_data[12];
        otp_val = otp_val | (otp_data[11] << 4);
        otp_val = otp_val | (otp_data[10] << 8);
        otp_val = otp_val | (otp_data[9] << 12);
        break;
    case OTP_AWB_BG:
        otp_val = otp_val |  otp_data[16];
        otp_val = otp_val | (otp_data[15] << 4);
        otp_val = otp_val | (otp_data[14] << 8);
        otp_val = otp_val | (otp_data[13] << 12);
        break;
    case OTP_AWB_GRGB:
        otp_val = otp_val |  otp_data[20];
        otp_val = otp_val | (otp_data[19] << 4);
        otp_val = otp_val | (otp_data[18] << 8);
        otp_val = otp_val | (otp_data[17] << 12);
        break;
    case OTP_AF_START_DAC:
        otp_val = otp_val |  otp_data[23];
        otp_val = otp_val | (otp_data[22] << 4);
        otp_val = otp_val | (otp_data[21] << 8);
        break;
    case OTP_AF_INFINITY_DAC:
        otp_val = otp_val |  otp_data[26];
        otp_val = otp_val | (otp_data[25] << 4);
        otp_val = otp_val | (otp_data[24] << 8);
        break;
    case OTP_AF_100CM_DAC:
        otp_val = otp_val |  otp_data[29];
        otp_val = otp_val | (otp_data[28] << 4);
        otp_val = otp_val | (otp_data[27] << 8);
        break;
    case OTP_AF_10CM_DAC:
        otp_val = otp_val |  otp_data[32];
        otp_val = otp_val | (otp_data[31] << 4);
        otp_val = otp_val | (otp_data[30] << 8);
        break;
    /* MM-MC-ImplementRegSwitchMechanismForShading-00+{ */
    case OTP_SHADING_INDEX:
        otp_val = otp_data[33];
        break;
    /* MM-MC-ImplementRegSwitchMechanismForShading-00+} */
        
    default:
        pr_err("fih_get_otp_data: ERR: Invalid ID !\n");
        rc = -EFAULT;
        break;
    }

    if (rc >= 0)
    {
        *val = otp_val;
        printk("fih_get_otp_data: ID = %d, otp_val = 0x%x. \n", id, otp_val);
    }
    
    return rc;
}
