LCD_DIR = $(BUILD_DIR)/lcd
LCD_OBJS = $(LCD_DIR)/lcd.c
CLIENT_OBJS = $(LCD_DIR)/client.c
SERIAL_OBJS ?= $(LCD_DIR)/serial.c
INSTALL_DIR = $(TARGET_DIR)/var/splice_web

LCD_CFLAGS += -Wimplicit-function-declaration -Wall -I$(STAGING_DIR)/usr/include/json-c/

# Always build with debug
LCD_CFLAGS += -g
LCD_LDFLAGS = -lpthread -ljson-c

$(LCD_DIR)/.source :
	mkdir -pv $(LCD_DIR)
	cp -rf package/lcd/*.c $(LCD_DIR)
	touch $@

$(LCD_DIR)/.configured : $(LCD_DIR)/.source
	touch $@

${LCD_DIR}/lcd : $(LCD_OBJS) $(SERIAL_OBJS)
	@echo [Building ... $@]
	$(TARGET_CC) -o $@ $(LCD_OBJS) $(SERIAL_OBJS) $(LCD_CFLAGS) $(LCD_LDFLAGS)

${LCD_DIR}/client : $(CLIENT_OBJS)
	@echo [Building ... $@]
	$(TARGET_CC) -o $@ $(CLIENT_OBJS) $(LCD_CFLAGS) $(LCD_LDFLAGS)

lcd : $(LCD_DIR)/.configured ${LCD_DIR}/lcd ${LCD_DIR}/client
	install -d -m 755 $(INSTALL_DIR)
	install -D -m 755 $(LCD_DIR)/lcd $(INSTALL_DIR)/lcd
	install -D -m 755 $(LCD_DIR)/client $(INSTALL_DIR)/client
	cp -rf package/lcd/font $(INSTALL_DIR)


##############################################################
#
# Add our target
#
#############################################################
TARGETS += lcd

