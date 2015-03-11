#include <Wiring.h>
#include <LCD.h>
#include "AvrTL.h"

/*
 * TODO: 
 * Improve learn stage robustness:
 * 		lower initial latch detection strictness (length and count),
 * 		detect bit coding symbols through entropy,
 * 		then deduce latch symbols
 * 
 * Stage 1:
 * 		decode message,
 * 		check for repeted sends
 * 		verify encoding algorithm (manchester, etc.)
 * 
 */

/*
 * TODO: improve pulse length encoding, using TIMER0PRESCALER value
 */

#define PULSE_LVL 			LOW
#define MAX_PULSES 			512
#define MIN_PULSE_LEN 		150	// under this threshold noise is too important
#define MAX_PULSE_LEN 		30000 // mainly due to 16-bits encoding
#define MIN_LATCH_LEN 		2000
#define MIN_PROLOG_LATCHES 	1
#define MIN_MESSAGE_PULSES 	64
#define PULSE_ERR_RATIO 	5
#define MAX_SYMBOLS 		16	// !! MUST VERIFY ( MAX_SYMBOLS < MIN_PULSE_LEN ) !!

#define RECEIVE_PIN 8
#define LED_PIN 13

LCD< 7,6, PinSet<5,4,3,2> > lcd;

using namespace avrtl;
constexpr auto led = pin(LED_PIN);
constexpr auto rx = pin(RECEIVE_PIN);

// Possibly detected encodings
enum MessageEncoding
{
	CODING_UNKNOWN = 0,
	CODING_BINARY = 1,
	CODING_MANCHESTER = 2
};
static constexpr char codingChar[] = {'?','B','M'};

struct SignalProperties
{
	uint16_t symbols[MAX_SYMBOLS];
	uint16_t nPulses;
	uint16_t messageBits;
	uint8_t nSymbols;
	uint8_t nLatches;
	uint8_t latchSeqLen;
	uint8_t nMessageRepeats;
	uint8_t coding;
	bool matchingRepeats;
	inline SignalProperties()
		: nPulses(0)
		, messageBits(0)
		, nSymbols(0)
		, nLatches(0)
		, latchSeqLen(0)
		, nMessageRepeats(0)
		, coding(CODING_UNKNOWN)
		, matchingRepeats(false) {}
};

// record a raw signal (succession of digital pulse length values)
static int recordSignal(uint16_t * buf)
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
		else // sample not in current cass, find the highest one
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

static bool learnSignal( SignalProperties& sp )
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
			
			// detect starting latch sequence
			sp.latchSeqLen = 0;
			while( buf[sp.latchSeqLen]<sp.nLatches ) ++sp.latchSeqLen;
			
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
						sp.messageBits /= 2;
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

int main(void) __attribute__((noreturn));
int main(void)
{
	boardInit();

	rx.SetInput();
	led.SetOutput();

	lcd.begin();
	lcd << "DIO sniffer\n" ;
	
	if( MAX_SYMBOLS >= MIN_PULSE_LEN )
	{
		lcd << "Error - 1";
		for(;;) { }
	}
		
	lcd << "Step 1: learn";
	
	for(;;)
	{
		SignalProperties sp;
		if( learnSignal(sp) )
		{
			lcd << '\n';
			lcd << sp.latchSeqLen <<' '<< codingChar[sp.coding] << sp.messageBits << " x" << sp.nMessageRepeats << (sp.matchingRepeats?'+':'-') << '\n';
			lcd << sp.symbols[sp.nLatches]<<' '<<sp.symbols[sp.nLatches+1];
			for(int j=0;j<50;j++) { led = j&1; DelayMicroseconds(10000); }
			DelayMicroseconds(10000000UL);
			lcd << "\nStep2: detect";
		}
	}
}

