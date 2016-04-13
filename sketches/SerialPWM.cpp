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
//using Scheduler = TimeSchedulerT<AvrTimer1<>,int16_t,true>;
using Scheduler = TimeSchedulerT<AvrTimer1<false>,int32_t,true>;
#else
//using Scheduler = TimeSchedulerT<AvrTimer0> ; // SlotMax=127uS, resolution=500nS, 16Bits wallclock
//using Scheduler = TimeSchedulerT<AvrTimer1<false>,int32_t> ; // SlotMax>1000S, resolution=62nS, 32Bits wallclock
using Scheduler = TimeSchedulerT<AvrTimer1<> > ; // SlotMax=32767uS, resolution=500nS, 16Bits wallclock
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
	using PWMPinGroupT = avrtl::StaticPinGroup<1>;
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

using WallClock = typename Scheduler::WallClockT;
using Linkuino = LinkuinoT< LinkuinoUnoTraits >;

static Scheduler ts;
static Linkuino li;

void setup()
{
	serialIO.m_rawIO.begin(57600);
	cout.begin(&serialIO);
	//pwm.SetOutput();
	cout<<"SerialPWM"<<endl;
	cout<<"Tt="<<ts.tickTime()<<"nS"<<endl;
	cout<<"Tf="<<ts.maxFuncTime()<<"uS"<<endl;
	cout<<"Tc="<<ts.maxCycleTime()<<"mS"<<endl;
	cli();
	ts.start();
}

static uint32_t counter = 0;

void loop()
{
	// for phase correctness, absorb loop latency here
	ts.exec( 50, [](){} );			// 9950 uS -> 10000 uS (begining of next cycle)

	ts.exec( 500, []()				// 0 -> 500 uS
		{
			//pwm = HIGH;
			li.allPwmHigh();
			// send reply bytes (x2), it takes approx 180 uS @57600 bauds
		} );

	ts.loop( 1500, [](WallClock t) 	// 500 uS -> 2000 uS
		{
			//pwm = (t<pwmValue);
			li.shutDownPWM( 500+t );
		} );

	ts.exec( 50, []()				// 2000 uS -> 2050 uS
		{
			//pwm = LOW;
			li.allPwmLow();
			li.prepareToReceive();
		} );

	// we have 5 milliseconds to listen for serial commands
	ts.loop( 5000, [](WallClock t)	// 2050 uS -> 7050 uS
		{
			li.receive( serialIO.m_rawIO );
		} );

	ts.exec( 2000, []()				// 7050 uS -> 9050 uS
		{
			li.process();
		} );

	// keep 50 uS to be transfered at the beginning to absorb loop latency
	ts.exec( 900, []()				// 9050 uS -> 9950 uS
		{
			li.reply( serialIO.m_rawIO );
		} );

#ifdef DEBUG_TIMINGS
	if( ( counter & 0xFF ) == 0 )
	{
		cout<<">"<<ts.m_dbg[0].m_i<<"\n";
		for(int i=0;i<ts.m_dbg[0].m_i;i++)
		{
			uint32_t T = ts.m_dbg[0].m_timings[i];
			T = ( T * ts.tickTime() ) / 1000;
			cout<<T<< ( (i%3)==2 ? '\n' : ' ' );
		}
		ts.reset();
	}
	else { ts.m_dbg[0].reset(); }
/*
	if( ( counter & 0xFF ) == 0 )
	{
		cout<<'*'<<li.m_pwmSeqLen<<'\n';
		for(int p=0;p<li.m_pwmSeqLen;p++)
		{
			cout<< (int)li.m_pwm[p] << '/' << (int)li.m_pwmShutDown[p] << ((p%2)? '\n' : ' ');
		}
		ts.reset();
	}
	*/
	++ counter;

#endif

}

