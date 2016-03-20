/* $Id: BoardDefs.h 1179 2011-06-11 06:02:39Z bhagman $
||
|| @author         Brett Hagman <bhagman@wiring.org.co>
|| @url            http://wiring.org.co/
||
|| @description
|| | Board Specific Definitions for:
|| |   Arduino Duemilanove, Uno, and all
|| |   ATmega168(P)A/328(P) Arduino compatible boards.
|| |   (Atmel AVR 8 bit microcontroller core)
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef WBOARDDEFS_H
#define WBOARDDEFS_H

// #include "WConstants.h"
#ifndef NOT_A_PORT
#define NOT_A_PORT 0xFF
#endif

#ifndef NOT_A_REG
#define NOT_A_REG  nullptr
#endif

#ifndef NOT_A_TIMER
#define NOT_A_TIMER nullptr
#endif

#ifndef LOW
#define LOW false
#endif

#ifndef HIGH
#define HIGH true
#endif

/*************************************************************
 * Pin to register mapping macros
 *************************************************************/

#define WdigitalPinToPortReg(PIN) \
        ( ((PIN) >= 0  && (PIN) <= 7)  ? &PORTD : \
        ( ((PIN) >= 8  && (PIN) <= 13) ? &PORTB : \
        ( ((PIN) >= 14 && (PIN) <= 19) ? &PORTC : NOT_A_REG)))

#define WdigitalPinToBit(PIN) \
        ( ((PIN) >= 0  && (PIN) <= 7)  ? ((PIN)) : \
        ( ((PIN) >= 8  && (PIN) <= 13) ? ((PIN) - 8) : \
        ( ((PIN) >= 14 && (PIN) <= 19) ? ((PIN) - 14) : 0)))

#define WdigitalPinToBitMask(PIN) (1 << (WdigitalPinToBit(PIN)))

#define WdigitalPinToPort(PIN) \
        ( ((PIN) >= 0  && (PIN) <= 7)  ? 0 : \
        ( ((PIN) >= 8  && (PIN) <= 13) ? 1 : \
        ( ((PIN) >= 14 && (PIN) <= 19) ? 2 : NOT_A_PORT)))

#define WportOutputRegister(PORT) \
        ( ((PORT) == 0 ) ? &PORTD : \
        ( ((PORT) == 1 ) ? &PORTB : \
        ( ((PORT) == 2 ) ? &PORTC : NOT_A_REG)))

#define WportInputRegister(PORT) \
        ( ((PORT) == 0 ) ? &PIND : \
        ( ((PORT) == 1 ) ? &PINB : \
        ( ((PORT) == 2 ) ? &PINC : NOT_A_REG)))

#define WportModeRegister(PORT) \
        ( ((PORT) == 0 ) ? &DDRD : \
        ( ((PORT) == 1 ) ? &DDRB : \
        ( ((PORT) == 2 ) ? &DDRC : NOT_A_REG)))

#endif
// BOARDDEFS_H
