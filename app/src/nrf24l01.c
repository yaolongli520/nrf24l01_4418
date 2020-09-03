#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "list.h"
#include "nrf24l01.h"

//typedef unsigned char u8;
typedef  unsigned short int u16;
typedef  unsigned  int u32;
static int fd;
static u32 to_len = 0;



static struct loss_desc loss_desc;

enum pack_flag{
	SEND, /*发*/
	RESEND, /*补发*/
	REQ_RESEND,/*请求补发*/
	HEAD_FAIL, /*头部错误*/
	COMPLETION, /*完整*/
};

static char pack_str[][20]={
	{"SEND"},
	{"RESEND"},
	{"REQ_RESEND"},
	{"HEAD_FAIL"},
	{"COMPLETION"},
};

struct pack_head{
	u32 	head;  /*'head'*/
	u32 	p_len;  /*包的个数 包长度*/
	u32 	d_len; /*数据总长度*/
	u32 	pack_sum; /*第几个*/
	enum pack_flag  flag; /*标志位*/
	u8  receve[12]; /*保留*/
};

struct pack_data{
	u32 num : 22;
	u32 length : 5;
	u32 cpmt : 5;/*补码 EACH_LEN_MAX - length */
	u8 data[EACH_LEN_MAX];/*EACH_LEN_MAX BYTE的数据*/
};

struct pack{
	struct pack_head head;
	struct pack_data data[0];
};

enum pack_status_val{
	E_HEAD, /*头部错误*/
	E_LOSE, /*数据包丢失*/
	S_COMPLE, /*完整*/
};


static struct loss_desc{
	struct pack *pack;
}loss_desc;



int get_buff_loss_len(void)
{
	return get_data_len();
}

/* 
loss 转为包数据 并返回有效长度 
进行倍增处理 防止64字节常丢包导致延误!!!
*/
int loss_buff_to_pack(struct pack_data *data)
{
	int i = 0;
	u32 total,len = get_buff_loss_len();
	u32 buff[len]; /*未完成的*/
	u16 p_len;
	u8  d_len;
	
	p_len = 2 * DIV_ROUND_UP(GET_LOSS_LEN,EACH_LEN_REQ); /*扩增2倍的包个数*/
	read_all_data(buff); /*读取所有丢失的包到buff*/
	total = len;
	
	for(i = 0; i < p_len/2; i++) {
			d_len = (total>EACH_LEN_REQ)?EACH_LEN_REQ:total;
			data[2*i].length = d_len;
			data[2*i].num	 = i;
			data[2*i].cpmt 	 = EACH_LEN_REQ - d_len; 
			memcpy(data[2*i].data,&buff[EACH_LEN_REQ*i],d_len*4);
			data[2*i + 1].length  = d_len;
			data[2*i + 1].num 	  = i;
			data[2*i + 1].cpmt 	  = EACH_LEN_REQ - d_len; 
			memcpy(data[2*i + 1].data,&buff[EACH_LEN_REQ*i],d_len*4);
			total = total - d_len;
	}
	
	return len*2;
}	

int if_loss_empty(void)
{
	int len = get_buff_loss_len();
	
	if(0 == len) return 0;
    else return 1;	
}




/*
构造包
内容 长度 类型
*/
struct pack* make_pack(u8 *buf,u32 length,enum pack_flag flag)
{
	
	struct pack *pack;
	struct pack_head *head;
	struct pack_data *data;
	u8 *p = buf;
	u32 p_len,d_len = length;
	int i;
	static unsigned long int num = 0;
	
	/*设置小包的个数*/
	if(flag == SEND)
		p_len = DIV_ROUND_UP(d_len,EACH_LEN_MAX);
	else if(flag == RESEND) /* 构造重发 */
		p_len = GET_LOSS_LEN + 1; /* +1 增加1个发送数据*/
	else if(flag == REQ_RESEND)
		p_len = 2 * DIV_ROUND_UP(GET_LOSS_LEN,EACH_LEN_REQ);//扩增2倍
	else if(flag == HEAD_FAIL)
		p_len = 0;
	else if(flag == COMPLETION)
		p_len = 0;	
	else {
		p_len = 0;
		printf("parameter error\n");
		return NULL;
	}
	
	/*分配内存*/
	pack = (struct pack *)malloc(sizeof(struct pack)+p_len*sizeof(struct pack_data));/*占用内存长度*/
	if(!pack) {
		printf("malloc is fail %d \n",__LINE__);
		return NULL;
	}
	head = &pack->head;
	data = pack->data;
	
	/*填充包头*/
	head->head = 'h';
	head->p_len = p_len;
	head->d_len = d_len;
	head->pack_sum = num++;
	head->flag = flag;	
		
	/*填充包身*/
	if(flag == SEND){
		for(i = 0; i < p_len; i++){
			data[i].length = (d_len>EACH_LEN_MAX)?EACH_LEN_MAX:d_len;
			data[i].cpmt = EACH_LEN_MAX - data[i].length;
			data[i].num = i;
			memcpy(data[i].data,buf+i*EACH_LEN_MAX,data[i].length);	
			d_len -= data[i].length;
		}
		SAVE_PACK_TO_DESC(pack);/*save pack use to resend*/
	}else if(flag == RESEND){ /* RESEND PACK 作为发送者使用 */
		struct pack_data *send = GET_SENDING_PACK_DATA;
		int lo_len = GET_LOSS_LEN; /*丢失的总长度*/
		int lo_buf[lo_len];
		u32 lo_num;
		READ_LOSS_BUFF(lo_buf);/*读出所有丢失的包*/
		head->d_len = 0;
		if(lo_len == 0 ) printf("warning lo_len =0 :%d\n",__LINE__);
		for(i = 0; i < p_len - 1; i++){ // 对最后的包进行发2次提高,降低补发总次数
			lo_num = lo_buf[i];
			data[i].length 	= send[lo_num].length;
			data[i].num    	= send[lo_num].num;
			data[i].cpmt 	= send[lo_num].cpmt;
			memcpy(data[i].data,send[lo_num].data,send[lo_num].length);
			head->d_len += send[lo_num].length;
		}
		data[i].length 	= data[i - 1].length; 
		data[i].num 	= data[i - 1].num; 
		data[i].cpmt 	= data[i -1].cpmt;
		memcpy(data[i].data,data[i -1].data,data[i].length);
		head->d_len += data[i].length;
	}else if(flag == REQ_RESEND) {
		head->d_len = loss_buff_to_pack(data);/*设置data*/		
	} else if (flag == HEAD_FAIL) {
		head->d_len = 0;
	} else if (flag == COMPLETION) {
		head->d_len = 0;
	}else {
		printf("parameter error ...%d \n",__LINE__);
	}
	
	
	/* 调试打印构建的包 */
	#ifdef DEBUG
	printf("head:%c  p_len:%02u  d_len:%02u  num:%02u  ",
	head->head,head->p_len,head->d_len,
	head->pack_sum);
	printf("FLAG: %s\n",pack_str[head->flag]);
	
	for(i = 0;i< head->p_len; i++)
	{
		int j;
		printf("len:%02d  num:%02d ==> ",data[i].length,
		data[i].num);
		for(j = 0;j< data[i].length; j++)
			if(flag == SEND || flag == RESEND){
				printf("%c ",data[i].data[j]);
			}else //请求重发
			{
				u32 *num;
				num = (u32 *)(data[i].data);
				printf("%03d  ",num[j]);
			}
		printf("\n");	
	}
	
	#endif
	return pack;
} 

#define make_new_pack(buf,len) 		make_pack(buf,len,SEND) /*第一次发送*/
#define make_resend_pack(buf,len) 	make_pack(buf,len,RESEND)    /*用于第二次发送*/
#define make_req_pack(buf,len) 		make_pack(buf,len,REQ_RESEND) /*用于第二次接收*/
#define make_reply_head_fail		make_pack(NULL,0,HEAD_FAIL) /*回复接收数据头部错误*/
#define make_reply_recv_completion	make_pack(NULL,0,COMPLETION) /*回复已接收完成*/


/*销毁包*/
int del_pack(struct pack* pack)
{
	free(pack);
	return 0;
}


/*
检查包是否丢失
并返回包的状态 E_HEAD E_LOSE S_COMPLE 
如果是 E_LOSE 构造请求重发描述
*/
enum pack_status_val check_recv_pack(struct pack *pack)
{
	static int first = 0;
	enum pack_status_val ret = S_COMPLE;
	const struct pack_head *head = &pack->head;
	const struct pack_data *data = pack->data;
	int i, p_len, loss = 0;
	
	p_len = head->p_len;

	for(i = 0; i < p_len; i++) {
		if((data[i].length + data[i].cpmt) != EACH_LEN_MAX){
			write_data(i);
			loss ++;	
		}
	}
	
	if(first == 0) {
		printf("to_len =%d  persen=      ",to_len);
		first++;
	}
	if(loss) {
		float persen; /*完成百分比*/
		int lo_bye;
		lo_bye = EACH_LEN_MAX * loss;/*包个数乘以长度 向整数取*/
		persen = 100 - (100.0*lo_bye) / to_len;
		printf("\b\b\b\b\b\b%5.2f%%",persen);
		ret = E_LOSE;
	}else
		printf("\b\b\b\b\b\b%5.2f%%...",100.00);
	fflush(stdout);
	return ret;
}



int  if_pace_true(const struct pack_head *head,struct pack_data *data)
{
	u32 p_len = head->p_len;
	u32 d_len = head->d_len;
	u32 each_len = (head->flag == SEND || head->flag == RESEND)?EACH_LEN_MAX:EACH_LEN_REQ;
	if(data->num < 0 ) {
		printf("%s:data->num < 0\n",__func__);
		return -1;
	}
	if(data->length > each_len || data->cpmt > each_len) return -1;
	if((data->length + data->cpmt)!= each_len) return -1;
	return 0;
}


/* 去掉重复的包 */
void remove_repeat_pack(struct pack* pack)
{
	const struct pack_head *head = &pack->head;
	struct pack_data *p1 = pack->data;
	struct pack_data *p2 = pack->data + 1;
	if(head->flag != REQ_RESEND) printf("warning %d\n",__LINE__);
	while(!if_pace_true(head,p2)) {
		while(p2->num == p1->num) p2++;
		memcpy(p1+1,p2,sizeof(*p2));
		p1++;
		p2++;
	}
	memset(p1+1,0,sizeof(*p1));
}

/*修复失去头的请求重发包*/
int repair_req_pack(struct pack* pack)
{
	int i;
	u32 p_len = 0,d_len = 0;
	struct pack_head *head = &pack->head;
	struct pack_data *data = pack->data;
	if(head->head == 'h' ) 
		return 0;
	else
		data = (struct pack_data *)pack;
	
	while(1) {
		if(data->length + data->cpmt !=EACH_LEN_REQ) break;
		p_len++;
		d_len += data->length;		
		data++;
	}
	if(p_len == 0 ) return 1;
	data = (struct pack_data *)pack;
	memcpy(&data[p_len],&data[0],PACK_TOTAL_LEN);
	memset(&data[0],0,PACK_TOTAL_LEN);
	head->head ='h';
	head->p_len = p_len;
	head->d_len = d_len;
	head->flag = REQ_RESEND;	
	return 0;
	
}


void debug_reqpack(struct pack* pack)
{
	int p_len;
	int i;
	u32 *p;
	const struct pack_head *head = &pack->head;
	struct pack_data *data = pack->data;
	p_len = head->p_len;
	
	/*失去头部*/
	if(head->head !='h') {
		printf("THIS IS NO HEAD\n");
		p_len = 10000;
		data = (struct pack_data *)pack;
	}
	
	for(i = 0;i < p_len; i++) {
		int j;
		int d_len = data->length;
		p = (u32*)data->data;
		if(data->length + data->cpmt ==EACH_LEN_REQ)
			printf("num:%d d_len:%d x_len:%d==> ",data->num,d_len,data->cpmt);
		else {
		//	printf("data->length:%d data->cpmt=%d\n",data->length,data->cpmt);
			break;
		}	
		for(j = 0;j < d_len ;j++)
			printf("%03d ",p[j]);
		printf("\n");	
		data++;
	}
	
}

void debug_data(struct pack_data *data)
{
	struct pack_data *p = data;
	printf("%s %d .....\n",__func__,__LINE__);
	while(p->length + p->cpmt == EACH_LEN_MAX){
		int i;
		printf("p->num: %d ==> ",p->num);
		for(i = 0;i < p->length ; i++)
			printf("%c ",p->data[i]);
		printf("\n");
		p++;
	}
	printf("%s %d .....\n",__func__,__LINE__);
}

enum pack_status_val  resolve_no_head_pack(struct pack_data* data,struct pack *result)
{
	struct pack_data *p = data;
	while(p->length + p->cpmt == EACH_LEN_MAX){
		int len = p->length;
		memcpy(&result->data[p->num],p,PACK_TOTAL_LEN);
		p++;
	}	
	return check_recv_pack(result);
}

/*
参数的 pack 可以是 一个 数据包 、 补发数据包 、重发请求包
解析包 解出数据 
并返回包的状态 E_HEAD E_LOSE S_COMPLE

*/
enum pack_status_val resolve_pack(struct pack* pack,struct pack *result,u32 *len)
{
	int i,p_len,d_len;
	const struct pack_head *head = &pack->head;
	struct pack_data *data = pack->data;
	enum pack_status_val ret = S_COMPLE;
	enum pack_flag  flag = head->flag;
	int p_sum = 0; /*记录处理的包个数*/
	
	if(head->head != 'h') /*check head!!*/
	{
		printf("pack head is fail %d \n",__LINE__);
		/*head is loss*/
		return E_HEAD;
	}
	

	*len  = 0;
	p_len = head->p_len;
	d_len = head->d_len;

	
	if(flag == SEND) { /*收到的是新包*/
		
		to_len = d_len; /* 新包设置 包总长*/
		memcpy(&result->head,head,PACK_HEAD_LEN); /* 包头 */
		while(!if_pace_true(head,data)) {
			struct pack_data *dts,*src;
			dts = &result->data[data->num];/*目的*/
			src = data;
			memcpy(dts,src,PACK_TOTAL_LEN);
			*len += data->length;
			data++; 
			p_sum ++;
		}	
	}else if(flag == RESEND) { /* 收到的是补发的包 */
		while(!if_pace_true(head,data)){ /*可能也丢包,收到的没有 p_len 个,不能循环这么多*/
			struct pack_data *dts,*src; /*目的 源地址*/
			dts = &result->data[data->num];/*目的*/
			src = data;	
			memcpy(dts,src,PACK_TOTAL_LEN);
			*len += data->length;
			data ++; 
			p_sum ++;
		}	
	}else if(flag == REQ_RESEND){ /*收到的是请求补发包*/
		u32 *d;
		remove_repeat_pack(pack); /*去除重复的包*/
	//	debug_reqpack(pack);
		while(!if_pace_true(head,data)){
			d = (u32*)data->data;
			for(i = 0; i < data->length; i++) {
				write_data(*d);
				d++;
			}
			*len += data->length; 
			data++;
			p_sum++;
		}
	}
	
	if(flag == RESEND || flag == SEND)
	ret = check_recv_pack(result);

	
	#ifdef DEBUG
	printf("\n");
	printf("recv totol len =%d p_sum=%d \n",*len,p_sum);
	printf("\n");
	printf("head:%c p_len:%02u d_len:%02u num:%02u  ",
	head->head,head->p_len,head->d_len,head->pack_sum);
	
	printf("FLAG: %s\n",(head->flag==0)?"SEND":(head->flag==1)?
	"RESEND":"REQ_RESEND");
	data  = pack->data;

	for(i = 0;i < p_sum; i++){
		int j;
		int num,len;
		u32 lobuf[7];
		len = data->length;
		num = data->num;
		if(head->flag == REQ_RESEND && len >EACH_LEN_REQ) break;
		if(flag == REQ_RESEND) memcpy(lobuf,data->data,sizeof(lobuf));
		printf("len:%02d  num:%02d ==> ",len,num);
		for(j = 0;j < len ; j++)
			if(flag == RESEND ||flag == SEND)
				printf("%c ",data->data[j]);
			else {
				printf("%03d ",lobuf[j]);
			}
		printf("\n");
		data ++;
	}
	printf("\n");
	#endif
	
	return ret;
}




/*计算整个包的长度*/
u32 get_pack_len(struct pack* p)
{
	u32 len = p->head.p_len;
	return (len*PACK_TOTAL_LEN + PACK_HEAD_LEN);
}

#define PACK_LEN(pack) get_pack_len(pack)


/* 成功返回0 否则返回其他值*/
int check_reply(struct pack* pack,int len)
{
	const struct pack_head *head = &pack->head;
	struct pack_data *data = pack->data;
	if(head->flag == HEAD_FAIL)  return 0; 	/*这个包是32BYTE*/
	if(head->flag == COMPLETION) return 0;	/*这个包是32BYTE*/
	if(head->flag == REQ_RESEND && len > 32) {
		if(data->length <= 7)	return 0; //防止出现两个都是包头重叠
	}
	  
	return 1;
}

static long file_len;
void pack_to_buff(struct pack *pack,u8 *buf)
{
	int i;
	u32 p_len;
	u8  d_len;
	const struct pack_head *head = &pack->head;
	struct pack_data *data = pack->data;
	p_len = head->p_len;
	printf("p_len =%d\n",p_len);
	file_len = head->d_len; /*记录文件大小*/
	for(i = 0;i < p_len; i++) {
		d_len = data[i].length;
		memcpy(&buf[i*EACH_LEN_MAX],data[i].data,d_len);
	}
}


void pr_hex(void *data,u32 len)
{
	int i;
	u8 *p = (u8 *)data;
	printf("pack: ");
	for(i = 0; i< len ; i++){
		printf("%02x \n",*p++);
	}
	printf("\n");
}


/*
接收包
成功  0 :
完成但超时未回复 :1
失败 -1 :
*/
int nrf_recv_data(u8 *buf,u32 len)
{
	struct pack *rp,*sp = NULL; /*收包指针,回复指针*/
	u8 *pbuf;
	u32 plen = DATALEN_TO_PACKLEN(len) + PACK_HEAD_LEN;/*获取添加包封装后的长度 不能漏了包头32字节*/
	pbuf = malloc(plen); /*read 函数的缓存*/
	struct pack *f_head = make_reply_head_fail;/*头部错误*/
	struct pack *s_com = make_reply_recv_completion; /* 完成 */
	struct pack *recpk = (struct pack *)malloc(plen); /*接收时构建的包缓存*/
	int start = 0;
	int r_count = -1,t_count = 0; //补发次数 , 计时次数
	int ret = 0;
	u32 rlen = 0;
	
	memset(recpk,0,plen); /*清空缓存*/
	
	while(ret != COMPLETION) /*直到完成接收*/
	{
		
		while(1)
		{
			memset(pbuf,0x00,plen); /*清空再收以免遗留上次的*/
			ret = read(fd,pbuf,plen); /* 接收包 */
			if(ret >=32 ){   /*读到消息*/
				start = 1; //第一次接收，启动激活！
				rp = (struct pack *)pbuf;
				if(rp->head.head == 'h') 
					break;/*有头*/
				else if(recpk->head.head == 'h') {
				//	printf("before is has head\n");/*之前已经有头*/
				//	debug_data((struct pack_data *)rp);
					break;
				}
				else {  /* rp 接收的头部不正确 */
					if(sp && sp->head.flag == REQ_RESEND ) del_pack(sp);//防止内存泄露删了sp
					printf("line:%d head is fail  ret:%d head:%c\n ",__LINE__,ret,rp->head.head);
					pr_hex(rp,32);
					sp = f_head;
				}
			} else {
				t_count ++;
				if(start == 1 && t_count%2==0) //未激活时,不写一直空等
				{
					ret = write(fd,sp,PACK_LEN(sp)); /* 3S内未响应继续去请求 */
				//	printf("was wait 3 seconds, resend\n");
				}
				sleep(1);		
			}
		
		}
		
		rlen = ret;	/*记下接收数据长度*/
		//printf("%d rlen =%d\n",__LINE__,rlen);
		if(rp->head.head == 'h')
			ret = resolve_pack(rp,recpk,&rlen);/*解析接收的包*/
		else  ret = resolve_no_head_pack((struct pack_data*)rp,recpk);	/*解析无头包*/

		if(ret == S_COMPLE)  /*接收完整*/
		{
			if(sp && sp->head.flag == REQ_RESEND ) del_pack(sp);
			sp = s_com;
			ret = 0;
			 /* 还需返回接收完整到发送方 */		
			
		} else if (ret == E_HEAD) { /* 头部错误 请求对方重发 */
			//printf("%d head fail \n",__LINE__);
			sp = f_head;
			continue;
		} else if (ret == E_LOSE) { /* 丢包 */
			if(sp && sp->head.flag == REQ_RESEND ) del_pack(sp);
			sp = make_req_pack(buf,0); /*制造请求重发包*/
		}
		
		write(fd,sp,PACK_LEN(sp));
		
		if(ret == 0 ) { /*已经读到完整的包了 等待对方也回复完成*/
			int num = 0;
			while(1){
				sleep(1);
				memset(pbuf,0xff,plen);
				ret = read(fd,pbuf,plen);
				rp = (struct pack *)pbuf;
				if(rp->head.head == 'h' && rp->head.flag == COMPLETION ) 
					break;
				sleep(1);
				write(fd,sp,PACK_LEN(sp));
				num++;
				if(num > 1) { ret = 1; break; }
			}
			
			break; /*退出*/
		}
	}
	
	sleep(1);
	if(sp->head.flag == REQ_RESEND ) del_pack(sp);
	del_pack(f_head);/*删除头错误*/
	del_pack(s_com); /*删除成功*/
	pack_to_buff(recpk,buf);
	free(pbuf);
	free(recpk);
	return ret;
}

/*
发送包
成功  0 :
失败 -1 :
*/
int nrf_send_data(u8 *buf,u32 len)
{
	int ret;
	struct pack *np,*rp,*sp; /*新包指针、重发包指针、发送指针*/
	struct pack* rpack; /*接收包*/
	struct pack *s_com = make_reply_recv_completion;/*完整回复*/
	u8 recv[DATALEN_TO_PACKLEN(len)]; /*read 函数使用*/
	u32 r_len; 
	np = make_new_pack(buf,len);/*构造新包*/
	sp = np; //第一个发出去的是新包 
	int r_count = -1,t_count = 0; //补发次数 , 计时次数
	int d_len = DATALEN_TO_PACKLEN(len);
	u32 w_total = 0; /*总共写入字节数*/
	
	while(ret != COMPLETION) /*直到对方返回接收完成*/
	{	
		ret = write(fd,sp,PACK_LEN(sp)); /*写入包*/	
		w_total +=PACK_LEN(sp);
	//	printf("write %d len....\n",PACK_LEN(sp));
		r_count ++;
		if(ret < PACK_LEN(sp)) {
			ret = -1;
			break;
		}
		
		t_count = 0;
		while(1)
		{
			memset(recv,0xff,d_len); /*清空再收以免遗留上次的*/
			ret = read(fd, recv, d_len); /*读接收方返回的回复消息*/
			if(ret >= 32) {  /*读到回复消息*/
				rpack = (struct pack*)recv;	
				if(rpack->head.head != 'h') {
					if(!repair_req_pack(rpack)) break;/*能被修复*/ 
					printf("%d :line head fail\n",__LINE__);
					continue; /*头部不正确继续读*/
				}
				if(check_reply(rpack,ret)){
					printf("%d :line  only head ret:%d\n",__LINE__,ret);
					continue; /*消息只有头部等*/
				}
				break; /*正确退出*/
				
			} else {
				sleep(1);
				t_count ++;
				if(t_count%1 == 0 ) 
					printf("was wait 2 seconds, no read anyone data.\n");
			}
		}
		
		/*对方返回头部错误 再次发送*/
		if(rpack->head.flag == HEAD_FAIL )	{
			printf("recv is head fail %d\n",__LINE__);
			continue;
		}
		
		/*补发包 对方确认收到后 要删除*/
		if(sp->head.flag == RESEND ) del_pack(sp);
		
		if(rpack->head.flag == COMPLETION )		/*对方返回完整*/
		{
			write(fd,s_com,PACK_LEN(s_com));/*收到完整回复完整*/
			ret = 0;
			break;
		}
		
		if(rpack->head.flag == REQ_RESEND )  /*对方要求继续补发*/
		{
			//printf("REQ_RESEND.......%d\n",__LINE__);
			r_len = ret;
			resolve_pack(rpack,NULL,&r_len); /*解析请求补发包*/
			rp = make_resend_pack(buf,len); /*构造重发的包*/
			sp = rp;
		}
		
	}
	printf("send d_len:%d   w_total:%d\n",d_len,w_total);
	printf("%s use %d sum completion \n",__func__,r_count);
	del_pack(np);//删除新包,新包用于构造重发包 最后才能释放
	del_pack(s_com);
	if(ret == -1 && sp->head.flag == RESEND ) del_pack(sp);
	return ret;
}

/* 打印接收的数据 */
void pr_data(u8 *buf,u32 len)
{
	int i = 0;
	printf("\n \brecv len = %d\n",len);
	printf("\n");
	for(i = 0; i < len ; i++)
	{
		printf("%c ",buf[i]);
		if((i+1)%EACH_LEN_MAX == 0 && i ) printf("\n");
	}
	printf("\n");
	
}


/*


*/
int nrf24l01_init(struct ctl_data cur)
{
	int ret;
	u8 is_find = 0;
	
	fd = open(DEVICE_NAME,O_RDWR);
	if(fd < 0) printf("open %s fail\n",DEVICE_NAME);
	
	ioctl(fd,CHECK_DEVICE,&is_find);
	if(is_find == DEVICE_NO_FIND)
	{	
		printf("device is no find \n"); //检查设备是否存在
		return -1;
	}
	

	ioctl(fd,SET_TX_ADDR,cur.tx_addr);//设置目的地址
	ioctl(fd,SET_RX_ADDR,cur.rx_addr);//设置本机接收地址
	ioctl(fd,SET_CHANNEL,&cur.channel);//设置频道 24 ==2.424GHZ
	ioctl(fd,SET_CRC_MODE,&cur.crc);//设置CRC模式
	
	
	init_data_base(); /*初始化链式缓存区*/
	return 0;
}


void nrf24l01_exit(void)
{
	close(fd);
	
}

struct pack_start{
	u32 	head;  /*s */
	u32  	len; /*数据总长度 包含包头*/
	u8		file_name[23];/*文件名称最长22字节*/
	u8		check; /*c*/
};

int nrf24l01_start_tx(long len,char *name)
{
	int try = 0;
	int ret;
	int finsh = 0;
	
	struct pack_start start ={
		.head = 's',
		.len  = len,
		.check = 'c',	
	};
	struct pack_start recv;
	
	
	memset(start.file_name,0,23);
	strcpy(start.file_name,name);
	while(try < 100){
		int t = 0;
		write(fd,&start,32);
		while(t < 100){
			ret = read(fd,(u8 *)&recv,32);
			if(ret == 32) {
				if(recv.head =='s' && recv.check =='c'){
					finsh = 1;
					break;
				}
			}
			usleep(10000);
			t++;
		}
		if(finsh == 1) break;
		try++;
	}
	if(try == 100 ) return -1;
	return 0;
}

int nrf24l01_start_rx(long *len,char *name)
{
	int ret = 0;
	int try = 0;
	struct pack_start recv;
	while(try < 1000){
		ret = read(fd,(u8 *)&recv,32);
		if(ret == 32) {
			if(recv.head =='s' && recv.check =='c')
				break;
			printf("%s recv fail recv.head=%c \n",__func__,recv.head);
		}
		usleep(2*1000);
		try++;
	}
	if(try == 1000) {
		printf("rx start timeout\n");
		return -1;
	}
	usleep(10*1000);
	write(fd,&recv,32);
	*len = recv.len;
	strcpy(name,recv.file_name);
	printf("rx start .......\n");
	return 0;
	
}


