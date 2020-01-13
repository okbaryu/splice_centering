#############################################################
#
# minigui
#
#############################################################

MINIGUI_VERSION = 3.0.12
MINIGUI_REVISION = r1
MINIGUI_SOURCE = minigui-$(MINIGUI_VERSION)-$(MINIGUI_REVISION).tar.gz
MINIGUI_INSTALL_STAGING = YES
MINIGUI_INSTALL_TARGET = YES
MINIGUI_CONF_OPT = --prefix=${TARGET_DIR} --host=arm-linux --build=i386-linux --with-targetname=fbcon --enable-sunximin --enable-savescreen \
	--enable-jpgsupport --enable-debug --enable-adv2dapi --enable-unicodesupport --with-ttfsupport=ft2 --enable-bmpfsupport \
	--with-ft2-includes=$(STAGING_DIR)/usr/include/freetype2

$(eval $(call AUTOTARGETS,package,minigui))

