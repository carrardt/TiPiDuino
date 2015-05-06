#include "RFSnifferInterpreter.h"

#define COMMAND_IMPL(_c_) void RFSnifferInterpreterBase::_c_ (ByteStream* rawInput)
#define POP() 				stack[ (sp>0) ? --sp : 0 ]
#define PUSH(x) 			stack[ sp++ ] = (x)

int16_t RFSnifferInterpreterBase::readValue(ByteStream* rawInput)
{
	InputStream cin;
	cin.begin( rawInput );
	int32_t x = 0;
	cin >> x;
	if( x < 0 ) { x = stack[sp-1+x]; }
	return x;
}

typedef void(RFSnifferInterpreterBase::*CommandFunctionPointer)(ByteStream*) ;

void RFSnifferInterpreterBase::processCommands(ByteStream* rawInput)
{
	while( !rawInput->eof() )
	{
		InputStream cin;
		cin.begin( rawInput );
		char c = cin.readFirstNonSpace();
		switch(c)
		{
			// reset EEPROM
			case 'Z':
			{
				RFSnifferEEPROM::resetEEPROM();
			}
			
			// reboot
			case 'o':
			{
				RFSnifferEEPROM::setOperationMode( POP() );
				asm volatile ("  jmp 0");
			}

			// remove protocol and associated messages/programs
			case 'z':
			{
				RFSnifferEEPROM::removeProtocol( POP() );
			}
			break;

			// push
			case 'P':
			{
				PUSH( readValue(rawInput) );
			}
			break;

			// pop
			case 'p':
			{
				POP();
			}	
			break;


			// set boot program id
			case 'b':
			{
				RFSnifferEEPROM::setBootProgram( POP() );
			}
			break;

			// query message content
			case 'q':
			{
				RFSnifferEEPROM::MessageInfo mesg = RFSnifferEEPROM::getMessageInfo( POP() );
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
				auto proto = RFSnifferEEPROM::readProtocol( POP() );
				proto.toStream(cout);
			}
			break;

			// add
			case 'a':
			{
				int16_t x = POP();
				int16_t y = POP();
				PUSH( x + y );
			}
			break;

			// add a program/message to eeprom
			case 'm':
			{
				int16_t bufSize = POP();
				uint8_t buf[bufSize];
				BufferStream progStream(buf,bufSize);
				progStream.copy( rawInput );
				progStream.rewind();
				int mId = RFSnifferEEPROM::appendMessage(RFSnifferEEPROM::PROGRAM_PROTOCOL_ID,&progStream);
				PUSH( mId );
			}
			break;

			// execute a program in eeprom
			case 'e':
			{
				int16_t progId = POP();
				auto progStream = RFSnifferEEPROM::getMessageStream(progId);
				processCommands(&progStream);
			}
			break;

			// wait x milliseconds
			case 'w':
			{
				int32_t duration = POP();
				duration*=1000;
				avrtl::DelayMicroseconds(duration);
			}
			break;

			// for loop
			case 'l':
			{
				uint16_t bufSize = POP();
				uint16_t N = POP();
				uint8_t buf[bufSize];
				BufferStream bufInput(buf,bufSize);
				bufInput.copy( rawInput );
				int ssp=sp++;
				for(stack[ssp]=0;stack[ssp]<N;stack[ssp]++)
				{
					bufInput.rewind();
					this->processCommands(&bufInput);
				}
				POP();
			}
			break;

			case 'r': this->vr(rawInput); break;
			case 'R': this->vR(rawInput); break;
			case 's': this->vs(rawInput); break;

			case 'd':
			{
				for(int i=0;i<sp;i++) { cout<<stack[i]<<' '; }
				cout<<endl;
			}
			break;

			// considered as a comment line, read until '\n'
			default:
			{
				while( !rawInput->eof() && c!='\n' ) c=rawInput->readByte();
			}
			break;
		}
	}
}
