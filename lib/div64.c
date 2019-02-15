
/**
 * Copyright(c) 2016-11-21 linux source code, Shangwen Wu
 *
 * 软件实现64位除法和模运算
 * 
 */
#include <mach/types.h>

uint32_t __div64_u32(uint64_t *n, uint32_t base)
{
	uint64_t rem = *n;
	uint64_t res = 0, d = 1;
	uint64_t b = base;
	uint32_t high = *n >> 32;
	
	if(high >= base) {					//先计算高32位除法的结果
		high /= base;
		res = (uint64_t)(high) << 32;
		rem -= (uint64_t)(high * base) << 32;
	}

	while((int64_t)(b) > 0 && b < rem) {
		b <<= 1;
		d <<= 1;
	}
	
	do {
		if(rem >= b) {
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while(d);

	*n = res;
	return rem;
}

