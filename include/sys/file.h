
/**
 * Copyright(c) 2017-2-27 Shangwen Wu	
 *
 * 底层文件抽象相关头文件
 * 
 */
#ifndef __SYS_FILE_H__
#define __SYS_FILE_H__

#include <sys/list.h>

#define NFILE			20

struct uio;
struct proc;
struct file;

struct fileops {
	int (*fo_write)(struct file *, struct uio *);
	int (*fo_read)(struct file *, struct uio *);
	int (*fo_ioctl)(struct file *, unsigned long, caddr_t, struct proc *);
	int (*fo_close)(struct file *, struct proc *);
};

struct file {
	short f_flag;
	short f_type;						//文件类型
#define FTYPE_VNODE		1
#define FTYPE_SOCKET	2
#define FTYPE_PIPE		3
	long f_refcnt;
	off_t f_offset;
	caddr_t f_data;
	struct list_head f_list;
	struct fileops *f_ops;
};

struct filedesc {
	/* 存放但前进程下所有打开文件的file结构，索引为每个文件的fd，每个元素被放入allfileslist链表 */
	struct file *fd_ofiles[NFILE];		//当前进程最大打开NFILE个文件
	int fd_nfiles;						//当前打开的文件个数，初值为6
};

extern int maxfiles;					//defined in sys_param.c
extern int nfiles;						//defined in filedesc.c
extern struct list_head allfileslist;	//defined in filedesc.c

extern int fdalloc(int *fdp);			//defined in filedesc.c
extern int falloc(struct proc *p, int *fdp, struct file **fp);
extern void ffree(struct proc *p, int fd);
extern int frelease(struct proc *p, int fd);
extern void fdfree(int fd);

#endif //__SYS_FILE_H__

