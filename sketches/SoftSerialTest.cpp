#include <AvrTL.h>
#include <LCD.h>
#include <SoftSerialIO.h>
#include <PrintStream.h>

using namespace avrtl;

static auto led = StaticPin<11>();
static auto rx = StaticPin<12>();
static auto tx = StaticPin<13>();

static auto serialIO = make_softserial<9600>(rx,tx);
PrintStream cout;

void setup()
{
	led.SetOutput();
	serialIO.begin();
	cout.begin( &serialIO );
	blink(led);
}

static int COUNTER = 0;
void loop()
{
	cout<<"Hello World "<<COUNTER<<'\n';
	DelayMicroseconds(1000000UL);
	++COUNTER;
	blink(led);
}
