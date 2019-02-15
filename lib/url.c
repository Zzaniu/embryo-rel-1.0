
/**
 * Copyright(c) 2018-9-6 Shangwen Wu	
 * URL解析
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <url.h>

/**
 * 描述：URL格式解析
 * 		 URL：协议://用户名:密码@主机名:端口号/文件路径
 */
int url_parse(const char *urlstr, struct URL *url)
{
	uint32_t n;
	char *t, *t2, *s = (char *)urlstr;
	char portstr[8];
	unsigned long port;

	if(!urlstr)
		return -1;		

	//protocol
	if((t = strstr(s, "://")) != NULL) {
		n = t - s;
		if(n >= PROTOCALSZ)
			return -1;
		strncpy(url->protocol, s, n);
		url->protocol[n] = 0;
		s = t + 3;
	} else
		return -1;
	
	//user & passwd
	if((t = strchr(s, '@')) != NULL) {
		if((t2 = strchr(s, ':')) != NULL && t2 < t) {
			n = t2 - s;
			if(n >= USERNAMESZ)
				return -1;
			strncpy(url->username, s, n);
			url->username[n] = 0;
			s = t2 + 1;
			n = t - s;
			if(n >= PASSWDSZ)
				return -1;
			strncpy(url->passwd, s, n);
			url->passwd[n] = 0;
		} else {
			n = t - s;
			if(n >= USERNAMESZ)
				return -1;
			strncpy(url->username, s, n);
			url->username[n] = 0;
			url->passwd[0] = 0;
		}
		s = t + 1;
	} else {
		url->username[0] = 0;
		url->passwd[0] = 0;
	}
	
	//hostname & port
	if((t = strchr(s, '/')) != NULL) {
		if((t2 = strchr(s, ':')) != NULL && t2 < t) {
			n = t2 - s;
			if(n >= HOSTNAMESZ)
				return -1;
			strncpy(url->hostname, s, n);
			url->hostname[n] = 0;
			s = t2 + 1;
			n = t - s;
			if(n >= sizeof(portstr))
				return -1;
			strncpy(portstr, s, n);
			portstr[n] = 0;
			if(atob(&port, portstr, 10))
				return -1;
			url->port = (uint16_t)port;
		} else {
			n = t - s;
			if(n >= HOSTNAMESZ)
				return -1;
			strncpy(url->hostname, s, n);
			url->hostname[n] = 0;
			url->port = 0;
		}
		s = t + 1;
	} else 
		return -1;	

	//filepath
	n = strlen(s);
	if(n >= FILEPATHSZ)
		return -1;
	strcpy(url->filepath, s);

	return 0;
}

