
include $(TOPDIR)/config.mk

COBJS	:= stmmac_main.o 
COBJS	+= stmmac_hw.o 
COBJS	+= stmmac_mdio.o 
COBJS	+= dwmac1000_mac.o 
COBJS	+= dwmac1000_dma.o 
COBJS	+= enh_desc.o 
COBJS	+= dwmac_lib.o 

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libstmmac.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

