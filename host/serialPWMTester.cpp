#include <stdio.h>
#include <cstdint>
#include <cmath>

#include "LinkuinoClient.h"

int main(int argc, char* argv[])
{
	if(argc<2) { fprintf(stderr,"Usage: %s /dev/ttySomething\n",argv[0]); return 1; }

	int serial_fd = LinkuinoClient::openSerialDevice( argv[1] );
	if( serial_fd < 0 ) { fprintf(stderr,"can't open device '%s'\n",argv[1]); return 1; }
	LinkuinoClient link( serial_fd );
	
	uint32_t x=1250;
	//scanf("%d",&x);
	
	double t=0.0;
	int p = 0;
	while( true )
	{
		for(int i=0;i<6;i++)
		{
			uint32_t x = sin(t+0.5*i) * 700.0 + 1250.0;
			link.setPWMValue( i , x );
		}
		//link.printBuffer();
		link.send();
		t += 0.01;
	}

	return 0;
}
