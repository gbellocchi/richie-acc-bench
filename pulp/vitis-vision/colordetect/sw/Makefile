ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# Paths
SRC_PATH 			:= $(ROOT)/src
FUNCTIONS_PATH	 	:= $(ROOT)/common/functions
BENCHMARKS_PATH	 	:= $(ROOT)/common/benchmarks
BUILD_PATH 			:= $(ROOT)/build

# Targets
SRC_TARGET 			?= a_baseline
EXE_PATH 			?= small/a_baseline
EXE_NAME 			?= $(SRC_TARGET)

# Exported vars
include_paths 		?= -I$(SRC_PATH)/$(EXE_PATH) -I$(FUNCTIONS_PATH) -I$(BENCHMARKS_PATH)
experiment_bins 	?= 

export include_paths experiment_bins

.PHONY: build
build:
	@rm -rf $(BUILD_PATH)/$(EXE_PATH) && mkdir -p $(BUILD_PATH)/$(EXE_PATH) 
	@cp -rf $(FUNCTIONS_PATH)/* $(BUILD_PATH)/$(EXE_PATH)
	@cp -rf $(BENCHMARKS_PATH)/* $(BUILD_PATH)/$(EXE_PATH)
	@cp -rf $(SRC_PATH)/$(EXE_PATH)/* $(BUILD_PATH)/$(EXE_PATH)
	cd $(BUILD_PATH)/$(EXE_PATH) && make clean all only=pulp EXE=$(EXE_NAME)

clean:
	@rm -rf $(BUILD_PATH)/$(EXE_PATH)

clean_all:
	@rm -rf $(BUILD_PATH)