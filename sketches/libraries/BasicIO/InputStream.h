#ifndef __TIPIDUINO_InputStream_h
#define __TIPIDUINO_InputStream_h

#include <ByteStream.h>

struct InputStream
{
	InputStream() : stream(0) {}
	
	void begin(ByteStream* _s)
	{
		stream = _s;
	}
	
	static bool isSpace(char x) { return x==' ' || x=='\t' || x=='\n' || x=='\r'; }
	static bool isDigit(char x) { return x>='0' && x<='9'; }
	
	char readFirstNonSpace()
	{
		char x = stream->readChar();
		while( isSpace(x) ) x = stream->readChar();
		return x;
	}
	
	InputStream& operator >> ( char& x )
	{
		x = stream->readChar();
		return *this;
	}
	
	InputStream& operator >> ( char* str )
	{
		char x = readFirstNonSpace();
		while( !isSpace(x) )
		{
			*(str++)=x;
			x=stream->readChar();
		}
		return *this;
	}

	int32_t readDec32()
	{
		int32_t n=0;
		char s='+';
		char x = readFirstNonSpace();
		if(x=='-' || x=='+')
		{ 
			s = x;
			x = stream->readChar();
		}
		while( isDigit(x) )
		{
			n *= 10;
			n += x-'0';
			x = stream->readChar();
		}
		if(s=='-') n = -n;
		return n;
	}

	InputStream& operator >> ( int32_t& n ) { n = readDec32(); return *this; }
	InputStream& operator >> ( uint32_t& n ) { n = readDec32(); return *this; }
	InputStream& operator >> ( int16_t& n ) { n = readDec32(); return *this; }
	InputStream& operator >> ( uint16_t& n ) { n = readDec32(); return *this; }
//	InputStream& operator >> ( int8_t& n ) { n = readDec32(); return *this; }
//	InputStream& operator >> ( uint8_t& n ) { n = readDec32(); return *this; }

	ByteStream* stream;
};


#endif
