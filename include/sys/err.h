
/**
 * Copyright(c) 2018-8-22 Shangwen Wu	
 *
 * 处理需要通过指针传递errno信息的情况 
 * 
 */

#ifndef __SYS_ERR_H__
#define __SYS_ERR_H__

#include <sys/errno.h>

#define MAX_ERRNO	200			//最多200个错误码

/** 
 * 这里使用32位地址空间高MAX_ERRNO个字节地址作为错误指针来传递错误信息，
 * 在bios中，这段空间不是有效的内存空间地址，故可以用于传递错误码
 */
#define IS_ERR_VAL(x)	((unsigned long)(x) >= (unsigned long)(-MAX_ERRNO))

/**
 * 描述：根据传入指针，判断是否为errno
 */
static inline int IS_ERR(const void *ptr)
{
	return IS_ERR_VAL(ptr);
}

/**
 * 描述：根据传入指针，判断是否为errno
 */
static inline int IS_ERR_OR_NULL(const void *ptr)
{
	return !ptr || IS_ERR_VAL(ptr);
}

/**
 * 描述：根据传入errno，转换成指针
 * 注意：这里要求errno必须为正数值，否则返回的指针会被认为是正常地址
 */
static inline void *ERR_PTR(long err)
{
	return (void *)(-err);
}

/**
 * 描述：根据传入的指针，转换成errno
 * 注意：该函数只有在在调用IS_ERR确认为错误码后，调用才有意义
 */
static inline long PTR_ERR(const void *ptr)
{
	return -((long)ptr);
}

#endif //__SYS_ERR_H__
