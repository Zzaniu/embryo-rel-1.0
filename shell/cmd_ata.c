
/**
 * Copyright(c) 2018-11-01 Shangwen Wu	
 *
 * ata设备读写测试命令
 * 
 */
#include <common.h>
#include <stdio.h>
#include <string.h>
#include <cvt.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <command.h>
#include <fs/termio.h>

extern int ata_write(const char *name, uint64_t blkno, ulong blkcnt, ulong addr);
extern int ata_read(const char *name, uint64_t blkno, ulong blkcnt, ulong addr);

static int cmd_ata_read(int argc, const char *argv[])
{
	unsigned long blkno, blkcnt, addr;
	
	if(atob(&blkno, argv[2], 16)) {
		fprintf(stderr, "blkno %s invaild\n", argv[2]);
		return -1;
	}
	if(atob(&blkcnt, argv[3], 10)) {
		fprintf(stderr, "blkcnt %s invaild\n", argv[3]);
		return -1;
	}
	if(atob(&addr, argv[4], 16)) {
		fprintf(stderr, "address %s invaild\n", argv[4]);
		return -1;
	}
	
	/* bad code, lost higth 32bit */
	if(ata_read(argv[1], (uint64_t)blkno, blkcnt, addr)) {
		perror("ata_read");
		return -1;
	}

	printf("Read %s done: blkno=0x%x, blkcnt=%lu\n", argv[1], blkno, blkcnt);

	return 0;
}

static int cmd_ata_write(int argc, const char *argv[])
{
	unsigned long blkno, blkcnt, addr;
	
	if(atob(&blkno, argv[2], 16)) {
		fprintf(stderr, "blkno %s invaild\n", argv[2]);
		return -1;
	}
	if(atob(&blkcnt, argv[3], 10)) {
		fprintf(stderr, "blkcnt %s invaild\n", argv[3]);
		return -1;
	}
	if(atob(&addr, argv[4], 16)) {
		fprintf(stderr, "address %s invaild\n", argv[4]);
		return -1;
	}

	/* bad code, lost higth 32bit */
	if(ata_write(argv[1], (uint64_t)blkno, blkcnt, addr)) {
		perror("ata_write");
		return -1;
	}

	printf("Write %s done: blkno=0x%x, blkcnt=%lu\n", argv[1], blkno, blkcnt);

	return 0;
}

static struct optdesc ata_read_opts[] = {
	{
		.name = "device", 
		.desc = "ata device",
	},
	{
		.name = "blkno", 
		.desc = "start block number(hexval)",
	},
	{
		.name = "blkcnt", 
		.desc = "block count",
	},
	{
		.name = "addr", 
		.desc = "data to memory address(hexval)",
	},
	{},
};

static struct optdesc ata_write_opts[] = {
	{
		.name = "device", 
		.desc = "ata device",
	},
	{
		.name = "blkno", 
		.desc = "start block number(hexval)",
	},
	{
		.name = "blkcnt", 
		.desc = "block count",
	},
	{
		.name = "addr", 
		.desc = "data from memory address(hexval)",
	},
	{},
};

/* ATA命令实现 */
static struct cmd cmds[] = {
	{{"ATA"}},			//表组名
	{
		{"ata_read"},
		"read n blk from ata device",
		"device blkno blkcnt addr",
		ata_read_opts, 
		cmd_ata_read,
		5,
		5,
		0, 
	},
	{
		{"ata_write"},
		"write n blk to ata device",
		"device blkno blkcnt addr",
		ata_write_opts, 
		cmd_ata_write,
		5,
		5,
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
