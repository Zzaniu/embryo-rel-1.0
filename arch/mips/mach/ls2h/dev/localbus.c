
/**
 * Copyright(c) 2017-1-19 Shangwen Wu
 * localbus设备驱动，该总线根具体的CPU平台相关，因此其路径位于各自CPU的板级相关的目录下
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <string.h>
#include <sys/list.h>
#include <sys/syslog.h>
#include <sys/device.h>

/* localbus设备描述 */
struct localbus_device {
	struct device ld_dev;						//device结构一定要在最前面
	struct bushook ld_bus;						//localbus私有数据
};

static int localbus_match(struct device *parent, void *match, void *aux)
{
	struct confargs *ca = (struct confargs *)aux;
	struct cfdata *cf = (struct cfdata *)match;//mainbus's indirect = 0
	struct bushook *bus = ca->ca_bus;
	
	if(bus->bh_matchname(ca, cf->cf_driver->cd_name))
		return 1;
	else
		return 0;
}

static void localbus_scan(struct device *parent, void *match)
{
	int pri;
	struct localbus_device *localbusdev = (struct localbus_device *)parent;
	struct device *subdev = (struct device *)match;	//localbus传入的match一定是device结构，因为其indirect = 1
	struct cfdata *cf = subdev->dv_cfdata;
	struct confargs localbusarg;
	
	log(LOG_DEBUG, "probe for %s at 2H-SOC's localbus\n", cf->cf_driver->cd_name);
	
	if(CFSTAT_DYNAMIC == subdev->dv_cfdata->cf_stat) {					//2H的localbus不支持动态设备
		log(LOG_WARNING, "2H-SOC's localbus don't support dynamic device\n");
		destroy_device(subdev);	
	}

	if(!cf->cf_attach->ca_match) 
		panic("no match function for device search");	

	/* 如果ioconf设置了IO地址，则设置地址参数 */
	if(-1 == *cf->cf_iobase)
		localbusarg.ca_iobase = 0;
	else 
		localbusarg.ca_iobase = *cf->cf_iobase;

	localbusarg.ca_bus = &localbusdev->ld_bus;
	if((pri = (*cf->cf_attach->ca_match)(parent, match, &localbusarg)) > 0)
		config_attach(parent, match, &localbusarg);
	else
		destroy_device(subdev);
	
	log(LOG_DEBUG, "%s probe pri = %d\n", cf->cf_driver->cd_name, pri);
}

static int localbus_attach(struct device *parent, struct device *self, void *aux)
{
	struct localbus_device *localbusdev = (struct localbus_device *)self;
	struct bushook *bus = &localbusdev->ld_bus;

	bus->bh_dev = self;
	bus->bh_type = BUS_LOCAL;
	bus->bh_matchname = NULL;

	log(LOG_DEBUG, "in %s attach function\n", self->dv_name);
	
	config_scan(self, localbus_scan);
	
	return 0;
}


static void localbus_detach(struct device *parent, struct device *self, void *aux)
{
	log(LOG_DEBUG, "in %s deattach function\n", self->dv_name);
}

struct cfattach localbus_ca = {
	sizeof(struct localbus_device),
	localbus_match,
	localbus_attach,
	localbus_detach,
};

struct cfdriver localbus_cd = {
	LIST_HEAD_INIT(localbus_cd.cd_devlist),
	"localbus",
	DEV_DULL,
	1,
};

