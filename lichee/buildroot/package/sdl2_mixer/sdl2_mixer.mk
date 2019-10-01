################################################################################
#
# sdl2_mixer
#
################################################################################

SDL2_MIXER_VERSION = 2.0.1
SDL2_MIXER_SOURCE = SDL2_mixer-$(SDL2_MIXER_VERSION).tar.gz
SDL2_MIXER_SITE = http://www.libsdl.org/projects/SDL_mixer/release
SDL2_MIXER_LICENSE = Zlib
SDL2_MIXER_LICENSE_FILES = COPYING.txt
SDL2_MIXER_INSTALL_STAGING = YES
SDL2_MIXER_DEPENDENCIES = sdl2 host-pkg-config

SDL2_MIXER_CONF_OPT = \
	--disable-fluidsynth \
	--disable-music-mp3

ifeq ($(BR2_PACKAGE_FLAC),y)
SDL2_MIXER_CONF_OPT += --enable-music-flac
SDL2_MIXER_DEPENDENCIES += flac
else
SDL2_MIXER_CONF_OPT += --disable-music-flac
endif

ifeq ($(BR2_PACKAGE_LIBMODPLUG),y)
SDL2_MIXER_CONF_OPT += --enable-music-mod-modplug
SDL2_MIXER_DEPENDENCIES += libmodplug
else
SDL2_MIXER_CONF_OPT += --disable-music-mod-modplug
endif

ifeq ($(BR2_PACKAGE_TREMOR),y)
SDL2_MIXER_CONF_OPT += --enable-music-ogg-tremor
SDL2_MIXER_DEPENDENCIES += tremor
else
SDL2_MIXER_CONF_OPT += --disable-music-ogg-tremor
endif

$(eval $(call AUTOTARGETS,package,sdl2_mixer))
