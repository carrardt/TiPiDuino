/*
 * Test de fréquence pour l'ATtiny85 (ou autre)
 * 1. Graver la séquence d'initialisation avec la config voulue (i.e. 8Mhz internal clock, etc.)
 * 2. téléverser le programme
 * 3. mesurer la durée d'un créneau à l'oscilo. par exemple T = 0.0005 sec (500uS)
 * 4. Fréquence d'horloge = 8192 / T
 */
#define LED_PIN 1
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(LED_PIN, OUTPUT);
  cli();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_PIN, (TCNT0>>7) & 0x01  );   // turn the LED on (HIGH is the voltage level)
}
