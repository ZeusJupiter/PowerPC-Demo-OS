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

INC_DIR := -I"$(ROOT_DIR)/kernel/include" -I"$(ROOT_DIR)/driver"
include $(ROOT_DIR)/rules.mk

all: ECHO $(CUR_OBJS)

ECHO:
	$(VERBOSE)echo 'current directories and files:' $(CUR_OBJS)
	$(VERBOSE)cp ldscript/layout $(OBJS_DIR)/BHOS.ld
	