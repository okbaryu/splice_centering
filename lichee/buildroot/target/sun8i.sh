#!/bin/sh

chmod +x ${LICHEE_BR_OUT}/target/lib/*
rm -rf ${LICHEE_BR_OUT}/target/init
(cd ${LICHEE_BR_OUT}/target && ln -s bin/busybox init)


#rm -rf ${LICHEE_BR_OUT}/target/lib/udev/rules.d

echo "${LICHEE_TOOLS_DIR}/pack/chips/${LICHEE_CHIP}/configs/${LICHEE_BOARD}"

if [ -d target/${LICHEE_BOARD}/cfg -a -d ${LICHEE_TOOLS_DIR}/pack/chips/${LICHEE_CHIP}/configs/${LICHEE_BOARD} ]; then
(cd target/${LICHEE_BOARD}/cfg && mkfs.jffs2 -n -r data -e 64 -o ${LICHEE_TOOLS_DIR}/pack/chips/${LICHEE_CHIP}/configs/${LICHEE_BOARD}/cfg.fex)
fi


echo "target/${LICHEE_BOARD}"
if [ -d target/${LICHEE_BOARD}/skel ]; then
(cd target/${LICHEE_BOARD}/skel && tar -c *) |tar -C ${LICHEE_BR_OUT}/target/ -xv
fi

find ${LICHEE_BR_OUT}/target/ -type f \( -name .empty -o -name '*~' \) -print0 | xargs -0 rm -vrf

#if [ -x ${LICHEE_BR_OUT}/target/etc/init.d/rcS ]; then
#
#fi
