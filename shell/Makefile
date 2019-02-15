
include $(TOPDIR)/config.mk

COBJS	:= main.o 
COBJS	+= command.o 
COBJS	+= console.o 
COBJS	+= history.o 
COBJS	+= getopt.o 
COBJS	+= cmd_hello.o 
COBJS	+= cmd_misc.o 
COBJS	+= cmd_shell.o 
COBJS	+= cmd_mem.o 
COBJS	+= cmd_lsdev.o 
COBJS	+= cmd_ping.o 
COBJS	+= cmd_ifconfig.o 
COBJS	+= cmd_mdio.o 
COBJS	+= cmd_tftp.o 
COBJS	+= cmd_ata.o 
COBJS	+= cmd_fdisk.o 
COBJS	+= cmd_ls.o 
COBJS	+= cmd_cat.o 
COBJS	+= cmd_load.o 
COBJS	+= cmd_go.o 
COBJS	+= cmd_boot.o 

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libshell.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

