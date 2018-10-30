/*
 *   File name: gnu_asm.h
 *
 *  Created on: Nov 10, 2016, 12:08:59 AM
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 */

#ifndef __INCLUDE_TOOLCHAIN_GNU_ASM_H__
#define __INCLUDE_TOOLCHAIN_GNU_ASM_H__

#define EXPORT_FUNC(func)  .global func; .type func, @function
#define EXPORT_DATA(val)   .global  val; .type var,  @object

#define IMPORT_FUNC(func)  .extern func;
#define IMPORT_DATA(val)   .extern val;

#define LABEL(name)  name:

#define BEG_FUNC(func) \
func:

#define END_FUNC(name) .size  name, . - name;

#define BEG_SEC(secName, alignNum) \
	.section secName; \
	.balign  alignNum;

#define END_SEC(secName) \

#define BEG_MACRO(mfun...)	\
		.macro mfun

#define END_MACRO(mfun...)	.endm

#define WEAK(symbol) .weak  symbol;

#endif

