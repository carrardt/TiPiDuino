#include "AvrTL.h"
#include "RFSniffer.h"
#include "RFSnifferProtocol.h"
#include "RFSnifferEEPROM.h"
#include "PrintStream.h"
#include "HWSerialIO.h"
#include "InputStream.h"

using namespace avrtl;

// what to use as a console to prin message
//#define LCD_CONSOLE 1
//#define SOFT_SERIAL_CONSOLE 1

// Sequence for learning a new protocol
#define RECORD_MODE_SEQ 0x05  		// press AABBA, A and B being any two different remote buttons

// sequence to go to command line mode
#define COMMAND_MODE_SEQ 0x07 		// press AAAAA, A and B being any two different remote buttons

// pinout
#define RF_RECEIVE_PIN 9
#define IR_RECEIVE_PIN 8
#define RF_EMIT_PIN 11
#define IR_EMIT_PIN 10
#define LED_PIN 13
#define SOFT_SERIAL_TX 12
#define SERIAL_SPEED 19200

#ifdef LCD_CONSOLE
#include "LCD.h"
#define LCD_PINS 7,6,5,4,3,2 // respectively RS, EN, D7, D6, D5, D4
LCD<LCD_PINS> lcd;
#elif defined(SOFT_SERIAL_CONSOLE)
#include "SoftSerialIO.h"
static auto serial_rx = AvrPin<NullPin>();
static auto serial_tx = pin(SOFT_SERIAL_TX);
static auto serialIO = make_softserial<SERIAL_SPEED>(serial_rx,serial_tx);
#endif

HWSerialIO hwserial; // pins 0,1
static auto rx = AvrPin< DualPin<IR_RECEIVE_PIN,RF_RECEIVE_PIN> >();
static auto tx = AvrPin< DualPin<IR_EMIT_PIN,RF_EMIT_PIN> >();
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
	tx.SetOutput();
	led.SetOutput();

	hwserial.begin(SERIAL_SPEED);

	// setup output to serial line or LCD display
#ifdef LCD_CONSOLE
	lcd.begin();
	cout.begin(&lcd);
#elif defined(SOFT_SERIAL_CONSOLE)
	serialIO.begin();
	cout.begin(&serialIO);
#else
	cout.begin(&hwserial);
#endif

	// try to read a previously analysed protocol from EEPROM
	RFSnifferEEPROM::initEEPROM();

	// permet de selectionner la bonne entree
	{
		RFSnifferProtocol sp;
		rx.SelectPin( sp.mediumRF() );
	}

	cout<<"* Mega Sniffer *\n";
}

void recordMessage(int pId);
void processInputCommand();


void loop(void)
{
	int op_mode = RFSnifferEEPROM::getOperationMode();
	switch( op_mode )
	{			
		case RFSnifferEEPROM::RECORD_PROTOCOL_0 :
		case RFSnifferEEPROM::RECORD_PROTOCOL_1 :
		case RFSnifferEEPROM::RECORD_PROTOCOL_2 :
		case RFSnifferEEPROM::RECORD_PROTOCOL_3 :
			recordMessage( op_mode - RFSnifferEEPROM::RECORD_PROTOCOL_0 );
			break;

		case RFSnifferEEPROM::COMMAND_MODE :
			processInputCommand(hwserial);
			break;

		case RFSnifferEEPROM::LEARN_NEW_PROTOCOL :
			{
				RFSnifferProtocol sp;
				auto sniffer = make_sniffer(rx,sp,led,cout);
				sniffer.learnProtocol();
				int pId = RFSnifferEEPROM::saveProtocol(sp);
				cout<<"P#"<<pId<<" saved\n";
				blink(led,50);
				RFSnifferEEPROM::setOperationMode( RFSnifferEEPROM::RECORD_PROTOCOL_0+pId );
			}
			break;

		default:
			break;
	}
}

void rebootToOperationMode(int op)
{
	RFSnifferEEPROM::setOperationMode(op);
	blink(led);
	asm volatile ("  jmp 0");
}

void recordMessage(int pId)
{
	cout << "record P#"<<pId<<"\n" ;
	uint8_t recordModeSeqIdx = 0;
	uint8_t commandModeSeqIdx = 0;
	uint8_t last_checksum = 0;
	
	auto sp = RFSnifferEEPROM::readProtocol(pId);
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
				int mesgId = RFSnifferEEPROM::saveMessage(pId,buf,nbytes);
				cout<<"M#"<<mesgId<<'\n';
				uint8_t cs = checksum8(buf,nbytes);
				bool same_mesg = (cs == last_checksum);
				last_checksum = cs;
				if( testSequence(cout,RECORD_MODE_SEQ,same_mesg,recordModeSeqIdx,"New protocol") )
				{
					rebootToOperationMode(RFSnifferEEPROM::LEARN_NEW_PROTOCOL);
				}
				if( testSequence(cout,COMMAND_MODE_SEQ,same_mesg,commandModeSeqIdx,"Program mode") )
				{
					rebootToOperationMode(RFSnifferEEPROM::COMMAND_MODE);
				}
			}
		}
	}
}

void processInputCommand(ByteStream* rawInput)
{
	InputStream cin;
	cin.begin( rawInput );
	char c = cin.readFirstNonSpace();
	switch( c )
	{
		case 'p':
			{
				int mesgId = 0;
				cin >> mesgId;
				RFSnifferEEPROM::MessageInfo mesg = RFSnifferEEPROM::getMessageInfo( mesgId );
				cout << "P#" << mesg.protocolId<<':';
				printEEPROM(cout,mesg.eeprom_addr,mesg.nbytes);
				cout << '\n';
				auto proto = RFSnifferEEPROM::readProtocol( mesg.protocolId );
				uint8_t buf[MAX_MESSAGE_BYTES];
				eeprom_read_block(buf,mesg.eeprom_addr,mesg.nbytes);
				tx.SelectPin( proto.mediumRF() );
				proto.writeMessage(buf,mesg.nbytes,tx);
			}
			break;

		case 'o':
			{
				int op_mode = RFSnifferEEPROM::COMMAND_MODE;
				cin >> op_mode;
				rebootToOperationMode( op_mode );
			}
			break;

		case 'r':
			{
				int bufSize=0;
				cin >> bufSize;
				char program[bufSize];
				for(int i=0;i<bufSize;i++) cin >> program[i];
				BufferInputStream progStream(program,bufSize);
				while( ! progStream.eof() )
				{
					processInputCommand( progStream );
				}
			}
			break;

		default:
			break;
	}
}

