#include <PCD8544.h>
#include <SoftSerial.h>
#include "BasicIO/ByteStream.h"
#include "BasicIO/InputStream.h"

/*
 * Important Note : if you connect screen's SCE pin to ground, it just works !
 * it seems SCE pin prevents undesired messages to be received.
 */

#define LCD_PINS_ATTINY85 1,2,3,4,PCD8544_UNASSIGNED
#define LCD_PINS_ATMEGA328 2,3,4,5,6,7 
#define SERIAL_SPEED 19200

static auto rx = avrtl::StaticPin<0>();
static auto tx = avrtl::NullPin();
using SerialScheduler = TimeSchedulerT<AvrTimer0NoPrescaler>;
static auto serialIO = make_softserial_hr<SERIAL_SPEED,SerialScheduler>(rx,tx);
static PCD8544 lcd( LCD_PINS_ATTINY85 );

void setup() {
	cli();

  // PCD8544-compatible displays may have a different resolution...
  lcd.begin(84, 48);
  
    // set medium contrast
    lcd.setContrast(50);

	serialIO.begin();
	serialIO.ts.start();
}

#define CHARS_PER_ROW 14
#define NB_ROWS 6

static uint8_t lcd_buffer[NB_ROWS][CHARS_PER_ROW];
static int lcd_row = 0;

static void renderText()
{
	for(int r=0;r<NB_ROWS;r++)
	{
		lcd.setCursor(0, r);
		for(int i=0;i<CHARS_PER_ROW;i++) { lcd.writeByte(lcd_buffer[r][i]); }
	}
}

void loop()
{
   uint8_t i = 0;
   char c = 0;
   
   do {
	c = serialIO.readByteFast();   
	lcd_buffer[lcd_row][i++]=c;
   } while( c!='\n' && i<CHARS_PER_ROW );
   for(;i<CHARS_PER_ROW;i++) lcd_buffer[lcd_row][i]=' ';
   
   if( lcd_buffer[lcd_row][0]=='&' && lcd_buffer[lcd_row][1]=='~' )
   {
	   BufferStream bufStream(lcd_buffer[lcd_row]+2,CHARS_PER_ROW-2);
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
			for(int r=0;r<NB_ROWS;r++) for(i=0;i<CHARS_PER_ROW;i++) { lcd_buffer[r][i]=' '; }
			lcd_row=0;
			renderText();
			break;
		   case 'l':
		    cin >> x >> y;
			lcd.setCursor(x,y);
			break;
		   case 'd':
			cin >> x ;
			lcd.drawBitmap( (uint8_t*)&x , 4 , 1 );
			break;
		   case 'D':
			cin >> cmd ;
			lcd.writeByte( cmd );
			break;
	   }
	   return;
   }
   
   renderText();
   if( lcd_row == (NB_ROWS-1) )
   {
		for(int r=0;r<(NB_ROWS-1);r++){
		   for(i=0;i<CHARS_PER_ROW;i++) { lcd_buffer[r][i]=lcd_buffer[r+1][i]; }
		}
   }
   else
   {
	   ++ lcd_row;
   }
}
