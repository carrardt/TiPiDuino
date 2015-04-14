#ifndef __RFSNIFFER_H
#define __RFSNIFFER_H

#include "RFSnifferConstants.h"
#include "RFSnifferProtocol.h"
#include "AvrTL.h"

/*
 * TODO: improve pulse length encoding, using TIMER0PRESCALER value
 */

template<typename RXPinT, typename LedPinT, typename OutputStreamT>
struct RFSniffer
{
	static constexpr uint16_t MIN_MESSAGE_PULSES = 32;
	static constexpr uint16_t ENTROPY_DETECTION_PULSES = 32;
	static constexpr uint16_t MAX_PULSES = 384;
	
	RFSniffer(RXPinT& _rx, RFSnifferProtocol& _sp, LedPinT& _led, OutputStreamT& _out)
		: rx(_rx)
		, sp(_sp)
		, led(_led)
		, cout(_out)
		{}
		
	// detect SeqLen consecutive similar symbols (at most NSymbols different symbols)
	// buf size must be at least NSymbols
	template<uint8_t NSymbols, uint8_t SeqLen>
	inline uint8_t detectEntropyDrop(uint16_t* buf, bool pulseLevel)
	{
		uint8_t nSymbolsInBuffer = 0;
		uint8_t nSymbolsRead = 0;
		while(nSymbolsInBuffer<NSymbols && nSymbolsRead<SeqLen)
		{
			long p = rx.PulseIn(pulseLevel,MAX_PULSE_LEN);
			if( p < MIN_PULSE_LEN ) return nSymbolsRead;
			uint8_t j=NSymbols;
			for(int s=0;s<nSymbolsInBuffer;++s)
			{
				uint16_t l = buf[s];
				uint16_t re = l / PULSE_ERR_RATIO;
				if( avrtl::abs(p-l) <= re ) j=s;			
			}
			if(j==NSymbols)
			{
				buf[nSymbolsInBuffer++] = p;
			}
			++ nSymbolsRead;
		}
		while( nSymbolsRead < SeqLen )
		{
			long p = rx.PulseIn(pulseLevel,MAX_PULSE_LEN);
			if( p < MIN_PULSE_LEN ) return nSymbolsRead;
			uint8_t j = NSymbols;
			for(int s=0;s<NSymbols;++s)
			{
				uint16_t l = buf[s];
				uint16_t re = l / PULSE_ERR_RATIO;
				if( avrtl::abs(p-l) <= re ) j=s;			
			}
			if(j==NSymbols) return nSymbolsRead;
			++ nSymbolsRead;
		}
		return nSymbolsRead;
	}

	// record a raw signal (succession of digital pulse lengthes)
	template<uint16_t bufSize, uint16_t minLatchLen, uint8_t minLatchCount>
	int recordSignalLatchDetect(uint16_t * buf,bool pulseLevel)
	{
	  int n=0;
	  for(;n<minLatchCount;++n)
	  {
		  buf[n] = rx.PulseIn(pulseLevel,MAX_PULSE_LEN);
		  if( buf[n]<minLatchLen || buf[n]>=MAX_PULSE_LEN ) return n;
	  }
	  do {
		buf[n++] = rx.PulseIn(pulseLevel,MAX_PULSE_LEN);
	  }
	  while( buf[n-1]>MIN_PULSE_LEN && buf[n-1]<=MAX_PULSE_LEN && n<bufSize);
	  if(n<bufSize) { --n; }
	  return n;
	}

	template<uint16_t bufSize>
	int recordSignalLatchSequenceDetect(uint16_t * buf,bool pulseLevel,const int nlacthes, const uint16_t* sequence)
	{
		for(uint8_t i=0;i<nlacthes;i++)
		{
			long p = rx.PulseIn(pulseLevel,MAX_PULSE_LEN);
			buf[i] = p;
			uint16_t l = sequence[i];
			uint16_t relerr = l / PULSE_ERR_RATIO;
			if( avrtl::abs(p-l) > relerr ) return 0;
		}
		int n = nlacthes;
	    do
	    {
			buf[n++] = rx.PulseIn(pulseLevel,MAX_PULSE_LEN);
	    }
	    while( buf[n-1]>MIN_PULSE_LEN && buf[n-1]<=MAX_PULSE_LEN && n<bufSize);
		if(n<bufSize) { --n; }
	    return n;
	}

	// record a raw signal (succession of digital pulse lengthes)
	template<uint16_t bufSize, uint16_t binarySeqLen>
	int recordSignalBinaryEntropyDetect(uint16_t * buf, bool pulseLevel, uint16_t& P1)
	{
		uint16_t fop0 = 0; // first occurence position
		uint16_t fop1 = 0; // first occurence position
		uint16_t curs = 0; // circular buffer cursor
		uint16_t re;
		int32_t l;

#define CURSOR_DIST(a,b) (((bufSize+b)-a)%bufSize)

		l = rx.PulseIn(pulseLevel,MAX_PULSE_LEN);
		if( l < MIN_PULSE_LEN ) return 0;
		buf[curs] = l;	
		curs=(curs+1)%bufSize;
	
		do
		{
			// whole buffer has been filled without finding another symbol
			if(curs==fop0) return 0;

			// read a valid pulse,
			do {
				l = rx.PulseIn(pulseLevel,MAX_PULSE_LEN);
			} while( l < MIN_PULSE_LEN ) ;
			
			// store the pulse
			buf[curs] = l;
			fop1 = curs;
			curs=(curs+1)%bufSize;
			
			// and test if it is different from the first one
			re = l / PULSE_ERR_RATIO;
		} while( avrtl::abs(l-buf[fop0]) <= re );

		// here, we have find a second symbol

		// now wait until we have a long enough binary sequence
		while( CURSOR_DIST(fop0,curs) < binarySeqLen )
		{
			l = rx.PulseIn(pulseLevel,MAX_PULSE_LEN);
			if( l < MIN_PULSE_LEN ) return 0;
			buf[curs] = l;
			re = l / PULSE_ERR_RATIO;
			bool equalsP0 = ( avrtl::abs(l-buf[fop0]) <= re );
			bool equalsP1 = ( avrtl::abs(l-buf[fop1]) <= re );
			
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
			l = rx.PulseIn(pulseLevel,MAX_PULSE_LEN);
			if( l < MIN_PULSE_LEN )
			{
				int recordSize = CURSOR_DIST(fop0,curs);
				P1 = (fop1+bufSize-fop0)%bufSize;
				avrtl::rotateBufferLeft(buf,bufSize,fop0);
				return recordSize;
			}
			buf[curs] = l;
			curs=(curs+1)%bufSize;
		}

		P1 = (fop1+bufSize-fop0)%bufSize;
		avrtl::rotateBufferLeft(buf,bufSize,fop0);
		return bufSize;
	}

	static bool identicalPulses(long p1, long p2)
	{
		long r = (p2>p1) ? p2 : p1 ;
		long re = r / PULSE_ERR_RATIO;
		return ( avrtl::abs(p2-p1) < re );
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
			if( avrtl::abs(sym-buf[i]) <= relerr ) // sample belongs to current class
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
	bool analyseSignal(uint16_t* buf, int np)
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
		if( identicalPulses(sp.bitSymbols[0],sp.bitSymbols[1]) ) return false;
		
		// non bit coding symbols are considered as latches (start/stop markers)		
		if( nSymbols == 2 ) // no latches, only bits
		{
			sp.latchSeqLen = 0;
			sp.nMessageRepeats = 1;
			sp.coding = CODING_BINARY;
			sp.messageBits = np;
			sp.setMatchingRepeats( true );
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
				while( ( i = avrtl::findOccurence(buf+lstart,sp.latchSeqLen,buf+bstart,bsize) ) != -1 )
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

			sp.setMatchingRepeats( false );
			if( sp.nMessageRepeats>1 && fullMessageSize>0 )
			{
				int bstart = fullMessageStart+fullMessageSize;
				int bsize = np - bstart;
				sp.setMatchingRepeats( avrtl::findOccurence(buf+fullMessageStart,fullMessageSize,buf+bstart,bsize) != -1 );
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

	void learnProtocol()
	{
		static const char* stageLabel[3] = {"detect","analyse","verify"};
		bool stageChanged=true;
		int stage=0;
		sp.init();
		while( ! sp.isValid() )
		{
			if( stageChanged )
			{
				cout<<stageLabel[stage]<<' '<<(sp.mediumRF()?"RF":"IR")<<'/'<<(sp.pulseLevel()?"Hi":"Lo")<<'\n';
				stageChanged=false;
			}
			// detect and record a signal
			if( stage == 0 )
			{
				uint16_t buf[MAX_PULSES];
				uint16_t P1Idx=0;
				int npulses = recordSignalBinaryEntropyDetect<MAX_PULSES,ENTROPY_DETECTION_PULSES>(buf,sp.pulseLevel(),P1Idx);
				bool signalOk = false;
				if( npulses >= MIN_MESSAGE_PULSES )
				{
						long P0=buf[0];
						long P1=buf[P1Idx];
						signalOk = analyseSignal(buf,npulses);
						if( signalOk )
						{
							/*
							cout<<"entropy detected bits "<<P0<<", "<<P1<<'\n';
							cout<<"analysed bits "<<sp.bitSymbols[0]<<", "<<sp.bitSymbols[1]<<'\n';
							*/
							long re0 = sp.bitSymbols[0] / PULSE_ERR_RATIO;
							long re1 = sp.bitSymbols[1] / PULSE_ERR_RATIO;
							int P0Bit=2, P1Bit=2;
							
							if( identicalPulses(P0,sp.bitSymbols[0]) ) P0Bit=0;
							else if( identicalPulses(P0,sp.bitSymbols[1]) ) P0Bit=1;
							
							if( identicalPulses(P1,sp.bitSymbols[0]) ) P1Bit=0;
							else if( identicalPulses(P1,sp.bitSymbols[1]) ) P1Bit=1;

							// cout<<"=> "<<P0Bit<<", "<<P1Bit<<'\n';
							if( P0Bit==2 || P1Bit==2 || P0Bit==P1Bit )
							{
								signalOk = false;
							}
						}
						if( ! signalOk )
						{
							sp.setPulseLevel( ! sp.pulseLevel() );
							//sp.setMediumRF( ! sp.mediumRF() );
							//rx.SelectPin( sp.mediumRF() );
							stageChanged=true;
						}
				}
				if(signalOk)
				{
					++stage;
					if( sp.latchSeqLen == 0 ) ++stage;
					stageChanged=true;
					sp.toStream(cout);
					blink(led);
				}
			}
			// make a new record with a latch detection for record content robustness
			else if( stage == 1 )
			{
				bool signalOk = false;
				if( sp.latchSeqLen > 0 )
				{
					uint16_t buf[MAX_PULSES];
					int npulses = recordSignalLatchSequenceDetect<MAX_PULSES>(buf,sp.pulseLevel(),sp.latchSeqLen,sp.latchSeq);
					if( npulses >= MIN_MESSAGE_PULSES )
					{
						signalOk = analyseSignal(buf,npulses);
					}
				}
				if(signalOk)
				{
					++stage;
					stageChanged=true;
					sp.toStream(cout);
					blink(led);
				}
			}
			// signal content analysis
			else if( stage == 2 )
			{
				if( sp.messageBits > MAX_MESSAGE_BITS ) sp.messageBits = MAX_MESSAGE_BITS;
				int bitsToRead = sp.messageBits;
				if( sp.coding == CODING_MANCHESTER ) bitsToRead *= 2;
				int nbytes = (bitsToRead+7) / 8;
				uint8_t signal1[nbytes];
				int br;
				do { br = sp.readMessage(rx,signal1); } while( br==0 );
				if( br == bitsToRead )
				{
					uint32_t retries=0;
					uint8_t signal2[nbytes];
					cout<<"press again\n";
					do
					{
						br = sp.readMessage(rx,signal2);
						++retries;
					} while( br==0 );
					for(int i=0;i<nbytes;i++) if(signal1[i]!=signal2[i]) br=0;
					if( br == bitsToRead )
					{
						sp.setValid(true);
					}
				}
				else
				{
					cout << "Err: "<<br<<'/'<<bitsToRead<<'\n';
					blink(led);
					stage=0;
					stageChanged=true;
				}
			}
		}
	}

	RXPinT& rx;
	LedPinT& led;
	OutputStreamT& cout;
	RFSnifferProtocol& sp;
};

template<typename RXPinT, typename LedPinT, typename OStreamT>
static RFSniffer<RXPinT,LedPinT,OStreamT> make_sniffer(RXPinT& rx, RFSnifferProtocol& sp, LedPinT& led, OStreamT& out) 
{
	return RFSniffer<RXPinT,LedPinT,OStreamT>(rx,sp,led,out);
}

#endif
