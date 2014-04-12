/*
*Copyright(C) 2011-2012 Foxconn International Holdings, Ltd. All rights reserved
*/
#ifndef _FIH_HW_INFO_H
#define _FIH_HW_INFO_H

typedef enum 
{
  PROJECT_S3A = 1,
  PROJECT_MAX = 0xFF
} fih_product_id_type;

typedef enum 
{
	PHASE_EVM = 10,
	PHASE_EVM2 = 15,
	PHASE_PD = 20,
	PHASE_DP = 30,
	PHASE_SP = 40,
	PHASE_PreAP = 45,
	PHASE_AP = 50,
	PHASE_TP = 60,
	PHASE_PQ = 70,
	PHASE_TP2_MP = 80,
	PHASE_ANY = 0XFE,
	PHASE_MAX = 0XFF
}fih_product_phase_type;

typedef enum 
{
  BAND_18 = 1,
  BAND_1245 = 2,
  BAND_18_INDIA = 3,
  BAND_MAX = 0xFF
}fih_band_id_type;

typedef enum {
    SINGLE_SIM=1,
    DUAL_SIM,
    SIM_MAX=0xFF,
}fih_sim_type;

typedef struct {
  fih_product_id_type virtual_project_id;
  fih_product_phase_type virtual_phase_id;
  fih_band_id_type virtual_band_id;
  fih_sim_type virtual_sim_type;  
}fih_hw_info_type;

typedef struct {
  int HWID1;
  int HWID2;
  int HWID3;
  fih_product_id_type project_id;
  fih_product_phase_type phase_id;
  fih_band_id_type band_id;
  fih_sim_type sim_type;  
}fih_hwid_entry;

//MTD-BSP-LC-SMEM-00+[
void fih_get_oem_info(void);
void fih_info_init(void);
unsigned int fih_get_product_id(void);
unsigned int fih_get_product_phase(void);
unsigned int fih_get_band_id(void);
unsigned int fih_get_sim_id(void);
int fih_hwid_read(void);
char *fih_get_amss_version(void);
//MTD-BSP-LC-SMEM-00+]
char *fih_get_nonHLOS_git_head(void);  //BSP-REXER-GIT-00+
void fih_get_NONHLOS_version(void);    //MTD-BSP-LC-Get_Version-00 +

#endif
