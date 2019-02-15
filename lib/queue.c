
/**
 * Copyright(c) 2016-6-11 Shangwen Wu 
 *
 * 环形队列实现
 * 
 */
#include <common.h>
#include <stdlib.h>
#include <queue.h>

Queue * Qcreate(unsigned short n)
{
	Queue *q = NULL;
	
	if(n & (n - 1))				//该环形队列算法的实现要求队列存储空间必须为2的质数倍 
		return NULL;
	
	if(NULL == (q = (Queue *)malloc(sizeof(Queue) + sizeof(Unit) * (n - 1))))
		return NULL;

	q->first = q->count = 0;
	q->limit = n - 1;

	return q;
}

/**
 * 注意：在调用put函数之前需要先判断队列是否有剩余空间，当队列
 * 		没有剩余空间可用，并且仍然调用此函数时，将引起死等
 */
void Qput(Queue *q, Unit u)
{
	while(Qisfull(q))
		;
	q->units[(q->first + q->count) & q->limit] = u;	
	q->count++;
}

/**
 * 注意：在调用get函数之前需要先判断队列是否有可用元素，当队列
 * 		没有可用元素，并且仍然调用此函数时，将引起死等
 */
Unit Qget(Queue *q)
{
	Unit u;

	while(Qisempty(q))
		;
	u = q->units[q->first & q->limit];
	q->first = (q->first + 1) & q->limit;
	q->count--;
	return u;
}

Unit Qread(Queue *q, unsigned short i)
{
	if(i >= Qused(q))
		return 0;

	return q->units[(q->first + i) & q->limit];
}

void Qfree(Queue *q)
{
	free(q);
}

