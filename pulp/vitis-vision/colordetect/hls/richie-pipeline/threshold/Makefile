# =====================================================================
# Copyright (C) 2023 University of Modena and Reggio Emilia.
#
# Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
# =====================================================================

ROOT 			= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

MK_PATH 		:= $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR 		:= $(patsubst %/,%,$(dir $(MK_PATH)))
XF_PROJ_ROOT 	?= $(shell bash -c 'export MK_PATH=$(MK_PATH);')

# Project
PRJ_ROOT		= $(CUR_DIR)
PRJ_NAME 		:= colordetect.prj
SOL_NAME 		:= sol1
ACC_NAME 		:= threshold_cv

# Vitis commands
CSIM			?= 0
CSYNTH			?= 1
COSIM			?= 0
COSIM_GUI		?= 0

# Directories
SRC_DIR 		= $(PRJ_ROOT)/src
RTL_DIR 		= $(PRJ_ROOT)/rtl
COMMON_DIR		= $(PRJ_ROOT)/../../../../common
MK_DIR 			= $(COMMON_DIR)/mk
TCL_DIR 		= $(COMMON_DIR)/tcl
GENHFILE_DIR	= $(COMMON_DIR)/gen_Hfile
VITIS_PATCH_DIR	= $(COMMON_DIR)/Vitis_Libraries_patch/vision_L1_patch

# Source files
CPP_TOP_PATH 	:= $(SRC_DIR)/xf_colordetect_accel.cpp
CPP_TB_PATH 	:= $(SRC_DIR)/xf_colordetect_tb.cpp

# Synthesis/Implementation
SYN_DIR			= $(PRJ_ROOT)/$(PRJ_NAME)/$(SOL_NAME)/syn/verilog
IMPL_DIR		= $(PRJ_ROOT)/$(PRJ_NAME)/$(SOL_NAME)/impl/verilog

-include $(MK_DIR)/hls.mk