
/**
 * Copyright(c) 2017-2-22 @PMON @Shangwen Wu	
 *
 * BSD中断处理接口定义 
 * 
 */

#ifndef __MACH_INTR_H_
#define __MACH_INTR_H_

/* 各种操作的中断级别 */
#define IPL_BIO		0	/* block I/O */
#define IPL_NET		1	/* block I/O */
#define IPL_TTY		2	/* block I/O */
#define IPL_CLOCK	3	/* block I/O */
#define IPL_IMP		4	/* block I/O */
#define IPL_NONE	5	/* block I/O */
#define IPL_HIGH	6	/* block I/O */

/* defined in kern_misc.c */
extern int splsoftclock(void);
extern int splsoftnet(void);
extern int splnet(void);
extern int splbio(void);
extern int spltty(void);
extern int splclock(void);
extern int splimp(void);
extern int splhigh(void);
extern int spl0(void);
extern void splx(int);

#endif //__MACH_INTR_H_
