/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
*
* Copyright (c) 2011
*
* ChangeLog
*
*
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/keyboard.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/timer.h> 
#include <linux/clk.h>
#include <mach/sys_config.h>
#undef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_ARCH_SUN9IW1P1
#include <linux/clk/clk-sun9iw1.h>
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#if defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_PM)
#include <linux/pm.h>
#endif
#include "sun8i-keyboard.h"
#include <linux/power/scenelock.h> 

#include "../../misc/hyperion/ht_syscfg.h"

#if defined(CONFIG_IIO) || defined(CONFIG_IIO_MODULE)
#define HAVE_IIO	1
#else
#undef HAVE_IIO
#endif

#ifdef HAVE_IIO
#include "../../staging/iio/iio.h"
#include "../../staging/iio/events.h"
#include "../../staging/iio/ring_sw.h"
#include "../../staging/iio/trigger_consumer.h"
#endif

#ifdef MODE_0V2
#ifndef CONFIG_ARCH_SUN9IW1P1
static unsigned char keypad_mapindex[64] = {
	0,0,0,0,0,0,0,0,0,0,0,          /* key 1, 8���� 0-10 */
	1,1,1,1,1,1,1,1,1,1,            /* key 2, 7���� 11-20 */
	2,2,2,               			/* key 3, 7���� 21-23 */
	3,3,3,3,3,3,3,             		/* key 4, 6���� 24-30 */
	4,4,4,4,              			/* key 5, 6���� 31-34 */
	5,5,5,5,5,5,5,5,5,             	/* key 6, 6���� 35-43 */
	6,6,6,6,6,6,6,6,6,6,           	/* key 7, 10����44-53 */
	7,7,7,7,7,7,7,7,7,7,	  		/* key 8, 17����54-63 */
};
#else
static unsigned char keypad_mapindex[64] = {
	0,0,0,0,0,0,0,0,0,            	/* key 1, 0-8 */
	1,1,1,1,1,                 	/* key 2, 9-13 */
	2,2,2,2,2,2,                 	/* key 3, 14-19 */
	3,3,3,3,3,3,                   	/* key 4, 20-25 */
	4,4,4,4,4,4,4,4,4,4,4,          /* key 5, 26-36 */
	5,5,5,5,5,5,5,5,5,5,5,          /* key 6, 37-39 */
	6,6,6,6,6,6,6,6,6,           	/* key 7, 40-49 */
	7,7,7,7,7,7,7  			/* key 8, 50-63 */
};
#endif
#endif
                        
#ifdef MODE_0V15
/* 0.15V mode */
static unsigned char keypad_mapindex[64] = {
	0,0,0,    			/* key1 */
	1,1,1,1,1,                     	/* key2 */
	2,2,2,2,2,
	3,3,3,3,
	4,4,4,4,4,
	5,5,5,5,5,
	6,6,6,6,6,
	7,7,7,7,
	8,8,8,8,8,
	9,9,9,9,9,
	10,10,10,10,
	11,11,11,11,
	12,12,12,12,12,12,12,12,12,12  	/*key13 */
};
#endif

struct lradc_data {
	u32	mode;
	u32	rate;
	u32	first_delay;

#ifdef HAVE_IIO
	struct iio_dev *indio_dev;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

#ifdef CONFIG_PM
static struct dev_pm_domain keyboard_pm_domain;
#endif

#if defined(CONFIG_ARCH_SUN8IW6P1)||defined(CONFIG_ARCH_SUN9IW1P1)
#define ADC_MEASURE	(1350)
#define ADC_RESOL 	(21)
#else
#define ADC_MEASURE	(2000)
#define ADC_RESOL 	(31)
#endif
#define VOL_NUM 16

static int key_vol[VOL_NUM];
static int key_num = 0;
static volatile u32 key_val;
static struct input_dev *sunxikbd_dev;
static unsigned char scancode;

static unsigned char key_cnt = 0;
static unsigned char compare_buffer[REPORT_START_NUM] = {0};
static unsigned char transfer_code = INITIAL_VALUE;
#ifdef CONFIG_ARCH_SUN9IW1P1
static struct clk *key_clk;
static struct clk *key_clk_source;
#endif
enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_INT = 1U << 1,
	DEBUG_DATA_INFO = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
};
static u32 debug_mask = 0;
#define dprintk(level_mask, fmt, arg...)	if (unlikely(debug_mask & level_mask)) \
	printk(fmt , ## arg)

module_param_named(debug_mask, debug_mask, int, 0644);


#ifdef CONFIG_HAS_EARLYSUSPEND
static struct sunxi_keyboard_data *keyboard_data;
#endif

#ifdef CONFIG_ARCH_SUN9IW1P1
static void sunxikbd_clk_cfg(void)
{

	unsigned long rate = 0; /* 3Mhz */

	key_clk_source = clk_get(NULL, APB0_CLK);
	if (!key_clk_source || IS_ERR(key_clk_source)) {
		pr_err("try to get key_clk_source failed!\n");
		return;
	}

	rate = clk_get_rate(key_clk_source);
	dprintk(DEBUG_INIT, "%s: get key_clk_source rate %dHZ\n", __func__, (__u32)rate);

	key_clk = clk_get(NULL, LRADC_CLK);
	if (!key_clk || IS_ERR(key_clk)) {
		pr_err("try to get key clock failed!\n");
		return;
	}

	if(clk_set_parent(key_clk, key_clk_source))
		pr_err("%s: set key_clk parent to key_clk_source failed!\n", __func__);

	if (clk_prepare_enable(key_clk)) {
			pr_err("try to enable key_clk failed!\n");
	}

	return;
}

static void sunxikbd_clk_uncfg(void)
{

	if(NULL == key_clk || IS_ERR(key_clk)) {
		pr_err("key_clk handle is invalid, just return!\n");
		return;
	} else {
		clk_disable_unprepare(key_clk);
		clk_put(key_clk);
		key_clk = NULL;
	}

	if(NULL == key_clk_source || IS_ERR(key_clk_source)) {
		pr_err("key_clk_source handle is invalid, just return!\n");
		return;
	} else {
		clk_put(key_clk_source);
		key_clk_source = NULL;
	}
	return;
}
#endif

static void sunxi_keyboard_ctrl_set(enum key_mode key_mode, u32 para)
{
	u32 ctrl_reg = 0;
	
	if (0 != para)
		ctrl_reg = readl((const volatile void __iomem *)(KEY_BASSADDRESS + LRADC_CTRL));
	
	if (CONVERT_DLY_SET & key_mode) {
		ctrl_reg &= ~FIRST_CONVERT_DLY;
		ctrl_reg |= (FIRST_CONVERT_DLY & para);
	}
	if (ADC_CHAN_SET & key_mode)
		ctrl_reg |= (ADC_CHAN_SELECT & para);
	if (KEY_MODE_SET & key_mode) {
		ctrl_reg &= ~KEY_MODE_SELECT;
		ctrl_reg |= (KEY_MODE_SELECT & para);
	}
	if (LRADC_HOLD_SET & key_mode)
		ctrl_reg |= (LRADC_HOLD_EN & para);
	if (LEVELB_VOL_SET & key_mode)
		ctrl_reg |= (LEVELB_VOL & para);
	if (LRADC_SAMPLE_SET & key_mode) {
		ctrl_reg &= ~LRADC_SAMPLE_RATE;
		ctrl_reg |= (LRADC_SAMPLE_RATE & para);
	}
	if (LRADC_EN_SET & key_mode)
		ctrl_reg |= (LRADC_EN & para);

	writel(ctrl_reg, (volatile void __iomem *)(KEY_BASSADDRESS + LRADC_CTRL));
}

static void sunxi_keyboard_int_set(enum int_mode int_mode, u32 para)
{
	u32 ctrl_reg = 0;

	if (0 != para)
		ctrl_reg = readl((const volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INTC));

	if (ADC0_DOWN_INT_SET & int_mode)
		ctrl_reg |= (LRADC_ADC0_DOWN_EN & para);
	if (ADC0_UP_INT_SET & int_mode)
		ctrl_reg |= (LRADC_ADC0_UP_EN & para);
	if (ADC0_DATA_INT_SET & int_mode)
		ctrl_reg |= (LRADC_ADC0_DATA_EN & para);

	if (ADC1_DOWN_INT_SET & int_mode)
		ctrl_reg |= (LRADC_ADC1_DOWN_EN & para);
	if (ADC1_UP_INT_SET & int_mode)
		ctrl_reg |= (LRADC_ADC1_UP_EN & para);
	if (ADC1_DATA_INT_SET & int_mode)
		ctrl_reg |= (LRADC_ADC1_DATA_EN & para);

	writel(ctrl_reg, (volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INTC));
}

static u32 sunxi_keyboard_read_ints(void)
{
	u32 reg_val;
	reg_val  = readl((const volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INT_STA));

	return reg_val;
}

static void sunxi_keyboard_clr_ints(u32 reg_val)
{
	writel(reg_val, (volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INT_STA));
}

static u32 lradc_read_data(unsigned long addr)
{
	u32 reg_val;
	reg_val = readl((const volatile void __iomem *)(addr));

	return reg_val;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
/* ͣ���豸 */
static void sunxi_keyboard_early_suspend(struct early_suspend *h)
{
	//int ret;
	//struct sunxi_keyboard_data *ts = container_of(h, struct sunxi_keyboard_data, early_suspend);

	dprintk(DEBUG_SUSPEND, "[%s] enter standby state: %d. \n", __FUNCTION__, (int)standby_type);

	disable_irq_nosync(SW_INT_IRQNO_LRADC);
    
	if (NORMAL_STANDBY == standby_type) {
		sunxi_keyboard_ctrl_set(0, 0);
#ifdef CONFIG_ARCH_SUN9IW1P1
		clk_disable_unprepare(key_clk);
#endif
	/* process for super standby */	
	} else if (SUPER_STANDBY == standby_type) {
		if (check_scene_locked(SCENE_TALKING_STANDBY) == 0) {
			printk("lradc-key: talking standby, enable wakeup source lradc!!\n");
			enable_wakeup_src(CPUS_LRADC_SRC, 0);
		} else {
			sunxi_keyboard_ctrl_set(0, 0);
#ifdef CONFIG_ARCH_SUN9IW1P1
			clk_disable_unprepare(key_clk);
#endif

		}
	}
	return ;
}

/* ���»��� */
static void sunxi_keyboard_late_resume(struct early_suspend *h)
{
	unsigned long mode, para;
	//int ret;
	//struct sunxi_keyboard_data *ts = container_of(h, struct sunxi_keyboard_data, early_suspend);

	dprintk(DEBUG_SUSPEND, "[%s] return from standby state: %d. \n", __FUNCTION__, (int)standby_type);

#ifdef CONFIG_ARCH_SUN9IW1P1
	clk_prepare_enable(key_clk);
#endif

	/* process for normal standby */
	if (NORMAL_STANDBY == standby_type) {
		mode = CONVERT_DLY_SET | ADC_CHAN_SET | KEY_MODE_SET | LRADC_HOLD_SET | LEVELB_VOL_SET \
			| LRADC_SAMPLE_SET | LRADC_EN_SET;
		para = (lradc->first_delay << 24) |LEVELB_VOL| (lradc->mode << 12) |LRADC_HOLD_EN|ADC_CHAN_SELECT \
			| (lradc->rate << 2) |LRADC_EN;
		sunxi_keyboard_ctrl_set(mode, para);
	/* process for super standby */
	} else if (SUPER_STANDBY == standby_type) {
		if (check_scene_locked(SCENE_TALKING_STANDBY) != 0) {
			mode = ADC0_DOWN_INT_SET | ADC0_UP_INT_SET | ADC0_DATA_INT_SET;
			para = LRADC_ADC0_DOWN_EN | LRADC_ADC0_UP_EN | LRADC_ADC0_DATA_EN;
#ifndef ONE_CHANNEL
			mode |= ADC1_DOWN_INT_SET | ADC1_UP_INT_SET | ADC1_DATA_INT_SET;
			para |= LRADC_ADC1_DOWN_EN | LRADC_ADC1_UP_EN | LRADC_ADC1_DATA_EN;
#endif
			sunxi_keyboard_int_set(mode, para);
			mode = CONVERT_DLY_SET | ADC_CHAN_SET | KEY_MODE_SET | LRADC_HOLD_SET | LEVELB_VOL_SET \
				| LRADC_SAMPLE_SET | LRADC_EN_SET;
			para = (lradc->first_delay << 24) |LEVELB_VOL| (lradc->mode << 12) |LRADC_HOLD_EN|ADC_CHAN_SELECT \
				| (lradc->rate << 2) |LRADC_EN;
			sunxi_keyboard_ctrl_set(mode, para);
		} else {
			disable_wakeup_src(CPUS_LRADC_SRC, 0);
			printk("lradc-key: resume from talking standby!!\n");
		}
	}

	enable_irq(SW_INT_IRQNO_LRADC);
}
#else
#ifdef CONFIG_PM
static int sunxi_keyboard_suspend(struct device *dev)
{
	// struct lradc_data *lradc = dev_get_drvdata(dev);
	//int ret;

	dprintk(DEBUG_SUSPEND, "[%s] enter standby state: %d. \n", __FUNCTION__, (int)standby_type);

	disable_irq_nosync(SW_INT_IRQNO_LRADC);

	if (NORMAL_STANDBY == standby_type) {
		sunxi_keyboard_ctrl_set(0, 0);
#ifdef CONFIG_ARCH_SUN9IW1P1
		clk_disable_unprepare(key_clk);
#endif

	/* process for super standby */
	} else if (SUPER_STANDBY == standby_type) {
		if (check_scene_locked(SCENE_TALKING_STANDBY) == 0) {
			printk("lradc-key: talking standby, enable wakeup source lradc!!\n");
			enable_wakeup_src(CPUS_LRADC_SRC, 0);
		} else {
			sunxi_keyboard_ctrl_set(0, 0);
#ifdef CONFIG_ARCH_SUN9IW1P1
			clk_disable_unprepare(key_clk);
#endif
		}
	}
	return 0;
}

/* ���»��� */
static int sunxi_keyboard_resume(struct device *dev)
{
	struct lradc_data *lradc = dev_get_drvdata(dev);
	unsigned long mode, para;
	//int ret;

	dprintk(DEBUG_SUSPEND, "[%s] return from standby state: %d. \n", __FUNCTION__, (int)standby_type);

#ifdef CONFIG_ARCH_SUN9IW1P1
	clk_prepare_enable(key_clk);
#endif

	/* process for normal standby */
	if (NORMAL_STANDBY == standby_type) {
		mode = CONVERT_DLY_SET | ADC_CHAN_SET | KEY_MODE_SET | LRADC_HOLD_SET | LEVELB_VOL_SET \
			| LRADC_SAMPLE_SET | LRADC_EN_SET;
		para = (lradc->first_delay << 24) |LEVELB_VOL| (lradc->mode << 12) |LRADC_HOLD_EN|ADC_CHAN_SELECT \
			| (lradc->rate << 2) |LRADC_EN;
		sunxi_keyboard_ctrl_set(mode, para);
	/* process for super standby */	
	} else if (SUPER_STANDBY == standby_type) {
		if (check_scene_locked(SCENE_TALKING_STANDBY) != 0) {
		mode = ADC0_DOWN_INT_SET | ADC0_UP_INT_SET | ADC0_DATA_INT_SET;
		para = LRADC_ADC0_DOWN_EN | LRADC_ADC0_UP_EN | LRADC_ADC0_DATA_EN;
#ifndef ONE_CHANNEL
		mode |= ADC1_DOWN_INT_SET | ADC1_UP_INT_SET | ADC1_DATA_INT_SET;
		para |= LRADC_ADC1_DOWN_EN | LRADC_ADC1_UP_EN | LRADC_ADC1_DATA_EN;
#endif
		sunxi_keyboard_int_set(mode, para);
		mode = CONVERT_DLY_SET | ADC_CHAN_SET | KEY_MODE_SET | LRADC_HOLD_SET | LEVELB_VOL_SET \
			| LRADC_SAMPLE_SET | LRADC_EN_SET;
		para = (lradc->first_delay << 24) |LEVELB_VOL| (lradc->mode << 12) |LRADC_HOLD_EN|ADC_CHAN_SELECT \
			| (lradc->rate << 2) |LRADC_EN;
		sunxi_keyboard_ctrl_set(mode, para);
		} else {
			disable_wakeup_src(CPUS_LRADC_SRC, 0);
			printk("lradc-key: resume from talking standby!!\n");
		}
	}

	enable_irq(SW_INT_IRQNO_LRADC);

	return 0; 
}
#endif
#endif

static irqreturn_t sunxi_isr_key(int irq, void *data)
{
	struct lradc_data *lradc = data;
#ifdef HAVE_IIO
	struct iio_dev *indio_dev = lradc->indio_dev;
	s64 timestamp = iio_get_time_ns();
#endif
	u32 reg_val;
	int judge_flag = 0;
	u32 adc_val;

	dprintk(DEBUG_INT, "Key Interrupt\n");

	reg_val = sunxi_keyboard_read_ints();

	if (reg_val & LRADC_ADC0_DOWNPEND) {
		dprintk(DEBUG_INT, "key down\n");

#ifdef HAVE_IIO
		iio_push_event(indio_dev,
					IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE,
						  0,
						  IIO_EV_TYPE_THRESH,
						  IIO_EV_DIR_FALLING),
					timestamp);
#endif
	}

	if (reg_val & LRADC_ADC0_DATAPEND) {
		key_val = lradc_read_data(KEY_BASSADDRESS+LRADC_DATA0);

		if (key_val < 0x3f) {
			compare_buffer[key_cnt] = key_val & 0x3f;
		}

		if ((key_cnt + 1) < REPORT_START_NUM) {
			key_cnt++;
			/* do not report key message */
		} else {
			if (compare_buffer[0] == compare_buffer[1]) {
				key_val = compare_buffer[1];
				scancode = keypad_mapindex[key_val&0x3f];
				judge_flag = 1;
				key_cnt = 0;
			} else {
				key_cnt = 0;
				judge_flag = 0;
			}

			if (1 == judge_flag) {
				dprintk(DEBUG_INT, "report data: key_val :%8d transfer_code: %8d , scancode: %8d\n", \
					key_val, transfer_code, scancode);

				if (transfer_code == scancode) {
					/* report repeat key value */
#ifdef REPORT_REPEAT_KEY_FROM_HW
					input_report_key(sunxikbd_dev, sunxi_scankeycodes[scancode], 0);
					input_sync(sunxikbd_dev);
					input_report_key(sunxikbd_dev, sunxi_scankeycodes[scancode], 1);
					input_sync(sunxikbd_dev);
#else
					/* do not report key value */
#endif
				} else if (INITIAL_VALUE != transfer_code) {
					/* report previous key value up signal + report current key value down */
					input_report_key(sunxikbd_dev, sunxi_scankeycodes[transfer_code], 0);
					input_sync(sunxikbd_dev);
					input_report_key(sunxikbd_dev, sunxi_scankeycodes[scancode], 1);
					input_sync(sunxikbd_dev);
					transfer_code = scancode;
				} else {
					/* INITIAL_VALUE == transfer_code, first time to report key event */
					input_report_key(sunxikbd_dev, sunxi_scankeycodes[scancode], 1);
					input_sync(sunxikbd_dev);
					transfer_code = scancode;
				}
			}
		}
	}

	if (reg_val & LRADC_ADC0_UPPEND) {
		if (INITIAL_VALUE != transfer_code) {
			dprintk(DEBUG_INT, "report data: key_val :%8d transfer_code: %8d \n", key_val, transfer_code);

			input_report_key(sunxikbd_dev, sunxi_scankeycodes[transfer_code], 0);
			input_sync(sunxikbd_dev);
		}

		dprintk(DEBUG_INT, "key up \n");

		key_cnt = 0;
		judge_flag = 0;
		transfer_code = INITIAL_VALUE;

#ifdef HAVE_IIO
		iio_push_event(indio_dev,
					IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE,
						  0,
						  IIO_EV_TYPE_THRESH,
						  IIO_EV_DIR_RISING),
					timestamp);
#endif
	}

	if (reg_val & LRADC_ADC1_DOWNPEND) {
#ifdef HAVE_IIO
		iio_push_event(indio_dev,
					IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE,
						  1,
						  IIO_EV_TYPE_THRESH,
						  IIO_EV_DIR_FALLING),
					timestamp);
#endif
	}
	if (reg_val & LRADC_ADC1_DATAPEND) {
		adc_val = lradc_read_data(KEY_BASSADDRESS+LRADC_DATA1);
	}
	if (reg_val & LRADC_ADC1_UPPEND) {
#ifdef HAVE_IIO
		iio_push_event(indio_dev,
					IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE,
						  1,
						  IIO_EV_TYPE_THRESH,
						  IIO_EV_DIR_RISING),
					timestamp);
#endif
	}

	sunxi_keyboard_clr_ints(reg_val);
	return IRQ_HANDLED;
}

static void sunxikbd_map_init(void)
{
	int i = 0;
	unsigned char j = 0;
	for(i = 0; i < 64; i++){
		if(i * ADC_RESOL > key_vol[j])
			j++;
		keypad_mapindex[i] = j;
	}
}

static int sunxikbd_script_init(void)
{
	int i;
	char key_name[16];
	script_item_u	val;
	script_item_value_type_e  type;

	type = script_get_item("key_para", "key_used", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
		printk(KERN_WARNING "sunxi keyboard is not used\n");
		return 0;
	}

	if(1 == val.val){
		type = script_get_item("key_para", "key_cnt", &val);
		if(SCIRPT_ITEM_VALUE_TYPE_INT != type){
			pr_err("%s: get key cnt err! \n", __func__);
			return -1;
		}
		key_num = val.val;
		dprintk(DEBUG_INT,"%s key number = %d.\n", __func__, key_num);
		if(key_num < 1 || key_num > VOL_NUM){
			pr_err("incorrect key number.\n");
			return -1;
		}
		for(i = 0; i<VOL_NUM; i++)
			key_vol[i] = ADC_MEASURE;
		for(i = 1; i <= key_num; i++){
			sprintf(key_name, "key%d_vol", i);
			type = script_get_item("key_para", key_name, &val);
			if(SCIRPT_ITEM_VALUE_TYPE_INT != type){
				pr_err("%s: get %s err! \n", __func__, key_name);
				return -1;
			}
			key_vol[i-1] = val.val;
			dprintk(DEBUG_INT,"%s: key%d vol = %d.\n", __func__, i, val.val);
		}

		sunxikbd_map_init();

	}else{
		dprintk(DEBUG_INT,"sunxi key board no used.\n");
		return -1;
	}

	return 0;
}

#ifdef HAVE_IIO
/* Whilst this makes a lot of calls to iio_sw_ring functions - it is to device
 * specific to be rolled into the core.
 */
static irqreturn_t lradc_trigger_handler(int irq, void *p)
{
	// struct iio_poll_func *pf = p;
	// struct iio_dev *indio_dev = pf->indio_dev;
	// struct lradc_data *lradc = iio_priv(indio_dev);
	// struct iio_buffer *ring = indio_dev->buffer;

	printk("lradc_trigger %d\n", irq);
	return IRQ_HANDLED;
}

static const struct iio_buffer_setup_ops lradc_ring_setup_ops = {
	.preenable = &iio_sw_buffer_preenable,
	.postenable = &iio_triggered_buffer_postenable,
	.predisable = &iio_triggered_buffer_predisable,
};

static int lradc_configure_ring(struct iio_dev *indio_dev)
{
	struct iio_buffer *ring;
	int ret = 0;

	ring = iio_sw_rb_allocate(indio_dev);
	if (!ring) {
		ret = -ENOMEM;
		return ret;
	}
	indio_dev->buffer = ring;
	ring->scan_timestamp = true;
	indio_dev->setup_ops = &lradc_ring_setup_ops;

	indio_dev->pollfunc = iio_alloc_pollfunc(&iio_pollfunc_store_time,
						 &lradc_trigger_handler,
						 IRQF_ONESHOT,
						 indio_dev,
						 "lradc_consumer%d",
						 indio_dev->id);
	if (indio_dev->pollfunc == NULL) {
		ret = -ENOMEM;
		goto error_iio_sw_rb_free;
	}

	indio_dev->modes |= INDIO_BUFFER_TRIGGERED;
	return 0;
error_iio_sw_rb_free:
	iio_sw_rb_free(indio_dev->buffer);
	return ret;
}

static void lradc_unconfigure_ring(struct iio_dev *indio_dev)
{
	iio_dealloc_pollfunc(indio_dev->pollfunc);
	iio_sw_rb_free(indio_dev->buffer);
}

static int lradc_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan,
			   int *val,
			   int *val2,
			   long m)
{
	// struct lradc_data *lradc = iio_priv(indio_dev);
	u32 adc_val;
	unsigned long scale_uv;

	if (chan->type == IIO_VOLTAGE) {
		switch (m) {
		case 0:
			adc_val = lradc_read_data(KEY_BASSADDRESS + chan->address);

			// printk("v = %d\n", adc_val * 2000 / 4096);
/* isjeon # jcnet*/
			printk("ch = 0x%x v = %d\n",chan->address, adc_val * 2000 / 4096);

			*val = adc_val;

			return IIO_VAL_INT;

		case IIO_CHAN_INFO_SCALE:
			scale_uv = (2000 * 100000) >> chan->scan_type.realbits;
			*val = scale_uv / 100000;
			*val2 = (scale_uv % 100000) * 10;
			return IIO_VAL_INT_PLUS_MICRO;
		}
	}

	return -EINVAL;
}

static int lradc_read_event_config(struct iio_dev *indio_dev, u64 event_code)
{
	// struct lradc_data *lradc = iio_priv(indio_dev);
	const struct iio_chan_spec *chan;
	u32 ctrl_reg;

	if (IIO_EVENT_CODE_EXTRACT_CHAN(event_code) >= indio_dev->num_channels)
		return -EINVAL;

	chan = &indio_dev->channels[IIO_EVENT_CODE_EXTRACT_CHAN(event_code)];

	switch (IIO_EVENT_CODE_EXTRACT_CHAN_TYPE(event_code)) {
	case IIO_VOLTAGE:
		switch (IIO_EVENT_CODE_EXTRACT_TYPE(event_code)) {
		case IIO_EV_TYPE_THRESH:
			ctrl_reg = readl((const volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INTC));

			if (IIO_EVENT_CODE_EXTRACT_DIR(event_code) == IIO_EV_DIR_FALLING) {
				switch (chan->channel) {
				case 0:
					return !!(ctrl_reg & LRADC_ADC0_DOWN_EN);
				case 1:
					return !!(ctrl_reg & LRADC_ADC1_DOWN_EN);
				default:
					break;
				}
			} else if (IIO_EVENT_CODE_EXTRACT_DIR(event_code) == IIO_EV_DIR_RISING) {
				switch (chan->channel) {
				case 0:
					return !!(ctrl_reg & LRADC_ADC0_UP_EN);
				case 1:
					return !!(ctrl_reg & LRADC_ADC1_UP_EN);
				default:
					break;
				}
			}
			break;

		default:
			break;
		}

	default:
		break;
	}

	return -EINVAL;
}

static int lradc_write_event_config(struct iio_dev *indio_dev,
					u64 event_code,
					int state)
{
	// struct lradc_data *lradc = iio_priv(indio_dev);
	const struct iio_chan_spec *chan;
	u32 ctrl_reg;

	if (IIO_EVENT_CODE_EXTRACT_CHAN(event_code) >= indio_dev->num_channels)
		return -EINVAL;

	chan = &indio_dev->channels[IIO_EVENT_CODE_EXTRACT_CHAN(event_code)];

	switch (IIO_EVENT_CODE_EXTRACT_CHAN_TYPE(event_code)) {
	case IIO_VOLTAGE:
		switch (IIO_EVENT_CODE_EXTRACT_TYPE(event_code)) {
		case IIO_EV_TYPE_THRESH:
			ctrl_reg = readl((const volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INTC));

			if (IIO_EVENT_CODE_EXTRACT_DIR(event_code) == IIO_EV_DIR_FALLING) {
				switch (chan->channel) {
				case 0:
					if (state)
						ctrl_reg |= LRADC_ADC0_DOWN_EN;
					else
						ctrl_reg &= ~LRADC_ADC0_DOWN_EN;
					break;
				case 1:
					if (state)
						ctrl_reg |= LRADC_ADC1_DOWN_EN;
					else
						ctrl_reg &= ~LRADC_ADC1_DOWN_EN;
					break;
				default:
					return -EINVAL;
				}
			} else if (IIO_EVENT_CODE_EXTRACT_DIR(event_code) == IIO_EV_DIR_RISING) {
				switch (chan->channel) {
				case 0:
					if (state)
						ctrl_reg |= LRADC_ADC0_UP_EN;
					else
						ctrl_reg &= ~LRADC_ADC0_UP_EN;
					break;
				case 1:
					if (state)
						ctrl_reg |= LRADC_ADC1_UP_EN;
					else
						ctrl_reg &= ~LRADC_ADC1_UP_EN;
					break;
				default:
					return -EINVAL;
				}
			} else {
				return -EINVAL;
			}

			writel(ctrl_reg, (volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INTC));
			return 0;

		default:
			break;
		}

	default:
		break;
	}

	return -EINVAL;
}

static int lradc_read_event_value(struct iio_dev *indio_dev,
				      u64 event_code,
				      int *val)
{
	*val = 2000;

	return 0;
}

static int lradc_write_event_value(struct iio_dev *indio_dev,
				       u64 event_code,
				       int val)
{
	return -EINVAL;
}

static struct iio_chan_spec lradc_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.indexed = 1,
		.channel = 0,
		.address = LRADC_DATA0,
		.info_mask = IIO_CHAN_INFO_SCALE_SHARED_BIT,
		.scan_index = 0,
		.scan_type = IIO_ST('u', 6, 8, 0),
		.event_mask = IIO_EV_BIT(IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING) |
		              IIO_EV_BIT(IIO_EV_TYPE_THRESH, IIO_EV_DIR_FALLING),
	},
	{
		.type = IIO_VOLTAGE,
		.indexed = 1,
		.channel = 1,
		.address = LRADC_DATA1,
		.info_mask = IIO_CHAN_INFO_SCALE_SHARED_BIT,
		.scan_index = 1,
		.scan_type = IIO_ST('u', 6, 8, 0),
		.event_mask = IIO_EV_BIT(IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING) |
		              IIO_EV_BIT(IIO_EV_TYPE_THRESH, IIO_EV_DIR_FALLING),
	},
};

static const struct iio_info lradc_info = {
	.driver_module = THIS_MODULE,
	.read_raw = &lradc_read_raw,
	.read_event_config = &lradc_read_event_config,
	.write_event_config = &lradc_write_event_config,
	.read_event_value = &lradc_read_event_value,
	.write_event_value = &lradc_write_event_value,
};
#endif

static int __init sunxikbd_init(void)
{
	struct lradc_data *lradc;
#ifdef HAVE_IIO
	struct iio_dev *indio_dev;
#endif
	int i;
	int err = 0;
	int val;
	unsigned long mode, para;

	dprintk(DEBUG_INIT, "sunxikbd_init\n");

	if (sunxikbd_script_init()) {
		err = -EFAULT;
		goto fail1;
	}

#ifndef HAVE_IIO
	lradc = kzalloc(sizeof(*lradc), GFP_KERNEL);
	if (lradc == NULL) {
		err = -ENOMEM;
		goto err_alloc_data;
	}
#else
printk("[JCNET] IIO driver !!\n");
	indio_dev = iio_allocate_device(sizeof(*lradc));
	if (indio_dev == NULL) {
		err = -ENOMEM;
		goto err_iio_alloc;
	}
	lradc = iio_priv(indio_dev);

	indio_dev->name = "lradc";
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = lradc_channels;
	indio_dev->num_channels = ARRAY_SIZE(lradc_channels);
	indio_dev->info = &lradc_info;

	err = lradc_configure_ring(indio_dev);
	if (err)
		goto err_iio_ring;

	err = iio_buffer_register(indio_dev,
				  lradc_channels,
				  ARRAY_SIZE(lradc_channels));
	if (err) {
		printk(KERN_ERR "failed to initialize the ring\n");
		goto err_iio_buffer;
	}

	err = iio_device_register(indio_dev);
	if (err)
		goto err_iio_register;

	lradc->indio_dev = indio_dev;
#endif

	val = syscfg_get_int("key_para", "lradc_mode");
	if (val >= 0) {
		lradc->mode = val;
	} else {
		lradc->mode = KEY_MODE_NORMAL;
	}

	val = syscfg_get_int("key_para", "lradc_rate");
	if (val >= 0) {
		lradc->rate = val;
	} else {
		lradc->rate = LRADC_SAMPLE_250HZ;
	}

	val = syscfg_get_int("key_para", "lradc_delay");
	if (val >= 0) {
		lradc->first_delay = val;
	} else {
		lradc->first_delay = FIRST_CONVERT_DLY_1;
	}

	sunxikbd_dev = input_allocate_device();
	if (!sunxikbd_dev) {
		pr_err("sunxikbd: not enough memory for input device\n");
		err = -ENOMEM;
		goto err_alloc_input;
	}

	input_set_drvdata(sunxikbd_dev, lradc);

	sunxikbd_dev->name = INPUT_DEV_NAME;
	sunxikbd_dev->phys = "sunxikbd/input0";
	sunxikbd_dev->id.bustype = BUS_HOST;
	sunxikbd_dev->id.vendor = 0x0001;
	sunxikbd_dev->id.product = 0x0001;
	sunxikbd_dev->id.version = 0x0100;

#ifdef REPORT_REPEAT_KEY_BY_INPUT_CORE
	sunxikbd_dev->evbit[0] = BIT_MASK(EV_KEY)|BIT_MASK(EV_REP);
	pr_info("REPORT_REPEAT_KEY_BY_INPUT_CORE is defined, support report repeat key value.\n");
#else
	sunxikbd_dev->evbit[0] = BIT_MASK(EV_KEY);
#endif

	for (i = 0; i < KEY_MAX_CNT; i++)
		set_bit(sunxi_scankeycodes[i], sunxikbd_dev->keybit);

#ifdef CONFIG_ARCH_SUN9IW1P1
	sunxikbd_clk_cfg();
#endif

	mode = ADC0_DOWN_INT_SET | ADC0_UP_INT_SET | ADC0_DATA_INT_SET;
	para = LRADC_ADC0_DOWN_EN | LRADC_ADC0_UP_EN | LRADC_ADC0_DATA_EN;
#ifndef ONE_CHANNEL
	mode |= ADC1_DOWN_INT_SET | ADC1_UP_INT_SET | ADC1_DATA_INT_SET;
	para |= LRADC_ADC1_DOWN_EN | LRADC_ADC1_UP_EN | LRADC_ADC1_DATA_EN;
#endif
	sunxi_keyboard_int_set(mode, para);
	mode = CONVERT_DLY_SET | ADC_CHAN_SET | KEY_MODE_SET | LRADC_HOLD_SET | LEVELB_VOL_SET \
		| LRADC_SAMPLE_SET | LRADC_EN_SET;
	para = (lradc->first_delay << 24) |LEVELB_VOL| (lradc->mode << 12) |LRADC_HOLD_EN|ADC_CHAN_SELECT \
		| (lradc->rate << 2) |LRADC_EN;
	sunxi_keyboard_ctrl_set(mode, para);

	if (request_irq(SW_INT_IRQNO_LRADC, sunxi_isr_key, 0, "sunxikbd", lradc)) {
		err = -EBUSY;
		pr_err("request irq failure.\n");
		goto fail2;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
#else
#ifdef CONFIG_PM
	keyboard_pm_domain.ops.suspend = sunxi_keyboard_suspend;
	keyboard_pm_domain.ops.resume = sunxi_keyboard_resume;
	sunxikbd_dev->dev.pm_domain = &keyboard_pm_domain;	
#endif
#endif

	err = input_register_device(sunxikbd_dev);
	if (err)
		goto fail3;

#ifdef CONFIG_HAS_EARLYSUSPEND 
	dprintk(DEBUG_INIT, "==register_early_suspend =\n");
	lradc->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 3;
	lradc->early_suspend.suspend = sunxi_keyboard_early_suspend;
	lradc->early_suspend.resume	= sunxi_keyboard_late_resume;
	register_early_suspend(&lradc->early_suspend);
#endif

	dprintk(DEBUG_INIT, "sunxikbd_init end\n");

	return 0;

	input_unregister_device(sunxikbd_dev);
fail3:
	free_irq(SW_INT_IRQNO_LRADC, lradc);
fail2:
	input_free_device(sunxikbd_dev);
err_alloc_input:
#ifndef HAVE_IIO
	kfree(lradc);
err_alloc_data:
#else
	iio_device_unregister(indio_dev);
err_iio_register:
	iio_buffer_unregister(indio_dev);
err_iio_buffer:
	lradc_unconfigure_ring(indio_dev);
err_iio_ring:
	iio_free_device(indio_dev);
err_iio_alloc:
#endif
fail1:
	pr_err("sunxikbd_init failed.\n");

	return err;
}

static void __exit sunxikbd_exit(void)
{
	struct lradc_data *lradc = input_get_drvdata(sunxikbd_dev);
#ifdef HAVE_IIO
	struct iio_dev *indio_dev = lradc->indio_dev;
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&lradc->early_suspend);
#endif
	free_irq(SW_INT_IRQNO_LRADC, lradc);
#ifdef CONFIG_ARCH_SUN9IW1P1
	sunxikbd_clk_uncfg();
#endif
	input_unregister_device(sunxikbd_dev);
#ifdef HAVE_IIO
	iio_buffer_unregister(indio_dev);
	iio_device_unregister(indio_dev);
	lradc_unconfigure_ring(indio_dev);
	iio_free_device(indio_dev);
#else
	kfree(lradc);
#endif
}

module_init(sunxikbd_init);
module_exit(sunxikbd_exit);

MODULE_AUTHOR("Allwinner <someone@allwinner.com>");
MODULE_DESCRIPTION("sunxi-keyboard driver");
MODULE_LICENSE("GPL");
