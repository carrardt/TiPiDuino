#include <SoftSerial.h>
#include <BasicIO/ByteStream.h>
#include <BasicIO/PrintStream.h>
#include <AvrTL.h>
#include <AvrTLPin.h>

using namespace avrtl;

/*
 * linux example usage :
 * stty -F /dev/ttyUSB0 57600 raw cs8
 * echo "Hello world" > /dev/ttyUSB0
 */

#define RX_PIN 3
#define TX_PIN 4
#define LED_PIN 2

static auto led = StaticPin<LED_PIN>();
static auto rx = StaticPin<RX_PIN>();
static auto tx = StaticPin<TX_PIN>();

/* 
 * Tests on an ATmega328P @16Mhz :
 * ===============================
 * 9600 : Tx Ok, Rx Ok
 * 19200 : Tx Ok, Rx Ok
 * 38400 : Tx Ok, Rx Ok
 * 57600 : Tx Ok, Rx Ok
 * 115200 : not reliable. Tx almost good (9/10 bytes ok)
 * 
 * Tests on an ATtiny85 @8Mhz :
 * ============================
 * 9600 : Tx Ok, Rx Ok
 * 19200 : Tx Ok, Rx Ok
 * 38400 : Tx Ok, Rx Ok
 * 57600 : Tx Ok, Rx Ok
 * 115200 : not working at all 
 * 
 * Tested using the following command line :
 * for i in `seq 1000`; do echo "Numéro $i s'affiche sans complexe                        543210" ; sleep 1 ; done > /dev/ttyUSB0
 */

using SerialScheduler = TimeSchedulerT<AvrTimer0NoPrescaler>;
static auto rawSerialIO = make_softserial_hr<19200,SerialScheduler>(rx,tx);

static ByteStreamAdapter<decltype(rawSerialIO),100000UL> serialIO = { rawSerialIO };
static PrintStream cout;
 
void setup()
{
	cli();
	led.SetOutput();
	serialIO.m_rawIO.begin();
	cout.begin( &serialIO );
	cout<<"F_CPU="<<F_CPU<<endl;
	cout<<"Ready"<<endl;
}

static uint32_t counter = 0;
void loop()
{
	led = !led;

	cout<<"Counter = "<<counter<<endl;
	avrtl::DelayMicroseconds( 100000UL );
	++ counter;

#if 0
	char buf[64];
	uint8_t i=0;
//	while( ( buf[i]=serialIO.readByte() ) != '\n' && i<63 ) ++i;
	serialIO.m_rawIO.ts.start();
	for(uint8_t i=0;i<64;i++) { buf[i]=serialIO.m_rawIO.readByteFast(); }
	serialIO.m_rawIO.ts.stop();
	buf[63]='\0';
	cout<<counter<<':'<<buf<<endl;
	++ counter;
#endif

	/*
	serialIO.writeByte('X');
	avrtl::DelayMicroseconds( 100000UL );
	*/
}
