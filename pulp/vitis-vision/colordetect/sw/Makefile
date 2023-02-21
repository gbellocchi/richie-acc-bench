ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# Paths
SRC_PATH 			:= $(ROOT)/src
EXPERIMENT_PATH	 	:= $(ROOT)/functions

dirs 				= $(shell find $(SRC_PATH) -type d) $(EXPERIMENT_PATH)

# Targets
SRC_TARGET 			?= 1cl_small
EXE_PATH 			?= $(SRC_TARGET)
EXE_NAME 			?= $(SRC_TARGET)

# Exported vars
include_paths 		?= -I$(SRC_PATH)/$(EXE_PATH) -I$(EXPERIMENT_PATH)
experiment_bins 	?= $(EXPERIMENT_PATH)/experiment.OMP.ll \
					   $(EXPERIMENT_PATH)/cluster_synch.OMP.ll \
					   $(EXPERIMENT_PATH)/icache_stats.OMP.ll \
					   $(EXPERIMENT_PATH)/profiling.OMP.ll 

export include_paths experiment_bins

build: build_experiment
	cd $(SRC_PATH)/$(EXE_PATH) && make clean all only=pulp EXE=$(EXE_NAME)

build_experiment:
	cd $(EXPERIMENT_PATH) && make clean all only=pulp

clean:
	@cd $(EXPERIMENT_PATH) && make $@
	@cd $(SRC_PATH)/$(EXE_PATH) && make $@

clean_all:
	@$(foreach dir,$(dirs), cd $(dir) && make clean;)