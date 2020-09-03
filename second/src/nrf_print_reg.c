#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DEVICE_NAME  "/dev/nrf24l01"

typedef unsigned char u8;


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
#define DEBUG_REG   	_IO(SPI_IOC_MAGIC,12)

struct ctl_data{
	u8 tx_addr[5];
	u8 rx_addr[5];
	u8 channel;
	u8 crc;
};


int main()
{
	int fd;

	fd = open(DEVICE_NAME,O_RDWR);
	if(fd < 0) {
		printf("open %s fail\n",DEVICE_NAME);
		return -1;
	}
	ioctl(fd,DEBUG_REG,NULL);//设置目的地址
	
	close(fd);
	return 0;
}