#include <SoftSerial.h>
#include <FastSerial.h>
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
 * 9200 : Tx Ok, Rx Ok
 * 38400 : Tx Ok, Rx Ok
 * 57600 : Tx Ok, Rx Ok
 * 115200 : not working at all 
 * 
 * Tested using the following command line :
 * for i in `seq 1000`; do echo "Numéro $i s'affiche sans complexe                        543210" ; sleep 1 ; done > /dev/ttyUSB0
 */

using SerialScheduler = TimeSchedulerT<AvrTimer0NoPrescaler>;
static auto rawSerialIO = make_softserial_hr<57600,SerialScheduler>(NullPin(),tx);

static auto fastSerial = make_fastserial(rx,NullPin());

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
	serialIO.m_rawIO.ts.start();
}

void loop()
{
	led = !led;
	uint32_t n = fastSerial.read<32>();
	cout<<n<<endl;
}
