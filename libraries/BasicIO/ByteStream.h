#ifndef __TIPIDUINO_ByteStream_h
#define __TIPIDUINO_ByteStream_h

#include <stdint.h>
#include <AvrTL/AvrTLSignal.h>

struct ByteStream
{
	virtual bool writeByte( uint8_t x ) { return false; }
	virtual uint8_t readByte() { return 0; }
	virtual bool eof() { return false; }
	virtual bool rewind() { return false; }
	virtual int16_t available() { return -1; }
	virtual const char* endline() const { return "\n"; }

	inline void copy( ByteStream* from )
	{
		while( !eof() && !from->eof() )
		{
			writeByte( from->readByte() );
		}
	}
};

struct TeeStream : public ByteStream
{
	inline TeeStream() : s1(0), s2(0) {}
	inline void begin(ByteStream* _s1, ByteStream* _s2) { s1=_s1; s2=_s2; }
	virtual bool writeByte( uint8_t x ) { return s1->writeByte(x) && s2->writeByte(x); }
	virtual uint8_t readByte() { return s1->readByte(); }
	virtual bool eof() { return s1->eof() ; }
	virtual bool rewind() { return s1->rewind(); }
	virtual int16_t available() { return s1->available(); }
	virtual const char* endline() const { return s1->endline(); }
	
	ByteStream* s1;
	ByteStream* s2;
};

struct BufferStream : public ByteStream
{
	inline BufferStream(uint8_t* b, uint16_t s) : m_buf(b), m_pos(0), m_size(s) {}

	virtual bool eof() { return m_pos>=m_size; }
	virtual bool rewind() { m_pos=0; return true; }
	virtual int16_t available() { return m_size-m_pos; }

	virtual uint8_t readByte()
	{
		if( eof() ) { return 0; }
		return m_buf[m_pos++];
	}
	virtual bool writeByte( uint8_t x )
	{
		if( eof() ) { return false; }
		m_buf[m_pos++] = x;
		return true;
	}

protected:
	uint8_t* m_buf;
	uint16_t m_pos;
	uint16_t m_size;
};

template<typename RawIOType,uint32_t _EndLineDelay=10000UL>
struct ByteStreamAdapter : public ByteStream
{
	static constexpr uint32_t EndLineDelay = _EndLineDelay;

	inline ByteStreamAdapter() {}
	inline ByteStreamAdapter(RawIOType io) : m_rawIO(io) {}
	inline ByteStreamAdapter(const ByteStreamAdapter& bsa) : m_rawIO(bsa.m_rawIO) {}

	void setEndLine(const char* el) { m_endl=el; }
	const char* endline() const override final { return m_endl; }
	bool writeByte( uint8_t x ) override final 
	{
		bool r = m_rawIO.writeByte(x);
		if( r && x=='\n' ) { avrtl::DelayMicroseconds(EndLineDelay); }
		return r;
	}
	uint8_t readByte() override final { return m_rawIO.readByte(); }

	RawIOType m_rawIO;
	const char* m_endl = "\n";
};

#endif
