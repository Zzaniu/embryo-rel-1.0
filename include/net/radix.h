
/**
 * Copyright(c) 2017-8-2 Shangwen Wu	
 *
 * BSD系统radix路由树实现定义 
 * 
 */

#ifndef __NET_RADIX_H__
#define __NET_RADIX_H__

struct radix_mask;

/**
 * radix树节点
 */
struct radix_node {
	struct radix_mask *rn_masklist;		//普适路由的链表头，中间节点该字段指向链表表头，叶子节点仅指向自己对应的radix_mask节点
	struct radix_node *rn_p;			//指向父节点
	int16_t rn_b;						//需要进行比较的bit位置，从0开始；叶子节点的该字段表示-1-netmask索引
	uint8_t rn_bmask;					//比较字节的bit位掩码
	uint8_t rn_flags;	
#define RNF_ROOT		0x1				//两个固定的根节点，一个key全为0，一个key值全为1
#define RNF_ACTIVE		0x2				//表示该节点正在被引用，但是并不参与路由匹配时的有效节点判定
#define RNF_NORMAL		0x4				//表示该路由节点是否包含一个“规范”的掩码值
	union {
		struct {
			caddr_t rnu_key;			//叶子节点比较建值
			caddr_t rnu_mask;			//叶子节点比较掩码
			struct radix_node *rnu_dupedkey;//重复链表，该链表所有节点的key相同，由掩码长度从到短的方式排列
		} rn_leaf;						//叶子节点
		struct {
			uint32_t rnu_off;			//需要比较的字节偏移位置，top节点的该字段表示参与匹配时，可跳过的字节数
			struct radix_node *rnu_l;	//左孩子
			struct radix_node *rnu_r;	//右孩子
		} rn_node;						//中间节点
	} rn_u;
};
#define rn_key rn_u.rn_leaf.rnu_key
#define rn_mask rn_u.rn_leaf.rnu_mask
#define rn_dupedkey rn_u.rn_leaf.rnu_dupedkey
#define rn_off rn_u.rn_node.rnu_off
#define rn_l rn_u.rn_node.rnu_l
#define rn_r rn_u.rn_node.rnu_r

/**
 * radix掩码链表节点
 */
struct radix_mask {
	uint16_t rm_b;						//该字段表示-1-netmask索引
	uint16_t rm_flags;	
	struct radix_mask *rm_next;			//下一节点
	union {
		caddr_t rmu_mask;					//掩码值，当掩码所在的radix_node节点为RNF_NORMAL时有效
		struct radix_node *rmu_leaf;		//指向该掩码对应的叶子节点
	} rm_u;
	ulong rm_refcnt;
};
#define rm_mask rm_u.rmu_mask
#define rm_leaf rm_u.rmu_leaf

#define MKGet(m) do { \
	if(rn_mkfreelist) { \
		(m) = rn_mkfreelist; \
		rn_mkfreelist = (m)->rm_next; \
	} else \
		(m) = (struct radix_mask *)kmem_malloc(sizeof(struct radix_mask)); \
} while(0)

#define MKFree(m) do { \
	(m)->rm_next = rn_mkfreelist; \
	rn_mkfreelist = (m); \
} while(0)

/**
 * radix树头节点
 */
struct radix_node_head {
	struct radix_node *rnh_treetop;		//指向树的顶节点
	struct radix_node *(*rnh_match)(void *, struct radix_node_head *);
	struct radix_node *(*rnh_addaddr)(void *, void *, struct radix_node_head *, struct radix_node[]);
	struct radix_node rnh_nodes[3];		//保存top以及两个root节点（一个key值全为0，一个全为1）
};

extern int rn_refines(void *m_arg, void *n_arg);
extern int rn_inithead(void **rnhp, int off);
extern void rn_init(void);

#endif //__NET_RADIX_H__
