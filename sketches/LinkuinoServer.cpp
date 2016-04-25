#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <TimeScheduler.h>
#include <HWSerialNoInt.h>
#include <BasicIO/ByteStream.h>
#include <BasicIO/PrintStream.h>

#include <Linkuino/Linkuino.h>

using namespace avrtl;

ByteStreamAdapter<HWSerialNoInt,100000UL> serialIO;
PrintStream cout;

//#define DEBUG_TIMINGS 1

#ifdef DEBUG_TIMINGS
using Scheduler = TimeSchedulerT<AvrTimer1<>,int16_t,true>;
//using Scheduler = TimeSchedulerT<AvrTimer1<false>,int32_t,true>;
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
	using DOutPinGroupT = avrtl::StaticPinGroup<1>; // Pins 8-13
	using DInPinGroupT = StaticPinGroup<2>; // Pins 14-19 (a.k.a. A0-A5)

	static constexpr uint8_t PWMPortFirstBit = 2; // first accessible for PWM
	static constexpr uint8_t PWMPortMask = 0xFC; // mask of bits accessible for PWM

	static constexpr uint8_t DOutPortFirstBit = 0; 
	static constexpr uint8_t DOutPortMask = 0x3F;

	static constexpr uint8_t DInPortFirstBit = 0;
	static constexpr uint8_t DInPortMask = 0x3F;

	static inline void selectAnalogChannel(uint8_t ch) {}
	static inline void analogAcquire() {}
	static inline uint16_t analogRead() { return 0; }
};

using WallClock = typename Scheduler::WallClockT;
using Linkuino = LinkuinoT< LinkuinoUnoTraits >;

static Scheduler ts;
static Linkuino li;

void setup()
{
	serialIO.m_rawIO.begin(57600);
	cout.begin(&serialIO);
	//pwm.SetOutput();
	cout<<"Linkuino"<<endl;
	cout<<"Tt="<<ts.tickTime()<<"nS"<<endl;
	cout<<"Tf="<<ts.maxFuncTime()<<"uS"<<endl;
	cout<<"Tc="<<ts.maxCycleTime()<<"mS"<<endl;
	cli();
	ts.start();
}

void loop()
{
	// for phase correctness, absorb loop latency here
	ts.exec( 50, [](){} );			// 9950 uS -> 10000 uS (begining of next cycle)

	ts.exec( 400, []()				// 0 -> 400 uS
		{
			li.allPwmHigh();
		} );

	// no communications for the first 2mS to ensure
	// maximum precision of servo compatible PWM pulses
	ts.loop( 1600, [](WallClock t) 	// 400 uS -> 2000 uS
		{
			li.shutDownPWM( 400+t );
		} );

	// during the second period, pulse length values and update frequencies are
	// less precise, but we can receive command packet at the same time
	// so we have 7.61 milliseconds to listen for serial commands
	ts.loop( 7610, [](WallClock t)	// 2000 uS -> 9610 uS
		{
			li.receive( serialIO.m_rawIO );
			li.shutDownPWM( 2000+t );
		} );

	ts.exec( 340, []()				// 9610 uS -> 9950 uS
		{
			li.process();
			li.reply( serialIO.m_rawIO );
			li.prepareToReceive();
		} );

	ts.printDebugInfo( cout );
	ts.endCycle();
}
