
include $(TOPDIR)/config.mk

COBJS	:= serial/libserial.o
COBJS	+= fb/libfb.o
COBJS	+= sys/libsys.o
COBJS	+= net/libnet.o
COBJS	+= ata/libata.o
COBJS	+= block/libblock.o

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libdrivers.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(OBJS):
	$(MAKE) -C $(dir $@)

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o
	(cd $(CURDIR)/serial; make clean)
	(cd $(CURDIR)/fb; make clean)
	(cd $(CURDIR)/sys; make clean)
	(cd $(CURDIR)/net; make clean)
	(cd $(CURDIR)/ata; make clean)
	(cd $(CURDIR)/block; make clean)

