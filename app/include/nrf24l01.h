#ifndef __NRF24L01_H_
#define __NRF24L01_H_

#define DEVICE_NAME  "/dev/nrf24l01"

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define TRAN_LEN_MAX  32 /*发送单次最大字节*/
#define EACH_LEN_MAX  28  /*每个包最多容纳数据个数*/
#define PACK_HEAD_LEN 32 /*包头的长度*/
#define PACK_TOTAL_LEN 32 /*数据包总长*/
#define EACH_LEN_REQ 7  /*每个REQ包的数据个数*/
/*数据容量转换为包占用内存容量 如:28-->32*/
#define DATALEN_TO_PACKLEN(len) (DIV_ROUND_UP(len,EACH_LEN_MAX)*TRAN_LEN_MAX)

//#define DEBUG  /* 调试 */

#define CRC_MODE_8  0x00
#define CRC_MODE_16  0x01
#define DEVICE_IS_FIND 0x00 
#define DEVICE_NO_FIND 0x01

#define SPI_IOC_MAGIC 'k'
#define SET_TX_ADDR 	_IO(SPI_IOC_MAGIC,0)
#define SET_RX_ADDR 	_IO(SPI_IOC_MAGIC,1)
#define SET_CHANNEL 	_IO(SPI_IOC_MAGIC,2)
#define SET_CRC_MODE	_IO(SPI_IOC_MAGIC,3)
#define GET_TX_ADDR 	_IO(SPI_IOC_MAGIC,4)
#define GET_RX_ADDR 	_IO(SPI_IOC_MAGIC,5)
#define GET_CHANNEL 	_IO(SPI_IOC_MAGIC,6)
#define GET_CRC_MODE 	_IO(SPI_IOC_MAGIC,7)
#define CHECK_DEVICE 	_IO(SPI_IOC_MAGIC,8)
#define SEND_MESSAGE 	_IO(SPI_IOC_MAGIC,9)
#define RECV_MESSAGE 	_IO(SPI_IOC_MAGIC,10)

#define WRITE_LOSS_BUFF(val) write_data(val)
#define READ_LOSS_BUFF(buf) read_all_data(buf)
#define GET_LOSS_LEN get_buff_loss_len()
#define SAVE_PACK_TO_DESC(p) loss_desc.pack = p
#define GET_SENDING_PACK_DATA loss_desc.pack->data
#define GET_SENDING_PACK_HEAD loss_desc.pack->head 
#define IF_LOSS_EMPTY	if_loss_empty()

typedef unsigned char u8;

struct ctl_data{
	u8 tx_addr[5];
	u8 rx_addr[5];
	u8 channel;
	u8 crc;
};



#endif


