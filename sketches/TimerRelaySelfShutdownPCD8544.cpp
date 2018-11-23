#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <AvrTL.h>
#include <AvrTLPin.h>
#include <TimeScheduler/TimeScheduler.h>
#include <PCD8544.h>


/*
 * Tailored for Attiny84
 */

// PCD8544 pins : CLK, DIN, DC, RST, SCE
#define LCD_PINS    5,   4,  3,   1,   2
#define TIME_INC_PIN 	6
#define TIME_DEC_PIN 	7
#define POWER_OFF_PIN 	8
#define RELAY_PIN  		9
#define LED_PIN  		0
#define DEBUG_PIN		10

#define DEFAULT_TIMER_SECONDS 300

// eeprom management
#define EEPROM_SIZE			512
#define EEPROM_ADDR_MAX		508
#define EEPROM_HEADER_FLAG	0x81
#define EEPROM_DEBUG_FLAG	0x02
#define EEPROM_FIRST_FLAG	0x04
uint8_t EEMEM __UPLOAD_EEPROM_VALUES__[3] = { EEPROM_HEADER_FLAG|EEPROM_FIRST_FLAG , DEFAULT_TIMER_SECONDS>>8, DEFAULT_TIMER_SECONDS&0xFF };

using namespace avrtl;

// pins
static auto powerOffPin = avrtl::StaticPin<POWER_OFF_PIN>();
static auto upPin = avrtl::StaticPin<TIME_INC_PIN>();
static auto downPin = avrtl::StaticPin<TIME_DEC_PIN>();
static auto relayPin = avrtl::StaticPin<RELAY_PIN>();
static auto ledPin = avrtl::StaticPin<LED_PIN>();
static auto debugPin = avrtl::StaticPin<DEBUG_PIN>();

// timer
static TimeSchedulerT< AvrTimer0 , int32_t > ts;

static int16_t timerSeconds = DEFAULT_TIMER_SECONDS;
static int16_t timerRemainingSeconds = DEFAULT_TIMER_SECONDS;
static bool timerSecondsChanged = false;
static bool backupOnPowerOff = false;
static bool timerRemainingSecondsChanged = false;
static bool timerLedState = false;

// rotating address of eeprom data
static uint16_t eeprom_address = 0;

// display
static PCD8544 lcdIO;

const PROGMEM unsigned char digits_bitmap[] = {
0x00,0x00,0x00,0xF8,0xFC,0xFC,0x04,0x84,0xE4,0x74,0xFC,0xFC,0xF8,0x00,0x00,0x00,
0x00,0x00,0x00,0x60,0x60,0x60,0xF0,0xFC,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x18,0x1C,0x1C,0x04,0x04,0x84,0xCC,0xFC,0x78,0x30,0x00,0x00,0x00,
0x00,0x00,0x00,0x18,0x1C,0x1C,0x84,0x84,0x84,0xCC,0xFC,0x78,0x30,0x00,0x00,0x00,
0x00,0x00,0x00,0x80,0xC0,0x60,0x30,0x18,0xFC,0xFC,0xFC,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFC,0xFC,0xFC,0x84,0x84,0x84,0x84,0x84,0x04,0x04,0x00,0x00,0x00,
0x00,0x00,0x00,0xE0,0xF0,0xF8,0x9C,0x8C,0x84,0x84,0x84,0x80,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x3C,0x3C,0x04,0x04,0x04,0x04,0x84,0xFC,0xFC,0x7C,0x00,0x00,
0x00,0x00,0x00,0x78,0x7C,0xFC,0xC4,0xC4,0x84,0x84,0xFC,0x7C,0x78,0x00,0x00,0x00,
0x00,0x00,0x00,0xF8,0xFC,0xFC,0x84,0x84,0x84,0x84,0xFC,0xFC,0xF8,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x70,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x1F,0x3F,0x3F,0x2E,0x27,0x21,0x20,0x3F,0x3F,0x1F,0x00,0x00,0x00,
0x00,0x00,0x00,0x20,0x20,0x20,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x30,0x38,0x3C,0x2E,0x27,0x23,0x21,0x38,0x38,0x38,0x00,0x00,0x00,
0x00,0x00,0x00,0x18,0x38,0x38,0x21,0x21,0x21,0x33,0x3F,0x1E,0x0C,0x00,0x00,0x00,
0x00,0x00,0x00,0x03,0x03,0x03,0x23,0x23,0x3F,0x3F,0x3F,0x23,0x23,0x00,0x00,0x00,
0x00,0x00,0x00,0x19,0x39,0x39,0x21,0x21,0x21,0x33,0x3F,0x1F,0x0E,0x00,0x00,0x00,
0x00,0x00,0x00,0x1F,0x3F,0x3F,0x21,0x21,0x21,0x21,0x3F,0x3F,0x1F,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x3C,0x3E,0x07,0x03,0x01,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x1E,0x3E,0x3F,0x21,0x21,0x23,0x23,0x3F,0x3E,0x1E,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x01,0x21,0x21,0x21,0x31,0x39,0x1F,0x0F,0x07,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x0E,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static bool g_firstRun = false;
static bool g_debugEnabled = false;
static bool debugEnabled()
{
	return g_firstRun || g_debugEnabled || (!debugPin.Get());
}

static void writeBigDigit(uint16_t col, uint16_t line, uint16_t d)
{
	unsigned char buf[16];
	lcdIO.setCursor(col, line);
	ts.wallclock();
	memcpy_P(buf, &digits_bitmap[d*16],16);
	ts.wallclock();
	lcdIO.drawBitmap(buf,16,1);
	ts.wallclock();
	lcdIO.setCursor(col, line+1);
	ts.wallclock();
	memcpy_P(buf, &digits_bitmap[176+d*16],16);
	ts.wallclock();
	lcdIO.drawBitmap(buf,16,1);
	ts.wallclock();
}


static void secondsToTime(uint16_t seconds, uint8_t* t)
{
	uint16_t m = seconds / 60;
	ts.wallclock();
	uint16_t m1 = m / 10;
	ts.wallclock();
	uint16_t m2 = m - ( m1 * 10 );
	ts.wallclock();
	seconds -= m * 60;
	ts.wallclock();
	uint16_t s1 = seconds / 10;
	ts.wallclock();
	uint16_t s2 = seconds - (s1 * 10);
	ts.wallclock();	
	t[0] = m1;
	t[1] = m2;
	t[2] = s1;
	t[3] = s2;
}

static void writeBigTime(uint16_t col, uint16_t line, uint16_t seconds)
{
	uint8_t t[4];
	secondsToTime(seconds,t);
	writeBigDigit(col,line,t[0]); col+=16;
	writeBigDigit(col,line,t[1]); col+=16;
	writeBigDigit(col,line,10); col+=16;
	writeBigDigit(col,line,t[2]); col+=16;
	writeBigDigit(col,line,t[3]);
}

static void writeLCDMessage(const char* s)
{
	while( (*s) != '\0' )
	{
		ts.wallclock();
		lcdIO.writeByte( *s );
		++ s;
	}
	ts.wallclock();
}

static const char* int2str(int16_t i,char* number_str)
{
	char* s = number_str;
	if(i<0)
	{
		*s++ = '-';
		i = -i;
	}
	ts.wallclock();
	int16_t d = 10000;
	do
	{
		uint8_t x =  (i/d);
		ts.wallclock();
		i -= x*d ;
		*s++ = '0' + x;
		ts.wallclock();
		d /= 10;
		ts.wallclock();
	}while(d>0);
	*s = '\0';
	return number_str;
}

static void writeLCDInt(int16_t i)
{
	char number_str[8];
	writeLCDMessage(int2str(i,number_str));
}

static void writeLCDTotalTime()
{
		ts.wallclock();
		lcdIO.setCursor(0, 0);
		ts.wallclock();
		writeLCDMessage("Tempo. ");
		uint8_t t[4];
		secondsToTime(timerSeconds,t);
		lcdIO.writeByte( '0'+t[0] );
		ts.wallclock();
		lcdIO.writeByte( '0'+t[1] );
		ts.wallclock();
		lcdIO.writeByte( ':' );
		ts.wallclock();
		lcdIO.writeByte( '0'+t[2] );
		ts.wallclock();
		lcdIO.writeByte( '0'+t[3] );
		ts.wallclock();
		timerSecondsChanged = false;
}

static void writeLCDRemainingTime()
{
		ts.wallclock();
		writeBigTime(0,2,timerRemainingSeconds);
		timerRemainingSecondsChanged = false;
}

static void writeLCDDebugInfo(const char* s, int16_t i)
{
		if( debugEnabled() )
		{
			char number_str[8];
			ts.wallclock();
			lcdIO.setCursor(0, 4);
			ts.wallclock();
			writeLCDMessage(s);
			ts.wallclock();
			writeLCDMessage(int2str(i,number_str));
			ts.wallclock();
		}
}

static void readEEPROMData()
{
	uint16_t addr = 0;
	uint8_t eem = 0;
	while( ( (eem=eeprom_read_byte((uint8_t*)addr)) & EEPROM_HEADER_FLAG ) != EEPROM_HEADER_FLAG && addr<EEPROM_ADDR_MAX ) ++addr;
	
	timerSeconds = eeprom_read_byte( (uint8_t*)(addr+1) )<<8 | eeprom_read_byte( (uint8_t*)(addr+2) );
	if( timerSeconds > 3600 || timerSeconds < 5 )
	{
		timerSeconds = DEFAULT_TIMER_SECONDS;
	}
	g_firstRun = (eem & EEPROM_FIRST_FLAG)!=0 || addr>=EEPROM_ADDR_MAX;
	g_debugEnabled = (eem & EEPROM_DEBUG_FLAG)!=0 ;
	
	eeprom_address = addr;
	timerRemainingSeconds = timerSeconds;
}

void setup()
{	
	// first thing to do in order to stay alive (otherwise power will shutdown
	relayPin.SetOutput();
	relayPin.Set( true );

	// we do not use any interrupt
	cli();

	// led pin
	ledPin.SetOutput();
	ledPin.Set(false);

	// command buttons
	powerOffPin.SetInput();
	powerOffPin.Set( HIGH ); // pullup
	upPin.SetInput();
	upPin.Set( HIGH ); // pullup
	downPin.SetInput();
	downPin.Set( HIGH ); // pullup
	debugPin.SetInput();
	debugPin.Set( HIGH ); // pullup
	
	// PCD8544 LCD display
	lcdIO.setPins( LCD_PINS );
	lcdIO.begin(84, 48);
	lcdIO.setContrast(63);

	readEEPROMData();

	ts.start();
	
	if( debugEnabled() )
	{
		if( g_firstRun )
		{
			writeLCDDebugInfo("1strun@",eeprom_address);
		}
		else
		{
			writeLCDDebugInfo("read@",eeprom_address);
		}
		auto setCursorTime = ts.wallclock();
		lcdIO.setCursor(0, 0);
		setCursorTime = ts.wallclock() - setCursorTime;
		auto writeByteTime = ts.wallclock();
		lcdIO.writeByte('X');
		writeByteTime = ts.wallclock() - writeByteTime;
		lcdIO.setCursor(0, 5);
		ts.wallclock();
		writeLCDInt(setCursorTime);
		lcdIO.writeByte('/');
		ts.wallclock();
		writeLCDInt(writeByteTime);
		ts.wallclock();
	}
	
	writeLCDTotalTime();
	writeLCDRemainingTime();
}

// allows for key combinations
#define POWER_BUTTON_MASK	0X04
#define UP_BUTTON_MASK		0X02
#define DOWN_BUTTON_MASK	0X01


// while a new key is pressed, add key press code.
static uint8_t g_keyPressed = 0;
static uint8_t getKeyCode()
{
	uint8_t s = ! powerOffPin.Get();
	uint8_t u = ! upPin.Get();
	uint8_t d = ! downPin.Get();
	uint8_t k = s<<2 | u<<1 | d;
	
	if( k != 0 ) { g_keyPressed |= k; return 0; }
	else { k=g_keyPressed; g_keyPressed=0; return k; }
}

static void powerOff()
{
	if( backupOnPowerOff )
	{
		eeprom_write_byte( (uint8_t*)(eeprom_address++) , 0 );
		eeprom_write_byte( (uint8_t*)(eeprom_address++) , 0 );
		eeprom_write_byte( (uint8_t*)(eeprom_address++) , 0 );
		if(eeprom_address >= EEPROM_ADDR_MAX ) { eeprom_address = 0; }
		writeLCDDebugInfo("backup@",eeprom_address);
		uint8_t h = EEPROM_HEADER_FLAG;
		if( g_debugEnabled ) { h |= EEPROM_DEBUG_FLAG; }
		eeprom_write_byte( (uint8_t*)(eeprom_address++) , h );
		eeprom_write_byte( (uint8_t*)(eeprom_address++) , timerSeconds>>8 );
		eeprom_write_byte( (uint8_t*)(eeprom_address++) , timerSeconds&0xFF );
	}
	ledPin.Set(true);
	while( true ) { relayPin.Set( false ); }
}

static void processButton(uint8_t bc)
{
	switch( bc )
	{
		case DOWN_BUTTON_MASK : // less delay 
			if( timerSeconds > 9 ) { timerSeconds -= 5; }
			if( timerRemainingSeconds > 5 ) timerRemainingSeconds -= 5;
			else { timerRemainingSeconds = 0; }
			timerSecondsChanged = true;
			backupOnPowerOff = true;
			timerRemainingSecondsChanged = true;
			break;
			
		case UP_BUTTON_MASK : // more delay
			timerSeconds += 5;
			if( timerSeconds > 3595 ) { timerSeconds = 3595; }
			else { timerRemainingSeconds += 5; }
			timerSecondsChanged = true;
			backupOnPowerOff = true;
			timerRemainingSecondsChanged = true;
			break;
			
		case DOWN_BUTTON_MASK|UP_BUTTON_MASK : // reset to default time value
			timerSeconds = DEFAULT_TIMER_SECONDS;
			timerRemainingSeconds = DEFAULT_TIMER_SECONDS;
			timerSecondsChanged = true;
			backupOnPowerOff = true;
			timerRemainingSecondsChanged = true;
			break;
			
		case POWER_BUTTON_MASK : // poweroff
			powerOff();
			
		case DOWN_BUTTON_MASK|UP_BUTTON_MASK|POWER_BUTTON_MASK : // toggle debug mode
			g_debugEnabled = ! g_debugEnabled;
			backupOnPowerOff = true;
			break;
	}
}

void updateTime()
{
	if( ts.wallclock() >= ts.m_timer.TicksPerSecond )
	{
		ts.rewindWallClock(ts.m_timer.TicksPerSecond);
		-- timerRemainingSeconds;
		timerRemainingSecondsChanged = true;
		timerLedState = !timerLedState;
		ts.wallclock();
		ledPin.Set(timerLedState);
		ts.wallclock();
	}
}

void loop()
{	
	updateTime();
	processButton( getKeyCode() );
	ts.wallclock();

	if( timerSecondsChanged )
	{
		writeLCDTotalTime();
	}

	if( timerRemainingSecondsChanged )
	{
		writeLCDRemainingTime();
	}

	if( timerRemainingSeconds == 0 )
	{
		powerOff();
	}

}

