
/**
 * Copyright(c) 2017-2-21 PMON, Shangwen Wu	
 *
 * 系统调用参数定义相关头文件
 * 
 */
#ifndef __SYS_SYSCALLARG_H__
#define __SYS_SYSCALLARG_H__

/* 系统调用底层实现函数参数定义
 * 以下宏定义要求实际存储的参数类型大小必须要小于register_t，否则将引起底层实现函数解析参数时出现参数空间溢出
 * 的情况；datum字段主要用于解析参数的一方识别参数的类型，regt表示syscall给每个参数分配固定物理空间为4字节
 */
#define syscallarg(t)	union {t datum; register_t regt;}

#define SCARG(a, m)		((typeof((a)->m.datum))((a)->m.regt))
#define SCARGL(a, m)	((a)->m.regt)

struct sys_socket_args {
	syscallarg(int) domain;
	syscallarg(int) type;
	syscallarg(int) protocol;
};

struct sys_bind_args {
	syscallarg(int) sfd;
	syscallarg(const struct sockaddr *) laddr;
	syscallarg(socklen_t) laddrlen;
};

struct sys_sendto_args {
	syscallarg(int) sfd;
	syscallarg(const void *) buf;
	syscallarg(size_t) len;
	syscallarg(int) flags;
	syscallarg(const struct sockaddr *) to;
	syscallarg(socklen_t) tolen;
};

struct sys_recvfrom_args {
	syscallarg(int) sfd;
	syscallarg(void *) buf;
	syscallarg(size_t) len;
	syscallarg(int) flags;
	syscallarg(struct sockaddr *) from;
	syscallarg(socklen_t *) fromlen;
};

struct sys_ioctl_args {
	syscallarg(int) fd;
	syscallarg(unsigned long) cmd;
	syscallarg(void *) arg;
};

struct sys_close_args {
	syscallarg(int) fd;
};

#endif	//__SYS_SYSCALLARG_H__
