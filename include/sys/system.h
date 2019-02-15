
/**
 * Copyright(c) 2017-3-6 Shangwen Wu	
 *
 * 系统相关头文件
 * 
 */
#ifndef __SYS_SYSTEM_H__
#define __SYS_SYSTEM_H__

#include <stdarg.h>

struct proc;							//defined in proc.h
struct list_head;						//defined list.h
typedef void (*spoll_func_t)(void *);

extern void paraminit(void);			//defined in sys_param.c 

extern void panic(char *msg);			//defined in kern_misc.c

extern int copyin(caddr_t buf, caddr_t user, size_t len); //defined in kern_misc.c

extern int copyout(caddr_t user, caddr_t buf, size_t len);//defined in kern_misc.c

extern void scandevs(void);				//defined in termio.c

extern void softnetpoll(void);			//defined in kern_misc.c

extern void schednetisr(int);			//defined in kern_misc.c

extern struct list_head *hashinit(ulong, ulong *);//defined in kern_misc.c

extern void *spoll_register(int lvl, spoll_func_t func, void *arg);//defined in kern_misc.c

extern void spoll_unregister(void *__del);//defined in kern_misc.c

extern void softpoll(void);	//defined in kern_misc.c

#endif //__SYS_SYSTEM_H__
