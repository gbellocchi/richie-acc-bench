

PULP_PROPERTIES += pulp_chip
include $(PULP_SDK_HOME)/install/rules/pulp_properties.mk

PULP_LIBS              			= arov-target
PULP_LIB_CL_SRCS_arov-target 	+= arov-target.c
PULP_CL_CFLAGS_arov-target    	= -Wall -O3 -g3 -I./ -I$(HERO_PULP_INC_DIR)

-include $(PULP_SDK_HOME)/install/rules/pulp.mk

header::
	mkdir -p $(PULP_SDK_HOME)/install/include/libarov-target
	cp ../inc/arov-target.h $(PULP_SDK_HOME)/install/include