# TiPiDuino
Arduino alike uController applications and libraries.
Purpose is to be able to use Arduino IDE (aka Wiring) libraries or standalone code without any overhead from the Wiring/Arduino libs.
now based on arduino-cmake

Contains :
  Signal generation and recording
  Lightweight hardware and software serial IO
  LCD1602 interface
  LCD12864 interface
  Lightweigth soft-serial supporting up to 57600 bauds
  Synchronous execution framework
  Linkuino, a PDuino replacement for accurate pwm generation with serial based communications
  and more ...

# Arduino cmake tips :
1. you need to install arduino package
2. ARDUINO_SDK_PATH is /usr/share/arduino
3. HARDWARE_PLATFORM_PATH is /usr/share/arduino/hardware/arduino
4. HARDWARE_PLATFORM_PATH may not contain boards.txt bootloaders cores programmers.txt variants
   as it should but an avr subdirectory instead, go to HARDWARE_PLATFORM_PATH and link avr/* to .
5. if needed, a custom boards.txt is available in misc/ArduinoIDE/boards.txt 

