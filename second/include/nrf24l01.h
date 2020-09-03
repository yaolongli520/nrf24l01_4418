#ifndef __NRF24L01_H_
#define __NRF24L01_H_

#include "system.h"

#define CRC_MODE_8   0x00
#define CRC_MODE_16  0x01

#define NRF_OK				0x00
#define	NRF_FAIL_NODEV		0x01 /*没找到设备节点*/
#define NRF_FAIL_NOFIND		0x01 /*没找到硬件设备*/
#define NRF_FAIL_TX			0x02 /* 发送出错 */

struct ctl_data{
	u8 tx_addr[5];
	u8 rx_addr[5];
	u8 channel;
	u8 crc;
};

int nrf24l01_init(struct ctl_data &cur);
int nrf24l01_tx(u8 *buf,u32 count);
u32 nrf24l01_rx(u8 *buf,u32 count);

#endif


