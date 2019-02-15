
/**
 * Copyright(c) 2016-6-30 Shangwen Wu	
 *
 * list_head链表相关头文件
 * 
 */

#ifndef __SYS_LIST_H__
#define __SYS_LIST_H__

/* 内核链表定义 */
struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) {&(name),&(name)}

/* 初始化头部节点,并声明一个名字为name的link_head头节点 */
#define LIST_HEAD(name)    						\
		struct list_head name = LIST_HEAD_INIT(name)

/*节点初始化，将next和prev指向自己*/
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/*插入一个节点*/
static inline void __list_add(struct list_head *new, 
			struct list_head *prev, struct list_head *next)
{
	new->prev = prev;
	new->next = next;
	prev->next = new;
	next->prev = new;
}

/*头部添加*/
static inline void list_add(struct list_head *new, 
			struct list_head *head)	
{
	__list_add(new, head, head->next);
}

/*尾部添加*/
static inline void list_add_tail(struct list_head *new,
			struct list_head *head)	
{
	__list_add(new, head->prev, head);
}

/*删除节点*/
static inline void __list_del(struct list_head *prev,
			struct list_head *next)
{
	prev->next = next;
	next->prev = prev;	
}

/*删除节点*/
static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

/*删除某个list_head并初始化删除的节点*/
static inline void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

/*判断链表是否为空*/
static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

/* 是否为队首位置元素 */
static inline int list_is_last(const struct list_head *entry,
			const struct list_head *head)
{
	return head->prev == entry;
}

/* 是否为队尾位置元素 */
static inline int list_is_first(const struct list_head *entry,
			const struct list_head *head)
{
	return head->next == entry;
}

/* 返回队尾位置list_head */
static inline struct list_head *list_last(struct list_head *head)
{
	return head->prev;
}

/* 返回队首位置list_head */
static inline struct list_head *list_first(struct list_head *head)
{
	return head->next;
}

/**
 * 计算link_head所在结构体的首地址偏移量
 * @TYPE: the type of container 
 * @MEMBER: the name of list_head whitin container struct
 */
#define offsetof(TYPE, MEMBER) (size_t)(&(((TYPE *)0)->MEMBER))

/**
 * 根据成员地址计算所在结构体的首地址
 * @ptr: the point of the list_head
 * @type: the type of the container
 * @member: the name of list_head whitin container struct
 */
#define container_of(ptr, type, member)							\
		({														\
			const typeof(*(ptr)) *__mptr = (ptr);				\
			(type *)((char *)__mptr - offsetof(type, member));	\
		})

/**
 * 调用container_of的中间方法
 * @ptr: the point of the list_head
 * @type: the type of the container struct
 * @member: the name of list_head whitin container struct
 */
#define list_entry(ptr, type, member)							\
		container_of(ptr, type, member)

/**
 * 遍历内核链表，不支持删除多个节点	
 * @pos: the point of struct list_head use as loop cursor
 * @head: the head point of struct list_head
 */	
#define list_for_each(pos, head)								\
		for(pos = (head)->next; pos != (head); pos = pos->next)
	
/**
 * 安全的遍历内核链表，支持删除多个节点
 * @pos: the point of list_head use as loop cursor
 * @n: use as  temporary storage
 * @head: the head point of list_head
 */	
#define list_for_each_safe(pos, n, head)						\
		for(pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * 遍历内核链表所在的结构体
 * @pos: the point of container struct,loop cursor
 * @head: the head piont of list_head  
 * @member: the name of list_head whitin the struct 
 */
#define list_for_each_entry(pos, head, member)					\
		for(pos = list_entry((head)->next, typeof(*pos), member); \
			&pos->member != (head);								\
			pos = list_entry(pos->member.next, typeof(*pos), member))
			 
/**
 * 遍历内核链表所在的结构体，用于安全地删除链表节点
 * @pos: the point of container struct,loop cursor
 * @n: use as  temporary storage
 * @head: the head piont of list_head  
 * @member: the name of list_head whitin the struct 
 */
#define list_for_each_entry_safe(pos, n, head, member)			\
		for(pos = list_entry((head)->next, typeof(*pos), member), \
			n = list_entry(pos->member.next, typeof(*pos), member);\
			&pos->member != (head);								\
			pos = n, n = list_entry(n->member.next, typeof(*n), member))

/* 返回队尾位置用户元素指针 */
#define list_first_entry(head, type, member)					\
		container_of(list_first(head), type, member)

/* 返回队首位置用户元素指针 */
#define list_last_entry(head, type, member)						\
		container_of(list_last(head), type, member)

#endif //__SYS_LIST_H__
