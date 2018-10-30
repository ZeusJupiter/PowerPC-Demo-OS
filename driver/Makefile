#/*
# *   File name: Makefile
# *
# *  Created on: 2016年11月14日, 下午13:42:28
# *      Author: victor 
# *   Toolchain: 
# *    Language: C/C++
# * description: 
# */

.PHONY: all ECHO

include $(ROOT_DIR)/rules.mk

SUBDIRS = pic rtc timer uart

all: $(SUBDIRS) $(CUR_OBJS)

$(SUBDIRS): ECHO
	$(VERBOSE)make -C $@
	
ECHO:
	$(VERBOSE)echo 'current directories and files:' $(SUBDIRS) $(CUR_SRCS)