
/**
 * Copyright(c) 2016-3-5 Shangwen Wu	
 *
 * 命令相关定义
 * 
 */

#ifndef __COMMAND_H__
#define __COMMAND_H__

/* 命令参数描述 */
typedef struct optdesc {
	char *name;			
	char *desc;
}Optdesc;

typedef int (*cmd_func_t)(int argc, const char *argv[]);//命令执行函数格式

/* 命令描述 */
typedef struct cmd {
	union {
		char *cmdname;				//命令名称
		char *grpname;				//当该结构体表示一个组时，该成员为组名
	}name;
	char *cmddesc;					//命令描述
	char *optname;					//参数名
	struct optdesc *opts;			//参数描述
	cmd_func_t cmdexec;				//命令执行函数
	int	minac;						//最小参数个数
	int maxac;						//最大参数个数
	int flags;						//命令标识
#define CMD_FLAGS_DISABLE	1		//命令是否可用
#define CMD_FLAGS_REPEAT	2		//命令是否可以重复
}Cmd;

#define CMD_MAX_NUM			100		//当前bios最大容纳的命令数
#define MAX_CMD_IN_LINE		10		//一行输入最大容纳的命令个数
#define MAX_CMD_ARG_NUM		20		//一个命令最大容纳的参数个数
#define MAX_CMD_BUF_SIZE	256		//容纳一个命令的缓冲区大小

extern void add_cmds(struct cmd *cmds, int tail);
extern int do_cmd(char *cmd);
extern int argparser(char *cmd, char *av[]);

#endif //__COMMAND_H__
