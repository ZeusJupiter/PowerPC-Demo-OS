#/*
# *   File name: Makefile
# *
# *  Created on: 2016年11月14日, 下午13:36:01
# *      Author: victor (DYD)
# *   Toolchain: 
# *    Language: C/C++
# * description: 
# */

#**********************************************
# POWERPC:powerpc, POWERPC64:powerpc64,
# ARM:arm, X86:x86
#**********************************************
ARCH=POWERPC

#**********************************************
# DEBUG_LEVEL:debug, release, relmsg
#**********************************************
DEBUG_LEVEL = relmsg

#*********************************************
# Toolchain
#**********************************************
TOOLCHAIN_PREFIX=powerpc-eabi-

#**********************************************
# platform: qemu750
#**********************************************
PLATFORM=qemu750

# -msoft-float
FPUFLAGS = -msoft-float

#**********************************************
# Qemu Simulator's Path
#**********************************************
QEMU=/i/sim/qemu