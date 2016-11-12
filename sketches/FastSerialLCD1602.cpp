#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <LCD1602.h>
#include <FastSerial.h>
#include <BasicIO/PrintStream.h>

/*
 * LCD1602	Arduino
 * VSS		GND
 * VDD		+5v
 * VO		Pontentiometer(10K)/resistor
 * RS		D2
 * RW		GND
 * EN		D3
 * D4		D9
 * D5		D10
 * D6		D11
 * D7		D12
 * A		+5v
 * K		GND
 */

#define LCD_PINS 2,3,9,10,11,12 // respectively RS, EN, D4, D5, D6, D7
#define RX_PIN 4 // fast-serial protocol receive onthis pin

using namespace avrtl;

LCD1602<LCD_PINS> lcd;
PrintStream cout;

static auto rx = StaticPin<RX_PIN>();
static auto fastSerial = make_fastserial(rx,NullPin());

void setup()
{
	cli();
	rx.SetInput();
	lcd.begin();
	cout.begin( &lcd );
	fastSerial.begin();
	cout<<"F_CPU="<<F_CPU<<endl;
	cout<<"Ready"<<endl;
}

static inline uint32_t debug_fast_serial_input()
{
	static constexpr uint8_t NBits = 24;
	uint8_t buf[NBits];
	uint8_t l=0;
	do
	{
		l = 0;
		while( rx.Get() );
		while( !rx.Get() ) ++l;
	} while( l<10 );
	while( rx.Get() );
	for(int i=0;i<NBits;i++)
	{
		buf[i] = 0;
		while( !rx.Get() );
		while( rx.Get() && buf[i]<255) ++buf[i];
	}
	uint32_t n = 0;
	for(int i=0;i<NBits;i++)
	{
		if( buf[i] > 250 ) { cout<<"Overflow"<<endl; return 0; }
		uint32_t b = (buf[i]>8) ? (1UL<<i) : 0 ;
		n |= b;
	}
	return n;
}

void loop()
{
	constexpr uint8_t NBits = 24;
	//uint32_t command = fastSerial.read<NBits>();
	uint32_t command = debug_fast_serial_input();

	uint16_t commandTR = (command>>20) & 0x0F;
	uint16_t commandRR = (command>>12) & 0xFF;
	uint16_t commandTL = (command>>8) & 0x0F;
	uint16_t commandRL = command & 0xFF;

	cout<<'R'<<commandRR<<'@'<<commandTR<<" L"<<commandRL<<'@'<<commandTL<<endl;
}

