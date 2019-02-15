
/**
 * Copyright(c) 2017-1-17 Shangwen Wu
 * mainbus设备驱动，该总线为虚拟的总线
 * 
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <string.h>
#include <sys/list.h>
#include <sys/syslog.h>
#include <sys/device.h>

/* mainbus设备描述 */
struct mainbus_device {
	struct device md_dev;						//device结构一定要在最前面
	struct bushook md_bus;						//mainbus私有数据
};

static int mainbus_match(struct device *parent, void *match, void *aux)
{
	return 1;
}

static int mainbus_matchname(struct confargs *args, const char *name)
{
	return !(strcmp(args->ca_name, name));
}

static int mainbus_attach(struct device *parent, struct device *self, void *aux)
{
	struct mainbus_device *maindev = (struct mainbus_device *)self;
	struct confargs mainca;

	log(LOG_DEBUG, "in %s attach function\n", self->dv_name);
	
	maindev->md_bus.bh_dev = self;	
	maindev->md_bus.bh_type = BUS_MAIN;
	maindev->md_bus.bh_matchname = mainbus_matchname;
	
	/* localbus */
	mainca.ca_name = "localbus";
	mainca.ca_bus = &maindev->md_bus;
	config_found(self, &mainca, NULL);
	
	return 0;
}


static void mainbus_detach(struct device *parent, struct device *self, void *aux)
{
	log(LOG_DEBUG, "in %s deattach function\n", self->dv_name);
}

struct cfattach mainbus_ca = {
	sizeof(struct mainbus_device),
	mainbus_match,
	mainbus_attach,
	mainbus_detach,
};

struct cfdriver mainbus_cd = {
	LIST_HEAD_INIT(mainbus_cd.cd_devlist),
	"mainbus",
	DEV_DULL,
	0,
};

