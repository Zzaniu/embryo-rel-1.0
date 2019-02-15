
/**
 * Copyright(c) 2018-3-7 FreeBSD Shangwen Wu	
 *
 * 网络数据包校验和算法
 * 基本思想为两个字节的累加
 * 为什么该算法不用考虑机器大小端问题？？？？？ 
 */
#include <common.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>

#include <netinet/in.h>

#define ADDCARRY(x) ((x) > 0xffff ? (x) -= 0xffff : (x))			//算法思想：x = x - 0x10000（去掉进位） + 1（加上进位）
#define REDUCE do { \
	l_util.l = sum; \
 	sum = l_util.s[0] + l_util.s[1]; \
	ADDCARRY(sum); \
}while(0)

/**
 * 校验和算法：该函数被freebsd组织做了相当的优化，以至于很简单的累加算法变得
 * 很复杂，其中主要优化有：整个算法中对于2字节的访存运算，地址都做了2字节对
 * 齐；累加采用了32字节和8字节的循环体，主要是为了提高cache利用率；对进位的加
 * 法很高效
 */
int in_cksum(struct mbuf *m, int len)
{
	int sum = 0, mlen = 0;
	uint16_t *w;//指向当前累加位置
	int byte_swapped = 0;//标记是否进行了强制地址对齐，以便进行累加时进行移位调整
		
	union {
		uint8_t c[2];
		uint16_t s;
	} s_util;
	union {
		uint16_t s[2];
		uint32_t l;
	} l_util;

	for(; m && len; m = m->m_next) {
		if(0 == m->m_len) 
			continue;
		w = mtod(m, uint16_t *);
		/* 上一个mbuf中还有一个字节没有算入sum，该字节保存在s_util.c[0] */
		if(-1 == mlen) {
			s_util.c[1] = *((uint8_t *)w);
			sum += s_util.s;
			w = (uint16_t *)((uint8_t *)w + 1);
			mlen = m->m_len - 1;
			--len;
		} else 
			mlen = m->m_len;
		if(mlen > len)
			mlen = len;
		len -= mlen;

		/* 强制w指针为2字节对齐 */
		if(((ulong)w & 0x1) && mlen) {
			REDUCE;			//后面要对sum移位，这里缩短sum防止sum进位
			sum <<= 8;		//？？？
			s_util.c[0] = *((uint8_t *)w); //待累加
			w = (uint16_t *)((uint8_t *)w + 1);
			--mlen;
			byte_swapped = 1;
		}

		/* 下面的设计主要增加cache命中率 */
		while((mlen -= 32) >= 0) {
			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
			sum += w[4]; sum += w[5]; sum += w[6]; sum += w[7];
			sum += w[8]; sum += w[9]; sum += w[10]; sum += w[11];
			sum += w[12]; sum += w[13]; sum += w[14]; sum += w[15];
			w += 16;
		}
		mlen += 32;	//上面的while循环造成mlen多减了
		while((mlen -= 8) >= 0) {
			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
			w += 4;
		}
		mlen += 8;	//上面的while循环造成mlen多减了
		if(0 == mlen && !byte_swapped)
			continue;
		REDUCE;
		while((mlen -= 2) >= 0) {
			sum += w[0];
			w += 1;
		}
		/** 
 		 * m的可能值为-2、-1，其中-2表示当前mbuf已经没有数据需要累加了，
 		 * -1表示当前mbuf还有一个数据遗留在w位置
 		 */
		if(byte_swapped) {
			REDUCE;			//后面要对sum移位，这里缩短sum防止sum进位
			sum <<= 8;		//？？？
			byte_swapped = 0;
			if(-1 == mlen) {
				s_util.c[1] = *((uint8_t *)w); 
				sum += s_util.s;
				mlen = 0;	//防止再一次累加
			} else
				mlen = -1;//遗留一个字节等待下一次累加，数据已经保存在s_util.c[0]中
		} else if(-1 == mlen)
			s_util.c[0] = *((uint8_t *)w); //待累加
	}
	/* 处理遗留的字节 */
	if(-1 == mlen) {
		s_util.c[1] = 0;
		sum += s_util.s;
	}

	REDUCE;	//处理进位

	return ~sum & 0xffff;
}
