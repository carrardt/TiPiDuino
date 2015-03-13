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

static uint32_t code = 0x3B32EC80;

void loop()
{
  mySwitch.sendDWord( code );
  code ^= 0x10;
  delay(5000);
}


