
include $(TOPDIR)/config.mk

SOBJS	:= mips.o
SOBJS	+= cache.o
SOBJS	+= longjmp.o

OBJS	:= $(addprefix $(CURDIR)/, $(SOBJS))

LIB		:= $(CURDIR)/libasm.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(SOBJS) *.o

