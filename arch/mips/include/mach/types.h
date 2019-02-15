
/**
 * Copyright(c) 2015-9-30 Shangwen Wu	
 *
 * 板级数据宽度相关头文件
 * 
 */

#ifndef __MACH_TYPES_H__
#define __MACH_TYPES_H__

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef unsigned long vm_offset_t;
typedef unsigned long vm_size_t;

typedef unsigned long dma_addr_t;

/** 
 * 注意：下面对寄存器的位宽进行定义时都采用有符号型，因为 
 * 		 某些架构在低位宽寄存器向高位宽寄存器进行扩展时，
 * 		 会自动对最高位进行符号扩展，为了与架构这一特性保
 * 		 持一致，要求定义的C变量在进行转换时也需要进行符号
 * 		 扩展
 */
#if __mips >= 3
#define HAVE_QUAD
typedef int64_t register_t;
typedef int64_t f_register_t;
typedef int64_t quad_t;
typedef uint64_t u_quad_t;
#else 
typedef int32_t register_t;
typedef int32_t f_register_t;
#endif

#endif //__MACH_TYPES_H__

