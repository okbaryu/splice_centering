#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include "ht_rfkill.h"
#include "ht_syscfg.h"

#define dlog(...)	do { printk("[ht_xradio] " __VA_ARGS__); } while (0)

#define USE_RFKILL	0	/*(defined CONFIG_HT_RFKILL || defined CONFIG_HT_RFKILL_MODULE)*/

struct xradio_platform_data {
	int		power_1_8_gpio;
	int		power_3_3_gpio;
	int		reset_gpio;
};

static struct xradio_platform_data xradio_pdata = {
	.power_1_8_gpio = -1,
	.power_3_3_gpio = -1,
	.reset_gpio = -1,
};

static int xradio_platform_set_power(struct xradio_platform_data *xradio, bool blocked)
{
	if (blocked) {
		if (gpio_is_valid(xradio->power_3_3_gpio))
			gpio_direction_output(xradio->power_3_3_gpio, 0);
		if (gpio_is_valid(xradio->power_1_8_gpio))
			gpio_direction_output(xradio->power_1_8_gpio, 0);
		if (gpio_is_valid(xradio->reset_gpio))
			gpio_direction_output(xradio->reset_gpio, 0);
		// if (rfkill->pwr_clk && PWR_CLK_ENABLED(rfkill))
		// 	clk_disable(rfkill->pwr_clk);
	} else {
		// if (rfkill->pwr_clk && PWR_CLK_DISABLED(rfkill))
		// 	clk_enable(rfkill->pwr_clk);
		if (gpio_is_valid(xradio->power_1_8_gpio))
			gpio_direction_output(xradio->power_1_8_gpio, 1);
		if (gpio_is_valid(xradio->power_3_3_gpio))
			gpio_direction_output(xradio->power_3_3_gpio, 1);
		mdelay(50);

		if (gpio_is_valid(xradio->reset_gpio))
			gpio_direction_output(xradio->reset_gpio, 1);
		mdelay(50);
	}

	// if (rfkill->pwr_clk)
	// 	PWR_CLK_SET(rfkill, blocked);

	return 0;
}

static int xradio_platform_setup(struct xradio_platform_data *xradio)
{
	int ret = 0;

	if (gpio_is_valid(xradio->reset_gpio)) {
		ret = gpio_request(xradio->reset_gpio, "xradio_reset");
		if (ret) {
			pr_warn("%s: failed to get reset gpio.\n", __func__);
			goto fail_setup;
		}
	}

	if (gpio_is_valid(xradio->power_1_8_gpio)) {
		ret = gpio_request(xradio->power_1_8_gpio, "xradio_1_8V");
		if (ret) {
			pr_warn("%s: failed to get power 1.8V gpio.\n", __func__);
			goto fail_reset;
		}
	}

	if (gpio_is_valid(xradio->power_3_3_gpio)) {
		ret = gpio_request(xradio->power_3_3_gpio, "xradio_3_3V");
		if (ret) {
			pr_warn("%s: failed to get power 3.3V gpio.\n", __func__);
			goto fail_1_8V;
		}
	}

	return xradio_platform_set_power(xradio, true);

// fail_3_3V:
// 	if (gpio_is_valid(rfkill->power_3_3_gpio))
// 		gpio_free(rfkill->power_3_3_gpio);
fail_1_8V:
	if (gpio_is_valid(xradio->power_1_8_gpio))
		gpio_free(xradio->power_1_8_gpio);
fail_reset:
	if (gpio_is_valid(xradio->reset_gpio))
		gpio_free(xradio->reset_gpio);
fail_setup:

	return ret;
}

static void xradio_platform_cleanup(struct xradio_platform_data *xradio)
{
	if (gpio_is_valid(xradio->power_3_3_gpio))
		gpio_free(xradio->power_3_3_gpio);
	if (gpio_is_valid(xradio->power_1_8_gpio))
		gpio_free(xradio->power_1_8_gpio);
	if (gpio_is_valid(xradio->reset_gpio))
		gpio_free(xradio->reset_gpio);
}

int xradio_wlan_power(int on)
{
	printk(KERN_INFO "xradio: power = %d", on);

	return xradio_platform_set_power(&xradio_pdata, !on);
}
EXPORT_SYMBOL(xradio_wlan_power);

#if USE_RFKILL
static int xradio_rfkill_setup(struct platform_device *pdev)
{
	struct ht_rfkill_platform_data *pdata = pdev->dev.platform_data;

	return xradio_platform_setup(pdata->data);
}

static void xradio_rfkill_cleanup(struct platform_device *pdev)
{
	struct ht_rfkill_platform_data *pdata = pdev->dev.platform_data;

	xradio_platform_cleanup(pdata->data);
}

static int xradio_rfkill_set_power(void *data, bool blocked)
{
	return xradio_platform_set_power(data, blocked);
}

static struct ht_rfkill_platform_data xradio_rfkill_pdata = {
	.name = "xradio",
	.type = RFKILL_TYPE_WLAN,
	.data = &xradio_pdata,
	.setup = xradio_rfkill_setup,
	.cleanup = xradio_rfkill_cleanup,
	.set_power = xradio_rfkill_set_power,
};

static struct platform_device xradio_rfkill_pdev = {
	.name = "rfkill_ht",
	.id = 0,
	.dev = {
		.platform_data = &xradio_rfkill_pdata,
	},
};
#endif	/* USE_RFKILL */

static const char *wifi_section = "wifi_para";

int __init ht_xradio_init(void)
{
	struct xradio_platform_data *pdata = &xradio_pdata;
	int ret = 0;
	int value;
	struct gpio_config gpio;

	value = syscfg_get_int(wifi_section, "pmu_wl_reset");
	if (value >= 0) {
		pdata->reset_gpio = GPIO_AXP(value);
	} else {
		// Fallback to legacy configuration
		value = syscfg_get_gpio(wifi_section, "wl_reg_on", &gpio);
		if (value >= 0) {
			pdata->reset_gpio = value;

			sunxi_gpio_req(&gpio);
		}
	}
	value = syscfg_get_int(wifi_section, "pmu_wl_1_8V");
	if (value >= 0) {
		pdata->power_1_8_gpio = GPIO_AXP(value);
	} else {
		value = syscfg_get_gpio(wifi_section, "wl_power0", &gpio);
		if (value >= 0) {
			pdata->power_1_8_gpio = value;

			sunxi_gpio_req(&gpio);
		}
	}
	value = syscfg_get_int(wifi_section, "pmu_wl_3_3V");
	if (value >= 0) {
		pdata->power_3_3_gpio = GPIO_AXP(value);
	} else {
		value = syscfg_get_gpio(wifi_section, "wl_power1", &gpio);
		if (value >= 0) {
			pdata->power_3_3_gpio = value;

			sunxi_gpio_req(&gpio);
		}
	}

#if USE_RFKILL
	ret = platform_device_register(&xradio_rfkill_pdev);
	if (ret < 0) {
		return ret;
	}
#else
	ret = xradio_platform_setup(&xradio_pdata);
#endif

	return ret;
}

void __exit ht_xradio_exit(void)
{
#if USE_RFKILL
	platform_device_unregister(&xradio_rfkill_pdev);
#else
	xradio_platform_set_power(&xradio_pdata, false);
	xradio_platform_cleanup(&xradio_pdata);
#endif
}
