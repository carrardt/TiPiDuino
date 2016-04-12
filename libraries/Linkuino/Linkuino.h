#ifndef __TiDuino_Linkuino_h
#define __TiDuino_Linkuino_h

#include <stdint.h>

template<typename ElemT>
static inline void siftDown(ElemT numbers[], int root, int bottom)
{
  int maxChild = root * 2 + 1;
 
  // Find the biggest child
  if(maxChild < bottom) {
	int otherChild = maxChild + 1;
	// Reversed for stability
	maxChild = (numbers[otherChild] > numbers[maxChild])?otherChild:maxChild;
  } else {
	// Don't overflow
	if(maxChild > bottom) return;
  }
 
  // If we have the correct ordering, we are done.
  if(numbers[root] >= numbers[maxChild]) return;
 
  // Swap
  {
	ElemT temp = numbers[root];
	numbers[root] = numbers[maxChild];
	numbers[maxChild] = temp;
  }
 
  // Tail queue recursion. Will be compiled as a loop with correct compiler switches.
  siftDown<ElemT>(numbers, maxChild, bottom);
}

template<typename ElemT, int16_t array_size>
static inline void heapSort(ElemT numbers[])
{	 
  for (int i = (array_size / 2); i >= 0; i--) {
	siftDown<ElemT>(numbers, i, array_size - 1);
  }
 
  for (int i = array_size-1; i >= 1; i--) {
	// Swap
	ElemT temp = numbers[0];
	numbers[0] = numbers[i];
	numbers[i] = temp;
 
	siftDown<ElemT>(numbers, 0, i-1);
  }
}

struct NullPinGroup
{
	static void SetOutput(uint8_t mask) { }
	static void SetInput(uint8_t mask) {}
	static uint8_t Get() { return 0; }
	static void Set(uint8_t x , uint8_t mask=0xFF) { }
};

template<typename PWMPinGroupT=NullPinGroup, typename DPinGroupT=NullPinGroup, typename APinGroupT=NullPinGroup>
struct LinkuinoT /* Server */
{
	static constexpr uint8_t PWM_COUNT   			= 6;
	static constexpr uint8_t ANALOG_COUNT 			= 6;
	static constexpr uint8_t CMD_COUNT 	 			= 16;
	static constexpr uint8_t REPLY_BUFFER_SIZE 		= 16;
	static constexpr uint16_t SERIAL_SPEED 			= 57600;
	static constexpr uint16_t PACKET_BYTES      	= SERIAL_SPEED/1000; // 8+2 bits / byte, @ 57600 Bauds, to cover 10ms => 57.6 bytes
	static constexpr uint16_t MAX_COMMAND_BYTES 	= (PACKET_BYTES-4)/2; // has to be repeated at least twice and we add timestamp

	// Per byte flags
	static constexpr uint8_t CLOCK_LOW 	= 0x00;
	static constexpr uint8_t CLOCK_HIGH	= 0x80;
	static constexpr uint8_t RS_ADDR	= 0x40;
	static constexpr uint8_t RS_DATA	= 0x00;

	/******** Input register addresses **********/
	static constexpr uint8_t TSTMP0_ADDR 	= 0x00;
	static constexpr uint8_t PWM0H_ADDR 	= 0x01;
	static constexpr uint8_t PWM0L_ADDR 	= 0x02;
	static constexpr uint8_t PWM1H_ADDR 	= 0x03;
	static constexpr uint8_t PWM1L_ADDR 	= 0x04;
	static constexpr uint8_t PWM2H_ADDR 	= 0x05;
	static constexpr uint8_t PWM2L_ADDR 	= 0x06;
	static constexpr uint8_t PWM3H_ADDR 	= 0x07;
	static constexpr uint8_t PWM3L_ADDR 	= 0x08;
	static constexpr uint8_t PWM4H_ADDR 	= 0x09;
	static constexpr uint8_t PWM4L_ADDR 	= 0x0A;
	static constexpr uint8_t PWM5H_ADDR 	= 0x0B;
	static constexpr uint8_t PWM5L_ADDR 	= 0x0C;
	static constexpr uint8_t DOUT_ADDR 	 	= 0x0D;
	static constexpr uint8_t REQ_ADDR	 	= 0x0E;
	static constexpr uint8_t TSTMP1_ADDR 	= 0x0F;
	// ... maybe additional request slots to process several requests per cycle ...
	// pwm value input smoothing/filtering parameter

	/************** Request messages ****************/
	static constexpr uint8_t REQ_PWM0_DISABLE 	= 0x00;
	static constexpr uint8_t REQ_PWM1_DISABLE 	= 0x01;
	static constexpr uint8_t REQ_PWM2_DISABLE 	= 0x02;
	static constexpr uint8_t REQ_PWM3_DISABLE 	= 0x03;
	static constexpr uint8_t REQ_PWM4_DISABLE 	= 0x04;
	static constexpr uint8_t REQ_PWM5_DISABLE 	= 0x05;
	static constexpr uint8_t REQ_PWM0_ENABLE 	= 0x08;
	static constexpr uint8_t REQ_PWM1_ENABLE 	= 0x09;
	static constexpr uint8_t REQ_PWM2_ENABLE 	= 0x0A;
	static constexpr uint8_t REQ_PWM3_ENABLE 	= 0x0B;
	static constexpr uint8_t REQ_PWM4_ENABLE 	= 0x0C;
	static constexpr uint8_t REQ_PWM5_ENABLE 	= 0x0D;

	static constexpr uint8_t REQ_ANALOG0H_READ 	= 0x10;
	static constexpr uint8_t REQ_ANALOG0L_READ 	= 0x11;
	static constexpr uint8_t REQ_ANALOG1H_READ 	= 0x12;
	static constexpr uint8_t REQ_ANALOG1L_READ 	= 0x13;
	static constexpr uint8_t REQ_ANALOG2H_READ 	= 0x14;
	static constexpr uint8_t REQ_ANALOG2L_READ 	= 0x15;
	static constexpr uint8_t REQ_ANALOG3H_READ 	= 0x16;
	static constexpr uint8_t REQ_ANALOG3L_READ 	= 0x17;
	static constexpr uint8_t REQ_ANALOG4H_READ 	= 0x18;
	static constexpr uint8_t REQ_ANALOG4L_READ 	= 0x19;
	static constexpr uint8_t REQ_ANALOG5H_READ 	= 0x1A;
	static constexpr uint8_t REQ_ANALOG5L_READ 	= 0x1B;

	static constexpr uint8_t REQ_DIGITAL_READ 	= 0x20;
	static constexpr uint8_t REQ_DBG0_READ 		= 0x21;
	static constexpr uint8_t REQ_DBG1_READ 		= 0x22;
	static constexpr uint8_t REQ_DBG2_READ 		= 0x23;
	static constexpr uint8_t REQ_DBG3_READ 		= 0x24;
	static constexpr uint8_t REQ_DBG4_READ 		= 0x25;
	static constexpr uint8_t REQ_DBG5_READ 		= 0x26;
	
	static constexpr uint8_t REQ_RESET		  	= 0x3F;

	inline LinkuinoT()
		: m_pwmOutput()
		, m_digitalOutput()
		, m_analogInput()
	{
		m_pwmMask = 0x3F; // we use only 6 bits
		m_pwmOutput.SetOutput( m_pwmMask );
		m_pwmSeqIndex = 0;
		m_pwmSeqLen = 1;
		m_pwm[0] = 1250;
		m_pwmShutDown[0] = ~m_pwmMask;
		
		m_cmd[DOUT_ADDR] = 0;
		for(uint8_t i=0; i<PWM_COUNT; i++)
		{
			m_cmd[2*i+1] = 0x08;
			m_cmd[2*i+2] = 0x00;
		}
	}

	inline void prepareToReceive()
	{
		m_addr = 0;
		m_clk = 255;
		m_cmd[TSTMP0_ADDR] = 255;
		m_cmd[REQ_ADDR] = 255;
		m_cmd[TSTMP1_ADDR] = 255;
	}

	template<typename IODeviceT>
	inline void receive(IODeviceT& io)
	{
		uint16_t x = io.readByteAsync();
		if( x )
		{
			uint8_t r = x;
			uint8_t data = r & 0x3F;
			r >>= 6;
			uint8_t nRS = r & 0x01;
			uint8_t nCLK = r >> 1;
			if(nRS) { m_addr = data; }
			else if(nCLK==m_clk) { m_cmd[m_addr] = data; }
			m_clk = nCLK;
		}
	}

	inline bool receiveComplete()
	{
		return (m_cmd[TSTMP0_ADDR] != 255) && (m_cmd[TSTMP0_ADDR] == m_cmd[TSTMP1_ADDR]);
	}

	inline void process()
	{
		// check received data is complete
		if( ! receiveComplete() ) return;

		// process PWM commands
		for(uint8_t i=0;i<PWM_COUNT;i++) { m_pwm[i] = makePwmDesc(i); }
		heapSort<uint16_t,PWM_COUNT>( m_pwm );
		m_pwmSeqLen = 0;
		m_pwmShutDown[0] = 1 << ( m_pwm[0] & 0x07 );
		m_pwm[0] >>= 3;
		for(int i=1;i<PWM_COUNT;i++)
		{
			uint16_t value = m_pwm[i] >> 3;
			uint8_t mask = 1 << (m_pwm[i] & 0x07);
			if( value > m_pwm[m_pwmSeqLen] )
			{
				++m_pwmSeqLen;
				m_pwmShutDown[m_pwmSeqLen] = m_pwmShutDown[m_pwmSeqLen-1];
				m_pwm[m_pwmSeqLen] = value;
			}
			m_pwmShutDown[m_pwmSeqLen] |= mask;
		}
		++ m_pwmSeqLen;
		for(int i=0;i<m_pwmSeqLen;i++) { m_pwmShutDown[i] = ~m_pwmShutDown[i]; }

		if( m_cmd[REQ_ADDR]!=255 )
		{
			if( m_cmd[REQ_ADDR]>=REQ_DBG0_READ && m_cmd[REQ_ADDR]<=REQ_DBG5_READ )
			{
				//uint8_t p = m_cmd[REQ_ADDR]-REQ_DBG0_READ;
				//cout<<"PWM"<<p<<'='<< (m_pwm[p]>>3) << '/' << (m_pwm[p]&0x07) << endl;
			}
		}
	}

	inline void allPwmHigh()
	{
		m_pwmOutput.Set( m_pwmMask, m_pwmMask );
		m_pwmSeqIndex = 0;
	}

	// t is time in uS since pulse start
	inline void shutDownPWM(uint16_t t)
	{
		if( t >= m_pwm[m_pwmSeqIndex] )
		{
			m_pwmOutput.Set( m_pwmShutDown[m_pwmSeqIndex], m_pwmMask );
			//m_pwmOutput.Set( 0, m_pwmShutDown[m_pwmSeqIndex] );
			if( (m_pwmSeqIndex+1) < m_pwmSeqLen ) { ++ m_pwmSeqIndex; }
		}
	}

	inline void allPwmLow()
	{
		m_pwmOutput.Set( 0, m_pwmMask );
	}

	// return 12 bits value + 3 bits pin number (port bit position)
	inline uint16_t makePwmDesc(uint8_t i)
	{
		uint16_t v = m_cmd[PWM0H_ADDR+i*2];
		v <<= 6;
		v |= m_cmd[PWM0L_ADDR+i*2];
		v <<= 3;
		v |= i & 0x07;
		return v;
	}

	uint8_t m_addr;
	uint8_t m_clk;
	uint8_t m_cmd[CMD_COUNT];
	uint8_t m_reply[REPLY_BUFFER_SIZE];
	uint8_t m_replyStart;
	uint8_t m_replyEnd;
	uint8_t m_dmask;
	uint8_t m_dout;
	uint8_t m_din;
	uint8_t m_pwmMask; // change name to PortMask or something
	uint8_t m_pwmSeqLen;
	uint8_t m_pwmSeqIndex;
	uint8_t m_pwmShutDown[PWM_COUNT];
	uint16_t m_pwm[PWM_COUNT];
	
	PWMPinGroupT m_pwmOutput;
	DPinGroupT m_digitalOutput;
	APinGroupT m_analogInput;
};

#endif
