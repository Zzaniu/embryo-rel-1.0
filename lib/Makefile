
include $(TOPDIR)/config.mk

COBJS	:= vsprintf.o
COBJS	+= sprintf.o
COBJS	+= string.o
COBJS	+= cvt.o
COBJS	+= malloc.o
COBJS	+= sbrk.o
COBJS	+= crc.o
COBJS	+= queue.o
COBJS	+= open.o
COBJS	+= close.o
COBJS	+= write.o
COBJS	+= read.o
COBJS	+= printf.o
COBJS	+= vfprintf.o
COBJS	+= fprintf.o
COBJS	+= stdio.o
COBJS	+= fputs.o
COBJS	+= ioctl.o
COBJS	+= lseek.o
COBJS	+= putchar.o
COBJS	+= fputc.o
COBJS	+= ungetc.o
COBJS	+= getchar.o
COBJS	+= fgetc.o
COBJS	+= puts.o
COBJS	+= ctype.o
COBJS	+= strings.o
COBJS	+= div64.o
COBJS	+= errno.o
COBJS	+= inet_addr.o
COBJS	+= url.o
COBJS	+= getservbyname.o
COBJS	+= getservent.o
COBJS	+= stat.o
COBJS	+= dirent.o

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/liblib.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

