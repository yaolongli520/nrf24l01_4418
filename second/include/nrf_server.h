#ifndef __SERVER_H_
#define __SERVER_H_

#include "system.h"
#include "cmd.h"


void print_op_cmd(struct nrf_cmd &cmd)
{
	printf("cmd.option =%02x\n",cmd.option);
	printf("cmd.noption =%02x\n",cmd.noption);
	printf("cmd.noption =%02x\n",cmd.noption);
	printf("cmd.flag =%02x\n",cmd.flag);
	printf("cmd.nflag =%02x\n",cmd.nflag);
	printf("cmd.index =%d\n",cmd.index);

}

enum {
	TYPE_CMD = 0, /* 命令 */
	TYPE_DATA, /*数据包*/
	TYPE_DATA_HEAD, /*数据包头*/
	TYPE_ERR, /*错误类型*/
	TYPE_NODEFINE, /* 未定义 */
};




/* 包头也是数据包 */
struct nrf_pack{
	u32	type:3; /* TYPE_ */
	u32 len:5;  /*data length,cmd Ignore this field*/
	u32 num:22;	/*data num/cmd num*/
	u32 check:2; /* ~type[1:0] check = (~type & 0x3);*/
	u8  data[EACH_LEN_MAX];
};

/* 数据头部 */
struct data_head{
	u32 	p_len;  /*包的个数 包长度*/
	u32 	d_len; /*包数据总长度*/
	u32 	pack_sum; /* 第几个包 */
	enum pack_type  type; /*类型*/
	u32		total; /*本次传输的总长度 比如1024字节 分2个512的d_len*/
};

#define IS_CMD(pack)  (pack->type==TYPE_CMD)
#define IS_DATA(pack) (pack->type==TYPE_DATA)
#define IS_CHECK_OK(pack) (pack->check==(~pack->type & 0x3))


class nrf_server {
private:

public:
	int nrf_check_type(void *data); /*检查类型*/
	int nrf_handle_pack(void *data); /* 处理包 */
	int nrf_transfer_setpar(struct nrf_cmd &cmd);
	int nrf_transfer_getpar(struct nrf_cmd &cmd);
	int nrf_transfer_file(struct nrf_cmd &cmd);
	int nrf_transfer_cmd(struct nrf_cmd &cmd);
	/*读命令*/
	int nrf_read_cmd(void *cmd,long int timeout);
};





#endif



