ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

CSRCS = $(wildcard *.c)

-include $(HERO_OV_OPENMP_TESTS)/common/default.mk 

INCPATHS += -I$(PULP_SDK_HOME)/install/include -I$(ROOT) $(include_paths)

# Add experiment binaries
ldflags += $(experiment_bins)

# Link PULP libraries
CFLAGS_PULP += -g -include $(PULP_SDK_HOME)/install/hero/hero-sim/cl_config.h
LDFLAGS_PULP += -lomptarget-pulp -lrt

# Link AROV library
LIBPATHS += -L$(PULP_SDK_HOME)/install/lib/hero-sim
LDFLAGS_PULP += -larov-target
LDFLAGS += -larov-target