#ifndef __RFSNIFFERPROTOCOL_H
#define __RFSNIFFERPROTOCOL_H

#include "RFSnifferConstants.h"
#include "AvrTL.h"
#include <stdint.h>
#include <avr/eeprom.h>

struct RFSnifferProtocol
{
	static constexpr uint8_t MATCHING_REPEATS_FLAG = 0x10;
	static constexpr uint8_t VALID_FLAG = 0x20;

	static constexpr uint8_t LOW_LEVEL_FLAG = 0x00;
	static constexpr uint8_t HIGH_LEVEL_FLAG = 0x01;

	static constexpr uint8_t IR_FLAG = 0x00;
	static constexpr uint8_t RF_FLAG = 0x02;
	
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
		pulseGap = 255;
		coding = CODING_UNKNOWN;
		flags = RFSnifferProtocol::defaultFlags;
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

	void toEEPROM(void* eeprom_addr) const
	{
		avrtl::eeprom_gently_write_block( (const uint8_t*)this, (uint8_t*)eeprom_addr, sizeof(RFSnifferProtocol) );
	}

	void fromEEPROM(const void* eeprom_addr)
	{
		eeprom_read_block( (void*)this, eeprom_addr, sizeof(RFSnifferProtocol) );
		if( ! isValid() )
		{
			init();
		}
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

	template<typename OStreamT>
	inline void toStream(OStreamT& out)
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
		out.print(pulseGap,16);
		out<<'\n';
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
		out<<'\n';
	}

	inline uint16_t getLatchGap(uint8_t l) const { return pulseGap; }
	inline uint16_t getBitGap(bool bvalue) const { return pulseGap; }

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
				long p = rx.PulseIn(lvl,MAX_PULSE_LEN);
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
				long p = rx.PulseIn(lvl,MAX_PULSE_LEN);
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
			long p = rx.PulseIn(lvl,MAX_PULSE_LEN);
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
				long p = rx.PulseIn(lvl,MAX_PULSE_LEN,gaps++);
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
				long p = rx.PulseIn(lvl,MAX_PULSE_LEN,gaps);
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
			long p = rx.PulseIn(lvl,MAX_PULSE_LEN,gaps++);
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


	template<typename TxPinT>
	void writeMessage(const uint8_t* buf, uint8_t len, TxPinT& tx)
	{
		tx = ! pulseLevel();
		avrtl::DelayMicroseconds(MAX_PULSE_LEN);
		for(uint8_t i=0;i<latchSeqLen;i++)
		{
			tx = pulseLevel();
			avrtl::DelayMicroseconds( latchSeq[i] );
			tx = ! pulseLevel();
			avrtl::DelayMicroseconds( getLatchGap(i) );
		}
		
		const bool manchester = (coding == CODING_MANCHESTER);
		for(uint8_t i=0;i<len;i++)
		{
			for(uint8_t j=0;j<8;j++)
			{
				tx = pulseLevel();
				uint8_t b = ( buf[i] >> j ) & 0x01 ;
				avrtl::DelayMicroseconds( bitSymbols[b] );
				tx = ! pulseLevel();
				avrtl::DelayMicroseconds( getBitGap(b) );
				if( manchester )
				{
					tx = pulseLevel();
					b ^= 0x01;
					avrtl::DelayMicroseconds( bitSymbols[b] );
					tx = ! pulseLevel();
					avrtl::DelayMicroseconds( getBitGap(b) );
				}
			}
		}
		tx = ! pulseLevel();
	}

	static uint8_t defaultFlags;

	uint16_t bitSymbols[2];
	uint16_t latchSeq[MAX_LATCH_SEQ_LEN];
	uint16_t pulseGap;
	uint16_t messageBits;
	uint8_t latchSeqLen;
	uint8_t nMessageRepeats;
	uint8_t coding;
	uint8_t flags;
};

#endif
