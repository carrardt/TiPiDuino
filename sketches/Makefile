# where to find Wiring sources
WIRING=/media/picuntu/linuxdata/Wiring

# default board model
FAMILY=Atmel
BOARD=atm328p16MHz

# compilation flags
C_FLAGS=-O2 -Os -w
CXX_FLAGS=-O2 -Os -w
CXX=avr-g++

BOARDSDB=$(WIRING)/framework/hardware/$(FAMILY)/boards.txt
MCU=$(shell cat $(BOARDSDB) | grep "$(BOARD)\.build\.mcu"|cut -d'=' -f2)
CORE=$(shell cat $(WIRING)/framework/hardware/$(FAMILY)/boards.txt | grep "$(BOARD)\.build\.core"|cut -d'=' -f2)
F_CPU=$(shell cat $(WIRING)/framework/hardware/$(FAMILY)/boards.txt | grep "$(BOARD)\.build\.f_cpu"|cut -d'=' -f2)
COREDIR=$(WIRING)/framework/cores/$(CORE)
CORESRC=$(wildcard $(COREDIR)/*.cpp) $(wildcard $(COREDIR)/*.c)
CORECOMMONDIR=$(WIRING)/framework/cores/Common
CORECOMMONSRC=$(wildcard $(CORECOMMONDIR)/*.cpp) $(wildcard $(CORECOMMONDIR)/*.c)



%.elf: %.ino
	$(CXX) $(CXX_FLAGS) \
	-mmcu=$(MCU) -DF_CPU=$(F_CPU) \
	-I$(COREDIR) -I$(CORECOMMONDIR) \
	$< \
	$(CORECOMMONSRC) $(CORESRC)
	-o $@
