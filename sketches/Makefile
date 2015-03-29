# where to build
BUILD_DIR=obj

# where to find Wiring sources
WIRING_SRC_DIR=$(HOME)/dev/Wiring

# ETL template library for avr
ETL_ROOT=$(HOME)/dev/ETL

# where to find personal sketches
SKETCHES=$(HOME)/dev/TiPiDuino/sketches

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
UPLOADBR=$(shell cat $(WIRING_SRC_DIR)/framework/hardware/$(FAMILY)/boards.txt | grep "$(BOARD)\.upload\.speed"|cut -d'=' -f2)
PART=$(shell $AVRDUDE -c $PROTOCOL 2>&1|grep -i "= $(MCU)"|tr -d ' '|cut -d'=' -f1)
VARIANT=$(FAMILY)-$(BOARD)

# Compile options
MCU_FLAGS=-mmcu=$(MCU)
COMMON_FLAGS=$(MCU_FLAGS) -DF_CPU=$(F_CPU)
C_FLAGS=-O3 -Os $(COMMON_FLAGS)
CXX_FLAGS=-std=c++1y -O3 -Os $(COMMON_FLAGS)
LD_FLAGS=-std=c++1y -Os $(MCU_FLAGS)
STRIP_FLAGS=-s -x -X
OBJCOPY_ELF2HEX_FLAGS=-O ihex -R .eeprom

# Wiring core and additional libraries
WIRING_LIBNAME=Wiring-$(VARIANT)
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
WIRING_OBJS=$(subst $(WIRING_SRC_DIR),$(BUILD_DIR),$(patsubst %.c,%.wiring.$(VARIANT).o,$(WIRING_SRC_C)) $(patsubst %.cpp,%.wiring.$(VARIANT).o,$(WIRING_SRC_CPP)))
WIRING_OBJS_DIRS=$(sort $(dir $(WIRING_OBJS)))

# User libraries
USER_LIBNAME=User-$(VARIANT)
USER_LIB=$(BUILD_DIR)/lib$(USER_LIBNAME).a
USER_LIBRARIES_DIR=$(SKETCHES)/libraries
USER_LIBRARIES_SRC_C=$(shell find $(USER_LIBRARIES_DIR) -name "*.c")
USER_LIBRARIES_SRC_CPP=$(shell find $(USER_LIBRARIES_DIR) -name "*.cpp")
USER_LIBRARIES_SRC_H=$(shell find $(USER_LIBRARIES_DIR) -name "*.h")
USER_LIBRARIES_INC_DIRS=$(sort $(dir $(USER_LIBRARIES_SRC_H)))
USER_LIBRARIES_INC_FLAGS=$(addprefix -I,$(USER_LIBRARIES_INC_DIRS))
USER_LIBRARIES_OBJS=$(subst $(SKETCHES),$(BUILD_DIR),$(patsubst %.c,%.user.$(VARIANT).o,$(USER_LIBRARIES_SRC_C)) $(patsubst %.cpp,%.user.$(VARIANT).o,$(USER_LIBRARIES_SRC_CPP)))
USER_LIBRARIES_OBJS_DIRS=$(sort $(dir $(USER_LIBRARIES_OBJS)))

# Ambroise Leclerc's libstdc++ for AVRs
ETL_CXX_FLAGS=-I$(ETL_ROOT)/include -I$(ETL_ROOT)/libstd/include

wiring: $(WIRING_LIB)
user: $(USER_LIB)
#exemples: $(WIRING_EXEMPLES)

%.flash: %.hex
	$(AVRDUDE) -c $(PROTOCOL) -p $(MCU) -P $(SERIALPORT) -b $(UPLOADBR) -D -U flash:w:$<:i 

%.hex: $(BUILD_DIR)/%.sketch.$(VARIANT).strip.elf
	$(OBJCOPY) $(OBJCOPY_ELF2HEX_FLAGS) $< $@

$(BUILD_DIR)/%.sketch.$(VARIANT).strip.elf: $(BUILD_DIR)/%.sketch.$(VARIANT).elf
	$(STRIP) $(STRIP_FLAGS) $< -o $@

#$(WIRING_LIB)
$(BUILD_DIR)/%.sketch.$(VARIANT).elf: $(BUILD_DIR)/%.sketch.$(VARIANT).o $(USER_LIB)
	$(CXX) $(LD_FLAGS) $< -L$(BUILD_DIR) -l$(USER_LIBNAME) -o $@
	#-l$(WIRING_LIBNAME)


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

# compilation des sources wiring
$(BUILD_DIR)/%.wiring.$(VARIANT).o: $(WIRING_SRC_DIR)/%.c
	$(CC) $(C_FLAGS) $(WIRING_INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.wiring.$(VARIANT).o: $(WIRING_SRC_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) $(WIRING_INC_FLAGS) -c $< -o $@

# cmpilation des bibliothèques utilisateur
$(BUILD_DIR)/libraries/%.user.$(VARIANT).o: $(USER_LIBRARIES_DIR)/%.c
	$(CC) $(C_FLAGS) $(USER_LIBRARIES_INC_FLAGS) $(WIRING_INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/libraries/%.user.$(VARIANT).o: $(USER_LIBRARIES_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) $(USER_LIBRARIES_INC_FLAGS) $(WIRING_INC_FLAGS) -c $< -o $@

# compilation des programmes
$(BUILD_DIR)/%.sketch.$(VARIANT).o: $(SKETCHES)/%.c
	$(CC) $(C_FLAGS) $(USER_LIBRARIES_INC_FLAGS) $(WIRING_INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.sketch.$(VARIANT).o: $(SKETCHES)/%.cpp
	$(CXX) $(CXX_FLAGS) $(USER_LIBRARIES_INC_FLAGS) $(WIRING_INC_FLAGS) $(ETL_CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.sketch.$(VARIANT).o: $(BUILD_DIR)/%.sketch.cpp
	$(CXX) $(CXX_FLAGS) $(USER_LIBRARIES_INC_FLAGS) $(WIRING_INC_FLAGS) $(ETL_CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.sketch.cpp: %.pde
	echo "#include <Wiring.h>" > $@
	cat $< >> $@

$(BUILD_DIR)/%.sketch.cpp: %.ino
	echo "#include <Wiring.h>" > $@
	cat $< >> $@
