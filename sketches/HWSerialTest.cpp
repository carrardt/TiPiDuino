#include <AvrTLPin.h>
#include <AvrTL.h>
#include <BasicIO/PrintStream.h>
#include <BasicIO/InputStream.h>
#include <HWSerial/HWSerialIO.h>
#include <AvrTL/timer.h>

using namespace avrtl;

HWSerialIO serialIO;

PrintStream cout;
//InputStream cin;

void setup()
{
	serialIO.begin(19200);

	cout.begin( &serialIO );
	//cin.begin( &serialIO );
	cout<<"Ready"<<endl;
}

static uint32_t counter = 0;
void loop()
{
	cout<< "Counter = "<<counter<<endl;
	avrtl::delayMicroseconds( 1000000UL );
	++ counter;

	/*
	serialIO.writeByte(129);
	avrtl::DelayMicroseconds( 1000000UL );
	*/
	
	/*
	char c = 0;
	cin >> c;
	cout<<"'"<<c<<"' TXr="<<HWSerialIO::Tx_ready<<" RXr="<<HWSerialIO::Rx_read<<" RXa="<<HWSerialIO::Rx_avail<<endl;
	*/
}
