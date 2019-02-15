
/**
 * Copyright(c) 2016-11-14 Shangwen Wu	
 *
 * SHELL相关兼容命令 
 * 
 */

#include <common.h>
#include <stdio.h>
#include <string.h>
#include <command.h>
#include <fs/termio.h>

#define CMD_MORE_MAX_LINE			10

extern struct cmd *cmds_list[]; 					//defined in command.c
extern int more(char *buf, int *lc, int pagesz);	//defined in more,c

/**
 * 描述：打印命令详细信息
 * 返回：-1表示结束信息显示（按下q退出）
 */
static int printhelp(struct cmd *cmdinfo, int *lc, int pagesz, int maxname, int maxopts)
{
	int i = 0;
	char linebuf[MAX_CMD_BUF_SIZE] = {0};
	struct optdesc *opts = NULL; 

	sprintf(linebuf, "%*s %-*s     %s", maxname, cmdinfo->name.cmdname, maxopts, cmdinfo->optname, cmdinfo->cmddesc);
	
	if(more(linebuf, lc, pagesz))
		return -1;
	
	if(opts = cmdinfo->opts) {
		while(opts->name) {
			sprintf(linebuf, "%*s %-*s     %s", maxname, " ", maxopts, opts->name, opts->desc);
			if(more(linebuf, lc, pagesz))
				return -1;
			opts++;
		}
	}

	return 0;
}

/**
 * 描述：执行help命令
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_help(int argc, const char *argv[])
{
	int i, cmdlistno, lc, pagesz, maxname, maxopts, maxdesc;
	char cmdbuf[MAX_CMD_BUF_SIZE] = {0}, oldgrp[MAX_CMD_BUF_SIZE] = "";
	struct cmd *cmdtable = NULL;

	maxname = maxopts = maxdesc = 0;
	
	for(cmdlistno = 0; cmdlistno < CMD_MAX_NUM && cmds_list[cmdlistno]; ++cmdlistno) {
		cmdtable = cmds_list[cmdlistno] + 1;
		while(cmdtable->name.cmdname) {
			if(strlen(cmdtable->name.cmdname) > maxname)
				maxname = strlen(cmdtable->name.cmdname);
			if(strlen(cmdtable->optname) > maxopts)
				maxopts = strlen(cmdtable->optname);
			if(strlen(cmdtable->cmddesc) > maxdesc)
				maxdesc = strlen(cmdtable->cmddesc);
			cmdtable++;
		}
	}

	ioctl(STDIN, TCSBREAK, NULL);
	lc = pagesz = CMD_MORE_MAX_LINE;
	if(argc >= 2) {					
		if(!strcmp("*", argv[1])) {						//显示所有命令详细信息
			for(cmdlistno = 0; cmdlistno < CMD_MAX_NUM && cmds_list[cmdlistno]; ++cmdlistno) {
				cmdtable = cmds_list[cmdlistno] + 1;
				while(cmdtable->name.cmdname) {
					if(!(cmdtable->flags & CMD_FLAGS_DISABLE)) {
						if(printhelp(cmdtable, &lc, pagesz, maxname, maxopts))
							return 0;
					}
					cmdtable++;
				}
			}
		} else {										//显示指定命令详细信息
			for(i = 1; i < argc; ++i) {
				for(cmdlistno = 0; cmdlistno < CMD_MAX_NUM && cmds_list[cmdlistno]; ++cmdlistno) {
					cmdtable = cmds_list[cmdlistno] + 1;
					while(cmdtable->name.cmdname) {
						if(!strcmp(cmdtable->name.cmdname, argv[i]))
							break;
						cmdtable++;
					}
					if(cmdtable->name.cmdname){
						if(!(cmdtable->flags & CMD_FLAGS_DISABLE))
							if(printhelp(cmdtable, &lc, pagesz, maxname, maxopts))
								return 0;
						break;
					}
				}
				if(cmdlistno >= CMD_MAX_NUM || !cmds_list[cmdlistno]) 
					printf("%s: command not found\n", argv[i]);
			}
		}
	} else {											//显示所有命令的概要信息
		for(cmdlistno = 0; cmdlistno < CMD_MAX_NUM && cmds_list[cmdlistno]; ++cmdlistno) {
			cmdtable = cmds_list[cmdlistno] + 1;
			if(strcmp(cmds_list[cmdlistno]->name.grpname, oldgrp)){
				sprintf(cmdbuf, "%~*s", maxname + maxdesc + 4, cmds_list[cmdlistno]->name.grpname);
				if(more(cmdbuf, &lc, pagesz))
					return 0;
				strcpy(oldgrp, cmds_list[cmdlistno]->name.grpname);
			}
			while(cmdtable->name.cmdname) {
				if(!(cmdtable->flags & CMD_FLAGS_DISABLE)) {
					sprintf(cmdbuf, "%*s    %-*s", maxname, cmdtable->name.cmdname, maxdesc, cmdtable->cmddesc);
					if(more(cmdbuf, &lc, pagesz))
						return 0;
				}
				cmdtable++;
			}
		}
	}	

	return 0;
}

/* help命令的选项描述 */
static struct optdesc help_opts[] = {
	{
		.name = "*", 
		.desc = "lookup all of commands in detail",
	},
	{
		.name = "cmd", 
		.desc = "lookup specific command in detail",
	},
	{},
};

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"Shell"}},			//表组名
	{
		{"help"}, 			
		"command helper", 
		"[*|cmd...]",
		help_opts, 
		cmd_help,
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
