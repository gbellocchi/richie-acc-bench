ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

CSRCS = $(wildcard *.c)

-include $(HERO_OV_OPENMP_TESTS)/common/default.mk 

INCPATHS += -I$(PULP_SDK_HOME)/install/include -I$(ROOT) $(include_paths)

ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# Add experiment binaries
ldflags += $(experiment_bins)

# Link PULP libraries
CFLAGS_PULP += -g -include $(PULP_SDK_HOME)/install/hero/hero-sim/cl_config.h
LDFLAGS_PULP += -lomptarget-pulp -lrt

# Link AROV library
LIBPATHS += -L$(ESWEEK_DIR)/sw/libs/arov_ncl1_nacc16/libarov-target/lib/
LDFLAGS_PULP += -larov-target
LDFLAGS += -larov-target

# Link HWPE libraries
LIBPATHS += -L$(ESWEEK_DIR)/sw/libs/arov_ncl1_nacc16/libhwpe/hwpe_cl0_lic0/lib/hwpe_cl0_lic0

LDFLAGS_PULP += -lhwpe_cl0_lic0

LDFLAGS += 	-lhwpe_cl0_lic0