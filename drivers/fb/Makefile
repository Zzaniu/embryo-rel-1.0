
include $(TOPDIR)/config.mk

COBJS	:= fb.o
COBJS	+= fb_console.o
COBJS	+= video_font.o

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libfb.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

