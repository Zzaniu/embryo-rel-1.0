
CROSS_COMPILE	?= /opt/gcc-4.4-gnu/bin/mipsel-linux-
ARCH			?= mips
MACH			?= ls2h

MACHDIR			:= $(TOPDIR)/arch/$(ARCH)/mach/$(MACH)

GZROMSTARTADDR	:= 0xffffffff8f900000
ROMSTARTADDR	:= 0xffffffff8f010000

CC				:= $(CROSS_COMPILE)gcc
LD				:= $(CROSS_COMPILE)ld
OBJCOPY			:= $(CROSS_COMPILE)objcopy

CCFLAGS			:= -mno-abicalls -fno-builtin \
						-fno-pic \
						-Wstrict-prototypes \
						-Wno-uninitialized \
 						-Wno-format \
						-Wno-main \
						-O2	\
						-G 0 \
						-mips3 \
						-I$(TOPDIR)/include \
						-I$(TOPDIR)/arch/$(ARCH)/include \
						-I$(TOPDIR)/arch/$(ARCH)/mach/$(MACH)/include \
						-I$(TOPDIR)/lib/include \
						-mno-abicalls -fno-builtin \
						-c 

ASMFLAGS			:= -mno-abicalls -fno-builtin \
						-fno-pic \
						-G 0 \
						-mips3 \
						-D_LOCORE \
						-I$(TOPDIR)/include \
						-I$(TOPDIR)/arch/$(ARCH)/include \
						-I$(TOPDIR)/arch/$(ARCH)/mach/$(MACH)/include \
						-I$(TOPDIR)/lib/include \
						-mno-abicalls -fno-builtin \
						-c 

LDFLAGS			:= -T$(MACHDIR)/ld.script -e _start 

%.o : %.c
	$(CC) $(CCFLAGS) $^ -o $@  
%.o : %.S
	$(CC) $(ASMFLAGS) $^ -o $@  

cmd_link_o_target = $(if $(strip $1), $(LD) -r $1 -o $@, echo "noting to do")

