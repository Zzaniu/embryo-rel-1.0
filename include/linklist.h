
/**
 * Copyright(c) 2016-8-15 Shangwen Wu	
 *
 * 链表操作头文件
 * 
 */

#ifndef __LINKLIST_H__
#define __LINKLIST_H__

/**
 * 1.单链操作
 */
#define SLIST_HEAD(name, type) 						\
struct name {										\
	struct type *slh_first;							\
}

#define SLIST_ENTRY(type)							\
struct {											\
	struct type *sle_next;							\
}

#define SLIST_INITIALIZER(head)						\
{NULL}

#define SLIST_FIRST(head)							\
((head)->slh_first)

#define SLIST_END(head)								\
NULL

#define SLIST_INIT(head)							\
do {												\
	SLIST_FIRST(head) = SLIST_END(head)				\
} while(0)

#define SLIST_IS_EMPTY(head)						\
(SLIST_FIRST(head) == SLIST_END(head))

#define SLIST_NEXT(ele, field)						\
((ele)->field.sle_next)

#define SLIST_FOREACH(head, var, field)				\
for((var) = SLIST_FIRST(head); 						\
    (var) != SLIST_END(head); 						\
	(var) = SLIST_NEXT(var, field))

#define SLIST_INSERT_HEAD(head, ele, field)			\
do {												\
	(ele)->field.sle_next = (head)->slh_first;		\
	(head)->slh_first = (ele);						\
} while (0)

#define SLIST_INSERT_TAIL(head, var, ele, field)	\
do {												\
	if(!SLIST_IS_EMPTY(head)) {						\
		for((var) = SLIST_FIRST(head); 				\
			(var)->field.sle_next != NULL;			\
			(var) = SLIST_NEXT(head));				\
		(var)->field.sle_next = (ele);				\
		(ele)->field.sle_next = SLIST_END(head);	\
	}												\
	else											\
		SLIST_INSERT_HEAD(head, ele, field);		\
} while (0)

#define SLIST_INSERT_AFTER(head, tgt, ele, field)	\
do {												\
	(ele)->field.sle_next = (tgt)->field.sle_next;	\
	(tgt)->field.sle_next = (ele);					\
} while(0)											

/**
 * 循环双链操作
 */
#define CLIST_HEAD(name, type) 						\
struct name {										\
	struct type *clh_first;							\
	struct type *clh_last;							\
}

#define CLIST_ENTRY(type)							\
struct {											\
	struct type *cle_next;							\
	struct type *cle_prev;							\
}

#define CLIST_INITIALIZER(head)						\
{CLIST_END(head), CLIST_END(head)}

#define CLIST_FIRST(head)							\
((head)->clh_first)

#define CLIST_LAST(head)							\
((head)->clh_last)

#define CLIST_END(head)								\
((void *)head)

#define CLIST_INIT(head)							\
do {												\
	CLIST_FIRST(head) = CLIST_END(head),			\
	CLIST_LAST(head) = CLIST_END(head),				\
} while(0)
 
#define CLIST_IS_EMPTY(head)						\
(CLIST_FIRST(head) == CLIST_END(head))

#define CLIST_NEXT(ele, field)						\
((ele)->field.cle_next)

#define CLIST_PREV(ele, field)						\
((ele)->field.cle_prev)

#define CLIST_FOREACH(head, var, field)				\
for((var) = CLIST_FIRST(head); 						\
    (var) != CLIST_END(head); 						\
	(var) = CLIST_NEXT(var, field))

#define CLIST_FOREACH_REVERSE(head, var, field)		\
for((var) = CLIST_LAST(head); 						\
    (var) != CLIST_END(head); 						\
	(var) = CLIST_PREV(var, field))

#define CLIST_INSERT_AFTER(head, tgt, ele, field)	\
do {												\
	(ele)->field.cle_next = (tgt)->field.cle_next;	\
	(ele)->field.cle_prev = (tgt);					\
	if((tgt)->field.cle_next == CLIST_END(head))	\
		(head)->clh_last = (ele);					\
	else											\
		(tgt)->field.cle_next->field.cle_prev = (ele);\
	(tgt)->field.cle_next = (ele);					\
} while(0)

#define CLIST_INSERT_BEFORE(head, tgt, ele, field)	\
do {												\
	(ele)->field.cle_prev = (tgt)->field.cle_prev;	\
	(ele)->field.cle_next = (tgt);					\
	if((tgt)->field.cle_prev == CLIST_END(head))	\
		(head)->clh_first = (ele);					\
	else											\
		(tgt)->field.cle_prev->field.cle_next = (ele);\
	(tgt)->field.cle_prev = (ele);					\
} while(0)

#define CLIST_INSERT_HEAD(head, ele, field)			\
do {												\
	(ele)->field.cle_prev = (void *)(head);			\
	(ele)->field.cle_next = CLIST_FIRST(head);		\
	if(CLIST_IS_EMPTY(head))						\
		CLIST_LAST(head) = (ele);					\
	else											\
		CLIST_FIRST(head)->field.cle_prev = (ele);	\
	CLIST_FIRST(head) = (ele);						\
} while(0)

#define CLIST_INSERT_TAIL(head, ele, field)			\
do {												\
	(ele)->field.cle_next = (void *)(head);			\
	(ele)->field.cle_prev = CLIST_LAST(head);		\
	if(CLIST_IS_EMPTY(head))						\
		CLIST_FIRST(head) = (ele);					\
	else											\
		CLIST_LAST(head)->field.cle_next = (ele);	\
	CLIST_LAST(head) = (ele);						\
} while(0)

#define CLIST_REMOVE(head, ele, field)				\
do {												\
	if((ele)->field.cle_prev == (void *)head)		\
		CLIST_FIRST(head) = (ele)->field.cle_next;	\
	else											\
		(ele)->field.cle_prev->field.cle_next = (ele)->field.cle_next;\
	if((ele)->field.cle_next == (void *)head)		\
		CLIST_LAST(head) = (ele)->field.cle_prev;	\
	else											\
		(ele)->field.cle_next->field.cle_prev = (ele)->field.cle_prev;\
} while(0)

#endif	//__LINKLIST_H__

