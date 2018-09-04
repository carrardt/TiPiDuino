#include <AvrTL.h>
#include <AvrTLPin.h>
#include <SoftSerial.h>
#include <BasicIO/ByteStream.h>
#include <BasicIO/PrintStream.h>
#include <TimeScheduler/TimeScheduler.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#define DEFAULT_TIMER_SECONDS 300
#define SERIAL_SPEED 19200


#define RELAY_PIN  		2
#define TIME_INC_PIN 	1
#define TIME_DEC_PIN 	0
#define POWER_OFF_PIN 	3
#define SERIAL_OUT_PIN 	4

#define TIMER_EEPROM_ADDR0 ((uint8_t*)0x0010)
#define TIMER_EEPROM_ADDR1 ((uint8_t*)0x0011)
#define MAGICNUMBER_EEPROM_ADDR ((uint8_t*)0x0012)
#define MAGICNUMBER_VALUE ((uint8_t)0x76)

using namespace avrtl;

static auto powerOffPin = avrtl::StaticPin<POWER_OFF_PIN>();
static auto upPin = avrtl::StaticPin<TIME_INC_PIN>();
static auto downPin = avrtl::StaticPin<TIME_DEC_PIN>();
static auto relayPin = avrtl::StaticPin<RELAY_PIN>();

static int32_t timerSeconds = DEFAULT_TIMER_SECONDS;
static int32_t timerRemainingSeconds = DEFAULT_TIMER_SECONDS;
static bool timerSecondsChanged = false;

/*
using SerialScheduler = TimeSchedulerT<AvrTimer0NoPrescaler>;
static avrtl::NullPin rx;
static auto tx = avrtl::StaticPin<SERIAL_OUT_PIN>();
static auto rawSerialIO = make_softserial_hr<SERIAL_SPEED,SerialScheduler>(rx,tx);
static ByteStreamAdapter<decltype(rawSerialIO),100000UL> serialIO = { rawSerialIO };
static PrintStream cout;
*/

static TimeSchedulerT< AvrTimer0 , int32_t > ts;

void setup()
{
	cli();
	
	// first thing to do in order to stay alive (otherwise power will shutdown
	relayPin.SetOutput();
	relayPin.Set( true );

/*
	powerOffPin.SetInput();
	powerOffPin.Set( HIGH ); // pullup

	upPin.SetInput();
	upPin.Set( HIGH ); // pullup

	downPin.SetInput();
	downPin.Set( HIGH ); // pullup
	
	timerSeconds = eeprom_read_byte( TIMER_EEPROM_ADDR0 )<<8 | eeprom_read_byte( TIMER_EEPROM_ADDR1 );
	uint8_t magic = eeprom_read_byte( MAGICNUMBER_EEPROM_ADDR );
	if( timerSeconds > 3600 || timerSeconds < 5 || magic != MAGICNUMBER_VALUE )
	{
		timerSeconds = DEFAULT_TIMER_SECONDS;
		eeprom_write_byte( TIMER_EEPROM_ADDR0 , timerSeconds>>8 );
		eeprom_write_byte( TIMER_EEPROM_ADDR1 , timerSeconds&0xFF );
		eeprom_write_byte( MAGICNUMBER_EEPROM_ADDR , MAGICNUMBER_VALUE );
	}
	timerRemainingSeconds = timerSeconds;
	*/
	
	/*
	avrtl::DelayMicroseconds( 200000UL );
	serialIO.m_rawIO.begin();
	cout.begin( &serialIO );
	cout<<"F_CPU="<<F_CPU<<endl;
	cout<<"Ready"<<endl;
	//rawSerialIO.ts.start();
	*/
	
	ts.start();
}

// allows for key combinations
static uint8_t getButtonCode()
{
	uint8_t s = ! powerOffPin.Get();
	uint8_t u = ! upPin.Get();
	uint8_t d = ! downPin.Get();
	uint8_t pc = s<<2 | u<<1 | d;
	uint8_t press_code;
	if( pc == 0 ) { return 0; }
	
	do
	{
		press_code = pc;
		s = ! powerOffPin.Get();
		u = ! upPin.Get();
		d = ! downPin.Get();
		pc = s<<2 | u<<1 | d;
	} while(  pc >= press_code );
	
	do
	{
		s = ! powerOffPin.Get();
		u = ! upPin.Get();
		d = ! downPin.Get();
		pc = s<<2 | u<<1 | d;
	} while( pc != 0 );
	
	return press_code;
}

/*
static uint8_t buffer[16];
static void updateTimerLCD()
{
	uint8_t i = 0;
	uint8_t* buf = buffer;
	rawSerialIO.ts.execFast( 1000, [&buf](){ *buf++ = 'H'; });
	rawSerialIO.ts.execFast( 1000, [&buf](){ *buf++ = 'e'; });
	rawSerialIO.ts.execFast( 1000, [&buf](){ *buf++ = 'l'; });
	rawSerialIO.ts.execFast( 1000, [&buf](){ *buf++ = 'l'; });
	rawSerialIO.ts.execFast( 1000, [&buf](){ *buf++ = 'o'; });
	rawSerialIO.ts.execFast( 1000, [&buf](){ *buf++ = '\n'; });
	rawSerialIO.ts.execFast( 1000, [&buf](){ *buf++ = '\0'; });
	rawSerialIO.writeBufferFast( buffer );
}
*/

static void powerOff()
{
	/*if( timerSecondsChanged )
	{
		eeprom_write_byte( TIMER_EEPROM_ADDR0 , timerSeconds>>8 );
		eeprom_write_byte( TIMER_EEPROM_ADDR1 , timerSeconds&0xFF );
	}*/
	while( true ) { relayPin.Set( false ); }
}

static void processButton(uint8_t bc)
{
	switch( bc )
	{
		case 3 : // up & down simultanoueously => reset to default time value
			timerSeconds = DEFAULT_TIMER_SECONDS;
			timerRemainingSeconds = DEFAULT_TIMER_SECONDS;
			timerSecondsChanged = true;
			break;
		case 1 : // more delay 
			if( timerSeconds > 9 ) { timerSeconds -= 5; }
			if( timerRemainingSeconds > 5 ) timerRemainingSeconds -= 5;
			else { timerRemainingSeconds = 0; }
			timerSecondsChanged = true;
			break;
		case 2 : // less delay
			if( timerSeconds < 3595 ) {timerSeconds += 5; }
			timerRemainingSeconds += 5;
			timerSecondsChanged = true;
			break;
		case 4 : // poweroff
			powerOff();
			break;
	}
}

void loop()
{
	//updateTimerLCD();
	/*
	for(uint8_t i=0;i<95;i++)
	{
		uint8_t bc = 0;
		rawSerialIO.ts.execFast( 10000, [&bc](){ bc = getButtonCode(); });
		if(bc!=0)
		{
			processButton(bc);
			rawSerialIO.ts.reset();
			return;
		}
	}
	*/
	ts.exec( 1000000UL, []()
		{
			-- timerRemainingSeconds;
			if( timerRemainingSeconds <= 0 )
			{
				powerOff();
			}
		});

}

