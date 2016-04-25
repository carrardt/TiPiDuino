#include <stdio.h>
#include <cstdint>
#include <cmath>
#include <iostream>

#include "LinkuinoClient.h"

int main(int argc, char* argv[])
{
	if(argc<2) { fprintf(stderr,"Usage: %s /dev/ttySomething\n",argv[0]); return 1; }

	std::cout<<"Speed="<<Linkuino::SERIAL_SPEED<<", repeats="<<LinkuinoClient::PacketRepeatCount<<'\n';

	int serial_fd = LinkuinoClient::openSerialDevice( argv[1] );
	if( serial_fd < 0 ) { fprintf(stderr,"can't open device '%s'\n",argv[1]); return 1; }
	LinkuinoClient link( serial_fd );
	if( ! link.testConnection() )
	{
		std::cerr<<"No valid Linkuino server on "<<argv[1]<<'\n';
		return 1;
	}
	std::cout<<"Connected to Linkuino server v"<<link.getServerVersionMajor()<<'.'<<link.getServerVersionMinor()<<'\n';
		
	char cmd=' ';
	scanf("%c",&cmd);

	if(cmd=='s')
	{
		int m=1250;
		int a=350;
		scanf("%d %d",&m,&a);
		printf("Sin : avg=%d, amp=%d\n",m,a);
		double t=0.0;
		int p = 0;
		while( true )
		{
			for(int i=0;i<6;i++)
			{
				uint32_t x = sin(t+0.5*i) * a + m;
				link.setPWMValue( i , x );
			}
			//link.printBuffer();
			link.send();
			//link.send();
			t += 0.0001;
		}
	}
	else if( cmd=='d' )
	{
		int p=0;
		scanf("%d",&p);
		printf("Disable PWM %d\n",p);
		link.setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_PWM0_DISABLE+p);
		link.send();
	}
	else if( cmd=='e' )
	{
		int p=0;
		scanf("%d",&p);
		printf("Enable PWM %d\n",p);
		link.setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_PWM0_ENABLE+p);
		link.send();
	}
	else if( cmd=='v' )
	{
		int p=0;
		int v = 1250;
		scanf("%d %d",&p,&v);
		printf("set PWM %d to %d\n",p,v);
		link.setPWMValue( p , v );
		link.send();
	}
	else if( cmd=='t' )
	{
		int v = 1250;
		scanf("%d",&v);
		int ve = Linkuino::encodePulseLength( v );
		int vd = Linkuino::decodePulseLength ( ve );
		printf("Encode Test : %d -> %d -> %d\n",v,ve,vd);
	}
	sleep(1);
	return 0;
}
