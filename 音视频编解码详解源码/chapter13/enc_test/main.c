
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "enc.h"

void main()
{
	int a,b,add,sub;
	clock_t start,stop,over_time;
	
	a = 200;
	b = 100;
	start = clock();
	add = func_add(a,b);
	sub = func_sub(a,b);
	stop = clock();
	
	printf("\na+b = %d, a-b = %d, time=%d\n",add,sub,stop-start);
	
}
