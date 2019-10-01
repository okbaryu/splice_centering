#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "ht_rfkill.h"

struct ht_rfkill_data {
	struct ht_rfkill_platform_data	*pdata;
	struct rfkill				*rfkill_dev;
};

static int ht_rfkill_set_power(void *data, bool blocked)
{
	struct ht_rfkill_data *rfkill = data;
	struct ht_rfkill_platform_data *pdata = rfkill->pdata;

	printk(KERN_INFO "ht_rfkill: %s %s\n", blocked ? "block" : "unblock", pdata->name);

	return pdata->set_power(pdata->data, blocked);
}

static const struct rfkill_ops ht_rfkill_ops = {
	.set_block = ht_rfkill_set_power,
};

static int ht_rfkill_probe(struct platform_device *pdev)
{
	struct ht_rfkill_data *rfkill;
	struct ht_rfkill_platform_data *pdata = pdev->dev.platform_data;
	int ret = 0;

	if (!pdata) {
		pr_warn("%s: No platform data specified\n", __func__);
		return -EINVAL;
	}

	if (!pdata->set_power) {
		pr_warn("%s: invalid platform data\n", __func__);
		return -EINVAL;
	}

	rfkill = kzalloc(sizeof(*rfkill), GFP_KERNEL);
	if (!rfkill)
		return -ENOMEM;

	if (pdata->setup) {
		ret = pdata->setup(pdev);
		if (ret) {
			pr_warn("%s: can't set up rfkill\n", __func__);
			goto fail_alloc;
		}
	}

	rfkill->pdata = pdata;

	rfkill->rfkill_dev = rfkill_alloc(pdata->name, &pdev->dev, pdata->type,
				&ht_rfkill_ops, rfkill);
	if (!rfkill->rfkill_dev)
		goto fail_alloc;

	ret = rfkill_register(rfkill->rfkill_dev);
	if (ret < 0)
		goto fail_rfkill;

	platform_set_drvdata(pdev, rfkill);

	dev_info(&pdev->dev, "%s device registered.\n", pdata->name);

	return 0;

fail_rfkill:
	rfkill_destroy(rfkill->rfkill_dev);
fail_alloc:
	kfree(rfkill);

	return ret;
}

static int ht_rfkill_remove(struct platform_device *pdev)
{
	struct ht_rfkill_data *rfkill = platform_get_drvdata(pdev);
	struct ht_rfkill_platform_data *pdata = pdev->dev.platform_data;

	if (pdata->cleanup)
		pdata->cleanup(pdev);

	rfkill_unregister(rfkill->rfkill_dev);
	rfkill_destroy(rfkill->rfkill_dev);
	kfree(rfkill);

	return 0;
}

static struct platform_driver ht_rfkill_driver = {
	.probe = ht_rfkill_probe,
	.remove = __devexit_p(ht_rfkill_remove),
	.driver = {
		.name = "rfkill_ht",
		.owner = THIS_MODULE,
	},
};

module_platform_driver(ht_rfkill_driver);

MODULE_DESCRIPTION("Hyperion Tech rfkill");
MODULE_AUTHOR("Hyperion Tech");
MODULE_LICENSE("GPL");
