#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>

/* LGE_CHANGE_S */
#include <mach/board_lge.h> /* platform data */

struct acceleration_platform_data *accel_pdata;
/* LGE_CHANGE_E */

#define I2C_RETRY_DELAY	5
#define I2C_RETRIES		5

/* Register Name for accsns */
#define ACC_XOUT		0x02
#define ACC_YOUT		0x04
#define ACC_ZOUT		0x06
#define ACC_TEMP		0x08
#define ACC_REG0B		0x0B
#define ACC_REG0A		0x0A
#define ACC_REG0F		0x0F
#define ACC_REG10		0x10
#define ACC_REG11		0x11
//#define ACC_REG14		0x14

#define ACC_DRIVER_NAME "accsns_i2c"

/* LGE_CHANGE_S, dont need to use */
#if 0
#define I2C_ACC_ADDR (0x08)		/* 000 1000	*/
#define I2C_BUS_NUMBER	2
#endif
/* LGE_CHANGE_E */

#define ALPS_DEBUG 1
static struct i2c_driver accsns_driver;
static struct i2c_client *client_accsns = NULL;

static atomic_t flgEna;

static int accsns_i2c_readm(u8 *rxData, int length)
{
	int err;
	int tries = 0;

	struct i2c_msg msgs[] = {
		{
		 .addr = client_accsns->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = rxData,
		 },
		{
		 .addr = client_accsns->addr,
		 .flags = I2C_M_RD,
		 .len = length,
		 .buf = rxData,
		 },
	};

	do {
		err = i2c_transfer(client_accsns->adapter, msgs, 2);
	} while ((err != 2) && (++tries < I2C_RETRIES));

	if (err != 2) {
		dev_err(&client_accsns->adapter->dev, "read transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

static int accsns_i2c_writem(u8 *txData, int length)
{
	int err;
	int tries = 0;
#ifdef ALPS_DEBUG
	int i;
#endif

	struct i2c_msg msg[] = {
		{
		 .addr = client_accsns->addr,
		 .flags = 0,
		 .len = length,
		 .buf = txData,
		 },
	};

#ifdef ALPS_DEBUG
	printk("[ACC] i2c_writem : ");
	for (i=0; i<length;i++) printk("0X%02X, ", txData[i]);
	printk("\n");
#endif

	do {
		err = i2c_transfer(client_accsns->adapter, msg, 1);
	} while ((err != 1) && (++tries < I2C_RETRIES));

	if (err != 1) {
		dev_err(&client_accsns->adapter->dev, "write transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

int accsns_get_acceleration_data(int *xyz)
{
	int err = -1;
	int i;
	u8 sx[6];

	sx[0] = ACC_XOUT;
	err = accsns_i2c_readm(sx, 6);
	if (err < 0) return err;
	for (i=0; i<3; i++) {
		xyz[i] = (int)((s8)sx[2 * i + 1]) * 4;                        // raw * 4
	}

#ifdef ALPS_DEBUG
	/*** DEBUG OUTPUT - REMOVE ***/
	printk("Acc_I2C, x:%d, y:%d, z:%d\n", xyz[0], xyz[1], xyz[2]);
	/*** <end> DEBUG OUTPUT - REMOVE ***/
#endif

	return err;
}

void accsns_activate(int flgatm, int flg)
{
	u8 buf[2];

	if (flg != 0) flg = 1;

	buf[0] = ACC_REG0F    ; buf[1] = 0x03;		//g-range +/-2g
	accsns_i2c_writem(buf, 2);
	buf[0] = ACC_REG10    ; buf[1] = /*0x0D*/0x0A;		//Bandwidth 250Hz -> 31.25Hz
	accsns_i2c_writem(buf, 2);
	buf[0] = ACC_REG11;							//Power modes
	if (flg == 0) buf[1] = 0x80;					//sleep
	else {
		buf[1] = 0x12;								//6ms
    }
	accsns_i2c_writem(buf, 2);
	mdelay(2);
	if (flgatm) atomic_set(&flgEna, flg);
}

static void accsns_register_init(void)
{
#ifdef ALPS_DEBUG
	printk("[ACC] register_init\n");
#endif
}

static int accsns_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int d[3];

	printk("[ACC] probe\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->adapter->dev, "client not i2c capable\n");
		return -ENOMEM;
	}

	client_accsns = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (!client_accsns) {
		dev_err(&client->adapter->dev, "failed to allocate memory for module data\n");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, client_accsns);

	client_accsns = client;
/* LGE_CHANGE_S, for platform data */
	accel_pdata = client->dev.platform_data;

	accel_pdata->power(1);
/* LGE_CHANGE_E */
	
	accsns_register_init();

	dev_info(&client->adapter->dev, "detected accelerometer\n");

	accsns_activate(0, 1);
	accsns_get_acceleration_data(d);
	printk("[ACC] x:%d y:%d z:%d\n",d[0],d[1],d[2]);
	accsns_activate(0, 0);

	return 0;
}

static void accsns_shutdown(struct i2c_client *client)
{
#ifdef ALPS_DEBUG
	printk("[ACC] shutdown\n");
#endif
	accsns_activate(0, 0);
}

static int accsns_suspend(struct i2c_client *client,pm_message_t mesg)
{
#ifdef ALPS_DEBUG
	printk("[ACC] suspend\n");
#endif
	accsns_activate(0, 0);

	return 0;
}

static int accsns_resume(struct i2c_client *client)
{
#ifdef ALPS_DEBUG
	printk("[ACC] resume\n");
#endif
	accsns_activate(0, atomic_read(&flgEna));

	return 0;
}

static const struct i2c_device_id accsns_id[] = {
	{ ACC_DRIVER_NAME, 0 },
	{ }
};

static struct i2c_driver accsns_driver = {
	.probe     = accsns_probe,
	.id_table  = accsns_id,
	.driver    = {
  		.name	= ACC_DRIVER_NAME,
	},
	.shutdown	   	= accsns_shutdown,
	.suspend		= accsns_suspend,
	.resume		= accsns_resume,
};

static int __init accsns_init(void)
{
/* LGE_CHANGE_S, we already use board info in platform data */
#if 0
	struct i2c_board_info i2c_info;
	struct i2c_adapter *adapter;
#endif
/* LGE_CHANGE_E */
	int rc;


#ifdef ALPS_DEBUG
	printk("[ACC] init\n");
#endif
	atomic_set(&flgEna, 0);
	rc = i2c_add_driver(&accsns_driver);
	if (rc != 0) {
		printk("can't add i2c driver\n");
		rc = -ENOTSUPP;
		return rc;
	}

/* LGE_CHANGE_S, we already use board info in platform data */
#if 0
	memset(&i2c_info, 0, sizeof(struct i2c_board_info));
	i2c_info.addr = I2C_ACC_ADDR;
	strlcpy(i2c_info.type, ACC_DRIVER_NAME , I2C_NAME_SIZE);
	
	adapter = i2c_get_adapter(I2C_BUS_NUMBER);
	if (!adapter) {
		printk("can't get i2c adapter %d\n", I2C_BUS_NUMBER);
		rc = -ENOTSUPP;
		goto probe_done;
	}

	client_accsns = i2c_new_device(adapter, &i2c_info);
	client_accsns->adapter->timeout = 0;
	client_accsns->adapter->retries = 0;
	  
	i2c_put_adapter(adapter);
	if (!client_accsns) {
		printk("can't add i2c device at 0x%x\n",(unsigned int)i2c_info.addr);
		rc = -ENOTSUPP;
	}
#endif
/* LGE_CHANGE_E */

#ifdef ALPS_DEBUG
	printk("accsns_open end !!!!\n");
#endif
	
//	probe_done: 

	return rc;
}

static void __exit accsns_exit(void)
{
#ifdef ALPS_DEBUG
	printk("[ACC] exit\n");
#endif
	i2c_del_driver(&accsns_driver);
}

module_init(accsns_init);
module_exit(accsns_exit);

EXPORT_SYMBOL(accsns_get_acceleration_data);
EXPORT_SYMBOL(accsns_activate);

MODULE_DESCRIPTION("Alps acc Device");
MODULE_AUTHOR("ALPS");
MODULE_LICENSE("GPL v2");
