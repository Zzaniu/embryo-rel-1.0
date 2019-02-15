
/**
 * Copyright(c) 2016-08-25 Shangwen Wu	
 *
 * 控制台相关头文件 
 * 
 */
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <fs/termio.h>

#define PREV		CTRL('p')
#define NEXT		CTRL('n')
#define FORW		CTRL('f')
#define BACK		CTRL('b')
#define BEGIN		CTRL('a')
#define END			CTRL('e')
#define DELETE		CTRL('d')
#define DELLINE		CTRL('k')
#define MARK		CTRL(' ')
#define KILL		CTRL('w')

enum esc_state {
	NONE,
	HAVE_ESC,
	HAVE_LB
};

#define MAX_LINE_SIZE			256			//一行shell输入的最大长度

extern int get_line(char *buf, int usehist);
extern int get_cmd(char *buf);

#endif //__CONSOLE_H__

