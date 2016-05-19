#include <FastSerial.h>
#include <AvrTL.h>
#include <AvrTLPin.h>
#include <avr/interrupt.h>
#include <BasicIO/InputStream.h>
#include <BasicIO/PrintStream.h>
#include <HWSerialNoInt/HWSerialNoInt.h>

using namespace avrtl;

#define RX_PIN 3
#define TX_PIN 4

static auto rx = StaticPin<RX_PIN>();
static auto tx = StaticPin<TX_PIN>();
static auto fastSerial = make_fastserial(rx,tx);
 
ByteStreamAdapter<HWSerialNoInt,100000UL> serialIO;
InputStream cin;
PrintStream cout;
 
void setup()
{
	cli();
	serialIO.m_rawIO.begin(38400);
	cin.begin( &serialIO );
	cout.begin( &serialIO );
	fastSerial.begin();
}

void loop()
{
	uint32_t targetTickCountR = serialIO.readByte();
	uint32_t targetRotationR = serialIO.readByte();
	uint32_t targetTickCountL = serialIO.readByte();
	uint32_t targetRotationL = serialIO.readByte();

	uint32_t data = targetTickCountR;
	data <<= 8;
	data |= targetRotationR;
	data <<= 4;
	data |= targetTickCountL;
	data <<= 8;
	data |= targetRotationL;
	
	cout<<"TR="<<targetTickCountR<<", RR="<<targetRotationR<<", TL="<<targetTickCountL<<", RL="<<targetRotationL<<" => "<<data<<endl;

	fastSerial.write<24>(data);
}
