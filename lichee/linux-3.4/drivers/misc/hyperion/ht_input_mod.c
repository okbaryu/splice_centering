#include <linux/module.h>

extern int ht_i2cdev_init(void);
extern void ht_i2cdev_exit(void);

extern int ht_touchpanel_init(void);
extern void ht_touchpanel_exit(void);

#ifdef CONFIG_HT_GPIO_KEYS
extern int ht_gpio_keys_init(void);
extern void ht_gpio_keys_exit(void);
#endif

#ifdef CONFIG_HT_SENSOR
extern int ht_sensor_init(void);
extern void ht_sensor_exit(void);
#endif

static int __init ht_input_init(void)
{
	int ret = 0;

	ret = ht_i2cdev_init();
	if (ret != 0) {
		printk(KERN_WARNING "ht_i2cdev_init error %d\n", ret);
	}

	ret = ht_touchpanel_init();
	switch (ret) {
		case 0:
			break;
		case -ENODEV:
			printk(KERN_WARNING "No touch panel");
			break;
		default:
			printk(KERN_WARNING "Touch panel init error %d", ret);
			break;
	}

#ifdef CONFIG_HT_SENSOR
	ret = ht_sensor_init();
	if (ret != 0) {
		printk(KERN_WARNING "Unable to inittialize sensors(%d)\n", ret);
	}
#endif

#ifdef CONFIG_HT_GPIO_KEYS
	ret = ht_gpio_keys_init();
	if (ret != 0) {
		printk(KERN_WARNING "ht_gpio_keys_init error %d\n", ret);
	}
#endif

	return 0;
}

static void __exit ht_input_exit(void)
{
#ifdef CONFIG_HT_GPIO_KEYS
	ht_gpio_keys_exit();
#endif
#ifdef CONFIG_HT_SENSOR
	ht_sensor_exit();
#endif
	ht_touchpanel_exit();
	ht_i2cdev_exit();
}

module_init(ht_input_init);
module_exit(ht_input_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hyperion Tech");
MODULE_DESCRIPTION("Hyperion Tech input device support module");
