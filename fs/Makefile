
include $(TOPDIR)/config.mk

COBJS		:= fscore.o
SUBCOBJS	+= termio/libtermio.o
SUBCOBJS	+= socket/libsocket.o
SUBCOBJS	+= netio/libnetio.o
SUBCOBJS	+= devio/libdevio.o
SUBCOBJS	+= fsio/libfsio.o

SUBOBJS		:= $(addprefix $(CURDIR)/, $(SUBCOBJS))
OBJS		:= $(SUBOBJS) $(addprefix $(CURDIR)/, $(COBJS))

LIB			:= $(CURDIR)/libfs.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(SUBOBJS):
	$(MAKE) -C $(dir $@)

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o
	(cd $(CURDIR)/termio; make clean)
	(cd $(CURDIR)/socket; make clean)
	(cd $(CURDIR)/netio; make clean)
	(cd $(CURDIR)/devio; make clean)
	(cd $(CURDIR)/fsio; make clean)

