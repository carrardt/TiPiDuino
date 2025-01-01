#include "AvrApp.h"
#include <avr/io.h>
#include <avr/interrupt.h>

int main(void)
{
	// Sets the timer prescale factor to 64;
	// for compatibility with Wiring/Arduino
	TCCR0B = (TCCR0B & 0b11111000) | 0b011;
	// TCCR1B = 0; // disabled
	// TCCR2B = 0; // disabled;

	// start interrupts
	sei();

	setup();
	for(;;) loop();
	
	return 0;
}
