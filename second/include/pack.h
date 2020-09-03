#ifndef __PACK_H_
#define __PACK_H_

#include "system.h"



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

void   pack_module_init(void); /* 初始化包模块 */

struct nrf_pack* get_fail_pack(void); /*错误包*/
struct nrf_pack* get_completion_pack(void); /* 完成包 */ 
struct pack* make_new_pack(u8 *buf,u32 len);  /* 新包 */
struct pack* make_null_pack(u32 len); /* 空包 */
struct pack* make_resend_pack(struct pack* np);

void   del_pack(struct pack* pack); /*销毁包*/

void   print_pack(struct pack* pack); /* 打印包 */
enum   pack_type get_pack_type(struct pack* pack); /*获取包类型*/
float get_recv_persen(struct pack *pack); /*获取完成百分比*/
enum receive_status  resolve_pack(struct pack* src,struct pack *result); /* 解析包 发 重发 无头*/
u32 resolve_req_pack(struct pack* src); /* 解析请求重发包 */
struct pack* make_req_pack(void); /* 请求重发包 */
u32 pack_to_len(struct pack* pack);
u32	 pack_to_data(struct pack* pack,u8 *data); /*把包数据到缓存*/



#endif

