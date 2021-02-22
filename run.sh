#!/bin/bash

avr-gcc -std=c11 -mmcu=atmega328 -O -o avr.o avr.c

avr-objcopy -O ihex avr.o avr.hex 

avrdude -c usbtiny -p m328p -U flash:w:avr.hex
