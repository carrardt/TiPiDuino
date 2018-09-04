#ifndef __RFSNIFFER_CommandLine_h
#define __RFSNIFFER_CommandLine_h

#include "BasicIO/ByteStream.h"
#include "BasicIO/InputStream.h"
#include "BasicIO/PrintStream.h"
#include "RFSnifferEEPROM.h"
#include "RFSnifferProtocol.h"
#include "AvrTL.h"
#include "SignalProcessing/SignalProcessing.h"
#include <stdint.h>

/*
 * Démarrer l'ampli Hi-Fi
 * 50 m0 s 9999 w 5 5 5 4 3 2 2 2 2 2 3 2 1 13 7 ls 999 wq
 */

template<typename RxPinT, typename RFTxPinT, typename IRTxPinT>
struct RFSnifferInterpreter 
{
	static constexpr uint16_t STACK_SIZE = 32;
	
	int16_t pop()
	{
		if( sp==0 ) return 0;
		else
		{
			-- sp;
			return stack[sp];
		}
	}
	
	void push(int16_t x)
	{
		if(sp<STACK_SIZE)
		{
			stack[sp] = x;
			++ sp;
		}
	}
	
	int16_t readValue(ByteStream* rawInput)
	{
		InputStream cin;
		cin.begin( rawInput );
		int32_t x = 0;
		cin >> x;
		if( x < 0 ) { x = stack[sp-1+x]; }
		return x;
	}
	
	void processCommands(ByteStream* rawInput)
	{
		SignalProcessing32 sproc;
		while( !rawInput->eof() )
		{
			InputStream cin;
			cin.begin( rawInput );
			char c = cin.readFirstNonSpace();
			if( c=='-' || c=='+' || ( c>='0' && c<='9' ) )
			{
				int16_t value = cin.readInteger( c );
				push( value );
			}
			else switch(c)
			{
				// reset EEPROM
				case 'Z':
				{
					RFSnifferEEPROM::resetEEPROM();
				}
				
				// reboot
				case 'o':
				{
					RFSnifferEEPROM::setOperationMode( pop() );
					asm volatile ("  jmp 0");
				}

				// remove protocol and associated messages/programs
				case 'z':
				{
					RFSnifferEEPROM::removeProtocol( pop() );
				}
				break;

				// pop
				case 'p':
				{
					pop();
				}	
				break;

				// set boot program id
				case 'b':
				{
					uint8_t bootProgId = pop();
					RFSnifferEEPROM::setBootProgram( bootProgId );
				}
				break;

				// query message content
				case 'q':
				{
					RFSnifferEEPROM::MessageInfo mesg = RFSnifferEEPROM::getMessageInfo( pop() );
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
					auto proto = RFSnifferEEPROM::readProtocol( pop() );
					proto.toStream(cout);
				}
				break;

				// add
				case 'a':
				{
					int16_t x = pop();
					int16_t y = pop();
					push( x + y );
				}
				break;

				// mul
				case 'x':
				{
					int16_t x = pop();
					int16_t y = pop();
					push( x * y );
				}
				break;

				// test values. if 2 values on top of stack are equal push 1, otherwise push 0
				case 't':
				{
					int16_t x = pop();
					int16_t y = pop();
					push( (x==y) ? 1 : 0 );
				}
				break;

				// add a program/message to eeprom
				case 'm':
				{
					int16_t bufSize = pop();
					uint8_t buf[bufSize];
					BufferStream progStream(buf,bufSize);
					progStream.copy( rawInput );
					progStream.rewind();
					int mId = RFSnifferEEPROM::appendMessage(RFSnifferEEPROM::PROGRAM_PROTOCOL_ID,&progStream);
					push( mId );
				}
				break;

				// execute a program in eeprom
				case 'e':
				{
					int16_t progId = pop();
					auto progStream = RFSnifferEEPROM::getMessageStream(progId);
					processCommands(&progStream);
				}
				break;

				// wait x milliseconds
				case 'w':
				{
					int32_t duration = pop();
					duration*=1000;
					avrtl::delayMicroseconds(duration);
				}
				break;

				// for loop
				case 'l':
				{
					uint16_t bufSize = pop();
					uint16_t N = pop();
					uint8_t buf[bufSize];
					BufferStream bufInput(buf,bufSize);
					bufInput.copy( rawInput );
					for(int i=0;i<N;i++)
					{
						bufInput.rewind();
						this->processCommands(&bufInput);
					}
				}
				break;

				case 'r':
				{
					int16_t mId;
					{
						int protoId = pop();
						int nbytes = 0;
						auto proto = RFSnifferEEPROM::readProtocol(protoId);
						nbytes = (proto.messageBits+7)/8;
						uint8_t buf[nbytes];
						rx.SelectPin( proto.mediumRF() );
						while( proto.readMessage(rx,buf) != proto.messageBits ) ;
						mId = RFSnifferEEPROM::findRecordedMessage(protoId,buf,nbytes);
					}
					push( mId );
				}
				break;
				
				case 'R':
				{
					uint32_t timeout = pop();
					uint16_t nSamples = pop();
					bool RFMode = pop();
					uint16_t buf[nSamples];
					rx.SelectPin( RFMode );
					uint16_t rSamples = 0;
					bool lvl = rx.Get();
					{
						do {
							rSamples = sproc.RecordSignal(rx,timeout*1000UL,nSamples,buf);
						} while( rSamples < 2 );
					}
					for(int i=0;i<rSamples;i++)
					{
						cout<<i<<':'<< lvl <<':' <<sproc.m_ts.m_timer.ticksToMicroseconds(buf[i])<<endl;
						lvl = !lvl;
					}
				}
				break;
				
				// Test emission
				case 'T':
				{
					int16_t RFMode = pop();
					if( RFMode ) { sproc.blink(rf_tx); }
					else { sproc.blink(ir_tx); }
				}
				break;
			
				case 's':
				{
					int16_t mesgId = pop();
					RFSnifferEEPROM::MessageInfo mesg = RFSnifferEEPROM::getMessageInfo( mesgId );
					auto proto = RFSnifferEEPROM::readProtocol( mesg.protocolId );
					uint8_t buf[mesg.nbytes];
					eeprom_read_block(buf,mesg.eeprom_addr,mesg.nbytes);
					
					if( proto.mediumRF() )
					{
						proto.writeMessage(buf,mesg.nbytes,[&](bool l,uint32_t t)
							{ sproc.setLineFlat(rf_tx,l,t); } );
						rf_tx = 0;
					}
					else
					{
						static constexpr uint16_t pulseFraction = 21845; // 1/3 coded on 16 bit unsigned int
						uint8_t mod = proto.pulseModulation();	
						if( mod == RFSnifferProtocol::MODULATION_NONE )
						{
							mod = RFSnifferProtocol::MODULATION_38KHZ;
						}
						switch( mod )
						{
							case RFSnifferProtocol::MODULATION_36KHZ:
								proto.writeMessage(buf,mesg.nbytes,[&](bool l,uint32_t t)
									{ sproc.setLinePWM<36000,pulseFraction>(ir_tx,!l,t); } );
								break;
							case RFSnifferProtocol::MODULATION_38KHZ:
								proto.writeMessage(buf,mesg.nbytes,[&](bool l,uint32_t t)
									{ sproc.setLinePWM<38000,pulseFraction>(ir_tx,!l,t); } );
								break;
							case RFSnifferProtocol::MODULATION_40KHZ:
								proto.writeMessage(buf,mesg.nbytes,[&](bool l,uint32_t t)
									{ sproc.setLinePWM<40000,pulseFraction>(ir_tx,!l,t); } );
								break;
						}
						ir_tx = 0;
					}
				}
				break;

				case 'd':
				{
					for(int i=0;i<sp;i++) { cout<<stack[i]<<' '; }
					cout<<endl;
				}
				break;

				case 'D':
				{
					cout<<endl;
					for(int i=0;i<1024;i++)
					{ 
						cout.print(eeprom_read_byte((const uint8_t*)i),16,2);
					}
					cout<<endl;
				}
				break;
				
				// considered as a comment line, read until '\n'
				default:
				/*{
					while( !rawInput->eof() && c!='\n' ) c=rawInput->readByte();
				}*/
				break;
			}
		}
	}


	RFSnifferInterpreter( RxPinT& _rx, RFTxPinT& _rf, IRTxPinT& _ir, PrintStream& _cout)
		: cout(_cout)
		, rx(_rx)
		, rf_tx(_rf)
		, ir_tx(_ir)
		{}
		
	int16_t stack[STACK_SIZE];
	PrintStream& cout;
	RxPinT& rx;
	RFTxPinT& rf_tx;
	IRTxPinT& ir_tx;
	uint8_t sp;
};

template<typename RxPinT, typename RFTxPinT, typename IRTxPinT>
static inline RFSnifferInterpreter<RxPinT,RFTxPinT,IRTxPinT>
make_rfsniffer_interpreter(RxPinT& _rx, RFTxPinT& _rftx, IRTxPinT& _irtx, PrintStream& _out)
{
	return RFSnifferInterpreter<RxPinT,RFTxPinT,IRTxPinT>(_rx,_rftx,_irtx,_out);
}

#endif
