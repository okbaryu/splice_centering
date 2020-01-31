#include <linux/input.h>
#include <linux/init-input.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/rotary_encoder.h>
#include <linux/gpio.h>

#define GPIO_ROTARY_A 34
#define GPIO_ROTARY_B 36

enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_CONTROL_INFO = 1U << 1,
	DEBUG_DATA_INFO = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
};

static u32 debug_mask = 0xF;
static u32 dynamic_debug = 0;

#define dprintk(level_mask, fmt, arg...)	if (unlikely(debug_mask & level_mask)) \
		printk(KERN_DEBUG fmt , ## arg)

module_param_named(debug_mask, debug_mask, int, 0644);

static ssize_t encoder_debug_status(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", dynamic_debug);
}

static ssize_t encoder_set_debug(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (data){
		dynamic_debug = 1;
	}
	else{
		dynamic_debug = 0;
	}

	return count;
}

static struct encoder_config_info encoder_info = {
	.input_type = ENCODER,
};

static struct device_attribute encoder_attributes[] = {
	__ATTR(interrupt, 0644, encoder_debug_status, encoder_set_debug),
	__ATTR_NULL
};

/*
static struct rotary_encoder_platform_data my_rotary_encoder_info = {
	.steps		= 24,
	.axis		= ABS_X,
	.relative_axis	= false,
	.rollover	= false,
	.gpio_a		= GPIO_ROTARY_A,
	.gpio_b		= GPIO_ROTARY_B,
	.inverted_a	= 0,
	.inverted_b	= 0,
	.half_period	= false,
};

static struct platform_device rotary_encoder_device = {
	.name		= "hansung-encoder",
	.id		= 0,
	.dev		= {
	.platform_data = &my_rotary_encoder_info,
	}
};
*/

static irqreturn_t encoder_int_handler(int irq, void *dev_id)
{
	dprintk(DEBUG_DATA_INFO, "encoder int handler, %d\n", irq);
	return IRQ_HANDLED;
}

static int __init hs_enc_init(void)
{
	dprintk(DEBUG_INIT, "hs_enc : init\n");
	//platform_device_register(&rotary_encoder_device);

	if (input_fetch_sysconfig_para(&(encoder_info.input_type))) {
		printk("%s: err.\n", __func__);
		return -1;
	}

	if(input_init_platform_resource(&(encoder_info.input_type)))
	{
		printk("%s: err.\n", __func__);
		return -1;
	}

	if(input_request_int(&(encoder_info.input_type), encoder_int_handler, \
				IRQF_TRIGGER_RISING, NULL))
	{
		printk("%s: err.\n", __func__);
		return -1;
	}

	dprintk(DEBUG_INIT, "%s: init succeed\n", __func__);


	return 0;
}

static void __exit hs_enc_exit(void)
{
	input_free_platform_resource(&(encoder_info.input_type));

	if(input_free_int(&(encoder_info.input_type), NULL))
	{
		printk("%s: err.\n", __func__);
	}
}

MODULE_AUTHOR("Jong Ok, KIM <kimjongok@empal.com>");
MODULE_DESCRIPTION("Encoder driver for Hansung");
MODULE_LICENSE("GPL");

module_init(hs_enc_init);
module_exit(hs_enc_exit);
