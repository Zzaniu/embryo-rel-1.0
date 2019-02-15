
include $(TOPDIR)/config.mk

COBJS	:= env/arch/$(ARCH)/$(MACH)_params.o 

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libboot.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

