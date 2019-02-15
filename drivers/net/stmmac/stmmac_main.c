
/**
 * Copyright(c) 2017-1-19 Shangwen Wu
 * stmmac(gmac)设备驱动
 * 
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <string.h>
#include <strings.h>
#include <sys/list.h>
#include <sys/system.h>
#include <sys/err.h>
#include <sys/param.h>
#include <sys/syslog.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/device.h>
#include <sys/mdio.h>
#include <sys/phy.h>
#include <fs/termio.h>

#include <asm/cache.h>
#include <asm/dma-mapping.h>

#include <net/if.h>
#include <net/route.h>
#include <net/if_ether.h>

#include <netinet/in.h>
#include <netinet/in_arp.h>

#include "dma_desc.h"
#include "common.h"
#include "stmmac.h"

/* stmmac驱动相关选项 */
static int phyaddr = -1;		/* -1：自动检测PHY地址，其他值：指定PHY地址 */
static int is_gmac = 1;			/* 使用千兆GMAC？ */
static int is_enh_desc = 1;		/* 使用增强型描述符？ */
static int is_gmac_3_3v = 0;	/* 使用3.3V RGMII接口电压 */
static int pbl = 32;			/* 收发突发长度 */
static int tc = TC_DEFAULT;		/* 发送最大阀值 */
static int tx_coe = HW_CKSUM;	/* 发送校验和是否硬件计算 */
static unsigned int bufsize = L1_CACHE_ALIGN(BUF_SIZE_2KB);/* 数据缓冲区长度 */
static unsigned int tx_ring_sz =  TX_DMA_SIZE;	/* 发送描述符个数 */
static unsigned int rx_ring_sz =  RX_DMA_SIZE;	/* 接收描述符个数 */
uint32_t csrclk = 0; 			/* 在stmmac_mdio_register中被设置成默认值 */

/**
 * 描述：分配硬件信息结构，设置对应的硬件操作函数操作集合
 */
static int stmmac_mac_info_setup(struct stmmac_device *stmdev)
{
	struct mac_device_info *hw;
	
	if(!(hw = kmem_malloc(sizeof(struct mac_device_info)))) {
		log(LOG_ERR, "stmmac_mac_info_setup: out of memory!\n");
		return ENOMEM;
	}

	if(is_gmac) {
		dwmac1000_info_setup(stmdev, hw);
	} else {
		/* 暂不支持 */
		kmem_free(hw);
		return EOPNOTSUPP;
	}

	if(is_gmac) {
		hw->desc = &enh_desc_ops;
	} else {
		/* 暂不支持 */
		kmem_free(hw);
		return EOPNOTSUPP;
	}

	stmdev->hw = hw;

	return 0;
}

/**
 * 描述：释放描述符占用内存
 */
static void free_dma_desc_resources(struct stmmac_device *stmdev)
{
	unsigned int i;
	struct dma_desc *d;

	if(stmdev->buf_rx) {
		for(i = 0; i < rx_ring_sz; ++i) {
			if(stmdev->buf_rx[i]) {
				if(stmdev->dma_rx) {
					d = stmdev->dma_rx + i;
					if(d->des2) {
						dma_unmap_single(stmdev, d->des2, 
								bufsize, DMA_FROM_DEVICE);
						d->des2 = 0;
					}
				}
				kmem_free(stmdev->buf_rx[i]);
				stmdev->buf_rx[i] = NULL;
			}
		}
	}

	if(stmdev->buf_tx) {
		for(i = 0; i < tx_ring_sz; ++i) {
			if(stmdev->buf_tx[i]) {
				if(stmdev->dma_tx) {
					d = stmdev->dma_tx + i;
					if(d->des2) {
						dma_unmap_single(stmdev, d->des2, 
								stmdev->hw->desc->get_tx_len(d), 
								DMA_TO_DEVICE);
						d->des2 = 0;
					}
				}
				kmem_free(stmdev->buf_tx[i]);
				stmdev->buf_tx[i] = NULL;
			}
		}
	}

	if(stmdev->dma_rx)
		dma_free_coherent(stmdev, rx_ring_sz * sizeof(struct dma_desc), stmdev->dma_rx, stmdev->dma_rx_phy);
	if(stmdev->dma_tx)
		dma_free_coherent(stmdev, tx_ring_sz * sizeof(struct dma_desc), stmdev->dma_tx, stmdev->dma_tx_phy);
	if(stmdev->buf_rx)
		kmem_free(stmdev->buf_rx);
	if(stmdev->buf_tx)
		kmem_free(stmdev->buf_tx);
}

/**
 * 描述：以环模式初始化DMA描述符
 */
static int init_dma_desc_rings(struct stmmac_device *stmdev)
{
	int err;
	unsigned int i;
	struct dma_desc *d;

	if(!(stmdev->buf_rx = (uint8_t **)kmem_zmalloc(rx_ring_sz * sizeof(uint8_t *))) 
			|| !(stmdev->buf_tx = (uint8_t **)kmem_zmalloc(tx_ring_sz * sizeof(uint8_t *)))) {
		log(LOG_ERR, "init_dma_desc_rings: buf_tx/rx[] alloc failed\n");
		err = ENOMEM;
		goto desc_failed;
	}

	/* 初始化接收描述符 */
	if(!(stmdev->dma_rx = (struct dma_desc *)dma_alloc_coherent(stmdev, 
					rx_ring_sz * sizeof(struct dma_desc), 
					&stmdev->dma_rx_phy))) {
		log(LOG_ERR, "init_dma_desc_rings: dma_rx alloc failed\n");
		err = ENOMEM;
		goto desc_failed;
	}
	/* 初始化发送描述符 */
	if(!(stmdev->dma_tx = (struct dma_desc *)dma_alloc_coherent(stmdev,
					tx_ring_sz * sizeof(struct dma_desc), 
					&stmdev->dma_tx_phy))) {
		log(LOG_ERR, "init_dma_desc_rings: dma_tx alloc failed\n");
		err = ENOMEM;
		goto desc_failed;
	}

	/**
 	 * 2018-8-30 23：45：00 Embryo 第一次稳定地发出了ARP数据报文
 	 * 纪念曾为之奋斗了4天解决的BUG
 	 */

	/* 分配接收数据缓冲区 */
	for(i = 0; i < rx_ring_sz; ++i) {
		if(!(stmdev->buf_rx[i] = (uint8_t *)kmem_zmalloc(bufsize))) {
			log(LOG_ERR, "init_dma_desc_rings: rx data buf alloc failed\n");
			err = ENOMEM;
			goto desc_failed;
		}
		d = stmdev->dma_rx + i;
		d->des2 = dma_map_single(stmdev, 
					stmdev->buf_rx[i], bufsize, DMA_FROM_DEVICE);
	}
	stmdev->cur_dma_rx = 0;
	//stmdev->dirty_dma_rx = 0;

	/* 清空部分tx字段 */
	for(i = 0; i < tx_ring_sz; ++i) {
		stmdev->buf_tx[i] = NULL;
		stmdev->dma_tx[i].des2 = 0;
	}
	stmdev->cur_dma_tx = 0;
	stmdev->dirty_dma_tx = 0;

	stmdev->hw->desc->init_rx_desc(stmdev->dma_rx, rx_ring_sz, 0);
	stmdev->hw->desc->init_tx_desc(stmdev->dma_tx, tx_ring_sz);

	return 0;

desc_failed:

	free_dma_desc_resources(stmdev);

	return err;
}

/**
 * 描述：设置DMA模式
 */
static void stmmac_set_dma_operation_mode(struct stmmac_device *stmdev)
{
	if(is_gmac) {			/* 1000M GMAC */
		/* 如果单次传输量小于以太网MTU并且硬件计算校验和，则开启存储转发模式 */
		if((stmdev->ethdev->eth_if.if_mtu <= ETHERMTU) && 
					(HW_CKSUM == tx_coe)){
			stmdev->hw->dma->dma_mode(stmdev, SF_DMA_MODE, SF_DMA_MODE);
			tc = SF_DMA_MODE;
		} else {
			stmdev->hw->dma->dma_mode(stmdev, tc, SF_DMA_MODE);
			tx_coe = SW_CKSUM;
		}
	} else {				/* 100/10 GMAC */
		/* 百兆仅支持软件校验和以及非存储转发模式 */	
		stmdev->hw->dma->dma_mode(stmdev, tc, 0);
		tx_coe = SW_CKSUM;
	}
}

/**
 * 描述：释放所有发送描述的数据缓冲区内存空间
 */
static void free_tx_buf(struct stmmac_device *stmdev)
{
	unsigned int i;
	struct dma_desc *d;

	if(stmdev->buf_tx) {
		for(i = 0; i < tx_ring_sz; ++i) {
			if(stmdev->buf_tx[i]) {
				d = stmdev->dma_tx + i;
				if(d->des2) {
					dma_unmap_single(stmdev, d->des2, 
							stmdev->hw->desc->get_tx_len(d), 
							DMA_TO_DEVICE);
					d->des2 = 0;
				}
				kmem_free(stmdev->buf_tx[i]);
				stmdev->buf_tx[i] = NULL;
			}
		}
	}
}

/**
 * 描述：接收数据是否符合要求
 */
static int stmmac_rx_status(struct stmmac_device *stmdev, struct dma_desc *d)
{
	int ret = 0;

	if((ret = stmdev->hw->desc->read_rx_status(d)) != 0)
		return ret;

	/* 这里对接收报文长度作出限制，要求报文不大于min(以太网最大包长, EXT_MAX_LEN) */
	if(stmdev->hw->desc->get_rx_frame_len(d) > ETH_FRAME_LEN) {
		STMMAC_DBG("%s: data is too loog for ethernet\n", __func__);
		ret = -1;
	}

	/* 这里对中间片段的描述符暂不做支持 */
	if(!stmdev->hw->desc->is_rx_first_desc(d) && !stmdev->hw->desc->is_rx_last_desc(d)) {
		STMMAC_DBG("%s: desc is segment\n", __func__);
		ret = -1;
	}

	return ret;
}

/**
 * 描述：分配一个用于接收的mbuf
 * 注意：该mbuf为EXT型，可存放最大以太网长度的数据
 */
static struct mbuf *stmmac_getmbuf(void)
{
#define IP_ALIGN_LEN	2

	struct mbuf *m;	

	if(!(m = mbuf_getpkt(MT_DATA)))
		return NULL;
	
	if(mbuf_getext(m) != 0) {
		mbuf_freem(m);
		return NULL;
	}

	/* 注意：这里数据指针+2是为了使IP报文首部4字节对齐 */
	m->m_data += IP_ALIGN_LEN;

	return m;
}

/**
 * 描述：处理接收数据
 */
static void stmmac_rx(struct stmmac_device *stmdev)
{
	struct dma_desc *d;
	unsigned int entry, flen;
	struct mbuf *m;

	STMMAC_DBG("stmmac_rx\n");

	entry = stmdev->cur_dma_rx % rx_ring_sz;
	d = stmdev->dma_rx + entry;

	while(!stmdev->hw->desc->get_rx_owner(d)) {
		if(!stmmac_rx_status(stmdev, d)) {
			/* 接收数据有效 */
			/* 分配mbuf，并且拷贝数据 */
			if((m = stmmac_getmbuf()) != NULL) {
				/* 上层协议无需以太网FCS */
				flen = stmdev->hw->desc->get_rx_frame_len(d) - ETH_FCS_LEN;
				if(d->des2)
					dma_unmap_single(stmdev, d->des2, 
							bufsize, DMA_FROM_DEVICE);
				bcopy(stmdev->buf_rx[entry], mtod(m, caddr_t), flen);
				m->m_pkthdr.mp_len = m->m_len = flen;
				m->m_pkthdr.mp_recvif = &stmdev->ethdev->eth_if;
				++stmdev->ethdev->eth_if.if_ipackets;
				/* 传入数据到上层协议栈中 */
				ether_input(&stmdev->ethdev->eth_if, m);
			} else {
				STMMAC_DBG("%s: can't allocate mbuf, discard packet\n", __func__);
				++stmdev->ethdev->eth_if.if_ierrors;
			}
		} else {
			/* 接收出错 */
			++stmdev->ethdev->eth_if.if_ierrors;
		}
		/* 回收接收描述符 */
		stmdev->hw->desc->set_rx_owner(d);
		d->des2 = dma_map_single(stmdev, 
					stmdev->buf_rx[entry], bufsize, DMA_FROM_DEVICE);

		/* next one */
		entry = ++stmdev->cur_dma_rx % rx_ring_sz;
		d = stmdev->dma_rx + entry;
	}
}

/**
 * 描述：处理发送完成后描述符的回收和资源释放
 */
static void stmmac_tx(struct stmmac_device *stmdev)
{
	struct dma_desc *d;
	unsigned int entry;

	STMMAC_DBG("stmmac_tx\n");

	while(stmdev->dirty_dma_tx != stmdev->cur_dma_tx) {
		STMMAC_DBG("%s, dirty %u, cur %u\n", __func__, stmdev->dirty_dma_tx, stmdev->cur_dma_tx);
		entry = stmdev->dirty_dma_tx % tx_ring_sz;
		d = stmdev->dma_tx + entry;
		/* 如果当前描述符还正在被DMA占用，那么将退出回收 */
		if(stmdev->hw->desc->get_tx_owner(d)) 
			break;
		/* 如果当前描述符为最后一个片段，则更新统计信息 */
		if(stmdev->hw->desc->is_tx_last_desc(d)) {
			if(!stmdev->hw->desc->read_tx_status(d)) {
				/* 发送成功 */
				++stmdev->ethdev->eth_if.if_opackets;
			} else{
				/* 发送出错*/
				++stmdev->ethdev->eth_if.if_oerrors;;
			}
		}
		if(d->des2)
			dma_unmap_single(stmdev, d->des2, 
					stmdev->hw->desc->get_tx_len(d), 
					DMA_TO_DEVICE);
		if(stmdev->buf_tx[entry]) {
			kmem_free(stmdev->buf_tx[entry]);
			stmdev->buf_tx[entry] = NULL;
		}

		stmdev->hw->desc->release_tx_desc(d);
		++stmdev->dirty_dma_tx;
	}
}

/**
 * 描述：处理发送异常时，tx发送逻辑重启
 */
static void stmmac_tx_err(struct stmmac_device *stmdev)
{
	stmdev->hw->dma->stop_tx(stmdev);
	free_tx_buf(stmdev);
	stmdev->hw->desc->init_tx_desc(stmdev->dma_tx, tx_ring_sz);
	stmdev->cur_dma_tx = 0;
	stmdev->dirty_dma_tx = 0;
	stmdev->hw->dma->start_tx(stmdev);
}

/**
 * 描述：判断当前gmac是否需要进行发送描述符回收或者接收数据
 */
static int stmmac_has_work(struct stmmac_device *stmdev)
{
	int tx_work, rx_work = tx_work = 0;

	/* 有需要进行回收的描述符 */
	if(stmdev->dirty_dma_tx != stmdev->cur_dma_tx)
		tx_work = 1;

	/* 当接收描述符的所有者被DMA控制器设置为host时，表示该描述符接收到新数据 */
	rx_work = !stmdev->hw->desc->get_rx_owner(stmdev->dma_rx + (stmdev->cur_dma_rx % rx_ring_sz));
	
	return (tx_work || rx_work);
}

/**
 * 描述：用来处理DMA中断
 */
static void stmmac_dma_interrupt(struct stmmac_device *stmdev)
{
	int status;

	status = stmdev->hw->dma->dma_interrupt(stmdev);

	if(handle_tx_rx == status) {
		if(stmmac_has_work(stmdev)) {
			stmmac_tx(stmdev);
			stmmac_rx(stmdev);
		}
	} else if(tx_hard_error == status) {
		stmmac_tx_err(stmdev);
	}
}

/**
 * 描述：网口中断处理程序
 */
static void stmmac_handler(void *arg)
{
	struct stmmac_device *stmdev = (struct stmmac_device *)arg;

	//STMMAC_DBG("%s handler...\n", stmdev->ethdev->eth_if.if_xname);
	if(is_gmac) 
		stmdev->hw->mac->host_interrupt(stmdev);

	stmmac_dma_interrupt(stmdev);
}

/**
 * 描述：该函数完成网络设备的初始化工作
 */
static int stmmac_open(struct ifnet *ifp)
{
	int err;
	struct stmmac_device *stmdev = (struct stmmac_device *)ifp->if_pri;
	struct ether_dev *ethdev = stmdev->ethdev;
	struct phy_device *phydev;

	STMMAC_DBG("stmmac_open...\n");

	/* 检查MAC地址是否合法 */
	if(!is_valid_ether_addr(ethdev->eth_addr)) {
		log(LOG_ERR, "stmmac_open: mac address is not valid!\n");
		return ENXIO;
	}

	/* 进行PHY的初始化工作 */
	if(IS_ERR(phydev = phy_connect(stmdev, stmdev->bus, stmdev->phyaddr))) {
		log(LOG_ERR, "stmmac_open: phy_connect failed!\n");
		return PTR_ERR(phydev);	
	}

	/* 初始化DMA描述符 */
	if((err = init_dma_desc_rings(stmdev)) != 0) {
		log(LOG_ERR, "stmmac_open: DMA desc ring init failed!\n");
		goto disconphy_failed;
	}

	/* DMA控制器初始化 */
	if((err = stmdev->hw->dma->dma_init(stmdev, pbl, stmdev->dma_tx_phy, stmdev->dma_rx_phy)) != 0) {
		log(LOG_ERR, "stmmac_open: DMA init failed!\n");
		goto freedesc_failed;
	}

	/* 写入MAC地址到硬件寄存器 */
	stmdev->hw->mac->set_mac_addr(stmdev, stmdev->ethdev->eth_addr, 0);
	
	/* 平台初始化相关代码 */
	if(stmdev->bsp_setup)
		stmdev->bsp_setup(stmdev);

	/* MAC控制器初始化 */
	stmdev->hw->mac->core_init(stmdev);

	/* 注册中断轮询函数 */
	if(!(ethdev->eth_intrhandler = spoll_register(0, stmmac_handler, stmdev))) {
		log(LOG_ERR, "stmmac_open: irq handler register failed!\n");
		err = ENOMEM;
		goto freedesc_failed;
	}

	/* 使能gmac收发 */
	stmdev->hw->mac->enable_tx(stmdev);
	stmdev->hw->mac->enable_rx(stmdev);

	/* 初始化DMA操作模式 */
	stmmac_set_dma_operation_mode(stmdev);

	/* 启动收发DMA */
	stmdev->hw->dma->start_tx(stmdev);
	stmdev->hw->dma->start_rx(stmdev);

	return 0;

freedesc_failed:
	free_dma_desc_resources(stmdev);
disconphy_failed:
	phy_disconnect(stmdev, stmdev->bus, stmdev->phyaddr);

	return err;
}

/**
 * 描述：该函数释放网络设备占用资源
 */
static void stmmac_remove(struct ifnet *ifp)
{
	struct stmmac_device *stmdev = (struct stmmac_device *)ifp->if_pri;
	struct ether_dev *ethdev = stmdev->ethdev;

	STMMAC_DBG("stmmac_remove...\n");

	stmdev->hw->dma->stop_tx(stmdev);
	stmdev->hw->dma->stop_rx(stmdev);

	stmdev->hw->mac->disable_tx(stmdev);
	stmdev->hw->mac->disable_rx(stmdev);

	spoll_unregister(ethdev->eth_intrhandler);
	free_dma_desc_resources(stmdev);
	phy_disconnect(stmdev, stmdev->bus, stmdev->phyaddr);
}

static int stmmac_ioctl(struct ifnet *ifp, unsigned long cmd, caddr_t data)
{
	int err;
	struct ifaddr *ifa;
	struct ifreq *ifr;
	struct ether_dev *ethdev = (struct ether_dev *)ifp;
	
	switch(cmd) {
		case SIOCSIFADDR:
			ifa = (struct ifaddr *)data;
			if(!(ifp->if_flags & IFF_UP)) {
				if((err = stmmac_open(ifp)) != 0)
					return err;
				ifp->if_flags |= IFF_UP;
			}	
			arp_ifinit(ethdev, ifa);//初始化ifa->ifa_rtrequest字段
			break;
		case SIOCSIFFLAGS:	//bad code
			ifr = (struct ifreq *)data;
			if((ifr->ifr_flags & IFF_UP) && !(ifp->if_flags & IFF_UP)) {
				if((err = stmmac_open(ifp)) != 0)
					return err;
				ifp->if_flags |= IFF_UP;
			} else if(!(ifr->ifr_flags & IFF_UP) && (ifp->if_flags & IFF_UP)) {
				stmmac_remove(ifp);
				ifp->if_flags &= ~IFF_UP;
			}
			break;
		case SIOCGETETHERADDR:
			ifr = (struct ifreq *)data;
			return copyout(ifr->ifr_data, ethdev->eth_addr, ETH_ADDR_LEN);
		defualt:
			return EOPNOTSUPP;
	}

	return 0;
}

/**
 * 描述：返回当面描述环中有空闲描述符的个数
 */ 
static unsigned int stmmac_tx_avail(struct stmmac_device *stmdev)
{
	return stmdev->dirty_dma_tx + tx_ring_sz - stmdev->cur_dma_tx - 1;
}

/**
 * 描述：发送一个数据报文
 * 注意：当前函数仅支持数据报文小于bufsize的单报文发送
 */ 
static int stmmac_xmit(struct ifnet *ifp)
{
	ulong len;
	struct mbuf *m;
	unsigned int entry;
	struct dma_desc *desc;
	int chksum_insert = 0;
	struct stmmac_device *stmdev = (struct stmmac_device *)ifp->if_pri;

	STMMAC_DBG("stmmac_xmit...\n");
	
	/**
 	 * 注意：每发起一次传输，将发送整个发送队列中正在排队的所有数据报文，
 	 * 		 而非只发送单个报文
     */

	while(!IFQ_ISEMPTY(&ifp->if_sndq)) {
		/* 注意：只要判断队列不空，就将引起出队操作 */
		IFQ_DEQUEUE(&ifp->if_sndq, m);

		if(!stmmac_tx_avail(stmdev)) {
			log(LOG_ERR, "stmmac_xmit: no free dma desc!\n");
			return EBUSY;
		}

		len = m->m_pkthdr.mp_len;
		if(len > bufsize) {
			log(LOG_ERR, "stmmac_xmit: tx packet too long!\n");
			return ENOBUFS;
		}
		/* 初始化DMA发送缓冲区 */	
		entry = stmdev->cur_dma_tx % tx_ring_sz;
		desc = stmdev->dma_tx + entry;

		if(!(stmdev->buf_tx[entry] = kmem_malloc(len))) {
			log(LOG_ERR, "stmmac_xmit: no buffer for xmit!\n");
			return ENOBUFS;
		}
		mbuf_copydata(m, 0, len, stmdev->buf_tx[entry]);
		mbuf_freem(m);
		/* 设置发送描述符 */
		if(HW_CKSUM == tx_coe)
			chksum_insert = 1;
		desc->des2 = dma_map_single(stmdev, stmdev->buf_tx[entry], len, DMA_TO_DEVICE);
		/* 写入DMA长度，校验和，起始和结束标志位等信息 */
		stmdev->hw->desc->prepare_tx_desc(desc, 1, len, chksum_insert);
		stmdev->hw->desc->set_tx_last_desc(desc);
		/* 将描述符设置为DMA控制 */
		stmdev->hw->desc->set_tx_owner(desc);
		/* 调整当前可用描述符位置 */
		++stmdev->cur_dma_tx;

		/* 启动DMA发送 */
		stmdev->hw->dma->enable_dma_transmission(stmdev);
	}

	return 0;
}

/**
 * 描述：与平台相关的硬件初始化工作
 */
static void ls2h_bsp_gmac_setup(void *p)
{
	if(is_gmac_3_3v) {
		*(volatile unsigned long *)0xbfd0020c |= 0x800f0000;	//10mA driver & 3.3V
		*(volatile unsigned long *)0xbfd0020c &= ~0x80000000;	//10mA driver & 3.3V
	}
}

int stmmac_init(struct stmmac_device *stmdev, uint32_t iobase)
{
	int err;
	uint64_t iobase64 = 0xffffffff00000000LL | (uint64_t)iobase;
	struct ether_dev *ethdev;
	struct ifnet *ifp;
	uint8_t dev_addr[ETH_ADDR_LEN] = {0x10, 0x01, 0x02, 0x03, 0x04, 0x05}; //bad code

	stmdev->iobase = iobase64;
	stmdev->phyaddr = phyaddr;

	stmdev->bsp_setup = ls2h_bsp_gmac_setup;

	if(IS_ERR(ethdev = alloc_etherdev(stmdev->dev.dv_name, dev_addr))) {
		err = PTR_ERR(ethdev);
		goto failed;
	}

	/* 初始化与特定网络接口驱动相关的变量 */
	ifp = &ethdev->eth_if;
	ifp->if_metric = 1;
	ifp->if_flags |= IFF_BROADCAST | IFF_MULTICAST;
	ifp->if_start = stmmac_xmit;
	ifp->if_ioctl = stmmac_ioctl;
	ifp->if_pri = stmdev;
	/* 设置ifp发送队列大小 */
	ifp->if_sndq.ifq_maxlen = TX_QUEUE_SIZE - 1;

	stmdev->ethdev = ethdev;

	if((err = stmmac_mac_info_setup(stmdev)) != 0)
		goto free_failed;
	
	if((err = stmmac_mdio_register(stmdev)) != 0)
		goto free_failed;

	return 0;

free_failed:
	if(stmdev->hw)
		kmem_free(stmdev->hw);
	if(stmdev->ethdev)
		free_etherdev(stmdev->ethdev);

failed:
	return err;
}

void stmmac_exit(struct stmmac_device *stmdev)
{
	stmmac_mdio_unregister(stmdev);

	if(stmdev->hw)
		kmem_free(stmdev->hw);
	if(stmdev->ethdev)
		free_etherdev(stmdev->ethdev);
}

static int stmmac_match(struct device *parent, void *match, void *aux)
{
	return 1;
}

static int stmmac_attach(struct device *parent, struct device *self, void *aux)
{
	int err = 0;
	uint32_t iobase;
	struct confargs *ca = (struct confargs *)aux;

	log(LOG_DEBUG, "in %s attach function\n", self->dv_name);
	iobase = ca->ca_iobase;

	if((err = stmmac_init((struct stmmac_device *)self, iobase)))
		log(LOG_ERR, "stmmac init failed\n");

	return err;
}


static void stmmac_detach(struct device *parent, struct device *self, void *aux)
{
	log(LOG_DEBUG, "in %s deattach function\n", self->dv_name);
	stmmac_exit((struct stmmac_device *)self);
}

struct cfattach stmmac_ca = {
	sizeof(struct stmmac_device),
	stmmac_match,
	stmmac_attach,
	stmmac_detach,
};

struct cfdriver stmmac_cd = {
	LIST_HEAD_INIT(stmmac_cd.cd_devlist),
	"stmmac",
	DEV_NET,
	0,
};

