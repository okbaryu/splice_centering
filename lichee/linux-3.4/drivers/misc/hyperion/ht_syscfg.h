#ifndef __HT_SYSCFG_H__
#define __HT_SYSCFG_H__

#include <mach/sys_config.h>

extern int sunxi_gpio_req(struct gpio_config *gpio);

int syscfg_get_bool(const char *section, const char *name);
int syscfg_get_int(const char *section, const char *name);
const char * syscfg_get_string(const char *section, const char *name, char *value, size_t size);
int syscfg_get_gpio(const char *section, const char *name, struct gpio_config *gpio);

#endif
