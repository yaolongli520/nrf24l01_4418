#ifndef __CLIENT_H_
#define __CLIENT_H_

#include "system.h"
#include "cmd.h"





class nrf_client {
private:

public:
//	int nrf_write_then_read(void *cmd,long int timeout);
	int nrf_transfer_setpar(struct nrf_cmd &cmd);
	int nrf_transfer_getpar(struct nrf_cmd &cmd);
	int nrf_transfer_file(struct nrf_cmd &cmd);
	int nrf_transfer_cmd(struct nrf_cmd &cmd);
};




#endif


