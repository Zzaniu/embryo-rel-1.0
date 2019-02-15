
/**
 * Copyright(c) 2016-8-9 Shangwen Wu	
 *
 * MIPS全局构造函数相关
 * 
 */
static int initialized = 0;

typedef void (*func_t)(void);

extern func_t __CTOR_LIST__[];			//defined in ld.script
extern func_t __DTOR_LIST__[];

extern void __init(void);
extern void __destory(void);

void __do_init(void)
{
	func_t *ptr = __CTOR_LIST__ + 1;

	while(*ptr)
		(**ptr++)();
}

void __do_destory(void)
{
	unsigned long i = (unsigned long)__DTOR_LIST__[0];
	func_t *ptr = __DTOR_LIST__ + 1;

	while(i--)
		(**ptr--)();
}

void __init(void)
{
	if(!initialized) {
		__do_init();		
		initialized = 1;	
	}
}

void __destory(void)
{
	if(initialized) {
		__do_destory();
		initialized = 0;
	}
}

#if 0
/* test for ctors */
#include <serial.h>
void __attribute__((constructor)) test_ctors(void)
{
	serial_printf("in the test constructor!!!\r\n");	
}

void __attribute__((destructor)) test_dtors(void)
{
	serial_printf("in the test destructor!!!\r\n");	
}
#endif
