cmake_minimum_required(VERSION 3.12)

project(WaveLight VERSION 1.0 LANGUAGES C)

set(RPI_WS281X_DIR $HOME/rpi_ws281x CACHE PATH "Directory where ws2811.h and libws2811.a are" )

add_executable(WaveLight main.c)
target_include_directories(WaveLight PRIVATE ${RPI_WS281X_DIR})
target_link_directories(WaveLight PRIVATE ${RPI_WS281X_DIR})
target_link_libraries(WaveLight PRIVATE ws2811)

