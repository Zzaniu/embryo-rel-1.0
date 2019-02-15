/**
 * Copyright(c) 2016-11-13 Shangwen Wu	
 *
 * 系统杂项命令集合
 * 
 */

#include <common.h>
#include <stdio.h>
#include <command.h>

extern void mach_warmreset(void);			//defined in machdep.c
extern void mach_shutdown(void);			//defined in machdep.c

/**
 * 描述：reboot命令处理函数
 */
static int cmd_reboot(int argc, const char *argv[])
{
	printf("System reboot...\n");
	mach_warmreset();
	while(1)
		;

	return 0;
}

/**
 * 描述：poweroff/halt命令处理函数
 */
static int cmd_poweroff(int argc, const char *argv[])
{
	printf("System poweroff...\n");
	mach_shutdown();
	while(1)
		;
	return 0;
}

/* misc命令组结构描述 */
static struct cmd cmds[] = {
	{{"Misc"}},
	{
		{"reboot"},
		"reboot system",
		"",
		NULL,
		cmd_reboot,
		0,
		MAX_CMD_ARG_NUM - 1,
		0,
	},
	{
		{"poweroff"},
		"halt system",
		"",
		NULL,
		cmd_poweroff,
		0,
		MAX_CMD_ARG_NUM - 1,
		0,
	},
	{
		{"shutdown"},
		"halt system",
		"",
		NULL,
		cmd_poweroff,
		0,
		MAX_CMD_ARG_NUM - 1,
		0,
	},
	{
		{"halt"},
		"halt system",
		"",
		NULL,
		cmd_poweroff,
		0,
		MAX_CMD_ARG_NUM - 1,
		0,
	},
	{},
};

/**
 * 描述：命令初始化函数
 */
static void __attribute__((constructor)) init_cmd(void)
{
	add_cmds(cmds, 1);
}
