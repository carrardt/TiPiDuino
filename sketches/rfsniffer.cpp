#include <Wiring.h>
#include <LCD.h>

#define USE_EXTERNAL_MAIN 1

#define MAX_PULSES 512
#define MIN_PULSE_LEN 150
#define MAX_PULSE_LEN 32000
#define MIN_LATCH_LEN 1000
#define MIN_MESSAGE_PULSES 32
#define PULSE_ERR_RATIO 6
#define MAX_SYMBOLS 16

#define RECEIVE_PIN 8
#define LED_PIN 13

LCD< 7,6, PinSet<5,4,3,2> > lcd;

static void s_setup()
{	
  pinMode(RECEIVE_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  lcd.begin();
  lcd.print("DIO sniffer");
}

static int recordSignal(uint16_t buf[])
{
  unsigned long t;
  int retry = 10;
  do {
  	-- retry;
  	if( retry == 0 ) return 0;
  	t = pulseIn(RECEIVE_PIN,LOW,MAX_PULSE_LEN);
  } while(t<MIN_LATCH_LEN || t>=MAX_PULSE_LEN );
  buf[0]=t;
  int n=1;
  do {
  	buf[n++]=pulseIn(RECEIVE_PIN,LOW,MAX_PULSE_LEN);
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

static uint16_t symbols[MAX_SYMBOLS];
static uint8_t symcount[MAX_SYMBOLS];

static void s_loop()
{
  uint16_t buf[MAX_PULSES];
  int n = recordSignal( buf );
  if( n >= MIN_MESSAGE_PULSES )
  {
  	int nsymbols = classifySymbols(buf,n,symbols,symcount);
  	lcd.clear();
  	lcd.setCursor(0,0);
  	lcd << n << ' ' << nsymbols << ' ';
  	lcd << symbols[0] << 'x' << symcount[0];
  	lcd.setCursor(0,1);
  	lcd << symbols[1] << 'x' << symcount[1] << ' ';
  	lcd << symbols[2] << 'x' << symcount[2] << ' ';
  	lcd << symbols[3] << 'x' << symcount[3];
  	for(int j=0;j<20;j++) { digitalWrite(LED_PIN,j&1); delay(100); }
  }
}

#ifdef USE_EXTERNAL_MAIN

void setup() { s_setup(); }
void loop() { s_loop(); }

#else

int main(void) __attribute__((noreturn));
int main(void)
{
	s_setup();
	for(;;) s_loop();
}

#endif

