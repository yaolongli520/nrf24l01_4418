#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "nrf24l01.h"


#define DEVICE_NAME  "/dev/nrf24l01"

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
	
#define DEVICE_IS_FIND 0x00 
#define DEVICE_NO_FIND 0x01
	
class NRF_OPS{
private:	
	int fd;
public:	
	int  init(struct ctl_data &cur);
	int  tx(u8 *buf,u32 count);
	int  rx(u8 *buf,u32 count);
	int  find_dev(void);
	~NRF_OPS()
	{
		close(fd);
	}
};

int NRF_OPS::init(struct ctl_data &cur)
{
	
	fd = open(DEVICE_NAME,O_RDWR);
	if(fd < 0) {
		printf("open is fail\n");
		return NRF_FAIL_NODEV;
	}

	if(find_dev()) 
		return NRF_FAIL_NOFIND;

	ioctl(fd,SET_TX_ADDR,cur.tx_addr);//设置目的地址
	ioctl(fd,SET_RX_ADDR,cur.rx_addr);//设置本机接收地址
	ioctl(fd,SET_CHANNEL,&cur.channel);//设置频道 24 ==2.424GHZ
	ioctl(fd,SET_CRC_MODE,&cur.crc);//设置CRC模式	
	return NRF_OK;
}

int NRF_OPS::tx(u8 *buf,u32 count)
{
	int ret;
	ret = write(fd, buf, count);
	return ret;
}

int NRF_OPS::rx(u8 *buf,u32 count)
{
	return  read(fd,buf,count);	
}

int NRF_OPS::find_dev(void)
{
	u8 is_find = 0;
	ioctl(fd,CHECK_DEVICE,&is_find);
	if(is_find == DEVICE_NO_FIND) {	
		printf("device is no find \n"); //检查设备是否存在
		return -1;
	}
	return 0;
}



static NRF_OPS ops; /*构造类*/


int nrf24l01_init(struct ctl_data &cur)
{
	int ret;
	ret = ops.init(cur);
	if(ret) 
		cout <<__func__<< "fail"<<endl;
	return ret;
}


int nrf24l01_tx(u8 *buf,u32 count)
{
	int ret;
	ret = ops.tx(buf,count);
	if(ret == 0) {
		cout << __func__<<"fail"<<endl;
		return NRF_FAIL_TX;
	}
	return NRF_OK;
}

u32 nrf24l01_rx(u8 *buf,u32 count)
{
	int ret;
	u32 len = count;
	count = ops.rx(buf,len);
	return count;
}

