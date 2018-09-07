#include <AvrTL.h>
#include <AvrTLPin.h>
#include <BasicIO/ByteStream.h>
#include <BasicIO/PrintStream.h>
#include <TimeScheduler/TimeScheduler.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#define DEFAULT_TIMER_SECONDS 300

#define RELAY_PIN  	2
#define TIME_INC_PIN 	1
#define TIME_DEC_PIN 	0
#define POWER_OFF_PIN 	3
#define SERIAL_OUT_PIN 	4

// TODO: rotating address
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

static TimeSchedulerT< AvrTimer1 , int32_t > ts;

void setup()
{
	cli();
	
	// first thing to do in order to stay alive (otherwise power will shutdown
	relayPin.SetOutput();
	relayPin.Set( true );

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
	
	ts.start();
}

// allows for key combinations
#define POWER_BUTTON_MASK	0X04
#define UP_BUTTON_MASK		0X02
#define DOWN_BUTTON_MASK	0X01

static uint8_t gKeyState = 0;
static uint8_t gKeyPressed = 0;
static uint8_t gReadKey = 0;
static void updateKeyPressed()
{
	uint8_t s = ! powerOffPin.Get();
	uint8_t u = ! upPin.Get();
	uint8_t d = ! downPin.Get();
	uint8_t pc = s<<2 | u<<1 | d;
	if( pc >= gKeyState )
	{
		gKeyState = pc;
	}
	else 
	{
		if( gKeyPressed == 0 ) { gKeyPressed = gKeyState; }
		if( pc == 0 ) { gKeyState = 0; }
	}
}
static uint8_t readKeyPressed()
{
	uint8_t k = gKeyPressed;
	gKeyPressed = 0;
	return k;
}

static void updateTimerLCD()
{
}

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
		case 1 : // less delay 
			if( timerSeconds > 9 ) { timerSeconds -= 5; }
			if( timerRemainingSeconds > 5 ) timerRemainingSeconds -= 5;
			else { timerRemainingSeconds = 0; }
			timerSecondsChanged = true;
			break;
		case 2 : // more delay
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
	uint8_t bc = 0;

	ts.exec( 100000 , [](){ updateTimerLCD(); } ); // 100 mS
	ts.loop( 10000, [](int32_t){ updateKeyPressed(); } ); // 10 mS
	for(uint8_t i=0;i<88;i++) // 880 mS
	{
		bc = readKeyPressed();
		if( bc != 0 )
		{
		    ts.exec( 10000 , [&bc](){ processButton(bc); bc=0; } );
		    if( i < 78 )
		    {
			ts.exec( 100000 , [](){ updateTimerLCD(); } );
			i += 10;
		    }
		}
		else
		{
		    ts.loop( 10000 , [](int32_t){ updateKeyPressed(); } );
		}
	}
	
	ts.exec( 10000, []() { if( (--timerRemainingSeconds) <= 0 ) { powerOff(); } } ); // 10 mS
}

