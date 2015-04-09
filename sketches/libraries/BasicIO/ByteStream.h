#ifndef __TIPIDUINO_ByteStream_h
#define __TIPIDUINO_ByteStream_h

#include <stdint.h>

struct ByteStream
{
	virtual bool writeChar( char c ) { return writeByte((uint8_t)c); }
	virtual bool writeByte( uint8_t x ) { return false; }
	virtual char readChar() { return (char)readByte(); }
	virtual uint8_t readByte() { return '@'; }
};

#endif
