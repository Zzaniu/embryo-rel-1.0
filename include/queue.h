
/**
 * Copyright(c) 2016-8-11 Shangwen Wu	
 *
 * 环形队列头文件
 * 
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef unsigned char Unit;

typedef struct queue {
	unsigned short first;			//表示下一个将要取走数据的地方
	unsigned short count;			//已经存放个数
	unsigned short limit;			//size - 1
	Unit units[1];					//舍弃一个空间用于区分满和空这两种状态
}Queue;

#define Qisempty(q)		(q->count == 0)
#define Qisfull(q)		(q->count == q->limit)
#define Qused(q)		(q->count)
#define Qspace(q)		(q->limit - q->count)

extern void Qput(Queue *, Unit);
extern Unit Qget(Queue *);
extern Unit Qread(Queue *, unsigned short);
extern Queue * Qcreate(unsigned short); 
extern void Qfree(Queue *);

#endif //__QUEUE_H__
