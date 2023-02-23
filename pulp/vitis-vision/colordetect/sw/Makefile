ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# Paths
SRC_PATH 			:= $(ROOT)/src
EXPERIMENT_PATH	 	:= $(ROOT)/functions
BUILD_PATH 			:= $(ROOT)/build

dirs 				= $(shell find $(SRC_PATH) -type d) $(EXPERIMENT_PATH)

# Targets
SRC_TARGET 			?= a_baseline
EXE_PATH 			?= small/a_baseline
EXE_NAME 			?= $(SRC_TARGET)

# Exported vars
include_paths 		?= -I$(SRC_PATH)/$(EXE_PATH) -I$(EXPERIMENT_PATH)
experiment_bins 	?= 

export include_paths experiment_bins

.PHONY: build
build:
	rm -rf $(BUILD_PATH)/$(SRC_TARGET) && mkdir -p $(BUILD_PATH)/$(SRC_TARGET) 
	cp -rf $(EXPERIMENT_PATH)/* $(BUILD_PATH)/$(SRC_TARGET)
	cp -rf $(SRC_PATH)/$(EXE_PATH)/* $(BUILD_PATH)/$(SRC_TARGET)
	cd $(BUILD_PATH)/$(SRC_TARGET) && make clean all only=pulp EXE=$(EXE_NAME)

clean:
	@cd $(EXPERIMENT_PATH) && make $@
	@cd $(SRC_PATH)/$(EXE_PATH) && make $@
	@rm -rf $(BUILD_PATH)/$(SRC_TARGET)

clean_all:
	@$(foreach dir,$(dirs), cd $(dir) && make clean;)
	@rm -rf $(BUILD_PATH)