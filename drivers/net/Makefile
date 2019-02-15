
include $(TOPDIR)/config.mk

SUBCOBJS	+= stmmac/libstmmac.o
SUBCOBJS	+= phy/libphy.o

SUBOBJS		:= $(addprefix $(CURDIR)/, $(SUBCOBJS))
OBJS		:= $(SUBOBJS) 

LIB			:= $(CURDIR)/libnet.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(SUBOBJS):
	$(MAKE) -C $(dir $@)

.PHONY : clean
clean:
	rm -rf *.o
	(cd $(CURDIR)/stmmac; make clean)
	(cd $(CURDIR)/phy; make clean)

