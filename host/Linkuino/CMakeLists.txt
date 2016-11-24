cmake_minimum_required(VERSION 3.0)

add_compile_options(-std=c++11) # in cmake 2.8.12 and later

if( WIN32 OR WIN64 )
	find_file(PD_HEADER "m_pd.h")
	find_library(PD_LIBRARY pd DOC "puredata library" )
	get_filename_component(PD_INCLUDE_DIR "${PD_HEADER}" DIRECTORY)
	set(EXTERNAL_LIBS "${PD_LIBRARY}")
else()
	add_compile_options(-fPIC) 
	set(EXTERNAL_LIBS "m")
endif()

include_directories(../../libraries ${PD_INCLUDE_DIR})

add_library(LinkuinoClient STATIC LinkuinoClient.cpp LinkuinoSerialPort.cpp)

add_library(linkuino SHARED pd_linkuino_core.c pd_linkuino_int.cpp)
target_link_libraries(linkuino LinkuinoClient ${EXTERNAL_LIBS})

add_executable(linkuino_tool linkuino_tool.cpp)
target_link_libraries(linkuino_tool LinkuinoClient ${EXTERNAL_LIBS})
