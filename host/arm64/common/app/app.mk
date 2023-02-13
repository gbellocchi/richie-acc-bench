SRC_DIR			:= $(ROOT)/src
BUILD_DIR		:= $(ROOT)/build

build_app: 
	@cd $(BUILD_DIR) && make -s clean all

build_env:
	@mkdir -p build
	@cd build && cmake $(ROOT) \
		-DCMAKE_APP_NAME:PATH=$(APP_NAME) \
		-DCMAKE_APP_ROOT:PATH=$(ROOT) \
		-DCMAKE_APP_UTILS:PATH=$(APP_UTILS_DIR)

clean_local: clean_build

clean_build:
	@rm -rf $(BUILD_DIR)