#ifndef __RF_EMITTER_H
#define __RF_EMITTER_H

#include "RFSnifferConstants.h"
#include "RFSnifferProtocol.h"
#include "AvrTL.h"

template<typename TXPinT>
struct RFEmitter
{
	RFEmitter(TXPinT& _tx) : tx(_tx) {}
/*
	RFEmitter(const RFSnifferProtocol& _sp, TXPinT& _tx) : sp(_sp), tx(_tx) {}
	void sendPulse(long pre, long len)
	{
		avrtl::DelayMicroseconds(pre);
		tx = PULSE_LVL;
		avrtl::DelayMicroseconds(len);
		tx = !PULSE_LVL;
	}
	void sendBit(bool b)
	{
		sendPulse( 500,  sp.bitSymbols[b?1:0] );
	}

	void sendSignalOnce(int nbits, const uint8_t* input)
	{
		const bool bitpair = (sp.coding==CODING_MANCHESTER);
		tx = !PULSE_LVL;
		for(int i=0;i<sp.latchSeqLen;i++)
		{
			long l = sp.latchSymbols[sp.latchSeq[i]];
			sendPulse(500,l);
		}
		int b = 8;
		while( nbits > 0 )
		{
			-- nbits;
			-- b;
			uint8_t x = (*input >> b ) & 1;
			sendBit( x );
			if(bitpair) sendBit( !x );
			if( b == 0 ) { b=8; ++input; }
		}
	}
	*/
	void sendSignal(int nbits, const uint8_t* input)
	{
		tx = !PULSE_LVL;
		/*
		for(int i=0;i<sp.nMessageRepeats;i++)
		{
			sendSignalOnce(nbits,input);
		}
		*/
	}

	//const RFSnifferProtocol& sp;
	TXPinT& tx;
};

/*
template<typename TXPinT>
static RFEmitter<TXPinT> make_emitter(const RFSnifferProtocol& sp, TXPinT& tx) 
{
	return RFEmitter<TXPinT>(sp,tx);
}
*/

template<typename TXPinT>
static RFEmitter<TXPinT> make_emitter(TXPinT& tx) 
{
	return RFEmitter<TXPinT>(tx);
}

#endif
