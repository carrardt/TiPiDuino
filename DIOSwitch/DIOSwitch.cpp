#include "DIOSwitch.h"

DIOSwitch::DIOSwitch(int r, int e, int l) : receivePin(r), emitPin(e), ledPin(l) {}

byte DIOSwitch::waitForSignalStart()
{
  long t = 0;

  // latch 1
  while( abs(t-LATCH1_LEN) > PULSE_LEN_ERR )
  {	
    startTimings();
    t = readPulse(receivePin, PULSE_LVL);
  }

  // latch 2
  int interLatchPulses = 0;
  while( ( abs(t-LATCH2_LEN) > PULSE_LEN_ERR ) && ( interLatchPulses <= MAX_ILP ) )
  {
    t = readPulse(receivePin, PULSE_LVL);
    ++ interLatchPulses;
  }
  -- interLatchPulses;

  return (abs(t-LATCH2_LEN)<=PULSE_LEN_ERR) && (interLatchPulses<=MAX_ILP) ;
}

byte DIOSwitch::getSignalBit()
{
  byte b = BAD_BIT;
  long t = readPulse(receivePin, PULSE_LVL);
  if( abs(t-BIT0_LEN) <= PULSE_LEN_ERR )
  {
    b = 0;
  }
  else if( abs(t-BIT1_LEN) <= PULSE_LEN_ERR )
  {
    b = 1;
  }
  return b;
}

byte DIOSwitch::decodeSignal(int nbits, byte* output)
{
  byte bcount = 0;
  while( nbits > 0 )
  {
    byte x = getSignalBit();
    byte y = getSignalBit();

    if( x==BAD_BIT || y==BAD_BIT || x==y ) { 
      return false; 
    }
    if( bcount == 0 ) { 
      *output = x; 
    }
    else { 
      *output = ( *output << 1 ) | x; 
    }
    -- nbits;
    ++ bcount;
    if( bcount == 8 ) { 
      bcount==0; 
      ++output; 
    }
  }
  return true;
}

byte DIOSwitch::readSwitchCommand(unsigned long* sender, byte* state)
{
  if( waitForSignalStart() )
  {
    byte message[MESSAGE_BYTES];
    if( decodeSignal( MESSAGE_BITS, message ) )
    {
    	*sender = ( ((unsigned int)(message[0])) << 3 ) | ( message[1] >> 5 );
    	*state = ( message[1] >> 4 ) & 1;
	this->printTimings();
      	return true;
    }
  }
  return false;
}

void DIOSwitch::sendPulse( unsigned long pre, unsigned long len )
{
	digitalWrite( emitPin, HIGH );
	delayMicroseconds( pre );
	digitalWrite( emitPin, LOW );
	delayMicroseconds( len );
}

void DIOSwitch::sendBitPair(byte b)
{
	if( b ) { sendBit1(); sendBit0(); }
	else    { sendBit0(); sendBit1(); }
}

void DIOSwitch::sendSignal(int nbits, const byte* input)
{
	sendLatch1();
	sendLatch2();
	int b = 8;
	while( nbits > 0 )
	{
		-- nbits;
		-- b;
		byte bit = (*input >> b ) & 1;
		sendBitPair( bit );
		if( b == 0 ) { b=8; ++input; }
	}
}

void DIOSwitch::sendSwitchCommand(unsigned long sender, byte state)
{
	byte message[4] ;
	message[0] = ( sender >> 3 ) & 0xFF;
	message[1] = ( sender << 5 ) & 0xFF ;
	message[1] |= state ? (1<<4) : 0 ;
	message[2] = 0;
	message[3] = 0;
/*	Serial.print(message[0],HEX);
	Serial.print(",");
	Serial.print(message[1],HEX);
	Serial.print(",");
	Serial.print(message[2],HEX);
	Serial.print(",");
	Serial.println(message[3],HEX); */
	for(int i=0;i<REPEAT_COMMAND;i++)
	{
		sendSignal( 32, message );
	}
}


