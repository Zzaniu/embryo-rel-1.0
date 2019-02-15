
/**
 * Copyright(c) 2018-11-15 Shangwen Wu	
 *
 * fdisk分区命令实现 
 * 
 */

#include <common.h>
#include <sys/types.h>					
#include <sys/endian.h>					
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/partition.h>
#include <sys/blkio.h>
#include <command.h>
#include <fs/termio.h>

static const char *id2fsname[] = {
	/*  0 */  "Empty",
	/*  1 */  "FAT12",
	/*  2 */  "XENIX root",
	/*  3 */  "XENIX usr",
	/*  4 */  "FAT16 <32M",
	/*  5 */  "Extended",
	/*  6 */  "FAT16",
	/*  7 */  "HPFS/NTFS",
	/*  8 */  "AIX",
	/*  9 */  "AIX bootable",
	/*  a */  "OS/2 Boot Manager",
	/*  b */  "W95 FAT32",
	/*  c */  "W95 FAT32 (LBA)",
	/*  e */  "W95 FAT16 (LBA)",
	/*  d */  "Unkown",
	/*  f */  "W95 Ext'd (LBA)",
	/* 10 */  "OPUS",
	/* 11 */  "Hidden FAT12",
	/* 12 */  "Compaq diagnostics",
	/* 13 */  "Unkown",
	/* 14 */  "Hidden FAT16 <32M",
	/* 15 */  "Unkown",
	/* 16 */  "Hidden FAT16",
	/* 17 */  "Hidden HPFS/NTFS",
	/* 18 */  "AST SmartSleep",
	/* 19 */  "Unkown",
	/* 1a */  "Unkown",
	/* 1b */  "Hidden W95 FAT32",
	/* 1c */  "Hidden W95 FAT32(LBA)",
	/* 1d */  "Unkown",
	/* 1e */  "Hidden W95 FAT16(LBA)",
	/* 1f */  "Unkown",
	/* 20 */  "Unkown",
	/* 21 */  "Unkown",
	/* 22 */  "Unkown",
	/* 23 */  "Unkown",
	/* 24 */  "NEC DOS",
	/* 25 */  "Unkown",
	/* 26 */  "Unkown",
	/* 27 */  "Unkown",
	/* 28 */  "Unkown",
	/* 29 */  "Unkown",
	/* 2a */  "Unkown",
	/* 2b */  "Unkown",
	/* 2c */  "Unkown",
	/* 2d */  "Unkown",
	/* 2e */  "Unkown",
	/* 2f */  "Unkown",
	/* 30 */  "Unkown",
	/* 31 */  "Unkown",
	/* 32 */  "Unkown",
	/* 33 */  "Unkown",
	/* 34 */  "Unkown",
	/* 35 */  "Unkown",
	/* 36 */  "Unkown",
	/* 37 */  "Unkown",
	/* 38 */  "Unkown",
	/* 39 */  "Plan 9",
	/* 3a */  "Unkown",
	/* 3b */  "Unkown",
	/* 3c */  "PartitionMagic recovery",
	/* 3d */  "Unkown",
	/* 3e */  "Unkown",
	/* 3f */  "Unkown",
	/* 40 */  "Venix 80286",
	/* 41 */  "PPC PReP Boot",
	/* 42 */  "SFS",
	/* 43 */  "Unkown",
	/* 44 */  "Unkown",
	/* 45 */  "Unkown",
	/* 46 */  "Unkown",
	/* 47 */  "Unkown",
	/* 48 */  "Unkown",
	/* 49 */  "Unkown",
	/* 4a */  "Unkown",
	/* 4b */  "Unkown",
	/* 4c */  "Unkown",
	/* 4d */  "QNX4.x",
	/* 4e */  "QNX4.x 2nd part",
	/* 4f */  "QNX4.x 3rd part",
	/* 50 */  "OnTrack DM",
	/* 51 */  "OnTrack DM6 Aux1",
	/* 52 */  "CP/M",
	/* 53 */  "OnTrack DM6 Aux3",
	/* 54 */  "OnTrackDM6",
	/* 55 */  "EZ-Drive",
	/* 56 */  "Golden Bow",
	/* 57 */  "Unkown",
	/* 58 */  "Unkown",
	/* 59 */  "Unkown",
	/* 5a */  "Unkown",
	/* 5b */  "Unkown",
	/* 5c */  "Priam Edisk",
	/* 5d */  "Unkown",
	/* 5e */  "Unkown",
	/* 5f */  "Unkown",
	/* 60 */  "Unkown",
	/* 61 */  "SpeedStor",
	/* 62 */  "Unkown",
	/* 63 */  "GNU HURD or SysV",
	/* 64 */  "Novell Netware 286",
	/* 65 */  "Novell Netware 386",
	/* 66 */  "Unkown",
	/* 67 */  "Unkown",
	/* 68 */  "Unkown",
	/* 69 */  "Unkown",
	/* 6a */  "Unkown",
	/* 6b */  "Unkown",
	/* 6c */  "Unkown",
	/* 6d */  "Unkown",
	/* 6e */  "Unkown",
	/* 6f */  "Unkown",
	/* 70 */  "DiskSecure Mult-Boot",
	/* 71 */  "Unkown",
	/* 72 */  "Unkown",
	/* 73 */  "Unkown",
	/* 74 */  "Unkown",
	/* 75 */  "PC/IX",
	/* 76 */  "Unkown",
	/* 77 */  "Unkown",
	/* 78 */  "Unkown",
	/* 79 */  "Unkown",
	/* 7a */  "Unkown",
	/* 7b */  "Unkown",
	/* 7c */  "Unkown",
	/* 7d */  "Unkown",
	/* 7e */  "Unkown",
	/* 7f */  "Unkown",
	/* 80 */  "Old Minix",
	/* 81 */  "Minix / old Linux",
	/* 82 */  "Linux swap / Solaris",
	/* 83 */  "Linux",
	/* 84 */  "OS/2 hidden C: drive",
	/* 85 */  "Linux extended",
	/* 86 */  "NTFS volume set",
	/* 87 */  "NTFS volume set",
	/* 88 */  "Linux plaintext",
	/* 89 */  "Unkown",
	/* 8a */  "Unkown",
	/* 8b */  "Unkown",
	/* 8c */  "Unkown",
	/* 8d */  "Unkown",
	/* 8e */  "Linux LVM",
	/* 8f */  "Unkown",
	/* 90 */  "Unkown",
	/* 91 */  "Unkown",
	/* 92 */  "Unkown",
	/* 93 */  "Amoeba",
	/* 94 */  "Amoeba BBT",
	/* 95 */  "Unkown",
	/* 96 */  "Unkown",
	/* 97 */  "Unkown",
	/* 98 */  "Unkown",
	/* 99 */  "Unkown",
	/* 9a */  "Unkown",
	/* 9b */  "Unkown",
	/* 9c */  "Unkown",
	/* 9d */  "Unkown",
	/* 9e */  "Unkown",
	/* 9f */  "BSD/OS",
	/* a0 */  "IBM Thinkpad hibernation",
	/* a1 */  "Unkown",
	/* a2 */  "Unkown",
	/* a3 */  "Unkown",
	/* a4 */  "Unkown",
	/* a5 */  "FreeBSD",
	/* a6 */  "OpenBSD",
	/* a7 */  "NeXTSTEP",
	/* a8 */  "Darwin UFS",
	/* a9 */  "NetBSD",
	/* aa */  "Unkown",
	/* ab */  "Darwin boot",
	/* ac */  "Unkown",
	/* ad */  "Unkown",
	/* ae */  "Unkown",
	/* af */  "HFS / HFS+",
	/* b0 */  "Unkown",
	/* b1 */  "Unkown",
	/* b2 */  "Unkown",
	/* b3 */  "Unkown",
	/* b4 */  "Unkown",
	/* b5 */  "Unkown",
	/* b6 */  "Unkown",
	/* b7 */  "BSDI fs",
	/* b8 */  "BSDI swap",
	/* b9 */  "Unkown",
	/* ba */  "Unkown",
	/* bb */  "Boot Wizard hidden",
	/* bc */  "Unkown",
	/* bd */  "Unkown",
	/* be */  "Solaris boot",
	/* bf */  "Solaris",
	/* c0 */  "Unkown",
	/* c1 */  "DRDOS/sec (FAT-12)",
	/* c2 */  "Unkown",
	/* c3 */  "Unkown",
	/* c4 */  "DRDOS/sec (FAT-16 < 32M)",
	/* c5 */  "Unkown",
	/* c6 */  "DRDOS/sec (FAT-16)",
	/* c7 */  "Syrinx",
	/* c8 */  "Unkown",
	/* c9 */  "Unkown",
	/* ca */  "Unkown",
	/* cb */  "Unkown",
	/* cc */  "Unkown",
	/* cd */  "Unkown",
	/* ce */  "Unkown",
	/* cf */  "Unkown",
	/* d0 */  "Unkown",
	/* d1 */  "Unkown",
	/* d2 */  "Unkown",
	/* d3 */  "Unkown",
	/* d4 */  "Unkown",
	/* d5 */  "Unkown",
	/* d6 */  "Unkown",
	/* d7 */  "Unkown",
	/* d8 */  "Unkown",
	/* d9 */  "Unkown",
	/* da */  "Non-FS data",
	/* db */  "CP/M / CTOS / ...",
	/* dc */  "Unkown",
	/* dd */  "Unkown",
	/* de */  "Dell Utility",
	/* df */  "BootIt",
	/* e0 */  "Unkown",
	/* e1 */  "DOS access",
	/* e2 */  "Unkown",
	/* e3 */  "DOS R/O",
	/* e4 */  "SpeedStor",
	/* e5 */  "Unkown",
	/* e6 */  "Unkown",
	/* e7 */  "Unkown",
	/* e8 */  "Unkown",
	/* e9 */  "Unkown",
	/* ea */  "Unkown",
	/* eb */  "BeOS fs",
	/* ec */  "Unkown",
	/* ed */  "Unkown",
	/* ee */  "GPT",
	/* ef */  "EFI (FAT-12/16/32)",
	/* f0 */  "Linux/PA-RISC boot",
	/* f1 */  "SpeedStor",
	/* f2 */  "DOS secondary",
	/* f3 */  "Unkown",
	/* f4 */  "SpeedStor",
	/* f5 */  "Unkown",
	/* f6 */  "Unkown",
	/* f7 */  "Unkown",
	/* f8 */  "Unkown",
	/* f9 */  "Unkown",
	/* fa */  "Unkown",
	/* fb */  "VMware VMFS",
	/* fc */  "VMware VMKCORE",
	/* fd */  "Linux raid autodetect",
	/* fe */  "LANstep",
	/* ff */  "BBT",
};

/**
 * 描述：执行fdisk命令
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_fdisk(int argc, const char *argv[])
{
	int fd, i, width = 12;
	char devname[64] = "/dev/block/";
	/** 
 	 * bad code
 	 * 这里有潜在BUG！！！，当结构体在栈中的地址为非4字节对齐时，
 	 * 访问结构体中的4字节成员变量将会引起地址对齐访问异常 
 	 */
	struct partition_info parts[PART_INFO_MAX], *pa = parts;

	if(argc < 2 || !argv[1]) {
		fprintf(stderr, "block device name is unspecified\n");
		return -1;
	}
	if(strlen(argv[1]) >= sizeof(devname) - 11) {
		fprintf(stderr, "block device name is too long\n");
		return -1;
	}
	strcat(devname, argv[1]);

	if(-1 == (fd = open(devname, O_RDWR))) {
		perror("open block");
		return -1;
	}

	if((-1 == ioctl(fd, BIOCGPARTINF, parts)) && errno != ENXIO) {
		perror("ioctl block");
		return -1;
	}
	
	printf("Disk %s\n\n", argv[1]);

	if(ENXIO == errno) {
		fprintf(stderr, "Disk %s dosen't contain a valid partition table\n", argv[1]);	
		return 0;
	}

	printf("%*s%*s%*s%*s%*s%*s  %-*s\n", width + 2, "Partition", 
			6, "Boot", width, "Start", width, "End", 
			width, "Blocks", 5, "Id", width + 4, "System");
	for(i = 0; i < PART_INFO_MAX; ++i) {
		if(pa->pi_valid) {
			printf("%*s%*s%*u%*u%*u%*x  %-*s\n", width + 2, pa->pi_name, 
					6, pa->pi_boot ? "*" : "", width, pa->pi_start, width, pa->pi_end, 
					width, pa->pi_blks, 5, pa->pi_sysid, width + 4, id2fsname[pa->pi_sysid]);
			
		}
		++pa;
	}

	close(fd);

	return 0;
}

static struct optdesc fdisk_opts[] = {
	{
		.name = "blockdev", 
		.desc = "block device which need operation",
	},
	{},
};

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"BLOCK"}},			//表组名
	{
		{"fdisk"}, 			
		"partition command", 
		"blockdev",
		fdisk_opts, 
		cmd_fdisk,
		2,
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
