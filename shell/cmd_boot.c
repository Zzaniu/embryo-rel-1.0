
/**
 * Copyright(c) 2019-1-28 Shangwen Wu	
 *
 * 内核引导命令，该命令与go命令类似，但是该命令永远不返回
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <command.h>
#include <fs/termio.h>

extern void md_dump_cpuregs(void);							//defined in mipsdep.c
extern register_t md_setstack(register_t val);	
extern register_t md_setargs(register_t, register_t, register_t, register_t);	
extern void __go(void);
extern unsigned long init_machine_kernel_env(uint8_t *envbuf);	//defined in xxx_params.c

/**
 * 描述：boot命令实现
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_boot(int argc, const char *argv[])
{
	int i;
	char *strp, **vecp;
	register_t sp;
	uint8_t *bootparamp;
	unsigned long bootparamlen, cmdargstrlen, cmdargveclen, totallen;

	/* 注意：PC指针应当在cmd_loader加载内核命令中正确设置 */
	if(!md_setpc(0LL)) {
		fprintf(stderr, "%s: kernel entry is null!\n", __func__);
		return -1;
	}

	/* 命令行参数字符串长度 */
	cmdargstrlen = 0;
	for(i = 0; i <  argc; ++i) 
		cmdargstrlen +=  strlen(argv[i]) + 1;
	cmdargstrlen = (cmdargstrlen + 7) & ~7;

	/* 为什么这里不是64位宽而是一个32位宽指针？？？ */
	cmdargveclen = ((argc + 1) * sizeof(char *) + 7) & ~7;
	//cmdargveclen = ((cmdargc + 1) * sizeof(register_t) + 7) & ~7;
	
	/* 内核环境参数长度长度 */
	bootparamlen = init_machine_kernel_env(NULL);
	bootparamlen = (bootparamlen + 7) & ~7;

	totallen = cmdargstrlen + cmdargveclen + bootparamlen;
	totallen = (totallen + 7) & ~7;
	printf("Arg Total Len: %u\n", totallen);

	/* 调整栈顶为默认值-totallen-参数预留空间 */
	sp = md_setstack(0LL) - totallen;
	vecp = (char **)(unsigned long)sp; 
	strp = (char *)(unsigned long)(sp + cmdargveclen);
	bootparamp = (uint8_t *)(unsigned long)(sp + cmdargveclen + cmdargstrlen);
	
	/* 将参数拷贝到sp到sp+totallen的区域中 */
	for(i = 0; i < argc; ++i) {
		*vecp  = strp;
		strcpy(strp, argv[i]);
		strp[strlen(argv[i])] = 0;
		printf("Arg[%u] Addr: 0x%x, Val: %s\n", i, *vecp, strp);
		strp += strlen(argv[i]) + 1;
		++vecp;
	}
	*vecp = NULL;		//最后一个为NULL
	
	/* 初始化平台相关的内核环境参数 */
	init_machine_kernel_env(bootparamp);

	/* 设置参数，arg0=argc，arg1=argvec，arg2=bootparam，arg3=0 */
	md_setargs(argc, (register_t)(long)sp, (register_t)(long)bootparamp, 0LL);

	/* 根据ABI o32/n32/n64 为被调函数参数预留4个register_t长度的空间 */
	md_setstack(sp - 4 * sizeof(register_t));
	memset((void *)(unsigned long)(sp - 4 * sizeof(register_t)), 0, 4 * sizeof(register_t));
	
	md_dump_cpuregs();

	/* 跳转到kernel，此处永远不会返回 */
	__go();

	return 0;
}

/* boot命令的选项描述 */
static struct optdesc boot_opts[] = {
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
		{"boot"}, 			
		"boot kernel", 
		"[arg...]",
		boot_opts, 
		cmd_boot,
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
