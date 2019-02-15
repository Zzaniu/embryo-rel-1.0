
/**
 * Copyright(c) 2016-08-26 Shangwen Wu	
 *
 * 历史命令相关代码 
 *
 */
#include <stdlib.h>
#include <string.h>
#include <history.h>

static int hist_cur = 0;			//记录当前命令在历史表中的位置
static int hist_new = 0;			//记录下一个新的命令将要插入的位置
static int hist_old	= 0;			//记录当前最久的一条命令保存的位置
static int hist_no	= 0;			//记录当前最新历史命令的编号

static char * hist_tab[HIST_MAX_NUM] = {NULL};

/**
 * 描述：添加一条命令到历史
 * 返回：成功返回0，添加失败返回-1
 */
int add_hist_cmd(const char *cmd)
{
	char *last, *new; 

	if(strempty(cmd))
		return 0;

	last = hist_tab[HIST_LAST(hist_new)];
	if(last && 0 == strcmp(cmd, last))	//是否与上一条命令重复
		return 0;
			
	if(NULL == (new = (char *)malloc(strlen(cmd))))
		return -1;

	strcpy(new, cmd);

	if(hist_new == hist_old) {			//新插入的位置为最老命令所在的位置
		if(hist_tab[hist_old]) {
			free(hist_tab[hist_old]);	//覆盖最老位置
			hist_old = HIST_NEXT(hist_old);	//更新最老位置
		}								//当old位置的字符为空时，什么也不做 
	}
	hist_tab[hist_new] = new;
	hist_cur = hist_new = HIST_NEXT(hist_new);//新插入一条命令需要更新当前位置
	hist_no++;

	return 0;
}

/**
 * 描述：获取当前位置的上一条命令，并且当前位置往回移动一个位置
 * 返回：返回上一条命令，当上一条命令为空或者到达最老的命令时返回NULL
 */
char *get_last_cmd(void)
{	
	char *last;

	if(hist_cur == hist_old)
		return NULL;

	last = hist_tab[HIST_LAST(hist_cur)];
	if(NULL == last)
		return NULL;

	hist_cur = HIST_LAST(hist_cur);	

	return last;
}

/**
 * 描述：获取当前位置的上一条命令
 * 返回：返回上一条命令，当上一条命令为空或者到达最老的命令时返回NULL
 */
char *read_last_cmd(void)
{	
	char *last;

	if(hist_cur == hist_old)
		return NULL;

	last = hist_tab[HIST_LAST(hist_cur)];
	if(NULL == last)
		return NULL;

	return last;
}

/**
 * 描述：获取当前位置的下一条命令位置，并且当前位置往前移动一个位置
 * 返回：返回下一条命令，当下一条命令为空或者到达最新的命令时返回NULL
 */
char *get_next_cmd(void)
{	
	char *next; 

	if(HIST_NEXT(hist_cur) == hist_new)
		return NULL;

	next = hist_tab[HIST_NEXT(hist_cur)];
	if(NULL ==  next)
		return NULL;

	hist_cur = HIST_NEXT(hist_cur);	

	return next;
}

/**
 * 描述：获取当前位置的下一条命令
 * 返回：返回下一条命令，当下一条命令为空或者到达最新的命令时返回NULL
 */
char *read_next_cmd(void)
{	
	char *next; 

	if(HIST_NEXT(hist_cur) == hist_new)
		return NULL;

	next = hist_tab[HIST_NEXT(hist_cur)];
	if(NULL ==  next)
		return NULL;

	return next;
}

/**
 * 描述：当前位置是否是最近的历史命令所在位置
 * 返回：满足条件返回1，否则返回0
 */
int pos_is_latest(void)
{
	return HIST_NEXT(hist_cur) == hist_new ? 1 : 0;
}

/**
 * 描述：当前位置是否是下一个最新命令将要插入的位置
 * 返回：满足条件返回1，否则返回0
 */
int pos_is_new(void)
{
	return hist_cur == hist_new ? 1 : 0;
}

/**
 * 描述：设置当前命令所在位置为下一个新命令将要插入的位置
 */
void pos_set_new(void)
{
	hist_cur = hist_new;
}

/**
 * 描述：查找历史命令替换符"!"
 * 返回：查找成功返回"!"位置指针，否则返回NULL
 */
char *find_bang(const char *str)
{
	char *p;

	for(p = (char *)str; *p; ++p) {
		if('\'' == *p || '"' == *p)     //在引号内的"!"不做特殊处理 
			while(*++p && ('\'' != *p && '"' != *p))
				;
		else if('!' == *p) 
			return p;
	}

	return NULL;
}

/**
 * 描述：根据字符串前缀查找最近的一个匹配命令
 * 返回：
 */
char *find_hist_by_prefix(const char *pfx)
{
	int i =  HIST_LAST(hist_new);

	while(i != HIST_LAST(hist_old)) {
		if(!hist_tab[i])
			break;
		if(0 == strncmp(hist_tab[i], pfx, strlen(pfx)))
			return hist_tab[i];
		i = HIST_LAST(i);
	}

	return NULL;
}

/**
 * 描述：根据历史编号查找最近的一个匹配命令
 * 返回：
 */
char *find_hist_by_num(int num)
{
	int pos = num - 1;

	if(num > hist_no || num <= hist_no - HIST_MAX_NUM)
		return NULL;
	
	if(num > HIST_MAX_NUM)
		pos -= HIST_MAX_NUM;
	
	return hist_tab[pos];
}

