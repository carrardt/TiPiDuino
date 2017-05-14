#ifndef __TIPIDUINO_InputStream_h
#define __TIPIDUINO_InputStream_h

#include <BasicIO/ByteStream.h>

struct InputStream
{
	InputStream() : stream(0) {}
	
	void begin(ByteStream* _s)
	{
		stream = _s;
	}
	
	static bool isSpace(char x) { return x==' ' || x=='\t' || x=='\n' || x=='\r'; }
	static bool isDigit(char x) { return x>='0' && x<='9'; }
	static bool isHexDigit(char x) { return (x>='0' && x<='9') || (x>='A' && x<='F'); }
	
	char readFirstNonSpace()
	{
		char x = stream->readByte();
		while( isSpace(x) ) x = stream->readByte();
		return x;
	}
	
	InputStream& operator >> ( char& x )
	{
		x = stream->readByte();
		return *this;
	}
	
	InputStream& operator >> ( char* str )
	{
		char x = readFirstNonSpace();
		while( !isSpace(x) )
		{
			*(str++)=x;
			x=stream->readByte();
		}
		*str='\0';
		return *this;
	}

	int32_t readInteger(char x='\0')
	{
		int32_t n=0;
		char s='+';
		if( x == '\0' ) { x = readFirstNonSpace(); }
		if(x=='-' || x=='+')
		{ 
			s = x;
			x = stream->readByte();
		}
		if( isDigit(x) )
		{
			n = x-'0';
			x = stream->readByte();
			if( n==0 && x=='x' )
			{
				x = stream->readByte();
				while( isHexDigit(x) )
				{
					n *= 16;
					n += isDigit(x) ? (x-'0') : (10+x-'A');
					x = stream->readByte();
				}
			}
			else
			{
				while( isDigit(x) )
				{
					n *= 10;
					n += x-'0';
					x = stream->readByte();
				}
			}
		}
		if(s=='-') n = -n;
		return n;
	}

	InputStream& operator >> ( int32_t& n ) { n = readInteger(); return *this; }
	InputStream& operator >> ( uint32_t& n ) { n = readInteger(); return *this; }
	InputStream& operator >> ( int16_t& n ) { n = readInteger(); return *this; }
	InputStream& operator >> ( uint16_t& n ) { n = readInteger(); return *this; }
//	InputStream& operator >> ( int8_t& n ) { n = readDec32(); return *this; }
//	InputStream& operator >> ( uint8_t& n ) { n = readDec32(); return *this; }

	ByteStream* stream;
};


#endif
