#include "RFSnifferEEPROM.h"
#include "RFSnifferProtocol.h"
#include <avr/eeprom.h>
#include "AvrTL.h"
#include "PrintStream.h" // for dbgout

//TODO: doit aussi initialiser le flag par defaut pour le prochain apprentissage
// detecter s'il faut nettoyer l'EEPROM et dans quel mode on démarre
namespace RFSnifferEEPROM
{

void initEEPROM()
{
	uint16_t magic = 0;
	eeprom_read_block(&magic,EEPROM_MAGIC_ADDR,sizeof(magic));
//	dbgout << "eeprom=" << (int)magic<<"\n";
//	dbgout << "magic="<< (int)EEPROM_MAGIC_ADDR<<"\n";
	if( magic != EEPROM_MAGIC_NUMBER )
	{
		//dbgout << "bad magic\nreset eeprom\n";
		magic = EEPROM_MAGIC_NUMBER;
		avrtl::eeprom_gently_write_block((uint8_t*)&magic,EEPROM_MAGIC_ADDR,sizeof(magic));
		setOperationMode(LEARN_NEW_PROTOCOL);
		for(int i=0;i<EEPROM_MAX_PROTOCOLS;i++)
		{
			RFSnifferProtocol sp;
			sp.setValid(false);
			sp.toEEPROM( EEPROM_PROTOCOLS_ADDR+i*sizeof(RFSnifferProtocol) );
		}
		avrtl::eeprom_gently_write_byte(EEPROM_CODES_ADDR,0);
	}
	else
	{
		RFSnifferProtocol::defaultFlags = eeprom_read_byte(EEPROM_FLAGS_ADDR);
		//dbgout << "EPROM ok, flags="<<(int)RFSnifferProtocol::defaultFlags<<"\n";
	}
	avrtl::eeprom_gently_write_byte(EEPROM_FLAGS_ADDR, (RFSnifferProtocol::defaultFlags+1) & RFSnifferProtocol::RESET_MODIFIY_FLAGS_MASK );
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

void readProtocol(int pId,RFSnifferProtocol& sp)
{
	sp.fromEEPROM(EEPROM_PROTOCOLS_ADDR+pId*sizeof(RFSnifferProtocol));
}

int saveProtocol(const RFSnifferProtocol& proto)
{
	int i = -1;
	forEachProtocolInEEPROM( [&](int pId, const RFSnifferProtocol& p) { if(i==-1 && !p.isValid()) i=pId; } );
	if( i != -1 )
	{
		proto.toEEPROM( EEPROM_PROTOCOLS_ADDR+i*sizeof(RFSnifferProtocol) );
	}
	return i;
}

int saveMessage(int pId, const uint8_t* buf, int nbytes)
{
	int mId = findRecordedMessage(pId,buf,nbytes);
	if( mId!=-1) return mId;
	uint8_t* nextMsgAddr = EEPROM_CODES_ADDR;
	forEachMessageInEEPROM( [&](int mesgIdx, int protocolIdx, int len, uint8_t* ptr) {
		nextMsgAddr = ptr + len;
		mId = mesgIdx;
	});
	++mId;
	avrtl::eeprom_gently_write_byte( nextMsgAddr++, nbytes );
	avrtl::eeprom_gently_write_byte( nextMsgAddr++, pId );
	for(int i=0;i<nbytes;i++) avrtl::eeprom_gently_write_byte( nextMsgAddr++, buf[i] );
	avrtl::eeprom_gently_write_byte( nextMsgAddr, 0 );
	return mId;
}

uint8_t getOperationMode()
{
	return eeprom_read_byte(EEPROM_OPERATION_ADDR);
}

void setOperationMode(uint8_t mode)
{
	eeprom_write_byte(EEPROM_OPERATION_ADDR,mode);
}

}
