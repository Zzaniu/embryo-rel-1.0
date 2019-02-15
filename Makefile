
TOPDIR			:= $(CURDIR)

export TOPDIR

include $(TOPDIR)/config.mk			

START			:= $(MACHDIR)/start.o
LIBS			:= $(MACHDIR)/lib$(MACH).o
LIBS			+= $(TOPDIR)/arch/$(ARCH)/asm/libasm.o
LIBS			+= $(TOPDIR)/arch/$(ARCH)/lib$(ARCH).o
LIBS			+= $(TOPDIR)/common/libcommon.o
LIBS			+= $(TOPDIR)/lib/liblib.o
LIBS			+= $(TOPDIR)/kernel/libkernel.o
LIBS			+= $(TOPDIR)/drivers/libdrivers.o
LIBS			+= $(TOPDIR)/fs/libfs.o
LIBS			+= $(TOPDIR)/net/libnet.o
LIBS			+= $(TOPDIR)/shell/libshell.o
LIBS			+= $(TOPDIR)/autoconf/libautoconf.o
LIBS			+= $(TOPDIR)/exec/libexec.o
LIBS			+= $(TOPDIR)/boot/libboot.o
##LIBS			+= $(TOPDIR)/zloader/libzloader.o

gzembryo.bin : $(START) zloader.o
#	$(LD) $(LDFLAGS) $^ -o $@
#	$(OBJCOPY) -O binary $^ $@ 

zloader.o : embryo.bin.c

embryo.bin.c : embryo.bin
	gzip embryo.bin -c > embryo.bin.gz
	./bin2c embryo.bin.gz embryo.bin.c biosdata

embryo.bin : embryo
	$(OBJCOPY) -O binary $^ $@ 

embryo : $(START) $(LIBS)
	$(CC) $(MACHDIR)/ld.script.S -E -P -DSTARTADDR=$(ROMSTARTADDR) -Umips > $(MACHDIR)/ld.script
	$(LD) $(LDFLAGS) $^ -o $@

$(START) : $(MACHDIR)/start.S
	$(CC) $(ASMFLAGS) $^ -o $@

$(LIBS) :  
	$(MAKE) -C $(dir $@) 

.PHONY : clean
clean:
	rm -rf $(MACHDIR)/ld.script
	rm -rf embryo.bin embryo embryo.bin.gz embryo.bin.c
	rm -rf $(START)
	(cd $(MACHDIR)/; make clean)
	(cd $(TOPDIR)/arch/$(ARCH)/asm/; make clean)
	(cd $(TOPDIR)/arch/$(ARCH)/; make clean)
	(cd $(TOPDIR)/common; make clean)
	(cd $(TOPDIR)/lib; make clean)
	(cd $(TOPDIR)/kernel; make clean)
	(cd $(TOPDIR)/drivers; make clean)
	(cd $(TOPDIR)/fs; make clean)
	(cd $(TOPDIR)/net; make clean)
	(cd $(TOPDIR)/shell; make clean)
	(cd $(TOPDIR)/autoconf; make clean)
	(cd $(TOPDIR)/exec; make clean)
	(cd $(TOPDIR)/boot; make clean)
##	(cd $(TOPDIR)/zloader; make clean)

