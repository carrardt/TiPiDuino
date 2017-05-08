         ESP8266 12-E
         ============

GND     -         - VCC
GPIO 15 -         - GPIO 13
GPIO  2 -         - GPIO 12
GPIO  0 -         - GPIO 14
GPIO  4 -         - GPIO 16
GPIO  5 -         - GPIO CHIP_EN / CH_PD
GPIO  3 -         - ADC
GPIO  1 -         - Reset
         __/\/\/\|

Normal Operation :
GPIO 15 => 0
GPIO  2 => 1
GPIO  0 => 1

UART Upload mode (Programming) :
GPIO 15 => 0
GPIO  2 => 1
GPIO  0 => 0

