
/**
 * Copyright(c) 2017-2-23 Shangwen Wu	
 *
 * socket底层实现接口相关头文件
 * 
 */
#ifndef __SYS_SOCKETVAR_H__
#define __SYS_SOCKETVAR_H__

struct file;
struct proc;
struct mbuf;
struct uio;
struct sockaddr;	//defined in socket.h

#define SB_MAX		(256*1024)

/* socket buf标识位 */
#define SBF_LOCK			0x0001			/* socket buf 被锁定 */
#define SBF_WAIT			0x0002			/* 有进程被阻塞等待buf锁被释放 */
#define SBF_WAITFORDATA		0x0004			/* 有进程被阻塞等待buf有新的数据到来 */

/* socket状态 */
#define SS_PRIV				0x0001			/* 广播、RAW优先？？？ */
#define SS_ISCONNECTED		0x0002			/* socket已链接 */
#define SS_CANTSENDMORE		0x0004			/* socket不能发送更多数据 */
#define SS_ISCONFIRMING		0x0008			/* 正在确认是否进行连接 */
#define SS_NBIO				0x0010			/* 非阻塞 */
#define SS_CANTRCVMORE		0x0020			/* socket不能接收更多数据 */
#define SS_RCVATMARK		0x0040			/* 用于表示当前第一个数据是否为紧急指针位置 */

struct socket {
	ushort soc_stat;
	ushort soc_options;				/* socket相关选项 */
	ushort soc_error;				/* 错误号 */
	ulong soc_oobmark;				/* 紧急指针 */
	struct protocol *soc_proto;		/* 协议结构体 */
	void *soc_pcb;					/* 协议控制块 */
	/* socket 缓冲区的数据使用情况以及缓冲区大小阀值 */
	struct sockbuf {
		ushort sb_flags;			/* 见上述定义 */
		ulong sb_cc;				/* 当前buf有效的数据字节数 */	
		ulong sb_hiwat;				/* 缓冲区的上下阀值，当空闲空间大小位于该阀值之外时，操作将被阻塞 */
		ulong sb_lowat;
		ulong sb_mbcnt;				/* 实际使用的mbuf占用的空间字节数，包括EXT类型指向的额外空间 */
		ulong sb_mbmax;				/* 最大使用的mbuf空间字节数 */
		struct mbuf *sb_mb;			/* 指向保存数据的mbuf链表 */
	} soc_sndsb, soc_rcvsb;
};

/* 获取当前socket buf剩余可用空间 */
#define sbspace(sb)			min(((sb)->sb_hiwat - (sb)->sb_cc), ((sb)->sb_mbmax - (sb)->sb_mbcnt))

/* 根据释放或者分配一个mbuf，调整socket buf有效数据字节个数 */
#define sballoc(sb, m) \
do { \
	(sb)->sb_cc += (m)->m_len; \
	(sb)->sb_mbcnt += MSIZE; \
	if((m)->m_flags & MF_EXT) \
		(sb)->sb_mbcnt += (m)->m_ext.me_size; \
} while(0)

#define sbfree(sb, m) \
do { \
	(sb)->sb_cc -= (m)->m_len; \
	(sb)->sb_mbcnt -= MSIZE; \
	if((m)->m_flags & MF_EXT) \
		(sb)->sb_mbcnt -= (m)->m_ext.me_size; \
} while(0)

extern int soreceive(struct socket *so, struct mbuf **addrp, struct uio *uio, struct mbuf **mp, struct mbuf **controlp, int *flagsp);
extern int sosend(struct socket *so, struct mbuf *addr, struct uio *uio, struct mbuf *top, struct mbuf *control, int flags); 
extern int sockargs(struct mbuf **mp, caddr_t buf, ulong len, ushort type);
extern int getsock(struct file **fpp, struct proc *p, int fd);
extern int sendit(int, struct msghdr *, int, register_t *, struct proc *);
extern int recvit(int, struct msghdr *, caddr_t, register_t *, struct proc *);
extern int socreate(struct socket **, int, int, int);		//defined in uipc_socket.c
extern void sofree(struct socket *so);
extern int soclose(struct socket *so);
extern int sobind(struct socket *so, struct mbuf *addr);
extern int soreserve(struct socket *so, ulong sndsz, ulong rcvsz);
extern int sbreserve(struct sockbuf *sb, ulong sz);
extern void sbrelease(struct sockbuf *sb);
extern int sblock(struct sockbuf *sb, int flags);
extern void sbunlock(struct sockbuf *sb);
extern int sbwait(struct sockbuf *sb);
extern void sowakeup(struct socket *so, struct sockbuf *sb);
extern void sbdroprecord(struct sockbuf *sb);
extern int sbappendaddr(struct sockbuf *, struct sockaddr *, struct mbuf *, struct mbuf *);

#endif //__SYS_SOCKETVAR_H__
