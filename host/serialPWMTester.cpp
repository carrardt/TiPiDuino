#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstdint>
#include <cmath>

#include "Linkuino/Linkuino.h"
using Linkuino = LinkuinoT<>;

struct LinkuinoClient
{
	inline LinkuinoClient(int fd)
		: m_fd(fd)
		, m_timeStamp(0)
	{
		setRegisterValue(Linkuino::TSTMP0_ADDR, 0);
		for(int i=0;i<Linkuino::PWM_COUNT;i++) { setPWMValue(i, 1250); }
		setRegisterValue(Linkuino::DOUT_ADDR, 0);
		setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_NOOP);
		setRegisterValue(Linkuino::REQDATA_ADDR, 0);
	}

	void setRegisterValue(uint8_t i, uint8_t value)
	{
		if(i>Linkuino::CMD_COUNT) return;

		if(i==0) { m_buffer[i] = value & 0x3F; }
		else
		{
			uint8_t clk = ( (i-1)%3 ) + 1; // 1, 2 or 3
			m_buffer[i] = (clk<<6) | ( value & 0x3F );
		}
	}

	void setPWMValue( int p, uint16_t value )
	{
		if( p<0 || p>5 ) return ;
		if(value<500) value = 500;
		else if(value>2000) value=2000;
		setRegisterValue( Linkuino::PWM0H_ADDR+2*p , value >> 6 );
		setRegisterValue( Linkuino::PWM0L_ADDR+2*p , value & 0x3F );
	}
	
	inline void updateTimeStamp()
	{
		m_timeStamp = (m_timeStamp+1) & 0x3F;
		setRegisterValue( Linkuino::TSTMP0_ADDR, m_timeStamp );
	}
	
	inline void printBuffer()
	{
		for(int i=0;i<16;i++)
		{
			printf("%d|%d ",m_buffer[i]>>6,m_buffer[i]&0x3F);
		}
		printf("\n");
	}
	
	inline void send()
	{
		int n = (Linkuino::PACKET_BYTES+Linkuino::CMD_COUNT-1) / Linkuino::CMD_COUNT ;
		for(int i=0;i<n;i++) { write(m_fd,m_buffer,Linkuino::CMD_COUNT); }
		updateTimeStamp();
	}

  private:

	int m_fd;
	uint8_t m_buffer[Linkuino::CMD_COUNT];
	uint8_t m_timeStamp;
};

int main(int argc, char* argv[])
{
	if(argc<2) { fprintf(stderr,"Usage: %s /dev/ttySomething\n",argv[0]); return 1; }

	int serial_fd = open( argv[1], O_RDWR | O_SYNC);	
	if( serial_fd < 0 ) { fprintf(stderr,"can't open device '%s'\n",argv[1]); return 1; }
	
	struct termios serialConfig;
	tcgetattr(serial_fd,&serialConfig);
	cfmakeraw(&serialConfig);
	cfsetspeed(&serialConfig, 57600);
	tcsetattr(serial_fd,TCSANOW,&serialConfig);
	
	LinkuinoClient link( serial_fd );
	
	double t=0.0;
	int p = 0;
	while( true )
	{
		for(int i=0;i<6;i++)
		{
			uint32_t x = sin(t+0.5*i) * 700.0 + 1250.0;
			link.setPWMValue( i , x );
		}
		link.printBuffer();
		link.send();
		t += 0.01;
	}

	return 0;
}
