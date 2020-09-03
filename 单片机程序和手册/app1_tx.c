#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "list.h"

#define DEVICE_NAME  "/dev/nrf24l01"

static int fd;
typedef unsigned char u8;
typedef  unsigned short int u16;
typedef  unsigned  int u32;
static u32 to_len = 0;

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define TRAN_LEN_MAX  32 /*发送单次最大字节*/
#define EACH_LEN_MAX  28  /*每个包最多容纳数据个数*/
#define PACK_HEAD_LEN 32 /*包头的长度*/
#define PACK_TOTAL_LEN 32 /*数据包总长*/
#define EACH_LEN_REQ 7  /*每个REQ包的数据个数*/
/*数据容量转换为包占用内存容量 如:28-->32*/
#define DATALEN_TO_PACKLEN(len) (DIV_ROUND_UP(len,EACH_LEN_MAX)*TRAN_LEN_MAX)

//#define DEBUG  /* 调试 */

#define CRC_MODE_8  0x00
#define CRC_MODE_16  0x01
#define DEVICE_IS_FIND 0x00 
#define DEVICE_NO_FIND 0x01

#define SPI_IOC_MAGIC 'k'
#define SET_TX_ADDR 	_IO(SPI_IOC_MAGIC,0)
#define SET_RX_ADDR 	_IO(SPI_IOC_MAGIC,1)
#define SET_CHANNEL 	_IO(SPI_IOC_MAGIC,2)
#define SET_CRC_MODE	_IO(SPI_IOC_MAGIC,3)
#define GET_TX_ADDR 	_IO(SPI_IOC_MAGIC,4)
#define GET_RX_ADDR 	_IO(SPI_IOC_MAGIC,5)
#define GET_CHANNEL 	_IO(SPI_IOC_MAGIC,6)
#define GET_CRC_MODE 	_IO(SPI_IOC_MAGIC,7)
#define CHECK_DEVICE 	_IO(SPI_IOC_MAGIC,8)
#define SEND_MESSAGE 	_IO(SPI_IOC_MAGIC,9)
#define RECV_MESSAGE 	_IO(SPI_IOC_MAGIC,10)

#define WRITE_LOSS_BUFF(val) write_data(val)
#define READ_LOSS_BUFF(buf) read_all_data(buf)
#define GET_LOSS_LEN get_buff_loss_len()
#define SAVE_PACK_TO_DESC(p) loss_desc.pack = p
#define GET_SENDING_PACK_DATA loss_desc.pack->data
#define GET_SENDING_PACK_HEAD loss_desc.pack->head 
#define IF_LOSS_EMPTY	if_loss_empty()

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
	
	read_all_data(buff); /*读取所有丢失的包到buff*/
	p_len = 2 * DIV_ROUND_UP(GET_LOSS_LEN,EACH_LEN_REQ); /*扩增2倍的包个数*/
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
struct pack* make_pack(u8 *buf,u16 length,enum pack_flag flag)
{
	
	struct pack *pack;
	struct pack_head *head;
	struct pack_data *data;
	u8 *p = buf;
	u16 p_len,d_len = length;
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
		float persen; /*完成百分比*/
		int lo_bye;
		head->d_len = loss_buff_to_pack(data);/*设置data*/	
		lo_bye = head->d_len;
		persen = 100 - (100.0*lo_bye) / to_len;
		printf("to_len =%d lo_bye =%d persen=%.2f%%\n",to_len,lo_bye,persen);	
		
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
	
	if(loss) {
		printf("is loss %d message\n",loss);
		ret = E_LOSE;
	}
	
	return ret;
}



int  if_pace_true(const struct pack_head *head,struct pack_data *data)
{
	u32 p_len = head->p_len;
	u32 d_len = head->d_len;
	u32 each_len = (head->flag == SEND)?EACH_LEN_MAX:EACH_LEN_REQ;
	

	if(data->num < 0 || data->num >p_len) return -1;
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
	while(if_pace_true(head,p2)) {
		while(p2->num == p1->num) p2++;
		memcpy(p1+1,p2,sizeof(*p2));
		p1++;
		p2++;
	}
	memset(p1+1,0,sizeof(*p1));
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
	
	ret = check_recv_pack(result);

	
	#ifdef DEBUG
	printf("\n");
	printf("recv totol len =%d \n",*len);
	printf("\n");
	printf("head:%c p_len:%02u d_len:%02u num:%02u  ",
	head->head,head->p_len,head->d_len,head->pack_sum);
	
	printf("FLAG: %s\n",(head->flag==0)?"SEND":(head->flag==1)?
	"RESEND":"REQ_RESEND");
	data  = pack->data;

	for(i = 0;i < p_sum; i++){
		int num,len;
		len = data->length;
		num = data->num;
		if(head->flag == REQ_RESEND && len >7) break;
		printf("len:%02d  num:%02d ==> ",len,num);
		for(i = 0;i < len ; i++)
			if(flag == RESEND ||flag == SEND)
				printf("%c ",buf[num*EACH_LEN_MAX+i]);
			else 	
			{
				u32 *data;
				data = (u32 *)(pack->data[i].data + 2);
				printf("%03d ",data[i]);
			}
		printf("\n");
		data ++;
	}
	printf("\n");
	#endif
	
	return ret;
}


void debug_pack(u8 *buf,int len)
{
	
	int i;
	for(i = 0;i< len ;i++)
	{	
		if(i != 0&&i%30==0) printf("\n");
		printf("%c ",buf[i]);
	}
	printf("\n");
	
}

struct ctl_data{
	u8 tx_addr[5];
	u8 rx_addr[5];
	u8 channel;
	u8 crc;
};

/*计算整个包的长度*/
u32 get_pack_len(struct pack* p)
{
	u32 len = p->head.p_len;
	return (len + 1)*32;
}

#define PACK_LEN(pack) get_pack_len(pack)
#define DLEN  7680 //总共的数据长度

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

#if 0
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
	u8 recv[DATALEN_TO_PACKLEN(len)]; /*回复的消息包缓冲*/
	u8 recv1[DATALEN_TO_PACKLEN(len)]; /*回复的消息缓冲*/
	u32 r_len; 
	np = make_new_pack(buf,len);/*构造新包*/
	sp = np; //第一个发出去的是新包 
	int r_count = -1,t_count = 0; //补发次数 , 计时次数
	int d_len = DATALEN_TO_PACKLEN(len);
	
	while(ret != COMPLETION) /*直到对方返回接收完成*/
	{	
		ret = write(fd,sp,PACK_LEN(sp)); /*写入包*/	
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
				if(t_count%2 == 0 ) 
					printf("was wait 3 seconds, no read anyone data.\n");
			}
		}
		
		/*对方返回头部错误 再次发送*/
		if(rpack->head.flag == HEAD_FAIL )	continue;
		
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
			r_len = ret;
			resolve_pack(rpack,recv1,&r_len); /*解析请求补发包*/
			rp = make_resend_pack(buf,len); /*构造重发的包*/
			sp = rp;
		}
		
	}

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
	printf("recv len = %d\n",len);
	printf("\n");
	for(i = 0; i < len ; i++)
	{
		printf("%c ",buf[i]);
		if((i+1)%30 == 0 && i ) printf("\n");
	}
	printf("\n");
	
}
#endif

int main()
{
	int i ;
	int ret;
	char data = 'A';
	u8 is_find = 0;
	u8 sbuf[DLEN]; /*发送的数据*/
	u32 len;
	struct ctl_data cur = {
		{0x01,0x02,0x03,0x04,0x05},//目的地址
		{0x01,0x02,0x03,0x04,0x05},//本机接收地址
		24, 
		CRC_MODE_16,
	};

	printf("dts addr %02x %02x %02x %02x %02x\n",
	cur.tx_addr[0],cur.tx_addr[1],cur.tx_addr[2],
	cur.tx_addr[3],cur.tx_addr[4]);
	
	printf("cur addr %02x %02x %02x %02x %02x\n",
	cur.rx_addr[0],cur.rx_addr[1],cur.rx_addr[2],
	cur.rx_addr[3],cur.rx_addr[4]);
	

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
	#if 0
	init_data_base();
	
	/*准备发送数据*/
	for(i = 0; i < DLEN; i++)
	{
		sbuf[i] = data;
		data ++;
		if(data > 'Z') data = 'A';	
	}

	ret =  nrf_send_data(sbuf,DLEN);

	if(ret) printf("nrf_send_data is fail\n");
	#endif
	
	
	close(fd);
	return 0;
}