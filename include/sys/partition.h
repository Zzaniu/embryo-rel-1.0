
/**
 * Copyright(c) 2018-11-15 Shangwen Wu	
 * 块设备分区定义
 * 
 */

#ifndef __SYS_PARTITION_H__
#define __SYS_PARTITION_H__

#define PART_NAME_MAX		64
#define PART_INFO_MAX		4		/* bad code, 这里为了简化程序限制了part分区个数 */

struct partition_info {
	ulong pi_start;					/* 起始位置 */
	ulong pi_end;					/* 结束位置 */
	ulong pi_blks;					/* 分区总块数（KB） */
	uint8_t pi_sysid;				/* 文件系统ID */
	uint8_t pi_valid;				/* 分区有效标记 */
	uint8_t pi_boot;				/* 引导分区标记 */
	char pi_name[PART_NAME_MAX];	/* 分区名称 */
};	

#endif //__SYS_PARTITION_H__

