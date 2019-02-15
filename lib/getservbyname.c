
/**
 * Copyright(c) 2018-10-09 Shangwen Wu 
 *
 * getservbyname函数实现
 */
#include <common.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>

/**
 * 描述：根据服务名称以及协议名获取服务端口等信息
 */
struct servent *getservbyname(const char *name, const char *proto)
{
	struct servent *sp;
	char **cp;

	setservent(0);
	
	while(sp = getservent()) {
		if(!strcmp(name, sp->s_name))
			goto foundname;
		for(cp = sp->s_aliases; *cp; ++cp)	
			if(!strcmp(name, *cp))
				goto foundname;
		continue;
foundname:
		if(!proto || !strcmp(proto, sp->s_proto))
			break;
	}
	
	endservent();

	return sp;
}

