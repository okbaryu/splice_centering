################################################################################
#
# android-tools
#
################################################################################

ANDROID_TOOLS_SITE = https://launchpad.net/ubuntu/+archive/primary/+files
ANDROID_TOOLS_VERSION = 4.2.2+git20130218
ANDROID_TOOLS_SOURCE = android-tools_$(ANDROID_TOOLS_VERSION).orig.tar.xz
#https://launchpad.net/ubuntu/+archive/primary/+files/android-tools_4.2.2+git20130218-3ubuntu13.debian.tar.gz
#https://launchpad.net/ubuntu/+archive/primary/+files/android-tools_4.2.2+git20130218-3ubuntu41.debian.tar.gz
#https://launchpad.net/ubuntu/+archive/primary/+files/android-tools_4.2.2+git20130218-3ubuntu42.debian.tar.xz
ANDROID_TOOLS_EXTRA_DOWNLOADS = android-tools_$(ANDROID_TOOLS_VERSION)-3ubuntu42.debian.tar.xz
HOST_ANDROID_TOOLS_EXTRA_DOWNLOADS = $(ANDROID_TOOLS_EXTRA_DOWNLOADS)
ANDROID_TOOLS_LICENSE = Apache-2.0
ANDROID_TOOLS_LICENSE_FILES = debian/copyright
ANDROID_TOOLS_DIR:=$(BUILD_DIR)/android_tools-$(ANDROID_TOOLS_VERSION)

ANDROID_TOOLS_TARGET_MAKE_ENV = PKG_CONFIG_PATH="$(STAGING_DIR)/usr/lib/pkgconfig:$(PKG_CONFIG_PATH)"

# Extract the Debian tarball inside the sources
define ANDROID_TOOLS_DEBIAN_EXTRACT
	$(call suitable-extractor,$(notdir $(ANDROID_TOOLS_EXTRA_DOWNLOADS))) \
		$(DL_DIR)/$(notdir $(ANDROID_TOOLS_EXTRA_DOWNLOADS)) | \
		$(TAR) -C $(@D) $(TAR_OPTIONS) -
endef

HOST_ANDROID_TOOLS_POST_EXTRACT_HOOKS += ANDROID_TOOLS_DEBIAN_EXTRACT
ANDROID_TOOLS_POST_EXTRACT_HOOKS += ANDROID_TOOLS_DEBIAN_EXTRACT


ifeq ($(BR2_PACKAGE_HOST_ANDROID_TOOLS_FASTBOOT),y)
HOST_ANDROID_TOOLS_TARGETS += fastboot
HOST_ANDROID_TOOLS_DEPENDENCIES += host-zlib host-libselinux
endif

ifeq ($(BR2_PACKAGE_HOST_ANDROID_TOOLS_ADB),y)
HOST_ANDROID_TOOLS_TARGETS += adb
HOST_ANDROID_TOOLS_DEPENDENCIES += host-zlib host-openssl
endif

ifeq ($(BR2_PACKAGE_ANDROID_TOOLS_FASTBOOT),y)
ANDROID_TOOLS_TARGETS += fastboot
ANDROID_TOOLS_DEPENDENCIES += zlib libselinux
endif

ifeq ($(BR2_PACKAGE_ANDROID_TOOLS_ADB),y)
ANDROID_TOOLS_TARGETS += adb
ANDROID_TOOLS_DEPENDENCIES += zlib openssl
endif

ifeq ($(BR2_PACKAGE_ANDROID_TOOLS_ADBD),y)
ANDROID_TOOLS_TARGETS += adbd
ANDROID_TOOLS_DEPENDENCIES += zlib openssl
endif

# Build each tool in its own directory not to share object files

define HOST_ANDROID_TOOLS_BUILD_CMDS
	$(foreach t,$(HOST_ANDROID_TOOLS_TARGETS),\
		mkdir -p $(@D)/build-$(t) && \
		$(HOST_MAKE_ENV) $(HOST_CONFIGURE_OPTS) $(MAKE) SRCDIR=$(@D) \
			-C $(@D)/build-$(t) -f $(@D)/debian/makefiles/$(t).mk$(sep))
endef

define ANDROID_TOOLS_BUILD_CMDS
	$(foreach t,$(ANDROID_TOOLS_TARGETS),\
		mkdir -p $(@D)/build-$(t) && \
		$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(ANDROID_TOOLS_TARGET_MAKE_ENV) $(MAKE) SRCDIR=$(@D) \
			-C $(@D)/build-$(t) -f $(@D)/debian/makefiles/$(t).mk$(sep))
endef

define HOST_ANDROID_TOOLS_INSTALL_CMDS
	$(foreach t,$(HOST_ANDROID_TOOLS_TARGETS),\
		$(INSTALL) -D -m 0755 $(@D)/build-$(t)/$(t) $(HOST_DIR)/usr/bin/$(t)$(sep))
endef

define ANDROID_TOOLS_INSTALL_TARGET_CMDS
	if [ "$(strip $(filter %adbd,$(ANDROID_TOOLS_TARGETS)))" = "adbd" ]; then \
		$(INSTALL) -m 0755 package/android_tools/android-gadget-setup $(TARGET_DIR)/usr/bin/; \
	fi
	$(foreach t,$(ANDROID_TOOLS_TARGETS),\
		$(INSTALL) -D -m 0755 $(@D)/build-$(t)/$(t) $(TARGET_DIR)/usr/bin/$(t)$(sep))
endef

$(eval $(call GENTARGETS,package,android_tools))
$(eval $(call GENTARGETS,package,android_tools,host))

