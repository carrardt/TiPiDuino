#ifndef __TiDuino_AVRAPP_h
#define __TiDuino_AVRAPP_h

// some wiring compatibility tricks
extern void loop();
extern void setup();
int main(void) __attribute__ ((noreturn,OS_main,weak));

#endif
