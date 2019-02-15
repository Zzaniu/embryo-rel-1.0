
OBJS += test.o

CROSS_COMPILE := /opt/gcc-4.4-64-gnu/bin/mips64el-linux-
TARGET := test

CCFLAG := -fno-builtin

export CROSS_COMPILE CCFLAG


$(TARGET).bin : $(TARGET)
	$(CROSS_COMPILE)objcopy -O binary $<  $@ 

$(TARGET) : $(OBJS) 
	$(CROSS_COMPILE)ld -T$(TARGET).lds $^ -o $@

%.o : %.c
	$(CROSS_COMPILE)gcc -c $< -o $@ $(CCFLAG)

%.o : %.S
	$(CROSS_COMPILE)gcc -c $< -o $@ $(CCFLAG)
	
.PHONY : clean
clean:
	rm *.o -rf
	rm test.bin test -rf
