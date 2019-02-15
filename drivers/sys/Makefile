
include $(TOPDIR)/config.mk

COBJS	:= mainbus.o 

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libsys.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

