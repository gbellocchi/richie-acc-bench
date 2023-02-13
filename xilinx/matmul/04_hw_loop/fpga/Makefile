# Author: Gianluca Bellocchi <gianluca.bellocchi@unimore.it>

ROOT 			:= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

PROJ_NAME 		:= xil_04
DESIGN_NAME 	:= matmul

COMMON			:= $(ROOT)/../../../common
TCL_DIR			:= $(COMMON)/tcl/fpga
VIVADO_DIR		:= $(ROOT)/vivado
HLS_IP_DIR		:= $(ROOT)/../hls/$(PROJ_NAME)_proj
HW_DESIGN_DIR	:= $(ROOT)/hw_design

ifeq ($(UNIMORE),)
	VIVADO := vivado
endif
ifeq ($(IIS),)
	VIVADO := vivado-2019.2 vivado
endif

VIVADO_OPT :=-mode batch

.PHONY: all run_fpga clean
all: $(PROJ_NAME)
run_fpga:
	@mkdir -p $(VIVADO_DIR) $(HW_DESIGN_DIR)
	@${VIVADO} ${VIVADO_OPT} \
		-source $(TCL_DIR)/$(DESIGN_NAME)/run_$(PROJ_NAME).tcl \
		-tclargs $(PROJ_NAME) $(VIVADO_DIR) $(HLS_IP_DIR) $(HW_DESIGN_DIR)
clean:
	@rm -rf $(VIVADO_DIR)/*
	@rm -f 	*.log *.jou *.str
clean_hw:
	@rm -f $(HW_DESIGN_DIR)/*