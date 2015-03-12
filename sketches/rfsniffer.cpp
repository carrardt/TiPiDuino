#include <Wiring.h>
#include <LCD.h>
#include "AvrTL.h"

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

#define PULSE_LVL 			LOW
#define MAX_PULSES 			512
#define MIN_PULSE_LEN 		150	// under this threshold noise is too important
#define MAX_PULSE_LEN 		30000 // mainly due to 16-bits limitation
#define MIN_LATCH_LEN 		2000
#define MIN_PROLOG_LATCHES 	1
#define MIN_MESSAGE_PULSES 	64
#define PULSE_ERR_RATIO 	5   // 4(25%) would speedup execution ...
#define MAX_SYMBOLS 		8	// !! MUST VERIFY ( MAX_SYMBOLS < MIN_PULSE_LEN ) !!
#define MAX_LATCH_SEQ_LEN	8
#define MAX_MESSAGE_BITS	128
#define MAX_MESSAGE_BYTES	(MAX_MESSAGE_BITS/8)

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
	uint8_t latchSeq[MAX_LATCH_SEQ_LEN];
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

// record a raw signal (succession of digital pulse lengthes)
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

static int readBinaryMessage(const SignalProperties& sp, uint8_t* buf)
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

static bool analyseSignal( SignalProperties& sp )
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
	lcd << "RF sniffer" ;
	
	if( MAX_SYMBOLS >= MIN_PULSE_LEN )
	{
		lcd << "Error - 1";
		for(;;) { }
	}
		
	lcd << "\nStep 1: learn";
	
	bool hasProtocol = false; // TODO: read from EEPROM
	while( ! hasProtocol )
	{
		SignalProperties sp;
		if( analyseSignal(sp) )
		{
			lcd << '\n';
			lcd << sp.nSymbols <<' '<< sp.latchSeqLen <<' '<< codingChar[sp.coding] << sp.messageBits << " x" << sp.nMessageRepeats << (sp.matchingRepeats?'+':'-') << '\n';
			lcd << sp.symbols[sp.nLatches]<<' '<<sp.symbols[sp.nLatches+1];
			for(int j=0;j<50;j++) { led = j&1; DelayMicroseconds(10000); }
			DelayMicroseconds(10000000UL);
			lcd << "\nStep2: read";
			if( sp.coding == CODING_MANCHESTER ) sp.messageBits *= 2;
			if( sp.messageBits > MAX_MESSAGE_BITS ) sp.messageBits = MAX_MESSAGE_BITS;
			int nbytes = (sp.messageBits+7) / 8;
			uint8_t signal1[nbytes];
			int br;
			do { br = readBinaryMessage(sp,signal1); } while( br==0 );
			if( br == sp.messageBits )
			{
				int tries=0;
				uint8_t signal2[nbytes];
				do
				{
					// don't loose time printing "step 3 if repeated signal can be catched right after the first one"
					br = readBinaryMessage(sp,signal2);
					++tries;
					if( tries > 1000 ) lcd << "\nStep3: verify";
				} while( br==0 );
				for(int i=0;i<nbytes;i++) if(signal1[i]!=signal2[i]) br=0;
				if( br == sp.messageBits )
				{
					lcd << "\nWriting protocol";
					lcd << "\nto EEPROM ...";
					for(int j=0;j<500;j++) { led = j&1; DelayMicroseconds(100000); }
					hasProtocol = true;
				}
			}
		}
	}
	lcd << "\n * RF sniffer *" ;
	lcd << "\n ***  ready ***" ;
	for(;;) {}
}

