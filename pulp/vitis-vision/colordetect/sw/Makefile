ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# Paths
SRC_PATH 			:= $(ROOT)/src
FUNCTIONS_PATH	 	:= $(ROOT)/common/functions
BENCHMARKS_PATH	 	:= $(ROOT)/common/benchmarks
BUILD_PATH 			:= $(ROOT)/build

dirs 				= $(shell find $(SRC_PATH) -type d) $(shell find $(ROOT)/common -type d)
bins 				= $(shell find $(SRC_PATH)/$(EXE_PATH) -type f -name "*.OMP.ll") $(shell find $(ROOT)/common -type f -name "*.OMP.ll")

# Targets
SRC_TARGET 			?= a_baseline
EXE_PATH 			?= small/a_baseline
EXE_NAME 			?= $(SRC_TARGET)

# Exported vars
include_paths 		?= -I$(SRC_PATH)/$(EXE_PATH) -I$(FUNCTIONS_PATH) -I$(BENCHMARKS_PATH)
experiment_bins 	?= $(bins)

export include_paths experiment_bins

.PHONY: build
build: build_experiment 
	cd $(SRC_PATH)/$(EXE_PATH) && make clean all only=pulp EXE=$(EXE_NAME)

build_experiment:
	cd $(FUNCTIONS_PATH) && make clean all only=pulp
	cd $(BENCHMARKS_PATH) && make clean all only=pulp

clean:
	cd $(FUNCTIONS_PATH) && make $@
	cd $(BENCHMARKS_PATH) && make $@
	cd $(SRC_PATH)/$(EXE_PATH) && make $@

clean_all:
	@$(foreach dir,$(dirs), cd $(dir) && make clean;)