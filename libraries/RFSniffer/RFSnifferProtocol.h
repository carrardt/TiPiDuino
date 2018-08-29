#ifndef __RFSNIFFERPROTOCOL_H
#define __RFSNIFFERPROTOCOL_H

#include "RFSnifferConstants.h"
#include <stdint.h>
#include "BasicIO/PrintStream.h"
#include "SignalProcessing/SignalProcessing.h"

/*
 	*** DI-O/Chacon RF433 protocol ***
 	RFSnifferProtocol remote;
	remote.coding = 77;
	remote.flags = 194;
	remote.latchSeqLen = 2;
	remote.latchSeq[0] = 10762;
	remote.latchSeq[1] = 2834;
	remote.latchGap = 221;
	remote.bitGap = 221;
	remote.bitSymbols[0] = 332;
	remote.bitSymbols[1] = 1397;
	remote.messageBits = 32;
	remote.nMessageRepeats = 4;
 */


struct RFSnifferProtocol
{
	static constexpr uint8_t LOW_LEVEL_FLAG = 0x00;
	static constexpr uint8_t HIGH_LEVEL_FLAG = 0x01;

	static constexpr uint8_t IR_FLAG = 0x00;
	static constexpr uint8_t RF_FLAG = 0x02;

	static constexpr uint8_t MODULATION_MASK  = 0x1C;
	static constexpr uint8_t MODULATION_NONE  = 0x00;
	static constexpr uint8_t MODULATION_36KHZ = 0x04;
	static constexpr uint8_t MODULATION_38KHZ = 0x08;
	static constexpr uint8_t MODULATION_40KHZ = 0x10;

	static constexpr uint8_t MATCHING_REPEATS_FLAG = 0x40;
	static constexpr uint8_t VALID_FLAG = 0x80;

	static constexpr uint8_t RESET_MODIFIY_FLAGS_MASK = 0x03;
	
	RFSnifferProtocol() { init(); }

	void init()
	{
		messageBits = 0;
		latchSeqLen = 0;
		bitSymbols[0] = 0;
		bitSymbols[1] = 0;
		for(int i=0;i<MAX_LATCH_SEQ_LEN;i++) latchSeq[i]=0;
		nMessageRepeats = 0;
		latchGap = 8192;
		bitGap = 512;
		coding = CODING_UNKNOWN;
		flags = RFSnifferProtocol::defaultFlags;
	}

	uint8_t pulseModulation() const
	{
		return flags & MODULATION_MASK;
	}
	
	uint8_t setPulseModulation(uint8_t m)
	{
		flags &= ~MODULATION_MASK;
		flags |= m;
	}

	bool mediumRF() const 
	{
		return (flags&RF_FLAG)!=0;
	}
	
	void setMediumRF(bool value)
	{
		if(value) { flags |= RF_FLAG; }
		else { flags &= ~RF_FLAG; }
	}

	bool matchingRepeats() const
	{
		return (flags&MATCHING_REPEATS_FLAG)!=0;
	}
	void setMatchingRepeats(bool value)
	{
		if(value) { flags |= MATCHING_REPEATS_FLAG; }
		else { flags &= ~MATCHING_REPEATS_FLAG; }
	}

	bool pulseLevel() const
	{
		return (flags&HIGH_LEVEL_FLAG)!=0;
	}

	void setPulseLevel(bool value)
	{
		if(value) { flags |= HIGH_LEVEL_FLAG; }
		else { flags &= ~HIGH_LEVEL_FLAG; }
	}

	void setValid(bool v)
	{
		if(v) flags |= VALID_FLAG;
		else flags &= ~VALID_FLAG;
	}

	bool isValid() const
	{
		return (flags&VALID_FLAG)!=0 && bitSymbols[0]>0 && bitSymbols[1]>0;
	}	

	inline void toStream(PrintStream& out) { toStreamFull(out); }

	inline void toStreamFull(PrintStream& out)
	{
		out<<"latchGap="<<(int)latchGap<<endl;
		out<<"latchSeq="; for(int i=0;i<MAX_LATCH_SEQ_LEN;i++){ out<<(int)(latchSeq[i])<<' '; } out<<endl;
		out<<"bitGap="<<(int)bitGap<<endl;
		out<<"bitSymbols=["<<(int)(bitSymbols[0])<<';'<<(int)(bitSymbols[1])<<']'<<endl;
		out<<"messageBits="<<(int)messageBits<<endl;
		out<<"latchSeqLen="<<(int)latchSeqLen<<endl;
		out<<"nMessageRepeats="<<(int)nMessageRepeats<<endl;
		out<<"coding="<<(int)coding<<endl;
		out<<"flags="<<(int)flags<<endl;
	}
	
	inline void toStreamShort(PrintStream& out)
	{
		if(latchSeqLen>0)
		{
			out <<'L';
			for(int i=0;i<latchSeqLen;i++)
			{
				if(i>0) out<<'-';
				out.print(latchSeq[i],16);
			}
		}
		out<<'G';
		out.print(latchGap,16);
		if(bitGap!=latchGap)
		{
			out<<'-';
			out.print(bitGap,16);
		}
		if(pulseModulation()!=MODULATION_NONE)
		{
			out<<'K';
			switch( pulseModulation() )
			{
				case MODULATION_36KHZ: out<<"36"; break;
				case MODULATION_38KHZ: out<<"38"; break;
				case MODULATION_40KHZ: out<<"40"; break;
			}
		}
		out<<endl;
		out << ( mediumRF() ? 'R' : 'I' );		
		out << ( pulseLevel() ? "H" : "L" );
		out << (char) coding;
		out.print(messageBits,16);
		if( nMessageRepeats > 1 )
		{
			out << 'x';
			out.print((int)nMessageRepeats);
		}
		for(int i=0;i<2;i++)
		{
			out<<'-';
			out.print((int)bitSymbols[i],16);
		}
		out<<endl;
	}

	inline uint16_t getLatchGap(uint8_t l) const
	{
		return latchGap;
	}
	inline uint16_t getBitGap(bool bvalue) const
	{
		return bitGap;
	}

	static bool decodeManchester(uint8_t* buf, int nbits)
	{
		int j=0, k=0;
		uint8_t byte=0;
		uint8_t x[2];
		for(int i=0;i<nbits;i++)
		{
			int bpos = i%8;
			x[i%2] = ( buf[i/8] >> (7-bpos) ) & 1;
			if( i%2 == 1 )
			{
				if( x[0]==x[1] ) return false;
				byte = (byte << 1) | x[0];
				++k;
				if( k==8 )
				{
					buf[j++] = byte;
					k=0;
				}
			}
		}
		if( k!=0 ) buf[j++] = byte;
		return true;
	}

	template<typename RxPinT>
	int readMessage(RxPinT& rx, uint8_t* buf)
	{
		SignalProcessing32 sp;
		
		const uint16_t b0 = bitSymbols[0];
		const uint16_t b1 = bitSymbols[1];
		const uint16_t b0_tol = b0 / PULSE_ERR_RATIO;
		const uint16_t b1_tol = b1 / PULSE_ERR_RATIO;

		int bitsToRead = messageBits;
		if( coding == CODING_MANCHESTER ) bitsToRead*=2;
		const bool lvl = pulseLevel();
		
		uint8_t byte=0;
		uint16_t j=0;
		if( latchSeqLen > 0 )
		{
			for(uint8_t i=0;i<latchSeqLen;)
			{
				long p = sp.PulseIn(rx,lvl,MAX_PULSE_LEN);
				uint16_t l = latchSeq[i];
				uint16_t relerr = l / PULSE_ERR_RATIO;
				if( avrtl::abs(p-l) > relerr ) i=0;
				else ++i;
			}
		}
		else
		{
			bool badBit;
			do
			{
				long p = sp.PulseIn(rx,lvl,MAX_PULSE_LEN);
				buf[byte] = 0;
				badBit = false;
				if( avrtl::abs(p-b1) <= b1_tol ) buf[byte] = 1;
				else if( avrtl::abs(p-b0) > b0_tol ) badBit=true;
			}
			while( badBit );
			++ j;
		}
		for(;j<bitsToRead;j++)
		{
			if(j%8==0) buf[byte]=0;
			long p = sp.PulseIn(rx,lvl,MAX_PULSE_LEN);
			uint8_t b = 0;
			if( avrtl::abs(p-b1) <= b1_tol ) b = 1;
			else if( avrtl::abs(p-b0) > b0_tol ) return 0;
			buf[byte] <<= 1;
			buf[byte] |= b;
			if( (j%8)==7 ) { ++byte;  }
		}
		if( coding == CODING_MANCHESTER )
		{
			if( ! decodeManchester(buf,bitsToRead) ) return 0;
			else return messageBits;
		}
		else { return bitsToRead; }
	}

	template<typename RxPinT>
	int readMessageWithGaps(RxPinT& rx, uint8_t* buf, uint16_t* in_gaps)
	{
		SCOPED_SIGNAL_PROCESSING;

		const uint16_t b0 = bitSymbols[0];
		const uint16_t b1 = bitSymbols[1];
		const uint16_t b0_tol = b0 / PULSE_ERR_RATIO;
		const uint16_t b1_tol = b1 / PULSE_ERR_RATIO;
		uint16_t* gaps = in_gaps;

		int bitsToRead = messageBits;
		if( coding == CODING_MANCHESTER ) bitsToRead*=2;
		const bool lvl = pulseLevel();
		
		uint8_t byte=0;
		uint16_t j=0;
		if( latchSeqLen > 0 )
		{
			for(uint8_t i=0;i<latchSeqLen;)
			{
				long p = avrtl::PulseInFast(rx,lvl,MAX_PULSE_LEN,gaps++);
				uint16_t l = latchSeq[i];
				uint16_t relerr = l / PULSE_ERR_RATIO;
				if( avrtl::abs(p-l) > relerr ) { i=0; gaps=in_gaps; }
				else ++i;
			}
		}
		else
		{
			bool badBit;
			do
			{
				long p = avrtl::PulseInFast(rx,lvl,MAX_PULSE_LEN,gaps);
				buf[byte] = 0;
				badBit = false;
				if( avrtl::abs(p-b1) <= b1_tol ) buf[byte] = 1;
				else if( avrtl::abs(p-b0) > b0_tol ) badBit=true;
			}
			while( badBit );
			++gaps;
			++ j;
		}
		for(;j<bitsToRead;j++)
		{
			if(j%8==0) buf[byte]=0;
			long p = avrtl::PulseInFast(rx,lvl,MAX_PULSE_LEN,gaps++);
			uint8_t b = 0;
			if( avrtl::abs(p-b1) <= b1_tol ) b = 1;
			else if( avrtl::abs(p-b0) > b0_tol ) return 0;
			buf[byte] <<= 1;
			buf[byte] |= b;
			if( (j%8)==7 ) { ++byte;  }
		}
		for(;in_gaps!=gaps;in_gaps++) { *in_gaps = avrtl::ticksToMicroseconds(*in_gaps); }
		if( coding == CODING_MANCHESTER )
		{
			if( ! decodeManchester(buf,bitsToRead) ) return 0;
			else return messageBits;
		}
		else { return bitsToRead; }
	}

	template<typename LineSetFuncT>
	void writeMessageFast(const uint8_t* buf, uint8_t len, LineSetFuncT lineState)
	{
		const bool manchester = (coding == CODING_MANCHESTER);

		for(int r=0;r<nMessageRepeats;r++)
		{
			for(uint8_t i=0;i<latchSeqLen;i++)
			{
				lineState( ! pulseLevel(), getLatchGap(i) + 50 );
				lineState( pulseLevel(), latchSeq[i] - 50 );
			}

			for(uint8_t i=0;i<len;i++)
			{
				uint8_t curbyte = buf[i];
				for(int8_t j=0;j<8;j++)
				{
					bool b = ( (curbyte&0x80) != 0 );
					lineState( ! pulseLevel(), getBitGap(b) + 50 );
					curbyte <<= 1;
					lineState( pulseLevel(), bitSymbols[b] - 50 );
					if( manchester )
					{
						b = !b;
						lineState( ! pulseLevel(), getBitGap(b) + 50 );
						lineState( pulseLevel(), bitSymbols[b] - 50 );
					}
				}
			}
		}
		lineState( ! pulseLevel() , 10000UL );
	}
	
	template<typename LineSetFuncT>
	void writeMessage(const uint8_t* buf, uint8_t len, LineSetFuncT lineState )
	{
		SCOPED_SIGNAL_PROCESSING;
		writeMessageFast(buf,len,lineState);
	}

	uint16_t messageTotalPulses() const
	{
		int bitsToRead = messageBits;
		if(coding==CODING_MANCHESTER) bitsToRead*=2;
		return latchSeqLen+bitsToRead;
	}

	uint16_t messageBytes() const
	{
		return (messageBits+7)/8;
	}

	uint16_t messageReadBufferSize() const
	{
		int bitsToRead = messageBits;
		if(coding==CODING_MANCHESTER) bitsToRead*=2;
		return (bitsToRead+7)/8;
	}

	static uint8_t defaultFlags;

	uint16_t latchGap;
	uint16_t latchSeq[MAX_LATCH_SEQ_LEN];
	uint16_t bitGap;
	uint16_t bitSymbols[2];
	uint16_t messageBits;
	uint8_t latchSeqLen;
	uint8_t nMessageRepeats;
	uint8_t coding;
	uint8_t flags;
};

#endif
