/**
 * Copyright(c) 2015-6-11 Shangwen Wu	
 *
 * LS2H I2C控制器主设备操作 
 * 
 */

#define I2C_REGS_BASE		_I2C_REGS_BASE(BIOS_I2C_CONTROLLER)

/*
 * 描述：初始化I2C控制器
 * 使用寄存器a1，a0
 */
LEAF(i2c_init)
	la			a0, I2C_REGS_BASE
	sb			zero, I2C_CTR(a0)
	li			a1, 0x2c
	sb			a1, I2C_PRERLO(a0)
	li			a1, 0x01
	sb			a1, I2C_PRERHI(a0)
	li			a1, 0x80
	sb			a1, I2C_CTR(a0)

	jr			ra
	nop		
END(i2c_init)

/* a0:设备从地址，a1：发送数据字节，a2：（保留），v1：I2C基地址，a3：CR控制命令 */
/*
 * 描述：等待总线空闲（可能导致死循环）
 * 参数：v1传入I2C控制器的首地址
 * 使用寄存器v0，v1
 */
#define I2C_WAIT_BUSY							\
1:												\
	lb			v0, I2C_SR(v1);					\
	andi		v0, I2C_SR_BUSY;				\
	bnez		v0, 1b;							\
	nop						

/*
 * 描述：等待传输完成（可能导致死循环）
 * 参数：v1传入I2C控制器的首地址
 * 使用寄存器v0，v1
 */
#define I2C_WAIT_TIP							\
1:												\
	lb			v0, I2C_SR(v1);					\
	andi		v0, I2C_SR_TIP;					\
	bnez		v0, 1b;							\
	nop	

/*
 * 描述：发送一个控制命令并等待传输完成（可能导致死循环）
 * 参数：v1传入I2C控制器的首地址，a3传入需要写入的命令
 * 使用寄存器a3，v0，v1
 */
#define I2C_TX_CR								\
	sb			a3, I2C_CR(v1);					\
	I2C_WAIT_TIP		

/*
 * 描述：是否收到ACK
 * 参数：v1传入I2C控制器的首地址
 * 返回：0表示收到ACK
 * 使用寄存器v0，v1
 */
#define I2C_RX_ACK								\
	lb			v0, I2C_SR(v1);					\
	andi		v0, I2C_SR_RXACK			

/*
 * 描述：等待ACK到达（可能导致死循环）
 * 参数：v1传入I2C控制器的首地址
 * 使用寄存器v0，v1
 */
#define I2C_WAIT_ACK							\
1:												\
	I2C_RX_ACK;									\
	bnez		v0, 1b;							\
	nop

/*
 * 描述：写一个字节操作（可能会死循环）
 * 参数：a0设备从地址，a1要写的数据字节
 * 使用寄存器：a0，a1，a3，v0，v1
 */
LEAF(i2c_write_byte)
	la			v1, I2C_REGS_BASE
	sll			a0, 1

	/* 写起始字节 */
	I2C_WAIT_BUSY
	sb			a0, I2C_TXR(v1)
	li			a3, I2C_CR_START | I2C_CR_WR 
	I2C_TX_CR
	I2C_WAIT_ACK

	/* 写数据字节 */
	sb			a1, I2C_TXR(v1)
	li			a3, I2C_CR_WR 
	I2C_TX_CR
	/* bug */
	I2C_WAIT_ACK

	/* 发送结束信号 */
	li			a3, I2C_CR_STOP 
	I2C_TX_CR
	I2C_WAIT_BUSY	

	jr			ra
	nop
END(i2c_write_byte)

/*
 * 描述：读一个字节操作（可能会死循环）
 * 参数：a0设备从地址，a1要读取的寄存器偏移
 * 返回：v0保存读到的数据
 * 使用寄存器a0，a1，a3，v0，v1
 */
LEAF(i2c_read_byte)
	la			v1, I2C_REGS_BASE
	sll			a0, 1

	/* 写起始字节 */
	I2C_WAIT_BUSY
	sb			a0, I2C_TXR(v1)
	li			a3, I2C_CR_START | I2C_CR_WR 
	I2C_TX_CR
	I2C_WAIT_ACK

	/* 写数据字节 */
	sb			a1, I2C_TXR(v1)
	li			a3, I2C_CR_WR 
	I2C_TX_CR
	/* bug */
	I2C_WAIT_ACK

	/* 写起始字节（读命令） */
	ori			a0, 0x01
	sb			a0, I2C_TXR(v1)
	li			a3, I2C_CR_START | I2C_CR_WR
	I2C_TX_CR
	/* bug */
	I2C_WAIT_ACK
	
	/* 读取接收的字节 */
	li			a3, I2C_CR_NACK | I2C_CR_RD
	I2C_TX_CR
	lb			v0, I2C_RXR(v1)
	
	/* 发送结束信号 */
	li			a3, I2C_CR_STOP 
	I2C_TX_CR
	I2C_WAIT_BUSY	
		
	jr			ra
	nop
END(i2c_read_byte)
