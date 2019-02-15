
/**
 * Copyright(c) 2015-12-28 Shangwen Wu	
 *
 * CRC算法相关
 * 
 */

/**
 * CRC32查询表
 */
static unsigned int crc32_table[256];

/**
 * 描述：生成CRC查询表
 * 		 该表为右移CRC表
 */
void mkcrc32tlb(void)
{
	unsigned int i, j, c;
	unsigned int ploy = 0;
	static const int p[] = {0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26};
	
	/* 求CRC32生成多项式，最高位32总为1，故省略 */
	for(i = 0 ; i < sizeof(p)/sizeof(p[0]); ++i) 
		ploy |= 1L << (31 - p[i]);
	
	for(i = 0; i < 256; ++i) {
		c = i;
		for(j = 0; j < 8; ++j) {
			if(c & 0x01) {
				c >>= 1;
				c ^= ploy;
			}
			else
				c >>= 1;
		}
		crc32_table[i] = c;
	}
}

