
/**
 * Copyright(c) 2018-11-27 Shangwen Wu	
 *
 * 文件系统IO相关头文件
 * 
 */
#ifndef __FS_FSIO_H__
#define __FS_FSIO_H__

#define FS_NAME_MAX	64

struct list_head;
struct fsio_file;

struct fsio_ops {
	int (*fio_open)(struct fsio_file *, const char *, int);
	int (*fio_close)(struct fsio_file *);
	ssize_t (*fio_read)(struct fsio_file *, void *, size_t);
	ssize_t (*fio_write)(struct fsio_file *, const void *, size_t);
	off_t (*fio_lseek)(struct fsio_file *, off_t, int);
	off_t (*fio_llseek)(struct fsio_file *, loff_t, int);
	int (*fio_ioctl)(struct fsio_file *, unsigned long, void *);
};

struct fsio {
	char *fsname;
	struct fsio_ops *ops;
	struct list_head list;
};

struct fsio_file {
	struct fsio *fio;
	void *pri;
};

//defined in fsio.c
extern int fsio_register(struct fsio *fio);
extern void fsio_unregister(struct fsio *fio);

#endif //__FS_FSIO_H__

