
/**
 * Copyright(c) 2018-10-22 Shangwen Wu
 * 基于平台而非PCI的AHCI host（HBA）控制器驱动实现
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/syslog.h>
#include <sys/device.h>
#include <sys/block.h>
#include <asm/io.h>

#include <ata/ata.h>
#include <ata/ahci.h>

static int ahci_plt_match(struct device *parent, void *match, void *aux)
{
	return 1;
}

static int ahci_plt_attach(struct device *parent, struct device *self, void *aux)
{
	int i, err;
	ulong iobase;
	struct confargs *ca = (struct confargs *)aux;
	struct ahci_host *plt_host = (struct ahci_host *)self;

	log(LOG_DEBUG, "in %s attach function\n", self->dv_name);
	iobase = (ulong)ca->ca_iobase;

	plt_host->mmio_base = iobase;
	plt_host->flags = ATA_FLAG_COMM;

	ahci_save_initial_config(plt_host);

	/**
     * 注意：CAP以及PI寄存器的某些bit位并不是只读（这一点与规范描述不同）的，而是
     * 		 作为某些特性的隐藏开关，这些Bit位上电后只允许写一次（哪怕复位HBA），
     *		 如果不对这些bit为进行写1使能，那么这些bit对应的特性默认是不会被打开
     *		 的。比如PI寄存器，对其进行写1时，如果对应的端口被芯片实现，那么该端
     *		 口将被打开，并且对应bit置1.如果对未实现的端口写1操作将被忽略
     */
	plt_host->cap &= ~(AHCI_CAP_SPM | AHCI_CAP_SMPS);	//disable PMP 
	plt_host->cap |= AHCI_CAP_SSS;	//enable Stagged spin-up
	if(!plt_host->port_map)
		plt_host->port_map = 0xffffffff;//open all impl'ports 

	if(err = ahci_reset_controller(plt_host)) {
		log(LOG_ERR, "ahci_plt: ahci reset failed\n");
		return err;	
	}

	plt_host->nports = 0;
	/* flush registers */
	plt_host->cap = readl(plt_host->mmio_base + AHCI_CAP);
	if(plt_host->cap2)
		plt_host->cap2 = readl(plt_host->mmio_base + AHCI_CAP2);
	plt_host->port_map = readl(plt_host->mmio_base + AHCI_PI);
		
	for(i = 0; i < AHCI_MAX_PORTS; ++i) {
		if(plt_host->port_map & (1u << i)) {
			if(ahci_init_port(plt_host, i))
				log(LOG_ERR, "ahci_plt: port %d init failed\n", i);
			/* 注意：这里不直接使用CAP.NP，因为某些坑爹实现NP可能大于实际实现的端口个数 */
			++plt_host->nports;
		}
	}

	/* Enable Global IRQ */
	ahci_enable_global_irq(plt_host);

	/* Show HBA's capabilities */
	ahci_print_info(plt_host, "plat");

	for(i = 0; i < AHCI_MAX_PORTS; ++i) {
		if(plt_host->link_map & (1u << i))
			config_found(self, &plt_host->ports[i], NULL);
	}

	return 0;
}

static void ahci_plt_detach(struct device *parent, struct device *self, void *aux)
{
	log(LOG_DEBUG, "in %s deattach function\n", self->dv_name);
}

struct cfattach ahci_plt_ca = {
	sizeof(struct ahci_host),
	ahci_plt_match,
	ahci_plt_attach,
	ahci_plt_detach,
};

struct cfdriver ahci_plt_cd = {
	LIST_HEAD_INIT(ahci_plt_cd.cd_devlist),
	"ahci_plt",
	DEV_DULL,
	0,
};

