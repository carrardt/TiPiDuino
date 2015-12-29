#ifndef __RFSnifferEEPROM_h
#define __RFSnifferEEPROM_h

#include "RFSnifferProtocol.h"
#include "ByteStream.h"
#include <avr/eeprom.h>
#include "AvrTLEEPROM.h"


namespace RFSnifferEEPROM
{
// EEPROM address where to write detected protocol
static constexpr uint16_t EEPROM_MAGIC_NUMBER = 0; //((uint16_t)(BUILD_TIMESTAMP & 0xFFFF));

static constexpr uint8_t EEPROM_MAX_PROTOCOLS = 4;
static constexpr uint8_t PROGRAM_PROTOCOL_ID = 0xFF;
static constexpr uint8_t EMPTY_BOOT_PROGRAM_ID = 0xFF;
static constexpr uint8_t LEARN_NEW_PROTOCOL = 0xFF;
static constexpr uint8_t RECORD_PROTOCOL_0  = 0x00;
static constexpr uint8_t RECORD_PROTOCOL_1  = 0x01;
static constexpr uint8_t RECORD_PROTOCOL_2  = 0x02;
static constexpr uint8_t RECORD_PROTOCOL_3  = 0x03;
static constexpr uint8_t COMMAND_MODE       = 0x10;
static constexpr uint8_t DEFAULT_OPERATING_MODE = COMMAND_MODE;

static constexpr uint8_t* EEPROM_MAGIC_ADDR		= ((uint8_t*)0x0000);		// 2 bytes
static constexpr uint8_t* EEPROM_INITPROG_ADDR 	= (EEPROM_MAGIC_ADDR+2);	// 1 byte
static constexpr uint8_t* EEPROM_FLAGS_ADDR 	= (EEPROM_INITPROG_ADDR+1);	// 1 byte
static constexpr uint8_t* EEPROM_OPERATION_ADDR	= (EEPROM_FLAGS_ADDR+1);	// 1 byte
static constexpr uint8_t* EEPROM_PROTOCOLS_ADDR = (EEPROM_OPERATION_ADDR+1);	// 4 * sizeof(RFSnifferProtocol)
static constexpr uint8_t* EEPROM_CODES_ADDR		= (EEPROM_PROTOCOLS_ADDR+EEPROM_MAX_PROTOCOLS*sizeof(RFSnifferProtocol));

struct MessageInfo
{
	uint8_t nbytes;
	uint8_t protocolId;
	uint8_t* eeprom_addr;
	inline MessageInfo(): nbytes(0) {}
};

struct EEPROMStream : public BufferStream
{
	inline EEPROMStream(uint8_t* p, uint16_t s) : BufferStream(p,s) {}
	inline EEPROMStream(const MessageInfo& mi) : BufferStream(mi.eeprom_addr,mi.nbytes) {}
	
	virtual uint8_t readByte()
	{
		if( m_pos >= m_size ) { return 0; }
		return eeprom_read_byte( m_buf + (m_pos++) );
	}

	virtual bool writeByte( uint8_t x )
	{
		if( m_pos >= m_size ) { return false; }
		avrtl::eeprom_gently_write_byte( m_buf + (m_pos++) , x );
		return true;
	}
};

void resetEEPROM();
void initEEPROM();
int findRecordedMessage(int pId, const uint8_t* buf, int nbytes);
int saveProtocol(const RFSnifferProtocol& proto);
void removeProtocol(int pId);
int appendMessage(int pId, ByteStream* stream);
int saveMessage(int pId, uint8_t* buf, int nbytes);
MessageInfo getMessageInfo(int mId);
uint8_t getOperationMode();
void setOperationMode(uint8_t mode);
void setBootProgram(uint8_t mesgId);
uint8_t getBootProgram();

static inline RFSnifferProtocol readProtocol(int pId)
{
	RFSnifferProtocol p;
	avrtl::eeprom_read(p,EEPROM_PROTOCOLS_ADDR+pId*sizeof(RFSnifferProtocol));
	if( !p.isValid() ) { p.init(); }
	return p;
}

template<typename FuncT>
static inline void forEachProtocolInEEPROM( FuncT afunc )
{
	for(int i=0;i<EEPROM_MAX_PROTOCOLS;i++)
	{
		RFSnifferProtocol p = readProtocol(i);
		afunc (i,p);
	}
}

template<typename FuncT>
static inline void forEachMessageInEEPROM( FuncT afunc )
{
	uint8_t* _ptr = EEPROM_CODES_ADDR;
	int _mesgIdx = 0;
	while( eeprom_read_byte(_ptr) != 0 )
	{
		int _len = eeprom_read_byte(_ptr++);
		int _protId = eeprom_read_byte(_ptr++);
		afunc( _mesgIdx, _protId, _len, _ptr );
		_ptr += _len;
		++ _mesgIdx;
	}
}

static inline uint16_t getMessageCount()
{
	int mId = -1;
	forEachMessageInEEPROM( [&](int m,int,int,uint8_t*) { mId=m; } );
	return mId+1;
}

static inline uint16_t getProtocolCount()
{
	int c = 0;
	forEachProtocolInEEPROM( [&](int, const RFSnifferProtocol& p) { if( p.isValid() ) ++c; } );
	return c;
}

static inline EEPROMStream getMessageStream(int mId)
{
	return EEPROMStream( getMessageInfo(mId) );
}


} // namespace RFSnifferEEPROM

#endif
