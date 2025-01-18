#include "AvrTL.h"
#include "AvrTLPin.h"
#include "RFSniffer.h"
#include "RFSnifferProtocol.h"
#include "RFSnifferEEPROM.h"
#include "RFSnifferInterpreter.h"
#include "BasicIO/PrintStream.h"
#include "HWSerial/HWSerialIO.h"
#include "BasicIO/InputStream.h"

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
#define RF_EMIT_PIN 10
#define IR_EMIT_PIN 11
#define LED_PIN 13
//#define SOFT_SERIAL_TX 12
#define SERIAL_SPEED 9600

#ifdef LCD_CONSOLE
#include "LCD.h"
#define LCD_PINS 7,6,5,4,3,2 // respectively RS, EN, D7, D6, D5, D4
LCD<LCD_PINS> lcd;
#elif defined(SOFT_SERIAL_CONSOLE)
#include "SoftSerialIO.h"
static auto serial_rx = NullPin();
static auto serial_tx = StaticPin<SOFT_SERIAL_TX>();
static auto serialIO = make_softserial<SERIAL_SPEED>(serial_rx,serial_tx);
#endif

HWSerialIO hwserial; // pins 0,1
static auto rx = DualPin<IR_RECEIVE_PIN,RF_RECEIVE_PIN>();
static auto rf_tx = StaticPin<RF_EMIT_PIN>();
static auto ir_tx = StaticPin<IR_EMIT_PIN>();
static auto led = StaticPin<LED_PIN>();
static PrintStream cout;
TeeStream tstream;

void setup()
{
	// setup pin mode
	rx.SetInput();
	rf_tx.SetOutput(); rf_tx=0;
	ir_tx.SetOutput(); ir_tx=0;
	led.SetOutput(); led=0;

	hwserial.begin(SERIAL_SPEED);

	// setup output to serial line or LCD display
#ifdef LCD_CONSOLE
	lcd.begin();
	tstream.begin(&hwserial,&lcd);
	cout.begin(&tstream);
#elif defined(SOFT_SERIAL_CONSOLE)
	serialIO.begin();
	cout.begin(&serialIO);
#else
	cout.begin(&hwserial);
#endif

	cout<<"* Mega Sniffer *"<<endl;

	// try to read a previously analysed protocol from EEPROM
	RFSnifferEEPROM::initEEPROM();
	cout<<"EEPROM OK"<<endl;

	// permet de selectionner la bonne entree
	{
		RFSnifferProtocol sp;
		rx.SelectPin( sp.mediumRF() );
	}

	cout<<"Mesg:"<<RFSnifferEEPROM::getMessageCount()<<" Prot:"<<RFSnifferEEPROM::getProtocolCount()<<endl;
}

void recordMessage(int pId);
void rebootToOperationMode(int op);

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
			{
				int16_t progId = RFSnifferEEPROM::getBootProgram();
				auto interpreter = make_rfsniffer_interpreter(rx,rf_tx,ir_tx,cout);
				if(progId!=0xFF)
				{
					cout<<"boot "<<progId<<endl;
					auto progStream = RFSnifferEEPROM::getMessageStream(progId);
					interpreter.processCommands(&progStream);
				}
				cout<<"ready"<<endl;
				interpreter.processCommands(&hwserial);
			}
			break;

		case RFSnifferEEPROM::LEARN_NEW_PROTOCOL :
			{
				RFSnifferProtocol sp;
				auto sniffer = make_sniffer(rx,sp,led,cout);
				sniffer.learnProtocol();
				int pId = RFSnifferEEPROM::saveProtocol(sp);
				sp.toStream(cout);
				rebootToOperationMode( RFSnifferEEPROM::RECORD_PROTOCOL_0+pId );
			}
			break;

		default:
			break;
	}
}

void rebootToOperationMode(int op)
{
	SignalProcessing32 sproc;
	RFSnifferEEPROM::setOperationMode(op);
	sproc.blink(led);
	asm volatile ("  jmp 0");
}

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
		cout << endl;
	}
	return seqmatch;
}

void recordMessage(int pId)
{
	SignalProcessing32 sproc;

	// in case the remote doesn't work, reset will get you back to command mode
	RFSnifferEEPROM::setOperationMode( RFSnifferEEPROM::COMMAND_MODE );

	uint8_t recordModeSeqIdx = 0;
	uint8_t commandModeSeqIdx = 0;
	uint8_t last_checksum = 0;

	auto sp = RFSnifferEEPROM::readProtocol(pId);
	sp.toStream(cout);
	rx.SetInput();
	rx.SelectPin( sp.mediumRF() );
	auto sniffer = make_sniffer(rx,sp,led,cout);
	for(;;)
	{
		int bitsToRead = sp.messageBits;
		if( sp.coding == CODING_MANCHESTER ) bitsToRead *= 2;
		int nbytes = (bitsToRead+7) / 8;
		uint8_t buf[nbytes+1];
		if( sp.readMessage(rx,buf) == sp.messageBits )
		{
			nbytes = (sp.messageBits+7) / 8;
			int mesgId = RFSnifferEEPROM::saveMessage(pId,buf,nbytes);
			cout<<"M#"<<mesgId<<endl;
			sproc.blink(led);
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
