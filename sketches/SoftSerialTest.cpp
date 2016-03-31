#include <AvrTL.h>
#include <SoftSerial.h>
#include <BasicIO/PrintStream.h>

using namespace avrtl;

//static auto led = StaticPin<11>();
static auto rx = StaticPin<13>();
static auto tx = StaticPin<12>();

static auto serialIO = make_softserial<9600>(rx,tx);
PrintStream cout;

void setup()
{
	//led.SetOutput();
	serialIO.begin();
	cout.begin( &serialIO );
	//blink(led);
}

static int COUNTER = 0;
void loop()
{
	//cout<<"Hello World "<<COUNTER<<'\n';
	//DelayMicroseconds(1000000UL);
	//++COUNTER;
	//blink(led);
	serialIO.writeByte(0xAA);
}
