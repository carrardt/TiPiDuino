#include "RFSnifferEEPROM.h"
#include <avr/eeprom.h>

//TODO: doit aussi initialiser le flag par defaut pour le prochain apprentissage
// detecter s'il faut nettoyer l'EEPROM et dans quel mode on démarre

void initEEPROM()
{
	uint16_t magic=0;
	eeprom_read_block(&magic,EEPROM_MAGIC_ADDR);
	if( magic != EEPROM_MAGIC_NUMBER )
	{
		avrtl::eeprom_gently_write_block(&magic,EEPROM_MAGIC_ADDR);
		avrtl::eeprom_gently_write_block(EEPROM_BEHAVIOR_ADDR,LEARN_NEW_PROTOCOL);
		for(int i=0;i<EEPROM_MAX_PROTOCOLS;i++)
		{
			RFSnifferProtocol sp;
			sp.toEEPROM( EEPROM_PROTOCOLS_ADDR+i*sizeof(RFSnifferProtocol) );
		}
		avrtl::eeprom_gently_write_block(EEPROM_CODES_ADDR,0);
	}
	else
	{
		RFSnifferProtocol::defaultFlags = (eeprom_read_byte(EEPROM_FLAGS_ADDR)+1) & RFSnifferProtocol::RESET_MODIFIY_FLAGS_MASK;
	}
	avrtl::eeprom_gently_write_block(EEPROM_FLAGS_ADDR,RFSnifferProtocol::defaultFlags);
}

int findRecordedMessage(int pId, const uint8_t* buf, int nbytes)
{
	int mesgFoundIdx = -1;
	forEachMessageInEEPROM( [&](int mesgIdx, int protocolIdx, int len, uint8_t* ptr)
		{
			if( pId==protocolIdx && len==nbytes )
			{
				int i=0;
				while(i<len && eeprom_read_byte(ptr+i)==buf[i]) ++i;
				if(i==len) mesgFoundIdx=mesgIdx;
			}
		}
		);
	return mesgFoundIdx;
}

int saveProtocolToEEPROM(RFSnifferProtocol& proto)
{
	int i = -1;
	forEachProtocolInEEPROM( [&](int pId, const RFSnifferProtocol& p) { if(i==-1 && !p.isValid()) i=pId; } );
	if( i != -1 )
	{
		proto.setValid(true);
		proto.toEEPROM( EEPROM_PROTOCOLS_ADDR+i*sizeof(RFSnifferProtocol) );
	}
	return i;
}

int saveMessageToEEPROM(int pId, const uint8_t* buf, int nbytes)
{
	int mId = findRecordedMessage(pId,buf,nbytes);
	if( mId!=-1) return mId;
	uint8_t* nextMsgAddr = EEPROM_CODES_ADDR;
	forEachMessageInEEPROM( [&](int mesgIdx, int protocolIdx, int len, uint8_t* ptr) {
		nextMsgAddr = ptr + len;
	});
	eeprom_gently_write_byte( nextMsgAddr++, nbytes );
	eeprom_gently_write_byte( nextMsgAddr++, pId );
	for(int i=0;i<nbytes;i++) eeprom_gently_write_byte( nextMsgAddr++, buf[i] );
	eeprom_gently_write_byte( nextMsgAddr, 0 );
}
