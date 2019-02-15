
/**
 * Copyright(c) 2018-10-22 Shangwen Wu
 * AHCI驱动用到的相关定义
 * 
 */
#ifndef __ATA_AHCI_H__
#define __ATA_AHCI_H__

#include <sys/endian.h>

/* 下面大部分定义来自AHCI spec，目前参考ahci spec1.3.1 */

/* AHCI最大端口数 */
#define AHCI_MAX_PORTS		32

/* HBA MEMIO寄存器 */

/* Generic Host Control */
#define AHCI_CAP			0x00	/* HBA支持特性寄存器 */
#define AHCI_CAP_S64A		(1u << 31)	
#define AHCI_CAP_SNCQ		(1u << 30)	
#define AHCI_CAP_SSNTF		(1u << 29)	
#define AHCI_CAP_SMPS		(1u << 28)	
#define AHCI_CAP_SSS		(1u << 27)	/* Staggered Spin-up支持，该位为1时，软件手动复位port */
#define AHCI_CAP_SALP		(1u << 26)	
#define AHCI_CAP_SAL		(1u << 25)	
#define AHCI_CAP_SCLO		(1u << 24)	
#define AHCI_CAP_ISS_OFFS	20			/* 接口速度支持 */
#define AHCI_CAP_ISS_MASK	0x0f			
#define AHCI_CAP_SAM		(1u << 18)	/* 仅支持AHCI */
#define AHCI_CAP_SPM		(1u << 17)	
#define AHCI_CAP_FBSS		(1u << 16)	
#define AHCI_CAP_PMD		(1u << 15)	
#define AHCI_CAP_SSC		(1u << 14)	
#define AHCI_CAP_PSC		(1u << 13)	
#define AHCI_CAP_NCS_OFFS	8			/* 命令slot支持个数，0表示最小1个，最大32 */
#define AHCI_CAP_NCS_MASK	0x1f
#define AHCI_CAP_CCCS		(1u << 7)	
#define AHCI_CAP_EMS		(1u << 6)	
#define AHCI_CAP_SXS		(1u << 5)	
#define AHCI_CAP_NP_OFFS	0			/* port支持个数，0表示最小1个，最大32 */
#define AHCI_CAP_NP_MASK	0x1f

#define AHCI_GHC			0x04	/* 全局控制寄存器 */
#define AHCI_GHC_AE			(1u << 31)	/* AHCI使能，当CAP.SAM为1时，该位始终为1 */
#define AHCI_GHC_IE			(1u << 1)	/* 全局中断使能 */
#define AHCI_GHC_HR			(1u << 0)	/* HBA复位 */

#define AHCI_IS				0x08	/* 中断状态寄存器 */
#define AHCI_PI				0x0c	/* 端口实现位图寄存器 */
#define AHCI_VS				0x10	/* 版本寄存器 */
#define AHCI_CCC_CTL		0x14	/* “命令完成接合”控制寄存器 */
#define AHCI_CCC_PORTS		0x18	/* “命令完成接合”端口支持寄存器 */
#define AHCI_EN_LOC			0x1c	/* “环绕管理”消息指针寄存器 */
#define AHCI_EM_CTL			0x20	/* “环绕管理”控制寄存器 */

#define AHCI_CAP2			0x24	/* 扩展特性寄存器 */
#define AHCI_CAP2_DESO		(1u << 5)
#define AHCI_CAP2_SADM		(1u << 4)
#define AHCI_CAP2_SDS		(1u << 3)
#define AHCI_CAP2_APST		(1u << 2)
#define AHCI_CAP2_NVMP		(1u << 1)
#define AHCI_CAP2_BOH		(1u << 0)

#define AHCI_BOHC			0x28	/* BIOS/OS握手控制与状态寄存器 */

/* Port Registers */
#define AHCI_PORT_REG_OFFS	0x100	
#define AHCI_PORT_REG_SZ	0x80
#define AHCI_PxCLB			0x00	/* Port x Command List基址寄存器 */
#define AHCI_PxCLBU			0x04	/* Port x Command List基址高32位（如果支持64位地址）寄存器 */
#define AHCI_PxFB			0x08	/* Port x 接收FIS基址寄存器 */
#define AHCI_PxFBU			0x0c	/* Port x 接收基址高32位（如果支持64位地址）寄存器 */
#define AHCI_PxIS			0x10	/* Port x 中断状态寄存器 */

#define AHCI_PxIE			0x14	/* Port x 中断使能寄存器 */
#define AHCI_PxIE_CPDE		(1u << 31)
#define AHCI_PxIE_TFEE		(1u << 30)
#define AHCI_PxIE_HBFE		(1u << 29)
#define AHCI_PxIE_HBDE		(1u << 28)
#define AHCI_PxIE_IFE		(1u << 27)
#define AHCI_PxIE_INFE		(1u << 26)
#define AHCI_PxIE_OFE		(1u << 24)
#define AHCI_PxIE_IPME		(1u << 23)
#define AHCI_PxIE_PRCE		(1u << 22)
#define AHCI_PxIE_DMPE		(1u << 7)
#define AHCI_PxIE_PCE		(1u << 6)
#define AHCI_PxIE_DPE		(1u << 5)
#define AHCI_PxIE_UFE		(1u << 4)
#define AHCI_PxIE_SDBE		(1u << 3)
#define AHCI_PxIE_DSE		(1u << 2)
#define AHCI_PxIE_PSE		(1u << 1)
#define AHCI_PxIE_DHRE		(1u << 0)

#define PORT_IRQ_FATAL		(AHCI_PxIE_TFEE | AHCI_PxIE_HBFE | \
								AHCI_PxIE_HBDE | AHCI_PxIE_IFE)
#define PORT_IRQ_DEFAULT	(PORT_IRQ_FATAL | AHCI_PxIE_PRCE | \
								AHCI_PxIE_DMPE | AHCI_PxIE_PCE | \
								AHCI_PxIE_DPE | AHCI_PxIE_UFE | \
								AHCI_PxIE_SDBE | AHCI_PxIE_DSE | \
								AHCI_PxIE_PSE | AHCI_PxIE_DHRE)

#define AHCI_PxCMD			0x18	/* Port x 命令与状态寄存器 */
#define AHCI_PxCMD_CPD		(1u << 20)
#define AHCI_PxCMD_CR		(1u << 15)
#define AHCI_PxCMD_FR		(1u << 14)
#define AHCI_PxCMD_FRE		(1u << 4)
#define AHCI_PxCMD_COL		(1u << 3)
#define AHCI_PxCMD_POD		(1u << 2)
#define AHCI_PxCMD_SUD		(1u << 1)
#define AHCI_PxCMD_ST		(1u << 0)

#define AHCI_PORT_RUNNING	(AHCI_PxCMD_CR | AHCI_PxCMD_FR | AHCI_PxCMD_FRE | AHCI_PxCMD_ST)

#define AHCI_Reserved		0x1c	/* 保留 */

#define AHCI_PxTFD			0x20	/* Port x Task File Data寄存器 */
#define AHCI_PxTFD_BSY		(1u << 7)
#define AHCI_PxTFD_DRQ		(1u << 3)

#define AHCI_PxSIG			0x24	/* Port x 签名寄存器 */

#define AHCI_PxSSTS			0x28	/* Port x SATA状态寄存器 */
#define AHCI_PxSSTS_DET_OFFS	0
#define AHCI_PxSSTS_DET_MASK	0x0f

#define AHCI_PxSCTL			0x2c	/* Port x SATA控制寄存器 */
#define AHCI_PxSCTL_DET_OFFS	0
#define AHCI_PxSCTL_DET_MASK	0x0f

#define AHCI_PxSERR			0x30	/* Port x SATA错误寄存器 */
#define AHCI_PxSACT			0x34	/* Port x NCQ命令有效寄存器 */
#define AHCI_PxCI			0x38	/* Port x 命令发射寄存器 */
#define AHCI_PxSNTF			0x3c	/* Port x SATA通知寄存器 */
#define AHCI_PxFBS			0x40	/* Port x FIS-Base switching控制寄存器 */
#define AHCI_PxDEVSLP		0x44	/* Port x Device Sleep相关寄存器 */

#define ATA_FLAG_SATA		0x01
#define ATA_FLAG_PIO_DMA	0x02
#define ATA_FLAG_MMIO		0x04 	/* 使用BAR空间访问，而非IDP端口 */
#define ATA_FLAG_NO_LEGACY	0x08	/* 非遗留（IDE）接口 */ 
#define ATA_FLAG_NO_ATAPI	0x10	/* 非ATAPI命令 */


#define AHCI_MAX_PRD_COUNT		56
#define AHCI_MAX_PRD_DATA_SZ	(4 * 1024 * 1024)
#define AHCI_RX_FIS_SZ			256
#define AHCI_CMD_SLOT_SZ		32
#define AHCI_CMD_TLB_HDR_SZ		0x80
#define AHCI_CMD_TLB_CFIS_SZ	0x40
#define AHCI_PRD_TLB_ITEM_SZ	16
#define AHCI_CMD_TLB_SZ			(AHCI_CMD_TLB_HDR_SZ + AHCI_MAX_PRD_COUNT * AHCI_PRD_TLB_ITEM_SZ)
#define AHCI_PORT_PRIV_DMA_SZ	(AHCI_CMD_SLOT_SZ + AHCI_RX_FIS_SZ + AHCI_CMD_TLB_SZ)

/* 硬盘标准配置 */
#define ATA_FLAG_COMM		(ATA_FLAG_SATA | ATA_FLAG_PIO_DMA | ATA_FLAG_MMIO | \
								ATA_FLAG_NO_LEGACY | ATA_FLAG_NO_ATAPI | ATA_FLAG_NO_ATAPI)

struct device;
struct ahci_host;
struct ata_port;

/* AHCI端口info抽象定义 */
struct ahci_port_info {
	ulong mmio;
	struct ahci_host *host;
	int id;
};

/* HBA抽象结构定义 */
struct ahci_host {
	struct device dev;
	ulong mmio_base;			//HBA MMIO
	uint32_t flags;				//某些标识位
	uint32_t nports;			//记录当前HBA实现端口个数
	uint32_t cap;				//保存HBA CAP寄存值
	uint32_t cap2;				//保存HBA CAP2寄存器值
	uint32_t port_map;			//保存HBA PI寄存器值
	uint32_t link_map;			//port_map子集，表示已建立通信的端口
	struct ahci_port_info ports[AHCI_MAX_PORTS];	//保存每个端口描述
};

struct ahci_port_priv {
	dma_addr_t cmd_list_dma;
	dma_addr_t rx_fis_dma;
	caddr_t cmd_list_base;
	caddr_t rx_fis_base;
	caddr_t cmd_tbl;
	caddr_t prd_tbl_base;
	ulong cmd_fis_len;
	ulong acmd_len;
};

#define AHCI_CMD_FLAGS_CLR		(1u << 10)
#define AHCI_CMD_FLAGS_BIST		(1u << 9)
#define AHCI_CMD_FLAGS_RESET	(1u << 8)
#define AHCI_CMD_FLAGS_PREF		(1u << 7)
#define AHCI_CMD_FLAGS_WRITE	(1u << 6)
#define AHCI_CMD_FLAGS_ATAPI	(1u << 5)

struct ahci_cmd_hdr {
	union {
#if BYTE_ORDER == LITTLE_ENDIAN
		uint32_t u_cfl:5;
		uint32_t u_flags_a:1;
		uint32_t u_flags_w:1;
		uint32_t u_flags_p:1;
		uint32_t u_flags_r:1;
		uint32_t u_flags_b:1;
		uint32_t u_flags_c:1;
		uint32_t u_resd1:1;
		uint32_t u_pmp:4;
		uint32_t u_prdtl:16;
#elif BYTE_ORDER == BIG_ENDIAN
		uint32_t u_prdtl:16;
		uint32_t u_pmp:4;
		uint32_t u_resd1:1;
		uint32_t u_flags_c:1;
		uint32_t u_flags_b:1;
		uint32_t u_flags_r:1;
		uint32_t u_flags_p:1;
		uint32_t u_flags_w:1;
		uint32_t u_flags_a:1;
		uint32_t u_cfl:5;
#endif
		uint32_t u_opts;
	} opts_u;
#define opts_field 			opts_u.u_opts
#define opts_prdtl  		opts_u.u_prdtl
#define opts_pmp    		opts_u.u_pmp
#define opts_resd1  		opts_u.u_resd1
#define opts_flags_c		opts_u.u_flags_c
#define opts_flags_b		opts_u.u_flags_b
#define opts_flags_r		opts_u.u_flags_r
#define opts_flags_p		opts_u.u_flags_p
#define opts_flags_w		opts_u.u_flags_w
#define opts_flags_a		opts_u.u_flags_a
#define opts_cfl 		opts_u.u_cfl
	uint32_t prdbc;
	uint32_t ctba;
	uint32_t ctba_hi;
	uint32_t resd2;
	uint32_t resd3;
	uint32_t resd4;
	uint32_t resd5;
};

#if 1
struct ahci_prd {
	uint32_t dba;
	uint32_t dba_hi;
	uint32_t resd1;
#if BYTE_ORDER == LITTLE_ENDIAN
	uint32_t dbc:22;
	uint32_t resd2:9;
	uint32_t flags_i:1;
#elif BYTE_ORDER == BIG_ENDIAN
	uint32_t flags_i:1;
	uint32_t resd2:9;
	uint32_t dbc:22;
#endif
};
#endif

extern struct ata_port_operations ahci_ops;

static inline ulong ahci_port_base(struct ahci_host *host, int i)
{
	return host->mmio_base + AHCI_PORT_REG_OFFS + (AHCI_PORT_REG_SZ * i);
}

static inline int ahci_nr_ports(uint32_t cap)
{
	return ((cap >> AHCI_CAP_NP_OFFS) & AHCI_CAP_NP_MASK) + 1;
}

extern void ahci_enable_ahci(void *);
extern int ahci_reset_controller(struct ahci_host *);
extern int ahci_init_port(struct ahci_host *, int);
extern void ahci_enable_global_irq(struct ahci_host *);
extern void ahci_disable_global_irq(struct ahci_host *);
extern void ahci_print_info(struct ahci_host *, const char *);
extern void ahci_save_initial_config(struct ahci_host *);
extern void ahci_restore_initial_config(struct ahci_host *);
extern int ahci_port_start(struct ata_port *);
extern int ahci_start_port(struct ata_port *);
extern int ahci_stop_port(struct ata_port *);
extern int ahci_start_cmd_engine(struct ata_port *);
extern int ahci_stop_cmd_engine(struct ata_port *);
extern int ahci_start_recv_fix(struct ata_port *);
extern int ahci_stop_recv_fix(struct ata_port *);
extern void ahci_port_poweron(struct ata_port *);
extern size_t ahci_device_data_xfer(struct ata_device *, caddr_t, ulong, int, int);
extern ulong ahci_fill_prd(struct ata_port *, caddr_t, ulong, int);
extern void ahci_fill_cmd_hdr(struct ata_port *, uint32_t);
extern int ahci_check_ready(struct ata_port *ap, ulong mtimeout);

#endif //__ATA_AHCI_H__

