#ifndef __SERVER_H_
#define __SERVER_H_

#include "system.h"
#include "cmd.h"



#define ONE_PROCESS_NUM  16







class nrf_server {
private:

public:
	int recv_comp_process(void);
	int nrf_server_process(void *data,u32 len);
	int nrf_handle_pack(void *data); /* 处理包 */
	int nrf_transfer_setpar(struct nrf_cmd &cmd);
	int nrf_transfer_getpar(struct nrf_cmd &cmd);
	int nrf_transfer_file(struct nrf_cmd &cmd);
	int nrf_transfer_cmd(struct nrf_cmd &cmd);
	/*读命令*/
	int nrf_read_cmd(void *cmd,long int timeout);
};





#endif



