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
#include "file.h"

using namespace std;
static nrf_server ops;

/*服务器参数*/
static u32 par[20] = {
0,1,2,3,4,5,6,7,8,9,10,
11,12,13,14,15,16,17,18,19
};

int index_to_data(u32 index,u8 *data)
{
	if(index > ARRAY_SIZE(par) - 1) {
		memset(data,0xff,4);
		return -1;
	}
	memcpy(data,&par[index],sizeof(par[index]));
	return 0;
}






#if 0



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
#endif

static struct file *file_buff = NULL; /*文件缓存*/
static u32 file_total = 0; /*文件总长度*/
static u32 recv_total = 0; /*接收总长度*/
struct nrf_pack *loss_pack = NULL; /*记录丢包*/
struct nrf_pack *loss_pack_off = NULL; /*记录完成偏移*/
struct timespec str_time; /*传输开始*/
struct timespec end_time; /*传输结束*/




/**
 * create_up_file_buffer - 创建存储文件的缓存区,和包丢失缓冲区
 * 
 * Return:
 */

int create_up_file_buffer(void *data)
{
	int ret = FILE_PROCESS_OK;
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	struct data_head *file_desc = (struct data_head *)pack->data;

	if(file_buff){
		ret = FILE_ALRE_ALLOC_BUFF;
		free(file_buff);
	}

	if(loss_pack){
		ret = FILE_ALRE_ALLOC_LOSS;
		free(loss_pack);
	}

	if(file_desc->d_len) {
		file_buff  = (struct file *)malloc(file_desc->d_len);
		loss_pack  = make_pack(file_desc->d_len);
		file_total = file_desc->d_len;
		recv_total = 0; /*总长度*/
		clock_gettime(CLOCK_REALTIME, &str_time);	
	}
	
	return ret;
}


/**
 * send_loss_pack_to_client - 从loss_pack 筛选出含丢失包的部分返回给从机
 * 若果没有丢失 返回完整包！！
 * Return:
 */
int  send_loss_pack_to_client(void)
{
	int sum = 0;
	struct nrf_pack *pack;
	if(loss_pack_off) 
		pack = loss_pack_off; /*如果有则从这个位置开始*/
	else
		pack = loss_pack; /*重来开始*/
	
	if(!pack)
		return ERR_SND_DATA;
	while(IS_CHECK_OK(pack) && (sum<16)) {
		for(int i = 0; i < EACH_LEN_REQ; i++) 
			if(pack->req[i]!=0xffffffff) { /*只要有一个未完成*/	
				if(sum == 0)  /*这里开始发现丢包 记录这个开始位置*/
					loss_pack_off = pack; 
				nrf24l01_tx((u8 *)pack,TRAN_LEN_MAX);
				sum++; /*回复计数*/
				break;
		   }
			pack++;	
	}
	
	return ERR_NONE;
}



void print_hex(u8 *buf,int len)
{
	int i = 0;
	for(i = 0; i< len ;i++)
	{
		printf("buf[%d]:%02x \n",i,buf[i]);
	}
	printf("\n");
}


int nrf_transfer_getpar(void *data)
{
	int	ret;
	u32 val = 0;
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	struct get_par_str *cur_par = (struct get_par_str *)pack->data;
	int index = cur_par->index;

	ret = index_to_data(index,cur_par->data);
	
	if(ret) 
		cout << "index =" <<index <<"no find "<<endl;
	memcpy(&val,cur_par->data,sizeof(val));
	return 0;
}

int nrf_transfer_setpar(void *data)
{
	int	ret;
	u32 val = 0;
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	struct get_par_str *cur_par = (struct get_par_str *)pack->data;
	int index = cur_par->index;
	
	memcpy(&val,cur_par->data,sizeof(val));
	if(index < ARRAY_SIZE(par))
		par[index] = val;
	else
		memset(cur_par->data,0xff,sizeof(val));
	return 0;
}



/**
 * nrf_cmd_handle - 对接收到的命令进行处理和回复
 * 
 * Return:
 */

int nrf_cmd_handle(void *data)
{
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	int cmd = pack->num;
	
	switch(cmd) {
		
	case CMD_SETPAR: /* 设置参数 */
		nrf_transfer_setpar(data);
		nrf24l01_tx((u8 *)data,TRAN_LEN_MAX);
		/* 设置并回复 */
		break;	
	case CMD_GETPAR: /* 获取参数 */
		nrf_transfer_getpar(data);
		nrf24l01_tx((u8 *)data,TRAN_LEN_MAX);
		/* 获取参数并回复 */
		break;
	case CMD_GET_LOSS: /* 获取丢包 */
		nrf24l01_tx((u8 *)data,TRAN_LEN_MAX); /*先回复再返回丢包*/
		send_loss_pack_to_client();
		break;
	case CMD_UP_FILE: /* 上传文件 */
		/* 创建文件缓存区并回复 */
		create_up_file_buffer(data);
		usleep(1000);
		nrf24l01_tx((u8 *)data,TRAN_LEN_MAX);
		break;
	case CMD_DOWN_FILE: /* 下载文件 */	
		/*创建文件数据并回复*/
	
		break;
		
	}
	
	return 0;
}


#define BIT_SET_OK  0
#define BIT_IS_SET  1


/**
 * set_already_recv_bit - 对接收到的包位进行设置
 * 
 * Return:
 */

bool set_already_recv_bit(u32 num)
{
	int pack_num = num/EACH_LEN_LOSS; /*包偏移 0~223 = 0 */
	int pack_bit_offset = num%EACH_LEN_LOSS; /*当前包的位偏移*/
	int pack_word_offset = pack_bit_offset/32; /*第几个字*/
	int pack_word_bit = pack_bit_offset%32; /*该字的第几个位*/
	if(loss_pack[pack_num].req[pack_word_offset] & (0x1)<<pack_word_bit)
		return BIT_IS_SET;
	loss_pack[pack_num].req[pack_word_offset] |=(0x1)<<pack_word_bit;
	return BIT_SET_OK;
}



/**
 * nrf_data_handle - 对接收到的包进行处理
 * 
 * Return:
 */

int nrf_data_handle(void *data)
{
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	u8 *dts = (u8 *)file_buff + EACH_LEN_MAX * pack->num;

	if(!file_buff) 
		return FILE_BUF_NODEFIDE; /*缓存未分配*/
	
	if(pack->type == TYPE_DATA) { /*次数大于总次数??*/
		bool ret;
		ret = set_already_recv_bit(pack->num); /*设置接收完成位*/
		if(ret == BIT_SET_OK) {
			memcpy(dts, pack->data, sizeof(u8) * pack->len);	
			recv_total += pack->len;
		}
	}
	else if(pack->type == TYPE_REQ_DATA) {
		memcpy(dts, pack->data, sizeof(u32) * pack->len);
		set_already_recv_bit(pack->num); /*设置接收完成位*/
	}
	else 
		cout <<"Unidentified type packet !!"<<endl;
	
	return FILE_PROCESS_OK;
}

void print_err(struct nrf_pack *pack)
{
	printf("pack->type:0x %x\n",pack->type);
	printf("pack->len:%d \n",pack->len);
	printf("pack->num:%d \n",pack->num);
	printf("pack->check:0x %x \n",pack->check);
}

/**
 * nrf_handle_pack - 对接收到的包进行处理
 * 
 * Return:
 */

int nrf_server::nrf_handle_pack(void *data)
{
	int ret = 0;
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;

	ret = nrf_check_type(data);
	if(ret >= TYPE_ERR) {
	//	cout <<"nrf_check_type data is fail"<<endl;
	//	print_err(pack);
		return ret;
	}
	
	switch(pack->type) {
		
	case TYPE_CMD:
		ret = nrf_cmd_handle(data);
		break;
	case TYPE_DATA:	
		ret = nrf_data_handle(data);
		break;
	case TYPE_REQ_DATA:
	
		break;
	case TYPE_ERR:	
	
		break;
	case TYPE_NODEFINE:	
	
		break;	
	case TYPE_SHOW_LOSS:
		print_nrf_pack(loss_pack);
		printf("recv_total = %u \n",recv_total);
		break;
	}
	
	return ret;
}



/**
 * nrf_server_process - 服务器进程 对接收指令和数据进行处理
 * 
 * Return:
 */

int nrf_server::nrf_server_process(void *data,u32 len)
{
	int ret = 0;
	int num = DIV_ROUND_UP(len,TRAN_LEN_MAX);
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	for(int i = 0; i < num; i++ ) {
		ret = nrf_handle_pack(&pack[i]);
	}	
	return ret;
}



/**
 * recv_comp_process - 文件接收完成处理
 * 
 * Return:
 */
int nrf_server::recv_comp_process(void)
{
	static u32 recving = 0;
	if(file_total == 0) 
		return FILE_UP_NULL; /*未有文件*/
	
	if(recv_total < file_total) {
		
		float persen; /*完成百分比*/
		if(recving == 0)
		printf("to_len =%d  persen=       ",file_total);
		recving = 1;
		persen = (100.0*recv_total)/file_total;
		if(persen >= 99.99) persen = 99.99; //防止非常接近100
		printf("\b\b\b\b\b\b%5.2f%%",persen); 	
		fflush(stdout);	
		
		return FILE_UP_RECVING;
	}
	
	if(recv_total > file_total) 
		return FILE_UP_FAIL;

	if(recv_total == file_total) {
		char file_name[50] = {0};
		FILE *fp;
		strcpy(file_name,file_buff->name);
		fp = fopen(file_buff->name,"w+");
		if(fp==NULL) {
			return FILE_UP_SAVE_FAIL;
		}
		fwrite(file_buff->data,file_total - 50,1,fp);
		recv_total = 0;
		file_total = 0;
		free(file_buff);
		free(loss_pack);
		file_buff = NULL;
		loss_pack = NULL;
		loss_pack_off = NULL; /*恢复*/
		fclose(fp);
	}
	printf("\b\b\b\b\b\b\b%6.2f%%......",100.00);
	fflush(stdout);	
	clock_gettime(CLOCK_REALTIME, &end_time);	
	struct timespec time = get_time_offset(str_time,end_time);
	printf(" %u.%u s \n",time.tv_sec,time.tv_nsec/1000000);
	
	recving = 0;
	return FILE_UP_COMP;
	
}



int main(int argc,char *argv[])
{

	int ret;
	nrf_server ops;
	int index = 0x100;
	u8 recv[(ONE_PROCESS_NUM + 1)*TRAN_LEN_MAX] = {0}; /*多一个空保留空间*/
	
	struct ctl_data cur = {
		{0x01,0x02,0x03,0x04,0x05},//目的地址
		{0x01,0x02,0x03,0x04,0x05},//本机接收地址
		24, 
		CRC_MODE_16,
	};
		
	ret = nrf24l01_init(cur);
	if(ret){
		cout <<"nrf24l01 init fail"<<endl;
		return -1;
	}
	
	pack_module_init(); //初始化链式缓存
	
	while(nrf24l01_rx((u8 *)&recv,32)>0); //清空缓存
	cout <<"server start"<<endl;
	if(sizeof(struct nrf_pack ) != 32)
		cout <<"warning sizeof cmd = " <<sizeof(struct nrf_pack )<<endl;
	
	while(1){
		ret = 0;
		while(ret == 0) {
			memset(recv,0,sizeof(recv));
			ret = nrf24l01_rx((u8 *)recv,sizeof(recv) - TRAN_LEN_MAX);
		}
		ops.nrf_server_process(recv,ret);
		ops.recv_comp_process();
	}

	return 0;
}
