#include <AvrTL/AvrTL.h>
#include <AvrTL/AvrTLPin.h>
#include <AvrTL/timer.h>
#include <AvrTL/AvrApp.h>

#define LED_PIN 13

static constexpr avrtl::StaticPin<LED_PIN> led = {};

void setup()
{
	led.SetOutput(); //pinMode(LED_PIN,OUTPUT);
}

void loop()
{
  led.Set(true);
  avrtl::delay( 1000 );
  led.Set(false);
  avrtl::delay( 1000 );
}

