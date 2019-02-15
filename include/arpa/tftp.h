
/**
 * Copyright(c) 2018-10-02 Shangwen Wu	
 *
 * TFTP协议头定义
 */

#ifndef __ARPA_TFTP_H__
#define __ARPA_TFTP_H__

/* tftp protocol header define */
struct tftp {
	uint16_t tfp_opcode;
	union {	
		uint16_t tu_blk;
		uint16_t tu_errcode;
		uint8_t tu_stuff[1];	/* 请求文件名、传输模式 */
	} tfp_u;
	uint8_t tfp_data[1];
#define tfp_blk tfp_u.tu_blk
#define tfp_errcode tfp_u.tu_errcode
#define tfp_stuff tfp_u.tu_stuff
#define tfp_msg tfp_data
};

#define TFTPBLKSIZE		512	//一个数据块大小
#define TFTPMAXSIZE		(4 + TFTPBLKSIZE)	//最大tftp报文长度
#define TFTPMINSIZE		(4) //opcode + blk

#define TFTP_OPCODE_RRQ		0x0001
#define TFTP_OPCODE_WRQ		0x0002
#define TFTP_OPCODE_DATA	0x0003
#define TFTP_OPCODE_ACK		0x0004
#define TFTP_OPCODE_ERR		0x0005

#define TFTP_ERRCODE_EUNDEF			0x0000
#define TFTP_ERRCODE_ENOTFOUND 		0x0001
#define TFTP_ERRCODE_EACCESS		0x0002
#define TFTP_ERRCODE_ENOSPACE		0x0003
#define TFTP_ERRCODE_EBADOP			0x0004
#define TFTP_ERRCODE_EBADID			0x0005
#define TFTP_ERRCODE_EEXISTS		0x0006
#define TFTP_ERRCODE_ENOUSER		0x0007

#endif //__ARPA_TFTP_H__
