#ifndef __LGE_PM_H__
#define __LGE_PM_H__

#define LGE_PM_DEVICE  "lge_pm_device"

// LGE_CHANGE_S,narasimha.chikka@lge.com,Add BATT_ID Check
#if defined(CONFIG_LGE_PM_BATT_ID_DETECTION)
#define BATT_UNKNOWN    0
#define BATT_NORMAL     1
#define BATT_DS2704_N   17
#define BATT_DS2704_L   32
#define BATT_ISL6296_N  73
#define BATT_ISL6296_L  94
#define BATT_DS2704_C 	48
#define BATT_ISL6296_C  105
#endif
// LGE_CHANGE_E,narasimha.chikka@lge.com,Add BATT_ID Check

#ifdef CONFIG_LGE_LOW_VOLTAGE_BATTERY_CHECK
void lge_pm_handle_poweroff(void);
#endif

#endif/*__LGE_PM_H__*/
