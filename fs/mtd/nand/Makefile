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

INC_DIR := -I"$(ROOT_DIR)/kernel/include"
include $(ROOT_DIR)/rules.mk

#%.o: %.cpp
##	$(VERBOSE)$(CXX) $(CXXFLAGS) -E $< -o $(OBJS_DIR)/$(addsuffix .i, $(basename $@))
##	$(VERBOSE)$(CXX) $(CXXFLAGS) -o $(OBJS_DIR)/$@.$$RANDOM -c $(OBJS_DIR)/$(basename $@).i
#	$(VERBOSE)$(CXX) $(CXXFLAGS) -S $< -o $(OBJS_DIR)/$(addsuffix .s, $(basename $@))
#	$(VERBOSE)$(CXX) $(CXXFLAGS) -o $(OBJS_DIR)/$@.$$RANDOM -c $(OBJS_DIR)/$(basename $@).s
##	$(VERBOSE)$(CXX) $(CXXFLAGS) -o $(OBJS_DIR)/$@.$$RANDOM -c $< 

all: ECHO $(CUR_OBJS)

ECHO:
	$(VERBOSE)echo 'current directories and files:' $(CUR_OBJS)