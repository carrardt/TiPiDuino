#ifndef __TIPIDUINO_ByteStream_h
#define __TIPIDUINO_ByteStream_h

#include <stdint.h>

struct ByteStream
{
	virtual bool writeChar( char c ) { return writeByte((uint8_t)c); }
	virtual bool writeByte( uint8_t x ) { return false; }
	virtual char readChar() { return (char)readByte(); }
	virtual uint8_t readByte() { return 0; }
};

struct BufferInputStream : public ByteStream
{
	inline BufferInputStream(const char* b, int s) : buf(b), size(s), cur(0) {}
	inline bool eof() const { return cur >= size; }
	virtual uint8_t readByte() { return (cur<size) ? buf[cur++] : 0; }
	
private:
	const uint8_t* buf;
	int size;
	int cur;
};

#endif
