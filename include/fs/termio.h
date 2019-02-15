
/**
 * Copyright(c) 2016-8-16 Shangwen Wu	
 *
 * 终端设备相关头文件
 * 
 */
#ifndef __FS_TERMIO_H__
#define __FS_TERMIO_H__

#include <queue.h>

/* 终端控制相关宏定义 */
//行设置标识相关宏定义
#define TLISIG		0x0001			//时能识别INTR\QUIT\SUSP等控制信号
#define TLICANON	0x0002			//规范化的行输入
#define TLECHO		0x0004			//回显功能
#define TLECHOE		0x0008			//虚拟清除字符\b

//输入设置标识相关宏定义
#define TICRNL		0x0001			//输入的\r字符被装化为\n字符
#define TISTRIP		0x0002			//清零接收字符的最高位，过滤可打印字符
#define TIERXFC		0x0004			//使能软件接收流控

//输入设置标识相关宏定义
#define TONLCR		0x0001			//输出的\n字符被装化为\r+\n字符
#define TOANYFC		0x0002			//处于发送关闭状态时，接收任意字符将开启发送
#define TOETXFC		0x0004			//使能软件发送流控

#define TERMIO_LINE_FLAGS_INIT		(TLISIG|TLICANON|TLECHO|TLECHOE)
#define TERMIO_OUTPUT_FLAGS_INIT	(TONLCR|TOANYFC|TOETXFC)
#define TERMIO_INPUT_FLAGS_INIT		(TICRNL|TIERXFC|TISTRIP)

//控制字符索引
#define CC_VINTR	0				//中断信号
#define CC_VERASE	1				//撤销字符信号
#define CC_VEOL		2				//行结束字符
#define CC_VEOL2	3				//行结束信号
#define CC_VXON		4				//软件流空发送允许信号
#define CC_VXOFF	5				//软件流控停止发送信号
#define CC_VCR		6				//\r

//定义CTRL+字符
#define CTRL(c)		((c) & 0x1f)

/* 终端设备相关宏定义 */
#define TTY_DEV_MAX	8

struct tty_device;

/* TTY操作功能函数 */
typedef struct tty_operations {
	int (*tty_device_init)(struct tty_device *);
	int (*tty_device_open)(struct tty_device *);
	int (*tty_device_reset)(struct tty_device *);
	int (*tty_set_baudrate)(struct tty_device *, int);
	int (*tty_tx_ready)(struct tty_device *);
	int (*tty_tx_byte)(struct tty_device *, unsigned char);
	int (*tty_rx_ready)(struct tty_device *);
	int (*tty_rx_byte)(struct tty_device *, unsigned char *);
	int (*tty_rx_hardflwctl)(struct tty_device *, int);
	int (*tty_tx_hardflwctl)(struct tty_device *, int *);
}Tty_Operations;

#define TERMIO_CTLCODE_NUM			23

struct termio {
	unsigned short l_flags;
	unsigned short i_flags;
	unsigned short o_flags;
	unsigned long i_speed;
	unsigned long o_speed;
	unsigned char t_ctlcodes[TERMIO_CTLCODE_NUM];	//终端控制字符定义
};

#define TERMIO_IO_QUEUE_SIZE		256				//终端的IO缓冲队列大小
#define TERMIO_RX_FC_TRIGGER		20				//终端接收流控触发大小

/* 终端设备结构 */
typedef struct tty_device {
	int	nopen;										//正在打开该设备的应用个数
	int txoff;										//发送是否关闭
	int rxoff;										//接收是否关闭
	unsigned long iobase;							//访问基地址
	unsigned long clk;								//参考时钟
	Queue *txqueue;									//发送缓冲队列
	Queue *rxqueue;									//接收缓冲队列
	struct termio t;								//终端相关描述
	struct tty_operations *ops;						//当前物理设备操作集合
}Tty_Device;

/* 终端设备配置结构体，该结构体与具体设备相关 */
typedef struct tty_configure {
	unsigned long io;								//访问基地址
	unsigned long clk;								//参考时钟
	unsigned long baud;								//波特率
	struct tty_operations *ops;						//当前物理设备操作集合
}Tty_Configure;

#define SERIALIN		0
#define SERIALOUT		1
#define SERIALERROUT	2
#define KBDIN			3
#define VGAOUT			4
#define VGAERROUT		5

/* 终端IOCTL命令 */
#define TCSCANON		1							//规范化行输入
#define TCSNCANON		2							//非规范化行输入
#define TCSTERMIO		3							//设置termio信息
#define TCGTERMIO		4							//获取termio信息
#define TCSFLUSH		5							//刷新读缓冲队列
#define TCSBREAK		6							//关闭回显以及标准输入行模式

#endif //__FS_TERMIO_H__

