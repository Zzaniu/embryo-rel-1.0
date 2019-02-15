
/**
 * Copyright(c) 2019-1-17 Shangwen Wu 
 *
 * 可执行文件加载器
 * 
 */
#include <common.h>
#include <sys/types.h>					
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <command.h>
#include <load_exec.h>
#include <fs/termio.h>

static LIST_HEAD(exectype_list);

/**
 *  描述：加载指定文件描述符对应的可执行文件
 *  参数：fd，源文件描述符；id，可执行文件类型；flags，加载标志位
 *  返回：返回可执行文件的程序入口地址，出错返回0
 */
unsigned long load_exec(int fd, int type, int flags)
{
	int res;
	unsigned long entry;
	struct exec_type *et;

	list_for_each_entry(et, &exectype_list, et_list) {	
		if(EXEC_TYPE_UNKOWN == type) {
			if(et->et_flags & EXEC_FLAGS_AUTODETECT) {
				res = (*et->et_loader)(fd, flags, &entry);
				if(EXEC_LOAD_RES_BADFMT == res)
					continue;
				else if(EXEC_LOAD_RES_LOADERR == res)
					return 0;
				else 		//load success
					return entry;
			}
		} else {
			if(et->et_type == type) {
				res = (*et->et_loader)(fd, flags, &entry);
				if(EXEC_LOAD_RES_BADFMT == res) {
					fprintf(stderr, "%s: executable format %d don't match\n", __func__, type);
					return 0;
				} else if(EXEC_LOAD_RES_LOADERR == res)
					return 0;
				else 		//load success
					return entry;
			}
		}
	}

	fprintf(stderr, "%s: unkown executable format\n", __func__);

	return 0;
}

/**
 * 描述：注册一个新的exec_type
 */
int exec_type_register(struct exec_type *et)
{
	list_add_tail(&et->et_list, &exectype_list);

	return 0;
}

void exec_type_unregister(struct exec_type *et)
{
	list_del(&et->et_list);
}

