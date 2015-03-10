#include <Wiring.h>
#include <LCD.h>
#include "AvrTL.h"

#define MAX_PULSES 512
#define MIN_PULSE_LEN 150
#define MAX_PULSE_LEN 30000
#define MIN_LATCH_LEN 2000
#define MIN_PROLOG_LATCHES 2
#define MIN_MESSAGE_PULSES 64
#define PULSE_ERR_RATIO 5
#define MAX_SYMBOLS 16

#define RECEIVE_PIN 8
#define LED_PIN 13

LCD< 7,6, PinSet<5,4,3,2> > lcd;

using namespace avrtl;
constexpr auto led = pin(LED_PIN);
constexpr auto rx = pin(RECEIVE_PIN);

static int recordSignal(uint16_t buf[])
{
  int n=0;
  for(;n<MIN_PROLOG_LATCHES;++n)
  {
	  buf[n] = rx.PulseIn(LOW,MAX_PULSE_LEN);
	  if( buf[n]<MIN_LATCH_LEN || buf[n]>=MAX_PULSE_LEN ) return n;
  }
  do {
  	buf[n++] = rx.PulseIn(LOW,MAX_PULSE_LEN);
  }
  while( buf[n-1]>MIN_PULSE_LEN && buf[n-1]<=MAX_PULSE_LEN && n<MAX_PULSES);
  return n;
}

static int classifySymbols(const uint16_t buf[], int n, uint16_t symbols[], uint8_t symcount[])
{
  int nsymbols = 0;
  int nextSym = 0;
  for(int i=1;i<n;i++) { if(buf[i]>buf[nextSym]) nextSym=i; }
  while( nextSym != -1 )
  {
    long sym = buf[nextSym];
    long symAvg = sym;
    symbols[nsymbols] = sym;
    symcount[nsymbols] = 1;
    ++ nsymbols;
    nextSym = -1;
    for(int i=nsymbols;i<n;i++)
    {
      if( buf[i] != 0 ) 
      {
      	long relerr = sym / PULSE_ERR_RATIO;
  	if( abs(sym-buf[i]) <= relerr )
  	{
  	  symAvg += buf[i];
  	  ++ symcount[nsymbols-1];
  	}
  	else if( buf[i] < sym )
  	{
  	  if( nextSym == -1 ) { nextSym = i; }
  	  else if( buf[i] > buf[nextSym] ) { nextSym = i; }
  	}
      }
    }
    symbols[nsymbols-1] = symAvg / symcount[nsymbols-1];
    if( nsymbols >= MAX_SYMBOLS ) return MAX_SYMBOLS;
  }
  return nsymbols;
}

static bool learnSignal(uint16_t *symbols, int& nsymbols, int &nLatches)
{
	  uint16_t buf[MAX_PULSES];
	  int n = recordSignal( buf );
	  if( n >= MIN_MESSAGE_PULSES )
	  {
		uint8_t symcount[MAX_SYMBOLS];
		nsymbols = classifySymbols(buf,n,symbols,symcount);
		nLatches=0;
		while( nLatches<nsymbols && symbols[nLatches]>=MIN_LATCH_LEN ) ++nLatches;
		if( (nLatches+2) <= nsymbols ) // at least 2 non-latch symbols are necessary to code a message
		{
			lcd << '\n';
			lcd << n << ' ' << nsymbols << ' ' << nLatches << '\n';
			lcd << symbols[nLatches]<<'x'<<symcount[nLatches]<<' '<<symbols[nLatches+1]<<'x'<<symcount[nLatches+1];
			for(int j=0;j<50;j++) { led = j&1; DelayMicroseconds(10000); }
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
	lcd << "Step 1: learn";
	
	uint8_t stage = 0;
	for(;;)
	{
		if( stage == 0 )
		{
			uint16_t symbols[MAX_SYMBOLS];
			int nSymbols=0, nLatches=0;
			if( learnSignal(symbols,nSymbols,nLatches) )
			{
				DelayMicroseconds(10000000UL);
				lcd << "\nStep2: detect";
				++stage;
			}
		}
	}
}

