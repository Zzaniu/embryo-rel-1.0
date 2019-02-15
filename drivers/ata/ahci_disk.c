
/**
 * Copyright(c) 2018-10-25 Shangwen Wu
 * AHCI硬盘驱动
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/delay.h>
#include <sys/syslog.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/err.h>
#include <sys/block.h>
#include <sys/mbr_part.h>
#include <sys/partition.h>
#include <ata/ata.h>
#include <ata/ahci.h>
#include <asm/io.h>

static int64_t disk_blk_write(struct block_device *bd, uint64_t startblk, 
						ulong blkcnt, const void *buf)
{
	struct ata_device *adev = (struct ata_device *)bd->bd_priv;

	return ata_blk_write(adev, startblk, blkcnt, buf);
}

static int64_t disk_blk_read(struct block_device *bd, uint64_t startblk, 
						ulong blkcnt, void *buf)
{
	struct ata_device *adev = (struct ata_device *)bd->bd_priv;

	return ata_blk_read(adev, startblk, blkcnt, buf);
}

static int disk_read_mbr(struct block_device *bd, uint8_t *mbrbuf)
{
	ulong blkcnt;
	int64_t res;
	struct ata_device *adev = (struct ata_device *)bd->bd_priv;

	if(bd->bd_part_type != BLOCK_PART_TYPE_DOS) 	//目前硬盘仅支持读MBR分区信息
		return EOPNOTSUPP;

	blkcnt = (MBR_SECTOR_SZ + bd->bd_blk_sz - 1) / bd->bd_blk_sz;
	res = ata_blk_read(adev, MBR_SECTOR_NUM, blkcnt, mbrbuf);

	return res < 0 ? errno : 0;
}

static int disk_read_partition(struct block_device *bd, struct partition_info *parttab)
{
	int ret, i;
	uint8_t mbr[MBR_SECTOR_SZ];
	struct mbr_partition *mbrpart;
	struct partition_info *part = parttab;
	char *cp;

	if(bd->bd_part_type != BLOCK_PART_TYPE_DOS) 	//目前硬盘支持读MBR分区信息
		return EOPNOTSUPP;
	
	if(ret = disk_read_mbr(bd, mbr))
		return ret;

	/* no valid partition */
	if(*(uint16_t *)(mbr + MBR_SECTOR_SZ - 2) != MBR_PART_MAGIC)
		return ENXIO;

	bzero(parttab, sizeof(struct partition_info) * PART_INFO_MAX);
	//mbrpart = (struct mbr_partition *)(mbr + MBR_PART_OFFS);		//bug!!! unalinged access
	if(!(mbrpart = (struct mbr_partition *)kmem_malloc(MBR_PART_SZ)))
		return ENOMEM;
	bcopy(mbr + MBR_PART_OFFS, (uint8_t *)mbrpart, MBR_PART_SZ);
	for(i = 0; i < PART_INFO_MAX; ++i) {
		if(!mbrpart->start_head && !mbrpart->end_head &&
				!mbrpart->start_sector && !mbrpart->end_sector &&
				!mbrpart->start_cylinder && !mbrpart->end_cylinder) {
			part->pi_valid = 0;	
			continue;
		}
		if(strlen(bd->bd_name) + 3 > PART_NAME_MAX) {
			kmem_free(mbrpart);
			return ENAMETOOLONG;
		}
		strcpy(part->pi_name, bd->bd_name);
		cp = part->pi_name + strlen(part->pi_name);
		*cp++ = '-'; *cp++ = i + '1'; *cp++ = 0;
		part->pi_valid = 1;	
		part->pi_boot = (BOOT_ACTIVE_PARTITION == mbrpart->boot_signature ? 1 : 0);
		part->pi_sysid = mbrpart->system_signature;
		part->pi_blks = mbrpart->sectors_total / (SZ_1K / bd->bd_blk_sz);
		part->pi_start = mbrpart->before_part_sector;
		part->pi_end = mbrpart->before_part_sector + mbrpart->sectors_total - 1;//-1?????
		++mbrpart;
		++part;
	}
	kmem_free(mbrpart);
	
	return 0;
}

static int ahci_disk_device_init(struct ata_device *adev)
{
	int err = 0;
	uint16_t *ata_id;
	struct ata_port *ap = adev->port;
	struct block_device *blkdev = adev->blkdev;

	if(IS_ERR(ata_id = ata_device_identify(adev))) {
		log(LOG_ERR, "inquery devive identify failed\n");
		return PTR_ERR(ata_id);
	}
	adev->devid = ata_id;

	/* get count of sectors */
	blkdev->bd_removeable = !!ata_id_removeable(ata_id);
	blkdev->bd_has_lba48 = !!ata_id_has_lba64(ata_id);
	if(blkdev->bd_has_lba48)
		blkdev->bd_maxblks = ATA_MAX_SECTORS_LBA48;
	else
		blkdev->bd_maxblks = ATA_MAX_SECTORS;
	blkdev->bd_lba = ata_id_n_sectors(ata_id);
	log(LOG_DEBUG, "Sect Info: %llu sectors %s\n", 
		blkdev->bd_lba, blkdev->bd_has_lba48 ? "has lba48": "");

	/* set xfer mode */
	adev->xfermode = XFER_UDMA6;
	if(err = ata_dev_set_xfermode(adev)) {
		log(LOG_ERR, "set devive xfer mode failed\n");
		goto failed;
	}
	log(LOG_DEBUG, "Set port xfer mode: 0x%x\n", adev->xfermode);

	/* software reset ata device */
	if(err = ata_device_softreset(adev)) {
		log(LOG_ERR, "devive softreset failed\n");
		goto failed;
	}

	return 0;

failed:
	if(adev->devid)
		kmem_free(adev->devid);

	return err;	
}

static int ahci_disk_match(struct device *parent, void *match, void *aux)
{
	uint32_t val;
	struct ahci_port_info *pi = (struct ahci_port_info *)aux;

	val = readl(pi->mmio + AHCI_PxSIG);
	/* 没有设备链接时，PxSIG寄存器返回全f */
	if(0xffffffff == val)
		return 0;

	/* Packet Command(ATAPI) SIG */
	if(0xeb14 == (val >> 16)) 
		return 0;

	return 1;
}

static int ahci_disk_attach(struct device *parent, struct device *self, void *aux)
{
	int err;
	struct block_device *blkdev;
	struct ata_port *ap;
	struct ahci_port_priv *priv;
	struct ahci_port_info *pi = (struct ahci_port_info *)aux;
	struct ata_device *adev = (struct ata_device *)self;

	log(LOG_DEBUG, "in %s attach function\n", self->dv_name);
	
	/* 初始化一个ATA设备 */
	if(!(ap = (struct ata_port *)kmem_malloc(sizeof(struct ata_port)))) {
		log(LOG_ERR, "ata port alloc failed\n");	
		err = ENOMEM;
		return err;
	}
	ap->port_mmio = pi->mmio;
	ap->port_id = pi->id;
	ap->port_host = pi->host;
	ap->port_attachdev = adev;
	ap->port_ops = &ahci_ops;
	adev->port = ap;	
	if(!(priv = (struct ahci_port_priv *)kmem_zmalloc(sizeof(struct ahci_port_priv)))) {
		log(LOG_ERR, "port priv data alloc failed\n");	
		err = ENOMEM;
		goto free_port;
	}
	adev->port->port_priv = priv;	

	/* 初始化一个块设备 */
	if(!(blkdev = (struct block_device *)kmem_zmalloc(sizeof(struct block_device)))) {
		log(LOG_ERR, "block device alloc failed\n");	
		err = ENOMEM;
		goto free_pri;
	}
	blkdev->bd_refcnt = 1;
	blkdev->bd_if_type = BLOCK_IF_TYPE_SATA;
	blkdev->bd_type = BLOCK_DEV_TYPE_HARDDISK;
	blkdev->bd_part_type = BLOCK_PART_TYPE_DOS;	//bad code 目前硬盘仅支持DOS（MBR）分区格式
	blkdev->bd_blk_sz = ATA_SECT_SIZE;
	blkdev->bd_blk_sz_log2 = ATA_SECT_SIZE_LOG2;//此处要求块大小应当为2的幂次方
	blkdev->bd_blk_ops.bo_read = disk_blk_read;
	blkdev->bd_blk_ops.bo_write = disk_blk_write;
	blkdev->bd_blk_ops.bo_read_mbr = disk_read_mbr;
	blkdev->bd_blk_ops.bo_read_partition = disk_read_partition;
	blkdev->bd_priv = adev;
	blkdev->bd_name = adev->dev.dv_name;
	blkdev_register(blkdev);
	adev->blkdev = blkdev;
	adev->is_atapi = 0;
	adev->pmp = 0;		//disable PM

	if(err = ahci_port_start(adev->port)) {
		log(LOG_ERR, "ahci_port_start failed\n");	
		goto free_blkdev;	
	}

	if(err = ahci_disk_device_init(adev)) {
		log(LOG_ERR, "disk device init failed\n");	
		goto stop_port;	
	}

	return 0;

stop_port:
	ahci_stop_port(adev->port);
free_blkdev:
	if(adev->blkdev) {
		blkdev_unregister(adev->blkdev);
		kmem_free(adev->blkdev);
	}
free_pri:
	if(adev->port->port_priv)
		kmem_free(adev->port->port_priv);
free_port:
	if(adev->port)
		kmem_free(adev->port);
	
	return err;
}

static void ahci_disk_detach(struct device *parent, struct device *self, void *aux)
{
	struct ata_device *adev = (struct ata_device *)self;
	struct ata_port *ap = adev->port;

	log(LOG_DEBUG, "in %s deattach function\n", self->dv_name);

	if(--adev->blkdev->bd_refcnt) {
		log(LOG_ERR, "%s deattach failed, blkdev is busy, refcnt=%lu\n", 
				self->dv_name, adev->blkdev->bd_refcnt);
		return;
	}
	kmem_free(adev->devid);
	ahci_stop_port(ap);
	blkdev_unregister(adev->blkdev);
	kmem_free(adev->blkdev);
	kmem_free(ap->port_priv);
	kmem_free(ap);
}

struct cfattach ahci_disk_ca = {
	sizeof(struct ata_device),
	ahci_disk_match,
	ahci_disk_attach,
	ahci_disk_detach,
};

struct cfdriver ahci_disk_cd = {
	LIST_HEAD_INIT(ahci_disk_cd.cd_devlist),
	"ahci_disk",
	DEV_DISK,
	0,
};

