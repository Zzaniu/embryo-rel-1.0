
include $(TOPDIR)/config.mk

SUBCOBJS	+= inet/libinet.o

SUBOBJS		:= $(addprefix $(CURDIR)/, $(SUBCOBJS))
OBJS		:= $(SUBOBJS) 
OBJS		+= if.o 
OBJS		+= if_loop.o 
OBJS		+= if_ether.o 
OBJS		+= radix.o 
OBJS		+= route.o 

LIB			:= $(CURDIR)/libnet.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(SUBOBJS):
	$(MAKE) -C $(dir $@)

.PHONY : clean
clean:
	rm -rf *.o
	(cd $(CURDIR)/inet; make clean)

