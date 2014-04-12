/*
* Copyright(C) 2011-2012 Foxconn International Holdings, Ltd. All rights reserved
*/
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/fih_hw_info.h>

#include <linux/version_host.h>    //MTD-BSP-LC-Show_Version-00 +

unsigned int get_boot_info(void);  /* MTD-BSP-VT-RECOVERY-00+ */

static int proc_calc_metrics(char *page, char **start, off_t off,
                             int count, int *eof, int len)
{
    if (len <= off+count) *eof = 1;
    *start = page + off;
    len -= off;
    if (len>count) len = count;
    if (len<0) len = 0;
    return len;
}


static int device_model_read_proc(char *page, char **start, off_t off,
                                  int count, int *eof, void *data)
{
    int len;
    int pi = fih_get_product_id();
    char ver[24]= {0} ;

    switch (pi) {
    case PROJECT_S3A:
        strncpy(ver, "S3A",3);
        ver[3]='\0';
        break;
    default:
        strncpy(ver, "Unkonwn Device Model",20);
        ver[20]='\0';
        break;
    }

    len = snprintf(page, count, "%s\n",
                   ver);  /* MTD-BSP-VT-PROC-00* */

    return proc_calc_metrics(page, start, off, count, eof, len);
}

static int phase_id_read_proc(char *page, char **start, off_t off,
                              int count, int *eof, void *data)
{
    int len;

    int pp = fih_get_product_phase();
    char ver[25]= {0};

    switch (pp) {
    case PHASE_EVM2:
        strncpy(ver, "EVM2", 4);
        ver[4]='\0';
        break;
    case PHASE_EVM:
        strncpy(ver, "EVM", 3);
        ver[3]='\0';
        break;
    case PHASE_PD:
        strncpy(ver, "PD", 2);
        ver[2]='\0';
        break;
    case PHASE_DP:
        strncpy(ver, "DP", 2);
        ver[2]='\0';
        break;
    case PHASE_SP:
        strncpy(ver, "SP", 2);
        ver[3]='\0';
        break;
    case PHASE_PreAP:
        strncpy(ver, "PreAP", 5);
        ver[5]='\0';
        break;
    case PHASE_AP:
        strncpy(ver, "AP", 2);
        ver[2]='\0';
        break;
    case PHASE_TP:
        strncpy(ver, "TP", 2);
        ver[2]='\0';
        break;
    case PHASE_PQ :
        strncpy(ver, "PQ", 2);
        ver[2]='\0';
        break;
    case PHASE_TP2_MP:
        strncpy(ver, "TP2_MP", 6);
        ver[6]='\0';
        break;
    default:
        strncpy(ver, "Unkonwn Baseband version",24);
        ver[24]='\0';
        break;
    }

    len = snprintf(page, count, "%s\n",
                   ver);  /* MTD-BSP-VT-PROC-00* */

    return proc_calc_metrics(page, start, off, count, eof, len);
}

static int band_read_proc(char *page, char **start, off_t off,
                          int count, int *eof, void *data)
{
    int len;
    int pi = fih_get_band_id();
    char ver[40]= {0};

    switch (pi) {
    case BAND_18:
        strncpy( ver, "BAND_18", 7);
        ver[7]='\0';
        break;
    case BAND_1245:
        strncpy( ver, "BAND_1245", 9);
        ver[9]='\0';
        break;
    case BAND_18_INDIA:
        strncpy( ver, "BAND_18_INDIA", 13);
        ver[13]='\0';
        break;
    default:
        strncpy( ver, "Unkonwn RF band id", 18);
        ver[18]='\0';
        break;
    }
    len = snprintf(page, count, "%s\n",ver);  /* MTD-BSP-VT-PROC-00* */

    return proc_calc_metrics(page, start, off, count, eof, len);
}

static int siminfo_read_proc(char *page, char **start, off_t off,
                             int count, int *eof, void *data)
{
    int len;
    int pi = fih_get_sim_id();
    char ver[24]= {0} ;

    switch (pi) {
    case SINGLE_SIM:
        strncpy( ver, "SINGLE SIM", 10);
        ver[10]='\0';
        break;
    case DUAL_SIM:
        strncpy(ver, "DUAL SIM",8);
        ver[8]='\0';
        break;
    default:
        strncpy(ver, "Unkonwn SIM TYPE",16);
        ver[16]='\0';
        break;
    }

    len = snprintf(page, count, "%s\n", ver);  /* MTD-BSP-VT-PROC-00* */

    return proc_calc_metrics(page, start, off, count, eof, len);
}

//MTD-BSP-LC-Get_Version-00 *[
#if 1

static int amss_version_read_proc(char *page, char **start, off_t off,
                                  int count, int *eof, void *data)
{
    int len;
    char ver[30];
    snprintf(ver, sizeof(ver), fih_get_amss_version());

    len = snprintf(page, count, "%s\n",ver);  /* MTD-BSP-VT-PROC-00* */

    return proc_calc_metrics(page, start, off, count, eof, len);
}
#endif
//MTD-BSP-LC-Get_Version-00 *]


//MTD-BSP-LC-Show_Version-00 *[
static int linux_version_read_proc(char *page, char **start, off_t off,
                                   int count, int *eof, void *data)
{
    int len;
    /* MTD-BSP-VT-PROC-00* */
    len = snprintf(page, count, "%s.%s.%s.%s\n",
                   VER_HOST_BSP_VERSION,
                   VER_HOST_PLATFORM_NUMBER,
                   VER_HOST_BRANCH_NUMBER,
                   VER_HOST_BUILD_NUMBER);

    return proc_calc_metrics(page, start, off, count, eof, len);
}


extern uint32_t HW_REVISION_NUMBER;

static int hw_rev_read_proc(char *page, char **start, off_t off,
                            int count, int *eof, void *data)
{
    int len;

    char HW_REVISION_NUMBER_str[32];
    sprintf(HW_REVISION_NUMBER_str, "%x", HW_REVISION_NUMBER);

    len = snprintf(page, count, "%s\n", HW_REVISION_NUMBER_str);

    return proc_calc_metrics(page, start, off, count, eof, len);
}


#if 0
//BSP-REXER-GIT-00+[
static int HLOS_git_head_read_proc(char *page, char **start, off_t off,
                                   int count, int *eof, void *data)
{
    int len;

    len = snprintf(page, count, "%s\n", VER_HOST_GIT_COMMIT);

    return proc_calc_metrics(page, start, off, count, eof, len);
}
static int nonHLOS_git_head_read_proc(char *page, char **start, off_t off,
                                      int count, int *eof, void *data)
{
    int len;
    char head[64];
    snprintf(head, sizeof(head), fih_get_nonHLOS_git_head());

    len = snprintf(page, count, "%s\n",head);

    return proc_calc_metrics(page, start, off, count, eof, len);
}
//BSP-REXER-GIT-00+]
//MTD-BSP-LC-Show_Version-00 *]
#endif

#if 0
/* MTD-BSP-VT-RECOVERY-00*[ */
static int reboot_info_read_proc(char *page, char **start, off_t off,
                                 int count, int *eof, void *data)
{
    int len;
    char ver[10]= {0};
    unsigned int boot_info = get_boot_info();

    if (boot_info == 0x77665502) {
        strncpy( ver, "recovery", 8);
        ver[8]='\0';
    } else {
        strncpy( ver, "normal", 6);
        ver[6]='\0';
    }
    len = snprintf(page, count, "%s\n",ver); /* MTD-BSP-VT-PROC-00* */

    return proc_calc_metrics(page, start, off, count, eof, len);
}
/* MTD-BSP-VT-RECOVERY-00*] */
#endif


static struct {
    char *name;
    int (*read_proc)(char*,char**,off_t,int,int*,void*);
} *p, fih_info[] = {
    {"devmodel",	device_model_read_proc},
    {"phaseid",	phase_id_read_proc},
    {"bandinfo",	band_read_proc},
    {"HW_REVISION_NUMBER",	hw_rev_read_proc},
    {"amssversion",	amss_version_read_proc},    //MTD-BSP-LC-Get_Version-00 *
    //MTD-BSP-LC-Show_Version-00 *[
    {"linuxversion",  linux_version_read_proc},
    //MTD-BSP-LC-Show_Version-00 *]
    //{"rebootinfo",	reboot_info_read_proc},  /* MTD-BSP-VT-RECOVERY-00* */
    {"siminfo", siminfo_read_proc},
    //BSP-REXER-GIT-00+[
    //{"nonHLOS_git_head", nonHLOS_git_head_read_proc},
    //{"HLOS_git_head", HLOS_git_head_read_proc},
    //BSP-REXER-GIT-00+]
    {NULL,},
};

void fih_info_init(void)
{
    for (p = fih_info; p->name; p++)
        create_proc_read_entry(p->name, 0, NULL, p->read_proc, NULL);
}
EXPORT_SYMBOL(fih_info_init);

void fih_info_remove(void)
{
    for (p = fih_info; p->name; p++)
        remove_proc_entry(p->name, NULL);
}
EXPORT_SYMBOL(fih_info_remove);
