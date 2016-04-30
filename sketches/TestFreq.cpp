#include <avr/io.h>
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
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  //pinMode(CLK_PIN, OUTPUT);
  //pinMode(LED_PIN, OUTPUT);
  DDRB |= 0x03;
  cli();
}

// the loop function runs over and over again forever
void loop() {
  static uint16_t counter = 0;
  static uint8_t tick = 0;
  uint8_t tock = (TCNT0>>7) & 0x01;
  //digitalWrite(CLK_PIN, tock  );   // turn the LED on (HIGH is the voltage level)
  counter += tick^tock;
  //digitalWrite(LED_PIN, (counter>>10) & 0x01  );
  tick = tock;
  PORTB = ( (counter>>10) & 0x02 ) | tock;
}
