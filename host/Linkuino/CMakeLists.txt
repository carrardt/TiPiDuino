include_directories(../../libraries)

add_compile_options(-std=c++11 -fPIC) # in cmake 2.8.12 and later

add_library(LinkuinoClient STATIC LinkuinoClient.cpp LinkuinoSerialPort.cpp)

add_library(pd-linkuino MODULE pd-linkuino.cpp )
target_link_libraries(pd-linkuino LinkuinoClient m)

add_executable(linkuino linkuino_tool.cpp)
target_link_libraries(linkuino LinkuinoClient m)
