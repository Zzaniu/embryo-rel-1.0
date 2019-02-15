
/**
 * Copyright(c) 2018-10-22 Shangwen Wu
 * ATA通用函数 
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/err.h>
#include <sys/list.h>
#include <sys/syslog.h>
#include <sys/delay.h>
#include <sys/device.h>
#include <sys/block.h>
#include <ata/ata.h>
#include <ata/ahci.h>
#include <asm/io.h>

/**
 * 描述：轮询等待某个寄存器设置为指定的值
 * 参数：reg, 寄存器地址，mask，要判断的bit位，val，要求达到的值，
 * 		 minterval，轮询间隔时间(ms)，mtimeout，轮询超时时间(ms)
 */
uint32_t ata_wait_register(void *reg, uint32_t mask, uint32_t val,
								long minterval, long mtimeout)
{
	uint32_t res;

	while(((res = readl((ulong)reg)) & mask) != val && mtimeout > 0) {
		mdelay(minterval);
		mtimeout -= minterval;
	}

	return res;
}

/**
 * 描述：轮询等待命令完成（指定bit清零即表示命令完成）
 * 参数：reg, 寄存器地址，mask，要判断的bit位
 * 		 mtimeout，轮询超时时间(ms)
 */
int ata_wait_cmd_completed(void *reg, uint32_t mask, long mtimeout)
{
	uint32_t res;

	while(((res = readl((ulong)reg)) & mask) && mtimeout > 0) {
		mdelay(1);			//每1ms轮询一次
		--mtimeout;
	}

	if(!(res & mask))
		return 0;
	
	return -1;
}

/**
 * 描述：发送DEVICE IDETIFY命令到ATA设备，查询设备ID信息
 *
 */
uint16_t *ata_device_identify(struct ata_device *adev)
{
	int err;
	uint16_t *devid;
	struct ata_port *ap = adev->port;

	if(!(devid = (uint16_t *)kmem_zmalloc(ATA_ID_WORDS * sizeof(uint16_t))))
		return ERR_PTR(ENOMEM);

	if((err = ap->port_ops->read_id(adev, devid)) != 0) {
		kmem_free(devid);
		return ERR_PTR(err);
	}

	return devid;
}

/**
 * 描述：返回设备块数
 */
uint64_t ata_id_n_sectors(uint16_t *id)
{
	if(ata_id_has_lba(id)) {
		if(ata_id_has_lba64(id))
			return ata_id_u64(id, ATA_ID_LBA_CAPACITY_2);
		else
			return ata_id_u32(id, ATA_ID_LBA_CAPACITY);
	}

	return 0;
}

/**
 * 描述：设备设备传输模式
 * 注意：bad code， 目前仅支持UDMA的几种传输模式，其他模式暂不支持
 */
int ata_dev_set_xfermode(struct ata_device *adev)
{
	uint8_t mode;
	uint16_t *ata_id = adev->devid;
	struct ata_port *ap = adev->port;
	uint8_t udma_cap = (uint8_t)(ata_id[ATA_ID_UDMA_MODES] & 0xff);
	
	if(!adev->devid)
		return EINVAL;	

	mode = adev->xfermode;
	if(!mode) {
		switch(udma_cap) {
			case ATA_UDMA6:
				mode = XFER_UDMA6;
			break;
			case ATA_UDMA5:
				mode = XFER_UDMA5;
			break;
			case ATA_UDMA4:
				mode = XFER_UDMA4;
			break;
			case ATA_UDMA3:
				mode = XFER_UDMA3;
			break;
			case ATA_UDMA2:
				mode = XFER_UDMA2;
			break;
			case ATA_UDMA1:
				mode = XFER_UDMA1;
			break;
			case ATA_UDMA0:
				mode = XFER_UDMA0;
			break;
			default:
				return EINVAL;	
		}
	}
	
	return ap->port_ops->set_xfermode(adev, mode);
}

/**
 * 描述：软复位SATA设备
 */
int ata_device_softreset(struct ata_device *adev)
{
	struct ata_port *ap = adev->port;

	return ap->port_ops->softreset(adev);
}

/**
 * 描述：读写ATA设备
 * 参数：startblk：起始LBA地址，blkcnt：访问块个数，buf：数据缓冲区，op：read or write
 */ 
int64_t ata_blk_data_rw(struct ata_device *adev, uint64_t startblk, 
						ulong blkcnt, void *buf, int op)
{
	struct block_device *blkdev = adev->blkdev;
	struct ata_port *ap = adev->port;
	ulong remain = blkcnt, blks;
	
	while(remain) {
		/* 注意：单次ATA命令传输字节不得超过blkdev->bd_maxblks */
		blks = (remain > blkdev->bd_maxblks) ? blkdev->bd_maxblks : remain;
		if(errno = ap->port_ops->blk_data_rw(adev, startblk, blks, buf, op)) {
			return (int64_t)-1;
		}
		remain -= blks;
		startblk += blks;
		buf += blks * blkdev->bd_blk_sz;
	}

	return (int64_t)(blkcnt - remain);
}

/* 块设备读写访问接口 */
int64_t ata_blk_read(struct ata_device *adev, uint64_t startblk, 
						ulong blkcnt, void *buf)
{
	return ata_blk_data_rw(adev, startblk, blkcnt, buf, ATA_READ);
}

int64_t ata_blk_write(struct ata_device *adev, uint64_t startblk, 
						ulong blkcnt, const void *buf)
{
	return ata_blk_data_rw(adev, startblk, blkcnt, (void *)buf, ATA_WRITE);
}

/* 用于shell命令测使用的SATA设备读写函数 */
int ata_write(const char *name, uint64_t blkno, ulong blkcnt, ulong addr)
{
	int64_t res;
	struct block_device *blkdev;

	if(!(blkdev = blkdev_lookup(name))) {
		log(LOG_ERR, "blkdev of %s not found!\n", name);
		errno = ENODEV;
		return -1;
	}

	res = blkdev->bd_blk_ops.bo_write(blkdev, blkno, blkcnt, (const void *)addr);
	if(res < 0) {
		log(LOG_ERR, "blkdev write data error, res blk %lld\n", res);
		return -1;
	}

	return 0;
}

int ata_read(const char *name, uint64_t blkno, ulong blkcnt, ulong addr)
{
	int64_t res;
	struct block_device *blkdev;

	if(!(blkdev = blkdev_lookup(name))) {
		log(LOG_ERR, "blkdev of %s not found!\n", name);
		errno = ENODEV;
		return -1;
	}

	res = blkdev->bd_blk_ops.bo_read(blkdev, blkno, blkcnt, (void *)addr);
	if(res < 0) {
		log(LOG_ERR, "blkdev read data error, res blk %lld\n", res);
		return -1;
	}

	return 0;
}

