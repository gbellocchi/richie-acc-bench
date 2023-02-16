ROOT 			= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# Local directories
SRC_DIR 		= $(ROOT)/src
RTL_DIR 		= $(ROOT)/rtl

# Shared directories
COMMON_DIR		= $(ROOT)/../../common
MK_DIR			= $(COMMON_DIR)/mk
TCL_DIR 		= $(COMMON_DIR)/tcl
GENHFILE_DIR	= $(COMMON_DIR)/gen_Hfile

# Project information
PRJ_NAME 		:= fast.prj
SOL_NAME 		:= sol1
ACC_NAME 		:= fast_corner_detect

CPP_TOP_PATH 	:= $(SRC_DIR)/xf_fast_accel.cpp
CPP_TB_PATH 	:= $(SRC_DIR)/xf_fast_tb.cpp

# Synthesis/Implementation
SYN_DIR			= $(ROOT)/$(PRJ_NAME)/$(SOL_NAME)/syn/verilog
IMPL_DIR		= $(ROOT)/$(PRJ_NAME)/$(SOL_NAME)/impl/verilog

MK_PATH 		:= $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR 		:= $(patsubst %/,%,$(dir $(MK_PATH)))
XF_PROJ_ROOT 	?= $(shell bash -c 'export MK_PATH=$(MK_PATH);')

-include $(MK_DIR)/hls.mk