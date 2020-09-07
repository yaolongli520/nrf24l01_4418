#ifndef __SYSTEM_H_
#define __SYSTEM_H_

#include <iostream>
#include <unistd.h>

#include "list.h"
#include "mytime.h"


using namespace std;

/* 公共类型 */
typedef unsigned char u8;
typedef  unsigned short int u16;
typedef  unsigned  int u32;

#define BIT(i)  (0x1<<i)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d)) 
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define DEBUG_POINT() printf("%s %d \n",__func__,__LINE__)


#endif


