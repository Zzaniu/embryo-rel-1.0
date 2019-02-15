
/**
 * Copyright(c) 2016-11-18 Shangwen Wu	@wsw 26th years 
 *
 * 命令参数列表解析函数，注意该函数的解析对目标命令行以及主调函数均
 * 有一定要求，其中目标命令必须遵循参数选项位于必须参数之前，并且选
 * 项必须以"-"打头，否则选项参数将不被识别；主调函数必须在解析之前
 * 将poptind初始化为0，否则解析出错 
 */

#include <common.h>
#include <stdio.h>
#include <string.h>
#include <fs/termio.h>

#define BADOPT		((int)'?')

int prnterr = 1;						//是否打印出错信息	

/**
 * 描述：获取参数函数
 * 参数：argc，最大参数个数；argv[]参数值数组；poptind，当前参数在argv的索引，注意这个参数由getopt负责更新
 * 		 ，并且初次调用时，该值必须为0；optval，当检测到在partter中符合的选项参数并且该选项后面将接着一个选
 * 		 项值时，将该指针指向该选项的值，否则为NULL；pattern，选项参数名的集合，若某个选项后面需要一个参数值
 * 		 时，用一个特殊字符':'标记该选项，以便程序检测
 * 返回：当检测到以'-'打头并且能够找到pattern中对应选项名时，返回该选项名字符，若是未检测到'-'或者poptind到
 * 		 达argc指定个数时，将返回EOF表示解析结束，若是检测到非法字符或者非法格式的参数格式，将返回BADOPT
 *
 */
int getopt(int argc, const char *argv[], int *poptind, \
		char **optval, const char *pattern)
{
	char *pos = NULL;								//指向pattern中已经匹配上的选项位置
	static char *place = NULL;						//指向参数的当前解析位置
	
	if(0 == *poptind) {
		place = "";
		*poptind = 1;
	}

	if('\0' == *place) {			//begin
		if(*poptind >= argc || *(place = (char *)argv[*poptind]) != '-')	//当poptind超过参数总个数或没以‘-’打头时退出 
			return EOF;
		place++;
	}
	if(!*place || NULL == (pos = strchr(pattern, *place++)) || ':' == pos[0]) {
		if(prnterr)
			fprintf(stderr, "%s: illegal option -- %c\n", *argv, *(place - 1));
		return BADOPT;
	}
	*optval = NULL; 
	if(!*place) {
		if(':' == pos[1]) {
			if(++*poptind < argc)
				*optval = (char *)argv[*poptind];
			else{
				if(prnterr)
					fprintf(stderr, "%s: option requires an argument -- %c\n", *argv, *(place - 1));
				return BADOPT;
			} 
		}
		place = "";	
	} else if(':' == pos[1]) {
		if(prnterr)
			fprintf(stderr, "%s: option requires an argument -- %c\n", *argv, *(place - 1));
		return BADOPT;
	}
	(*poptind)++;
	return pos[0];
}

