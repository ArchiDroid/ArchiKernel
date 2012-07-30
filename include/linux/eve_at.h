// BEGIN: Mantis번호 eternalblue@lge.com.2009-09-15
// MOD  Mantis의 Activity 번호 및 제목  
#ifndef	EVE_AT_H
#define	EVE_AT_H

/* LGE_CHANGE_S [kimeh@lge.com] 2009-04-15, for debugging */
struct atcmd_dev {
//	const char	*name;
	char	*name;
	struct device	*dev;
	int		index;
	int		state;
};

struct atcmd_platform_data {
	const char *name;
};

extern int atcmd_dev_register(struct atcmd_dev *sdev);
extern void atcmd_dev_unregister(struct atcmd_dev *sdev);

static inline int atcmd_get_state(struct atcmd_dev *sdev)
{
	return sdev->state;
}

extern void update_atcmd_state(struct atcmd_dev *sdev, char *cmd, int state);
extern struct atcmd_dev *atcmd_get_dev(void);
/* LGE_CHANGE_E [kimeh@lge.com] 2009-04-15, for debugging */

#endif	// EVE_AT_H
// END: Mantis번호 eternalblue@lge.com.2009-09-15