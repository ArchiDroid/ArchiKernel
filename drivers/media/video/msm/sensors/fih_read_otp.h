/* 
 * Copyright(C) 2013 Foxconn International Holdings, Ltd. All rights reserved.
 */

/* Product ID */
#define PID_S5K4E1      0x01 /* LiteOn - Samsung */
#define PID_S5K4E1_2ND  0x04 /* OMP- Samsung */
#define PID_AR0543      0x05 /* Aptina */

enum otp_inft_id {
	OTP_MI_PID,
	OTP_MI_SERIAL_ID,
	OTP_AWB_RG,
	OTP_AWB_BG,
	OTP_AWB_GRGB,
	OTP_AF_START_DAC,
	OTP_AF_INFINITY_DAC,
	OTP_AF_100CM_DAC,
	OTP_AF_10CM_DAC,
	OTP_SHADING_INDEX,/* MM-MC-ImplementRegSwitchMechanismForShading-00+ */
};

/* MM-MC-ImplementRegSwitchMechanismForShading-00+{ */
enum otp_shading_index {
	OTP_SHADING_1 = 1, /* Golden */
	OTP_SHADING_2,     /* Corner_60% */
	OTP_SHADING_MAX,
};
/* MM-MC-ImplementRegSwitchMechanismForShading-00+} */

extern int32_t g_IsFihMainCamProbe;
/* MM-UW-Improve camera open performance-00+{ */
extern int32_t g_IsOtpInitDone;
extern int32_t g_IsOtpInitDone_ar0543;
/* MM-UW-Improve camera open performance-00+} */

/* For S5K54E1 and S5K54E1_2ND */
int32_t fih_init_otp(struct msm_sensor_ctrl_t *s_ctrl);
int32_t fih_get_otp_data(int32_t id, int32_t* val);

/* For AR0543 */
/* MM-MC-ImplementOtpReadFunctionForAR0543-00+{ */
int32_t fih_init_otp_ar0543(struct msm_sensor_ctrl_t *s_ctrl);
int32_t fih_get_otp_data_ar0543(int32_t id, int32_t* val);
/* MM-MC-ImplementOtpReadFunctionForAR0543-00+} */

