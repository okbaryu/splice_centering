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
MODULE_DESCRIPTION("A low-level driver for TP2825 video decoder");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN	1

#if(DEV_DBG_EN == 1)
#define vfe_dev_dbg(x,arg...) printk("[TP2825]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...)
#endif
#define vfe_dev_err(x,arg...) printk("[TP2825]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[TP2825]"x,##arg)

#define LOG_ERR_RET(x)  { \
		int __ret;  \
		__ret = x; \
		if(__ret < 0) {\
			vfe_dev_err("error at %s:%d\n",__func__, __LINE__);  \
			return __ret; \
		} \
	}

#define LOG_CHECK_RET(x) { \
		int __ret; \
		__ret = x; \
		if(__ret < 0) {\
			vfe_dev_err("error at %s:%d\n",__func__, __LINE__);  \
		} \
	}


enum {
	CH_1 = 0,
	CH_2,
	CH_3,
	CH_4,
	CH_ALL = 4,
	DATA_PAGE = 5,
	AUDIO_PAGE = 9
};

enum {
	SCAN_DISABLE = 0,
	SCAN_AUTO,
	SCAN_TVI,
	SCAN_HDA,
	SCAN_HDC,
	SCAN_MANUAL,
	SCAN_TEST
};

enum {
	STD_TVI = 0,
	STD_HDA, //AHD
	STD_HDC,
	STD_HDA_DEFAULT,
	STD_HDC_DEFAULT
};

enum {
	PTZ_TVI = 0,
	PTZ_HDA = 1,
	PTZ_HDC = 4
};

#define MAX_CHIPS			1
#define MAX_COUNT 			0xffff
#define CHANNELS_PER_CHIP	1

enum {
	VIDEO_UNPLUG = 0,
	VIDEO_IN,
	VIDEO_LOCKED,
	VIDEO_UNLOCK
};

enum {
	MUX656_8BIT = 0,    //Y/C-mux 4:2:2 8-bit with embedded sync
	SEP656_8BIT,    //Y/C-mux 4:2:2 8-bit with seperate sync
	EMB422_16BIT,   //YCbCr 4:2:2 16-bit with embedded sync
	SEP422_16BIT     //YCbCr 4:2:2 10-bit with embedded sync
};

static struct _chip_info {
	int state[CHANNELS_PER_CHIP];
	int count[CHANNELS_PER_CHIP];
	int mode[CHANNELS_PER_CHIP];
	int scan[CHANNELS_PER_CHIP];
	int gain[CHANNELS_PER_CHIP][4];
	int force[CHANNELS_PER_CHIP];
	int std[CHANNELS_PER_CHIP];
} chip_info[MAX_CHIPS];


/*
#define FLAG_LOSS		0x80
#define FLAG_LOCKED		0x60
#define FLAG_DETECTED		0x08
#define FLAG_PROGRESSIVE	0x02
*/

#define FLAG_LOSS			0x80
#define FLAG_H_LOCKED		0x20
#define FLAG_HV_LOCKED		0x60

#define FLAG_HDC_MODE	0x80
#define FLAG_HALF_MODE	0x40
#define FLAG_MEGA_MODE	0x20
#define FLAG_HDA_MODE	0x10


#if 1
enum {
	SENSOR_1080P25 =	    0x03,
	SENSOR_1080P30 =	    0x02,
	SENSOR_720P25  =	    0x05,
	SENSOR_720P30  =    	0x04,
	SENSOR_720P50  =	    0x01,
	SENSOR_720P60  =    	0x00,
	SENSOR_SD      =        0x06,
	INVALID_FORMAT =		0x07,
	SENSOR_720P25V2 =	    0x0D,
	SENSOR_720P30V2 =		0x0C,
	SENSOR_PAL	   =        0x08,
	SENSOR_NTSC	   =    	0x09,
	SENSOR_HALF1080P25  =	0x43,
	SENSOR_HALF1080P30  =	0x42,
	SENSOR_HALF720P25   =	0x45,
	SENSOR_HALF720P30   =   0x44,
	SENSOR_HALF720P50   =	0x41,
	SENSOR_HALF720P60   =   0x40
};
#else
enum {
	SENSOR_720P60 = 0,
	SENSOR_720P50,
	SENSOR_1080P30,
	SENSOR_1080P25,
	SENSOR_720P30,
	SENSOR_720P25,
	SENSOR_SD,
	INVALID_FORMAT,
	SENSOR_720P30V2,
	SENSOR_720P25V2,
	SENSOR_NTSC,
	SENSOR_PAL,

	SENSOR_HALF1080P25  =	0x43,
	SENSOR_HALF1080P30  =	0x42,
	SENSOR_HALF720P25   =	0x45,
	SENSOR_HALF720P30   =   0x44,
	SENSOR_HALF720P50   =	0x41,
	SENSOR_HALF720P60   =   0x40
};

#endif

static const char *get_cvstd_name(uint8_t cvstd)
{
	switch(cvstd) {
	case SENSOR_1080P25: return "1080P25";
	case SENSOR_1080P30: return "1080P30";
	case SENSOR_720P25: return "720P25";
	case SENSOR_720P30: return "720P30";
	case SENSOR_720P50: return "720P50";
	case SENSOR_720P60: return "720P60";
	case SENSOR_SD: return "SENSOR_SD";
	case INVALID_FORMAT: return "Invalid";
	case SENSOR_720P25V2: return "720P25V2";
	case SENSOR_720P30V2: return "720P30V2";
	case SENSOR_PAL: return "PAL";
	case SENSOR_NTSC: return "NTSC";
	case SENSOR_HALF1080P25: return "HALF1080P25";
	case SENSOR_HALF1080P30: return "HALF1080P30";
	case SENSOR_HALF720P25: return "HALF720P25";
	case SENSOR_HALF720P30: return "HALF720P30";
	case SENSOR_HALF720P50: return "HALF720P50";
	case SENSOR_HALF720P60: return "HALF720P60";
	default: 
	{
		uint8_t v2 = ((cvstd&0x8) != 0);
		if(v2) {
			return get_cvstd_name(cvstd&0x7);
		} else {
			return "Unknown";
		}
	}
	}
}


static const char *get_std_name(uint8_t std)
{
	switch(std) {
	case STD_TVI: return "TVI";
	case STD_HDA: return "HDA";
	case STD_HDC: return "HDC";
	case STD_HDA_DEFAULT: return "HDA_DEFAULT";
	case STD_HDC_DEFAULT: return "HDC_DEFAULT";
	default: return "Undefined";
	}
}


//module parameters
static int HDC_enable = 1;
static int init_mode = SENSOR_720P30V2;
static int init_std = STD_TVI;
static int chips = 1;
static int output[MAX_CHIPS] = { MUX656_8BIT };

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



// Video Format
static struct regval_list sensor_1080p25_raster[] =
{
	// Start address 0x15, Size = 9B
	//0x03, 0xD3, 0x80, 0x29, 0x38, 0x47, 0x00, 0x0A, 0x50
	{ 0x15, 0x03 },
	{ 0x16, 0xD3 },
	{ 0x17, 0x80 },
	{ 0x18, 0x29 },
	{ 0x19, 0x38 },
	{ 0x1a, 0x48 },
	{ 0x1b, 0x00 },
	{ 0x1c, 0x0A },
	{ 0x1d, 0x50 }
};

static struct regval_list sensor_1080p30_raster[] =
{
	// Start address 0x15, Size = 9B
	{ 0x15, 0x03 },
	{ 0x16, 0xD3 },
	{ 0x17, 0x80 },
	{ 0x18, 0x29 },
	{ 0x19, 0x38 },
	{ 0x1a, 0x47 },
	{ 0x1b, 0x00 },
	{ 0x1c, 0x08 },
	{ 0x1d, 0x98 }
};

static struct regval_list sensor_720p25_raster[] =
{
	// Start address 0x15, Size = 9B
	{ 0x15, 0x13 },
	{ 0x16, 0x16 },
	{ 0x17, 0x00 },
	{ 0x18, 0x19 },
	{ 0x19, 0xD0 },
	{ 0x1a, 0x25 },
	{ 0x1b, 0x00 },
	{ 0x1c, 0x0F },
	{ 0x1d, 0x78 }
};

static struct regval_list sensor_720p30_raster[] = {
	// Start address 0x15, Size = 9B
	{ 0x15, 0x13 },
	{ 0x16, 0x16 },
	{ 0x17, 0x00 },
	{ 0x18, 0x19 },
	{ 0x19, 0xD0 },
	{ 0x1a, 0x25 },
	{ 0x1b, 0x00 },
	{ 0x1c, 0x0C },
	{ 0x1d, 0xE4 }
};

static struct regval_list sensor_720p50_raster[] =
{
	// Start address 0x15, Size = 9B
	{ 0x15, 0x13 },
	{ 0x16, 0x16 },
	{ 0x17, 0x00 },
	{ 0x18, 0x19 },
	{ 0x19, 0xD0 },
	{ 0x1a, 0x25 },
	{ 0x1b, 0x00 },
	{ 0x1c, 0x07 },
	{ 0x1d, 0xBC }
};

static struct regval_list sensor_720p60_raster[] =
{
	// Start address 0x15, Size = 9B
	{ 0x15, 0x13 },
	{ 0x16, 0x16 },
	{ 0x17, 0x00 },
	{ 0x18, 0x19 },
	{ 0x19, 0xD0 },
	{ 0x1a, 0x25 },
	{ 0x1b, 0x00 },
	{ 0x1c, 0x06 },
	{ 0x1d, 0x72 }
};

static struct regval_list sensor_PAL_raster[] = {
	// Start address 0x15, Size = 9B
	{ 0x15, 0x13 },
	{ 0x16, 0x5f },
	{ 0x17, 0xbc },
	{ 0x18, 0x17 },
	{ 0x19, 0x20 },
	{ 0x1a, 0x17 },
	{ 0x1b, 0x00 },
	{ 0x1c, 0x09 },
	{ 0x1d, 0x48 }
};

static struct regval_list sensor_NTSC_raster[] =
{
	// Start address 0x15, Size = 9B
	{ 0x15, 0x13 },
	{ 0x16, 0x4e },
	{ 0x17, 0xbc },
	{ 0x18, 0x15 },
	{ 0x19, 0xf0 },
	{ 0x1a, 0x07 },
	{ 0x1b, 0x00 },
	{ 0x1c, 0x09 },
	{ 0x1d, 0x38 }
};

#if 0
static struct regval_list sensor_common_pll_regs[] =
{
	// Start address 0x42, Size = 4B
	{ 0x42, 0x00 }, 
	{ 0x43, 0x12 }, 
	{ 0x44, 0x07 }, 
	{ 0x45, 0x49 }
};



static struct regval_list sensor_common_oe_enable_regs[] =
{
	// Start address 0x4B, Size = 11B
	{ 0x4b, 0x10 },
	{ 0x4c, 0x32 }, 
	{ 0x4d, 0x0F }, 
	{ 0x4e, 0xFF }, 
	{ 0x4f, 0x0F }, 
	{ 0x50, 0x00 }, 
	{ 0x51, 0x0A }, 
	{ 0x52, 0x0B }, 
	{ 0x53, 0x1F }, 
	{ 0x54, 0xFA }, 
	{ 0x55, 0x27 }
};

static struct regval_list sensor_common_rx_regs[] =
{
	// Start address 0x7E, Size = 13B
	0x2F, 0x00, 0x07, 0x08, 0x04, 0x00, 0x00, 0x60, 0x10,
	0x06, 0xBE, 0x39, 0xA7
};
#endif

// PLLs
unsigned char tbl_tp2802_common_pll[] = {
		
		
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
static int sensor_read(struct v4l2_subdev *sd, unsigned char reg, unsigned char *value)
{
	int ret = 0;
	int cnt = 0;

	ret = cci_read_a8_d8(sd, reg, value);
	while (ret != 0 && cnt < 3) {
		ret = cci_read_a8_d8(sd, reg, value);
		cnt++;
	}

	if (cnt > 0)
		vfe_dev_dbg("sensor read retry=%d\n", cnt);

	return ret;
}

static int reg_val_show(struct v4l2_subdev *sd, unsigned char reg)
{
	uint8_t tmp;
	sensor_read(sd, reg, &tmp);
	printk("0x%x value is 0x%x\n", reg, tmp);
	return 0;
}

static int sensor_write(struct v4l2_subdev *sd, unsigned char reg, unsigned char value)
{
	int ret = 0;
	int cnt = 0;

	ret = cci_write_a8_d8(sd, reg, value);
	while (ret != 0 && cnt < 2) {
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

static int sensor_set_reg_page(struct v4l2_subdev *sd, unsigned char ch)
{
	switch (ch)
	{
	case CH_1:
		LOG_ERR_RET(sensor_write(sd, 0x40, 0x00));
		break;  // VIN1 registers
	case CH_2:
		LOG_ERR_RET(sensor_write(sd, 0x40, 0x01));
		break;  // VIN2 registers
	case CH_3:
		LOG_ERR_RET(sensor_write(sd, 0x40, 0x02));
		break;  // VIN3 registers
	case CH_4:
		LOG_ERR_RET(sensor_write(sd, 0x40, 0x03));
		break;  // VIN4 registers
	case CH_ALL:
		LOG_ERR_RET(sensor_write(sd, 0x40, 0x04));
		break;  // Write All VIN1-4 regsiters
	case AUDIO_PAGE:
		LOG_ERR_RET(sensor_write(sd, 0x40, 0x40));
		break;  // Audio
	case DATA_PAGE:
		LOG_ERR_RET(sensor_write(sd, 0x40, 0x10));
		break;  // PTZ data
	default:
		LOG_ERR_RET(sensor_write(sd, 0x40, 0x04));
		break;
	}

	return 0;
}


static int sensor_reset_default(struct v4l2_subdev *sd)
{
	uint8_t tmp;

	LOG_ERR_RET(sensor_write(sd, 0x26, 0x01));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0xC0));
	LOG_ERR_RET(sensor_write(sd, 0x0B, 0xC0));
	LOG_ERR_RET(sensor_write(sd, 0x22, 0x35));

	LOG_ERR_RET(sensor_read(sd, 0x06, &tmp));
	tmp &= 0xfb;
	LOG_ERR_RET(sensor_write(sd, 0x06, tmp));

	return 0;
}

static int sensor_ntsc_setup(struct v4l2_subdev *sd)
{
	uint8_t tmp;
	LOG_ERR_RET(sensor_write(sd, 0x0c, 0x43));
	LOG_ERR_RET(sensor_write(sd, 0x0d, 0x10));
	LOG_ERR_RET(sensor_write(sd, 0x20, 0xa0));
	LOG_ERR_RET(sensor_write(sd, 0x26, 0x12));
	LOG_ERR_RET(sensor_write(sd, 0x2b, 0x50));
	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x68));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x5e));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x62));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0xbb));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x96));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0xc0));
	LOG_ERR_RET(sensor_write(sd, 0x35, 0x25));
	LOG_ERR_RET(sensor_write(sd, 0x39, 0x84));
	LOG_ERR_RET(sensor_write(sd, 0x2c, 0x2a));

	LOG_ERR_RET(sensor_write(sd, 0x27, 0x2d));
	LOG_ERR_RET(sensor_write(sd, 0x28, 0x00));

	LOG_ERR_RET(sensor_write(sd, 0x13, 0x00));
	LOG_ERR_RET(sensor_read(sd, 0x14, &tmp));
	tmp &= 0x9f;
	LOG_ERR_RET(sensor_write(sd, 0x14, tmp));

	return 0;
}

static int sensor_pal_setup(struct v4l2_subdev *sd)
{
	uint8_t tmp;
	LOG_ERR_RET(sensor_write(sd, 0x0c, 0x53));
	LOG_ERR_RET(sensor_write(sd, 0x0d, 0x11));
	LOG_ERR_RET(sensor_write(sd, 0x20, 0xb0));
	LOG_ERR_RET(sensor_write(sd, 0x26, 0x02));
	LOG_ERR_RET(sensor_write(sd, 0x2b, 0x50));
	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x5e));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x7a));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x4a));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x4d));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0xf0));
	LOG_ERR_RET(sensor_write(sd, 0x35, 0x25));
	LOG_ERR_RET(sensor_write(sd, 0x39, 0x84));
	LOG_ERR_RET(sensor_write(sd, 0x2c, 0x2a));

	LOG_ERR_RET(sensor_write(sd, 0x27, 0x2d));
	LOG_ERR_RET(sensor_write(sd, 0x28, 0x00));

	LOG_ERR_RET(sensor_write(sd, 0x13, 0x00));
	LOG_ERR_RET(sensor_read(sd, 0x14, &tmp));
	tmp &= 0x9f;
	LOG_ERR_RET(sensor_write(sd, 0x14, tmp));

	return 0;
}

static int sensor_v1_setup(struct v4l2_subdev *sd)
{
	uint8_t tmp;
	LOG_ERR_RET(sensor_write(sd, 0x0c, 0x03));
	LOG_ERR_RET(sensor_write(sd, 0x0d, 0x10));
	LOG_ERR_RET(sensor_write(sd, 0x20, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x26, 0x02));
	LOG_ERR_RET(sensor_write(sd, 0x2b, 0x58));
	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x30));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x70));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x48));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0xbb));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x2e));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0x90));
	LOG_ERR_RET(sensor_write(sd, 0x35, 0x05));
	LOG_ERR_RET(sensor_write(sd, 0x39, 0x8C));
	LOG_ERR_RET(sensor_write(sd, 0x2c, 0x0a));

	LOG_ERR_RET(sensor_write(sd, 0x27, 0x2d));
	LOG_ERR_RET(sensor_write(sd, 0x28, 0x00));

	LOG_ERR_RET(sensor_write(sd, 0x13, 0x00));
	LOG_ERR_RET(sensor_read(sd, 0x14, &tmp));
	tmp &= 0x9f;
	LOG_ERR_RET(sensor_write(sd, 0x14, tmp));

	return 0;
}

static int sensor_v2_setup(struct v4l2_subdev *sd)
{
	uint8_t tmp;
	LOG_ERR_RET(sensor_write(sd, 0x0c, 0x03));
	LOG_ERR_RET(sensor_write(sd, 0x0d, 0x10));
	LOG_ERR_RET(sensor_write(sd, 0x20, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x26, 0x02));
	LOG_ERR_RET(sensor_write(sd, 0x2b, 0x58));
	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x30));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x70));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x48));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0xbb));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x2e));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0x90));
	LOG_ERR_RET(sensor_write(sd, 0x35, 0x25));
	LOG_ERR_RET(sensor_write(sd, 0x39, 0x88));
	LOG_ERR_RET(sensor_write(sd, 0x2c, 0x0a));

	LOG_ERR_RET(sensor_write(sd, 0x27, 0x2d));
	LOG_ERR_RET(sensor_write(sd, 0x28, 0x00));

	LOG_ERR_RET(sensor_write(sd, 0x13, 0x00));
	LOG_ERR_RET(sensor_read(sd, 0x14, &tmp));
	tmp &= 0x9f;
	LOG_ERR_RET(sensor_write(sd, 0x14, tmp));

	return 0;
}

static int sensor_a720p30_setup(struct v4l2_subdev *sd)
{
	uint8_t tmp;
	LOG_ERR_RET(sensor_read(sd, 0x14, &tmp));
	tmp |= 0x40;
	LOG_ERR_RET(sensor_write(sd, 0x14, tmp));

	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x48));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x5e));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x27));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x72));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x80));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0x77));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0x80));

	return 0;

}

static int sensor_a720p25_setup(struct v4l2_subdev *sd)
{
	uint8_t tmp;
	LOG_ERR_RET(sensor_read(sd, 0x14, &tmp));
	tmp |= 0x40;
	LOG_ERR_RET(sensor_write(sd, 0x14, tmp));

	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x48));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x5e));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x27));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x88));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x04));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0x23));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0x80));

	return 0;

}

static int sensor_a1080p30_setup(struct v4l2_subdev *sd)
{
	uint8_t tmp;
	//LOG_ERR_RET(sensor_write(sd, 0x14, 0x60);
	LOG_ERR_RET(sensor_read(sd, 0x14, &tmp));
	tmp |= 0x60;
	LOG_ERR_RET(sensor_write(sd, 0x14, tmp));

	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x45));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x50));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x29));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x65));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x78));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0x16));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0x80));

	return 0;

}

static int sensor_a1080p25_setup(struct v4l2_subdev *sd)
{
	uint8_t tmp;
	//LOG_ERR_RET(sensor_write(sd, 0x14, 0x60));
	LOG_ERR_RET(sensor_read(sd, 0x14, &tmp));
	tmp |= 0x60;
	LOG_ERR_RET(sensor_write(sd, 0x14, tmp));

	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x45));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x40));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x29));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x61));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x78));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0x16));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0x80));

	return 0;
}

static int sensor_c1080p25_setup(struct v4l2_subdev *sd)
{

	LOG_ERR_RET(sensor_write(sd, 0x13, 0x40));
	//LOG_ERR_RET(sensor_write(sd, 0x15, 0x13));
	//LOG_ERR_RET(sensor_write(sd, 0x16, 0x84));

	//LOG_ERR_RET(sensor_write(sd, 0x20, 0x50));
	LOG_ERR_RET(sensor_write(sd, 0x20, 0xa0));
	LOG_ERR_RET(sensor_write(sd, 0x2b, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x54));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x40));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x41));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x82));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x27));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0xa6));

	LOG_ERR_RET(sensor_write(sd, 0x28, 0x04));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0x80));
	LOG_ERR_RET(sensor_write(sd, 0x27, 0x5a));

	return 0;
}

static int sensor_c720p25_setup(struct v4l2_subdev *sd)
{

	LOG_ERR_RET(sensor_write(sd, 0x13, 0x40));
	//LOG_ERR_RET(sensor_write(sd, 0x16, 0x40));

	//LOG_ERR_RET(sensor_write(sd, 0x20, 0x3a));
	LOG_ERR_RET(sensor_write(sd, 0x20, 0x74));
	LOG_ERR_RET(sensor_write(sd, 0x2b, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x42));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x40));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x48));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x67));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x6f));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0x31));

	LOG_ERR_RET(sensor_write(sd, 0x28, 0x04));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0x80));
	LOG_ERR_RET(sensor_write(sd, 0x27, 0x5a));

	return 0;
}

static int sensor_c720p50_setup(struct v4l2_subdev *sd)
{

	LOG_ERR_RET(sensor_write(sd, 0x13, 0x40));
	//LOG_ERR_RET(sensor_write(sd, 0x16, 0x40));

	//LOG_ERR_RET(sensor_write(sd, 0x20, 0x3a));
	LOG_ERR_RET(sensor_write(sd, 0x20, 0x74));
	LOG_ERR_RET(sensor_write(sd, 0x2b, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x42));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x40));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x41));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x82));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x27));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0xa6));

	LOG_ERR_RET(sensor_write(sd, 0x28, 0x04));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0x80));
	LOG_ERR_RET(sensor_write(sd, 0x27, 0x5a));

	return 0;
}

static int sensor_c1080p30_setup(struct v4l2_subdev *sd)
{

	LOG_ERR_RET(sensor_write(sd, 0x13, 0x40));

	//LOG_ERR_RET(sensor_write(sd, 0x15, 0x13));
	//LOG_ERR_RET(sensor_write(sd, 0x16, 0x44));

	//LOG_ERR_RET(sensor_write(sd, 0x20, 0x40));
	LOG_ERR_RET(sensor_write(sd, 0x20, 0x80));
	LOG_ERR_RET(sensor_write(sd, 0x2b, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x47));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x40));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x41));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x82));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x27));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0xa6));

	LOG_ERR_RET(sensor_write(sd, 0x28, 0x04));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0x80));
	LOG_ERR_RET(sensor_write(sd, 0x27, 0x5a));

	return 0;
}

static int sensor_c720p30_setup(struct v4l2_subdev *sd)
{

	LOG_ERR_RET(sensor_write(sd, 0x13, 0x40));
	//LOG_ERR_RET(sensor_write(sd, 0x16, 0x02));

	//LOG_ERR_RET(sensor_write(sd, 0x20, 0x30));
	LOG_ERR_RET(sensor_write(sd, 0x20, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x2b, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x37));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x40));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x48));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x67));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x6f));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0x31));

	LOG_ERR_RET(sensor_write(sd, 0x28, 0x04));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0x80));
	LOG_ERR_RET(sensor_write(sd, 0x27, 0x5a));

	return 0;
}

static int sensor_c720p60_setup(struct v4l2_subdev *sd)
{

	LOG_ERR_RET(sensor_write(sd, 0x13, 0x40));
	//LOG_ERR_RET(sensor_write(sd, 0x16, 0x02));

	//LOG_ERR_RET(sensor_write(sd, 0x20, 0x30));
	LOG_ERR_RET(sensor_write(sd, 0x20, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x2b, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0x2d, 0x37));
	LOG_ERR_RET(sensor_write(sd, 0x2e, 0x40));

	LOG_ERR_RET(sensor_write(sd, 0x30, 0x41));
	LOG_ERR_RET(sensor_write(sd, 0x31, 0x82));
	LOG_ERR_RET(sensor_write(sd, 0x32, 0x27));
	LOG_ERR_RET(sensor_write(sd, 0x33, 0xa6));

	LOG_ERR_RET(sensor_write(sd, 0x28, 0x04));
	LOG_ERR_RET(sensor_write(sd, 0x07, 0x80));
	LOG_ERR_RET(sensor_write(sd, 0x27, 0x5a));

	return 0;
}


static int sensor_set_video_mode(struct v4l2_subdev *sd, int chip, unsigned char mode, unsigned char ch, unsigned char std)
{
	int err = 0;
	uint8_t tmp;

	if (STD_HDA_DEFAULT == std) {
		std = STD_HDA;
	}

	vfe_dev_dbg("sensor_set_video_mode: mode %02x(%s), std %02x(%s)\n", mode, get_cvstd_name(mode), std, get_std_name(std));

	switch (mode) {
	case SENSOR_HALF1080P25:
	case SENSOR_1080P25:
		LOG_ERR_RET(sensor_write_array(sd, sensor_1080p25_raster, ARRAY_SIZE(sensor_1080p25_raster)));
		LOG_ERR_RET(sensor_write(sd, 0x02, 0xC8));
		LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
		tmp &= 0xFB;
		LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		LOG_ERR_RET(sensor_v1_setup(sd));

		if ( STD_HDA == std) {
			LOG_ERR_RET(sensor_a1080p25_setup(sd));
		} else if (STD_HDC == std || STD_HDC_DEFAULT == std) {
			LOG_ERR_RET(sensor_c1080p25_setup(sd));
			if (STD_HDC == std) { //HDC 1080p25 position adjust
				LOG_ERR_RET(sensor_write(sd, 0x15, 0x13));
				LOG_ERR_RET(sensor_write(sd, 0x16, 0x84));
			}
		}
		break;

	case SENSOR_HALF1080P30:
	case SENSOR_1080P30:
		LOG_ERR_RET(sensor_write_array(sd, sensor_1080p30_raster, ARRAY_SIZE(sensor_1080p30_raster)));
		LOG_ERR_RET(sensor_write(sd, 0x02, 0xC8));
		LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
		tmp &= 0xFB;
		LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		LOG_ERR_RET(sensor_v1_setup(sd));

		if ( STD_HDA == std) {
			LOG_ERR_RET(sensor_a1080p30_setup(sd));
		} else if (STD_HDC == std || STD_HDC_DEFAULT == std) {
			LOG_ERR_RET(sensor_c1080p30_setup(sd));
			if (STD_HDC == std) { //HDC 1080p30 position adjust
				LOG_ERR_RET(sensor_write(sd, 0x15, 0x13));
				LOG_ERR_RET(sensor_write(sd, 0x16, 0x44));
			}
		}
		break;

	case SENSOR_HALF720P25:
	case SENSOR_720P25:
		LOG_ERR_RET(sensor_write_array(sd, sensor_720p25_raster, ARRAY_SIZE(sensor_720p25_raster)));
		LOG_ERR_RET(sensor_write(sd, 0x02, 0xCA));
		LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
		tmp &= 0xFB;
		LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		LOG_ERR_RET(sensor_v1_setup(sd));

		break;

	case SENSOR_HALF720P30:
	case SENSOR_720P30:
		LOG_ERR_RET(sensor_write_array(sd, sensor_720p30_raster, ARRAY_SIZE(sensor_720p30_raster)));
		LOG_ERR_RET(sensor_write(sd, 0x02, 0xCA));
		LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
		tmp &= 0xFB;
		LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		LOG_ERR_RET(sensor_v1_setup(sd));

		break;

	case SENSOR_HALF720P50:
	case SENSOR_720P50:
		LOG_ERR_RET(sensor_write_array(sd, sensor_720p50_raster, ARRAY_SIZE(sensor_720p50_raster)));
		LOG_ERR_RET(sensor_write(sd, 0x02, 0xCA));
		LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
		tmp &= 0xFB;
		LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		LOG_ERR_RET(sensor_v1_setup(sd));

		if (STD_HDA == std) {

		} else if (STD_HDC == std || STD_HDC_DEFAULT == std) {
			LOG_ERR_RET(sensor_c720p50_setup(sd));
			if (STD_HDC == std) { //HDC 720p50 position adjust
				LOG_ERR_RET(sensor_write(sd, 0x16, 0x40));
			}
		}
		break;

	case SENSOR_HALF720P60:
	case SENSOR_720P60:
		LOG_ERR_RET(sensor_write_array(sd, sensor_720p60_raster, ARRAY_SIZE(sensor_720p60_raster)));
		LOG_ERR_RET(sensor_write(sd, 0x02, 0xCA));
		LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
		tmp &= 0xFB;
		LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		LOG_ERR_RET(sensor_v1_setup(sd));

		if (STD_HDA == std) {

		} else if (STD_HDC == std || STD_HDC_DEFAULT == std) {
			LOG_ERR_RET(sensor_c720p60_setup(sd));

			if (STD_HDC == std) { //HDC 720p60 position adjust
				LOG_ERR_RET(sensor_write(sd, 0x16, 0x02));
			}
		}
		break;

	case SENSOR_720P30V2:
		LOG_ERR_RET(sensor_write_array(sd, sensor_720p60_raster, ARRAY_SIZE(sensor_720p60_raster)));
		LOG_ERR_RET(sensor_write(sd, 0x02, 0xCA));
		LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
		tmp |= 0x04;
		LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		LOG_ERR_RET(sensor_v2_setup(sd));

		if (STD_HDA == std) {
			LOG_ERR_RET(sensor_a720p30_setup(sd));
		} else if (STD_HDC == std || STD_HDC_DEFAULT == std) {
			LOG_ERR_RET(sensor_c720p30_setup(sd));
			if (STD_HDC == std) { //HDC 720p30 position adjust
				LOG_ERR_RET(sensor_write(sd, 0x16, 0x02));
			}
		}
		break;

	case SENSOR_720P25V2:
		LOG_ERR_RET(sensor_write_array(sd, sensor_720p50_raster, ARRAY_SIZE(sensor_720p50_raster)));
		LOG_ERR_RET(sensor_write(sd, 0x02, 0xCA));
		LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
		tmp |= 0x04;
		LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		LOG_ERR_RET(sensor_v2_setup(sd));

		if (STD_HDA == std) {
			LOG_ERR_RET(sensor_a720p25_setup(sd));
		} else if (STD_HDC == std || STD_HDC_DEFAULT == std) {
			LOG_ERR_RET(sensor_c720p25_setup(sd));

			if (STD_HDC == std) { //HDC 720p25 position adjust
				LOG_ERR_RET(sensor_write(sd, 0x16, 0x40));
			}
		}
		break;

	case SENSOR_PAL:
		LOG_ERR_RET(sensor_write_array(sd, sensor_PAL_raster, ARRAY_SIZE(sensor_PAL_raster)));
		LOG_ERR_RET(sensor_write(sd, 0x02, 0xCF));
		LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
		tmp |= 0x04;
		LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		LOG_ERR_RET(sensor_pal_setup(sd));
		break;

	case SENSOR_NTSC:
		LOG_ERR_RET(sensor_write_array(sd, sensor_NTSC_raster, ARRAY_SIZE(sensor_NTSC_raster)));
		LOG_ERR_RET(sensor_write(sd, 0x02, 0xCF));
		LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
		tmp |= 0x04;
		LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		LOG_ERR_RET(sensor_ntsc_setup(sd));
		break;

	default:
		err = -1;
		break;
	}

	if (mode & FLAG_HALF_MODE) {
		LOG_ERR_RET(sensor_read(sd, 0x35, &tmp));
		tmp |= FLAG_HALF_MODE;
		LOG_ERR_RET(sensor_write(sd, 0x35, tmp));
	}

	if (MUX656_8BIT == output[chip]) {
		if (SENSOR_PAL == mode || SENSOR_NTSC == mode ) {
			LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
			tmp &= 0xcf;
			tmp |= 0x30;
			LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
			LOG_ERR_RET(sensor_read(sd, 0x02, &tmp));
			tmp &= 0x7f;
			LOG_ERR_RET(sensor_write(sd, 0x02, tmp));
			LOG_ERR_RET(sensor_read(sd, 0x35, &tmp));
			tmp |= FLAG_HALF_MODE;
			LOG_ERR_RET(sensor_write(sd, 0x35, tmp));
		} else if (SENSOR_720P25V2 == mode || SENSOR_720P30V2 == mode ) {
			LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
			tmp &= 0xcf;
			tmp |= 0x10;
			LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		} else if (FLAG_HALF_MODE == (mode & FLAG_HALF_MODE) ) {
			LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
			tmp &= 0xcf;
			tmp |= 0x10;
			LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		} else {
			LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp));
			tmp &= 0xcf;
			LOG_ERR_RET(sensor_write(sd, 0x4E, tmp));
		}
	} else { //16bit output
		LOG_ERR_RET(sensor_read(sd, 0x02, &tmp));
		tmp &= 0x7f;
		LOG_ERR_RET(sensor_write(sd, 0x02, tmp));
	}

	return err;
}


static int sensor_ptz_init(struct v4l2_subdev *sd)
{
	unsigned int i;

	LOG_ERR_RET(sensor_write(sd, 0x40, 0x00)); //bank

	LOG_ERR_RET(sensor_write(sd, 0xc9, 0x00));
	LOG_ERR_RET(sensor_write(sd, 0xca, 0x00));
	LOG_ERR_RET(sensor_write(sd, 0xcb, 0x05));
	LOG_ERR_RET(sensor_write(sd, 0xcc, 0x06));
	LOG_ERR_RET(sensor_write(sd, 0xcd, 0x08));
	LOG_ERR_RET(sensor_write(sd, 0xce, 0x09)); //line6,7,8,9
	LOG_ERR_RET(sensor_write(sd, 0xcf, 0x03));
	LOG_ERR_RET(sensor_write(sd, 0xd0, 0x48));
	LOG_ERR_RET(sensor_write(sd, 0xd1, 0x34)); //39 clock per bit 0.526us
	LOG_ERR_RET(sensor_write(sd, 0xd2, 0x60));
	LOG_ERR_RET(sensor_write(sd, 0xd3, 0x10));
	LOG_ERR_RET(sensor_write(sd, 0xd4, 0x04)); //
	LOG_ERR_RET(sensor_write(sd, 0xd5, 0xf0));
	LOG_ERR_RET(sensor_write(sd, 0xd6, 0xd8));
	LOG_ERR_RET(sensor_write(sd, 0xd7, 0x17)); //24bit


	LOG_ERR_RET(sensor_write(sd, 0x7E, 0x01));   //TX pin  enable

	return 0;
}

static int sensor_ptz_mode(struct v4l2_subdev *sd, unsigned char ch, unsigned char mod)
{
	unsigned char tmp;
	int i;

	static const unsigned char PTZ_reg[7] = {0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8};
	static const unsigned char PTZ_dat[4][7] = {
		{0x0b, 0x0c, 0x00, 0x00, 0x19, 0x78, 0x21}, //TVI1.0
		{0x0b, 0x0c, 0x00, 0x00, 0x33, 0xf0, 0x21}, //TVI2.0
		{0x0e, 0x0f, 0x10, 0x11, 0x64, 0xf0, 0x17}, //HDA
		{0x10, 0x11, 0x12, 0x13, 0x95, 0xb8, 0x21}, //HDC
	};

	LOG_ERR_RET(sensor_write(sd, 0x40, 0x00)); //reg bank1 switch for 2822

	for (i = 0; i < 7; i++) {
		if (PTZ_TVI == mod) {
			LOG_ERR_RET(sensor_read(sd, 0x4E, &tmp)); //check TVI 1 or 2
			if ( tmp & 0x04) {
				LOG_ERR_RET(sensor_write(sd, PTZ_reg[i], PTZ_dat[1][i]));
			} else {
				LOG_ERR_RET(sensor_write(sd, PTZ_reg[i], PTZ_dat[0][i]));
			}
		} else if (PTZ_HDA == mod) { //HDA 1080p
			LOG_ERR_RET(sensor_write(sd, PTZ_reg[i], PTZ_dat[2][i]));
		} else if (PTZ_HDC == mod) { // test
			LOG_ERR_RET(sensor_write(sd, PTZ_reg[i], PTZ_dat[3][i]));
		}
	}

	LOG_ERR_RET(sensor_write(sd, 0xB7, 0x01)); //enable TX interrupt flag

	return 0;
}


static int sensor_output_setup(struct v4l2_subdev *sd, int chip, int mode)
{
	uint8_t tmp;

	if ( MUX656_8BIT == output[chip] ) {
		LOG_ERR_RET(sensor_write(sd, 0x4C, 0x00));
		LOG_ERR_RET(sensor_write(sd, 0x4D, 0x03)); //both port enable
		if (SENSOR_720P25V2 == mode || SENSOR_720P30V2 == mode || SENSOR_PAL == mode || SENSOR_NTSC == mode ) {
			LOG_ERR_RET(sensor_write(sd, 0x4E, 0x15));
		} else {
			LOG_ERR_RET(sensor_write(sd, 0x4E, 0x01));
		}
	} else if ( SEP656_8BIT == output[chip] ) {
		LOG_ERR_RET(sensor_write(sd, 0x4C, 0x03));
		LOG_ERR_RET(sensor_write(sd, 0x4D, 0x03)); //both port enable
		if (SENSOR_720P25V2 == mode || SENSOR_720P30V2 == mode || SENSOR_PAL == mode || SENSOR_NTSC == mode ) {
			LOG_ERR_RET(sensor_write(sd, 0x4E, 0x15));
		} else {
			LOG_ERR_RET(sensor_write(sd, 0x4E, 0x01));
		}
	} else if ( EMB422_16BIT == output[chip] ) {
		LOG_ERR_RET(sensor_write(sd, 0x4C, 0x00));
		LOG_ERR_RET(sensor_write(sd, 0x4D, 0x0B));
		if (SENSOR_720P25V2 == mode || SENSOR_720P30V2 == mode || SENSOR_PAL == mode || SENSOR_NTSC == mode ) {
			LOG_ERR_RET(sensor_write(sd, 0x4E, 0x35));
		} else {
			LOG_ERR_RET(sensor_write(sd, 0x4E, 0x31));
		}
	} else if ( SEP422_16BIT == output[chip] ) {
		LOG_ERR_RET(sensor_write(sd, 0x4C, 0x03));
		LOG_ERR_RET(sensor_write(sd, 0x4D, 0x0B));
		if (SENSOR_720P25V2 == mode || SENSOR_720P30V2 == mode || SENSOR_PAL == mode || SENSOR_NTSC == mode ) {
			LOG_ERR_RET(sensor_write(sd, 0x4E, 0x35));
		} else {
			LOG_ERR_RET(sensor_write(sd, 0x4E, 0x31));
		}
	}

	return 0;
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


static int sensor_comm_init(struct v4l2_subdev *sd, int chip)
{
	uint8_t tmp;

	LOG_ERR_RET(sensor_reset_default(sd));

	LOG_ERR_RET(sensor_set_video_mode(sd, 0, init_mode, CH_ALL, init_std /*STD_TVI*/));

#if WATCHDOG
	if (SENSOR_NTSC == init_mode ) {
		LOG_ERR_RET(sensor_write(sd, 0x26, 0x11));
	} else {
		LOG_ERR_RET(sensor_write(sd, 0x26, 0x01));
	}
#endif

	//LOG_ERR_RET(sensor_set_pll_enable(sd, 1));

	//MUX output
	LOG_ERR_RET(sensor_output_setup(sd, chip, init_mode));

	LOG_ERR_RET(sensor_ptz_init(sd));

	//soft reset
	LOG_ERR_RET(sensor_read(sd, 0x06, &tmp));
	LOG_ERR_RET(sensor_write(sd, 0x06, 0x80 | tmp));
	usleep_range(1000, 1200);

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

	ret = sensor_comm_init(sd, 0);

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
	cfg->flags = V4L2_MBUS_MASTER | VREF_POL | HREF_POL | CLK_POL ;

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

#if 0
	LOG_ERR_RET(sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size))

	ret = 0;
	if (wsize->regs)
	{
		// usleep_range(5000,6000);
		LOG_ERR_RET(sensor_write_array(sd, wsize->regs, wsize->regs_size))
	}
#endif

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
	int i, interval, ret = 0;
	int chip = 0, format = 0, format_loop = 0;
	unsigned char status, vin, tmp, cvstd;
	unsigned char flag_locked;
	//unsigned char gain, agc, flag_locked;

	struct _chip_info *ci;

	vfe_dev_dbg("tp2825_watchdog_deamon: start!\n");

	sched_setscheduler(current, SCHED_FIFO, &param);
	current->flags |= PF_NOFREEZE;

	set_current_state(TASK_INTERRUPTIBLE);

	while (watchdog_state != WATCHDOG_EXIT)	{
		spin_lock_irq(&watchdog_lock);

		do_gettimeofday(&start);

		if (glb_sd && tp2825_initialized) {


			ci = &chip_info[chip];
			for (i = 0; i < CHANNELS_PER_CHIP; i++) {

				if (ci->scan[i] == SCAN_DISABLE) continue;

				ret = sensor_set_reg_page(glb_sd, i);
				LOG_CHECK_RET(ret);

				ret = sensor_read(glb_sd, 0x01, &status);
				LOG_CHECK_RET(ret);
				if (ret) break;

				ret = sensor_read(glb_sd, 0x02, &tmp);
				ret = sensor_read(glb_sd, 0x03, &cvstd);
				vfe_dev_dbg("### status = %02x, config = %02x, det = %02x(%s)\n", status, tmp, cvstd, get_cvstd_name(cvstd&0xf));

				//channel
				ret = sensor_read(glb_sd, 0x41, &vin);
				LOG_CHECK_RET(ret);
				if (ret) break;
				vin &= 0x7;

				if (status & FLAG_LOSS) {

					if (VIDEO_UNPLUG != ci->state[i]) {
						ci->state[i] = VIDEO_UNPLUG;
						ci->count[i] = 0;
						if (SCAN_MANUAL != ci->scan[i]) {
							ci->mode[i] = INVALID_FORMAT;
						}
					}

#if 0
					ret = sensor_read(glb_sd, 0x41, &vin);
					if (ret) break;
					vin &= 0x7;
					//vfe_dev_dbg("channel = %02x\n", vin);
					//set next channel
					if (vin == 0x04) vin = 0;
					else vin++;
					sensor_write(glb_sd, 0x41, vin);
#endif

					if (ci->count[i] == 0) {
						ret = sensor_set_video_mode(glb_sd, 0, init_mode, i, init_std);
						LOG_CHECK_RET(ret);
						ret = sensor_reset_default(glb_sd);
						LOG_CHECK_RET(ret);
						ci->count[i]++;
					} else {
						if (ci->count[i] < MAX_COUNT) ci->count[i]++;
					}

					vfe_dev_dbg("video loss#%d\n", ci->count[i]);

				} else {
					//there is a video
					if ( SENSOR_PAL == ci->mode[i] || SENSOR_NTSC == ci->mode[i] ) {
						flag_locked = FLAG_HV_LOCKED;
					} else {
						flag_locked = FLAG_HV_LOCKED;
					}

					if ( flag_locked == (status & flag_locked) ) {//video locked
						if (VIDEO_LOCKED == ci->state[i]) {
							if (ci->count[i] < MAX_COUNT) ci->count[i]++;
						} else if (VIDEO_UNPLUG == ci->state[i]) {
							ci->state[i] = VIDEO_IN;
							ci->count[i] = 0;
							vfe_dev_dbg("[1] video in ch = %02x\n", vin);
						} else if (ci->mode[i] != INVALID_FORMAT) {
							//if( FLAG_HV_LOCKED == (FLAG_HV_LOCKED & status) )//H&V locked
							{
								ci->state[i] = VIDEO_LOCKED;
								ci->count[i] = 0;
								vfe_dev_dbg("*** video locked#%d ch = %02x\n", ci->count[i], vin);
							}
						}
					} else {
						//video in but unlocked
						if (VIDEO_UNPLUG == ci->state[i]) {
							ci->state[i] = VIDEO_IN;
							ci->count[i] = 0;
							vfe_dev_dbg("[2] video in ch = %02x\n", vin);
						} else if (VIDEO_LOCKED == ci->state[i]) {
							ci->state[i] = VIDEO_UNLOCK;
							ci->count[i] = 0;
							vfe_dev_dbg("video unstable ch = %02x\n", vin);
						} else {
							if (ci->count[i] < MAX_COUNT) ci->count[i]++;
							if (VIDEO_UNLOCK == ci->state[i] && ci->count[i] > 2) {
								ci->state[i] = VIDEO_IN;
								ci->count[i] = 0;
								if (SCAN_MANUAL != ci->scan[i]) {
									ret = sensor_reset_default(glb_sd);
									LOG_CHECK_RET(ret);
								}
								vfe_dev_dbg("video unlocked ch = %02x\n", vin);
							}
						}
					}

					if ( ci->force[i] ) { //manual reset for V1/2 switching
						ci->state[i] = VIDEO_UNPLUG;
						ci->count[i] = 0;
						ci->mode[i] = INVALID_FORMAT;
						ci->force[i] = 0;
						ret = sensor_reset_default(glb_sd);
						LOG_CHECK_RET(ret);
						//ret = sensor_set_video_mode(glb_sd, 0, init_mode, i, init_std	);
						//LOG_CHECK_RET(ret);
					}
				}

				if ( VIDEO_IN == ci->state[i]) {
					//video-in detected.
					if (SCAN_MANUAL != ci->scan[i]) {

						//ret = sensor_read(glb_sd, 0x03, &cvstd);
						//LOG_CHECK_RET(ret);
						//if (ret) break;

						cvstd &= 0x0f;
						//vfe_dev_dbg(">>> video in cvstd = %02x\n", cvstd);

						ci->std[i] = STD_TVI;

						if ( SENSOR_SD == (cvstd & 0x07) ) {
							if (ci->count[i] & 0x01) {
								ci->mode[i] = SENSOR_PAL;
								//sensor_set_video_mode(glb_sd, 0, ci->mode[i], i, STD_TVI);
							} else {
								ci->mode[i] = SENSOR_NTSC;
								//sensor_set_video_mode(glb_sd, 0, ci->mode[i], i, STD_TVI);
							}
						} else if ((cvstd & 0x07) < 6 ) {
							if (SCAN_HDA == ci->scan[i]) {
								ci->std[i] = STD_HDA;
							} else if (SCAN_HDC == ci->scan[i]) {
								ci->std[i] = STD_HDC;
							}

							if ( SENSOR_720P25 == (cvstd & 0x07) ) {
								ci->mode[i] = SENSOR_720P25V2;
							} else if ( SENSOR_720P30 == (cvstd & 0x07) ) {
								ci->mode[i] = SENSOR_720P30V2;
							} else {
								//ignore tvi ver.2
								ci->mode[i] = cvstd & 0x07;
							}
						} else {
							ci->mode[i] = cvstd;
						}

						vfe_dev_dbg(">>> video in#%d ch %02x cvstd = %02x(%s) mode %02x(%s) std %02x(%s)\n", ci->count[i], vin, cvstd, get_cvstd_name(cvstd), ci->mode[i], get_cvstd_name(ci->mode[i]), ci->std[i], get_std_name(ci->std[i]));
						if((ci->mode[i]&0x7) != INVALID_FORMAT) {
							ret = sensor_set_video_mode(glb_sd, 0, ci->mode[i], i, ci->std[i]);
							LOG_CHECK_RET(ret);
						}

					} /* fi(SCAN_MANUAL != ci->scan[i]) */
				} /* fi(VIDEO_IN == ci->state[i]) */

#define EQ_COUNT 10

				if ( VIDEO_LOCKED == ci->state[i]) {
					if (0 == ci->count[i] ) {

						if ( (SCAN_AUTO == ci->scan[i] || SCAN_TVI == ci->scan[i] )) {
							//ci->std[i] = STD_TVI;
							if ( (SENSOR_720P30V2 == ci->mode[i]) || (SENSOR_720P25V2 == ci->mode[i]) ) {
								ret = sensor_read(glb_sd, 0x03, &tmp);
								LOG_CHECK_RET(ret);
								vfe_dev_dbg("*** video locked#%d ch %02x, cvstd %02x(%s)\n", ci->count[i], vin, tmp, get_cvstd_name(tmp&0xf));
								if ( ! (0x08 & tmp) ) {
									vfe_dev_dbg("*** video locked#%d ch %02x, 720p v1 detected\n", ci->count[i], vin);
									ci->mode[i] &= 0xf7;
									ret = sensor_set_video_mode(glb_sd, 0, ci->mode[i], i, STD_TVI); //to speed the switching
									LOG_CHECK_RET(ret);
								}
							}

							//these code need to keep bottom
							if(0) {
								ret = sensor_read(glb_sd, 0xb9, &tmp);
								LOG_CHECK_RET(ret);
								ret = sensor_write(glb_sd, 0xb9, tmp & 0xfe);
								LOG_CHECK_RET(ret);
								ret = sensor_write(glb_sd, 0xb9, tmp | 0x01);
								LOG_CHECK_RET(ret);
							}

							ret = sensor_read(glb_sd, 0x4e, &tmp);
							LOG_CHECK_RET(ret);
							vfe_dev_dbg("*** video locked#%d, 0x4e=%02x\n", ci->count[i], tmp);
							tmp &= 0xcf; //clear clock MD bits
							if ( tmp & 0x04) {
								ret = sensor_write(glb_sd, 0x4e, tmp | 0x10);
								LOG_CHECK_RET(ret);
							} else {
								int half_scaler = 1;
								if (half_scaler) { //down scaler output
									ret = sensor_write(glb_sd, 0x35, 0x45);
									LOG_CHECK_RET(ret);
									ret = sensor_write(glb_sd, 0x4e, tmp | 0x10);
									LOG_CHECK_RET(ret);
								} else {
									ret = sensor_write(glb_sd, 0x4e, tmp);
									LOG_CHECK_RET(ret);
								}
							}

						}

					} else if (1 == ci->count[i]) {

						ret = sensor_read(glb_sd, 0x01, &status);
						LOG_CHECK_RET(ret);
						ret = sensor_read(glb_sd, 0x02, &tmp);
						LOG_CHECK_RET(ret);
						ret = sensor_read(glb_sd, 0x03, &cvstd);
						LOG_CHECK_RET(ret);
						vfe_dev_dbg("*** video locked#%d ch %02x status = %02x, config = %02x, det = %02x(%s)\n", ci->count[i], vin, status, tmp, cvstd, get_cvstd_name(cvstd&0xf));

					} else if ( ci->count[i] < EQ_COUNT - 3) {

						if ( SCAN_AUTO == ci->scan[i] && STD_TVI == ci->std[i]) {

							ret = sensor_read(glb_sd, 0x01, &tmp);
							LOG_CHECK_RET(ret);

							if ((SENSOR_PAL == ci->mode[i]) || (SENSOR_NTSC == ci->mode[i])) {
								//nothing to do
							} else if (0x60 == (tmp & 0x64) && STD_TVI == ci->std[i] ) {
								uint8_t rx2;
								ret = sensor_read(glb_sd, 0x94, &rx2);
								LOG_CHECK_RET(ret);
								vfe_dev_dbg("*** video locked#%d ch %02x rx2 = %02x\n", ci->count[i], vin, rx2);

								if (HDC_enable) {
									if(0xff == rx2) {
										ci->std[i] = STD_HDC;
									} /* else if(0x00 == rx2) {
										ci->std[i] = STD_HDC_DEFAULT;
									} */ else {
										ci->std[i] = STD_HDA;
									}
								}else {
									ci->std[i] = STD_HDA;
								}

								if (STD_TVI != ci->std[i]) {
									ret = sensor_set_video_mode(glb_sd, 0, ci->mode[i], i, ci->std[i]);
									LOG_CHECK_RET(ret);
									vfe_dev_dbg("*** video locked#%d ch %02x standard to %02x(%s)\n", ci->count[i], vin, ci->std[i], get_std_name(ci->std[i]));
								}
							}
						}
					}
				} /* VIDEO_LOCKED */

			} /* for channels */
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

	memset(chip_info, 0, sizeof(chip_info));
	for (i = 0; i < MAX_CHIPS; i++) {
		for (j = 0; j < CHANNELS_PER_CHIP; j++) {
			chip_info[i].count[j] = 0;
			chip_info[i].force[j] = 0;
			chip_info[i].mode[j] = INVALID_FORMAT;
			chip_info[i].scan[j] = SCAN_AUTO;
			chip_info[i].state[j] = VIDEO_UNPLUG;
			chip_info[i].std[j] = STD_TVI;
		}
	}

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

