
/**
 * Copyright(c) 2017-2-20 Shangwen Wu	
 *
 * 系统大小端处理 
 * 
 */

#ifndef __SYS_ENDIAN_H__
#define __SYS_ENDIAN_H__

#include <mach/endian.h>

#define LITTLE_ENDIAN		1234
#define BIG_ENDIAN			4321

#define __swap16(x)			\
	(u_int16_t)((((u_int16_t)(x) & 0x00ff) << 8) | (((u_int16_t)(x) & 0xff00) >> 8))
#define __swap32(x)			\
	(u_int32_t)((((u_int32_t)(x) & 0x000000ff) << 24) 	\
	| (((u_int32_t)(x) & 0x0000ff00) << 8)				\
	| (((u_int32_t)(x) & 0x00ff0000) >> 8)				\
	| (((u_int32_t)(x) & 0xff000000) >> 24))

#define swap16	__swap16
#define swap32	__swap32

#if BYTE_ORDER == LITTLE_ENDIAN

#define htole16(x)	(x)
#define htole32(x)	(x)
#define letoh16(x)	(x)
#define letoh32(x)	(x)

#define htobe16		swap16
#define htobe32		swap32
#define betoh16		swap16
#define betoh32		swap32

#define cpu_to_le32(x)	(x)
#define cpu_to_be32(x)	swap32(x)

#endif

#if BYTE_ORDER == BIG_ENDIAN

#define htole16		swap16
#define htole32		swap32
#define letoh16		swap16
#define letoh32		swap32

#define htobe16(x)	(x)
#define htobe32(x)	(x)
#define betoh16(x)	(x)
#define betoh32(x)	(x)

#define cpu_to_le32(x)	swap32(x)
#define cpu_to_be32(x)	(x)

#endif

#define htons		htobe16
#define htonl		htobe32
#define ntohs		betoh16
#define ntohl		betoh32

#endif //__SYS_ENDIAN_H__
