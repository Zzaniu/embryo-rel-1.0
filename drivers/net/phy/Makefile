
include $(TOPDIR)/config.mk

COBJS	:= mdio.o 
COBJS	+= phy_device.o 
COBJS	+= marvell.o 

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libphy.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

