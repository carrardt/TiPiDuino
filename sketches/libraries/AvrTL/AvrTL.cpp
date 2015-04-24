#include "AvrTL.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

int main(void)
{
	// Sets the timer prescale factor to 64;
	// TCCR0B = (TCCR0B & 0b11111000) | 0b011;

	// Sets the timer prescale factor to 8;
	TCCR0B = (TCCR0B & 0b11111000) | 0b010;

	// start interrupts
	sei();

	setup();
	for(;;) loop();
}
