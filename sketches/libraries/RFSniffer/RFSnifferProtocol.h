#ifndef __RFSNIFFERPROTOCOL_H
#define __RFSNIFFERPROTOCOL_H

#include <avr/eeprom.h>
#include "RFSnifferConstants.h"

struct RFSnifferProtocol
{
	uint32_t magic;
	uint16_t bitSymbols[2];
	uint16_t latchSymbols[MAX_LATCH_SEQ_LEN];
	int8_t nLatches;
	uint8_t latchSeq[MAX_LATCH_SEQ_LEN];
	uint8_t latchSeqLen;
	uint16_t messageBits;
	uint8_t nMessageRepeats;
	uint8_t coding;
	bool matchingRepeats;
	
	inline RFSnifferProtocol()
	{
		init();
	}

	inline void init()
	{
		magic = 0xFFFFFFFF ;
		messageBits = 0;
		nLatches = 0;
		latchSeqLen = 0;
		nMessageRepeats = 0;
		coding = CODING_UNKNOWN;
		matchingRepeats = false;
	}

	inline void toEEPROM(void* eeprom_addr)
	{
		setValid(true);
		eeprom_write_block( (const void*)this, eeprom_addr, sizeof(RFSnifferProtocol) );
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
		out << '\n';
		for(int i=0;i<latchSeqLen;i++)
		{
			out.print((int)latchSymbols[latchSeq[i]],16,4);
		}
		out << 'x';
		out.print((int)nMessageRepeats);
		out << '\n';
		for(int i=0;i<2;i++)
		{
			out.print((int)bitSymbols[i],16,4);
		}
		out << (char)coding;
		out.print((int)messageBits,16);
	}

};

#endif
