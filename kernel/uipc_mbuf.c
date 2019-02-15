
/**
 * Copyright(c) 2017-4-21 Shangwen Wu 
 *
 * 网络协议栈使用的内存管理
 * 
 */

#include <common.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/system.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <mach/intr.h>
#include <sys/syslog.h>
#include <sys/mbuf.h>

extern ulong max_protohdr;							//defined in uipc_domain.c

static long *mclrefcnt = NULL;
static union mcluster *mclfree = NULL;
static struct mbufstats mbstats = {0};

/**
 * 描述：拷贝mbuf中的数据到指定的内存区域
 * 参数：off，拷贝相对于mbuf链表头节点databuf的起始字节偏移位置；len，拷贝长度，当len
 * 		 为M_COPYALL时，将拷贝从off到mbuf链最后一个数据字节的长度
 * 返回：返回成功拷贝的字节数
 */
ulong mbuf_copydata(struct mbuf *m, ulong off, ulong len, caddr_t buf)
{
	ulong copyed = 0, cnt;

	/* 找到指定mbuf的偏移位置 */
	while(m) {
		if(off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	
	/* 拷贝数据 */
	while(len && m) {
		cnt = min(m->m_len, len);
		bcopy(mtod(m, caddr_t) + off, buf + copyed, cnt);
		copyed += cnt;
		len -= cnt;
		m = m->m_next;
		off = 0;
	}

	return copyed;
}

/**
 * 描述：截取掉一个mbuf链中指定长度的数据
 * 参数：trimlen，截取数据长度，tail，为1时为从尾部截取数据，反之从头部截取数据
 * 注意：该函数仅调整有效数据部分的指针以及mbuf有效数据长度，而并不释放mbuf节点
 */
void mbuf_trim(struct mbuf *m, ulong trimlen, int tail)
{
	ulong len = trimlen, count = 0;
	struct mbuf *mp;

	if(!(mp = m))
		return;

	/* 截取头部 */
	if(!tail) {
		while(mp && len) {	
			if(mp->m_len >= len) {
				mp->m_len -= len;
				mp->m_data += len;	//注意数据截取方向
				len = 0;
			} else {
				len -= mp->m_len;
				mp->m_len = 0;
				mp = mp->m_next;
			}
		}
		if(m->m_flags & MF_PKTHDR) {
			if(!mp && len)
				m->m_pkthdr.mp_len = 0; 
			else
				m->m_pkthdr.mp_len -= trimlen; 
		}
	} else {						/* 截取尾部 */
		do { 						/* 计算整个链表长度 */
			count += mp->m_len;
		} while((mp = mp->m_next) != NULL);
		count -= min(count, len);
		if(m->m_flags & MF_PKTHDR)
			m->m_pkthdr.mp_len = count;
		mp = m;
		do {
			if(mp->m_len >= count) {
				mp->m_len = count;
				break;
			}
			count -= mp->m_len;
		} while((mp = mp->m_next) != NULL);
		if(mp) {
			while((mp = mp->m_next) != NULL) 
				mp->m_len = 0;
		}
	}
}

/**
 * 描述：将mbuf链后面节点的数据往前填充到第一个节点，该函数要求第一个mbuf节点的的有效
 * 		 数据至少为len
 * 参数：len，第一个mbuf节点至少要求的有效数据长度
 * 返回：成功返回一个新的mbuf链表头节点，否则返回NULL
 * 注意：当该函数操作失败时（整个mbuf链的有效数据个数小于len或者len长度大于MHLEN），该
 * 		 该函数将会释放掉传入的mbuf链表所有空间
 */
struct mbuf *mbuf_pullup(struct mbuf *n, ulong len)
{
	ulong space, count;
	struct mbuf *m;

	if(!n)
		return NULL;

	/* 当第一个节点的空闲空间小于要求的len或者该节点为MF_EXT类型时，需要新分配一个mbuf */
	if(!(n->m_flags & MF_EXT) && (n->m_data + len < &n->m_dat[MLEN])) {
		len -= min(len, n->m_len);	
		m = n;
		n = n->m_next;	
	} else {
		if(len > MHLEN)
			goto bad;
		if(!(m = mbuf_get(n->m_type)))
			goto bad;
		m->m_len = 0;
		if(n->m_flags & MF_PKTHDR) {
			MBUF_COPY_PKTHDR(m, n);
			n->m_flags &= ~MF_PKTHDR;
		}
	}
	//注意，m此时已经足够保存下len长度的空间，因此space不可能小于等于0，因此无需作为循环退出条件
	space = &m->m_dat[MLEN] - (m->m_data + m->m_len);
	//len表示还需要向前拷贝的长度
	while(len && n) {
		count = min(min(max(max_protohdr, len), space), n->m_len);
		bcopy(mtod(n, caddr_t), m->m_data + m->m_len, count);
		len -= count;
		n->m_len -= count;
		m->m_len += count;
		space -= count;
		//n节点空间是否被向前挪动完了
		if(n->m_len) 
			n->m_data += count;
		else
			n = mbuf_free(n);
	}
	if(len > 0) {
		mbuf_free(m);
		goto bad;	
	}
	m->m_next = n;

	return m;

bad:	
	mbuf_freem(n);
	return NULL;
}

/**
 * 描述：将mbuf链后面节点的数据往前填充到第一个节点，该函数要求第一个mbuf节点的的有效
 * 		 数据至少为len，该函数类似于mbuf_pullup，但是该函数可接受的len更大，在0-MCLBYTES
 * 		 之间的大小，都是被允许的
 * 参数：len，第一个mbuf节点至少要求的有效数据长度
 * 返回：成功返回一个新的mbuf链表头节点，否则返回NULL
 * 注意：当该函数操作失败时（整个mbuf链的有效数据个数小于len或者len长度大于MCLBYTES），该
 * 		 该函数将会释放掉传入的mbuf链表所有空间
 */
struct mbuf *mbuf_pullup_big(struct mbuf *n, ulong len)
{
	ulong count;
	struct mbuf *m;

	if(!n)
		return NULL;
	if(len <= MHLEN)
		return mbuf_pullup(n, len);

	/* 当第一个节点的空闲空间小于要求的len或者该节点为MF_EXT类型时，需要新分配一个mbuf */
	if((n->m_flags & MF_EXT) && (n->m_data + len < &n->m_ext.me_buf[MCLBYTES])) {
		len -= min(len, n->m_len);	
		m = n;
		n = n->m_next;	
	} else {
		if(len > MCLBYTES)
			goto bad;
		if(!(m = mbuf_get(n->m_type)))
			goto bad;
		/* 分配一个mcl格式的mbuf */
		if(mbuf_getext(m) != 0) {
			mbuf_free(m);
			goto bad;
		}
		m->m_len = 0;
		if(n->m_flags & MF_PKTHDR) {
			m->m_pkthdr = n->m_pkthdr;
			m->m_flags = (n->m_flags & M_CPOYFLAGS) | MF_EXT;
			n->m_flags &= ~MF_PKTHDR;
		}
	}
	//注意，m此时已经足够保存下len长度的空间，len表示还需要向前拷贝的长度
	while(len && n) {
		count = min(len, n->m_len);
		bcopy(mtod(n, caddr_t), m->m_data + m->m_len, count);
		len -= count;
		n->m_len -= count;
		m->m_len += count;
		//n节点空间是否被向前挪动完了
		if(n->m_len) 
			n->m_data += count;
		else
			n = mbuf_free(n);
	}
	if(len > 0) {
		mbuf_free(m);
		goto bad;	
	}
	m->m_next = n;

	return m;

bad:	
	mbuf_freem(n);
	return NULL;
}

/**
 * 描述：拷贝一个已经存在的mbuf链到新分配的一个mbuf链表中
 * 参数：off0，拷贝相对于mbuf链表头节点databuf的起始字节偏移位置；len，拷贝长度，当len
 * 		 为M_COPYALL时，将拷贝从off0到mbuf链最后一个数据字节的长度
 * 返回：拷贝成功返回指向新分配mbuf链表的头节点指针，失败返回NULL
 * 注意：以下函数将会分配一个新的链表；参数len, off0为int类型，这对于mbuf长度而言足够
 */
struct mbuf *mbuf_copy(struct mbuf *m, ulong off0, ulong len)
{
	int off, copyhdr = 0;
	struct mbuf *top = NULL, *n, **np;
	
	/* 是否拷贝pkthdr */
	if(0 == off0 && (m->m_flags & MF_PKTHDR))
		copyhdr = 1;

	off = off0;
	while(off) {
		if(NULL == m) {
			log(LOG_ERR, "mbuf_copy: off %d is too big\n", off0);
			return NULL;
		}
		if(off < m->m_len)	
			break;
		off -= m->m_len;
		m = m->m_next;
	}

	np = &top;
	while(len > 0){
		if(NULL == m) {
			if(len != M_COPYALL)	
				goto toolong;
			break;  		//整个mbuf链已经拷贝完成
		}	
		n = mbuf_get(m->m_type);
		if(NULL == n)
			goto nospace;
		*np = n;
		/* 拷贝packet头 */
		if(copyhdr) {
			MBUF_COPY_PKTHDR(n, m);
			/* 调整packet长度 */
			if(M_COPYALL == len)
				n->m_pkthdr.mp_len -= off0;
			else
				n->m_pkthdr.mp_len = len;
			copyhdr = 0;
		}
		/* 此次操作需要拷贝的长度 */
		n->m_len = min(m->m_len, len);
		
		/* 
 		 * 拷贝数据，如果mbuf为EXT类型，则仅将新mbuf数据缓冲区指针指向原来分配的EXT空间加上off偏移，
 		 * 并增加该EXT空间的引用计数，否则将原来mbuf的缓冲区的数据拷贝到新mbuf的数据缓冲区
 		 */
		if(m->m_flags & MF_EXT) {
			n->m_data = m->m_data + off; 
			if(m->m_ext.me_ref)
				(*(m->m_ext.me_ref))(m);
			else
				++mclrefcnt[mtocl(m->m_ext.me_buf)];	
			n->m_ext = m->m_ext;	//注意：这里的ext中的信息没有加上off偏移
			n->m_flags |= MF_EXT;
		} else 
			bcopy(mtod(m, caddr_t) + off, mtod(n, caddr_t), n->m_len);

		if(len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;	//连接链表
	}

	return top;

nospace:
	log(LOG_ERR, "mbuf_copy: alloc new mbuf failed\n");
	mbuf_freem(top);
	return NULL;

toolong:
	log(LOG_ERR, "mbuf_copy: copy size is too long\n");
	mbuf_freem(top);
	return NULL;
}

/* 分配一个mbuf在m之前，作为预留空间，这里要求len要小于mbuf的数据空间长度 */
static struct mbuf *__mbuf_reserve(struct mbuf *m, ulong len)
{
	struct mbuf *new = NULL;

	new = mbuf_get(m->m_type);
	if(!new) {
		mbuf_freem(m);
		return NULL;
	}

	if(m->m_flags & MF_PKTHDR) {
		MBUF_COPY_PKTHDR(new, m);
		m->m_flags &= ~MF_PKTHDR;
	}

	new->m_next = m;
	m = new;

	if(len < MHLEN)
		MH_ALIGN(m, len);
	m->m_len = len;		

	return m;
}

/**
 * 描述：在mbuf当前有效数据位置向前预留指定长度的空间
 * 参数：m，需要进行向前预留的mbuf节点，len，预留长度
 * 返回：成功返回操作成功后的mbuf指针，否则释放m并返回NULL
 */
struct mbuf *mbuf_reserve(struct mbuf *m, ulong len)
{
	if(!m)
		return NULL;

	if(MBUF_LEADINGSPACE(m) >= len) {
		m->m_data -= len;	
		m->m_len += len;
	} else {
		if(len > MHLEN) {					/* 在这里限制预留的空间不能超过一个pkt格式的mbuf数据空间长度 */
			mbuf_freem(m);
			return NULL;
		}
		m = __mbuf_reserve(m, len);
		if(!m)
			return NULL;
	}

	if(m->m_flags & MF_PKTHDR) 
		m->m_pkthdr.mp_len += len;

	return m;
}

/**
 * 描述：为一个mbuf内存区分配一个外部存储区
 * 返回：操作成功返回0，否则返回1
 */
int mbuf_getext(struct mbuf *m)
{
	MCLGET(m);

	return !(m->m_flags & MF_EXT);
}

/* 尝试回收空间，并进行再一次分配 */
struct mbuf *mbuf_retry(int type)
{
	struct mbuf *m = NULL;

	mbuf_reclaim();
	
#define mbuf_retry(t) NULL
	MGET(m, type);
#undef mbuf_retry
	
	return m;
}

/* 尝试回收空间，并进行再一次分配 */
struct mbuf *mbuf_retrypkt(int type)
{
	struct mbuf *m = NULL;

	mbuf_reclaim();
	
#define mbuf_retry(t) NULL
	MGETPKT(m, type);
#undef mbuf_retry
	
	return m;
}

/**
 * 描述：获取一个一般格式的mbuf空间
 */
struct mbuf *mbuf_get(int type)
{
	struct mbuf *m = NULL;

	if(type >= 256)
		return NULL;
	MGET(m, type);
	
	return m;	
}

/**
 * 描述：获取一个pkt格式的mbuf空间
 */
struct mbuf *mbuf_getpkt(int type)
{
	struct mbuf *m = NULL;

	if(type >= 256)
		return NULL;
	MGETPKT(m, type);
	
	return m;	
}

/**
 * 描述：释放一个mbuf空间
 * 返回：释放节点的下一个mbuf节点
 */
struct mbuf *mbuf_free(struct mbuf *m)
{
	struct mbuf *n = NULL;

	if(!m)
		return NULL;

	MFREE(m, n);
	
	return n;	
}

/**
 * 描述：释放一个mbuf链
 *
 */
void mbuf_freem(struct mbuf *m)
{
	if(!m)
		return;

	do {
		m = mbuf_free(m);
	} while (m);
}

/**
 * 描述：分配一个ncl个vmem簇大小（注意不是mbuf簇）的内存空间
 * 参数：ncl，簇个数（当前系统下使用的簇大小等于4KB）
 * 返回：操作成功返回0，否则返回1
 */
int mbuf_clalloc(int ncl)
{
	caddr_t p;
	int i, nmcl = ncl * (CLBYTES / MCLBYTES);

	if(NULL == (p = (caddr_t)__kmem_alloc(ncl * CLBYTES)))
		return 1;
	
	for(i = 0; i < nmcl; ++i) {
		((union mcluster *)p)->mcl_next = mclfree;
		mclfree = (union mcluster *)p;
		p += MCLBYTES;
		mbstats.ms_freemcl++;
	}
	mbstats.ms_nmcl += nmcl;

	return 0;
}

/**
 * 描述：mbuf空间初始化，该函数将预分配一个vmem簇大小的mbuf空间
 * 返回：操作成功返回0，否则返回1
 */
int mbufinit(void)
{
	int s;

	if(NULL == (mclrefcnt = (long *)kmem_malloc(KMEM_SIZE / MCLBYTES)))
		panic("mclrefcnt");
	bzero(mclrefcnt, KMEM_SIZE / MCLBYTES);

	s = splimp();
	
	if(mbuf_clalloc(1)) {
		splx(s);
		mbuf_reclaim();
		return (mclfree == NULL);
	}

	splx(s);

	log(LOG_DEBUG, "mbuf init done\n");

	return 0;
}

/* 回收不用mbuf内存空间 */
void mbuf_reclaim(void)
{
	int s = splimp();

	splx(s);
}
