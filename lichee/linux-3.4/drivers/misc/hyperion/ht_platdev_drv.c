#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/gpio.h>

#include "ht_syscfg.h"

#ifdef CONFIG_HT_GPIO_LEDS
extern int ht_gpio_leds_init(void);
extern void ht_gpio_leds_exit(void);
#endif

#if defined(CONFIG_HT_BACKLIGHT) || defined(CONFIG_HT_BACKLIGHT_MODULE)
extern int ht_backlight_init(void);
extern void ht_backlight_exit(void);
#endif

#ifdef CONFIG_HT_MT66XX
extern int ht_mt66xx_init(void);
extern void ht_mt66xx_exit(void);
#endif

#ifdef CONFIG_HT_XRADIO
extern int ht_xradio_init(void);
extern void ht_xradio_exit(void);
#endif

static void (*do_power_off)(void);
static int power_hold_gpio;
static bool power_hold_is_active_high;

static void ht_power_off(void)
{
	if (gpio_is_valid(power_hold_gpio)) {
		printk(KERN_EMERG "Drop power hold.\n");

		gpio_direction_output(power_hold_gpio, !power_hold_is_active_high);
	}

	if (do_power_off) {
		do_power_off();
	}
}

static int __init ht_power_off_init(void)
{
	struct gpio_config gpio;
	int value;

	value = syscfg_get_gpio("gpio_pins", "power_hold", &gpio);
	if (value >= 0 && gpio_is_valid(value)) {
		sunxi_gpio_req(&gpio);

		power_hold_gpio = value;
		// Default is active high
		power_hold_is_active_high = (gpio.data != 0);

		do_power_off = pm_power_off;
		pm_power_off = ht_power_off;
	}

	return 0;
}

static int __init ht_platdev_init(void)
{
	int ret = 0;

	ret = ht_power_off_init();
	if (ret != 0) {
		printk(KERN_WARNING "ht_power_off_init error %d\n", ret);
	}

#ifdef CONFIG_HT_GPIO_LEDS
	ret = ht_gpio_leds_init();
	if (ret != 0) {
		printk(KERN_WARNING "ht_gpio_leds_init error %d\n", ret);
	}
#endif
#if defined(CONFIG_HT_BACKLIGHT) || defined(CONFIG_HT_BACKLIGHT_MODULE)
	ret = ht_backlight_init();
	if (ret != 0) {
		printk(KERN_WARNING "ht_backlight_init error %d\n", ret);
	}
#endif
#ifdef CONFIG_HT_MT66XX
	ret = ht_mt66xx_init();
	if (ret != 0) {
		printk(KERN_WARNING "ht_mtk_66xx_init error %d\n", ret);
	}
#endif
#ifdef CONFIG_HT_XRADIO
	ret = ht_xradio_init();
	if (ret != 0) {
		printk(KERN_WARNING "ht_xradio_init error %d\n", ret);
	}
#endif

	return 0;
}

static void __exit ht_platdev_exit(void)
{
#ifdef CONFIG_HT_XRADIO
	ht_xradio_exit();
#endif
#ifdef CONFIG_HT_MT66XX
	ht_mt66xx_exit();
#endif
#if defined(CONFIG_HT_BACKLIGHT) || defined(CONFIG_HT_BACKLIGHT_MODULE)
	ht_backlight_exit();
#endif
#ifdef CONFIG_HT_GPIO_LEDS
	ht_gpio_leds_exit();
#endif

	if (do_power_off) {
		pm_power_off = do_power_off;
	}
}

module_init(ht_platdev_init);
module_exit(ht_platdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hyperion Tech");
MODULE_DESCRIPTION("Hyperion Tech Platform Support Driver");
