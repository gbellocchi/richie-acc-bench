# ------------------------------------------------------------------- #
# 	Author:      Gianluca Bellocchi <gianluca.bellocchi@unimore.it>   #
# ------------------------------------------------------------------- #

ROOT := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

CSRCS = main.c

ifeq ($(HERO_OV_OPENMP_TESTS),)
$(error HERO_OV_OPENMP_TESTS is not set)
else
-include $(HERO_OV_OPENMP_TESTS)/common/default.mk
endif 