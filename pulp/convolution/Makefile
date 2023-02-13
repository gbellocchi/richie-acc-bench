ROOT 			:= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
DIRECTORIES 	:= 01_hw_baseline
	
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