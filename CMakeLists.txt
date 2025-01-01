# I target a recent cmake, it shouldn't be a problem on a dev machine

cmake_minimum_required(VERSION 3.26)
project(TiDuino C CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS NO)

# Use AVR GCC toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

# add a macro for build timestamp
string(TIMESTAMP BUILD_TIMSESTAMP "%Y%j%HUL")

macro(add_avr_board BOARDNAME F_CPU MCU BAUD PROG_TYPE E_FUSE H_FUSE L_FUSE LOCK_BIT)

  set(${BOARDNAME}_F_CPU ${F_CPU}) # 

  # CPU, you can find the list here:
  # https://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
  set(${BOARDNAME}_MCU ${MCU}) # atmega328p

  # Default Baudrate for UART, read avr include/util/setbaud.h for usage
  set(${BOARDNAME}_BAUD ${BAUD}) # 9600

  # The programmer to use, read avrdude manual for list
  set(${BOARDNAME}_PROG_TYPE ${PROG_TYPE}) # avrispmkII
  
  # AVR Fuses, must be in concordance with your hardware and F_CPU
  # http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p
  set(${BOARDNAME}_E_FUSE ${E_FUSE}) # 0xfd
  set(${BOARDNAME}_H_FUSE ${H_FUSE}) # 0xda
  set(${BOARDNAME}_L_FUSE ${L_FUSE}) # 0xfd
  set(${BOARDNAME}_LOCK_BIT ${LOCK_BIT}) # 0xff
endmacro()


# add a program firmware
function(generate_avr_firmware SKETCH_NAME)

  set(options)
  set(oneValueArgs)
  set(multiValueArgs SRCS LIBS BOARDS)
  cmake_parse_arguments(PARSE_ARGV 0 ${SKETCH_NAME} "${options}" "${oneValueArgs}" "${multiValueArgs}")
  message(STATUS "sketch ${SKETCH_NAME} : SRCS=${${SKETCH_NAME}_SRCS} , LIBS=${${SKETCH_NAME}_LIBS} , BOARDS=${${SKETCH_NAME}_BOARDS}")

  foreach(BOARD ${${SKETCH_NAME}_BOARDS})

    message(STATUS "Build ${SKETCH_NAME}_${BOARD} : F_CPU=${${BOARD}_F_CPU} BAUD=${${BOARD}_BAUD} MCU=${${BOARD}_MCU} L_FUSE=${${BOARD}_L_FUSE} H_FUSE=${${BOARD}_H_FUSE} E_FUSE=${${BOARD}_E_FUSE} LOCK_BIT=${${BOARD}_LOCK_BIT}")

    # Create one target
    add_executable(${SKETCH_NAME}_${BOARD} ${${SKETCH_NAME}_SRCS})

    target_compile_definitions(${SKETCH_NAME}_${BOARD} PRIVATE F_CPU=${${BOARD}_F_CPU} BAUD=${${BOARD}_BAUD} BUILD_TIMSESTAMP=${BUILD_TIMSESTAMP})
    target_compile_options(${SKETCH_NAME}_${BOARD} PRIVATE
        -mmcu=${${BOARD}_MCU} # MCU
        -std=c++11 # C++ dialect
        -Os # optimize
        -Wall # enable warnings
        -Wno-main
        -Wundef
        -Werror
        -Wfatal-errors
        -Wl,--relax,--gc-sections
        -fshort-enums
        -ffunction-sections
        -fdata-sections
        -fno-split-wide-types
        -fno-tree-scev-cprop
        )
    target_include_directories(${SKETCH_NAME}_${BOARD} PRIVATE ${PROJECT_SOURCE_DIR}/libraries)

    target_link_libraries(${SKETCH_NAME}_${BOARD} PRIVATE ${${SKETCH_NAME}_LIBS})

    add_custom_target(${SKETCH_NAME}_${BOARD}_strip COMMAND avr-strip ${SKETCH_NAME}_${BOARD} -o ${SKETCH_NAME}_${BOARD}.elf DEPENDS ${SKETCH_NAME}_${BOARD} BYPRODUCTS ${SKETCH_NAME}_${BOARD}.elf)
    add_custom_target(${SKETCH_NAME}_${BOARD}_hex COMMAND avr-objcopy -R .eeprom -O ihex ${SKETCH_NAME}_${BOARD}.elf ${SKETCH_NAME}_${BOARD}.hex DEPENDS ${SKETCH_NAME}_${BOARD}_strip BYPRODUCTS ${SKETCH_NAME}_${BOARD}.hex)
    add_custom_target(${SKETCH_NAME}_${BOARD}_eeprom COMMAND avr-objcopy -j .eeprom  --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex ${SKETCH_NAME}_${BOARD}.elf ${SKETCH_NAME}_${BOARD}.eep DEPENDS ${SKETCH_NAME}_${BOARD}_strip)

    # Upload the firmware with avrdude
    add_custom_target(${SKETCH_NAME}_${BOARD}_upload
      COMMAND avrdude -c ${${BOARD}_PROG_TYPE} -p ${${BOARD}_MCU} -U flash:w:${SKETCH_NAME}_${BOARD}.hex -U eeprom:w:${SKETCH_NAME}_${BOARD}.eep -U lfuse:w:${${BOARD}_L_FUSE}:m -U hfuse:w:${${BOARD}_H_FUSE}:m -U efuse:w:${${BOARD}_E_FUSE}:m -U lock:w:${${BOARD}_LOCK_BIT}:m
      DEPENDS ${SKETCH_NAME}_${BOARD}_hex ${SKETCH_NAME}_${BOARD}_eeprom)

  endforeach()

endfunction()

function(generate_avr_library LIBRARY_NAME)

  set(options MANUAL NO_AUTOLIBS)
  set(oneValueArgs)
  set(multiValueArgs RCS HDRS LIBS BOARDS)
  cmake_parse_arguments(PARSE_ARGV 0 ${LIBRARY_NAME} "${options}" "${oneValueArgs}" "${multiValueArgs}")
  message(STATUS "library ${LIBRARY_NAME} : SRCS=${${LIBRARY_NAME}_SRCS} , HDRS=${${LIBRARY_NAME}_HDRS} , LIBS=${${LIBRARY_NAME}_LIBS} , BOARDS=${${LIBRARY_NAME}_BOARDS} , MANUAL=${${LIBRARY_NAME}_MANUAL}, NO_AUTOLIBS=${${LIBRARY_NAME}_NO_AUTOLIBS}")

  # Create one target
  foreach(BOARD ${${LIBRARY_NAME}_BOARDS})

    if(${LIBRARY_NAME}_SRCS)
      message(STATUS "add ${LIBRARY_NAME}_${BOARD} library : F_CPU=${${BOARD}_F_CPU} BAUD=${${BOARD}_BAUD} MCU=${${BOARD}_MCU} L_FUSE=${${BOARD}_L_FUSE} H_FUSE=${${BOARD}_H_FUSE} E_FUSE=${${BOARD}_E_FUSE} LOCK_BIT=${${BOARD}_LOCK_BIT}")
      add_library(${LIBRARY_NAME}_${BOARD} STATIC ${${LIBRARY_NAME}_SRCS})
      target_compile_definitions(${LIBRARY_NAME}_${BOARD} PRIVATE F_CPU=${${BOARD}_F_CPU} BAUD=${${BOARD}_BAUD} BUILD_TIMSESTAMP=${BUILD_TIMSESTAMP})
      target_compile_options(${LIBRARY_NAME}_${BOARD} PUBLIC
          -mmcu=${${BOARD}_MCU} # MCU
          -std=c++11 # C++ dialect
          -Os # optimize
          -Wall # enable warnings
          -Wno-main
          -Wundef
          -Werror
          -Wfatal-errors
          -Wl,--relax,--gc-sections
          -fshort-enums
          -ffunction-sections
          -fdata-sections
          -fno-split-wide-types
          -fno-tree-scev-cprop
          )
      target_link_libraries(${LIBRARY_NAME}_${BOARD} PUBLIC ${${LIBRARY_NAME}_LIBS})
    else()
      message(STATUS "add ${LIBRARY_NAME}_${BOARD} interface : F_CPU=${${BOARD}_F_CPU} BAUD=${${BOARD}_BAUD} MCU=${${BOARD}_MCU} L_FUSE=${${BOARD}_L_FUSE} H_FUSE=${${BOARD}_H_FUSE} E_FUSE=${${BOARD}_E_FUSE} LOCK_BIT=${${BOARD}_LOCK_BIT}")
      add_library(${LIBRARY_NAME}_${BOARD} INTERFACE)
      target_link_libraries(${LIBRARY_NAME}_${BOARD} INTERFACE ${${LIBRARY_NAME}_LIBS})
    endif()

  endforeach()

endfunction()

add_avr_board(arduino 16000000UL m328p 19200 stk500v1 0x05 0xDA 0xFF 0x3F)
add_avr_board(nano 16000000UL atmega328p 57600 arduino 0x05 0xDA 0xFF 0x3F)
add_avr_board(attiny84 8000000UL attiny84 19200 stk500v1 0xff 0xdf 0xe2 0x3F)
add_avr_board(attiny85 8000000UL attiny85 19200 stk500v1 0xff 0xdf 0xe2 0x3F)

# finally, compile home made libraries
add_subdirectory(libraries)

# and programs
add_subdirectory(sketches)
