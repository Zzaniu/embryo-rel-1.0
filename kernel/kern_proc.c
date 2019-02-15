
/**
 * Copyright(c) 2017-4-20 Shangwen Wu 
 *
 * 这个文件定义了系统进程相关的函数
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/system.h>
#include <sys/uio.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/syslog.h>

struct proc *curproc;								//指向当前进程指针

static int nproc = 0;								//当前进程数
static struct proc allprocs[NPROC] = {0};
static struct filedesc filedesc0 = {0};				//给进程1用的文件描述结构

static pid_t __alloc_pid(void)
{
	static pid_t cur = 0;

	return ++cur;
}

/**
 * 描述：初始化全局的进程数组，并初始化filedesc
 *
 */
int procinit(void)
{
	/* 初始化主进程 */
	curproc = &allprocs[++nproc];					//当前指针指向allprocs数组的第一个元素
	curproc->p_id = __alloc_pid();	
	curproc->p_name = "Main";					
	curproc->p_fd = &filedesc0;
	/* SERIALIN,SERIALOUT,SERIALERROUT,KBDIN,VGAOUT, VGAERROUT，termio已使用6个描述符 */
	curproc->p_fd->fd_nfiles = 6;

	log(LOG_DEBUG, "proc init done\n");

	return 0;
}
