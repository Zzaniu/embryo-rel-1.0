
include $(TOPDIR)/config.mk

COBJS	:= fsio.o
COBJS	+= ext2.o

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libfsio.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

