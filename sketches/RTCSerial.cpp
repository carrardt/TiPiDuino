#include <AvrTLPin.h>
#include <AvrTL.h>
#include <BasicIO/PrintStream.h>
#include <BasicIO/InputStream.h>
#include <HWSerial/HWSerialIO.h>
#include <AvrTL/timer.h>

#include <uRTCLib/uRTCLib.h>


using namespace avrtl;

HWSerialIO serialIO;

PrintStream cout;
//InputStream cin;

// DS1307 RTC instance
static uRTCLib rtc(0x68/*,URTCLIB_MODEL_DS1307*/);

void setup()
{
	serialIO.begin(19200);

	cout.begin( &serialIO );
	//cin.begin( &serialIO );
	
	URTCLIB_WIRE.begin();
	rtc.refresh();
	cout<<"Ready"<<endl;
}

static uint32_t counter = 0;
void loop()
{
  rtc.refresh();
  const int h=rtc.hour(), m=rtc.minute(), s=rtc.second();
  cout<< h/10 << h%10 <<'h' << m/10 << m%10 << ( (counter%2==0) ? ':' : ' ' ) << s/10 << s%10 <<'\n';

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
