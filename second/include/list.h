#ifndef __LIST__H
#define __LIST__H

#include <stdio.h>

#define BUFF_MAX  50

typedef unsigned int u32;

struct list_head
{
  struct  list_head *next,*prev;
};


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





static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}


/*
 * 在两个已知的连续条目之间插入一个新条目。
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new1,
			      struct list_head *prev, struct list_head *next)
{
	next->prev = new1;
	new1->next = next;
	new1->prev = prev;
	prev->next = new1;
}


static inline void list_add(struct list_head *new1, struct list_head *head)
{
	__list_add(new1, head, head->next);
}

static inline void list_add_tail(struct list_head *new1, struct list_head *head)
{
	__list_add(new1, head->prev, head);
}


#define WRITE_ONCE(var, val) \
	(*((volatile typeof(val) *)(&(var))) = (val))

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	WRITE_ONCE(prev->next, next);
}

static inline void __list_del_entry(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}


static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}



#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({			\
	const typeof(((type *)0)->member) * __mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member)); })


#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_next_entry(pos, member))



int get_data_len(void);
void init_data_base(void);
int read_all_data(u32 *buf);
int write_data(u32 val);		 


#endif