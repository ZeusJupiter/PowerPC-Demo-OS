#/*
# *   File name: Makefile
# *
# *  Created on: 2016年11月14日, 下午15:45:31
# *      Author: victor 
# *   Toolchain: 
# *    Language: C/C++
# * description: 
# */

.PHONY: all
.PHONY: ECHO

INC_DIR := -I"$(ROOT_DIR)/driver"
include $(ROOT_DIR)/rules.mk

SUBDIRS = libc 

all: $(SUBDIRS) $(CUR_OBJS)

$(SUBDIRS): ECHO
	$(VERBOSE)make -C $@

ECHO:
	$(VERBOSE)echo 'current directories and files:' $(CUR_OBJS)
	