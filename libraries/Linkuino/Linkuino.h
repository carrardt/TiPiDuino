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


/*
 * Restrictions :
 * 8-bits pin ports
 * contiguous bits in the same port for PWM pins
 * contiguous bits in the same port for Digital out pins
 * contiguous bits in the same port for Digital in pins
 */
struct LinkuinoNullTraits
{
	using PWMPinGroupT = NullPinGroup;
	using DOutPinGroupT = NullPinGroup;
	using DInPinGroupT = NullPinGroup;
	
	static constexpr uint8_t PWMPortFirstBit = 0;
	static constexpr uint8_t PWMPortMask = 0x3F;

	static constexpr uint8_t DOutPortFirstBit = 0;
	static constexpr uint8_t DOutPortMask = 0x3F;

	static constexpr uint8_t DInPortFirstBit = 0;
	static constexpr uint8_t DInPortMask = 0x3F;

	static inline void selectAnalogChannel(uint8_t ch) {}
	static inline void analogAcquire() {}
	static inline uint16_t analogRead() { return 0; }
};

template<typename _LinkuinoTraits=LinkuinoNullTraits>
struct LinkuinoT /* Server */
{
	/******** Hardware ports configuration **********/
	using LinkuinoTraits = _LinkuinoTraits;
	using PWMPinGroupT = typename LinkuinoTraits::PWMPinGroupT;
	using DOutPinGroupT = typename LinkuinoTraits::DOutPinGroupT;
	using DInPinGroupT = typename LinkuinoTraits::DInPinGroupT;
	
	static constexpr uint8_t PWMPortMask = LinkuinoTraits::PWMPortMask;
	static constexpr uint8_t PWMPortFirstBit = LinkuinoTraits::PWMPortFirstBit;

	static constexpr uint8_t DOutPortMask = LinkuinoTraits::DOutPortMask;
	static constexpr uint8_t DOutPortFirstBit = LinkuinoTraits::DOutPortFirstBit;

	static constexpr uint8_t DInPortMask = LinkuinoTraits::DInPortMask;
	static constexpr uint8_t DInPortFirstBit = LinkuinoTraits::DInPortFirstBit;

	/**************** Communication settings ***********/
	static constexpr uint8_t PWM_COUNT   			= 6;
	static constexpr uint8_t ANALOG_COUNT 			= 6;
	static constexpr uint8_t CMD_COUNT 	 			= 16;
	static constexpr uint8_t REPLY_BUFFER_SIZE 		= 2;
	static constexpr uint16_t SERIAL_SPEED 			= 57600;
	static constexpr uint16_t PACKET_BYTES      	= 64; //SERIAL_SPEED/1000; // 8+2 bits / byte, @ 57600 Bauds, to cover 10ms => 57.6 bytes

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
	static constexpr uint8_t PWMSMTH_ADDR 	= 0x0F; // smoothing factor : [0;3] 0 means no smoothing

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
	
	static constexpr uint8_t REQ_NOOP		  	= 0x30;
	static constexpr uint8_t REQ_RESET		  	= 0x3F;

	inline LinkuinoT()
		: m_pwmOutput()
		, m_digitalOutput()
		, m_digitalInput()
	{
		m_pwmPortMask = PWMPortMask;
		m_pwmOutput.SetOutput( m_pwmPortMask << PWMPortFirstBit );
		m_pwmSeqIndex = 0;
		m_pwmSeqLen = 1;
		m_pwm[0] = 1250;
		m_pwmShutDown[0] = ~m_pwmPortMask;
		
		m_digitalOutput.SetOutput( DOutPortMask << DOutPortFirstBit );
		m_doutMask = DOutPortMask;
		
		m_digitalInput.SetInput( DInPortMask << DInPortFirstBit );
		m_dinMask = DInPortMask;
		m_din = 0;
		
		m_bufferIndex = 0;
		m_readCycles = 0;
		m_buffer[TSTMP0_ADDR] = 255;
		for(uint8_t i=0; i<PWM_COUNT; i++)
		{
			m_buffer[2*i+1] = 0x08;
			m_buffer[2*i+2] = 0x00;
		}
		m_buffer[DOUT_ADDR] = 0;
		m_buffer[REQ_ADDR] = 255;
		m_buffer[PWMSMTH_ADDR] = 0;
		
		m_reply[0] = 'O';
		m_reply[1] = 'k';
		m_reply[2] = '\n';
	}

	inline void prepareToReceive()
	{
		m_buffer[TSTMP0_ADDR] = 255;
		m_buffer[REQ_ADDR] = 255;
		m_readCycles = 0;
	}

	template<typename IODeviceT>
	inline void receive(IODeviceT& io)
	{
		uint16_t x = io.readByteAsync();
		if( x )
		{
			uint8_t r = x;
			uint8_t data = r & 0x3F;
			uint8_t clk = r >> 6;
			if( clk == 0 )
			{
				if( m_buffer[TSTMP0_ADDR] == data ) ++ m_readCycles;
				else m_readCycles = 0;
				m_buffer[TSTMP0_ADDR] = data;
				m_bufferIndex = 1;
			}
			else
			{
				m_buffer[m_bufferIndex] = r;
				m_bufferIndex = (m_bufferIndex+1) % CMD_COUNT;
			}
		}
	}

	template<typename IODeviceT>
	inline void reply(IODeviceT& io)
	{
		// @ 57600 bauds, each byte takes about 180-200 uS to send
		io.writeByte(m_reply[0]);
		io.writeByte(m_reply[1]);
		io.writeByte(m_reply[2]);
	}

	inline bool receiveComplete()
	{
		if( m_readCycles == 0 ) return false;
		uint8_t check = 0;
		for(int i=0;i<5;i++)
		{
			m_buffer[i*3+1] = m_buffer[i*3+1]^0x40;
			m_buffer[i*3+2] = m_buffer[i*3+2]^0x80;
			m_buffer[i*3+3] = m_buffer[i*3+3]^0xC0;
		}
		for(int i=1;i<16;i++)
		{
			if( ( m_buffer[i] & 0xC0 ) != 0 ) return false;
		}
		return true;
	}

	inline void process()
	{
		// check received data is complete
		if( ! receiveComplete() ) return;

		// process PWM commands, sort values, prepare next cycle pulses sequence
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

		// write Digital output updated values
		m_digitalOutput.Set( ( m_buffer[DOUT_ADDR] & m_doutMask ) << DOutPortFirstBit , DOutPortMask << DOutPortFirstBit );

		// read digital input state
		m_din = ( m_digitalInput.Get() >> DInPortFirstBit ) & m_dinMask;

		if( m_buffer[REQ_ADDR]!=255 )
		{
			if( m_buffer[REQ_ADDR] == REQ_DIGITAL_READ )
			{
				m_reply[0] = REQ_DIGITAL_READ;
				m_reply[1] = m_din;
				m_reply[2] = 0;
			}
		}
	}

	inline void allPwmHigh()
	{
		m_pwmOutput.Set( m_pwmPortMask << PWMPortFirstBit , m_pwmPortMask  << PWMPortFirstBit );
		m_pwmSeqIndex = 0;
	}

	// t is time in uS since pulse start
	inline void shutDownPWM(uint16_t t)
	{
		if( t >= m_pwm[m_pwmSeqIndex] )
		{
			m_pwmOutput.Set( m_pwmShutDown[m_pwmSeqIndex] << PWMPortFirstBit , m_pwmPortMask << PWMPortFirstBit );
			if( (m_pwmSeqIndex+1) < m_pwmSeqLen ) { ++ m_pwmSeqIndex; }
		}
	}

	inline void allPwmLow()
	{
		m_pwmOutput.Set( 0, m_pwmPortMask << PWMPortFirstBit );
	}

	// return 12 bits value + 3 bits pin number (port bit position)
	inline uint16_t makePwmDesc(uint8_t i)
	{
		uint16_t v = m_buffer[PWM0H_ADDR+i*2];
		v <<= 6;
		v |= m_buffer[PWM0L_ADDR+i*2];
		v <<= 3;
		v |= i & 0x07;
		return v;
	}

	// PWM state
	uint16_t m_pwm[PWM_COUNT];
	uint8_t m_pwmShutDown[PWM_COUNT];
	uint8_t m_pwmPortMask;
	uint8_t m_pwmSeqLen;
	uint8_t m_pwmSeqIndex;

	// Digital output state
	uint8_t m_doutMask;

	// Digital input state
	uint8_t m_dinMask;
	uint8_t m_din;

	// receive buffer
	uint8_t m_buffer[CMD_COUNT];
	uint8_t m_bufferIndex;
	uint8_t m_readCycles;
	
	// reply buffer
	uint8_t m_reply[3];

	// hardware pins accessors
	PWMPinGroupT m_pwmOutput;
	DOutPinGroupT m_digitalOutput;
	DInPinGroupT m_digitalInput;
};

#endif
