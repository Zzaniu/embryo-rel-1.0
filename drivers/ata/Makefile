
include $(TOPDIR)/config.mk

COBJS	:= ahci_platform.o
COBJS	+= ahci_disk.o
COBJS	+= libahci.o
COBJS	+= libata_core.o

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libata.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

