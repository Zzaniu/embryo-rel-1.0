
/**
 * Copyright(c) 2017-2-21 PMON, Shangwen Wu	
 *
 * 系统调用函数声明头文件
 * 
 */
#ifndef __SYS_SYSCALL_H__
#define __SYS_SYSCALL_H__

struct proc;

/* 当前BIOS下所有系统调用底层实现函数原型声明 */
//defined in uipc_syscall.c
extern int sys_socket(struct proc *p, void *, register_t *);
extern int sys_bind(struct proc *p, void *args, register_t *retval);
extern int sys_sendto(struct proc *p, void *, register_t *);
extern int sys_recvfrom(struct proc *p, void *, register_t *);
//defined in sys_generic.c
extern int sys_ioctl(struct proc *p, void *, register_t *);
//defined in kern_filedesc.c
extern int sys_close(struct proc *p, void *args, register_t *retval);

#endif	//__SYS_SYSCALL_H__
