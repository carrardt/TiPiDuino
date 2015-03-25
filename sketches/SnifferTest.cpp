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
#define ENTROPY_DETECTION_PULSES 32
#define MAX_PULSES 			384

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
using Console = LCD<LCD_PINS>;
#else
#include "SerialConsole.h"
using Console = SerialConsole;
#endif

#define BLINK_LED blink(led)

static const char* stageLabel[3] = {"detect","analyse","verify"};

int main(void) __attribute__((noreturn));
int main(void)
{
	boardInit();

	auto rx = AvrPin<RECEIVE_PIN>();
	rx.SetInput();
	auto sniffer = make_sniffer(rx);

	Console lcd;
	lcd.begin();

	auto led = AvrPin<LED_PIN>();
	led.SetOutput();
	
	// try to read a previously analysed protocol from EEPROM
	RFSnifferProtocol sp;
	sp.fromEEPROM(EEPROM_BASE_ADDR);

	bool stageChanged=true;
	int stage=0;
	while( ! sp.isValid() )
	{
		if( stageChanged )
		{
			lcd<<(stage+1)<<") "<<stageLabel[stage]<<'\n';
			stageChanged=false;
		}
		// detect and record a signal
		if( stage == 0 )
		{
			uint16_t buf[MAX_PULSES];
			//int npulses = sniffer.recordSignalLatchDetect<MAX_PULSES,MIN_LATCH_LEN,MIN_PROLOG_LATCHES>(buf);
			int npulses = sniffer.recordSignalBinaryEntropyDetect<MAX_PULSES,ENTROPY_DETECTION_PULSES>(buf);
			bool signalOk = false;
			if( npulses >= MIN_MESSAGE_PULSES )
			{
				signalOk = sniffer.analyseSignal(buf,npulses,sp);
			}
			if(signalOk)
			{ 
				lcd << "0: "<<sp.bitSymbols[0]<<", 1: "<<sp.bitSymbols[1]<<'\n';
				lcd << "nl=" << sp.latchSeqLen << " :";
				for(int i=0;i<sp.latchSeqLen;i++) { if(i>0)lcd<<','; lcd<<sp.latchSeq[i]; }
				lcd << '\n';
				++stage;
				stageChanged=true;
				BLINK_LED;
			}
		}
		// make a new record with a latch detection for record content robustness
		else if( stage == 1 )
		{
			bool signalOk = false;
			if( sp.latchSeqLen > 0 )
			{
				uint16_t buf[MAX_PULSES];
				int npulses = sniffer.recordSignalLatchSequenceDetect<MAX_PULSES>(buf,sp.latchSeqLen,sp.latchSeq);
				if( npulses >= MIN_MESSAGE_PULSES )
				{
					signalOk = sniffer.analyseSignal(buf,npulses,sp);
				}
			}
			if(signalOk)
			{ 
				lcd <<"nl="<< sp.latchSeqLen<<" " ;
				for(int i=0;i<sp.latchSeqLen;i++) { lcd<<sp.latchSeq[i]<<' '; }
				lcd <<'\n';
				lcd <<' '<< (char)sp.coding << sp.messageBits << "x" << sp.nMessageRepeats << (sp.matchingRepeats?'+':'-') << '\n';
				lcd << sp.bitSymbols[0]<<' '<<sp.bitSymbols[1] << '\n';
				++stage;
				stageChanged=true;
			}
		}
 		// signal content analysis
		else if( stage == 2 )
		{			
			if( sp.messageBits > MAX_MESSAGE_BITS ) sp.messageBits = MAX_MESSAGE_BITS;
			int bitsToRead = sp.messageBits;
			if( sp.coding == CODING_MANCHESTER ) bitsToRead *= 2;
			int nbytes = (bitsToRead+7) / 8;
			uint8_t signal1[nbytes];
			int br;
			do { br = sniffer.readBinaryMessage(sp,signal1); } while( br==0 );
			if( br == bitsToRead )
			{
				uint32_t retries=0;
				uint8_t signal2[nbytes];
				do
				{
					br = sniffer.readBinaryMessage(sp,signal2);
					++retries;
				} while( br==0 );
				for(int i=0;i<nbytes;i++) if(signal1[i]!=signal2[i]) br=0;
				if( br == bitsToRead )
				{
					lcd << "Save protocol...\n";
					sp.toEEPROM(EEPROM_BASE_ADDR);
					BLINK_LED;
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

	lcd << "RF sniffer ready\n" ;
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
				if( rstSeqIdx>=3 )
				{
					lcd << "Reset protocol ";
					if(rstSeqIdx==4)
					{
						lcd << '!';
						BLINK_LED;
						sp.invalidateEEPROM(EEPROM_BASE_ADDR);
						asm volatile ("  jmp 0"); 
					}
					else { lcd << '?'; }
					lcd << '\n';
				}
				
				for(int i=0;i<nbytes;i++)
				{
					lcd.print((unsigned int)buf[i],16,2);
				}
				lcd << '\n';
				BLINK_LED;
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
				lcd << "Bad message\n";
			}
		}
	}
}

