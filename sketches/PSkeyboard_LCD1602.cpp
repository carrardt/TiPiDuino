#include <AvrTL.h>
#include <LCD.h>
#include <PrintStream.h>
#include <avr/pgmspace.h>

using namespace avrtl;

/*
 * LCD1602	Arduino
 * VSS		GND
 * VDD		+5v
 * VO		Pontentiometer(10K)/resistor
 * RS		D7
 * RW		GND
 * EN		D6
 * D4		D5
 * D5		D4
 * D6		D3
 * D7		D2
 * A		+5v
 * K		GND
 */

#define PS1_CLOCK 8
#define PS1_DATA 9
#define LCD_PINS 7,6,5,4,3,2

auto ps1Clock = StaticPin<PS1_CLOCK>();
auto ps1Data = StaticPin<PS1_DATA>();
LCD<LCD_PINS> lcd;
PrintStream cout;

static const char table[128] = 
{
'?', // 0
'?', // 1
'?', // 2
'?', // 3
'?', // 4
'?', // 5
'?', // 6
'?', // 7
'?', // 8
'?', // 9
'?', // 10
'?', // 11
'?', // 12
'?', // 13
'?', // 14
'?', // 15
'?', // 16
'?', // 17
'?', // 18
'?', // 19
'?', // 20
'A', // 21
'?', // 22
'?', // 23
'?', // 24
'?', // 25
'W', // 26
'S', // 27
'Q', // 28
'Z', // 29
'?', // 30
'?', // 31
'?', // 32
'C', // 33
'X', // 34
'D', // 35
'E', // 36
'?', // 37
'?', // 38
'?', // 39
'?', // 40
' ', // 41
'V', // 42
'F', // 43
'T', // 44
'R', // 45
'?', // 46
'?', // 47
'?', // 48
'N', // 49
'B', // 50
'H', // 51
'G', // 52
'Y', // 53
'?', // 54
'?', // 55
'?', // 56
'?', // 57
'?', // 58
'J', // 59
'U', // 60
'?', // 61
'?', // 62
'?', // 63
'?', // 64
'?', // 65
'K', // 66
'I', // 67
'O', // 68
'?', // 69
'?', // 70
'?', // 71
'?', // 72
'?', // 73
'?', // 74
'L', // 75
'M', // 76
'P', // 77
'?', // 78
'?', // 79
'?', // 80
'?', // 81
'?', // 82
'?', // 83
'?', // 84
'?', // 85
'?', // 86
'?', // 87
'?', // 88
'?', // 89
'\n', // 90
'?', // 91
'?', // 92
'?', // 93
'?', // 94
'?', // 95
'?', // 96
'?', // 97
'?', // 98
'?', // 99
'?', // 100
'?', // 101
'?', // 102
'?', // 103
'?', // 104
'1', // 105
'?', // 106
'4', // 107
'7', // 108
'?', // 109
'?', // 110
'?', // 111
'0', // 112
'?', // 113
'2', // 114
'5', // 115
'6', // 116
'8', // 117
'?', // 118
'?', // 119
'?', // 120
'+', // 121
'3', // 122
'-', // 123
'*', // 124
'9', // 125
'?', // 126
'?' // 127
};

void setup()
{
	lcd.begin();
	cout.begin( &lcd );
	ps1Clock.SetInput();
	ps1Data.SetInput();
}

static uint8_t readBit()
{
	while( ps1Clock.Get() );
	while( ! ps1Clock.Get() );
	return ps1Data.Get();
}

static inline char readKeyboard()
{	
	SCOPED_SIGNAL_PROCESSING;
	uint8_t x=0, p=0, one=0;
	while( readBit() != 0 );
	x = readBit();
	x |= readBit()<<1;
	x |= readBit()<<2;
	x |= readBit()<<3;
	x |= readBit()<<4;
	x |= readBit()<<5;
	x |= readBit()<<6;
	x |= readBit()<<7;
	p = readBit();
	one = readBit();
	uint8_t cp=0;
	for(int i=0;i<8;i++)
	{
		cp += (x>>i)&0x01;
	}
	cp &= 0x01;
	if( cp == p ) return '!';
	if( one != 1 ) return '?';
	return x;
}

void loop()
{
	uint8_t c,c2;
	c = readKeyboard();
	do { c2 = readKeyboard(); } while( c2 != 0xF0 );
	c2 = readKeyboard();
	if( c != c2 ) return;
	if( c >= 128 ) return;
	cout << (char) table[c];
	//cout << (int)c << '\n';
	/*cout.print(c,16,2);
	cout<<' ';
	++COUNTER;
	if( (COUNTER % 4) == 0 ) cout <<'\n';*/
}
