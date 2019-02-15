
/**
 * Copyright(c) 2016-12-30 Shangwen Wu 
 *
 * 系统级杂项函数实现
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <sys/syslog.h>
#include <sys/list.h>
#include <sys/system.h>
#include <sys/malloc.h>
#include <sys/uio.h>
#include <sys/netisr.h>
#include <mach/intr.h>

static int sysloglevel = 6;				//当前日志级别

static const char * const loglvlnames[] = {
	"EMERG",
	"ALERT",
	"CRIT",
	"ERROR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG",
};

void loginit(void)
{
	sysloglevel = 8;
}

/**
 * 描述：系统日志函数
 */
int log(int lvl, const char *fmt, ...)
{
	int n, _lvl;
	va_list ap;

	_lvl = lvl & LOG_LVLMASK;
	if(sysloglevel < _lvl)
		return 0;

	printf("%s: ", loglvlnames[_lvl]);
	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);

	return n;
}

/**
 * 描述：系统崩溃函数，该函数将陷入死循环
 */
void panic(char *msg)
{
	printf("System panic '%s'\n", msg);	
	while(1)
		;
}

/**
 * 描述用户层数据与系统层数据拷贝
 */
int copyin(caddr_t buf, caddr_t user, size_t len)
{
	memcpy(buf, user, len);
	return 0;
}

int copyout(caddr_t user, caddr_t buf, size_t len)
{
	memcpy(user, buf, len);
	return 0;
}

/**
 * 描述：uio与地址之间的数据移动
 * 		调用程序必须保证len与uio中有效数据之间的大小关系，以及uio内部信息的一致性
 *
 */
int uiomove(caddr_t cp, ulong len, struct uio *uio)
{
	ulong cnt;
	struct iovec *iov;
	
	while(len && uio->uio_resid && uio->uio_iovcnt) {
		iov = uio->uio_iov;
		if(!iov->iov_len) {
			++uio->uio_iov;
			--uio->uio_iovcnt;
			continue;
		}	
		cnt = min(len, iov->iov_len);
		if(UIO_READ == uio->uio_rw)
			copyout(iov->iov_base, cp, cnt);
		else
			copyin(cp, iov->iov_base, cnt);
		len -= cnt;
		cp += cnt;
		uio->uio_resid -= cnt; 
		iov->iov_base += cnt;
		iov->iov_len -= cnt;
	}

	return 0;
}

/** 
 * BIOS使用虚拟中断机制处理所有外设IO，实际上当前并未使用任何硬件中断，
 * 而是通过不断的轮询软件中断状态来模拟硬件中断的效果，所有中断状态的
 * 查询以及中端处理程序（ISR）均在splx函数中被调用
 */

/**
 * 网络中断状态寄存器
 */
static volatile int netisr = 0;

/**
 * 当前中断屏蔽级别，当外设对应的中断级别小于等于当前级别时，中断将被屏蔽
 * 以下是各个外设对应的中断级别：
 * 最高---7，clock---7，tty---5，bio---4，imp---7，net---3，softclock---1
 * softnet---1，不屏蔽任何中断---0
 */
static volatile int curspl = 0;

/* 以下函数将屏蔽指定外设以及更低级别的中断，并返回原来的屏蔽级别 */
int splsoftclock(void)
{
	int old = curspl;

	if(curspl < 1)
		curspl = 1;
	
	return old;
}

int splsoftnet(void)
{
	int old = curspl;

	if(curspl < 1)
		curspl = 1;
	
	return old;
}

int splnet(void)
{
	int old = curspl;

	if(curspl < 3)
		curspl = 3;
	
	return old;
}

int splbio(void)
{
	int old = curspl;

	if(curspl < 4)
		curspl = 4;
	
	return old;
}

int spltty(void)
{
	int old = curspl;

	if(curspl < 5)
		curspl = 5;
	
	return old;
}

int splclock(void)
{
	int old = curspl;

	if(curspl < 7)
		curspl = 7;
	
	return old;
}

int splimp(void)
{
	int old = curspl;

	if(curspl < 7)
		curspl = 7;
	
	return old;
}

int splhigh(void)
{
	int old = curspl;

	curspl = 7;
	
	return old;
}

/* 打开所有中断，并查询当前中断状态 */
int spl0(void)
{
	int old = curspl;
	
	splx(0);
	
	return old;
}

/* 
 * 将设置当前中断到指定级别，并查询未决中断，并调用对应ISR，
 * 该函数常用于恢复临时提高的中断级别，并且充当轮询中断的触
 * 发函数
 */
void splx(int new)
{
	/* 当新设置的中断级别低于当前级别时，将查询未决中断 */
	if(new < curspl) {
		if(new < 7) {
			curspl = 7;	//屏蔽当前中断
		}
		if(new < 3) {
			curspl = 3;	//屏蔽当前中断
		}
		if(new < 1 && netisr) {
			curspl = 1;	//屏蔽当前中断
			softnetpoll();
		}
	}

	curspl = new;
	if(0 == curspl) {
		/* 最低级别的软件轮询 */
		softpoll();
	}
}

#define DONETISR(isr, handler) \
if(netisr & (1 << (isr))){ \
	netisr &= ~(1 << (isr)); \
	handler(); \
}

/**
 * softnet网络状态轮询函数，用于协议栈数据包传递
 */
void softnetpoll(void)
{
	extern void ipintr(void); //defined in ip_input.c
	extern void arpintr(void); //defined in arp.c

	/* 需要用到中断的外设 */
	DONETISR(NETISR_IP, ipintr);
	DONETISR(NETISR_ARP, arpintr);
}

void schednetisr(int isr)
{ 
	netisr |= 1 << isr;
}

/* 软件轮询，运行在spl为0的级别 */
struct spoll_handler;

struct spoll_handler {
	int level;			//reserved		
	spoll_func_t func;
	void *arg;
	struct list_head list;
};

static LIST_HEAD(spoll_list);

/**
 * 描述：注册一个轮询函数
 */
void *spoll_register(int lvl, spoll_func_t func, void *arg)
{
	struct spoll_handler *new;	

	if(NULL == (new = (struct spoll_handler *)kmem_malloc(sizeof(struct spoll_handler))))
		return NULL;

	new->level = lvl;
	new->func = func;
	new->arg = arg;
	/* 插入链表头 */
	list_add(&new->list, &spoll_list);

	return new;
}

/**
 * 描述：注销一个轮询函数
 */
void spoll_unregister(void *__del)
{
	struct spoll_handler *del = (struct spoll_handler *)__del;

	if(!del)
		return;

	list_del(&del->list);
	kmem_free(del);
}

/**
 * 描述：软件级别的轮询函数
 */
void softpoll(void)
{
	struct spoll_handler *cur;

	if(curspl != 0)	
		return;

	curspl = 7;

	list_for_each_entry(cur, &spoll_list, list) {
		/* 这里需要添加优先级代码 */
		if(cur->func)
			(*cur->func)(cur->arg);
	}

	curspl = 0;
}

/**
 * 初始化一个hash表
 */
struct list_head *hashinit(ulong elements, ulong *hashmask)
{
	struct list_head *table;
	ulong i, hashsize;
	
	if(!elements)
		panic("hashsize is zero");

	/* hashsize为不大于elements的2的幂 */
	for(hashsize = 1; hashsize <= elements; hashsize <<= 1)
		;
	hashsize >>= 1;

	if(NULL == (table = (struct list_head *)kmem_malloc(hashsize * sizeof(struct list_head))))
		panic("hashtable is null");

	for(i = 0; i < hashsize; ++i)
		INIT_LIST_HEAD(table + i);

	*hashmask = hashsize - 1;

	return table;
}
