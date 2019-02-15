
/**
 * Copyright(c) 2018-10-02 Shangwen Wu	
 *
 * tftp IO相关头文件
 * 
 */
#ifndef __FS_TFTPFILE_H__
#define __FS_TFTPFILE_H__

struct sockaddr_in;

struct tftp_file {
	struct sockaddr_in sin;		/* 服务器地址信息 */
	ulong block;				/* 当前期望操作的块号 */
	ulong start, end;			/* read函数中表示，上一次接收数据的缓冲区起始和结束位置，write函数仅使用end,表示缓冲区有效数据长度 */
	off_t foffs;				/* 文件的当前位置 */	
	int flags;					/* open flags */
	int eof;					/* 表示文件是否到末尾 */
	int sckfd;					/* socket描述符 */
	uint8_t buf[TFTPMAXSIZE];	/* 接收以及发送数据缓冲区 */
};

#endif //__FS_TFTPFILE_H__

