#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "nrf24l01.h"

#define DLEN  100000 //总共的数据长度

int main(int argc,char *argv[])
{
	int ret;
	FILE *fp;
	long f_len;
	char file_name[30] ={0};
	struct ctl_data cur = {
		{0x01,0x02,0x03,0x04,0x05},//目的地址
		{0x01,0x02,0x03,0x04,0x05},//本机接收地址
		24, 
		CRC_MODE_16,
	};


	printf("dts addr %02x %02x %02x %02x %02x\n",
	cur.tx_addr[0],cur.tx_addr[1],cur.tx_addr[2],
	cur.tx_addr[3],cur.tx_addr[4]);
	
	printf("cur addr %02x %02x %02x %02x %02x\n",
	cur.rx_addr[0],cur.rx_addr[1],cur.rx_addr[2],
	cur.rx_addr[3],cur.rx_addr[4]);

	ret =  nrf24l01_init(cur);
	if(ret == -1) {
		printf("nrf24l01 is not find \n");
		return -1;
	}



	ret =  nrf24l01_start_rx(&f_len,file_name);
	if(ret ){
		printf("rx is timeout \n");
		return -1;
	}
	printf("file name =%s f_len =%ld \n",file_name,f_len);
	u8 rbuf[f_len];
	memset(rbuf,'#',f_len);
	ret = nrf_recv_data(rbuf,f_len);
	
	fp = fopen(file_name,"w");
	fwrite(rbuf,f_len,1,fp);
	
	if(ret == -1 ) printf("nrf_recv_data is fail\n");
	if(ret == 1 ) printf("nrf_recv_data is com buf time out\n");
	
	//pr_data(rbuf,DLEN);

	nrf24l01_exit();
	fclose(fp);
	return 0;
}
