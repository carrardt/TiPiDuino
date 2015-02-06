#define EMIT_PIN       7
#define RECEIVE_PIN    8

#include "DIOSwitch.h"

DIOSwitch mySwitch(RECEIVE_PIN, EMIT_PIN);

void setup()
{	
  pinMode(EMIT_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("state ?");
}

unsigned long sender = 420;
byte state = 0;

void loop()
{
    mySwitch.sendSwitchCommand(sender, state );
  state = 1 - state;
  delay(5000);
}


