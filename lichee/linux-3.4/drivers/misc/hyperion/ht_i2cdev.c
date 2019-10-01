#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

#include "ht_i2cdev.h"

#define dlog(...)	do { printk("[ht_i2cdev] " __VA_ARGS__); } while (0)

int ht_i2c_new_device(int twi_id, struct ht_i2c_board_info *device_list, size_t size)
{
	struct i2c_adapter *default_adapter = NULL;
	struct i2c_client *client;
	int i;

	if (twi_id >= 0) {
		default_adapter = i2c_get_adapter(twi_id);
		if (!default_adapter) {
			dlog("No I2C adapter #%u\n", twi_id);
			return -EINVAL;
		}
	}

	for (i = 0; i < size; i++) {
		struct ht_i2c_board_info *info = &device_list[i];
		struct i2c_adapter *adapter = default_adapter;

		if (info->twi_id >= 0) {
			adapter = i2c_get_adapter(info->twi_id);
			if (!adapter) {
				adapter = default_adapter;
			}
		}

		if (info->addr_list == NULL) {
			client = i2c_new_device(adapter, &info->board_info);
		} else {
			client = i2c_new_probed_device(adapter, &info->board_info, info->addr_list, info->probe);
		}
		if (!client) {
			printk(KERN_WARNING "ht_i2cdev: failed to probe I2C device\n");
			continue;
		}

		// TODO: keep client handle for later unregistration
	}

	return 0;
}
EXPORT_SYMBOL(ht_i2c_new_device);

static struct ht_i2c_board_info i2c_device_list[] = {
#ifdef CONFIG_HT_RTC_S35390A
	{
		.board_info = {
			I2C_BOARD_INFO("s35390a", 0x30),
		},
		// TODO: load ID from sys_config
		.twi_id = 1,
	},
#endif
};

int __init ht_i2cdev_init(void)
{
	return ht_i2c_new_device(-1, i2c_device_list, ARRAY_SIZE(i2c_device_list));
}

void __exit ht_i2cdev_exit(void)
{
	// TODO: unregister registered devs

}
