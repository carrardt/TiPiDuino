#ifndef __RFSNIFFERPROTOCOL_H
#define __RFSNIFFERPROTOCOL_H

#include <avr/eeprom.h>

struct RFSnifferProtocol
{
	uint32_t magic;
	/* uint16_t symbolgaps[MAX_SYMBOLS]; */ // unused
	uint16_t symbols[MAX_SYMBOLS];
	uint16_t nPulses;
	uint16_t messageBits;
	uint8_t latchSeq[MAX_LATCH_SEQ_LEN];
	uint8_t nSymbols;
	uint8_t nLatches;
	uint8_t latchSeqLen;
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
		nPulses = 0;
		messageBits = 0;
		nSymbols = 0;
		nLatches = 0;
		latchSeqLen = 0;
		nMessageRepeats = 0;
		coding = CODING_UNKNOWN;
		matchingRepeats = false;
	}

	inline void toEEPROM(void* eeprom_addr)
	{
		setValid(true);
		eeprom_write_block( (void*)this, eeprom_addr, sizeof(RFSnifferProtocol) );
	}

	inline void fromEEPROM(const void* eeprom_addr)
	{
		eeprom_read_block( (void*)this, eeprom_addr, sizeof(RFSnifferProtocol) );
		if( ! isValid() )
		{
			init();
		}
	}

	inline void setValid(bool v) { magic = v ? EEPROM_MAGIC_NUMBER : 0xFFFFFFFF; }
	inline bool isValid() { return magic == EEPROM_MAGIC_NUMBER; }

};

#endif
