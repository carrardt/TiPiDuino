#ifndef __TIPIDUINO_PrintStream_h
#define __TIPIDUINO_PrintStream_h

static constexpr const char* endl = "\n";

template<class StreamT>
struct PrintStream
{
	PrintStream(StreamT& _s) : stream(_s) {}
	
	template<typename T>
	PrintStream& operator << ( const T& x )
	{
		print( x );
		return *this;
	}
	
	template<typename T>
	void println( const T& x )
	{
		print(x);
		print(endl);
	}

	void print( char x )
	{ 
		stream.writeChar( x ); 
	}
	
	void print( const char* s )
	{
		while(*s != '\0') print( *(s++) );
	}

	void print( void* s )
	{
		uint32_t addr = (uint32_t)s;
		print(addr,16,4);
	}
		
	void print(unsigned long x, int base=10, int ndigits=0) { print((long)x,base,ndigits); }
	void print(unsigned int x, int base=10, int ndigits=0) { print((long)x,base,ndigits); }
	void print(int x, int base=10, int ndigits=0) { print((long)x,base,ndigits); }
	void print(long x, int div=10, int ndigits=0)
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
	
	StreamT& stream;
};


#endif
