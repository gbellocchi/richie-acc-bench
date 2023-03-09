ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# Paths
SRC_PATH 			:= $(ROOT)/src
FUNCTIONS_PATH	 	:= $(ROOT)/common/functions
BENCHMARKS_PATH	 	:= $(ROOT)/common/benchmarks
BUILD_PATH 			:= $(ROOT)/build

dirs 				= $(shell find $(SRC_PATH) -type d) $(shell find $(ROOT)/common -type d)
bins 				= $(shell find $(SRC_PATH) -type f -name "*.OMP.ll") $(shell find $(ROOT)/common -type f -name "*.OMP.ll")

# Targets
SRC_TARGET 			?= a_baseline
EXE_PATH 			?= small/a_baseline
EXE_NAME 			?= $(SRC_TARGET)

# Exported vars
include_paths 		?= -I$(SRC_PATH)/$(EXE_PATH) -I$(FUNCTIONS_PATH) -I$(BENCHMARKS_PATH)
experiment_bins 	?= $(bins)

export include_paths experiment_bins

.PHONY: build
# build:
# 	@rm -rf $(BUILD_PATH)/$(EXE_PATH) && mkdir -p $(BUILD_PATH)/$(EXE_PATH) 
# 	@cp -rf $(FUNCTIONS_PATH)/* $(BUILD_PATH)/$(EXE_PATH)
# 	@cp -rf $(BENCHMARKS_PATH)/* $(BUILD_PATH)/$(EXE_PATH)
# 	@cp -rf $(SRC_PATH)/$(EXE_PATH)/* $(BUILD_PATH)/$(EXE_PATH)
# 	cd $(BUILD_PATH)/$(EXE_PATH) && make clean all only=pulp EXE=$(EXE_NAME)

# clean:
# 	@rm -rf $(BUILD_PATH)/$(EXE_PATH)

# clean_all:
# 	@rm -rf $(BUILD_PATH)

build: build_experiment 
	cd $(SRC_PATH)/$(EXE_PATH) && make clean all only=pulp EXE=$(EXE_NAME) BENCHMARK=$(BUILD_PATH)/$(EXE_PATH)

build_experiment:
	rm -rf $(BUILD_PATH)/$(EXE_PATH) && mkdir -p $(BUILD_PATH)/$(EXE_PATH)
	cd $(FUNCTIONS_PATH) && make clean all only=pulp
	cd $(BENCHMARKS_PATH) && make clean all only=pulp

clean:
	cd $(FUNCTIONS_PATH) && make $@
	cd $(BENCHMARKS_PATH) && make $@
	cd $(SRC_PATH)/$(EXE_PATH) && make $@

clean_all:
	@$(foreach dir,$(dirs), cd $(dir) && make clean;)