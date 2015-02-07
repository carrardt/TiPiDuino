/*
 * (c) Thierry Carrard
 */

#define EMIT_PIN       7
#define RECEIVE_PIN    8
#define LED_PIN        13

#include "DIOSwitch.h"

DIOSwitch mySwitch(RECEIVE_PIN, EMIT_PIN, LED_PIN);

void setup()
{	
  pinMode(RECEIVE_PIN, INPUT);
  pinMode(EMIT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  // Serial.begin(9600);
  // Serial.println("state ?");
}

unsigned long sender = 420;
byte state = 0;

void loop()
{
  mySwitch.sendSwitchCommand(sender, state );
  state = 1 - state;
  delay(5000);
}


