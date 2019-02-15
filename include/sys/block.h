
/**
 * Copyright(c) 2018-10-25 Shangwen Wu	
 * 块设备定义
 * 
 */

#ifndef __SYS_BLOCK_H__
#define __SYS_BLOCK_H__

#define BLOCK_IF_TYPE_UNKOWN	0x00
#define BLOCK_IF_TYPE_IDE		0x01
#define BLOCK_IF_TYPE_SCSI		0x02
#define BLOCK_IF_TYPE_ATAPI		0x03
#define BLOCK_IF_TYPE_USB		0x04
#define BLOCK_IF_TYPE_DOC		0x05
#define BLOCK_IF_TYPE_MMC		0x06
#define BLOCK_IF_TYPE_SD		0x07
#define BLOCK_IF_TYPE_SATA		0x08

#define BLOCK_PART_TYPE_UNKOWN	0x00
#define BLOCK_PART_TYPE_MAC		0x01
#define BLOCK_PART_TYPE_DOS		0x02
#define BLOCK_PART_TYPE_ISO		0x03
#define BLOCK_PART_TYPE_AMIGA	0x04

#define BLOCK_DEV_TYPE_UNKOWN	0xff
#define BLOCK_DEV_TYPE_HARDDISK	0x00
#define BLOCK_DEV_TYPE_TAPE		0x01
#define BLOCK_DEV_TYPE_CDROM	0x05
#define BLOCK_DEV_TYPE_OPDISK	0x07

struct block_device;
struct partition_info;
struct list_head;

struct block_operation {
	int64_t (*bo_write)(struct block_device *, uint64_t, 
						ulong, const void *);
	int64_t (*bo_read)(struct block_device *, uint64_t, 
						ulong, void *);
	int (*bo_read_partition)(struct block_device *, 
				struct partition_info *);
	int (*bo_read_mbr)(struct block_device *, uint8_t *);
};

/* 块设备描述 */
struct block_device {
	uint8_t bd_if_type;		/* 接口类型那个 */
	uint8_t bd_type;		/* 设备类型 */
	uint8_t bd_part_type;
	uint8_t bd_removeable;
	ulong bd_blk_sz;		/* 每块包含字节数 */
	ulong bd_blk_sz_log2;	/* 每块包含字节数LOG2 */
	struct block_operation bd_blk_ops;
	void *bd_priv;			/* 私有数据，一般保存具体物理设备的指针 */
	int bd_has_lba48;		/* 是否支持48位宽逻辑块地址 */
	uint64_t bd_lba;		/* 逻辑块个数 */
	ulong bd_maxblks;		/* 每次允许传输的最大数据块个数 */
	struct list_head bd_list;
	char *bd_name;			/* 与对应的ATA设备同名 */
	ulong bd_refcnt;
};

extern int blkdev_register(struct block_device *blkdev);//defined in block_dev.c
extern int blkdev_unregister(struct block_device *blkdev);
extern struct block_device *blkdev_lookup(const char *devname);

#endif //__SYS_BLOCK_H__

