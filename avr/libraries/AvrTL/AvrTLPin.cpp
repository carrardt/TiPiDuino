#include <AvrTL/AvrTLPin.h>

// compatibility layer
void pinMode(uint8_t pId, uint8_t mode)
{
	auto p = avrtl::make_pin(pId);
	if( mode == INPUT ) p.SetInput();
	else if( mode == OUTPUT ) p.SetOutput();
	else if( mode == INPUT_PULLUP ) { p.SetInput(); p.Set(HIGH); }
}

void digitalWrite(uint8_t pId, bool level)
{
	avrtl::make_pin(pId).Set( level );
}

bool digitalRead(uint8_t pId)
{
	return avrtl::make_pin(pId).Get();
}

uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
{
        uint8_t value = 0;
        uint8_t i;
        for (i = 0; i < 8; ++i) {
                digitalWrite(clockPin, HIGH);
                if (bitOrder == LSBFIRST)
                        value |= digitalRead(dataPin) << i;
                else
                        value |= digitalRead(dataPin) << (7 - i);
                digitalWrite(clockPin, LOW);
        }
        return value;
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
        uint8_t i;
        for (i = 0; i < 8; i++)  {
                if (bitOrder == LSBFIRST)
                        digitalWrite(dataPin, !!(val & (1 << i)));
                else
                        digitalWrite(dataPin, !!(val & (1 << (7 - i))));

                digitalWrite(clockPin, HIGH);
                digitalWrite(clockPin, LOW);
        }
}


