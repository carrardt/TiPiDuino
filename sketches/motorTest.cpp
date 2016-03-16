#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>

using namespace avrtl;

#define MOTOR1_PIN1 2
#define MOTOR1_PIN2 3
#define MOTOR2_PIN1 4
#define MOTOR2_PIN2 5

static constexpr auto m1p1 = StaticPin<MOTOR1_PIN1>();
static constexpr auto m1p2 = StaticPin<MOTOR1_PIN2>();
static constexpr auto m2p1 = StaticPin<MOTOR2_PIN1>();
static constexpr auto m2p2 = StaticPin<MOTOR2_PIN2>();

void setup()
{
	m1p1.SetOutput(); m1p1=false;
	m1p2.SetOutput(); m1p2=false;
	m2p1.SetOutput(); m2p1=false;
	m2p2.SetOutput(); m2p2=false;
}

void loop()
{
	static bool leftRight=false;
	leftRight = !leftRight;
	
	m1p1=true;
	m2p1=true;
	DelayMicroseconds(2000000UL);
	
	m1p1=leftRight;
	m2p1=!leftRight;
	DelayMicroseconds(500000UL);
	
	m1p1=false;
	m2p1=false;
	DelayMicroseconds(2000000UL);
}

