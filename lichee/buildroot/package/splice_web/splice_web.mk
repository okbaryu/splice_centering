SPLICE_WEB_DIR = $(BUILD_DIR)/splice_web
LDFLAGS += -lpthread -lm -ljson-c -L${SPLICE_WEB_DIR} -lcmdtool
MAIN_OBJS = ${SPLICE_WEB_DIR}/splice_cgi.c
SERVER_OBJS = ${SPLICE_WEB_DIR}/splice_server.c ${SPLICE_WEB_DIR}/actuator.c ${SPLICE_WEB_DIR}/plc.c ${SPLICE_WEB_DIR}/centering.c \
			  ${SPLICE_WEB_DIR}/centering_libs.c ${SPLICE_WEB_DIR}/cmd_parser_init.c ${SPLICE_WEB_DIR}/cmd_parser_trace.c ${SPLICE_WEB_DIR}/cmd_parser_centering.c
EXTRA_OBJS += ${SPLICE_WEB_DIR}/splice_libs.c

MINIGUI_OBJS += ${SPLICE_WEB_DIR}/gui_lcd.c ${SPLICE_WEB_DIR}/serial.c
MINIGUI_LD += -lminigui_ths -lfreetype -lpthread -ljpeg 

CFLAGS += -I${SPLICE_WEB_DIR}/include
CFLAGS += -Wimplicit-function-declaration -Wall -lm

# Always build with debug
CFLAGS += -g -I$(STAGING_DIR)/usr/include/json-c/
LDFLAGS += -ljson-c

$(SPLICE_WEB_DIR)/.source :
	mkdir -pv $(SPLICE_WEB_DIR)
	cp -rf package/splice_web/src/* $(SPLICE_WEB_DIR)
	cp -rf package/splice_web/MiniGUI.cfg $(SPLICE_WEB_DIR)
	cp -rf package/splice_web//font/NanumGothic.ttf $(SPLICE_WEB_DIR)
	touch $@

$(SPLICE_WEB_DIR)/.configured : $(SPLICE_WEB_DIR)/.source
	touch $@

splice_web-binary: $(SPLICE_WEB_DIR)/.configured
	$(MAKE) BUILD_DIR=$(BUILD_DIR) LINK="$(TARGET_CC)" CC="$(TARGET_CC)" -C $(TIOBENCH_DIR) all

${SPLICE_WEB_DIR}/splice.cgi: $(MAIN_OBJS) $(EXTRA_OBJS)
	@echo [Building ... $@]
	$(TARGET_CC) -o $@ $(MAIN_OBJS) $(EXTRA_OBJS) $(CFLAGS) $(LDFLAGS)

${SPLICE_WEB_DIR}/splice_server: $(SERVER_OBJS)
	@echo [Building ... $@]
	$(TARGET_CC) -o $@ $(SERVER_OBJS) $(EXTRA_OBJS) $(CFLAGS) $(LDFLAGS)

${SPLICE_WEB_DIR}/gui_lcd: $(MINIGUI_OBJS)
	@echo [Building ... $@]
	$(TARGET_CC) -o $@ $(MINIGUI_OBJS) $(MINIGUI_LD) -I$(STAGING_DIR)/usr/include -L$(TARGET_DIR)/usr/lib $(CFLAGS) $(LDFLAGS)

splice_web : $(SPLICE_WEB_DIR)/.configured ${SPLICE_WEB_DIR}/splice.cgi ${SPLICE_WEB_DIR}/splice_server ${SPLICE_WEB_DIR}/gui_lcd
	install -d -m 755 $(TARGET_DIR)/var/splice_web/
	install -d -m 755 $(TARGET_DIR)/var/splice_web/font
	install -D -m 755 $(SPLICE_WEB_DIR)/splice.cgi $(TARGET_DIR)/var/splice_web/splice.cgi
	install -D -m 755 $(SPLICE_WEB_DIR)/splice_server $(TARGET_DIR)/var/splice_web/splice_server
	install -D -m 755 $(SPLICE_WEB_DIR)/gui_lcd $(TARGET_DIR)/var/splice_web/gui_lcd
	install -D -m 755 $(SPLICE_WEB_DIR)/MiniGUI.cfg $(TARGET_DIR)/var/splice_web/
	install -D -m 755 $(SPLICE_WEB_DIR)/NanumGothic.ttf $(TARGET_DIR)/var/splice_web/font
	cp -rf package/splice_web/* $(TARGET_DIR)/var/splice_web
	ln -sf /usr/bin/php-cgi $(TARGET_DIR)/var/splice_web/php
	ln -sf /data/ipaddr $(TARGET_DIR)/var/splice_web/ipaddr
	ln -sf /data/usersdb.php $(TARGET_DIR)/var/splice_web/usersdb.php


##############################################################
#
# Add our target
#
#############################################################
TARGETS += splice_web

