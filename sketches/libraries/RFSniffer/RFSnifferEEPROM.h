#ifndef __RFSnifferEEPROM_h
#define __RFSnifferEEPROM_h

#include "RFSnifferProtocol.h"
#include "ByteStream.h"
#include <avr/eeprom.h>

namespace RFSnifferEEPROM
{

// EEPROM address where to write detected protocol
static constexpr uint16_t EEPROM_MAGIC_NUMBER = 0; //((uint16_t)(BUILD_TIMESTAMP & 0xFFFF));
static constexpr uint8_t EEPROM_MAX_PROTOCOLS = 4;

static constexpr uint8_t* EEPROM_MAGIC_ADDR		= ((uint8_t*)0x0000);		// 2 bytes
static constexpr uint8_t* EEPROM_FLAGS_ADDR 	= (EEPROM_MAGIC_ADDR+2);	// 1 byte
static constexpr uint8_t* EEPROM_OPERATION_ADDR	= (EEPROM_FLAGS_ADDR+1);	// 1 byte
static constexpr uint8_t* EEPROM_PROTOCOLS_ADDR = (EEPROM_OPERATION_ADDR+1);	// 4 * sizeof(RFSnifferProtocol)
static constexpr uint8_t* EEPROM_CODES_ADDR		= (EEPROM_PROTOCOLS_ADDR+EEPROM_MAX_PROTOCOLS*sizeof(RFSnifferProtocol));

static constexpr uint8_t LEARN_NEW_PROTOCOL = 0xFF;
static constexpr uint8_t RECORD_PROTOCOL_0  = 0x00;
static constexpr uint8_t RECORD_PROTOCOL_1  = 0x01;
static constexpr uint8_t RECORD_PROTOCOL_2  = 0x02;
static constexpr uint8_t RECORD_PROTOCOL_3  = 0x03;
static constexpr uint8_t COMMAND_MODE       = 0x10;
static constexpr uint8_t STANDALONE_MODE    = 0x20;

template<typename FuncT>
static inline void forEachProtocolInEEPROM( FuncT afunc )
{
	uint8_t* protPtr = EEPROM_PROTOCOLS_ADDR;
	for(int i=0;i<EEPROM_MAX_PROTOCOLS;i++)
	{
		RFSnifferProtocol p;
		p.fromEEPROM( protPtr );
		afunc (i,p);
		protPtr += sizeof(RFSnifferProtocol);
	}
}

template<typename FuncT>
static inline void forEachMessageInEEPROM( FuncT afunc )
{
	uint8_t* ptr = EEPROM_CODES_ADDR;
	int mesgIdx = 0;
	while( eeprom_read_byte(ptr) != 0 )
	{
		int len = eeprom_read_byte(ptr++);
		int protId = eeprom_read_byte(ptr++);
		afunc( mesgIdx, protId, len, ptr );
		ptr += len;
		++ mesgIdx;
	}
}

static inline RFSnifferProtocol readProtocol(int pId)
{
	RFSnifferProtocol sp;
	sp.fromEEPROM(EEPROM_PROTOCOLS_ADDR+pId*sizeof(RFSnifferProtocol));
	return sp;
}

static inline uint8_t getOperationMode()
{
	return eeprom_read_byte(EEPROM_OPERATION_ADDR);
}

static inline void setOperationMode(uint8_t mode)
{
	eeprom_write_byte(EEPROM_OPERATION_ADDR,mode);
}

struct MessageInfo
{
	uint8_t nbytes;
	uint8_t protocolId;
	uint8_t* eeprom_addr;
	inline MessageInfo(): nbytes(0) {}
};

struct EEPROMInputStream : public BufferInputStream
{
	inline EEPROMInputStream(const uint8_t* b, int s) : BufferInputStream(b,s) {}
	virtual uint8_t readPtr( const uint8_t* p ) { return eeprom_read_byte(p); }
};

void initEEPROM();
int findRecordedMessage(int pId, const uint8_t* buf, int nbytes);
int saveProtocol(const RFSnifferProtocol& proto);
int appendMessage(int pId, ByteStream* stream, int nbytes);
int saveMessage(int pId, const uint8_t* buf, int nbytes);
MessageInfo getMessageInfo(int mId);

static inline EEPROMInputStream getMessageStream(int mId)
{
	MessageInfo info = getMessageInfo(mId);
	return EEPROMInputStream(info.eeprom_addr,info.nbytes);
}

}

#endif
