#include <stdio.h>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <string>

#include "LinkuinoClient.h"

template<typename T>
static inline T clamp(T x, T l, T h)
{
	if( x < l ) return l;
	if( x > h ) return h;
	return x;
}

int main(int argc, char* argv[])
{
	if(argc<2) { fprintf(stderr,"Usage: %s /dev/ttySomething\n",argv[0]); return 1; }

	std::cout<<"Speed="<<Linkuino::SERIAL_SPEED<<'\n';

	int serial_fd = LinkuinoClient::openSerialDevice( argv[1] );
	if( serial_fd < 0 ) { fprintf(stderr,"can't open device '%s'\n",argv[1]); return 1; }

	bool freq100Hz = false;
	if( argc>=3 ) { freq100Hz=atoi(argv[2]); }

	LinkuinoClient link( serial_fd , freq100Hz );
	if( ! link.testConnection() )
	{
		std::cerr<<"No valid Linkuino server on "<<argv[1]<<'\n';
		return 1;
	}
	std::cout<<"Connected to Linkuino server v"<<link.getServerVersionMajor()<<'.'<<link.getServerVersionMinor()<<'\n';
	std::cout<<"Operation frequency is "<< ( freq100Hz ? "100Hz" : "50Hz") << "\n";
	std::cout<<"message repeats = "<<link.getMessageRepeats()<<'\n';

	for(int i=1;i<argc;i++)
	{
		if( std::string(argv[i])=="-mr" )
		{
			++i;
			if(i<argc)
			{
				int mr = atoi(argv[i]);
				std::cout<<"force repeats to "<<mr<<'\n';
				link.forceMessageRepeats(mr);
			}
		}
	}

	char cmd=' ';
	scanf("%c",&cmd);

	if(cmd=='s')
	{
		int m=1250;
		int a=350;
		scanf("%d %d",&m,&a);
		printf("Sin : avg=%d, amp=%d\n",m,a);
		double t=0.0;
		while( true )
		{
			for(int i=0;i<6;i++)
			{
				uint32_t x = sin(t+0.33*i) * a + m;
				link.setPWMValue( i , x );
			}
			link.send();
			//link.printBuffer();
			//link.send();
			//sleep(1);
			t+=0.01;
		}
	}

	if(cmd=='S')
	{
		int m=1250;
		int a=350;
		int p=0;
		int uS=0;
		scanf("%d %d %d %d",&p,&uS,&m,&a);
		printf("Sinus wave : PWM=%d interval=%duS, avg=%d, amp=%d\n",p,m,a);
		double t=0.0;
		while( true )
		{
			uint32_t x = sin(t) * a + m;
			link.setPWMValue( p , x );
			link.send();
			//link.printBuffer();
			//link.send();
			usleep(uS);
			t+=0.1;
		}
	}

	else if( cmd=='d' )
	{
		int p=0;
		scanf("%d",&p);
		printf("Disable PWM %d\n",p);
		link.disablePWM(p);
		link.send();
	}
	else if( cmd=='e' )
	{
		int p=0;
		scanf("%d",&p);
		printf("Enable PWM %d\n",p);
		link.enablePWM(p);
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
		link.send(); // ??
	}
	else if( cmd=='t' )
	{
		int v = 1250;
		scanf("%d",&v);
		int ve = Linkuino::encodePulseLength( v );
		int vd = Linkuino::decodePulseLength ( ve );
		printf("Encode Test : %d -> %d -> %d\n",v,ve,vd);
	}
	else if( cmd=='o' )
	{
		uint16_t v=0;
		scanf("%d",&v);
		printf("digital out = %d\n",v);
		link.setRegisterValue(Linkuino::DOUT_ADDR, v);
		link.send();
	}
	else if( cmd=='f' )
	{
		uint32_t a=0, b=0, c=0, d=0;
		scanf("%d %d %d %d",&a,&b,&c,&d);
		a = clamp(a,0U,15U);
		b = clamp(b,0U,255U);
		c = clamp(c,0U,15U);
		d = clamp(d,0U,255U);
		uint32_t data = a<<20 | b<<12 | c<<8 | d;
		uint16_t d0 = (data>>18) & 0x3F;
		uint16_t d1 = (data>>12) & 0x3F;
		uint16_t d2 = (data>>6) & 0x3F;
		uint16_t d3 = data & 0x3F;
		printf("forward: %d %d %d %d => %d (0x%08X) => %02X %02X %02X %02X\n",a,b,c,d,data,data,d0,d1,d2,d3);
		link.setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_FWD_SERIAL);
		link.setRegisterValue(Linkuino::REQ_DATA0_ADDR, d0);
		link.setRegisterValue(Linkuino::REQ_DATA1_ADDR, d1);
		link.setRegisterValue(Linkuino::REQ_DATA2_ADDR, d2);
		link.setRegisterValue(Linkuino::REQ_DATA3_ADDR, d3);
		link.send();
	}
	else if( cmd=='F' )
	{
		uint32_t a=0, b=0, c=0, d=0;
		for(int i=0;i<10;i++)
		{
			a = lrand48() % 15;
			b = lrand48() % 255;
			c = lrand48() % 15;
			d = lrand48() % 255;
			a = clamp(a,0U,15U);
			b = clamp(b,0U,255U);
			c = clamp(c,0U,15U);
			d = clamp(d,0U,255U);
			uint32_t data = a<<20 | b<<12 | c<<8 | d;
			uint16_t d0 = (data>>18) & 0x3F;
			uint16_t d1 = (data>>12) & 0x3F;
			uint16_t d2 = (data>>6) & 0x3F;
			uint16_t d3 = data & 0x3F;
			printf("forward: %d %d %d %d => %d (0x%08X) => %02X %02X %02X %02X\n",a,b,c,d,data,data,d0,d1,d2,d3);
			link.setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_FWD_SERIAL);
			link.setRegisterValue(Linkuino::REQ_DATA0_ADDR, d0);
			link.setRegisterValue(Linkuino::REQ_DATA1_ADDR, d1);
			link.setRegisterValue(Linkuino::REQ_DATA2_ADDR, d2);
			link.setRegisterValue(Linkuino::REQ_DATA3_ADDR, d3);
			link.send();
			sleep(1);
		}
	}
	else if( cmd=='r' )
	{
		std::cout<<"reset to 50Hz mode\n";
		link.setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_RESET);
		link.setRegisterValue(Linkuino::REQ_DATA0_ADDR,Linkuino::RESET_TO_50Hz);
		link.send();
		link.send();
	}
	else if( cmd=='R' )
	{
		std::cout<<"reset to 100Hz mode\n";
		link.setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_RESET);
		link.setRegisterValue(Linkuino::REQ_DATA0_ADDR,Linkuino::RESET_TO_100Hz);
		link.send();
		link.send();
	}
	//sleep(1);
	return 0;
}
