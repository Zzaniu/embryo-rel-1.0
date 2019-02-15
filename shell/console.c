
/**
 * Copyright(c) 2016-08-25 Shangwen Wu	
 *
 * 控制台相关代码 
 * 
 */
#include <stdio.h>
#include <ctype.h>
#include <cvt.h>
#include <string.h>
#include <fs/termio.h>
#include <unistd.h>
#include <fcntl.h>
#include <history.h>
#include <console.h>
#include <autoconf.h>

static int cursor = 0;				//当前光标的位置
static char esc_to_ctl[] = {PREV, NEXT, FORW, BACK, 'E', END, 'G', BEGIN};

static int left(char *buf, int n)
{
	int i = 0; 

	while(n-- && cursor > 0) {
		putchar('\b');
		--cursor;
		++i;
	}
	
	return i;
}

static int right(char *buf, int n)
{
	int i = 0; 

	while(n-- && cursor < strlen(buf)){
		putchar(buf[cursor]);
		++cursor;
		++i;
	}

	return i;
}

static int puts_cur(const char *s)
{
	int n;

	n = puts(s);
	cursor += n;

	return n;	
}

/**
 * 描述：获取一个字符串中的单词
 * 返回：返回一个单词之后的首地址，返回NULL表示
 */
char *get_word(char *dst, const char *buf)
{
	const char *a, *p = buf;

	if(!dst || !buf)
		return NULL;
	
	while(isspace(*p))
		p++;
	if(!*p)
		return NULL;		//空串
	
	a = p;

	while(*a && !isspace(*a))
		a++;
	
	dst[0] = '\0';			//当a-p==0时，dst为空串
	strncpy(dst, p, a - p);
	dst[a - p] = '\0';			

	return (char *)a;
}

#if USE_SHELL_CONSOLE
/**
 * 描述：获取标准输入和串口输入的一行数据，使用shell控制台时，
 *       将提供丰富的历史记录以及行编辑功能
 * 参数：buf，输入缓冲区，usehist，是否启动上下键打开历史命令
 */
int get_line(char *buf, int usehist)
{
	int c, oc, i, n;
	struct termio t;
	enum esc_state esc_state = NONE;
	char *hist = NULL;
	char bak[MAX_LINE_SIZE];

	ioctl(STDIN, TCSNCANON, &t);

	cursor = 0;

	while(1) {
		c = getchar();
		switch(esc_state) {
		case NONE:
			if('\033' == c) {
				esc_state = HAVE_ESC;	
				continue;
			}
		break;
		case HAVE_ESC:
			if('[' == c) {
				esc_state = HAVE_LB;	
				continue;
			} 
			esc_state = NONE;
		break;
		case HAVE_LB:
			if(c >= 'A' && c <= 'H') {
				c = esc_to_ctl[c - 'A'];
			}
		default:
			esc_state = NONE;
		}
		if(PREV == c && usehist) {
			hist = get_last_cmd();
			if(hist != NULL) {
				right(buf, strlen(buf) - cursor);
				for(i = 0; i < strlen(buf); ++i) 
					puts("\b \b");
				cursor = 0;
				if(pos_is_latest())
					strcpy(bak, buf);
				strcpy(buf, hist);
				puts_cur(buf);
			} 
		} else if(NEXT == c && usehist) {
			if(!pos_is_new()) {
				right(buf, strlen(buf) - cursor);
				for(i = 0; i < strlen(buf); ++i)
					puts("\b \b");
				cursor = 0;
				hist = get_next_cmd();
				if(hist != NULL) {
					strcpy(buf, hist);
				} else if(pos_is_latest()) {	//回到原始输入命令
					strcpy(buf, bak);
					pos_set_new();
				} else 
					;					//error!!!, bad codes 
				puts_cur(buf);
			}
		} else if(FORW == c) {
			right(buf, 1);				//光标往前移动一格
		} else if(BACK == c) {
			left(buf, 1);
		} else if(BEGIN	== c) {
			left(buf, cursor);
		} else if(END == c) {
			right(buf, strlen(buf) - cursor);
		} else if(DELETE == c) {
			strdchr(buf + cursor);
			oc = cursor;
			puts_cur(buf + cursor);
			puts_cur(" ");
			left(buf, strlen(buf + oc) + 1);
		} else if(DELLINE == c) {
			n = strlen(buf + cursor);
			buf[cursor] = '\0';
			oc = cursor;
			for(i = 0; i < n; ++i)
				puts_cur(" ");
			left(buf, n);
		} else if('\b' == c || 0x7f == c) {
			if(1 == left(buf, 1)) {
				strdchr(buf + cursor);
				oc = cursor;
				puts_cur(buf + cursor);
				puts_cur(" ");
				left(buf, strlen(buf + oc) + 1);
			}
		} else if('\n' == c || '\r' == c) {
			putchar(c);
			break;
		} else if(isprint(c)){
			if(strlen(buf) < MAX_LINE_SIZE - 1) {
				strichr(buf + cursor, c);
				oc = cursor + 1;
				puts_cur(buf + cursor);
				left(buf, strlen(buf + oc));
			}
		} else 
			putchar('\a');			//bell beep!!!
	}
	
	pos_set_new();					//按下回车后将当前命令指针回到最新位置

	ioctl(STDIN, TCSTERMIO, &t);

	return strlen(buf);
}

/**
 * 描述：提取命令，处理特殊字符"!"
 * 返回：命令解析后的长度
 */
int get_cmd(char *buf)
{
	int n, i;
	unsigned long no = 0;
	char *bp = NULL, *rp, *cp, word[MAX_LINE_SIZE];

	*buf = '\0';
	n = get_line(buf, 1);

	rp = buf;
	while(bp = find_bang(rp)) {		
		rp = get_word(word, bp);	//rp指向"!单词"之后的位置
		strdchr(word);				//删除特殊字符"!"
		if(!strempty(word)) {
			if(0 == strcmp(word, "!")) 	//重复上一条命令
				cp = read_last_cmd();		
			else if(!atob(&no, word, 10))
				cp = find_hist_by_num(no);
			else 
				cp = find_hist_by_prefix(word);

			if(cp != NULL) { 
				for(i = 0; i < strlen(word) + 1; ++i)
					strdchr(bp);		//删除“！”以及跟随的单词
				stristr(bp, cp);		//插入替换的历史命令
				rp = rp - (strlen(word) + 1) + strlen(cp);	//调整rp位置
			} else {
				printf("!%s: event not found\n", word);
				*buf = '\0';		//这条命令将不会被添加到历史命令表中
				break;
			}
		}
	}
	
	add_hist_cmd(buf);

	return strlen(buf);
}
#else
int get_line(char *buf, int usehist)
{
	int n;

	if(buf) {
		n = read(STDIN, buf, MAX_LINE_SIZE);
		if(n > 0)
			buf[--n] = '\0';				//去掉换行符或最后一个字节
		return n;
	} else
		return -1;
}
#endif


