#ifndef __TIPIDUINO_PrintStream_h
#define __TIPIDUINO_PrintStream_h

#include <BasicIO/ByteStream.h>
#include <AvrTL/AvrTL.h>

struct EndLineT {};
#define endl EndLineT()
//static constexpr EndLineT endl;

struct PrintStream
{
	PrintStream() : stream(0) {}

	void begin(ByteStream* _s)
	{
		stream = _s;
	}

	PrintStream& operator << ( const char& x ) { print( x ); return *this; }
	PrintStream& operator << ( const int16_t& x ) { print( x ); return *this; }
	PrintStream& operator << ( const uint16_t& x ) { print( x ); return *this; }
	PrintStream& operator << ( const int32_t& x ) { print( x ); return *this; }
	PrintStream& operator << ( const uint32_t& x ) { print( x ); return *this; }
	PrintStream& operator << ( const char* x ) { print( x ); return *this; }
	PrintStream& operator << ( const EndLineT& x ) { print( x ); return *this; }

	template<typename T>
	void println( const T& x )
	{
		print(x);
		print(endl);
	}


	void print( char x )
	{ 
		if( stream != 0 )
		{
			//if( x == '\n' ) { avrtl::DelayMicroseconds(300000); }
			stream->writeByte( x );
		}
	}
	
	void print( const char* s )
	{
		if(s==0) return;
		while( *s != '\0')
		{
			print( *s );
			++s;
		}
	}

	void print( const EndLineT& )
	{
		if( stream != 0 ) { print( stream->endline() ); }
	}
	
	void print( void* s )
	{
		uint32_t addr = (uint32_t)s;
		print(addr,16,4);
	}

	void printStreamHex(ByteStream* byteSream)
	{
		while( ! byteSream->eof() )
		{
			print( (uint16_t) byteSream->readByte(), 16, 2 );
		}
	}

	void print(uint32_t x, int base=10, int ndigits=0) { print((int32_t)x,base,ndigits); }
	void print(uint16_t x, int base=10, int ndigits=0) { print((int32_t)x,base,ndigits); }
	void print(int16_t x, int base=10, int ndigits=0) { print((int32_t)x,base,ndigits); }
	void print(int32_t x, int div=10, int ndigits=0)
	{
		if(div<2) return;
		char digits[32];
		/*
		if(div==16) print("0x");
		else if(div==2) print("0b");
		else if(div==8) print("0");
		*/
		int n = 0;
		if( x < 0 ) { print((char)'-'); x=-x; }
		do
		{
			digits[n++] = x % div;
			x /= div;
		} while( x > 0 );
		for(int i=0;i<(ndigits-n);i++) print('0');
		for(int i=0;i<n;i++)
		{
			int dg = digits[n-i-1];
			print ( (dg<10) ? (char)('0'+dg) : (char)('A'+(dg-10)) );
		}
	}
	
	ByteStream* stream;
};

//extern PrintStream dbgout;

#endif
