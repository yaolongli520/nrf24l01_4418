#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "nrf_client.h"
#include "nrf24l01.h"
#include "mytime.h"
#include "list.h"
#include "pack.h"

using namespace std;

static nrf_client ops;



int nrf_client::nrf_transfer_file(struct nrf_cmd &cmd)
{
	struct file *fp = (struct file *)cmd.p8.data;
	int ret = 0;
	nrf_init_cmd(cmd); /*初始化命令*/
	
	while(nrf_write_then_read((void *)&cmd,500*T_MSEC))
		
	if(ret) {
		cout << "server no response"<<endl;
		return ret;
	}
	cout << "cmd is response ok"<<endl;
	usleep(cmd.udelay); /*等待对方恢复接收模式*/
	
	if(cmd.index == NRF_UP_LOAD)  /* 客户端上传 */
	{
		/* 构造包 */
		struct pack* send_pack = make_new_pack((u8 *)fp,cmd.p8.len); 
		
		 /* 总长度 发送长度 状态 尝试次数 偏移*/
		u32 t_len = cmd.p8.len, s_len, state, try_num, send_off = 0;
		u8  *send = (u8 *)send_pack; /* 发送数据 */
		struct file_cmd sta;
		
		/*确定对方初始状态 */
		for(try_num = 0; try_num < RETRY_NUM; try_num++)
		{
			cmd_get_file_state(sta); /* 初始化命令 */
			ret = nrf_write_then_read((void *)&sta,500*T_MSEC); 
			if(ret)  continue; /*未响应*/
			state = sta.state;
			break; 
		}
		
		if(state != STA_WAIT_HEAD) {
			printf("state =%d \n",state);
			return RET_HEAD_FAIL;	
		}
		
		
		/* 完成包含头部的 TRAN_SIZE_MAX 字节*/
		for(try_num = 0; try_num < RETRY_NUM; try_num++)
		{
			if(t_len > TRAN_SIZE_MAX)
				s_len = TRAN_SIZE_MAX;
			else
				s_len = t_len;
			nrf24l01_tx(send,s_len);	
			usleep(SEND_SLEEP_US);	
			cmd_get_file_state(sta); /* 初始化命令 */
			ret = nrf_write_then_read((void *)&sta,2*T_SEC); 
			if(ret)  continue; /*未响应*/
			state = sta.state;
			if(state == STA_WAIT_HEAD) continue;/*状态未变*/
			break; /*其他状态表示可以退出*/
		}
		
		send_off += s_len;
		
		#if 0
		/* 继续发送 */
		if(state == STA_WAIT_DATA)
		for(try_num = 0; try_num < RETRY_NUM; try_num++)
		{
			if(t_len - send_off > TRAN_SIZE_MAX)
				s_len = TRAN_SIZE_MAX;
			else
				s_len = t_len - send_off;
			if(s_len <= 0) break; /*已发完*/
			nrf24l01_tx(send+send_off,s_len);
			usleep(SEND_SLEEP_US);	
			cmd_get_file_state(sta); /* 初始化命令 */
			ret = nrf_write_then_read((void *)&sta,2*T_SEC);
			if(ret)  continue; /*未响应*/
			try_num = 0; /* 响应后清除 */
			send_off += s_len; /*增加已发*/
			state = sta.state;			
		}

		/* 补发 */
		if(state == STA_WAIT_DATA)
		for(try_num = 0; try_num < RETRY_NUM; try_num++)
		{
			int loss_len;
			cmd_get_file_loss_num(sta); /*获取丢包数*/
			ret = nrf_write_then_read((void *)&sta,500*T_MSEC);
			if(ret)  continue; /*未响应*/
			try_num = 0; 
			loss_len = sta.loss;
			cout << "sta.loss = "<<sta.loss<<endl;
		}
		/* 结束发送 */
		for(try_num = 0; try_num < RETRY_NUM; try_num++)
		{
			cmd_set_file_stop(sta);
			ret = nrf_write_then_read((void *)&sta,500*T_MSEC);
			if(ret)  continue; /*未响应*/
			try_num = 0; 
		}
		#endif
		 
	} else if(cmd.index == NRF_DOWN_LOAD) /* 客户端下载 */
	{	

		
		
		
	}else {
		cout <<"not this cmd.index" <<endl;
		ret = -1;
	}

	
	
	return 0;


	
}

int nrf_client::nrf_transfer_setpar(struct nrf_cmd &cmd)
{
	nrf_init_cmd(cmd);
	nrf24l01_tx((u8 *)&cmd,sizeof(cmd));
	return 0;
}

int nrf_client::nrf_transfer_getpar(struct nrf_cmd &cmd)
{
	int ret;
	nrf_init_cmd(cmd);
	ret = nrf_write_then_read((void *)&cmd,500*T_MSEC);
	return ret;
}


/*客户端 发送者*/
int nrf_client::nrf_transfer_cmd(struct nrf_cmd &cmd)
{
	int ret;
	
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

/*
 * nrf_write_then_read 
 * 用于发送1个命令同时获取回复。
 * @param: cmd, 发送的命令
 * @param: timeout, 等待回复的超时时间
 * @return: 正常返回0
 */ 
int nrf_client::nrf_write_then_read(void *cmd,long int timeout)
{
	int ret = 0;
	int num = 0;
	struct timespec t1 = {0, 0},t2 = {0, 0};
	u8	recv[TRAN_CMD_SIZE] = {0};
	struct timespec time;

retry:	
	nrf24l01_tx((u8 *)cmd,TRAN_CMD_SIZE);
	clock_gettime(CLOCK_REALTIME, &t1);	
	ret = 0;
	while(ret == 0)
	{
		ret = nrf24l01_rx(recv,TRAN_CMD_SIZE);
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
	memcpy((u8 *)cmd,recv,TRAN_CMD_SIZE);
	if(num) cout <<"retry num = "<<num <<endl;
	return 0;
}



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

/*设置参数 测试*/
void set_par_test(char *argv[])
{
	struct nrf_cmd cmd ={
	.option = FLAG_SEND_SETPAR,
	.noption = 0,
	.flag =  0,
	.nflag = 0,
	.udelay = 6000,
	.index	= (u32)atoi(argv[2]),
	};
	
	cmd.data32[0] = atoi(argv[3]);
	ops.nrf_transfer_cmd(cmd);
}

/*获取参数 测试*/
void get_par_test(char *argv[])
{
	struct nrf_cmd cmd ={
	.option = FLAG_SEND_GETPAR,
	.noption = 0,
	.flag =  FLAG_REPLY,
	.nflag = 0,
	.udelay = 6000,
	.index	= (u32)atoi(argv[2]),
	};
	
	ops.nrf_transfer_cmd(cmd);
	printf("get par = %u \n",cmd.data32[0]);
}

/*上传/下载 文件 测试*/
void tran_file_test(char *argv[])
{
	int len = 1000;
	struct file *fp = NULL;
	struct nrf_cmd cmd ={
	.option = FLAG_TRAN_FILE, 
	.noption = 0,
	.flag =  FLAG_REPLY,
	.nflag = 0,
	.udelay = 7000,
	};
	if(!strcmp(argv[1],"up")){
		cmd.index	= NRF_UP_LOAD;
		fp = (struct file *)malloc(sizeof(struct file) + len);/*文件名 + 文件数据*/
		memset(fp->data,'A',len);
		cmd.p8.len = sizeof(struct file) + len ;
	} 
	else if(!strcmp(argv[1],"down"))
	{
		 cmd.index	= NRF_DOWN_LOAD;
		 fp = (struct file *)malloc(sizeof(struct file));/*文件名*/
		 cmd.p8.len = 0;
	}else cmd.index	= 0xff;
	
	cmd.p8.data = (u8 *)fp;
	memset(fp->name,0,sizeof(struct file));
	memcpy(fp->name,argv[2],strlen(argv[2])+1);
	ops.nrf_transfer_cmd(cmd);
	if(fp) free(fp);
}




int main(int argc,char *argv[])
{
	int ret;
	u8 buff[10000] ={0x00};
	
	struct ctl_data cur = {
		{0x01,0x02,0x03,0x04,0x05},//目的地址
		{0x01,0x02,0x03,0x04,0x05},//本机接收地址
		24, //频道
		CRC_MODE_16, //crc模式
	};
	
	init_data_base(); //初始化链式缓存
	ret = nrf24l01_init(cur);
	if(ret){
		cout <<"nrf24l01 init fail\n";
		return -1;
	}
	

	if(!strcmp(argv[1],"get")){
		get_par_test(argv);
	}
	else if(!strcmp(argv[1],"set")){
		set_par_test(argv);
	}
	else if(!strcmp(argv[1],"up")){
		cout << "up file: " <<argv[2]<<endl;
		tran_file_test(argv);
	}
	else if(!strcmp(argv[1],"down")){
		cout << "down file: " <<argv[2]<<endl;
		tran_file_test(argv);
	}else cout << "this cmd is not"<<endl;

	return 0;
}