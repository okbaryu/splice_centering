#include <linux/module.h>
#include <linux/delay.h>
#include <linux/init-input.h>

#include "ht_input.h"
#include "ht_i2cdev.h"

static inline bool ht_touchpanel_is_down(const struct ht_touchpanel *tp, void *context)
{
	return tp->is_down( context);
}

static inline int ht_touchpanel_read_sample(const struct ht_touchpanel *tp, void *context)
{
	return tp->read_sample(context);
}

static inline void ht_touchpanel_report_down(const struct ht_touchpanel *tp, void *context)
{
	if (tp->report_down)
		tp->report_down(context);
}

static inline void ht_touchpanel_report_sample(const struct ht_touchpanel *tp, void *context)
{
	tp->report_sample(context);
}

static inline void ht_touchpanel_report_up(const struct ht_touchpanel *tp, void *context)
{
	tp->report_up(context);
}

int ht_touchpanel_loop_run(const struct ht_touchpanel *tp, void *context)
{
	bool down = false;
	unsigned int up = 0;
	int ret;

	do {
		if (up == 0) {
			ret = ht_touchpanel_read_sample(tp, context);
			if (ret > 0) {
				if (!down) {
					ht_touchpanel_report_down(tp, context);
					down = true;
					printk("touch down\n");
				}

				ht_touchpanel_report_sample(tp, context);
			}
		}

		msleep(tp->report_period_ms);

		if (ht_touchpanel_is_down(tp, context)) {
			up = 0;
		} else {
			up++;
		}
	} while (up < tp->up_threshold);

	ht_touchpanel_report_up(tp, context);

	if (down)
		printk("touch up\n");

	return 0;
}
EXPORT_SYMBOL(ht_touchpanel_loop_run);


static struct ctp_config_info config_info = {
	.input_type = CTP_TYPE,
};

#ifdef CONFIG_TOUCHSCREEN_ZT2003
static const unsigned short zt2003_addr_list[] = {0x48, I2C_CLIENT_END};

static int zt2003_detect(struct i2c_adapter *adapter, unsigned short addr)
{
	printk(KERN_INFO "zt2003: detected\n");
	return 1;
}
#endif

static struct ht_i2c_board_info touchpanel_device_list[] = {
#ifdef CONFIG_TOUCHSCREEN_ZT2003
	{
		.board_info = {
			I2C_BOARD_INFO("zt2003", 0x48),
			.platform_data = &config_info,
		},
		.twi_id = -1,
		.addr_list = zt2003_addr_list,
		.probe = zt2003_detect,
	},
#endif
};

int __init ht_touchpanel_init(void)
{
	int ret;

	if ((ret = input_fetch_sysconfig_para(&config_info.input_type))) {
		printk("%s: input_fetch_sysconfig_para err.\n", __func__);
		return ret;
	} 

	if (!config_info.ctp_used) {
		printk("*** `ctp_used` is 0!\n");
		printk("*** If you want to use CTP, set `cpt_used = 1` in your sys_config.fex.\n");
		return -ENODEV;
	}

	return ht_i2c_new_device(config_info.twi_id, touchpanel_device_list, ARRAY_SIZE(touchpanel_device_list));
}

void __exit ht_touchpanel_exit(void)
{
	// TODO: unregister registered devs

	input_free_platform_resource(&config_info.input_type);
}
