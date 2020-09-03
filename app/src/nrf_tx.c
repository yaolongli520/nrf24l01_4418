#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "nrf24l01.h"




long get_file_size(FILE *fp)
{
	long st,end;
	fseek(fp,0,SEEK_SET);
	st = ftell(fp);
	fseek(fp,0,SEEK_END);
	end = ftell(fp);
	fseek(fp,0,SEEK_SET);
	return end - st;
}

int main(int argc,char *argv[])
{
	int ret;
	unsigned char *sbuf =NULL; /*发送的数据指针*/
	FILE *fp;
	long f_len;
	
	struct ctl_data cur = {
		{0x01,0x02,0x03,0x04,0x05},//目的地址
		{0x01,0x02,0x03,0x04,0x05},//本机接收地址
		24, //频道
		CRC_MODE_16, //crc模式
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
	
	if(argc < 2 ) {
		printf("TX file is not set \n");	
		return -1;
	} else {
		fp = fopen(argv[1],"r");
		f_len = get_file_size(fp);
		printf("send file :%s f_len:%ld\n",argv[1],f_len);
		if(f_len > 0 ) sbuf = (unsigned char *)malloc(f_len);
		fread(sbuf, f_len, 1, fp);
	}
	printf("start_tx ...\n");
	sleep(1);
	ret = nrf24l01_start_tx(f_len,argv[1]);
	if(ret == -1){
		printf("start_tx is fail\n");
		
	}
	ret =  nrf_send_data(sbuf,f_len);
	

	if(ret) printf("nrf_send_data is fail\n");
	
	if(sbuf) free(sbuf);
	nrf24l01_exit();
	fclose(fp);
	return 0;
}
