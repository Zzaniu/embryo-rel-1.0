
/**
 * Copyright(c) 2016-8-26 GNU c Shangwen Wu	
 *
 * ctype.h标准头文件
 * 
 */

#ifndef __CTYPE_H__
#define __CTYPE_H__

#define _U	0x01		//大写字母
#define _L 	0x02		//小写字母
#define _N	0X04		//
#define _S	0X08		//
#define _P	0X10		//
#define _C	0X20		//
#define _X	0X40		//
#define _B	0X80		//

extern const char*_ctype_tab_;
extern const short *_tolower_tab_;
extern const short *_toupper_tab_;

#define isalnum(c) 	((_ctype_tab_ + 1)[(unsigned char)(c)] & (_L|_N|_U))
#define isalpha(c) 	((_ctype_tab_ + 1)[(unsigned char)(c)] & (_L|_U)) 
#define iscntrl(c) 	((_ctype_tab_ + 1)[(unsigned char)(c)] & _C) 
#define isdigit(c) 	((_ctype_tab_ + 1)[(unsigned char)(c)] & _N) 
#define isgraph(c) 	((_ctype_tab_ + 1)[(unsigned char)(c)] & (_P|_U|_L|_N)) 
#define islower(c) 	((_ctype_tab_ + 1)[(unsigned char)(c)] & _L) 
#define isprint(c) 	((_ctype_tab_ + 1)[(unsigned char)(c)] & (_P|_U|_L|_N|_B))
#define ispunct(c)	((_ctype_tab_ + 1)[(unsigned char)(c)] & _P) 
#define isspace(c)	((_ctype_tab_ + 1)[(unsigned char)(c)] & _S) 
#define isupper(c)	((_ctype_tab_ + 1)[(unsigned char)(c)] & _U) 
#define isxdigit(c) ((_ctype_tab_ + 1)[(unsigned char)(c)] & (_N|_X))   
#define tolower(c)	((_tolower_tab_ + 1)[(unsigned char)(c)])   
#define toupper(c)	((_toupper_tab_ + 1)[(unsigned char)(c)])   

#define isascii(c)	((unsigned char)(c) <= 0x7f)
#define toascii(c)	((unsigned char)(c) & 0x7f)

#endif	//__CTYPE_H__
