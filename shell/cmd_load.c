
/**
 * Copyright(c) 2019-1-17 Shangwen Wu	
 *
 * 加载可执行文件命令
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <command.h>
#include <load_exec.h>
#include <fs/termio.h>
#include <asm/cache.h>

/* 用于load选项参数 */
#define LOPT_RFLAG		0x0001

extern int getopt(int argc, const char *argv[], int *optind, \
		char **optval, const char *pattern);				//defined in getopt.c
extern register_t md_setpc(register_t val);					//defined in mipsdep.c

/**
 * 描述：执行load命令，内核加载也通过该命令完成
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_load(int argc, const char *argv[])
{
	int optind;
	char opt, *optval = NULL;
	unsigned long entry;
	int fd, flags = 0, type = EXEC_TYPE_UNKOWN;

	optind = 0;
	while((opt = getopt(argc, argv, &optind, &optval, "r")) != EOF) {
		switch(opt) {
			case 'r':
				flags |= LOPT_RFLAG;
			break;
			default:							//BADOPT
				return -2;
		}
	}

	if(optind >= argc) {
		fprintf(stderr, "%s:  missing file operand\n", *argv);
		return -2;
	}
	
	if(-1 == (fd = open(argv[optind], O_RDONLY))) {
		perror("open");
		return -1;
	}
	
	entry = load_exec(fd, type, flags);
	if(!entry) {
		fprintf(stderr, "load exec file failed\n");
		goto err;
	}

	/* flush all of cache */
	flush_cache(FLUSH_TYPE_CACHE, NULL);

	printf("Entry point: 0x%x\n", entry);

	/* 为后面的执行命令设置程序入口地址 */
	md_setpc((register_t)(long)entry);

	close(fd);

	return 0;

err:
	close(fd);
		
	return -1;
}

/* load命令的选项描述 */
static struct optdesc load_opts[] = {
	{
		.name = "-r", 
		.desc = "load raw file(.bin)",
	},
	{
		.name = "file", 
		.desc = "executable file path",
	},
	{},
};

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"Execute"}},			//表组名
	{
		{"load"}, 			
		"load executable file", 
		"[-r] file",
		load_opts, 
		cmd_load,
		1,
		2,
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
