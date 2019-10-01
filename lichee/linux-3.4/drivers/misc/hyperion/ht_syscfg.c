#include "ht_syscfg.h"

#include <linux/module.h>

#if 0
#define dlog(...)	do { printk("[ht_syscfg] " __VA_ARGS__); } while (0)
#else
#define dlog(...)
#endif

int syscfg_get_bool(const char *section, const char *name)
{
    return syscfg_get_int(section, name);
}
EXPORT_SYMBOL(syscfg_get_bool);

int syscfg_get_int(const char *section, const char *name)
{
	script_item_value_type_e type;
	script_item_u val;

	type = script_get_item((char *)section, (char *)name, &val);
	if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
		dlog("failed to get int %s.%s\n", section, name);
		return -1;
	}

	return val.val;
}
EXPORT_SYMBOL(syscfg_get_int);

const char * syscfg_get_string(const char *section, const char *name, char *value, size_t size)
{
	script_item_value_type_e type;
	script_item_u val;

	type = script_get_item((char *)section, (char *)name, &val);
	if (SCIRPT_ITEM_VALUE_TYPE_STR != type) {
		dlog("failed to get string %s.%s\n", section, name);
		return NULL;
	}

	return strncpy(value, val.str, size);
}
EXPORT_SYMBOL(syscfg_get_string);

int syscfg_get_gpio(const char *section, const char *name, struct gpio_config *gpio)
{
	script_item_value_type_e type;
	script_item_u val;

	type = script_get_item((char *)section, (char *)name, &val);
	if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
		dlog("failed to get gpio %s.%s\n", section, name);
		return -1;
	}

	if (gpio != NULL) {
		*gpio = val.gpio;
	}

	return val.gpio.gpio;
}
EXPORT_SYMBOL(syscfg_get_gpio);
