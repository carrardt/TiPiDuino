#include "Arduino.h"

// #define DIO_DEBUG 1

class DIOSwitch
{
public:

  enum {
    PULSE_LVL     = LOW,
    LATCH1_PRE	  = 1630,
    LATCH1_LEN 	  = 9300,
    LATCH2_PRE    = 600, 
    LATCH2_LEN 	  = 2440, 
    BIT0_PRE 	  = 280, 
    BIT0_LEN 	  = 270, 
    BIT1_PRE 	  = 420, 
    BIT1_LEN 	  = 1200, 
    MAX_ILP 	  = 0, 
    PULSE_LEN_ERR = 30, 
    BAD_BIT 	  = 0xFF,
    MESSAGE_BITS  = 32, 
    MESSAGE_BYTES = 4,
    REPEAT_COMMAND= 3
  };

  DIOSwitch(int rpin, int epin, int lpin);

  byte waitForSignalStart();
  byte getSignalBit();
  byte decodeSignal(int nbits, byte* output);
  byte readSwitchCommand(unsigned long* sender, byte* state);

  void sendPulse( unsigned long pre, unsigned long len );
  void sendLatch1() { sendPulse(LATCH1_PRE,LATCH1_LEN); }
  void sendLatch2() { sendPulse(LATCH2_PRE,LATCH2_LEN); }
  void sendBit0()   { sendPulse(BIT0_PRE,BIT0_LEN); }
  void sendBit1()   { sendPulse(BIT1_PRE,BIT1_LEN); }
  void sendBitPair(byte b);
  void sendSignal(int nbits, const byte* input);
  void sendSwitchCommand(unsigned long sender, byte state);

protected:
  byte receivePin, emitPin, ledPin;

#ifdef DIO_DEBUG
#define MAX_TIMINGS 256
  int tcounter;
  unsigned long clk;
  unsigned int timings[MAX_TIMINGS];
  void startTimings() { clk=micros(); tcounter=0; }
  unsigned long readPulse(int p, int lvl)
  {
//	digitalWrite(ledPin,HIGH);
	unsigned long ts = micros();
	unsigned long d = pulseIn( p, lvl );
	unsigned long t = micros() - ts;
	timings[tcounter++] = (t-d);
	timings[tcounter++] = d ;
	if(tcounter>=(MAX_TIMINGS-1))
	{
		Serial.println("TOVF");
		tcounter=0;
	}
//	digitalWrite(ledPin,LOW);
	return d;
  }
public:
  void printTimings()
  {
	Serial.println(tcounter);
	for(int i=0;i<tcounter;i++)
	{
		Serial.print( ((i%2)==0) ? 'x' : 'p' );
		Serial.print(timings[i]);
		Serial.print(" ");
	}
	Serial.println("");
  }
#else
  void startTimings() { }
  unsigned long readPulse(int p, int lvl) { return pulseIn(p,lvl); }
public:
  void printTimings() 
  {
	for(int i=0;i<10;i++) { digitalWrite( ledPin, i&1 ); delay(100); }
	digitalWrite( ledPin, LOW );
  }
#endif

};


