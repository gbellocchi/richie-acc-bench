

# Copyright (C) 2018 ETH Zurich and University of Bologna
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

MAKEFLAGS=--warn-undefined-variables

CC       := $(CROSS_COMPILE)gcc
AR       := $(CROSS_COMPILE)ar
OBJCDUMP := $(CROSS_COMPILE)objdump

SRC_DIR  := .
INC_DIR  := ../inc
LIB_DIR  := ../lib

SOURCES  := $(SRC_DIR)/hwpe_cl0_lic0.c
CFLAGS   := -Wall -O3 -g -I. -I$(HERO_PULP_INC_DIR)
CFLAGS_SO:= $(CFLAGS) -shared -fPIC

SO       := $(LIB_DIR)/libhwpe_cl0_lic0.so
A        := $(LIB_DIR)/libhwpe_cl0_lic0.a

all: build

build:
	$(MAKE) $(A)
	$(MAKE) $(SO)

$(SO): $(SOURCES)
	mkdir -p $(LIB_DIR)
	$(foreach l,$(SOURCES),$(CC) $(CFLAGS_SO) -I$(INC_DIR) -c $(l);)
	$(CC) $(CFLAGS_SO) -o $@ $(addsuffix .o,$(basename $(notdir $(SOURCES))))
	-rm -rf $(addsuffix .o,$(basename $(notdir $(SOURCES))))

$(A): $(SOURCES)
	mkdir -p $(LIB_DIR)
	$(foreach l,$(SOURCES),$(CC) $(CFLAGS) -I$(INC_DIR) -c $(l);)
	$(AR) rvs -o $@ $(addsuffix .o,$(basename $(notdir $(SOURCES))))
	-rm -rf $(addsuffix .o,$(basename $(notdir $(SOURCES))))

.PHONY: clean install

clean:
	-rm -rf $(LIB_DIR) $(SO) $(A)
	-rm -rf $(addsuffix .o,$(basename $(notdir $(SOURCES))))

install:
ifndef HERO_TARGET_HOST
	$(error HERO_TARGET_HOST is not set)
endif
	ssh -t $(HERO_TARGET_HOST) 'mkdir -p $(HWPE_TARGET_PATH_LIB)'
	scp $(LIB_DIR)/* $(HERO_TARGET_HOST):$(HWPE_TARGET_PATH_LIB)
