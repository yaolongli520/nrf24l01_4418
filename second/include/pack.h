#ifndef __PACK_H_
#define __PACK_H_

#include "system.h"

/* n 除以 d 向上取整*/
#define TRAN_LEN_MAX  32 /*发送单次最大字节*/
#define EACH_LEN_MAX  28  /*每个包最多容纳数据个数*/
#define PACK_HEAD_LEN 32 /*包头的长度*/
#define PACK_TOTAL_LEN 32 /*数据包总长*/
#define EACH_LEN_REQ 7  /*每个REQ包的数据个数*/
#define EACH_LEN_LOSS 224 /*每个REQ包保存包个数*/
#define EACH_REQ_BIT  32 /*每个REQ变量占用位*/

/*数据容量转换为包占用内存容量 如:28-->32*/
#define DATALEN_TO_PACKLEN(len) (DIV_ROUND_UP(len,EACH_LEN_MAX)*TRAN_LEN_MAX)

/* 小于8 */
enum {
	TYPE_CMD = 0, /* 命令 */
	TYPE_DATA, /*数据包*/
	TYPE_REQ_DATA, /*请求数据*/
	TYPE_SHOW_LOSS, /*显示丢包结构*/
	TYPE_ERR, /*错误类型*/
	TYPE_NODEFINE, /* 未定义 */
};

extern const char *type_name[]; /*对应名称*/


#define BITS_TYPE 	3
#define BITS_LEN 	5
#define BITS_NUM 	22
#define BITS_CHECK 	2



/* 包头也是数据包 */
struct nrf_pack{
	u32	type:BITS_TYPE; /* TYPE_ */
	u32 len:BITS_LEN;  /*data length,cmd Ignore this field*/
	u32 num:BITS_NUM;	/*data num/cmd num CMD_*/
	u32 check:BITS_CHECK; /* ~type[1:0] check = (~type & 0x3);*/
	union {
		u8  data[EACH_LEN_MAX];
		u32 req[EACH_LEN_REQ];
	};
};

/* 数据头部 */
struct data_head{
	u32 	p_len;  /*包的个数 包长度*/
	u32 	d_len; /*包数据总长度*/
	u32 	pack_sum; /* 第几个包 */
	u32  	type; /*类型*/
};


/* 包标志 */
enum pack_type{
	SEND, /*发*/
	RECV, /*收*/
	RESEND, /*补发*/
	REQ_RESEND,/*请求补发*/
	HEAD_FAIL, /*头部错误*/
	COMPLETION, /*完整*/
	UNDEFINED, /* 未定义 */
};

/* 当前接收状态 */
enum receive_status{
	S_IDEL,	/*空闲状态*/
	E_HEAD, /*头部错误*/
	E_LOSE, /*数据包丢失*/
	S_COMPLE, /*完整*/
};

#define IS_CMD(pack)  (pack->type==TYPE_CMD)
#define IS_DATA(pack) (pack->type==TYPE_DATA)
#define IS_CHECK_OK(pack) (pack->check==(~pack->type & 0x3))


extern u32 pack_sum;


void   pack_module_init(void); /* 初始化包模块 */
struct nrf_pack* get_fail_pack(void); /*错误包*/
struct nrf_pack* get_completion_pack(void); /* 完成包 */ 
int nrf_check_type(void *data); /*检查类型*/

u32 data_to_pack_packlen(u32 lenght);
u32 data_to_pack_datalen(u32 lenght);

struct nrf_pack* make_pack(u8 *buf,u32 length); /*发送*/
struct nrf_pack* make_pack(u32 length); /*请求重发*/
struct nrf_pack* make_pack(struct nrf_pack *snd_pack,struct nrf_pack *lo_pack); /*补发*/

void print_nrf_pack(struct nrf_pack* pack);

void del_pack(struct nrf_pack* pack);

#if 0
struct pack* make_new_pack(u8 *buf,u32 len);  /* 新包 */
struct pack* make_null_pack(u32 len); /* 空包 */
struct pack* make_resend_pack(struct pack* np);



void   print_pack(struct pack* pack); /* 打印包 */
enum   pack_type get_pack_type(struct pack* pack); /*获取包类型*/
float get_recv_persen(struct pack *pack); /*获取完成百分比*/
enum receive_status  resolve_pack(struct pack* src,struct pack *result); /* 解析包 发 重发 无头*/
u32 resolve_req_pack(struct pack* src); /* 解析请求重发包 */
struct pack* make_req_pack(void); /* 请求重发包 */
u32 pack_to_len(struct pack* pack);
u32	 pack_to_data(struct pack* pack,u8 *data); /*把包数据到缓存*/

#endif


#endif

