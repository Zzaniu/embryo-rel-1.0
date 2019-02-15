
/**
 * Copyright(c) 2018-11-12 Shangwen Wu
 * 块设备文件支持（驱动层）
 * bad code 该模块将应用层与内核层之间耦合在一起，属于PMON遗传的槽糕设计
 */
#include <common.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/syslog.h>
#include <sys/param.h>
#include <sys/system.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/block.h>
#include <sys/mbr_part.h>
#include <sys/partition.h>
#include <sys/blkio.h>
#include <fs/blkfile.h>
#include <unistd.h>
#include <fcntl.h>

static int blkfile_open(struct block_file *bf, int flags)
{
	struct block_device *blkdev;

	if(!(blkdev = blkdev_lookup(bf->bf_name))) {
		log(LOG_ERR, "blkdev of %s not found!\n", bf->bf_name);
		return ENODEV;
	}

	bf->bf_priv = blkdev;
	bf->bf_oflags = flags;
	bf->bf_base = bf->bf_offs = 0; 
	bf->bf_end = blkdev->bd_lba * blkdev->bd_blk_sz; 

	return 0;
}

static int blkfile_close(struct block_file *bf)
{
	struct block_device *blkdev;

	blkdev = (struct block_device *)bf->bf_priv;
	if(!blkdev)
		return EINVAL;
	--blkdev->bd_refcnt;
	bf->bf_priv = NULL;
	
	return 0;
}

/**
 * 描述：分配一个以blksz为边界对齐的缓冲区
 */
static inline uint8_t *__alloc_llbuf_aligned(ulong blksz, uint8_t **__raw_llbuf)
{
	if(!(*__raw_llbuf = (uint8_t *)kmem_malloc(blksz << 1)))
		return NULL;

	return (uint8_t *)(((ulong)(*__raw_llbuf) + blksz - 1) & ~(blksz - 1));//round blksz
}

/**
 * 描述：块设备文件传输
 * 注意：bad code,buf参数直接由应用层传来，此处未做转换直接使用
 * 		 该函数的大部分操作要求块大小应当为2的幂次方
 */
static ssize_t blkfile_xfer(struct block_file *bf, void *__buf, size_t nb, int is_w)
{
	int64_t res;
	uint8_t *llbuf = NULL, *__raw_llbuf, *buf = (uint8_t *)__buf;
	struct block_device *blkdev = (struct block_device *)bf->bf_priv;
	ulong suboffs, resid = nb, n, blksz = blkdev->bd_blk_sz, blkcnt;

	while(resid) {
		/* 处理数据未对齐到块边界的情况 */
		if((suboffs = (bf->bf_offs & (blksz - 1))) || (resid < blksz)) {
			if(!llbuf && !(llbuf = __alloc_llbuf_aligned(blksz, &__raw_llbuf))) {
				errno = ENOMEM;			
				return -1;
			}
			n = min(blksz - suboffs, resid);
			if((res = blkdev->bd_blk_ops.bo_read(blkdev, 
					bf->bf_offs >> blkdev->bd_blk_sz_log2, 1, llbuf)) < 0) 
				goto err;
			if(is_w) {
				/* 保证buf仅更新需要写进的数据而不会覆盖以前块内的老数据 */
				bcopy(buf, llbuf + suboffs, n);
				if((res = blkdev->bd_blk_ops.bo_write(blkdev, 
						bf->bf_offs >> blkdev->bd_blk_sz_log2, 1, llbuf)) < 0) 
					goto err;
			} else
				bcopy(llbuf + suboffs, buf, n);
		} else {
			n = resid & ~(blksz - 1);
			/* 注意：这里要求BLKFILE_MAX_IO_BYTES的大小必须大于一个块 */
			n = min(n, BLKFILE_MAX_IO_BYTES);
			blkcnt = n >> blkdev->bd_blk_sz_log2; 
			if(is_w)
				res = blkdev->bd_blk_ops.bo_write(blkdev, 
						bf->bf_offs >> blkdev->bd_blk_sz_log2, blkcnt, buf);
			else
				res = blkdev->bd_blk_ops.bo_read(blkdev, 
						bf->bf_offs >> blkdev->bd_blk_sz_log2, blkcnt, buf);
			if(res < 0)
				goto err;	
		}
		bf->bf_offs += n; 
		resid -= n;
		buf += n;
	}

	if(llbuf)
		kmem_free(__raw_llbuf);

	return nb - resid;

err:
	if(llbuf)
		kmem_free(__raw_llbuf);

	return -1;
}

/* bad code 读数据字节仅支持32位长度 */
static ssize_t blkfile_read(struct block_file *bf, void *buf, size_t len)
{
	size_t nb = len;
	struct block_device *blkdev;
	
	if(O_WRONLY == (bf->bf_oflags & O_ACCMODE)) {
		errno = EPERM;
		return -1;
	}

	blkdev = (struct block_device *)bf->bf_priv;
	if(!blkdev) {
		errno = EINVAL;
		return -1;
	}

	if(bf->bf_offs < 0 || bf->bf_offs > bf->bf_end)
		return 0;
	if(len > bf->bf_end - bf->bf_offs)
		nb = bf->bf_end - bf->bf_offs;

	return blkfile_xfer(bf, buf, nb, 0);
}

/* bad code 写数据字节仅支持32位长度 */
static ssize_t blkfile_write(struct block_file *bf, const void *buf, size_t len)
{
	size_t nb = len;
	struct block_device *blkdev;
	
	if(O_RDONLY == (bf->bf_oflags & O_ACCMODE)) {
		errno = EPERM;
		return -1;
	}

	blkdev = (struct block_device *)bf->bf_priv;
	if(!blkdev) {
		errno = EINVAL;
		return -1;
	}

	if(bf->bf_offs < 0 || bf->bf_offs > bf->bf_end)
		return 0;
	if(len > bf->bf_end - bf->bf_offs)
		nb = bf->bf_end - bf->bf_offs;

	return blkfile_xfer(bf, (void *)buf, nb, 1);
}

/* 64位lseek */
static loff_t blkfile_lseek(struct block_file *bf, loff_t offs, int whence)
{
	loff_t noff;

	switch(whence) {
		case SEEK_CUR:
			noff = bf->bf_offs + offs;
			if(noff < bf->bf_base || noff > bf->bf_end) {
				errno = EINVAL;
				return -1;
			}
			break;
		case SEEK_SET:
			noff = offs;
			if(noff < bf->bf_base || noff > bf->bf_end) {
				errno = EINVAL;
				return -1;
			}
			break;
		case SEEK_END:
		default:
			errno = EPERM;
			return -1;
	}
	bf->bf_offs = noff;

	return bf->bf_offs - bf->bf_base;
}

static int blkfile_ioctl(struct block_file *bf, unsigned long cmd, void *arg)
{
	int res;
	struct block_device *blkdev;
	uint8_t mbr[MBR_SECTOR_SZ];
	
	blkdev = (struct block_device *)bf->bf_priv;
	if(!blkdev) 
		return EINVAL;

	switch(cmd) {
		case BIOCGPARTINF:
			/**
 			 * bad code, 本来这里需要处理内核空间到用户空间的数据映射，
 			 * 但是这里为了简化程序和节省空间没这么做 
 			 */	
			if(!blkdev->bd_blk_ops.bo_read_partition)
				return EOPNOTSUPP;
			if(res = blkdev->bd_blk_ops.bo_read_partition(blkdev, 
					(struct partition_info *)arg))
				return res;
			break;
		case BIOCGMBRPART:
			/* 读取引导扇区 */
			if(!blkdev->bd_blk_ops.bo_read_mbr)
				return EOPNOTSUPP;
			if(res = blkdev->bd_blk_ops.bo_read_mbr(blkdev, mbr))
				return res;
			if(*(uint16_t *)(mbr + MBR_SECTOR_SZ - 2) != MBR_PART_MAGIC)
				return ENXIO;
			if(copyout((uint8_t *)arg, mbr + MBR_PART_OFFS, MBR_PART_SZ))
				return EIO;
			break;
		default:
			return ENOTTY;
	}

	return 0;
}

struct block_fileops blkops = {
	.bfo_open = blkfile_open,
	.bfo_close = blkfile_close,
	.bfo_read = blkfile_read,
	.bfo_write = blkfile_write,
	.bfo_lseek = blkfile_lseek,
	.bfo_ioctl = blkfile_ioctl, 
};

