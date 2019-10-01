#############################################################
#
# libpng (Portable Network Graphic library)
#
#############################################################

LIBPNG_VERSION = 1.6.28
LIBPNG_SERIES = 16
LIBPNG_SOURCE = libpng-$(LIBPNG_VERSION).tar.xz
LIBPNG_SITE = http://downloads.sourceforge.net/project/libpng/libpng${LIBPNG_SERIES}/$(LIBPNG_VERSION)
LIBPNG_LICENSE = libpng license
LIBPNG_LICENSE_FILES = LICENSE
LIBPNG_INSTALL_STAGING = YES
LIBPNG_DEPENDENCIES = host-pkg-config zlib
HOST_LIBPNG_DEPENDENCIES = host-pkg-config host-zlib
LIBPNG_CONFIG_SCRIPTS = libpng$(LIBPNG_SERIES)-config libpng-config
LIBPNG_CONF_OPT = $(if $(BR2_ARM_CPU_HAS_NEON),--enable-arm-neon=yes,--enable-arm-neon=no)

$(eval $(call AUTOTARGETS,package,libpng))
$(eval $(call AUTOTARGETS,package,libpng,host))
