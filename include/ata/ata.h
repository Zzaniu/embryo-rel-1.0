
/**
 * Copyright(c) 2018-10-22 Shangwen Wu
 * ATA驱动用到的相关定义
 * 
 */
#ifndef __ATA_ATA_H__
#define __ATA_ATA_H__

#include <sys/endian.h>

#define ATA_SECT_SIZE_LOG2		9			
#define ATA_SECT_SIZE			(1 << ATA_SECT_SIZE_LOG2)	//512B
#define ATA_MAX_SECTORS_LBA48	65535
#define ATA_MAX_SECTORS			256

#define ATA_READ				0
#define ATA_WRITE				1

/* bits in ATA command block registers */
#define	ATA_SRST				(1u << 2)	/* software reset */

/* ATA device commands */
#define ATA_CMD_ID_ATA			0xec
#define ATA_CMD_ID_ATAPI		0xa1
#define ATA_CMD_SET_FEATURES	0xef
#define ATA_CMD_WRITE_EXT		0x35
#define ATA_CMD_READ_EXT		0x25
#define ATA_CMD_READ			0xc8
#define ATA_CMD_WRITE			0xca
#define ATA_CMD_PACKET			0xa0

/* ATAPI command stuff */
#define ATAPI_CMD_LEN12			12
#define ATAPI_CMD_LEN16			16
#define ATAPI_PKT_DMA			(1u << 0)
#define ATAPI_DMA_READ			(1u << 2)

#define GPCMD_READ_10			0x28
#define GPCMD_WRITE_10			0x2a

/* Device Identify */
#define ATA_ID_WORDS			256
#define ATA_ID_CONFIG			0
#define ATA_ID_CYLS				1
#define ATA_ID_HEADS			3
#define ATA_ID_SECTORS			6
#define ATA_ID_SERNO			10
#define ATA_ID_BUF_SIZE			21
#define ATA_ID_FW_REV			23
#define ATA_ID_PROD				27
#define ATA_ID_MAX_MULTSECT		47
#define ATA_ID_DWORD_IO			48
#define ATA_ID_CAPABILITY		49
#define ATA_ID_OLD_PIO_MODES	51
#define ATA_ID_OLD_DMA_MODES	52
#define ATA_ID_FIELD_VALID		53
#define ATA_ID_CUR_CYLS			54
#define ATA_ID_CUR_HEADS		55
#define ATA_ID_CUR_SECTORS		56
#define ATA_ID_MULTSECT			59
#define ATA_ID_LBA_CAPACITY		60
#define ATA_ID_SWDMA_MODES		62
#define ATA_ID_MWDMA_MODES		63
#define ATA_ID_PIO_MODES		64
#define ATA_ID_EIDE_DMA_MIN		65
#define ATA_ID_EIDE_DMA_TIME	66
#define ATA_ID_EIDE_PIO			67
#define ATA_ID_EIDE_PIO_IORDY	68
#define ATA_ID_ADDITIONAL_SUPP	69
#define ATA_ID_QUEUE_DEPTH		75
#define ATA_ID_MAJOR_VER		80
#define ATA_ID_COMMAND_SET_1	82
#define ATA_ID_COMMAND_SET_2	83
#define ATA_ID_CFSSE			84
#define ATA_ID_CFS_ENABLE_1		85
#define ATA_ID_CFS_ENABLE_2		86
#define ATA_ID_CSF_DEFAULT		87
#define ATA_ID_UDMA_MODES		88
#define ATA_ID_HW_CONFIG		93
#define ATA_ID_SPG				98
#define ATA_ID_LBA_CAPACITY_2	100
#define ATA_ID_SECTOR_SIZE		106
#define ATA_ID_LAST_LUN			126
#define ATA_ID_DLF				128
#define ATA_ID_CSFO				129
#define ATA_ID_CFA_POWER		160
#define ATA_ID_CFA_KEY_MGMT		162
#define ATA_ID_CFA_MODES		163
#define ATA_ID_DATA_SET_MGMT	169
#define ATA_ID_ROT_SPEED		217

#define ATA_ID_SERNO_LEN		20
#define ATA_ID_FW_REV_LEN		8
#define ATA_ID_PROD_LEN			40

#define ata_id_is_ata(id)		(!((id)[ATA_ID_CONFIG] & (1 << 15)))
#define ata_id_has_lba(id)		((id)[ATA_ID_CAPABILITY] & (1 << 9))
#define ata_id_has_dma(id)		((id)[ATA_ID_CAPABILITY] & (1 << 8))
#define ata_id_has_ncq(id)		((id)[76] & (1 << 8))
#define ata_id_removeable(id)	((id)[ATA_ID_CONFIG] & (1 << 7))
#define ata_id_u32(id, n)		\
	(((uint32_t)(id)[(n) + 1] << 16) | \
	((uint32_t)(id)[(n)]))
#define ata_id_u64(id, n)		\
	(((uint64_t)(id)[(n) + 3] << 48) | \
	((uint64_t)(id)[(n) + 2] << 32) | \
	((uint64_t)(id)[(n) + 1] << 16) | \
	((uint64_t)(id)[(n)]))

/* Device supported's Xfer mode */
#define	ATA_UDMA0				(1 << 0)
#define	ATA_UDMA1				(ATA_UDMA0 | (1 << 1))
#define	ATA_UDMA2				(ATA_UDMA1 | (1 << 2))
#define	ATA_UDMA3				(ATA_UDMA2 | (1 << 3))
#define	ATA_UDMA4				(ATA_UDMA3 | (1 << 4))
#define	ATA_UDMA5				(ATA_UDMA4 | (1 << 5))
#define	ATA_UDMA6				(ATA_UDMA5 | (1 << 6))
#define	ATA_UDMA7				(ATA_UDMA6 | (1 << 7))

/* SET FEATURE subcommand code */
#define SETFEATURES_XFER		0x03

/* Xfer mode */
#define	XFER_UDMA7				0x47
#define	XFER_UDMA6				0x46
#define	XFER_UDMA5				0x45
#define	XFER_UDMA4				0x44
#define	XFER_UDMA3				0x43
#define	XFER_UDMA2				0x42
#define	XFER_UDMA1				0x41
#define	XFER_UDMA0				0x40
#define	XFER_MW_DMA4			0x24	/* CFA only */
#define	XFER_MW_DMA3			0x23	/* CFA only */
#define	XFER_MW_DMA2			0x22
#define	XFER_MW_DMA1			0x21
#define	XFER_MW_DMA0			0x20
#define	XFER_SW_DMA2			0x12
#define	XFER_SW_DMA1			0x11
#define	XFER_SW_DMA0			0x10
#define	XFER_PIO6				0x0E	/* CFA only */
#define	XFER_PIO5				0x0D	/* CFA only */
#define	XFER_PIO4				0x0C
#define	XFER_PIO3				0x0B
#define	XFER_PIO2				0x0A
#define	XFER_PIO1				0x09
#define	XFER_PIO0				0x08
#define	XFER_PIO_SLOW			0x00

static inline int ata_id_has_lba64(uint16_t *id)
{
	if((id[ATA_ID_COMMAND_SET_2] & 0xc000) != 0x4000)
		return 0;
	if(!ata_id_u64(id, ATA_ID_LBA_CAPACITY_2))
		return 0;
	return id[ATA_ID_COMMAND_SET_2] & (1 << 10);
}

struct device;
struct block_device;
struct ata_port;

/* Command FIS structures */
#define FIS_TYPE_H2D_REG		0x27
#define FIS_TYPE_D2H_REG		0x34
#define FIS_TYPE_DMA_ACT		0x39
#define FIS_TYPE_DMA_SETUP		0x41
#define FIS_TYPE_DATA			0x46
#define FIS_TYPE_BIST_ACT		0x58
#define FIS_TYPE_PIO_SETUP		0x5f
#define FIS_TYPE_SET_DEV_BITS	0xa1

/* Host to Device Register FIS */
struct h2d_fis {
	uint8_t fis_type;
#if BYTE_ORDER == LITTLE_ENDIAN
	uint8_t pmp:4;
	uint8_t reserved1:3;
	uint8_t flags_c:1;
#elif BYTE_ORDER == BIG_ENDIAN
	uint8_t flags_c:1;
	uint8_t reserved1:3;
	uint8_t pmp:4;
#endif
	uint8_t command;
	uint8_t features_0_7;
	uint8_t lba_0_7;
	uint8_t lba_8_15;
	uint8_t lba_16_23;
	uint8_t device;
	uint8_t lba_24_31;
	uint8_t lba_32_39;
	uint8_t lba_40_47;
	uint8_t features_8_15;
	uint8_t count_0_7;
	uint8_t count_8_15;
	uint8_t icc;
	uint8_t control;
	uint8_t reserved2[4];
};

/* 描述ATA设备 */
struct ata_device {
	struct device dev;
	struct ata_port *port;
	struct block_device *blkdev;
	uint16_t *devid;
	uint16_t pmp;
	uint8_t is_atapi;
	uint8_t xfermode;
};

struct ata_port_operations {
	int (*read_id)(struct ata_device *, uint16_t *);
	int (*set_xfermode)(struct ata_device *, uint8_t);
	int (*softreset)(struct ata_device *);
	int (*blk_data_rw)(struct ata_device *, uint64_t, ulong, void *, int);
};

/* ATA端口抽象定义 */
struct ata_port {
	struct ata_device *port_attachdev;
	ulong port_mmio;
	struct ata_port_operations *port_ops;
	void *port_host;
	void *port_priv;
	int port_id;
};

extern uint32_t ata_wait_register(void *reg, uint32_t mask, uint32_t val,
								long minterval, long mtimeout);//defined in libata.c
extern int ata_wait_cmd_completed(void *reg, uint32_t mask, long mtimeout);
extern uint16_t *ata_device_identify(struct ata_device  *adev);
extern uint64_t ata_id_n_sectors(uint16_t *id);
extern int ata_dev_set_xfermode(struct ata_device *adev);
extern int ata_device_softreset(struct ata_device *adev);
extern int64_t ata_blk_write(struct ata_device *adev, uint64_t startblk, 
						ulong blkcnt, const void *buf);
extern int64_t ata_blk_read(struct ata_device *adev, uint64_t startblk, 
						ulong blkcnt, void *buf);
extern int64_t ata_blk_data_rw(struct ata_device *adev, uint64_t startblk, 
						ulong blkcnt, void *buf, int op);

#endif //__ATA_ATA_H__

