
/**
 * Copyright(c) 2017-1-19 Shangwen Wu	
 *
 * 显示设备命令
 * 
 */

#include <common.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/device.h>
#include <command.h>
#include <fs/termio.h>

extern struct list_head alldevslist;			//BIOS设备链表, defined in autoconf.c

static const char *clasname[] = {"DULL", "TTY", "NET", "DISK"};

/**
 * 描述：执行lsdev测试命令，默认只显示实际的物理设备
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_lsdev(int argc, const char *argv[])
{
	int optind;
	char opt, *optval = NULL;
	int opt_a = 0;
	struct device *dev;

	optind = 0;
	while((opt = getopt(argc, argv, &optind, &optval, "a")) != EOF) {
		switch(opt) {
			case 'a':
				opt_a = 1;
			break;
			default:							//BADOPT
				return -2;
		}
	}

	if(optind < argc)
		return -2;

	printf("Device name     Type\n");

	list_for_each_entry(dev, &alldevslist, dv_node) {	
		if(!opt_a && DEV_DULL == dev->dv_class) 
			continue;
		printf("%-11s     %-4s\n", dev->dv_name, clasname[dev->dv_class]);	
	}
	
	return 0;
}

/* lsdev命令的选项描述 */
static struct optdesc lsdev_opts[] = {
	{
		.name = "-a", 
		.desc = "show all device types",
	},
	{},
};

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"Misc"}},			//表组名
	{
		{"lsdev"}, 			
		"list devices", 
		"[-a]",
		lsdev_opts, 
		cmd_lsdev,
		0,
		MAX_CMD_ARG_NUM - 1,
		0, 
	},
	{},						//最后一个强制要求为空
};

/**
 * 描述：命令初始化函数
 */
static void __attribute__((constructor)) init_cmds(void)
{
	add_cmds(cmds, 0);
}
