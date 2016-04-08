#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <TimeScheduler.h>
#include <HWSerialNoInt.h>
#include <BasicIO/ByteStream.h>
#include <BasicIO/PrintStream.h>

using namespace avrtl;

//#define DEBUG_TIMINGS 1
#define PWM_PIN 13
#define SMOOTHING 2 // 0,1,2 or 3 level of input value smoothing

static auto pwm = StaticPin<PWM_PIN>();
ByteStreamAdapter<HWSerialNoInt,100000UL> serialIO;
PrintStream cout;

#ifdef DEBUG_TIMINGS
//using Scheduler = TimeSchedulerT<AvrTimer1<>,int16_t,true>;
using Scheduler = TimeSchedulerT<AvrTimer1<false>,int32_t,true>;
#else
//using Scheduler = TimeSchedulerT<AvrTimer0> ; // SlotMax=127uS, resolution=500nS, 16Bits wallclock
//using Scheduler = TimeSchedulerT<AvrTimer1<false>,int32_t> ; // SlotMax>1000S, resolution=62nS, 32Bits wallclock
using Scheduler = TimeSchedulerT<AvrTimer1<> > ; // SlotMax=32767uS, resolution=500nS, 16Bits wallclock
#endif

using WallClock = typename Scheduler::WallClockT;

Scheduler ts;


static uint8_t addr = 0;
static uint8_t clk = 255;
static uint8_t cmd[64];
static uint16_t pwmValue = 1024;
static uint16_t pwmTargetValue = 1024;

#ifdef DEBUG_TIMINGS
static uint32_t counter = 0;
#endif

void setup()
{
	serialIO.m_rawIO.begin(57600);
	cout.begin(&serialIO);
	pwm.SetOutput();
	cout<<"SerialPWM"<<endl;
	cout<<"Tt="<<ts.tickTime()<<"nS"<<endl;
	cout<<"Tf="<<ts.maxFuncTime()<<"uS"<<endl;
	cout<<"Tc="<<ts.maxCycleTime()<<"mS"<<endl;
	cli();
	ts.start();
}

void loop()
{
	// for phase correctness, absorb loop latency here
	ts.exec( 50, [](){} );

	ts.exec( 400, []()
		{
			pwm = HIGH;
			addr = 0;
			clk = 255;
		} );

	ts.loop( 2000, [](WallClock t)
		{
			pwm = (t<pwmValue);
		} );

	ts.exec( 600, []()
		{
			pwm = LOW;
			cmd[0]=255;
			cmd[1]=255;
			cmd[2]=255;
			cmd[3]=255;
		} );
			
	// we have 5 milliseconds to listen for serial commands
	ts.loop( 5000, [](WallClock t) 
		{
			uint16_t x = HWSerialNoInt::readByteAsync();
			if( x )
			{
				uint8_t r = x;
				uint8_t data = r & 0x3F;
				r >>= 6;
				uint8_t nRS = r & 0x01;
				uint8_t nCLK = r >> 1;
				if(nRS) { addr = data; }
				else if(nCLK==clk) { cmd[addr] = data; }
				clk = nCLK;
			}
		} );

	// keep 50 uS to be transfered at the beginning to absorb loop latency
	ts.exec( 1950, []()
		{
			if( cmd[1]!=255 && cmd[2]!=255 )
			{
				pwmTargetValue = cmd[1];
				pwmTargetValue <<= 6;
				pwmTargetValue |= cmd[2];
			}
			uint16_t target = pwmTargetValue;
			if( target <= 400 ) target=0;
			else if( target >= 2000 ) target=1600;
			else target -= 400;
			pwmValue = ( (pwmValue<<SMOOTHING)-pwmValue+target ) >> SMOOTHING ;
		} );

#ifdef DEBUG_TIMINGS
	if( ( counter & 0xFF ) == 0 )
	{
		cout<<"-\n";
		for(int i=0;i<ts.m_dbg[0].m_i;i++)
		{
			cout<<ts.m_dbg[0].m_timings[i]<< ( (i%3)==3 ? '\n' : ' ' );
		}
		ts.reset();
	}
	ts.m_dbg[0].reset();
	++ counter;
#endif
}

