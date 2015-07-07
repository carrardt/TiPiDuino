#ifndef __AVRTL_EEPROM_H
#define __AVRTL_EEPROM_H

#include <avr/eeprom.h>
#include <stdint.h>

namespace avrtl
{
	static constexpr uint16_t EEPROM_SIZE = 1024;

	static void eeprom_gently_write_byte(uint8_t* ptr, uint8_t value)
	{
		uint32_t ptrval = (uint32_t)ptr;
		if( ptrval < EEPROM_SIZE )
		{
			if( eeprom_read_byte(ptr) != value )
			{
				eeprom_write_byte(ptr,value);
			}
		}
	}

	static void eeprom_gently_write_block(const uint8_t* src, uint8_t* ptr, uint16_t N)
	{
		for(int i=0;i<N;i++)
		{
			eeprom_gently_write_byte(ptr+i,src[i]);
		}
	}

	template<typename T>
	static void eeprom_read(T& x, const uint8_t* ptr)
	{
		eeprom_read_block( (uint8_t*)&x, ptr, sizeof(T) );
	}

	template<typename T>
	static void eeprom_gently_write(const T& x, uint8_t* ptr)
	{
		eeprom_gently_write_block( (uint8_t*)&x, ptr, sizeof(T) );
	}
}

#endif
