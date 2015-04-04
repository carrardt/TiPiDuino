#include "AvrTL.h"
#include "RFSniffer.h"
#include "PrintStream.h"

using namespace avrtl;

// what to use as a console to prin message
//#define LCD_CONSOLE 1

// for latch detection based analysis
#define MIN_MESSAGE_PULSES 		 32
#define ENTROPY_DETECTION_PULSES 32
#define MAX_PULSES 				384

// EEPROM address where to write detected protocol
#define EEPROM_PULSELVL_ADDR 	((uint8_t*)0x0001)
#define EEPROM_PROTOCOL_ADDR 	((uint8_t*)0x0004)
#define EEPROM_CODES_ADDR 		(EEPROM_PROTOCOL_ADDR+sizeof(RFSnifferProtocol))
#define EEPROM_CODES_BYTES 		512
#define RESET_SEQ 0x05  		// press AABBA, A and B being any two different remote buttons
#define REPLAY_SEQ 0x08 		// press ABABB, A and B being any two different remote buttons

// pinout
#define RECEIVE_PIN 9
#define LED_PIN 13
#define SERIAL_RX 10
#define SERIAL_TX 11
#define SERIAL_SPEED 19200

#ifdef LCD_CONSOLE
#include "LCD.h"
#define LCD_PINS 7,6,5,4,3,2 // respectively RS, EN, D7, D6, D5, D4
LCD<LCD_PINS> lcd;
#else
#include "SoftSerialIO.h"
static auto serial_rx = pin(SERIAL_RX);
static auto serial_tx = pin(SERIAL_TX);
static auto serialIO = make_softserial<SERIAL_SPEED>(serial_rx,serial_tx);
#endif

static const char* stageLabel[3] = {"detect","analyse","verify"};
static auto rx = pin(RECEIVE_PIN);
static auto led = pin(LED_PIN);
static auto sniffer = make_sniffer(rx);
static RFSnifferProtocol sp;
static uint8_t* eeprom_ptr = (uint8_t*)EEPROM_CODES_ADDR;
static PrintStream cout;

template<typename OutStream>
static bool testSequence(OutStream& out, uint8_t seq, bool value, uint8_t& seqIdx, const char* mesg)
{
	bool seqmatch=false;
	if( value == (((seq>>seqIdx)&1)!=0) ) ++seqIdx;
	else seqIdx=0;
	if( seqIdx>=3 )
	{
		cout << mesg;
		if(seqIdx==4)
		{
			cout << '!';
			seqmatch=true;
		}
		else { cout << '?'; }
		cout << '\n';
	}
	return seqmatch;
}

template<typename OutputStream>
static void retry_mesg(OutputStream& out)
{
	out<<"press button\n";
}

static int findRecordedMessage(const uint8_t* buf, int nbytes)
{
	uint8_t* ptr = EEPROM_CODES_ADDR;
	int offset = 0;
	int mesgFoundAt = -1;
	int mesgIdx = 0;
	while( (ptr-EEPROM_CODES_ADDR)<EEPROM_CODES_BYTES && (mesgFoundAt==-1) && eeprom_read_byte(ptr+offset)!=0 )
	{
		int len = eeprom_read_byte(ptr);
		++ptr;
		if( len > MAX_MESSAGE_BYTES ) return -1;
		bool match = true;
		int i=0;
		for(;i<len;i++)
		{
			if( i < nbytes )
			{
				match = match && ( eeprom_read_byte(ptr) == buf[i] );
			}
			else 
			{
				match = false;
			}
			++ptr;
		}
		if( match ) { mesgFoundAt=mesgIdx; }
		++ mesgIdx;
	}
	return mesgFoundAt;
}

void setup()
{
	// setup pin mode
	rx.SetInput();
	led.SetOutput();

	// setup output to serial line or LCD display
#ifdef LCD_CONSOLE
	lcd.begin();
	cout.begin(&lcd);
#else
	serialIO.begin();
	cout.begin(&serialIO);
#endif

	// try to read a previously analysed protocol from EEPROM
	sp.fromEEPROM(EEPROM_PROTOCOL_ADDR);
	if( sp.isValid() )
	{
		int n=0;
		while( eeprom_read_byte(eeprom_ptr) != 0 )
		{
			int len = eeprom_read_byte(eeprom_ptr++);
			cout.print(n++,10,3);
			cout<<" : ";
			for(int i=0;i<len;i++)
			{
				cout.print( (int) eeprom_read_byte(eeprom_ptr++) , 16 , 2 );
			}
			cout<<'\n';
		}
	}
	else
	{
		eeprom_gently_write_byte(EEPROM_CODES_ADDR,0);
	}
}

void loop(void)
{
	bool stageChanged=true;
	int stage=0;

	while( ! sp.isValid() )
	{
		if( stageChanged )
		{
			cout<<(stage+1)<<") "<<stageLabel[stage]<<' '<<(sp.pulseLevel?'H':'L')<<'\n';
			stageChanged=false;
		}
		// detect and record a signal
		if( stage == 0 )
		{
			uint16_t buf[MAX_PULSES];
			uint16_t P1Idx=0;
			int npulses = sniffer.recordSignalBinaryEntropyDetect<MAX_PULSES,ENTROPY_DETECTION_PULSES>(buf,sp.pulseLevel,P1Idx);
			bool signalOk = false;
			if( npulses >= MIN_MESSAGE_PULSES )
			{
					long P0=buf[0];
					long P1=buf[P1Idx];
					signalOk = sniffer.analyseSignal(buf,npulses,sp);
					if( signalOk )
					{
						/*
						cout<<"entropy detected bits "<<P0<<", "<<P1<<'\n';
						cout<<"analysed bits "<<sp.bitSymbols[0]<<", "<<sp.bitSymbols[1]<<'\n';
						*/
						long re0 = sp.bitSymbols[0] / PULSE_ERR_RATIO;
						long re1 = sp.bitSymbols[1] / PULSE_ERR_RATIO;
						int P0Bit=2, P1Bit=2;
						
						if( sniffer.identicalPulses(P0,sp.bitSymbols[0]) ) P0Bit=0;
						else if( sniffer.identicalPulses(P0,sp.bitSymbols[1]) ) P0Bit=1;
						
						if( sniffer.identicalPulses(P1,sp.bitSymbols[0]) ) P1Bit=0;
						else if( sniffer.identicalPulses(P1,sp.bitSymbols[1]) ) P1Bit=1;

						// cout<<"=> "<<P0Bit<<", "<<P1Bit<<'\n';
						if( P0Bit==2 || P1Bit==2 || P0Bit==P1Bit )
						{
							signalOk = false;
						}
					}
					if( ! signalOk )
					{
						sp.pulseLevel = !sp.pulseLevel;
						stageChanged=true;
					}
			}
			if(signalOk)
			{
				/*
				cout << "0: "<<sp.bitSymbols[0]<<", 1: "<<sp.bitSymbols[1]<<'\n';
				cout << "nl=" << sp.latchSeqLen << " :";
				for(int i=0;i<sp.latchSeqLen;i++) { if(i>0)cout<<','; cout<<sp.latchSeq[i]; }
				cout << '\n';
				cout<< (char)sp.coding << ',' << sp.messageBits << ',' << npulses<<'\n';
				*/
				++stage;
				if( sp.latchSeqLen == 0 ) ++stage;
				stageChanged=true;
				sp.toStream(cout);
				blink(led);
			}
		}
		// make a new record with a latch detection for record content robustness
		else if( stage == 1 )
		{
			bool signalOk = false;
			if( sp.latchSeqLen > 0 )
			{
				uint16_t buf[MAX_PULSES];
				int npulses = sniffer.recordSignalLatchSequenceDetect<MAX_PULSES>(buf,sp.pulseLevel,sp.latchSeqLen,sp.latchSeq);
				if( npulses >= MIN_MESSAGE_PULSES )
				{
					signalOk = sniffer.analyseSignal(buf,npulses,sp);
				}
			}
			if(signalOk)
			{ 
				/*
				cout <<"nl="<< sp.latchSeqLen<<" " ;
				for(int i=0;i<sp.latchSeqLen;i++) { cout<<sp.latchSeq[i]<<' '; }
				cout <<'\n';
				cout <<' '<< (char)sp.coding << sp.messageBits << "x" << sp.nMessageRepeats << (sp.matchingRepeats?'+':'-') << '\n';
				cout << sp.bitSymbols[0]<<' '<<sp.bitSymbols[1] << '\n';
				*/
				++stage;
				stageChanged=true;
				sp.toStream(cout);
				blink(led);
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
				cout<<"Press same button\n";
				do
				{
					br = sniffer.readBinaryMessage(sp,signal2);
					++retries;
				} while( br==0 );
				for(int i=0;i<nbytes;i++) if(signal1[i]!=signal2[i]) br=0;
				if( br == bitsToRead )
				{
					cout << "Save protocol...\n";
					sp.toEEPROM(EEPROM_PROTOCOL_ADDR);
					blink(led);
				}
			}
			else
			{
				cout << "Err: "<<br<<'/'<<bitsToRead<<'\n';
			}
		}
	}
	
/*
	auto tx = AvrPin<EMIT_PIN>();
	tx.SetOutput();
	auto sender = make_emitter(tx);
*/

	uint8_t rstSeqIdx = 0;
	uint8_t replaySeqIdx = 0;
	uint8_t last_checksum = 0;

	cout << "Ready\n" ;
	sp.toStream(cout);
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
				uint8_t cs = checksum8(buf,nbytes);
				bool same_mesg = (cs == last_checksum);
				last_checksum = cs;
				if( testSequence(cout,RESET_SEQ,same_mesg,rstSeqIdx,"Reset") )
				{
					blink(led);
					sp.invalidateEEPROM(EEPROM_PROTOCOL_ADDR);
					asm volatile ("  jmp 0"); 
				}
				if( testSequence(cout,REPLAY_SEQ,same_mesg,replaySeqIdx,"Replay") )
				{
					blink(led);
				}

				int mesgFoundAt = findRecordedMessage(buf,nbytes);
				if( mesgFoundAt == -1 )
				{
					if( (eeprom_ptr-EEPROM_CODES_ADDR) >= EEPROM_CODES_BYTES )
					{
						cout<<"#Full\n";
					}
					else
					{
						eeprom_gently_write_byte( eeprom_ptr++, nbytes );
						for(int i=0;i<nbytes;i++)
						{
							eeprom_gently_write_byte( eeprom_ptr++, buf[i] );
							cout.print((unsigned int)buf[i],16,2);
						}
						eeprom_gently_write_byte( eeprom_ptr, 0 );
						cout << '\n';
					}
				}
				else
				{
					cout<<"#"<<mesgFoundAt<<'\n';
				}
				blink(led);
			}
			else
			{
				cout << "#Bad\n";
			}
		}
	}
}

