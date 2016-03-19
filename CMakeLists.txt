set(ARDUINO_CMAKE_PATH "/home/pi/arduino-cmake" CACHE STRING "arduino-cmake root path")
set(CMAKE_TOOLCHAIN_FILE ${ARDUINO_CMAKE_PATH}/cmake/ArduinoToolchain.cmake) # Arduino Toolchain

cmake_minimum_required(VERSION 3.0)
project(TiDuino C CXX)

#print_board_list()
#print_programmer_list()

set(ARDUINO_DEFAULT_BOARD diecimila CACHE STRING "default board type") # Default Board ID, when not specified
set(ARDUINO_DEFAULT_PORT /dev/ttyUSB0 CACHE STRING "default serial port" ) # Default Port, when not specified

add_subdirectory(sketches)   #add the example directory into build
