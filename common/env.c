
/**
 * Copyright(c) 2016-3-7 Shangwen Wu	
 *
 * embryo环境变量功能函数
 * 
 */
#include <common.h>
#include <stdlib.h>
#include <string.h>
#include <env.h>
#include <mach/early_log.h>

extern int mach_mapenv(int (*func)(const char *name, const char *val));		//defined in machdep.c 

static struct envpair envar[NVAR];

/**
 * 描述：设置环境变量，当name已存在时，将修改成新设置的值，当name不存在时，、
 * 将新添加一个环境变量，注意该函数并不操作物理FLASH，而仅操作内存的env键值对
 */
int _setenv(const char *name, const char *val)
{
	int i;
	struct envpair *ep = NULL;

	for(i = 0; i < NVAR; ++i) {
		if(!envar[i].name && !ep)
			ep = envar + i;
		else if(0 == strcmp(envar[i].name, name))
			break;
	}
	if(i < NVAR) {
		//name已经存在，更新该环境变量的值
		if(envar[i].val) 
			free(envar[i].val);
		if(NULL == (envar[i].val = malloc(strlen(val) + 1)))
			return -1;
		strcpy(envar[i].val, val);
	} else if(ep != NULL){
		//name不存在，设置一个新的环境变量
		if(NULL == (ep->name = malloc(strlen(name) + 1)))
			return -1;
		strcpy(ep->name, name);
		if(ep->val) 
			free(ep->val);
		if(NULL == (ep->val = malloc(strlen(val) + 1))) {
			free(ep->name);
			return -1;
		}
		strcpy(ep->val, val);
	} else {
		//没有剩余可用空间,设置失败
		return -1;
	}

	return 0;
}

int envinit(void)
{
	int ret;

	bzero(envar, NVAR);
	ret = mach_mapenv(_setenv);	
	if(-1 == ret) 
		early_printf("flash map find faild!\r\n");
	else if(-2 == ret)
		early_printf("read env from nvram error!\r\n");
	else
		early_printf("nvram envint done!\r\n");
	
	return ret;
}


