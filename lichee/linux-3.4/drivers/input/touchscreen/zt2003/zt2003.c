/* capacivite multi-touch device driver.*/
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/miscdevice.h>

#include <linux/cdev.h>
#include <linux/uaccess.h>

#include <linux/kthread.h>
#include <mach/irqs.h>
#include <mach/hardware.h>
#include <mach/sys_config.h>
#include <linux/init-input.h>
#include <linux/gpio.h>

#if defined(CONFIG_IIO) || defined(CONFIG_IIO_MODULE)
#define HAVE_IIO	1
#else
#undef HAVE_IIO
#endif

#ifdef HAVE_IIO
#include "../../../staging/iio/iio.h"
#endif

#include "../../../misc/hyperion/ht_input.h"


//#define REG_DATA_DEBUG
#ifdef CONFIG_HAS_EARLYSUSPEND
    #include <linux/pm.h>
    #include <linux/earlysuspend.h>
#endif

#define BUF_SIZE 		REPORT_BUF_SIZE
#define COORD_INTERPRET(MSB_BYTE, LSB_BYTE) \
		(MSB_BYTE << 8 | LSB_BYTE)

#ifdef CONFIG_HAS_EARLYSUSPEND
static void tu_early_suspend(struct early_suspend *h);
static void tu_late_resume(struct early_suspend *h);
#endif

static u32 debug_mask = 0;
enum{
	DEBUG_INIT = 1U << 0,
	DEBUG_SUSPEND = 1U << 1,
	DEBUG_INT_INFO = 1U << 2,
	DEBUG_X_Y_INFO = 1U << 3,
	DEBUG_KEY_INFO = 1U << 4,
	DEBUG_WAKEUP_INFO = 1U << 5,
	DEBUG_OTHERS_INFO = 1U << 6,
};

#define dprintk(level_mask,fmt,arg...)    if(unlikely(debug_mask & level_mask)) \
        printk("[CTP]: "fmt, ## arg)

module_param_named(debug_mask,debug_mask,int,S_IRUGO | S_IWUSR | S_IWGRP);

#define USE_THREADED_IRQ	1
#define USE_WQ	(!USE_THREADED_IRQ)

#define MAGICKEY	0x19790429

// --------------------------------------------------------------
#ifdef HAVE_IIO
struct zt2003_state {
	struct i2c_client *client;
};
#endif

struct zt2003_data {
	u32 magickey;
	struct ctp_config_info	config_info;
	u16 x, y, z1,z2,w, p, id;
	struct i2c_client *client;
	/* capacivite device*/
	struct input_dev *dev;
	/* digitizer */
	struct timer_list timer;
	struct input_dev *dig_dev;
#ifdef HAVE_IIO
	struct iio_dev *indio_dev;
#endif
	struct mutex lock;

#if USE_WQ
	struct workqueue_struct	*wq;
	struct work_struct	work;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
struct early_suspend early_suspend;
#endif

	bool			stopped;
};

///////////////////////////////////////////////
//specific tp related macro: need be configured for specific tp
//#define CTP_IRQ_NO			(gpio_int_info[0].port_num)
//#define CTP_IRQ_MODE			(NEGATIVE_EDGE)
#define CTP_IRQ_NUMBER          (config_info.int_number)
#define CTP_IRQ_MODE			(IRQF_TRIGGER_FALLING)
#define CTP_NAME			"zt2003"
#define TS_RESET_LOW_PERIOD		(15)
#define TS_INITIAL_HIGH_PERIOD		(15)
#define TS_WAKEUP_LOW_PERIOD	(100)
#define TS_WAKEUP_HIGH_PERIOD	(100)
#define TS_POLL_DELAY				(100)	/* ms delay between samples */
#define TS_POLL_PERIOD			(10)	/* ms delay between samples */
#define SCREEN_MAX_X			(screen_max_x)
#define SCREEN_MAX_Y  		(screen_max_y)
#define PRESS_MAX				(255)

#define TSC2007_MEASURE_TEMP0		(0x0 << 4)
#define TSC2007_MEASURE_VBAT1		(0x1 << 4)
#define TSC2007_MEASURE_AUX1		(0x2 << 4)
#define TSC2007_MEASURE_TEMP1		(0x4 << 4)
#define TSC2007_MEASURE_VBAT2		(0x5 << 4)
#define TSC2007_MEASURE_AUX2		(0x6 << 4)
#define TSC2007_ACTIVATE_XN		(0x8 << 4)
#define TSC2007_ACTIVATE_YN		(0x9 << 4)
#define TSC2007_ACTIVATE_YP_XN		(0xa << 4)
#define TSC2007_SETUP			(0xb << 4)
#define TSC2007_MEASURE_X		(0xc << 4)
#define TSC2007_MEASURE_Y		(0xd << 4)
#define TSC2007_MEASURE_Z1		(0xe << 4)
#define TSC2007_MEASURE_Z2		(0xf << 4)

#define TSC2007_POWER_OFF_IRQ_EN	(0x0 << 2)
#define TSC2007_ADC_ON_IRQ_DIS0		(0x1 << 2)
#define TSC2007_ADC_OFF_IRQ_EN		(0x2 << 2)
#define TSC2007_ADC_ON_IRQ_DIS1		(0x3 << 2)

#define TSC2007_12BIT			(0x0 << 1)
#define TSC2007_8BIT			(0x1 << 1)

#define	MAX_12BIT			((1 << 12) - 1)

#define ADC_ON_12BIT	(TSC2007_12BIT | TSC2007_ADC_ON_IRQ_DIS0)

#define READ_TEMP0		(TSC2007_12BIT | TSC2007_ADC_ON_IRQ_DIS1 | TSC2007_MEASURE_TEMP0)
#define READ_TEMP1		(TSC2007_12BIT | TSC2007_ADC_ON_IRQ_DIS1 | TSC2007_MEASURE_TEMP1)
#define READ_Y		(ADC_ON_12BIT | TSC2007_MEASURE_Y)
#define READ_Z1		(ADC_ON_12BIT | TSC2007_MEASURE_Z1)
#define READ_Z2		(ADC_ON_12BIT | TSC2007_MEASURE_Z2)
#define READ_X		(ADC_ON_12BIT | TSC2007_MEASURE_X)
#define PWRDOWN		(TSC2007_12BIT | TSC2007_POWER_OFF_IRQ_EN)

static int tsc2007_i2c_read(struct i2c_client *client, u8 *buf, s32 len)
{
    struct i2c_msg msgs[2];
	int retries = 0;
	int ret = -1;

    msgs[0].flags = !I2C_M_RD;
    msgs[0].addr  = client->addr;
    msgs[0].len   = 1;
    msgs[0].buf   = &buf[0];

    msgs[1].flags = I2C_M_RD;
    msgs[1].addr  = client->addr;
    msgs[1].len   = len;
    msgs[1].buf   = &buf[1];

	while (retries < 5)
    {
        ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret == 2)
			break;
        retries++;
    }
	if (retries >= 5)
    {
		dev_err(&client->dev, "I2C retry timeout");
    }

	return ret;
}

static int tsc2007_xfer(struct zt2003_data *tu, u8 cmd)
{
	u8 data[3];
	u16 retdata;
	int val;
	int ret;

	data[0] = cmd;

	ret = tsc2007_i2c_read(tu->client, data, 2);
	if (ret < 0) {
		return ret;
	}

	retdata = ((u16)data[1] << 8) | data[2];

	/* The protocol and raw data format from i2c interface:
	 * S Addr Wr [A] Comm [A] S Addr Rd [A] [DataLow] A [DataHigh] NA P
	 * Where DataLow has [D11-D4], DataHigh has [D3-D0 << 4 | Dummy 4bit].
	 */

	val = retdata >> 4;

	dev_dbg(&tu->client->dev, "data: 0x%x, val: 0x%x\n", retdata, val);

	return val;
}

static u32 tsc2007_calculate_pressure(struct zt2003_data *tc)
{
	u32 rt = 0;

	/* range filtering */
	if (tc->x == MAX_12BIT)
		tc->x = 0;

	if (likely(tc->x && tc->z1)) {
		/* compute touch pressure resistance using equation #1 */
		rt = tc->z2 - tc->z1;
		rt *= tc->x;
		rt *= 700;
		rt /= tc->z1;
		rt = (rt + 2047) >> 12;
	}

	return rt;
}

static void tsc2007_read_values(struct zt2003_data *tc)
{
	/* y- still on; turn on only y+ (and ADC) */
	tc->y = tsc2007_xfer(tc, READ_Y);

	/* turn y- off, x+ on, then leave in lowpower */
	tc->x = tsc2007_xfer(tc, READ_X);

	/* turn y+ off, x- on; we'll use formula #1 */
	tc->z1 = tsc2007_xfer(tc, READ_Z1);
	tc->z2 = tsc2007_xfer(tc, READ_Z2);

	tc->w=tsc2007_calculate_pressure(tc);

	/* Prepare for next touch reading - power down ADC, enable PENIRQ */
	tsc2007_xfer(tc, PWRDOWN);
}


/**
 * ctp_wakeup - function
 *
 */
static int ctp_wakeup(const struct ctp_config_info *config_info, int status, int ms)
{
	dprintk(DEBUG_INIT,"***CTP*** %s:status:%d,ms = %d\n",__func__,status,ms);

	if (status == 0) {

		if(ms == 0) {
			__gpio_set_value(config_info->wakeup_gpio.gpio, 0);
		}else {
			__gpio_set_value(config_info->wakeup_gpio.gpio, 0);
			msleep(ms);
			__gpio_set_value(config_info->wakeup_gpio.gpio, 1);
		}
	}
	if (status == 1) {
		if(ms == 0) {
			__gpio_set_value(config_info->wakeup_gpio.gpio, 1);
		}else {
			__gpio_set_value(config_info->wakeup_gpio.gpio, 1);
			msleep(ms);
			__gpio_set_value(config_info->wakeup_gpio.gpio, 0);
		}
	}
	msleep(5);

	return 0;
}


/**
 * ctp_print_info - sysconfig print function
 * return value:
 *
 */
static void ctp_print_info(const struct ctp_config_info *info, int debug_level)
{
	if (debug_level == DEBUG_INIT)
	{
		printk("info.ctp_used:%d\n", info->ctp_used);
		printk("info.twi_id:%d\n", info->twi_id);
		printk("info.screen_max_x:%d\n", info->screen_max_x);
		printk("info.screen_max_y:%d\n", info->screen_max_y);
		printk("info.revert_x_flag:%d\n", info->revert_x_flag);
		printk("info.revert_y_flag:%d\n", info->revert_y_flag);
		printk("info.exchange_x_y_flag:%d\n", info->exchange_x_y_flag);
		printk("info.irq_gpio_number:%d\n", info->irq_gpio.gpio);
		printk("info.wakeup_gpio_number:%d\n", info->wakeup_gpio.gpio);
	}
}

static inline void tu_report(struct zt2003_data *tu)
{
	if (1 == tu->config_info.exchange_x_y_flag) {
		swap(tu->x, tu->y);
	}
	if (1 == tu->config_info.revert_x_flag) {
		tu->x = MAX_12BIT - tu->x;
	}
	if (1 == tu->config_info.revert_y_flag) {
		tu->y = MAX_12BIT - tu->y;
	}
	// printk(" report (%d, %d, %d)\n", tu->x, tu->y, tu->w);	
	input_report_abs(tu->dev, ABS_X, tu->x);
	input_report_abs(tu->dev, ABS_Y, tu->y);
	input_report_abs(tu->dev, ABS_PRESSURE, tu->w);
	input_sync(tu->dev);
}

static inline bool zt2003_get_pendown(const struct ctp_config_info *config_info)
{
	return !__gpio_get_value(config_info->irq_gpio.gpio);
}

static bool zt2003_is_down(void *context)
{
	struct zt2003_data *tu = context;
	return zt2003_get_pendown(&tu->config_info);
}

static int zt2003_read_sample(void *context)
{
	struct zt2003_data *tu = context;

	tsc2007_read_values(tu);

	return (tu->w > 0) && (tu->w < 3000);
}

static void zt2003_report_sample(void *context)
{
	struct zt2003_data *tu = context;

	tu_report(tu);
}

static void zt2003_report_down(void *context)
{
	struct zt2003_data *tu = context;
	input_report_key(tu->dev, BTN_TOUCH, 1);
}

static void zt2003_report_up(void *context)
{
	struct zt2003_data *tu = context;
	input_report_key(tu->dev, BTN_TOUCH, 0);
	input_report_abs(tu->dev, ABS_PRESSURE, 0);
	input_sync(tu->dev);
}

static struct ht_touchpanel zt2003_touchpanel = {
	.up_threshold = 5,
	.report_period_ms = TS_POLL_PERIOD,
	.is_down = zt2003_is_down,
	.read_sample = zt2003_read_sample,
	.report_sample = zt2003_report_sample,
	.report_down = zt2003_report_down,
	.report_up = zt2003_report_up,
};

#if USE_WQ
static void tu_i2c_work(struct work_struct *work)
#else
static void tu_i2c_work(struct zt2003_data *tu)
#endif
{
#if USE_WQ 
	struct zt2003_data *tu = container_of(work, struct zt2003_data, work);
#endif
	int ret;

	ret = input_set_int_enable(&tu->config_info.input_type, 0);
	if (ret < 0) {
		dprintk(DEBUG_INT_INFO,"%s irq disable failed\n", __func__);
	}

	mutex_lock(&tu->lock);

	if (zt2003_get_pendown(&tu->config_info)) {
		ret = ht_touchpanel_loop_run(&zt2003_touchpanel, tu);
	}

	mutex_unlock(&tu->lock);

	ret = input_set_int_enable(&tu->config_info.input_type, 1);
	if (ret < 0)
		dprintk(DEBUG_SUSPEND, "%s irq enable failed\n", __func__);
}

static irqreturn_t tu_irq(int irq, void *dev_id)
{
	struct zt2003_data *tu = (struct zt2003_data *)dev_id;

#if USE_WQ
	queue_work(tu->wq, &tu->work);
#else
	tu_i2c_work(tu);
#endif

	return IRQ_HANDLED;
}

#ifdef HAVE_IIO
static int zt2003_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan,
			   int *val,
			   int *val2,
			   long m)
{
	struct zt2003_state *state = iio_priv(indio_dev);
	struct i2c_client *client = state->client;
	struct zt2003_data *tu = i2c_get_clientdata(client);
	int v0 = 0, v1 = 0, dv, c;
	unsigned long vref = 2500;
	unsigned long scale_uv;
	int ret;

	switch (chan->type) {
	case IIO_TEMP:
		switch (m) {
		case 0:
			mutex_lock(&indio_dev->mlock);
			mutex_lock(&tu->lock);

			// Temp. ADC reading doesn't work!
			ret = tsc2007_xfer(tu, READ_TEMP1);
			if (ret >= 0) {
				v1 = ret;

				ret = tsc2007_xfer(tu, READ_TEMP0);
			}
			if (ret >= 0) {
				v0 = ret;

				ret = tsc2007_xfer(tu, PWRDOWN);
			}

			mutex_unlock(&tu->lock);
			mutex_unlock(&indio_dev->mlock);

			if (ret < 0)
				return ret;

			dv = v1 - v0;
			c = 2573 * dv * vref / 4096 - 273000;

			printk("dv %d = %d - %d, c = %d\n", dv, v1, v0, c);

			*val = c;

			return IIO_VAL_INT;

		case IIO_CHAN_INFO_SCALE:
			*val = 1;
			*val2 = 0;
			return IIO_VAL_INT_PLUS_MICRO;
		}
		break;

	case IIO_VOLTAGE:
		switch (m) {
		case 0:
			mutex_lock(&indio_dev->mlock);
			mutex_lock(&tu->lock);

			ret = tsc2007_xfer(tu, chan->address | TSC2007_12BIT | TSC2007_ADC_ON_IRQ_DIS1);
			if (ret >= 0) {
				v0 = ret & ((1 << chan->scan_type.realbits) - 1);
			}

			tsc2007_xfer(tu, PWRDOWN);

			mutex_unlock(&tu->lock);
			mutex_unlock(&indio_dev->mlock);

			if (ret < 0)
				return ret;

			if (chan->address == TSC2007_MEASURE_VBAT1 || chan->address == TSC2007_MEASURE_VBAT2)
				v0 *= 4;

			// printk("v = %d\n", v0 * vfre / 4096);

			*val = v0;

			return IIO_VAL_INT;

		case IIO_CHAN_INFO_SCALE:
			scale_uv = (vref * 100000) >> chan->scan_type.realbits;
			*val = scale_uv / 100000;
			*val2 = (scale_uv % 100000) * 10;
			return IIO_VAL_INT_PLUS_MICRO;
		}
		break;

	default:
		break;
	}

	return -EINVAL;
}

static struct iio_chan_spec zt2003_channels[] = {
	{
		.type = IIO_TEMP,
		.indexed = 0,
		.channel = 0,
		.info_mask = IIO_CHAN_INFO_SCALE_SEPARATE_BIT,
		// .scan_index = 0,
		// .scan_type = IIO_ST('s', 24, 32, 0),
	},
	{
		.type = IIO_VOLTAGE,
		.indexed = 1,
		.channel = 0,
		.address = TSC2007_MEASURE_VBAT1,
		.info_mask = IIO_CHAN_INFO_SCALE_SHARED_BIT,
		// .scan_index = 1,
		.scan_type = IIO_ST('u', 12, 16, 0)
	},
	{
		.type = IIO_VOLTAGE,
		.indexed = 1,
		.channel = 1,
		.address = TSC2007_MEASURE_VBAT2,
		.info_mask = IIO_CHAN_INFO_SCALE_SHARED_BIT,
		// .scan_index = 1,
		.scan_type = IIO_ST('u', 12, 16, 0)
	},
	{
		.type = IIO_VOLTAGE,
		.indexed = 1,
		.channel = 2,
		.address = TSC2007_MEASURE_AUX1,
		.info_mask = IIO_CHAN_INFO_SCALE_SHARED_BIT,
		// .scan_index = 1,
		.scan_type = IIO_ST('u', 12, 16, 0)
	},
	{
		.type = IIO_VOLTAGE,
		.indexed = 1,
		.channel = 3,
		.address = TSC2007_MEASURE_AUX2,
		.info_mask = IIO_CHAN_INFO_SCALE_SHARED_BIT,
		// .scan_index = 1,
		.scan_type = IIO_ST('u', 12, 16, 0)
	},
};

static const struct iio_info zt2003_info = {
	.read_raw = &zt2003_read_raw,
	.driver_module = THIS_MODULE,
};
#endif

static int tu_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;
	struct zt2003_data *tu;
	struct input_dev *input_dev;
#ifdef HAVE_IIO
	struct zt2003_state *state;
	struct iio_dev *indio_dev;
#endif

	dprintk(DEBUG_INIT,"====%s begin=====.  \n", __func__);
	tu = kzalloc(sizeof(struct zt2003_data), GFP_KERNEL);
	if (!tu)
		return -ENOMEM;

	tu->magickey = MAGICKEY;
	tu->config_info = *(struct ctp_config_info *)dev_get_platdata(&client->dev);
	tu->client = client;

	dev_info(&tu->client->dev, "device probing\n");
	i2c_set_clientdata(client, tu);
	mutex_init(&tu->lock);

	err = input_init_platform_resource(&tu->config_info.input_type);
	if (0 != err) {
		printk("%s:init_platform_resource err. \n", __func__);
		goto err_platform_resource;
	}

	ctp_print_info(&tu->config_info, DEBUG_INIT);

	if (tu->config_info.wakeup_gpio.gpio)
		ctp_wakeup(&tu->config_info, 0, 50);

	/* allocate input device for capacitive */
	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&tu->client->dev, "failed to allocate input device \n");
		goto err_input_alloc;
	}

	//Initial Device Parameters
	input_dev->name = CTP_NAME;
	input_dev->phys = "input/zt2003";
	input_dev->id.bustype = BUS_I2C;

	tu->dev = input_dev;
	tu->config_info.dev = &tu->dev->dev;

	//Key bit initialize
	input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

	//Set Coordinate
	input_set_abs_params(input_dev, ABS_X, 0, MAX_12BIT, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, MAX_12BIT, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, MAX_12BIT, 0, 0);

	//Register Device Object
	err = input_register_device(input_dev);
	if (err)
		goto err_input_register;

#ifdef HAVE_IIO
	indio_dev = iio_allocate_device(sizeof(*state));
	if (indio_dev == NULL) {
		err = -ENOMEM;
		goto err_iio_alloc;
	}
	state = iio_priv(indio_dev);

	state->client = client;

	indio_dev->name = id->name;
	indio_dev->dev.parent = &client->dev;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = zt2003_channels;
	indio_dev->num_channels = ARRAY_SIZE(zt2003_channels);
	indio_dev->info = &zt2003_info;

	err = iio_device_register(indio_dev);
	if (err)
		goto err_iio_register;

	tu->indio_dev = indio_dev;
#endif

#if USE_THREADED_IRQ
	err = input_request_irq(&tu->config_info.input_type, NULL, tu_irq, CTP_IRQ_MODE, tu);
#else
	INIT_WORK(&tu->work, tu_i2c_work);
	tu->wq = create_singlethread_workqueue("zt2003_wq");
	if (!tu->wq) {
		printk(KERN_ALERT "Creat %s workqueue failed.\n", __func__);
		goto err_create_wq;
	}

	err = input_request_int(&tu->config_info.input_type, tu_irq, CTP_IRQ_MODE, tu);
#endif
	if (err) {
		pr_info( "tu_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	tu->early_suspend.level		= EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	tu->early_suspend.suspend	= tu_early_suspend;
	tu->early_suspend.resume	= tu_late_resume;
	register_early_suspend(&tu->early_suspend);
#endif

	return 0;

	input_free_int(&tu->config_info.input_type, tu);
exit_irq_request_failed:
#if USE_WQ
err_create_wq:
	flush_workqueue(tu->wq);
	destroy_workqueue(tu->wq);
#endif
#ifdef HAVE_IIO
err_iio_register:
	iio_free_device(indio_dev);
err_iio_alloc:
#endif
	input_unregister_device(tu->dev);
err_input_register:
err_input_alloc:
	input_free_platform_resource(&tu->config_info.input_type);
err_platform_resource:
	kfree(tu);
	return err;
}

static int __devexit tu_remove(struct i2c_client *client)
{
	struct zt2003_data *tu = i2c_get_clientdata(client);
#ifdef HAVE_IIO
	struct iio_dev *indio_dev = tu->indio_dev;
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND	
	unregister_early_suspend(&tu->early_suspend);
#endif
	input_free_int(&tu->config_info.input_type, tu);
#if USE_WQ 
	flush_workqueue(tu->wq);
	destroy_workqueue(tu->wq);
#endif
#ifdef HAVE_IIO
	iio_device_unregister(indio_dev);
	iio_free_device(indio_dev);
#endif
	input_unregister_device(tu->dev);
	input_free_platform_resource(&tu->config_info.input_type);
	kfree(tu);

	return 0;
}

#ifdef CONFIG_PM
static int tu_suspend(struct i2c_client *client, pm_message_t state)
{
	struct zt2003_data *tu = i2c_get_clientdata(client);
	int ret;

	printk("[CTP]: %s         suspend!\n",__func__);

	ret = input_set_int_enable(&tu->config_info.input_type, 0);
	if (ret < 0)
		dprintk(DEBUG_SUSPEND,"%s irq disable failed\n", __func__);

#if USE_WQ 
	ret = cancel_work_sync(&tu->work);
#endif

	return ret;
}

static int tu_resume(struct i2c_client *client)
{
	struct zt2003_data *tu = i2c_get_clientdata(client);
	int ret;

	printk("[CTP]: %s         resume!\n",__func__);
	ret = input_set_int_enable(&tu->config_info.input_type, 1);
	if (ret < 0)
		dprintk(DEBUG_SUSPEND,"%s irq enable failed\n", __func__);

	return 0;
}

static int zt2003_pm_suspend(struct device *dev)
{
	struct i2c_client *client = i2c_verify_client(dev);

	return tu_suspend(client, PMSG_SUSPEND);
}

static int zt2003_pm_resume(struct device *dev)
{
	struct i2c_client *client = i2c_verify_client(dev);

	return tu_resume(client);
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void tu_early_suspend(struct early_suspend *h)
{
	struct zt2003_data *tu = container_of(handler, struct zt2003_data, early_suspend);

	dprintk(DEBUG_SUSPEND, "tu_early_suspend\n"); 

	tu_suspend(tu->client, PMSG_SUSPEND);
}

static void tu_late_resume(struct early_suspend *h)
{
	struct zt2003_data *tu = container_of(handler, struct zt2003_data, early_suspend);

	dprintk(DEBUG_WAKEUP_INFO, "tu_late_resume\n");

	tu_resume(tu->client);
}
#endif

static struct i2c_device_id tu_id_table[] = {
	{CTP_NAME, 0},
	{}
};

#ifdef CONFIG_PM
static const struct dev_pm_ops zt2003_pm_ops = {
	.suspend = zt2003_pm_suspend,
	.resume = zt2003_pm_resume,
};
#endif

static struct i2c_driver zt2003_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = tu_probe,
	.remove = tu_remove,
	.id_table 	= tu_id_table,
	.driver = {
		.name	= CTP_NAME,
		.owner	= THIS_MODULE,
#ifdef CONFIG_HAS_EARLYSUSPEND
#else
#ifdef CONFIG_PM
		.pm		= &zt2003_pm_ops,
#endif
#endif
	},
};

module_i2c_driver(zt2003_driver);

MODULE_DESCRIPTION("ZT2003 Touchscreen Driver");
MODULE_LICENSE("GPL v2");
