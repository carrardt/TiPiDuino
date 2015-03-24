#ifndef __RFSNIFFER_H
#define __RFSNIFFER_H

#include "RFSnifferConstants.h"
#include "RFSnifferProtocol.h"

/*
 * TODO: improve pulse length encoding, using TIMER0PRESCALER value
 */

template<typename RXPinT>
struct RFSniffer
{
	RFSniffer(RXPinT& _rx) : rx(_rx) {}
		
	template<typename T>
	static inline T abs(T x) { return (x<0) ? -x : x ; }

	// detect SeqLen consecutive similar symbols (at most NSymbols different symbols)
	// buf size must be at least NSymbols
	template<uint8_t NSymbols, uint8_t SeqLen>
	inline uint8_t detectEntropyDrop(uint16_t* buf)
	{
		uint8_t nSymbolsInBuffer = 0;
		uint8_t nSymbolsRead = 0;
		while(nSymbolsInBuffer<NSymbols && nSymbolsRead<SeqLen)
		{
			long p = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
			if( p < MIN_PULSE_LEN ) return nSymbolsRead;
			uint8_t j=NSymbols;
			for(int s=0;s<nSymbolsInBuffer;++s)
			{
				uint16_t l = buf[s];
				uint16_t re = l / PULSE_ERR_RATIO;
				if( abs(p-l) <= re ) j=s;			
			}
			if(j==NSymbols)
			{
				buf[nSymbolsInBuffer++] = p;
			}
			++ nSymbolsRead;
		}
		while( nSymbolsRead < SeqLen )
		{
			long p = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
			if( p < MIN_PULSE_LEN ) return nSymbolsRead;
			uint8_t j = NSymbols;
			for(int s=0;s<NSymbols;++s)
			{
				uint16_t l = buf[s];
				uint16_t re = l / PULSE_ERR_RATIO;
				if( abs(p-l) <= re ) j=s;			
			}
			if(j==NSymbols) return nSymbolsRead;
			++ nSymbolsRead;
		}
		return nSymbolsRead;
	}

	// record a raw signal (succession of digital pulse lengthes)
	template<uint16_t bufSize, uint16_t minLatchLen, uint8_t minLatchCount>
	int recordSignalLatchDetect(uint16_t * buf)
	{
	  int n=0;
	  for(;n<minLatchCount;++n)
	  {
		  buf[n] = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
		  if( buf[n]<minLatchLen || buf[n]>=MAX_PULSE_LEN ) return n;
	  }
	  do {
		buf[n++] = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
	  }
	  while( buf[n-1]>MIN_PULSE_LEN && buf[n-1]<=MAX_PULSE_LEN && n<bufSize);
	  if(n<bufSize) { --n; }
	  return n;
	}

	template<uint16_t bufSize>
	int recordSignalLatchSequenceDetect(uint16_t * buf,const int nlacthes, const uint16_t* sequence)
	{
		for(uint8_t i=0;i<nlacthes;i++)
		{
			long p = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
			buf[i] = p;
			uint16_t l = sequence[i];
			uint16_t relerr = l / PULSE_ERR_RATIO;
			if( abs(p-l) > relerr ) return 0;
		}
		int n = nlacthes;
	    do
	    {
			buf[n++] = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
	    }
	    while( buf[n-1]>MIN_PULSE_LEN && buf[n-1]<=MAX_PULSE_LEN && n<bufSize);
		if(n<bufSize) { --n; }
	    return n;
	}

	static void rotateBufferLeft1(uint16_t * ptr, int n)
	{
		uint8_t first = ptr[0];
		for(int i=0;i<(n-1);i++)
		{
			ptr[i] = ptr[i+1];
		}
		ptr[n-1] = first;
	}
	
	static void rotateBufferLeft(uint16_t * ptr, int n, int disp)
	{
		for(int i=0;i<disp;i++)
		{
			rotateBufferLeft1(ptr,n);
		}
	}

	template<typename T>
	static int findOccurence(const T* pattern, int psize, const T* buf, int bsize)
	{
		int j=0;
		for(int i=0;i<bsize;i++)
		{
			if( buf[i]==pattern[j] ) ++j;
			else j=0;
			if( j == psize ) return i-psize+1;
		}
		return -1;
	}

	// record a raw signal (succession of digital pulse lengthes)
	template<uint16_t bufSize, uint16_t binarySeqLen>
	int recordSignalBinaryEntropyDetect(uint16_t * buf)
	{
		uint16_t fop0 = 0; // first occurence position
		uint16_t fop1 = 0; // first occurence position
		uint16_t curs = 0; // circular buffer cursor
		uint16_t re;
		int32_t l;

#define CURSOR_DIST(a,b) (((bufSize+b)-a)%bufSize)

		l = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
		if( l < MIN_PULSE_LEN ) return 0;
		buf[curs] = l;	
		curs=(curs+1)%bufSize;
	
		do
		{
			// whole buffer has been filled without finding another symbol
			if(curs==fop0) return 0;

			// read a pulse,
			l = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
			if( l < MIN_PULSE_LEN ) return 0;
			
			// store the pulse
			buf[curs] = l;
			fop1 = curs;
			curs=(curs+1)%bufSize;
			
			// and test if it is different from the first one
			re = l / PULSE_ERR_RATIO;
		} while( abs(l-buf[fop0]) <= re );

		// here, we have find a second symbol

		// now wait until we have a long enough binary sequence
		while( CURSOR_DIST(fop0,curs) < binarySeqLen )
		{
			l = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
			if( l < MIN_PULSE_LEN ) return 0;
			buf[curs] = l;
			re = l / PULSE_ERR_RATIO;
			bool equalsP0 = ( abs(l-buf[fop0]) <= re );
			bool equalsP1 = ( abs(l-buf[fop1]) <= re );
			
			// whenever we find a third symbol, we discard p0 and shift p1 to p0
			// the new symbol becomes p1
			if( !equalsP0 && !equalsP1 )
			{
				fop0 = fop1;
				fop1 = curs;
			}
			curs=(curs+1)%bufSize;
		}

		while( curs != fop0 )
		{
			l = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
			if( l < MIN_PULSE_LEN )
			{
				int recordSize = CURSOR_DIST(fop0,curs);
				rotateBufferLeft(buf,bufSize,fop0);
				return recordSize;
			}			
			buf[curs] = l;
			curs=(curs+1)%bufSize;
		}

		rotateBufferLeft(buf,bufSize,fop0);
		return bufSize;
	}

	int readBinaryMessage(const RFSnifferProtocol& sp, uint8_t* buf)
	{
		const uint16_t b0 = sp.bitSymbols[0];
		const uint16_t b1 = sp.bitSymbols[1];
		const uint16_t b0_tol = b0 / PULSE_ERR_RATIO;
		const uint16_t b1_tol = b1 / PULSE_ERR_RATIO;

		for(uint8_t i=0;i<sp.latchSeqLen;i++)
		{
			long p = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
			uint16_t l = sp.latchSeq[i];
			uint16_t relerr = l / PULSE_ERR_RATIO;
			if( abs(p-l) > relerr ) return 0;
		}

		int bitsToRead = sp.messageBits;
		if( sp.coding == CODING_MANCHESTER ) bitsToRead*=2;
		
		uint8_t byte=0;
		for(uint16_t j=0;j<bitsToRead;j++)
		{
			if(j%8==0) buf[byte]=0;
			long p = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
			uint8_t b = 0;
			if( abs(p-b1) <= b1_tol ) b = 1;
			else if( abs(p-b0) > b0_tol ) return j;
			buf[byte] <<= 1;
			buf[byte] |= b;
			if( (j%8)==7 ) { ++byte;  }
		}
		return bitsToRead;
	}

	static bool decodeManchester(uint8_t* buf, int nbits)
	{
		int j=0, k=0;
		uint8_t byte=0;
		uint8_t x[2];
		for(int i=0;i<nbits;i++)
		{
			int bpos = i%8;
			x[i%2] = ( buf[i/8] >> (7-bpos) ) & 1;
			if( i%2 == 1 )
			{
				if( x[0]==x[1] ) return false;
				byte = (byte << 1) | x[0];
				++k;
				if( k==8 )
				{
					buf[j++] = byte;
					k=0;
				}
			}
		}
		if( k!=0 ) buf[j++] = byte;
		return true;
	}

	// build a classification of pulse lengthes in a signal record
	static int classifySymbols(uint16_t * buf, int n, uint16_t * symbols, uint8_t * symcount)
	{
	  int nsymbols = 0;
	  uint16_t nextSym = 0;
	  for(int i=0;i<n;i++) { if(buf[i]>nextSym) nextSym=buf[i]; }
	  while( nextSym > 0 )
	  {
		long sym = nextSym;
		long symAvg = 0;
		symbols[nsymbols] = nextSym; // a symbol class is represented by the highest value of its class
		symcount[nsymbols] = 0;
		nextSym = 0;
		for(int i=0;i<n;i++)
		{
		  if( buf[i] >= MIN_PULSE_LEN ) // sample not classified yet
		  {
			long relerr = sym / PULSE_ERR_RATIO;
			if( abs(sym-buf[i]) <= relerr ) // sample belongs to current class
			{
			  symAvg += buf[i];
			  ++ symcount[nsymbols];
			  buf[i] = nsymbols;
			}
			else // sample not in current class, find the highest one
			{
			  if( buf[i] > nextSym ) { nextSym = buf[i]; }
			}
		  }
		}
		symbols[nsymbols] = symAvg / symcount[nsymbols];
		++ nsymbols;   
		if( nsymbols >= MAX_SYMBOLS ) return MAX_SYMBOLS;
	  }
	  return nsymbols;
	}

	// restrict to binary symbol coding schemes
	bool analyseSignal(uint16_t* buf, int np, RFSnifferProtocol& sp )
	{
		
		int si0=0, si1=1;
		int nSymbols=0;
		uint16_t symbols[MAX_SYMBOLS];
		{
			// TODO: 8-bits symbols counters may not be large enough
			uint8_t symcount[MAX_SYMBOLS];
			nSymbols = classifySymbols(buf,np,symbols,symcount);
			if( nSymbols < 2 ) return false;
			
			// to find bit symbols we look for the 2 most frequent symbols
			si0 = 0;
			for(int i=1;i<nSymbols;i++)
			{
				if( symcount[i] > symcount[si0] ) si0=i;
			}
			si1 = (si0+1)%nSymbols;
			for(int i=0;i<nSymbols;i++) if( i!=si0 && i!=si1)
			{
				if( symcount[i] > symcount[si1] ) si1=i;
			}
		}
		
		// assume the shortest pulse codes 0, the longest codes 1
		if( symbols[si0] > symbols[si1] ) { int t=si0; si0=si1; si1=t; }
		sp.bitSymbols[0] = symbols[si0];
		sp.bitSymbols[1] = symbols[si1];
		
		// non bit coding symbols are considered as latches (start/stop markers)		
		if( nSymbols == 2 ) // no latches, only bits
		{
			sp.latchSeqLen = 0;
			sp.nMessageRepeats = 1;
			sp.coding = CODING_UNKNOWN;
			sp.messageBits = np;
			sp.matchingRepeats = true;
		}
		else
		{
			// detect start latch sequence
			int lstart = 0;
			while( lstart<np && ( buf[lstart]==si0 || buf[lstart]==si1 ) ) ++ lstart;
			sp.latchSeqLen = 0;
			while( sp.latchSeqLen<MAX_LATCH_SEQ_LEN && (lstart+sp.latchSeqLen)<np && buf[lstart+sp.latchSeqLen]!=si0 && buf[lstart+sp.latchSeqLen]!=si1 )
			{
				sp.latchSeq[ sp.latchSeqLen ] = symbols[ buf[ lstart + sp.latchSeqLen ] ];
				++ sp.latchSeqLen;
			}
			
			// detect repeated message
			int fullMessageStart = -1;
			int fullMessageSize = 0;
			sp.nMessageRepeats = 1;
			if( sp.latchSeqLen >= 1 ) 
			{
				int bstart = lstart+sp.latchSeqLen;
				int bsize = np - bstart;
				int i=-1;
				while( ( i = findOccurence(buf+lstart,sp.latchSeqLen,buf+bstart,bsize) ) != -1 )
				{
					if( fullMessageStart == -1 )
					{
						fullMessageStart = bstart;
						fullMessageSize = i;
					}
					bstart += i+sp.latchSeqLen;
					bsize = np - bstart;
					++ sp.nMessageRepeats;
				}
			}
			
			if( fullMessageSize > 0 )
			{
				sp.messageBits = fullMessageSize;
			}
			else
			{
				sp.messageBits = np - sp.nMessageRepeats*sp.latchSeqLen;
			}

			sp.matchingRepeats = false;
			if( sp.nMessageRepeats>1 && fullMessageSize>0 )
			{
				int bstart = fullMessageStart+fullMessageSize;
				int bsize = np - bstart;
				sp.matchingRepeats = ( findOccurence(buf+fullMessageStart,fullMessageSize,buf+bstart,bsize) != -1 );
			}

			sp.coding = CODING_BINARY;
			if( fullMessageSize>0 )
			{
				sp.messageBits = 0;
				bool manchester = true;
				bool secondBitTst = false;
				bool firstBit;
				for(int i=fullMessageStart;i<(fullMessageStart+fullMessageSize) && manchester;i++)
				{
					if( buf[i]==si0 || buf[i]==si1 )
					{
						++ sp.messageBits;
						bool cbit = ( buf[i]==si0 );
						if( secondBitTst )
						{
							if( cbit == firstBit ) { manchester = false; }
							else { secondBitTst = false; }
						}
						else
						{
							firstBit = cbit;
							secondBitTst = true;
						}
					}
				}
				if( manchester )
				{
					sp.coding = CODING_MANCHESTER;
					sp.messageBits /= 2;
				}
			}
		}

		return true;
	}

	RXPinT& rx;
};

template<typename RXPinT>
static RFSniffer<RXPinT> make_sniffer(RXPinT& rx) 
{
	return RFSniffer<RXPinT>(rx);
}

#endif
