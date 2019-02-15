
/**
 * Copyright(c) 2017-2-24 Shangwen Wu	
 *
 * open函数实现（open本应该交由系统调用实现，
 * PMON的BSD代码却将其作为一般的库函数使用）
 * 
 */
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <linklist.h>
#include <fs/file.h>
#include <fcntl.h>

/**
 * 描述：遍历全局filesystem链表，寻找匹配的fs结构
 * 		 匹配规则：1）在fs链表中找到匹配dname的节点，并且打开设备相关的open函数成功
 * 		 		   2）如果fs设备类型不为FS_NULL，且需要打开的设备类型与fs设备类型匹配，
 * 		 		   并且打开设备相关的open函数成功
 * 	
 */
static int __open(int fd, const char *fname, 
				const char *dname, int mode, int fstype)
{
	struct file_system *fs;

	for(fs = fs_first(); fs != fs_end(); fs = fs_next(fs)) {
		if(dname && strprefix(dname, fs->fs_name)) {
			if(fs->open && (*fs->open)(fd, fname, mode, 0) != fd)
				return -1;
			break;
		} else if(fstype != FS_NULL && fstype == fs->fs_type) {
			if(fs->open && (*fs->open)(fd, fname, mode, 0) == fd)
				break;
		}
	}
	
	if(fs != fs_end()) {
		__file[fd].valid = 1;
		__file[fd].fs = fs;
		return fd;
	} else {
		errno = ENOENT;
		return -1;
	}
}

/**
 * 描述：用于处理内核态使用open函数后出错情况下的fd释放工作, bad code
 */
void __open_err(int fd)
{
	if(fd < 0 || fd >= OPEN_MAX)
		return;

	if(__file[fd].valid) {
		__file[fd].valid = 0;
		__file[fd].fs = NULL;
	}
}

/**
 * 描述：open函数实现，该函数将调用file_system结构中跟具
 * 		 体设备相关的open函数，路径必须是以dev打头的设别
 * 		 文件
 */
int open(const char *fname, int mode)
{
	int fd, _fd;
	const char *dname;

	for(_fd = 0; __file[_fd].valid; ++_fd)
		;

	if(_fd > OPEN_MAX) {
		errno = EMFILE;
		return -1; 
	}

	/* 下面将_fd置为有效，是为了在open函数打开另一个文件的情形中，防止文件描述符分配错误 */
	__file[_fd].valid = 1;
	if(!strncmp(fname, DEVICE_PREFIX, strlen(DEVICE_PREFIX))) {
		dname = fname + strlen(DEVICE_PREFIX);
		if(-1 == (fd = __open(_fd, fname, dname, mode, FS_NULL)))
			goto err;	
	} else if(strpat(fname, "tftp://*")) {
		if(-1 == (fd = __open(_fd, fname, "netio", mode, FS_NETIO)))
			goto err;	
	} else {
		errno = ENOENT;
		goto err;	
	}
	
	return fd;	

err:
	__file[_fd].valid = 0;

	return -1;
}

