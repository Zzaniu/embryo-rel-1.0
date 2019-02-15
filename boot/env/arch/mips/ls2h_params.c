
/**
 * Copyright(c) 2019-1-28 @PMON Shangwen Wu	
 *
 * 2H平台相关的内核环境参数
 * 
 */

#include <common.h>
#include <sys/types.h>
#include "loongson_param.h"

static struct efi_memory_map_loongson g_memmap = {0}; 
static struct efi_cpuinfo_loongson g_cpuinfo = {0}; 
static struct system_loongson g_loongsonsys = {0}; 
static struct irq_source_routing_table g_irqinfo = {0};
static struct interface_info g_intfinfo = {0};
static struct board_devices g_boardinfo = {0};
static struct loongson_special_attribute g_specinfo = {0};

static struct efi_memory_map_loongson *init_memory_map(void)
{
	struct efi_memory_map_loongson *memmap = &g_memmap;
	
	memmap->nr_map = 7;
	memmap->mem_freq = 300000000;			//mem 300MHz
	
	memmap->map[0].node_id = 0;
	memmap->map[0].mem_type = 1;
	memmap->map[0].mem_start = 0x01000000;
	memmap->map[0].mem_size = 0xd0;
	
	memmap->map[1].node_id = 0;
	memmap->map[1].mem_type = 2;
	memmap->map[1].mem_start = 0x118000000ULL;
	memmap->map[1].mem_size = 0x680;
	
	memmap->map[2].node_id = 0;
	memmap->map[2].mem_type = 10;
	memmap->map[2].mem_start = 0x0fffe000UL;
	memmap->map[2].mem_size = 0x0;
	
	memmap->map[6].node_id = 0;
	memmap->map[6].mem_type = 11;
	memmap->map[6].mem_start = 0x110000000ULL;
	memmap->map[6].mem_size = 0x80;

	return memmap;
}

static struct efi_cpuinfo_loongson *init_cpu_info(void)
{
	struct efi_cpuinfo_loongson *cpuinfo = &g_cpuinfo;

	cpuinfo->processor_id = PRID_IMP_LOONGSON2H;
	cpuinfo->cputype = Loongson_3A;
	cpuinfo->cpu_clock_freq = 800000000;		//cpu 800MHz
	cpuinfo->total_node = 1;
	cpuinfo->nr_cpus = 1;
	cpuinfo->cpu_startup_core_id = 0;

	return cpuinfo;
}

static struct system_loongson *init_loongson_system(void)
{
	struct system_loongson *loongsonsys = &g_loongsonsys;

	loongsonsys->ccnuma_smp = 0;
	loongsonsys->sing_double_channel = 1;

	return loongsonsys;
}

static struct irq_source_routing_table *init_irq_source(void)
{
	struct irq_source_routing_table *irqinfo = &g_irqinfo;

	irqinfo->PIC_type = HT;
	irqinfo->ht_int_bit = 1 << 24;
	irqinfo->ht_enable = 0x0000d17b;
	irqinfo->node_id = 0x0;
	irqinfo->pci_mem_start_addr = 0x40000000UL;
	irqinfo->pci_mem_end_addr = 0x7fffffffUL;

	return irqinfo;
}

static struct interface_info *init_interface_info(void)
{
	struct interface_info *intfinfo = &g_intfinfo;

	intfinfo->vers = 0x0001;
	intfinfo->size = 0;
	intfinfo->flag = 0x1;	
	strcpy(intfinfo->description, "Loogson-Embryo-1.0");

	return intfinfo;
}

static struct board_devices *init_board_device_info(void)
{
	struct board_devices *boardinfo = &g_boardinfo;

	strcpy(boardinfo->name, "Loonson-2H-SOC-demo");
	boardinfo->num_resources = 10;

	return boardinfo;
}

static struct loongson_special_attribute *init_special_info(void)
{
	struct loongson_special_attribute *specinfo = &g_specinfo;

	strcpy(specinfo->special_name, "2019-02-14");	
	
	specinfo->resource[0].flags = 0;
	specinfo->resource[0].start = 0;
	specinfo->resource[0].end = 0;
	strcpy(specinfo->resource[0].name, "SPMODULE");

	return specinfo;
}

static void init_loongson_params(struct loongson_params *lp)
{
	lp->memory_offset = (u64)(long)init_memory_map() - (u64)(long)lp;
	lp->cpu_offset = (u64)(long)init_cpu_info() - (u64)(long)lp;
	lp->system_offset = (u64)(long)init_loongson_system() - (u64)(long)lp;
	lp->irq_offset = (u64)(long)init_irq_source() - (u64)(long)lp;
	lp->interface_offset = (u64)(long)init_interface_info() - (u64)(long)lp;
	lp->special_offset = (u64)(long)init_special_info() - (u64)(long)lp;
	lp->boarddev_table_offset = (u64)(long)init_board_device_info() - (u64)(long)lp;
}

static void init_smbios_tables(struct smbios_tables *smbios)
{
	init_loongson_params(&smbios->lp);
}

static void init_efi(struct efi *efi)
{
	init_smbios_tables(&efi->smbios);
}

static void init_boot_params(struct boot_params *params)
{
	init_efi(&params->efi);
}

/**
 * 描述：初始化2H相关的内核环境参数
 * 参数：envbuf, 当该参数为NULL时，可仅用来获取平台相关参数的长度
 * 返回：函数执行结果，返回环境参数长度
 */
unsigned long init_machine_kernel_env(uint8_t *envbuf)
{
	struct boot_params *params;

	if(envbuf) {
		params = (struct boot_params *)envbuf;
		init_boot_params(params);
	}

	return sizeof(struct boot_params);
}
