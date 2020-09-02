OBJCOPY = avr-objcopy
AVRDUDE = avrdude

SHELL=cmd

avr-files = avr/buttons.o avr/display.o avr/eep.o
common-files = common/*.h
common-obj = common/gui.o common/term.o
console-files = console/buttons.o console/display.o console/eep.o

default: eeprom upload clean

%.o: %.c
	$(CC) -c $^ $(CCFLAGS)

pre-avr:
	$(eval CC = avr-gcc)
	$(eval MCU = atmega1284p)
	$(eval CCFLAGS += -std=c99 -mmcu=$(MCU) -O2)
	$(eval LDFLAGS = -Wl,-u,vfprintf -lprintf_flt -lm)

pre-console:
	$(eval CC = gcc)
	$(eval CCFLAGS = -O0 -Dconsole -g)

avr: pre-avr main.o $(avr-files) $(common-obj)
	$(CC) main.o $(subst avr/,,$(avr-files)) $(subst common/,,$(common-obj)) $(CCFLAGS) $(LDFLAGS) -lm -o main.elf
	$(OBJCOPY) -O ihex main.elf main.hex
	
console: pre-console main.o $(console-files) $(common-obj)
	$(CC) -o server/calc.exe main.o $(subst console/,,$(console-files)) $(subst common/,,$(common-obj))
	cd server & node server.js

eepromprog.hex: pre-avr eepromprog.o $(avr-files)
	$(CC) eepromprog.o $(CCFLAGS) $(subst avr/,,$(avr-files)) -o eepromprog.elf
	$(OBJCOPY) -O ihex eepromprog.elf eepromprog.hex

server/eepromdata.bin: eepromgen/*.png
	node eepromgen/makeEEPROMBin.js eepromgen/calcks.json server/eepromdata.bin

eeprom: server/eepromdata.bin
	$(AVRDUDE) -v -p atmega1284 -c arduino -P COM3 -b 57600 -U eeprom:w:"server/eepromdata.bin":r

upload: avr
	$(AVRDUDE) -v -p atmega1284 -c arduino -P COM3 -b 57600 -U flash:w:"main.hex":i

clean:
	rm *.o
	rm *.elf
	rm *.hex