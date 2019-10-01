#include <linux/init.h>
#include <linux/platform_device.h>

#include "ht_backlight.h"
#include "ht_syscfg.h"

#define dlog(...)	do { printk("[ht_backlightdev] " __VA_ARGS__); } while (0)

static struct ht_backlight_platform_data backlight_pdata = {
	.name = "lcd0",
	.enable_gpio = -1,
};

static struct platform_device backlight_pdev = {
	.name = "ht-backlight",
	.id = 0,
	.dev = {
		.platform_data = &backlight_pdata,
	},
};

int __init ht_backlight_init(void)
{
	struct ht_backlight_platform_data *pdata = &backlight_pdata;
	struct gpio_config gpio;
	int value;

	value = syscfg_get_gpio("lcd0_para", "lcd_bl_en", &gpio);
	if (value >= 0) {
		pdata->enable_gpio = value;
	}

	return platform_device_register(&backlight_pdev);
}

void __exit ht_backlight_exit(void)
{
	platform_device_unregister(&backlight_pdev);
}
