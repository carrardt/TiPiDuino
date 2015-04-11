#ifndef __RFSNIFFERPROTOCOL_H
#define __RFSNIFFERPROTOCOL_H

#include "RFSnifferConstants.h"
#include <stdint.h>

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
	void init();
	bool mediumRF() const;
	void setMediumRF(bool value);
	bool matchingRepeats() const;
	void setMatchingRepeats(bool value);
	inline bool pulseLevel() const { return (flags&HIGH_LEVEL_FLAG)!=0; }
	void setPulseLevel(bool value);
	void toEEPROM(void* eeprom_addr) const;
	void fromEEPROM(const void* eeprom_addr);
	void setValid(bool v);
	bool isValid() const;

	template<typename OStreamT>
	inline void toStream(OStreamT& out)
	{
		if( latchSeqLen>0 )
		{
			out <<'L';
			for(int i=0;i<latchSeqLen;i++)
			{
				out<<'-';
				out.print(latchSeq[i],16);
			}
			out<<'\n';
		}
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


	static uint8_t defaultFlags;
	
	uint16_t bitSymbols[2];
	uint16_t latchSeq[MAX_LATCH_SEQ_LEN];
	uint16_t messageBits;
	uint8_t latchSeqLen;
	uint8_t nMessageRepeats;
	uint8_t coding;
	uint8_t flags;
};

#endif
