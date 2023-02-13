# Author: Gianluca Bellocchi <gianluca.bellocchi@unimore.it>

ROOT 			:= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

PROJ_NAME 		:= mmult
HWPE_NAME		:= mmul_opt
IP_NAME			:= mmult_proj

SRC_DIR			:= $(ROOT)/src
TCL_DIR			:= $(ROOT)/tcl
RTL_DIR			:= $(ROOT)/rtl

VIVADO_HLS		:= vivado-2019.1.1 vivado_hls

SYN_DIR			:= $(ROOT)/$(PROJ_NAME)_proj/solution1/syn
IMPL_DIR		:= $(ROOT)/$(PROJ_NAME)_proj/solution1/impl

# -------- #
# RUN_MODE #
# -------- #
# Set to 0: to run setup
# Set to 1: to run setup and synthesis
# Set to 2: to run setup, synthesis and RTL simulation
# Set to 3: to run setup, synthesis, RTL simulation and RTL synthesis
# Any other value will run setup only

RUN_MODE		:= 0

.PHONY: clean
get_rtl:
	@rm -f $(RTL_DIR)/*
	@cp -rf $(SYN_DIR)/verilog/* $(RTL_DIR)

run_hls:
	@rm -rf $(PROJ_NAME)_proj
	@${VIVADO_HLS} -f $(TCL_DIR)/run_hls.tcl $(HLS_INTF) $(RUN_MODE) $(DESIGN_TYPE) $(TARGET_DESIGN)

clean:
	@rm -rf $(PROJ_NAME)_proj
	@rm -f 	*.log *.jou