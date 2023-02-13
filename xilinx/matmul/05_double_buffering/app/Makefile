ROOT 			:= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

PROJ_NAME 		:= xil_05
IP_NAME 		:= mmult_hw

SRC_DIR			:= $(ROOT)/src
INC_DIR			:= $(ROOT)/include
BUILD_DIR		:= $(ROOT)/build

BOARD_ROOT		:= /storage/srv/rootfs/xil_exp/home/root

PETALINUX_DIR	:= $(ROOT)/../petalinux
DRIVERS_DIR		:= $(PETALINUX_DIR)/zcu102/components/plnx_workspace/device-tree/device-tree/drivers
COMMON			:= $(ROOT)/../../../common
BOARD_DIR		:= $(COMMON)/common/board
APP_UTILS_DIR	:= $(COMMON)/common/app_utils
XSDB_DIR		:= $(BOARD_DIR)/xsdb

app_deploy: clean_board
	@sudo cp $(BUILD_DIR)/mmult_exec $(BOARD_ROOT)

build_app: 
	@cd $(BUILD_DIR) && make -s clean all

build_env: get_drivers
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(APP_UTILS_DIR) -DCMAKE_APP_ROOT:PATH=$(ROOT) -DCMAKE_APP_UTILS:PATH=$(APP_UTILS_DIR)

get_drivers: clean_local
	@mkdir -p $(INC_DIR)
	@cp -rf $(DRIVERS_DIR)/* hw_description
	@cp hw_description/src/*.h $(INC_DIR)
	@cp hw_description/src/*.c $(SRC_DIR)
	@sed -i 's/typedef uint32_t u32;/typedef uint64_t u32;/' $(INC_DIR)/xmmult_hw.h
	@rm -rf hw_description

clean_local: clean_build clean_drivers

clean_build:
	@rm -rf $(BUILD_DIR)/*

clean_drivers:
	@rm -rf $(INC_DIR)/*
	@rm -rf $(SRC_DIR)/*_hw*.c

clean_board:
	@sudo rm -rf $(BOARD_ROOT)/mmult_exec