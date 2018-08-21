#include <AvrTL.h>
#include <SoftSerial.h>
#include <TimeScheduler/TimeScheduler.h>
#include <avr/eeprom.h>

// !!! Warning, PCD844 works with 3.3v !!!

#define RELAY_PIN  		2
#define TIME_INC_PIN 	1
#define TIME_DEC_PIN 	0
#define POWER_OFF_PIN 	3
#define SERIAL_OUT_PIN 	4

#define DEFAULT_TIMER_SECONDS 90

#define TIMER_EEPROM_ADDR0 ((uint8_t*)0x0010)
#define TIMER_EEPROM_ADDR1 ((uint8_t*)0x0011)
#define MAGICNUMBER_EEPROM_ADDR ((uint8_t*)0x0012)
#define MAGICNUMBER_VALUE ((uint8_t)0x76)

using namespace avrtl;

static auto powerOffPin = StaticPin<POWER_OFF_PIN>();
static auto upPin = StaticPin<TIME_INC_PIN>();
static auto downPin = StaticPin<TIME_DEC_PIN>();
static auto relayPin = StaticPin<RELAY_PIN>();

static bool upButtonState = false;
static bool downButtonState = false;
static bool powerOffButtonState = false;

static int32_t timerSeconds = DEFAULT_TIMER_SECONDS;
static int32_t timerRemainingSeconds = DEFAULT_TIMER_SECONDS;
static bool timerSecondsChanged = false;
static bool timerState = false;
static uint8_t timerCounter = 0;

static TimeSchedulerT< AvrTimer1<1024> , int32_t > ts;

void setup()
{
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

static void updateTimerLCD()
{
	static char counter_char[2] = { '_', '-' };
	
	uint32_t elapsed = timerSeconds - timerRemainingSeconds;
	if( elapsed > timerSeconds ) elapsed = 0;
	
	uint32_t minutes = timerRemainingSeconds / 60;
	if(minutes>99) minutes=99;
	
	uint32_t seconds = timerRemainingSeconds % 60;
	
	uint32_t pct = ( elapsed * 100UL ) / timerSeconds;
	uint32_t bar = ( elapsed * 16UL ) / timerSeconds;

/*	
	lcd.home();
	lcd.sendByte( '0'+(minutes/10) );
	lcd.sendByte( '0'+(minutes%10) );
	lcd.sendByte( 'M' );
	lcd.sendByte( 'n' );
	lcd.sendByte( ' ' );
	lcd.sendByte( '0'+(seconds/10) );
	lcd.sendByte( '0'+(seconds%10) );
	lcd.sendByte( 's' );
	lcd.sendByte( ' ' );
	
	if( timerState )
	{
		lcd.sendByte( '0'+(pct/100) );
		lcd.sendByte( '0'+(pct%100)/10 );
		lcd.sendByte( '0'+(pct%10) );
		lcd.sendByte( '%' );
		lcd.sendByte( ' ' );
	}
	else
	{
		lcd.sendByte( 'R' );
		lcd.sendByte( 'e' );
		lcd.sendByte( 'a' );
		lcd.sendByte( 'd' );
		lcd.sendByte( 'y' );
	}

	lcd.sendByte( ' ' );
	lcd.sendByte( timerState ? counter_char[timerCounter] : ' ' );
	*/
	
	timerCounter = (timerCounter+1)%2;

	//lcd.setCursor(0,1);
	//uint8_t i=0;
	//if( timerState ) { for(;i<bar;i++) lcd.sendByte( '#' ); }
	//for(;i<16;i++) lcd.sendByte( ' ' );
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
			if( timerSecondsChanged )
			{
				eeprom_write_byte( TIMER_EEPROM_ADDR0 , timerSeconds>>8 );
				eeprom_write_byte( TIMER_EEPROM_ADDR1 , timerSeconds&0xFF );
				while( true ) { relayPin.Set( false ); }
			}
			break;
	}
}

void loop()
{
	if( timerState )
	{
		ts.exec( 100000, [](){ updateTimerLCD(); } );
		uint8_t bc;
		for(uint8_t i=0;i<85;i++)
		{
			ts.exec( 10000, [&bc](){ bc = getButtonCode(); } );
			if(bc!=0) { processButton(bc); ts.reset(); return; }
		}
		ts.exec( 50000, [](){
			-- timerRemainingSeconds;
			if( timerRemainingSeconds <= 0 )
			{
				timerState = false;
				timerRemainingSeconds = timerSeconds;
			}
		});
	}
	else
	{
		updateTimerLCD();
		processButton( getButtonCode() );
		ts.reset();
	}
	...
}

