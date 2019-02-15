

/**
 * Copyright(c) 2017-4-21 Shangwen Wu	
 *
 * mbuf空间管理相关头文件，mbuf空间用于网络数据的存储和管理
 * 
 */

#ifndef __SYS_MBUF_H__
#define __SYS_MBUF_H__

struct m_hdr;
struct pkthdr;
struct mbuf;
struct ifnet;

#define MSIZE				128				/* mbuf小块内存区单元大小 */
#define MLEN				(MSIZE - sizeof(struct m_hdr))		/* 一般类型的mbuf数据部分长度 */
#define MHLEN				(MLEN - sizeof(struct pkthdr))		/* pkt类型的mbuf数据部分长度 */

#define MINMCLSIZE			(MLEN + MHLEN + 1)					/* 当需要的空间大于此值时，才分配一个mcluster */


/* mbuf数据基地址以4字节为边界向右对齐len长度 */
#define MH_ALIGN(m, len)	((m)->m_data += (MHLEN - (len)) & ~(sizeof(long) - 1))

/* mbuf类型 */
#define MT_SONAME			1				/* 用于存储socket name（sockaddr）类型的mbuf */
#define MT_DATA				2				/* 用于存储数据 */
#define MT_CONTROL			3				/* 用于存储控制信息 */
#define MT_OOBDATA			4				/* 用于存储OOB数据 */
#define MT_SOOPTS			5				/* 用于存储协议选项数据 */
#define MT_HEADER			6				/* 用于存储协议头数据 */

/* mbuf标识位 */
#define MF_PKTHDR			0x01			/* 表示该mbuf为pkt格式，该数据包为所有数据片段的第一个数据存储区 */
#define MF_EXT				0x02			/* 表示该mbuf为ext外部存储格式 */
#define MF_EOR				0x04			/* 标识该mbuf为最后一个记录节点 */
#define MF_BCAST			0x08			/* 标识广播数据 */
#define MF_MCAST			0x10			/* 标识多播数据 */

/* mbuf内存信息头 */
struct m_hdr {
	struct mbuf *mh_next;					/* 指向mbuf链表的下一个节点 */
	struct mbuf *mh_nextpkt;				/* 指向pkt类型的mbuf链表的下一个节点，用于ifqueue的队列操作 */
	caddr_t mh_dat;							/* 指向数据起始位置，mbuf的flags不同其，数据存储地址也不同 */
	ulong mh_len;							/* 当前mbuf节点的实际数据长度 */
	ushort mh_flags;						/* 记录mbuf的是否有大块数据、是否有pkthdr等信息 */
	ushort mh_type;							/* mbuf类型 */
	//uchar mh_type;							/* mbuf类型 */
};

/** 
 * pkt类型的mbuf头信息，一般一个协议数据包可能分多次传送，
 * 如果mbuf为pkt格式，则表示该数据据内存区为这些数据包的
 * 第一个数据包
 */
struct pkthdr {
	ulong mp_len;							/* 当前以pkt格式mbuf打头的mbuf链的总长度 */
	struct ifnet *mp_recvif;				/* 指定接收接口 */
};

/* 外部存储区（mcluster）信息描述 */
struct m_ext {
	caddr_t me_buf;							/* 指向外部存储取的基地值 */
	ulong me_size;							/* 外部数据大小 */
	void (*me_free)(struct mbuf *);			/* 外部存储区私有的free方法 */
	void (*me_ref)(struct mbuf *);			/* 外部存储区私有的引用计数方法 */
	void *me_handler;						/* 私有处理方法 */
};

/* 小块（128B）的mbuf管理的内存区 */
struct mbuf {
	struct m_hdr M_hdr;	
	union {
		struct {
			struct pkthdr MH_pkthdr;
			union {
				struct m_ext MH_ext;
				char MH_databuf[MHLEN];
			} MH_dat;
		} MH;
		char M_databuf[MLEN];
	} M_dat;
};

/* 用于访问mbuf字段的简便宏定义 */
#define m_next		M_hdr.mh_next
#define m_nextpkt	M_hdr.mh_nextpkt
#define m_data		M_hdr.mh_dat
#define m_len		M_hdr.mh_len
#define m_flags		M_hdr.mh_flags
#define m_type		M_hdr.mh_type

#define m_dat		M_dat.M_databuf
#define m_pkthdr	M_dat.MH.MH_pkthdr
#define m_ext		M_dat.MH.MH_dat.MH_ext
#define m_pktdat	M_dat.MH.MH_dat.MH_databuf


/* 用于大块（2KB）的mbuf管理的内存区 */
union mcluster {
	union mcluster *mcl_next;
	char mcl_data[MCLBYTES];
};

/* 下面必须保证分配的mcluster地址必须为MCLBYTES对齐 */
#define mtocl(m)		(((ulong)(m) - (ulong)(kmembase)) >> MCLSHIFT)					/* mcluster地址转换成mclrefcnt中的索引位置 */
#define cltom(cl)		(struct mcluster *()((ulong)(kmembase) + ((cl) << MCLSHIFT)))	/* mcl索引位置转换成mcluster地址 */
/* 下面必须保证分配的mbuf地址必须为MSIZE对齐 */
#define mtod(m, t)		((t)((m)->m_data))
#define dtom(d)			((struct mbuf *)((d) & ~(MSIZE - 1)))

#define MBUFLOCK(code) 	\
do {					\
	int s = splimp();	\
	{code}				\
	splx(s);			\
} while (0)

/* 分配一个标准格式的mbuf内存区，并初始化部分字段 */
#define MGET(m, type)							\
do {											\
	(m) = (struct mbuf *)kmem_malloc(sizeof(struct mbuf));\
	if(m) {										\
		MBUFLOCK(++mbstats.ms_mtypes[(type)];);	\
		(m)->m_type = (type);					\
		(m)->m_data = (m)->m_dat;				\
		(m)->m_next = NULL;						\
		(m)->m_nextpkt = NULL;					\
		(m)->m_flags = 0;						\
	} else {									\
		(m) = mbuf_retry(type);					\
	}											\
} while (0)

/* 分配一个pkt格式的mbuf内存区，并初始化部分字段 */
#define MGETPKT(m, type)						\
do {											\
	(m) = (struct mbuf *)kmem_malloc(sizeof(struct mbuf));\
	if(m) {										\
		MBUFLOCK(++mbstats.ms_mtypes[(type)];);	\
		(m)->m_type = (type);					\
		(m)->m_data = (m)->m_pktdat;			\
		(m)->m_next = NULL;						\
		(m)->m_nextpkt = NULL;					\
		(m)->m_flags = MF_PKTHDR;				\
	} else {									\
		(m) = mbuf_retrypkt(type);				\
	}											\
} while (0)

/* 释放一个mbuf，返回当前释放mbuf节点的下一个节点 */
#define MFREE(m, n)								\
do {											\
	MBUFLOCK(--mbstats.ms_mtypes[(m)->m_type];);\
	if((m)->m_flags & MF_EXT) {					\
		if((m)->m_ext.me_free)					\
			(*((m)->m_ext.me_free))(m);			\
		else									\
			MCLFREE((m)->m_ext.me_buf);			\
	}											\
	(n) = (m)->m_next;							\
	kmem_free(m);								\
} while(0)

/* 取出一个大小为MCLBYTES的大块mbuf内存区，并初始化部分字段，若获取成功flags的MF_EXT将被置位 */
#define MCLGET(m)								\
do {											\
	MCLALLOC((m)->m_ext.me_buf);				\
	if((m)->m_ext.me_buf != NULL) {				\
		(m)->m_flags |= MF_EXT;					\
		(m)->m_data = (m)->m_ext.me_buf;		\
		(m)->m_ext.me_size = MCLBYTES;			\
		(m)->m_ext.me_free = NULL;				\
		(m)->m_ext.me_ref = NULL;				\
		(m)->m_ext.me_handler = NULL;			\
	} else 										\
		printf("mcluster failed!\n");			\
} while(0)

/* 分配一个大快mbuf内存，当mclfree链表为空时，将调用mbuf_clalloc新分配一块存储区作为mbuf使用 */
#define MCLALLOC(p)								\
MBUFLOCK (										\
	if(NULL == mclfree) 						\
		mbuf_clalloc(1);						\
	if(mclfree != NULL) {						\
		++mclrefcnt[mtocl(mclfree)];			\
		--mbstats.ms_freemcl;					\
		(p) = (caddr_t)mclfree;					\
		mclfree = mclfree->mcl_next;			\
	}											\
)

/* 释放一个大块mbuf内存 */
#define MCLFREE(p)								\
MBUFLOCK (										\
	if(!--mclrefcnt[mtocl(p)]) {				\
		++mbstats.ms_freemcl;					\
		((union mcluster *)(p))->mcl_next = mclfree;\
		mclfree = (union mcluster *)(p);		\
	}											\
)

#define M_CPOYFLAGS	(MF_PKTHDR | MF_EOR) 

/* 拷贝pkt格式mbuf */
#define MBUF_COPY_PKTHDR(d, s)					\
do {											\
	(d)->m_pkthdr.mp_len = (s)->m_pkthdr.mp_len;\
	(d)->m_flags = (s)->m_flags & M_CPOYFLAGS;	\
	(d)->m_data = (d)->m_pktdat;				\
} while (0)

/* 计算当前数据起始位置之前可用空间大小 */
#define MBUF_LEADINGSPACE(m)	((m)->m_flags & MF_EXT ? 0 : \
									((m)->m_flags & MF_PKTHDR ?	\
										(m)->m_data - (m)->m_pktdat : \
										(m)->m_data - (m)->m_dat))		

struct mbufstats {
	ulong ms_nmcl;									/* 当前系统从kmem中分配出来的mcluster总个数 */
	ulong ms_freemcl;								/* 当前可用mcluster个数 */
	ulong ms_mtypes[256];							/* 当前各种类型的mbuf个数，目前最大支持256种类型 */
};

#define M_COPYALL	10000000

/* defined in mbuf.c */
extern struct mbuf *mbuf_reserve(struct mbuf *m, ulong len);
extern int mbuf_getext(struct mbuf *m);
extern struct mbuf *mbuf_retry(int type);
extern struct mbuf *mbuf_retrypkt(int type);
extern struct mbuf *mbuf_get(int type);
extern struct mbuf *mbuf_getpkt(int type);
extern struct mbuf *mbuf_free(struct mbuf *m);
extern void mbuf_freem(struct mbuf *m);
extern int mbuf_clalloc(int ncl);		
extern int mbufinit(void);
extern void mbuf_reclaim(void);
extern struct mbuf *mbuf_copy(struct mbuf *m, ulong off0, ulong len);
extern struct mbuf *mbuf_pullup(struct mbuf *n, ulong len);
extern struct mbuf *mbuf_pullup_big(struct mbuf *n, ulong len);
extern void mbuf_trim(struct mbuf *m, ulong trimlen, int tail);
extern ulong mbuf_copydata(struct mbuf *m, ulong off, ulong len, caddr_t buf);

#endif //__SYS_MBUF_H__

