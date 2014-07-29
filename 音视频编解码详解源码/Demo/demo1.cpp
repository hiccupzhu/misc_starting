#include "stdio.h"
extern int maxm(int,int);
extern int minm(int,int);
int main()
{
	printf("maxm=%d  minm=%d\n",maxm(2,7),minm(34,43));
	getchar();
	return 0;
}