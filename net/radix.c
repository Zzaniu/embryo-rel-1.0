
/**
 * Copyright(c) 2017-8-2 Shangwen Wu	
 *
 * EM-BIOS目前使用radix数来实现路由查找，该文件时bsd系统对路由的设计实现
 * 
 */

#include <common.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/system.h>
#include <sys/syslog.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <sys/domain.h>
#include <sys/protocol.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_type.h>
#include <net/radix.h>

static caddr_t rn_zeros, rn_ones, rn_addrmaskkey;
static int rn_maxkeylen = 0;
static struct radix_node_head *mask_rnhead = NULL;

static struct radix_mask *rn_mkfreelist = NULL;
static char normal_chars[] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};

/**
 * 描述：该函数用于表较两个bit串之前，bit1的亲和度
 * 返回：如果m_arg更优，返回1，否则返回0
 *
 */
int rn_refines(void *m_arg, void *n_arg)
{
	caddr_t n = (caddr_t)n_arg, m = (caddr_t)m_arg;
	caddr_t lim, lim2 = lim = n + *(uchar *)n;
	int longer = (*(uchar *)n++) - (*(uchar *)m++);
	int isequal = 1;

	if(longer > 0)
		lim -= longer;

	while(n < lim) {
		if(*n & ~(*m))
			return 0;
		if(*n++ != *m++)
			isequal = 0;
	}

	while(n < lim2)
		if(*n++)
			return 0;

	if(isequal && longer < 0) 
		for(lim2 = m - longer; m < lim2; ++m)
			if(*m)   
				return 1;

	return !isequal;
}

/**
 * 描述：比较两个掩码谁的掩码更长（根本不知道比的是什么-_-）
 * 返回：如果m_arg更优，返回1，否则返回0
 */
static int rn_lexobetter(void *m_arg, void *n_arg)
{
	uchar *n = (uchar *)n_arg, *m = (uchar *)m_arg, *lim;

	if(*m > *n)
		return 1;

	if(*m == *n) {
		lim = n + *n;
		while(n < lim)
			if(*++m > *++n)
				return 1;
	}

	return 0;
}

/**
 * 描述：对叶子节点的key进行带掩码的比较
 * 返回：返回1表示满足匹配，否则不匹配
 */
static int rn_satisfies_leaf(void *trial, struct radix_node *leaf, uint skip)
{
	caddr_t cp = (caddr_t)trial, cp2 = leaf->rn_key, cplim, cp3;
	ulong len = min(*(uchar *)cp, *(uchar *)cp2);

	if(leaf->rn_mask)
		len = min(len, *(uchar *)leaf->rn_mask);
	else
		leaf->rn_mask = rn_ones;

	cplim = cp + len;
	for(cp += skip, cp2 += skip, cp3 = leaf->rn_mask + skip; cp < cplim; ++cp, ++cp2)
		if((*cp ^ *cp2) & *cp3)
			return 0;

	return 1;
}

/**
 * 描述：根据键值查找当前节点的子树中对应的叶子节点
 */
static struct radix_node *rn_search(void *v, struct radix_node *top)
{
	struct radix_node *rn = top;
	caddr_t cp = (caddr_t)v;

	while(rn->rn_b >= 0) {
		log(LOG_DEBUG, "rn_search: rn_b==%d, rn_off=%d, rn_bmask=0x%x\n", rn->rn_b, rn->rn_off, rn->rn_bmask);
		if(cp[rn->rn_off] & rn->rn_bmask) 
			rn = rn->rn_r;
		else
			rn = rn->rn_l;
	}

	return rn;
}

/**
 * 描述：根据掩码和键值查找当前节点的子树中对应的叶子节点
 */
static struct radix_node *rn_search_mask(void *v, void *m, struct radix_node *top)
{
	struct radix_node *rn = top;
	caddr_t cp = (caddr_t)v, cm = (caddr_t)m;

	while(rn->rn_b >= 0) {
		if((cp[rn->rn_off] & rn->rn_bmask) && (cm[rn->rn_off] & rn->rn_bmask))
			rn = rn->rn_r;
		else
			rn = rn->rn_l;
	}

	return rn;
}

/**
 * 描述：插入一个节点到radix树，该函数仅实现树的插入操作，而不设计普适提升以及重复链表方面的内容
 */
static struct radix_node *rn_insert(void *v, int *dupedkey, \
			struct radix_node_head *rnh, struct radix_node nodes[2])
{
	struct radix_node *t, *tt;
	struct radix_node *top = rnh->rnh_treetop;
	ulong off = top->rn_off, b; 
	int len = *(uchar *)v;
	caddr_t cp = (caddr_t)v + off;

	/**
 	 * 第一步：寻叶求异，查找路径匹配的叶子节点，如果该叶子节点的key等于
     * 新键值则记录当前新插入节点位于重复链表，并返回这个重复链表表头的叶
     * 子节点，否则将计算第一次出现不同的bit位置
     */
	{
		caddr_t cp2, cplim = (caddr_t)v + len;
		uchar diff;

		t = rn_search(v, top);
		cp2 = (caddr_t)t->rn_key + off, cplim = (caddr_t)v + len;

		while(cp < cplim) {
			if(*cp++ != *cp2++)
				goto noequal;
		}
		*dupedkey = 1;
		
		return t;

noequal:
		*dupedkey = 0;
		b = (cp - (caddr_t)v) << 3;
		for(diff = cp[-1] ^ cp2[-1]; diff; diff >>= 1)
			--b;
	}

	/**
 	 * 第二步：存异求同，新节点插入指定位置 
 	 */
	{
		struct radix_node *p, *x = top;

		cp = (caddr_t)v;
		/* 找到一个节点其中x的rn_b要大于b，而p的rn_b要小于b */
		do {
			p = x;
			if(cp[x->rn_off] & x->rn_bmask)
				x = x->rn_r;
			else
				x = x->rn_l;
		/** 
 		 * 注意这里要将b->rn_b转成无符号整数，因为叶子节点的rn_b是个负数，而遇到叶子节点时
 		 * 同样要结束循环，而强转成无符号的rn_b必然要远大于b，因而能导致循环退出
 		 */
		} while(b > (ulong)x->rn_b);

		/* t为叶子节点，tt为中间节点 */
		t = nodes, tt = t + 1;
		tt->rn_b = b;
		tt->rn_off = b >> 3;
		tt->rn_bmask = 0x80 >> (b & 0x7);
		t->rn_b = -1;	//叶子节点默认值为1，当实际的掩码不为NULL时，需要修改该值
		t->rn_p = tt;
		t->rn_key = (caddr_t)v;
		t->rn_flags = tt->rn_flags = RNF_ACTIVE;
		
		tt->rn_p = p;
		if(cp[p->rn_off] & p->rn_bmask)
			p->rn_r = tt;
		else
			p->rn_l = tt;
		
		x->rn_p = tt;
		if(cp[tt->rn_off] & tt->rn_bmask) {
			tt->rn_r = t;
			tt->rn_l = x;
		} else {
			tt->rn_r = x;
			tt->rn_l = t;
		}
	}

	return t;
}

/**
 * 描述：路由查找函数，bad codes，自始至终该函数均未对v以及leaf的key值进行边界检查，未对可能的越界做任何处理
 * 参数：v，输入的需要进行比较的键（dst地址，sockaddr结构的数据），当前查找的radix树头节点
 */
static struct radix_node *rn_match(void *v, struct radix_node_head *rnh)
{
	caddr_t cp = (caddr_t)v, cp2, cplim;
	struct radix_node *rx, *rn, *save_rn = rn = rnh->rnh_treetop;
	uchar vlen = *(uchar *)v;			//取v的第一个字节为比较长度
	uint off = rn->rn_off, matched_off;				
	ushort rn_b;	//-1 - 未匹配的起始bit位置
	uchar b, test;
	struct radix_mask *rm;

	/* 1.寻叶，host地址匹配查找 */
	/* 查找bit位置测试匹配的叶子节点 */
	while(rn->rn_b >= 0)	{
		if(cp[rn->rn_off] & rn->rn_bmask)
			rn = rn->rn_r;
		else
			rn = rn->rn_l;
	}

	if(rn->rn_mask)
		vlen = *(uchar *)rn->rn_mask;		//当叶子节点的mask不为空时，取mask的第一个字节为比较长度

	cplim = cp + vlen;
	cp += off;
	cp2 = rn->rn_key + off;

	while(cp < cplim) 
		if(*cp++ != *cp2++)	//注意：这里进行key的完全比较而非掩码比较，原因是路由查找时，优先查找host地址匹配的路由
			goto lookup_dup;

	/* 找到host地址匹配的叶子节点 */
	if((rn->rn_flags & RNF_ROOT) && rn->rn_dupedkey)
		rn = rn->rn_dupedkey;

	return rn;

	/* 2.辩重，网络地址匹配查找 */
lookup_dup:
	/* 计算匹配不成功的起始bit位置 */	
	test = (*--cp ^ *--cp2) & 0xff;
	for(b = 7; test >>= 1; --b)		//大端
		;
	matched_off = cp - (caddr_t)v;
	b += matched_off << 3;
	rn_b = -1 - b;
	
	/* 
 	 * 若mask为空表示当前rn为host直接地址，即跳过该节点的网络掩码比较，对于非host地址，
 	 * 将会在完全比较之后再进行带掩码的比较 
 	 */
	if(!(save_rn = rn)->rn_mask)
		rn = rn->rn_dupedkey;
	/* 遍历重复链表 */
	for(; rn; rn = rn->rn_dupedkey) {
		if(rn->rn_flags & RNF_NORMAL) {
			if(rn->rn_b >= rn_b)
				return rn;
		} else if(rn_satisfies_leaf(v, rn, matched_off))
			return rn;
	}

	/* 3.回溯，开始查找普适路由 */
	rn = save_rn;
	
	do {
		rn = rn->rn_p;
		if(rn->rn_masklist) {
			rm = rn->rn_masklist;
			do {
				if(rm->rm_flags & RNF_NORMAL) {
					if(rm->rm_b >= rn_b)
						return rm->rm_leaf;
				} else {
					rx = rn_search_mask(v, rm->rm_mask, rn);
					while(rx && rx->rn_mask != rm->rm_mask)
						rx = rx->rn_dupedkey;
					matched_off = min(matched_off, rn->rn_off);
					if(rn_satisfies_leaf(v, rn, matched_off))
						return rx;
				}
				rm = rm->rm_next;
			} while (rm);
		}
	} while (rn->rn_p != rn);

	return NULL;
}

/**
 * 描述：新分配一个radix_mask
 */
static struct radix_mask *rn_new_radix_mask(struct radix_node *t, struct radix_mask *next)
{
	struct radix_mask *rm;

	MKGet(rm);
	if(NULL == rm) {
		log(LOG_ERR, "rn_new_radix_mask: mask for route not entered, out of memory\n");
		return NULL;
	}
	bzero(rm, sizeof(struct radix_mask));

	rm->rm_b = t->rn_b;
	rm->rm_flags = t->rn_flags;
	if(rm->rm_flags & RNF_NORMAL)
		rm->rm_leaf = t;
	else
		rm->rm_mask = t->rn_mask;

	t->rn_masklist = rm;
	rm->rm_next = next;
	
	return rm;
}

/**
 * 描述：新创建一个radix节点，其键值为netmask，并将该节点添加到mask_rnhead树中
 * 参数：v，表示新节点的key值，也是某个路由的网络掩码；search，是否表示仅查找匹配该key的节点，
 * 		 而不进行插入操作；skip，表示传入netmask应当跳过的比较位，当netmask的长度小于该值时，
 * 		 将直接返回一个key为全0的ROOT节点
 */
static struct radix_node *rn_addmask(void *v, int search, ulong skip)
{
	caddr_t netmask = (caddr_t)v, cp, cplim;
	int mlen, mlen0;
	int isduped, isnormal;
	int bit, b = 0;	//掩码索引值
	static int last_zeros = 0;	//上一次掩码bit为0开始的位置
	struct radix_node *t, *rnm, *saved_rnm;
	
	mlen = *(uchar *)netmask;
	if(mlen > rn_maxkeylen)
		mlen = rn_maxkeylen;
	
	if(0 == skip)
		skip = 1;	//留出len的位置

	if(mlen <= skip)
		return mask_rnhead->rnh_nodes;	//返回掩码radix树的全0 ROOT节点

	if(skip > 1)
		bcopy(rn_ones + 1, rn_addrmaskkey + 1, skip - 1);
	bcopy(netmask + skip, rn_addrmaskkey + skip, mlen - skip);

	/* 截取掩码尾部为0的部分 */
	for(cp = rn_addrmaskkey + mlen; !cp[-1]; --cp)
		;
	mlen0 = mlen;
	mlen = cp - rn_addrmaskkey;

	if(mlen <= skip) {
		if(mlen0 >= last_zeros)		//还是画图理解吧-_-
			last_zeros = mlen;		//更新bit0开始的位置为当前掩码的末尾
		return mask_rnhead->rnh_nodes;	
	}
	if(mlen0 < last_zeros)
		bzero(rn_addrmaskkey + mlen0, last_zeros - mlen0); //确保掩码后续部分（包括超过mlen0的部分）以全0结尾
	*rn_addrmaskkey = last_zeros = mlen;

	t = rn_search(rn_addrmaskkey, mask_rnhead->rnh_treetop);
	
	/* 查找的叶子节点是否与当前的掩码key值相等 */
	if(bcmp(rn_addrmaskkey, t->rn_key, mlen) != 0)
		t = NULL;

	if(t || search)
		return t;

	/* 分配一个叶子节点一个中间节点，以及存放key的空间 */
	if(NULL == (rnm = saved_rnm = (struct radix_node *)kmem_zmalloc(2 * sizeof(struct radix_node) + rn_maxkeylen))) {
		log(LOG_ERR, "malloc mask radix_node failed\n");
		return NULL;
	}
	netmask = cp = (caddr_t)(rnm + 2);
	bcopy(rn_addrmaskkey, (caddr_t)(rnm + 2), mlen);
	
	/* 插入到radix树 */
	rnm = rn_insert(cp, &isduped, mask_rnhead, rnm);
	if(isduped) {
		log(LOG_ERR, "rn_addmask: mask impossibly in tree\n");
		kmem_free(saved_rnm);
		return rnm;
	}

	/* 检查该掩码是否时规范的，并将设置该新叶子节点的掩码索引值 */
	isnormal = 1;
	for(cplim = cp + mlen, cp = netmask + skip; (cp < cplim) && (*(uchar *)cp == 0xff); ++cp)
		;
	if(cp != cplim) {
		for(bit = 0x80; bit && (bit & *cp); bit >>= 1) 
			++b;
		if(*cp != normal_chars[b] || cp != cplim - 1)
			isnormal = 0;			/* 当掩码值中间出现bit0，则认为非规范 */
	}

	/* 设置掩码索引值 */
	b += (cp - netmask) << 3;
	rnm->rn_b = -1 - b;	

	if(isnormal)
		rnm->rn_flags |= RNF_NORMAL;

	return rnm;
}

/**
 * 描述：添加一个路由表项对应的radix_node到radix树中，根据传入的sockaddr信息
 * 参数：v，表示新增路由节点的KEY值即该路由的目的地址，注意，该地址将被新的radix节点引用，
 * 		 因此该地址不能被随意释放，而只能在释放radix后才能被释放；m，新路由的掩码，当RTF_HOST
 * 		 被置位时，该值为NULL；rnh，当前网络域所在的radix树头节点；nodes，包含两个radix节点的
 * 		 数组，其中第一个元素用于保存新增的叶子节点，第二个元素保存可能新增的中间节点
 */
static struct radix_node *rn_addroute(void *v, void *m, struct radix_node_head *rnh, struct radix_node nodes[2])
{
	int isduped;
	ulong rn_b, b;
	caddr_t rmmask, netmask = (caddr_t)m;
	struct radix_node *rnm;			//新创建的mask掩码树的radix节点
	struct radix_node *top = rnh->rnh_treetop;	//当前radix数的顶节点
	struct radix_node *head_t, *t, *tt;
	struct radix_node *p, *x;
	struct radix_mask *rm, **rmp;
	
	if(netmask) {
		if(NULL == (rnm = rn_addmask(netmask, 0, top->rn_off)))
			return NULL;
		rn_b = rnm->rn_b;
		b = -1 - rn_b;
		netmask =  rnm->rn_key;
	}

	head_t = t = rn_insert(v, &isduped, rnh, nodes);
	/* 当插入的新节点位于重复链表时，查找适合的插入位置 */
	if(isduped) {
		log(LOG_DEBUG, "rn_addroute: new node in duplicated list\n");
		for(tt = t; t; tt = t, t = t->rn_dupedkey) {
			/* 掩码地址一样的不做任何操作（包括NULL），直接返回NULL */
			if(t->rn_mask == netmask) {
				log(LOG_ERR, "rn_addroute: same netmask reference\n");
				return NULL;
			}
			/* 新节点掩码和当前重复链表节点掩码相同的情况，不做任何操作，直接返回NULL */
			if((*(uchar *)t->rn_mask == *(uchar *)netmask) && \
						!bcmp(t->rn_mask, netmask, *(uchar *)netmask)) {
				log(LOG_ERR, "rn_addroute: the same netmask\n");
				return NULL;
			}
			/* 链表按照掩码从到短顺序排列，无掩码排在链表头 */
			if(!netmask || \
				(t->rn_mask && \
				(rn_b < t->rn_b || \
				rn_refines(netmask, t->rn_mask) || \
				rn_lexobetter(netmask, t->rn_mask))))
				break;
		}
		/* 插入位置位于队头 */
		if(t == head_t) {
			p = head_t->rn_p;
			(t = nodes)->rn_dupedkey = head_t;
			if(p->rn_l == head_t)
				p->rn_l = t;
			else
				p->rn_r = t;
			t->rn_p = p;
			head_t = t;
		} else {
			(t = nodes)->rn_dupedkey = tt->rn_dupedkey;
			tt->rn_dupedkey = t;
		}
		t->rn_b = -1;
 		//bad code 在rtrequest函数中，已经进行了key的赋值，如果此处的v与rn->rn_key指针不一致，将会引起内存泄漏
		t->rn_key = (caddr_t)v;
		t->rn_flags = RNF_ACTIVE;
	}

	if(netmask) {
		t->rn_b = rnm->rn_b;
		t->rn_mask = netmask;
		t->rn_flags |= rnm->rn_flags & RNF_NORMAL;
	}
	
	p = head_t->rn_p;		//P指向新增加的叶子节点的父节点

	if(isduped)
		goto no_promote;
	
	/* 提升新节点的兄弟节点的普适路由 */
	rn_b = -1 - p->rn_b;
	if(p->rn_l == t)
		x = p->rn_r;
	else
		x = p->rn_l;		//x指向新叶子节点的兄弟节点
	
	if(x->rn_b < 0) {
		/**
 		 * 如果兄弟节点为叶子节点，则需要对该叶子节点的重复链表中那些可能位于新
 		 * 中间节点的普适路由进行提升，前提是这些叶子节点之前没有任何普适路由（
 		 * 即rn_masklist为NULL）
 		 */
		rmp = &p->rn_masklist;
		for(;x;x = x->rn_dupedkey) {
			if(x->rn_mask && !x->rn_masklist && x->rn_b >= rn_b) {
				*rmp = rm = rn_new_radix_mask(x, NULL);
				if(rm)
					rmp = &rm->rm_next;
			}
		}
	} else if(x->rn_masklist) {
		/**
 		 * 如果兄弟节点为中间节点，且该节点已经有掩码链表，那么需要该链表中掩码
		 * 索引值小于新中间节点的节点提升到新中间节点的掩码链表 
		 */
		for(rmp = &x->rn_masklist; (rm = *rmp); rmp = &rm->rm_next) 
			if(rm->rm_b >= rn_b)
				break;
		p->rn_masklist = rm;	//截取后面的链表部分到p的掩码链表中
		*rmp = NULL;
	}

no_promote:

	/**
 	 * 提升新叶子节点的普适路由
     */
	if(!netmask || b > p->rn_b)
		return t;	//无掩码的host路由以及掩码索引值达不到祖先节点的普适要求的无需提升普适路由
	rn_b = t->rn_b;
	/* 向上回溯，找到新节点普适路由的最大子树的树顶节点 */	
	do {
		x = p;
		p = p->rn_p;
	} while(b <= p->rn_b && x != top);

	/* 按照特定的优先级插入节点x的掩码链表  */
	for(rmp = &x->rn_masklist; (rm = *rmp); rmp = &rm->rm_next) {
		if(rm->rm_b < rn_b)
			continue;
		if(rm->rm_b > rn_b)
			break;
		/* rn_b ==  rm->rm_b */
		if(rm->rm_flags & RNF_NORMAL) {
			rmmask = rm->rm_leaf->rn_mask;
			/* 出现相同的掩码 */
			if(t->rn_flags & RNF_NORMAL) {
				log(LOG_ERR, "rn_addroute: mask for route not entered, non-unique normal route\n");
				return t;
			}
		} else 
			rmmask = rm->rm_mask;
		/* 重复掩码引用的，增加该radix_mask的引用计数 */
		if(rmmask == netmask) {
			rm->rm_refcnt++;
			t->rn_masklist = rm;
			return t;
		}
		/* 新节点掩码和某个已经在链表中的掩码相同的情况，不做任何操作，直接返回 */
		if((*(uchar *)rmmask == *(uchar *)netmask) && \
					!bcmp(rmmask, netmask, *(uchar *)netmask)) {
			log(LOG_ERR, "rn_addroute: mask for route not entered, the same netmask\n");
			return t;
		}
		if(rn_refines(netmask, rmmask) || rn_lexobetter(netmask, rmmask))
			break;
	}

	*rmp = rn_new_radix_mask(t, *rmp);

	return t;
}

/**
 * 描述：初始化一个radix树，并初始化radix树最初的三个radix_node节点，其中node[1]为top节点，
 * 		 node[0]为左节点，node[2]为右节点
 */
int rn_inithead(void **rnhp, int off)
{
	struct radix_node_head *rnh;
	struct radix_node *l, *t, *r;

	if(NULL == (rnh = kmem_zmalloc(sizeof(struct radix_node_head)))) 
		panic("rn_init: out of memory for rn radix_node_head");
	t = &rnh->rnh_nodes[1];
	l = &rnh->rnh_nodes[0];
	r = &rnh->rnh_nodes[2];
	
	/* top node */
	t->rn_p = t;	
	t->rn_r = r;
	t->rn_l = l;
	t->rn_b = off;
	t->rn_bmask = 0x80 >> (off & 7);	//注意：大端模式
	t->rn_off = off >> 3;
	t->rn_flags = RNF_ROOT | RNF_ACTIVE;

	/* right and left child node */
	l->rn_p = t;
	l->rn_b = -1 - off;
	l->rn_flags = t->rn_flags;
	
	*r = *l;
	l->rn_key = rn_zeros;
	r->rn_key = rn_ones;

	rnh->rnh_treetop = t; 
	rnh->rnh_match = rn_match; 
	rnh->rnh_addaddr = rn_addroute; 
	*(struct radix_node_head **)rnhp = rnh;

	return 0;
}

/**
 * 描述：初始化一个掩码radix树
 * 注意：该函数出现任何错误将引起系统崩溃
 */
void rn_init(void)
{
	struct domain *dp;
	caddr_t cp;
	extern struct domain *domains;
	
	for(dp = domains; dp != NULL; dp = dp->dom_next) 
		if(rn_maxkeylen < dp->dom_maxrtkey)
			rn_maxkeylen = dp->dom_maxrtkey;
	if(!rn_maxkeylen) 
		panic("rn_init: rn_maxkeylen is not set");
		
	if(NULL == (cp = kmem_zmalloc(rn_maxkeylen * 3))) 
		panic("rn_init: out of memory for rn buf");
	
	rn_zeros = cp;
	bzero(rn_zeros, rn_maxkeylen);
	rn_ones = rn_zeros + rn_maxkeylen;
	memset(rn_ones, 0xff, rn_maxkeylen);
	rn_addrmaskkey = rn_ones + rn_maxkeylen;

	rn_inithead((void **)&mask_rnhead, 0);
}
