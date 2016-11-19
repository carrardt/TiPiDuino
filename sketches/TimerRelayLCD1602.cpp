#include <AvrTL.h>
#include <LCD1602.h>
#include <TimeScheduler/TimeScheduler.h>
#include <avr/eeprom.h>

// the following wiring is compatible with the circuit in pcb/TimerRelayLCD1602.fzz
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

#define BUTTON_START 14 // aka A0
#define BUTTON_UP    15 // aka A1
#define BUTTON_DOWN  16 // aka A2
#define RELAY_CMD    17 // aka A3

#define DEFAULT_TIMER_SECONDS 105

#define TIMER_EEPROM_ADDR0 ((uint8_t*)0x0010)
#define TIMER_EEPROM_ADDR1 ((uint8_t*)0x0011)

using namespace avrtl;

LCD1602<LCD_PINS> lcd;

static auto startPin = StaticPin<BUTTON_START>();
static auto upPin = StaticPin<BUTTON_UP>();
static auto downPin = StaticPin<BUTTON_DOWN>();
static auto relayPin = StaticPin<RELAY_CMD>();

static bool startButtonState = false;
static bool upButtonState = false;
static bool downButtonState = false;
static int32_t timerSeconds = DEFAULT_TIMER_SECONDS;
static int32_t timerRemainingSeconds = DEFAULT_TIMER_SECONDS;
static bool timerSecondsChanged = false;
static bool timerState = false;
static uint8_t timerCounter = 0;

static TimeSchedulerT< AvrTimer1<1024> , int32_t > ts;

void setup()
{
	lcd.begin();
	
	startPin.SetInput();
	startPin.Set( HIGH ); // pullup

	upPin.SetInput();
	upPin.Set( HIGH ); // pullup

	downPin.SetInput();
	downPin.Set( HIGH ); // pullup
	
	relayPin.SetOutput();
	relayPin.Set( false );
	
	timerSeconds = eeprom_read_byte( TIMER_EEPROM_ADDR0 )<<8 | eeprom_read_byte( TIMER_EEPROM_ADDR1 );
	if( timerSeconds > 3600 )
	{
		timerSeconds = DEFAULT_TIMER_SECONDS;
		timerSecondsChanged = true;
	}
	timerRemainingSeconds = timerSeconds;
	ts.start();
}

static uint8_t getButtonCode()
{
	uint8_t s = ! startPin.Get();
	uint8_t u = ! upPin.Get();
	uint8_t d = ! downPin.Get();
	uint8_t pc = s<<2 | u<<1 | d;
	uint8_t press_code;
	if( pc == 0 ) { return 0; }
	
	do
	{
		press_code = pc;
		s = ! startPin.Get();
		u = ! upPin.Get();
		d = ! downPin.Get();
		pc = s<<2 | u<<1 | d;
	} while(  pc >= press_code );
	
	do
	{
		s = ! startPin.Get();
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
	timerCounter = (timerCounter+1)%2;

	lcd.setCursor(0,1);
	uint8_t i=0;
	if( timerState ) { for(;i<bar;i++) lcd.sendByte( '#' ); }
	for(;i<16;i++) lcd.sendByte( ' ' );
}

static void processButton(uint8_t bc)
{
	switch( bc )
	{
		case 3 :
			timerSeconds = DEFAULT_TIMER_SECONDS;
			timerRemainingSeconds = DEFAULT_TIMER_SECONDS;
			timerSecondsChanged = true;
			break;
		case 1 :
			if( timerSeconds > 5 ) { timerSeconds -= 5; }
			if( timerRemainingSeconds > 5 ) timerRemainingSeconds -= 5;
			else { timerRemainingSeconds = 0; }
			timerSecondsChanged = true;
			break;
		case 2 :
			timerSeconds += 5;
			timerRemainingSeconds += 5;
			timerSecondsChanged = true;
			break;
		case 4 :
			timerState = ! timerState;
			if( !timerState ) { timerRemainingSeconds = timerSeconds; }
			if( timerSecondsChanged )
			{
				eeprom_write_byte( TIMER_EEPROM_ADDR0 , timerSeconds>>8 );
				eeprom_write_byte( TIMER_EEPROM_ADDR1 , timerSeconds&0xFF );
				timerSecondsChanged = false;
			}
			break;
	}
}

void loop()
{
	relayPin.Set( timerState );
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
}

