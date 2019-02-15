
/**
 * Copyright(c) 2015-10-16 Shangwen Wu	
 *
 * 命令相关函数 
 * 
 */

#include <common.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <command.h>

//全局命令描述保存列表
struct cmd *cmds_list[CMD_MAX_NUM] = {0};

/**
 * 描述：添加一组命令
 * 参数：cmds表示要添加的多个命令，tail表示添加位置是否是当前命令所在组的末尾
 * 
 */
void add_cmds(struct cmd *cmds, int tail)
{
	int i;
	
	if(!cmds || !cmds->name.grpname)	//丢弃首元素名称为空的命令
		return;
	for(i = 0; i < CMD_MAX_NUM; ++i) {
		if(NULL == cmds_list[i]) {
			cmds_list[i] = cmds;
			return;
		}
		if(!strcmp(cmds->name.grpname, cmds_list[i]->name.grpname))
			break;
	}
	if(i >= CMD_MAX_NUM)
		return;							//溢出
	if(tail) {
		while(i < CMD_MAX_NUM && cmds_list[i] 
			&& !strcmp(cmds->name.grpname, cmds_list[i]->name.grpname))
			++i;
	}

	bcopy(&cmds_list[i], &cmds_list[i + 1], sizeof(struct cmd *) * (99 - i));
	cmds_list[i] = cmds;
}

/**
 * 描述：解析参数，空格符以及其他控制字符作为分割参数的界定符
 * 参数：输入cmd，表示需要进行解析的命令行，输出av[]，表示解析好的参数数组
 * 返回：返回参数个数
 */
int argparser(char *cmd, char *av[])
{
	int ac;
	char *p = cmd, c; 

	for(ac = 0; ac < MAX_CMD_ARG_NUM; ++ac) {
		while(*p && *p <= ' ')		//去掉开始的控制字符和空格
			++p;
		if(!*p)						//解析结束
			break;
		av[ac] = p;
		while(*p && *p > ' ') {		//当*P为'\0'或者空格时表示一个参数到达边界
			c = *p;
			if('\''== c || '"' == c) {	//跳过引号内的空格字符
				strdchr(p);
				while(*p && *p != c)
					++p;
				if(!*p)				//非平衡的引号情况，提前结束
					return ac + 1;
				strdchr(p);
			} else
				++p;
		}
		if(!*p)						//解析结束
			return ac + 1;
		*p++ = '\0';
	}

	return ac;
}

/**
 * 描述：命令字符串解析，并调用对应命令的执行函数
 * 返回：返回命令执行结果
 */
int do_cmd(char *cmd)
{
	int ac, nc = 0, i, j, k, state = -1;
	char c, *p = cmd;
	char *cmdlist[MAX_CMD_IN_LINE];
	char *av[MAX_CMD_ARG_NUM];
	struct cmd *cp = NULL;

	if(!cmd || strempty(cmd)) 
		return -1;

	//检查引号是否平衡以及分割多条命令
	cmdlist[nc++] = p;
	while(*p) {
		c = *p;
		if('\''== c || '"' == c) {
			while(*++p && *p != c)
				;
			if(!*p) {
				fprintf(stderr, "Syntax error, unbalanced quote!\n");
				return -1;
			}
			++p;
		} else if(';' == c) {			//一行输入用;号区分多条命令
			*p++ = '\0';
			if(nc >= MAX_CMD_IN_LINE)	//防止";"过多引起越界
				break;
			cmdlist[nc++] = p;
		} else 
			++p;
	}

	//查找以及执行命令
	for(i = 0; i < nc; ++i) {
		ac = argparser(cmdlist[i], av);
		if(ac > 0) {
			state = -2;
			//printf("cmd: %s, argc: %d\n", av[0], ac);
			for(j = 0; cmds_list[j]; ++j) {
				cp = cmds_list[j];
				for(k = 1; cp[k].name.cmdname; ++k) 
					if(!strcmp(av[0], cp[k].name.cmdname)) 
						break;			//找到命令
				if(cp[k].name.cmdname) 
					break;
			}
			if(!cmds_list[j] || (cp[k].flags & CMD_FLAGS_DISABLE)) {
				fprintf(stderr, "%s: command not found\n", av[0]);
			} else {
				if(cp[k].cmdexec) { 
					if(ac < cp[k].minac) 
						fprintf(stderr, "%s: not enough arguments\n", av[0]);
					else if(ac > cp[k].maxac) 
						fprintf(stderr, "%s: too many arguments\n", av[0]);
					else
						state = (*cp[k].cmdexec)(ac, (const char **)av);
					if(-2 == state)
						fprintf(stderr, "usage: %s %s\n", cp[k].name.cmdname, cp[k].optname);
				}
				else
					fprintf(stderr, "%s: not have any handler\n", av[0]);
			}
		}
	}

	return state;
}
