//
// A V4L2 driver for TP2825 video decoder
//
//

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-mediabus.h>
#include <linux/io.h>

#include <asm/delay.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#include "camera.h"


MODULE_AUTHOR("cobenhan");
MODULE_DESCRIPTION("A low-level driver for TP2825 raw video decoder");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN	1

#if(DEV_DBG_EN == 1)
#define vfe_dev_dbg(x,arg...) printk("[TP2825 Raw]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...)
#endif
#define vfe_dev_err(x,arg...) printk("[TP2825 Raw]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[TP2825 Raw]"x,##arg)

#define LOG_ERR_RET(x)  { \
		int ret;  \
		ret = x; \
		if(ret < 0) {\
			vfe_dev_err("error at %s\n",__func__);  \
			return ret; \
		} \
	}

enum {
	CH1 = 0,
	CH2,
	CH3,
	CH4,
	AutoSel
};


#define MAX_CHIPS	1
#define MAX_COUNT 	0xff

enum {
	VIDEO_UNPLUG = 0,
	VIDEO_IN,
	VIDEO_LOCKED,
	VIDEO_UNLOCK
};

static struct {
	int state;
	int count;
	int std;
} chip_info[MAX_CHIPS];

#define FLAG_LOSS		0x80
#define FLAG_LOCKED		0x60
#define FLAG_DETECTED		0x08
#define FLAG_PROGRESSIVE	0x02

//cvstd
enum {
	R720P60 = 0,
	R720P50,
	R1080P30,
	R1080P25,
	R720P30,
	R720P25,
	NTPAL,
	NoDet,
	R720P30V2,
	R720P25V2,
	NTSC,
	PAL
};


//define module timing
#define MCLK              (24*1000*1000)
#define VREF_POL          V4L2_MBUS_VSYNC_ACTIVE_LOW
#define HREF_POL          V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_FALLING
#define V4L2_IDENT_SENSOR  0x2825

//define the voltage level of control signal
#define CSI_STBY_ON     1
#define CSI_STBY_OFF    0
#define CSI_RST_ON      0
#define CSI_RST_OFF     1
#define CSI_PWR_ON      1
#define CSI_PWR_OFF     0
#define CSI_AF_PWR_ON   1
#define CSI_AF_PWR_OFF  0
#define regval_list reg_list_a8_d8


#define REG_TERM 0xff
#define VAL_TERM 0xfe
#define REG_DLY  0xff

//define the registers
#define EXP_HIGH    0xff
#define EXP_MID     0x02
#define EXP_LOW     0x01
#define GAIN_HIGH   0xff
#define GAIN_LOW    0x00
//#define FRACTION_EXP

#define ID_REG_HIGH   0xfe
#define ID_REG_LOW    0xff
#define ID_VAL_HIGH   ((V4L2_IDENT_SENSOR) >> 8)
#define ID_VAL_LOW    ((V4L2_IDENT_SENSOR) & 0xff)


/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30


/*
 * The i2c address
 */
#define I2C_ADDR 0x88
//SAD: low(0x88), high(0x8A)
//read: +1, write: +0

//static struct delayed_work sensor_s_ae_ratio_work;
static struct v4l2_subdev *glb_sd;
#define SENSOR_NAME "tp2825"


volatile static unsigned int watchdog_state = 0;
volatile static unsigned int tp2825_initialized = 0;
struct task_struct *task_watchdog_deamon = NULL;

static DEFINE_SPINLOCK(watchdog_lock);

#define WATCHDOG_EXIT		0
#define WATCHDOG_RUNNING	1
#define WATCHDOG			1

static int  tp2825_watchdog_init(void);
static void tp2825_watchdog_exit(void);

/*
 * Information we maintain about a known sensor.
 */
struct sensor_format_struct;  /* coming later */

struct cfg_array { /* coming later */
	struct regval_list * regs;
	int size;
};

static inline struct sensor_info *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct sensor_info, sd);
}


/*
 * The default register settings
 *
 */


static struct regval_list sensor_default_regs[] =
{
	//video
	{ 0x40, 0x00 }, /* data page0 set access */
	{ 0x07, 0xc0 }, /* bypass test function enabled([7]), adaptive eq2 enabeld([6]) */
	{ 0x0b, 0xc0 }, /* bypass test function enabled([7]), adaptive eq1 enabeld([6]) */
	{ 0x39, 0x8c }, /* bypass eq3bp([7]), lpf 50MHz[3:2] */

	/* vd2[1] output enabled, vd1[0] output enabled */
	{ 0x4d, 0x03 }, 
	/* 20mA output driver on CLKO pin */
	{ 0x4e, 0x03 }, 

	//PTZ
	{ 0xc8, 0x21 },
	{ 0x7e, 0x01 }, /* TXD pin output enabled */
	{ 0xb9, 0x01 }, /* VIN Rx data enabled */

};


//148MHz version (8-bit)
static struct regval_list sensor_v1_regs[] =
{
	{ 0x0c, 0x43 },
	{ 0x0d, 0x10 },
	{ 0x20, 0x60 },
	{ 0x26, 0x02 },
	
	{ 0x2b, 0x4a },

	{ 0x2d, 0x30 },
	{ 0x2e, 0x70 },

	{ 0x30, 0x48 },
	{ 0x31, 0xbb },
	{ 0x32, 0x2e },
	{ 0x33, 0x90 },
	{ 0x35, 0x05 },
	{ 0x39, 0x8c },

};


//74MH version (16-bit)
static struct regval_list sensor_v2_regs[] =
{
	/* Sync EQ control[6], EQ Freq;74MHz(1)[4], TLHY[3:0] */
	{ 0x0c, 0x53 }, //74MHz

	/* Combo filter[4], Y control[3], SD decoding format[2:0] */
	{ 0x0d, 0x10 },
	/* Clamp position */
	{ 0x20, 0x60 },
	/* Clamp Enable(0) Disable(1) [7], Clamp Current 1x(0) 2x(1)[6], GTST[5], 
	   Sync filter bandwidth High(0), Low(1)[4], Clamp mode[1:0] */ 
	{ 0x26, 0x02 },
	/* Color killter threshold reg: Hysteresis[7:6], threshold[5:0], 0x70 for SD mode */
	{ 0x2b, 0x70 },
	/* Color burst gate position[7:0] */
	{ 0x2d, 0x30 },
	/* color gain loop ref. */
	{ 0x2e, 0x70 },

	/* Color Carrier DDS */
	{ 0x30, 0x48 },
	{ 0x31, 0xbb },
	{ 0x32, 0x2e },
	{ 0x33, 0x90 },

	/* DS2 output mode[6], Status reflect: 74.25MHz(1), 148.5Mhz(0)[5] */
	{ 0x35, 0x25 },

	/* EQ3BP Bypass(1), Enabled(0)([7]), lpf Bypass(0), 10Mhz(1), 25Mhz(2) 50MHz(3)[3:2] */
	{ 0x39, 0x88 },
};

#if 0
//for EQ detect
static struct regval_list sensor_720p30_regs[] =
{
//	{ 0x14, 0x40 },

	{ 0x2d, 0x48 }, /* color burst gate control position rel. to internal horz sync pos. */
	{ 0x2e, 0x5e }, /* color gain loop ref. */

	{ 0x30, 0x27 }, /* color carrier DDS control registers 0x30 ~ 0x33 */
	{ 0x31, 0x72 },
	{ 0x32, 0x80 },
	{ 0x33, 0x77 },

};


static struct regval_list sensor_720p25_regs[] =
{
//	{ 0x14, 0x40 },

	{ 0x2d, 0x48 }, /* color burst gate control position rel. to internal horz sync pos. */
	{ 0x2e, 0x5e }, /* color gain loop ref. */

	{ 0x30, 0x27 }, /* color carrier DDS control registers 0x30 ~ 0x33 */
	{ 0x31, 0x88 },
	{ 0x32, 0x04 },
	{ 0x33, 0x23 },

};


static struct regval_list sensor_1080p30_regs[] =
{
//	{ 0x14, 0x60 },

	{ 0x2d, 0x45 }, /* color burst gate control position rel. to internal horz sync pos. */
	{ 0x2e, 0x50 }, /* color gain loop ref. */

	{ 0x30, 0x29 }, /* color carrier DDS control registers 0x30 ~ 0x33 */
	{ 0x31, 0x65 },
	{ 0x32, 0x78 },
	{ 0x33, 0x16 },

};

static struct regval_list sensor_1080p25_regs[] =
{
//	{ 0x14, 0x60 },

	{ 0x2d, 0x45 }, /* color burst gate control position rel. to internal horz sync pos. */
	{ 0x2e, 0x40 }, /* color gain loop ref. */

	{ 0x30, 0x29 }, /* color carrier DDS control registers 0x30 ~ 0x33 */
	{ 0x31, 0x61 },
	{ 0x32, 0x78 },
	{ 0x33, 0x16 },

};

#else

/* NPXL (# of pixels per line, reg.1c & 1d)
	fmt			decimal		hex
	720p@60		1650		672
	720p@50		1980		7bc
	720p@30		3300		ce4
	720p@25		3960		f78
	1080p@30	2200		898
	1080p@25	2640		a50
	480i		4720		1270/938
	576i		4752		1290/948
*/
static struct regval_list sensor_1080p30_regs[] =
{
	/* 8bit[7], Cb/Cr first[6], Output limit(1~254)[5], F444: event pixel count[4],
	    BT.656 [3], HD mode[2], 1080p decoding[1], Progressive mode[0]
	*/
	{ 0x02, /*0x8a*/ 0xca },
	/* H delay(9-8)[5:4], Y delay (luma data delay rel. to chroma data delay) [1:0] */
	{ 0x15, 0x03 },
	/* H delay(7-0): distance from internal sync ref. to start of video output: -512~+511 pixels */
	{ 0x16, 0xd3 },
	/* H active horz output length in output pixels#(7-0): for 1080p, 1920d(780h), for 720p, 1280d(500h)  */
	{ 0x17, 0x80 },
	/* V delay (starting line of display output from vertical sync) */
	{ 0x18, 0x29 },
	/* V active (# of active output lines per frame): for 1080p, 1080d(438h), for 720p, 720d(2d0h) */
	{ 0x19, 0x38 },
	/* V active(11-8)[7:0], H active(11-8)[3:0] */
	{ 0x1a, 0x47 },
	/* NPXL(# of pixels per line): (12-8)[4:0] */
	{ 0x1c, 0x08 },
	/* NPXL(7-0) */
	{ 0x1d, 0x98 },
};

static struct regval_list sensor_1080p25_regs[] =
{
	{ 0x02, /*0x8a*/ 0xca },
	{ 0x15, 0x03 },
	{ 0x16, 0xd3 },
	{ 0x17, 0x80 },
	{ 0x18, 0x29 },
	{ 0x19, 0x38 },
	{ 0x1a, 0x47 },
	{ 0x1c, 0x0a },
	{ 0x1d, 0x50 },
};


static struct regval_list sensor_720p60_regs[] =
{
	{ 0x02, /*0x8a*/ 0xca },
	{ 0x15, 0x13 },
	{ 0x16, 0x16 },
	{ 0x17, 0x00 },
	{ 0x18, 0x19 },
	{ 0x19, 0xd0 },
	{ 0x1a, 0x25 },
	{ 0x1c, 0x06 },
	{ 0x1d, 0x72 },
};

static struct regval_list sensor_720p50_regs[] =
{
	{ 0x02, /*0x8a*/ 0xca },
	{ 0x15, 0x13 },
	{ 0x16, 0x16 },
	{ 0x17, 0x00 },
	{ 0x18, 0x19 },
	{ 0x19, 0xd0 },
	{ 0x1a, 0x25 },
	{ 0x1c, 0x07 },
	{ 0x1d, 0xbc },
};

static struct regval_list sensor_720p30_regs[] =
{
	{ 0x02, /*0x8a*/ 0xca },
	{ 0x15, 0x13 },
	{ 0x16, 0x16 },
	{ 0x17, 0x00 },
	{ 0x18, 0x19 },
	{ 0x19, 0xd0 },
	{ 0x1a, 0x25 },
	{ 0x1c, 0x0c },
	{ 0x1d, 0xe4 },
};

static struct regval_list sensor_720p25_regs[] =
{
	{ 0x02, /*0x8a*/ 0xca },
	{ 0x15, 0x13 },
	{ 0x16, 0x16 },
	{ 0x17, 0x00 },
	{ 0x18, 0x19 },
	{ 0x19, 0xd0 },
	{ 0x1a, 0x25 },
	{ 0x1c, 0x0f },
	{ 0x1d, 0x78 },
};

#endif


static struct {
	char *name;
	struct regval_list *regs;
	int size;
} format_list[] = {
	{ "1080p30", sensor_1080p30_regs, ARRAY_SIZE(sensor_1080p30_regs) },
	//{ "1080p25", sensor_1080p25_regs, ARRAY_SIZE(sensor_1080p25_regs) },
	{ "720p60", sensor_720p60_regs, ARRAY_SIZE(sensor_720p60_regs) },
	//{ "720p50", sensor_720p50_regs, ARRAY_SIZE(sensor_720p50_regs) },
	{ "720p30", sensor_720p30_regs, ARRAY_SIZE(sensor_720p30_regs) },
	//{ "720p25", sensor_720p25_regs, ARRAY_SIZE(sensor_720p25_regs) },
	{ NULL, }
};




static struct regval_list sensor_oe_disable_regs[] = {

};


static struct regval_list sensor_oe_enable_regs[] = {
};

/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 *
 */

static struct regval_list sensor_fmt_yuv422_yuyv[] = {
};

static struct regval_list sensor_fmt_yuv422_uyvy[] = {
};

/*
 * Low-level register I/O.
 *
 */
static int sensor_read(struct v4l2_subdev *sd, unsigned char reg,
                       unsigned char *value)
{
	int ret = 0;
	int cnt = 0;

	ret = cci_read_a8_d8(sd, reg, value);
	while (ret != 0 && cnt < 3)
	{
		ret = cci_read_a8_d8(sd, reg, value);
		cnt++;
	}
	if (cnt > 0)
		vfe_dev_dbg("sensor read retry=%d\n", cnt);

	return ret;
}

static int reg_val_show(struct v4l2_subdev *sd, unsigned char reg)
{
	unsigned char tmp;
	sensor_read(sd, reg, &tmp);
	printk("0x%x value is 0x%x\n", reg, tmp);
	return 0;
}

static int sensor_write(struct v4l2_subdev *sd, unsigned char reg,
                        unsigned char value)
{
	int ret = 0;
	int cnt = 0;
	ret = cci_write_a8_d8(sd, reg, value);
	while (ret != 0 && cnt < 2)
	{
		ret = cci_write_a8_d8(sd, reg, value);
		cnt++;
	}
	if (cnt >= 1)
		vfe_dev_dbg("sensor write retry=%d\n", cnt);

	return ret;
}

static int sensor_bwrite(struct v4l2_subdev *sd, unsigned char reg, unsigned char bitpos, unsigned char bitval)
{
	unsigned char val, mask = 0x00;
	int ret;

	mask = 0x01 << bitpos;

	ret = sensor_read(sd, reg, &val);
	if (ret) {
		return ret;
	}

	if (bitval) {
		val |= mask;
	} else {
		val &= ~mask;
	}

	return sensor_write(sd, reg, val);
}



/*
 * Write a list of register settings;
 */
static int sensor_write_array(struct v4l2_subdev *sd, struct regval_list *regs, int array_size)
{
	int i = 0;

	if (!regs)
		return -EINVAL;

	while (i < array_size)
	{
		if (regs->addr == REG_DLY) {
			msleep(regs->data);
		}
		else
		{
			sensor_write(sd, regs->addr, regs->data);
		}
		i++;
		regs++;
	}
	return 0;
}


static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{

	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{

	return 0;
}


static int sensor_g_gain(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->gain;
	//vfe_dev_dbg("sensor_get_gain = %d\n", info->gain);
	return 0;
}

static int tp2825_sensor_vts;


static int sensor_s_gain(struct v4l2_subdev *sd, int gain_val)
{
	return 0;

}


static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{


	return 0;
}


static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret = 0;

	return ret;
}

static int sensor_set_power_sleep(struct v4l2_subdev *sd, int on_off)
{
	int ret = 0;


	return ret;
}

static int sensor_set_pll_enable(struct v4l2_subdev *sd, int on_off)
{

	uint8_t pll_con;

	LOG_ERR_RET(sensor_read(sd, 0x44, &pll_con));

	//pll power down[6]
	LOG_ERR_RET(sensor_write(sd, 0x44, (pll_con | 0x40)));
	LOG_ERR_RET(sensor_write(sd, 0x44, (pll_con & 0xbf)));

	return 0;
}

static int sensor_reset_all_channels(struct v4l2_subdev *sd)
{
	//reg 0x06 : reset control reg. soft reset[7]
	uint8_t reg40, ch;
	int ret;

	LOG_ERR_RET(sensor_read(sd, 0x40, &reg40));

	for (ch = CH1; ch <= CH4; ch++) {
		LOG_ERR_RET(sensor_write(sd, 0x40, ch));
		LOG_ERR_RET(sensor_bwrite(sd, 0x06, 7, 1));
	}

	LOG_ERR_RET(sensor_write(sd, 0x40, reg40));

	return 0;
}

static int sensor_sysclk_v2(struct v4l2_subdev *sd)
{
	int ret;
	uint8_t tmp;

	LOG_ERR_RET(sensor_read(sd, 0x4e, &tmp));

#if 1
	tmp |= 0x04; //74.25MHz (16-bit)
#else
	tmp &= 0xcf;
	tmp |= 0x14; //74.25MHz (16-bit)
#endif


	LOG_ERR_RET(sensor_write(sd, 0x4e, tmp));

#if 0
	//free-run[3] and blue screen[2]
	LOG_ERR_RET(sensor_read(sd, 0x2a, &tmp));
	LOG_ERR_RET(sensor_write(sd, 0x2a, (tmp|0x0c)));
	//LOG_ERR_RET(sensor_write(sd, 0x2a, (tmp|0x08)));
#endif

	return 0;
}


static int sensor_sysclk_v1(struct v4l2_subdev *sd)
{
	int ret;
	uint8_t tmp;

	LOG_ERR_RET(sensor_read(sd, 0x4e, &tmp));

	tmp &= 0xfb;

	LOG_ERR_RET(sensor_write(sd, 0x4e, tmp));

#if 0
	//free-run[3] and blue screen[2]
	LOG_ERR_RET(sensor_read(sd, 0x2a, &tmp));
	LOG_ERR_RET(sensor_write(sd, 0x2a, (tmp|0x0c)));
	//LOG_ERR_RET(sensor_write(sd, 0x2a, (tmp|0x08)));
#endif

}

static int sensor_reset_default(struct v4l2_subdev *sd)
{
	uint8_t tmp;

	//sharpness[0:4] control
	LOG_ERR_RET(sensor_read(glb_sd, 0x14, &tmp));
	tmp &= 0x9F;
	LOG_ERR_RET(sensor_write(glb_sd, 0x14, tmp));
	//EQ2 adaptive enabled
	//LOG_ERR_RET(sensor_write(glb_sd, 0x07, 0x40));
	//adaptive EQ1 enabled
	//LOG_ERR_RET(sensor_write(glb_sd, 0x0b, 0x40));
	//eq control
	//LOG_ERR_RET(sensor_write(glb_sd, 0x3a, 0x88));
	//clamping control
	LOG_ERR_RET(sensor_write(glb_sd, 0x26, 0x02));

	return 0;
}


/*
 * Stuff that knows about the sensor.
 */

static int sensor_power(struct v4l2_subdev *sd, int on)
{
	int ret;

	//insure that clk_disable() and clk_enable() are called in pair
	//when calling CSI_SUBDEV_STBY_ON/OFF and CSI_SUBDEV_PWR_ON/OFF
	ret = 0;
	switch (on)
	{
	case CSI_SUBDEV_STBY_ON:
		vfe_dev_dbg("CSI_SUBDEV_STBY_ON!\n");
		break;

	case CSI_SUBDEV_STBY_OFF:
		vfe_dev_dbg("CSI_SUBDEV_STBY_OFF!\n");
		break;

	case CSI_SUBDEV_PWR_ON:
		vfe_dev_dbg("CSI_SUBDEV_PWR_ON!\n");
		//make sure that no device can access i2c bus during sensor initial or power down
		//when using i2c_lock_adpater function, the following codes must not access i2c bus before calling i2c_unlock_adapter
		cci_lock(sd);

		//power on reset
		//vfe_gpio_set_status(sd, PWDN, 1); //set the gpio to output
		vfe_gpio_set_status(sd, RESET, 1); //set the gpio to output

		vfe_gpio_write(sd, RESET, CSI_GPIO_LOW);
		//vfe_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		usleep_range(1000, 1200);

		//power supply
		vfe_gpio_write(sd, POWER_EN, CSI_PWR_ON);

		/*
				//add by chao
				usleep_range(1000, 1200);
				vfe_set_pmu_channel(sd, AVDD, ON);
				vfe_set_pmu_channel(sd, DVDD, ON);
				vfe_set_pmu_channel(sd, IOVDD, ON);
				//vfe_set_pmu_channel(sd,AFVDD,ON);
		*/

		//active mclk before power on
		vfe_set_mclk_freq(sd, MCLK);
		vfe_set_mclk(sd, ON);
		usleep_range(10000, 12000);

		//reset on io
		vfe_gpio_write(sd, RESET, CSI_GPIO_HIGH);
		usleep_range(7000, 8000);
		//vfe_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		//usleep_range(10000, 12000);

		//remember to unlock i2c adapter, so the device can access the i2c bus again
		cci_unlock(sd);
		break;


	case CSI_SUBDEV_PWR_OFF:
		vfe_dev_dbg("CSI_SUBDEV_PWR_OFF!\n");

		spin_lock_irq(&watchdog_lock);
		tp2825_initialized = 0;
		spin_unlock_irq(&watchdog_lock);

		//make sure that no device can access i2c bus during sensor initial or power down
		//when using i2c_lock_adpater function, the following codes must not access i2c bus before calling i2c_unlock_adapter
		cci_lock(sd);

		//vfe_gpio_set_status(sd, PWDN, 1); //set the gpio to output
		vfe_gpio_set_status(sd, RESET, 1); //set the gpio to output

		vfe_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		vfe_gpio_write(sd, RESET, CSI_GPIO_LOW);

		//power supply
		vfe_gpio_write(sd, POWER_EN, CSI_PWR_OFF);


		//inactive mclk before power off
		vfe_set_mclk(sd, OFF);


		/*
				//power supply off
				//vfe_gpio_write(sd,POWER_EN,CSI_PWR_OFF);
				//vfe_set_pmu_channel(sd,AFVDD,OFF);
				vfe_set_pmu_channel(sd, IOVDD, OFF);
				vfe_set_pmu_channel(sd, DVDD, OFF);
				vfe_set_pmu_channel(sd, AVDD, OFF);
		*/

		//set the io to hi-z
		vfe_gpio_set_status(sd, RESET, 0); //set the gpio to input
		//vfe_gpio_set_status(sd, PWDN, 0); //set the gpio to input
		//remember to unlock i2c adapter, so the device can access the i2c bus again
		cci_unlock(sd);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
	switch (val)
	{
	case 0:
		vfe_gpio_write(sd, RESET, CSI_RST_OFF);
		usleep_range(10000, 12000);
		break;

	case 1:
		vfe_gpio_write(sd, RESET, CSI_RST_ON);
		usleep_range(10000, 12000);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	unsigned char rdval = 0;


	vfe_dev_dbg("sensor_detect\n");


	LOG_ERR_RET(sensor_read(sd, ID_REG_HIGH, &rdval))

	if (rdval != ID_VAL_HIGH)
	{
		printk(KERN_DEBUG"*********sensor error,read id is %d.\n", rdval);
		return -ENODEV;
	}

	LOG_ERR_RET(sensor_read(sd, ID_REG_LOW, &rdval))
	if (rdval != ID_VAL_LOW)
	{
		printk(KERN_DEBUG"*********sensor error,read id is %d.\n", rdval);
		return -ENODEV;
	}
	else
	{
		printk(KERN_DEBUG"*********find tp2825 raw data camera sensor now.\n");
		return 0;
	}
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);

	vfe_dev_dbg("sensor_init(val %d)\n", val);

	/*Make sure it is a target sensor*/
	ret = sensor_detect(sd);
	if (ret) {
		vfe_dev_err("chip found is not an target chip.\n");
		return ret;
	}


	vfe_get_standby_mode(sd, &info->stby_mode);

	if ((info->stby_mode == HW_STBY || info->stby_mode == SW_STBY) \
	        && info->init_first_flag == 0) {
		vfe_dev_print("stby_mode and init_first_flag = 0\n");
		return 0;
	}

	info->focus_status = 0;
	info->low_speed = 0;
	info->width = HD720_WIDTH;
	info->height = HD720_HEIGHT;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;

	info->tpf.numerator = 1;
	info->tpf.denominator = 30;    /* 30fps */

	memset(chip_info, 0, sizeof(chip_info));
	chip_info[0].std = NoDet;

	ret = sensor_write_array(sd, sensor_default_regs, ARRAY_SIZE(sensor_default_regs));
	if (ret < 0) {
		vfe_dev_err("write sensor_default_regs error\n");
		return ret;
	}

	ret = sensor_set_pll_enable(sd, 1);
	if (ret < 0) {
		vfe_dev_err("write pll enable error\n");
		return ret;
	}


	ret = sensor_write_array(sd, sensor_720p60_regs, ARRAY_SIZE(sensor_720p60_regs));
	if (ret < 0) {
		vfe_dev_err("write sensor_720p60_regs error\n");
		return ret;
	}

	sensor_reset_all_channels(sd);


	if (info->stby_mode == 0)
		info->init_first_flag = 0;

	info->preview_first_flag = 1;

	spin_lock_irq(&watchdog_lock);
	tp2825_initialized = 1;
	spin_unlock_irq(&watchdog_lock);

	return 0;
}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct sensor_info *info = to_state(sd);
//  vfe_dev_dbg("[]cmd=%d\n",cmd);
//  vfe_dev_dbg("[]arg=%0x\n",arg);
	switch (cmd) {
	case GET_CURRENT_WIN_CFG:
		if (info->current_wins != NULL)
		{
			memcpy( arg,
			        info->current_wins,
			        sizeof(struct sensor_win_size) );
			ret = 0;
		}
		else
		{
			vfe_dev_err("empty wins!\n");
			ret = -1;
		}
		break;
	case SET_FPS:
		ret = 0;
//      if((unsigned int *)arg==1)
//        ret=sensor_write(sd, 0x3036, 0x78);
//      else
//        ret=sensor_write(sd, 0x3036, 0x32);
		break;
	case ISP_SET_EXP_GAIN:
		sensor_s_exp_gain(sd, (struct sensor_exp_gain *)arg);
		break;
	default:
		return -EINVAL;
	}
	return ret;
}


/*
 * Store information about the video data format.
 */
static struct sensor_format_struct {
	__u8 *desc;
	//__u32 pixelformat;
	enum v4l2_mbus_pixelcode mbus_code;
	struct regval_list *regs;
	int regs_size;
	int bpp;   /* Bytes per pixel */
} sensor_formats[] = {
	{
		.desc   = "UYVY 4:2:2",
		.mbus_code  = V4L2_MBUS_FMT_UYVY8_2X8,
		.regs     = sensor_fmt_yuv422_uyvy,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_uyvy),
		.bpp    = 2
	},
	/*
	{
		.desc   = "YUYV 4:2:2",
		.mbus_code  = V4L2_MBUS_FMT_YUYV8_2X8,
		.regs     = sensor_fmt_yuv422_yuyv,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_yuyv),
		.bpp    = 2
	},
	*/

};
#define N_FMTS ARRAY_SIZE(sensor_formats)



/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */


static struct sensor_win_size sensor_win_sizes[] = {

	/* 720p */
	{
		.width      = HD720_WIDTH,
		.height     = HD720_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 1600,//1288,
		.vts        = 750,
		.pclk       = 36000000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = (750 - 4) << 4,
		.gain_min   = 1 << 4,
		.gain_max   = (16 << 4) - 1,
		.regs       = sensor_720p30_regs,//
		.regs_size  = ARRAY_SIZE(sensor_720p30_regs),//
		.set_size   = NULL,
	},

};

#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))

static int sensor_enum_fmt(struct v4l2_subdev *sd, unsigned index,
                           enum v4l2_mbus_pixelcode *code)
{
	if (index >= N_FMTS)
		return -EINVAL;

	*code = sensor_formats[index].mbus_code;
	return 0;
}

static int sensor_enum_size(struct v4l2_subdev *sd,
                            struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index > N_WIN_SIZES - 1)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->discrete.width = sensor_win_sizes[fsize->index].width;
	fsize->discrete.height = sensor_win_sizes[fsize->index].height;

	return 0;
}

static int sensor_try_fmt_internal(struct v4l2_subdev *sd,
                                   struct v4l2_mbus_framefmt *fmt,
                                   struct sensor_format_struct **ret_fmt,
                                   struct sensor_win_size **ret_wsize)
{
	int index;
	struct sensor_win_size *wsize;
	struct sensor_info *info = to_state(sd);

	for (index = 0; index < N_FMTS; index++)
		if (sensor_formats[index].mbus_code == fmt->code)
			break;

	if (index >= N_FMTS)
		return -EINVAL;

	if (ret_fmt != NULL)
		*ret_fmt = sensor_formats + index;

	/*
	 * Fields: the sensor devices claim to be progressive.
	 */

	fmt->field = V4L2_FIELD_NONE;

	/*
	 * Round requested image size down to the nearest
	 * we support, but not below the smallest.
	 */
	for (wsize = sensor_win_sizes; wsize < sensor_win_sizes + N_WIN_SIZES;
	        wsize++)
		if (fmt->width >= wsize->width && fmt->height >= wsize->height)
			break;

	if (wsize >= sensor_win_sizes + N_WIN_SIZES)
		wsize--;   /* Take the smallest one */
	if (ret_wsize != NULL)
		*ret_wsize = wsize;
	/*
	 * Note the size we'll actually handle.
	 */
	fmt->width = wsize->width;
	fmt->height = wsize->height;
	info->current_wins = wsize;
	//pix->bytesperline = pix->width*sensor_formats[index].bpp;
	//pix->sizeimage = pix->height*pix->bytesperline;

	return 0;
}

static int sensor_try_fmt(struct v4l2_subdev *sd,
                          struct v4l2_mbus_framefmt *fmt)
{
	return sensor_try_fmt_internal(sd, fmt, NULL, NULL);
}

static int sensor_g_mbus_config(struct v4l2_subdev *sd,
                                struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_BT656;
	//cfg->flags = V4L2_MBUS_MASTER | VREF_POL | HREF_POL | CLK_POL ;

	return 0;
}


/*
 * Set a format.
 */
static int sensor_s_fmt(struct v4l2_subdev *sd,
                        struct v4l2_mbus_framefmt *fmt)
{
	int ret;
	struct sensor_format_struct *sensor_fmt;
	struct sensor_win_size *wsize;
	struct sensor_info *info = to_state(sd);

	vfe_dev_dbg("sensor_s_fmt(capture mode %d)\n", info->capture_mode);

//  sensor_write_array(sd, sensor_oe_disable_regs, ARRAY_SIZE(sensor_oe_disable_regs));

	ret = sensor_try_fmt_internal(sd, fmt, &sensor_fmt, &wsize);


	if (ret)
		return ret;

	if (info->capture_mode == V4L2_MODE_VIDEO)
	{
		//video
	}
	else if (info->capture_mode == V4L2_MODE_IMAGE)
	{
		//image

	}

	LOG_ERR_RET(sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size))

	ret = 0;
	if (wsize->regs)
	{
		// usleep_range(5000,6000);
		LOG_ERR_RET(sensor_write_array(sd, wsize->regs, wsize->regs_size))
	}

	if (wsize->set_size)
		LOG_ERR_RET(wsize->set_size(sd))

		info->fmt = sensor_fmt;
	info->width = wsize->width;
	info->height = wsize->height;

	tp2825_sensor_vts = wsize->vts;


	vfe_dev_print("s_fmt set width = %d, height = %d\n", wsize->width, wsize->height);

	if (info->capture_mode == V4L2_MODE_VIDEO)
	{
		//video

	} else {
		//capture image

	}

	//sensor_write_array(sd, sensor_oe_enable_regs, ARRAY_SIZE(sensor_oe_enable_regs));

	return 0;
}

/*
 * Implement G/S_PARM.  There is a "high quality" mode we could try
 * to do someday; for now, we just do the frame rate tweak.
 */
static int sensor_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	struct v4l2_captureparm *cp = &parms->parm.capture;
	struct sensor_info *info = to_state(sd);

	if (parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	memset(cp, 0, sizeof(struct v4l2_captureparm));
	cp->capability = V4L2_CAP_TIMEPERFRAME;
	cp->capturemode = info->capture_mode;

	return 0;
}

static int sensor_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	struct v4l2_captureparm *cp = &parms->parm.capture;
	//struct v4l2_fract *tpf = &cp->timeperframe;
	struct sensor_info *info = to_state(sd);
	//unsigned char div;

	vfe_dev_dbg("sensor_s_parm(type %d, numerator %d)\n", parms->type, info->tpf.numerator);

	return 0;
}


static int sensor_queryctrl(struct v4l2_subdev *sd,
                            struct v4l2_queryctrl *qc)
{
	/* Fill in min, max, step and default value for these controls. */
	/* see include/linux/videodev2.h for details */

	switch (qc->id) {
	case V4L2_CID_GAIN:
		return v4l2_ctrl_query_fill(qc, 1 * 16, 16 * 16, 1, 16);
	case V4L2_CID_EXPOSURE:
		return v4l2_ctrl_query_fill(qc, 1, 65536 * 16, 1, 1);
	case V4L2_CID_FRAME_RATE:
		return v4l2_ctrl_query_fill(qc, 15, 120, 1, 30);
	}
	return -EINVAL;
}

static int sensor_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_g_gain(sd, &ctrl->value);
	case V4L2_CID_EXPOSURE:
		return sensor_g_exp(sd, &ctrl->value);
	}
	return -EINVAL;
}

static int sensor_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct v4l2_queryctrl qc;
	int ret;

	qc.id = ctrl->id;
	ret = sensor_queryctrl(sd, &qc);
	if (ret < 0) {
		return ret;
	}

// if (ctrl->value < qc.minimum || ctrl->value > qc.maximum) {
//   return -ERANGE;
// }

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_s_gain(sd, ctrl->value);

	case V4L2_CID_EXPOSURE:
		return sensor_s_exp(sd, ctrl->value);

		//case V4L2_CID_FRAME_RATE:
		//  return sensor_s_framerate(sd, ctrl->value);
	}
	return -EINVAL;
}


static int sensor_g_chip_ident(struct v4l2_subdev *sd,
                               struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_SENSOR, 0);
}


/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops sensor_core_ops = {
	.g_chip_ident = sensor_g_chip_ident,
	.g_ctrl = sensor_g_ctrl,
	.s_ctrl = sensor_s_ctrl,
	.queryctrl = sensor_queryctrl,
	.reset = sensor_reset,
	.init = sensor_init,
	.s_power = sensor_power,
	.ioctl = sensor_ioctl,
};

static const struct v4l2_subdev_video_ops sensor_video_ops = {
	.enum_mbus_fmt = sensor_enum_fmt,
	.enum_framesizes = sensor_enum_size,
	.try_mbus_fmt = sensor_try_fmt,
	.s_mbus_fmt = sensor_s_fmt,
	.s_parm = sensor_s_parm,
	.g_parm = sensor_g_parm,
	.g_mbus_config = sensor_g_mbus_config,
};

static const struct v4l2_subdev_ops sensor_ops = {
	.core = &sensor_core_ops,
	.video = &sensor_video_ops,
};

/* ----------------------------------------------------------------------- */
static struct cci_driver cci_drv = {
	.name = SENSOR_NAME,
	.addr_width = CCI_BITS_8,
	.data_width = CCI_BITS_8,
};

static int sensor_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct sensor_info *info;
//  int ret;

	vfe_dev_dbg("sensor_probe(client %p, id %p)\n", client, id);

	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	sd = &info->sd;

	glb_sd = sd;

	cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv);

	info->fmt = &sensor_formats[0];
	info->af_first_flag = 1;
	info->init_first_flag = 1;

#if WATCHDOG
	tp2825_watchdog_init();
#endif

	return 0;
}
static int sensor_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd;

	vfe_dev_dbg("sensor_remove(client %p)\n", client);

#if WATCHDOG
	tp2825_watchdog_exit();
#endif

	sd = cci_dev_remove_helper(client, &cci_drv);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id sensor_id[] = {
	{ SENSOR_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sensor_id);


static struct i2c_driver sensor_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = SENSOR_NAME,
	},
	.probe = sensor_probe,
	.remove = sensor_remove,
	.id_table = sensor_id,
};

static __init int init_sensor(void)
{
	vfe_dev_dbg("init_sensor\n");
	return cci_dev_init_helper(&sensor_driver);
}

static __exit void exit_sensor(void)
{
	cci_dev_exit_helper(&sensor_driver);
}



static int tp2825_watchdog_deamon(void *data)
{
	struct sched_param param = {
		.sched_priority = 99
	};

	struct timeval start, end;
	int interval, ret;
	int chip = 0, format = 0, format_loop = 0;
	unsigned char status, vin, tmp, cvstd;
	//unsigned char gain, agc, flag_locked;

	vfe_dev_dbg("tp2825_watchdog_deamon: start!\n");

	sched_setscheduler(current, SCHED_FIFO, &param);
	current->flags |= PF_NOFREEZE;

	set_current_state(TASK_INTERRUPTIBLE);


	while (watchdog_state != WATCHDOG_EXIT)	{
		spin_lock_irq(&watchdog_lock);

		do_gettimeofday(&start);

		if (glb_sd && tp2825_initialized) {
			do {
				ret = sensor_read(glb_sd, 0x01, &status);
				if (ret) break;
				ret = sensor_read(glb_sd, 0x02, &tmp);
				ret = sensor_read(glb_sd, 0x03, &cvstd);
				vfe_dev_dbg("status = %02x, config = %02x, det = %02x\n", status, tmp, cvstd);
				//channel
				ret = sensor_read(glb_sd, 0x41, &vin);
				if (ret) break;
				vin &= 0x7;

				if (status & FLAG_LOSS) {
					if (VIDEO_UNPLUG == chip_info[chip].state) {
						if (chip_info[chip].count < MAX_COUNT) chip_info[chip].count++;

						ret = sensor_read(glb_sd, 0x41, &vin);
						if (ret) break;
						vin &= 0x7;
						//vfe_dev_dbg("channel = %02x\n", vin);
						//set next channel
						if (vin == 0x04) vin = 0;
						else vin++;
						sensor_write(glb_sd, 0x41, vin);
						continue;
					} else {
						chip_info[chip].state = VIDEO_UNPLUG;
						chip_info[chip].count = 0;
						chip_info[chip].std = NoDet;
						ret = sensor_reset_default(glb_sd);
						vfe_dev_dbg("video loss\n");
					}
				} else {
					if ((status & FLAG_LOCKED) == FLAG_LOCKED) {
						//stable
						if (VIDEO_LOCKED == chip_info[chip].state) {
							if (chip_info[chip].count < MAX_COUNT) chip_info[chip].count++;
						} else {
							chip_info[chip].state = VIDEO_LOCKED;
							chip_info[chip].count = 0;
							vfe_dev_dbg("*** video locked\n");
						}
					} else {
						//video in
						if (VIDEO_UNPLUG == chip_info[chip].state) {
							chip_info[chip].state = VIDEO_IN;
							chip_info[chip].count = 0;
							vfe_dev_dbg("video in\n");
						} else if (VIDEO_LOCKED == chip_info[chip].state) {
							chip_info[chip].state = VIDEO_UNLOCK;
							chip_info[chip].count = 0;
							vfe_dev_dbg("video unstable\n");
						} else {
							if (chip_info[chip].count < MAX_COUNT) chip_info[chip].count++;
							if (VIDEO_UNLOCK == chip_info[chip].state && chip_info[chip].count > 2) {
								chip_info[chip].state = VIDEO_IN;
								chip_info[chip].count = 0;
								ret = sensor_reset_default(glb_sd);
								vfe_dev_dbg("video unlocked -> video in\n");
							}
						}
					}
				}

				if ( VIDEO_IN == chip_info[chip].state) {
					//video-in detected.
					uint8_t v2;
					ret = sensor_read(glb_sd, 0x03, &cvstd);
					if (ret) break;
					v2 = ((cvstd & 0x8) != 0) ? 1 : 0;
					cvstd &= 0x7;
					vfe_dev_dbg("video in > tvi ver.%d, format = %d\n", (v2) ? 2 : 1, cvstd);

					//if(chip_info[chip].std != cvstd) {
						chip_info[chip].std = cvstd;

						if (cvstd == R720P60) {
							vfe_dev_dbg("R720P60\n");
							ret = sensor_write_array(glb_sd, sensor_720p60_regs, ARRAY_SIZE(sensor_720p60_regs));
						} else if(cvstd == R720P50) {
							vfe_dev_dbg("R720P50\n");
							ret = sensor_write_array(glb_sd, sensor_720p50_regs, ARRAY_SIZE(sensor_720p50_regs));
						} else if(cvstd == R1080P30) {
							vfe_dev_dbg("R1080P30\n");
							ret = sensor_write_array(glb_sd, sensor_1080p30_regs, ARRAY_SIZE(sensor_1080p30_regs));
						} else if(cvstd == R1080P25) {
							vfe_dev_dbg("R1080P25\n");
							ret = sensor_write_array(glb_sd, sensor_1080p25_regs, ARRAY_SIZE(sensor_1080p25_regs));
						} else if(cvstd == R720P30) {
							vfe_dev_dbg("R720P30\n");
							ret = sensor_write_array(glb_sd, sensor_720p30_regs, ARRAY_SIZE(sensor_720p30_regs));
						} else if(cvstd == R720P25) {
							vfe_dev_dbg("R720P25\n");
							ret = sensor_write_array(glb_sd, sensor_720p25_regs, ARRAY_SIZE(sensor_720p25_regs));
						} else {
							ret = -1;
							vfe_dev_dbg("Unknown format\n");
						}

						if (ret) continue;

						if (v2) ret = sensor_write_array(glb_sd, sensor_v2_regs, ARRAY_SIZE(sensor_v2_regs));
						else ret = sensor_write_array(glb_sd, sensor_v1_regs, ARRAY_SIZE(sensor_v1_regs));

						if (v2) ret = sensor_sysclk_v2(glb_sd);
						else ret = sensor_sysclk_v1(glb_sd);
						if (ret) continue;

						vfe_dev_dbg("tvi ver.%d, format = %d, result=%d\n", (v2) ? 2 : 1, cvstd, ret);
					//}
				}

				if ( VIDEO_LOCKED == chip_info[chip].state) {
					uint8_t v2;
					ret = sensor_read(glb_sd, 0x03, &cvstd);
					if (ret) break;
					v2 = ((cvstd & 0x8) != 0) ? 1 : 0;
					cvstd &= 0x7;
					vfe_dev_dbg("video locked > tvi ver.%d, format = %d\n", (v2) ? 2 : 1, cvstd);

					if (chip_info[chip].count == 0) {

						sensor_read(glb_sd, 0x4e, &tmp);
						tmp &= 0xcf; //clear clock MD bits
						if ( tmp & 0x04)
						{
							sensor_write(glb_sd, 0x4e, tmp | 0x10);
						}
						else
						{
							int half_scaler = 1;
							if (half_scaler) //down scaler output
							{
								sensor_write(glb_sd, 0x35, 0x45);
								sensor_write(glb_sd, 0x4e, tmp | 0x10);
							}
							else
							{
								sensor_write(glb_sd, 0x4e, tmp);
							}
						}
					} else if (chip_info[chip].count == 1) {

					}
				}


			} while (0);
		}

		do_gettimeofday(&end);

		interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);


		spin_unlock_irq(&watchdog_lock);

		/* sleep 0.5 seconds */
		schedule_timeout_interruptible(msecs_to_jiffies(500) + 1);
	}

	set_current_state(TASK_RUNNING);

	vfe_dev_dbg("tp2825_watchdog_deamon: exit!\n");

	return 0;

}

static int tp2825_watchdog_init(void)
{
	struct task_struct *p_dog;
	int i, j;

	watchdog_state = WATCHDOG_RUNNING;

	p_dog = kthread_create(tp2825_watchdog_deamon, NULL, "tp2825_wdog");

	if ( IS_ERR(p_dog) < 0) {
		vfe_dev_err("tp2825_watchdog_init: create watchdog_deamon failed!\n");
		return -1;
	}

	wake_up_process(p_dog);

	task_watchdog_deamon = p_dog;

	vfe_dev_dbg("tp2825_watchdog_init: done!\n");

	return 0;
}

static void tp2825_watchdog_exit(void)
{
	struct task_struct *p_dog = task_watchdog_deamon;
	watchdog_state = WATCHDOG_EXIT;

	if ( p_dog == NULL )
		return;

	wake_up_process(p_dog);

	kthread_stop(p_dog);

	yield();

	task_watchdog_deamon = NULL;

	vfe_dev_dbg("tp2825_watchdog_exit: done!\n");
}

module_init(init_sensor);
module_exit(exit_sensor);

