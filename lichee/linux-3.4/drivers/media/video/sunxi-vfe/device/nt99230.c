/*
 * A V4L2 driver for NT99230 cameras.
 * Novatek 2013-12-10
 */
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
#include <linux/miscdevice.h>


#include "camera.h"
#include "../csi_cci/bsp_cci.h"

MODULE_AUTHOR("raymonxiu");
MODULE_DESCRIPTION("A low-level driver for NT99230 sensors");
MODULE_LICENSE("GPL");

//static struct v4l2_subdev *nt99230_sd = NULL;

//for internel driver debug
#define DEV_DBG_EN   		0
#if(DEV_DBG_EN == 1)		
#define vfe_dev_dbg(x,arg...) printk("[CSI_DEBUG][NT99230]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[CSI_ERR][NT99230]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[CSI][NT99230]"x,##arg)

#define LOG_ERR_RET(x)  { \
                          int ret;  \
                          ret = x; \
                          if(ret < 0) {\
                            vfe_dev_err("error at %s\n",__func__);  \
                            return ret; \
                          } \
                        }

//define module timing
#define MCLK (24*1000*1000)
#define VREF_POL          V4L2_MBUS_VSYNC_ACTIVE_HIGH
#define HREF_POL          V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_RISING
#define V4L2_IDENT_SENSOR 0x2300

//define the voltage level of control signal
#define CSI_STBY_ON			1
#define CSI_STBY_OFF 		0
#define CSI_RST_ON			0
#define CSI_RST_OFF			1
#define CSI_PWR_ON			1
#define CSI_PWR_OFF			0

#define regval_list reg_list_a16_d8
#define REG_TERM 0xff
#define VAL_TERM 0xff
#define REG_DLY  0xffff

#define NT99230_0X3022 0x24 //0x26 //Normal
//#define NT99230_0X3022 0x27

#if 0	//(NT99230_AE_TARGET_MEAN == 0x38)
#define NT99230_REG_0X32B8	0x3F
#define NT99230_REG_0X32B9	0x31
#define NT99230_REG_0X32BC	0x38
#define NT99230_REG_0X32BD	0x3C
#define NT99230_REG_0X32BE	0x34
#endif
#if 0	//(NT99230_AE_TARGET_MEAN == 0x36)
#define NT99230_REG_0X32B8	0x3D
#define NT99230_REG_0X32B9	0x2F
#define NT99230_REG_0X32BC	0x36
#define NT99230_REG_0X32BD	0x3A
#define NT99230_REG_0X32BE	0x32
#endif
#if 1	//(NT99230_AE_TARGET_MEAN == 0x34) 
#define NT99230_REG_0X32B8	0x3B
#define NT99230_REG_0X32B9	0x2D
#define NT99230_REG_0X32BC	0x34
#define NT99230_REG_0X32BD	0x38
#define NT99230_REG_0X32BE	0x30
#endif
#if	0	//(NT99230_AE_TARGET_MEAN == 0x32)
#define NT99230_REG_0X32B8	0x39
#define NT99230_REG_0X32B9	0x2B
#define NT99230_REG_0X32BC	0x32
#define NT99230_REG_0X32BD	0x36
#define NT99230_REG_0X32BE	0x2E
#endif
#if 0	//(NT99230_AE_TARGET_MEAN == 0x30)
#define NT99230_REG_0X32B8	0x36
#define NT99230_REG_0X32B9	0x2A
#define NT99230_REG_0X32BC	0x30
#define NT99230_REG_0X32BD	0x33
#define NT99230_REG_0X32BE	0x2D
#endif
/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE 15

/*
 * The nt99230 sits on i2c with ID 0x66
 */
#define I2C_ADDR 0x66
#define SENSOR_NAME "nt99230"

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

// ################ xz:I2C_TWI_USED is defined start####################################

//#define I2C_TWI_USED

#ifdef I2C_TWI_USED
#define cci_print(x,arg...) printk(KERN_INFO"[VFE_DEV_I2C]"x,##arg)
#define cci_dbg(l,x,arg...) if(cci_dbg_en && l <= cci_dbg_lv) printk(KERN_DEBUG"[VFE_DEV_I2C_DBG]"x,##arg)
#define cci_err(x,arg...) printk(KERN_ERR"[VFE_DEV_I2C_ERR]"x,##arg)
#else
#ifdef USE_SPECIFIC_CCI
#define cci_print(x,arg...) printk(KERN_INFO"[VFE_DEV_CCI]"x,##arg)
#define cci_dbg(l,x,arg...) if(cci_dbg_en && l <= cci_dbg_lv) printk(KERN_DEBUG"[VFE_DEV_CCI_DBG]"x,##arg)
#define cci_err(x,arg...) printk(KERN_ERR"[VFE_DEV_CCI_ERR]"x,##arg)
#endif
#endif

int cci_write_a16_d8_xz(struct v4l2_subdev *sd, unsigned char addr,
    unsigned short value)
{
#ifdef I2C_TWI_USED
	struct i2c_msg msg;
	unsigned char data[3];
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	
	data[0] = addr;
	data[1] = (value&0xff00)>>8;
	data[2] = (value&0x00ff);
	
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 3;
	msg.buf = data;
	
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret >= 0) {
	ret = 0;
	} else {
	cci_err("%s error! slave = 0x%x, addr = 0x%2x, value = 0x%4x\n ",__func__, client->addr, addr,value);
	}
	return ret;
#else
	#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv;
	cci_drv = v4l2_get_subdevdata(sd);
	return cci_wr_8_16(cci_drv->cci_id, addr, value, cci_drv->cci_saddr);
	#endif
#endif
}

int cci_read_a16_d8_xz(struct v4l2_subdev *sd, unsigned short addr,
    unsigned char *value)
{
#ifdef I2C_TWI_USED 
	int ret;
	unsigned char data[3];
	struct i2c_msg msg[2];
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	data[0] = (addr&0xff00)>>8;
	data[1] = (addr&0x00ff);
	data[2] = 0xee;
	/*
	* Send out the register address...
	*/ 
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = &data[0];
	/*
	* ...then read back the result.
	*/
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &data[2];
	
	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret >= 0) {
	*value = data[2];
	ret = 0;
	} else {
	cci_err("%s error! slave = 0x%x, addr = 0x%4x, value = 0x%2x\n ",__func__, client->addr, addr,*value);
	}
	return ret;
#else
	#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv;
	cci_drv = v4l2_get_subdevdata(sd);
	return cci_rd_16_8(cci_drv->cci_id, addr, value, cci_drv->cci_saddr);
	#endif

#endif
}

void cci_lock_xz(struct v4l2_subdev *sd)
{
#ifdef I2C_TWI_USED
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	i2c_lock_adapter(client->adapter); 
#else
	#ifdef USE_SPECIFIC_CCI
	#endif
#endif
}

void cci_unlock_xz(struct v4l2_subdev *sd)
{
#ifdef I2C_TWI_USED
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	i2c_unlock_adapter(client->adapter);
#else
	#ifdef USE_SPECIFIC_CCI
	#endif
#endif
}


// ################ xz:I2C_TWI_USED is defined ######################################

/*
 * The default register settings
 *
 */
#define NTK_FIX_FPS		//2013-09-17

static struct regval_list sensor_default_regs[] = {    
         {0x3100, 0x03},
		 {0x3101, 0x00},
		 {0x3102, 0x09},
		 {0x3103, 0x09},
		 {0x3104, 0x00},
		 {0x3105, 0x03},
		 {0x3106, 0x07},
		 {0x3107, 0x30},
		 {0x3108, 0x50},
		 {0x3109, 0x02},
		 {0x310A, 0x75},
		 {0x310B, 0xC0},
		 {0x310C, 0x00},
		 {0x310D, 0x43},
		 {0x310E, 0x01},//0x00-0x01
		 {0x3110, 0x88},
		 {0x3111, 0xCE},//0x18->0x38
		 {0x3112, 0x88},
		 {0x3113, 0x66},
		 {0x3114, 0x33},
		 {0x3115, 0x88},
		 {0x3116, 0x86},
		 {0x3118, 0xAF},
		 {0x3119, 0xAD},
		 {0x311A, 0xAF},
		 {0x303F, 0x32},
		 {0x3055, 0x00},
		 {0x3056, 0x24},
		 {0x308B, 0x2D},
		 {0x308C, 0x28},
		 {0x308D, 0x24},
		 {0x308F, 0x10},
		 {0x30A0, 0x01},
		 {0x30A4, 0x1F},
		 {0x350B, 0xFF},
		 {0x3514, 0x05},
		 {0x3515, 0x01},
		 {0x3518, 0x06},
		 {0x3519, 0x00},
		 {0x351A, 0xFF},
		 {0x3530, 0x50},
		 {0x3534, 0x00},
		 {0x3512, 0x05},
		 {0x3511, 0x09},
		 {0x3513, 0x00},
		 {0x3532, 0x35},
		 {0x3533, 0x20},
		 {0x3210, 0x10},
		 {0x3211, 0x10},
		 {0x3212, 0x10},
		 {0x3213, 0x10},
		 {0x3214, 0x10},
		 {0x3215, 0x10},
		 {0x3216, 0x10},
		 {0x3217, 0x10},
		 {0x3218, 0x10},
		 {0x3219, 0x10},
		 {0x321A, 0x10},
		 {0x321B, 0x10},
		 {0x321C, 0x14},
		 {0x321D, 0x14},
		 {0x321E, 0x14},
		 {0x321F, 0x14},
		 {0x3230, 0x00},
		 {0x3231, 0x00},
		 {0x3232, 0x00},
		 {0x3233, 0x0C},
		 {0x3234, 0x00},
		 {0x3235, 0x00},
		 {0x3236, 0x00},
		 {0x3237, 0x00},
		 {0x3238, 0x18},
		 {0x3239, 0x18},
		 {0x323A, 0x28},
		 {0x3243, 0xC3},
		 {0x3244, 0x00},
		 {0x3245, 0x00},
		 {0x3241, 0x30},
		 {0x3270, 0x00},
		 {0x3271, 0x0B},
		 {0x3272, 0x16},
		 {0x3273, 0x2B},
		 {0x3274, 0x3F},
		 {0x3275, 0x51},
		 {0x3276, 0x72},
		 {0x3277, 0x8F},
		 {0x3278, 0xA7},
		 {0x3279, 0xBC},
		 {0x327A, 0xDC},
		 {0x327B, 0xF0},
		 {0x327C, 0xFA},
		 {0x327D, 0xFE},
		 {0x327E, 0xFF},
		 {0x3700, 0x18},
		 {0x3701, 0x21},
		 {0x3702, 0x2B},
		 {0x3703, 0x41},
		 {0x3704, 0x50},
		 {0x3705, 0x5D},
		 {0x3706, 0x74},
		 {0x3707, 0x88},
		 {0x3708, 0x99},
		 {0x3709, 0xA9},
		 {0x370A, 0xC3},
		 {0x370B, 0xD3},
		 {0x370C, 0xDD},
		 {0x370D, 0xE4},
		 {0x370E, 0xEB},
		 {0x3710, 0x02},
		 {0x3800, 0x00},
		 {0x3302, 0x00},
		 {0x3303, 0x4C},
		 {0x3304, 0x00},
		 {0x3305, 0x96},
		 {0x3306, 0x00},
		 {0x3307, 0x1D},
		 {0x3308, 0x07},
		 {0x3309, 0xD6},
		 {0x330A, 0x06},
		 {0x330B, 0x84},
		 {0x330C, 0x01},
		 {0x330D, 0xA6},
		 {0x330E, 0x01},
		 {0x330F, 0x17},
		 {0x3310, 0x07},
		 {0x3311, 0x10},
		 {0x3312, 0x07},
		 {0x3313, 0xDA},
		 {0x3250, 0x38},
		 {0x3251, 0x1A},
		 {0x3252, 0x2A},
		 {0x3253, 0x18},
		 {0x3254, 0x2F},
		 {0x3255, 0x1C},
		 {0x3256, 0x17},
		 {0x3257, 0x0C},
		 {0x3258, 0x3D},
		 {0x3259, 0x2F},
		 {0x325A, 0x15},
		 {0x325B, 0x0A},
		 {0x325C, 0xE0},
		 {0x325D, 0x05},
		 {0x3296, 0x01},
		 {0x3298, 0x20},
		 {0x3299, 0x96},
		 {0x329C, 0x15},
		 {0x329D, 0x90},
		 {0x3290, 0x50},
		 {0x3292, 0x50},
		 {0x3297, 0x03},
		 {0x329E, 0x00},
		 {0x32B8, 0x07},
		 {0x32B9, 0x07},
		 {0x32B0, 0x57},
		 {0x32B1, 0xAC},
		 {0x32B2, 0x11},
		 {0x32B3, 0x80},
		 {0x32B4, 0x00},
		 {0x32BC, 0x40},
		 {0x32BD, 0x04},
		 {0x32BE, 0x04},
		 {0x32CB, 0x20},
		 {0x32CC, 0x70},
		 {0x32CD, 0xA0},
		 {0x3326, 0x0A},
		 {0x3327, 0x05},
		 {0x335B, 0xD0},
		 {0x3360, 0x20},
		 {0x3361, 0x28},
		 {0x3362, 0x30},
		 {0x3365, 0x08},
		 {0x3366, 0x0A},
		 {0x3367, 0x10},
		 {0x3368, 0x30},
		 {0x3369, 0x24},
		 {0x336B, 0x18},
		 {0x336D, 0x20},
		 {0x336E, 0x16},
		 {0x3370, 0x0E},
		 {0x3371, 0x08},
		 {0x3372, 0x18},
		 {0x3374, 0x20},
		 {0x3375, 0x10},
		 {0x3376, 0x10},
		 {0x3378, 0x10},
		 {0x3379, 0x02},
		 {0x337A, 0x08},
		 {0x337C, 0x10},
		 {0x33A0, 0x40},
		 {0x33A1, 0x58},//0x04->0x06
		 {0x33A2, 0x00},
		 {0x33A3, 0x40},
		 {0x33A4, 0x00},
		 {0x33A5, 0x8C},
		 {0x33A6, 0x02},
		 {0x33A7, 0x00},
		 {0x33A9, 0x02},
		 {0x33AA, 0x03},
		 {0x33AC, 0x04},
		 {0x33AD, 0x04},
		 {0x33AE, 0x05},
		 {0x33B0, 0x08},
		 {0x33B1, 0x00},
		 {0x33B4, 0xBB},
		 {0x33B5, 0xA4},
		 {0x33BA, 0x03},
		 {0x33BB, 0x07},
		 {0x33C0, 0x01},
		 {0x33C6, 0x03},
		 {0x33C7, 0x43},
		 {0x33C8, 0x33},
		 {0x33C9, 0x0A},
		 {0x3363, 0x31},
		 {0x3364, 0x0B},
		 {0x333F, 0x07},
};


/* 1920X1080 FHD*/
static struct regval_list sensor_FHD_regs[] =
{

		//MCLK:      24.00MHz 
		//Pixel Clk: 32.00MHz 
		//Out Clk:   64.00MHz 
		//Size:      1280x720 
		//FPS:       20.00~20.02 
		//Line:      1772 
		//Frame:      902 
		//Flicker:   50Hz 
		 {0x32BB, 0x67},  //AE Start
		 {0x32BF, 0x60}, 
		 {0x32C0, 0x64}, 
		 {0x32C1, 0x64}, 
		 {0x32C2, 0x64}, 
		 {0x32C3, 0x00}, 
		 {0x32C4, 0x30}, 
		 {0x32C5, 0x30}, 
		 {0x32C6, 0x30}, 
		 {0x32D3, 0x00}, 
		 {0x32D4, 0xB5}, 
		 {0x32D5, 0x76}, 
		 {0x32D6, 0x00}, 
		 {0x32D7, 0x96}, 
		 {0x32D8, 0x72},  //AE End
		 {0x32F0, 0x00},  //Output Format
		 {0x3200, 0x3E},  //Mode Control
		 {0x3201, 0x3F},  //Mode Control
		 {0x302A, 0x80},  //PLL Start
		 {0x302B, 0x08}, 
		 {0x302C, 0x0F}, 
		 {0x302D, 0x01}, 
		 {0x302E, 0x00}, 
		 {0x302F, 0x11},  //PLL End
		 {0x3022, 0x24},  //Timing Start
		 {0x3023, 0x24}, 
		 {0x3002, 0x01}, 
		 {0x3003, 0x48}, 
		 {0x3004, 0x00}, 
		 {0x3005, 0xB6}, 
		 {0x3006, 0x06}, 
		 {0x3007, 0x47}, 
		 {0x3008, 0x03}, 
		 {0x3009, 0x85}, 
		 {0x300A, 0x06}, 
		 {0x300B, 0xEC}, 
		 {0x300C, 0x03}, 
		 {0x300D, 0x86}, 
		 {0x300E, 0x05}, 
		 {0x300F, 0x00}, 
		 {0x3010, 0x02}, 
		 {0x3011, 0xD0},  //Timing End
		 {0x320A, 0x00}, 
		 {0x3021, 0x02}, 
		 {0x3060, 0x01}, 
	
	
};

/* 1600X1200 UXGA*/
static struct regval_list sensor_uxga_regs[] =
{
	//not support
};

/* 1280X1024 SXGA */
static struct regval_list sensor_sxga_regs[] =
{
	//not support
};

/* 1280X720 SXGA */
static struct regval_list sensor_720P_regs[] =
{
//VCO: 576 MHz
//PCLK: 64 MHz
//Line Length: 1772
//Frame Length: 902
//Image Size: 1280x720
//Output Size: 1280x720
//Scaler Size: 0x0
//Min FPS: 20.00
//Max FPS: 20.02
		{0x32BB, 0x67},
		{0x32BF, 0x60},
		{0x32C0, 0x64},
		{0x32C1, 0x64},
		{0x32C2, 0x64},
		{0x32C3, 0x00},
		{0x32C4, 0x30},
		{0x32C5, 0x30},
		{0x32C6, 0x30},
		{0x32D3, 0x00},
		{0x32D4, 0xB5},
		{0x32D5, 0x76},
		{0x32D6, 0x00},
		{0x32D7, 0x96},
		{0x32D8, 0x72},
		{0x32F0, 0x00},
		{0x3200, 0x3E},
		{0x3201, 0x3F},
		{0x302A, 0x80},
		{0x302B, 0x08},
		{0x302C, 0x0F},
		{0x302D, 0x01},
		{0x302E, 0x00},
		{0x302F, 0x11},
		{0x3022, 0x24},
		{0x3023, 0x24},
		{0x3002, 0x01},
		{0x3003, 0x48},
		{0x3004, 0x00},
		{0x3005, 0xB6},
		{0x3006, 0x06},
		{0x3007, 0x47},
		{0x3008, 0x03},
		{0x3009, 0x85},
		{0x300A, 0x06},
		{0x300B, 0xEC},
		{0x300C, 0x03},
		{0x300D, 0x86},
		{0x300E, 0x05},
		{0x300F, 0x00},
		{0x3010, 0x02},
		{0x3011, 0xD0},
		{0x320A, 0x00},
		{0x3021, 0x02},
		{0x3060, 0x01},
		
};


/* 1280X720 SXGA */
static struct regval_list sensor_720P_regs_12_5fps[] =
{
   //[YUYV_1280x720_12.50_12.51_Fps]
    

};

/* 1280X720 SXGA */
static struct regval_list sensor_720P_regs_15fps[] =
{
    //[YUYV_1280x720_15.00_15.00_Fps]
 
};

/* 1280X720 SXGA */
static struct regval_list sensor_720P_regs_20fps[] =
{
 
};

/* 1280X720 SXGA */
static struct regval_list sensor_720P_regs_25fps[] =
{
  
};


/* 1280X720 SXGA */
static struct regval_list sensor_720P_regs_30fps[] =
{       

};

/*1024*576*/
static struct regval_list sensor_xga_regs[] =
{
	//not support

	
};

/* 800X600 SVGA*/
static struct regval_list sensor_svga_regs[] =
{
	//{0x3025, 0x02},   //Test pattern: Color bars(On)
	

};

/* 640X480 VGA */
static struct regval_list sensor_vga_regs[] =
{
	

};

/*
 * The white balance settings
 * Here only tune the R G B channel gain. 
 * The white balance enalbe bit is modified in sensor_s_autowb and sensor_s_wb
 */
 
#if 1 //0 Gamma Test  1 Normal 
static struct regval_list sensor_wb_manual[] = { 
//null
};

static struct regval_list sensor_wb_auto_regs[] = {
    {0x3201, 0x7F},
};

static struct regval_list sensor_wb_incandescence_regs[] = {
	//bai re guang
	{0x3201, 0x6F},
	{0x3290, 0x01},
	{0x3291, 0x30},
	{0x3296, 0x01},
	{0x3297, 0xcb},
};

static struct regval_list sensor_wb_fluorescent_regs[] = {
    {0x3201, 0x6F},
    {0x3290, 0x01},
    {0x3291, 0x70},
    {0x3296, 0x01},
    {0x3297, 0xFF},
};

static struct regval_list sensor_wb_tungsten_regs[] = {
	//wu si deng
	/* Office Colour Temperature : 3500K - 5000K  */
	{0x3201, 0x6F},
	{0x3290, 0x01},
	{0x3291, 0x00},
	{0x3296, 0x02},
	{0x3297, 0x30},
};

static struct regval_list sensor_wb_horizon[] = { 
//null
};

static struct regval_list sensor_wb_daylight_regs[] = {
	//Sunny
	/* ClearDay Colour Temperature : 5000K - 6500K  */
	{0x3201, 0x6F},
	{0x3290, 0x01},
	{0x3291, 0x38},
	{0x3296, 0x01},
	{0x3297, 0x68},
};

static struct regval_list sensor_wb_flash[] = { 
//null
};


static struct regval_list sensor_wb_cloud_regs[] = {
	/* Cloudy Colour Temperature : 6500K - 8000K  */
	{0x3201, 0x6F},
	{0x3290, 0x01},
	{0x3291, 0x51},
	{0x3296, 0x01},
	{0x3297, 0x00},
};

static struct regval_list sensor_wb_shade[] = { 
//null
};
#elif 0

static struct regval_list sensor_wb_manual[] = { 
//null
};

static struct regval_list sensor_wb_auto_regs[] = {
	{0x3201, 0x7F},
	{0x3270, 0x00}, //GammaT1
	{0x3271, 0x10},
	{0x3272, 0x1c},
	{0x3273, 0x31},
	{0x3274, 0x44},
	{0x3275, 0x54},
	{0x3276, 0x6d},
	{0x3277, 0x82},
	{0x3278, 0x93},
	{0x3279, 0xa1},
	{0x327A, 0xb8},
	{0x327B, 0xcc},
	{0x327C, 0xde},
	{0x327D, 0xf0},
	{0x327E, 0xFF},
	{0x3060, 0x01},
};

static struct regval_list sensor_wb_incandescence_regs[] = {
	{0x3270, 0x00},   //[Gamma3]                                                       
	{0x3271, 0x0B},                                                        
	{0x3272, 0x16},                                                        
	{0x3273, 0x2B},                                                        
	{0x3274, 0x3F},                                                        
	{0x3275, 0x51},                                                        
	{0x3276, 0x72},                                                        
	{0x3277, 0x8F},                                                        
	{0x3278, 0xA7},                                                        
	{0x3279, 0xBC},                                                        
	{0x327A, 0xDC},                                                        
	{0x327B, 0xF0},                                                        
	{0x327C, 0xFA},                                                        
	{0x327D, 0xFE},                                                        
	{0x327E, 0xFF},   
	{0x3060, 0x01},
};

static struct regval_list sensor_wb_fluorescent_regs[] = {
	{0x3270, 0x00},  //[Gamma_HDR2]
	{0x3271, 0x0A}, 
	{0x3272, 0x16}, 
	{0x3273, 0x30}, 
	{0x3274, 0x3F}, 
	{0x3275, 0x50}, 
	{0x3276, 0x6E}, 
	{0x3277, 0x88}, 
	{0x3278, 0xA0}, 
	{0x3279, 0xB3}, 
	{0x327A, 0xD2}, 
	{0x327B, 0xE8}, 
	{0x327C, 0xF5}, 
	{0x327D, 0xFF}, 
	{0x327E, 0xFF},
	{0x3060, 0x01}, 
};

static struct regval_list sensor_wb_tungsten_regs[] = {
    {0x3270, 0x00}, //[Gamma_MDR]
    {0x3271, 0x0D},
    {0x3272, 0x19},
    {0x3273, 0x2A},
    {0x3274, 0x3C},
    {0x3275, 0x4D},
    {0x3276, 0x67},
    {0x3277, 0x81},
    {0x3278, 0x98},
    {0x3279, 0xAD},
    {0x327A, 0xCE},
    {0x327B, 0xE0},
    {0x327C, 0xED},
    {0x327D, 0xF6},
    {0x327E, 0xFF},
    {0x3060, 0x01},  
};

static struct regval_list sensor_wb_horizon[] = { 
//null
};

static struct regval_list sensor_wb_daylight_regs[] = {
	{0x3270, 0x00},  //[PN_Gamma]
	{0x3271, 0x0B}, 
	{0x3272, 0x17},
	{0x3273, 0x2B},
	{0x3274, 0x3D},
	{0x3275, 0x4F},
	{0x3276, 0x68},
	{0x3277, 0x7F},
	{0x3278, 0x93},
	{0x3279, 0xA1},
	{0x327A, 0xB8},
	{0x327B, 0xC9},
	{0x327C, 0xDB},
	{0x327D, 0xED},
	{0x327E, 0xFF},
	{0x3060, 0x01}, 
};

static struct regval_list sensor_wb_flash[] = { 
//null
};


static struct regval_list sensor_wb_cloud_regs[] = {
	{0x3270, 0x00},  //[Gamma_9712_2]
	{0x3271, 0x08},  
	{0x3272, 0x13},  
	{0x3273, 0x2b},  
	{0x3274, 0x41},  
	{0x3275, 0x53},  
	{0x3276, 0x72},  
	{0x3277, 0x8b},  
	{0x3278, 0x9e},  
	{0x3279, 0xb3},  
	{0x327A, 0xcf},  
	{0x327B, 0xe2},  
	{0x327C, 0xef},  
	{0x327D, 0xF7},  
	{0x327E, 0xFF},
	{0x3060, 0x01},  
};

static struct regval_list sensor_wb_shade[] = { 
//null
};

#else
static struct regval_list sensor_wb_manual[] = { 
//null
};

static struct regval_list sensor_wb_auto_regs[] = {
    {0x3302, 0x00}, //[CC_R1]
    {0x3303, 0x5b}, 
    {0x3304, 0x00}, 
    {0x3305, 0x6c}, 
    {0x3306, 0x00}, 
    {0x3307, 0x3A}, 
    {0x3308, 0x07}, 
    {0x3309, 0xbf}, 
    {0x330A, 0x06}, 
    {0x330B, 0xf9}, 
    {0x330C, 0x01}, 
    {0x330D, 0x48}, 
    {0x330E, 0x01}, 
    {0x330F, 0x0b}, 
    {0x3310, 0x06}, 
    {0x3311, 0xfd}, 
    {0x3312, 0x07}, 
    {0x3313, 0xFb}, 
    {0x3060, 0x01},
};

static struct regval_list sensor_wb_incandescence_regs[] = {
	//[CC_Matrix_115%_LowContrast]                      
	                        
	{0x3302, 0x00},                                 
	{0x3303, 0x4C},                                 
	{0x3304, 0x00},                                 
	{0x3305, 0x9D},                                 
	{0x3306, 0x00},                                 
	{0x3307, 0x15},                                 
	{0x3308, 0x07},                                 
	{0x3309, 0xC5},                                 
	{0x330A, 0x07},                                 
	{0x330B, 0x19},                                 
	{0x330C, 0x01},                                 
	{0x330D, 0x23},                                 
	{0x330E, 0x00},                                 
	{0x330F, 0xEF},                                 
	{0x3310, 0x07},                                 
	{0x3311, 0x1F},                                 
	{0x3312, 0x07},                                 
	{0x3313, 0xF3},   
	{0x3060, 0x01},
};

static struct regval_list sensor_wb_fluorescent_regs[] = {
	//[CC_Iphone5_Gamma9712]                                                             
	                        
	{0x3302, 0x00},  
	{0x3303, 0x4C},  
	{0x3304, 0x00},  
	{0x3305, 0x96},  
	{0x3306, 0x00},  
	{0x3307, 0x1D},  
	{0x3308, 0x07},  
	{0x3309, 0xCC},  
	{0x330A, 0x07},  
	{0x330B, 0x32},  
	{0x330C, 0x01},  
	{0x330D, 0x02},  
	{0x330E, 0x00},  
	{0x330F, 0xEC},  
	{0x3310, 0x07},  
	{0x3311, 0x11},  
	{0x3312, 0x00},  
	{0x3313, 0x02},  
	{0x3060, 0x01}, 
};

static struct regval_list sensor_wb_tungsten_regs[] = {
	//[CC_Ricoh_GRD3_Gamma9712]                                      
	{0x3302, 0x00}, 
	{0x3303, 0x4C}, 
	{0x3304, 0x00}, 
	{0x3305, 0x96}, 
	{0x3306, 0x00}, 
	{0x3307, 0x1D}, 
	{0x3308, 0x07}, 
	{0x3309, 0xC6}, 
	{0x330A, 0x07}, 
	{0x330B, 0x2F}, 
	{0x330C, 0x01}, 
	{0x330D, 0x0B}, 
	{0x330E, 0x00}, 
	{0x330F, 0xEE}, 
	{0x3310, 0x07}, 
	{0x3311, 0x27}, 
	{0x3312, 0x07}, 
	{0x3313, 0xEC}, 
	{0x3060, 0x01},  
};

static struct regval_list sensor_wb_horizon[] = { 
//null
};

static struct regval_list sensor_wb_daylight_regs[] = {
	//[CC_Cannon_S100_Gamma9712]                                     
	{0x3302, 0x00}, 
	{0x3303, 0x4C}, 
	{0x3304, 0x00}, 
	{0x3305, 0x96}, 
	{0x3306, 0x00}, 
	{0x3307, 0x1D}, 
	{0x3308, 0x07}, 
	{0x3309, 0xCC}, 
	{0x330A, 0x07}, 
	{0x330B, 0x13}, 
	{0x330C, 0x01}, 
	{0x330D, 0x21}, 
	{0x330E, 0x00}, 
	{0x330F, 0xE3}, 
	{0x3310, 0x07}, 
	{0x3311, 0x22}, 
	{0x3312, 0x07}, 
	{0x3313, 0xFB}, 
	{0x3060, 0x01}, 
};

static struct regval_list sensor_wb_flash[] = { 
//null
};


static struct regval_list sensor_wb_cloud_regs[] = {
	{0x3270, 0x00},  //[Gamma_9712_2]
	{0x3271, 0x08},  
	{0x3272, 0x13},  
	{0x3273, 0x2b},  
	{0x3274, 0x41},  
	{0x3275, 0x53},  
	{0x3276, 0x72},  
	{0x3277, 0x8b},  
	{0x3278, 0x9e},  
	{0x3279, 0xb3},  
	{0x327A, 0xcf},  
	{0x327B, 0xe2},  
	{0x327C, 0xef},  
	{0x327D, 0xF7},  
	{0x327E, 0xFF},
	{0x3060, 0x01},  
};

static struct regval_list sensor_wb_shade[] = { 
//null
};


#endif


static struct cfg_array sensor_wb[] = {
  { 
  	.regs = sensor_wb_manual,             //V4L2_WHITE_BALANCE_MANUAL       
    .size = ARRAY_SIZE(sensor_wb_manual),
  },
  {
  	.regs = sensor_wb_auto_regs,          //V4L2_WHITE_BALANCE_AUTO      
    .size = ARRAY_SIZE(sensor_wb_auto_regs),
  },
  {
  	.regs = sensor_wb_incandescence_regs, //V4L2_WHITE_BALANCE_INCANDESCENT 
    .size = ARRAY_SIZE(sensor_wb_incandescence_regs),
  },
  {
  	.regs = sensor_wb_fluorescent_regs,   //V4L2_WHITE_BALANCE_FLUORESCENT  
    .size = ARRAY_SIZE(sensor_wb_fluorescent_regs),
  },
  {
  	.regs = sensor_wb_tungsten_regs,      //V4L2_WHITE_BALANCE_FLUORESCENT_H
    .size = ARRAY_SIZE(sensor_wb_tungsten_regs),
  },
  {
  	.regs = sensor_wb_horizon,            //V4L2_WHITE_BALANCE_HORIZON    
    .size = ARRAY_SIZE(sensor_wb_horizon),
  },
  {
  	.regs = sensor_wb_daylight_regs,      //V4L2_WHITE_BALANCE_DAYLIGHT     
    .size = ARRAY_SIZE(sensor_wb_daylight_regs),
  },
  {
  	.regs = sensor_wb_flash,              //V4L2_WHITE_BALANCE_FLASH        
    .size = ARRAY_SIZE(sensor_wb_flash),
  },
  {
  	.regs = sensor_wb_cloud_regs,         //V4L2_WHITE_BALANCE_CLOUDY       
    .size = ARRAY_SIZE(sensor_wb_cloud_regs),
  },
  {
  	.regs = sensor_wb_shade,              //V4L2_WHITE_BALANCE_SHADE  
    .size = ARRAY_SIZE(sensor_wb_shade),
  },
//  {
//  	.regs = NULL,
//    .size = 0,
//  },
};
                                          

/*
 * The color effect settings
 */
static struct regval_list sensor_colorfx_none_regs[] = {
	//sensor_Effect_Normal
	{0x32f1, 0x00}, 
};

static struct regval_list sensor_colorfx_bw_regs[] = {
	//sensor_Effect_WandB
	{0x32f1, 0x01},
};

static struct regval_list sensor_colorfx_sepia_regs[] = {
	//sensor_Effect_Sepia
	{0x32f1, 0x02},
};

static struct regval_list sensor_colorfx_negative_regs[] = {
	//sensor_Effect_Negative
	{0x32f1, 0x03},
};

static struct regval_list sensor_colorfx_emboss_regs[] = {
	//NULL
};

static struct regval_list sensor_colorfx_sketch_regs[] = {
	//NULL
};

static struct regval_list sensor_colorfx_sky_blue_regs[] = {
	//sensor_Effect_Bluish
	{0x32f1, 0x05},
	{0x32f4, 0xf0},
	{0x32f5, 0x80},
};

static struct regval_list sensor_colorfx_grass_green_regs[] = {
	//sensor_Effect_Green
	{0x32f1, 0x05},
	{0x32f4, 0x60},
	{0x32f5, 0x20},
};

static struct regval_list sensor_colorfx_skin_whiten_regs[] = {
//NULL
};

static struct regval_list sensor_colorfx_vivid_regs[] = {
//NULL
};

static struct regval_list sensor_colorfx_aqua_regs[] = {
//null
};

static struct regval_list sensor_colorfx_art_freeze_regs[] = {
//null
};

static struct regval_list sensor_colorfx_silhouette_regs[] = {
//null
};

static struct regval_list sensor_colorfx_solarization_regs[] = {

};

static struct regval_list sensor_colorfx_antique_regs[] = {
//null
};

static struct regval_list sensor_colorfx_set_cbcr_regs[] = {
//null
};

static struct cfg_array sensor_colorfx[] = {
  {
  	.regs = sensor_colorfx_none_regs,         //V4L2_COLORFX_NONE = 0,         
    .size = ARRAY_SIZE(sensor_colorfx_none_regs),
  },
  {
  	.regs = sensor_colorfx_bw_regs,           //V4L2_COLORFX_BW   = 1,  
    .size = ARRAY_SIZE(sensor_colorfx_bw_regs),
  },
  {
  	.regs = sensor_colorfx_sepia_regs,        //V4L2_COLORFX_SEPIA  = 2,   
    .size = ARRAY_SIZE(sensor_colorfx_sepia_regs),
  },
  {
  	.regs = sensor_colorfx_negative_regs,     //V4L2_COLORFX_NEGATIVE = 3,     
    .size = ARRAY_SIZE(sensor_colorfx_negative_regs),
  },
  {
  	.regs = sensor_colorfx_emboss_regs,       //V4L2_COLORFX_EMBOSS = 4,       
    .size = ARRAY_SIZE(sensor_colorfx_emboss_regs),
  },
  {
  	.regs = sensor_colorfx_sketch_regs,       //V4L2_COLORFX_SKETCH = 5,       
    .size = ARRAY_SIZE(sensor_colorfx_sketch_regs),
  },
  {
  	.regs = sensor_colorfx_sky_blue_regs,     //V4L2_COLORFX_SKY_BLUE = 6,     
    .size = ARRAY_SIZE(sensor_colorfx_sky_blue_regs),
  },
  {
  	.regs = sensor_colorfx_grass_green_regs,  //V4L2_COLORFX_GRASS_GREEN = 7,  
    .size = ARRAY_SIZE(sensor_colorfx_grass_green_regs),
  },
  {
  	.regs = sensor_colorfx_skin_whiten_regs,  //V4L2_COLORFX_SKIN_WHITEN = 8,  
    .size = ARRAY_SIZE(sensor_colorfx_skin_whiten_regs),
  },
  {
  	.regs = sensor_colorfx_vivid_regs,        //V4L2_COLORFX_VIVID = 9,        
    .size = ARRAY_SIZE(sensor_colorfx_vivid_regs),
  },
  {
  	.regs = sensor_colorfx_aqua_regs,         //V4L2_COLORFX_AQUA = 10,        
    .size = ARRAY_SIZE(sensor_colorfx_aqua_regs),
  },
  {
  	.regs = sensor_colorfx_art_freeze_regs,   //V4L2_COLORFX_ART_FREEZE = 11,  
    .size = ARRAY_SIZE(sensor_colorfx_art_freeze_regs),
  },
  {
  	.regs = sensor_colorfx_silhouette_regs,   //V4L2_COLORFX_SILHOUETTE = 12,  
    .size = ARRAY_SIZE(sensor_colorfx_silhouette_regs),
  },
  {
  	.regs = sensor_colorfx_solarization_regs, //V4L2_COLORFX_SOLARIZATION = 13,
    .size = ARRAY_SIZE(sensor_colorfx_solarization_regs),
  },
  {
  	.regs = sensor_colorfx_antique_regs,      //V4L2_COLORFX_ANTIQUE = 14,     
    .size = ARRAY_SIZE(sensor_colorfx_antique_regs),
  },
  {
  	.regs = sensor_colorfx_set_cbcr_regs,     //V4L2_COLORFX_SET_CBCR = 15, 
    .size = ARRAY_SIZE(sensor_colorfx_set_cbcr_regs),
  },
};



/*
 * The brightness setttings
 */
static struct regval_list sensor_brightness_neg4_regs[] = {
	// Brightness -4
	{0x32fc, 0x80},
};

static struct regval_list sensor_brightness_neg3_regs[] = {
	// Brightness -3
	{0x32fc, 0xa0},
};

static struct regval_list sensor_brightness_neg2_regs[] = {
	// Brightness -2
	{0x32fc, 0xc0},
};

static struct regval_list sensor_brightness_neg1_regs[] = {
	// Brightness -1
	{0x32fc, 0xe0},
};

static struct regval_list sensor_brightness_zero_regs[] = {
	//  Brightness 0
	{0x32fc, 0x00},
};

static struct regval_list sensor_brightness_pos1_regs[] = {
	// Brightness +1
	{0x32fc, 0x20},
};

static struct regval_list sensor_brightness_pos2_regs[] = {
	//  Brightness +2
	{0x32fc, 0x40},
};

static struct regval_list sensor_brightness_pos3_regs[] = {
	//  Brightness +3
	{0x32fc, 0x60},
};

static struct regval_list sensor_brightness_pos4_regs[] = {
	//  Brightness +4
	{0x32fc, 0x7f},
};

static struct cfg_array sensor_brightness[] = {
  {
  	.regs = sensor_brightness_neg4_regs,
  	.size = ARRAY_SIZE(sensor_brightness_neg4_regs),
  },
  {
  	.regs = sensor_brightness_neg3_regs,
  	.size = ARRAY_SIZE(sensor_brightness_neg3_regs),
  },
  {
  	.regs = sensor_brightness_neg2_regs,
  	.size = ARRAY_SIZE(sensor_brightness_neg2_regs),
  },
  {
  	.regs = sensor_brightness_neg1_regs,
  	.size = ARRAY_SIZE(sensor_brightness_neg1_regs),
  },
  {
  	.regs = sensor_brightness_zero_regs,
  	.size = ARRAY_SIZE(sensor_brightness_zero_regs),
  },
  {
  	.regs = sensor_brightness_pos1_regs,
  	.size = ARRAY_SIZE(sensor_brightness_pos1_regs),
  },
  {
  	.regs = sensor_brightness_pos2_regs,
  	.size = ARRAY_SIZE(sensor_brightness_pos2_regs),
  },
  {
  	.regs = sensor_brightness_pos3_regs,
  	.size = ARRAY_SIZE(sensor_brightness_pos3_regs),
  },
  {
  	.regs = sensor_brightness_pos4_regs,
  	.size = ARRAY_SIZE(sensor_brightness_pos4_regs),
  },
};

/*
 * The contrast setttings
 */
static struct regval_list sensor_contrast_neg4_regs[] = {
//NULL
};

static struct regval_list sensor_contrast_neg3_regs[] = {
//NULL
};

static struct regval_list sensor_contrast_neg2_regs[] = {
//NULL
};

static struct regval_list sensor_contrast_neg1_regs[] = {
//NULL
};

static struct regval_list sensor_contrast_zero_regs[] = {
//NULL
};

static struct regval_list sensor_contrast_pos1_regs[] = {
//NULL
};

static struct regval_list sensor_contrast_pos2_regs[] = {
//NULL
};

static struct regval_list sensor_contrast_pos3_regs[] = {
//NULL
};

static struct regval_list sensor_contrast_pos4_regs[] = {
//NULL
};

static struct cfg_array sensor_contrast[] = {
  {
  	.regs = sensor_contrast_neg4_regs,
  	.size = ARRAY_SIZE(sensor_contrast_neg4_regs),
  },
  {
  	.regs = sensor_contrast_neg3_regs,
  	.size = ARRAY_SIZE(sensor_contrast_neg3_regs),
  },
  {
  	.regs = sensor_contrast_neg2_regs,
  	.size = ARRAY_SIZE(sensor_contrast_neg2_regs),
  },
  {
  	.regs = sensor_contrast_neg1_regs,
  	.size = ARRAY_SIZE(sensor_contrast_neg1_regs),
  },
  {
  	.regs = sensor_contrast_zero_regs,
  	.size = ARRAY_SIZE(sensor_contrast_zero_regs),
  },
  {
  	.regs = sensor_contrast_pos1_regs,
  	.size = ARRAY_SIZE(sensor_contrast_pos1_regs),
  },
  {
  	.regs = sensor_contrast_pos2_regs,
  	.size = ARRAY_SIZE(sensor_contrast_pos2_regs),
  },
  {
  	.regs = sensor_contrast_pos3_regs,
  	.size = ARRAY_SIZE(sensor_contrast_pos3_regs),
  },
  {
  	.regs = sensor_contrast_pos4_regs,
  	.size = ARRAY_SIZE(sensor_contrast_pos4_regs),
  },
};

/*
 * The saturation setttings
 */
static struct regval_list sensor_saturation_neg4_regs[] = {
	//[SATURATION : -4]
	{0x32f1, 0x05},
	{0x32f3, 0x40},
};

static struct regval_list sensor_saturation_neg3_regs[] = {
	//[SATURATION : -3]
	{0x32f1, 0x05},
	{0x32f3, 0x50},
};

static struct regval_list sensor_saturation_neg2_regs[] = {
	//[SATURATION : -2]
	{0x32f1, 0x05},
	{0x32f3, 0x60},
};

static struct regval_list sensor_saturation_neg1_regs[] = {
	//[SATURATION : -1]
	{0x32f1, 0x05},
	{0x32f3, 0x70},
};

static struct regval_list sensor_saturation_zero_regs[] = {
	//[SATURATION : 0]
	{0x32f1, 0x05},
	{0x32f3, 0x80},
};

static struct regval_list sensor_saturation_pos1_regs[] = {
	//[SATURATION : +1]	
	{0x32f1, 0x05},
	{0x32f3, 0x90},
};

static struct regval_list sensor_saturation_pos2_regs[] = {
	//[SATURATION : +2]	
	{0x32f1, 0x05},
	{0x32f3, 0xA0},
};

static struct regval_list sensor_saturation_pos3_regs[] = {
	//[SATURATION : +3]	
	{0x32f1, 0x05},
	{0x32f3, 0xB0},
};

static struct regval_list sensor_saturation_pos4_regs[] = {
	//[SATURATION : +4]	
	{0x32f1, 0x05},
	{0x32f3, 0xC0},
};

static struct cfg_array sensor_saturation[] = {
  {
  	.regs = sensor_saturation_neg4_regs,
  	.size = ARRAY_SIZE(sensor_saturation_neg4_regs),
  },
  {
  	.regs = sensor_saturation_neg3_regs,
  	.size = ARRAY_SIZE(sensor_saturation_neg3_regs),
  },
  {
  	.regs = sensor_saturation_neg2_regs,
  	.size = ARRAY_SIZE(sensor_saturation_neg2_regs),
  },
  {
  	.regs = sensor_saturation_neg1_regs,
  	.size = ARRAY_SIZE(sensor_saturation_neg1_regs),
  },
  {
  	.regs = sensor_saturation_zero_regs,
  	.size = ARRAY_SIZE(sensor_saturation_zero_regs),
  },
  {
  	.regs = sensor_saturation_pos1_regs,
  	.size = ARRAY_SIZE(sensor_saturation_pos1_regs),
  },
  {
  	.regs = sensor_saturation_pos2_regs,
  	.size = ARRAY_SIZE(sensor_saturation_pos2_regs),
  },
  {
  	.regs = sensor_saturation_pos3_regs,
  	.size = ARRAY_SIZE(sensor_saturation_pos3_regs),
  },
  {
  	.regs = sensor_saturation_pos4_regs,
  	.size = ARRAY_SIZE(sensor_saturation_pos4_regs),
  },
};

/*
 * The exposure target setttings
 */
static struct regval_list sensor_ev_neg4_regs[] = {
	{0x32f2, 0x40},
};

static struct regval_list sensor_ev_neg3_regs[] = {
	{0x32f2, 0x50},
};

static struct regval_list sensor_ev_neg2_regs[] = {
	{0x32f2, 0x60},
};

static struct regval_list sensor_ev_neg1_regs[] = {
	{0x32f2, 0x70},
};                     

static struct regval_list sensor_ev_zero_regs[] = {
	//default
	{0x32f2, 0x80},
};

static struct regval_list sensor_ev_pos1_regs[] = {
	{0x32f2, 0x90},
};

static struct regval_list sensor_ev_pos2_regs[] = {
	{0x32f2, 0xa0},
};

static struct regval_list sensor_ev_pos3_regs[] = {
	{0x32f2, 0xb0},
};

static struct regval_list sensor_ev_pos4_regs[] = {
	{0x32f2, 0xc0},
};


static struct cfg_array sensor_ev[] = {
  {
  	.regs = sensor_ev_neg4_regs,
  	.size = ARRAY_SIZE(sensor_ev_neg4_regs),
  },
  {
  	.regs = sensor_ev_neg3_regs,
  	.size = ARRAY_SIZE(sensor_ev_neg3_regs),
  },
  {
  	.regs = sensor_ev_neg2_regs,
  	.size = ARRAY_SIZE(sensor_ev_neg2_regs),
  },
  {
  	.regs = sensor_ev_neg1_regs,
  	.size = ARRAY_SIZE(sensor_ev_neg1_regs),
  },
  {
  	.regs = sensor_ev_zero_regs,
  	.size = ARRAY_SIZE(sensor_ev_zero_regs),
  },
  {
  	.regs = sensor_ev_pos1_regs,
  	.size = ARRAY_SIZE(sensor_ev_pos1_regs),
  },
  {
  	.regs = sensor_ev_pos2_regs,
  	.size = ARRAY_SIZE(sensor_ev_pos2_regs),
  },
  {
  	.regs = sensor_ev_pos3_regs,
  	.size = ARRAY_SIZE(sensor_ev_pos3_regs),
  },
  {
  	.regs = sensor_ev_pos4_regs,
  	.size = ARRAY_SIZE(sensor_ev_pos4_regs),
  },
};

/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 * 
 */


static struct regval_list sensor_fmt_yuv422_yuyv[] = {	
	{0x32f0, 0x01},	//YCbYCr
};

static struct regval_list sensor_fmt_yuv422_yvyu[] = {	
	{0x32f0, 0x03},	//YCrYCb
};

static struct regval_list sensor_fmt_yuv422_vyuy[] = {	
	{0x32f0, 0x02},	//CrYCbY
};

static struct regval_list sensor_fmt_yuv422_uyvy[] = {	
	{0x32f0, 0x00},	//CbYCrY
};

static struct regval_list sensor_fmt_raw[] = {	
	{0x32f0, 0x50}, //raw
};


/*
 * Low-level register I/O.
 *
 */


/*
 * On most platforms, we'd rather do straight i2c I/O.
 */
static int sensor_read(struct v4l2_subdev *sd, unsigned short reg,
    unsigned char *value)
{
	int ret=0;
	int cnt=0;

  ret = cci_read_a16_d8(sd,reg,value);
  while(ret!=0&&cnt<2)
  {
  	ret = cci_read_a16_d8(sd,reg,value);
  	cnt++;
  }
  if(cnt>0)
  	vfe_dev_dbg("sensor read retry=%d\n",cnt);
  
  return ret;
}

static int sensor_write(struct v4l2_subdev *sd, unsigned short reg,
    unsigned char value)
{
	int ret=0;
	int cnt=0;
  ret = cci_write_a16_d8(sd,reg,value);
  while(ret!=0&&cnt<2)
  {
  	ret = cci_write_a16_d8(sd,reg,value);
  	cnt++;
  }
  if(cnt>0)
  	vfe_dev_dbg("sensor write retry=%d\n",cnt);
  
  return ret;
}

/*
 * Write a list of register settings;
 */
static int sensor_write_array(struct v4l2_subdev *sd, struct regval_list *regs, int array_size)
{
	int i=0;
	
	if(!regs)
  		return 0;
  	//return -EINVAL;

	while(i<array_size)
	{
		if(regs->addr == REG_DLY) {
			msleep(regs->data);
		} else {
			LOG_ERR_RET(sensor_write(sd, regs->addr, regs->data))
		}
		i++;
		regs++;
	}
	return 0;
}

static int sensor_g_hflip(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	unsigned char val;
	

	ret = sensor_read(sd, 0x3022, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_hflip!\n");
		return ret;
	}
  	vfe_dev_dbg("NT99230 sensor_read sensor_g_hflip(%x)\n",val);

	val &= (1<<1);
	val = val>>0;		//0x14 bit0 is mirror
		
	*value = val;

	info->hflip = *value;
	return 0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	unsigned char val;
	

	ret = sensor_read(sd, 0x3022, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_hflip!\n");
		return ret;
	}
	
	switch (value) {
		case 0:
		  val &= 0xfd;
			break;
		case 1:
			val |= 0x02;
			break;
		default:
			return -EINVAL;
	}
	ret = 0;//sensor_write(sd, 0x3022, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_hflip!\n");
		return ret;
	}
	
	mdelay(20);
	
	info->hflip = value;
	return 0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	unsigned char val;
	
	
	ret = sensor_read(sd, 0x3022, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_vflip!\n");
		return ret;
	}
  	vfe_dev_dbg("NT99230 sensor_read sensor_g_vflip(%x)\n",val);
	
	val &= 0x01;
		
	*value = val;

	info->vflip = *value;
	return 0;
}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	unsigned char val;
	
	ret = sensor_read(sd, 0x3022, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_vflip!\n");
		return ret;
	}
	
	switch (value) {
		case 0:
		  val &= 0x24;
			break;
		case 1:
			val |= 0x01;
			break;
		default:
			return -EINVAL;
	}
	ret = sensor_write(sd, 0x3022, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_vflip!\n");
		return ret;
	}
	
	mdelay(20);
	
	info->vflip = value;
	return 0;
}

static int sensor_g_autogain(struct v4l2_subdev *sd, __s32 *value)
{
	return -EINVAL;
}

static int sensor_s_autogain(struct v4l2_subdev *sd, int value)
{
	return -EINVAL;
}

static int sensor_g_autoexp(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	unsigned char val;
	

	ret = sensor_read(sd, 0x3201, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_autoexp!\n");
		return ret;
	}


	val = ((val & 0x20) >> 5);
	if (val == 0x01) {
		*value = V4L2_EXPOSURE_AUTO;
	}
	else
	{
		*value = V4L2_EXPOSURE_MANUAL;
	}
	
	info->autoexp = *value;
	return 0;
}

static int sensor_s_autoexp(struct v4l2_subdev *sd,
		enum v4l2_exposure_auto_type value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	unsigned char val;
	
	ret = sensor_read(sd, 0x3201, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autoexp!\n");
		return ret;
	}



	switch (value) {
		case V4L2_EXPOSURE_AUTO:
		  val |= 0x20;
			break;
		case V4L2_EXPOSURE_MANUAL:
			val &= 0xdf;
			break;
		case V4L2_EXPOSURE_SHUTTER_PRIORITY:
			return -EINVAL;    
		case V4L2_EXPOSURE_APERTURE_PRIORITY:
			return -EINVAL;
		default:
			return -EINVAL;
	}
		
	ret = sensor_write(sd, 0x3201, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autoexp!\n");
		return ret;
	}
	
	mdelay(20);
	
	info->autoexp = value;
	return 0;
}

static int sensor_g_autowb(struct v4l2_subdev *sd, int *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	unsigned char val;
	

	ret = sensor_read(sd, 0x3201, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_autowb!\n");
		return ret;
	}

       val= ((val & 0x10) >> 4);

		//0x22 bit1 is awb enable
		
	*value = val;
	info->autowb = *value;
	
	return 0;
}

static int sensor_s_autowb(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	unsigned char val;
	
	ret = sensor_write_array(sd, sensor_wb_auto_regs, ARRAY_SIZE(sensor_wb_auto_regs));
	if (ret < 0) {
		vfe_dev_err("sensor_write_array err at sensor_s_autowb!\n");
		return ret;
	}
	

	ret = sensor_read(sd, 0x3201, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autowb!\n");
		return ret;
	}



	switch(value) {
	case 0:
		val &= 0xef;
		break;
	case 1:
		val |= 0x10;
		break;
	default:
		break;
	}	
	ret = sensor_write(sd, 0x3201, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autowb!\n");
		return ret;
	}
	
	mdelay(10);
	
	info->autowb = value;
	return 0;
}

static int sensor_g_hue(struct v4l2_subdev *sd, __s32 *value)
{
	return -EINVAL;
}

static int sensor_s_hue(struct v4l2_subdev *sd, int value)
{
	return -EINVAL;
}

static int sensor_g_gain(struct v4l2_subdev *sd, __s32 *value)
{
/*	struct sensor_info *info = to_state(sd);
	
    *value = info->gain;
    vfe_dev_dbg("sensor_get_gain = %d\n", info->gain);
    return 0;*/
	return -EINVAL;
}

static int sensor_s_gain(struct v4l2_subdev *sd, int value)
{
	return -EINVAL;
}
/* *********************************************end of ******************************************** */

static int sensor_g_brightness(struct v4l2_subdev *sd, __s32 *value)
{
  struct sensor_info *info = to_state(sd);
  
  *value = info->brightness;
  return 0;
}

static int sensor_s_brightness(struct v4l2_subdev *sd, int value)
{
  struct sensor_info *info = to_state(sd);
  
  if(info->brightness == value)
    return 0;
  
  if(value < -4 || value > 4)
    return -ERANGE;
  
  LOG_ERR_RET(sensor_write_array(sd, sensor_brightness[value+4].regs, sensor_brightness[value+4].size))

  info->brightness = value;
  
  printk("nt99230 set brightness: %d\n", value);
  
  return 0;
}

static int sensor_g_contrast(struct v4l2_subdev *sd, __s32 *value)
{
  struct sensor_info *info = to_state(sd);
  
  *value = info->contrast;
  return 0;
}

static int sensor_s_contrast(struct v4l2_subdev *sd, int value)
{
  struct sensor_info *info = to_state(sd);
  
  if(info->contrast == value)
    return 0;
  
  if(value < -4 || value > 4)
    return -ERANGE;
    
  LOG_ERR_RET(sensor_write_array(sd, sensor_contrast[value+4].regs, sensor_contrast[value+4].size))
  
  info->contrast = value;
  return 0;
}

static int sensor_g_saturation(struct v4l2_subdev *sd, __s32 *value)
{
  struct sensor_info *info = to_state(sd);
  
  *value = info->saturation;
  return 0;
}

static int sensor_s_saturation(struct v4l2_subdev *sd, int value)
{
  struct sensor_info *info = to_state(sd);
  
  if(info->saturation == value)
    return 0;

  if(value < -4 || value > 4)
    return -ERANGE;
      
  LOG_ERR_RET(sensor_write_array(sd, sensor_saturation[value+4].regs, sensor_saturation[value+4].size))

  info->saturation = value;
  return 0;
}

static int sensor_g_exp_bias(struct v4l2_subdev *sd, __s32 *value)
{
  struct sensor_info *info = to_state(sd);
  
  *value = info->exp_bias;
  return 0;
}

static int sensor_s_exp_bias(struct v4l2_subdev *sd, int value)
{
  struct sensor_info *info = to_state(sd);

  if(info->exp_bias == value)
    return 0;

  if(value < -4 || value > 4)
    return -ERANGE;
      
  LOG_ERR_RET(sensor_write_array(sd, sensor_ev[value+4].regs, sensor_ev[value+4].size))

  info->exp_bias = value;
  return 0;
}

static int sensor_g_wb(struct v4l2_subdev *sd, int *value)
{
  struct sensor_info *info = to_state(sd);
  enum v4l2_auto_n_preset_white_balance *wb_type = (enum v4l2_auto_n_preset_white_balance*)value;
  
  *wb_type = info->wb;
  
  return 0;
}

static int sensor_s_wb(struct v4l2_subdev *sd,
    enum v4l2_auto_n_preset_white_balance value)
{
  struct sensor_info *info = to_state(sd);
  
  if(info->capture_mode == V4L2_MODE_IMAGE)
    return 0;
  
  if(info->wb == value)
    return 0;
  
  LOG_ERR_RET(sensor_write_array(sd, sensor_wb[value].regs ,sensor_wb[value].size) )
  
  if (value == V4L2_WHITE_BALANCE_AUTO) 
    info->autowb = 1;
  else
    info->autowb = 0;
	
	info->wb = value;
	return 0;
}

static int sensor_g_colorfx(struct v4l2_subdev *sd,
		__s32 *value)
{
	struct sensor_info *info = to_state(sd);
	enum v4l2_colorfx *clrfx_type = (enum v4l2_colorfx*)value;
	
	*clrfx_type = info->clrfx;
	return 0;
}

static int sensor_s_colorfx(struct v4l2_subdev *sd,
    enum v4l2_colorfx value)
{
  struct sensor_info *info = to_state(sd);

  if(info->clrfx == value)
    return 0;
  
  LOG_ERR_RET(sensor_write_array(sd, sensor_colorfx[value].regs, sensor_colorfx[value].size))

  info->clrfx = value;
  return 0;
}

static int sensor_g_flash_mode(struct v4l2_subdev *sd,
    __s32 *value)
{
  struct sensor_info *info = to_state(sd);
  enum v4l2_flash_led_mode *flash_mode = (enum v4l2_flash_led_mode*)value;
  
  *flash_mode = info->flash_mode;
  return 0;
}

static int sensor_s_flash_mode(struct v4l2_subdev *sd,
    enum v4l2_flash_led_mode value)
{
  struct sensor_info *info = to_state(sd);
//  struct vfe_dev *dev=(struct vfe_dev *)dev_get_drvdata(sd->v4l2_dev->dev);
//  int flash_on,flash_off;
//  
//  flash_on = (dev->flash_pol!=0)?1:0;
//  flash_off = (flash_on==1)?0:1;
//  
//  switch (value) {
//  case V4L2_FLASH_MODE_OFF:
//    os_gpio_write(&dev->flash_io,flash_off);
//    break;
//  case V4L2_FLASH_MODE_AUTO:
//    return -EINVAL;
//    break;  
//  case V4L2_FLASH_MODE_ON:
//    os_gpio_write(&dev->flash_io,flash_on);
//    break;   
//  case V4L2_FLASH_MODE_TORCH:
//    return -EINVAL;
//    break;
//  case V4L2_FLASH_MODE_RED_EYE:   
//    return -EINVAL;
//    break;
//  default:
//    return -EINVAL;
//  }
  
  info->flash_mode = value;
  return 0;
}

//static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
//{
//	int ret=0;
////	unsigned char rdval;
////	
////	ret=sensor_read(sd, 0x00, &rdval);
////	if(ret!=0)
////		return ret;
////	
////	if(on_off==CSI_STBY_ON)//sw stby on
////	{
////		ret=sensor_write(sd, 0x00, rdval&0x7f);
////	}
////	else//sw stby off
////	{
////		ret=sensor_write(sd, 0x00, rdval|0x80);
////	}
//	return ret;
//}

/*
 * Stuff that knows about the sensor.
 */
 
static int sensor_power(struct v4l2_subdev *sd, int on)
{
  int ret = 0;

//    ret = 0;
	printk("%s:%d==========on:%d into\n",__func__,__LINE__,on);
  //make sure that no device can access i2c bus during sensor initial or power down
  //when using i2c_lock_adpater function, the following codes must not access i2c bus before calling i2c_unlock_adapter
  cci_lock(sd);

  //insure that clk_disable() and clk_enable() are called in pair
  //when calling CSI_SUBDEV_STBY_ON/OFF and CSI_SUBDEV_PWR_ON/OFF
  switch(on)
  {
    case CSI_SUBDEV_STBY_ON:
      vfe_dev_dbg("CSI_SUBDEV_STBY_ON\n");

     vfe_gpio_write(sd,RESET,CSI_RST_OFF);
     mdelay(10);
      //standby on io
      vfe_gpio_write(sd,PWDN,CSI_STBY_ON);
      mdelay(30);
      vfe_gpio_write(sd,PWDN,CSI_STBY_OFF);
      mdelay(10);
      vfe_gpio_write(sd,PWDN,CSI_STBY_ON);
      mdelay(30);
      //inactive mclk after stadby in
      vfe_set_mclk(sd,OFF);
      break;
    case CSI_SUBDEV_STBY_OFF:
      vfe_dev_dbg("CSI_SUBDEV_STBY_OFF\n");
      //active mclk before stadby out
      vfe_set_mclk_freq(sd,MCLK);
      vfe_set_mclk(sd,ON);
      mdelay(30);
      //standby off io
      vfe_gpio_write(sd,PWDN,CSI_STBY_OFF);
      mdelay(10);
     vfe_gpio_write(sd,RESET,CSI_RST_OFF);
     mdelay(10);
      vfe_gpio_write(sd,RESET,CSI_RST_ON);
      mdelay(30);
    vfe_gpio_write(sd,RESET,CSI_RST_OFF);
     mdelay(10);
 

//			//reset off io
//			csi_gpio_write(sd,&dev->reset_io,CSI_RST_OFF);
//			mdelay(30);
      break;
    case CSI_SUBDEV_PWR_ON:
      vfe_dev_dbg("CSI_SUBDEV_PWR_ON\n");
      //power on reset
      vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
      vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
      mdelay(10);
      //standby off io
      vfe_gpio_write(sd,PWDN,CSI_STBY_ON);
			//reset on io
      vfe_gpio_write(sd,RESET,CSI_RST_ON);

      //power supply
      vfe_gpio_write(sd,POWER_EN,CSI_PWR_ON);
      vfe_set_pmu_channel(sd,IOVDD,ON);
      vfe_set_pmu_channel(sd,AVDD,ON);
      vfe_set_pmu_channel(sd,DVDD,ON);
      vfe_set_pmu_channel(sd,AFVDD,ON);
      mdelay(20);
      //standby off io
      vfe_gpio_write(sd,PWDN,CSI_STBY_OFF);
      mdelay(10);
     //active mclk
      vfe_set_mclk_freq(sd,MCLK);
      vfe_set_mclk(sd,ON);
      mdelay(10);

      vfe_gpio_write(sd,RESET,CSI_RST_OFF);
      mdelay(30);
      //reset on io
      vfe_gpio_write(sd,RESET,CSI_RST_ON);
      mdelay(30);
     //reset off io
      vfe_gpio_write(sd,RESET,CSI_RST_OFF);
      mdelay(30);
	break;
    case CSI_SUBDEV_PWR_OFF:
      vfe_dev_dbg("CSI_SUBDEV_PWR_OFF\n");
      //reset io
      mdelay(10);
      vfe_gpio_write(sd,RESET,CSI_RST_ON);
			mdelay(10);
			//inactive mclk after power off
      vfe_set_mclk(sd,OFF);
      //power supply off
      vfe_gpio_write(sd,POWER_EN,CSI_PWR_OFF);
      vfe_set_pmu_channel(sd,AFVDD,OFF);
      vfe_set_pmu_channel(sd,DVDD,OFF);
      vfe_set_pmu_channel(sd,AVDD,OFF);
      vfe_set_pmu_channel(sd,IOVDD,OFF);  
      mdelay(10);
      //standby and reset io
			//standby of io
      vfe_gpio_write(sd,PWDN,CSI_STBY_ON);
      mdelay(10);
      //set the io to hi-z
      vfe_gpio_set_status(sd,RESET,0);//set the gpio to input
      vfe_gpio_set_status(sd,PWDN,0);//set the gpio to input
			break;
		default:
			return -EINVAL;
	}		

	//remember to unlock i2c adapter, so the device can access the i2c bus again
	cci_unlock(sd);
	return 0;
}
 
static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
    printk("%s:%==========val:%d into\n",__func__,__LINE__,val);
  switch(val)
  {
    case 0:
      vfe_gpio_write(sd,RESET,CSI_RST_OFF);
      mdelay(10);
      break;
    case 1:
      vfe_gpio_write(sd,RESET,CSI_RST_ON);
      mdelay(10);
      break;
    default:
      return -EINVAL;
  }
    
  return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	int ret;
	unsigned char val;
	unsigned   int SENSOR_ID=0;
	

//	printk("###init into %s:%d\n",__func__,__LINE__);
	//	return 0;

	ret = sensor_read(sd, 0x3000, &val);
//	printk("nt99230_SENSOR_ID=%x\n",val);	
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}

    printk("[%s:%d]>>>>>>nt99230_sensor_id = %x<<<<<<\n",__func__,__LINE__,val);	
	SENSOR_ID|= (val<< 8);

	ret = sensor_read(sd, 0x3001, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}

	return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	int i =0 ;
    printk("[%s:%d]>>>into<<<\n",__func__,__LINE__);
	
	/*Make sure it is a target sensor*/
	for(i=0;i<3;i++){
		ret = sensor_detect(sd);

		if (ret) {
			vfe_dev_err("chip found is not an target chip.\n");
			return ret;
		}else
	   		break;
		msleep(100);
	}
	return sensor_write_array(sd, sensor_default_regs , ARRAY_SIZE(sensor_default_regs));
		
	//return sensor_write_array(sd, sensor_FHD_regs , ARRAY_SIZE(sensor_FHD_regss));

}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret=0;
    printk("[%s:%d]>>>into<<<\n",__func__,__LINE__);
		return ret;
}


/*
 * Store information about the video data format. 
 */
static struct sensor_format_struct {
	__u8 *desc;
	//__u32 pixelformat;
	enum v4l2_mbus_pixelcode mbus_code;//linux-3.0
	struct regval_list *regs;
	int	regs_size;
	int bpp;   /* Bytes per pixel */
} sensor_formats[] = {
	{
		.desc		= "YUYV 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_YUYV8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_yuyv,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_yuyv),
		.bpp		= 2,
	},
	{
		.desc		= "YVYU 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_YVYU8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_yvyu,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_yvyu),
		.bpp		= 2,
	},
	{
		.desc		= "UYVY 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_UYVY8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_uyvy,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_uyvy),
		.bpp		= 2,
	},
	{
		.desc		= "VYUY 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_VYUY8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_vyuy,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_vyuy),
		.bpp		= 2,
	},
	{
		.desc		= "Raw RGB Bayer",
		.mbus_code	= V4L2_MBUS_FMT_SBGGR8_1X8,//linux-3.0
		.regs 		= sensor_fmt_raw,
		.regs_size = ARRAY_SIZE(sensor_fmt_raw),
		.bpp		= 1
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)


/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */


static struct sensor_win_size 
sensor_win_sizes[] = {
 /* FHD */
  {
    .width      = HD1080_WIDTH,
    .height     = HD1080_HEIGHT,
    .hoffset    = 0,
    .voffset    = 0,
    .regs       = sensor_FHD_regs,
    .regs_size  = ARRAY_SIZE(sensor_FHD_regs),
    .set_size   = NULL,
  },
  /* 720p */
  {
    .width      = HD720_WIDTH,
    .height     = HD720_HEIGHT,
    .hoffset    = 0,
    .voffset    = 0,
    .regs       = sensor_720P_regs,
    .regs_size  = ARRAY_SIZE(sensor_720P_regs),
    .set_size   = NULL,
  },/*
  // XGA 
   {
    .width      = XGA_WIDTH,
    .height     = XGA_HEIGHT,
    .hoffset    = 0,
    .voffset    = 0,
    .regs       = sensor_720P_regs,
    .regs_size  = ARRAY_SIZE(sensor_720P_regs),
    .set_size   = NULL,
  },
  // SVGA 
  {
    .width      = SVGA_WIDTH,
    .height     = SVGA_HEIGHT,
    .hoffset    = 0,
    .voffset    = 0,
    .regs       = sensor_720P_regs,
    .regs_size  = ARRAY_SIZE(sensor_720P_regs),
    .set_size   = NULL,
  },
  // VGA 
  {
    .width      = VGA_WIDTH,
    .height     = VGA_HEIGHT,
    .hoffset    = 0,
    .voffset    = 0,
    .regs       = sensor_720P_regs,
    .regs_size  = ARRAY_SIZE(sensor_720P_regs),
    .set_size   = NULL,
  }, */
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
  if(fsize->index > N_WIN_SIZES-1)
  	return -EINVAL;
  
  fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
  
  fsize->discrete.width = sensor_win_sizes[fsize->index].width;
  fsize->discrete.height = sensor_win_sizes[fsize->index].height;
  printk("[%s:%d]^^^^^^%d, %d^^^^^^\n",__func__,__LINE__,fsize->discrete.width, fsize->discrete.height);
  return 0;
}


static int sensor_try_fmt_internal(struct v4l2_subdev *sd,
    struct v4l2_mbus_framefmt *fmt,
    struct sensor_format_struct **ret_fmt,
    struct sensor_win_size **ret_wsize)
{
  int index;
  struct sensor_win_size *wsize;

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
  printk("[%s:%d]^^^^^^%d, %d^^^^^^\n",__func__,__LINE__,fmt->width, fmt->height);
  //pix->bytesperline = pix->width*sensor_formats[index].bpp;
  //pix->sizeimage = pix->height*pix->bytesperline;

  return 0;
}

static int sensor_try_fmt(struct v4l2_subdev *sd, 
             struct v4l2_mbus_framefmt *fmt)//linux-3.0
{
	return sensor_try_fmt_internal(sd, fmt, NULL, NULL);
}

static int sensor_g_mbus_config(struct v4l2_subdev *sd,
           struct v4l2_mbus_config *cfg)
{
  cfg->type = V4L2_MBUS_PARALLEL;
  cfg->flags = V4L2_MBUS_MASTER | VREF_POL | HREF_POL | CLK_POL ;
  
  return 0;
}

/*
 * Set a format.
 */
static int sensor_s_fmt(struct v4l2_subdev *sd, 
             struct v4l2_mbus_framefmt *fmt)//linux-3.0
{
	int ret;
	unsigned char val;
	struct sensor_format_struct *sensor_fmt;
	struct sensor_win_size *wsize;
	struct sensor_info *info = to_state(sd);
	vfe_dev_dbg("sensor_s_fmt\n");
	ret = sensor_try_fmt_internal(sd, fmt, &sensor_fmt, &wsize);
	if (ret)
		return ret;
	
		
	sensor_write_array(sd, sensor_fmt->regs , sensor_fmt->regs_size);
	
	ret = 0;
	if (wsize->regs)
	{
		ret = sensor_write_array(sd, wsize->regs , wsize->regs_size);
		if (ret < 0)
			return ret;
	}

	/*ret = sensor_read(sd, 0x3201, &val);  // add
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}
	printk("------0x3201 val = %x ----\n",val);
	printk("----wsize->width = %d, wsize->height = %d -----\n",wsize->width,wsize->height); // add
	*/
	msleep(250); // add 250	

	if (wsize->set_size)
	{
		ret = wsize->set_size(sd);
		if (ret < 0)
			return ret;
	}
	
	info->fmt = sensor_fmt;
	info->width = wsize->width;
	info->height = wsize->height;
	printk("[%s:%d]---width = %d,height = %d ----\n",__func__,__LINE__,wsize->width,wsize->height);
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
	
	cp->timeperframe.numerator = info->tpf.numerator;
	cp->timeperframe.denominator = info->tpf.denominator;
	printk("[sensor_g_parm]---info->tpf.denominator=%d\n", info->tpf.denominator);
	 
	return 0;
}

static int sensor_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
  struct v4l2_captureparm *cp = &parms->parm.capture;
  struct v4l2_fract *tpf = &cp->timeperframe;
  struct sensor_info *info = to_state(sd);
  unsigned char div;
  int fps = 0;
  
//  printk("sensor_s_parm\n");
  printk("[%s:%d]>>>>>>((([%d])))<<<<<<\n",__func__,__LINE__,tpf->denominator);

  fps = tpf->denominator;

  	switch(fps)
	{
		case 30:
			printk("switch fps to 30.\n");
			//sensor_write_array(sd, sensor_FHD_regs, ARRAY_SIZE(sensor_FHD_regs));
			sensor_win_sizes[1].regs = sensor_FHD_regs;
			sensor_win_sizes[1].regs_size = ARRAY_SIZE(sensor_FHD_regs);
			break;
		case 25:
		case 60:
			printk("switch fps to 25.\n");
			//sensor_write_array(sd, sensor_720P_regs, ARRAY_SIZE(sensor_720P_regs));
			sensor_win_sizes[1].regs = sensor_720P_regs;
			sensor_win_sizes[1].regs_size = ARRAY_SIZE(sensor_720P_regs);
			break;
		default:
			printk("switch fps to nothing.\n");
			break;

	}
/*  
  if (parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE){
  	printk("parms->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE\n");
    return -EINVAL;
  }
  printk("[King]------------info->tpf.denominator is %d\n", info->tpf.denominator);
  if (info->tpf.numerator == 0){
  	printk("info->tpf.numerator == 0\n");
    return -EINVAL;
  }
    
  info->capture_mode = cp->capturemode;
  
  if (info->capture_mode == V4L2_MODE_IMAGE) {
    printk("capture mode is not video mode,can not set frame rate!\n");
    return 0;
  }
    
  if (tpf->numerator == 0 || tpf->denominator == 0) {
    tpf->numerator = 1;
    tpf->denominator = SENSOR_FRAME_RATE;
    printk("sensor frame rate reset to full rate!\n");
  }
  
  div = SENSOR_FRAME_RATE/(tpf->denominator/tpf->numerator);
  if(div > 15 || div == 0)
  {
  	printk("SENSOR_FRAME_RATE=%d\n",SENSOR_FRAME_RATE);
  	printk("tpf->denominator=%d\n",tpf->denominator);
  	printk("tpf->numerator=%d\n",tpf->numerator);
    return -EINVAL;
  }
  
  printk("set frame rate %d\n",tpf->denominator/tpf->numerator);
  
  info->tpf.denominator = SENSOR_FRAME_RATE; 
  info->tpf.numerator = div;
  
	if(info->tpf.denominator/info->tpf.numerator < 30)
		info->low_speed = 1;
*/    
  return 0;
}


/* 
 * Code for dealing with controls.
 * fill with different sensor module
 * different sensor module has different settings here
 * if not support the follow function ,retrun -EINVAL
 */

/* *********************************************begin of ******************************************** */
static int sensor_queryctrl(struct v4l2_subdev *sd,
		struct v4l2_queryctrl *qc)
{
	/* Fill in min, max, step and default value for these controls. */
	/* see include/linux/videodev2.h for details */
	/* see sensor_s_parm and sensor_g_parm for the meaning of value */
	
	switch (qc->id) {
	case V4L2_CID_BRIGHTNESS:
		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
	case V4L2_CID_CONTRAST:
		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
	case V4L2_CID_SATURATION:
		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
	case V4L2_CID_HUE:
		return v4l2_ctrl_query_fill(qc, -180, 180, 5, 0);
	case V4L2_CID_VFLIP:
	case V4L2_CID_HFLIP:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);
	// case V4L2_CID_GAIN:
		// return v4l2_ctrl_query_fill(qc, 0, 255, 1, 128);
	// case V4L2_CID_AUTOGAIN:
		// return v4l2_ctrl_query_fill(qc, 0, 32, 1, 1);
	case V4L2_CID_EXPOSURE:
  case V4L2_CID_AUTO_EXPOSURE_BIAS:
		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 0);
	case V4L2_CID_EXPOSURE_AUTO:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);
  case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
    return v4l2_ctrl_query_fill(qc, 0, 9, 1, 1);
	case V4L2_CID_AUTO_WHITE_BALANCE:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 1);
	case V4L2_CID_COLORFX:
    return v4l2_ctrl_query_fill(qc, 0, 15, 1, 0);
  case V4L2_CID_FLASH_LED_MODE:
	  return v4l2_ctrl_query_fill(qc, 0, 4, 1, 0);	
	}
	return -EINVAL;
}


static int sensor_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		return sensor_g_brightness(sd, &ctrl->value);
	case V4L2_CID_CONTRAST:
		return sensor_g_contrast(sd, &ctrl->value);
	case V4L2_CID_SATURATION:
		return sensor_g_saturation(sd, &ctrl->value);
	case V4L2_CID_HUE:
		return sensor_g_hue(sd, &ctrl->value);	
	case V4L2_CID_VFLIP:
		return sensor_g_vflip(sd, &ctrl->value);
	case V4L2_CID_HFLIP:
		return sensor_g_hflip(sd, &ctrl->value);
	// case V4L2_CID_GAIN:
		// return sensor_g_gain(sd, &ctrl->value);
	// case V4L2_CID_AUTOGAIN:
		// return sensor_g_autogain(sd, &ctrl->value);
	case V4L2_CID_EXPOSURE:
  case V4L2_CID_AUTO_EXPOSURE_BIAS:
    return sensor_g_exp_bias(sd, &ctrl->value);
  case V4L2_CID_EXPOSURE_AUTO:
    return sensor_g_autoexp(sd, &ctrl->value);
  case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
    return sensor_g_wb(sd, &ctrl->value);
  case V4L2_CID_AUTO_WHITE_BALANCE:
    return sensor_g_autowb(sd, &ctrl->value);
  case V4L2_CID_COLORFX:
    return sensor_g_colorfx(sd, &ctrl->value);
  case V4L2_CID_FLASH_LED_MODE:
    return sensor_g_flash_mode(sd, &ctrl->value);
	}
	return -EINVAL;
}

static int sensor_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
  struct v4l2_queryctrl qc;
  int ret;
  
//  vfe_dev_dbg("sensor_s_ctrl ctrl->id=0x%8x\n", ctrl->id);
  qc.id = ctrl->id;
  ret = sensor_queryctrl(sd, &qc);
  if (ret < 0) {
    return ret;
  }

	if (qc.type == V4L2_CTRL_TYPE_MENU ||
		qc.type == V4L2_CTRL_TYPE_INTEGER ||
		qc.type == V4L2_CTRL_TYPE_BOOLEAN)
	{
	  if (ctrl->value < qc.minimum || ctrl->value > qc.maximum) {
	    return -ERANGE;
	  }
	}
	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		return sensor_s_brightness(sd, ctrl->value);
	case V4L2_CID_CONTRAST:
		return sensor_s_contrast(sd, ctrl->value);
	case V4L2_CID_SATURATION:
		return sensor_s_saturation(sd, ctrl->value);
	case V4L2_CID_HUE:
		return sensor_s_hue(sd, ctrl->value);		
	case V4L2_CID_VFLIP:
		return sensor_s_vflip(sd, ctrl->value);
	case V4L2_CID_HFLIP:
		return sensor_s_hflip(sd, ctrl->value);
	case V4L2_CID_GAIN:
		return sensor_s_gain(sd, ctrl->value);
	case V4L2_CID_AUTOGAIN:
		return sensor_s_autogain(sd, ctrl->value);
	case V4L2_CID_EXPOSURE:
    case V4L2_CID_AUTO_EXPOSURE_BIAS:
      return sensor_s_exp_bias(sd, ctrl->value);
    case V4L2_CID_EXPOSURE_AUTO:
      return sensor_s_autoexp(sd,
          (enum v4l2_exposure_auto_type) ctrl->value);
    case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
  		return sensor_s_wb(sd,
          (enum v4l2_auto_n_preset_white_balance) ctrl->value); 
    case V4L2_CID_AUTO_WHITE_BALANCE:
      return sensor_s_autowb(sd, ctrl->value);
    case V4L2_CID_COLORFX:
      return sensor_s_colorfx(sd,
          (enum v4l2_colorfx) ctrl->value);
    case V4L2_CID_FLASH_LED_MODE:
      return sensor_s_flash_mode(sd,
          (enum v4l2_flash_led_mode) ctrl->value);
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
};

static int sensor_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct sensor_info *info;
    printk("[%s:%d]>>>>>>into<<<<<<\n",__func__,__LINE__);

	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	sd = &info->sd;
	cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv);

	info->fmt = &sensor_formats[0];
	// info->width = HD1080_WIDTH;
    // info->height = HD1080_HEIGHT;
	info->brightness = 1;
	info->contrast = 1;
	info->saturation = 1;
	info->hue = 0;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 1;
	info->autogain = 1;
	info->exp = 0;
	info->autoexp = 1;
	info->autowb = 1;
	info->wb = 1;
	info->clrfx = 1;

//	nt99230_sd = sd;
	printk("[%s:%d]>>>>>>into<<<<<<\n",__func__,__LINE__);
	return 0;
}


static int sensor_remove(struct i2c_client *client)
{
  struct v4l2_subdev *sd;

  sd = cci_dev_remove_helper(client, &cci_drv);
  kfree(to_state(sd));
  return 0;
}

static const struct i2c_device_id sensor_id[] = {
  { SENSOR_NAME, 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, sensor_id);

//linux-3.0
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
	printk("###%s:%d into \n",__func__,__LINE__);
	return cci_dev_init_helper(&sensor_driver);
}

static __exit void exit_sensor(void)
{
	cci_dev_exit_helper(&sensor_driver);
}

module_init(init_sensor);
module_exit(exit_sensor);
