
/**
 * Copyright(c) 2016-8-24 Shangwen Wu 
 *
 * stdio实现
 * 
 */
#include <stdio.h>			

FILE __iob[OPEN_MAX] = {		//注意！这里的FD必须和termio.h中的FD相对应
	{0, 1},					
	{1, 1},
	{2, 1},
	{3, 1},
	{4, 1},
	{5, 1},
};
