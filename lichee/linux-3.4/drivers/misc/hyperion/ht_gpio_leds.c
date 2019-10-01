#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>

#include "ht_syscfg.h"

#define GPIO_LEDS_MAX	(8)
#define LED_NAME_MAX	(16)

#define dlog(...)	do { printk("[ht_gpio_leds] " __VA_ARGS__); } while (0)

static const char *gpio_leds_section = "gpio_leds";

static char led_names[GPIO_LEDS_MAX][LED_NAME_MAX] = {
	"led-0",
	"led-1",
	"led-2",
	"led-3",
	"led-4",
	"led-5",
	"led-6",
	"led-7",
};

static struct gpio_led gpio_leds[GPIO_LEDS_MAX] = {
	{
		.name = led_names[0],
		.default_trigger = "none",
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = led_names[1],
		.default_trigger = "none",
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = led_names[2],
		.default_trigger = "none",
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = led_names[3],
		.default_trigger = "none",
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = led_names[4],
		.default_trigger = "none",
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = led_names[5],
		.default_trigger = "none",
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = led_names[6],
		.default_trigger = "none",
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
	{
		.name = led_names[7],
		.default_trigger = "none",
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
	},
};

static struct gpio_led_platform_data gpio_leds_pdata = {
	.num_leds = 0,
	.leds = gpio_leds,
};

static struct platform_device gpio_leds_pdev = {
	.name = "leds-gpio",
	.id = 0,
	.dev = {
		.platform_data = &gpio_leds_pdata,
	},
};

int __init ht_gpio_leds_init(void)
{
	int num_leds;
	int i;

	num_leds = syscfg_get_int(gpio_leds_section, "num_leds");
	if (num_leds <= 0) {
		printk(KERN_WARNING "ht_gpio_leds: no LEDs defined\n");
	} else {
		if (num_leds > GPIO_LEDS_MAX) {
			printk(KERN_WARNING "ht_gpio_leds: %d GPIO LEDs requested! Only %d will be enabled",
					num_leds, GPIO_LEDS_MAX);
			num_leds = GPIO_LEDS_MAX;
		}

		gpio_leds_pdata.num_leds = num_leds;

		for (i = 0; i < gpio_leds_pdata.num_leds; i++) {
			struct gpio_led *led = &gpio_leds[i];
			char name[32];
			int value;
			struct gpio_config gpio;

			sprintf(name, "led%d_name", i);
			syscfg_get_string(gpio_leds_section, name, led_names[i], LED_NAME_MAX-1);

			sprintf(name, "led%d_gpio", i);
			value = syscfg_get_gpio(gpio_leds_section, name, &gpio);
			if (value < 0) {
				dlog("failed to read %s\n", name);
				return -1;
			}
			led->gpio = value;

			sprintf(name, "led%d_active", i);
			value = syscfg_get_int(gpio_leds_section, name);
			if (value >= 0) {
				led->active_low = !value;
			}

			sunxi_gpio_req(&gpio);

			printk(KERN_INFO " LED %s @ gpio%d - active %s, trigger %s\n",
				led->name, led->gpio, led->active_low ? "lo" : "hi", led->default_trigger);
		}

		return platform_device_register(&gpio_leds_pdev);
	}

	return 0;
}

void __exit ht_gpio_leds_exit(void)
{
	if (gpio_leds_pdata.num_leds > 0) {
		platform_device_unregister(&gpio_leds_pdev);
	}
}
