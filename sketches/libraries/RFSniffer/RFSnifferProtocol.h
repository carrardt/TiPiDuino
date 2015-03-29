#ifndef __RFSNIFFERPROTOCOL_H
#define __RFSNIFFERPROTOCOL_H

#include <avr/eeprom.h>
#include "RFSnifferConstants.h"
#include "AvrTL.h"

struct RFSnifferProtocol
{
	static constexpr uint32_t EEPROM_MAGIC_NUMBER = 0x040411UL;
	
	uint32_t magic;
	uint16_t bitSymbols[2];
	uint16_t latchSeq[MAX_LATCH_SEQ_LEN];
	uint8_t latchSeqLen;
	uint16_t messageBits;
	uint8_t nMessageRepeats;
	uint8_t coding;
	bool matchingRepeats;
	bool pulseLevel;
	
	inline RFSnifferProtocol()
	{
		init();
	}

	inline void init()
	{
		magic = 0xFFFFFFFF ;
		messageBits = 0;
		latchSeqLen = 0;
		bitSymbols[0] = 0;
		bitSymbols[1] = 0;
		for(int i=0;i<MAX_LATCH_SEQ_LEN;i++) latchSeq[i]=0;
		nMessageRepeats = 0;
		coding = CODING_UNKNOWN;
		matchingRepeats = false;
		pulseLevel = true;
	}

	inline void toEEPROM(void* eeprom_addr)
	{
		setValid(true);
		avrtl::eeprom_gently_write_block( (const uint8_t*)this, (uint8_t*)eeprom_addr, sizeof(RFSnifferProtocol) );
	}

	inline void fromEEPROM(const void* eeprom_addr)
	{
		eeprom_read_block( (void*)this, eeprom_addr, sizeof(RFSnifferProtocol) );
		if( ! isValid() )
		{
			init();
		}
	}

	inline void invalidateEEPROM(void* eeprom_addr)
	{
		setValid(false);
		eeprom_write_block( (const void*)this, eeprom_addr, sizeof(RFSnifferProtocol) );
	}

	inline void setValid(bool v) { magic = v ? EEPROM_MAGIC_NUMBER : 0xFFFFFFFF; }
	inline bool isValid() { return magic == EEPROM_MAGIC_NUMBER; }

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
		out << (char) coding;
		out.print((int)messageBits,16);
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

};

#endif
