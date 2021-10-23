###############################################################################
# Makefile for the project WaterPump
###############################################################################

## General Flags
PROJECT = WaterPump
MCU = atmega328p
F_CPU = 20000000
TARGET = WaterPump.elf
CC = avr-gcc.exe

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -DF_CPU=$(F_CPU)UL
CFLAGS += -Wall -gstabs  -O3 -fsigned-char -fshort-enums -std=gnu99
##CFLAGS += -Wa,-ahlns=$(<:.c=.lst)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map,WaterPump.map

## Flag to prevent the 'count leading zeros' table from consuming 256 bytes of RAM
LIBS = -lm


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

COMMON_CODE_DIR = C:/files/LightingUPS/firmware/CommonCode

## Include Directories
##INCLUDES = -I"C:\WinAVR-20100110\avr\include" -I"C:\WinAVR-20100110\avr\bin" -I".." -I"C:\files\LightingUPS\firmware\CommonCode"
INCLUDES = -I"C:\avr8-gnu-toolchain-win32_x86\avr\include" -I"C:\avr8-gnu-toolchain-win32_x86\avr\bin" -I".." -I"C:\files\LightingUPS\firmware\CommonCode"

## Objects that must be built in order to link
OBJECTS = WaterPump.o \
        Console.o CommandProcessor.o \
        SystemTime.o EEPROMStorage.o \
		WaterPumpControl.o TachometerOdometer.o LinearMotionControl.o \
        SystemTimeCommon.o ByteQueue.o DataHistory.o \
		CharString.o CharStringSpan.o StringScan.o StringInteger.o \
        EEPROM_Util.o PinChangeMonitor.o IOPortBitfield.o \
		UART_async.o \
        RamSentinel.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) WaterPump.hex WaterPump.eep size

## Compile
WaterPump.o: ../WaterPump.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Console.o: ../Console.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

CommandProcessor.o: ../CommandProcessor.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

SystemTime.o: ../SystemTime.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

EEPROMStorage.o: ../EEPROMStorage.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

WaterPumpControl.o: ../WaterPumpControl.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

TachometerOdometer.o: ../TachometerOdometer.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

LinearMotionControl.o: ../LinearMotionControl.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

SystemTimeCommon.o: $(COMMON_CODE_DIR)/SystemTimeCommon.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ByteQueue.o: $(COMMON_CODE_DIR)/ByteQueue.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

DataHistory.o: $(COMMON_CODE_DIR)/DataHistory.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

UART_async.o: $(COMMON_CODE_DIR)/UART_async.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

EEPROM_Util.o: $(COMMON_CODE_DIR)/EEPROM_Util.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

CharString.o: $(COMMON_CODE_DIR)/CharString.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

CharStringSpan.o: $(COMMON_CODE_DIR)/CharStringSpan.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

StringScan.o: $(COMMON_CODE_DIR)/StringScan.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

StringInteger.o: $(COMMON_CODE_DIR)/StringInteger.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

PinChangeMonitor.o: $(COMMON_CODE_DIR)/PinChangeMonitor.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

IOPortBitfield.o: $(COMMON_CODE_DIR)/IOPortBitfield.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

RamSentinel.o: $(COMMON_CODE_DIR)/RamSentinel.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) WaterPump.elf dep/* WaterPump.hex WaterPump.eep

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

