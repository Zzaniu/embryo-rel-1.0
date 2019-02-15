
/**
 * Copyright(c) 2018-10-22 Shangwen Wu
 * AHCI通用函数 
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/syslog.h>
#include <sys/errno.h>
#include <sys/endian.h>
#include <sys/malloc.h>
#include <sys/delay.h>
#include <sys/device.h>
#include <sys/block.h>
#include <ata/ata.h>
#include <ata/ahci.h>
#include <asm/io.h>
#include <asm/cpu.h>
#include <asm/cache.h>
//#include <asm/dma-mapping.h>

/* AHCI驱动调试开关 */
#define AHCI_DEBUG		0
#if AHCI_DEBUG
#define AHCI_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define AHCI_DBG(fmt, args...)		do{}while(0)	
#endif

/**
 * 描述：检查当前端口是否处于空间状态
 */
int ahci_check_ready(struct ata_port *ap, ulong mtimeout)
{
	uint32_t val;
	
	while(mtimeout--) {
		val = readl(ap->port_mmio + AHCI_PxTFD);
		if(!(val & AHCI_PxTFD_BSY))
			return 0;
		if(0xff == (val & 0xff))
			return ENODEV;
		mdelay(1);
	}

	return ETIMEDOUT;
}

/** 
 * 描述：重启cmd list DMA引擎，并确保DRQ以及BSY信号空闲
 */ 
int ahci_kick_engine(struct ata_port *ap)
{
	int err;
	uint32_t val;
	struct ahci_host *host = (struct ahci_host *)ap->port_host;

	if(err = ahci_stop_cmd_engine(ap))
		return err;

	val = readl(ap->port_mmio + AHCI_PxTFD);
	if(!(val & (AHCI_PxTFD_BSY | AHCI_PxTFD_DRQ)))
		goto start_engine;

	if(!(host->cap & AHCI_CAP_SCLO))
		return EOPNOTSUPP;

	val = readl(ap->port_mmio + AHCI_PxCMD);
	val |= AHCI_PxCMD_COL;
	writel_with_flush(val, ap->port_mmio + AHCI_PxCMD);//issue cmd slot0
	if(ata_wait_cmd_completed((void *)(ap->port_mmio + AHCI_PxCMD), AHCI_PxCMD_COL, 1000)) 
		return EIO;

start_engine:
	
	ahci_start_cmd_engine(ap);

	return 0;
}

/**
 * 描述：分配FIS以及CMD SLOT空间，并将port进入运行状态
 */
int ahci_port_start(struct ata_port *ap)
{
	ulong buf;
	struct ahci_host *host = (struct ahci_host *)ap->port_host;
	struct ahci_port_priv *pp = (struct ahci_port_priv *)ap->port_priv;
		
	if(!(buf = (ulong)kmem_zmalloc(2048 + AHCI_PORT_PRIV_DMA_SZ))) 
		return ENOMEM;
	buf = (buf + 2048 - 1) & ~0x7ff;//2K字节对齐,AHCI规范要求

	pp->cmd_list_base = (caddr_t)buf;
	buf += AHCI_CMD_SLOT_SZ + 224;	//保证rx_fis_base为256字节对齐
	pp->rx_fis_base = (caddr_t)buf;
	buf += AHCI_RX_FIS_SZ;
	pp->cmd_tbl = (caddr_t)buf;
	buf += AHCI_CMD_TLB_HDR_SZ;
	pp->prd_tbl_base = (caddr_t)buf;
	
	AHCI_DBG("Cmd slot: 0x%x\n", pp->cmd_list_base);
	AHCI_DBG("Cmd tbl: 0x%x\n", pp->cmd_tbl);
	AHCI_DBG("PRD tbl: 0x%x\n", pp->prd_tbl_base);

	/* DMA map ??? */
#if 0
	pp->cmd_list_dma = dma_map_single(ap->port_attachdev, pp->cmd_list_base, AHCI_CMD_SLOT_SZ, DMA_TO_DEVICE);
	pp->rx_fis_dma = dma_map_single(ap->port_attachdev, pp->rx_fis_base, AHCI_RX_FIS_SZ, DMA_FROM_DEVICE);
#else
	pp->cmd_list_dma = VA_TO_PHY(pp->cmd_list_base);
	pp->rx_fis_dma = VA_TO_PHY(pp->rx_fis_base);
#endif
	
	writel_with_flush((uint32_t)pp->cmd_list_dma, ap->port_mmio + AHCI_PxCLB);
	if(host->cap & AHCI_CAP_S64A)
		writel_with_flush((uint32_t)((pp->cmd_list_dma >> 16) >> 16), ap->port_mmio + AHCI_PxCLBU);
		
	writel_with_flush((uint32_t)pp->rx_fis_dma, ap->port_mmio + AHCI_PxFB);
	if(host->cap & AHCI_CAP_S64A)
		writel_with_flush((uint32_t)((pp->rx_fis_dma >> 16) >> 16), ap->port_mmio + AHCI_PxFBU);
		
	ahci_port_poweron(ap);

	ahci_start_port(ap);

	return 0;
}

/**
 * 描述：当使能冷设备探测时，该函数将给设备供电
 */
void ahci_port_poweron(struct ata_port *ap)
{
	uint32_t val;

	val = readl(ap->port_mmio + AHCI_PxCMD);
	if(val & AHCI_PxCMD_CPD) {
		val |= AHCI_PxCMD_POD;
		writel_with_flush(val, ap->port_mmio + AHCI_PxCMD);
	}
}

/**
 * 描述：开启cmd list以及recv FIS引擎
 */
int ahci_start_port(struct ata_port *ap)
{
	/* 注意：RX FIS必须比CMD LIST先进入运行状态 */
	ahci_start_recv_fix(ap);	
	ahci_start_cmd_engine(ap);	
}

int ahci_stop_port(struct ata_port *ap)
{
	int err = 0;
	
	if((err = ahci_stop_cmd_engine(ap)) || (err = ahci_stop_recv_fix(ap)))
		return err;

	return 0;
}

int ahci_start_cmd_engine(struct ata_port *ap)
{
	uint32_t val;

	val = readl(ap->port_mmio + AHCI_PxCMD);
	val |= AHCI_PxCMD_ST;
	writel_with_flush(val, ap->port_mmio + AHCI_PxCMD);

	return 0;
}

int ahci_stop_cmd_engine(struct ata_port *ap)
{
	uint32_t val;

	val = readl(ap->port_mmio + AHCI_PxCMD);
	if(!(val & (AHCI_PxCMD_ST | AHCI_PxCMD_CR)))
		return 0;

	val &= ~AHCI_PxCMD_ST;
	writel_with_flush(val, ap->port_mmio + AHCI_PxCMD);
	
	val = ata_wait_register((void *)(ap->port_mmio + AHCI_PxCMD), 
							AHCI_PxCMD_CR | AHCI_PxCMD_ST, 0, 10, 1000);
	/* 等待超时 */
	if(val & (AHCI_PxCMD_ST | AHCI_PxCMD_CR))
		return EBUSY;

	return 0;
}

int ahci_start_recv_fix(struct ata_port *ap)
{
	uint32_t val;

	val = readl(ap->port_mmio + AHCI_PxCMD);
	val |= AHCI_PxCMD_FRE;
	writel_with_flush(val, ap->port_mmio + AHCI_PxCMD);

	return 0;
}

int ahci_stop_recv_fix(struct ata_port *ap)
{
	uint32_t val;

	val = readl(ap->port_mmio + AHCI_PxCMD);
	if(!(val & (AHCI_PxCMD_FRE | AHCI_PxCMD_FR)))
		return 0;

	val &= ~AHCI_PxCMD_FRE;
	writel_with_flush(val, ap->port_mmio + AHCI_PxCMD);
	
	val = ata_wait_register((void *)(ap->port_mmio + AHCI_PxCMD), 
							AHCI_PxCMD_FR | AHCI_PxCMD_FRE, 0, 10, 1000);
	/* 等待超时 */
	if(val & (AHCI_PxCMD_FRE | AHCI_PxCMD_FR))
		return EBUSY;

	return 0;
}

void ahci_save_initial_config(struct ahci_host *host)
{
	uint32_t val;

	ahci_enable_ahci((void *)host->mmio_base);
	
	host->cap = readl(host->mmio_base + AHCI_CAP);
	val = readl(host->mmio_base + AHCI_VS);
	if((val >> 16) > 1 || (1 == (val >> 16) && (val & 0xffff) >= 0x200))
		host->cap2 = readl(host->mmio_base + AHCI_CAP2);
	else
		host->cap2 = 0;
	host->port_map = readl(host->mmio_base + AHCI_PI);
}

void ahci_restore_initial_config(struct ahci_host *host)
{
	writel(host->cap, host->mmio_base + AHCI_CAP);
	if(host->cap)
		writel(host->cap2, host->mmio_base + AHCI_CAP2);
	writel(host->port_map, host->mmio_base + AHCI_PI);
}

/**
 * 描述：使能HBA的AHCI模式，在进行AHCI操作时，必须先使能AHCI模式
 */ 
void ahci_enable_ahci(void *mmio)
{	
	int i;
	uint32_t val;
	ulong __mmio = (ulong)mmio;

	val = readl(__mmio + AHCI_GHC);
	if(val & AHCI_GHC_AE)
		return;
	
	val |= AHCI_GHC_AE;
	/* 某些设备要写多次 */
	for(i = 0; i < 5; ++i) {
		writel(val, __mmio + AHCI_GHC);
		if(readl(__mmio + AHCI_GHC) & AHCI_GHC_AE)
			return;
		mdelay(10);
	}
}

/**
 * 描述：复位HBA
 */
int ahci_reset_controller(struct ahci_host *host)
{
	uint32_t val;

	/* 在复位之前必须使能AHCI模式 */
	ahci_enable_ahci((void *)host->mmio_base);

	val = readl(host->mmio_base + AHCI_GHC);
	if(!(val & AHCI_GHC_HR)) {
		val |= AHCI_GHC_HR;	
		writel_with_flush(val, host->mmio_base + AHCI_GHC);
	}

	/* 复位使能后，必须等待HBA将该位清0，该等待时间不会超过1s */
	val = ata_wait_register((void *)(host->mmio_base + AHCI_GHC), 
							AHCI_GHC_HR, 0, 10, 1000);
	/* 等待超时 */
	if(val & AHCI_GHC_HR) 
		return EIO;

	/* 对于支持IDE的HBA，复位后AE默认为0 */
	ahci_enable_ahci((void *)host->mmio_base);

	ahci_restore_initial_config(host);

	return 0;
}

/**
 * 描述：初始化某个port
 * 
 */
int ahci_init_port(struct ahci_host *host, int i)
{
	uint32_t val;
	struct ahci_port_info *pi;

	if(i < 0 || i >= AHCI_MAX_PORTS)
		return EINVAL;		

	pi = &host->ports[i];
	pi->mmio = ahci_port_base(host, i);
	pi->host = host;
	pi->id = i;
	
	val = readl(pi->mmio + AHCI_PxCMD);
	if(val & AHCI_PORT_RUNNING) {
		writel_with_flush(val & ~AHCI_PORT_RUNNING, pi->mmio + AHCI_PxCMD);
		val = ata_wait_register((void *)(pi->mmio + AHCI_PxCMD), 
								AHCI_PORT_RUNNING, 0, 10, 500);
		/* 等待超时 */
		if(val & AHCI_PORT_RUNNING) 
			return EIO;
	}

	/* flush AHCI_PxSSTS register */
	val = readl(pi->mmio + AHCI_PxSSTS);
	if(val & 0x3) {
		val &= ~0x3;
		writel_with_flush(val, pi->mmio + AHCI_PxSSTS);
		mdelay(500);
	}

	/* stagged spin-up port */
	if(host->cap & AHCI_CAP_SSS) {
		/* 当HBA支持stagger spin-up特性时，软件必须设置PxCMD.SUD，使设备建立通信 */
		val = readl(pi->mmio + AHCI_PxSCTL);
		val &= ~(AHCI_PxSCTL_DET_MASK << AHCI_PxSCTL_DET_OFFS);	//DET set to 0
		writel(val, pi->mmio + AHCI_PxSCTL);
		val = readl(pi->mmio + AHCI_PxCMD);
		writel(val | AHCI_PxCMD_SUD, pi->mmio + AHCI_PxCMD);	//SUD set to 1
	}

	/* wait for communication established */
	val = ata_wait_register((void *)(pi->mmio + AHCI_PxSSTS), 
							AHCI_PxSSTS_DET_MASK << AHCI_PxSSTS_DET_OFFS, 0x3, 10, 1000);

	if(((val >> AHCI_PxSSTS_DET_OFFS) & AHCI_PxSSTS_DET_MASK) == 0x3) 
		host->link_map |= 1u << i;
	
	/* clear error status */
	val = readl(pi->mmio + AHCI_PxSERR);
	writel(val, pi->mmio + AHCI_PxSERR);	
	
	/* clear IRQ status */
	val = readl(pi->mmio + AHCI_PxIS);
	if(val)
		writel(val, pi->mmio + AHCI_PxIS);	

	/* enable IRQ */
	writel(PORT_IRQ_DEFAULT, pi->mmio + AHCI_PxIE);	
	
	return 0;
}

void ahci_enable_global_irq(struct ahci_host *host)
{
	uint32_t val;

	val = readl(host->mmio_base + AHCI_GHC);
	writel(val | AHCI_GHC_IE, host->mmio_base + AHCI_GHC);	

}

void ahci_disable_global_irq(struct ahci_host *host)
{
	uint32_t val;

	val = readl(host->mmio_base + AHCI_GHC);
	writel(val & ~AHCI_GHC_IE, host->mmio_base + AHCI_GHC);	
}

/**
 * 描述：打印当前AHCI控制器支持的属性
 */
void ahci_print_info(struct ahci_host *host, const char *mode)
{
	uint32_t ver, cap, cap2;
	const char *speed;

	ver = readl(host->mmio_base + AHCI_VS);
	cap = host->cap;
	cap2 = host->cap2;

	switch((cap >> AHCI_CAP_ISS_OFFS ) & AHCI_CAP_ISS_MASK) {
		case 1:
			speed = "1.5";
			break;
		case 2:
			speed = "3";
			break;
		case 3:
			speed = "6";
			break;
		default:
			speed = "?";
	}

	printf("AHCI %02x%02x.%02x%02x "
			"%u slots %u ports %s Gbps 0x%x impl %s mode\n", 
			(ver >> 24) & 0xff,
			(ver >> 16) & 0xff,
			(ver >> 8) & 0xff,
			(ver) & 0xff,
			((cap >> AHCI_CAP_NCS_OFFS) & AHCI_CAP_NCS_MASK) + 1,
			((cap >> AHCI_CAP_NP_OFFS) & AHCI_CAP_NP_MASK) + 1,
			speed, host->port_map, mode);	
	
	printf("flags:"
			"%s%s%s%s%s%s%s%s"
			"%s%s%s%s%s%s%s%s"
			"%s%s%s%s\n",
			cap & AHCI_CAP_S64A ? " 64bit" : "",
			cap & AHCI_CAP_SNCQ ? " ncq" : "",
			cap & AHCI_CAP_SSNTF ? " sntf" : "",
			cap & AHCI_CAP_SMPS ? " ilck" : "",
			cap & AHCI_CAP_SSS ? " stag" : "",
			cap & AHCI_CAP_SALP ? " pm" : "",
			cap & AHCI_CAP_SAL ? " led" : "",
			cap & AHCI_CAP_SCLO ? " clo" : "",
			cap & AHCI_CAP_SAM ? " only" : "",
			cap & AHCI_CAP_SPM ? " pmp" : "",
			cap & AHCI_CAP_FBSS ? " fbs" : "",
			cap & AHCI_CAP_PMD ? " pio" : "",
			cap & AHCI_CAP_SSC ? " slum" : "",
			cap & AHCI_CAP_PSC ? " part" : "",
			cap & AHCI_CAP_CCCS ? " ccc" : "",
			cap & AHCI_CAP_EMS ? " ems" : "",
			cap & AHCI_CAP_SXS ? " sxs" : "",
			cap2 & AHCI_CAP2_APST ? " apst" : "",
			cap2 & AHCI_CAP2_NVMP ? " nvmp" : "",
			cap2 & AHCI_CAP2_BOH ? " boh" : "");
}

/**
 * 描述：填充指定端口的PRD信息
 */
ulong ahci_fill_prd(struct ata_port *ap, caddr_t buf, ulong len, int irq)
{
	ulong prd_cnt = 0, i;
	struct ahci_host *host = (struct ahci_host *)ap->port_host;
	struct ahci_port_priv *pp = (struct ahci_port_priv *)ap->port_priv;
	struct ahci_prd *prd = (struct ahci_prd *)pp->prd_tbl_base;

	prd_cnt = (len - 1) / AHCI_MAX_PRD_DATA_SZ + 1;
	if(prd_cnt > AHCI_MAX_PRD_COUNT) {
		log(LOG_ERR, "PRD cnt too much\n");	
		return 0;
	}
	
	for(i = 0; i < prd_cnt; ++i) {
		prd->dba = cpu_to_le32(VA_TO_PHY(buf));
		if(host->cap & AHCI_CAP_S64A)	
			prd->dba_hi = cpu_to_le32((VA_TO_PHY(buf) >> 16) >> 16);
		else
			prd->dba_hi = 0;
		prd->dbc = len < AHCI_MAX_PRD_DATA_SZ ? (len - 1) : (AHCI_MAX_PRD_DATA_SZ - 1);
		if(irq)
			prd->flags_i = 1;
		else
			prd->flags_i = 0;
		len -= AHCI_MAX_PRD_DATA_SZ;
		buf += AHCI_MAX_PRD_DATA_SZ;
		++prd;
	}

	return prd_cnt;
}

/**
 * 描述：填充指定端口的CMD头信息
 */
void ahci_fill_cmd_hdr(struct ata_port *ap, uint32_t opts)
{
	struct ahci_host *host = (struct ahci_host *)ap->port_host;
	struct ahci_port_priv *pp = (struct ahci_port_priv *)ap->port_priv;
	struct ahci_cmd_hdr *ch = (struct ahci_cmd_hdr *)pp->cmd_list_base;
	
	ch->opts_field = cpu_to_le32(opts);
	ch->prdbc = 0;
	ch->ctba = cpu_to_le32(VA_TO_PHY(pp->cmd_tbl));
	if(host->cap & AHCI_CAP_S64A)	
		ch->ctba_hi = cpu_to_le32((VA_TO_PHY(pp->cmd_tbl) >> 16) >> 16);
	else
		ch->ctba_hi = 0;
}

/**
 * 描述：HBA发送读写命令FIS到设备
 */
size_t ahci_device_data_xfer(struct ata_device *adev, caddr_t buf, 
						ulong len, int is_acmd, int dir)
{
	int err;
	ulong prd_cnt;
	uint32_t opts;
	struct ata_port *ap = adev->port;
	struct ahci_port_priv *pp = (struct ahci_port_priv *)ap->port_priv;
	struct ahci_cmd_hdr *ch = (struct ahci_cmd_hdr *)pp->cmd_list_base;
	
	if(!(prd_cnt = ahci_fill_prd(ap, buf, len, 0))) {
		errno = EINVAL;
		return 0;
	}

	opts = (pp->cmd_fis_len & 0x1f) >> 2 | (prd_cnt & 0xffff) << 16 | (adev->pmp & 0xf) << 12;
	if(is_acmd) 
		opts |= AHCI_CMD_FLAGS_ATAPI | AHCI_CMD_FLAGS_PREF;
	if(ATA_WRITE == dir)
		opts |= AHCI_CMD_FLAGS_WRITE;

	ahci_fill_cmd_hdr(ap, opts);

#if 0
	cpu_flush_cache_io(pp->cmd_list_base, AHCI_CMD_SLOT_SZ, SYNC_W);
	cpu_flush_cache_io(pp->cmd_tbl, AHCI_CMD_TLB_HDR_SZ, SYNC_W);
	cpu_flush_cache_io(pp->prd_tbl_base, AHCI_PRD_TLB_ITEM_SZ * prd_cnt, SYNC_W);
	cpu_flush_cache_io(pp->rx_fis_base, AHCI_RX_FIS_SZ, SYNC_R);
	cpu_flush_cache_io(buf, len, dir ? SYNC_W : SYNC_R);
#endif

	writel_with_flush(0x01, ap->port_mmio + AHCI_PxCI);//issue cmd slot0
	/* 注意：当传输数据量特别大时，下面的超时时间需要作相应调整 */
	if(err = ata_wait_cmd_completed((void *)(ap->port_mmio + AHCI_PxCI), 0x01, 2000)) {
		errno = ETIMEDOUT;
		return 0;
	}

	return ch->prdbc;
}

/**
 * 描述：获取设备ID信息
 */
static int ahci_read_dev_id(struct ata_device *adev, uint16_t *devid)
{
	int err;
	struct ata_port *ap = adev->port;
	struct ahci_port_priv *pp = (struct ahci_port_priv *)ap->port_priv;
	struct h2d_fis *fis = (struct h2d_fis *)pp->cmd_tbl;
	
	bzero(fis, sizeof(struct h2d_fis));
	fis->fis_type = FIS_TYPE_H2D_REG;
	fis->flags_c = 1;		/* command fis */
	fis->pmp = adev->pmp;
	if(!adev->is_atapi)
		fis->command = ATA_CMD_ID_ATA;
	else
		fis->command = ATA_CMD_ID_ATAPI;
	pp->cmd_fis_len = sizeof(struct h2d_fis);
	
	if(!ahci_device_data_xfer(adev, (caddr_t)devid, 
			ATA_ID_WORDS * sizeof(uint16_t), 0, ATA_READ)) {
		err = errno;
		return err;
	}

	return 0;
}

/**
 * 描述：设置设备传输模式
 */
static int ahci_set_device_xfermode(struct ata_device *adev, uint8_t mode)
{
	uint32_t opts;
	struct ata_port *ap = adev->port;
	struct ahci_port_priv *pp = (struct ahci_port_priv *)ap->port_priv;
	struct h2d_fis *fis = (struct h2d_fis *)pp->cmd_tbl;
	
	bzero(fis, sizeof(struct h2d_fis));
	fis->fis_type = FIS_TYPE_H2D_REG;
	fis->flags_c = 1;		/* command fis */
	fis->pmp = adev->pmp;
	fis->command = ATA_CMD_SET_FEATURES;	/* ata-acs4 defined */
	fis->features_0_7 = SETFEATURES_XFER;
	fis->count_0_7 = mode;
	pp->cmd_fis_len = sizeof(struct h2d_fis);
	
	opts = (pp->cmd_fis_len & 0x1f) >> 2 | (adev->pmp & 0xf) << 12;
	ahci_fill_cmd_hdr(ap, opts);

#if 0
	cpu_flush_cache_io(pp->cmd_list_base, AHCI_CMD_SLOT_SZ, SYNC_W);
	cpu_flush_cache_io(pp->cmd_tbl, AHCI_CMD_TLB_HDR_SZ, SYNC_W);
	cpu_flush_cache_io(pp->rx_fis_base, AHCI_RX_FIS_SZ, SYNC_R);
#endif

	writel_with_flush(0x01, ap->port_mmio + AHCI_PxCI);//issue cmd slot0
	if(ata_wait_cmd_completed((void *)(ap->port_mmio + AHCI_PxCI), 0x01, 100)) 
		return ETIMEDOUT;
	
	adev->xfermode = mode;

	return 0;
}

/**
 * 描述：软复位ATA设备
 */
static int ahci_dev_softreset(struct ata_device *adev)
{
	int err;
	uint32_t opts;
	struct ata_port *ap = adev->port;
	struct ahci_port_priv *pp = (struct ahci_port_priv *)ap->port_priv;
	struct h2d_fis *fis = (struct h2d_fis *)pp->cmd_tbl;

	if(err = ahci_kick_engine(ap))
		return err;

	bzero(fis, sizeof(struct h2d_fis));
	fis->fis_type = FIS_TYPE_H2D_REG;
	fis->pmp = adev->pmp;
	fis->flags_c = 0;		/* block register fis */
	fis->control = ATA_SRST;
	pp->cmd_fis_len = sizeof(struct h2d_fis);
	opts = (pp->cmd_fis_len & 0x1f) >> 2 | (adev->pmp & 0xf) << 12;
	opts |= AHCI_CMD_FLAGS_CLR | AHCI_CMD_FLAGS_RESET;
	ahci_fill_cmd_hdr(ap, opts);

#if 0
	cpu_flush_cache_io(pp->cmd_list_base, AHCI_CMD_SLOT_SZ, SYNC_W);
	cpu_flush_cache_io(pp->cmd_tbl, AHCI_CMD_TLB_HDR_SZ, SYNC_W);
	cpu_flush_cache_io(pp->rx_fis_base, AHCI_RX_FIS_SZ, SYNC_R);
#endif

	writel_with_flush(0x01, ap->port_mmio + AHCI_PxCI);//issue cmd slot0
	if(ata_wait_cmd_completed((void *)(ap->port_mmio + AHCI_PxCI), 0x01, 100)) {
		ahci_kick_engine(ap);
		return ETIMEDOUT;
	}
		
	mdelay(1);
	
	fis->control &= ~ATA_SRST;
	opts &= ~(AHCI_CMD_FLAGS_CLR | AHCI_CMD_FLAGS_RESET);
	ahci_fill_cmd_hdr(ap, opts);
	
#if 0
	cpu_flush_cache_io(pp->cmd_list_base, AHCI_CMD_SLOT_SZ, SYNC_W);
	cpu_flush_cache_io(pp->cmd_tbl, AHCI_CMD_TLB_HDR_SZ, SYNC_W);
	cpu_flush_cache_io(pp->rx_fis_base, AHCI_RX_FIS_SZ, SYNC_R);
#endif

	writel_with_flush(0x01, ap->port_mmio + AHCI_PxCI);//issue cmd slot0
	if(ata_wait_cmd_completed((void *)(ap->port_mmio + AHCI_PxCI), 0x01, 100)) {
		ahci_kick_engine(ap);
		return ETIMEDOUT;
	}

	if(err = ahci_check_ready(ap, 1000))
		return err;

	return 0;
}

/**
 * 描述：组装ATAPI命令FIS
 */
static void make_data_atapi_command(uint8_t *cmd, ulong blkno, ulong blkcnt)
{
	bzero(cmd, ATAPI_CMD_LEN12);
	cmd[0] = GPCMD_READ_10;				//bad code???
	cmd[2] = (blkno >> 24) & 0xff;
	cmd[3] = (blkno >> 16) & 0xff;
	cmd[4] = (blkno >> 8) & 0xff;
	cmd[5] = blkno & 0xff;
	cmd[7] = (blkcnt >> 8) & 0xff;
	cmd[8] = blkcnt & 0xff;
}

static int ahci_blk_data_rw(struct ata_device *adev, uint64_t blkno, 
				ulong blkcnt, void *buf, int op)
{
	int err;
	struct ata_port *ap = adev->port;
	struct ahci_port_priv *pp = (struct ahci_port_priv *)ap->port_priv;
	struct h2d_fis *fis = (struct h2d_fis *)pp->cmd_tbl;
	struct block_device *blkdev = adev->blkdev;
	uint8_t *acmd = pp->cmd_tbl + AHCI_CMD_TLB_CFIS_SZ;
	
	bzero(fis, sizeof(struct h2d_fis));
	fis->fis_type = FIS_TYPE_H2D_REG;
	fis->pmp = adev->pmp;
	fis->flags_c = 1;		/* command fis */

	if(blkdev->bd_has_lba48) {
		if(adev->is_atapi) {
			fis->command = ATA_CMD_PACKET;
			fis->features_0_7 = (op == ATA_WRITE) ? \
					ATAPI_PKT_DMA : (ATAPI_DMA_READ | ATAPI_PKT_DMA); 
			make_data_atapi_command(acmd, blkno, blkcnt);
		} else 
			fis->command = (op == ATA_WRITE) ? ATA_CMD_WRITE_EXT : ATA_CMD_READ_EXT;
		fis->lba_40_47 = (blkno >> 40) & 0xff;
		fis->lba_32_39 = (blkno >> 32) & 0xff;
		fis->lba_24_31 = (blkno >> 24) & 0xff;
		fis->count_8_15 = (blkcnt >> 8) & 0xff;
	} else {	/* lba 28 bits */
		if(adev->is_atapi) {
			fis->command = ATA_CMD_PACKET;
			fis->features_0_7 = (op == ATA_WRITE) ? \
					ATAPI_PKT_DMA : (ATAPI_DMA_READ | ATAPI_PKT_DMA);
			make_data_atapi_command(acmd, blkno, blkcnt);
		} else 
			fis->command = (op == ATA_WRITE) ? ATA_CMD_WRITE : ATA_CMD_READ;
		fis->device = (blkno >> 24) & 0x0f;
	}
	fis->lba_16_23 = (blkno >> 16) & 0xff;
	fis->lba_8_15 = (blkno >> 8) & 0xff;
	fis->lba_0_7 = blkno & 0xff;
	fis->device |= 0xe0;
	fis->count_0_7 = blkcnt & 0xff;
	pp->cmd_fis_len = sizeof(struct h2d_fis);

	AHCI_DBG("%s: ", (ATA_WRITE == op) ? "ata_write" : "ata_read");
	AHCI_DBG("blkno = 0x%llx, blkcnt = %lu, blksz = %lu\n", 
				blkno, blkcnt, blkdev->bd_blk_sz);
	if(!ahci_device_data_xfer(adev, buf, 
			blkdev->bd_blk_sz * blkcnt, adev->is_atapi, op)) {
		err = errno;
		return err;
	}

	return 0;
}

struct ata_port_operations ahci_ops = {
	.read_id = ahci_read_dev_id,
	.set_xfermode = ahci_set_device_xfermode,
	.softreset = ahci_dev_softreset,
	.blk_data_rw = ahci_blk_data_rw,
};
