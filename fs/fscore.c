
/**
 * Copyright(c) 2016-8-21 Shangwen Wu	
 *
 * 文件核心接口相关文件
 * 
 */
#include <common.h>
#include <sys/types.h>

#include <stdio.h>
#include <linklist.h>
#include <fs/file.h>

/* 全局文件系统接口链表 */
static SLIST_HEAD(fs_list, file_system) fs_list = SLIST_INITIALIZER(fs_list);

File __file[OPEN_MAX] = {0};			//保存所有设备文件操作接口，文件描述符fd就是该数组的索引

void fs_register(struct file_system *fs)
{
	SLIST_INSERT_HEAD(&fs_list, fs, fs_next);
}

struct file_system *fs_first(void)
{
	return SLIST_FIRST(&fs_list);
}

struct file_system *fs_end(void)
{
	return SLIST_END(&fs_list);
}

struct file_system *fs_next(struct file_system *cur)
{
	return SLIST_NEXT(cur, fs_next);
}

