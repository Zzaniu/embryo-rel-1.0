
/**
 * Copyright(c) 2017-2-23 Shangwen Wu 
 *
 * socket底层实现接口处理 
 * 
 */
#include <common.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/sync.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/protocol.h>
#include <sys/domain.h>
#include <sys/socketvar.h>
#include <mach/intr.h>

extern ulong max_hdr;					//defined in uipc_domain.c	

static ulong sb_max = SB_MAX;			/* 最大socket buf大小 */

/**
 * 描述：bind本地地址
 */	
int sobind(struct socket *so, struct mbuf *addr)
{
	return so->soc_proto->pr_usrreq(so, PRU_BIND, NULL, addr, NULL);
}

/**
 * 描述：接收网络数据，
 * 参数：该函数可以被系统曾调用也可被应用层调用，由系统曾调用时，mp不为NULL，并且uio的reside指定需要接收
 * 		 数据的大小，该函数将数据拷贝或者直接赋值一个mbuf链到mp参数；由应用曾调用时，该按数将数据拷贝到uio
 * 		 指向的用户空间中。uio任何情况下不能为NULL
 * 注意：该函数将会分配用于保存接收到的数据和控制信息的mbuf，需要在上层调用中，将这些存储空间释放
 */
int soreceive(struct socket *so, struct mbuf **addrp, struct uio *uio, struct mbuf **mp0, struct mbuf **controlp, int *flagsp)
{
	int type, s, uio_err = 0, err = 0, flags = 0;
	ulong len, resid, offset, moff;
	size_t ori_resid = uio->uio_resid;
	struct mbuf **mp, *m, *nextpkt;
	
	mp = mp0;
	if(addrp)
		*addrp = NULL;
	if(mp)
		*mp = NULL;
	if(controlp)
		*controlp = NULL;
	
	if(flagsp)
		flags = *flagsp & ~(MSG_EOR);
	if(so->soc_stat & SS_NBIO) 
		flags |= MSG_DONTWAIT;
	
	/* 处理OOB消息 */
	if(flags & MSG_OOB) {
		m = mbuf_get(MT_DATA);
		if(!m) 
			return ENOMEM;
		/* 协议曾函数将OOB数据组成链表到传入的起始mbuf节点 */
		if(err = (*so->soc_proto->pr_usrreq)(so, PRU_RCVOOB, m, (struct mbuf *)(long)(flags & MSG_PEEK), NULL)) 
			goto bad;
		do {
			len = min(uio->uio_resid, m->m_len);
			err = uiomove(mtod(m, caddr_t), len, uio);
			if(!err) 
				m = mbuf_free(m);
		} while(!err && uio->uio_resid && m);

bad:
		if(m)
			mbuf_freem(m);
		return err;
	}

	/* 缓冲区空间通知 */
	if(so->soc_stat & SS_ISCONFIRMING && uio->uio_resid)
		 (*so->soc_proto->pr_usrreq)(so, PRU_RCVD, m, (struct mbuf *)(long)(flags & MSG_PEEK), NULL);
	
	/* 轮询socket中的接收buf */	
waitrecv:
	if(err = sblock(&so->soc_rcvsb, flags))
		return err;

	s = splsoftnet();
	
	/**
 	 * 满足以下情况，当前操作将会阻塞
 	 * 1）接收缓冲区为空，或
 	 * 2）a，调用者没有设置非阻塞标识，
 	 * 	  b，且当前socket接收缓冲区的实际字节数小于调用者要求的字节数，
 	 * 	  c，且实际字节数小于接收缓冲区设置的下边界，或调用者设置了MSG_WAITALL且调用者要求的字节数没有超过缓冲区设置的上边界
 	 * 	  d，接收队列中没有下一个网络包
 	 * 	  e，当前网络包使用的协议为原子数据类型
 	 */
	m = so->soc_rcvsb.sb_mb;
	if(m == NULL ||
		((!(flags & MSG_DONTWAIT) && !(so->soc_stat & SS_NBIO)) && 
		so->soc_rcvsb.sb_cc < uio->uio_resid &&
		(so->soc_rcvsb.sb_cc < so->soc_rcvsb.sb_lowat || (flags & MSG_WAITALL && uio->uio_resid <= so->soc_rcvsb.sb_hiwat)) &&
		m->m_nextpkt == NULL && (so->soc_proto->pr_flags & PRF_ATOMIC)
		)) {
		/* 以下情况将跳出阻塞 */
		if(so->soc_error) {
			if(m)
				goto dontblock;
			err = so->soc_error;
			if(!(flags & MSG_PEEK))
				so->soc_error = 0;
			goto release;
		}

		if(so->soc_stat & SS_CANTSENDMORE) {
			if(m)
				goto dontblock;
			else
				goto release;
		}
	
		for(;m;m = m->m_next)
			if(((MT_OOBDATA == m->m_type) || (m->m_flags & MF_EOR))) {
				m = so->soc_rcvsb.sb_mb;
				goto dontblock;
			}
		/* 注意：m指针已经不再指向第一个mbuf节点 */
		if((so->soc_proto->pr_flags & PRF_CONNREQUIRED) && 
			!(so->soc_stat & (SS_ISCONFIRMING | SS_ISCONNECTED))) {
			err = ENOTCONN;
			goto release;
		}
		/* 当m为空，但是用户设置了非阻塞IO时将会退出操作 */
		if((so->soc_stat & SS_NBIO) || (flags & MSG_DONTWAIT)) {
			err = EWOULDBLOCK;
			goto release;
		}
		
		if(!uio->uio_resid && !controlp)
			goto release;

		/* 阻塞当前线程，直到接收缓冲区接收到数据唤醒当前线程 */
		splx(s);
		sbunlock(&so->soc_rcvsb);
		if(err = sbwait(&so->soc_rcvsb))
			return err;
		goto waitrecv;
	}

dontblock:	
	nextpkt = m->m_nextpkt;

	/* 如果协议要求报文包含地址信息，那么每个mbuf链的第一个节点将保存地址信息 */
	if(so->soc_proto->pr_flags & PRF_ADDR) {
		if(flags & MSG_PEEK) { //MSG_PEEK将不会引起接收队列的出队
			if(addrp)
				*addrp = mbuf_copy(m, 0, m->m_len);
			m = m->m_next;
		} else {
			/* 将当前mbuf从接收队列中出队 */
			sbfree(&so->soc_rcvsb, m);
			if(addrp) {
				so->soc_rcvsb.sb_mb = m->m_next;
				*addrp = m;
				m->m_next = NULL;
			} else {
				so->soc_rcvsb.sb_mb = mbuf_free(m);
			}
			m = so->soc_rcvsb.sb_mb;
		}
		ori_resid = 0;	//防止程序再一次被阻塞等待接收数据
	}

	/* 拷贝控制信息 */
	while(m && (MT_CONTROL == m->m_type) && !err) {
		if(flags & MSG_PEEK) { //MSG_PEEK将不会引起接收队列的出队
			if(controlp)
				*controlp = mbuf_copy(m, 0, m->m_len);
			m = m->m_next;
		} else {
			/* 将当前mbuf从接收队列中出队 */
			sbfree(&so->soc_rcvsb, m);
			if(controlp) {
				if(so->soc_proto->pr_domain->dom_externalize && 
					mtod(m, struct cmsghdr *)->cmsg_type == SCM_RIGHTS)
					err = (*so->soc_proto->pr_domain->dom_externalize)(m); 
				so->soc_rcvsb.sb_mb = m->m_next;
				*controlp = m;
				m->m_next = NULL;
			} else {
				so->soc_rcvsb.sb_mb = mbuf_free(m);
			}
			m = so->soc_rcvsb.sb_mb;
		}
		if(controlp) {
			if(*controlp)
				controlp = &(*controlp)->m_next;
			ori_resid = 0;	//防止程序再一次被阻塞等待接收数据
		}
	}

	if(m) {
		if(!(flags & MSG_PEEK))
			m->m_nextpkt = nextpkt;
		type = m->m_type;
		if(MT_OOBDATA == type)
			flags |= MSG_OOB;
		else if(m->m_flags & MF_BCAST)
			flags |= MSG_BCAST;
		else if(m->m_flags & MF_MCAST)
			flags |= MSG_MCAST;
	}

	/* 拷贝数据 */

	/**
 	 * offet为相对整个包长的偏移，用于记录当前拷贝位置到oobmark之间的距离，
 	 * moff为当前拷贝位置相对与mbuf数据基地址之间的偏移，这两个变量均用于
 	 * MSG_PEEK标志被设置的情况下，该标志未设置时，moff与offset始终为0
 	 */
	offset = moff = 0;

	while(m && uio->uio_resid > 0 && !err) {
		/* 拷贝连续的OOB数据或一般数据 */
		if(MT_OOBDATA == type) {
			if(m->m_type != MT_OOBDATA)
				break;
		} else {
			if(MT_OOBDATA == m->m_type)
				break;
		}
		so->soc_stat &= ~SS_RCVATMARK;
		
		len = uio->uio_resid;
		/* len = min(resid, oobmark - offset, mlen - moff) */
		if(so->soc_oobmark && len > so->soc_oobmark - offset)
			len = so->soc_oobmark - offset;
		if(len > m->m_len - moff)
			len = m->m_len - moff;

		/* 需要将数据拷贝到uio，同时更新uio_resid */
		if(!mp && !uio_err) {
			resid = uio->uio_resid;
			splx(s);
			uio_err = uiomove(mtod(m, caddr_t) + moff, len, uio);
			s = splsoftnet();
			if(uio_err)
				uio->uio_resid = resid - len;
		} else
			uio->uio_resid -= len;
		/* 取出的数据长度刚好等于当前mbuf的数据长度 */
		if(len == m->m_len - moff) {
			if(m->m_flags & MF_EOR)
				flags |= MSG_EOR;
			if(!(flags & MSG_PEEK)) {
				nextpkt = m->m_nextpkt;
				sbfree(&so->soc_rcvsb, m);
				if(mp) {
					*mp = m;
					mp = &(*mp)->m_next;
					so->soc_rcvsb.sb_mb = m = m->m_next;
					*mp = NULL;
				} else {
					so->soc_rcvsb.sb_mb = mbuf_free(m);
					m = so->soc_rcvsb.sb_mb;
				}
				if(m) 
					m->m_nextpkt = nextpkt;
			} else {
				moff = 0;
				m = m->m_next;
			}
		} else {
			if(!(flags & MSG_PEEK)) {
				if(mp)
					*mp = mbuf_copy(m, 0, len);
				m->m_data += len;
				m->m_len -= len;
				so->soc_rcvsb.sb_cc -= len;
			} else {
				moff += len;
			}
		}
		/* 调整oobmark位置，并判断紧急指针是否到达数据起始位置 */
		if(so->soc_oobmark) {
			if(!(flags & MSG_PEEK)) {
				so->soc_oobmark -= len;
				if(!so->soc_oobmark) {
					so->soc_stat |= SS_RCVATMARK;
					break;
				}
			} else {
				offset += len;
				if(offset == so->soc_oobmark) 
					break;	
			}
		}
		if(flags & MSG_EOR)
			break;

		/* 满足以下条件时，将阻塞等待接收更多数据 */
		while((flags & MSG_WAITALL) && !m && uio->uio_resid > 0 && 
			!(so->soc_proto->pr_flags & PRF_ATOMIC) && !nextpkt) {
			if(so->soc_error || (so->soc_stat & SS_CANTRCVMORE))
				break;
			splx(s);
			err = sbwait(&so->soc_rcvsb);
			s = splsoftnet();
			if(err) {
				splx(s);
				sbunlock(&so->soc_rcvsb);	//bad codes
				//return err; 
				return 0;					//这里正常返回0 
			}
			if((m = so->soc_rcvsb.sb_mb) != NULL)
				nextpkt = m->m_nextpkt;
		}
	}

	/* 如果当前网络包协议为原子的，那么将丢弃当前mbuf数据链 */
	if(m && (so->soc_proto->pr_flags & PRF_ATOMIC)) {
		flags |= MSG_TRUNC;
		if(!(flags & MSG_PEEK))
			sbdroprecord(&so->soc_rcvsb);
	}

	if(!(flags & MSG_PEEK)) {
		if(!m)
			so->soc_rcvsb.sb_mb = nextpkt;
		if((so->soc_proto->pr_flags & PRF_WANTRCVD) && so->soc_pcb)
			 (*so->soc_proto->pr_usrreq)(so, PRU_RCVD, m, (struct mbuf *)(long)(flags & MSG_PEEK), NULL);
	}

	if(ori_resid && ori_resid == uio->uio_resid &&
		!(flags & MSG_EOR) && !(so->soc_stat & SS_CANTRCVMORE)) {
		splx(s);
		sbunlock(&so->soc_rcvsb);
		goto waitrecv;
	}

	if(uio_err)
		err = uio_err;
	
	if(flagsp)
		*flagsp |= flags;

release:
	sbunlock(&so->soc_rcvsb);
	splx(s);
	return err;
}

/**
 * 描述：发送网络数据，注意传入的top参数和uio至少要有一个不为NULL，当uio为NULL时，将以原子方式直接发送top中的数据；
 * 		 当top为NULL时，将根据发送选择的协议决定是否原子发送，如果采用原子发送，函数将一次性发送一个mbuf链，链表头采
 * 		 用pkt的mbuf，其他为标准格式的mbuf，如果协议不要求原子发送，每次发送数据的一部分，每部分都是一个pkt格式的mbuf;
 * 		 当top与uio均不为空时，发送的数据存储在uio中，但是仍将这些uio中的数据一次性发送出去，每个数据以标准格式组成链的
 * 		 形式发送，而不存在pkt格式的mbuf
 * 注意：该函数将会释放top以及可能释放control所占用的空间，当函数在执行usrreq函数之前出错时，将释放control变量，最终
 * 		 释放与否由usrreq函数决定
 */
int sosend(struct socket *so, struct mbuf *addr, struct uio *uio, struct mbuf *top, struct mbuf *control, int flags)
{
	int err = 0, s;
	quad_t resid;				//注意：这里使用有符号的64位来装size_t类型，以判断size_t的变量是否超过ssize_t最大值
	/* 当top不为NULL或协议要求atomic发送时，要求所有数据必须一次性发送完 */
	int atomic = (so->soc_proto->pr_flags & PRF_ATOMIC) || top; 
	ulong space, mlen, len, clen = 0;		//mlen为分配空间长度，len为当前mbuf实际数据有效长度，space为sockbuf的空闲空间，clen为control信息长度
	struct mbuf *m = NULL;
	struct mbuf **mp;
	int dontroute = 0;
	
	if(!uio && !top)
		return EINVAL;

	if(uio)
		resid = uio->uio_resid;
	else
		resid = top->m_pkthdr.mp_len;
	/* bad code，因为sendto函数返回有符号类型，因此实际上传入的len最大值只能时有符号数的最大值 */
	if(resid < 0 || (SOCK_STREAM == so->soc_proto->pr_type && (flags & MSG_EOR)))
		return EINVAL;

	if(control)
		clen = control->m_len;

	if((atomic && resid > so->soc_sndsb.sb_hiwat) || clen > so->soc_sndsb.sb_hiwat)
		return EMSGSIZE;

	dontroute = (flags & MSG_DONTROUTE) && !(so->soc_options & SO_DONTROUTE) \
				&& (so->soc_proto->pr_flags & PRF_ATOMIC);

#define gotoerr(e)	do {err = e; splx(s); goto release;} while(0)

restart:

	if(err = sblock(&so->soc_sndsb, flags)) 
		goto out;		
	do {
		s = splsoftnet();
		if(so->soc_stat & SS_CANTSENDMORE)
			gotoerr(EPIPE);	
		if(err = so->soc_error)
			gotoerr(err);
		if(!(so->soc_stat & SS_ISCONNECTED)) {
			if(so->soc_proto->pr_flags & PRF_CONNREQUIRED) {
				if(!resid && !(so->soc_stat & SS_ISCONFIRMING))
					gotoerr(ENOTCONN);
			} else if(!addr)
				gotoerr(EDESTADDRREQ);
		}
		space = sbspace(&so->soc_sndsb);

		/* 当空间不够时，将阻塞 */	
		//printf("space: %d, hi: %d, lo: %d, cc: %d, maxm: %d, maxc: %d\n", space, so->soc_sndsb.sb_hiwat, so->soc_sndsb.sb_lowat, so->soc_sndsb.sb_cc, so->soc_sndsb.sb_mbmax,so->soc_sndsb.sb_mbcnt);	
		if(space < resid + clen && uio && (atomic || space < so->soc_sndsb.sb_lowat || space < clen)) {
			if(so->soc_stat & SS_NBIO)
				gotoerr(EWOULDBLOCK);
			sbunlock(&so->soc_sndsb);
			err = sbwait(&so->soc_sndsb);
			splx(s);
			if(err)
				goto out;
			goto restart;
		}
		splx(s);
		mp = &top;
		space -= clen;

		do {
			/* uio为空时，直接发送top中的数据 */
			if(NULL == uio) {
				resid = 0;
				if(flags & MSG_EOR)
					top->m_flags |= MF_EOR;
			} else {
				/* 原子发送时，所有数据将组成一条mbuf链，并将这些数据一次性通过usrreq发送 */
				do {
					if(!top) {
						m = mbuf_getpkt(MT_DATA);
						if(!m) {
							err = ENOBUFS;
							goto release;
						}
						mlen = MHLEN;
						m->m_pkthdr.mp_len = 0;
						m->m_pkthdr.mp_recvif = NULL;
					} else {
						m = mbuf_get(MT_DATA);
						if(!m) {
							err = ENOBUFS;
							goto release;
						}
						mlen = MLEN;
					}
					/* 当需要发送的数据较大时，并且空间足够，将分配外部存储区 */
					if(resid >= MINMCLSIZE && space >= MCLBYTES) {
						if(!mbuf_getext(m))		/* 是否分配外部存储区成功 */
							goto nopages;
						mlen = MCLBYTES;
						/* 如果当前mbuf为原子发送的第一个节点，并且传入的top不包含数据时，将预留一个最大协议头的长度 */
						if(atomic && !top) {
							len = min(mlen - max_hdr, resid);
							m->m_data += max_hdr;
						} else 
							len = min(mlen, resid);
					} else {
nopages:
						len = min(min(mlen, space), resid);
						/* 如果当前mbuf为原子发送的第一个节点，并且传入的top不包含数据时，将数据基地址右对齐 */
						if(atomic && !top && len < mlen) 
							MH_ALIGN(m, len);
					}
					space -= len;

					err = uiomove(mtod(m, caddr_t), len, uio);
					m->m_len = len;
					resid = uio->uio_resid;
					*mp = m;
					top->m_pkthdr.mp_len += len;
					if(err)
						goto release;
					mp = &m->m_next;

					//printf("pktlen: %d, resid: %lld, space: %d\n", top->m_pkthdr.mp_len, resid, space);
					if(resid <= 0) {
						if(flags & MSG_EOR)
							top->m_flags |= MF_EOR;
						break;
					}
				} while (space > 0 && atomic);
			}
			if(dontroute)
				so->soc_options |= SO_DONTROUTE;
			s = splsoftnet();
			/* 注意调用该函数时，top的内存将被释放 */
			err = (*so->soc_proto->pr_usrreq)(so, PRU_SEND, top, addr, control);
			splx(s);			
			if(dontroute)
				so->soc_options &= ~SO_DONTROUTE;
			clen = 0;
			control = NULL;
			top = NULL;
			if(err)
				goto release;
			mp = &top;

		} while (resid && space > 0);

	} while(resid);

release:
	sbunlock(&so->soc_sndsb);

out:
	if(top)
		mbuf_freem(top);
	if(control)
		mbuf_freem(control);

	return err;
}

/**
 * 描述：创建一个sokcet结构并初始化某些字段
 */
int socreate(struct socket **spp, int domain, int type, int proto)
{
	int err;
	struct socket *sp;
	struct protocol *prop;

	if(proto)
		prop = profindbyproto(domain, type, proto);
	else 
		prop = profindbytype(domain, type);
	if(!prop || !prop->pr_usrreq) {
		return EPROTONOSUPPORT;
	}

	if(NULL == (sp = (struct socket *)kmem_malloc(sizeof(struct socket))))
		return ENOMEM;
	bzero(sp, sizeof(struct socket));
	
	sp->soc_stat = SS_PRIV;
	sp->soc_proto = prop;

	if(err = (*prop->pr_usrreq)(sp, PRU_ATTACH, NULL, (struct mbuf *)proto, NULL)) {
		sofree(sp);
		return err;
	}

	*spp = sp;

	return 0;
}

/**
 * 描述：关闭并释放一个socket
 */
int soclose(struct socket *so)
{
	int ret = 0, s = splsoftnet();

	/* 关闭协议相关资源 */
	if(so->soc_pcb)	
		ret = (*so->soc_proto->pr_usrreq)(so, PRU_DETACH, NULL, NULL, NULL);

	sofree(so);
	splx(s);

	return ret;
}

void sofree(struct socket *so)
{
	sbrelease(&so->soc_sndsb);	
	kmem_free(so);
}

int soreserve(struct socket *so, ulong sndsz, ulong rcvsz)
{
	if(sbreserve(&so->soc_sndsb, sndsz)) 
		return ENOBUFS;

	if(sbreserve(&so->soc_rcvsb, rcvsz))
		goto failed;
		
	if(!so->soc_sndsb.sb_lowat)
		so->soc_sndsb.sb_lowat = MCLBYTES;		/* 发送buf下阀值设置为一个mcl的大小 */
	if(!so->soc_rcvsb.sb_lowat)
		so->soc_rcvsb.sb_lowat = 1;				/* 发送buf下阀值设置为1 */
	if(so->soc_sndsb.sb_lowat > so->soc_sndsb.sb_hiwat)
		so->soc_sndsb.sb_lowat = so->soc_sndsb.sb_hiwat;

	return 0;

failed:
	sbrelease(&so->soc_sndsb);	
	return ENOBUFS;
}

int sbreserve(struct sockbuf *sb, ulong sz)
{
	if(!sz || sz > sb_max * MCLBYTES / (MSIZE + MCLBYTES))
		return -1;

	sb->sb_hiwat = sz;
	sb->sb_mbmax = min(sz * 2, sb_max);
	if(sb->sb_lowat > sb->sb_hiwat)
		sb->sb_lowat = sb->sb_hiwat;

	return 0;
}

void sbrelease(struct sockbuf *sb)
{
	sb->sb_hiwat = 0;
	sb->sb_lowat = 0;
}

/* 丢弃当前socket buf的第一个记录，将下一个记录改为当前socket buf记录 */
void sbdroprecord(struct sockbuf *sb)
{
	struct mbuf *m, *nm;

	m = sb->sb_mb;
	
	if(m) {
		sb->sb_mb = m->m_nextpkt;
		do {
			sbfree(sb, m);
			nm = mbuf_free(m);
		} while((m = nm) != NULL);
	}
}

/* 锁定sockbuf，一旦buf被其他线程锁定，当前线程将被阻塞 */
int sblock(struct sockbuf *sb, int flags)
{
	int err = 0, s;

	s = splsoftnet();

	if(sb->sb_flags & SBF_LOCK) {
		if(flags & MSG_DONTWAIT) {
			err = EWOULDBLOCK;
			goto out;
		}

		sb->sb_flags |= SBF_WAIT;			//表示当前有进程因为buf锁定而被阻塞，该标识应当被释放锁的进程清除

		do {
			splx(s);
			if(err = tsleep())
				return err;
			s = splsoftnet();
		} while(sb->sb_flags & SBF_LOCK); 
	}
	
	sb->sb_flags |= SBF_LOCK;

out:
	splx(s);
	return err;
}

/* 释放缓冲队列访问锁，并唤醒被阻塞的线程 */
void sbunlock(struct sockbuf *sb) 
{	
	int s = splsoftnet();
		
	sb->sb_flags &= ~SBF_LOCK;
	if(sb->sb_flags & SBF_WAIT) {
		sb->sb_flags &= ~SBF_WAIT;
		splx(s);
		wakeup();
		return;
	}
	splx(s);
}

/* 阻塞等待socket缓冲队列中的数据锁 */
int sbwait(struct sockbuf *sb)
{
	int s = splsoftnet();
	
	sb->sb_flags |= SBF_WAITFORDATA;	

	splx(s);

	return tsleep();
}

/* 与sbwait对应，用于释放缓冲队列的数据锁，并唤醒线程 */
void sowakeup(struct socket *so, struct sockbuf *sb)
{
	int s = splsoftnet();

	if(sb->sb_flags & SBF_WAITFORDATA) {
		sb->sb_flags &= ~SBF_WAITFORDATA;	
		splx(s);
		wakeup();
		goto out;
	}
	splx(s);

out:
	;//socke相关处理
}

/**
 * 描述：向socket缓冲队列添加一条记录，该记录从前到后依次包括地址、数据、控制部分信息
 * 返回：添加失败返回-1，否则返回0
 * 注意：该函数data中必须包含协议数据头信息
 */
int sbappendaddr(struct sockbuf *sb, struct sockaddr *addr, struct mbuf *data, struct mbuf *control)
{
	struct mbuf *m, *n;
	ulong space = addr->sa_len;//记录总长度
	
	/* data的第一个节点必须为pkthdr */
	if(data && !(data->m_flags & MF_PKTHDR))
		return -1;
	if(data)
		space += data->m_pkthdr.mp_len;
		
	for(n = control; n; n = n->m_next) {
		space += n->m_len;
		if(!n->m_next)				//指向最后一个control数据节点
			break;
	}

	/* socket缓冲队列没有足够的空间 */
	if(space > sbspace(sb))
		return -1;

	/* 地址节点必须能够保存在一个非ext类型的mbuf的容量之内 */
	if(addr->sa_len > MLEN)
		return -1;

	/* 分配一个节点保存地址信息 */
	if(!(m = mbuf_get(MT_SONAME)))
		return -1;
	m->m_len = addr->sa_len;
	bcopy((caddr_t)addr, mtod(m, caddr_t), addr->sa_len);

	/* 依次连接addr,control,data到一个记录链表中 */
	if(n)
		n->m_next = data;
	else 
		control = data;
	m->m_next = control;

	/* 调整sockbuf中的大小信息 */
	for(n = m; n; n = n->m_next)
		sballoc(sb, n);

	/* 将记录链表挂接到socket缓冲队列尾部 */
	if((n = sb->sb_mb) != NULL) {
		while(n->m_nextpkt)
			n = n->m_nextpkt;
		n->m_nextpkt = m;
	} else
		sb->sb_mb = m;		

	return 0;
}


