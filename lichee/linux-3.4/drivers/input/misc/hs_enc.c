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

static struct device_attribute encoder_attributes[] = {
	__ATTR(interrupt, 0644, encoder_debug_status, encoder_set_debug),
	__ATTR_NULL
};

static struct encoder_config_info encoder_info = {
	.input_type = ENCODER,
};

static struct rotary_encoder_platform_data my_rotary_encoder_info = {
};

static struct platform_device rotary_encoder_device = {
	.name		= "rotary-encoder",
	.id		= 0,
	.dev		= {
	.platform_data = &my_rotary_encoder_info,
	}
};

static int __init hs_enc_init(void)
{
	dprintk(DEBUG_INIT, "hs_enc : init\n");

	if (input_fetch_sysconfig_para(&(encoder_info.input_type))) {
		printk("%s: err.\n", __func__);
		return -1;
	}

	my_rotary_encoder_info.steps = encoder_info.steps;
	my_rotary_encoder_info.gpio_a = encoder_info.irq_gpio_a.gpio;
	my_rotary_encoder_info.gpio_b = encoder_info.irq_gpio_b.gpio;
	my_rotary_encoder_info.axis = encoder_info.axis;
	my_rotary_encoder_info.relative_axis = encoder_info.axis_relative;
	my_rotary_encoder_info.rollover = encoder_info.rollover;
	my_rotary_encoder_info.half_period = encoder_info.encoder_half_period;

	dprintk(DEBUG_INIT, "steps = %d, gpio_a = %d, gpio_b = %d, axis = %d, relative_axis = %d, rollover = %d, half = %d\n",\
			my_rotary_encoder_info.steps, my_rotary_encoder_info.gpio_a, my_rotary_encoder_info.gpio_b,\
			my_rotary_encoder_info.axis, my_rotary_encoder_info.relative_axis, my_rotary_encoder_info.rollover,\
			my_rotary_encoder_info.half_period);

	platform_device_register(&rotary_encoder_device);

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
