ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# Targets
TARGET				?= src/tests/test-l2-pipeline-scl-no-tile-1cl
EXE_TARGET 			:= $(TARGET)
EXE_PATH 			:= $(ROOT)/$(EXE_TARGET)
EXE_NAME 			:= $(shell basename $(EXE_TARGET))

# Paths
SRC_PATH 			:= $(ROOT)/src
FUNCTIONS_PATH	 	:= $(ROOT)/common/functions
BENCHMARKS_PATH	 	:= $(ROOT)/common/benchmarks
BUILD_PATH 			:= $(ROOT)/build

dirs 				= $(shell find $(SRC_PATH) -type d) $(shell find $(ROOT)/common -type d)
bins 				= $(shell find $(EXE_PATH) -type f -name "*.OMP.ll") $(shell find $(ROOT)/common -type f -name "*.OMP.ll")

# Exported vars
include_paths 		?= -I$(EXE_PATH) -I$(FUNCTIONS_PATH) -I$(BENCHMARKS_PATH)
experiment_bins 	?= $(bins)

export include_paths experiment_bins

.PHONY: build
build: build_bench 
	cd $(EXE_PATH) && make clean all only=pulp EXE_TARGET=$(EXE_NAME)

build_bench:
	cd $(FUNCTIONS_PATH) && make clean all only=pulp
	cd $(BENCHMARKS_PATH) && make clean all only=pulp

print_target:
	@echo TARGET=$(TARGET)
	@echo EXE_TARGET=$(EXE_TARGET)
	@echo EXE_PATH=$(EXE_PATH)
	@echo EXE_NAME=$(EXE_NAME)
	
clean:
	cd $(FUNCTIONS_PATH) && make $@
	cd $(BENCHMARKS_PATH) && make $@
	cd $(EXE_PATH) && make $@

clean_all:
	@$(foreach dir,$(dirs), cd $(dir) && make clean;)