#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/combo_mt66xx.h>

#include "ht_syscfg.h"

#define dlog(...)	do { printk("[ht_mt66xx] " __VA_ARGS__); } while (0)

static struct mtk_wmt_platform_data mtk_wmt_pdata = {
	.pmu = -EINVAL,
	.rst = -EINVAL,
	.bgf_int = -EINVAL,
	.urt_cts = -EINVAL,
	.rtc = -EINVAL,
	.gps_sync = -EINVAL,
	.gps_lna = -EINVAL
};

static struct platform_device mtk_wmt_pdev = {
	.name = "mtk_wmt",
	.id = 0,
	.dev = {
		.platform_data = &mtk_wmt_pdata,
	},
};

static struct mtk_sdio_eint_platform_data mtk_sdio_eint_pdata = {
	.sdio_eint = -EINVAL
};

static struct platform_device mtk_sdio_eint_pdev = {
	.name = "mtk_sdio_eint",
	.id = 0,
	.dev = {
		.platform_data = &mtk_sdio_eint_pdata,
	},
};


static const char *wifi_section = "wifi_para";
static const char *bt_config = "bt_para";

int __init ht_mt66xx_init(void)
{
	int value;
	struct gpio_config gpio;
	int ret;

	/* step1. Make sure all the GPIOs MT66xx used are set to correct
	   mode. including PMUEN, SYSRST, SDIO CMD, SDIO CLK, SDIO D0~D3,
	   UART_RTS, UART_CTS, UART_TX, UART_RX, I2S, PCM, SDIO_EINT,
	   BGF_EINT, RTC, GPS_SYNC, GSP_LNA*/

	/* [rf_para] section has already been configured at drivers/misc/rf_pm/rf_pm.c */
	value = syscfg_get_bool(wifi_section, "wifi_used");
	if (value < 0) {
		dlog("failed to fetch wifi configuration!\n");
		return -1;
	}
	if (value) {
		mtk_wmt_pdata.pmu = syscfg_get_gpio(wifi_section, "wl_reg_on", &gpio);
		if (mtk_wmt_pdata.pmu >= 0) {
			sunxi_gpio_req(&gpio);
		}

		mtk_wmt_pdata.gps_sync = syscfg_get_gpio(wifi_section, "wl_gps_sync", &gpio);
		if (mtk_wmt_pdata.gps_sync >= 0) {
			sunxi_gpio_req(&gpio);
		}

		mtk_sdio_eint_pdata.sdio_eint = syscfg_get_gpio(wifi_section, "wl_host_wake", &gpio);
	}

	value = syscfg_get_bool(bt_config, "bt_used");
	if (value < 0) {
		dlog("failed to fetch bt configuration!\n");
		return -1;
	}
	if (value) {
		/* already done in bt_pm.c (rfkill) */
		mtk_wmt_pdata.rst = syscfg_get_gpio(bt_config, "bt_rst_n", &gpio);
		if (mtk_wmt_pdata.rst >= 0) {
			sunxi_gpio_req(&gpio);
		}

		mtk_wmt_pdata.bgf_int = syscfg_get_gpio(bt_config, "bt_host_wake", &gpio);
	}

	/* step2. Make sure PMUEN&SYSRST output low when system startup.*/

	/* step3. if use UART interface, config UART_CTS(host end) to UART_CTS mode;
	   if use common SDIO interface, config UART_CTS(host_end) to GPIO mode. */

	/* done */

	ret = platform_device_register(&mtk_wmt_pdev);
	if (ret < 0) {
		return ret;
	}

	ret = platform_device_register(&mtk_sdio_eint_pdev);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

void __exit ht_mt66xx_exit(void)
{
	platform_device_unregister(&mtk_sdio_eint_pdev);
	platform_device_unregister(&mtk_wmt_pdev);
}
