#include <PCD8544/PCD8544.h>
#include <SoftSerial/SoftSerial.h>
#include <BasicIO/ByteStream.h>
#include <BasicIO/InputStream.h>
#include <avr/interrupt.h>
#include <AvrTL/AvrApp.h>

//#include "BasicIO/PrintStream.h"

/*
 * Important Note : if you connect screen's SCE pin to ground, it just works !
 * it seems SCE pin prevents undesired messages to be received.
 */


// PCD8544 pins :         SCLK, SDIN, DC, RST, SCE
#define LCD_PINS_ATTINY85    1,    2,  3,   4, PCD8544_UNASSIGNED
#define LCD_PINS_ATTINY84    5,    4,  3,   1, 2
#define LCD_PINS_ATMEGA328   8,    7,  6,   5, PCD8544_UNASSIGNED

#if defined(__AVR_ATtiny84__)
#define LCD_PINS LCD_PINS_ATTINY84
static avrtl::NullPin led;
#elif defined(__AVR_ATtiny85__)
#define LCD_PINS LCD_PINS_ATTINY85
static avrtl::NullPin led;
#elif defined(__AVR_ATmega328P__)
#define LCD_PINS LCD_PINS_ATMEGA328
static auto led = avrtl::StaticPin<13>();
#else
#error MCU not supported
#endif

#define SERIAL_SPEED 19200

static auto rx = avrtl::StaticPin<0>();
static auto tx = avrtl::NullPin();
using SerialScheduler = TimeSchedulerT<avrtl::AvrTimer0NoPrescaler>;
static auto serialIO = make_softserial_hr<SERIAL_SPEED,SerialScheduler>(rx,tx);
static PCD8544 lcd( LCD_PINS );

#define LCD_WIDTH 		84
#define LCD_HEIGHT 		48
#define CHARS_PER_ROW 14
#define NB_ROWS       6

static uint8_t lcd_buffer[NB_ROWS*CHARS_PER_ROW];
static int lcd_row = 0;

static void clearText()
{
	for(int i=0;i<(NB_ROWS*CHARS_PER_ROW);i++)
	{
		lcd_buffer[i] = ' ';
	}
	lcd_row = 0;
}

static void renderText()
{
	for(int r=0;r<NB_ROWS;r++)
	{
		lcd.setCursor(0, r);
		for(int i=0;i<CHARS_PER_ROW;i++) { lcd.writeByte(lcd_buffer[r*CHARS_PER_ROW+i]); }
	}
}

void setup()
{
	cli();

	// PCD8544-compatible displays may have a different resolution...
	lcd.begin(84, 48);

	// set medium contrast
	lcd.setContrast(63);

	serialIO.begin();
	
  static const char* welcome = "Ready to recv";
  const char*p=welcome; int i=0; while(*p!='\0') lcd_buffer[i++]=*(p++);
  lcd_row=1;
	renderText();
	
	serialIO.ts.start();
}

void loop()
{
   uint8_t* buf = lcd_buffer + lcd_row*CHARS_PER_ROW;
   {
		uint8_t i;
		uint8_t c='\0';
		for(i=0;i<CHARS_PER_ROW && c!='\n';i++)
		{
			c = serialIO.readByteFast() ;
			buf[i] = c;
		}
		while(c!='\n') { c = serialIO.readByteFast(); }
		for(;i<CHARS_PER_ROW;i++) buf[i]=' ';
		led = ! led;
	}
	
	//buf = lcd_buffer + lcd_row*CHARS_PER_ROW;
   if( buf[0]=='&' && buf[1]=='~' )
   {
	   BufferStream bufStream(buf+2,CHARS_PER_ROW-2);
	   InputStream cin;
	   int32_t x=0,y=0;
	   cin.begin(&bufStream);
	   char cmd='\0';
	   cin >> cmd;

	   switch(cmd)
	   {
		   case 'c':
		    cin >> x;
			lcd.setContrast(x);
			break;
		   case 'p':
			lcd.setPower(false);
			break;
		   case 'P':
			lcd.setPower(true);
			break;
		   case 'i':
			lcd.setInverse(false);
			break;
		   case 'I':
			lcd.setInverse(true);
			break;
		   case 'C':
			clearText();
			renderText();
			break;
		   case 'l':
		    cin >> x >> y;
			lcd.setCursor(x,y);
			break;
		   case 'D':
			{
				char c=0;
				while( !cin.eof() && c!='\n' )
				{
					cin >> c;
					if( c>=' ') { lcd.writeByte( c ); }
				}
			}
			break;
		   case 'd':
		    {
				cin >> x ;
				lcd.drawBitmap( (uint8_t*)&x , 4 , 1 );
			}
			break;
	   }
	   return;
   }
   
   renderText();
   if( lcd_row == (NB_ROWS-1) )
   {
		for(int i=0;i<((NB_ROWS-1)*CHARS_PER_ROW);i++)
		{
		   lcd_buffer[i]=lcd_buffer[i+CHARS_PER_ROW];
		}
   }
   else
   {
	   ++ lcd_row;
   }
}
