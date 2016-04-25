include_directories(../../libraries)

add_compile_options(-std=c++11) # in cmake 2.8.12 and later

add_library(linkuino.pd SHARED pd-linkuino.cpp)

add_executable(linkuino-client linkuino-client.cpp)
target_link_libraries(linkuino-client m)
