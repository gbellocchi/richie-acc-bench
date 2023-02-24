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
LIBPATHS += -L$(PULP_SDK_HOME)/install/lib/hero-sim
LDFLAGS_PULP += -larov-target
LDFLAGS += -larov-target

# Link HWPE libraries
LIBPATHS += -L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl0_lic0 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl0_lic1 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl0_lic2 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl0_lic3

LDFLAGS_PULP += -lhwpe_cl0_lic0 -lhwpe_cl0_lic1 -lhwpe_cl0_lic2 -lhwpe_cl0_lic3

LDFLAGS += 	-lhwpe_cl0_lic0 -lhwpe_cl0_lic1 -lhwpe_cl0_lic2 -lhwpe_cl0_lic3

