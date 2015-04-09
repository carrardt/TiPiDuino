#include "RFSnifferProtocol.h"

uint8_t RFSnifferProtocol::defaultFlags = 0;

RFSnifferProtocol::RFSnifferProtocol()
{
	init();
}

void RFSnifferProtocol::init()
{
	messageBits = 0;
	latchSeqLen = 0;
	bitSymbols[0] = 0;
	bitSymbols[1] = 0;
	for(int i=0;i<MAX_LATCH_SEQ_LEN;i++) latchSeq[i]=0;
	nMessageRepeats = 0;
	coding = CODING_UNKNOWN;
	flags = 0;
}

bool RFSnifferProtocol::mediumRF() const 
{
	return (flags&RF_FLAG)!=0;
}
void RFSnifferProtocol::setMediumRF(bool value)
{
	if(value) { flags |= RF_FLAG; }
	else { flags &= ~RF_FLAG; }
}

bool RFSnifferProtocol::matchingRepeats() const
{
	return (flags&MATCHING_REPEATS_FLAG)!=0;
}
void RFSnifferProtocol::setMatchingRepeats(bool value)
{
	if(value) { flags |= MATCHING_REPEATS_FLAG; }
	else { flags &= ~MATCHING_REPEATS_FLAG; }
}

void RFSnifferProtocol::setPulseLevel(bool value)
{
	if(value) { flags |= HIGH_LEVEL_FLAG; }
	else { flags &= ~HIGH_LEVEL_FLAG; }
}

void RFSnifferProtocol::toEEPROM(void* eeprom_addr) const
{
	avrtl::eeprom_gently_write_block( (const uint8_t*)this, (uint8_t*)eeprom_addr, sizeof(RFSnifferProtocol) );
}

void RFSnifferProtocol::fromEEPROM(const void* eeprom_addr)
{
	eeprom_read_block( (void*)this, eeprom_addr, sizeof(RFSnifferProtocol) );
	if( ! isValid() )
	{
		init();
	}
}

void RFSnifferProtocol::setValid(bool v)
{
	if(v) flags |= VALID_FLAG;
	else flags &= ~VALID_FLAG;
}

bool RFSnifferProtocol::isValid()
{
	return (flags&VALID_FLAG)!=0 && bitSymbols[0]>0 && bitSymbols[1]>0;
}	
