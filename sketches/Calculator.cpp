#include <PCD8544.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

static auto led = avrtl::StaticPin<10>();

/*
 * Important Note : if you connect screen's SCE pin to ground, it just works !
 * it seems SCE pin prevents undesired messages to be received.
 */
 
// PCD8544 pins : SCLK, SDIN, DC, RST, SCE
#define LCD_SCLK 	4
#define LCD_SDIN 	3
#define LCD_DC 		2
#define LCD_RST 	0
#define LCD_SCE 	1

#define LCD_WIDTH 		84
#define LCD_HEIGHT 		48
#define LCD_CHARS_PER_ROW 	14
#define LCD_NB_ROWS      6

#define LCD_PINS LCD_SCLK,LCD_SDIN,LCD_DC,LCD_RST,LCD_SCE

static PCD8544 lcd( LCD_PINS );

enum CalculatorMode { Calculator=0 , Quizz=1 };
enum KeyPosition { K_1, K_2, K_3, K_Plus, K_4, K_5, K_6, K_Minus, K_7, K_8, K_9, K_Mult, K_Mode, K_0, K_Equal, K_Div };

uint8_t calculator_mode = Calculator;

uint8_t operation = 0;
uint8_t opi = 0;
bool opValid[3] = { false, false, false };
int32_t operand[3] = { 0 , 0 , 0 };


void resetData()
{
	operation = 0;
	opi = 0;
	operand[0] = 0 ;
	operand[1] = 0 ;
	operand[2] = 0 ;
	opValid[0] = false;
	opValid[1] = false;
	opValid[2] = false;
}

void writeOperand(int i,int row)
{
	if( opValid[i] )
	{
		int32_t x = operand[i];
		char signChar = ' ';
		if(x<0) { signChar='-'; x=-x; }
		int p=13;
		do
		{
			int d = x % 10;
			x /= 10;
			lcd.setCursor(p*6,row);
			lcd.writeByte( '0'+d );
			--p;
		} while( x > 0 );
		lcd.setCursor(p*6,row);
		lcd.writeByte( signChar );
	}
	else if( calculator_mode==Quizz )
	{
		lcd.setCursor(40,row);
		lcd.writeString("???");
	}
}

void printCalculatorScreen()
{
	lcd.clear();
	writeOperand(0,0);
	
	lcd.setCursor(0,1);
	char opc = ' ';
	switch(operation)
	{
		case K_Plus : opc = '+'; break;
		case K_Minus : opc = '-'; break;
		case K_Mult : opc = '*'; break;
		case K_Div : opc = '/'; break;
	}
	lcd.writeByte(opc);

	writeOperand(1,2);
	
	lcd.setCursor(0,3);
	lcd.writeString("--------------");
	lcd.setCursor(0,4);
	lcd.writeByte('=');

	writeOperand(2,5);
}

void printMessage(const char* s)
{
	lcd.clear();
	lcd.setCursor(6, 2);
	lcd.writeString(s);
	delay(2000);
}

void printCalculatorMode()
{
	lcd.clear();
	switch(calculator_mode)
	{
		case Calculator:
			lcd.setCursor(6, 2);
			lcd.writeString("Calculatrice");
			break;
		case Quizz:
			lcd.setCursor(30, 2);
			lcd.writeString("Quizz");
			break;
	}
	avrtl::blink(led);
	led = false;
}

void refreshCalculator()
{
	lcd.setCursor(0, 0);
	
}

void setup()
{
	cli();

	led.SetOutput();
	led = false;

	// PCD8544-compatible displays may have a different resolution...
	lcd.begin(84, 48);

	// set medium contrast
	lcd.setContrast(63);

	printCalculatorMode();
}

int keyToDigit(int k)
{
	switch(k)
	{
		case K_0 : return 0;
		case K_1 : return 1;
		case K_2 : return 2;
		case K_3 : return 3;
		case K_4 : return 4;
		case K_5 : return 5;
		case K_6 : return 6;
		case K_7 : return 7;
		case K_8 : return 8;
		case K_9 : return 9;
		default : return -1;
	}
}
bool isDigit(int k)
{
	return keyToDigit(k)>=0;
}

#define NCOLS 4
#define NROWS 4
int COL_PIN(int i)
{
	switch(i){
		case 0: return 2;
		case 1: return 3;
		case 2: return 4;
		case 3: return 5;
	}
	return 1;
}
int ROW_PIN(int i)
{
	switch(i){
		case 0: return 6;
		case 1: return 7;
		case 2: return 8;
		case 3: return 9;
	}
}

int scanKeyPad()
{
	int k = -1;
	for(int i=0;i<NCOLS && k==-1;i++)
	{
		pinMode( COL_PIN(i) , OUTPUT );
		digitalWrite( COL_PIN(i) , LOW );
		for(int j=0;j<NROWS && k==-1;j++)
		{
			if( ! digitalRead(ROW_PIN(j)) )
			{
				k = i*NROWS+j;
			}
		}
		pinMode( COL_PIN(i) , INPUT_PULLUP );
	}
	return k;
}

int readKeyPad()
{
	for(int i=0;i<NCOLS;i++) { pinMode( COL_PIN(i) , INPUT_PULLUP ); }
	for(int i=0;i<NROWS;i++) { pinMode( ROW_PIN(i) , INPUT_PULLUP ); }
	
	int k = -1;
	while(k==-1) { k = scanKeyPad(); }
	led = true;
	int l = 0;
	while(l!=-1) { l = scanKeyPad(); }
	led = false;

	pinMode( LCD_SCLK , OUTPUT );
	digitalWrite( LCD_SCLK , LOW );
	pinMode( LCD_SDIN , OUTPUT );
	digitalWrite( LCD_SDIN , LOW );
	pinMode( LCD_DC , OUTPUT );
	digitalWrite( LCD_DC , LOW );

	return k;
}

int32_t computeOperation()
{
	int r = -1 ;
	if( opValid[0] and opValid[1] )
	{
		switch(operation)
		{
			case K_Plus : 	r = operand[0] + operand[1]; break;
			case K_Minus : 	r = operand[0] - operand[1]; break;
			case K_Mult : 	r = operand[0] * operand[1]; break;
			case K_Div : 	r = operand[0] / operand[1]; break;
		}
	}
	return r;
}

void chooseRandomOperands()
{
	srand(TCNT0);
	int32_t a=0,b=0;
	while(a==0) a = rand() % 1000;
	while(b==0) b = rand() % 1000;
	if( ( operation==K_Div or operation==K_Minus ) and a<b )
	{
		int32_t r=a; a=b; b=r;
	}
	if( operation==K_Div )
	{
		b /= 10;
		if(b<2) { b=2; }
		int32_t r = a/b;
		if(r<2) { r=2; }
		a = b * r;
	}
	operand[0] = a;
	operand[1] = b;
	operand[2] = 0;
	opValid[0] = true;
	opValid[1] = true;
	opValid[2] = false;
	opi = 2;
}

void loop()
{
	int k = readKeyPad();

	if( opi==2 and calculator_mode == Calculator)
	{
		resetData();
	}

	int digit = keyToDigit(k);
	if( digit>=0 )
	{
		if(operand[opi]==-1) { operand[opi]=0; }
		operand[opi] *= 10;
		operand[opi] += digit;
		opValid[opi] = true;
	}
	else
	{
		switch( k )
		{
			case K_Plus : 
			case K_Minus : 
			case K_Mult : 
			case K_Div :
				if( calculator_mode == Calculator )
				{
					if(opi==1)
					{
						operand[0] = computeOperation();
					}
					opi = 1;
					opValid[opi] = false;
				}
				operation = k;
				if( calculator_mode == Quizz )
				{
					chooseRandomOperands();
				}
				break;
				
			case K_Equal:
				if( calculator_mode == Calculator )
				{
					if(opi==1)
					{
						opi = 2;
						operand[opi] = computeOperation();
						opValid[opi] = true;
					}
				}
				else
				{
					if( opValid[2] )
					{
						int32_t r = computeOperation();
						if( r == operand[2] ) {	printMessage("Gagne :-)"); }
						else { printMessage("Perdu :-("); }
					}
					chooseRandomOperands();
				}
				break;
				
			case K_Mode:
				calculator_mode = 1 - calculator_mode;
				printCalculatorMode();
				resetData();
				break;
		}
	}
	printCalculatorScreen();
}
