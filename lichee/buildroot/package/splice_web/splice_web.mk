SPLICE_WEB_DIR = $(BUILD_DIR)/splice_web
LDFLAGS += -lpthread -lm -ljson-c -L${SPLICE_WEB_DIR} -lcmdtool
MAIN_OBJS = ${SPLICE_WEB_DIR}/splice_cgi.c
SERVER_OBJS = ${SPLICE_WEB_DIR}/splice_server.c ${SPLICE_WEB_DIR}/actuator.c ${SPLICE_WEB_DIR}/plc.c ${SPLICE_WEB_DIR}/centering.c
EXTRA_OBJS += ${SPLICE_WEB_DIR}/splice_libs.c ${SPLICE_WEB_DIR}/cmd_parser_init.c ${SPLICE_WEB_DIR}/cmd_parser_trace.c

CFLAGS += -I${SPLICE_WEB_DIR}/include
CFLAGS += -Wimplicit-function-declaration -Wall -lm

# Always build with debug
CFLAGS += -g

$(SPLICE_WEB_DIR)/.source :
	mkdir -pv $(SPLICE_WEB_DIR)
	cp -rf package/splice_web/src/* $(SPLICE_WEB_DIR)
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

splice_web : $(SPLICE_WEB_DIR)/.configured ${SPLICE_WEB_DIR}/splice.cgi ${SPLICE_WEB_DIR}/splice_server
	install -d -m 755 $(TARGET_DIR)/var/splice_web/
	install -D -m 755 $(SPLICE_WEB_DIR)/splice.cgi $(TARGET_DIR)/var/splice_web/splice.cgi
	install -D -m 755 $(SPLICE_WEB_DIR)/splice_server $(TARGET_DIR)/var/splice_web/splice_server
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

