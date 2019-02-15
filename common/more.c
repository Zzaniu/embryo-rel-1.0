
/**
 * Copyright(c) 2016-11-14 Shangwen Wu	
 *
 * linux more命令实现函数
 * 
 */
#include <stdio.h>
#include <string.h>

#define MORE_MSG		"more..."

static int getinp(int *lc, int sz);

/**
 * 描述：more命令实现函数，该函数没调用一次打印一条信息，并将lc递减
 * 参数: lc表示当前页剩余需要显示行数，pagesz表示一页最大可显示的有效行数，
 * 		 当pagesize为0时表示不再显示下一页
 * 返回：-1表示没有任何行可进行输出,退出分页显示
 */
int more(char *buf, int *lc, int pagesz)
{
	int ret; 
	if(0 == *lc) {						//当前页显示完
		if(0 == pagesz) 
			return -1;
		printf(MORE_MSG);			
		if(1 == (ret = getinp(lc, pagesz)))
			return -1;					//按下了q
		if(-1 == *lc) {					//查询模式
	
		} 
	}	
	printf("%s\n", buf);
	(*lc)--;
	return 0;
}

/**
 * 描述：more命令行模式获取输入
 * 参数：lc表示新页将要显示输出的行数，sz表示新页可显示输出的最大有效行数
 * 返回：接收到字符q时返回1，收到正确的命令时返回0，否则永不返回！
 */
static int getinp(int *lc, int sz)
{
	int c, i; 

	while(NULL == strchr("qn/ \n", (c = getchar()))) 
		;
	
	for(i = 0; i < strlen(MORE_MSG); ++i)
		printf("\b \b");
	switch(c) {
		case '\n':									//按下回车输出一行
			*lc = 1;	
		break;
		case ' ':									//按下空格输出最大more行
			*lc = sz;
		break;
		case '/':									//进入匹配
			*lc = -1;
		break;
		case 'n':
		break;
		case 'q':
			return 1;
	}

	return 0;
}

