#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>    
#include <string.h>

#include "mytime.h"

using namespace std;

#define TIME_GET_TIME  0x01  //时间
#define TIME_GET_DATE 0x02  //日期


struct time_str{
	char time[10];//时间
	char date[30];//日期
}glo_time={
	{0},{0},
};



/*
获取当前时间

*/
int get_cur_time(int flag,char *buf)    
{       
		time_t tm; //时间结构
		time(&tm);//获取时间秒 从1970.1.1 0:0:0 到现在
		struct tm  *loctime;//当地时间
		loctime = localtime(&tm); //转换为当地时间
		#if 0
		printf("localtime() get the time is %d-%d-%d  %d:%d:%d \n", 
		loctime->tm_year+1900, loctime->tm_mon+1, loctime->tm_mday,      
		loctime->tm_hour, loctime->tm_min, loctime->tm_sec); 
		#endif
		if(flag == TIME_GET_TIME) //时间
		{
			sprintf(buf,"%02d:%02d:%02d",loctime->tm_hour,loctime->tm_min
			,loctime->tm_sec);
			
		}else if(flag == TIME_GET_DATE) //日期
		{
			sprintf(buf,"%04d--%02d--%02d--",loctime->tm_year+1900, loctime->tm_mon+1, 
			loctime->tm_mday);
			
		}
		
		return 0;       
}

char * get_time_time(void)//时间
{
	char *s = glo_time.time;
	memset(s,0,sizeof(glo_time.time));
	get_cur_time(TIME_GET_TIME,s);
	printf("%s %s\n",__func__,s);
	return s;
}

char * get_time_date(void) //日期
{
	char *s = glo_time.date;
	memset(s,0,sizeof(glo_time.date));
	get_cur_time(TIME_GET_DATE,s);
	printf("%s %s\n",__func__,s);
	return s;
}

//set_time_time();
//修改时间 ：时分秒
int set_time_time(char *t)
{
	int s,m,h;
	struct timeval tv;
	gettimeofday(&tv, NULL); //获取时间
	struct tm  *loctime;//当地时间
	loctime = localtime(&tv.tv_sec); //转换为当地时间
	
	sscanf(t,"%d:%d:%d",&h,&m,&s); //提取时分秒

	//修改时分秒
	loctime->tm_sec = h;         
    loctime->tm_min = m;         
    loctime->tm_hour = s;      
	
	tv.tv_sec =  mktime(loctime); //将时间转为秒格式
	
    settimeofday(&tv,NULL); //设置
	system("hwclock -w");
	
}


/*获取时间差*/
struct timespec get_time_offset(struct timespec &prev,struct timespec &cur)
{
	long int sec;
	long int nsec;
	struct timespec ret;
	sec  = cur.tv_sec - prev.tv_sec;
	nsec = cur.tv_nsec - prev.tv_nsec;
	
	if(sec > 0 && nsec < 0){
		ret.tv_sec = sec - 1;
		ret.tv_nsec = 1000000000 + nsec;
	} else {
		ret.tv_sec = sec;
		ret.tv_nsec = nsec;
	}
	return ret;
}



//判断时间差是否大于 timeout
int get_timeout(struct timespec &time,long int timeout)
{
	long int t = time.tv_sec*1000000000 + time.tv_nsec;
	if(t > timeout)
		return TIME_OUT;
	return ON_TIME;
}





