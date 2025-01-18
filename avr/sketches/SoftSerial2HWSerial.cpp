#include <SoftSerial.h>
#include <BasicIO/ByteStream.h>
#include <BasicIO/PrintStream.h>
#include <AvrTL.h>
#include <AvrTLPin.h>
#include <HWSerialNoInt/HWSerialNoInt.h>

#include <avr/interrupt.h>

using namespace avrtl;

/*
 * linux example usage :
 * stty -F /dev/ttyUSB0 57600 raw cs8
 * echo "Hello world" > /dev/ttyUSB0
 */

#define RX_PIN 3
#define TX_PIN 4
#define LED_PIN 13

static auto led = StaticPin<LED_PIN>();
static auto rx = StaticPin<RX_PIN>();
static auto tx = StaticPin<TX_PIN>();

/* 
 * SoftSerial tests summury :
 * 
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
 * for i in `seq 1000`; do echo "Numéro $i s'affiche sans complexe                        543210\n" ; sleep 1 ; done > /dev/ttyUSB0
 */

using SerialScheduler = TimeSchedulerT<AvrTimer0NoPrescaler>;
static auto softSerialIO = make_softserial_hr<57600,SerialScheduler>(rx,tx);

ByteStreamAdapter<HWSerialNoInt,100000UL> hwSerialIO;
PrintStream cout;
 
void setup()
{
	cli();
	led.SetOutput();
	softSerialIO.begin();
	hwSerialIO.m_rawIO.begin(57600);
	cout.begin( &hwSerialIO );
	cout<<"Bauds=57600"<<endl;
	cout<<"F_CPU="<<F_CPU<<endl;
	cout<<"SoftSerial2HWSerial ready"<<endl;
	softSerialIO.ts.start();
}

static uint32_t counter = 0;
void loop()
{
	led = !led;
	char buf[64];
	uint8_t i=0;
	while( ( buf[i]=softSerialIO.readByteFast() ) != '\n' && i<63 ) ++i;
	buf[i]='\0';
	cout<<counter<<':'<<buf<<endl;
	++ counter;
}
