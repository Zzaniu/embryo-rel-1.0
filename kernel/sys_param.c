
/**
 * Copyright(c) 2017-3-6 Shangwen Wu 
 *
 * 系统使用的各种全局参数
 * 
 */

#include <sys/types.h>
#include <sys/system.h>
#include <sys/proc.h>
#include <sys/file.h>

int maxfiles = 0;				//系统下可打开的最大文件数

void paraminit(void)
{
	maxfiles = NPROC * NFILE;
}

