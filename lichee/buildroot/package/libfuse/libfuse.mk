#############################################################
#
# libfuse
#
#############################################################

LIBFUSE_VERSION = 3.0.1
LIBFUSE_SOURCE = fuse-$(LIBFUSE_VERSION).tar.gz
LIBFUSE_SITE = https://github.com/libfuse/libfuse/releases/download/fuse-$(LIBFUSE_VERSION)
LIBFUSE_LICENSE = GPLv2, LGPLv2.1
LIBFUSE_LICENSE_FILES = COPYING COPYING.LIB
LIBFUSE_INSTALL_STAGING = YES
LIBFUSE_DEPENDENCIES = $(if $(BR2_PACKAGE_LIBICONV),libiconv)
LIBFUSE_CONF_OPT = \
	--program-prefix="" \
	--disable-example \
	--enable-lib \
	--enable-util

define LIBFUSE_INSTALL_TARGET_CMDS
	cp -dpf $(STAGING_DIR)/usr/bin/fusermount3 $(TARGET_DIR)/usr/bin/
	cp -dpf $(STAGING_DIR)/usr/lib/libfuse3.so* $(TARGET_DIR)/usr/lib/
endef

define LIBFUSE_CLEAN_CMDS
	-$(MAKE) -C $(@D) DESTDIR=$(STAGING_DIR) uninstall
	-$(MAKE) -C $(@D) clean
	rm -f $(TARGET_DIR)/usr/bin/fusermount3 $(TARGET_DIR)/usr/lib/libfuse3.so*
endef

$(eval $(call AUTOTARGETS,package,libfuse))
