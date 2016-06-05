#ifndef __TiDuino_Linkuinoclient_h
#define __TiDuino_Linkuinoclient_h

#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "Linkuino/Linkuino.h"
#include <string>
#include <iostream>
using Linkuino = LinkuinoT<>;

struct LinkuinoClient
{
	static constexpr int PacketRepeatCount = Linkuino::MESSAGE_REPEATS ;
	
	inline LinkuinoClient(int fd)
		: m_fd(fd)
		, m_timeStamp(0)
		, m_serverMajor(0)
		, m_serverMinor(0)
	{
		setRegisterValue(Linkuino::TSTMP_ADDR, 0);
		for(int i=0;i<Linkuino::PWM_COUNT;i++) { setPWMValue(i, 1250); m_pwmEnabled[i]=false; }
		setRegisterValue(Linkuino::PWMSMTH_ADDR, 0);
		setRegisterValue(Linkuino::DOUT_ADDR, 0);
		setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_NULL);
		setRegisterValue(Linkuino::REQ_DATA0_ADDR, 0);
		setRegisterValue(Linkuino::REQ_DATA1_ADDR, 0);
		setRegisterValue(Linkuino::REQ_DATA2_ADDR, 0);
		setRegisterValue(Linkuino::REQ_DATA3_ADDR, 0);
	}
	
	inline ~LinkuinoClient()
	{
		close(m_fd);
		m_fd = -1;
	}

	inline int getServerVersionMajor() const {return m_serverMajor; }
	inline int getServerVersionMinor() const {return m_serverMinor; }

	static inline int openSerialDevice(const std::string& devpath)
	{
		int serial_fd = open( devpath.c_str(), O_RDWR | O_SYNC | O_NONBLOCK);	
		if( serial_fd < 0 ) return -1;
		struct termios serialConfig;
		tcgetattr(serial_fd,&serialConfig);
		cfmakeraw(&serialConfig);
		cfsetspeed(&serialConfig, Linkuino::SERIAL_SPEED );
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

	void enablePWM(int p)
	{
		setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_PWM0_ENABLE+p);
		send();
		m_pwmEnabled[p] = true;
	}

	void disablePWM(int p)
	{
		setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_PWM0_DISABLE+p);
		send();
		m_pwmEnabled[p] = false;
	}

	void setPWMValue( int p, uint16_t value )
	{
		// PWM cycle are 10mS long
		// pulse length can be 0 (always LOW), 10mS (always HIGH) or in the range [400;9600]
		// encoding : values in range[0;1536] encode pulse lengths in [400;1936]
		// values in range [1536;4095] encode pulse lengths in [1936;9613]
		if( p<0 || p>5 ) return ;
		if( value>=400 && !m_pwmEnabled[p] ) { enablePWM(p); }
		if( value<400 && m_pwmEnabled[p]) { disablePWM(p); }
		uint16_t valueEnc = Linkuino::encodePulseLength( value );
		//printf("PWM%d : %d => %d\n",p,value, valueEnc);
		setRegisterValue( Linkuino::PWM0H_ADDR+2*p , (valueEnc >> 6) & 0x3F );
		setRegisterValue( Linkuino::PWM0L_ADDR+2*p , valueEnc & 0x3F );
	}

	inline void updateTimeStamp()
	{
		m_timeStamp = (m_timeStamp+1) & 0x3F;
		setRegisterValue( Linkuino::TSTMP_ADDR, m_timeStamp );
	}
	
	bool testConnection()
	{
		const double timeoutNanoSecs = 1.e9;
		
		setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_REV);
		send();
		send();

		struct timespec T0, T1;
		clock_gettime(CLOCK_REALTIME,&T0);

		char reply[256];
		int R=32;
		int l=0;
		while( l<R )
		{
			clock_gettime(CLOCK_REALTIME,&T1);
			if( ((T1.tv_sec-T0.tv_sec)*1.e9+T1.tv_nsec-T0.tv_nsec) > timeoutNanoSecs)
			{ 
				reply[l]='\0';
				printf("Linkuino client : Timeout, reply='%s'\n",reply);
				return false;
			}
			int n = read(m_fd,reply+l,R-l);
			if( n>0 ) l += n;
		}

		setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_NOREPLY);
		send();
		send();

		reply[R-1]='\0';
		//printf("REQ_REV='%s'\n",reply);
		l=0;
		while( reply[l]!=Linkuino::REQ_REV && l<R ) ++l;
		if( l>=(R-2) || reply[l]!=Linkuino::REQ_REV ) { return false; }
		m_serverMajor = reply[l+1];
		m_serverMinor = reply[l+2];
		return true;
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
		// TODO: rather than waiting for 10mS, we should just check that last send terminated at least 10mS earlier and wait just enough
		struct timespec st = { 0 , 10000000UL }; // 10 milliseconds delay
		struct timespec rem = { 0 , 0 };
		int r = 0;
		for(int i=0;i<PacketRepeatCount;i++)
		{
			write(m_fd,m_buffer,Linkuino::CMD_COUNT);
		}
		write(m_fd,m_buffer,1); // finish with a timestamp marker
		fsync(m_fd);
		r = nanosleep( &st , &rem );
		while( r == EINTR )
		{
			st = rem;
			rem = {0,0};
			fprintf(stderr,"Warning: nanosleep interrupted\n");
			r = nanosleep( &st , &rem );
		}
		if( r==EFAULT || r==EINVAL)
		{
			fprintf(stderr,"ERROR: nanosleep failed\n");
		}
		updateTimeStamp();
	}

  private:

	int m_fd;
	int m_serverMajor;
	int m_serverMinor;
	uint8_t m_buffer[Linkuino::CMD_COUNT];
	uint8_t m_timeStamp;
	bool m_pwmEnabled[Linkuino::PWM_COUNT];
};

#endif
