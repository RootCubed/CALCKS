# CALCKS
An open-source calculator I made for a school finals project.

## Hardware
All hardware files to make your own CALCKS are in this repository. The PCB design files can be opened with [KiCad](https://kicad.org/). The case can be 3D printed with the files found in <Hardware/3D Models>.

## Building
To compile this project for AVR architecture, you will need the GCC suite for AVR. For Windows, I used [WinAVR](http://winavr.sourceforge.net/).
`cd` into the Firmware directory of your locally cloned repository and run `make eeprom upload`. You might need to adjust the used COM port in the Makefile to get avrdude working.