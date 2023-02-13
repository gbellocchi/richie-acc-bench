ROOT 			:= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
DIRECTORIES 	:= 01_uncached 02_cached

.PHONY: clean all
all: build_env build_app
clean: clean_env

# Build benchmark applications.
build_app:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s build_app;)
build_env:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s build_env;)
clean_env:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s clean_local;)