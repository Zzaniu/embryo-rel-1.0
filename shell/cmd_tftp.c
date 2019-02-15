
/**
 * Copyright(c) 2018-9-6 Shangwen Wu	
 *
 * tftp客户端命令实现 
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <command.h>
#include <fcntl.h>
#include <unistd.h>
#include <fs/termio.h>
#include <arpa/tftp.h>

extern int getopt(int argc, const char *argv[], int *optind, \
		char **optval, const char *pattern);				//defined in getopt.c
extern int get_rsa(unsigned long *vp, const char *p);

/**
 * 描述：执行tftp命令
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_tftp(int argc, const char *argv[])
{
	int fd, ret, optind, oflags = 0, accmode;
	ulong total = 0, addr;
	char opt, *optval = NULL;
	char buf[TFTPBLKSIZE], path[64] = "tftp://";
	char *file, *host;

	optind = 0;
	while((opt = getopt(argc, argv, &optind, &optval, "p:g:")) != EOF) {
		switch(opt) {
			case 'p':
				oflags |= O_WRONLY;
				file = optval;
			break;
			case 'g':
				oflags |= O_RDONLY;
				file = optval;
			break;
			default:							//BADOPT
				return -2;
		}
	}

	if((oflags != O_WRONLY) && (oflags != O_RDONLY)) {
		fprintf(stderr, "%s: no operation specified\n", *argv);
		return -2;
	} else if((O_WRONLY | O_RDONLY) == oflags) {
		fprintf(stderr, "%s: multiple accesss mode specified\n", *argv);
		return -2;
	} 

	if(optind >= argc) {
		fprintf(stderr, "%s: need specify tftp server\n", *argv);
		return -2;
	}
	printf("file=[%s]\n", file);

	host = (char *)argv[optind];
	printf("host=[%s]\n", host);
	if(++optind >= argc) {
		fprintf(stderr, "%s: need specify memory address\n", *argv);
		return -2;
	}
	if(get_rsa(&addr, argv[optind])) {
		fprintf(stderr, "%s: illegal modify address -- %s\n", *argv, argv[optind]);
		return -2;
	}
	printf("adress=[0x%x]\n", addr);

	if(strlen(path) + strlen(host) + strlen(file) + 1 >= sizeof(path)) {
		fprintf(stderr, "%s: path too long\n", *argv);
		return -2;
	}
	/* tftp:// + host + / + file */
	strcat(strcat(strcat(path, host), "/"), file);
	printf("path=[%s]\n", path);

	if(-1 == (fd = open(path, oflags))) {
		perror("open");
		return -2;
	}

	if(O_RDONLY == (oflags & O_ACCMODE)) {
		while(1) {
			if((ret = read(fd, buf, TFTPBLKSIZE)) < 0) {
				perror("read");
				goto err;	
			}
			memcpy((char *)addr, buf, ret);
			addr += ret;
			total += ret;
			if(ret < TFTPBLKSIZE) {
				printf("read complete, total receive bytes: %lu\n", total);	
				break;
			}
		}
	} else if(O_WRONLY == (oflags & O_ACCMODE)) {
		memcpy(buf, (char *)addr, TFTPBLKSIZE);
		if((ret = write(fd, buf, TFTPBLKSIZE)) < 0) {
			perror("write");
			goto err;	
		}
		total += ret;
		printf("write complete, total send bytes: %lu\n", total);	
	}

	close(fd);

	return 0;

err:
	close(fd);

	return -2;
}

static struct optdesc tftp_opts[] = {
	{
		.name = "-g|p", 
		.desc = "get/put file",
	},
	{
		.name = "file", 
		.desc = "remote file name",
	},
	{
		.name = "host", 
		.desc = "tftp server host",
	},
	{
		.name = "address", 
		.desc = "local memory address",
	},
	{},
};

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"Network"}},			//表组名
	{
		{"tftp"}, 			
		"tftp client command", 
		"-g|p file host address",
		tftp_opts, 
		cmd_tftp,
		5,
		5,
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
