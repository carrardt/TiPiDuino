#ifndef __RFSNIFFER_H
#define __RFSNIFFER_H

#include "RFSnifferConstants.h"
#include "RFSnifferProtocol.h"

/*
 * TODO: 
 * Improve learn stage robustness:
 * 		lower initial latch detection strictness (length and count),
 * 		detect bit coding symbols --through entropy-- by discriminating latches => Ok
 * 		then deduce latch symbols => Ok
 * 
 * Stage 1:
 * 		decode message,
 * 		check for repeated sends ==> Ok
 * 		verify encoding algorithm (manchester, etc.) ==> Ok
 * 
 */

/*
 * TODO: improve pulse length encoding, using TIMER0PRESCALER value
 */
//#include "Wiring.h"

template<typename RXPinT>
struct RFSniffer
{
	RFSniffer(RXPinT& _rx) : rx(_rx) {}
		
	template<typename T>
	static inline T abs(T x) { return (x<0) ? -x : x ; }

	// detect SeqLen consecutive similar symbols (at most NSymbols different symbols)
	// buf's size must be at least NSymbols
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
	int recordSignal(uint16_t * buf)
	{
	  int n=0;
	  for(;n<MIN_PROLOG_LATCHES;++n)
	  {
		  buf[n] = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
		  if( buf[n]<MIN_LATCH_LEN || buf[n]>=MAX_PULSE_LEN ) return n;
	  }
	  do {
		buf[n++] = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
	  }
	  while( buf[n-1]>MIN_PULSE_LEN && buf[n-1]<=MAX_PULSE_LEN && n<MAX_PULSES);
	  return n;
	}

	int readBinaryMessage(const RFSnifferProtocol& sp, uint8_t* buf)
	{
		const uint16_t b0 = sp.symbols[sp.nSymbols-1];
		const uint16_t b1 = sp.symbols[sp.nSymbols-2];
		const uint16_t b0_tol = b0 / PULSE_ERR_RATIO;
		const uint16_t b1_tol = b1 / PULSE_ERR_RATIO;

		for(uint8_t i=0;i<sp.latchSeqLen;i++)
		{
			long p = rx.PulseIn(PULSE_LVL,MAX_PULSE_LEN);
			uint16_t l = sp.symbols[sp.latchSeq[i]];
			uint16_t relerr = l / PULSE_ERR_RATIO;
			if( abs(p-l) > relerr ) return 0;
		}
		
		uint8_t byte=0;
		for(uint16_t j=0;j<sp.messageBits;j++)
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
		return sp.messageBits;
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

	bool analyseSignal( RFSnifferProtocol& sp )
	{
		  uint16_t buf[MAX_PULSES];
		  sp.nPulses = recordSignal( buf );
		  if( sp.nPulses >= MIN_MESSAGE_PULSES )
		  {
			uint8_t symcount[MAX_SYMBOLS];
			sp.nSymbols = classifySymbols(buf,sp.nPulses,sp.symbols,symcount);
			sp.nLatches=0;
			// discriminate longest pulses as latches
			while( sp.nLatches<sp.nSymbols && sp.symbols[sp.nLatches]>=MIN_LATCH_LEN ) ++ sp.nLatches;
			if( (sp.nLatches+2) <= sp.nSymbols ) // at least 2 non-latch symbols are necessary to code a message
			{
				int nCodingSymbols = sp.nSymbols - sp.nLatches;
				
				// FIXME:
				// weakness: assumes that message starts with a correct full latch sequence

				// detect start latch sequence
				sp.latchSeqLen = 0;
				while( sp.latchSeqLen<MAX_LATCH_SEQ_LEN && buf[sp.latchSeqLen]<sp.nLatches )
				{ 
					sp.latchSeq[sp.latchSeqLen] = buf[sp.latchSeqLen];
					++sp.latchSeqLen;
				}
				
				// detect repeated message
				sp.nMessageRepeats = 1;
				{
					int j=0;
					for(int i=sp.latchSeqLen;i<sp.nPulses;++i)
					{
						if( buf[i] == buf[j] )
						{ 
							++j;
							if(j==sp.latchSeqLen)
							{
								j=0;
								++sp.nMessageRepeats;
							}
						}
						else { j=0; }
					}
				}
				sp.matchingRepeats = true;
				if( sp.nMessageRepeats>1 )
				{
					int p = sp.nPulses/sp.nMessageRepeats;
					for(int i=0;i<p;++i)
					{
						for(int j=1;j<sp.nMessageRepeats;++j)
						{
							if( buf[j*p+i] != buf[i] ) sp.matchingRepeats=false;
						}
					}
				}
				
				if( nCodingSymbols > 2 )
				{
					sp.coding = CODING_UNKNOWN;
					sp.messageBits = -1; // could be computed though
				}
				else 
				{
					sp.coding = CODING_BINARY;
					if( symcount[sp.nLatches] == symcount[sp.nLatches+1] )
					{
						sp.messageBits = 0;
						bool manchester = true;
						bool secondBitTst = false;
						bool firstBit;
						for(int i=0;i<sp.nPulses && manchester;i++)
						{
							if( buf[i]==sp.nLatches || buf[i]==(sp.nLatches+1) )
							{
								++ sp.messageBits;
								bool cbit = ( buf[i]==sp.nLatches );
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
						}
					}
					sp.messageBits /= sp.nMessageRepeats;
				}
				return true;
			}
			else { return false; }
		  }
		  else { return false; }
	}

	RXPinT& rx;
};

template<typename RXPinT>
static RFSniffer<RXPinT> make_sniffer(RXPinT& rx) 
{
	return RFSniffer<RXPinT>(rx);
}

#endif
