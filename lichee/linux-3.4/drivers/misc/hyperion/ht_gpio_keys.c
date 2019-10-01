#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gpio_keys.h>
#include <linux/gpio.h>

#include "ht_syscfg.h"

#define GPIO_KEYS_MAX	(8)

#define dlog(...)	do { printk("[ht_gpio_keys] " __VA_ARGS__); } while (0)

struct gpio_keys_device {
	struct kobject	kobj;
	const struct gpio_keys_button *	button;
};

#define to_gpio_keys_device(x) container_of((x), struct gpio_keys_device, kobj)

static int ht_gpio_keys_open(struct device *dev);
static void ht_gpio_keys_close(struct device *dev);

static const char *gpio_keys_section = "gpio_keys";

static struct gpio_keys_button gpio_buttons[GPIO_KEYS_MAX];

static struct gpio_keys_platform_data gpio_keys_pdata = {
	.name = "ht-gpio-keys",
	.buttons = gpio_buttons,
	.nbuttons = 0,
	.enable = ht_gpio_keys_open,
	.disable = ht_gpio_keys_close,
};

static ssize_t ht_gpio_keys_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	struct gpio_keys_device *dev = to_gpio_keys_device(kobj);
	const struct gpio_keys_button *button = dev->button;

	if (!button || button->gpio < 0)
		return -EINVAL;

	return sprintf(buf, "%d\n", gpio_get_value_cansleep(button->gpio));
}

static void ht_gpio_keys_release(struct kobject *kobj)
{
	struct gpio_keys_device *dev = to_gpio_keys_device(kobj);

	dev->button = NULL;
}

static struct sysfs_ops ht_gpio_keys_sysops =
{
	.show = ht_gpio_keys_show,
};

static const struct attribute dev_attr_value = { .name = "value", .mode = 0444 };

static const struct attribute *ht_gpio_keys_attrs[] = {
	&dev_attr_value,
	NULL,
};

static const struct attribute_group ht_gpio_attr_group = {
	.attrs = (struct attribute **) ht_gpio_keys_attrs,
};

static struct kobj_type ht_gpio_keys_ktype =
{
	.release = ht_gpio_keys_release,
	.sysfs_ops = &ht_gpio_keys_sysops,
	.default_attrs = (struct attribute **) ht_gpio_keys_attrs
};

static struct gpio_keys_device gpio_keys_devices[GPIO_KEYS_MAX];

static void ht_gpio_keys_close(struct device *dev)
{
	// struct platform_device *pdev = to_platform_device(dev);
	const struct gpio_keys_platform_data *pdata = &gpio_keys_pdata;
	int i;

	for (i = 0; i < pdata->nbuttons; i++) {
		kobject_del(&gpio_keys_devices[i].kobj);
		kobject_put(&gpio_keys_devices[i].kobj);
	}
}

static int ht_gpio_keys_open(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	const struct gpio_keys_platform_data *pdata = &gpio_keys_pdata;
	int i;
	int ret;

	for (i = 0; i < pdata->nbuttons; i++) {
		const struct gpio_keys_button *button = &pdata->buttons[i];

		if (button->gpio < 0) continue;

		gpio_keys_devices[i].button = button;

		ret = kobject_init_and_add(&gpio_keys_devices[i].kobj, &ht_gpio_keys_ktype,
								   &pdev->dev.kobj, "gpio%d", button->gpio);
		if (ret < 0) {
			goto error;
		}
	}

	return 0;
error:
	ht_gpio_keys_close(dev);
	return ret;
}

static struct platform_device gpio_keys_pdev = {
	.name = "gpio-keys",
	.id = 0,
	.dev = {
		.platform_data = &gpio_keys_pdata,
	},
};

int __init ht_gpio_keys_init(void)
{
	struct gpio_keys_platform_data *pdata = &gpio_keys_pdata;
	int num_keys;
	int i;

	num_keys = syscfg_get_int(gpio_keys_section, "num_keys");
	if (num_keys <= 0) {
		printk(KERN_WARNING "ht_gpio_keys: no keys defined\n");
	} else {
		if (num_keys > GPIO_KEYS_MAX) {
			printk(KERN_WARNING "ht_gpio_keys: %d GPIO keys requested! Only %d will be enabled",
					num_keys, GPIO_KEYS_MAX);
			num_keys = GPIO_KEYS_MAX;
		}

		pdata->nbuttons = num_keys;

		for (i = 0; i < pdata->nbuttons; i++) {
			struct gpio_keys_button *button = &pdata->buttons[i];
			char name[32];
			int value;
			struct gpio_config gpio;

			sprintf(name, "key%d_type", i);
			value = syscfg_get_int(gpio_keys_section, name);
			if (value < 0) {
				dlog("failed to read %s\n", name);
				return -1;
			}
			button->type = value;

			sprintf(name, "key%d_code", i);
			value = syscfg_get_int(gpio_keys_section, name);
			if (value < 0) {
				dlog("failed to read %s\n", name);
				return -1;
			}
			button->code = value;

			sprintf(name, "key%d_debounce", i);
			value = syscfg_get_int(gpio_keys_section, name);
			if (value >= 0) {
				button->debounce_interval = value;
			}

			sprintf(name, "key%d_gpio", i);
			value = syscfg_get_gpio(gpio_keys_section, name, &gpio);
			if (value < 0) {
				dlog("failed to read %s\n", name);
				return -1;
			}
			button->gpio = value;
			button->irq = gpio_to_irq(value);

			// Active Low by default
			button->active_low = 1;
			// Translate data as active low/high indicator
			if (gpio.data != GPIO_DATA_DEFAULT) {
				button->active_low = !gpio.data;

				gpio.data = GPIO_DATA_DEFAULT;
			}

			sunxi_gpio_req(&gpio);

			printk(KERN_INFO " ev type %d code %d @ gpio%d irq %d, active %s\n",
				button->type, button->code, button->gpio, button->irq, button->active_low ? "lo" : "hi");
		}

		return platform_device_register(&gpio_keys_pdev);
	}

	return 0;
}

void __exit ht_gpio_keys_exit(void)
{
	if (gpio_keys_pdata.nbuttons > 0) {
		platform_device_unregister(&gpio_keys_pdev);
	}
}
