ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

CSRCS = $(wildcard *.c)

cflags = -v

-include $(ROOT)/../common/default.mk

INCPATHS += -I$(PULP_SDK_HOME)/install/include

# Link PULP libraries
CFLAGS_PULP += -g -include $(PULP_SDK_HOME)/install/hero/hero-urania/cl_config.h
LDFLAGS_PULP += -lomptarget-pulp -lrt
