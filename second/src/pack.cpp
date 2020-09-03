#include <stdlib.h>
#include <string.h>
#include "pack.h"




/* n 除以 d 向上取整*/
#define TRAN_LEN_MAX  32 /*发送单次最大字节*/
#define EACH_LEN_MAX  28  /*每个包最多容纳数据个数*/
#define PACK_HEAD_LEN 32 /*包头的长度*/
#define PACK_TOTAL_LEN 32 /*数据包总长*/
#define EACH_LEN_REQ 7  /*每个REQ包的数据个数*/
/*数据容量转换为包占用内存容量 如:28-->32*/
#define DATALEN_TO_PACKLEN(len) (DIV_ROUND_UP(len,EACH_LEN_MAX)*TRAN_LEN_MAX)

int  if_pace_true(const struct pack_head *head,struct pack_data *data);








void pack_module_init(void)
{
	init_data_base(); //初始化链式缓存
}

int get_buff_loss_len(void)
{
	return get_data_len();
}

static struct nrf_pack head_fail; /* 头错误 */
static struct nrf_pack head_com; /* 完成 */

struct nrf_pack* init_fail_pack(void)
{
	struct data_head head = {
		.p_len = 0,
		.d_len = 0,
		.pack_sum = 0,
		.type	= HEAD_FAIL,
		.total  = 0, 	
	};
	head_fail.type = TYPE_DATA;
	head_fail.len  = 0;
	head_fail.num  = 0;
	head_fail.check = (~head_fail.type & 0x3);
	memcpy(head_fail.data,&head,sizeof(head));
	return &head_fail;
}

/**
 * get_fail_pack - 通过获取 head_fail 结构体的地址并返回
 * 
 *
 * Return: 返回一个头部错误的回复包结构
 */
struct nrf_pack* get_fail_pack(void)
{
	return &head_fail;
}

struct nrf_pack* init_completion_pack(void)
{
	struct data_head head = {
		.p_len = 0,
		.d_len = 0,
		.pack_sum = 0,
		.type	= COMPLETION,
		.total  = 0, 	
	};
	head_com.type = TYPE_DATA;
	head_com.len  = 0;
	head_com.num  = 0;
	head_com.check = (~head_com.type & 0x3);
	memcpy(head_com.data,&head,sizeof(head));
	return &head_com;	
}



/**
 * get_completion_pack - 通过获取 head_com 结构体的地址并返回
 * 
 *
 * Return: 返回一个头部错误的回复包结构
 */
struct nrf_pack* get_completion_pack(void)
{
	return &head_com;
}


#if 0

/*
构造包 : 有数据
*/
struct pack* make_pack(u8 *buf,u32 length)
{
	int i;
	struct pack *pack;
	struct pack_head *head;
	struct pack_data *data;
	u32 p_len, d_len = length;
	static unsigned long int num = 0;
	
	/*设置小包的个数*/
	p_len = DIV_ROUND_UP(d_len,EACH_LEN_MAX);
	pack = (struct pack *)malloc(sizeof(struct pack) + sizeof(struct pack_data) * p_len);
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
	head->type = SEND;	
	
	
	/*填充包身*/
	for(i = 0; i < p_len; i++){
		data[i].length = (d_len>EACH_LEN_MAX)?EACH_LEN_MAX:d_len;
		data[i].cpmt = EACH_LEN_MAX - data[i].length;
		data[i].num = i;
		memcpy(data[i].data,buf+i*EACH_LEN_MAX,data[i].length);	
		d_len -= data[i].length;
	}
	return pack;
} 


/*
构造新包 : 以0填充 空包 容纳数据 length
*/
struct pack* make_pack(u32 length)
{
	struct pack *pack;
	u32 p_len, d_len = length;
	
	/*设置小包的个数*/
	p_len = DIV_ROUND_UP(d_len,EACH_LEN_MAX);
	pack = (struct pack *)malloc(sizeof(struct pack) + sizeof(struct pack_data) * p_len);
	if(!pack) {
		cout <<"malloc is fail"<<__LINE__<<endl;
		return NULL;
	}
	memset(pack,0,sizeof(struct pack) + sizeof(struct pack_data) * p_len);
	return pack;
} 


/*
构造包 重发包 
*/
struct pack* make_pack(struct pack * new_pack)
{
	
	struct pack *pack;
	struct pack_head *head;
	struct pack_data *data;
	u32 p_len,d_len;
	int i;
	static unsigned long int num = 0;
	
	p_len = get_buff_loss_len(); 
	if(p_len == 0) return NULL;
	p_len++;/* 增加发送数据尾巴翻倍*/
	
	/*分配内存*/
	pack = (struct pack *)malloc(sizeof(struct pack) + sizeof(struct pack_data) *   p_len);   

	if(!pack) {
		printf("malloc is fail %d \n",__LINE__);
		return NULL;
	}
	head = &pack->head;
	data = pack->data;
	
	/*填充包头*/
	head->head = 'h';
	head->p_len = p_len;
	head->pack_sum = num++;
	head->type = RESEND;	
		
	/* RESEND PACK 作为发送者使用 */
	struct pack_data *send = new_pack->data; 
	u32 lo_buf[p_len];
	u32 lo_num;
	
	read_all_data(lo_buf);/*读出所有丢失的包*/
	// 对最后的包进行发2次提高,降低补发总次数
	lo_buf[p_len -1] = lo_buf[p_len -2];
	
	/*开始填充*/
	head->d_len = 0;
	for(i = 0; i < p_len ; i++){ 
		lo_num = lo_buf[i];
		data[i].length 	= send[lo_num].length;
		data[i].num    	= send[lo_num].num;
		data[i].cpmt 	= send[lo_num].cpmt;
		memcpy(data[i].data,send[lo_num].data,send[lo_num].length);
		head->d_len += send[lo_num].length;
	}
	
	return pack;
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
	
	p_len = 2 * DIV_ROUND_UP(len,EACH_LEN_REQ); /*扩增2倍的包个数*/
	read_all_data(buff); /*读取所有丢失的包到buff*/
	total = len;
	
	for(i = 0; i < p_len/2; i++) {
			d_len = (total>EACH_LEN_REQ)?EACH_LEN_REQ:total;
			data[2*i].length = d_len;
			data[2*i].num	 = i;
			data[2*i].cpmt 	 = EACH_LEN_REQ - d_len; 
			memcpy(data[2*i].data, &buff[EACH_LEN_REQ*i], sizeof(u32) * d_len);
			data[2*i + 1].length  = d_len;
			data[2*i + 1].num 	  = i;
			data[2*i + 1].cpmt 	  = EACH_LEN_REQ - d_len; 
			memcpy(data[2*i + 1].data, &buff[EACH_LEN_REQ*i], sizeof(u32) * d_len);
			total = total - d_len;
	}
	return len*2;
}	

/*
构造包 请求重发
*/
struct pack* make_pack(enum pack_type type)
{
	
	struct pack *pack;
	struct pack_head *head;
	struct pack_data *data;
	u32 p_len,d_len;
	int i;
	static unsigned long int num = 0;
	
	
	if(type == REQ_RESEND)
		p_len = 2 * DIV_ROUND_UP(get_buff_loss_len(),EACH_LEN_REQ);//扩增2倍
	else {
		p_len = 0;
		cout<<"parameter error"<<endl;
		return NULL;
	}
	
	/*分配内存*/
	pack = (struct pack *)malloc(sizeof(struct pack) + sizeof(struct pack_data)* p_len);
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
	head->type = type;	
		
	if(type == REQ_RESEND) {
		/*填充包数据 :data*/
		head->d_len = loss_buff_to_pack(data);		
	} else {
		printf("parameter error ...%d \n",__LINE__);
	}
	return pack;
} 


struct pack* make_new_pack(u8 *buf,u32 len)
{
	return make_pack(buf,len);
}

struct pack* make_null_pack(u32 len)
{
	return make_pack(len);
}

/*
参数:新包 
返回: 重发包
 */
struct pack* make_resend_pack(struct pack* np)
{
	return make_pack(np);
}

struct pack* make_req_pack(void)
{
	return make_pack(REQ_RESEND);
}



static const char *pack_str[] = {	
	"SEND",
	"RECV",
	"RESEND",
	"REQ_RESEND",
	"HEAD_FAIL",
	"COMPLETION",
	"UNDEFINED",
};






/*打印包*/
void print_pack(struct pack* pack)
{
	int i;
	struct pack_head *head = &pack->head;
	struct pack_data *data = pack->data;
	
	printf("head:%c  p_len:%02u  d_len:%02u  num:%02u  ",
		head->head, head->p_len, head->d_len, head->pack_sum);
	printf("TYPE: %s\n", pack_str[head->type]);
	
	switch(head->type) {
	/* */
	case SEND:	
	case RESEND:	
		while(!if_pace_true(head, data)) {
			printf("len:%02d  num:%02d ==> ", data->length, data->num);
			for(i = 0; i < data->length; i++) {
				if(data->data[i] != 0 )
					printf("%c ",data->data[i]);
				else
					printf("  ");
			}
			cout <<endl;
			data++;
		}
		break;
	case REQ_RESEND:
		while(!if_pace_true(head, data)) {
			printf("len:%02d  num:%02d ==> ", data->length, data->num);
			for(i = 0; i < data->length; i++) {
				printf("%03d ",data->req[i]);
			}
			cout <<endl;
			data++;
		}
		break;
	}
	
	printf("\n");
	
}



void del_pack(struct pack* pack)
{
	if(pack->head.type == HEAD_FAIL) 
		cout<<"fail to free HEAD_FAIL pack"<<endl;
	else if(pack->head.type == COMPLETION)
		cout<<"fail to free COMPLETION pack"<<endl;
	else if(pack->head.type >= UNDEFINED) 
		cout<<"fail to free UNDEFINED pack"<<endl;
	else
		free(pack);
}


enum pack_type get_pack_type(struct pack* pack)
{
	
	enum pack_type type = pack->head.type;
	if(type>=SEND && type< UNDEFINED) 
		return type;
	else 
		return UNDEFINED;
}



/* 获取接收进度 */
float get_recv_persen(struct pack *pack)
{
	/* 检查头部 */
	if(pack->head.head != 'h') return 0.0;
	if(pack->head.type != SEND) return 0.0;
	u32 total_len = pack->head.total;
	u32 lo_bye = get_buff_loss_len() * EACH_LEN_MAX ;
	float persen; 	
	persen = 100 - (100.0*lo_bye) / total_len;
	return persen;
}




/*
检查包是否丢失 并构造丢包缓存
并返回包的状态 E_HEAD E_LOSE S_COMPLE 
如果是 E_LOSE 构造请求重发描述
*/
enum receive_status check_recv_pack(struct pack *pack)
{
	int i, p_len, loss = 0;
	const struct pack_head *head = &pack->head;
	const struct pack_data *data = pack->data;
	u32 total_len = pack->head.total;
	p_len = head->p_len;
	
	/*头部错误*/
	if(pack->head.head !='h'|| pack->head.type != SEND ) 
		return E_HEAD;
	
	for(i = 0; i < p_len; i++) {
		if((data[i].length + data[i].cpmt) != EACH_LEN_MAX){
			write_data(i);
			loss++;
		}
	}
	
	if(loss > 0) 
		return E_LOSE;
	else
		return S_COMPLE;
}



/*检查包合法性 正确返回0*/
int  if_pace_true(const struct pack_head *head,struct pack_data *data)
{
	u32 p_len = head->p_len;/*包长度*/
	u32 d_len = head->d_len;/*数据总长度*/
	/*每个包的数据长度 28/7 */
	u32 each_len = (head->type == SEND || head->type == RESEND)?EACH_LEN_MAX:EACH_LEN_REQ;
	if(data->num < 0 ) {
		printf("%s:data->num < 0\n",__func__);
		return -1;
	}
	if(data->length > each_len || data->cpmt > each_len) return -1; /*长度超出 包最大长度*/
	if((data->length + data->cpmt)!= each_len) return -1; /*检验失败*/
	return 0;
}




/*解析丢去包头的包*/
enum receive_status  resolve_no_head_pack(struct pack_data* data,struct pack *result)
{
	int num = 0;
	struct pack_data *p = data;
	while(p->length + p->cpmt == EACH_LEN_MAX){
		int len = p->length;
		memcpy(&result->data[p->num],p,PACK_TOTAL_LEN);
		p++;
		num++;
	}
	/* 没有有效的数据包 */
	if(num == 0) return E_HEAD;
	return E_LOSE;
	
}

/*
参数的 src 可以是 一个 新数据包 、 补发数据包 
解析包 解出数据 
并返回包的状态
*/
enum receive_status resolve_pack(struct pack* src,struct pack *result)
{
	int i,p_len,d_len;
	const struct pack_head *head = &src->head;
	struct pack_data *data = src->data;
	enum receive_status ret = S_COMPLE;
	enum pack_type  type = head->type;
	int p_sum = 0; /* 记录处理的包个数 */
	
	
	/*已确定长度 尝试用无头 方式去解析 */
	/* 比如每次接收512字节 很多时候是没头的 */
	if(head->head != 'h') {
		ret = resolve_no_head_pack((struct pack_data* )src, result);
		if(ret == E_HEAD )  return E_HEAD;
		return check_recv_pack(result);
	}
		
	p_len = head->p_len;
	d_len = head->d_len;

	if(type == SEND) { /*收到的是新包*/
		memcpy(&result->head,head,PACK_HEAD_LEN); /* 包头 */
		while(!if_pace_true(head,data)) {
			struct pack_data *dts;
			dts = &result->data[data->num];/*目的*/
			memcpy(dts,data,PACK_TOTAL_LEN);
			//*len += data->length;
			data++; 
			p_sum ++;
		}	
	}else if(type == RESEND) { /* 收到的是补发的包 */
		while(!if_pace_true(head,data)){ /*可能也丢包,收到的没有 p_len 个,不能循环这么多*/
			struct pack_data *dts; /*目的 源地址*/
			dts = &result->data[data->num];/*目的*/
			memcpy(dts,data,PACK_TOTAL_LEN);
			//*len += data->length;
			data ++; 
			p_sum ++;
		}	
	} else {
		cout <<"this is not normal"<<__func__<<endl;
	}
	
	if(type == RESEND || type == SEND)
		ret = check_recv_pack(result);
	else
		cout <<"FAIL LINE:"<<__LINE__<<endl;	
	return ret;
}

/* 去掉重复的包 */
void remove_repeat_pack(struct pack* pack)
{
	const struct pack_head *head = &pack->head;
	struct pack_data *p1 = pack->data;
	struct pack_data *p2 = pack->data + 1;
	if(head->type != REQ_RESEND) 
		cout<<"warning"<<__LINE__<<endl;
	while(!if_pace_true(head,p2)) {
		while(p2->num == p1->num) p2++;
		memcpy(p1+1,p2,sizeof(*p2));
		p1++;
		p2++;
	}
	memset(p1+1,0,sizeof(*p1));
}

/*解析请求重发包*/
u32 resolve_req_pack(struct pack* src)
{
	int i, p_sum = 0;;
	const struct pack_head *head = &src->head;
	struct pack_data *data = src->data;
	enum pack_type  type = head->type;
	u32 len;	

	if(head->head != 'h') {
		return E_HEAD;
	}
	
	len  = 0;
	/*请求补发包*/
	if(type == REQ_RESEND){ 
		remove_repeat_pack(src); /*去除重复的包*/
		while(!if_pace_true(head,data)){
			for(i = 0; i < data->length; i++) {
				write_data(data->req[i]);
			}
			len += data->length; 
			data++;
			p_sum++;
		}
	} else {
		cout <<"this is not normal"<<__func__<<endl;
	}

	return len;
}


u32 pack_to_len(struct pack* pack)
{
	if(pack->head.type == SEND)
		return pack->head.total;
	else
		return 0;
}



u32 pack_to_data(struct pack *pack,u8 *buf)
{
	int i;
	u32 p_len;
	u8  d_len;
	const struct pack_head *head = &pack->head;
	struct pack_data *data = pack->data;
	p_len = head->p_len;
	for(i = 0;i < p_len; i++) {
		d_len = data[i].length;
		memcpy(&buf[i*EACH_LEN_MAX],data[i].data,d_len);
	}
}




int if_loss_empty(void)
{
	int len = get_buff_loss_len();
	
	if(0 == len) return 0;
    else return 1;	
}



/* 成功返回0 否则返回其他值*/
int check_reply(struct pack* pack,int len)
{
	const struct pack_head *head = &pack->head;
	struct pack_data *data = pack->data;
	if(head->type == HEAD_FAIL)  return 0; 	/*这个包是32BYTE*/
	if(head->type == COMPLETION) return 0;	/*这个包是32BYTE*/
	if(head->type == REQ_RESEND && len > 32) {
		if(data->length <= 7)	return 0; //防止出现两个都是包头重叠
	}
	  
	return 1;
}


#endif

