#pragma once

#include <stdint.h>
#include <avr/io.h>

struct AvrAnalogDigitalConverter
{
	inline void begin()
	{
		ADMUX = (1 << REFS0); // AREF = AVcc
		ADMUX &= ~(1 << ADLAR); // set ADLAR to 0 => right adjust

		// ADC Enable and prescaler of 128
		// with 16Mhz clock : 16000000/128 = 125000 Hz = 125 Khz
		// with 8 Mhz clock :  8000000/128 =  62500 Hz = 62,5 Khz
		ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

		// selects analog channel
		ADMUX = (ADMUX & 0xF8) | m_analogChannel;
	}

	inline void startRead()
	{
		ADCSRA |= (1 << ADSC);
	}

	inline uint8_t endRead()
	{
		while (ADCSRA & (1 << ADSC));
		return (ADC);
	}

	inline void setChannel(uint8_t ch)
	{
		m_analogChannel = ch & 7;
		ADMUX = (ADMUX & 0xF8) | m_analogChannel;
	}

private:
	int8_t m_analogChannel = 0;
};

struct NullAnalogDigitalConverter
{
	inline void begin() {}
	inline void startRead() {}
	inline uint8_t endRead() { return 0;  }
	inline void setChannel(uint8_t ch) { }
};
