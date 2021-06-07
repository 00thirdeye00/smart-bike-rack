# Contiki network configuration

#MAKE_MAC = MAKE_MAC_CSMA
#MAKE_NET = MAKE_NET_NULLNET

CONTIKI_PROJECT = observer login
PROJECT_SOURCEFILES += accl.c dist.c #alarm.c
all: $(CONTIKI_PROJECT)

CONTIKI = /home/wcnes/contiki-ng-wcnes

# ----- below copied from timesync-demo example ---- 
#PLATFORMS_EXCLUDE = sky nrf52dk native

# force Orchestra from command line
MAKE_WITH_ORCHESTRA ?= 0
# force Security from command line
MAKE_WITH_SECURITY ?= 0

MAKE_MAC = MAKE_MAC_TSCH

include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/shell

ifeq ($(MAKE_WITH_ORCHESTRA),1)
MODULES += $(CONTIKI_NG_SERVICES_DIR)/orchestra
endif

ifeq ($(MAKE_WITH_SECURITY),1)
CFLAGS += -DWITH_SECURITY=1
endif

#border-router
MODULES += $(CONTIKI_NG_SERVICES_DIR)/rpl-border-router
#RIinclude webserver module
MODULES_REL += webserver
# Include optional target-specific module
include $(CONTIKI)/Makefile.identify-target
MODULES_REL += $(TARGET)


include $(CONTIKI)/Makefile.include
