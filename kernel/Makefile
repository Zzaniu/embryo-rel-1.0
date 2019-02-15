
include $(TOPDIR)/config.mk

COBJS	:= kern_misc.o 
COBJS	+= kern_malloc.o 
COBJS	+= autoconf.o 
COBJS	+= sys_errno.o 
COBJS	+= sys_param.o 
COBJS	+= kern_syscall.o 
COBJS	+= uipc_syscall.o 
COBJS	+= uipc_domain.o 
COBJS	+= uipc_socket.o 
COBJS	+= kern_filedesc.o 
COBJS	+= kern_proc.o 
COBJS	+= uipc_mbuf.o 
COBJS	+= kern_sync.o 
COBJS	+= sys_generic.o 
COBJS	+= sys_socket.o 

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libkernel.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

