
include $(TOPDIR)/config.mk

COBJS	:= machdep.o 
COBJS	+= post.o
COBJS	+= dev/early_log.o
COBJS	+= dev/time.o
COBJS	+= dev/flash.o
COBJS	+= dev/dc.o
COBJS	+= dev/localbus.o 

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/lib$(MACH).o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o
	rm -rf dev/*.o
	
