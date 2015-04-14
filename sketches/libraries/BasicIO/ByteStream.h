#ifndef __TIPIDUINO_ByteStream_h
#define __TIPIDUINO_ByteStream_h

#include <stdint.h>

struct ByteStream
{
	virtual bool writeChar( char c ) { return writeByte((uint8_t)c); }
	virtual bool writeByte( uint8_t x ) { return false; }
	virtual char readChar() { return (char)readByte(); }
	virtual uint8_t readByte() { return 0; }
	virtual bool eof() const { return false; }
};

struct BufferInputStream : public ByteStream
{
	inline BufferInputStream(const uint8_t* b, int s) : buf(b), size(s) {}
	virtual bool eof() const { return size <= 0; }
	virtual uint8_t readPtr( const uint8_t* p ) { return *p; }
	virtual uint8_t readByte()
	{
		if(size<=0) { return 0; }
		--size;
		return readPtr(buf++);
	}
private:
	const uint8_t* buf;
	int size;
};

#endif
