ROOT 			:= $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
DIRECTORIES 	:= dilate erode rgb2hsv threshold

.PHONY: clean all

get_rtl:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s get_rtl;)
all:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s run;)
clean:
	@$(foreach dir,$(DIRECTORIES), cd $(ROOT)/$(dir) && make -s clean;)