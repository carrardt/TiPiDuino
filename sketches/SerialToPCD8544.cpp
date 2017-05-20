#include <PCD8544.h>
#include <SoftSerial.h>
#include <avr/pgmspace.h>

#define SERIAL_SPEED 38400

static auto rx = avrtl::StaticPin<0>();
static auto tx = avrtl::NullPin();
using SerialScheduler = TimeSchedulerT<AvrTimer0NoPrescaler>;
static auto serialIO = make_softserial_hr<SERIAL_SPEED,SerialScheduler>(rx,tx);
static PCD8544 lcd( 2,3,4,5,6 );

const PROGMEM unsigned char message[8]={'B','o','n','j','o','u','r','\0'};

void setup() {
	cli();

  // PCD8544-compatible displays may have a different resolution...
  lcd.begin(84, 48);
  
    // set medium contrast
    lcd.setContrast(70);

	serialIO.begin();
	serialIO.ts.start();
}

#define CHARS_PER_ROW 14
#define NB_ROWS 6

static uint8_t buffer[NB_ROWS][CHARS_PER_ROW];
static int row = 0;

void renderText()
{
	for(int r=0;r<NB_ROWS;r++)
	{
		lcd.setCursor(0, r);
		for(int i=0;i<CHARS_PER_ROW;i++) { lcd.writeByte(buffer[r][i]); }
	}
}

void loop()
{
   uint8_t i = 0;
   char c = 0;
   
   do {
	c = serialIO.readByteFast();   
	buffer[row][i++]=c;
   } while( c!='\n' && i<CHARS_PER_ROW );
   for(;i<CHARS_PER_ROW;i++) buffer[row][i]=' ';
   
   if( buffer[row][0]=='&' && buffer[row][1]=='~' )
   {
	   switch(buffer[row][2])
	   {
		   case 'c':
			{
				int c = (buffer[row][3]-'0')*10+buffer[row][4]-'0';
				lcd.setContrast(c);
			}
			break;
	   }
	   return;
   }
   
   renderText();
   if( row == (NB_ROWS-1) )
   {
		for(int r=0;r<(NB_ROWS-1);r++){
		   for(i=0;i<CHARS_PER_ROW;i++) { buffer[r][i]=buffer[r+1][i]; }
		}
   }
   else
   {
	   ++ row;
   }
}
