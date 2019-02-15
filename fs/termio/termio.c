
/**
 * Copyright(c) 2016-8-16 Shangwen Wu	
 *
 * 终端设备相关
 * 
 */
#include <autoconf.h>
#include <common.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fs/file.h>
#include <fs/termio.h>

#include <mach/early_log.h>
#include <mach/intr.h>

#define TERMIO_DEBUG			1
#if TERMIO_DEBUG
	#define TTY_DEBUG(fmt, args...) 	early_debug(fmt, ##args)
#else
	#define TTY_DEBUG(fmt, args...) 	do {} while(0)
#endif
#define TTY_ERROR(fmt, args...)			early_err(fmt, ##args)
#define TTY_INFO(fmt, args...)			early_info(fmt, ##args)

#ifdef OUTPUT_TO_BOTH
static int output_to_both = 1;
#else
static int output_to_both = 0;
#endif

/* 保存在File结构中，用于保存FD和物理设备之间的映射关系 */
struct termio_data {
	int devid;					//物理设备的索引和其物理设备的名称对应
};

void scandevs(void);

extern struct tty_configure tty_cfgtable[];			//defined in machdep.c

static struct tty_device tty_devtable[TTY_DEV_MAX] = {};

/**
 * 描述：设置终端相关协议，以及初始化终端功能
 */
static void termio_setup(struct tty_device *dev)
{
	dev->t.i_flags = TERMIO_INPUT_FLAGS_INIT;
	dev->t.o_flags = TERMIO_OUTPUT_FLAGS_INIT;
	dev->t.l_flags = TERMIO_LINE_FLAGS_INIT;
	dev->t.t_ctlcodes[CC_VINTR] = CTRL('c');
	dev->t.t_ctlcodes[CC_VERASE] = CTRL('h');
	dev->t.t_ctlcodes[CC_VEOL] = '\n';
	dev->t.t_ctlcodes[CC_VEOL2] = CTRL('c');
	dev->t.t_ctlcodes[CC_VXON] = CTRL('q');
	dev->t.t_ctlcodes[CC_VXOFF] = CTRL('s');
	dev->t.t_ctlcodes[CC_VCR] = '\r';
}

/**
 * 描述：立即发送一个字节，而不是将该字符插入发送队列
 */
static void termio_tx_noq(struct tty_device *dev, char c)
{
	while(!(*dev->ops->tty_tx_ready)(dev))
		;
	(*dev->ops->tty_tx_byte)(dev, c);
};

/**
 * 描述：扫描指定终端设备，轮询物理链路的接收状态，将接收到的数据插入接收队列；
 *       将tx队列中的数据发送到物理链路
 */
static int termio_scandev(struct tty_device *dev)
{
	int n;
	char c;

	/* 轮询接收状态 */
	while((*dev->ops->tty_rx_ready)(dev)) {
		(*dev->ops->tty_rx_byte)(dev, &c);
			
		if(dev->t.i_flags & TISTRIP) 
			c &= 0x7f;
		if((dev->t.l_flags & TLISIG) && dev->t.t_ctlcodes[CC_VINTR] == c)
			;				//send intr signal
		//发送流控
		if((dev->t.o_flags & TOETXFC)) {
			if(dev->t.t_ctlcodes[CC_VXOFF] == c) {
				dev->txoff = 1;
				continue;
			} else if(dev->t.t_ctlcodes[CC_VXON] == c) {
				dev->txoff = 0;
				continue;
			} else if(dev->t.o_flags & TOANYFC) {
				dev->txoff = 0;
			}
		}
		n = Qspace(dev->rxqueue);
		//将收到的新数据插入到接收队列
		if(n > 0) {
			Qput(dev->rxqueue, c);
			//接收流控
			if(dev->t.i_flags & TIERXFC) {
				if(n < TERMIO_RX_FC_TRIGGER && !dev->rxoff) {
					termio_tx_noq(dev, dev->t.t_ctlcodes[CC_VXOFF]);
					dev->rxoff = 1;			//注意rxoff=1并不会阻止接收新数据
				}
			}
		} else
			break;							//接收队列溢出，丢弃新收到的字符
	}

	/* 发送TX缓冲队列中的数据 */
	if(!dev->txoff) {
		n = Qused(dev->txqueue);
		while(n-- > 0 && ((*dev->ops->tty_tx_ready)(dev))) 
			(*dev->ops->tty_tx_byte)(dev, Qget(dev->txqueue));
	}

	return 0;
}

/**
 * 描述：扫描所有交互式设备
 * bad code
 */
void scandevs(void)
{
	int s;

	struct tty_device *dev;
	/* 轮询串口以及显示输出终端 */
	for(dev = tty_devtable; dev->iobase || dev->ops; ++dev)
		termio_scandev(dev);
	/* 这里将轮询所有虚拟中断 */
	s = splhigh();
	splx(s);	
}

/**
 * 描述：终端设备打开函数
 * 返回：打开成功返回当前文件描述符，否则返回-1
 */
static int termio_open(int fd, const char *filepath, int flags, int perms)
{
	int devid = TTY_DEV_MAX;
	struct termio_data *data = NULL;

	if(strncmp(filepath, DEVICE_PREFIX, strlen(DEVICE_PREFIX))) {
		TTY_ERROR("open failed, %s is not a device\r\n", filepath);
		errno = ENOENT;	
		return -1;
	}
	filepath += strlen(DEVICE_PREFIX);
	if(strncmp(filepath, TTY_DEVICE, strlen(TTY_DEVICE))) {
		TTY_ERROR("open failed, %s is not a TTY device\r\n", filepath);
		errno = ENOENT;	
		return -1;
	}
	filepath += strlen(TTY_DEVICE);
	if(*filepath >= '0' && *filepath <= '9')
		devid = *filepath - '0';
	else if(*filepath >= 'A' && *filepath <= 'Z')
		devid = *filepath - 'A' + 10;
	else if(*filepath >= 'a' && *filepath <= 'z')
		devid = *filepath - 'a' + 10;	

	if(devid >=  TTY_DEV_MAX) {
		TTY_ERROR("open failed, %d is exceed max tty device num %d\r\n", devid, TTY_DEV_MAX);
		errno = EMFILE;	
		return -1;
	}
	if(!tty_devtable[devid].iobase) {
		TTY_ERROR("open failed, tty%d is non-initialized\r\n", devid);
		errno = ENXIO;	
		return -1;
	}

	if(__file[fd].data) {
		TTY_ERROR("open failed, data is not null\r\n");
		errno = EPERM;	
		return -1;
	}

	if(NULL == (data = (struct termio_data *)malloc(sizeof(struct termio_data)))) {
		TTY_ERROR("open failed, out of memory\r\n");
		errno = ENOMEM;	
		return -1;
	}

	(*tty_devtable[devid].ops->tty_device_open)(&tty_devtable[devid]);
	tty_devtable[devid].nopen++;
	
	data->devid = devid;
	__file[fd].data = data;

	return fd;
}

static int termio_release(int fd)
{
	int devid;
	struct termio_data *data = NULL;
	struct tty_device *dev = NULL;

	if(!(data = (struct termio_data *)(__file[fd].data))) {
		errno = EPERM;	
		return -1;
	}
	devid = data->devid;
	if(devid >= TTY_DEV_MAX) {
		errno = EPERM;	
		return -1;
	}
	dev = tty_devtable + devid;

	dev->nopen--;
	free(__file[fd].data);
	__file[fd].data = NULL;

	return 0;
}

static ssize_t termio_read(int fd, void *buf, size_t len)
{
	int n, devid;
	size_t i = 0; 
	char *ptr = (char *)buf, c;
	struct termio_data *data = NULL;
	struct tty_device *dev = NULL;

	if(!(data = (struct termio_data *)(__file[fd].data))) {
		errno = EPERM;
		return -1;
	}
	devid = data->devid;
	if(devid >= TTY_DEV_MAX) {
		errno = EPERM;	
		return -1;
	}
	dev = tty_devtable + devid;

	while(i < len) {
		scandevs();					//扫描设备
		if(!(n = Qused(dev->rxqueue))) 
			continue;

		//接收流控
		if(dev->t.i_flags & TIERXFC) {
			if(n < TERMIO_IO_QUEUE_SIZE - TERMIO_RX_FC_TRIGGER && dev->rxoff) {
				termio_tx_noq(dev, dev->t.t_ctlcodes[CC_VXON]);
				dev->rxoff = 0;			
			}
		}
		c = Qget(dev->rxqueue);
		if(dev->t.i_flags & TICRNL) {
			if(c == dev->t.t_ctlcodes[CC_VCR]) 
				c = dev->t.t_ctlcodes[CC_VEOL];
		}
		if(dev->t.l_flags & TLICANON) {
			if(c == dev->t.t_ctlcodes[CC_VERASE]) {
				if(i > 0) {
					if(dev->t.l_flags & TLECHOE) 
						write(fd, "\b \b", 3);
					else if(dev->t.l_flags & TLECHO) 
						write(fd, "\b", 1);
					i--;
				}
				continue;
			} 
			if(dev->t.l_flags & TLECHO) 
				write(fd, &c, 1);
			ptr[i++] = c;
			if(c == dev->t.t_ctlcodes[CC_VEOL] || c == dev->t.t_ctlcodes[CC_VEOL2]) 
				break;
		} else { 
			ptr[i++] = c;
		} 
	}

	return i;
}

static ssize_t termio_write(int fd, const void *buf, size_t len)
{
	int n, devid;
	size_t i, ofs, _len;
	const char *ptr = (const char *)buf;
	struct termio_data *data = NULL;
	struct tty_device *dev = NULL;

	if(!(data = (struct termio_data *)(__file[fd].data))) {
		errno = EPERM;
		return -1;
	}
	devid = data->devid;
	if(devid >= TTY_DEV_MAX) {
		errno = EPERM;
		return -1;
	}
	dev = tty_devtable + devid;
	if(!dev->iobase || !dev->ops) {
		errno = EPERM;
		return -1;
	}

	do {
		for(_len = len, ofs = 0; _len > 0; ofs += i, _len -= i) {
			n = Qspace(dev->txqueue);
			for(i = 0; i < _len; ++i){
				if(n >= 2) {
					if(dev->t.o_flags & TONLCR) {
						if(ptr[ofs + i] == dev->t.t_ctlcodes[CC_VEOL]) {
							Qput(dev->txqueue, dev->t.t_ctlcodes[CC_VCR]);
							n--;	
						}
					}
					Qput(dev->txqueue, ptr[ofs + i]);	
					n--;	
				} else 							//发送队列满，退出发送步骤
					break;
			}
		
			while(!Qisempty(dev->txqueue))		//等待物理设备将数据发送完成，注意若是txoff=1会造成该函数死等
				scandevs();
		}
		dev = ((dev + 1)->iobase || (dev + 1)->ops) ? dev + 1 : tty_devtable;
	} while (output_to_both && dev != tty_devtable + devid);

	return ofs;
}

static off_t termio_lseek(int fd, off_t off, int whence)
{
	return 0;
}

static int termio_ioctl(int fd, unsigned long cmd, ...)
{
	int devid;
	va_list ap;
	void *arg;
	struct termio_data *data = NULL;
	struct tty_device *dev = NULL;
	struct termio *t;

	if(!(data = (struct termio_data *)(__file[fd].data))) {
		errno = EPERM;
		return -1;
	}
	devid = data->devid;
	if(devid >= TTY_DEV_MAX) {
		errno = EPERM;
		return -1;
	}
	dev = tty_devtable + devid;

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);
	
	switch(cmd) {
		case TCSCANON:
			if(arg)
				*(struct termio *)arg = dev->t;
			dev->t.l_flags |= (TLECHO|TLECHOE|TLICANON);
		break;	
		case TCSNCANON:
			if(arg)
				*(struct termio *)arg = dev->t;
			dev->t.l_flags &= ~(TLECHO|TLECHOE|TLICANON);
		break;
		case TCSTERMIO:
			if(arg) {
				t = (struct termio *)arg;
				if(t->i_speed != dev->t.i_speed) 
					(*dev->ops->tty_set_baudrate)(dev, t->i_speed);
				dev->t = *t;
			} else {
				errno = EINVAL;
				return -1;
			}
		break;
		case TCGTERMIO:
			if(arg)
				*(struct termio *)arg = dev->t;
			else  {
				errno = EINVAL;
				return -1;
			}
		break;
		case TCSBREAK:
			if(arg)
				*(struct termio *)arg = dev->t;
			dev->t.l_flags &= ~(TLECHO|TLICANON);
		break;
		case TCSFLUSH:
			while(!Qisempty(dev->rxqueue))
				Qget(dev->rxqueue);
			if(dev->t.l_flags & TIERXFC) {
				if(dev->rxoff) {
					termio_tx_noq(dev, dev->t.t_ctlcodes[CC_VXON]);
					dev->rxoff = 0;
				}
			}
		break;
		default:
			errno = EINVAL;
			return -1;
	}

	return 0;
}

static struct file_system termio_fs = {
	.fs_name = "tty",
	.fs_type = FS_TTY,
	.open = termio_open,
	.close = termio_release,
	.read = termio_read,
	.write = termio_write,
	.lseek = termio_lseek,
	.ioctl = termio_ioctl,
};

static int termio_devinit(void)
{
	int i; 

	for(i = 0; tty_cfgtable[i].io != 0 && i < TTY_DEV_MAX; ++i) {
		tty_devtable[i].iobase = tty_cfgtable[i].io;
		tty_devtable[i].clk = tty_cfgtable[i].clk;
		tty_devtable[i].t.i_speed = tty_cfgtable[i].baud;
		tty_devtable[i].t.o_speed = tty_cfgtable[i].baud;
		tty_devtable[i].ops = tty_cfgtable[i].ops;
		tty_devtable[i].nopen = 0;
		tty_devtable[i].rxoff = 0;
		tty_devtable[i].txoff = 0;
			
		tty_devtable[i].rxqueue = Qcreate(TERMIO_IO_QUEUE_SIZE);
		tty_devtable[i].txqueue = Qcreate(TERMIO_IO_QUEUE_SIZE);
		if(NULL == tty_devtable[i].rxqueue || NULL == tty_devtable[i].txqueue)
			goto failed;

		//初始化终端
		termio_setup(&tty_devtable[i]);
		//初始化物理设备
		(*tty_devtable[i].ops->tty_device_init)(&tty_devtable[i]);
	}

	return 0;

failed:
	for(; i >= 0; i--) {
		if(tty_devtable[i].rxqueue)
			Qfree(tty_devtable[i].rxqueue);
		if(tty_devtable[i].txqueue)
			Qfree(tty_devtable[i].txqueue);
	}

	return -1;
}

/**
 * 描述：终端设备文件模块初始化函数，注意由于该函数处于ctor段，因此该
 * 		函数将在__init函数中被调用
 */
static void __attribute__((constructor)) termiofs_init(void)
{
	if(termio_devinit()) {
		TTY_ERROR("termio devinit failed!\r\n");
		return ;
	} 

	fs_register(&termio_fs);

	/* 初始化几个默认打开的终端设备文件 */
	__file[SERIALIN].valid = 1;
	__file[SERIALIN].fs= &termio_fs;
	__file[SERIALOUT].valid = 1;
	__file[SERIALOUT].fs= &termio_fs;
	__file[SERIALERROUT].valid = 1;
	__file[SERIALERROUT].fs= &termio_fs;
	__file[KBDIN].valid = 1;
	__file[KBDIN].fs= &termio_fs;
	__file[VGAOUT].valid = 1;
	__file[VGAOUT].fs= &termio_fs;
	__file[VGAERROUT].valid = 1;
	__file[VGAERROUT].fs= &termio_fs;
	
	termio_open(SERIALIN, 		TTY_SERIAL_DEVICE, 0, 0);		// 0 ---> tty0 ---> 串口输入
	termio_open(SERIALOUT, 		TTY_SERIAL_DEVICE, 0, 0);		// 1 ---> tty0 ---> 串口输出
	termio_open(SERIALERROUT, 	TTY_SERIAL_DEVICE, 0, 0);		// 2 ---> tty0 ---> 串口输出
	termio_open(KBDIN, 			TTY_STDIO_DEVICE, 0, 0);		// 3 ---> tty1 ---> 键盘输入
	termio_open(VGAOUT, 		TTY_STDIO_DEVICE, 0, 0);		// 4 ---> tty1 ---> 显示器输出
	termio_open(VGAERROUT, 		TTY_STDIO_DEVICE, 0, 0);		// 5 ---> tty1 ---> 显示器输出
	
	TTY_INFO("termio fs init done!\r\n");
}
