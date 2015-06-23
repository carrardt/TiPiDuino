#include <stdio.h>
#include "gpio.h"
#include <unistd.h>

int main(int argc, char* argv[])
{
	int i=0,X=0,Y=0,L=0;
	double Xf=0.0, Yf=0.0;
	if(argc<4) return 1;

	X = atoi(argv[2]);
	Y = atoi(argv[3]);
	L = atoi(argv[4]);
	
	Xf = X * 0.017453292519943295 ;
	Yf = Y * 0.017453292519943295 ;
	
	init_gpio();
	
	switch( argv[1][0] )
	{
		
		case 'r':
			gpio_write_xy_i(X,Y,L);
			break;

		case 'S':
			for(i=1;i<1024;++i)
			{
				gpio_write_xy_i(X,i,L);
				usleep(X);
			}
			gpio_write_xy_i(X,0,L);
			break;

		case 's':
			for(i=1;i<1024;++i)
			{
				gpio_write_xy_i(i,Y,L);
				usleep(Y);
			}
			gpio_write_xy_i(0,Y,L);
			break;
			
		case 'a':
			printf("angle: theta = %g, phi=%g, laser=%d\n",Xf,Yf,L);
			gpio_write_theta_phi(Xf,Yf,L);
			break;
	}

	return 0;
}
