#include "AvrTL.h"
#include "RFSniffer.h"
#include "RFSnifferProtocol.h"
#include "RFSnifferEEPROM.h"
#include "PrintStream.h"
#include "HWSerialIO.h"
#include "InputStream.h"

using namespace avrtl;

// what to use as a console to prin message
#define LCD_CONSOLE 1
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
#define SOFT_SERIAL_TX 12
#define SERIAL_SPEED 9600

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
int16_t processCommands(ByteStream* rawInput, int16_t* cmem=0);

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
				//cout<<RFSnifferEEPROM::getMessageCount()<<" messages\n";
				int16_t progId = RFSnifferEEPROM::getBootProgram();
				auto progStream = RFSnifferEEPROM::getMessageStream(progId);
				processCommands(&progStream);
				processCommands(&hwserial);
			}
			break;

		case RFSnifferEEPROM::LEARN_NEW_PROTOCOL :
			{
				RFSnifferProtocol sp;
				auto sniffer = make_sniffer(rx,sp,led,cout);
				sniffer.learnProtocol();
				int pId = RFSnifferEEPROM::saveProtocol(sp);
				cout<<'P'<<pId<<" saved\n";
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
	//cout << "record P#"<<pId<<"\n" ;
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
		if( sp.readMessage(rx,buf) == sp.messageBits )
		{
			nbytes = (sp.messageBits+7) / 8;
			int mesgId = RFSnifferEEPROM::saveMessage(pId,buf,nbytes);
			cout<<"M#"<<mesgId<<'\n';
			blink(led);
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


int16_t readCommandMemory(const int16_t* cmem, int regIndex)
{
	if(cmem==0) return 0;
	else return cmem[regIndex];
}

void writeCommandMemory(int16_t* cmem, int regIndex, int16_t value)
{
	if(cmem!=0)
	{
		cmem[regIndex] = value;
	}
}

int16_t readCommandIntgerValue(const int16_t* cmem, InputStream& cin)
{
	int32_t x = 0;
	cin >> x;
	if( x >= 32768 ) { return x; }
	if( x < 0 ) x = readCommandMemory(cmem,(-x)-1);
	return x;
}

int16_t processCommands(ByteStream* rawInput, int16_t* cmem)
{
	while( !rawInput->eof() )
	{
		InputStream cin;
		cin.begin( rawInput );
		char c = cin.readFirstNonSpace();
		switch( c )
		{
			// exit
			case 'x':
				return readCommandMemory(cmem,0);

			// reboot in another operation mode
			case 'o':
				rebootToOperationMode( readCommandIntgerValue(cmem,cin) );
				break;

			// set boot program id
			case 'b':
				RFSnifferEEPROM::setBootProgram( readCommandIntgerValue(cmem,cin) );
				break;

			// query message content
			case 'q':
				{
					int16_t mesgId = readCommandIntgerValue(cmem,cin);
					RFSnifferEEPROM::MessageInfo mesg = RFSnifferEEPROM::getMessageInfo( mesgId );
					cout << 'P' << mesg.protocolId<<'L'<<(int)mesg.nbytes<<'\n';
					RFSnifferEEPROM::EEPROMStream progStream(mesg);
					if( mesg.protocolId < RFSnifferEEPROM::EEPROM_MAX_PROTOCOLS )
					{
						cout.printStreamHex(&progStream);
					}
					else
					{
						cout.stream->copy(&progStream);
					}
					cout<<'\n';
				}
				break;
			
			// query protocol content
			case 'Q':
				{
					int16_t pId = readCommandIntgerValue(cmem,cin);
					auto proto = RFSnifferEEPROM::readProtocol( pId );
					proto.toStream(cout);
				}
				break;

			// send (emit) a message
			case 's':
				{
					int16_t mesgId = readCommandIntgerValue(cmem,cin);
					RFSnifferEEPROM::MessageInfo mesg = RFSnifferEEPROM::getMessageInfo( mesgId );
					auto proto = RFSnifferEEPROM::readProtocol( mesg.protocolId );
					uint8_t buf[mesg.nbytes];
					eeprom_read_block(buf,mesg.eeprom_addr,mesg.nbytes);
					tx.SelectPin( proto.mediumRF() );
					proto.writeMessage(buf,mesg.nbytes,tx);
				}
				break;

			// wite/move command memory value
			case 'm':
				{
					int16_t regIndex = readCommandIntgerValue(cmem,cin);
					int16_t value = readCommandIntgerValue(cmem,cin);
					writeCommandMemory(cmem,regIndex,value);
				}
				break;

			// increment a memory operand
			case '+':
				{
					int16_t regIndex = readCommandIntgerValue(cmem,cin);
					int16_t value = readCommandIntgerValue(cmem,cin);
					writeCommandMemory(cmem,regIndex,readCommandMemory(cmem,regIndex)+value);
				}
				break;

			// receive a message and write its index (if found) to command memory #0
			case 'r':
				{
					int16_t mId;
					{
						uint8_t buf[MAX_MESSAGE_BYTES];
						int protoId = readCommandIntgerValue(cmem,cin);
						int nbytes = 0;
						{
							auto proto = RFSnifferEEPROM::readProtocol(protoId);
							tx.SelectPin( proto.mediumRF() );
							while( proto.readMessage(rx,buf) != proto.messageBits ) ;
							nbytes = (proto.messageBits+7)/8;
						}
						mId = RFSnifferEEPROM::findRecordedMessage(protoId,buf,nbytes);
					}
					if( mId != -1 )
					{
						writeCommandMemory(cmem,0,mId);
					}
				}
				break;

			// alloc command memory, aka 'call'. previous command memory is pushed and then popped on exit
			case 'a':
				{
					int16_t allocSize = readCommandIntgerValue(cmem,cin);
					int16_t buf[allocSize];
					buf[0] = readCommandMemory(cmem,0);
					int16_t r = processCommands(rawInput,buf);
					writeCommandMemory(cmem,0,r);
				}
				break;

			// add a program/message to eeprom
			// for programs, protocol id must be >= 0x80.
			// if protocol id is 0xFF, program will be executed at init when operating in program mode
			case 'p':
				{
					int16_t bufSize = readCommandIntgerValue(cmem,cin); // number of bytes to read
					uint8_t buf[bufSize];
					BufferStream progStream(buf,bufSize);
					progStream.copy( rawInput );
					progStream.rewind();
					int mId = RFSnifferEEPROM::appendMessage(RFSnifferEEPROM::PROGRAM_PROTOCOL_ID,&progStream);
					/*cout<<"M"<<mId<<'\n';
					progStream.rewind();
					cout.stream->copy( &progStream );
					cout<<'\n';*/
					writeCommandMemory( cmem, 0, mId );
				}
				break;

			// execute a program in eeprom
			case 'e':
				{
					int16_t progId = readCommandIntgerValue(cmem,cin);
					auto progStream = RFSnifferEEPROM::getMessageStream(progId);
					processCommands(&progStream,cmem);
				}
				break;

			// dump command memory
			case 'd':
				{
					int16_t regIndex = readCommandIntgerValue(cmem,cin);
					cout <<regIndex<<":"<< readCommandMemory(cmem,regIndex) << '\n';
				}
				break;

			// wait delay in milliseconds
			case 'w':
				{
					int32_t duration = readCommandIntgerValue(cmem,cin);
					if(duration>0)
					{
						duration*=1000;
						avrtl::DelayMicroseconds(duration);
					}
				}
				break;

			// loop
			case 'l':
				{
					int bufSize = readCommandIntgerValue(cmem,cin);
					uint8_t buf[bufSize];
					BufferStream bufInput(buf,bufSize);
					bufInput.copy( rawInput );
					while( readCommandMemory(cmem,0) != 0 )
					{
						bufInput.rewind();
						processCommands(&bufInput,cmem);
					}
				}
				break;

			// comment
			default:
				while( !rawInput->eof() && rawInput->readByte()!='\n' );
				break;
		}
	}
	return readCommandMemory(cmem,0);
}

