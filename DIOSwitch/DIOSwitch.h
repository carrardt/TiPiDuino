
#include "Arduino.h"

class DIOSwitch
{
public:

  enum {
    PULSE_LVL     = LOW,
    LATCH1_LEN 	  = 9300, 
    LATCH2_LEN 	  = 2440, 
    BIT0_LEN 	  = 270, 
    BIT1_LEN 	  = 1200, 
    PULSE_GAP     = 300,
    MAX_ILP 	  = 0, 
    PULSE_LEN_ERR = 30, 
    BAD_BIT 	  = 0xFF,
    MESSAGE_BITS  = 32, 
    MESSAGE_BYTES = 4,
  };

  DIOSwitch(int rpin, int epin);

  byte waitForSignalStart();
  byte getSignalBit();
  byte decodeSignal(int nbits, byte* output);
  byte readSwitchCommand(unsigned long* sender, byte* state);

  void sendPulse( unsigned long us );
  void sendBit0();
  void sendBit1();
  void sendBitPair(byte b);
  void sendSignal(int nbits, const byte* input);
  void sendSwitchCommand(unsigned long sender, byte state);

protected:
  byte receivePin, emitPin;
};


