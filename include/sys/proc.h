

/**
 * Copyright(c) 2017-4-19 Shangwen Wu	
 *
 * EMBRYO进程相关头文件
 * 
 */

#ifndef __SYS_PROC_H__
#define __SYS_PROC_H__

#define NPROC	3							//当前系统最大允许的进程个数

struct filedesc;							//defined in kern_filedesc.h

struct proc {
	pid_t p_id;								//进程ID号
	char *p_name;							//进程名称
	struct filedesc *p_fd;					//进程的文件描述符
};

extern struct proc *curproc;				//defined in kern_proc.c

extern int procinit(void);				

#endif //__SYS_PROC_H__

