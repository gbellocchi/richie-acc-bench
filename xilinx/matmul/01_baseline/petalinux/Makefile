ROOT 			:= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

PROJ_NAME 		:= xil_01
DESIGN_NAME 	:= matmul
BOARD_MODEL		:= zcu102

HW_DESIGN_DIR	:= $(ROOT)/../fpga/hw_design
COMMON			:= $(ROOT)/../../../common
BOARD_DIR		:= $(COMMON)/board
XSDB_DIR		:= $(BOARD_DIR)/xsdb

boot:
	@$(XSDB_DIR)/boot_jtag.sh $(ROOT) $(BOARD_DIR) $(HW_DESIGN_DIR) $(PROJ_NAME) $(DESIGN_NAME) $(BOARD_MODEL)

install:
	@$(XSDB_DIR)/install_tftp_nfs_rootfs.sh $(ROOT) $(BOARD_DIR)

update_output:
	@rm -rf $(ROOT)/output/*
	@cp -r $(ROOT)/$(BOARD_MODEL)/images/linux/* $(ROOT)/output

run_petalinux:
	@$(BOARD_DIR)/$(BOARD_MODEL).sh $(ROOT) $(HW_DESIGN_DIR) $(PROJ_NAME) $(DESIGN_NAME) $(BOARD_DIR)

clean_petalinux:
	@rm -rf $(BOARD_MODEL)

clean_output:
	@rm -rf $(ROOT)/output/*

reset_board:
	@$(XSDB_DIR)/reset_jtag.sh $(ROOT) $(BOARD_DIR)