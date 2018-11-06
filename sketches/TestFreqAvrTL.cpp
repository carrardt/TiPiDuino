#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTL/timer.h>
#include <avr/interrupt.h>

/*
 * Test de fréquence pour l'ATtiny85 (ou autre)
 * 1. Graver la séquence d'initialisation avec la config voulue (i.e. 8Mhz internal clock, etc.)
 * 2. téléverser le programme
 * 3. mesurer la durée d'un créneau à l'oscilo. par exemple T = 0.0005 sec (500uS)
 * 4. Fréquence d'horloge = 8192 / T
 */
#if defined(__AVR_ATtiny84__)
//#warning ATtiny84
#define CLK_PIN 5
#define LED_PIN 9
#elif defined(__AVR_ATtiny85__)
//#warning ATtiny85
#define CLK_PIN 4
#define LED_PIN 3
#else
#define CLK_PIN 8
#define LED_PIN 13
#endif

auto clk = avrtl::StaticPin<CLK_PIN>();
auto led = avrtl::StaticPin<LED_PIN>();

avrtl::AvrTimer0 g_timer; // by default, prescaler is 8, so clock freq is divided by 8 already

// the setup function runs once when you press reset or power the board
void setup()
{
  clk.SetOutput();
  led.SetOutput();
  cli();
  g_timer.start();
}

// the loop function runs over and over again forever
void loop() {
  static uint16_t counter = 0;
  static uint8_t tick = 0;
  uint8_t tock = (g_timer.counter()>>7) & 0x01;
  counter += tick^tock;
  tick = tock;
  
  clk.Set( (counter>>3) & 0x01); // clock interval * 8192, i.e. 1 millisecond with 8Mhz clock
  led.Set( (counter>>14) & 0x01 ); // 1 sec interval with 16Mhz clock, 2 seconds interval with 8Mhz clock
}
