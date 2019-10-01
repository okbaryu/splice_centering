#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init-input.h>
#include <linux/gpio.h>

#include "ht_sensor.h"
#include "ht_i2cdev.h"
#include "ht_syscfg.h"

#define dlog(...)	do { printk("[ht_sensor] " __VA_ARGS__); } while (0)

#ifdef CONFIG_HT_BMA250
#define SENSOR_NAME	"bma250"

#define BMA150_CHIP_ID	2
#define BMA250_CHIP_ID	3
#define BMA250E_CHIP_ID	0xF9

#define BMA250_CHIP_ID_REG	0x00

static const unsigned short bma250_addr_list[] = {0x18, I2C_CLIENT_END};

/**
 * gsensor_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int bma250_detect(struct i2c_adapter *adapter, unsigned short addr)
{
	union i2c_smbus_data data;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	ret = i2c_smbus_xfer(adapter, addr, 0, I2C_SMBUS_READ, BMA250_CHIP_ID_REG, I2C_SMBUS_BYTE_DATA, &data);
	if (ret < 0) {
		return ret;
	}

	ret = data.byte & 0xFF;
	if (ret == BMA250_CHIP_ID) {
		pr_info("Bosch Sensortec BMA250 detected!\n" );
		return 1;
	} else if (ret == BMA150_CHIP_ID) {
		pr_info("Bosch Sensortec BMA150 detected!\n");  
		return 1;
	} else if (ret == BMA250E_CHIP_ID) {
		pr_info("Bosch Sensortec BMA250E detected!\n");  
		return 1; 
	}

	pr_info("%s:Bosch Sensortec Device not found, maybe the other gsensor equipment! \n", __func__);
	return -ENODEV;
}
#endif

#ifdef CONFIG_HT_INV_MPU9250
#include <linux/mpu.h>

/*---- MPU6500 ----*/
#define MPU6500_ID               0x70      /* unique WHOAMI */
/*---- MPU9250 ----*/
#define MPU9250_ID               0x71      /* unique WHOAMI */

static unsigned short mpu9250_addr_list[] = { 0x68, 0x69, I2C_CLIENT_END };

static int mpu6500_detect(struct i2c_adapter *adapter, unsigned short addr)
{
	union i2c_smbus_data data;
	int err;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		return -ENODEV;
	}

	err = i2c_smbus_xfer(adapter, addr, 0, I2C_SMBUS_READ, 0x75, I2C_SMBUS_BYTE_DATA, &data);

	switch (data.byte & 0x00ff) {
		default:
			printk("reg(0x75):0x%x, addr:0x%x. device not found!\n", err, addr);
			return 0;

		case MPU6500_ID:
		case 0x7d:
			printk("%s: found MPU6500 @ 0x%x\n", __func__, addr);
			// strlcpy(info->type, "mpu6500", I2C_NAME_SIZE);
			break;

		case MPU9250_ID:
			printk("%s: found MPU9250 @ 0x%x\n", __func__, addr);
			// strlcpy(info->type, "mpu9250", I2C_NAME_SIZE);
			break;
	}

	return 1;
}

static struct mpu_platform_data inv_mpu9250_pdata = {
	.int_config		= 0x10,
	.orientation	= { 0,  1,  0,
						1,  0,  0,
						0,  0,  -1 },
	.level_shifter	= 0,
	.sec_slave_type	= SECONDARY_SLAVE_TYPE_COMPASS,
 	.sec_slave_id	= COMPASS_ID_AK8963,
	.secondary_i2c_addr = 0x0C,
	.secondary_orientation = { 1,  0,  0,
							   0,  1,  0,
							   0,  0,  1 }
};
#endif /* CONFIG_HT_INV_MPU9250 */

static struct ht_i2c_board_info gsensor_device_list[] = {
#ifdef CONFIG_HT_BMA250
	{
		.board_info = {
			I2C_BOARD_INFO("bma250", 0x18),
		},
		.twi_id = -1,
		.addr_list = bma250_addr_list,
		.probe = bma250_detect,
	},
#endif
#ifdef CONFIG_HT_INV_MPU9250
	{
		.board_info = {
			I2C_BOARD_INFO("mpu9250", 0x68),
			.platform_data = &inv_mpu9250_pdata
		},
		.twi_id = -1,
		.addr_list = mpu9250_addr_list,
		.probe = mpu6500_detect,
	},
#endif
};

static struct sensor_config_info gsensor_info = {
	.input_type = GSENSOR_TYPE,
	.int_number = 0,
	.ldo = NULL,
};

int __init ht_gsensor_init(void)
{
	int ret;

	if ((ret = input_fetch_sysconfig_para(&gsensor_info.input_type))) {
		return ret;
	} 

	if (!gsensor_info.sensor_used) {
		printk("*** `gsensor_used` is 0!\n");
		printk("*** If you want to use G-sensor, set `gsensor_used = 1` in your sys_config.fex.\n");
		return -ENODEV;
	}

	if (ARRAY_SIZE(gsensor_device_list) == 0) {
		printk(KERN_WARNING "No G-sensor device defined\n");
		return 0;
	}

	gsensor_device_list[0].board_info.irq = gpio_to_irq(gsensor_info.int_number);

	sunxi_gpio_req(&gsensor_info.irq_gpio);

	return ht_i2c_new_device(gsensor_info.twi_id, gsensor_device_list, ARRAY_SIZE(gsensor_device_list));
}


#ifdef CONFIG_HT_STMVL6180
static struct ht_sensor_pdata stmvl6180_sensor_pdata = {
	.irq_no = -1,
};
#endif

static struct ht_i2c_board_info lsensor_device_list[] = {
#ifdef CONFIG_HT_STMVL6180
	{
#define VL6180_I2C_ADDRESS  (0x52>>1)
		.board_info = {
			I2C_BOARD_INFO("stmvl6180", VL6180_I2C_ADDRESS),
			.platform_data = &stmvl6180_sensor_pdata
		},
		.twi_id = -1,
	},
#endif
};

static struct sensor_config_info lsensor_info = {
	.input_type = LS_TYPE,
	.int_number = 0,
	.ldo = NULL,
};

int __init ht_lsensor_init(void)
{
	struct ht_sensor_pdata *pdata;
	int ret;

	if ((ret = input_fetch_sysconfig_para(&lsensor_info.input_type))) {
		return ret;
	} 

	if (lsensor_info.sensor_used == 0) {
		printk("*** `lsensor_used` is 0!\n");
		printk("*** If you want to use L-sensor, set `ls_used = 1` in your sys_config.fex.\n");
		return -ENODEV;
	}

	if (ARRAY_SIZE(lsensor_device_list) == 0) {
		printk(KERN_WARNING "No L-sensor device defined\n");
		return 0;
	}

	lsensor_device_list[0].board_info.irq = gpio_to_irq(lsensor_info.int_number);

	sunxi_gpio_req(&lsensor_info.irq_gpio);

	pdata = (struct ht_sensor_pdata *)lsensor_device_list[0].board_info.platform_data;

	pdata->irq_no = lsensor_info.int_number;
	pdata->regulator_name = lsensor_info.ldo;

	return ht_i2c_new_device(lsensor_info.twi_id, lsensor_device_list, ARRAY_SIZE(lsensor_device_list));
}

int __init ht_sensor_init(void)
{
	int ret;

	ret = ht_gsensor_init();
	if (ret < 0 && ret != -ENODEV) {
		printk(KERN_WARNING "Unable to initialize G-sensor\n");
		return ret;
	}

	ret = ht_lsensor_init();
	if (ret < 0 && ret != -ENODEV) {
		printk(KERN_WARNING "Unable to initialize L-sensor\n");
	}

	return 0;
}

void __exit ht_sensor_exit(void)
{
	// TODO: unregister registered devs

	input_free_platform_resource(&gsensor_info.input_type);
	input_free_platform_resource(&lsensor_info.input_type);
}
