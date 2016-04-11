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
		, m_bufSize(0)
		, m_clock(false)
		, m_timeStamp(0)
		, m_timeStampSwitch(false)
	{
		clear();
	}

	bool pushPWMCommand( int p, uint16_t value )
	{
		if( p<0 || p>5 ) return false;
		if(value<500) value = 500;
		else if(value>2000) value=2000;
		bool r;
		r = pushCommand( Linkuino::PWM0H_ADDR+2*p , value>>6 );
		r = r && pushCommand( Linkuino::PWM0L_ADDR+2*p , value & 0x3F );
		return r;
	}

	bool pushCommand(uint8_t cmd, uint8_t value)
	{
		if( m_cmdCount >= (Linkuino::MAX_COMMAND_BYTES-1) ) return false;
		m_cmd[ m_cmdCount++ ] = cmd;
		m_cmd[ m_cmdCount++ ] = value;
		return true;
	}
	
	int compileCommands()
	{
		int nRepeats=0;
		while( ! full() )
		{
			for(int i=0;i<m_cmdCount/2;i++)
			{
				addCommand( m_cmd[i*2+0] , m_cmd[i*2+1] );
			}
			addTimeStamp();
			++ nRepeats;
		}
		m_cmdCount = 0;
		return nRepeats;
	}

	inline void send()
	{
		//printBuffer();
		write(m_fd,m_buffer,m_bufSize);
		clear();
	}

  
	inline void printBuffer()
	{
		for(int i=0;i<m_bufSize/2;i++)
		{
			uint8_t x= m_buffer[i];
			printf("%c%c|%02X%c",(x&Linkuino::CLOCK_HIGH)?'+':'-', (x&Linkuino::RS_ADDR)?'A':'D',x&0x3F, (i%16)==15 ? '\n' : ' ' );
		}
		printf("\nsize=%d\n",m_bufSize);		
	}

  private:

	inline void clear()
	{
		m_timeStamp = (m_timeStamp+1) & 0x3F;
		m_bufSize = 0;
		m_cmdCount = 0;
		addTimeStamp();
	}

	bool full() const
	{ 
		return m_bufSize >= Linkuino::PACKET_BYTES;
	}

	inline void addTimeStamp()
	{
		addCommand( m_timeStampSwitch ? Linkuino::TSTMP1_ADDR : Linkuino::TSTMP0_ADDR, m_timeStamp);
		m_timeStampSwitch = ! m_timeStampSwitch;
	}

	inline void addCommand(uint8_t cmd, uint8_t value)
	{
		m_buffer[m_bufSize] = m_clock ? Linkuino::CLOCK_HIGH : Linkuino::CLOCK_LOW;
		m_buffer[m_bufSize] |= Linkuino::RS_ADDR;
		m_buffer[m_bufSize] |= cmd & 0x3F;
		++ m_bufSize;
		m_buffer[m_bufSize] = m_clock ? Linkuino::CLOCK_HIGH : Linkuino::CLOCK_LOW;
		m_buffer[m_bufSize] |= Linkuino::RS_DATA;
		m_buffer[m_bufSize] |= value & 0x3F;
		++ m_bufSize;
		m_clock = ! m_clock;
	}

	int m_fd;
	int m_bufSize;
	int m_cmdCount;
	uint8_t m_buffer[Linkuino::PACKET_BYTES+Linkuino::MAX_COMMAND_BYTES+2];
	uint8_t m_cmd[Linkuino::MAX_COMMAND_BYTES];
	uint8_t m_timeStamp;
	bool m_timeStampSwitch;
	bool m_clock;
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
			uint32_t x = sin(t+0.13*i) * 700.0 + 1250.0;
			link.pushPWMCommand( i , x );
		}
		//uint32_t x = sin(t) * 700.0 + 1250.0;
		//pbuf.pushPWMCommand( 0 , x );
		//pbuf.pushCommand( REQ_ADDR , REQ_DBG0_READ+p );
		int n = link.compileCommands();
		printf("R=%d\n",n);
		link.printBuffer();
		link.send();
		t += 0.01;
	}

	return 0;
}
