
/**
 * Copyright(c) 2018-11-12 Shangwen Wu	
 * 块设备文件定义（驱动层）
 * 
 */

#ifndef __FS_BLOCK_FILE_H__
#define __FS_BLOCK_FILE_H__

#define BLK_FILENAME_MAX		16

/**
 *  注意：这里限制每次硬件IO的最大字节数，适当调整该值将影响IO性能 
 *  	  该值最小应不小于具体块设备的一个块的字节数，否则将引起读
 *  	  写数据出错
 */
#define BLKFILE_MAX_IO_BYTES	(32 * 1024)

struct block_fileops;

/* 块设备文件描述 */
struct block_file {
	char bf_name[BLK_FILENAME_MAX];					//此名字应与块设备名同名
	loff_t bf_offs;
	loff_t bf_end;
	loff_t bf_base;
	void *bf_priv;
	struct block_fileops *bf_ops;
	int bf_oflags;									//open flags
};

struct block_fileops {
	int (*bfo_open)(struct block_file *, int);
	int (*bfo_close)(struct block_file *);
	ssize_t (*bfo_read)(struct block_file *, void *, size_t);
	ssize_t (*bfo_write)(struct block_file *, const void *, size_t);
	loff_t (*bfo_lseek)(struct block_file *, loff_t, int);
	int (*bfo_ioctl)(struct block_file *, unsigned long, void *);
};

extern struct block_fileops blkops;		//defined in block_file.c

#endif //__FS_BLOCK_FILE_H__

