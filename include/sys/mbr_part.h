

/**
 * Copyright(c) 2018-11-14 Shangwen Wu	
 *
 * MBR分区格式定义 
 * 
 */

#ifndef __SYS_MBR_PART_H__
#define __SYS_MBR_PART_H__

#include <sys/endian.h>

#define MBR_SECTOR_SZ			512					/* 主引导扇区大小 */
#define MBR_SECTOR_NUM			0					/* 主引导扇区号 */
#define MBR_DISK_IDENT			0x1b8				/* 磁盘标记符 */
#define MBR_PART_MAGIC			0xaa55				/* 主引导扇区的最后两个字节作为分区结束标志 */
#define MBR_PART_OFFS			0x1be				/* MBR分区起始偏移 */
#define MBR_PART_SZ				64					/* MBR分区信息大小 */
#define MBR_PRI_PART_CNT		4					/* MBR主分区最大个数 */

struct mbr_partition {
	uint8_t boot_signature;				/* 分区状态 */
	uint8_t start_head;					/* 分区起始磁头号 */
#if BYTE_ORDER == LITTLE_ENDIAN
	uint16_t start_sector:6;
	uint16_t start_cylinder:10;	
#elif BYTE_ORDER == BIG_ENDIAN
	uint16_t start_cylinder:10;			/* 分区起始磁柱号 */
	uint16_t start_sector:6;			/* 分区起始扇区号 */
#endif
	uint8_t system_signature;			/* 文件系统标志位 */
	uint8_t end_head;					/* 分区结束磁头号 */
#if BYTE_ORDER == LITTLE_ENDIAN
	uint16_t end_sector:6;
	uint16_t end_cylinder:10;
#elif BYTE_ORDER == BIG_ENDIAN
	uint16_t end_cylinder:10;			/* 分区结束磁柱号 */
	uint16_t end_sector:6;				/* 分区结束扇区号 */
#endif
	uint32_t before_part_sector;		/* 分区起始相对扇区号 */
	uint32_t sectors_total;				/* 分区总扇区数 */
};

#define BOOT_ACTIVE_PARTITION		0x80	/* 活动分区 */
#define BOOT_INACTIVE_PARTITION		0X00	/* 非活动分区 */

#endif //__SYS_MBR_PART_H__

