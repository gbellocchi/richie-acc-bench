ROOT 			:= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
DIRECTORIES 	:= 01_sw_baseline 02_sw_double_buffering 03_hw_baseline 04_hw_double_prefetching 05_hw_tcdm_parallelism
	
# Build benchmark application.
build_app:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir)/app && make -s all;)
clean_app:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir)/app && make -s clean;)
	
# Build hls designs.
build_hls:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir)/hls && make -s run_hls;)
clean_hls:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir)/hls && make -s clean;)