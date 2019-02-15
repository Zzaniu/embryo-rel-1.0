
/**
 * Copyright(c) 2018-12-30 Shangwen Wu	
 *
 * cat命令
 * 
 */

#include <common.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/list.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <command.h>
#include <sys/stat.h>
#include <fs/file.h>
#include <fs/termio.h>

#define MAX_READ	512

extern int getopt(int argc, const char *argv[], int *optind, \
		char **optval, const char *pattern);				//defined in getopt.c

/**
 * 描述：执行cat命令
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_cat(int argc, const char *argv[])
{
	int optind, i;
	char opt, *optval = NULL, *cp;
	int opt_n = 0, fd;
	struct stat st; 
	char buf[MAX_READ];
	ssize_t res;

	optind = 0;
	while((opt = getopt(argc, argv, &optind, &optval, "n")) != EOF) {
		switch(opt) {
			case 'n':
				opt_n = 1;
			break;
			default:							//BADOPT
				return -2;
		}
	}

	if(optind >= argc) {
		fprintf(stderr, "%s:  missing file operand\n", *argv);
		return -2;
	}

	/* 文件名必须以/dev/fs/打头 */
	if(!strprefix(argv[optind], FSIO_PREFIX)) {
		fprintf(stderr, "%s:  file's prefix is not \"/dev/fs\"\n", *argv);
		return -1;
	}

	/* 检查文件是否为目录 */
	if(-1 == stat(argv[optind], &st)) {
		perror("cat");
		return -1;
	}
	
	if(S_ISDIR(st.st_mode)) {	/* 目录文件不支持cat命令 */
		errno = EISDIR;
		perror("cat");
		return -1;
	}

	if(-1 == (fd = open(argv[optind], O_RDONLY))) {
		perror("cat");
		return -1;
	}
	
	/* 设置文件偏移为开始位置 */
	llseek(fd, 0, SEEK_SET);
	do {
		if((res = read(fd, buf, MAX_READ)) < 0) {
			perror("cat");
			return -1;
		}
		if(res > 0) {
			for(i = 0; i < res; ++i)
				printf("%c", buf[i]);
		}
	} while(res);

	close(fd);

	return 0;
}

/* cat命令的选项描述 */
static struct optdesc cat_opts[] = {
	{
		.name = "-n", 
		.desc = "number all output lines",
	},
	{
		.name = "file", 
		.desc = "target file",
	},
	{},
};

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"Shell"}},			//表组名
	{
		{"cat"}, 			
		"concatenate files and print on the standard output", 
		"[-n] file",
		cat_opts, 
		cmd_cat,
		2,
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
