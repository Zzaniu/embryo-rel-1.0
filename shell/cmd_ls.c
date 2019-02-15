
/**
 * Copyright(c) 2018-11-28 Shangwen Wu	
 *
 * 伪ls命令
 * 
 */

#include <common.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/list.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <command.h>
#include <sys/stat.h>
#include <fs/file.h>
#include <fs/termio.h>

extern int getopt(int argc, const char *argv[], int *optind, \
		char **optval, const char *pattern);				//defined in getopt.c

/**
 * 描述：执行ls命令
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_ls(int argc, const char *argv[])
{
	int optind;
	char opt, *optval = NULL, *cp;
	int opt_a = 0, opt_l = 0, fd;
	struct stat st; 
	DIR *dp;
	struct dirent *ent;
	int width = 10;

	optind = 0;
	while((opt = getopt(argc, argv, &optind, &optval, "al")) != EOF) {
		switch(opt) {
			case 'a':
				opt_a = 1;
			break;
			case 'l':
				opt_l = 1;
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
		perror("ls");
		return -1;
	}
	
	if(!S_ISDIR(st.st_mode)) {	/* 非目录文件仅显示文件名 */
		cp = strlchr(argv[optind], '/');
		printf("%s\n", ++cp);
		return 0;
	}

	if(NULL == (dp = opendir(argv[optind]))) {
		perror("ls");
		return -1;
	}
	
	/* 遍历目录项 */
	while((ent = readdir(dp)) != NULL) {
		if(!opt_a && '.' == ent->d_name[0]) 
			continue;
		printf("%-*s\n", width, ent->d_name);
	}

	if(closedir(dp)) {
		perror("ls");
		return -1;
	}

	return 0;
}

/* ls命令的选项描述 */
static struct optdesc ls_opts[] = {
	{
		.name = "-a", 
		.desc = "do not ignore entries starting with .",
	},
	{
		.name = "-l", 
		.desc = "use a long listing format",
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
		{"ls"}, 			
		"list directory contents", 
		"[-a|l] file",
		ls_opts, 
		cmd_ls,
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
