#ifndef __CLIENT_H_
#define __CLIENT_H_

#include "system.h"
#include "cmd.h"

class nrf_client {
private:

public:
	void nrf_init_cmd(struct nrf_cmd &cmd)
	{
		cmd.noption = TEXT_AND - cmd.option;
		cmd.nflag 	= TEXT_AND - cmd.flag; 
	}
	int nrf_write_then_read(void *cmd,long int timeout);
	int nrf_transfer_setpar(struct nrf_cmd &cmd);
	int nrf_transfer_getpar(struct nrf_cmd &cmd);
	int nrf_transfer_file(struct nrf_cmd &cmd);
	int nrf_transfer_cmd(struct nrf_cmd &cmd);
};




#endif


