OBJCOPY = avr-objcopy
AVRDUDE = avrdude

SHELL=cmd

avr-files = avr/buttons.o avr/display.o avr/eep.o
common-files := common/*.h
console-files := console/buttons.o console/display.o console/eep.o

default: eeprom upload clean

%.o: %.c
	$(CC) -c $^ $(CCFLAGS)

pre-avr:
	$(eval CC = avr-gcc)
	$(eval MCU = atmega328p)
	$(eval CCFLAGS = -std=c99 -mmcu=$(MCU) -O2)

pre-console:
	$(eval CC = gcc)
	$(eval CCFLAGS = -O0 -Dconsole -g)

avr: pre-avr main.o $(avr-files)
	$(CC) main.o $(subst avr/,,$(avr-files)) $(CCFLAGS) -lm -o main.elf
	$(OBJCOPY) -O ihex main.elf main.hex
	
console: pre-console main.o $(console-files)
	$(CC) -o server/calc.exe main.o $(subst console/,,$(console-files))

eepromprog.hex: pre-avr eepromprog.o $(avr-files)
	$(CC) eepromprog.o $(CCFLAGS) $(subst avr/,,$(avr-files)) -o eepromprog.elf
	$(OBJCOPY) -O ihex eepromprog.elf eepromprog.hex

server/eepromdata.bin: eepromgen/*.png
	node eepromgen/makeEEPROMBin.js eepromgen/calcks.json server/eepromdata.bin

eeprom: server/eepromdata.bin eepromprog.hex
	$(AVRDUDE) -v -patmega328p -carduino -PCOM3 -b115200 -D -Uflash:w:"eepromprog.hex":i
	node binsend/transferdata.js server/eepromdata.bin

upload: avr
	$(AVRDUDE) -v -patmega328p -carduino -PCOM3 -b115200 -D -Uflash:w:"main.hex":i

clean:
	rm *.o
	rm *.elf
	rm *.hex