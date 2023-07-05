#include <AvrTL.h>
#include <AvrTLPin.h>

extern avrtl::StaticPin<8> ws2811_pin; // pin 8 => PB0 on atmega328p
void ws2811_send_bytes_PB0( const uint8_t* s, uint16_t len);

