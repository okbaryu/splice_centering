#############################################################
#
# minigui
#
#############################################################

MINIGUI_VERSION = 3.0.12
MINIGUI_REVISION = r1
MINIGUI_SOURCE = minigui-$(MINIGUI_VERSION)-$(MINIGUI_REVISION).tar.gz
MINIGUI_INC_SRC_DIR = $(BUILD_DIR)/minigui-$(MINIGUI_VERSION)/include
MINIGUI_HEADERS = $(MINIGUI_INC_SRC_DIR)/common.h \
				  $(MINIGUI_INC_SRC_DIR)/endianrw.h \
				  $(MINIGUI_INC_SRC_DIR)/fixedmath.h \
				  $(MINIGUI_INC_SRC_DIR)/minigui.h \
				  $(MINIGUI_INC_SRC_DIR)/gdi.h \
				  $(MINIGUI_INC_SRC_DIR)/window.h \
				  $(MINIGUI_INC_SRC_DIR)/control.h \
				  $(MINIGUI_INC_SRC_DIR)/own_malloc.h \
				  $(MINIGUI_INC_SRC_DIR)/own_stdio.h \
				  $(MINIGUI_INC_SRC_DIR)/xvfb.h \
				  $(MINIGUI_INC_SRC_DIR)/customial.h \
				  $(MINIGUI_INC_SRC_DIR)/dti.c
MINIGUI_INSTALL_STAGING = YES
MINIGUI_INSTALL_TARGET = YES
MINIGUI_CONF_OPT = --prefix=${TARGET_DIR} --host=arm-linux --build=i386-linux --with-targetname=fbcon --enable-sunximin --enable-savescreen \
	--enable-jpgsupport --enable-debug --enable-adv2dapi --enable-unicodesupport --with-ttfsupport=ft2 --enable-bmpfsupport \
	--with-ft2-includes=$(STAGING_DIR)/usr/include/freetype2

define MINIGUI_INSTALL_STAGING_CMDS
	$(INSTALL) -d -m 0755 $(STAGING_DIR)/usr/include/minigui
	$(INSTALL) -D -m 0644 $(MINIGUI_HEADERS) $(STAGING_DIR)/usr/include/minigui
endef

$(eval $(call AUTOTARGETS,package,minigui))

