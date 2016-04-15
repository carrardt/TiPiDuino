#ifndef __TiDuino_Linkuinoclient_h
#define __TiDuino_Linkuinoclient_h

#include <cstdint>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

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
		setRegisterValue(Linkuino::PWMSMTH_ADDR, 0);
	}

	static inline int openSerialDevice(const char* devpath)
	{
		int serial_fd = open( devpath, O_RDWR | O_SYNC);	
		if( serial_fd < 0 ) return -1;
		struct termios serialConfig;
		tcgetattr(serial_fd,&serialConfig);
		cfmakeraw(&serialConfig);
		cfsetspeed(&serialConfig, 57600);
		tcsetattr(serial_fd,TCSANOW,&serialConfig);
		return serial_fd;
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

#endif
