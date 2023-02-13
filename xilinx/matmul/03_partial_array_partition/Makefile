ROOT 			:= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# Boot board.
boot_zcu102: 
	@cd petalinux && make -s update_output install boot;

# Build benchmark application.
build_app:
	@cd app && make -s build_app;
build_env:
	@cd app && make -s build_env;
clean_env:
	@cd app && make -s clean_local;
	
# Build hls designs.
build_hls:
	@cd hls && make -s run_hls;
clean_hls:
	@cd hls && make -s clean;

# Build fpga designs.
build_fpga:
	@cd fpga && make -s run_fpga;
clean_fpga:
	@cd fpga && make -s clean;

# Build petalinux projects.
build_petalinux:
	@cd petalinux && make -s run_petalinux;
update_output:
	@cd petalinux && make -s update_output;
clean_petalinux:
	@cd petalinux && make -s clean_petalinux clean_output;
	
