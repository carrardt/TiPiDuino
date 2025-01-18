#include <FastSerial.h>
#include <AvrTL.h>
#include <AvrTLPin.h>
#include <avr/interrupt.h>
#include <BasicIO/InputStream.h>
#include <BasicIO/PrintStream.h>
#include <HWSerialNoInt/HWSerialNoInt.h>

using namespace avrtl;

#define TX_PIN 13

static auto tx = StaticPin<TX_PIN>();
static auto fastSerial = make_fastserial(NullPin(),tx);
 
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
	uint8_t buf[4];
	buf[0] = serialIO.readByte();
	buf[1] = serialIO.readByte();
	buf[2] = serialIO.readByte();
	buf[3] = serialIO.readByte();
	
	// synchronization sequence : 0x00 (many times, >=8 ) 0xFF
	if( buf[1]==0 && buf[3]==0 )
	{
		while( serialIO.readByte() != 0xFF );
		return;
	}
	uint32_t targetTickCountR = buf[0];
	uint32_t targetRotationR  = buf[1];
	uint32_t targetTickCountL = buf[2];
	uint32_t targetRotationL  = buf[3];

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
