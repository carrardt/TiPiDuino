#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <PrintStream.h>

//#define ONLY_5BITS 1

#define PWM1_PIN 12
#define PWM2_PIN 13
#define SMOOTHING 1 // 0,1,2 or 3 level of input value smoothing

using namespace avrtl;

HWSerialIO serialIO;
PrintStream cout;

auto dualpwm = DualPin<PWM1_PIN,PWM2_PIN>();
constexpr uint16_t cycleTicks =  19809;// this is valid only for 16Mhz clock, otherwise use this => microsecondsToTicks(9904);

// PWM value in microsecond (between 500 and 2000)
static uint16_t Value1 = 1000;
static uint16_t Value2 = 1000;

static uint8_t VALUES[4] = {0x80,0x80,0x80,0x80};

ISR(USART_RX_vect)
{
	uint8_t i = UDR0;
	uint8_t r = i>>6;
	VALUES[r] = i & 0x3F;
}

void setup()
{
	dualpwm.SetOutput();
	serialIO.begin(57600);
	cout.begin( &serialIO );
	cout<<"Ready"<<endl;
}

static uint16_t getPWMTicks1()
{
	uint8_t old_SREG = SREG;
	sei();
	DelayMicrosecondsFast(5000); // Fast because we're called from within loopDualPWM
	SREG=old_SREG;
	if( (VALUES[0]|VALUES[1])&0x80 == 0 )
	{
		Value1 = VALUES[0];
		Value1 = (Value1 << 6) | VALUES[1];
	}
	VALUES[0]=0x80;
	VALUES[1]=0x80;
	return microsecondsToTicks(Value1);
}

static uint16_t getPWMTicks2()
{
	if( (VALUES[2]|VALUES[3])&0x80 == 0 )
	{
		Value2 = VALUES[2];
		Value2 = (Value2 << 6) | VALUES[3];
	}
	VALUES[2]=0x80;
	VALUES[3]=0x80;
	return microsecondsToTicks(Value2);
}

void loop()
{
	if( (VALUES[0]|VALUES[1])&0x80 == 0 )
	{
		Value1 = VALUES[0];
		Value1 = (Value1 << 6) | VALUES[1];
		VALUES[0]=0x80;
		VALUES[1]=0x80;
		cout<<Value1<<endl;
	}

//	loopDualPWM<cycleTicks>( dualpwm, getPWMTicks1, getPWMTicks2 );
}
