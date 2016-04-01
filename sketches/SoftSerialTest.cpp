#include <AvrTL.h>
#include <SoftSerial.h>
#include <BasicIO/PrintStream.h>

using namespace avrtl;

//static auto led = StaticPin<11>();
static auto rx = StaticPin<12>();
static auto tx = StaticPin<13>();

/* Tests
 * 9600 : Tx Ok, Rx Ok
 * 19200 : Tx Ok, Rx Ok
 * 38400 : Tx Ok, Rx Ok
 * 57600 : Tx Ok, Rx Ok
 * 115200 : not reliable. Tx almost good (9/10 bytes ok)
 */

static auto serialIO = make_softserial<57600>(rx,tx);

PrintStream cout;

void setup()
{
	serialIO.begin();
	cout.begin( &serialIO );
	cout<<"Ready"<<endl;
}

static uint32_t counter = 0;
void loop()
{
	/*
	cout<<"Counter = "<<counter<<endl;
	avrtl::DelayMicroseconds( 1000000UL );
	++ counter;
	* */
	
	char buf[32];
	uint8_t i=0;
	while( ( buf[i]=serialIO.readByte() ) != '\n' && i<32 ) ++i;
	buf[i]='\0';
	cout<<buf<<endl;
	
	/*
	serialIO.writeByte('X');
	avrtl::DelayMicroseconds( 100000UL );
	*/
}
