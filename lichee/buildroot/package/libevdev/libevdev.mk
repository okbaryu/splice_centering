################################################################################
#
# libevdev
#
################################################################################

LIBEVDEV_VERSION = 1.5.6
LIBEVDEV_SITE = http://www.freedesktop.org/software/libevdev
LIBEVDEV_SOURCE = libevdev-$(LIBEVDEV_VERSION).tar.xz
LIBEVDEV_LICENSE = X11
LIBEVDEV_LICENSE_FILES = COPYING

# patch touches configure.ac
LIBEVDEV_AUTORECONF = YES

# Uses PKG_CHECK_MODULES() in configure.ac
LIBEVDEV_DEPENDENCIES = host-pkg-config

LIBEVDEV_INSTALL_STAGING = YES

LIBEVDEV_CONF_OPT += --disable-runtime-tests

$(eval $(call AUTOTARGETS,package,libevdev))
