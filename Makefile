#/*
# *   File name: Makefile
# *
# *  Created on: 2016年11月14日, 下午13:38:03
# *      Author: victor (DYD)
# *   Toolchain: 
# *    Language: C/C++
# * description: 
# */

.PHONY: all 
.PHONY: ECHO 
.PHONY: clean
.PHONY: run

TARGET = Kernel

ROOT_DIR=$(shell pwd)
export ROOT_DIR

include $(ROOT_DIR)/rules.mk
SUBDIRS = driver elf fs generic kernel

all: $(SUBDIRS) $(CUR_OBJS)
	@$(LD) -T$(OBJS_DIR)/BHOS.ld  $(LDFLAGS)  -o $(BIN_DIR)/$(TARGET).elf $(OBJS_DIR)/*.o
	
$(SUBDIRS): ECHO
	@make -C $@

ECHO:
	@if [ ! -d $(OBJS_DIR) ] ; then  mkdir -p $(OBJS_DIR); fi
	@if [ ! -d $(BIN_DIR) ] ; then mkdir -p $(BIN_DIR); else  rm -rf $(BIN_DIR)/*.elf;  fi
	@if [ ! -f $(BIN_DIR)/$(TARGET).elf.boot ]; then cp $(ROOT_DIR)/$(TARGET).elf.boot $(BIN_DIR)/$(TARGET).elf.boot ; fi
	@echo 'current directories and files:' $(SUBDIRS) $(CUR_SRCS)

clean:
	@rm -rf $(ROOT_DIR)/build/$(DEBUG_LEVEL)/objs
	@rm -rf $(BIN_DIR)/*.elf $(BIN_DIR)/*.bin $(BIN_DIR)/*.dump $(BIN_DIR)/*.sz

run:
	$(QEMU)/qemu-system-ppc.exe -cpu 750 -m 256 -M ppc_sylixos \
	-serial telnet:127.0.0.1:1200,server \
	-kernel $(BIN_DIR)/$(TARGET).elf &
	$(QEMU)/../putty.exe telnet://127.0.0.1:1200/

		