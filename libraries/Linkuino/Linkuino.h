#ifndef __TiDuino_Linkuino_h
#define __TiDuino_Linkuino_h

#include <stdint.h>
#include "Linkuino/heapsort.h"

// define this to have 100Hz (10ms) cycles. otherwise, standard 50Hz (20ms) cycles are used.
//#define LINKUINO_FREQ_100HZ 1

struct LkNullPinGroup
{
	static void SetOutput(uint8_t mask) { }
	static void SetInput(uint8_t mask) {}
	static uint8_t Get() { return 0; }
	static void Set(uint8_t x , uint8_t mask=0xFF) { }
};

struct LkNullPin
{
	static void SetOutput() {}
	static void SetInput() {}
	static bool Get() { return false; }
	static void Set(bool x) { }
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
	using PWMPinGroupT = LkNullPinGroup;
	using DOutPinGroupT = LkNullPinGroup;
	using DInPinGroupT = LkNullPinGroup;
	using ForwardSerialPinT = LkNullPin;

	static constexpr uint8_t PWMPortFirstBit = 0;
	static constexpr uint8_t PWMPortMask = 0x3F;

	static constexpr uint8_t DOutPortFirstBit = 0;
	static constexpr uint8_t DOutPortMask = 0x3F;

	static constexpr uint8_t DInPortFirstBit = 0;
	static constexpr uint8_t DInPortMask = 0x3F;

	static inline void selectAnalogChannel(uint8_t ch) {}
	static inline void analogAcquire() {}
	
	// 10 bit value is expected
	static inline uint16_t analogRead() { return 0; }
};

template<typename _LinkuinoTraits=LinkuinoNullTraits, uint8_t _CyclePeriodScale=2>
struct LinkuinoT /* Server */
{
	/******** Hardware ports configuration **********/
	using LinkuinoTraits = _LinkuinoTraits;
	using PWMPinGroupT = typename LinkuinoTraits::PWMPinGroupT;
	using DOutPinGroupT = typename LinkuinoTraits::DOutPinGroupT;
	using DInPinGroupT = typename LinkuinoTraits::DInPinGroupT;
	
	static constexpr uint8_t PWMPortMask = LinkuinoTraits::PWMPortMask ;
	static constexpr uint8_t PWMPortFirstBit = LinkuinoTraits::PWMPortFirstBit;

	static constexpr uint8_t DOutPortMask = LinkuinoTraits::DOutPortMask;
	static constexpr uint8_t DOutPortFirstBit = LinkuinoTraits::DOutPortFirstBit;

	static constexpr uint8_t DInPortMask = LinkuinoTraits::DInPortMask;
	static constexpr uint8_t DInPortFirstBit = LinkuinoTraits::DInPortFirstBit;

	static constexpr uint8_t CYCLE_PERIOD_SCALE = _CyclePeriodScale; // period is CyclePeriodScale x 10ms . should be 1 (100Hz) or 2 (50Hz)

	static constexpr uint16_t PWM_DEFAULT_VALUE     = 1500;
	static constexpr uint16_t PWM_UNREACHABLE_VALUE = 65535;

	static constexpr uint8_t NO_RESET = 0;
	static constexpr uint8_t RESET_TO_50Hz = 1;
	static constexpr uint8_t RESET_TO_100Hz = 2;

	/*************** Server version ********************/
	static constexpr uint8_t REV_MAJOR = 1;
	static constexpr uint8_t REV_MINOR = 5;

	/**************** Communication settings ***********/
	static constexpr uint8_t PWM_COUNT   			= 6;
	static constexpr uint8_t ANALOG_COUNT 			= 6;
	static constexpr uint8_t CMD_COUNT 	 			= 20;
	static constexpr uint8_t REPLY_BUFFER_SIZE 		= 2;
	static constexpr uint32_t SERIAL_SPEED 			= 115200;
	static constexpr uint32_t PACKET_BYTES      	= (CYCLE_PERIOD_SCALE*SERIAL_SPEED)/1000; //SERIAL_SPEED/1000; // 8+2 bits / byte, @ 57600 Bauds, to cover 10ms => 57.6 bytes
	static constexpr uint32_t MESSAGE_REPEATS		= (PACKET_BYTES+CMD_COUNT-1)/CMD_COUNT; //SERIAL_SPEED/1000; // 8+2 bits / byte, @ 57600 Bauds, to cover 10ms => 57.6 bytes

	/******** Input register addresses **********/
	static constexpr uint8_t TSTMP_ADDR 	= 0x00; // Timestamp, cyclic in the range [0;63]
	static constexpr uint8_t PWM0H_ADDR 	= 0x01; // PWM #0, 6 MSB
	static constexpr uint8_t PWM0L_ADDR 	= 0x02; // PWM #0, 6 LSB
	static constexpr uint8_t PWM1H_ADDR 	= 0x03; // PWM #1, 6 MSB
	static constexpr uint8_t PWM1L_ADDR 	= 0x04; // PWM #1, 6 LSB
	static constexpr uint8_t PWM2H_ADDR 	= 0x05; // PWM #2, 6 MSB
	static constexpr uint8_t PWM2L_ADDR 	= 0x06; // PWM #2, 6 LSB
	static constexpr uint8_t PWM3H_ADDR 	= 0x07; // PWM #3, 6 MSB
	static constexpr uint8_t PWM3L_ADDR 	= 0x08; // PWM #3, 6 LSB
	static constexpr uint8_t PWM4H_ADDR 	= 0x09; // PWM #4, 6 MSB
	static constexpr uint8_t PWM4L_ADDR 	= 0x0A; // PWM #4, 6 LSB
	static constexpr uint8_t PWM5H_ADDR 	= 0x0B; // PWM #5, 6 MSB
	static constexpr uint8_t PWM5L_ADDR 	= 0x0C; // PWM #5, 6 LSB
	static constexpr uint8_t PWMEN_ADDR 	= 0x0D; // PWM enable bits
	static constexpr uint8_t DOUT_ADDR 	 	= 0x0E; // digital output pins
	static constexpr uint8_t REQ_ADDR	 	= 0x0F; // Request ID
	static constexpr uint8_t REQ_DATA0_ADDR	= 0x10; // Request DATA 0
	static constexpr uint8_t REQ_DATA1_ADDR	= 0x11; // Request DATA 1
	static constexpr uint8_t REQ_DATA2_ADDR	= 0x12; // Request DATA 2
	static constexpr uint8_t REQ_DATA3_ADDR	= 0x13; // Request DATA 3

	/************** Request messages ****************/
	static constexpr uint8_t REQUEST_COUNT		= 8;
	static constexpr uint8_t REQ_ANALOG_READ 	= 0x00; // read one analog input. input number given in request data
	static constexpr uint8_t REQ_DIGITAL_READ 	= 0x01; // read digital pins
	static constexpr uint8_t REQ_FWD_SERIAL	    = 0x02; // forward bits in request data to slave (implementation dependent)
	static constexpr uint8_t REQ_RESET		  	= 0x03; // reset to 50Hz or 100Hz mode depending on code stored at REQ_DATA0_ADDR
	static constexpr uint8_t REQ_REV		  	= 0x04; // sends 0x00, version major-1 | message repeats<<2 (!=0) , version minor+1 (>=1)
	static constexpr uint8_t REQ_NOREPLY		= 0x05; // stop replying messages
	static constexpr uint8_t REQ_NO_OP			= 0x06; // do nothing (don't change reply status)
	static constexpr uint8_t REQ_ACK		  	= 0x07; // i.e. ACKnowledge => sends 'Ok\n'
	static constexpr uint8_t REQ_NULL		  	= 0xFF; // no request received

	/* reply signatures */
	static constexpr uint8_t REPLY_NULL			= 0x00;
	static constexpr uint8_t REPLY_ANALOG_READ 	= 0xAA;
	static constexpr uint8_t REPLY_DIGITAL_READ = 0xDD;

	inline LinkuinoT()
		: m_pwmOutput()
		, m_digitalOutput()
		, m_digitalInput()
	{}

	void begin()
	{
		m_pwmPortMask = 0 ;
		m_pwmOutput.SetOutput( PWMPortMask  );
		m_pwmSeqIndex = 0;
		m_pwmSeqLen = 0;
		m_pwmShutDown[0] = 0;
		m_pwm[PWM_COUNT] = PWM_UNREACHABLE_VALUE;
		
		m_digitalOutput.SetOutput( DOutPortMask );
		m_doutMask = DOutPortMask;
		
		m_digitalInput.SetInput( DInPortMask );
		m_dinMask = DInPortMask;
		m_din = 0;
		
		m_bufferIndex = 0;
		m_readCycles = 0;
		m_buffer[TSTMP_ADDR] = 255;
		for(uint8_t i=0; i<PWM_COUNT; i++)
		{
			m_buffer[2*i+1] = 0x08;
			m_buffer[2*i+2] = 0x00;
			m_pwm[i] = PWM_DEFAULT_VALUE;
		}
		m_buffer[DOUT_ADDR] = 0;
		m_buffer[REQ_ADDR] = REQ_NULL;
		m_buffer[PWMEN_ADDR] = 0;
		
		m_replyEnable = false;
		m_reply[0] = 'O';
		m_reply[1] = 'k';
		m_reply[2] = '\n';

		m_fwdEnable = false;
		m_fwd = 0;
		
		m_resetRequest = NO_RESET;
	}

	inline void prepareToReceive()
	{
		m_buffer[TSTMP_ADDR] = 255;
		// default is to reply to the last request unless NO_REPLY request is received
		// so we do not overwrite REQ_ADDR until we receive a valid message overwriting it
		//m_buffer[REQ_ADDR] = REQ_NULL;
		m_readCycles = 0;
		//m_replyEnable = false;
		m_fwdEnable = false;
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
				if( m_buffer[TSTMP_ADDR] == data ) ++ m_readCycles;
				else m_readCycles = 0;
				m_buffer[TSTMP_ADDR] = data;
				m_bufferIndex = 1;
			}
			else
			{
				m_buffer[m_bufferIndex] = r;
				m_bufferIndex = (m_bufferIndex+1) % CMD_COUNT;
			}
		}
	}

	template<typename IODeviceT, typename FwdSerialT>
	inline void reply(IODeviceT& io, FwdSerialT& fwdSerial)
	{
		if( m_replyEnable )
		{
			io.writeByte(m_reply[0]);
			io.writeByte(m_reply[1]);
			io.writeByte(m_reply[2]);
		}
		if( m_fwdEnable )
		{
			fwdSerial.write24( m_fwd );
		}
	}

	inline bool receiveComplete()
	{
		if( m_readCycles == 0 ) return false;
		uint8_t clk=0;
		for(int i=1;i<CMD_COUNT;i++)
		{
			uint8_t cm = (clk+1)<<6;
			m_buffer[i] = m_buffer[i]^cm;
			if( ( m_buffer[i] & 0xC0 ) != 0 ) { return false; }
			++clk;
			if(clk==3) clk=0;
		}
		return true;
	}

	static inline uint16_t encodePulseLength(uint16_t length)
	{
		uint16_t r;
		if( length > 400 )
		{
			length -= 400;
			if(length>1536)
			{
				if(length>9213) { length = 9213; }
				r = 1536 + ( (length-1536)/3 );
			}
			else { r = length; }
		}
		else { r = 0; }
		return r & 0x0FFF;
	}
	
	static inline uint16_t decodePulseLength(uint16_t length)
	{
		if(length<=1536) return length+400;
		return (length-1536)*3 + 1936;
	}

	inline void process()
	{
		// check received data is complete
		if( ! receiveComplete() )
		{
			return;
		}

		// prepare request processing
		uint8_t req = m_buffer[REQ_ADDR];
		if( req >= REQUEST_COUNT ) { req = REQ_NO_OP; }
		m_fwdEnable = false;
		((this)->*(m_requestDispatchTable[REQUEST_COUNT+req]))();

		// process PWM commands, sort values, prepare next cycle's pulses sequence

		// copy pwm enable mask to pwm port mask
		uint8_t pwmMaskReg = m_buffer[PWMEN_ADDR] & 0x3F ;
		m_pwmPortMask = pwmMaskReg << PWMPortFirstBit;

		// sort pwm values to lower in-cycle processing time
		for(uint8_t i=0;i<PWM_COUNT;i++) { m_pwm[i] = makePwmDesc(i); }
		heapSort<uint16_t,PWM_COUNT>( m_pwm );
		m_pwmSeqLen = 0;
		m_pwmShutDown[0] = ~m_pwmPortMask;
		m_pwmShutDown[0] |= 1 << ( ( m_pwm[0] & 0x07 ) + PWMPortFirstBit );
		m_pwm[0] = decodePulseLength( m_pwm[0] >> 3 ) * CYCLE_PERIOD_SCALE;
		for(int i=1;i<PWM_COUNT;i++)
		{
			uint16_t value = decodePulseLength( m_pwm[i] >> 3 ) * CYCLE_PERIOD_SCALE;
			uint8_t mask = 1 << ( (m_pwm[i] & 0x07) + PWMPortFirstBit );
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
		for(int i=m_pwmSeqLen;i<=PWM_COUNT;i++) { m_pwm[i] = PWM_UNREACHABLE_VALUE; }

		// write Digital output updated values
		m_digitalOutput.Set( ( m_buffer[DOUT_ADDR] << DOutPortFirstBit ) & m_doutMask , m_doutMask );

		// processs request, if any
		((this)->*(m_requestDispatchTable[req]))();
	}

	inline void allPwmHigh()
	{
		m_pwmOutput.Set( m_pwmPortMask , m_pwmPortMask );
		m_pwmSeqIndex = 0;
	}

	inline void allPwmLow()
	{
		m_pwmOutput.Set( 0, m_pwmPortMask );
	}

	// t is time in uS since pulse start
	inline void shutDownPWM(uint16_t t)
	{
		if( t >= m_pwm[m_pwmSeqIndex] )
		{
			m_pwmOutput.Set( m_pwmShutDown[m_pwmSeqIndex] , m_pwmPortMask );
			++ m_pwmSeqIndex;
		}
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

	inline uint8_t resetRequest() const
	{
		return m_resetRequest;
	}

	/************** functions to process requests *********/
	void requestPrologAnalogRead()
	{
		if( m_analogLastTimeStamp != m_buffer[TSTMP_ADDR] )
		{
			m_analogLastTimeStamp = m_buffer[TSTMP_ADDR];
			uint8_t a  = m_buffer[REQ_DATA0_ADDR] & 0x3F;
			LinkuinoTraits::selectAnalogChannel( a );
			LinkuinoTraits::analogAcquire();
		}
		else
		{
			m_buffer[REQ_ADDR] = REQ_NO_OP;
		}
	}
	void processRequestAnalogRead()
	{
		if( m_buffer[REQ_ADDR] != REQ_ANALOG_READ ) { return ; }
		// 10 bits value is expected
		uint16_t value = LinkuinoTraits::analogRead() & 0x03FF;
		// packet will always have the form 0xAA0... where the dots are 12 bits value
		m_reply[0] = REPLY_ANALOG_READ;
		m_reply[1] = (value >> 8 ) & 0x03 ;
		m_reply[2] = value & 0xFF;
		m_replyEnable = true;
	}
	void processRequestDigitalRead()
	{
		// read digital input state
		m_din = ( m_digitalInput.Get() >> DInPortFirstBit ) & m_dinMask;
		m_reply[0] = REPLY_DIGITAL_READ;
		m_reply[1] = m_din;
		m_reply[2] = REPLY_DIGITAL_READ;
		m_replyEnable = true;
	}
	void processRequestForwardSerial()
	{
		m_fwd  = m_buffer[REQ_DATA0_ADDR] & 0x3F;
		m_fwd <<= 6;
		m_fwd |= m_buffer[REQ_DATA1_ADDR] & 0x3F;
		m_fwd <<= 6;
		m_fwd |= m_buffer[REQ_DATA2_ADDR] & 0x3F;
		m_fwd <<= 6;
		m_fwd |= m_buffer[REQ_DATA3_ADDR] & 0x3F;
		m_fwdEnable = true;
		m_replyEnable = false;
	}
	void processRequestReset()
	{
		m_resetRequest = m_buffer[REQ_DATA0_ADDR] & 0x3F;
	}
	void processRequestRevision()
	{
		m_reply[0] = 0x00;
		m_reply[1] = ( (REV_MAJOR-1) & 0x03 ) | ( (MESSAGE_REPEATS<<2) & 0xFC );
		m_reply[2] = REV_MINOR + 1;
		m_replyEnable = true;
	}
	void processRequestNoReply()
	{
		m_replyEnable = false;
	}
	void processRequestNoOp() {}
	void processRequestAcknowledge()
	{
		m_reply[0] = 'O';
		m_reply[1] = 'k';
		m_reply[2] = '\n';
		m_replyEnable = true;
	}

	/***************** private data members *******/

	// request function type
	using ProcessRequestFuncT = void(LinkuinoT::*)();

	// request function dispatch table
	ProcessRequestFuncT m_requestDispatchTable[REQUEST_COUNT*2] =
	{
		// first set of functions are processing functions
		& LinkuinoT::processRequestAnalogRead,
		& LinkuinoT::processRequestDigitalRead,
		& LinkuinoT::processRequestForwardSerial,
		& LinkuinoT::processRequestReset,
		& LinkuinoT::processRequestRevision,
		& LinkuinoT::processRequestNoReply,
		& LinkuinoT::processRequestNoOp,
		& LinkuinoT::processRequestAcknowledge,
		
		// second set of functions are prolog functions
		& LinkuinoT::requestPrologAnalogRead,
		& LinkuinoT::processRequestNoOp,
		& LinkuinoT::processRequestNoOp,
		& LinkuinoT::processRequestNoOp,
		& LinkuinoT::processRequestNoOp,
		& LinkuinoT::processRequestNoOp,
		& LinkuinoT::processRequestNoOp,
		& LinkuinoT::processRequestNoOp
	};

	// PWM state
	uint16_t m_pwm[PWM_COUNT+1];
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
	bool m_replyEnable;
	uint8_t m_reply[3];
	
	// forward buffer
	bool m_fwdEnable;
	uint32_t m_fwd;
	
	// reset requested
	uint8_t m_resetRequest;
	
	// analog state machine
	uint8_t m_analogLastTimeStamp;

	// hardware pins accessors
	PWMPinGroupT m_pwmOutput;
	DOutPinGroupT m_digitalOutput;
	DInPinGroupT m_digitalInput;
};

#endif
