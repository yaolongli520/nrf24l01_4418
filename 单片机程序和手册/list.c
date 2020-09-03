#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

#define BUFF_MAX  1024




static struct str_data_base data_all;

struct str_data{
	u32 used; 			/*已用长度*/
	u32 residue; 		/*剩余长度*/
	u32 buf[BUFF_MAX];	
	struct list_head list; /* list */
};

struct str_data_base{
	int total; /*总长度 */
	struct list_head entry;
};

/* 成功返回0 */
int write_data(u32 val)
{
	int ret = 0;
	struct str_data *data =NULL;
	list_for_each_entry(data,&data_all.entry,list) {
		if(data->residue) {
			data->buf[data->used] = val;
			data->used++;
			data->residue = BUFF_MAX - data->used;
			data_all.total ++;
			return ret;
		}		
	}
	data = malloc(sizeof(*data));
	if(!data) return -1;
	printf("data malloc addr =%p \n",data);
	memset(data,0,sizeof(*data));
	data->buf[0] = val;
	data->used = 1;
	data->residue = BUFF_MAX - data->used;
	INIT_LIST_HEAD(&data->list);
	list_add_tail(&data->list,&data_all.entry);
	data_all.total ++;
	return 0;
}

void free_all_date(void)
{
	struct list_head *list;
	struct str_data *data;
	while(!list_empty(&data_all.entry))
	{
		list = data_all.entry.next;
		__list_del_entry(data_all.entry.next);/*解离出链表*/
		data = list_entry(list, struct str_data, list);
		printf("free data addr= %p \n",data);
		free(data);
	}
	data_all.total = 0;
}



/*读出所有数据到 buf 并释放内存*/
int read_all_data(int *buf)
{
	int *p = buf;
	struct str_data *data =NULL;
	
	list_for_each_entry(data,&data_all.entry,list) {
		int i;
		for(i = 0; i < data->used; i++) {
			*p++ = data->buf[i];
		}
	}
	free_all_date();
	return 0;
}

void init_data_base(void)
{
	data_all.total = 0;
	INIT_LIST_HEAD(&data_all.entry);	
}

/*返回长度 */
int get_data_len(void)
{
	return data_all.total;	
}


