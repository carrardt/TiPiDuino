#include <AvrTL.h>
#include <LCD.h>
#include <SoftSerialIO.h>
#include <PrintStream.h>

using namespace avrtl;

static auto led = pin(13);
static auto rx = pin(7);
static auto tx = pin(8);

static auto serialIO = make_softserial<19200>(rx,tx);
PrintStream cout;

void setup()
{
	serialIO.begin();
	cout.begin( &serialIO );
}

static int COUNTER = 0;
void loop()
{
	cout<<"Hello World "<<COUNTER<<'\n';
	DelayMicroseconds(1000000UL);
	++COUNTER;
}
