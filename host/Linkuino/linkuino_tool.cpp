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
	LinkuinoSerialPort serial;
	LinkuinoClient link( &serial );

	if( argc>=2 )
	{
		if( ! serial.open(argv[1]) ) { fprintf(stderr,"can't open device '%s'\n",argv[1]); return 1; }
		if( ! link.testConnection() )
		{
			std::cerr<<"No valid Linkuino server on "<<argv[1]<<'\n';
			return 1;
		}
	}
	else
	{
		link.scanSerialPorts();
	}
	link.printStatus();

	char cmd=' ';
	while( cmd != 'q' )
	{
		scanf("%c",&cmd);
		if(cmd=='S')
		{
			float m=0.5;
			float a=0.5;
			scanf("%f %f",&m,&a);
			printf("Sin : avg=%g, amp=%g\n",m,a);
			double t=0.0;
			for( int s=0;s<10000;s++ )
			{
				for(int i=0;i<6;i++)
				{
					link.setPWMValue( i , sin(t+0.33*i) * a + m );
				}
				link.send();
				t+=0.01;
			}
		}

		if(cmd=='s')
		{
			int p=0;
			float m=0.5;
			float a=0.5;
			scanf("%d %f %f",&p,&m,&a);
			printf("Sinus wave : PWM=%d avg=%g, amp=%g\n",p,m,a);
			double t=0.0;
			for( int s=0;s<10000;s++ )
			{
				link.setPWMValue( p , sin(t) * a + m );
				link.send();
				t+=0.01;
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
			float v = 0.5;
			scanf("%d %f",&p,&v);
			printf("set PWM %d to %f\n",p,v);
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
		else if( cmd=='o' )
		{
			uint16_t v=0;
			scanf("%d",&v);
			printf("digital out = %d\n",v);
			link.setDigitalOutput(v);
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
			link.forwardMessage( d0, d1, d2, d3 );
		}
#ifndef _WIN32
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
				link.forwardMessage( d0, d1, d2, d3 );
			}
		}
#endif
		else if( cmd=='r' )
		{
			std::cout<<"reset to 50Hz mode\n";
			link.resetTo50Hz();
			link.printStatus();
		}
		else if( cmd=='R' )
		{
			std::cout<<"reset to 100Hz mode\n";
			link.resetTo100Hz();
			link.printStatus();
		}
	}

	link.quiet();
	return 0;
}
