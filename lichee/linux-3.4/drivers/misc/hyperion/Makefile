# Hyperion Tech Platform Support Driver

obj-$(CONFIG_HYPERION_HACK) += ht_syscfg.o

ifneq ($(strip $(CONFIG_ARCH_SUN8IW8P1)),)
obj-$(CONFIG_SUNXI_NAND) += ../../../modules/nand/sun8iw8p1/
endif

obj-$(CONFIG_HT_INPUT) += ht_input.o

obj-$(CONFIG_HT_RFKILL) += ht_rfkill.o
obj-$(CONFIG_HT_PLATDEV) += ht_platdev.o
obj-$(CONFIG_HT_BACKLIGHT) += ht_backlight.o

ht_input-objs += ht_input_mod.o ht_i2cdev.o ht_touchpanel.o

ht_platdev-objs := ht_platdev_drv.o

ifneq ($(CONFIG_HT_GPIO_KEYS),)
ht_input-objs += ht_gpio_keys.o
endif
ifneq ($(CONFIG_HT_SENSOR),)
ht_input-objs += ht_sensor.o
endif

ifneq ($(CONFIG_HT_GPIO_LEDS),)
ht_platdev-objs += ht_gpio_leds.o
endif
ifneq ($(CONFIG_HT_BACKLIGHT),)
ht_platdev-objs += ht_backlightdev.o
endif

ifneq ($(CONFIG_HT_MT66XX),)
ht_platdev-objs += ht_mt66xx.o
endif
ifneq ($(CONFIG_HT_XRADIO),)
ht_platdev-objs += ht_xradio.o
endif
