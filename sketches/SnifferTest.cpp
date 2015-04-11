#include "AvrTL.h"
#include "RFSniffer.h"
#include "RFSnifferProtocol.h"
#include "RFSnifferEEPROM.h"
#include "PrintStream.h"
#include "HWSerialIO.h"

using namespace avrtl;

// what to use as a console to prin message
//#define LCD_CONSOLE 1
//#define SOFT_SERIAL_CONSOLE 1

#define RESET_SEQ 0x05  		// press AABBA, A and B being any two different remote buttons
//#define REPLAY_SEQ 0x07 		// press ABABB, A and B being any two different remote buttons

// pinout
#define RF_RECEIVE_PIN 9
#define IR_RECEIVE_PIN 8
#define RF_EMIT_PIN 7
#define IR_EMIT_PIN 6
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

HWSerialIO hwserial;
static auto rx = AvrPin< DualPin<IR_RECEIVE_PIN,RF_RECEIVE_PIN> >();
static auto led = pin(LED_PIN);
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

void setup()
{
	// setup pin mode
	rx.SetInput();
	led.SetOutput();

	// hwserial.begin(SERIAL_SPEED);
	// dbgout.begin(&hwserial);

	// setup output to serial line or LCD display
#ifdef LCD_CONSOLE
	lcd.begin();
	cout.begin(&lcd);
#else
	serialIO.begin();
	cout.begin(&serialIO);
#endif

	// try to read a previously analysed protocol from EEPROM
	RFSnifferEEPROM::initEEPROM();
}

void recordMessage(int pId)
{
	cout << "record mode P#"<<pId<<"\n" ;
	uint8_t rstSeqIdx = 0;
	uint8_t last_checksum = 0;
	RFSnifferProtocol sp;
	RFSnifferEEPROM::readProtocol(pId,sp);
	sp.toStream(cout);
	auto sniffer = make_sniffer(rx,sp,led,cout);
	for(;;)
	{
		int bitsToRead = sp.messageBits;
		if( sp.coding == CODING_MANCHESTER ) bitsToRead *= 2;

		int nbytes = (bitsToRead+7) / 8;
		uint8_t buf[nbytes+1];
		if( sniffer.readBinaryMessage(buf) == bitsToRead )
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
				if( testSequence(cout,RESET_SEQ,same_mesg,rstSeqIdx,"New protocol") )
				{
					blink(led);
					RFSnifferEEPROM::setOperationMode(RFSnifferEEPROM::LEARN_NEW_PROTOCOL);
					asm volatile ("  jmp 0"); 
				}

				int mesgId = RFSnifferEEPROM::saveMessage(pId,buf,nbytes);
				cout<<"M#"<<mesgId<<'\n';
				blink(led);
			}
		}
	}
}

void loop(void)
{
	int op_mode = RFSnifferEEPROM::getOperationMode();
	// dbgout << "Operation mode = "<<op_mode<<"\n";
	switch( op_mode )
	{
		case RFSnifferEEPROM::LEARN_NEW_PROTOCOL :
			{
				RFSnifferProtocol sp;
				rx.SelectPin( sp.mediumRF() ); // sp influenced by RFSnifferProtocol::defaultFlags, handled through EEPROM
				auto sniffer = make_sniffer(rx,sp,led,cout);
				sniffer.learnProtocol();
				int pId = RFSnifferEEPROM::saveProtocol(sp);
				cout<<"P#"<<pId<<" saved\n";
				blink(led,50);
				RFSnifferEEPROM::setOperationMode( RFSnifferEEPROM::RECORD_PROTOCOL_0 );
			}
			break;
			
		case RFSnifferEEPROM::RECORD_PROTOCOL_0 :
		case RFSnifferEEPROM::RECORD_PROTOCOL_1 :
		case RFSnifferEEPROM::RECORD_PROTOCOL_2 :
		case RFSnifferEEPROM::RECORD_PROTOCOL_3 :
			{
				int pId = op_mode - RFSnifferEEPROM::RECORD_PROTOCOL_0;
				recordMessage( pId );
			}
			break;
			
		default:
			break;
	}
	
	/*
	if( eeprom_read_byte(EEPROM_CODES_ADDR) != 0 )
	{
		cout << "Replay mode\n" ;
		HWSerialIO hwserial;
		hwserial.begin(9600);
		InputStream cin;
		cin.begin( & hwserial );
		for(;;)
		{
			int mesgId=0, nbytes=0;
			cin >> mesgId;
			uint8_t* ptr = getMessageAddr( mesgId, nbytes );
			for(int i=0;i<nbytes;++i)
			{
				cout.print( (unsigned int) eeprom_read_byte(ptr+i), 16, 2 );
			}
			cout<<'\n';
		}
	}
	*/
}


