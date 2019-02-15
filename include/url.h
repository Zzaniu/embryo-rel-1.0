
/**
 * Copyright(c) 2018-9-6 Shangwen Wu	
 *
 * URL定义 
 * 
 */

#ifndef __URL_H__
#define __URL_H__

#define PROTOCALSZ	16
#define USERNAMESZ	16
#define PASSWDSZ	16
#define HOSTNAMESZ	32
#define FILEPATHSZ	32

/* URL定义 */
struct URL {
	char protocol[PROTOCALSZ];
	char username[USERNAMESZ];
	char passwd[PASSWDSZ];
	char hostname[HOSTNAMESZ];
	char filepath[FILEPATHSZ];
	uint16_t port;
};

int url_parse(const char *urlstr, struct URL *url);

#endif //__URL_H__
