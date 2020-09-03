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



//cmd list 
enum {
	CMD_SETPAR, /* 设置参数 */
	CMD_GETPAR, /* 获取参数 */
	CMD_UP_FILE, /* 上传文件 */
	CMD_DOWN_FILE, /* 下载文件 */
	CMD_NODEFINE, /* 未定义 */
}cmd_list;













#endif

