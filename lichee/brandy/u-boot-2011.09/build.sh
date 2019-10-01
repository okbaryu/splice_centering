#!/bin/bash
set -e

PLATFORM="sun6i"
MODULE=""
TOOLSPATH=`pwd`
show_help()
{
	printf "\nbuild.sh - Top level build scritps\n"
	echo "Valid Options:"
	echo "  -h  Show help message"
	echo "  -p <platform> platform, e.g. sun4i, sun4i-lite, sun4i_crane"
	printf "  -m <module> module\n\n"
}

while getopts hp:m: OPTION
do
	case $OPTION in
	h) show_help
	;;
	p) PLATFORM=$OPTARG
	;;
	m) MODULE=$OPTARG
	;;
	*) show_help
	;;
esac
done

if [ -z "$PLATFORM" ]; then
	show_help
	exit 1
fi

if [ -z "$MODULE" ]; then
	MODULE="all"
fi

export PATH=$PATH:${TOOLSPATH}/../../out/sun8iw8p1/linux/common/buildroot/external-toolchain/bin/
if [ "sun8iw8p1" == ${PLATFORM} ]; then
	make distclean CROSS_COMPILE=arm-linux-gnueabi- && make -j16 $PLATFORM CROSS_COMPILE=arm-linux-gnueabi- && make -j16 boot0 CROSS_COMPILE=arm-linux-gnueabi- \
    && make -j16 fes CROSS_COMPILE=arm-linux-gnueabi-
elif [ "sun8iw8p1_nor" == ${PLATFORM} ]; then
	make distclean CROSS_COMPILE=arm-linux-gnueabi- && make -j16 $PLATFORM CROSS_COMPILE=arm-linux-gnueabi-
elif [ "sun8iw8p1_spinand_emmc" == ${PLATFORM} ]; then
	make distclean CROSS_COMPILE=arm-linux-gnueabi- && make -j16 $PLATFORM CROSS_COMPILE=arm-linux-gnueabi-
fi
