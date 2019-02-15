
include $(TOPDIR)/config.mk

COBJS	:= ctors.o
COBJS	+= mipsdep.o
COBJS	+= dma.o
COBJS	+= clock.o

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/lib$(ARCH).o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

