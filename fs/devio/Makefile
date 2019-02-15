
include $(TOPDIR)/config.mk

COBJS	:= blockio.o

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libdevio.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

