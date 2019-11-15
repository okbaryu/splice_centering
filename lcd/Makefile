TOPDIR=./
TOOL_PATH=$(TOPDIR)/external-toolchain/bin
CROSS_COMPILE=arm-linux-gnueabihf-
CC=$(TOOL_PATH)/$(CROSS_COMPILE)gcc
CPP=$(TOOL_PATH)/$(CROSS_COMPILE)g++
CFLAGS_X=-Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter
CFLAGS_X1=-pipe -Os -mtune=cortex-a7 -march=armv7-a -mabi=aapcs-linux -mhard-float
CFLAGS= --sysroot=$(TOPDIR)/staging $(CFLAGS_X) $(CFLAGS_X1) -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -I$(TOPDIR)/staging/usr/include -I./include -I$(TOOL_PATH)/include/json-c
CFLAGS+= -D__OS_LINUX
LOCAL_LIB_PATH=./linux_lib/
LIBS=-lpthread -lVE -lvencoder -lMemAdapter -ljson-c -L$(TOOL_PATH)/lib -L$(LOCAL_LIB_PATH) 

SRCCS = main.c serial.c
OBJ      := $(SRCCS:%.c=%.o)
TARGET=lcd

$(TARGET): $(OBJ)
	$(CPP) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	rm -rf $(TARGET) $(OBJ)
