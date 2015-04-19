#ifndef __TIPIDUINO_ByteStream_h
#define __TIPIDUINO_ByteStream_h

#include <stdint.h>

struct ByteStream
{
	virtual bool writeByte( uint8_t x ) { return false; }
	virtual uint8_t readByte() { return 0; }
	virtual bool eof() { return false; }
	virtual bool rewind() { return false; }
	virtual int16_t available() { return -1; }
	
	inline void copy( ByteStream* from )
	{
		while( !eof() && !from->eof() )
		{
			writeByte( from->readByte() );
		}
	}
};

struct BufferStream : public ByteStream
{
	inline BufferStream(uint8_t* b, uint16_t s) : m_buf(b), m_pos(0), m_size(s) {}

	virtual bool eof() { return m_pos>=m_size; }
	virtual bool rewind() { m_pos=0; return true; }
	virtual int16_t available() { return m_size-m_pos; }

	virtual uint8_t readByte()
	{
		if( m_pos>=m_size ) { return 0; }
		return m_buf[m_pos++];
	}
	virtual bool writeByte( uint8_t x )
	{
		if( m_pos>=m_size ) { return false; }
		m_buf[m_pos++] = x;
		return true;
	}

protected:
	uint8_t* m_buf;
	uint16_t m_pos;
	uint16_t m_size;
};

#endif
