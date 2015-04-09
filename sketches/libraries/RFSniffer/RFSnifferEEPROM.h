#ifndef __RFSnifferEEPROM_h
#define __RFSnifferEEPROM_h

#include "RFSnifferProtocol.h"

// EEPROM address where to write detected protocol
#define EEPROM_MAGIC_NUMBER 	((uint16_t)(BUILD_TIMESTAMP & 0xFFFF))
#define EEPROM_MAX_PROTOCOLS	4

#define EEPROM_MAGIC_ADDR		((uint8_t*)0x0000)		// 2 bytes
#define EEPROM_FLAGS_ADDR 		(EEPROM_MAGIC_ADDR+2)	// 1 byte
#define EEPROM_BEHAVIOR_ADDR	(EEPROM_FLAGS_ADDR+1)	// 1 byte
#define EEPROM_PROTOCOLS_ADDR 	(EEPROM_BEHAVIOR_ADDR+1)	// 4 * sizeof(RFSnifferProtocol)
#define EEPROM_CODES_ADDR		(EEPROM_PROTOCOLS_ADDR+EEPROM_MAX_PROTOCOLS*sizeof(RFSnifferProtocol))

#define LEARN_NEW_PROTOCOL		0xFF
#define RECORD_PROTOCOL_0		0x00
#define RECORD_PROTOCOL_1		0x01
#define RECORD_PROTOCOL_2		0x02
#define RECORD_PROTOCOL_3		0x03
#define INTERACTIVE_MODE		0x10
#define STANDALONE_MODE			0x20

template<typename FuncT>
static void forEachProtocolInEEPROM( FuncT afunc )
{
	uint8_t* protPtr = EEPROM_PROTOCOLS_ADDR;
	for(int i=0;i<EEPROM_MAX_PROTOCOLS;i++)
	{
		RFSnifferProtocol p;
		p.fromEEPROM( protPtr );
		if( p.isValid() ) { afunc (i,p); }
		protPtr += sizeof(RFSnifferProtocol);
	}
}

template<typename FuncT>
static void forEachMessageInEEPROM( FuncT afunc )
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
	if( mesgFoundAt != -1 && )
	return mesgFoundAt;
}

void initEEPROM();
int findRecordedMessage(int pId, const uint8_t* buf, int nbytes);
int saveProtocolToEEPROM(RFSnifferProtocol& proto);
int saveMessageToEEPROM(int pId, const uint8_t* buf, int nbytes);

#endif
