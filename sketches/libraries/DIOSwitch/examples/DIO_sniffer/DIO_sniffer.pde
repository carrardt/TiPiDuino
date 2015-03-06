#include "DIOSwitch.h"

#define RECEIVE_PIN 8
#define EMIT_PIN 7
#define LED_PIN 13

DIOSwitch mySwitch( RECEIVE_PIN, EMIT_PIN, LED_PIN );

void setup()
{	
  pinMode(RECEIVE_PIN, INPUT);
  pinMode(EMIT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
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

