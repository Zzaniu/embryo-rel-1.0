
/**
 * Copyright(c) 2018-9-6 Shangwen Wu	
 *
 * mdio_read/write命令实现
 * 
 */
#include <common.h>
#include <stdio.h>
#include <string.h>
#include <cvt.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <command.h>
#include <fs/termio.h>

#define READ_ALL		0xff
#define MAX_PHY_ADDR	0x1f
#define MAX_REG_ADDR	0x1f

extern int mdio_write(const char *name, uint8_t phyaddr, uint8_t reg, uint16_t val); //defined in mdio.c
extern int mdio_read(const char *name, uint8_t phyaddr, uint8_t reg, uint16_t *val); //defined in mdio.c

static int cmd_mdio_read(int argc, const char *argv[])
{
	unsigned long phyaddr, reg;
	uint16_t val;
	uint8_t phy_start, phy_end, reg_start, reg_end, __reg_start;

	if(argc <= 2)
		phyaddr = READ_ALL;
	else if(atob(&phyaddr, argv[2], 16) || (phyaddr > MAX_PHY_ADDR && phyaddr != READ_ALL)) {
		fprintf(stderr, "phyaddr %s invaild\n", argv[2]);
		return -1;
	}

	if(argc <= 3)
		reg = READ_ALL;
	else if(atob(&reg, argv[3], 16) || (reg > MAX_REG_ADDR && reg != READ_ALL)) {
		fprintf(stderr, "regaddr %s invaild\n", argv[3]);
		return -1;
	}

	if(READ_ALL == phyaddr) {
		phy_start = 0;
		phy_end = MAX_PHY_ADDR;
	} else 
		phy_start = phy_end = phyaddr;
	
	if(READ_ALL == reg) {
		reg_start = 0;
		reg_end = MAX_REG_ADDR;
	} else 
		reg_start = reg_end = reg;
	
	for(; phy_start <= phy_end; ++phy_start) {
		for(__reg_start = reg_start; __reg_start <= reg_end; ++__reg_start) {
			if(mdio_read(argv[1], phy_start, __reg_start, &val)) {
				perror("mdio_read");
				return -1;
			}
			printf("phyaddr 0x%02x, regaddr 0x%02x, val 0x%04x\n", phy_start, __reg_start, (uint32_t)val);
		}
	}

	return 0;
}

static int cmd_mdio_write(int argc, const char *argv[])
{
	unsigned long phyaddr, reg, val;
	
	if(atob(&phyaddr, argv[2], 16) || phyaddr > MAX_PHY_ADDR) {
		fprintf(stderr, "phyaddr %s invaild\n", argv[2]);
		return -1;
	}
	if(atob(&reg, argv[3], 16) || reg > MAX_REG_ADDR) {
		fprintf(stderr, "regaddr %s invaild\n", argv[3]);
		return -1;
	}
	if(atob(&val, argv[4], 16)) {
		fprintf(stderr, "regval %s invaild\n", argv[4]);
		return -1;
	}

	if(mdio_write(argv[1], (uint8_t)phyaddr, (uint8_t)reg, (uint16_t)val)) {
		perror("mdio_write");
		return -1;
	}

	return 0;
}

static struct optdesc mdio_read_opts[] = {
	{
		.name = "ifname", 
		.desc = "network interface name",
	},
	{
		.name = "phyaddr", 
		.desc = "PHY address (hexval)",
	},
	{
		.name = "regaddr", 
		.desc = "PHY's register address (hexval)",
	},
	{},
};

static struct optdesc mdio_write_opts[] = {
	{
		.name = "ifname", 
		.desc = "network interface name",
	},
	{
		.name = "phyaddr", 
		.desc = "PHY address (hexval)",
	},
	{
		.name = "regaddr", 
		.desc = "PHY's register address (hexval)",
	},
	{
		.name = "regval", 
		.desc = "PHY's register value (hexval)",
	},
	{},
};

/* phy访问命令实现 */
static struct cmd cmds[] = {
	{{"Network"}},			//表组名
	{
		{"mdio_read"},
		"read PHY register",
		"ifname [phyaddr] [regaddr]",
		mdio_read_opts, 
		cmd_mdio_read,
		2,
		4,
		0, 
	},
	{
		{"mdio_write"},
		"write PHY register",
		"ifname phyaddr regaddr regval",
		mdio_write_opts, 
		cmd_mdio_write,
		5,
		5,
		0, 
	},
	{},						//最后一个强制要求为空
};

/**
 * 描述：命令初始化函数
 */
static void __attribute__((constructor)) init_cmds(void)
{
	add_cmds(cmds, 0);
}
