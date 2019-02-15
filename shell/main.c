
/**
 * Copyright(c) 2015-10-16 Shangwen Wu	
 *
 * SHELL主循环 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <fs/termio.h>
#include <shell.h>
#include <console.h>
#include <autoconf.h>

extern void mach_showlogo(void);			//defined in machdep.c

struct termio consoleterm;		
char line_buf[MAX_LINE_SIZE] = {0};					//输入缓冲区

void enter_shell(void)
{
	mach_showlogo();

	ioctl(STDIN, TCGTERMIO, &consoleterm);			//保存默认的控制台终端配置
	printf("\nEnter embryo bios console.\n");
	main_loop();
}

/**
 * 描述：进入命令循环
 */
void main_loop(void)
{
	int len, ret;
	char *prompt = "[EM-Boot]: ";

	while(1) {
		printf("%s", prompt);
#if USE_SHELL_CONSOLE
		len = get_cmd(line_buf);
#else 
		len = get_line(line_buf, 0);
#endif
		do_cmd(line_buf);
		ioctl(STDIN, TCSTERMIO, &consoleterm);			//恢复默认的控制台终端配置
	}
}
