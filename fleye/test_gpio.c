#include <stdio.h>
#include "gpio.h"
#include <unistd.h>

int main(int argc, char* argv[])
{
	int i,X,Y,s;
	if(argc<3) return 1;
	init_gpio();
	X = atoi(argv[1]);
	Y = atoi(argv[2]);
	if( X==-1 )
	{
		for(i=1;i<1024;++i)
		{
			gpio_write_xy_i(i,Y);
			usleep(Y);
		}
		gpio_write_xy_i(0,Y);
	}
	else
	{
		gpio_write_xy_i(X,Y);
	}
	return 0;
}
