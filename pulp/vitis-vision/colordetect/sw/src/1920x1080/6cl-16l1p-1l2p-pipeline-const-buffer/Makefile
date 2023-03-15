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
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl0_lic3 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl0_lic4 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl0_lic5 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl1_lic0 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl1_lic1 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl1_lic2 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl1_lic3 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl1_lic4 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl1_lic5 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl2_lic0 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl2_lic1 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl2_lic2 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl2_lic3 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl2_lic4 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl2_lic5 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl3_lic0 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl3_lic1 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl3_lic2 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl3_lic3 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl3_lic4 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl3_lic5 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl4_lic0 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl4_lic1 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl4_lic2 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl4_lic3 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl4_lic4 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl4_lic5 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl5_lic0 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl5_lic1 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl5_lic2 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl5_lic3 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl5_lic4 \
			-L/home/gbellocchi/workspace_pulp/hero/hero-arov/pulp/sdk/pkg/sdk/dev/install/lib/hero-sim/hwpe_cl5_lic5

LDFLAGS_PULP += -lhwpe_cl0_lic0 -lhwpe_cl0_lic1 -lhwpe_cl0_lic2 -lhwpe_cl0_lic3 -lhwpe_cl0_lic4 -lhwpe_cl0_lic5 \
				-lhwpe_cl1_lic0 -lhwpe_cl1_lic1 -lhwpe_cl1_lic2 -lhwpe_cl1_lic3 -lhwpe_cl1_lic4 -lhwpe_cl1_lic5 \
				-lhwpe_cl2_lic0 -lhwpe_cl2_lic1 -lhwpe_cl2_lic2 -lhwpe_cl2_lic3 -lhwpe_cl2_lic4 -lhwpe_cl2_lic5 \
				-lhwpe_cl3_lic0 -lhwpe_cl3_lic1 -lhwpe_cl3_lic2 -lhwpe_cl3_lic3 -lhwpe_cl3_lic4 -lhwpe_cl3_lic5 \
				-lhwpe_cl4_lic0 -lhwpe_cl4_lic1 -lhwpe_cl4_lic2 -lhwpe_cl4_lic3 -lhwpe_cl4_lic4 -lhwpe_cl4_lic5 \
				-lhwpe_cl5_lic0 -lhwpe_cl5_lic1 -lhwpe_cl5_lic2 -lhwpe_cl5_lic3 -lhwpe_cl5_lic4 -lhwpe_cl5_lic5

LDFLAGS += 	-lhwpe_cl0_lic0 -lhwpe_cl0_lic1 -lhwpe_cl0_lic2 -lhwpe_cl0_lic3 -lhwpe_cl0_lic4 -lhwpe_cl0_lic5 \
			-lhwpe_cl1_lic0 -lhwpe_cl1_lic1 -lhwpe_cl1_lic2 -lhwpe_cl1_lic3 -lhwpe_cl1_lic4 -lhwpe_cl1_lic5 \
			-lhwpe_cl2_lic0 -lhwpe_cl2_lic1 -lhwpe_cl2_lic2 -lhwpe_cl2_lic3 -lhwpe_cl2_lic4 -lhwpe_cl2_lic5 \
			-lhwpe_cl3_lic0 -lhwpe_cl3_lic1 -lhwpe_cl3_lic2 -lhwpe_cl3_lic3 -lhwpe_cl3_lic4 -lhwpe_cl3_lic5 \
			-lhwpe_cl4_lic0 -lhwpe_cl4_lic1 -lhwpe_cl4_lic2 -lhwpe_cl4_lic3 -lhwpe_cl4_lic4 -lhwpe_cl4_lic5 \
			-lhwpe_cl5_lic0 -lhwpe_cl5_lic1 -lhwpe_cl5_lic2 -lhwpe_cl5_lic3 -lhwpe_cl5_lic4 -lhwpe_cl5_lic5

