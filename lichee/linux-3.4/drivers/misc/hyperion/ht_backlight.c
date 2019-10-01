#include <linux/module.h>
// #include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/backlight.h>
#include <linux/gpio.h>
#include <linux/err.h>

#include "ht_backlight.h"
#include "ht_syscfg.h"

#define dlog(...)	do { printk("[ht_backlight] " __VA_ARGS__); } while (0)

#define MAX_BRIGHTNESS  1

struct ht_backlight_data {
	struct ht_backlight_platform_data	*pdata;
};

static int ht_backlight_update_status(struct backlight_device *bl)
{
	struct ht_backlight_data *data = bl_get_data(bl);
	int ret;

	if (bl->props.state & BL_CORE_SUSPENDED)
		bl->props.brightness = 0;

	if (gpio_is_valid(data->pdata->enable_gpio)) {
		ret = gpio_request(data->pdata->enable_gpio, "ht_bl_en");
		if (ret) {
			pr_warn("%s: failed to get backlight enable gpio.\n", __func__);
			return ret;
		}

		gpio_direction_output(data->pdata->enable_gpio, !!bl->props.brightness);

		gpio_free(data->pdata->enable_gpio);
	}

	return 0;
}

static int ht_backlight_get_brightness(struct backlight_device *bl)
{
	struct ht_backlight_data *data = bl_get_data(bl);

	if (gpio_is_valid(data->pdata->enable_gpio)) {
		bl->props.brightness = gpio_get_value(data->pdata->enable_gpio);
	}

	return bl->props.brightness;
}

static const struct backlight_ops ht_backlight_ops = {
	.options = BL_CORE_SUSPENDRESUME,
	.update_status = ht_backlight_update_status,
	.get_brightness = ht_backlight_get_brightness,
};

static int __devinit ht_backlight_probe(struct platform_device *pdev)
{
	struct ht_backlight_platform_data *pdata = pdev->dev.platform_data;
	struct ht_backlight_data *data;
	struct backlight_device *bl;
	struct backlight_properties props;

	if (!pdata) {
		dev_err(&pdev->dev, "platform data isn't assigned to backlight\n");
		return -EINVAL;
	}

	data = devm_kzalloc(&pdev->dev, sizeof(struct ht_backlight_data),
				GFP_KERNEL);
	if (data == NULL)
		return -ENOMEM;

	props.type = BACKLIGHT_PLATFORM;
	props.max_brightness = MAX_BRIGHTNESS;

	props.brightness = 1;

	bl = backlight_device_register(pdata->name, &pdev->dev, data,
			&ht_backlight_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		return PTR_ERR(bl);
	}

	data->pdata = pdata;

	platform_set_drvdata(pdev, bl);

	backlight_update_status(bl);

	return 0;
}

static int __devexit ht_backlight_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);

	backlight_device_unregister(bl);

	return 0;
}

static struct platform_driver ht_backlight_driver = {
	.driver		= {
		.name	= "ht-backlight",
		.owner	= THIS_MODULE,
	},
	.probe		= ht_backlight_probe,
	.remove		= __devexit_p(ht_backlight_remove),
};

module_platform_driver(ht_backlight_driver);

MODULE_DESCRIPTION("Backlight Driver for Hyperion Tech boards");
MODULE_AUTHOR("Tae-il Lim <trlim@allwinnerkorea.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ht-backlight");
