#/*
# *   File name: Makefile
# *
# *  Created on: 2016年11月14日, 下午15:39:07
# *      Author: victor (DYD)
# *   Toolchain: 
# *    Language: C/C++
# * description: 
# */

include $(ROOT_DIR)/config.mk
	
INC_DIR += -I"$(ROOT_DIR)/include" -I"$(ROOT_DIR)/kernel/include" -I"$(ROOT_DIR)/"

CC    := $(TOOLCHAIN_PREFIX)gcc
CXX   := $(TOOLCHAIN_PREFIX)g++
AS    := $(TOOLCHAIN_PREFIX)g++
LD    := $(TOOLCHAIN_PREFIX)g++

DEBUG_FLAGS := -O2 -D_RELEASE_WITH_MSG

DEBUG_MACROS += -DQEMU750=1
CPUFLAGS := -mcpu=750

DEBUG_FLAGS += $(DEBUG_MACROS)

OUTDIR := $(ROOT_DIR)/build/$(DEBUG_LEVEL)

OBJS_DIR := $(OUTDIR)/objs
BIN_DIR  := $(OUTDIR)/bin

BSFLAGS  := $(CPUFLAGS) $(FPUFLAGS) $(DEBUG_FLAGS) -D$(ARCH) -ffreestanding -nostdinc -nostdlib -Wall -Wextra
CFFLAGS  := -fmessage-length=0 -fsigned-char -fno-short-enums
CFLAGS   := $(INC_DIR) $(BSFLAGS) $(CFFLAGS) -std=gnu11
CXXFLAGS := $(INC_DIR) $(BSFLAGS) $(CFFLAGS) -std=gnu++1y -fno-rtti -fno-exceptions 
LDFLAGS  := $(BSFLAGS) -nostartfiles -nodefaultlibs -lgcc 
ASFLAGS  := $(INC_DIR) $(BSFLAGS) -x assembler-with-cpp -DASSEMBLY

CUR_SRCS := ${wildcard *.c}
CUR_SRCS += ${wildcard *.S}
CUR_SRCS += ${wildcard *.cpp} 
CUR_OBJS := ${addsuffix .o, $(basename $(CUR_SRCS))}
RAND:=

%.o: %.S
	@$(AS) $(ASFLAGS) -o $(OBJS_DIR)/$@ -c $<
	
%.o: %.c
	@$(CC) $(CFLAGS) -o $(OBJS_DIR)/$@ -c $<
	
%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -o $(OBJS_DIR)/$@ -c $< 
	
