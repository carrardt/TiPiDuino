#ifndef __RFSNIFFER_CommandLine_h
#define __RFSNIFFER_CommandLine_h

#include "ByteStream.h"
#include "InputStream.h"
#include "PrintStream.h"
#include "RFSnifferEEPROM.h"
#include "RFSnifferProtocol.h"
#include "AvrTL.h"
#include <stdint.h>


template<typename TxPinT, typename RxPinT>
struct RFSnifferInterpreter
{
	TxPinT& tx;
	RxPinT& rx;
	PrintStream& cout;

	inline RFSnifferInterpreter(TxPinT& _tx, RxPinT& _rx, PrintStream& _out) : rx(_rx), tx(_tx), cout(_out) {}

	static int16_t readCommandMemory(const int16_t* cmem, int regIndex)
	{
		if(cmem==0) return 0;
		else return cmem[regIndex];
	}

	static void writeCommandMemory(int16_t* cmem, int regIndex, int16_t value)
	{
		if(cmem!=0)
		{
			cmem[regIndex] = value;
		}
	}

	static int16_t readCommandIntgerValue(const int16_t* cmem, InputStream& cin)
	{
		int32_t x = 0;
		cin >> x;
		if( x >= 32768 ) { return x; }
		if( x < 0 ) x = readCommandMemory(cmem,(-x)-1);
		return x;
	}

	int16_t processCommands(ByteStream* rawInput, int16_t* cmem=0)
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
					{
						int op = readCommandIntgerValue(cmem,cin);
						RFSnifferEEPROM::setOperationMode(op);
						asm volatile ("  jmp 0");
					}
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
						cout << 'P' << mesg.protocolId<<'L'<<(int)mesg.nbytes<<endl;
						RFSnifferEEPROM::EEPROMStream progStream(mesg);
						if( mesg.protocolId < RFSnifferEEPROM::EEPROM_MAX_PROTOCOLS )
						{
							cout.printStreamHex(&progStream);
						}
						else
						{
							cout.stream->copy(&progStream);
						}
						cout<<endl;
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

				// read and print a raw binary message
				case 'R':
					{
						int protoId = readCommandIntgerValue(cmem,cin);
						int nbytes = 0;
						auto proto = RFSnifferEEPROM::readProtocol(protoId);
						int bitsToRead = proto.messageBits;
						int nPulses = proto.messageTotalPulses();
						uint16_t gaps[nPulses];
						{
							nbytes = proto.messageReadBufferSize();
							uint8_t buf[nbytes];
							rx.SelectPin( proto.mediumRF() );
							int br=0;
							while( (br=proto.readMessageWithGaps(rx,buf,gaps)) == 0 ) ;
							if(proto.nMessageRepeats>1) { br=proto.readMessageWithGaps(rx,buf,gaps); }
							BufferStream bufStream(buf,proto.messageBytes());
							cout.printStreamHex(&bufStream);
							cout<<endl;
						}
						uint16_t minPulse=MAX_PULSE_LEN, maxPulse=0;
						for(int i=0;i<nPulses;i++)
						{
							if( gaps[i] < minPulse) minPulse=gaps[i];
							else if( gaps[i] > maxPulse) maxPulse=gaps[i];
						}
						cout<<minPulse<<' '<<maxPulse<<endl;
					}
					break;

				// record a raw binary signal
				case 'S':
					{
						uint32_t timeout = readCommandIntgerValue(cmem,cin);
						uint16_t nSamples = readCommandIntgerValue(cmem,cin);
						bool RFMode = readCommandIntgerValue(cmem,cin);
						uint16_t buf[nSamples];
						rx.SelectPin( RFMode );
						nSamples = RecordSignal(rx,timeout*1000UL,nSamples,buf);
						for(int i=0;i<nSamples;i++)
						{
							cout<<i<<':'<<avrtl::ticksToMicroseconds(buf[i])<<endl;
						}
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
							int protoId = readCommandIntgerValue(cmem,cin);
							int nbytes = 0;
							auto proto = RFSnifferEEPROM::readProtocol(protoId);
							nbytes = (proto.messageBits+7)/8;
							uint8_t buf[nbytes];
							rx.SelectPin( proto.mediumRF() );
							while( proto.readMessage(rx,buf) != proto.messageBits ) ;
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
						cout <<regIndex<<":"<< readCommandMemory(cmem,regIndex) << endl;
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

};

template<typename TxPinT, typename RxPinT>
static inline RFSnifferInterpreter<TxPinT,RxPinT> make_rfsniffer_interpreter(TxPinT& tx, RxPinT& rx, PrintStream& out)
{
	return RFSnifferInterpreter<TxPinT,RxPinT>(tx,rx,out);
}

#endif
