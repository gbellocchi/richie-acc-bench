

PULP_PROPERTIES += pulp_chip
include $(PULP_SDK_HOME)/install/rules/pulp_properties.mk

PULP_LIBS                  = hwpe_cl0_lic0
PULP_LIB_CL_SRCS_hwpe_cl0_lic0 += hwpe_cl0_lic0.c
PULP_CL_CFLAGS_hwpe_cl0_lic0    = -Wall -O3 -g3 -I./ -I$(HERO_PULP_INC_DIR)

-include $(PULP_SDK_HOME)/install/rules/pulp.mk

header::
	mkdir -p $(PULP_SDK_HOME)/install/include/libhwpe/archi
	mkdir -p $(PULP_SDK_HOME)/install/include/libhwpe/hal
	cp ../inc/hwpe_cl0_lic0.h $(PULP_SDK_HOME)/install/include/libhwpe
	cp ../inc/archi_hwpe.h $(PULP_SDK_HOME)/install/include/libhwpe/archi/hwpe_cl0_lic0.h
	cp ../inc/hal_hwpe.h $(PULP_SDK_HOME)/install/include/libhwpe/hal/hwpe_cl0_lic0.h