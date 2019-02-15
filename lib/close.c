
/**
 * Copyright(c) 2017-4-20 Shangwen Wu	
 *
 * close函数实现（close本应该交由系统调用实现，
 * PMON的BSD代码却将其作为一般的库函数使用）
 * 
 */
#include <common.h>
#include <stdio.h>
#include <sys/errno.h>
#include <fs/file.h>
#include <unistd.h>
#include <sys/errno.h>

int close(int fd)
{
	int ret;

	if(fd > OPEN_MAX) {
		errno = EMFILE;
		return -1;
	}

	if(__file[fd].valid) {
		if(__file[fd].fs->close)
			if(ret = (*__file[fd].fs->close)(fd))
				return ret;
		__file[fd].valid = 0;
		__file[fd].fs= NULL;
		/* 具体的file_system需要对data字段进行清空处理 */
	}

	return 0;
}
