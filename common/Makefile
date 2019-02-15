
include $(TOPDIR)/config.mk

COBJS	:= devinit.o 
COBJS	+= env.o 
COBJS	+= more.o 
COBJS	+= rsa.o 

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libcommon.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

