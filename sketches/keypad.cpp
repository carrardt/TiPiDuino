#include <AvrTL.h>
#include <AvrTLPin.h>
#include <HWSerial/HWSerialIO.h>
#include <BasicIO/PrintStream.h>
#include <SignalProcessing/SignalProcessing.h>
#include <avr/pgmspace.h>

using namespace avrtl;

HWSerialIO serialIO;
PrintStream cout;

#define NCOLS 4
#define NROWS 4
#define COL_PIN(i) (2+i)
#define ROW_PIN(i) (2+4+i)
static auto led = StaticPin<13>();

void setup()
{
	serialIO.begin(9600);
	cout.begin( &serialIO );
	led.SetOutput();
	led.Set(LOW);
	for(int i=0;i<NCOLS;i++) { pinMode( COL_PIN(i) , INPUT_PULLUP ); }
	for(int i=0;i<NROWS;i++) { pinMode( ROW_PIN(i) , INPUT_PULLUP ); }
}

const char pgm_KeyTable[16] PROGMEM = {'D','C','B','A','#','9','6','3','0','8','5','2','*','7','4','1'};

char scanKeyPad()
{
	int k = -1;
	for(int i=0;i<NCOLS;i++)
	{
		pinMode( COL_PIN(i) , OUTPUT );
		digitalWrite( COL_PIN(i) , LOW );
		for(int j=0;j<NROWS;j++)
		{
			if( ! digitalRead(ROW_PIN(j)) )
			{
				digitalWrite( COL_PIN(i) , HIGH );
				pinMode( COL_PIN(i) , INPUT );
				char c = pgm_read_byte_near(pgm_KeyTable + i*NROWS+j);
				return c;
			}
		}
		digitalWrite( COL_PIN(i) , HIGH );
		pinMode( COL_PIN(i) , INPUT );
	}
	return '\0';
}

void loop()
{
	char c = scanKeyPad();
	if( c != '\0' )
	{
		SignalProcessing32 sp;
		cout<<c<<endl;
		sp.blink(led);
	}
}
