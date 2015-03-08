# where to build
BUILD_DIR=obj

# where to find Wiring sources
WIRING_SRC_DIR=/media/picuntu/linuxdata/Wiring

# ETL template library for avr
ETL_ROOT=/media/picuntu/linuxdata/ETL

# where to find personal sketches
SKETCHES=/media/picuntu/linuxdata/TiPiDuino/sketches

# default board model
FAMILY=Arduino
BOARD=ArduinoUno

# cross compiler
CC=avr-gcc
CXX=avr-g++
AR=avr-ar
STRIP=avr-strip
OBJCOPY=avr-objcopy

# flash tool
AVRDUDE=avrdude
SERIALPORT=/dev/ttyACM0

# Deduced properties from board database
BOARDSDB=$(WIRING_SRC_DIR)/framework/hardware/$(FAMILY)/boards.txt
MCU=$(shell cat $(BOARDSDB) | grep "$(BOARD)\.build\.mcu"|cut -d'=' -f2)
CORE=$(shell cat $(WIRING_SRC_DIR)/framework/hardware/$(FAMILY)/boards.txt | grep "$(BOARD)\.build\.core"|cut -d'=' -f2)
F_CPU=$(shell cat $(WIRING_SRC_DIR)/framework/hardware/$(FAMILY)/boards.txt | grep "$(BOARD)\.build\.f_cpu"|cut -d'=' -f2)
HARDWARE=$(shell cat $(WIRING_SRC_DIR)/framework/hardware/$(FAMILY)/boards.txt | grep "$(BOARD)\.build\.hardware"|cut -d'=' -f2)
PROTOCOL=$(shell cat $(WIRING_SRC_DIR)/framework/hardware/$(FAMILY)/boards.txt | grep "$(BOARD)\.upload\.protocol"|cut -d'=' -f2)
PART=$(shell $AVRDUDE -c $PROTOCOL 2>&1|grep -i atmega328p|tr -d ' '|cut -d'=' -f1)

# Compile options
MCU_FLAGS=-mmcu=$(MCU)
COMMON_FLAGS=$(MCU_FLAGS) -DF_CPU=$(F_CPU)
C_FLAGS=-O3 -Os -std=gnu11 $(COMMON_FLAGS)
CXX_FLAGS=-O3 -Os -std=c++1y $(COMMON_FLAGS)
LD_FLAGS=-Os $(MCU_FLAGS)
STRIP_FLAGS=-s -x -X
OBJCOPY_ELF2HEX_FLAGS=-O ihex -R .eeprom

# Wiring core and additional libraries
WIRING_LIBNAME=Wiring-$(FAMILY)-$(BOARD)
WIRING_LIB=$(BUILD_DIR)/lib$(WIRING_LIBNAME).a
WIRING_HW_DIR=$(WIRING_SRC_DIR)/framework/hardware/$(FAMILY)/$(HARDWARE)
WIRING_CORE_DIR=$(WIRING_SRC_DIR)/framework/cores/$(CORE)
WIRING_COMMON_DIR=$(WIRING_SRC_DIR)/framework/cores/Common
WIRING_LIBRARIES_DIR=$(WIRING_SRC_DIR)/framework/libraries
WIRING_DIRS=$(WIRING_HW_DIR) $(WIRING_CORE_DIR) $(WIRING_COMMON_DIR) $(WIRING_LIBRARIES_DIR)
WIRING_SRC_C=$(shell find $(WIRING_DIRS) -name "*.c")
WIRING_SRC_CPP=$(shell find $(WIRING_DIRS) -name "*.cpp")
WIRING_SRC_H=$(shell find $(WIRING_DIRS) -name "*.h")
WIRING_INC_DIRS=$(sort $(dir $(WIRING_SRC_H)))
WIRING_INC_FLAGS=$(addprefix -I,$(WIRING_INC_DIRS))
WIRING_OBJS=$(subst $(WIRING_SRC_DIR),$(BUILD_DIR),$(patsubst %.c,%.wiring.o,$(WIRING_SRC_C)) $(patsubst %.cpp,%.wiring.o,$(WIRING_SRC_CPP)))
WIRING_OBJS_DIRS=$(sort $(dir $(WIRING_OBJS)))

# User libraries
USER_LIBNAME=User-$(FAMILY)-$(BOARD)
USER_LIB=$(BUILD_DIR)/lib$(USER_LIBNAME).a
USER_LIBRARIES_DIR=$(SKETCHES)/libraries
USER_LIBRARIES_SRC_C=$(shell find $(USER_LIBRARIES_DIR) -name "*.c")
USER_LIBRARIES_SRC_CPP=$(shell find $(USER_LIBRARIES_DIR) -name "*.cpp")
USER_LIBRARIES_SRC_H=$(shell find $(USER_LIBRARIES_DIR) -name "*.h")
USER_LIBRARIES_INC_DIRS=$(sort $(dir $(USER_LIBRARIES_SRC_H)))
USER_LIBRARIES_INC_FLAGS=$(addprefix -I,$(USER_LIBRARIES_INC_DIRS))
USER_LIBRARIES_OBJS=$(subst $(SKETCHES),$(BUILD_DIR),$(patsubst %.c,%.user.o,$(USER_LIBRARIES_SRC_C)) $(patsubst %.cpp,%.user.o,$(USER_LIBRARIES_SRC_CPP)))
USER_LIBRARIES_OBJS_DIRS=$(sort $(dir $(USER_LIBRARIES_OBJS)))

# Ambroise Leclerc's libstdc++ for AVRs
ETL_CXX_FLAGS=-I$(ETL_ROOT)/include -I$(ETL_ROOT)/libstd/include

wiring: $(WIRING_LIB)
user: $(USER_LIB)
#exemples: $(WIRING_EXEMPLES)

%.flash: %.hex
	$(AVRDUDE) -c $(PROTOCOL) -p $(MCU) -P $(SERIALPORT) -D -V -U flash:w:$<:i 

%.hex: $(BUILD_DIR)/%.user.elf
	$(OBJCOPY) $(OBJCOPY_ELF2HEX_FLAGS) $< $@

$(BUILD_DIR)/%.user.elf: $(BUILD_DIR)/%.user.o $(USER_LIB) $(WIRING_LIB)
	$(CXX) $(LD_FLAGS) $< -L$(BUILD_DIR) -l$(USER_LIBNAME) -l$(WIRING_LIBNAME) -o $@
	$(STRIP) $(STRIP_FLAGS) $@

$(USER_LIB): $(USER_LIBRARIES_OBJS)
	$(AR) rcs $@ $?

$(WIRING_LIB): $(WIRING_OBJS) 
	$(AR) rcs $@ $?

$(USER_LIBRARIES_OBJS): | MakeUserBuildDirs

$(WIRING_OBJS): | MakeWiringBuildDirs

MakeWiringBuildDirs:
	mkdir -p $(WIRING_OBJS_DIRS)

MakeUserBuildDirs:
	mkdir -p $(USER_LIBRARIES_OBJS_DIRS)

$(BUILD_DIR)/%.wiring.o: $(WIRING_SRC_DIR)/%.c
	$(CC) $(C_FLAGS) $(WIRING_INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.wiring.o: $(WIRING_SRC_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) $(WIRING_INC_FLAGS) -c $< -o $@

#$(BUILD_DIR)/%.wiring.o: $(WIRING_SRC_DIR)/%.pde
#	$(CXX) $(CXX_FLAGS) -xc++ $(WIRING_INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.user.o: $(SKETCHES)/%.c
	$(CC) $(C_FLAGS) $(USER_LIBRARIES_INC_FLAGS) $(WIRING_INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.user.o: $(SKETCHES)/%.cpp
	$(CXX) $(CXX_FLAGS) $(USER_LIBRARIES_INC_FLAGS) $(WIRING_INC_FLAGS) $(ETL_CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.user.o: $(SKETCHES)/%.ino
	$(CXX) $(CXX_FLAGS) -xc++ $(USER_LIBRARIES_INC_FLAGS) $(WIRING_INC_FLAGS) $(ETL_CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.user.o: $(SKETCHES)/%.pde
	$(CXX) $(CXX_FLAGS) -xc++ $(USER_LIBRARIES_INC_FLAGS) $(WIRING_INC_FLAGS) -c $< -o $@

