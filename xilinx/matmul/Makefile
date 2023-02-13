ROOT 			:= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
DIRECTORIES 	:= 01_baseline 02_blocking 03_partial_array_partition 04_hw_loop 05_double_buffering 06_array_partition
	
# Build benchmark application.
build_app:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s build_app;)
build_env:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s build_env;)
clean_env:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s clean_env;)
	
# Build hls designs.
build_hls:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s build_hls;)
clean_hls:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s clean_hls;)

# Build fpga designs.
build_fpga:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s build_fpga;)
clean_fpga:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s clean_fpga;)

# Build petalinux projects.
build_petalinux:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s build_petalinux;)
update_output:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s update_output;)
clean_petalinux:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s clean_petalinux;)

	
