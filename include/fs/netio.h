
/**
 * Copyright(c) 2018-9-6 Shangwen Wu	
 *
 * 网络IO相关头文件
 * 
 */
#ifndef __FS_NETIO_H__
#define __FS_NETIO_H__

struct list_head;
struct URL;
struct netio_file;

struct netio_ops {
	int (*nio_open)(struct netio_file *, struct URL *, int);
	int (*nio_close)(struct netio_file *);
	ssize_t (*nio_read)(struct netio_file *, void *, size_t);
	ssize_t (*nio_write)(struct netio_file *, const void *, size_t);
	off_t (*nio_lseek)(struct netio_file *, off_t, int);
	int (*nio_ioctl)(struct netio_file *, unsigned long, void *);
};

struct netio {
	char *proto;
	struct netio_ops *ops;
	struct list_head list;
};

struct netio_file {
	struct netio *nio;
	void *pri;
};

//defined in netio.c
extern int netio_register(struct netio *nio);
extern void netio_unregister(struct netio *nio);

#endif //__FS_NETIO_H__

