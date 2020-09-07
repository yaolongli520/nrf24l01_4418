#include <stdlib.h>
#include <string.h>
#include "pack.h"
#include "nrf_server.h"

const char *type_name[] ={
	"TYPE_CMD",
	"TYPE_DATA",
	"TYPE_REQ_DATA",
	"TYPE_ERR",
	"TYPE_NODEFINE",
};



int  if_pace_true(const struct pack_head *head,struct pack_data *data);

u32 pack_sum = 0; /*数据号 每次传输自加*/

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
	};
	head_fail.type = TYPE_DATA;
	head_fail.len  = 0;
	head_fail.num  = CMD_HEAD_FAIL; /*命令号*/
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
	};
	head_com.type = TYPE_DATA;
	head_com.len  = 0;
	head_com.num  = CMD_COMPLETION;/*命令号*/
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


/**
 * data_to_pack_packlen - 有效数据长度转换为包的个数 比如长度285 ->11个
 * 
 *
 * Return: 返回lenght长度的数据需要打包成的包个数
 */

u32 data_to_pack_packlen(u32 lenght)
{
	return DIV_ROUND_UP(lenght,EACH_LEN_MAX);
}

/**
 * data_to_pack_datalen - 有效数据长度转换为加包头的数据长度
 * 
 *
 * Return: 返回lenght长度的数据打包后的新长度 29->64
 */

u32 data_to_pack_datalen(u32 lenght)
{
	return DATALEN_TO_PACKLEN(lenght);
}



/**
 * make_pack - 用于释放构造的包空间
 * 
 *
 * Return: NULL
 */

void del_pack(struct nrf_pack* pack)
{
	if(pack->type == TYPE_DATA || pack->type == TYPE_REQ_DATA) 
		free(pack);
}



/**
 * make_pack - 用于构造数据包进行发送
 * 
 *
 * Return: 返回用于发送的数据包结构
 */

struct nrf_pack* make_pack(u8 *buf,u32 length)
{
	int i;
	struct nrf_pack *pack;
	u32 p_len, d_len = length;
	
	/*设置小包的个数*/
	p_len = DIV_ROUND_UP(d_len,EACH_LEN_MAX);
	pack = (struct nrf_pack *)malloc(sizeof(struct nrf_pack) * p_len);
	if(!pack) {
		printf("malloc is fail %d \n",__LINE__);
		return NULL;
	}
	
	/*填充包身*/
	for(i = 0; i < p_len; i++){
		pack[i].type = TYPE_DATA;
		pack[i].len = (d_len>EACH_LEN_MAX)?EACH_LEN_MAX:d_len;
		pack[i].num = i;
		pack[i].check = (~pack[i].type & 0x3);
		memcpy(pack[i].data,buf+i*EACH_LEN_MAX,pack[i].len);	
		d_len -= pack[i].len;
	}
	if(d_len!=0) cout << "warning d_len no == 0"<<endl;
	
	return pack;
} 


/**
 * get_pack_max_num - 用于获取包的最大下标
 *
 *
 * Return: 
 */

u32 get_pack_max_num(struct nrf_pack *pack)
{
	u32 num;
	while(IS_CHECK_OK(pack)) 	
		pack++;
	pack--;
	num = pack->num;
	return num;
}



/**
 * get_loss_sum - 用于获取丢失包的总数,并把丢失包写入链表
 *
 *
 * Return: 
 */

u32 get_loss_sum(u32 max,struct nrf_pack *pack)
{
	u32 sum = 0;
	
	while(IS_CHECK_OK(pack)) {
		for(int i = 0; i < EACH_LEN_REQ; i++) {
			for(int j = 0; j < EACH_REQ_BIT; j++)	
				if((pack->req[i] & (0x1<<j)) == 0)/*位 = 0*/
				{
					if(pack->num * EACH_LEN_LOSS + i * EACH_REQ_BIT + j <= max) {
						sum ++;
						write_data(pack->num * EACH_LEN_LOSS + i * EACH_REQ_BIT + j);
					//	printf("val =%d \n",pack->num * EACH_LEN_LOSS + i * EACH_REQ_BIT + j);
					//	printf("j = %d pack->req[i]=%d \n",j,pack->req[i]); 
					}
				}
		}
		pack++;
	}
	return sum;
}


/**
 * make_pack - 用于构造数据进行补发
 * @snd_pack: 第一次发送的完整包
 * @lo_pack:  描述丢失的包 
 * Return: 返回用于再次发送的数据包结构
*/
struct nrf_pack* make_pack(struct nrf_pack *snd_pack,struct nrf_pack *lo_pack)
{
	struct nrf_pack *pack;
	u32 max_num =  get_pack_max_num(snd_pack); /*最大包号*/
	u32 p_len = get_loss_sum(max_num,lo_pack); /*丢失总数*/
	u32 loss[p_len];
	int i = 0;

	if(p_len == 0) {
		printf("is not loss \n");
		return NULL;
	}
	read_all_data(loss);

	pack = (struct nrf_pack *)malloc(sizeof(struct nrf_pack) * p_len);
	if(!pack) {
		cout <<"malloc is fail"<<__LINE__<<endl;
		return NULL;
	}

	for(i = 0; i < p_len; i++) {
		memcpy(&pack[i],&snd_pack[loss[i]],TRAN_LEN_MAX);
	}

	return pack;
} 



/**
 * make_pack - 用于构造请求重发包
 *  每个包 224 bit 收到的位=1 未收到=0
 *
 * Return: 返回用于发送的数据包结构
 */

struct nrf_pack* make_pack(u32 length)
{
	struct nrf_pack *pack;
	u32 p_len, d_len = length;
	
	/*设置小包的个数*/
	p_len = DIV_ROUND_UP(d_len,EACH_LEN_MAX);/*多少个数据包*/
	p_len = DIV_ROUND_UP(p_len,EACH_LEN_LOSS); /*需要多少个请求重发包*/
	printf("length:%d p_len=%d \n",length,p_len);
	pack = (struct nrf_pack *)malloc(sizeof(struct nrf_pack) * p_len);
	if(!pack) {
		cout <<"malloc is fail"<<__LINE__<<endl;
		return NULL;
	}
	memset(pack,0,sizeof(struct nrf_pack) * p_len);
	for(int i = 0; i < p_len; i++){
		pack[i].type = TYPE_REQ_DATA;
		pack[i].len	 = EACH_LEN_REQ;
		pack[i].num  = i;
		pack[i].check = (~pack[i].type & 0x3);
	}
	return pack;
} 


/**
 * nrf_check_type - 检查 nrf_pack 数据的类型是否合法
 * 
 * Return: 校验失败 返回 TYPE_ERR,否则返回类型,或者未定义类型 TYPE_NODEFINE。
 */
int nrf_check_type(void *data)
{
	struct nrf_pack *pack = NULL;
	pack = (typeof (pack)) data;
	if(!IS_CHECK_OK(pack))  /*检验*/
		return TYPE_ERR;
	if(pack->type < TYPE_ERR) 
		return pack->type;
	else 
		return TYPE_NODEFINE; /*校验正确但不存在此类型*/
}


/*打印包*/
void print_nrf_pack(struct nrf_pack* pack)
{
	int i;
	while(IS_CHECK_OK(pack)) {
		printf("type:%s  len:%02d  num:%02d ==>",type_name[pack->type], pack->len, pack->num);
		switch(pack->type) {
		/* */
		case TYPE_CMD:
			printf("unknown...");
			break;
		case TYPE_DATA:	
			for(i = 0; i < pack->len; i++) {
				if(pack->data[i] != 0 )
					printf("%c ",pack->data[i]);
				else
					printf("  ");
			}	
			break;
		case TYPE_REQ_DATA:		
			for(i = 0; i < pack->len; i++) {
				printf("%08x ",pack->req[i]);
			}
			break;
		}
		printf("\n");
		pack++;
	}
	printf("\n");	
}




#if 0



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

