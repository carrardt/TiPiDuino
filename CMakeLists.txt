# You need to install arduino IDE before building the code.

set(ARDUINO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" CACHE STRING "arduino-cmake root path")
set(CMAKE_TOOLCHAIN_FILE ${ARDUINO_CMAKE_PATH}/cmake/ArduinoToolchain.cmake) # Arduino Toolchain
set(ARDUINO_CXX_FLAGS "${ARDUINO_CXX_FLAGS} -std=c++11")

cmake_minimum_required(VERSION 3.0)
project(TiDuino C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS NO)

#message(STATUS "ARDUINO_SDK_PATH=${ARDUINO_SDK_PATH}")
#message(STATUS "HARDWARE_PLATFORM_PATH=${HARDWARE_PLATFORM_PATH}")

print_board_list()
print_programmer_list()

# set default board from list
foreach(PLATFORM ${ARDUINO_PLATFORMS})
	set(ARDUINO_BOARD_LIST ${ARDUINO_BOARD_LIST} ${${PLATFORM}_BOARDS})
endforeach()
set(ARDUINO_DEFAULT_BOARD nano328 CACHE STRING "Default Board") 
set_property(CACHE ARDUINO_DEFAULT_BOARD PROPERTY STRINGS ${ARDUINO_BOARD_LIST})

# set default programmer from list
foreach(PLATFORM ${ARDUINO_PLATFORMS})
	set(ARDUINO_PROGRAMMER_LIST ${ARDUINO_PROGRAMMER_LIST} ${${PLATFORM}_PROGRAMMERS})
endforeach()
list(GET ARDUINO_PROGRAMMER_LIST 0 ARDUINO_FIRST_PROGRAMMER)
set(ARDUINO_DEFAULT_BOARD "${ARDUINO_FIRST_PROGRAMMER}" CACHE STRING "Programmer type") 
set_property(CACHE ARDUINO_DEFAULT_PROGRAMMER PROPERTY STRINGS ${ARDUINO_PROGRAMMER_LIST})

# set serial port from list of available ports
file(GLOB AVAILABLE_SERIAL_PORTS "/dev/ttyUSB*" "/dev/ttyACM*" "/dev/ttyAMA*" "/dev/ttyS[1-9]")
list(GET AVAILABLE_SERIAL_PORTS 0 FIRST_SERIAL_PORT)
set(ARDUINO_DEFAULT_PORT "${FIRST_SERIAL_PORT}" CACHE STRING "Serial port")
set_property(CACHE ARDUINO_DEFAULT_PORT PROPERTY STRINGS ${AVAILABLE_SERIAL_PORTS})

# use cu as a serial terminal
set(ARDUINO_DEFAULT_SERIAL "cu -t -h -s 9600 -l @SERIAL_PORT@" CACHE STRING "Serial monitor command")

# add a macro for build timestamp
string(TIMESTAMP BUILD_TIMSESTAMP_DEFINITION "-DBUILD_TIMESTAMP=%Y%j%HUL")
add_definitions(${BUILD_TIMSESTAMP_DEFINITION})

# tell arduino-cmake where to find our libraries
link_directories(${CMAKE_SOURCE_DIR}/libraries)
include_directories(${CMAKE_SOURCE_DIR}/libraries)
#add_compile_options("-std=c++11")

# finally, compile home made libraries
add_subdirectory(libraries)

# and programs
add_subdirectory(sketches)
