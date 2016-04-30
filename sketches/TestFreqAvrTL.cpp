#include <AvrTL.h>
#include <AvrTLPin.h>
#include <avr/interrupt.h>

/*
 * Test de fréquence pour l'ATtiny85 (ou autre)
 * 1. Graver la séquence d'initialisation avec la config voulue (i.e. 8Mhz internal clock, etc.)
 * 2. téléverser le programme
 * 3. mesurer la durée d'un créneau à l'oscilo. par exemple T = 0.0005 sec (500uS)
 * 4. Fréquence d'horloge = 8192 / T
 */
#define CLK_PIN 0
#define LED_PIN 1

auto clk = avrtl::StaticPin<CLK_PIN>();
auto led = avrtl::StaticPin<LED_PIN>();

// the setup function runs once when you press reset or power the board
void setup()
{
  clk.SetOutput();
  led.SetOutput();
  cli();
}

// the loop function runs over and over again forever
void loop() {
  static uint16_t counter = 0;
  static uint8_t tick = 0;
  uint8_t tock = (TCNT0>>7) & 0x01;
  counter += tick^tock;
  tick = tock;
  clk.Set( tock );
  led.Set( (counter>>10) & 0x01 );
}
