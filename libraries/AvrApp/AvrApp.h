#pragma once

// some wiring compatibility tricks
extern void loop();
extern void setup();
int main(void) __attribute__ ((OS_main,weak));

