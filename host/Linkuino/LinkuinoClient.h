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
	static constexpr int PacketRepeatCount = (Linkuino::PACKET_BYTES+Linkuino::CMD_COUNT-1) / Linkuino::CMD_COUNT ;
	
	inline LinkuinoClient(int fd)
		: m_fd(fd)
		, m_timeStamp(0)
	{
		setRegisterValue(Linkuino::TSTMP_ADDR, 0);
		for(int i=0;i<Linkuino::PWM_COUNT;i++) { setPWMValue(i, 1250); }
		setRegisterValue(Linkuino::DOUT_ADDR, 0);
		setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_NULL);
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
		// PWM cycle are 10mS long
		// pulse length can be 0 (always LOW), 10mS (always HIGH) or in the range [400;9600]
		// encoding : values in range[0;1536] encode pulse lengths in [400;1936]
		// values in range [1536;4095] encode pulse lengths in [1936;9613]
		if( p<0 || p>5 ) return ;
		if( value > 1536 )
		{
			uint16_t extValue = 1536 + ( (value-1536)/3 );
		}
		setRegisterValue( Linkuino::PWM0H_ADDR+2*p , (value >> 6) & 0x3F );
		setRegisterValue( Linkuino::PWM0L_ADDR+2*p , value & 0x3F );
	}

	inline void updateTimeStamp()
	{
		m_timeStamp = (m_timeStamp+1) & 0x3F;
		setRegisterValue( Linkuino::TSTMP_ADDR, m_timeStamp );
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
		for(int i=0;i<PacketRepeatCount;i++) { write(m_fd,m_buffer,Linkuino::CMD_COUNT); }
		write(m_fd,m_buffer,1); // finish with a timestamp marker
		fsync(m_fd);
		updateTimeStamp();
	}

  private:

	int m_fd;
	uint8_t m_buffer[Linkuino::CMD_COUNT];
	uint8_t m_timeStamp;
};

#endif
