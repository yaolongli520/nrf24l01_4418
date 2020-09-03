#ifndef __FILE_H_
#define __FILE_H_

/* 线性缓冲 pack[h]-[name]-[len]-[data]*/
struct file{
	char name[50];
	u8 	 data[0];
};




struct file_cmd{
	u16 	flag;  /* F_FLAG_ */
	u16		nflag; /*F_FLAG_AND - flag */
	u32		state;  /* 状态 */
	u32		loss;   /* 丢包*/
	u32     udelay; /*延时返回*/
	u32 receve[4]; /*保留*/
};



int cmd_check_file_flag(struct file_cmd cmd)
{
	if(cmd.flag + cmd.nflag != F_FLAG_AND) { 
		return NO_FILE_CMD;
	}
	return IS_FILE_CMD;
}

/* 令服务器停止接收 */
void cmd_set_file_stop(struct file_cmd &cmd)
{
	cmd.flag   = F_FLAG_STOP;
	cmd.nflag  = F_FLAG_AND - F_FLAG_STOP;
	cmd.state  = 0; /*这里返回状态值*/
	cmd.loss   = 0;
	cmd.udelay	= 500;
}


/*获取状态 返回状态 cmd.state 状态*/
void cmd_get_file_state(struct file_cmd &cmd)
{
	cmd.flag   = F_FLAG_GET;
	cmd.nflag  = F_FLAG_AND - F_FLAG_GET;
	cmd.state  = 1; /*这里返回状态值*/
	cmd.loss   = 0;
	cmd.udelay	= 500;
}

/*获取丢包 返回丢包 cmd.loss 丢包数*/
void cmd_get_file_loss_num(struct file_cmd &cmd)
{
	cmd.flag   = F_FLAG_GET;
	cmd.nflag  = F_FLAG_AND - F_FLAG_GET;
	cmd.state  = 0;
	cmd.loss   = 1; /*这里返回丢包数*/
	cmd.udelay	= 500;
}

/*获取丢包 返回丢包结构体 */
void cmd_get_file_loss(struct file_cmd &cmd)
{
	cmd.flag   = F_FLAG_GET_LOSS;
	cmd.nflag  = F_FLAG_AND - F_FLAG_GET_LOSS;
	cmd.state  = 0;
	cmd.loss   = 0; /*这里返回丢包数*/
	cmd.udelay	= 500;
}



#endif

