#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


#include "nrf_server.h"
#include "mytime.h"
#include "list.h"
#include "nrf24l01.h"
#include "pack.h"

using namespace std;
static nrf_server ops;

/*服务器代码*/
static u32 par[20] = {
0,1,2,3,4,5,6,7,8,9,10,
11,12,13,14,15,16,17,18,19
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

int index_to_data(u32 index,struct nrf_cmd &cmd)
{
	if(index > ARRAY_SIZE(par) - 1) {
		cmd.data32[0] = 0xffff;
		return -1;
	}
	cmd.data32[0] = par[index];	
	return 0;
}

int nrf_server::nrf_transfer_setpar(struct nrf_cmd &cmd)
{
	u32 index;
	index = cmd.index;
	par[index] = cmd.data32[0];
	return 0;
}

int nrf_server::nrf_transfer_getpar(struct nrf_cmd &cmd)
{
	int index;
	int	ret;
	index = cmd.index; /* 获取索引 */
	index_to_data(index,cmd);
	usleep(cmd.udelay);
	ret = nrf24l01_tx((u8 *)&cmd,sizeof(cmd));
	return 0;
}


int nrf_server::nrf_read_cmd(void *cmd,long int timeout)
{
	int ret = 0;
	int num = 0;
	struct timespec t1 = {0, 0},t2 = {0, 0};
	struct timespec time;

retry:	
	clock_gettime(CLOCK_REALTIME, &t1);	
	ret = 0;
	while(ret == 0)
	{
		ret = nrf24l01_rx((u8 *)cmd,TRAN_CMD_SIZE);
		clock_gettime(CLOCK_REALTIME, &t2);
		time = get_time_offset(t1,t2);
		if(get_timeout(time,timeout)) break;
	}
	
	if(ret == 0 && num++ < RETRY_NUM) {
		goto retry;
	}
	else if (ret == 0 && num == RETRY_NUM + 1) {
		cout << __func__ <<" timeout"<<endl;
		return -1;
	}
	
	if(num) cout <<"retry num = "<<num <<endl;
	return 0;
	
}


void print_hex(u8 *buf,int len)
{
	int i = 0;
	for(i = 0; i< len ;i++)
	{
		printf("buf[%d]:%02x ",i,buf[i]);
	}
	printf("\n");
}



void print_file_cmd(struct file_cmd &fcmd)
{
	printf("fcmd.flag =%x \n",fcmd.flag);
	printf("fcmd.nflag =%x \n",fcmd.nflag);
	printf("fcmd.state =%x \n",fcmd.state);
	printf("fcmd.loss =%x \n",fcmd.loss);
	printf("fcmd.udelay =%d \n",fcmd.udelay);
}




int nrf_server::nrf_transfer_file(struct nrf_cmd &cmd)
{
	int ret;
	int dir = cmd.index; /*上传/下载*/
	char *filename;
	static int num =0;
	
	usleep(cmd.udelay);
	printf("num:%d cmd.option =%d \n",num++,cmd.option);
	 //这里返回 有可能对方也在发送模式 导致回复失效 对方继续等回复 这里往下执行 导致后续错误
	nrf24l01_tx((u8 *)&cmd,sizeof(cmd));
	
	
	switch(dir) {
	
	case NRF_UP_LOAD:  /*客户端上传*/
	{
		/* 总长度 发送长度 状态 尝试次数 偏移*/
		u32 t_len = cmd.p8.len, s_len, state, try_num, send_off = 0;
		struct file_cmd sta;
		struct pack* result = make_null_pack(cmd.p8.len); /*result*/
		struct pack* r_buf  = make_null_pack(cmd.p8.len); /*rev buf*/
		
		int ret = 0;
		
		for(try_num = 0; try_num < RETRY_NUM; try_num++)
		{
			ret = nrf_read_cmd((void *)&sta,500*T_MSEC);
			if(!ret) break;
		}
		
		if(ret) {
			cout <<"read time is timeout"<<endl;
			return RET_HEAD_FAIL;
		}else if(cmd_check_file_flag(sta) == NO_FILE_CMD) {
			cout  <<"cmd is not file_cmd "<<endl;
			return RET_HEAD_FAIL;
		}else{
			usleep(sta.udelay);
			sta.state = STA_WAIT_HEAD;	
			nrf24l01_tx((u8 *)&sta,sizeof(sta));//这里返回 有可能对方也在发送模式
		}
		
		/* 进入接收 数据就绪区域 */
		
		

		
	}
		break;
	case NRF_DOWN_LOAD:	
		cout << "NRF_DOWN_LOAD" <<endl;
		break;
	default:
		cout << "no this case "<<endl;
		ret = -1;
	}

	return ret;
}

/* 服务器 接收者*/
int nrf_server::nrf_transfer_cmd(struct nrf_cmd &cmd)
{
	int ret;
	ret = nrf_check_cmd(cmd);	
	if(ret) {
		cout << "cmd is fail ret = "<<ret <<endl;
		if(ret == CMD_FAIL_OPTION) {
			struct file_cmd file;
			memcpy(&file,&cmd,sizeof(file));
			if(cmd_check_file_flag(file) == IS_FILE_CMD)
			{
				
				cout <<"was return STA_WAIT_IDEL"<<endl;
				usleep(file.udelay);
				file.state = STA_WAIT_IDEL;
				nrf24l01_tx((u8 *)&file,sizeof(file));
			}
		}
		return ret;
	}
	switch (cmd.option) {
		
	case FLAG_SEND_SETPAR:
		ret = nrf_transfer_setpar(cmd);
		break;
	case FLAG_SEND_GETPAR:	
		ret = nrf_transfer_getpar(cmd);
		break;
	case FLAG_TRAN_FILE:
		ret = nrf_transfer_file(cmd);
		break;	
	}
	
	return 0;
}


int nrf_up_file(void *data)
{
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	struct data_head *file_desc = (struct data_head *)pack->data;
	
	printf("file_desc->total =%u \n",file_desc->total);
	printf("file_desc->pack_sum =%u \n",file_desc->pack_sum);
	
	
	
	return 0;
}


int nrf_cmd_handle(void *data)
{
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	int cmd = pack->num;
	
	switch(cmd) {
		
	case CMD_SETPAR: /* 设置参数 */
		break;	
	case CMD_GETPAR: /* 获取参数 */
		break;
	case CMD_UP_FILE: /* 上传文件 */
		
		break;
	case CMD_DOWN_FILE: /* 下载文件 */	
	
		break;
		
	}
	
	return 0;
}

int nrf_server::nrf_handle_pack(void *data)
{
	int ret = 0;
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	
	
	switch(pack->type) {
		
	case TYPE_CMD:
		ret = nrf_cmd_handle(data);
		break;
	case TYPE_DATA:	
		
		break;
	case TYPE_DATA_HEAD:
	
		break;
	case TYPE_ERR:	
	
		break;
	case TYPE_NODEFINE:	
	
		break;	
	}
	
	return ret;
}


/**
 * nrf_check_type - 检查接收数据的类型是否合法
 * 
 * Return: 校验失败 返回 TYPE_ERR,否则返回类型,或者未定义类型 TYPE_NODEFINE。
 */
int nrf_server::nrf_check_type(void *data)
{
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	if(!IS_CHECK_OK(pack)) 
		return TYPE_ERR;
	if(pack->type < TYPE_ERR) 
		return pack->type;
	else 
		return TYPE_NODEFINE;
}


int main(int argc,char *argv[])
{

	int ret;
	nrf_server ops;
	int index =0x100;
	
	struct ctl_data cur = {
		{0x01,0x02,0x03,0x04,0x05},//目的地址
		{0x01,0x02,0x03,0x04,0x05},//本机接收地址
		24, 
		CRC_MODE_16,
	};
	
	struct nrf_cmd rec;
	memset(&rec,0,32);
	pack_module_init(); //初始化链式缓存
	ret = nrf24l01_init(cur);
	if(ret){
		cout <<"nrf24l01 init fail"<<endl;
		return -1;
	}

	while(nrf24l01_rx((u8 *)&rec,32)>0); //清空缓存
	if(sizeof(struct nrf_cmd ) >= 32)
		cout <<"warning sizeof cmd = " <<sizeof(struct nrf_cmd )<<endl;
	
	while(1){
		ret = 0;
		while(ret == 0) {
			ret = nrf24l01_rx((u8 *)&rec,32);
		}
		ops.nrf_transfer_cmd(rec);
	}

	return 0;
}
