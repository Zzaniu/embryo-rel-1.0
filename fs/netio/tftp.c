
/**
 * Copyright(c) 2018-9-6 Shangwen Wu	
 *
 * tftp客户端相关实现 
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <sys/endian.h>					
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <url.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/list.h>
#include <sys/syslog.h>
#include <sys/socket.h>					//for socket API
#include <sys/fileio.h>
#include <netinet/in.h>
#include <arpa/tftp.h>
#include <arpa/inet.h>
#include <fs/file.h>
#include <fs/termio.h>
#include <fs/netio.h>
#include <fs/tftpfile.h>

/* tftp传输调试信息开关 */
#define TFTP_DEBUG	0

#if TFTP_DEBUG
static int tftptrace = 1;
#endif
static const char *trans_mode = "octet";//"ascii"		//default tftp trans mode

/* tftp 错误码与errno对应关系 */
static int tftperrmap[] = {
	EIO,	/* EUNDEF */
	ENOENT, /* ENOTFOUND */
	EACCES, /* EACCESS */
	ENOSPC, /* ENOSPACE */
	EIO, 	/* EBADOP */
	EINVAL,	/* EBADID */
	EEXIST,	/* EEXISTS */
	EACCES, /* ENOUSER */
};

#if TFTP_DEBUG
static void tracepacket(const char *s, struct tftp *tfp, int n)
{
	char *cp;
	ushort opcode = ntohs(tfp->tfp_opcode);
	static const char *opstr[] = {"null", "RRQ", "WRQ", "DATA", "ACK", "ERROR"};
	
	if(opcode < TFTP_OPCODE_RRQ || opcode > TFTP_OPCODE_ERR)
		printf("%s opcode %hu\n", s, opcode);
	else
		printf("%s %s", s, opstr[opcode]);
	
	switch(opcode) {
		case TFTP_OPCODE_RRQ:
		case TFTP_OPCODE_WRQ:
			cp = tfp->tfp_stuff; 
			printf(" <file=%s, mode=%s>\n", cp, cp + strlen(cp) + 1);
			break;
		case TFTP_OPCODE_DATA:
			printf(" <blk=%hu, bytes=%d>\n", ntohs(tfp->tfp_blk), n - TFTPMINSIZE);	
			break;
		case TFTP_OPCODE_ACK:
			printf(" <blk=%hu>\n", ntohs(tfp->tfp_blk));	
			break;
		case TFTP_OPCODE_ERR:
			printf(" <code=%hu, msg=%s>\n", ntohs(tfp->tfp_errcode), 
				tfp->tfp_msg[0] != '\0' ? (char *)tfp->tfp_msg : "null");	
			break;
	}
}
#endif

/**
 * 描述：丢弃socket接收缓冲区的剩余数据
 */
static ulong netflush(int sfd)
{
	ulong cnt = 0, nb;
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	char buf[TFTPMAXSIZE];

	while(1) {
		if(-1 == ioctl(sfd, FIONREAD, &nb))
			break;
		if(nb) {
			//discard rest packet	
			recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
			++cnt;
		} else 
			break;
	}
	
	return cnt;
}

/**
 * 描述：发送读请求，或者响应一个数据块的ACK，该函数会等待服务器端
 *		 发送过来的数据快	
 */ 
static int tftprrq(struct tftp_file *tf, char *buf, int pktsize)
{
	int size = 0;
	ushort opcode, block;
	struct tftp *tfp;
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);

	while(1) {
		if(sendto(tf->sckfd, buf, pktsize, 0, 
				(const struct sockaddr *)&tf->sin, sizeof(tf->sin)) != pktsize)
			return -1;
#if TFTP_DEBUG
		if(tftptrace)
			tracepacket("sent", (struct tftp *)buf, pktsize);
#endif		

		/* bad code下面的代码应当使用超时机制 */
		//timeout process
	
		/* 之所以要在sendto之后，是为了保证响应最后一个不满512字节的块  */
		if(tf->eof)
			return 0;
		
		size = recvfrom(tf->sckfd, tf->buf, sizeof(tf->buf), 0, 
				(struct sockaddr *)&from, &fromlen);
#if TFTP_DEBUG
		if(tftptrace)
			tracepacket("received", (struct tftp *)tf->buf, size);
#endif		
		if(size < TFTPMINSIZE) {
			log(LOG_WARNING, "tftp: data is too small\n");
			continue;
		}
		
		/**
 		 * 注意：这里服务器端很可能不使用tftp知名端口号进行数据响应，
 		 * 		 客户端需要识别出这一情况并进行端口切换 
 		 */
		if(tf->block <= 1) 
			tf->sin.sin_port = from.sin_port;

		if(tf->sin.sin_port != from.sin_port)
			continue;

		tfp = (struct tftp *)tf->buf;
		opcode = ntohs(tfp->tfp_opcode);	
		block = ntohs(tfp->tfp_blk);
		if(TFTP_OPCODE_ERR == opcode) {
			errno = tftperrmap[ntohs(tfp->tfp_errcode)];
			if(tfp->tfp_msg[0] != '\0')
				log(LOG_ERR, "tftp: receive err msg, %s\n", tfp->tfp_msg);
			return -1;
		}
		if(TFTP_OPCODE_DATA == opcode) {
			if(block == tf->block) {
				size -= TFTPMINSIZE;
				/* 当接收到报文数据小于512字节的块时，被认为已经到达文件末尾 */
				if(size < TFTPBLKSIZE)
					tf->eof = 1;
				if(!size) {
					/**
 					 * 当传输的文件刚好为512字节的整数倍时，数据发送方会额外发送一个数据长度为0的块，
 					 * 提醒对方已经到达文件末尾，下面这里针对最后一个数据长度为0的数据块进行响应
 					 */
					tfp = (struct tftp *)buf;
					tfp->tfp_opcode = htons((ushort)TFTP_OPCODE_ACK);
					tfp->tfp_blk = htons((ushort)block);
					sendto(tf->sckfd, buf, TFTPMINSIZE, 0, 
						(const struct sockaddr *)&tf->sin, sizeof(tf->sin));
#if TFTP_DEBUG
					if(tftptrace)
						tracepacket("sent", (struct tftp *)buf, TFTPMINSIZE);
#endif		
				}
				break;	
			}
			/* 接收到的块不是期望的块，flush socket接收缓存 */
			netflush(tf->sckfd);
			if(block != tf->block - 1) {
				errno = EIO;
				return -1;
			}
		}
	}

	return size;
}

/**
 * 描述：发送一个数据或写请求报文，并等待服务器的ACK
 */
static int tftpwrq(struct tftp_file *tf, char *buf, int pktsize)
{
	int size = 0;
	ushort opcode, block;
	struct tftp *tfp;
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	char ackbuf[TFTPMAXSIZE];

	while(1) {
		if(sendto(tf->sckfd, buf, pktsize, 0, 
				(const struct sockaddr *)&tf->sin, sizeof(tf->sin)) != pktsize)
			return -1;
#if TFTP_DEBUG
		if(tftptrace)
			tracepacket("sent", (struct tftp *)buf, pktsize);
#endif		

		/* bad code下面的代码应当使用超时机制 */
		//timeout process
	
		size = recvfrom(tf->sckfd, ackbuf, sizeof(ackbuf), 0, 
				(struct sockaddr *)&from, &fromlen);
#if TFTP_DEBUG
		if(tftptrace)
			tracepacket("received", (struct tftp *)ackbuf, size);
#endif		
		if(size < TFTPMINSIZE) {
			log(LOG_WARNING, "tftp: data is too small\n");
			continue;
		}
		
		/**
 		 * 注意：这里服务器端很可能不使用tftp知名端口号进行数据响应，
 		 * 		 客户端需要识别出这一情况并进行端口切换 
 		 */
		if(0 == tf->block) 
			tf->sin.sin_port = from.sin_port;

		if(tf->sin.sin_port != from.sin_port)
			continue;

		tfp = (struct tftp *)ackbuf;
		opcode = ntohs(tfp->tfp_opcode);	
		block = ntohs(tfp->tfp_blk);
		if(TFTP_OPCODE_ERR == opcode) {
			errno = tftperrmap[ntohs(tfp->tfp_errcode)];
			if(tfp->tfp_msg[0] != '\0')
				log(LOG_ERR, "tftp: receive err msg, %s\n", tfp->tfp_msg);
			return -1;
		}
		if(TFTP_OPCODE_ACK == opcode) {
			if(block == tf->block) {
				break;	
			}
			/* 接收到的块不是期望的块，flush socket接收缓存 */
			netflush(tf->sckfd);
			if(block != tf->block - 1) {
				errno = EIO;
				return -1;
			}
		}
	}

	return pktsize - TFTPMINSIZE;
}

/**
 * 描述：组装一个tftp读或写请求
 */
static int makerequest(struct tftp *tfp, int req, const char *filename, const char *mode)
{
	char *cp = (char *)tfp->tfp_stuff;

	tfp->tfp_opcode = htons((ushort)req);
	strcpy(cp, filename);
	cp += strlen(filename);
	*cp++ = '\0';
	strcpy(cp, mode);
	cp += strlen(mode);
	*cp++ = '\0';
	
	return cp - (char *)tfp;
}

static int tftp_open(struct netio_file *nf, struct URL *url, int flags)
{
	int accmode = flags & O_ACCMODE, pktsize, size;
	struct sockaddr_in myaddr = {0};
	struct tftp_file *tf = NULL;
	struct servent *sp;
	struct tftp *tfp;
	char reqbuf[TFTPMAXSIZE];
	
	if((accmode != O_RDONLY) && (accmode != O_WRONLY)) {
		errno = EACCES;
		return -1;
	}
	
	if(!(tf = (struct tftp_file *)malloc(sizeof(struct tftp_file)))) {
		errno = ENOMEM;
		return -1;
	}
	memset(tf, 0, sizeof(struct tftp_file));
	tf->sckfd = -1;
	tf->flags = flags;
	nf->pri = tf;
	
	/* server address & port */
	if(!(sp = getservbyname("tftp", "udp"))) {
		errno = EPROTONOSUPPORT;
		goto failed;
	}

	tf->sin.sin_len = sizeof(struct sockaddr_in);
	tf->sin.sin_family = AF_INET;
	tf->sin.sin_addr.s_addr = inet_addr(url->hostname);	//bad code, gethostbyname
	tf->sin.sin_port = (in_port_t)sp->s_port;

	/* client address */
	myaddr.sin_len = sizeof(struct sockaddr_in);
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = 0;
	
	if(-1 == (tf->sckfd = socket(AF_INET, SOCK_DGRAM, 0))) 
		goto failed;	
	if(-1 == bind(tf->sckfd, (const struct sockaddr *)&myaddr, sizeof(myaddr))) 
		goto failed;	
	
	tfp = (struct tftp *)reqbuf;
	if(O_RDONLY == accmode) {
		tf->block = 1;
		pktsize = makerequest(tfp, TFTP_OPCODE_RRQ, url->filepath, trans_mode);	
		size = tftprrq(tf, reqbuf, pktsize);	
		tf->end += size;
	} else {	//O_WRONLY
		tf->block = 0;
		pktsize = makerequest(tfp, TFTP_OPCODE_WRQ, url->filepath, trans_mode);	
		size = tftpwrq(tf, reqbuf, pktsize);	
	}

	if(size >= 0)
		return 0;

failed:
	if(tf) {
		if(tf->sckfd >= 0)
			close(tf->sckfd);
		free(tf);
		nf->pri = NULL;
	}

	return -1;
}

/**
 * 描述：关闭一个tftp传输，对于写操作，该函数将flush缓冲中未发送的剩余字节
 *       对于读操作，该函数将判断是否读到文件末尾，如果还有文件未读完，将
 *       向服务器端发送一个文件已关闭错误包
 */
static int tftp_close(struct netio_file *nf)
{
	int pktsize;
	struct tftp_file *tf = (struct tftp_file *)nf->pri;
	struct tftp *tfp = (struct tftp *)tf->buf;
	const char *msg = "file closed";
	
	if(O_WRONLY == (tf->flags & O_ACCMODE)) {
		tfp->tfp_opcode = htons((ushort)TFTP_OPCODE_DATA);
		++tf->block;
		tfp->tfp_blk = htons((ushort)tf->block);
		/* 当发送文件大小为512字节整数倍时，这里将会发送一个空数据包，通知服务端传输完成 */
		if((tftpwrq(tf, tf->buf, TFTPMINSIZE + tf->end)) < 0) 
			log(LOG_ERR, "tftp close: flush last block failed, errcode %d\n", errno);	
	} else /* O_RDONLY */ {
		/* 检查是否还有数据未读取完 */
		if(tf->foffs < tf->end || !tf->eof) {
			tfp->tfp_opcode = htons((ushort)TFTP_OPCODE_ERR);
			tfp->tfp_errcode = htons((ushort)TFTP_ERRCODE_EUNDEF);
			strcpy(tfp->tfp_msg, msg);
			tfp->tfp_msg[strlen(msg)] = '\0';
			pktsize = TFTPMINSIZE + strlen(msg);	//errcode + opcode + msg
			if(sendto(tf->sckfd, tf->buf, pktsize, 0, 
					(const struct sockaddr *)&tf->sin, sizeof(tf->sin)) != pktsize)
				log(LOG_ERR, "tftp close: errmsg send failed, errcode %d\n", errno);	
#if TFTP_DEBUG
			if(tftptrace)
				tracepacket("sent", (struct tftp *)tf->buf, pktsize);
#endif		
		}
	}

	close(tf->sckfd);
	nf->pri = NULL;
	free(tf);

	return 0;
}

static ssize_t tftp_read(struct netio_file *nf, void *buf, size_t len)
{
	size_t nb = len, n;
	struct tftp_file *tf = (struct tftp_file *)nf->pri;
	struct tftp ack = {.tfp_opcode = htons((ushort)TFTP_OPCODE_ACK)}, *tfp;

	if((tf->flags & O_ACCMODE) != O_RDONLY) {
		errno = EPERM;
		return -1;
	}

	tfp = (struct tftp *)tf->buf;
	while(nb && tf->start < tf->end) {
		if(tf->foffs >= tf->start && tf->foffs < tf->end) {
			n = tf->end - tf->foffs;
			if(nb < n)
				n = nb;
			memcpy(buf, &tfp->tfp_data[tf->foffs - tf->start], n);
			buf = (char *)buf + n;
			nb -= n;
			tf->foffs += n;
		}

		if(tf->foffs >= tf->end) {
			ack.tfp_blk = htons((ushort)tf->block);
			tf->block++;
			if((n = tftprrq(tf, (char *)&ack, TFTPMINSIZE)) < 0) {
				errno = EIO;
				return -1;
			}
			tf->start = tf->end;
			tf->end += n;
		}
	}

	return len - nb;
}

static ssize_t tftp_write(struct netio_file *nf, const void *buf, size_t len)
{
	size_t nb = len, n;
	struct tftp_file *tf = (struct tftp_file *)nf->pri;
	struct tftp *tfp = (struct tftp *)tf->buf;
	tfp->tfp_opcode = htons((ushort)TFTP_OPCODE_DATA);

	if((tf->flags & O_ACCMODE) != O_WRONLY) {
		errno = EPERM;
		return -1;
	}

	while(nb) {
		n = TFTPBLKSIZE - tf->end;
		if(nb < n)
			n = nb;
		memcpy(tfp->tfp_data + tf->end, buf, n);
		buf = (char *)buf + n;
		nb -= n;
		tf->foffs += n;
		tf->end += n;

		/* write函数仅当缓冲区数据超过512字节时才进行真正的数据发送 */
		if(TFTPBLKSIZE == tf->end) {
			++tf->block;
			tfp->tfp_blk = htons((ushort)tf->block);
			if((n = tftpwrq(tf, tf->buf, TFTPMAXSIZE)) < 0) {
				errno = EIO;
				return -1;
			}
			tf->end = 0;
		}
	}

	return len - nb;
}

static off_t tftp_lseek(struct netio_file *nf, off_t off, int whence)
{
	off_t noff;
	struct tftp_file *tf = (struct tftp_file *)nf->pri;

	switch(whence) {
		case SEEK_CUR:
			noff = tf->foffs + off;
			break;
		case SEEK_SET:
			noff = off;
			break;
		case SEEK_END:
		default:
			errno = EPERM;
			return -1;
	}

	if(O_WRONLY == (tf->flags & O_ACCMODE)) {
		/* can't move foffs */
		if(noff != tf->foffs) {
			errno = ESPIPE;
			return -1;
		}
	} else /* O_RDONLY */ {
		if(noff < tf->start) {
			errno = ESPIPE;
			return -1;
		}
		tf->foffs = noff;
	}

	return noff;
}

static int tftp_ioctl(struct netio_file *nf, unsigned long cmd, void *data)
{
	errno = ENOTTY;

	return -1;
}

static struct netio_ops tftp_ops = {
	.nio_open = tftp_open,
	.nio_close = tftp_close,
	.nio_read = tftp_read,
	.nio_write = tftp_write,
	.nio_lseek = tftp_lseek,
	.nio_ioctl = tftp_ioctl,
};

static struct netio tftp_nio = {
	.proto = "tftp",
	.ops = &tftp_ops,
};

/**
 * 描述：终端设备文件模块初始化函数，注意由于该函数处于ctor段，因此该
 * 		函数将在__init函数中被调用
 */
static void __attribute__((constructor)) tftp_init(void)
{
	netio_register(&tftp_nio);
}
