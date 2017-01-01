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

	std::cout << "Info: Linkuino size is " << sizeof(Linkuino) << "\n";

	if( argc>=2 )
	{
		if( ! serial.open(argv[1]) ) { std::cerr<<"can't open device '"<< argv[1] <<"'\n"; return 1; }
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
		std::cin >> cmd;
		if(cmd=='S')
		{
			float m=0.5;
			float a=0.5;
			std::cin >> m >> a;
			std::cout << "Sin : avg=" << m << ", amp=" << a << "\n";
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
		else if(cmd=='s')
		{
			int p=0;
			float m=0.5;
			float a=0.5;
			std::cin >> p >> m >> a;
			std::cout<<"Sinus wave : PWM="<<p<<", avg="<<m<<", amp="<<a<<"\n";
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
			std::cin >> p;
			std::cout<<"Disable PWM "<<p<<"\n";
			link.disablePWM(p);
			link.send();
		}
		
		else if( cmd=='e' )
		{
			int p=0;
			std::cin >> p;
			std::cout << "Enable PWM " << p << "\n";
			link.enablePWM(p);
			link.send();
		}
		
		else if( cmd=='v' )
		{
			int p=0;
			float v = 0.5;
			std::cin >> p >> v;
			std::cout<<"set PWM "<<p<<" to "<<v<<"\n";
			link.setPWMValue( p , v );
			link.send();
		}
		else if( cmd=='t' )
		{
			int v = 1250;
			std::cin >> v;
			int ve = Linkuino::encodePulseLength( v );
			int vd = Linkuino::decodePulseLength ( ve );
			std::cout<<"Encode Test : "<<v<<" -> "<<ve<<"-> "<<vd<<"\n";
		}
		else if( cmd=='o' )
		{
			int v=0;
			std::cin >> v;
			std::cout<<"digital out = "<<v<<"\n";
			link.setDigitalOutput(v);
			link.send();
		}
		else if( cmd=='f' )
		{
			uint32_t a=0, b=0, c=0, d=0;
			std::cin >> a >> b >> c >> d;
			a = clamp(a,0U,15U);
			b = clamp(b,0U,255U);
			c = clamp(c,0U,15U);
			d = clamp(d,0U,255U);
			uint32_t data = a<<20 | b<<12 | c<<8 | d;
			uint16_t d0 = (data>>18) & 0x3F;
			uint16_t d1 = (data>>12) & 0x3F;
			uint16_t d2 = (data>>6) & 0x3F;
			uint16_t d3 = data & 0x3F;
			std::cout<<"forward: "<<a<<' '<<b<<' '<<c<<' '<<d<<" => "<<data<<'/'<<std::hex<<data<<" => "<<d0<<' '<<d1<<' '<<d2<<' '<<d3<<"\n";
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
				std::cout << "forward: " << a << ' ' << b << ' ' << c << ' ' << d << " => " << data << '/' << std::hex << data << " => " << d0 << ' ' << d1 << ' ' << d2 << ' ' << d3 << "\n";
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
		else if( cmd=='a' )
		{
			int ch=0, nsamples=1;
			std::cin >> ch >> nsamples;
			std::cout<<"read "<<nsamples<<" samples on channel #"<<ch<<"\n";
			float v = link.requestAnalogRead( ch, nsamples );
			std::cout<<"=> "<<v<<"\n";
		}
		else if (cmd == 'A')
		{
			int ch = 0, nsamples = 1, count=100;
			std::cin >> ch >> nsamples >> count;
			//
			//std::cout << "flushInput time = " <<  << "uS\n";
			std::cout << "benchmark analog channel #" << ch <<", with "<< nsamples<<" samples per read, "<<count<<" reads\n";
			double v = 0.0;
			uint64_t totalTime = 0;
			for (int i = 0; i<count && v>=0.0 ; i++)
			{
				auto T1 = std::chrono::high_resolution_clock::now();
				float s = link.requestAnalogRead(ch, nsamples);
				if( v>=0.0 && s!=-1.0f ) { v += s; }
				else { v = -1.0; }
				auto T2 = std::chrono::high_resolution_clock::now();
				totalTime += std::chrono::duration_cast<std::chrono::microseconds>(T2 - T1).count();
			}
			std::cout <<"avg="<< totalTime/count<<" uS, value=" << v/count << "\n";
		}
		else if( cmd=='i' )
		{
			std::cout<<"request digital read\n";
			int v = link.requestDigitalRead();
			std::cout<<"=> "<< std::hex << v<< std::dec <<"\n";
		}
		else if( cmd=='m' )
		{
			int mr=24;
			std::cin>>mr;
			std::cout<<"forcing message repeats to "<<std::dec<<mr<<"\n";
			link.forceMessageRepeats( mr );
		}
	}

	link.quiet();
	return 0;
}
