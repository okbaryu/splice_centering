#ifndef __HT_I2CDEV_H__
#define __HT_I2CDEV_H__

#include <linux/i2c.h>

struct ht_i2c_board_info {
	struct i2c_board_info board_info;
	int twi_id;
	unsigned short const *addr_list;
	/** NULL to use default probe */
	int (*probe)(struct i2c_adapter *, unsigned short addr);
};

int ht_i2c_new_device(int twi_id, struct ht_i2c_board_info *device_list, size_t size);

#endif  /* __HT_I2CDEV_H__ */
