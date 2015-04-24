#include <AvrTL.h>
#include <AvrTLPin.h>
#include <LCD.h>
#include <HWSerialIO.h>
#include <PrintStream.h>

using namespace avrtl;

#define LCD_PINS 7,6,5,4,3,2 // respectively RS, EN, D7, D6, D5, D4
LCD<LCD_PINS> lcd;
HWSerialIO serialIO;
PrintStream lcdOut;
PrintStream serialOut;
static auto led = StaticPin<13>();

void setup()
{
	lcd.begin();
	lcdOut.begin( &lcd );
	serialIO.begin(9600);
	serialOut.begin( &serialIO );
	led.SetOutput();
}

static int COUNTER = 0;
void loop()
{
	lcdOut<<"Hello LCD\n"<<COUNTER<<'\n';
	serialOut<<"Hello Serial\n"<<COUNTER<<'\n';
	blink(led);
	++COUNTER;
}
