SPLICE_WEB_DIR = $(BUILD_DIR)/splice_web
LDFLAGS += -lpthread -lm
MAIN_OBJS = ${SPLICE_WEB_DIR}/splice_cgi.c
SERVER_OBJS = ${SPLICE_WEB_DIR}/splice_server.c
EXTRA_OBJS += ${SPLICE_WEB_DIR}/splice_libs.c

CFLAGS += -I${SPLICE_WEB_DIR}
CFLAGS += -Wimplicit-function-declaration -Wall -lm

# Always build with debug
CFLAGS += -g
CFLAGS += -D_LINUX_TYPES_H 

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
	install -d -m 755 $(TARGET_DIR)/var/splice_web
	install -D -m 755 $(SPLICE_WEB_DIR)/splice.cgi $(TARGET_DIR)/var/splice_web/splice.cgi
	install -D -m 755 $(SPLICE_WEB_DIR)/splice_server $(TARGET_DIR)/var/splice_web/splice_server
	install -D -m 644 package/splice_web/boundingbox.js $(TARGET_DIR)/var/splice_web
	install -D -m 644 package/splice_web/chart1.js $(TARGET_DIR)/var/splice_web
	install -D -m 644 package/splice_web/chart2.js $(TARGET_DIR)/var/splice_web
	install -D -m 644 package/splice_web/hs_logo.gif $(TARGET_DIR)/var/splice_web
	install -D -m 644 package/splice_web/splice.css $(TARGET_DIR)/var/splice_web
	install -D -m 644 package/splice_web/splice.html $(TARGET_DIR)/var/splice_web
	install -D -m 644 package/splice_web/splice.js $(TARGET_DIR)/var/splice_web
	install -D -m 644 package/splice_web/svgvml3d.js $(TARGET_DIR)/var/splice_web
	ln -s   /usr/bin/php-cgi $(TARGET_DIR)/var/splice_web/php


##############################################################
#
# Add our target
#
#############################################################
TARGETS += splice_web

