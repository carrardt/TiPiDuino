#include "RFSniffer.h"
//#include "RFEmitter.h"
#include "AvrTL.h"

using namespace avrtl;

// what to use as a console to prin message
// #define LCD_CONSOLE 1

// for latch detection based analysis
#define MIN_LATCH_LEN 		2000
#define MIN_PROLOG_LATCHES 	1
#define MIN_MESSAGE_PULSES 	64
#define ENTROPY_DETECTION_PULSES 16
#define MAX_PULSES 			512

// EEPROM address where to write detected protocol
#define EEPROM_BASE_ADDR ((void*)0x0004)
#define RESET_SEQ 0x05 // press AABBA, A and B being any two different remote buttons

// pinout
#define RECEIVE_PIN 8
#define EMIT_PIN 12
#define LED_PIN 13

#ifdef LCD_CONSOLE
#include "LCD.h"
#define LCD_PINS 7,6,5,4,3,2 // respectively RS, EN, D7, D6, D5, D4
#else
#include "Wiring.h"
struct SerialConsole
{
	template<typename T> SerialConsole& operator << ( const T& x )
	{
		Serial.print( x );
		return *this;
	}
	void print(unsigned long x, int base=10, int ndigits=0) { print((long)x,base,ndigits); }
	void print(unsigned int x, int base=10, int ndigits=0) { print((long)x,base,ndigits); }
	void print(int x, int base=10, int ndigits=0) { print((long)x,base,ndigits); }
	void print(long x, int div=10, int ndigits=0)
	{
		if(div<2) return;
		char digits[16];
		int n = 0;
		if( x < 0 ) { Serial.print('-'); x=-x; }
		do
		{
			digits[n++] = x % div;
			x /= div;
		} while( x > 0 );
		for(int i=0;i<(ndigits-n);i++) Serial.print('0');
		for(int i=0;i<n;i++)
		{
			int dg = digits[n-i-1];
			Serial.print( (dg<10) ? ('0'+dg) : ('A'+(dg-10)) );
		}
	}
};
#endif

int main(void) __attribute__((noreturn));
int main(void)
{
	boardInit();

	auto rx = AvrPin<RECEIVE_PIN>();
	rx.SetInput();
	auto sniffer = make_sniffer(rx);

	RFSnifferProtocol sp;
	sp.fromEEPROM(EEPROM_BASE_ADDR);

#ifdef LCD_CONSOLE
	LCD<LCD_PINS> lcd;
	lcd.begin();
#else
	Serial.begin(9600);
	SerialConsole lcd;
#endif

	lcd << "Step 1: learn";

	auto led = AvrPin<LED_PIN>();
	led.SetOutput();
	
	while( ! sp.isValid() )
	{
		/*
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
		*/

		// detect and record a signal
		bool signalOk = false;
		{
			uint16_t buf[MAX_PULSES];
			//int npulses = sniffer.recordSignalLatchDetect<MAX_PULSES,MIN_LATCH_LEN,MIN_PROLOG_LATCHES>(buf);
			int npulses = sniffer.recordSignalBinaryEntropyDetect<MAX_PULSES,ENTROPY_DETECTION_PULSES>(buf);
			if( npulses >= MIN_MESSAGE_PULSES )
			{
				/*
				long pulseMax = 0;
				long pulseMin = MAX_PULSE_LEN;
				for(int i=0;i<npulses;i++)
				{
					if(buf[i]>pulseMax) pulseMax=buf[i];
					if(buf[i]<pulseMin) pulseMin=buf[i];
				}
				lcd<<'\n'<<npulses<<' '<<pulseMin<<' '<<pulseMax;
				*/
				signalOk = sniffer.analyseSignal(buf,npulses,sp);
			}
		}
				
 		// signal content analysis
		if( signalOk )
		{
			/*
			lcd<<'\n' << sp.rsv[0]<<' '<<sp.rsv[1];
			blink(led);
			*/
			lcd << '\n' << sp.nLatches;
			if(sp.nLatches>0) lcd <<'/'<< sp.latchSeqLen ;
			lcd <<' '<< (char)sp.coding << sp.messageBits << "x" << sp.nMessageRepeats << (sp.matchingRepeats?'+':'-') ;
			lcd << '\n' << sp.bitSymbols[0]<<' '<<sp.bitSymbols[1];
			blink(led);
			if(sp.latchSeqLen>0)
			{
				lcd <<'\n'; for(int i=0;i<sp.latchSeqLen;i++) lcd<<sp.latchSeq[i]<<' '; 
			}
			if(sp.nLatches>0)
			{
				lcd <<'\n'; for(int i=0;i<sp.nLatches;i++) lcd<<sp.latchSymbols[i]<<' ';
			}
			if( sp.latchSeqLen>0 || sp.nLatches>0 ) blink(led);
			
			lcd << "\nStep2: read";
			if( sp.messageBits > MAX_MESSAGE_BITS ) sp.messageBits = MAX_MESSAGE_BITS;
			int bitsToRead = sp.messageBits;
			if( sp.coding == CODING_MANCHESTER ) bitsToRead *= 2;
			int nbytes = (bitsToRead+7) / 8;
			uint8_t signal1[nbytes];
			int br;
			do { br = sniffer.readBinaryMessage(sp,signal1); } while( br==0 );
			if( br == bitsToRead )
			{
				uint16_t tries=0;
				uint8_t signal2[nbytes];
				do
				{
					// don't loose time printing "step 3" if repeated signal can be catched right after the first one"
					br = sniffer.readBinaryMessage(sp,signal2);
					++tries;
					if( tries == 1000 ) lcd << "\nStep3: verify";
				} while( br==0 );
				for(int i=0;i<nbytes;i++) if(signal1[i]!=signal2[i]) br=0;
				if( br == bitsToRead )
				{
					lcd << "\nSave protocol...";
					sp.toEEPROM(EEPROM_BASE_ADDR);
					blink(led);
				}
			}
		}
	}
	
/*
	auto tx = AvrPin<EMIT_PIN>();
	tx.SetOutput();
	auto sender = make_emitter(tx);
*/

	int rstSeqIdx = 0;
	uint8_t checksum = 0;

	lcd << "\nRF sniffer ready" ;
	for(;;)
	{
		int bitsToRead = sp.messageBits;
		if( sp.coding == CODING_MANCHESTER ) bitsToRead *= 2;
		
		int nbytes = (bitsToRead+7) / 8;
		uint8_t buf[nbytes+1];
		if( sniffer.readBinaryMessage(sp,buf) == bitsToRead )
		{
			bool mesgValid = true;
			if( sp.coding == CODING_MANCHESTER )
			{
				mesgValid = sniffer.decodeManchester(buf,bitsToRead);
				nbytes = ( (bitsToRead/2) + 7 ) / 8;
			}
			if( mesgValid )
			{
				uint8_t cs = 0;
				for(int i=0;i<nbytes;i++)
				{
					cs = ( (cs<<1) | (cs >>7) ) ^ buf[i];
				}
				bool rstcode = (cs == checksum);
				if( rstcode == (((RESET_SEQ>>rstSeqIdx)&1)!=0) ) ++rstSeqIdx;
				else rstSeqIdx=0;
				if( !rstcode ){ checksum=cs; }
				lcd << '\n';
				if( rstSeqIdx>=3 )
				{
					lcd << "Reset protocol ";
					if(rstSeqIdx==4)
					{
						lcd << '!';
						blink(led);
						sp.invalidateEEPROM(EEPROM_BASE_ADDR);
						asm volatile ("  jmp 0"); 
					}
					else { lcd << '?'; }
				}
				
				for(int i=0;i<nbytes;i++)
				{
					lcd.print((unsigned int)buf[i],16,2);
				}
				blink(led);
				/*lcd << '\n';
				for(int i=5;i>0;--i)
				{
					lcd << i << ' ';
					blink(led,10);
				}
				sender.sendSignal(sp.messageBits,buf);
				lcd << '*';*/
			}
			else
			{
				lcd << "\nBad message";
			}
		}
	}
}

