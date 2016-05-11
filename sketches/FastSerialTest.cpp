#include <FastSerial.h>
#include <AvrTL.h>
#include <AvrTLPin.h>
#include <avr/interrupt.h>

using namespace avrtl;

/*
 * linux example usage :
 * stty -F /dev/ttyUSB0 57600 raw cs8
 * echo "Hello world" > /dev/ttyUSB0
 */

#define RX_PIN 3
#define TX_PIN 4
#define LED_PIN 2

static auto rx = StaticPin<RX_PIN>();
static auto tx = StaticPin<TX_PIN>();

static auto fastSerial = make_fastserial(rx,tx);
 
void setup()
{
	cli();
	fastSerial.begin();
}

static uint32_t counter = 0;
void loop()
{
	fastSerial.write<32>(counter);
	++ counter;
}
