#include "RFSniffer.h"
#include "LCD.h"
#include "AvrTL.h"

using namespace avrtl;

#define EEPROM_BASE_ADDR ((void*)0x0004)

#define RECEIVE_PIN 8
#define LED_PIN 13
#define LCD_PINS 7,6,5,4,3,2 // respectively RS, EN, D7, D6, D5, D4

int main(void) __attribute__((noreturn));
int main(void)
{
	boardInit();

	auto rx = AvrPin<RECEIVE_PIN>();
	rx.SetInput();
	auto sniffer = make_sniffer(rx);

	RFSnifferProtocol sp;
	//sp.fromEEPROM(EEPROM_BASE_ADDR);

	LCD<LCD_PINS> lcd;
	lcd.begin();
	lcd << "RF sniffer\n" ;
	lcd << "Step 1: learn";

	auto led = AvrPin<LED_PIN>();
	led.SetOutput();

	while( ! sp.isValid() )
	{
		bool entropyOk = false;
		uint16_t entropybuf[2] = {0,0};
		{
			
			int n = sniffer.detectEntropyDrop<2,32>(entropybuf);
			entropyOk = ( n == 32 );
		}
		if( entropyOk )
		{
			lcd << "\nB0=" << entropybuf[0]<<", B1="<<entropybuf[1];
			blink(led);
		}
		
		if(0)
		if( sniffer.analyseSignal(sp) )
		{
			int mesgBits = sp.messageBits;
			if( sp.coding == CODING_MANCHESTER ) mesgBits /= 2;
			lcd << '\n';
			lcd << sp.nSymbols <<' '<< sp.latchSeqLen <<' '<< codingChar[sp.coding] << mesgBits << " x" << sp.nMessageRepeats << (sp.matchingRepeats?'+':'-') << '\n';
			lcd << sp.symbols[sp.nLatches]<<' '<<sp.symbols[sp.nLatches+1];
			blink(led);
			lcd << "\nStep2: read";
			if( sp.messageBits > MAX_MESSAGE_BITS ) sp.messageBits = MAX_MESSAGE_BITS;
			int nbytes = (sp.messageBits+7) / 8;
			uint8_t signal1[nbytes];
			int br;
			do { br = sniffer.readBinaryMessage(sp,signal1); } while( br==0 );
			if( br == sp.messageBits )
			{
				int tries=0;
				uint8_t signal2[nbytes];
				do
				{
					// don't loose time printing "step 3" if repeated signal can be catched right after the first one"
					br = sniffer.readBinaryMessage(sp,signal2);
					++tries;
					if( tries > 1000 ) lcd << "\nStep3: verify";
				} while( br==0 );
				for(int i=0;i<nbytes;i++) if(signal1[i]!=signal2[i]) br=0;
				if( br == sp.messageBits )
				{
					lcd << "\nWriting protocol";
					lcd << "\nto EEPROM ...";
					//sp.toEEPROM(EEPROM_BASE_ADDR);
					blink(led);
				}
			}
		}
	}
	lcd << "\n** RF sniffer **" ;
	lcd << "\n**** ready *****" ;
	for(;;)
	{
		int nbytes = (sp.messageBits+7) / 8;
		uint8_t buf[nbytes+1];
		if( sniffer.readBinaryMessage(sp,buf) != 0 )
		{
			bool mesgValid = true;
			if( sp.coding == CODING_MANCHESTER )
			{
				mesgValid = sniffer.decodeManchester(buf,sp.messageBits);
				nbytes = ( (sp.messageBits/2) + 7 ) / 8;
			}
			if( mesgValid )
			{
				lcd << "\n" ;
				for(int i=0;i<nbytes;i++)
				{
					uint8_t x = buf[i];
					bool half=false;
					do {
						uint8_t a = ( x & 0xF0 ) >> 4;
						char dg = a;
						if( a < 10 ) dg += '0';
						else { dg += 'A'-10; }
						lcd << dg;
						x <<= 4;
						half = !half;
					} while(half);
				}
			}
		}
	}
}

