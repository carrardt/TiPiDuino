#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <TimeScheduler.h>
#include <HWSerialNoInt.h>
#include <BasicIO/ByteStream.h>
#include <BasicIO/PrintStream.h>
#include <FastSerial.h>

#include <Linkuino/Linkuino.h>

// 'old' setting, use led pin to forward commands to slave chip
//#define FWD_SERIAL_PIN 13

// 'new' setting, don't use led pin, so that it's not lit all the time
#define FWD_SERIAL_PIN 8


using namespace avrtl;

ByteStreamAdapter<HWSerialNoInt,100000UL> serialIO;

//#define DEBUG_TIMINGS 1

#ifdef DEBUG_TIMINGS
using Scheduler = TimeSchedulerT<AvrTimer1<>,int16_t,true>;
//using Scheduler = TimeSchedulerT<AvrTimer1<false>,int32_t,true>;
PrintStream cout;
#else
//using Scheduler = TimeSchedulerT<AvrTimer0> ; // SlotMax=127uS, resolution=500nS, 16Bits wallclock
//using Scheduler = TimeSchedulerT<AvrTimer1<false>,int32_t> ; // SlotMax>1000S, resolution=62nS, 32Bits wallclock
using Scheduler = TimeSchedulerT<AvrTimer1<>,int16_t > ; // SlotMax=32767uS, resolution=500nS, 16Bits wallclock
#endif

/*
 * Restrictions :
 * 8-bits pin ports
 * contiguous bits in the same port for PWM pins
 * contiguous bits in the same port for Digital out pins
 * contiguous bits in the same port for Digital in pins
 */
struct LinkuinoUnoTraits
{
	using PWMPinGroupT = avrtl::StaticPinGroup<0>; // Pins 0-7 (first bit set to 2 to let pins 0 & 1 free for hardware serial)
	using DOutPinGroupT = avrtl::StaticPinGroup<1>; // Pins 9-13 (pin 8 reserved for link with slave uController)
	using DInPinGroupT = StaticPinGroup<2>; // Pins 14-19 (a.k.a. A0-A5)

	static constexpr uint8_t PWMPortFirstBit = 2; // first accessible for PWM
	static constexpr uint8_t PWMPortMask = 0xFC; // mask of bits accessible for PWM
	static constexpr uint8_t PWMCount = 6;

#if FWD_SERIAL_PIN==13
	static constexpr uint8_t DOutPortFirstBit = 0; 
	static constexpr uint8_t DOutPortMask = 0x1F; // last digital output bit is disabled to be used for message forwarding
#else
	// fast serial link (forward message to slave uController) is on pin 8
	static constexpr uint8_t DOutPortFirstBit = 1; 
	static constexpr uint8_t DOutPortMask = 0x3E; // first digital output bit is disabled to be used for message forwarding
#endif
	static constexpr uint8_t DOutCount = 5;

	static constexpr uint8_t DInPortFirstBit = 0;
	static constexpr uint8_t DInPortMask = 0x3F;
	static constexpr uint8_t DInCount = 6;

	static inline void selectAnalogChannel(uint8_t ch) {}
	static inline void analogAcquire() {}
	static inline uint16_t analogRead() { return 0; }
};

using WallClock = typename Scheduler::WallClockT;
using Linkuino = LinkuinoT< LinkuinoUnoTraits >;

static Scheduler ts;
static Linkuino li;

// One of the digital output pin is reserved for message forwarding, using the FastSerial protocol
static auto fastSerialPin = StaticPin<FWD_SERIAL_PIN>();
static auto fastSerial = make_fastserial(NullPin(),fastSerialPin);

void setup()
{
	cli();

	serialIO.m_rawIO.begin(Linkuino::SERIAL_SPEED);

#ifdef DEBUG_TIMINGS
	cout.begin(&serialIO);
	cout<<"Linkuino"<<endl;
	cout<<"Tt="<<ts.tickTime()<<"nS"<<endl;
	cout<<"Tf="<<ts.maxFuncTime()<<"uS"<<endl;
	cout<<"Tc="<<ts.maxCycleTime()<<"mS"<<endl;
#endif

	li.begin();

	// last bit of digital output will be used 
	// fastSerialPin.SetOutput();
	// fastSerialPin = HIGH;
	fastSerial.begin();
	fastSerial.write<24>(0);

	ts.start();
}

#ifdef LINKUINO_FREQ_100HZ
#define LOOP_STARTUP_TIME	50
#define ALL_HIGH_TIME		400
#define FAST_SHUTDOWN_TIME	1600
#define RECV_SHUTDOWN_TIME	7500
#define END_PROCESS_TIME	450
//      TOTAL				10000
#else
#define LOOP_STARTUP_TIME	50
#define ALL_HIGH_TIME		800
#define FAST_SHUTDOWN_TIME	3200
#define RECV_SHUTDOWN_TIME	15000
#define END_PROCESS_TIME	950
//      TOTAL				20000
#endif

void loop()
{
	// for phase correctness, absorb loop latency here
	ts.exec( LOOP_STARTUP_TIME, [](){} );			// 9950 uS -> 10000 uS (begining of next cycle)

	ts.exec( ALL_HIGH_TIME, []()				// 0 -> 400 uS
		{
			li.allPwmHigh();
		} );

	// no communications for the first 2mS to ensure
	// maximum precision of servo compatible PWM pulses
	ts.loop( FAST_SHUTDOWN_TIME, [](WallClock t) 	// 400 uS -> 2000 uS
		{
			li.shutDownPWM( ALL_HIGH_TIME + t );
		} );

	// during the second period, pulse length values and update frequencies are
	// less precise, but we can receive command packet at the same time
	// so we have 7.61 milliseconds to listen for serial commands
	ts.loop( RECV_SHUTDOWN_TIME, [](WallClock t)	// 2000 uS -> 9500 uS
		{
			li.receive( serialIO.m_rawIO );
			li.shutDownPWM( ALL_HIGH_TIME + FAST_SHUTDOWN_TIME + t );
		} );

	ts.exec( END_PROCESS_TIME, []()				// 9500 uS -> 9950 uS
		{
			li.process();
			li.reply( serialIO.m_rawIO, fastSerial );
			li.prepareToReceive();
		} );

#ifdef DEBUG_TIMINGS
	ts.printDebugInfo( cout );
#endif
	ts.endCycle();
}

