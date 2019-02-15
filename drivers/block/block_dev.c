
/**
 * Copyright(c) 2018-11-01 Shangwen Wu
 * 块设备支持
 * 
 */
#include <common.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <sys/syslog.h>
#include <sys/device.h>
#include <sys/block.h>

/* 全局block device链表 */
static LIST_HEAD(blkdev_list);

/* 注册块设备函数函数 */
int blkdev_register(struct block_device *blkdev)
{
	list_add(&blkdev->bd_list, &blkdev_list);

	return 0;
}

int blkdev_unregister(struct block_device *blkdev)
{
	list_del(&blkdev->bd_list);

	return 0;
}

/* 根据网络devname查找匹配的block_device */
struct block_device *blkdev_lookup(const char *devname)
{
	struct block_device *blkdev;

	list_for_each_entry(blkdev, &blkdev_list, bd_list) {
		if(!strcmp(blkdev->bd_name, devname)) {
			++blkdev->bd_refcnt;
			return blkdev;	
		}
	}

	return NULL;
}
