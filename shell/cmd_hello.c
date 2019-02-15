
/**
 * Copyright(c) 2016-9-1 Shangwen Wu	
 *
 * 第一个测试命令；hello world!!! 
 * 
 */

#include <common.h>
#include <sys/types.h>					
#include <sys/endian.h>					
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <setjmp.h>
#include <command.h>
#include <fs/termio.h>
#include <sys/socket.h>					//for socket API
#include <sys/delay.h>					//for socket API
#include <sys/mbr_part.h>
#include <sys/blkio.h>
#include <netinet/in.h>
#include <arpa/inet.h>					//for inet_addr API

static int e;

static void go_jump(void)
{
	extern jmp_buf go_return_jmpbuf;	//defined in mipsdep.c

	printf("in go_jump\n");
	longjmp(go_return_jmpbuf, 1);
}

/**
 * 描述：执行hello测试命令
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_hello(int argc, const char *argv[])
{
	printf("hello world !\n");

#if 1 //test for longjmp/setjmp 
	int ret;
	extern jmp_buf go_return_jmpbuf;	//defined in mipsdep.c
	auto int a;
	register int b;
	static int c;
	volatile int d;

	a = b = c = d = e = 0;
	
	if(ret = setjmp(go_return_jmpbuf)) {
		printf("ret 0x%x\n", ret);
		printf("longjmp: a %d, b %d, c %d, d %d, e %d\n", a, b, c, d, e);
		return 0;
	} 
	a = 1, b = 2, c = 3, d = 4, e = 5;
	
	printf("setjmp: a %d, b %d, c %d, d %d, e %d\n", a, b, c, d, e);

	go_jump();

#elif 0 //test for malloc heap space
	char *ptr = NULL;

	if(!(ptr = (char *)malloc(4096))) {
		fprintf(stderr, "out of heap memory\n");
		return -1;
	}
	printf("ptr: %p\n", ptr);
	free(ptr);

#elif 0 //test for ext2 filesystem
	int fd;

	if(-1 == (fd = open("/dev/fs/ext2@ahci_disk0/file", O_RDONLY))) {
		perror("open ext2 file");
		return -1;
	}	

	close(fd);

#elif 0 //test for mbr partition 
	int fd, i;
	unsigned char buf[MBR_PART_SZ];
	struct mbr_partition *mbrpart;

	if(-1 == (fd = open("/dev/block/ahci_disk0", O_RDWR))) {
		perror("open block");
		return -1;
	}

	if(-1 == ioctl(fd, BIOCGMBRPART, buf)) {
		perror("ioctl block");
		return -1;
	}
	
#if 0
	for(i = 0; i < MBR_PART_SZ; ++i) 
		printf("buf[%d]=%02x ", i, buf[i]);
	printf("\n");
#endif

	/** 
 	 * bad code
 	 * 这里有潜在BUG！！！，当结构体在栈中的地址为非4字节对齐时，
 	 * 访问结构体中的4字节成员变量将会引起地址对齐访问异常 
 	 */
	mbrpart = (struct mbr_partition *)buf;
	for(i = 0; i < MBR_PRI_PART_CNT; ++i) {
		printf("partition-%d: \n", i);
		printf(" boot: %s\n", BOOT_ACTIVE_PARTITION == mbrpart->boot_signature ? "act" : "inact");
		printf(" filesystem: 0x%x\n", mbrpart->system_signature);
		printf(" heads: %u - %u\n", mbrpart->start_head, mbrpart->end_head);
		printf(" sectors: %u - %u\n", mbrpart->start_sector, mbrpart->end_sector);
		printf(" cylinders: %u - %u\n", mbrpart->start_cylinder, mbrpart->end_cylinder);
		printf(" before start sectors: %u\n", mbrpart->before_part_sector);
		printf(" total sectors: %u\n", mbrpart->sectors_total);
		++mbrpart;
	}

	close(fd);
#elif 0 /* test for block file */
	int fd, i, j, ret;
	ulong len;
	unsigned char buf[1024];

	if(-1 == (fd = open("/dev/block/ahci_disk0", O_RDONLY))) {
		perror("open block");
		return -1;
	}
#if 1		//test basic
	for(i = 0; i < 8; ++i) {
		if((ret = read(fd, buf, 128)) < 0) {
			perror("read block");
			close(fd);
			return -1;
		}
		printf("read %d bytes\n", ret);
		for(j = 0; j < 128; ++j)
			printf("buf[%d] = 0x%02x ", j, buf[j]);
		printf("\n", buf[j]);
	}
#endif
#if 1		//test llseek
	if((ret = read(fd, buf, 32)) < 0) {
		perror("read block");
		close(fd);
		return -1;
	}
	printf("read %d bytes\n", ret);
	for(j = 0; j < 32; ++j)
		printf("buf[%d] = 0x%02x ", j, buf[j]);
	printf("\n", buf[j]);
	//reread 32 byte
	if((ret = llseek(fd, 0, SEEK_SET)) < 0) {
		perror("llseek block");
		close(fd);
		return -1;
	}
	printf("lseek ===> %d\n", ret);
	if((ret = read(fd, buf, 32)) < 0) {
		perror("read block");
		close(fd);
		return -1;
	}
	printf("read %d bytes\n", ret);
	for(j = 0; j < 32; ++j)
		printf("buf[%d] = 0x%02x ", j, buf[j]);
	printf("\n");
	//read 480-512 indx byte
	if((ret = llseek(fd, 448, SEEK_CUR)) < 0) {
		perror("llseek block");
		close(fd);
		return -1;
	}
	printf("lseek ===> %d\n", ret);
	if((ret = read(fd, buf, 32)) < 0) {
		perror("read block");
		close(fd);
		return -1;
	}
	printf("read %d bytes\n", ret);
	for(j = 0; j < 32; ++j)
		printf("buf[%d] = 0x%02x ", j, buf[j]);
	printf("\n");
	//test next block
	if((ret = read(fd, buf, 32)) < 0) {
		perror("read block");
		close(fd);
		return -1;
	}
	printf("read %d bytes\n", ret);
	for(j = 0; j < 32; ++j)
		printf("buf[%d] = 0x%02x ", j, buf[j]);
	printf("\n");
	//test error
	if(llseek(fd, 0, SEEK_END) < 0) {
		perror("llseek block");
	}
	//test end 
	if((ret = read(fd, buf, 32)) < 0) {
		perror("read block");
		close(fd);
		return -1;
	}
	printf("read %d bytes\n", ret);
	for(j = 0; j < 32; ++j)
		printf("buf[%d] = 0x%02x ", j, buf[j]);
	printf("\n");
#endif

	close(fd);
#elif 0 /* test for delay func */
	udelay(1000000);
	printf("1s after\n");
	mdelay(4000);
	printf("5s after\n");
	delay(10);
	printf("15s after\n");
#elif 0	/* test for udp-socket */
	int sfd, ret, i;
	socklen_t len = sizeof(struct sockaddr_in);
	char data[8] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};
#define RECV_SIZE	1024
	char recv_data[RECV_SIZE] = {0};
	struct sockaddr_in to = {0}, myaddr = {0}, from = {0};

	if(-1 == (sfd = socket(AF_INET, SOCK_DGRAM, 0))) {
		perror("socket");
		return -1;
	}
	
	myaddr.sin_family = AF_INET;
	myaddr.sin_len = sizeof(struct sockaddr_in);
	myaddr.sin_port = htons(10010);
	myaddr.sin_addr.s_addr = inet_addr("192.168.6.123");
	if(-1 == bind(sfd, (const struct sockaddr *)&myaddr, sizeof(struct sockaddr_in))) {
		perror("bind");
		return -1;
	}

	//send
	to.sin_family = AF_INET;
	to.sin_len = sizeof(struct sockaddr_in);
	to.sin_port = htons(10086);
	to.sin_addr.s_addr = inet_addr("192.168.6.66");
	ret = sendto(sfd, data, sizeof(data), 0, (const struct sockaddr *)&to, sizeof(struct sockaddr_in));
	printf("sendto: %d\n", ret);

	//recv
	ret = recvfrom(sfd, recv_data, sizeof(recv_data), 0, (struct sockaddr *)&from, &len);
	printf("recvfrom: %d\n", ret);
	if(ret > 0)	{
		printf("from: %s, %d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port));
		for(i = 0; i < ret; ++i) 
			printf("0x%02x ", recv_data[i]);
		printf("\n");
	}

	close(sfd);
#elif 0			/* test strpat */
	if(strpat("123123123123321123", "*123*123*123"))
		printf("match ok\n");
	else
		printf("match fail\n");
#else			/* test kmem_malloc */
	char *test, *test2;
		
	test  = (char *)kmem_malloc(1024);
	if(!test) 
		printf("kmem_malloc test failed\n");
	else 
		printf("kmem_malloc test success ==> 0x%x\n", test);
	kmem_free(test);

	test2  = (char *)kmem_malloc(32768*16+1000);
	if(!test) 
		printf("kmem_malloc test failed\n");
	else 
		printf("kmem_malloc test success ==> 0x%x\n", test);
	kmem_free(test2);
#endif

	return 0;
}

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"Test"}},			//表组名
	{
		{"hello"}, 			
		"hello test command", 
		"",
		NULL, 
		cmd_hello,
		0,
		MAX_CMD_ARG_NUM - 1,
		0, 
	},
	{},						//最后一个强制要求为空
};

/**
 * 描述：命令初始化函数
 */
static void __attribute__((constructor)) init_cmds(void)
{
	add_cmds(cmds, 0);
}
