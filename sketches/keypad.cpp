#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <BasicIO/HWSerialIO.h>
#include <BasicIO/PrintStream.h>

#include <avr/pgmspace.h>

using namespace avrtl;

HWSerialIO serialIO;
PrintStream cout;

#define NCOLS 4
#define NROWS 4
static DynamicPin colwires[NCOLS];
static DynamicPin rowwires[NROWS];
static auto led = StaticPin<13>();

void setup()
{
	serialIO.begin(9600);
	cout.begin( &serialIO );
	led.SetOutput();
	led.Set(LOW);
	for(int i=0;i<NCOLS;i++)
	{
		colwires[i].setPinId(2+i);
		colwires[i].SetInputPullup();
	}
	for(int i=0;i<NROWS;i++)
	{
		rowwires[i].setPinId(2+4+i);
		rowwires[i].SetInputPullup();
	}
}

const char pgm_KeyTable[16] PROGMEM = {'D','C','B','A','#','9','6','3','0','8','5','2','*','7','4','1'};

char scanKeyPad()
{
	int k = -1;
	for(int i=0;i<NCOLS;i++)
	{
		colwires[i].SetOutput();
		colwires[i].Set(LOW);
		for(int j=0;j<NROWS;j++)
		{
			if( !rowwires[j].Get() )
			{
				colwires[i].Set(HIGH);
				colwires[i].SetInput();
				char c = pgm_read_byte_near(pgm_KeyTable + i*NROWS+j);
				return c;
			}
		}
		colwires[i].Set(HIGH);
		colwires[i].SetInput();
	}
	return '\0';
}

void loop()
{
	char c = scanKeyPad();
	if( c != '\0' )
	{
		cout<<c<<endl;
		blink(led);
	}
}
