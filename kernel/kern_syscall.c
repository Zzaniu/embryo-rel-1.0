
/**
 * Copyright(c) 2017-2-21 Shangwen Wu 
 *
 * 这个文件定义了当前使用的所有系统调用函数
 * 
 */

#include <common.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/system.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/syscall.h>

#define SYSCALL(name)	sys_##name						//底层实现函数名在底层调用接口名前加上“sys_”前缀
#define MAXARGS			6								//底层接口实现函数最大参数个数

/* 底层实现接口函数框架, 第一个参数指向要传递的参数，第二指向底层函数返回值，函数返回执行结果 */
typedef int (syscall_t)(struct proc *p, void *, register_t *);

/**
 * 上层接口和底层实现之间的过渡函数
 */
static int gensyscall(syscall_t *func, long a1, va_list ap, int nargs)
{
	int err, i;
	struct args {register_t a[MAXARGS];}	args;
	register_t retval[2];							//为何要多出一个空间
	struct proc *p = curproc;

	args.a[0] = a1;
	for(i = 1; i < nargs; ++i)						//这里要求参数个数不能多于6个
		args.a[i] = va_arg(ap, long);				//这里要求所有的系统调用参数类型长度为long

	while(1) {
		retval[0] = 0;
		err = (*func)(p, &args, retval);
		if(err != ERESTART) {
			if(err) {
				errno = err;						//底层出错时，返回-1，并且设置全局errno
				return -1;
			}
			return retval[0];
		}											//返回ERESTART将重新进行系统调用
	}
}

/**
 * 描述：上层系统调用函数接口定义，上层调用接口框架为“int func(long, ...)”
 * 参数：pub，上层接口可见接口名；pri，底层接口名，对上层不可见；nargs，参数个数
 * 		 一般而言pub和pri相同，nargs参数个数由设计者提供
 */
#define syscall(pub, pri, nargs)						\
extern int pub(long a1, ...);							\
int pub(long a1, ...)									\
{														\
	int res;											\
	va_list ap;											\
														\
	va_start(ap, a1);									\
	res = gensyscall(SYSCALL(pri), a1, ap, nargs);		\
	va_end(ap);											\
														\
	return res;											\
}

/* 当前BIOS使用的系统调用 */
syscall(socket, socket, 3)		//int socket(int, int, int)
syscall(bind, bind, 3)			//int bind(int, const struct sockaddr *, socklen_t)
syscall(sendto, sendto, 6)		//ssize_t sendto(int,const void *,size_t,int,const struct socketaddr *,socklen_t)
syscall(recvfrom, recvfrom, 6)	//ssize_t recvfrom(int,void *,size_t,int,struct socketaddr *,socklen_t *)
syscall(soc_ioctl, ioctl, 3)	//int soc_ioctl(int, unsigned long, void *) 
syscall(soc_close, close, 1)	//int soc_close(int)

