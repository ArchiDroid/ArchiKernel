#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>

/* LGE_CHANGE_S */
#include <mach/board_lge.h> /* platform data */
static struct ecom_platform_data *ecom_pdata;
static atomic_t hscd_report_enabled = ATOMIC_INIT(0);
static void hscd_suspend_resume(int mode);
/* LGE_CHANGE_E */

#define I2C_RETRY_DELAY		5
#define I2C_RETRIES		5

/* LGE_CHANGE_S, dont need to use */
#if 0
#define I2C_HSCD_ADDR (0x0c)	/* 000 1100	*/
#define I2C_BUS_NUMBER	2
#endif
/* LGE_CHANGE_E */

#define HSCD_DRIVER_NAME "hscd_i2c"

#define HSCD_STBA		0x0B
#define HSCD_STBB		0x0C
#define HSCD_XOUT		0x10
#define HSCD_YOUT		0x12
#define HSCD_ZOUT		0x14
#define HSCD_XOUT_H		0x11
#define HSCD_XOUT_L		0x10
#define HSCD_YOUT_H		0x13
#define HSCD_YOUT_L		0x12
#define HSCD_ZOUT_H		0x15
#define HSCD_ZOUT_L		0x14

#define HSCD_STATUS		0x18
#define HSCD_CTRL1		0x1b
#define HSCD_CTRL2		0x1c
#define HSCD_CTRL3		0x1d
#ifdef HSCDTD002A
	#define HSCD_CTRL4	0x28
#endif

/* #define ALPS_DEBUG 1 */

static struct i2c_driver hscd_driver;
static struct i2c_client *client_hscd = NULL;

static atomic_t flgEna;
static atomic_t delay;

static int hscd_i2c_readm(char *rxData, int length)
{
	int err;
	int tries = 0;

	struct i2c_msg msgs[] = {
		{
		 .addr = client_hscd->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = rxData,
		 },
		{
		 .addr = client_hscd->addr,
		 .flags = I2C_M_RD,
		 .len = length,
		 .buf = rxData,
		 },
	};

	do {
		err = i2c_transfer(client_hscd->adapter, msgs, 2);
	} while ((err != 2) && (++tries < I2C_RETRIES));

	if (err != 2) {
		dev_err(&client_hscd->adapter->dev, "read transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

static int hscd_i2c_writem(char *txData, int length)
{
	int err;
	int tries = 0;
#ifdef ALPS_DEBUG
	int i;
#endif
#if 0
	struct i2c_msg msg[] = {
		{
		 .addr = client_hscd->addr,
		 .flags = 0,
		 .len = length,
		 .buf = txData,
		 },
	};
#else
/* for debugging */
	struct i2c_msg msg[1];
	if(client_hscd != NULL){
		if(txData!=NULL){
			printk("[HSCD] i2c_writem param check(addr %x, length %d, txData %x %x ", client_hscd->addr,length,txData[0],txData[1]);
		}else{
			printk("[HSCD] i2c_writem txData is NULL");
			return -EIO;
		}
	}else{
		printk("[HSCD] i2c_writem client_hscd is NULL");
		return -EIO;
	}
	msg[0].addr = client_hscd->addr;
	msg[0].flags = 0;
	msg[0].len = length;
	msg[0].buf = txData;
#endif
#ifdef ALPS_DEBUG
	printk("[HSCD] i2c_writem(0x%x): ",client_hscd->addr);

	for (i = 0; i < length; i++)
		printk("0X%02X, ", txData[i]);

	printk("\n");
#endif

	do {
		err = i2c_transfer(client_hscd->adapter, msg, 1);
	} while ((err != 1) && (++tries < I2C_RETRIES));

	if (err != 1) {
		dev_err(&client_hscd->adapter->dev, "write transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

int hscd_self_test_A(void)
{
	u8 sx[2], cr1[1];

    /* Control resister1 backup  */
    cr1[0] = HSCD_CTRL1;
    if (hscd_i2c_readm(cr1, 1)) return 1;
#ifdef ALPS_DEBUG
    else printk("[HSCD] Control resister1 value, %02X\n", cr1[0]);
#endif
    mdelay(1);

    /* Stndby Mode  */
    if (cr1[0] & 0x80) {
        sx[0] = HSCD_CTRL1;
        sx[1] = 0x60;
        if (hscd_i2c_writem(sx, 2)) return 1;
    }

    /* Get inital value of self-test-A register  */
	sx[0] = HSCD_STBA;
    if (hscd_i2c_readm(sx, 1)) return 1;
#ifdef ALPS_DEBUG
    else printk("[HSCD] self test A register value, %02X\n", sx[0]);
#endif
    if (sx[0] != 0x55) {
        printk("error: self-test-A, initial value is %02X\n", sx[0]);
        return 2;
    }

    /* do self-test-A  */
	sx[0] = HSCD_CTRL3;
	sx[1] = 0x20;
	if (hscd_i2c_writem(sx, 2)) return 1;
    mdelay(1);

    /* Get 1st value of self-test-A register  */
	sx[0] = HSCD_STBA;
    if (hscd_i2c_readm(sx, 1)) return 1;
#ifdef ALPS_DEBUG
    else printk("[HSCD] self test A register value, %02X\n", sx[0]);
#endif
    if (sx[0] != 0xAA) {
        printk("error: self-test-A, 1st value is %02X\n", sx[0]);
        return 3;
    }
    mdelay(1);

    /* Get 2nd value of self-test-A register  */
	sx[0] = HSCD_STBA;
    if (hscd_i2c_readm(sx, 1)) return 1;
#ifdef ALPS_DEBUG
    else printk("[HSCD] self test A register value, %02X\n", sx[0]);
#endif
    if (sx[0] != 0x55) {
        printk("error: self-test-A, 2nd value is %02X\n", sx[0]);
        return 4;
    }

    /* Active Mode  */
    if (cr1[0] & 0x80) {
        sx[0] = HSCD_CTRL1;
        sx[1] = cr1[0];
        if (hscd_i2c_writem(sx, 2)) return 1;
    }

	return 0;
}

int hscd_self_test_B(void)
{
    int rc = 0;
	u8 sx[2], cr1[1];


    /* Control resister1 backup  */
    cr1[0] = HSCD_CTRL1;
    if (hscd_i2c_readm(cr1, 1)) return 1;
#ifdef ALPS_DEBUG
    else printk("[HSCD] Control resister1 value, %02X\n", cr1[0]);
#endif
    mdelay(1);

    /* Get inital value of self-test-B register  */
    sx[0] = HSCD_STBB;
    if (hscd_i2c_readm(sx, 1)) return 1;
#ifdef ALPS_DEBUG
    else printk("[HSCD] self test B register value, %02X\n", sx[0]);
#endif
    if (sx[0] != 0x55) {
        printk("error: self-test-B, initial value is %02X\n", sx[0]);
        return 2;
    }

    /* Active mode (Force state)  */
    sx[0] = HSCD_CTRL1;
    sx[1] = 0xC2;
    if (hscd_i2c_writem(sx, 2)) return 1;
    mdelay(1);

    do {
        /* do self-test-B  */
	    sx[0] = HSCD_CTRL3;
	    sx[1] = 0x10;
	    if (hscd_i2c_writem(sx, 2)) {
            rc = 1;
            break;
        }
        mdelay(4);

        /* Get 1st value of self-test-A register  */
	    sx[0] = HSCD_STBB;
        if (hscd_i2c_readm(sx, 1)) {
            rc = 1;
            break;
        }
#ifdef ALPS_DEBUG
        else printk("[HSCD] self test B register value, %02X\n", sx[0]);
#endif
        if (sx[0] != 0xAA) {
            if ((sx[0] < 0x01) || (sx[0] > 0x07)) {
                printk("error: self-test-B, 1st value is %02X\n", sx[0]);
                rc = 3;
                break;
            }
            else {
                printk("error: self-test-B, 1st value is %02X\n", sx[0]);
                rc = (int)(sx[0] | 0x10);
                break;
            }
        }
        mdelay(1);

        /* Get 2nd value of self-test-B register  */
	    sx[0] = HSCD_STBB;
        if (hscd_i2c_readm(sx, 1)) {
            rc = 1;
            break;
        }
#ifdef ALPS_DEBUG
        else printk("[HSCD] self test B register value, %02X\n", sx[0]);
#endif
        if (sx[0] != 0x55) {
            printk("error: self-test-B, 2nd value is %02X\n", sx[0]);
            rc = 4;
            break;
        }
    } while(0);

    /* Active Mode  */
    if (cr1[0] & 0x80) {
        sx[0] = HSCD_CTRL1;
        sx[1] = cr1[0];
        if (hscd_i2c_writem(sx, 2)) return 1;
    }

	return rc;
}

int hscd_get_magnetic_field_data(int *xyz)
{
	int err = -1;
	int i;
	u8 sx[6];

	sx[0] = HSCD_XOUT;
	err = hscd_i2c_readm(sx, 6);
	if (err < 0)
		return err;

	for (i = 0; i < 3; i++) {
		xyz[i] = (int) ((short)((sx[2*i+1] << 8) | (sx[2*i])));
	}

#ifdef ALPS_DEBUG
	/*** DEBUG OUTPUT - REMOVE ***/
	printk("Mag_I2C, x:%d, y:%d, z:%d\n",xyz[0], xyz[1], xyz[2]);
	/*** <end> DEBUG OUTPUT - REMOVE ***/
#endif

	return err;
}

void hscd_activate(int flgatm, int flg, int dtime)
{
	static u8 buf[2];
	int ret ;
	if (flg != 0)
		flg = 1;

	memset(buf,0,sizeof(buf));
	
	if (dtime <= 10)
		buf[1] = (0x60 | 3<<2);		/* 100Hz- 10msec */

	else if (dtime <= 20)
		buf[1] = (0x60 | 2<<2);		/*  50Hz- 20msec */

	else if (dtime <= 60)
		buf[1] = (0x60 | 1<<2);		/*  20Hz- 50msec */

	else
		buf[1] = (0x60 | 0<<2);		/*  10Hz-100msec */

	buf[0]  = HSCD_CTRL1;
	buf[1] |= (flg<<7);
	buf[1] |= 0x60;	/* RS1:1(Reverse Input Substraction drive), RS2:1(13bit) */
	printk("hscd_activate (flg : %d, flgatm : %d)\n",flg,flgatm);
	ret = hscd_i2c_writem(buf, 2);
	if(ret != 0)
	{
		printk("hscd_activate  fail 1(%d, %d   / %d)\n",flg,dtime,flgatm);
	}
	mdelay(1);

	if (flg) {
		buf[0] = HSCD_CTRL3;
		buf[1] = 0x02;
		ret = hscd_i2c_writem(buf, 2);
		if(ret != 0)
		{
			printk("hscd_activate  fail 2  (%d, %d   / %d)\n",flg,dtime,flgatm);

		}
	}

	if (flgatm) {
		atomic_set(&flgEna, flg);
		atomic_set(&delay, dtime);
	}
}

static void hscd_register_init(void)
{
#ifdef ALPS_DEBUG
	printk("[HSCD] register_init\n");
#endif
}
/* LGE_CHANGE_S, compass power on/off add for diag test mode 8.7 and debug option add. */
static void hscd_suspend_resume(int mode)
{
	printk(KERN_INFO "[HSCD] hscd_suspend_resume(%s)\n",mode==1?"resume":"suspend");
	if (mode) {
		 /* if already mode normal, pass this routine.*/
		if (atomic_read(&hscd_report_enabled) == 0) {
			/* turn on vreg power */
			ecom_pdata->power(1);
			mdelay(5);
			hscd_activate(0, 1, atomic_read(&delay));
			atomic_set(&hscd_report_enabled, 1);
#ifdef LGE_DEBUG
			printk(KERN_INFO "HSCD ECOM_Power On\n");
#endif
		} else { /* already power on state */
			hscd_activate(0, 1, atomic_read(&delay));
		}
	} else {
		atomic_set(&hscd_report_enabled, 0);
#ifdef LGE_DEBUG
		printk(KERN_INFO "HSCD ECOM_Power Off\n");
#endif
		/* turn off vreg power */
		ecom_pdata->power(0);
    }
	return;
}
static ssize_t show_hscd_enable(struct device *dev, \
struct device_attribute *attr, char *buf)
{
    char strbuf[256];
    snprintf(strbuf, PAGE_SIZE, "%d", atomic_read(&hscd_report_enabled));
    return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);
}

static ssize_t store_hscd_enable(struct device *dev,\
struct device_attribute *attr, const char *buf, size_t count)
{
    int mode = 0;

    sscanf(buf, "%d", &mode);
	/* actual routine */
	hscd_suspend_resume(mode);

    return 0;
}
static DEVICE_ATTR(enable,  S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP, show_hscd_enable, store_hscd_enable);
static struct attribute *hscd_attributes[] = {
    &dev_attr_enable.attr,
    NULL,
};

static struct attribute_group hscd_attribute_group = {
    .attrs = hscd_attributes
};
static int hscd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int d[3];
	int err = 0;
	printk("[HSCD] probe\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->adapter->dev, "client not i2c capable\n");
		return -ENOMEM;
	}

	client_hscd = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (!client_hscd) {
		dev_err(&client->adapter->dev, "failed to allocate memory for module data\n");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, client_hscd);

	client_hscd = client;
/* LGE_CHANGE_S, for platform data */
	ecom_pdata = client->dev.platform_data;

	ecom_pdata->power(1);

	mdelay(5);
	atomic_set(&hscd_report_enabled, 1);
	
	err = sysfs_create_group(&client->dev.kobj, &hscd_attribute_group);
	if (err) {
		printk(KERN_ERR "hscd sysfs register failed\n");
		goto exit_sysfs_create_group_failed;
	}
/* LGE_CHANGE_E */

	hscd_register_init();

	dev_info(&client->adapter->dev, "detected HSCD magnetic field sensor\n");

	hscd_activate(0, 1, atomic_read(&delay));
	mdelay(5);
	hscd_get_magnetic_field_data(d);
	printk("[HSCD] x:%d y:%d z:%d\n",d[0],d[1],d[2]);
	hscd_activate(0, 0, atomic_read(&delay));
	return 0;
/* LGE_CHANGE_S*/
exit_sysfs_create_group_failed:
    sysfs_remove_group(&client->dev.kobj, &hscd_attribute_group);
/* LGE_CHANGE_E*/
	return 0;
}

static void hscd_shutdown(struct i2c_client *client)
{
#ifdef ALPS_DEBUG
	printk("[HSCD] shutdown\n");
#endif
	hscd_activate(0, 0, atomic_read(&delay));
}

static int hscd_suspend(struct i2c_client *client, pm_message_t mesg)
{
#ifdef ALPS_DEBUG
	printk("[HSCD] suspend\n");
#endif
	hscd_activate(0, 0, atomic_read(&delay));
	return 0;
}

static int hscd_resume(struct i2c_client *client)
{
#ifdef ALPS_DEBUG
	printk("[HSCD] resume\n");
#endif
	hscd_activate(0, atomic_read(&flgEna), atomic_read(&delay));
	return 0;
}

static const struct i2c_device_id ALPS_id[] = {
	{ HSCD_DRIVER_NAME, 0 },
	{ }
};

static struct i2c_driver hscd_driver = {
	.probe    = hscd_probe,
	.id_table = ALPS_id,
	.driver   = {
  		.name	= HSCD_DRIVER_NAME,
	},
	.shutdown		= hscd_shutdown,
	.suspend		= hscd_suspend,
	.resume		= hscd_resume,
};

static int __init hscd_init(void)
{
/* LGE_CHANGE_S, we already use board info in platform data */
#if 0
	struct i2c_board_info i2c_info;
	struct i2c_adapter *adapter;
#endif
/* LGE_CHANGE_E */
	int rc;

#ifdef ALPS_DEBUG
	printk("[HSCD] init\n");
#endif
	atomic_set(&flgEna, 0);
	atomic_set(&delay, 200);

	rc = i2c_add_driver(&hscd_driver);
	if (rc != 0) {
		printk("can't add i2c driver\n");
		rc = -ENOTSUPP;
		return rc;
	}

/* LGE_CHANGE_S, we already use board info in platform data */
#if 0
	memset(&i2c_info, 0, sizeof(struct i2c_board_info));
	i2c_info.addr = I2C_HSCD_ADDR;
	strlcpy(i2c_info.type, HSCD_DRIVER_NAME , I2C_NAME_SIZE);

	adapter = i2c_get_adapter(I2C_BUS_NUMBER);
	if (!adapter) {
		printk("can't get i2c adapter %d\n", I2C_BUS_NUMBER);
		rc = -ENOTSUPP;
		goto probe_done;
	}
	client_hscd = i2c_new_device(adapter, &i2c_info);
	client_hscd->adapter->timeout = 0;
	client_hscd->adapter->retries = 0;

	i2c_put_adapter(adapter);
	if (!client_hscd) {
		printk("can't add i2c device at 0x%x\n", (unsigned int)i2c_info.addr);
		rc = -ENOTSUPP;
	}
#endif
/* LGE_CHANGE_E */

#ifdef ALPS_DEBUG
	printk("hscd_open Init end!!!!\n");
#endif

/* probe_done: */

	return rc;
}

static void __exit hscd_exit(void)
{
#ifdef ALPS_DEBUG
	printk("[HSCD] exit\n");
#endif
	i2c_del_driver(&hscd_driver);
}

module_init(hscd_init);
module_exit(hscd_exit);

EXPORT_SYMBOL(hscd_self_test_A);
EXPORT_SYMBOL(hscd_self_test_B);
EXPORT_SYMBOL(hscd_get_magnetic_field_data);
EXPORT_SYMBOL(hscd_activate);

MODULE_DESCRIPTION("Alps hscd Device");
MODULE_AUTHOR("ALPS");
MODULE_LICENSE("GPL v2");
