
/**
 * Copyright(c) 2018-10-09 Shangwen Wu 
 *
 * 兼容代码，正常情况下，下面的函数可能会使用到数据库
 */
#include <common.h>
#include <sys/types.h>
#include <sys/endian.h>					
#include <netdb.h>

static int __stay_open = 0; /* 表示调用endservent后仍然保持检索打开状态 */
static int __entindx = -1;

struct servtab {
	const char *name;
	const char *alias;	
	short port;
	short proto;
};

#define UDP 0
#define TCP 1

static struct servtab servtab[] = {
	{"echo", NULL, 7, UDP},
	{"echo", NULL, 7, TCP},
	{"discard", "sink", 9, UDP},
	{"discard", "sink", 9, TCP},
	{"ftp", NULL, 21, TCP},
	{"telnet", NULL, 23, TCP},
	{"tftp", NULL, 69, UDP},
	{},
};

struct servent *getservent(void)
{
	struct servtab *sbp;
	static struct servent ent;
	static char *alias[2] = {0};

	if(-1 == __entindx)
		return NULL;
	
	sbp = servtab + __entindx;

	if(!sbp->name) {
		__entindx = 0;
		return NULL;
	}

	ent.s_name = (char *)sbp->name;
	alias[0] = (char *)sbp->alias;
	ent.s_aliases = alias;
	ent.s_port = (int)(htons(sbp->port));
	ent.s_proto = (sbp->proto == UDP ? "udp" : "tcp");
	++__entindx;	

	return &ent;
}

void setservent(int stayopen)
{
	__stay_open = !!stayopen;
	__entindx = 0;
}

void endservent(void)
{	
	if(!__stay_open)
		__entindx = -1;
}
