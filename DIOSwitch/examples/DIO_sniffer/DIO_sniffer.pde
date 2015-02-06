#define EMIT_PIN       7
#define RECEIVE_PIN    8

#include "DIOSwitch.h"

DIOSwitch mySwitch(RECEIVE_PIN, EMIT_PIN);

void setup()
{	
  pinMode(RECEIVE_PIN, INPUT);
  Serial.begin(9600);
  Serial.println("start");
}

void loop()
{
  unsigned long sender=0;
  byte state=0;
  if( mySwitch.readSwitchCommand(&sender,&state) )
  {
    Serial.print("sender=");
    Serial.print(sender);
    Serial.print(", state=");
    Serial.println(state);
  }
}

