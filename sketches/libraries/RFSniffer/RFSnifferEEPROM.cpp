#include "RFSnifferEEPROM.h"
#include "RFSnifferProtocol.h"
#include <avr/eeprom.h>
#include "AvrTL.h"
#include "PrintStream.h" // for dbgout

//TODO: doit aussi initialiser le flag par defaut pour le prochain apprentissage
// detecter s'il faut nettoyer l'EEPROM et dans quel mode on démarre
namespace RFSnifferEEPROM
{

// reset EEPROM state
void resetEEPROM()
{
	uint16_t magic = EEPROM_MAGIC_NUMBER;
	avrtl::eeprom_gently_write_block((uint8_t*)&magic,EEPROM_MAGIC_ADDR,sizeof(magic));
	setOperationMode( COMMAND_MODE );
	setBootProgram( EMPTY_BOOT_PROGRAM_ID );
	for(int i=0;i<EEPROM_MAX_PROTOCOLS;i++)
	{
		RFSnifferProtocol sp;
		sp.setValid(false);
		avrtl::eeprom_gently_write(sp,EEPROM_PROTOCOLS_ADDR+i*sizeof(RFSnifferProtocol));
	}
	avrtl::eeprom_gently_write_byte(EEPROM_CODES_ADDR,0);
}

void initEEPROM()
{
	uint16_t magic = 0;
	eeprom_read_block(&magic,EEPROM_MAGIC_ADDR,sizeof(magic));
	if( EEPROM_MAGIC_NUMBER!=0 && magic!=EEPROM_MAGIC_NUMBER )
	{
		resetEEPROM();
	}
	else
	{
		RFSnifferProtocol::defaultFlags = eeprom_read_byte(EEPROM_FLAGS_ADDR);
		//dbgout << "EPROM ok, flags="<<(int)RFSnifferProtocol::defaultFlags<<"\n";
	}
	if( getOperationMode() == LEARN_NEW_PROTOCOL )
	{
		avrtl::eeprom_gently_write_byte(EEPROM_FLAGS_ADDR, (RFSnifferProtocol::defaultFlags+1) & RFSnifferProtocol::RESET_MODIFIY_FLAGS_MASK );
	}
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
		} );
	return mesgFoundIdx;
}

int saveProtocol(const RFSnifferProtocol& proto)
{
	int i = -1;
	forEachProtocolInEEPROM( [&](int pId, const RFSnifferProtocol& p) { if(i==-1 && !p.isValid()) i=pId; } );
	if( i != -1 )
	{
		avrtl::eeprom_gently_write(proto,EEPROM_PROTOCOLS_ADDR+i*sizeof(RFSnifferProtocol));
	}
	return i;
}

// Warning, stream has to be reliable and patient (not byte droping if reading to slowly)
int appendMessage(int pId, ByteStream* stream)
{
	uint8_t* nextMsgAddr = EEPROM_CODES_ADDR;
	int mId = -1;
	forEachMessageInEEPROM( [&](int mesgIdx, int protocolIdx, int len, uint8_t* ptr) {
		nextMsgAddr = ptr + len;
		mId = mesgIdx;
	});
	++mId;
	int16_t nbytes = stream->available();
	if(nbytes<0 || nbytes>255) { return -1; }
	// we write the message + 3 bytes: length, protocolId, <MESSAGE>, 0
	EEPROMStream es(nextMsgAddr, nbytes+3); 
	es.writeByte( nbytes );
	es.writeByte( pId );
	es.copy( stream );
	es.writeByte( 0 );
	return mId;
}

int saveMessage(int pId, uint8_t* buf, int nbytes)
{
	int mId = findRecordedMessage(pId,buf,nbytes);
	if( mId!=-1) return mId;
	BufferStream stream(buf,nbytes);
	return appendMessage(pId,&stream);
}

MessageInfo getMessageInfo(int mId)
{
	MessageInfo info;
	forEachMessageInEEPROM( [&](int mesgIdx, int protocolIdx, int len, uint8_t* ptr) {
		if( mId == mesgIdx )
		{
			info.nbytes = len;
			info.protocolId = protocolIdx;
			info.eeprom_addr = ptr;
		}
	});
	return info;
}

uint8_t getOperationMode()
{
	return eeprom_read_byte(EEPROM_OPERATION_ADDR);
}

void setOperationMode(uint8_t mode)
{
	avrtl::eeprom_gently_write_byte(EEPROM_OPERATION_ADDR,mode);
}

uint8_t getBootProgram()
{
	return eeprom_read_byte(EEPROM_INITPROG_ADDR);
}

void setBootProgram(uint8_t mesgId)
{
	avrtl::eeprom_gently_write_byte(EEPROM_INITPROG_ADDR,mesgId);
}


} // namespace RFSnifferEEPROM
