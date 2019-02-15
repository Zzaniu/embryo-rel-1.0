
/**
 * Copyright(c) 2019-1-26 Shangwen Wu	
 *
 * 执行客户端程序命令
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <command.h>
#include <fs/termio.h>

extern int get_rsa_reg(register_t *vp, const char *p);		//defined in rsa.c
extern int getopt(int argc, const char *argv[], int *optind, \
		char **optval, const char *pattern);				//defined in getopt.c
extern void md_dump_cpuregs(void);							//defined in mipsdep.c
extern register_t md_setpc(register_t val);	
extern register_t md_setlr(register_t val);	
extern register_t md_setstack(register_t val);	
extern register_t md_setargs(register_t, register_t, register_t, register_t);	
extern void __go(void);
extern void __exit(void);

extern int go_return_val;									//defined in mipsdep.c
extern jmp_buf go_return_jmpbuf;

/**
 * 描述：go命令实现
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_go(int argc, const char *argv[])
{
	int optind, i, cmdargc;
	char opt, *optval = NULL, *strp, **vecp;
	register_t stack, entry, sp;
	/* 子程序的最大参数个数为命令行最大参数个数-最大选项参数个数 */
	char *cmdargvec[MAX_CMD_ARG_NUM - 5];
	unsigned long cmdargstrlen, cmdargveclen, totallen;

	optind = 0;
	while((opt = getopt(argc, argv, &optind, &optval, "e:s:")) != EOF) {
		switch(opt) {
			case 'e':
				if(get_rsa_reg(&entry, optval)) {
					fprintf(stderr, "%s: illegal value -- %s\n", *argv, optval);
					return -2;
				}
				printf("Set Entry: 0x%llx\n", entry);
				md_setpc(entry);
			break;
			case 's':
				if(get_rsa_reg(&stack, optval)) {
					fprintf(stderr, "%s: illegal value -- %s\n", *argv, optval);
					return -2;
				}
				printf("Set Stack: 0x%llx\n", stack);
				md_setstack(stack);
			break;
			default:							//BADOPT
				return -2;
		}
	}

	/* 注意：如果没有设值e选项，那么PC指针应当在cmd_loader命令中正确设置 */
	if(!md_setpc(0LL)) {
		fprintf(stderr, "%s: client entry is null!\n", __func__);
		return -1;
	}

	/* 初始化参数指针数组，第一个参数为命令行输入的命令名称，这里是“go” */
	cmdargvec[0] = (char *)*argv, cmdargc = 1, cmdargstrlen = strlen(*argv) + 1;
	/* 添加后续参数 */
	while(optind < argc) {
		cmdargvec[cmdargc++] = (char *)argv[optind];
		cmdargstrlen +=  strlen(argv[optind++]) + 1;
	}
	cmdargstrlen = (cmdargstrlen + 7) & ~7;
	/* 为什么这里不是64位宽而是一个32位宽指针？？？ */
	cmdargveclen = ((cmdargc + 1) * sizeof(char *) + 7) & ~7;
	//cmdargveclen = ((cmdargc + 1) * sizeof(register_t) + 7) & ~7;
	totallen = cmdargstrlen + cmdargveclen;
	totallen = (totallen + 7) & ~7;
	printf("Arg Total Len: %u\n", totallen);

	/* 调整栈顶为默认值-totallen-参数预留空间 */
	sp = md_setstack(0LL) - totallen;
	vecp = (char **)(unsigned long)sp; 
	strp = (char *)(unsigned long)(sp + cmdargveclen);
	
	/* 将参数拷贝到sp到sp+totallen的区域中 */
	for(i = 0; i < cmdargc; ++i) {
		*vecp  = strp;
		strcpy(strp, cmdargvec[i]);
		strp[strlen(cmdargvec[i])] = 0;
		printf("Arg[%u] Addr: 0x%x, Val: %s\n", i, *vecp, strp);
		++vecp;
		strp += strlen(cmdargvec[i]) + 1;
	}
	*vecp = NULL;		//最后一个为NULL
	/* 设置参数，arg0=argc，arg1=argvec，arg2=0，arg3=0 */
	md_setargs(cmdargc, (register_t)(long)vecp, 0LL, 0LL);

	/* 根据ABI o32/n32/n64 为被调函数参数预留4个register_t长度的空间 */
	md_setstack(sp - 4 * sizeof(register_t));
	memset((void *)(unsigned long)(sp - 4 * sizeof(register_t)), 0, 4 * sizeof(register_t));
	
	/* 设置返回地址 */
	md_setlr((register_t)(long)__exit);	

	md_dump_cpuregs();

	/* 跳转到客户程序 */
	if(!setjmp(go_return_jmpbuf)) {
		/* 直接调用setjmp，将跳转至客户程序，而下面的__go函数将永不返回 */	
		__go();
	} 
	/* 客户程序返回 */
	printf("client return, retcode: %d\n", go_return_val);	

	return 0;
}

/* go命令的选项描述 */
static struct optdesc go_opts[] = {
	{
		.name = "-e", 
		.desc = "setup client entry address",
	},
	{
		.name = "-s", 
		.desc = "setup client stack pointer",
	},
	{
		.name = "arg...", 
		.desc = "command line arguments list",
	},
	{},
};

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"Execute"}},			//表组名
	{
		{"go"}, 			
		"start execute client's program", 
		"[-e entry|s stack|arg...]",
		go_opts, 
		cmd_go,
		0,
		MAX_CMD_ARG_NUM - 1,
		0, 
	},
	{},						//最后一个强制要求为空
};

/**
 * 描述：命令初始化函数
 */
static void __attribute__((constructor)) init_cmds(void)
{
	add_cmds(cmds, 0);
}
