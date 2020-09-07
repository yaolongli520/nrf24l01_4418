#ifndef __CMD_H_
#define __CMD_H_
#include "pack.h"


#define NRF_SERVER	0 /*服务器*/	
#define NRF_CLIENT	1 /*客户端*/	

#define CMD_TIMEOUT  500000000  //500MS
#define RETRY_NUM 	 2  //重试次数

#define TRAN_SIZE_MAX	512  //单次最大传输 32*16 字节
#define TRAN_CMD_SIZE	32



#define DATA_LEN		28

enum {
	ERR_NONE = 0, /*没有出错*/
	ERR_SND_CMD, /*发送命令失败*/
	ERR_SND_DATA, /*发送数据失败*/
	ERR_NO_REPLY, /*没有回复数据*/
};


//cmd list 
enum {
	CMD_SETPAR, /* 设置参数 */
	CMD_GETPAR, /* 获取参数 */
	CMD_GET_LOSS, /*获取丢包*/
	CMD_UP_FILE, /* 上传文件 */
	CMD_DOWN_FILE, /* 下载文件 */
	CMD_HEAD_FAIL, /*头错误*/
	CMD_COMPLETION, /*完成*/
	CMD_NODEFINE, /* 未定义 */
};

struct get_par_str{
	u32 index;
	u8  data[24]; 
};

struct nrf_cmd {
	u32 cmd_num;
	u8  *cmd_data;
};









#endif

