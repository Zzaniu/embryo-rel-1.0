
include $(TOPDIR)/config.mk

COBJS	:= in_proto.o 
COBJS	+= raw_ip.o 
COBJS	+= in_pcb.o 
COBJS	+= ip_output.o 
COBJS	+= ip_input.o 
COBJS	+= udp.o 
COBJS	+= in.o 
COBJS	+= ip_icmp.o 
COBJS	+= in_cksum.o 
COBJS	+= arp.o 

OBJS	:= $(addprefix $(CURDIR)/, $(COBJS))

LIB		:= $(CURDIR)/libinet.o

$(LIB) : $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

.PHONY : clean
clean:
	rm -rf $(COBJS) *.o

