#ifndef __RFSNIFFER_CommandLine_h
#define __RFSNIFFER_CommandLine_h

#include "ByteStream.h"
#include "InputStream.h"
#include "PrintStream.h"
#include "RFSnifferEEPROM.h"
#include "RFSnifferProtocol.h"
#include "AvrTL.h"
#include <stdint.h>

#define POP() 				stack[--sp]
#define PUSH(x) 			stack[sp++]=(x)
#define VCOMMAND_IMPL(_c_) virtual void v##_c_ (ByteStream* rawInput)
#define COMMAND_DECL(_c_) void _c_ (ByteStream* rawInput)
#define VCOMMAND_DECL(_c_) \
	inline void _c_ (ByteStream* i) { this->v##_c_ (i); } \
	virtual void v##_c_ (ByteStream* i)=0

struct RFSnifferInterpreterBase
{
	inline RFSnifferInterpreterBase(PrintStream& _cout) : cout(_cout), sp(0) {}
	
	int16_t readValue(ByteStream* rawIput);

	// send a message
	VCOMMAND_DECL(s);
	
	// read a message and push it's Id
	VCOMMAND_DECL(r);
	
	// Read a raw message and print it, for debug purposes
	VCOMMAND_DECL(R);
	
	void processCommands(ByteStream* rawIput);

	int16_t stack[8];
	PrintStream& cout;
	uint8_t sp;
};

template<typename RxPinT, typename RFTxPinT, typename IRTxPinT>
struct RFSnifferInterpreter : public RFSnifferInterpreterBase
{	
	inline RFSnifferInterpreter( RxPinT& _rx, RFTxPinT& _rf, IRTxPinT& _ir, PrintStream& _cout)
		: RFSnifferInterpreterBase(_cout)
		, rx(_rx)
		, rf_tx(_rf)
		, ir_tx(_ir)
		{}
		
	VCOMMAND_IMPL(s)
	{
		int16_t mesgId = POP();
		RFSnifferEEPROM::MessageInfo mesg = RFSnifferEEPROM::getMessageInfo( mesgId );
		auto proto = RFSnifferEEPROM::readProtocol( mesg.protocolId );
		uint8_t buf[mesg.nbytes];
		eeprom_read_block(buf,mesg.eeprom_addr,mesg.nbytes);
		
		SCOPED_SIGNAL_PROCESSING;
		if( proto.mediumRF() )
		{
			proto.writeMessageFast(buf,mesg.nbytes,[&](bool l,uint32_t t)
				{ avrtl::setLineFlatFast(rf_tx,l,t); } );
			rf_tx = 0;
		}
		else
		{
			switch( proto.pulseModulation() )
			{
				case RFSnifferProtocol::MODULATION_36KHZ:
					proto.writeMessageFast(buf,mesg.nbytes,[&](bool l,uint32_t t)
						{ avrtl::setLinePWMFast<36000,avrtl::pwmval(0.33)>(ir_tx,!l,t); } );
					break;
				case RFSnifferProtocol::MODULATION_38KHZ:
					proto.writeMessageFast(buf,mesg.nbytes,[&](bool l,uint32_t t)
						{ avrtl::setLinePWMFast<38000,avrtl::pwmval(0.33)>(ir_tx,!l,t); } );
					break;
				case RFSnifferProtocol::MODULATION_40KHZ:
					proto.writeMessageFast(buf,mesg.nbytes,[&](bool l,uint32_t t)
						{ avrtl::setLinePWMFast<40000,avrtl::pwmval(0.33)>(ir_tx,!l,t); } );
					break;
			}
			ir_tx = 0;
		}
	}

	VCOMMAND_IMPL(r)
	{
		int16_t mId;
		{
			int protoId = POP();
			int nbytes = 0;
			auto proto = RFSnifferEEPROM::readProtocol(protoId);
			nbytes = (proto.messageBits+7)/8;
			uint8_t buf[nbytes];
			rx.SelectPin( proto.mediumRF() );
			while( proto.readMessage(rx,buf) != proto.messageBits ) ;
			mId = RFSnifferEEPROM::findRecordedMessage(protoId,buf,nbytes);
		}
		PUSH( mId );
	}

	VCOMMAND_IMPL(R)
	{
		uint32_t timeout = POP();
		uint16_t nSamples = POP();
		bool RFMode = POP();
		uint16_t buf[nSamples];
		rx.SelectPin( RFMode );
		uint16_t rSamples = 0;
		bool lvl = rx.Get();
		{
			SCOPED_SIGNAL_PROCESSING;
			do {
				rSamples = RecordSignalFast(rx,timeout*1000UL,nSamples,buf);
			} while( rSamples < 2 );
		}
		for(int i=0;i<rSamples;i++)
		{
			cout<<i<<':'<< lvl <<':' <<avrtl::ticksToMicroseconds(buf[i])<<endl;
			lvl = !lvl;
		}
	}

	RxPinT& rx;
	RFTxPinT& rf_tx;
	IRTxPinT& ir_tx;
};

#undef POP
#undef PUSH
#undef VCOMMAND_IMPL
#undef COMMAND_DECL
#undef VCOMMAND_DECL

template<typename RxPinT, typename RFTxPinT, typename IRTxPinT>
static inline RFSnifferInterpreter<RxPinT,RFTxPinT,IRTxPinT>
make_rfsniffer_interpreter(RxPinT& _rx, RFTxPinT& _rftx, IRTxPinT& _irtx, PrintStream& _out)
{
	return RFSnifferInterpreter<RxPinT,RFTxPinT,IRTxPinT>(_rx,_rftx,_irtx,_out);
}

#endif
