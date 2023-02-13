# ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# .PHONY: build clean

# all: env build

# run:
# 	@cd build && ./aes_128_sw

# build: 
# 	@cd build && make -s clean all

# env:
# 	@mkdir -p build && cd build && cmake ./.. -DCMAKE_APP_ROOT:PATH=$(ROOT) -DCMAKE_APP_UTILS:PATH=$(ROOT)/src/inc

# clean:
# 	@rm -rf build

ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

############################ Sources ############################

APP_NAME := aes_128_arm

-include ${APP_UTILS_DIR}/app.mk
-include ${BOARD_UTILS_DIR}/board.mk

INC_DIR	:= $(SRC_DIR)/inc
