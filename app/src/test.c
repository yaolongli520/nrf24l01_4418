#include <stdio.h>
#include <unistd.h>

void main()
{
  int i,j;
  for(i =0;i < 100;i++)
  {
     if(i%10==0) printf("1....\n");
     usleep(100000);
  }


}
