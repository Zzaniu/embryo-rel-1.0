
/**
 * Copyright(c) 2017-2-21 Shangwen Wu	
 *
 * 系统大小端处理 
 * 
 */

#ifndef __MACH_ENDIAN_H_
#define __MACH_ENDIAN_H_

#ifdef __MIPSEL__
	#define BYTE_ORDER 1234
#else
#ifdef __MIPSEB__
	#define BYTE_ORDER 4321
#else
	#error	__MIPSEB__ or __MIPSEL__ not defined!
#endif
#endif

#endif //__MACH_ENDIAN_H_

